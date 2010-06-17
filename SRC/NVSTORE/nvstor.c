/****************************************************************************/
/*                            Conexant Systems                              */
/****************************************************************************/
/*                                                                          */
/* Filename:    nvstor.c                                                    */
/*                                                                          */
/* Description: NonVolatile Storage driver source file                      */
/*              This driver is a thin abstraction layer which makes the     */
/*              use of a particular NVStorage device transparent to the     */
/*              other software components                                   */
/* Author:      Senthil Veluswamy                                           */
/*                                                                          */
/* Date:        6/2/99                                                      */
/*                                                                          */
/* Copyright Conexant Systems, 1999                                         */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/

/*****************/
/* Include Files */
/*****************/
#include "stbcfg.h"
#include "kal.h"
#include "retcodes.h"
#include "startup.h"
#include "nvstore.h"
#include "e2p.h"
#include "flashcfg.h"

/***************/
/* Global Data */
/***************/
sem_id_t nv_access;         // Serialize accesses to NV device

bool nv_init_done = FALSE;  // Was the driver initalized?

// Function Pointers to switch between nv_devices: e2p and flash
u_int32 (*pfn_nv_allocate)(u_int8 Client_Id, u_int32 Num_Bytes);
u_int32 (*pfn_nv_client_size)(u_int8 Client_Id, u_int32 *Client_Data_Size);
u_int32 (*pfn_nv_read)(u_int8 Client_Id, void *Read_Buf, 
                    u_int32 Num_Bytes_to_Read, u_int32 Read_Offset, 
                                    u_int32 *Num_Bytes_Actually_Read);
u_int32 (*pfn_nv_write)(u_int8 Client_Id, void *Write_Buf, 
                        u_int32 Num_Bytes_to_Write, u_int32 Write_Offset, 
                                        u_int32 *Num_Bytes_Actually_Written);
u_int32 (*pfn_nv_erase)(void);

/***************/
/* Extern Data */
/***************/
extern bool bKalUp;                  // To be used for self initialization

/********************/
/* Extern Functions */
/********************/
#ifdef DRIVER_INCL_FLASHCFG
extern u_int32 read_operation(u_int8 id, void* buffer, u_int32 size, 
                                                            u_int32 offset);
#endif

/************************/
/* Function definitions */
/************************/

/********************************************************************/
/*  nv_init()                                                       */
/*                                                                  */
/*  PARAMETERS: (void)                                              */
/*                                                                  */
/*  DESCRIPTION:Checks which NV device is used and initializes it   */
/*                                                                  */
/*  RETURNS:    TRUE: on success.                                   */
/*              FALSE: on failure.                                  */
/********************************************************************/
bool nv_init(void){

        bool init_status = FALSE;
        bool init_f_status = FALSE;

        if(nv_init_done)
          return(TRUE);

        /* Initialise both possible storage media */
        #ifdef DRIVER_INCL_E2P
        init_status   = e2p_init();
        #endif
        #ifdef DRIVER_INCL_FLASHCFG
        init_f_status = flash_init();
        #endif

        // Priority for EEPROM.
        #ifdef DRIVER_INCL_E2P
        if(init_status==TRUE){
            trace_new(TRACE_NV | TRACE_LEVEL_2, "NVInit: E2P Initialized.\n");

            // Point to E2P functions.
            pfn_nv_allocate = e2p_allocate;
            pfn_nv_client_size = e2p_client_size;
            pfn_nv_read = e2p_read;
            pfn_nv_write = e2p_write;
            pfn_nv_erase = e2p_erase;
        }
        else {
        #endif
            #ifdef OPENTV_12
            nv_init_done = FALSE;
            return FALSE;
            #else
            /* Dont do this if for a OTV 1.2 system */
            // Try FLASH now.
            init_status = init_f_status;
            if(init_status==FALSE){ // Init failed!
                trace_new(TRACE_NV | TRACE_LEVEL_3, \
                                                "NVInit: Detect-Init failed.");
                error_log(RC_NV_ERR | 1);
                return FALSE;
            }

            trace_new(TRACE_NV | TRACE_LEVEL_2, "NVInit: Flash Initialized.\n");

            // Point to Flash functions.
            #ifdef DRIVER_INCL_FLASHCFG
            pfn_nv_allocate = flash_alloc;
            pfn_nv_client_size = flash_query_size;
            pfn_nv_read = flash_read;
            pfn_nv_write = flash_write;
            pfn_nv_erase = flash_erase;
            #endif
            #endif // OPENTV_12
        #ifdef DRIVER_INCL_E2P
        }
        #endif

    // Create driver resources and complete initialization

    // Create semaphore with one outstanding token.
    nv_access = sem_create(1, "NVES");

    if(nv_access==0){
        trace_new(TRACE_NV | TRACE_LEVEL_3, \
                                        "NVInit: Err! creating Sema.\n");
        error_log(RC_NV_ERR | 2);
        return FALSE;
    }

    trace_new(TRACE_NV | TRACE_LEVEL_2, \
                            "NVInit: Access Sema created successfully.\n");

    trace_new(TRACE_NV | TRACE_LEVEL_2, \
                            "NVInit: status=%d. Returning.\n", init_status);

    // Announce Init done.
    nv_init_done = TRUE;

    return init_status;
}

