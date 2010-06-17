/****************************************************************************/
/*                            Conexant Systems                              */
/****************************************************************************/
/*                                                                          */
/* Filename:       e2pdrv.c                                                 */
/*                                                                          */
/* Description:    E2PROM driver source file.                               */
/*                                                                          */
/* Author:         Senthil Veluswamy                                        */
/*                                                                          */
/* Date:           05/28/99                                                 */
/*                                                                          */
/* Copyright Conexant Systems, 1999                                         */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/

/****************************************************************************
 * $Header: e2pdrv.c, 71, 5/6/04 5:21:52 PM, Miles Bintz$
 ****************************************************************************/

/******************/
/* Include Files  */
/******************/
#include "e2prom.h"

/***********/
/* Globals */
/***********/
u_int8 e2p_cache[MAX_EEPROM_SIZE];  // Will be used to cache data before
                                    // writing to E2P.
                                    // local E2P Header storage

#if EEPROM_TYPE == EEPROM_512B
   #define E2P_PAGE_SIZE    16
#else 
   #define E2P_PAGE_SIZE    64
#endif

#define E2P_NUMPAGES  (MAX_EEPROM_SIZE / E2P_PAGE_SIZE)

u_int32 e2p_page_size = E2P_PAGE_SIZE;    // Write page size for E2P.
u_int32 e2p_num_pages = E2P_NUMPAGES;

bool bDataWriteBackPage[E2P_NUMPAGES];


p_E2P_Header_Info e2p_cache_h_ptr;  // Pointer to Header data in Cache.

u_int16 client_data_offset[ROM_CONFIG_NUM_CLIENTS];
                                    // Client data offsets from E2P base.

sem_id_t sem_writeback_mutex;       // synchronize the tasks accessing the
                                    // write_back word

sem_id_t sem_task_control;          // prevent the write task from looping
                                    // when there are no write requests.

/***************/
/* Extern data */
/***************/
extern bool nv_init_done;           // Was the driver initalized?

/************************/
/* Function definitions */
/************************/

/********************************************************************/
/*  e2p_present()                                                   */
/*                                                                  */
/*  PARAMETERS: void                                                */
/*                                                                  */
/*  DESCRIPTION:Determine whether EEPROM is present.                */
/*                                                                  */
/*  RETURNS:    TRUE: if EEPROM Present.                            */
/*              FALSE: otherwise                                    */
/********************************************************************/
bool e2p_present(void){
    bool present=FALSE;

    // Check if E2P is present.
    present = iicAddressTest(I2C_ADDR_EEPROM1, I2C_BUS_EEPROM1, FALSE);

    return present;
}

