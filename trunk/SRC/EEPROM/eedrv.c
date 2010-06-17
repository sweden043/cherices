/****************************************************************************/
/*                            Conexant Systems                              */
/****************************************************************************/
/*                                                                          */
/* Filename:       eedrv.c                                                  */
/*                                                                          */
/* Description:    EEPROM Low level driver Source file                      */
/*                                                                          */
/* Author:         Senthil Veluswamy                                        */
/*                                                                          */
/* Date:           02/08/00                                                 */
/*                                                                          */
/* Copyright Conexant Systems, 1999                                         */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/* Notes: If this driver is built with the /DFAKE_EE compiler switch, it    */
/*        will fake out access to the EEPROM and use a RAM buffer to hold   */
/*        the file system and NVRAM private data instead.                   */
/*                                                                          */
/*        The ee_read and ee_write functions in this file are low level     */
/*        access functions which allow reading and writing to the whole     */
/*        EEPROM device, including the NVRAM private data area. The         */
/*        functions registered with OpenTV for file system access must be   */
/*        ee_fs_read and ee_fs_write from the eefs.c file since these       */
/*        specifically prevent reads and writes to this area.               */
/*                                                                          */
/****************************************************************************/

/*****************/
/* Include Files */
/*****************/
#include "hwconfig.h"
#include "eepriv.h"

#undef TIME_ANAL
//#define TIME_ANAL

#define EPG_TIME

#ifdef TIME_ANAL
#define EET_BUF_SIZE 4095
u_int8 eet_buf[EET_BUF_SIZE+1];
int32 eet_idx=0;
int32 rtc_old_time = 0,
      rtc_cur_time = 0;
/* 0xE0 - mark read request */
/* 0xE1 - mark start of read request */
/* 0xE5 - mark get write count */
/* 0xE6 - mark inside of write request */
/* 0xE7 - mark start of IIC trans */
/* 0xE8 - mark end of Write */
/* 0xEA - mark no IIC write */
#endif

#ifdef EPG_TIME
task_id_t cur_task_id;
u_int8 cur_task_priority, new_task_priority;
#endif

/***************/
/* Global Data */
/***************/
sem_id_t ee_sema;                           // EE lock semaphore to prevent 
                                            // multiple tasks from 
                                            // simultaneously accessing EE.
bool gbEEInitialized = FALSE;               // Has the ee been initialized

unsigned char RamEECache[EE_DEVICE_SIZE];
bool     bUseEECache=FALSE;

#ifdef FAKE_EE

/*******************************************/
/* Function definitions (fake EEPROM case) */
/*******************************************/

/********************************************************************/
/*  FUNCTION:   ee_present (RAM fake version)                       */
/*                                                                  */
/*  PARAMETERS: void                                                */
/*                                                                  */
/*  DESCRIPTION:Determine whether EEPROM is present.                */
/*                                                                  */
/*  RETURNS:    TRUE: in all cases                                  */
/*                                                                  */
/********************************************************************/
bool ee_present(void){

    return TRUE;
}

/********************************************************************/
/*  FUNCTION:    ee_initialized() (RAM fake version)                */
/*                                                                  */
/*  PARAMETERS:  None                                               */
/*                                                                  */
/*  DESCRIPTION: Informs the caller of whether or not the low level */
/*               EEPROM interface has been initialised.             */
/*                                                                  */
/*  RETURNS:     TRUE if initialised, FALSE otherwise               */
/********************************************************************/
bool ee_initialized(void){
    return gbEEInitialized;
}

/********************************************************************/
/*  FUNCTION:    ee_init (RAM fake version)                         */
/*                                                                  */
/*  PARAMETERS:  None                                               */
/*                                                                  */
/*  DESCRIPTION: Initialise the low level fake EE driver            */
/*                                                                  */
/*  RETURNS:     TRUE on success, FALSE on error                    */
/********************************************************************/
bool ee_init(void)
{
    // Already initialised ?
    if(gbEEInitialized){
        trace_new(TRACE_LEVEL_1 | TRACE_EE, "EE: Already Initialized.\n");
        return TRUE;
    }

    // Announce end of Init
    gbEEInitialized = TRUE;

    trace_new(TRACE_LEVEL_2 | TRACE_EE, "EE: Init Finished.\n");

    return TRUE;
}