/********************************************************************/
/*  nv_allocate()                                                   */
/*                                                                  */
/*  PARAMETERS: #bytes requested and the ID of the Client which     */
/*              wants it                                            */
/*                                                                  */
/*  DESCRIPTION:Allocates a specified size of memory (for a         */
/*              specified client) from NV device used.              */
/*                                                                  */
/*  RETURNS:    RC_OK on successfully allocating memory             */
/*              On failure: a positive value (coded in the NV       */
/*              header)explaining the cause of the failure.         */
/********************************************************************/
u_int32 nv_allocate(u_int8 Client_Id, u_int32 Num_Bytes){

    int32 retcode;

    // Is the driver initialized?
    if(!nv_init_done){
        // Do Self initialization
        if(bKalUp){
            if(!nv_init()){
                trace_new(TRACE_NV | TRACE_LEVEL_3, "NVAlloc: Init failed!\n");
                error_log(RC_NV_ERR | 3);
                return RC_NV_INIT_ERR;
            }
            else{
                error_log(RC_KAL_SYSERR);
                return RC_KAL_SYSERR;
            }
        }
    }

    // Lock access before communicating with device.
    retcode = sem_get(nv_access, MAX_NV_WAIT_TIME);

    if(retcode!=RC_OK){
        trace_new(TRACE_NV | TRACE_LEVEL_3, \
                "NVAlloc: failed. Sem Access Err! Retcode=%d.\n", retcode);
        error_log(RC_NV_ERR | 4);
        return RC_NV_SEMA_ERR;
    }

    // Call device Interface
    retcode = pfn_nv_allocate(Client_Id, Num_Bytes);

    // Return semaphore to enable other calls to access E2P
    sem_put(nv_access);

    trace_new(TRACE_NV | TRACE_LEVEL_2, \
                                "NVAlloc: retcode=%d. Returning\n", retcode);

    return retcode;
}