/********************************************************************/
/*  e2p_init()                                                      */
/*                                                                  */
/*  PARAMETERS: None.                                               */
/*                                                                  */
/*  DESCRIPTION:This function is called to Initialize the E2P, only */
/*              after the KAL is up and running.                    */
/*              This creates an E2P Task and Resouces needed for the*/
/*              driver to function                                  */
/*              It also checks whether there is valid config info   */
/*              stored in the EEPROM. It then copies E2P data into  */
/*              a global storage buf This then acts as a cached     */
/*              resource for fast E2P data access. If there is no   */
/*              valid E2P data, E2P Cache and the actual E2P device */
/*              are initialized to prepare for future data/access.  */
/*  RETURNS:    TRUE if initialization successful.                  */
/*              FALSE if not.                                       */
/********************************************************************/
bool e2p_init(void){

    bool present = FALSE;           // Is the E2P Present?

    task_id_t pid;                  // stores the pid of a new created task.

    int32 retcode,                  // return codes.
        i,                          // counter.
        num_bytes_deleted=0,        // To remove invalid client data
        write_start_page = 0,       // to update writeback word
        write_end_page = 0,         // to update writeback word
        writeback_client_start=-1;  // to update e2p device.

    u_int8 checksum_store;          // to verify the checksum.

    // Check if E2P is present.
    present = iicAddressTest(I2C_ADDR_EEPROM1, I2C_BUS_EEPROM1, FALSE);

    if(!present){
        trace_new(TRACE_E2P | TRACE_LEVEL_3, "E2PInit: E2P not Present!");
        return FALSE;
    }

    trace_new(TRACE_E2P | TRACE_LEVEL_2, "E2PInit: E2P Present.\n");

    // Initialize the device
    memset(e2p_cache, 0, sizeof(e2p_cache));

    #if EEPROM_TYPE == EEPROM_512B

        /* Older 512B eeprom */
        /* Get and check header to see if E2P has valid data. */
        retcode = read_data(HEADER_BASE_ADD,
               (u_int16)sizeof(E2P_Header_Info), e2p_cache+HEADER_BASE_ADD);

        if(retcode!=RC_OK){    // Error reading from E2P!
            trace_new(TRACE_E2P | TRACE_LEVEL_3, \
                    "E2PInit: Error reading Header! Retcode=%d.\n", retcode);
            error_log(ERROR_WARNING | RC_E2P_ERR | 0x1);
            return FALSE;
        }

    #else /* EEPROM_TYPE */

        /* Newer 16 or 32K EEproms */
        #ifdef DRIVER_INCL_EEPROM
           retcode = ee_init();
           if(!retcode){
               trace_new(TRACE_E2P | TRACE_LEVEL_3, \
                                               "E2PInit: EE Init Failed!\n");
               return FALSE;
           }

           retcode = ee_read(HEADER_BASE_ADD, e2p_cache+HEADER_BASE_ADD,
                   (u_int16)sizeof(E2P_Header_Info), (void*)NULL);
           if(retcode!=sizeof(E2P_Header_Info)){
               trace_new(TRACE_E2P | TRACE_LEVEL_3, \
                   "E2PInit: Error reading Header! #Bytes Read(%d)=%d.\n", \
                           retcode, sizeof(E2P_Header_Info));
               error_log(ERROR_WARNING | RC_E2P_ERR | 0x2);
               return FALSE;
           }
       #else // DRIVER_INCL_EEPROM
           trace_new(TRACE_E2P | TRACE_LEVEL_3, "Attempt to use \"ee_read\"!\n");
           error_log(ERROR_FATAL | RC_E2P_ERR | 0x3);
           return FALSE;
       #endif // DRIVER_INCL_EEPROM

    #endif  /* EEPROM_TYPE */

    trace_new(TRACE_E2P | TRACE_LEVEL_1, "E2PInit: Header read.\n");

    // Initialize the header structure pointer.
    e2p_cache_h_ptr = (p_E2P_Header_Info)(e2p_cache+HEADER_BASE_ADD);
                                       // Header starts at the Base.

    trace_new(TRACE_E2P | TRACE_LEVEL_2, "E2PInit: Checking E2P.\n");

    // Check for Magic Number. Presence of "Magic Number" indicates presence of
    // valid data.

    if(e2p_cache_h_ptr->Magic_Number!=(u_int32)E2P_MAGIC_NUM){
        // E2P does not store data. Write header to prepare for data later.
        trace_new(TRACE_E2P | TRACE_LEVEL_2, "E2PInit: No data in E2P.\n");

        e2p_cache_h_ptr->Magic_Number = (u_int32)E2P_MAGIC_NUM;
        for(i=ROM_CONFIG_NUM_CLIENTS-1;i>=0;i--){ // No Client data.
            e2p_cache_h_ptr->Client_Info[i].Num_Bytes = 0;
            e2p_cache_h_ptr->Client_Info[i].CheckSum = 0;
        }

        trace_new(TRACE_E2P | TRACE_LEVEL_2, \
                            "E2PInit: Initialized header data in cache.\n");

        // Initialize the Client Offsets array
        for(i=ROM_CONFIG_NUM_CLIENTS-1;i>=0;i--){ // No Client data.
            client_data_offset[i] = (u_int16)sizeof(E2P_Header_Info);
        }

        trace_new(TRACE_E2P | TRACE_LEVEL_2, \
                                    "E2PInit: Initialized data offsets.\n");

        // Write writeback_word to request task to writebak header to e2p dev.
        write_start_page = (HEADER_BASE_ADD/e2p_page_size);

        write_end_page =
                (HEADER_BASE_ADD+sizeof(E2P_Header_Info))/e2p_page_size;

        for(i = write_start_page; i <= write_end_page; i++)
        {
            bDataWriteBackPage[i] = TRUE;
        }

        trace_new(TRACE_E2P | TRACE_LEVEL_2, \
                            "E2PInit: Wrote writeback word for header.\n");
    }
    else{   // Valid data present in E2P.
            // Read the data to cache and update Offsets
        trace_new(TRACE_E2P | TRACE_LEVEL_2, "E2PInit: Magic# Present.\n");

        // Update local client offset table
        client_data_offset[0] = sizeof(E2P_Header_Info);
                                    // Account for the header

        // If client has data Read client data into cache
        // check params before calling read
        if((u_int16)e2p_cache_h_ptr->Client_Info[0].Num_Bytes>MAX_EEPROM_SIZE){
            trace_new(TRACE_E2P | TRACE_LEVEL_3, \
            "E2PInit: NumBytes wrong for Client#=0, Value=%x.\n", \
                            e2p_cache_h_ptr->Client_Info[0].Num_Bytes);
            error_log(ERROR_WARNING | RC_E2P_ERR | 0x4);
            e2p_erase();
//            return FALSE;
        }

        if(e2p_cache_h_ptr->Client_Info[0].Num_Bytes>0){
            // read and store the bytes in cache
            #if EEPROM_TYPE == EEPROM_512B
                retcode = read_data(client_data_offset[0]+num_bytes_deleted,
                        (u_int16)e2p_cache_h_ptr->Client_Info[0].Num_Bytes, \
                                        e2p_cache+client_data_offset[0]);

                // check for errors
                if(retcode!=RC_OK){
                    trace_new(TRACE_E2P | TRACE_LEVEL_3, \
                    "E2PInit: Read failed. Client#=0, Retcode=%d.\n", retcode);
                    error_log(ERROR_WARNING | RC_E2P_ERR | 0x5);
                    return FALSE;
                }
            #else /* EEPROM_TYPE */
                #ifdef DRIVER_INCL_EEPROM
                retcode = ee_read(client_data_offset[0]+num_bytes_deleted,
                                e2p_cache+client_data_offset[0],
            (u_int16)e2p_cache_h_ptr->Client_Info[0].Num_Bytes, (void*)NULL);
                if(retcode!=(u_int16)e2p_cache_h_ptr->Client_Info[0].Num_Bytes){
                    trace_new(TRACE_E2P | TRACE_LEVEL_3, \
                    "E2PInit: Read failed. Client#=0, #Bytes Read(%d)=%d.\n", \
                            retcode, e2p_cache_h_ptr->Client_Info[0].Num_Bytes);
                    error_log(ERROR_WARNING | RC_E2P_ERR | 0x6);
                    return FALSE;
                }
                #else /* DRIVER_INCL_EEPROM */
                trace_new(TRACE_E2P | TRACE_LEVEL_3, \
                                            "Attempt to use \"ee_read\"!\n");
                error_log(ERROR_FATAL | RC_E2P_ERR | 0x7);
                return FALSE;
                #endif /* DRIVER_INCL_EEPROM */
            #endif /* EEPROM_TYPE */

            // verify data
            checksum_store = get_checksum(client_data_offset[0],
                    (u_int16)e2p_cache_h_ptr->Client_Info[0].Num_Bytes, FALSE);

            checksum_store += (u_int8)e2p_cache_h_ptr->Client_Info[0].CheckSum;

            if(checksum_store!=0){  // Checksum failed.
                trace_new(TRACE_E2P | TRACE_LEVEL_3, \
                                "E2PInit: Client=0, CheckSum Err!\n");

                // Client data invalid. So initialize the data/header and
                // write back updated data to e2p device.
                num_bytes_deleted +=
                        (u_int16)e2p_cache_h_ptr->Client_Info[0].Num_Bytes;
                // Make note of Client whose data will need to be removed
                // from e2p device.
                if(writeback_client_start==-1){
                // No client before this needs to be written back.
                    writeback_client_start = 0;
                }

                e2p_cache_h_ptr->Client_Info[0].Num_Bytes = 0;
                e2p_cache_h_ptr->Client_Info[0].CheckSum = 0;
            }
            else{
                trace_new(TRACE_E2P | TRACE_LEVEL_1, \
                                "E2PInit: checksum ok. Client=0\n");
            }

            trace_new(TRACE_E2P | TRACE_LEVEL_2, \
                                            "E2PInit: Read data. Client=0 \n");
        }
        else{
            trace_new(TRACE_E2P | TRACE_LEVEL_2, \
                                            "E2PInit: No data. Client=0.\n");
        }

        for(i=1;i<ROM_CONFIG_NUM_CLIENTS;i++){ // Check each client for data.

            // Update local client offset table
            client_data_offset[i] = client_data_offset[i-1] +
                        (u_int16)e2p_cache_h_ptr->Client_Info[i-1].Num_Bytes;

            // check params before calling read
            if((u_int16)e2p_cache_h_ptr->Client_Info[i].Num_Bytes>MAX_EEPROM_SIZE){
                trace_new(TRACE_E2P | TRACE_LEVEL_3, \
                "E2PInit: NumBytes wrong for Client#=%x, Value=%x.\n", \
                                i, e2p_cache_h_ptr->Client_Info[i].Num_Bytes);
                error_log(ERROR_WARNING | RC_E2P_ERR | 0x8);
                e2p_erase();
//                return FALSE;
            }

            // If client has data Read client data into cache
            if(e2p_cache_h_ptr->Client_Info[i].Num_Bytes>0){
                // read and store the bytes in cache
                #if EEPROM_TYPE == EEPROM_512B
                    retcode = read_data(client_data_offset[i]+num_bytes_deleted,
                        (u_int16)e2p_cache_h_ptr->Client_Info[i].Num_Bytes, \
                                    e2p_cache+client_data_offset[i]);

                    // check for errors
                    if(retcode!=RC_OK){
                        trace_new(TRACE_E2P | TRACE_LEVEL_3, \
                    "E2PInit: Read failed. Client#=%d, #Bytes Read(%d)=%d.\n", \
                        i, e2p_cache_h_ptr->Client_Info[i].Num_Bytes, retcode);
                        error_log(ERROR_WARNING | RC_E2P_ERR | 0x9);
                        return FALSE;
                    }
                #else /* EEPROM_TYPE */
                   #ifdef DRIVER_INCL_EEPROM
                    retcode = ee_read(client_data_offset[i]+num_bytes_deleted,
                                    e2p_cache+client_data_offset[i],
            (u_int16)e2p_cache_h_ptr->Client_Info[i].Num_Bytes, (void*)NULL);
                    if(retcode!=
                        (u_int16)e2p_cache_h_ptr->Client_Info[i].Num_Bytes){
                        trace_new(TRACE_E2P | TRACE_LEVEL_3, \
                    "E2PInit: Read failed. Client#=d, #Bytes Read(%d)=%d.\n", \
                        i, e2p_cache_h_ptr->Client_Info[i].Num_Bytes, retcode);
                        error_log(ERROR_WARNING | RC_E2P_ERR | 0xA);
                        return FALSE;
                    }
                   #else /* DRIVER_INCL_EEPROM */
                    trace_new(TRACE_E2P | TRACE_LEVEL_3, \
                                            "Attempt to use \"ee_read\"!\n");
                    error_log(ERROR_FATAL | RC_E2P_ERR | 0xB);
                    return FALSE;
                   #endif /* DRIVER_INCL_EEPROM */
                #endif /* EEPROM_TYPE */

                // verify data
                checksum_store = get_checksum(client_data_offset[i],
                    (u_int16)e2p_cache_h_ptr->Client_Info[i].Num_Bytes, FALSE);

                checksum_store +=
                        (u_int8)e2p_cache_h_ptr->Client_Info[i].CheckSum;

                if(checksum_store!=0){  // Checksum failed.
                    trace_new(TRACE_E2P | TRACE_LEVEL_3, \
                                    "E2PInit: Client=%d, CheckSum Err!\n", i);

                    // Client data invalid. So initialize the data/header and
                    // write back updated data to e2p device.
                    num_bytes_deleted +=
                            (u_int16)e2p_cache_h_ptr->Client_Info[i].Num_Bytes;
                    // Make note of Client whose data will need to be removed
                    // from e2p device.
                    if(writeback_client_start==-1){
                    // No client before this needs to be written back.
                        writeback_client_start = i;
                    }

                    e2p_cache_h_ptr->Client_Info[i].Num_Bytes = 0;
                    e2p_cache_h_ptr->Client_Info[i].CheckSum = 0;
                }
                else{
                    trace_new(TRACE_E2P | TRACE_LEVEL_1, \
                                    "E2PInit: checksum ok. Client=%d\n", i);
                }

                trace_new(TRACE_E2P | TRACE_LEVEL_2, \
                                    "E2PInit: Read data. Client=%d\n", i);
            }
            else{
                trace_new(TRACE_E2P | TRACE_LEVEL_2, \
                                        "E2PInit: No data. Client=%d\n", i);
            }
        }

        // If any client had invalid data, make a request to writeback
        // header/data to the e2p device to cancel the invalid client's data.
        if(writeback_client_start!=-1){
            // Request header writeback
            write_start_page = (MAGIC_NUM_SIZE+
    (sizeof(E2P_Header_Client_Info)*writeback_client_start))/e2p_page_size;

            write_end_page = (MAGIC_NUM_SIZE+
(sizeof(E2P_Header_Client_Info)*(ROM_CONFIG_NUM_CLIENTS-1)))/e2p_page_size;

            for(i = write_start_page; i <= write_end_page; i++)
            {
                bDataWriteBackPage[i] = TRUE;
            }

            trace_new(TRACE_E2P | TRACE_LEVEL_2, \
            "E2PInit: Header update request made starting from client:%d.\n", \
                                                    writeback_client_start);

            // Request data writeback
            if(writeback_client_start!=(ROM_CONFIG_NUM_CLIENTS-1)){
            // Special case to save unnecessary write of 1 page.
                write_start_page =
                client_data_offset[writeback_client_start]/e2p_page_size;

                write_end_page = (client_data_offset[ROM_CONFIG_NUM_CLIENTS-1]+
                        // Account for all clients, including last client
    (u_int16)e2p_cache_h_ptr->Client_Info[ROM_CONFIG_NUM_CLIENTS-1].Num_Bytes)
                                                        /e2p_page_size;

                for(i = write_start_page; i <= write_end_page; i++)
                {
                    bDataWriteBackPage[i] = TRUE;
                }

                trace_new(TRACE_E2P | TRACE_LEVEL_2, \
                                        "E2PInit: Data update request.\n");
            }
        }
    }

    trace_new(TRACE_E2P | TRACE_LEVEL_2, "E2PInit: Header/Data checked.\n");

    // Create E2P semas
    sem_writeback_mutex = sem_create(1, "E2WS");

    if(sem_writeback_mutex==0){
        trace_new(TRACE_E2P | TRACE_LEVEL_3, \
                                "E2PInit: Err! creating writeback Sema.\n");
        error_log(ERROR_WARNING | RC_E2P_ERR | 0xC);
        return FALSE;
    }

    trace_new(TRACE_E2P | TRACE_LEVEL_2, "E2PInit: Writeback Sema Created.\n");

    sem_task_control = sem_create(1, "E2CS");

    if(sem_task_control==0){
        trace_new(TRACE_E2P | TRACE_LEVEL_3, \
                                    "E2PInit: Err! creating control Sema.\n");
        error_log(ERROR_WARNING | RC_E2P_ERR | 0xD);
        return FALSE;
    }

    trace_new(TRACE_E2P | TRACE_LEVEL_2, "E2PInit: Control Sema Created.\n");

    // Create E2P Task.
    pid = task_create(e2p_task, (void *)NULL, (void *)NULL,
                                E2PDRV_TASK_STACK_SIZE, E2PDRV_TASK_PRIORITY, E2PDRV_TASK_NAME);

    if(pid==0){     // Task create failed!
        trace_new(TRACE_E2P | TRACE_LEVEL_3, "E2PInit: Task Create Error!\n");
        error_log(ERROR_WARNING | RC_E2P_ERR | 0xE);
        return FALSE;
    }

    trace_new(TRACE_E2P | TRACE_LEVEL_2, "E2PInit: E2P Task Created.\n");

    trace_new(TRACE_E2P | TRACE_LEVEL_2, "E2PInit: completed successfully.\n");

    return TRUE;
}