/********************************************************************/
/*  FUNCTION:    ee_read (RAM fake version)                         */
/*                                                                  */
/*  PARAMETERS:  address - EEPROM address to read from              */
/*               buffer  - Buffer to store read data in             */
/*               count   - Number of bytes to read                  */
/*               private - Driver private data ptr (unused)         */
/*                                                                  */
/*  DESCRIPTION: Read a block of data from the EEPROM               */
/*                                                                  */
/*  RETURNS:     Number of bytes read                               */
/********************************************************************/
u_int16 ee_read(u_int16 address, voidF buffer, u_int16 count, void* private)
{
  /* Make sure request is inside file system */
  if (address < EE_DEVICE_SIZE)
  {
    /* Correct count if too large */
    count = min(count, EE_DEVICE_SIZE - address);
    
    /* Copy a chunk of bytes from our RAM buffer to the output buffer */
    memcpy(buffer, &(RamEECache[address]), count);
    return(count);
  }
  else
    return(0);
}  
        
/********************************************************************/
/*  FUNCTION:    ee_write (RAM fake version)                        */
/*                                                                  */
/*  PARAMETERS:  address - EEPROM address to write to               */
/*               buffer  - Buffer containing data to write          */
/*               count   - Number of bytes to write                 */
/*               private - Driver private data ptr (unused)         */
/*                                                                  */
/*  DESCRIPTION: Write a block of data to the EEPROM                */
/*                                                                  */
/*  RETURNS:     Number of bytes written                            */
/********************************************************************/
u_int16 ee_write(u_int16 address, voidF buffer, u_int16 count, void* private)
{
  /* Make sure request is inside file system */
  if (address < EE_DEVICE_SIZE)
  {
    /* Correct count if too large */
    count = min(count, EE_DEVICE_SIZE - address);

    /* Write the requisite number of bytes to the file system */
    memcpy(&(RamEECache[address]), buffer, count);
    return(count);
  }
  else
    return(0);
}
#else
/*******************************************/
/* Function definitions (real EEPROM case) */
/*******************************************/

/********************************************************************/
/*  ee_present()                                                    */
/*                                                                  */
/*  PARAMETERS: None                                                */
/*                                                                  */
/*  DESCRIPTION:Determine whether EEPROM is present.                */
/*                                                                  */
/*  RETURNS:    TRUE: if EEPROM Present.                            */
/*              FALSE: otherwise                                    */
/********************************************************************/
bool ee_present(void){

    bool present=FALSE;

    // Check if E2P is present.
    present = iicAddressTest(I2C_ADDR_EEPROM1, I2C_BUS_EEPROM1, FALSE);

    return present;
}

/********************************************************************/
/*  ee_initialized()                                                */
/*                                                                  */
/*  PARAMETERS: None.                                               */
/*                                                                  */
/*  DESCRIPTION:Returns Status to prevent multiple initializations  */
/*              of the EEPROM.                                      */
/*                                                                  */
/*  RETURNS:    TRUE: if EE already initialized                     */
/*              FALSE: otherwise                                    */
/********************************************************************/
bool ee_initialized(void){
    return gbEEInitialized;
}