/********************************************************************/
/*  nv_client_size()                                                */
/*                                                                  */
/*  PARAMETERS: Client Id whose size needs to be known, Pointer to  */
/*              return the memory size.                             */
/*                                                                  */
/*  DESCRIPTION:Return the current memory size of a client block in */
/*              the NV device.                                      */
/*                                                                  */
/*  RETURNS:    Size of Client data                                 */
/*              can be 0, if there is no client data.               */
/*              On failure: a positive value (coded in the NV       */
/*              header)explaining the cause of the failure.         */
/********************************************************************/
u_int32 nv_client_size(u_int8 Client_Id, u_int32 *Client_Data_Size){

    int32 retcode;

    // Is the driver initialized?
    if(!nv_init_done){
        // Do Self initialization
        if(bKalUp){
            if(!nv_init()){
                trace_new(TRACE_NV | TRACE_LEVEL_3, "NVCSize: Init failed!\n");
                error_log(RC_NV_ERR | 5);
                return RC_NV_INIT_ERR;
            }
            else{
                error_log(RC_KAL_SYSERR);
                return RC_KAL_SYSERR;
            }
        }
    }

    // Lock access before communicating with device.
    retcode = sem_get(nv_access, MAX_NV_WAIT_TIME);

    if(retcode!=RC_OK){
        trace_new(TRACE_NV | TRACE_LEVEL_3, \
                "NVCSize: failed. Sem Access Err! Retcode=%d.\n", retcode);
        error_log(RC_NV_ERR | 0x6);
        return RC_NV_SEMA_ERR;
    }

    // Call nv device Interface
    retcode = pfn_nv_client_size(Client_Id, Client_Data_Size);

    // Return semaphore to enable other calls to access E2P
    sem_put(nv_access);

    trace_new(TRACE_NV | TRACE_LEVEL_2, "NVCSize: retcode=%d.\n", retcode);

    return retcode;
}

/********************************************************************/
/*  nv_read()                                                       */
/*                                                                  */
/*  PARAMETERS: Client Id, Pointer to return the data read,         */
/*              Number of bytes of data requested, Address in Client*/
/*              block to start read at, Number of bytes that were   */
/*              read from the NV device.                            */
/*                                                                  */
/*  DESCRIPTION:Reads a specified number of bytes, starting from a  */
/*              specified address from a particular client data     */
/*              block in the NV device and returns it.              */
/*                                                                  */
/*  RETURNS:    Size of Client data actually read.                  */
/*              can be 0, if there is no client data.               */
/*              On failure: a positive value (coded in the NV       */
/*              header)explaining the cause of the failure.         */
/********************************************************************/
u_int32 nv_read(u_int8 Client_Id, void *Read_Buf, u_int32 Num_Bytes_to_Read, 
                        u_int32 Read_Offset, u_int32 *Num_Bytes_Actually_Read){

    int32 retcode, i;
    BANK_INFO FlashInfo[NUM_FLASH_BANKS];
    bool present=FALSE;

    // Is the driver initialized?
    if(!nv_init_done){

        // Detect which device is actually present and go from there

        #ifdef DRIVER_INCL_E2P
        // Priority for EEPROM.
        present = e2p_present();

        if(present){ // e2p has higher priority
            retcode = e2p_read(Client_Id, Read_Buf, Num_Bytes_to_Read, 
                                    Read_Offset, Num_Bytes_Actually_Read);
        }
        else {
        #endif
            // Now parse Flash Info to determine if Flash is present.
            present = FALSE;

            for(i=0;i<NUM_FLASH_BANKS;i++){
                if(FlashInfo[i].ROMType==ROM_TYPE_FLASH){
                    present = TRUE;
                    break;
                }
            }

            if(!present){ // Trouble.
                return RC_NV_UNK_DEV_ERR;
            }

            #ifdef DRIVER_INCL_FLASHCFG
            // call low level flash read
            *Num_Bytes_Actually_Read = read_operation(Client_Id, Read_Buf, 
                                        Num_Bytes_to_Read, Read_Offset);
            #endif

            retcode = RC_OK;
        #ifdef DRIVER_INCL_E2P
        }
        #endif

        return retcode;
    }

    // Driver Initialization was done.

    // Lock access before communicating with device.
    retcode = sem_get(nv_access, MAX_NV_WAIT_TIME);

    if(retcode!=RC_OK){
        trace_new(TRACE_NV | TRACE_LEVEL_3, \
                            "NVRead: Sem Access Err! Retcode=%d.\n", retcode);
        error_log(RC_NV_ERR | 0x7);
        return RC_NV_SEMA_ERR;
    }

    // Call device Interface
    retcode = pfn_nv_read(Client_Id, Read_Buf, Num_Bytes_to_Read, 
                                        Read_Offset, Num_Bytes_Actually_Read);

    // Return semaphore to enable other calls to access E2P
    sem_put(nv_access);

    trace_new(TRACE_NV | TRACE_LEVEL_2, \
                                "NVRead: retcode=%d. Returning\n", retcode);

    return retcode;
}