/********************************************************************/
/*  e2p_task()                                                      */
/*                                                                  */
/*  PARAMETERS: Mandatory Pointer. Essentially nothing.             */
/*                                                                  */
/*  DESCRIPTION:Created only after KAL is up and e2p_init has been  */
/*              called. This is a task which lives forever.         */
/*              On coming up, It enters a loop, waiting for write   */
/*              requests(conveyed thru the datawriteback word)      */
/*              and writing back data to the E2P, forever.          */
/*  RETURNS:    (void)                                              */
/********************************************************************/
void e2p_task(void *Params){

    bool task_continue_loop = TRUE; // Conditional to break Task loop

    int32 retcode;                  // get return codes.

    u_int32 //local_writeback_word=0, // Local store for writeback word
        write_page_num=0;           // page# of page to write back.

    u_int32 eeaddr;
    u_int8 sema_err_count=0;

    int i;
    int found_data_to_write = 0;
    
    trace_new(TRACE_E2P | TRACE_LEVEL_2, "E2PTask: Entering Write Loop.\n");

    // Loop while there are no errors writing to the e2p device, waiting for
    // write requests coded in the writeback word and writing data to the E2P.
    // During a duration of no write requests, this task goes to sleep, waiting
    // on the control sema
    while(task_continue_loop){
        // get task control sema - indicates there is work (write) to do.
        retcode = sem_get(sem_task_control, KAL_WAIT_FOREVER);

        if(retcode!=RC_OK){
            sema_err_count++;
            continue;

            if(sema_err_count>3){
                trace_new(TRACE_E2P | TRACE_LEVEL_3, \
                                        "E2PTask: Control Sema!!\n");
                error_log(ERROR_WARNING | RC_E2P_ERR | 0xF);
            }
        }

        trace_new(TRACE_E2P | TRACE_LEVEL_2, \
                                    "E2PTask: Servicing Write requests.\n");

        // Re-initialize error counter for next sema.
        sema_err_count = 0;
        found_data_to_write = 0;
        for(i = 0; i < e2p_num_pages; i++)
        {
            if ( bDataWriteBackPage[i] )
            {
                found_data_to_write = 1;
                trace_new(TRACE_E2P | TRACE_LEVEL_2, \
                                    "E2PTask: Storing writeback word:%d.\n", i);
                write_page_num = i;

                // Get sync sema
                retcode = sem_get(sem_writeback_mutex, KAL_WAIT_FOREVER);

                if(retcode!=RC_OK){
                    sema_err_count++;
                    continue;

                    if(sema_err_count>3){
                        trace_new(TRACE_E2P | TRACE_LEVEL_ALWAYS, \
                                                    "E2PTask: Sync Sema!!\n");
                        error_log(ERROR_WARNING | RC_E2P_ERR | 0x10);
                    }
                }

                eeaddr = write_page_num * e2p_page_size;
                // write the page to the e2p device.
          #if EEPROM_TYPE == EEPROM_512B
                  trace_new(TRACE_E2P | TRACE_LEVEL_1, \
                      "E2PTask: Writing \"%d\" bytes from Addr:%d.\n", \
                          e2p_page_size, eeaddr);
                  retcode = write_data(eeaddr, e2p_page_size, e2p_cache + eeaddr);
                  if (retcode != RC_OK)
                  {
                     trace_new(TRACE_E2P | TRACE_LEVEL_3, \
                         "E2PTask: Data write failed. Retcode=%d.\n", retcode);
                     error_log(ERROR_WARNING | RC_E2P_ERR | 0x11);
                     task_continue_loop = FALSE;
                  }
                  
          #else /* EEPROM_TYPE != EEPROM_512B */
              #ifdef DRIVER_INCL_EEPROM

                  retcode = ee_write(eeaddr,
                                 e2p_cache + eeaddr,
                                 e2p_page_size,
                                 (void*)NULL);

                  if ( retcode != e2p_page_size )
                  {
                     trace_new(TRACE_E2P | TRACE_LEVEL_3, \
                         "E2PTask: Data write failed. #Bytes Write(%x)=%d.\n", \
                                             e2p_page_size, retcode);
                     error_log(ERROR_WARNING | RC_E2P_ERR | 0x12);
                     task_continue_loop = FALSE;
                  }
              #else // DRIVER_INCL_EEPROM
                  trace_new(TRACE_E2P | TRACE_LEVEL_3, \
                                     "Attempt to use \"ee_write\"!\n");
                  error_log(ERROR_FATAL | RC_E2P_ERR | 0x13);
                  task_continue_loop = FALSE;
              #endif // DRIVER_INCL_EEPROM

          #endif /* EEPROM_TYPE == EEPROM_512B */

                if ( task_continue_loop == FALSE )
                {
                    break;
                }

                trace_new(TRACE_E2P | TRACE_LEVEL_2, 
                    "E2PTask: Wrote \"%d\" bytes at Addr:%d.\n",
                    e2p_page_size, eeaddr);

                // indicate page written.
                bDataWriteBackPage[i] = FALSE;

                trace_new(TRACE_E2P | TRACE_LEVEL_2,
                              "E2PTask: Cleared Page bit.\n");

                // Release the synch sema.
                sem_put(sem_writeback_mutex);
            }
        }
        if (!found_data_to_write)
        {
           trace_new(TRACE_E2P | TRACE_LEVEL_3,
                              "E2PTask: (WHY?) Released with no data to write.\n");
        }

        if ( task_continue_loop == FALSE )
        {
            break;
        }

        trace_new(TRACE_E2P | TRACE_LEVEL_2, \
                                "E2PTask: Serviced all write requests.\n");
    }

    trace_new(TRACE_E2P | TRACE_LEVEL_ALWAYS, "E2PTask: Task terminating\n");

    task_terminate();
}

/********************************************************************/
/*  e2p_allocate()                                                  */
/*                                                                  */
/*  PARAMETERS: Client Id, Size needed to be allocated for Client.  */
/*                                                                  */
/*  DESCRIPTION:Can be called only after KAL is up.                 */
/*              Checks to see current allocation size for client and*/
/*              allocates required size. If Client data block is in */
/*              the middle of other client blocks, the other blocks */
/*              are moved to accomodate for this client.            */
/*              A call with a size of 0 will delete all client data */
/*                                                                  */
/*  RETURNS:    If successful:RC_OK                                 */
/*              otherwise: a return value coding the cause          */
/*               of failure. These values are coded in the NV header*/
/********************************************************************/
u_int32 e2p_allocate(u_int8 Client_Id, u_int32 Num_Bytes){

    int32 retcode,              // return codes
        i;                      // counter.
    unsigned int tmp;

    u_int16 old_client_end=0,
        new_client_end=0,
        num_bytes_moved=0;      // #bytes (of other clients) which need to be
                                // moved.

    int32 write_start_page=0,   // to update writeback word
        write_end_page=0;       // to update writeback word

    // Check the input parameters
    if(Client_Id>=ROM_CONFIG_NUM_CLIENTS){  // Trouble
        trace_new(TRACE_E2P | TRACE_LEVEL_3, "E2PAll: Invalid Client.\n");
        error_log(ERROR_WARNING | RC_E2P_ERR | 0x14);
        return RC_NV_ID_ERR;
    }

    // Just a warning message to indicate all client data will be lost
    if(Num_Bytes==0){
        trace_new(TRACE_E2P | TRACE_LEVEL_3, \
                "E2PAll: Data will be erased for Client:%d!\n", Client_Id);
    }

    if(Num_Bytes==(u_int16)e2p_cache_h_ptr->Client_Info[Client_Id].Num_Bytes){
        trace_new(TRACE_E2P | TRACE_LEVEL_3, "E2PAll: #bytes is same!\n");
        return RC_OK;
    }

    if(Num_Bytes>(u_int16)e2p_cache_h_ptr->Client_Info[Client_Id].Num_Bytes){
    // New size is greater than old size. Client needs more bytes.
        tmp = MAX_EEPROM_SIZE;
        // Are there enough free bytes to satisfy this alloc request?
        if((Num_Bytes-
            (u_int16)e2p_cache_h_ptr->Client_Info[Client_Id].Num_Bytes)>
                            // Extra bytes needed by Client "Client_Id"
           (tmp-(client_data_offset[ROM_CONFIG_NUM_CLIENTS-1]+
   (u_int16)e2p_cache_h_ptr->Client_Info[ROM_CONFIG_NUM_CLIENTS-1].Num_Bytes))){
                            // Total free bytes present in E2P
            // Insufficient free bytes.
            trace_new(TRACE_E2P | TRACE_LEVEL_3, \
                                            "E2PAll: Insufficient #bytes.\n");
            error_log(ERROR_WARNING | RC_E2P_ERR | 0x15);
            return RC_NV_FULL_ERR;
        }

        trace_new(TRACE_E2P | TRACE_LEVEL_2, \
                                        "E2PAll: Sufficient free bytes.\n");
    }// New Size is smaller than old size. Will free up some bytes.

    // store old client boundary.
    old_client_end = client_data_offset[Client_Id] +
                    (u_int16)e2p_cache_h_ptr->Client_Info[Client_Id].Num_Bytes;

    // Update cache header
    e2p_cache_h_ptr->Client_Info[Client_Id].Num_Bytes = (u_int16)Num_Bytes;

    // store new client boundary.
    new_client_end = client_data_offset[Client_Id] +
                    (u_int16)e2p_cache_h_ptr->Client_Info[Client_Id].Num_Bytes;

    // Are there any other clients? Was this the last client?
    if(Client_Id!=(ROM_CONFIG_NUM_CLIENTS-1)){ // This was not the last client.
        trace_new(TRACE_E2P | TRACE_LEVEL_2, \
                                "E2PAll: Checking for other client bytes.\n");

        // Get byte count of data due to other clients and also update their
        // data Offsets.
        for(i=Client_Id+1;i<ROM_CONFIG_NUM_CLIENTS;i++){
            // byte count.
            num_bytes_moved +=
                        (u_int16)e2p_cache_h_ptr->Client_Info[i].Num_Bytes;

            // Update the Client data Offsets.
            client_data_offset[i] = client_data_offset[i-1] +
                        (u_int16)e2p_cache_h_ptr->Client_Info[i-1].Num_Bytes;
        }

        if(num_bytes_moved>0){
        // move the other client data
            if(new_client_end>old_client_end){
            // Bytes were added to increase the size of client "Client Id"
                for(i=num_bytes_moved-1;i>=0;i--){
                    e2p_cache[new_client_end+i] = e2p_cache[old_client_end+i];
                }
            }
            else{
                for(i=0;i<num_bytes_moved;i++){
                    e2p_cache[new_client_end+i] = e2p_cache[old_client_end+i];
                }
            }

            trace_new(TRACE_E2P | TRACE_LEVEL_2, \
                                        "E2PAll: Moved other client bytes.\n");

            // request for e2p device (data) to be updated.
            write_start_page = new_client_end/e2p_page_size;
            write_end_page = (new_client_end+num_bytes_moved)/e2p_page_size;

            // Lock access to synch sema.
            retcode = sem_get(sem_writeback_mutex, KAL_WAIT_FOREVER);

            if(retcode!=RC_OK){
                trace_new(TRACE_E2P | TRACE_LEVEL_3,
                                            "E2PAll: Sync Sema failed!!\n");
                error_log(ERROR_WARNING | RC_E2P_ERR | 0x16);
                return RC_NV_ESYNS_ERR;
            }

            trace_new(TRACE_E2P | TRACE_LEVEL_2, \
            "E2PAll: Making data update request for Client:%d.\n", Client_Id);

            for(i = write_start_page; i <= write_end_page; i++)
            {
                bDataWriteBackPage[i] = TRUE;
            }

            // release sync sema to allow others to access writeback word
            sem_put(sem_writeback_mutex);

            trace_new(TRACE_E2P | TRACE_LEVEL_2, \
            "E2PAll: Data update request made for client:%d.\n", Client_Id);

            // if task sema is locked, release it to wakeup write back task
            // MFB:  Why the following line?
            // sem_get(sem_task_control, 0);
            sem_put(sem_task_control);

            trace_new(TRACE_E2P | TRACE_LEVEL_2, \
                                                "E2PAll: Woke up WriteTask.\n");
        }
    }

    // If Client_Id has newly allocated bytes, initialize them.
    if(new_client_end>old_client_end){
    // New bytes added. fill extra byes with "0"
        memset(e2p_cache+old_client_end, 0, (new_client_end-old_client_end));
        trace_new(TRACE_E2P | TRACE_LEVEL_2, \
    "E2PAll: Initialized newly allocated bytes for Client:%d.\n", Client_Id);

        // Request for the e2p device (data) to be updated.
        write_start_page = old_client_end/e2p_page_size;
        write_end_page = new_client_end/e2p_page_size;

        // Lock access to synch sema.
        retcode = sem_get(sem_writeback_mutex, KAL_WAIT_FOREVER);

        if(retcode!=RC_OK){
            trace_new(TRACE_E2P | TRACE_LEVEL_3,
                                        "E2PAll: Sync Sema failed!!\n");
            error_log(ERROR_WARNING | RC_E2P_ERR | 0x17);
            return RC_NV_ESYNS_ERR;
        }

        trace_new(TRACE_E2P | TRACE_LEVEL_2, \
            "E2PAll: Making data update request for Client:%d.\n", Client_Id);

        for( i = write_start_page; i <= write_end_page; i++)
        {
            bDataWriteBackPage[i] = TRUE;
        }

        // release sync sema to allow others to access writeback word
        sem_put(sem_writeback_mutex);

        trace_new(TRACE_E2P | TRACE_LEVEL_2, \
            "E2PAll: Data update request made for client:%d.\n", Client_Id);

        // if task sema is locked, release it to wakeup write back task
        // MFB:  Why the following line?
        // sem_get(sem_task_control, 0);
        sem_put(sem_task_control);

        trace_new(TRACE_E2P | TRACE_LEVEL_2, "E2PAll: Woke up WriteTask.\n");
    }// client data was truncated. no change necessary.

    // Update header (CheckSum) for Client "Client_Id"
    e2p_cache_h_ptr->Client_Info[Client_Id].CheckSum =
                get_checksum(client_data_offset[Client_Id],
            (u_int16)e2p_cache_h_ptr->Client_Info[Client_Id].Num_Bytes, TRUE);

    trace_new(TRACE_E2P | TRACE_LEVEL_2, \
                "E2PAll: Updated Cache CKS for Client:%d.\n", Client_Id);

    // put request to update e2p device header (Client Num_Bytes and CheckSum).
    write_start_page =
            (MAGIC_NUM_SIZE+(sizeof(E2P_Header_Client_Info)*Client_Id))
                                                            /e2p_page_size;

    write_end_page =
            (MAGIC_NUM_SIZE+(sizeof(E2P_Header_Client_Info)*(Client_Id+1)))
                                                            /e2p_page_size;

    // Lock access to writeback word.
    retcode = sem_get(sem_writeback_mutex, KAL_WAIT_FOREVER);

    if(retcode!=RC_OK){
        trace_new(TRACE_E2P | TRACE_LEVEL_3,
                                    "E2PAll: Sync Sema failed!!\n");
        error_log(ERROR_WARNING | RC_E2P_ERR | 0x18);
        return RC_NV_ESYNS_ERR;
    }

    trace_new(TRACE_E2P | TRACE_LEVEL_2, \
                                "E2PAll: Making header update request.\n");

    for( i = write_start_page; i <= write_end_page; i++)
    {
        bDataWriteBackPage[i] = TRUE;
    }

    // release sync sema to allow others to access writeback word
    sem_put(sem_writeback_mutex);

    trace_new(TRACE_E2P | TRACE_LEVEL_2, \
            "E2PAll: Header update request made for client:%d.\n", Client_Id);

    // if task sema is locked, release it to wakeup write back task
    // MFB: Why the following line?
    // sem_get(sem_task_control, 0);
    sem_put(sem_task_control);

    trace_new(TRACE_E2P | TRACE_LEVEL_2, "E2PAll: Woke up WriteTask.\n");

    trace_new(TRACE_E2P | TRACE_LEVEL_2, "E2PAll: Complete. Returning.\n");

    return RC_OK;
}