/********************************************************************/
/*  ee_init()                                                       */
/*                                                                  */
/*  PARAMETERS: None.                                               */
/*                                                                  */
/*  DESCRIPTION:Initialize the EEPROM.                              */
/*                                                                  */
/*  RETURNS:    TRUE: if Initialization is successful,              */
/*              FALSE: otherwise                                    */
/********************************************************************/
bool ee_init(void){

    u_int16 count;

    if(gbEEInitialized){
        trace_new(TRACE_LEVEL_1 | TRACE_EE, "EE: Already Initialized.\n");
        return TRUE;
    }

    // Is the EEPROM Present?
    if(!ee_present()){
        trace_new(TRACE_LEVEL_3 | TRACE_EE, "EE: Init. EEPROM Not Found!\n");
        return FALSE;
    }

    // Create EE lock semaphore to prevent multiple tasks from simultaneously 
    // trying to access EE.
    ee_sema = sem_create(1, "EELS");

    if(ee_sema==0){
        trace_new(TRACE_LEVEL_3 | TRACE_EE, "EEI: Sema create Failed!\n");
        error_log(ERROR_WARNING | RC_EE_ERR | 0x11);
        return FALSE;
    }

    // Announce end of Init
    gbEEInitialized = TRUE;

    // Read the whole device contents into our RAM cache
    count = ee_read(0, RamEECache, EE_DEVICE_SIZE, NULL);
    if(count != EE_DEVICE_SIZE)
    {
      trace_new(TRACE_LEVEL_ALWAYS | TRACE_EE, "EE: Unable to read contents into cache!\n");
      bUseEECache = FALSE;
    }
    else
     bUseEECache = TRUE;

    trace_new(TRACE_LEVEL_2 | TRACE_EE, "EE: Init Finished.\n");

    return TRUE;
}