/********************************************************************/
/*  nv_write()                                                      */
/*                                                                  */
/*  PARAMETERS: Client Id, Source (pointer) of the data to be       */
/*              written.#bytes requested to be written, Offset (in  */
/*              client block) to start write at, Number of bytes    */
/*              successfully written to the NV device.              */
/*                                                                  */
/*  DESCRIPTION:Writes a specified number of bytes, starting from a */
/*              specified address to a particular client data block */
/*              in the NV device.                                   */
/*                                                                  */
/*  RETURNS:    #bytes of Client data actually written.             */
/*              On failure: a positive value (coded in the NV       */
/*              header)explaining the cause of the failure.         */
/********************************************************************/
u_int32 nv_write(u_int8 Client_Id, void *Write_Buf, 
                        u_int32 Num_Bytes_to_Write, u_int32 Write_Offset, 
                                        u_int32 *Num_Bytes_Actually_Written){

    int32 retcode;

    // Is the driver initialized?
    if(!nv_init_done){
        // Do Self initialization
        if(bKalUp){
            if(!nv_init()){
                trace_new(TRACE_NV | TRACE_LEVEL_3, "NVWrite: Init failed!\n");
                error_log(RC_NV_ERR | 8);
                return RC_NV_INIT_ERR;
            }
        }
    }

    // Lock access before communicating with device.
    retcode = sem_get(nv_access, MAX_NV_WAIT_TIME);

    if(retcode!=RC_OK){
        trace_new(TRACE_NV | TRACE_LEVEL_3, \
                        "NVWrite: Sem Access Err! Retcode=%d.\n", retcode);
        error_log(RC_NV_ERR | 0x9);
        return RC_NV_SEMA_ERR;
    }

    // Call E2P Interface
    retcode = pfn_nv_write(Client_Id, Write_Buf, Num_Bytes_to_Write, 
                                    Write_Offset, Num_Bytes_Actually_Written);

    // Return semaphore to enable other calls to access E2P
    sem_put(nv_access);

    trace_new(TRACE_NV | TRACE_LEVEL_2, \
                                "NVWrite: retcode=%d. Returning\n", retcode);

    return retcode;
}


/********************************************************************/
/*  nv_erase()                                                      */
/*                                                                  */
/*  PARAMETERS: None.                                               */
/*                                                                  */
/*  DESCRIPTION:Invalidate all the config data stored in the NV     */
/*              device.                                             */
/*                                                                  */
/*  RETURNS:    RC_OK: if successful,                               */
/*              On failure: a positive value (coded in the NV       */
/*              header)explaining the cause of the failure.         */
/********************************************************************/
u_int32 nv_erase(void){

    int32 retcode;

    // Is the driver initialized?
    if(!nv_init_done){
        // Do Self initialization
        if(bKalUp){
            if(!nv_init()){
                trace_new(TRACE_NV | TRACE_LEVEL_3, "NVErase: Init failed!\n");
                error_log(RC_NV_ERR | 0xA);
                return RC_NV_INIT_ERR;
            }
            else{
                error_log(RC_KAL_SYSERR);
                return RC_KAL_SYSERR;
            }
        }
    }

    // Lock access before communicating with device.
    retcode = sem_get(nv_access, MAX_NV_WAIT_TIME);

    if(retcode!=RC_OK){
        trace_new(TRACE_NV | TRACE_LEVEL_3, \
                "NVErase: failed. Sem Access Err! Retcode=%d.\n", retcode);
        error_log(RC_NV_ERR | 0xB);
        return RC_NV_SEMA_ERR;
    }

    // Call nv device Interface
    retcode = pfn_nv_erase();

    // Return semaphore to enable other calls to access E2P
    sem_put(nv_access);

    trace_new(TRACE_NV | TRACE_LEVEL_2, "NVErase: retcode=%d.\n", retcode);

    return retcode;
}