/********************************************************************/
/*  e2p_client_size()                                               */
/*                                                                  */
/*  PARAMETERS: Client Id (Offset) and                              */
/*              DataSize: a pointer which returns the status of     */
/*                  execution                                       */
/*                                                                  */
/*  DESCRIPTION:Can be called only after KAL is up.                 */
/*              This returns client size from e2p header.           */
/*                                                                  */
/*  RETURNS:    If successful:RC_OK                                 */
/*              otherwise: a return value coding the cause          */
/*               of failure. These values are coded in the NV header*/
/********************************************************************/
u_int32 e2p_client_size(u_int8 Client_Id, u_int32 *Client_Data_Size){

    // is client a valid client?
    if(Client_Id>=ROM_CONFIG_NUM_CLIENTS){  // Trouble
        trace_new(TRACE_E2P | TRACE_LEVEL_3, "E2PCSiz: Invalid Client.\n");
        error_log(ERROR_WARNING | RC_E2P_ERR | 0x19);
        return RC_NV_ID_ERR;
    }

    trace_new(TRACE_E2P | TRACE_LEVEL_2, "E2PCSiz: Returning Client Size.\n");

    *Client_Data_Size =
                (u_int16)e2p_cache_h_ptr->Client_Info[Client_Id].Num_Bytes;

    return RC_OK;
}

/********************************************************************/
/*  e2p_read()                                                      */
/*                                                                  */
/*  PARAMETERS: Client Id: Identifying the client                   */
/*              Read_Buf:  Storage buf to return the read data      */
/*              #Bytes_Read:#bytes asked to be returned             */
/*              Read Offset:Read start location                     */
/*              Actually Read: a pointer which returns the status of*/
/*               execution                                          */
/*                                                                  */
/*  DESCRIPTION:This function can be called before or after KAL is  */
/*              up.                                                 */
/*              It Reads and returns the requested #bytes for the   */
/*              requested  Client.                                  */
/*              The Bytes are read from the cached E2P data and not */
/*              from the E2P. If a call is made before the KAL is up*/
/*              the Cache is first filled up and the data returned  */
/*              from the Cache.                                     */
/*                                                                  */
/*  RETURNS:    If successful:RC_OK                                 */
/*              otherwise: a return value coding the cause          */
/*               of failure. These values are coded in the NV header*/
/********************************************************************/
u_int32 e2p_read(u_int8 Client_Id, void *Read_Buf, u_int32 Num_Bytes_to_Read,
                        u_int32 Read_Offset, u_int32 *Num_Bytes_Actually_Read){

    bool present=FALSE;             // is the E2P present?

    u_int16 num_read_possible;      // #bytes that are present in the client
                                    // data and can be returned irrespective of
                                    // the #asked for in calling parameters.

    int16 i;                        // to be used as counter.

    u_int8 checksum_store;          // temp storage to verify data checksum

    int32 retcode;                  // return values.

    // Check the input parameters
    if(Client_Id>=ROM_CONFIG_NUM_CLIENTS){  // Trouble
        if(nv_init_done){
            trace_new(TRACE_E2P | TRACE_LEVEL_3, "E2PRe: Invalid Client.\n");
            error_log(ERROR_WARNING | RC_E2P_ERR | 0x1A);
        }
        return RC_NV_ID_ERR;
    }

    if(Num_Bytes_to_Read==0){
        if(nv_init_done){
            trace_new(TRACE_E2P | TRACE_LEVEL_3, "E2PRe: #bytes=0!\n");
        }
        *Num_Bytes_Actually_Read = 0;
        return RC_OK;
    }

    if(nv_init_done){// KAL is up. So can use cached E2P data for fast response.

        // make sure the client has valid data stored.
        if(e2p_cache_h_ptr->Client_Info[Client_Id].Num_Bytes==0){// No data.
            trace_new(TRACE_E2P | TRACE_LEVEL_3, \
                                "E2PRe: No data. Client=%d\n", Client_Id);
            *Num_Bytes_Actually_Read = 0;
            return RC_OK;
        }

        // make sure read Offset is within Client data boundary.
        if(Read_Offset>=
                (u_int16)e2p_cache_h_ptr->Client_Info[Client_Id].Num_Bytes){
            trace_new(TRACE_E2P | TRACE_LEVEL_3, "E2PRe: Wrong Offset!\n");
            error_log(ERROR_WARNING | RC_E2P_ERR | 0x1B);
            return RC_NV_OFF_ERR;
        }

        // Load required Client data.
        // If requested #bytes is outside the Client data boundary, return only
        // the number possible, within the boundary.
        num_read_possible =
            ((Read_Offset+Num_Bytes_to_Read)>
                    (u_int16)e2p_cache_h_ptr->Client_Info[Client_Id].Num_Bytes)
            ? ((u_int16)e2p_cache_h_ptr->Client_Info[Client_Id].Num_Bytes
                                            -Read_Offset): Num_Bytes_to_Read;

        trace_new(TRACE_E2P | TRACE_LEVEL_2, \
                                "E2PRe: #requested=%d, #read possible=%d.\n",\
                                        Num_Bytes_to_Read, num_read_possible);

        memcpy(Read_Buf, (e2p_cache+client_data_offset[Client_Id]+Read_Offset),
                                                    num_read_possible);

        trace_new(TRACE_E2P | TRACE_LEVEL_2, \
                                        "E2PRe: Loaded data. Returning.\n");

        *Num_Bytes_Actually_Read = num_read_possible;

        return RC_OK;
    }

    // KAL is still not up. So Init not called and E2P data is not cached.
    // So need to do a few extra things

    // Check if e2p is on the IIC bus.
    present = iicAddressTest(I2C_ADDR_EEPROM1, I2C_BUS_EEPROM1, FALSE);

    if(!present){
        return RC_NV_ACC_ERR;
    }

       // Get header. Will be stored in cache
       #if EEPROM_TYPE == EEPROM_512B
           retcode = read_data(HEADER_BASE_ADD, sizeof(E2P_Header_Info),
                                                   e2p_cache+HEADER_BASE_ADD);
           if(retcode!=RC_OK){
               return RC_NV_DATA_ERR;
           }
       #else /* EEPROM_TYPE */
          #ifdef DRIVER_INCL_EEPROM
           // On to newer things!
           retcode = ee_read(HEADER_BASE_ADD, e2p_cache+HEADER_BASE_ADD,
                   (u_int16)sizeof(E2P_Header_Info), (void*)NULL);
           if(retcode!=sizeof(E2P_Header_Info)){
               return RC_NV_DATA_ERR;
           }
          #else   // DRIVER_INCL_EEPROM
           trace_new(TRACE_E2P | TRACE_LEVEL_3, "Attempt to use \"ee_read\"!\n");
           error_log(ERROR_FATAL | RC_E2P_ERR | 0x1C);
           return FALSE;
          #endif // DRIVER_INCL_EEPROM
       #endif /* EEPROM_TYPE */

    // Get client data address,size from header and get client data.

    // Initialize the cache header pointer
    e2p_cache_h_ptr = (p_E2P_Header_Info)(e2p_cache+HEADER_BASE_ADD);
                                       // Header starts at the Base.

    // Check for Magic Number. Presence of "Magic Number" indicates presence of
    // valid data.
    if(e2p_cache_h_ptr->Magic_Number!=(u_int32)E2P_MAGIC_NUM){
        // E2P does not store data. Write header to prepare for data later.
        *Num_Bytes_Actually_Read = 0;
        return RC_OK;
    }

    // make sure the client has valid data stored.
    if(e2p_cache_h_ptr->Client_Info[Client_Id].Num_Bytes==0){   // No data.
        *Num_Bytes_Actually_Read = 0;
        return RC_OK;
    }

    // make sure read Offset is within Client data boundary.
    if(Read_Offset>=
                (u_int16)e2p_cache_h_ptr->Client_Info[Client_Id].Num_Bytes){
        return RC_NV_OFF_ERR;
    }

    // Update local client offset table
    client_data_offset[0] = (u_int16)sizeof(E2P_Header_Info);
                                                    // Account for the header

    for(i=1;i<=Client_Id;i++){ // Check each client for data.
        client_data_offset[i] = client_data_offset[i-1] +
                        (u_int16)e2p_cache_h_ptr->Client_Info[i-1].Num_Bytes;
    }

    // Read client data (into cache).
    // check params before calling read
    if((u_int16)e2p_cache_h_ptr->Client_Info[Client_Id].Num_Bytes>
                                                        MAX_EEPROM_SIZE){
        return FALSE;
    }

    #if EEPROM_TYPE == EEPROM_512B
        retcode = read_data(client_data_offset[Client_Id],
               (u_int16)e2p_cache_h_ptr->Client_Info[Client_Id].Num_Bytes,
                                   e2p_cache+client_data_offset[Client_Id]);
        if(retcode!=RC_OK){
           return RC_NV_DATA_ERR;
        }
    #else /* EEPROM_TYPE */
       #ifdef DRIVER_INCL_EEPROM
           // On to newer things!
           retcode = ee_read(client_data_offset[Client_Id],
                           e2p_cache+client_data_offset[Client_Id],
       (u_int16)e2p_cache_h_ptr->Client_Info[Client_Id].Num_Bytes, (void*)NULL);
           if(retcode!=(u_int16)e2p_cache_h_ptr->Client_Info[Client_Id].Num_Bytes){
               return RC_NV_DATA_ERR;
           }
       #else // DRIVER_INCL_EEPROM
           return FALSE;
       #endif // DRIVER_INCL_EEPROM
    #endif /* EEPROM_TYPE */

    // verify checksum
    checksum_store = get_checksum(client_data_offset[Client_Id],
            (u_int16)e2p_cache_h_ptr->Client_Info[Client_Id].Num_Bytes, FALSE);

    checksum_store += (u_int8)e2p_cache_h_ptr->Client_Info[Client_Id].CheckSum;

    if(checksum_store!=0){  // Checksum failed.
        *Num_Bytes_Actually_Read = 0;
        return RC_OK;
    }

    // Load required Client data.
    // If requested #bytes is outside the Client data boundary, return only
    // the number possible, within the boundary.
    num_read_possible =
        ((Read_Offset+Num_Bytes_to_Read)>
                    (u_int16)e2p_cache_h_ptr->Client_Info[Client_Id].Num_Bytes)
            ? ((u_int16)e2p_cache_h_ptr->Client_Info[Client_Id].Num_Bytes
                                    -Read_Offset) : Num_Bytes_to_Read;

    memcpy(Read_Buf, (e2p_cache+client_data_offset[Client_Id]+Read_Offset),
                                                    num_read_possible);

    *Num_Bytes_Actually_Read = num_read_possible;

    return RC_OK;
}