/********************************************************************/
/*  ee_read()                                                       */
/*                                                                  */
/*  PARAMETERS: address - EEPROM address to read from               */
/*              buffer  - Buffer to store read data in              */
/*              count   - Number of bytes to read                   */
/*              private - Driver private data ptr (unused)          */
/*                                                                  */
/*  DESCRIPTION:Read a block of data from the EEPROM                */
/*                                                                  */
/*  RETURNS:    Number of bytes read                                */
/********************************************************************/
u_int16 ee_read(u_int16 address, voidF buffer, u_int16 count, void* private){

    char *read_buf = (char *)buffer;

    int16 num_read_bytes;       // #bytes to be read in one transaction.

    int32 i;                    // to be used as a counter.

    u_int16 read_count=count;       // temp var to keep track of #bytes read

    bool read_error = FALSE,    // Indicate error reading EE.
        ee_ready;               // Is the EE ready for Reading?

    u_int32 retcode;

    // data structures for IIC transfer
    IICTRANS iicTransBuf; 
                            // entire E2P can be read  in one go.
                            // Here Reads are restricted to a fixed #bytes so 
                            // as to not use up too much stack resources.
    u_int8 RData[TRANSFER_SIZE+5];
                            // Bytes 5 to do IIC transaction:
                            // Start, Address(2), Repeated Start(for Read), Stop
    u_int8 RCmd[TRANSFER_SIZE+5];

    // Start Transaction

/*
   #ifdef TIME_ANAL
   // Mark Start of Read request
   if(eet_idx>EET_BUF_SIZE){
      eet_idx = 0;
   }
   eet_buf[eet_idx++] = 0xE0;
   #endif
*/

    if(gbEEInitialized){
        // Lock out multiple accesses
        retcode = sem_get(ee_sema, KAL_WAIT_FOREVER);

        switch(retcode){
            case RC_OK:
                trace_new(TRACE_EE | TRACE_LEVEL_1, "EER: Got Sema.\n");
            break;

            case RC_KAL_TIMEOUT:
                trace_new(TRACE_EE | TRACE_LEVEL_1, "EER: Sema Lock Timeout!\n");
                error_log(ERROR_WARNING | RC_EE_ERR | 0x12);
                return 0;
            break;

            case RC_KAL_SYSERR:
                trace_new(TRACE_EE | TRACE_LEVEL_1, "EER: Sema Lock SysErr!\n");
                error_log(ERROR_WARNING | RC_EE_ERR | 0x13);
                return 0;
            break;

            default:
                trace_new(TRACE_EE | TRACE_LEVEL_1, "EER: Sema Lock Err=%x!\n", 
                                                                retcode);
                error_log(ERROR_WARNING | RC_EE_ERR | 0x14);
                return 0;
        }

        trace_new(TRACE_EE | TRACE_LEVEL_1, "EER: Locked Sema.\n");
    }

/*
    #ifdef TIME_ANAL
    // Mark start of time stamp
    if(eet_idx>EET_BUF_SIZE){
       eet_idx = 0;
    }
    eet_buf[eet_idx++] = 0xE1;
    rtc_old_time = *(LPREG)(RTC_DATA_REG);
    #endif
*/

    if(bUseEECache)
    {
      /* Read from the RAM cache rather than the device */
      memcpy(buffer, &(RamEECache[address]), count);
      read_count = 0;
    }
    else
    {
      /* Read from the device rather than the cache */
      do{
          // because of the possibility of being called for a read immediately
          // after a  write, first determine if the EE is ready (will respond to 
          // addressing) before sending read request.
          ee_ready = iicAddressTest(I2C_ADDR_EEPROM1, I2C_BUS_EEPROM1, FALSE);
          while(!ee_ready){
              task_time_sleep(EE_WRITE_STIME);
                                      // To account for the E2P max write cycle 
                                      // time of 10mS.
              ee_ready = iicAddressTest(I2C_ADDR_EEPROM1, I2C_BUS_EEPROM1, FALSE);
          }

          if(gbEEInitialized){
              trace_new(TRACE_EE | TRACE_LEVEL_1, "EER: Loading Params.\n");
          }

          if(read_count>TRANSFER_SIZE){
              num_read_bytes = TRANSFER_SIZE;
          }
          else{
              num_read_bytes = read_count;
          }

          if(gbEEInitialized){
              trace_new(TRACE_EE | TRACE_LEVEL_1, "EER: Loading Params.\n");
          }

          // Load IIC bus transaction command/data words
          RCmd[0] = IIC_START;             // Take Data stored in Data[0].
          RCmd[1] = IIC_DATA;              // Take Data stored in Data[1].
          RCmd[2] = IIC_DATA;              // Take Data stored in Data[2].
          RCmd[3] = IIC_START;             // Take Data stored in Data[3].
          RData[0] = I2C_ADDR_EEPROM1 | WRITE_FLAG; // Device Addr, R/W
          RData[1] = (char)(address>>8);
                                              // HigherB Address for Read/Writes
          RData[2] = (char)address;
                                              // LowerB Address for Read/Writes
          RData[3] = I2C_ADDR_EEPROM1 | READ_FLAG;  // Device Addr, R/W

          for(i=0;i<num_read_bytes-1;i++){
              RCmd[4+i] = IIC_DATA | IIC_ACK; // Data read from IIC bus is stored 
                                              // here.Multiple Read will continue 
                                              // as long as each Read is 
                                              // acknowledged.
          }

          RCmd[4+(num_read_bytes-1)] = IIC_DATA;// No ack on last data cmd.
                                              // This stops multiple read.
          RCmd[4+(num_read_bytes-1)+1] = IIC_STOP; // End of Read. 
                                          // Total #bytes = #client_data+4
                                          // [0]..[4]: Start, Address, Repeat S,
                                          // [4]..[4+#client_data]: Client data
                                          // [4+#client_data+1]: Stop
          iicTransBuf.pCmd = RCmd;            // Command Words here.
          iicTransBuf.dwCount = num_read_bytes+5;  
                                          // The number of Command Words
          iicTransBuf.pData = RData;      // Client data is contained in Data 
                                      // array in elements [3]..[3+#client_data]

          trace_new(TRACE_EE | TRACE_LEVEL_1, "EER: Starting Read.\n");

          iicTransaction(&iicTransBuf, I2C_BUS_EEPROM1);

          switch(iicTransBuf.dwError){    // What is the Read status?

          case IIC_ERROR_NOERR:           // No Error. Read Successful.
              if(gbEEInitialized){
                  trace_new(TRACE_EE | TRACE_LEVEL_1, "EER: Read Successful.\n");
              }
          break;

          case IIC_ERROR_NOACK:
              if(gbEEInitialized){
                  trace_new(TRACE_EE | TRACE_LEVEL_3, \
                      "EER: Read Failed! Ack not received after byte write.\n");
                  error_log(ERROR_WARNING | RC_EE_ERR | 0x15);
              }
              read_error = TRUE;
          break;

          case IIC_ERROR_NOADDRESSACK:
              if(gbEEInitialized){
                  trace_new(TRACE_EE | TRACE_LEVEL_3, \
                  "EER: Read Failed! Device not responding to address.\n");
                  error_log(ERROR_WARNING | RC_EE_ERR | 0x16);
              }
              read_error = TRUE;
          break;

          case IIC_ERROR_INVALIDDATA:
              if(gbEEInitialized){
                  trace_new(TRACE_EE | TRACE_LEVEL_3, \
                              "EER: Read Failed! Invalid data pointer.\n");
                  error_log(ERROR_WARNING | RC_EE_ERR | 0x17);
              }
              read_error = TRUE;
          break;

          case IIC_ERROR_INVALIDCMD:
              if(gbEEInitialized){
                  trace_new(TRACE_EE | TRACE_LEVEL_3, \
                              "EER: Read Failed! Invalid command pointer.\n");
                  error_log(ERROR_WARNING | RC_EE_ERR | 0x18);
              }
              read_error = TRUE;
          break;

          default:
              if(gbEEInitialized){
                  trace_new(TRACE_EE | TRACE_LEVEL_3, \
                                      "EER: Read Failed! Reason Unknown!!.\n");
                  error_log(ERROR_WARNING | RC_EE_ERR | 0x19);
              }
              read_error = TRUE;
          }

          if(read_error){
              if(gbEEInitialized){
                  trace_new(TRACE_EE | TRACE_LEVEL_3, \
                                          "EER: Error reading data!\n");
              }
              break;
          }
          else{
              // Copy data to cache.
              memcpy(read_buf, RData+4, num_read_bytes);

              // Updating count, address, Read Buf ptr for next read cycle, 
              // if needed.
              read_count -= num_read_bytes;
              address += num_read_bytes;
              read_buf += num_read_bytes;
          }
      }
      while(read_count>0);
    }

/*
   #ifdef TIME_ANAL
    rtc_cur_time = *(LPREG)(RTC_DATA_REG);
    if((rtc_cur_time-rtc_old_time)<0xff){
       eet_buf[eet_idx++] = (rtc_cur_time-rtc_old_time);
    }
    else{
       eet_buf[eet_idx++] = 0xff;
    }

    trace_new(TRACE_TST | TRACE_LEVEL_4, "EER: Read Time=%x\n", 
                                          (rtc_cur_time-rtc_old_time));
   #endif
*/

    if(gbEEInitialized){
        // Unlock access
        sem_put(ee_sema);

        trace_new(TRACE_EE | TRACE_LEVEL_1, "EER: Read data. Returning.\n");
    }

    return (count-read_count);
}