/********************************************************************/
/*  e2p_write()                                                     */
/*                                                                  */
/*  PARAMETERS: Client Id: Identifying the client                   */
/*              Write_Buf:  Storage buf to return the read data     */
/*              #Bytes_Write:#bytes asked to be returned            */
/*              Write Offset:Write start location                   */
/*              Actually Written: a pointer used to return the      */
/*                status of execution                               */
/*  DESCRIPTION:This function should only be called after KAL is up */
/*              and the E2P is completely initialized               */
/*              It writes and returns the #bytes written for the    */
/*              requested  Client and updates the Client header.    */
/*              The Bytes are written to the cached E2P data and not*/
/*              to the E2P device.                                  */
/*              A request is made to the E2P Task (thru the data    */
/*              writeback word) to write back the data and the      */
/*              updated Checksum to the E2P device.                 */
/*              SPECIAL CASE: If there is a request to write more   */
/*              #bytes than currently allocated to a client, this   */
/*              will try to allocate more bytes to the client (by   */
/*              calling e2p_allocate()). If there not enough free   */
/*              bytes to satisfy this alloc() request. It does not  */
/*              write anything and returns a "NV_FULL" error.       */
/*                                                                  */
/*  RETURNS:    If successful:RC_OK                                 */
/*              otherwise: a return value coding the cause          */
/*               of failure. These values are coded in the NV header*/
/********************************************************************/
u_int32 e2p_write(u_int8 Client_Id, void *Write_Buf,
                    u_int32 Num_Bytes_to_Write, u_int32 Write_Offset,
                                    u_int32 *Num_Bytes_Actually_Written){

    u_int8 *write_buffer = (u_int8*)Write_Buf;
                                    // Source of data to be written.

    bool data_changed=FALSE;        // did something change in the e2p data?

    unsigned int i;                        // to be used as counter.

    int32 retcode;                  // store return codes.
    u_int32 cli_off; /* offset into eeprom for this client */
    u_int32 page_index;


    /* check e2p_cache_h_ptr->Magic_Number to make sure it is valid */
    if (e2p_cache_h_ptr->Magic_Number!=(u_int32)E2P_MAGIC_NUM)
    {
       e2p_cache_h_ptr->Magic_Number=(u_int32)E2P_MAGIC_NUM;
    }

    // Check the input parameters
    if(Client_Id>=ROM_CONFIG_NUM_CLIENTS){  // Trouble
        trace_new(TRACE_E2P | TRACE_LEVEL_3, "E2PWr: Invalid Client!\n");
        error_log(ERROR_WARNING | RC_E2P_ERR | 0x1D);
        return RC_NV_ID_ERR;
    }

    cli_off = client_data_offset[Client_Id];

    if(Num_Bytes_to_Write==0){
        trace_new(TRACE_E2P | TRACE_LEVEL_3, "E2PWr: Read. #bytes=0!\n");
        *Num_Bytes_Actually_Written = 0;
        return RC_OK;
    }

    // If write request is for more bytes than currently allocated to client,
    // call e2p_allocate() to try to add more bytes to client.
    if((Write_Offset+Num_Bytes_to_Write)>
                (u_int16)e2p_cache_h_ptr->Client_Info[Client_Id].Num_Bytes){
        retcode = e2p_allocate(Client_Id, Write_Offset+Num_Bytes_to_Write);

        if(retcode!=RC_OK){ // Allocate failed.
            return retcode;
        }
    }

    // Now, Update the cache with new data, calculate checksum, update
    // header and put messages on queue for e2p_task to update E2P.

    // Compare the new and old data and Update cache with only modified data

    // Update Cache
    for( i = 0; i < Num_Bytes_to_Write; i++ )
    {
        u_int32 tmpOffset = cli_off + Write_Offset + i;
        /* Is write buffer different from cache? */
        if ( e2p_cache[tmpOffset] != *(write_buffer+i) )
        {
            // Data not equal. Write new data to cache
            e2p_cache[tmpOffset] = *(write_buffer+i);

            trace_new(TRACE_E2P | TRACE_LEVEL_1,
                "E2PWr: Wrote 1byte at addr:%d.\n", tmpOffset);

            // Update writeback word to request write task to write new data
            // to device.
            // Lock access to synch sema.
            retcode = sem_get(sem_writeback_mutex, KAL_WAIT_FOREVER);

            // Announce data change
            data_changed = TRUE;

            if(retcode!=RC_OK){
                trace_new(TRACE_E2P | TRACE_LEVEL_3,
                                            "E2PWr: Sync Sema failed!!\n");
                error_log(ERROR_WARNING | RC_E2P_ERR | 0x1E);
                return RC_NV_ESYNS_ERR;
            }

            trace_new(TRACE_E2P | TRACE_LEVEL_1, \
                                    "E2PWr: Writing writeback data word.\n");

            // The whole page will be written back.
            // page_index = tmpOffset / e2p_num_pages;
            page_index = tmpOffset / e2p_page_size;
            bDataWriteBackPage [page_index] = TRUE;

            // release sync sema to allow others to access writeback word
            sem_put(sem_writeback_mutex);

            trace_new(TRACE_E2P | TRACE_LEVEL_1, \
                                    "E2PWr: Put Request for data update.\n");

            trace_new(TRACE_E2P | TRACE_LEVEL_1, "E2PWr: Woke up WriteTask.\n");
        }
    }
    // if task sema is locked, release it to wakeup write back task
    // MFB:  Why was the following line here?
    // sem_get(sem_task_control, 0);
    if (data_changed) sem_put(sem_task_control);


    trace_new(TRACE_E2P | TRACE_LEVEL_2, \
                    "E2PWr: Updated Cache for Client:%d.\n", Client_Id);

    if( data_changed )
    {    
        // Get checksum.
        e2p_cache_h_ptr->Client_Info[Client_Id].CheckSum =
                    get_checksum(client_data_offset[Client_Id],
            (u_int16)e2p_cache_h_ptr->Client_Info[Client_Id].Num_Bytes, TRUE);

        trace_new(TRACE_E2P | TRACE_LEVEL_2, \
                    "E2PWr: Updated Cache CKS for Client:%d.\n", Client_Id);

        // Write back Checksum.
        // Update writeback word to request write task to write new data
        // e2p to device.
        // Lock access to synch sema.
        retcode = sem_get(sem_writeback_mutex, KAL_WAIT_FOREVER);

        if(retcode!=RC_OK){
            trace_new(TRACE_E2P | TRACE_LEVEL_3,
                                            "E2PWr: Sync Sema failed!!\n");
            error_log(ERROR_WARNING | RC_E2P_ERR | 0x1F);
            return RC_NV_ESYNS_ERR;
        }

        trace_new(TRACE_E2P | TRACE_LEVEL_1, \
                                "E2PWr: Writing writeback data word.\n");

        /*
         * Header starts at the Base
         * 2: Magic#
         * 2: NumBytes (Client data)
         * i : upto 4 checksum bytes
         */

        for( i = 0; i < 4 ; i++ )
        {
            u_int32 cksumaddr = 2 + 2 + i +
                                sizeof(E2P_Header_Client_Info) * Client_Id;
            bDataWriteBackPage [cksumaddr / e2p_page_size] = TRUE;
        }                                       
        
        // release sync sema to allow others to access writeback word
        sem_put(sem_writeback_mutex);

        trace_new(TRACE_E2P | TRACE_LEVEL_2, \
                            "E2PWr: Put Request for Header data update.\n");

        // if task sema is still locked, release it to wakeup write back task
        // MFB:  Why the following line?
        // sem_get(sem_task_control, 0);
        sem_put(sem_task_control);

        trace_new(TRACE_E2P | TRACE_LEVEL_2, "E2PWr: Woke up Write Task.\n");
    }

    trace_new(TRACE_E2P | TRACE_LEVEL_2, "E2PWr: Write done. Returning.\n");

    // Load return value
    *Num_Bytes_Actually_Written =  Num_Bytes_to_Write;

    return RC_OK;
}

/********************************************************************/
/*  e2p_erase()                                                     */
/*                                                                  */
/*  PARAMETERS: None.                                               */
/*                                                                  */
/*  DESCRIPTION:Erase all config data in E2P cache and indicate to  */
/*              writeback task to write back E2P to invalidate all  */
/*              data in the physical device.                        */
/*                                                                  */
/*  RETURNS:    RC_OK on completion.                                */
/********************************************************************/
u_int32 e2p_erase(void){

    int32 i,
        write_start_page = 0,       // to update writeback word
        write_end_page = 0,         // to update writeback word
        retcode;

    // Invalidate all e2p data. Erase header to prepare for data later.
    trace_new(TRACE_E2P | TRACE_LEVEL_2, \
                            "E2PErase: Clearing all data in E2P.\n");

    e2p_cache_h_ptr->Magic_Number = (u_int32)E2P_ERASE_MAGIC_NUM;
    for(i=ROM_CONFIG_NUM_CLIENTS-1;i>=0;i--){ // Clear Client data.
        e2p_cache_h_ptr->Client_Info[i].Num_Bytes = 0;
        e2p_cache_h_ptr->Client_Info[i].CheckSum = 0;
    }

    trace_new(TRACE_E2P | TRACE_LEVEL_2, \
                        "E2PErase: Erased header data in cache.\n");

    // Initialize the Client Offsets array
    for(i=ROM_CONFIG_NUM_CLIENTS-1;i>=0;i--){ // No Client data.
        client_data_offset[i] = (u_int16)sizeof(E2P_Header_Info);
    }

    trace_new(TRACE_E2P | TRACE_LEVEL_2, \
                                "E2PErase: Erased client data offsets.\n");

    // Write writeback_word to request task to writebak header to e2p dev.
    write_start_page = (HEADER_BASE_ADD/e2p_page_size);

    write_end_page =
            (HEADER_BASE_ADD+sizeof(E2P_Header_Info))/e2p_page_size;

    // Lock access to synch sema.
    retcode = sem_get(sem_writeback_mutex, KAL_WAIT_FOREVER);

    if(retcode!=RC_OK){
        trace_new(TRACE_E2P | TRACE_LEVEL_3,
                                    "E2PErase: Sync Sema failed!!\n");
        error_log(ERROR_WARNING | RC_E2P_ERR | 0x2E);
        return RC_NV_ESYNS_ERR;
    }

    // Clear bDataWriteBackPage to halt all e2p writes. Only this header erase
    // write back needs to be processed, ASAP.
    for(i = 0;  i < e2p_num_pages; i++)
    {
        bDataWriteBackPage[i] = 0;
    }

    for(i = write_start_page; i <= write_end_page; i++)
    {
        bDataWriteBackPage[i] = TRUE;
    }

    // release sync sema to allow others to access writeback word
    sem_put(sem_writeback_mutex);

    trace_new(TRACE_E2P | TRACE_LEVEL_2, \
                        "E2PErase: Wrote writeback word for header.\n");

    return RC_OK;
}

// private helper functions

/********************************************************************/
/*  read_data()                                                     */
/*                                                                  */
/*  PARAMETERS: Address to read from, #bytes to read.               */
/*                                                                  */
/*  DESCRIPTION:Can be called only after KAL is up.                 */
/*              This function reads the E2P (from the specified     */
/*              address) and returns the requested #bytes           */
/*                                                                  */
/*  RETURNS:    If successful:RC_OK                                 */
/*              otherwise: a return value coding the cause          */
/*               of failure. These values are coded in the NV header*/
/********************************************************************/
int32 read_data(u_int16 Address, u_int16 Num_Bytes, u_int8 *Read_Buffer){

    int16 num_read_transaction; // #bytes to be read in one transaction.

    int32 i;                // to be used as a counter.

    // data structures for IIC transfer
    IICTRANS iicTransBuf;
    u_int8 RData[READ_TRANSFER_SIZE+4];
                            // entire E2P can be read  in one go.
                            // Here Reads are restricted to a fixed #bytes so
                            // as to not use up too much stack resources.
                            // Bytes 4 to do IIC transaction:
                            // Start, Address, Repeated Start(for Read),
                            // Stop
    u_int8 RCmd[READ_TRANSFER_SIZE+4];

    // Check the input parameters
    if(Address>=MAX_EEPROM_SIZE){
        if(nv_init_done){
            trace_new(TRACE_E2P | TRACE_LEVEL_3, "E2PRDa: Addr Error!\n");
            error_log(ERROR_WARNING | RC_E2P_ERR | 0x20);
        }
        return RC_NV_ADDR_ERR;
    }

    if(Num_Bytes==0){
        if(nv_init_done){
            trace_new(TRACE_E2P | TRACE_LEVEL_3, "E2PRDa: #Bytes=0!\n");
            error_log(ERROR_WARNING | RC_E2P_ERR | 0x21);
        }
        return RC_NV_NUMB_ERR;
    }

    while(Num_Bytes>0){

        if(Num_Bytes>e2p_page_size){
            num_read_transaction = e2p_page_size;
        }
        else{
            num_read_transaction = Num_Bytes;
        }

        // Update Num_Bytes
        Num_Bytes -= num_read_transaction;

        if(nv_init_done){
            trace_new(TRACE_E2P | TRACE_LEVEL_1, "E2PRDa: Loading Params.\n");
        }

        // Load IIC bus transaction command/data words
        RData[0] = I2C_ADDR_EEPROM1 | WRITE_FLAG;  // Device Addr, R/W
        RData[1] = Address;              // Address location for Read/Writes
        RData[2] = I2C_ADDR_EEPROM1 | READ_FLAG;   // Device Addr, R/W
        RCmd[0] = IIC_START;             // Take Data stored in Data[0].
        RCmd[1] = IIC_DATA;              // Take Data stored in Data[1].
        RCmd[2] = IIC_START;             // Take Data stored in Data[2].

        for(i=0;i<num_read_transaction-1;i++){
            RCmd[3+i] = IIC_DATA | IIC_ACK; // Data read from IIC bus is
                                            //stored  here.Multiple Read
                                            // will continue  as long as
                                            // each Read is acknowledged.
        }

        RCmd[3+(num_read_transaction-1)] = IIC_DATA;
                                            // No ack on last data cmd.
                                            // This stops multiple read.
        RCmd[3+(num_read_transaction-1)+1] = IIC_STOP; // End of Read.
                                        // Total #bytes = #client_data+4
                                        // [0]..[3]: Start, Address,Repeat S,
                                        // [3]..[3+#client_data]:Client data
                                        // [3+#client_data+1]: Stop
        iicTransBuf.dwCount = num_read_transaction+4;
                                        // The number of Command Words

        // Load the IIC Trans structure
        iicTransBuf.pCmd = RCmd;            // Command Words here.
        iicTransBuf.pData = RData;      // Client data is contained in Data
                                // array in elements [3]..[3+#client_data]

        if(nv_init_done){
            trace_new(TRACE_E2P | TRACE_LEVEL_1, "E2PRDa: Starting Read.\n");
        }

        iicTransaction(&iicTransBuf, I2C_BUS_EEPROM1);

        switch(iicTransBuf.dwError){    // What is the Read status?

        case IIC_ERROR_NOERR:           // No Error. Read Successful.
            if(nv_init_done){
                trace_new(TRACE_E2P | TRACE_LEVEL_1,
                                            "E2PRDa: Read Successful.\n");
            }
        break;

        case IIC_ERROR_NOACK:
            if(nv_init_done){
                trace_new(TRACE_E2P | TRACE_LEVEL_3, \
                "E2PRDa: Read Failed! Ack not received after byte write.\n");
                error_log(ERROR_WARNING | RC_E2P_ERR | 0x22);
            }
            return RC_NV_DATA_ERR;
        break;

        case IIC_ERROR_NOADDRESSACK:
            if(nv_init_done){
                trace_new(TRACE_E2P | TRACE_LEVEL_3, \
                "E2PRDa: Read Failed! Device not responding to address.\n");
                error_log(ERROR_WARNING | RC_E2P_ERR | 0x23);
            }
            return RC_NV_ACC_ERR;
        break;

        case IIC_ERROR_INVALIDDATA:
            if(nv_init_done){
                trace_new(TRACE_E2P | TRACE_LEVEL_3, \
                            "E2PRDa: Read Failed! Invalid data pointer.\n");
                error_log(ERROR_WARNING | RC_E2P_ERR | 0x24);
            }
            return RC_NV_DATA_ERR;
        break;

        case IIC_ERROR_INVALIDCMD:
            if(nv_init_done){
                trace_new(TRACE_E2P | TRACE_LEVEL_3, \
                        "E2PRDa: Read Failed! Invalid command pointer.\n");
                error_log(ERROR_WARNING | RC_E2P_ERR | 0x25);
            }
            return RC_NV_DATA_ERR;
        break;

        default:
            if(nv_init_done){
                trace_new(TRACE_E2P | TRACE_LEVEL_3, \
                                "E2PRDa: Read Failed! Reason Unknown!!.\n");
                error_log(ERROR_WARNING | RC_E2P_ERR | 0x26);
            }
            return RC_NV_DATA_ERR;
        }

        if(nv_init_done){
            trace_new(TRACE_E2P | TRACE_LEVEL_1, \
                                        "E2PRDa: Loading data in Cache.\n");
        }

        // Copy data to Read Buf.
        memcpy(Read_Buffer, RData+3, num_read_transaction);

        // Updating Address, Read Buf ptr for next read cycle, if needed.
        Address += num_read_transaction;
        Read_Buffer += num_read_transaction;
    }

    if(nv_init_done){
        trace_new(TRACE_E2P | TRACE_LEVEL_1, \
                                "E2PRDa: Loaded data in Cache. Returning.\n");
    }

    return RC_OK;
}