/********************************************************************/
/*  ee_write                                                        */
/*                                                                  */
/*  PARAMETERS: address - EEPROM address to write to                */
/*              buffer  - Buffer containing data to write           */
/*              count   - Number of bytes to write                  */
/*              private - Driver private data ptr (unused)          */
/*                                                                  */
/*  DESCRIPTION:Write a block of data to the EEPROM                 */
/*                                                                  */
/*  RETURNS:    Number of bytes written                             */
/********************************************************************/
u_int16 ee_write(u_int16 address, voidF buffer, u_int16 count, void* private){

    char *write_buf = (char *)buffer;

    bool ee_ready;          // Can EE do next write request?

    u_int16 write_count=count;
                            // temp var to keep track of #bytes to write

    int32 i;                // counter.

    u_int16 num_write_possible, // #that can be written in one go(page write).
                                // This depends on start addr, with max possible
                                // in a page write being 16.
            num_write_bytes;    // Number of bytes to write this IIC cycle.

    bool write_error = FALSE;   // Indicate error Writing EE.

    u_int32 retcode;

    // data structures for IIC transfer
    IICTRANS iicTransBuf;
                            // Can write a max of 64 bytes (WRITE_PAGE_SIZE) at 
                            // a time. 
    u_int8 WData[TRANSFER_SIZE+4];    
                            // 4 Bytes to do IIC transaction:
                            // Start, Address(2), Stop
    u_int8 WCmd[TRANSFER_SIZE+4];

    // Start Transaction
    if(!gbEEInitialized){
        retcode = ee_init();
        if(!retcode){
            trace_new(TRACE_EE | TRACE_LEVEL_3, "EEW: EE Init Failed!\n");
            return 0;
        }
    }

    // Lock out multiple accesses
    retcode = sem_get(ee_sema, KAL_WAIT_FOREVER);

    switch(retcode){
        case RC_OK:
            trace_new(TRACE_EE | TRACE_LEVEL_1, "EEW: Got Sema.\n");
        break;

        case RC_KAL_TIMEOUT:
            trace_new(TRACE_EE | TRACE_LEVEL_1, "EEW: Sema Lock Timeout!\n");
            error_log(ERROR_WARNING | RC_EE_ERR | 0x1A);
            return 0;
        break;

        case RC_KAL_SYSERR:
            trace_new(TRACE_EE | TRACE_LEVEL_1, "EEW: Sema Lock SysErr!\n");
            error_log(ERROR_WARNING | RC_EE_ERR | 0x1B);
            return 0;
        break;

        default:
            trace_new(TRACE_EE | TRACE_LEVEL_1, "EEW: Sema Lock Err=%x!\n", 
                                                            retcode);
            error_log(ERROR_WARNING | RC_EE_ERR | 0x1C);
            return 0;
    }

        trace_new(TRACE_EE | TRACE_LEVEL_1, "EEW: Locked Sema.\n");

   #ifdef TIME_ANAL
   if(eet_idx>EET_BUF_SIZE){
      eet_idx = 0;
   }
   eet_buf[eet_idx++] = 0xEF;
   #endif

   #ifdef EPG_TIME
   /* Get current priority */
   cur_task_id = task_id();
   cur_task_priority = task_priority(cur_task_id, 0);
   #endif
   
   #ifdef TIME_ANAL
   /* Write the task priority */
   if(eet_idx>EET_BUF_SIZE){
      eet_idx = 0;
   }
   eet_buf[eet_idx++] = cur_task_priority;
   #endif
   
   #ifdef EPG_TIME
   /* Increase task priority */
   new_task_priority = task_priority(cur_task_id, 5);
   #endif

    #ifdef TIME_ANAL
    /* Mark Start of Write request */
    if(eet_idx>EET_BUF_SIZE){
       eet_idx = 0;
    }
    eet_buf[eet_idx++] = 0xE5;
 
    /* Write the number of bytes */
    if(eet_idx>EET_BUF_SIZE){
       eet_idx = 0;
    }
    if(count>0xff){
      eet_buf[eet_idx++] = 0xff;
    }
    else{
      eet_buf[eet_idx++] = count;
    }

    /* Mark start of Write compare */
    if(eet_idx>EET_BUF_SIZE){
       eet_idx = 0;
    }
    eet_buf[eet_idx++] = 0xE6;
    rtc_old_time = *(LPREG)(RTC_DATA_REG);
    #endif

    /* Check that the data to be written is different from the data already stored */
    if (memcmp(&(RamEECache[address]), buffer, min(count, EE_DEVICE_SIZE - address)))
    {
      /* Write the requisite number of bytes to the RAM cache */
    
      memcpy(&(RamEECache[address]), buffer, min(count, EE_DEVICE_SIZE - address));

      /* Then go ahead and write them to the device itself */
      do{
          // because of the possibility of being called for back to back writes,
          // first determine if the EE is ready (will respond to addressing) 
          // before sending next write request.
          ee_ready = iicAddressTest(I2C_ADDR_EEPROM1, I2C_BUS_EEPROM1, FALSE);
          while(!ee_ready){
              task_time_sleep(EE_WRITE_STIME);
                                      // To account for the E2P max write cycle 
                                      // time of 10mS.
              ee_ready = iicAddressTest(I2C_ADDR_EEPROM1, I2C_BUS_EEPROM1, FALSE);
          }

          trace_new(TRACE_EE | TRACE_LEVEL_1, "EEW: Loading Params.\n");

          // Load IIC bus and write the bytes to E2P
          // load IIC command and data buffers
          // Start and Address data.
          WCmd[0] = IIC_START;            // Take Data stored in Data[0].
          WCmd[1] = IIC_DATA;             // Take Data stored in Data[1].
          WCmd[2] = IIC_DATA;             // Take Data stored in Data[2].
          WData[0] = I2C_ADDR_EEPROM1 | WRITE_FLAG; // Device Addr, R/W
          WData[1] = (char)(address>>8);
                                          // UpperB Address for Writes
          WData[2] = (char)address;
                                             // LowerB Address for Writes

          num_write_possible = (TRANSFER_SIZE-(address%TRANSFER_SIZE));
          num_write_bytes = (num_write_possible>write_count) ? write_count 
                                                      : num_write_possible;

          for(i=0;i<num_write_bytes;i++){ // copy bytes to DATA buf.
              WCmd[3+i] = IIC_DATA;
              WData[3+i] = *write_buf++;  // Update data source pointer
          }

          WCmd[3+num_write_bytes] = IIC_STOP;     // End of this write.
          iicTransBuf.dwCount = 3+num_write_bytes+1;
                                              // Start, Data, Stop bytes

          iicTransBuf.pCmd = WCmd;    // Command Words found here.
          iicTransBuf.pData = WData;  // Data words here.

          trace_new(TRACE_EE | TRACE_LEVEL_1, \
          "EEW: Starting Write. #IIC write bytes=%x.\n", iicTransBuf.dwCount);

      #ifdef TIME_ANAL
       /* Mark start of IIC Write  */
       if(eet_idx>EET_BUF_SIZE){
          eet_idx = 0;
       }
       rtc_cur_time = *(LPREG)(RTC_DATA_REG);
       if(rtc_cur_time>=rtc_old_time){
          eet_buf[eet_idx++] = (u_int8)(rtc_cur_time-rtc_old_time);
          if(eet_idx>EET_BUF_SIZE){
             eet_idx = 0;
          }
          eet_buf[eet_idx++] = (u_int8)((rtc_cur_time-rtc_old_time)>>8);
       }
       else{
          eet_buf[eet_idx++] = 0xff;
          if(eet_idx>EET_BUF_SIZE){
             eet_idx = 0;
          }
          eet_buf[eet_idx++] = 0xff;
       }
       if(eet_idx>EET_BUF_SIZE){
          eet_idx = 0;
       }
       eet_buf[eet_idx++] = 0xE7;
      #endif
          iicTransaction(&iicTransBuf, I2C_BUS_EEPROM1);
      #ifdef TIME_ANAL
       /* Mark End of IIC Write  */
       if(eet_idx>EET_BUF_SIZE){
          eet_idx = 0;
       }
       rtc_cur_time = *(LPREG)(RTC_DATA_REG);
       if(rtc_cur_time>=rtc_old_time){
          eet_buf[eet_idx++] = (u_int8)(rtc_cur_time-rtc_old_time);
          if(eet_idx>EET_BUF_SIZE){
             eet_idx = 0;
          }
          eet_buf[eet_idx++] = (u_int8)((rtc_cur_time-rtc_old_time)>>8);
       }
       else{
          eet_buf[eet_idx++] = 0xff;
          if(eet_idx>EET_BUF_SIZE){
             eet_idx = 0;
          }
          eet_buf[eet_idx++] = 0xff;
       }
       if(eet_idx>EET_BUF_SIZE){
          eet_idx = 0;
       }
       eet_buf[eet_idx++] = 0xE8;
      #endif
          switch(iicTransBuf.dwError){    // What is the Write status?

          case IIC_ERROR_NOERR:           // No Error. Write Successful.
              trace_new(TRACE_EE | TRACE_LEVEL_1, "EEW: Write Successful.\n");
          break;

          case IIC_ERROR_NOACK:
              trace_new(TRACE_EE | TRACE_LEVEL_3, \
                  "EEW: Write Failed! Ack not received after byte write.\n");
              error_log(ERROR_WARNING | RC_EE_ERR | 0x1D);
              write_error = TRUE;
          break;

          case IIC_ERROR_NOADDRESSACK:
              trace_new(TRACE_EE | TRACE_LEVEL_3, \
                  "EEW: Write Failed! Device not responding to address.\n");
              error_log(ERROR_WARNING | RC_EE_ERR | 0x1E);
              write_error = TRUE;
          break;

          case IIC_ERROR_INVALIDDATA:
              trace_new(TRACE_EE | TRACE_LEVEL_3, \
                              "EEW: Write Failed! Invalid data pointer.\n");
              error_log(ERROR_WARNING | RC_EE_ERR | 0x1F);
              write_error = TRUE;
          break;

          case IIC_ERROR_INVALIDCMD:
              trace_new(TRACE_EE | TRACE_LEVEL_3, \
                          "EEW: Write Failed! Invalid command pointer.\n");
              error_log(ERROR_WARNING | RC_EE_ERR | 0x20);
              write_error = TRUE;
          break;

          default:
              trace_new(TRACE_EE | TRACE_LEVEL_3, \
                                  "EEW: Write Failed! Reason Unknown!!.\n");
              error_log(ERROR_WARNING | RC_EE_ERR | 0x21);
              write_error = TRUE;
          }

          if(write_error){
              trace_new(TRACE_EE | TRACE_LEVEL_3, \
                                          "EER: Error writing data!\n");
              break;
          }
          else{
              // Updating count, address for next read cycle, if needed.
              write_count -= num_write_bytes; // How many more bytes to write?
              address += num_write_bytes;     // Capacity is in bytes, so 
                                              // #bytes written can be used to
                                              // update addr
          }
      }
      while(write_count>0);
    }
    else
    {
      #ifdef TIME_ANAL
      if(eet_idx>EET_BUF_SIZE){
         eet_idx = 0;
      }
      eet_buf[eet_idx++] = 0xEA;
      #endif
      /* Data passed to write was the same as the data already stored so */
      /* just return an appropriate value                                */
      trace_new(TRACE_EE | TRACE_LEVEL_2, "EEW: Data passed is the same as that already stored.\n");
      write_count = count - min(count, EE_DEVICE_SIZE - address);
    }
    
    #ifdef TIME_ANAL
     if(eet_idx>EET_BUF_SIZE){
        eet_idx = 0;
     }
     rtc_cur_time = *(LPREG)(RTC_DATA_REG);
     if(rtc_cur_time>=rtc_old_time){
        eet_buf[eet_idx++] = (u_int8)(rtc_cur_time-rtc_old_time);
        if(eet_idx>EET_BUF_SIZE){
           eet_idx = 0;
        }
        eet_buf[eet_idx++] = (u_int8)((rtc_cur_time-rtc_old_time)>>8);
     }
     else{
        eet_buf[eet_idx++] = 0xff;
        if(eet_idx>EET_BUF_SIZE){
           eet_idx = 0;
        }
        eet_buf[eet_idx++] = 0xff;
     }

    trace_new(TRACE_TST | TRACE_LEVEL_4, "EEW: Write Time=%x\n", 
                                          (rtc_cur_time-rtc_old_time));
    #endif
 
    // Unlock access
    sem_put(ee_sema);

   #ifdef EPG_TIME
   /* Decrease task priority */
   task_priority(cur_task_id, cur_task_priority-new_task_priority);
   #endif

    trace_new(TRACE_EE | TRACE_LEVEL_1, "EEW: write done. Returning.\n");

    return (count-write_count);
}

#endif /* FAKE_EE */