/********************************************************************/
/*  write_data()                                                    */
/*                                                                  */
/*  PARAMETERS: Address to write to, #bytes to write                */
/*                                                                  */
/*  DESCRIPTION:Can be called only after KAL is up.                 */
/*              This function actually writes to the E2P.           */
/*              It writes "n" bytes to the specified address        */
/*                                                                  */
/*  RETURNS:    If successful:RC_OK                                 */
/*              otherwise: a return value coding the cause          */
/*               of failure. These values are coded in the NV header*/
/********************************************************************/
int32 write_data(u_int16 Address, u_int16 Num_Bytes, u_int8 *Write_Buffer){

    bool e2p_ready;         // Can E2P receive next write request?

    int32 i;                // counter.

    // data structures for IIC transfer
    IICTRANS iicTransBuf;
    u_int8 WData[WRITE_PAGE_SIZE+3];   // Can write a max of 16(WRITE_PAGE_SIZE)
                            //  bytes at a time. 3 Bytes to do IIC transaction:
                            // Start, Address, Stop
    u_int8 WCmd[WRITE_PAGE_SIZE+3];

    // Check the input parameters
    if(Address>=MAX_EEPROM_SIZE){
        trace_new(TRACE_E2P | TRACE_LEVEL_3, "E2PWDa: Addr Error!\n");
        error_log(ERROR_WARNING | RC_E2P_ERR | 0x27);
        return RC_NV_ADDR_ERR;
    }

    // Is Address on a 16 byte (WRITE_PAGE_SIZE) Page boundary?
    if(Address%e2p_page_size){
        trace_new(TRACE_E2P | TRACE_LEVEL_3, \
                                "E2PWDa: Address not on Page boundary!\n");
        error_log(ERROR_WARNING | RC_E2P_ERR | 0x28);
        return RC_NV_EADDP_ERR;
    }

    if(Num_Bytes==0){
        trace_new(TRACE_E2P | TRACE_LEVEL_3, "E2PWDa: #Bytes=0!\n");
        error_log(ERROR_WARNING | RC_E2P_ERR | 0x29);
        return RC_NV_NUMB_ERR;
    }

    // because of the possibility of being called for back to back writes,
    // first determine if the e2p is ready (will respond to addressing) before
    // sending next write request.
    e2p_ready = iicAddressTest(I2C_ADDR_EEPROM1, I2C_BUS_EEPROM1, FALSE);
    while(!e2p_ready){
        task_time_sleep(E2P_WRITE_STIME);
                                // To account for the E2P max write cycle
                                // time of 10mS.
        e2p_ready = iicAddressTest(I2C_ADDR_EEPROM1, I2C_BUS_EEPROM1, FALSE);
    }

    trace_new(TRACE_E2P | TRACE_LEVEL_1, "E2PWDa: Loading Params.\n");

    // Load IIC bus and write the bytes to E2P
    // load IIC command and data buffers
    // Start and Address data.
    WCmd[0] = IIC_START;             // Take Data stored in Data[0].
    WCmd[1] = IIC_DATA;              // Take Data stored in Data[1].
    WData[0] = I2C_ADDR_EEPROM1 | WRITE_FLAG; // Device Addr, R/W
    WData[1] = Address;              // Address location for Writes

    for(i=0;i<e2p_page_size;i++){ // copy bytes to DATA buf.
        WCmd[2+i] = IIC_DATA;
        WData[2+i] = *(Write_Buffer+i);
    }

    WCmd[2+e2p_page_size] = IIC_STOP;    // End of this write.
    iicTransBuf.dwCount = 2+e2p_page_size+1;
                                           // Start, Data, Stop bytes

    // Load IIC Trans structure
    iicTransBuf.pCmd = WCmd;    // Command Words found here.
    iicTransBuf.pData = WData;  // Data words here.

    trace_new(TRACE_E2P | TRACE_LEVEL_1, \
    "E2PWDa: Starting Write. #IIC write bytes=%x.\n", iicTransBuf.dwCount);

    iicTransaction(&iicTransBuf, I2C_BUS_EEPROM1);

    switch(iicTransBuf.dwError){    // What is the Write status?

    case IIC_ERROR_NOERR:           // No Error. Write Successful.
        trace_new(TRACE_E2P | TRACE_LEVEL_1, "E2PWDa: Write Successful.\n");
    break;

    case IIC_ERROR_NOACK:
        trace_new(TRACE_E2P | TRACE_LEVEL_3, \
            "E2PWDa: Write Failed! Ack not received after byte write.\n");
        error_log(ERROR_WARNING | RC_E2P_ERR | 0x2A);
        return RC_NV_DATA_ERR;
    break;

    case IIC_ERROR_NOADDRESSACK:
        trace_new(TRACE_E2P | TRACE_LEVEL_3, \
            "E2PWDa: Write Failed! Device not responding to address.\n");
        error_log(ERROR_WARNING | RC_E2P_ERR | 0x2B);
        return RC_NV_ACC_ERR;
    break;

    case IIC_ERROR_INVALIDDATA:
        trace_new(TRACE_E2P | TRACE_LEVEL_3, \
                        "E2PWDa: Write Failed! Invalid data pointer.\n");
        error_log(ERROR_WARNING | RC_E2P_ERR | 0x2C);
        return RC_NV_DATA_ERR;
    break;

    case IIC_ERROR_INVALIDCMD:
        trace_new(TRACE_E2P | TRACE_LEVEL_3, \
                    "E2PWDa: Write Failed! Invalid command pointer.\n");
        error_log(ERROR_WARNING | RC_E2P_ERR | 0x2D);
        return RC_NV_DATA_ERR;
    break;

    default:
        trace_new(TRACE_E2P | TRACE_LEVEL_3, \
                            "E2PWDa: Write Failed! Reason Unknown!!.\n");
        error_log(ERROR_WARNING | RC_E2P_ERR | 0x2E);
        return RC_NV_DATA_ERR;
    }

    trace_new(TRACE_E2P | TRACE_LEVEL_1, "E2PWDa: write done. Returning.\n");

    return RC_OK;
}

/********************************************************************/
/*  get_checksum()                                                  */
/*                                                                  */
/*  PARAMETERS: Address to read from, #bytes to get checksum        */
/*              and a Flag saying whether or not to return the      */
/*              complemented checksum                               */
/*                                                                  */
/*  DESCRIPTION:Can be called whether the  KAL is up or not.        */
/*              This function reads the requested #bytes from the   */
/*              specified address (in the cached E2P Data) and      */
/*              returns it's checksum.                              */
/*              The Checksum may be 2's complemented depending on   */
/*              the value of COMPLEMENT.                            */
/*                                                                  */
/*  RETURNS:    8 bit checksum (UnComplemented or 2's complemented) */
/********************************************************************/
u_int8 get_checksum(u_int16 Address, u_int16 Num_Bytes, bool COMPLEMENT){

    u_int8 store_sum=0;     // temp store for calculating Checksum.

    while(Num_Bytes--){     // Start adding up the bytes.
        store_sum += e2p_cache[Address++];
    }

    if(COMPLEMENT){         // Send back 2's complement.
            trace_new(TRACE_E2P | TRACE_LEVEL_1,
                        "E2PCKS: ~CKS=%x. Returning.\n", (~store_sum+1));
        return (~store_sum+1);
    }

    trace_new(TRACE_E2P | TRACE_LEVEL_1,
                                "E2PCKS: CKS=%x. Returning.\n", store_sum);

    return store_sum;       // Just return checksum
}

/****************************************************************************
 * Modifications:
 * $Log: 
 *  71   mpeg      1.70        5/6/04 5:21:52 PM      Miles Bintz     CR(s) 
 *        8897 8898 : fix for e2p not storing setting properly
 *  70   mpeg      1.69        5/1/04 3:34:31 PM      Miles Bintz     CR(s) 
 *        9043 9044 : dirty e2p pages were managed by an array of integers 
 *        where each bit represented a page.  this was bug-ridden and imposed a
 *         2k limitation on the eeprom.  this change removes all that
 *  69   mpeg      1.68        11/26/02 4:01:28 PM    Dave Wilson     SCR(s) 
 *        4902 :
 *        Changed to use label from config file to define which I2C bus device 
 *        is on.
 *        
 *  68   mpeg      1.67        11/25/02 1:56:38 PM    Larry Wang      SCR(s) 
 *        4994 :
 *        When calling e2p_write(), make sure e2prom magic number is valid.
 *        
 *  67   mpeg      1.66        2/11/02 11:03:12 AM    Ganesh Banghar  SCR(s): 
 *        3155 
 *        change definitions to be taken from hwconfig.h.
 *        
 *  66   mpeg      1.65        2/4/02 10:49:08 AM     Senthil Veluswamy SCR(s) 
 *        2982 :
 *        Removed 0 Initialize for data_writeback_word array as this is not 
 *        supported for GNU
 *        
 *  65   mpeg      1.64        2/1/02 4:50:00 PM      Senthil Veluswamy SCR(s) 
 *        2982 :
 *        Added fix for NV_CMSG clients to store more than 2K of data
 *        
 *  64   mpeg      1.63        1/30/02 7:24:36 PM     Senthil Veluswamy SCR(s) 
 *        2933 :
 *        merged changes for WabashBranch (1.61.1.1)
 *        
 *  63   mpeg      1.62        12/18/01 3:52:38 PM    Senthil Veluswamy SCR(s) 
 *        2933 :
 *        Merged Wabash Branch Changes (Coderot issues)
 *        
 *  62   mpeg      1.61        7/3/01 10:36:34 AM     Tim White       SCR(s) 
 *        2178 2179 2180 :
 *        Merge branched Hondo specific code back into the mainstream source 
 *        database.
 *        
 *        
 *  61   mpeg      1.60        2/8/01 7:40:06 PM      Senthil Veluswamy bug 
 *        fix: corrected the cache dirty page writeback bit mask. Was earlier
 *        - using the offset twice erroneously
 *        Semaphore protected access to the writeback word, when used in 
 *        e2p_erase()
 *        
 *  60   mpeg      1.59        9/28/00 5:59:50 PM     Dave Wilson     Fixed 
 *        some errors which crept into vendor A builds
 *        Changed to use I2C address definitions from vendor header files
 *        
 *  59   mpeg      1.58        7/13/00 1:15:08 PM     Senthil Veluswamy added 
 *        EEPROM calls for Vendor_A
 *        
 *  58   mpeg      1.57        5/30/00 11:38:08 AM    Senthil Veluswamy added 
 *        writeback for 4 byte Checksum (only 1 byte used)
 *        updated error_log return codes
 *        
 *  57   mpeg      1.56        5/25/00 8:22:28 PM     Senthil Veluswamy made 
 *        changes to work with u_int32 header data members
 *        
 *  56   mpeg      1.55        5/2/00 8:08:50 PM      Senthil Veluswamy made 
 *        e2p_init initialize ee_init
 *        
 *  55   mpeg      1.54        4/19/00 5:18:30 PM     Senthil Veluswamy 
 *        modified e2p driver to use ee driver on klondike
 *        
 *  54   mpeg      1.53        4/14/00 5:48:06 PM     Senthil Veluswamy fixed 
 *        bug - 16 address to EE: MSB first
 *        changed 16K EE page size to 64
 *        removed usage of page_store[] in e2p_task
 *        
 *  53   mpeg      1.52        4/13/00 11:55:02 AM    Senthil Veluswamy added 
 *        support for Klondike 16K E2P
 *        
 *  52   mpeg      1.51        4/11/00 6:48:16 PM     Senthil Veluswamy no 
 *        change
 *        
 *  51   mpeg      1.50        4/6/00 12:36:10 PM     Ray Mack        fixes to 
 *        remove warnings
 *        
 *  50   mpeg      1.49        3/21/00 6:56:08 PM     Senthil Veluswamy 
 *        >"E2PDRV.c", line 948: Fatal error: Failure of internal consistency 
 *        check
 *        >Corrupted register: corrupt 5000 used 1000
 *        >
 *        >Internal inconsistency: either resource shortage or compiler fault. 
 *        If you
 *        >cannot alter your program to avoid this failure, please contact your
 *         supplier
 *        
 *        Compiler bug probably. Went away with arm 2.51
 *        
 *  49   mpeg      1.48        3/15/00 12:21:06 PM    Dave Wilson     Added 
 *        task_terminate call to prevent e2p_task from returning.
 *        
 *  48   mpeg      1.47        11/24/99 10:54:42 AM   Senthil Veluswamy Added 
 *        ERROR_WARNING flags to error_log calls for clarification.
 *        
 *  47   mpeg      1.46        11/9/99 11:00:54 AM    Senthil Veluswamy 
 *        commented out a spurious error_log() call in e2p_init()
 *        
 *  46   mpeg      1.45        10/27/99 4:56:18 PM    Dave Wilson     Changed 
 *        WAIT_FOREVER to KAL_WAIT_FOREVER.
 *        
 *  45   mpeg      1.44        10/19/99 4:43:58 PM    Senthil Veluswamy added 
 *        interface e2p_erase to erase all config data
 *        
 *  44   mpeg      1.43        10/6/99 7:28:04 PM     Senthil Veluswamy removed
 *         commented out usage of local writeback word in e2p task
 *        
 *  43   mpeg      1.42        8/2/99 5:55:38 PM      Senthil Veluswamy o:
 *        Added public interface e2p_present. used to check for e2p before
 *        - KAL is up
 *        
 *  42   mpeg      1.41        7/30/99 3:37:24 PM     Senthil Veluswamy o:
 *        changed e2p_write() to allocate() more bytes if requested to write
 *        - more bytes than currently given to client
 *        
 *  41   mpeg      1.40        7/28/99 7:19:58 PM     Senthil Veluswamy o:
 *        changed read/write interfaces to work with void* buffers
 *        completed bug fix for init(): checksum, by adding new variable
 *        - storing #bytes deleted.
 *        added conditonal to break e2p_task loop 
 *        changed return codes to standardized nvstore.h values
 *        bug fix in allocate: when size of a client increases
 *        modified read_data() to include a target buffer
 *        modified e2p_write() to work in cases where more data
 *        - was requested to be written to client than was currently 
 *        - allocated.
 *        
 *  40   mpeg      1.39        7/27/99 6:20:02 PM     Senthil Veluswamy o:
 *        updated return codes, function descriptions
 *        in the middle of fixing a bug(checksum error) in init()
 *        
 *  39   mpeg      1.38        7/26/99 7:54:56 PM     Senthil Veluswamy o:
 *        changes for new MAX EEPROM SIZE config file variable
 *        bug fix for writing writeback word
 *        
 *  38   mpeg      1.37        7/22/99 7:16:32 PM     Senthil Veluswamy o:
 *        replaced hard coded value for e2p device max write size (16) with 
 *        - WRITE_PAGE_SIZE
 *        changed Init() to initialize  a client's data when it's checksum 
 *        value
 *        - failed, instead of exiting and returning an error
 *        completed changes to allocate() to move around client data without
 *        - use of temp buffer.
 *        changed write_data() to work with 16byte source buffers only. So now,
 *        - this () will be called to do only one write cycle at a time. The 
 *        task
 *        - takes care of serializing the write requests and making sure that
 *        - they are restricted to one page (16bytes)
 *        
 *  37   mpeg      1.36        7/21/99 7:11:44 PM     Senthil Veluswamy o:
 *        removed commented out global declaration/variables. These
 *        - have been replaced as discussed in the code review.
 *        removed public interface: e2p_present(). Functionality has been 
 *        - made inherent in the e2p driver
 *        changed return value pointer types in funtion defenitions to match 
 *        - those in nvstore.h
 *        changed functionality of e2p_task to do only write backs, depending
 *        - on the value of the writeback_data word.
 *        In the middle of modifying alloc() to use writeback_data word and 
 *        - do client data block moves without a large local buffer
 *        modified client_size() to return value in one of the passed 
 *        parameters.
 *        changed read(), write() to work with new control structures as 
 *        - discussed in the code review.
 *        removed helper function: read_poll_data(). This functionality has
 *        - been included in the IIC driver. So read_data() can now be called
 *        - whether the KAL is up or not.
 *        modified read_data() to restrict reads to a fixed size so that the 
 *        temp 
 *        - read buffer used internally  will not use up a lot of the
 *        
 *  36   mpeg      1.35        7/21/99 10:33:58 AM    Senthil Veluswamy o:
 *        changes from codereview:
 *        - use of d_word instead of queue.
 *        - control/mutex semas
 *        - intial caching moved to init() from task()
 *        - use of nv_init_status_flag.
 *        - all sema calls block
 *        Completed changes upto task()
 *        
 *  35   mpeg      1.34        7/16/99 11:35:02 AM    Senthil Veluswamy o:
 *        codereview done on this version
 *        
 *  34   mpeg      1.33        7/12/99 7:06:12 PM     Senthil Veluswamy o:
 *        new check in read() to make sure client has valid data
 *        
 *  33   mpeg      1.32        7/9/99 7:38:40 PM      Senthil Veluswamy o:
 *        completed eeprom driver.
 *        changed return codes to confirm to (negative value) nv store 
 *        - convention
 *        Added Polled IIC read/writes to e2p_present and e2p_read
 *        Added new private interface: read_poll_data.
 *        removed usage of public interface: e2p_raw_data.
 *        
 *  32   mpeg      1.31        7/8/99 7:05:02 PM      Senthil Veluswamy o:
 *        modified e2p_read() to work with call before KAL is up.
 *        
 *  31   mpeg      1.30        7/7/99 6:31:20 PM      Senthil Veluswamy o:
 *        changed/assigned return codes and error log values.
 *        
 *  30   mpeg      1.29        7/6/99 6:31:12 PM      Senthil Veluswamy O:
 *        changed return types to int
 *        added new raw_read() to read e2p before kal is up
 *        
 *  29   mpeg      1.28        7/2/99 5:16:40 PM      Senthil Veluswamy o:
 *        driver done. tested. 
 *        alloc of smaller size added newly
 *        
 *  28   mpeg      1.27        6/30/99 10:53:20 PM    Senthil Veluswamy h:
 *        
 *  27   mpeg      1.26        6/30/99 7:05:14 PM     Senthil Veluswamy o:
 *        new queue buffers, pointers.
 *        updated all the other interfaces except allocate()
 *        changed e2p_ready to a semaphore for atomic checking.
 *        
 *  26   mpeg      1.25        6/29/99 10:40:16 PM    Senthil Veluswamy h:
 *        
 *  25   mpeg      1.24        6/29/99 7:13:40 PM     Senthil Veluswamy o:
 *        returned write_header code to -r1.21. ie, multiple writes if data is
 *        - across page boundary instead of Repeated Starts.
 *        Rewriting E2P driver with a new e2p_task() and caching of E2P
 *        data.
 *        
 *  24   mpeg      1.23        6/25/99 6:42:16 PM     Senthil Veluswamy o:
 *        tried to use repeated start as workaround for E2P write hold time.
 *        failed. Go back to earlier code, with time_sleep of 10 mS after
 *        each write and an extra task, "caching"
 *        
 *  23   mpeg      1.22        6/24/99 5:30:16 PM     Senthil Veluswamy O:
 *        middle of changing multiple write cycles to repeat starts
 *        
 *  22   mpeg      1.21        6/23/99 8:47:10 PM     Senthil Veluswamy o:
 *        some reformatting
 *        moved data writes before header writes(updates)
 *        bug-fix: added code to copy data in e2p_read to return buf.
 *        
 *  21   mpeg      1.20        6/23/99 1:13:00 AM     Senthil Veluswamy h:
 *        reformatting code
 *        
 *  20   mpeg      1.19        6/22/99 7:24:28 PM     Senthil Veluswamy o:
 *        minor changes for checksum (int8) data type. not complete
 *        
 *  19   mpeg      1.18        6/21/99 11:47:20 PM    Senthil Veluswamy h:
 *        finished driver
 *        
 *  18   mpeg      1.17        6/21/99 5:14:22 PM     Senthil Veluswamy O:
 *        removed usage of e2p_access semaphore
 *        access control will be taken care of by nvstore driver
 *        
 *  17   mpeg      1.16        6/21/99 12:09:14 AM    Senthil Veluswamy 
 *        checkpoint 6/20/99:h
 *        new return code: RC_E2P_PARAM_ERR 
 *        updated return values in Function headers, functions
 *        
 *  16   mpeg      1.15        6/18/99 6:26:22 PM     Senthil Veluswamy 
 *        checkpoint 6/18/99:o
 *        changed header file names
 *        populated client_size(), write_data()
 *        changed data write condition for write_header
 *        standardized return codes/error codes
 *        
 *  15   mpeg      1.14        6/17/99 6:39:10 PM     Senthil Veluswamy 
 *        checkpoint 6/17/99:o
 *        new read_data(), write_data()
 *        - moved read IIC transactions from e2p_read to read_data
 *        - moved write IIC transactions from e2p_write to write_data
 *        new e2p_client_size()
 *        - will return client data sizes.
 *        removed e2p_reallocate()
 *        - no need. size will be got from above API before allocation
 *        
 *  14   mpeg      1.13        6/16/99 6:44:24 PM     Senthil Veluswamy 
 *        checkpoint 6/16/99:o
 *        
 *  13   mpeg      1.12        6/15/99 7:54:46 PM     Senthil Veluswamy 
 *        checkpoint 6/15/99:o
 *        
 *  12   mpeg      1.11        6/14/99 12:20:12 AM    Senthil Veluswamy 
 *        checkpoint 6/13/99:h
 *        
 *  11   mpeg      1.10        6/11/99 6:47:24 PM     Senthil Veluswamy 
 *        checkpoint 6/11/99:o
 *        
 *  10   mpeg      1.9         6/10/99 9:16:44 PM     Senthil Veluswamy 
 *        checkpoint 6/10/99:h
 *        e2p read done
 *        
 *  9    mpeg      1.8         6/10/99 3:02:36 PM     Senthil Veluswamy 
 *        checkpoint 6/10/99:o
 *        
 *  8    mpeg      1.7         6/8/99 7:48:30 PM      Senthil Veluswamy 
 *        checkpoint 6/8/99:o
 *        finished(?) populating init, header_read, header_write
 *        
 *  7    mpeg      1.6         6/7/99 5:45:24 PM      Senthil Veluswamy 
 *        checkpoint 6/7/99:o
 *        
 *  6    mpeg      1.5         6/6/99 10:48:16 PM     Senthil Veluswamy 
 *        checkpoint 6/6/99:h
 *        
 *  5    mpeg      1.4         6/4/99 6:39:50 PM      Senthil Veluswamy 
 *        checkpoint 6/4/99:o
 *        
 *  4    mpeg      1.3         6/3/99 10:24:50 PM     Senthil Veluswamy 
 *        checkpoint 6/3/99:h
 *        
 *  3    mpeg      1.2         6/3/99 6:06:40 PM      Senthil Veluswamy 
 *        checkpoint 6/3/99:o
 *        
 *  2    mpeg      1.1         5/28/99 6:19:14 PM     Senthil Veluswamy 
 *        checkpoint 5/28/99:o
 *        
 *  1    mpeg      1.0         5/28/99 4:34:18 PM     Senthil Veluswamy 
 * $
 * 
 *    Rev 1.68   26 Nov 2002 16:01:28   dawilson
 * SCR(s) 4902 :
 * Changed to use label from config file to define which I2C bus device is on.
 * 
 *    Rev 1.67   25 Nov 2002 13:56:38   wangl2
 * SCR(s) 4994 :
 * When calling e2p_write(), make sure e2prom magic number is valid.
 * 
 *    Rev 1.66   11 Feb 2002 11:03:12   banghag
 * SCR(s): 3155 
 * change definitions to be taken from hwconfig.h.
 *
 *    Rev 1.65   04 Feb 2002 10:49:08   velusws
 * SCR(s) 2982 :
 * Removed 0 Initialize for bDataWriteBackPage array as this is not supported for GNU
 *
 *    Rev 1.64   01 Feb 2002 16:50:00   velusws
 * SCR(s) 2982 :
 * Added fix for NV_CMSG clients to store more than 2K of data
 ****************************************************************************/

