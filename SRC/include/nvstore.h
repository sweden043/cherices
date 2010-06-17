/****************************************************************************/
/*                            Conexant Systems                              */
/****************************************************************************/
/*                                                                          */
/* Filename:       nvstore.h                                                */
/*                                                                          */
/* Description:    NonVolatile Storage header file                          */
/*                                                                          */
/* Author:         Senthil Veluswamy                                        */
/*                                                                          */
/* Date:           6/2/99                                                   */
/*                                                                          */
/* Copyright Conexant Systems, 1999                                         */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/

/****************************************************************************
 * $Header: nvstore.h, 33, 4/16/04 3:45:12 AM, Xiao Guang Yan$
 ****************************************************************************/

#ifndef _NV_STORE_
#define _NV_STORE_

/***********/
/* Aliases */
/***********/
// macro values

// Identify Clients. This is also offset to determine NVmemory storage location 
// for client.
#define NV_BSP_CLIENT_ID        0
#define NV_KAL_CLIENT_ID        1
#define NV_CRYPTO_CLIENT_ID     2
#define NV_NDSCORE_CLIENT_ID    3
#ifndef DRIVER_INCL_NV_CMSG
 #define ROM_CONFIG_NUM_CLIENTS  4
#else
 #define ROM_CONFIG_NUM_CLIENTS  7
#endif /* DRIVER_INCL_NV_CMSG */

#define MAX_NV_WAIT_TIME        0x64    // 100mS. Sema block time

#define NUM_FLASH_BANKS         4       // There can be upto 4 banks of Flash.

// Return Codes
#define RC_NV_ERR       (MOD_NVS + 0x0)

#define RC_NV_SEMA_ERR  (MOD_NVS + 0x30)    // Serial sema error
#define RC_NV_INIT_ERR  (MOD_NVS + 0x31)    // NV device Init failed
#define RC_NV_UNK_DEV_ERR (MOD_NVS + 0x32)  // Unknown NV Device.

#define RC_NV_ADDR_ERR  (MOD_NVS + 0x40)    // Parameter Address wrong.
#define RC_NV_NUMB_ERR  (MOD_NVS + 0x41)    // Parameter NumBytes wrong.
#define RC_NV_DATA_ERR  (MOD_NVS + 0x42)    // Could not Read/Write data.
#define RC_NV_ACC_ERR   (MOD_NVS + 0x43)    // NV Device not responding.
#define RC_NV_ID_ERR    (MOD_NVS + 0x44)    // Parameter Client ID wrong.
#define RC_NV_OFF_ERR   (MOD_NVS + 0x47)    // Offset is outside Client boundary
#define RC_NV_FULL_ERR  (MOD_NVS + 0x46)    // Insufficient free bytes in NV 
                                            // Device.
#define RC_NV_ROMINFO_ERR (MOD_NVS + 0x49)  // Error getting ROM info from 
                                            // Startup
// these are currently used in the EEPROM only
#define RC_NV_ESYNS_ERR (MOD_NVS + 0x45)    // E2P Sync sema error.
#define RC_NV_EADDP_ERR (MOD_NVS + 0x48)    // E2P Parameter Address not on page
                                            // boundary.

/***********************/
/* Function prototypes */
/***********************/
bool nv_init(void);
u_int32 nv_allocate(u_int8 Client_Id, u_int32 Num_Bytes);
u_int32 nv_client_size(u_int8 Client_Id, u_int32 *Client_Data_Size);
u_int32 nv_read(u_int8 Client_Id, void *Read_Buf, u_int32 Num_Bytes_to_Read, 
                        u_int32 Read_Offset, u_int32 *Num_Bytes_Actually_Read);
u_int32 nv_write(u_int8 Client_Id, void *Write_Buf, 
                        u_int32 Num_Bytes_to_Write, u_int32 Write_Offset, 
                                        u_int32 *Num_Bytes_Actually_Written);
u_int32 nv_erase(void);

#endif  // 

/****************************************************************************
 * Modifications: 
 * $Log: 
 *  33   mpeg      1.32        4/16/04 3:45:12 AM     Xiao Guang Yan  CR(s) 
 *        8822 8823 : 
 *        Added client ID for NDS core private data. 
 *  32   mpeg      1.31        9/3/02 7:39:04 PM      Matt Korte      SCR(s) 
 *        4498 :
 *        Remove warnings
 *        
 *  31   mpeg      1.30        4/25/02 1:39:34 PM     Senthil Veluswamy SCR(s) 
 *        3587 :
 *        fixed warning due to re-define of ROM_CONFIG_NUM_CLIENTS
 *        
 *  30   mpeg      1.29        4/22/02 5:14:30 PM     Senthil Veluswamy SCR(s) 
 *        3587 :
 *        Changed the number of CM Clients to 4. Removed the Client defines 
 *        here. Will be define elsewhere.
 *        
 *  29   mpeg      1.28        2/6/02 12:55:50 PM     Senthil Veluswamy SCR(s) 
 *        2982 :
 *        removed define of DRIVER_INCL_NV_CMSG - debug hack which should not 
 *        have gone into the archive
 *        
 *  28   mpeg      1.27        2/1/02 4:47:36 PM      Senthil Veluswamy SCR(s) 
 *        2982 :
 *        Fixed #ifdef (typo) for NV_CMSG changes
 *        
 *  27   mpeg      1.26        12/19/01 3:35:32 PM    Senthil Veluswamy SCR(s) 
 *        2982 :
 *        Removed Client ID for WriteOnce sector
 *        
 *  26   mpeg      1.25        12/18/01 3:30:16 PM    Senthil Veluswamy SCR(s) 
 *        2933 :
 *        Merged Wabash Changes (addition of CM Clients)
 *        
 *  25   mpeg      1.24        4/19/00 5:22:32 PM     Senthil Veluswamy no 
 *        change
 *        
 *  24   mpeg      1.23        10/19/99 4:45:30 PM    Senthil Veluswamy added 
 *        interface nv_erase to erase all config data
 *        
 *  23   mpeg      1.22        10/6/99 7:36:26 PM     Senthil Veluswamy 
 *        corrected error code : RC_NV_ROMINFO_ERR
 *        
 *  22   mpeg      1.21        10/6/99 7:25:16 PM     Senthil Veluswamy removed
 *         NV Device defines. No longer used. NV device is autodetected.
 *        Moved ROM_CONFIG_NUM_CLIENTS from cfg to here.
 *        new define for #flash banks.
 *        new error define for GetRomInfo
 *        
 *  21   mpeg      1.20        8/2/99 5:56:46 PM      Senthil Veluswamy o:
 *        changed config file macro values, to include a value from hwconfigs.h
 *        
 *  20   mpeg      1.19        8/2/99 11:29:42 AM     Senthil Veluswamy o:
 *        changed return code comments
 *        removed FLASH_ERROR: no longer used. replaced by common 
 *        - nv return codes
 *        
 *  19   mpeg      1.18        7/30/99 3:34:50 PM     Senthil Veluswamy o:
 *        
 *  18   mpeg      1.17        7/28/99 7:21:18 PM     Senthil Veluswamy o:
 *        changed read/write interfaces to work with void* buffers
 *        changed return codes to standardized nvstore.h values
 *        renamed ClientIds for clarity
 *        added defines for #if macro values
 *        
 *  17   mpeg      1.16        7/27/99 6:20:32 PM     Senthil Veluswamy o:
 *        assigned values to return codes
 *        
 *  16   mpeg      1.15        7/22/99 7:04:52 PM     Senthil Veluswamy o:
 *        new return code: ADDP - for Parameter Address not on page boundary
 *        
 *  15   mpeg      1.14        7/21/99 6:54:18 PM     Senthil Veluswamy o:
 *        changed type of return value pointer in all interfaces from int to 
 *        u_int
 *        
 *  14   mpeg      1.13        7/21/99 10:36:38 AM    Senthil Veluswamy o:
 *        changes from codereview:
 *        - moved return code values here.
 *        - renamed client_ids
 *        - moved sema block time define here.
 *        - removed use of Num CLients define. Now use the config file value
 *        - changed Hex negative values assigned to return codes to dec values
 *        
 *  13   mpeg      1.12        7/19/99 6:21:54 PM     Senthil Veluswamy o:
 *        changed return value types to u_int32: all functions will
 *        - return standard retcode values
 *        added parameter variables to receive status of API call
 *        
 *  12   mpeg      1.11        7/16/99 11:35:08 AM    Senthil Veluswamy o:
 *        codereview done on this version
 *        
 *  11   mpeg      1.10        7/9/99 7:39:16 PM      Senthil Veluswamy o:
 *        added negative error return codes
 *        
 *  10   mpeg      1.9         7/7/99 6:32:08 PM      Senthil Veluswamy o:
 *        changed/assigned return codes and error log values
 *        changed return types of interfaces to int32
 *        
 *  9    mpeg      1.8         7/6/99 6:30:16 PM      Senthil Veluswamy O:
 *        changed return types to int
 *        added return codes
 *        
 *  8    mpeg      1.7         7/2/99 5:16:54 PM      Senthil Veluswamy o:
 *        
 *  7    mpeg      1.6         6/25/99 6:42:22 PM     Senthil Veluswamy o:
 *        
 *  6    mpeg      1.5         6/23/99 8:47:42 PM     Senthil Veluswamy o:
 *        
 *  5    mpeg      1.4         6/18/99 6:26:36 PM     Senthil Veluswamy 
 *        checkpoint 6/18/99:o
 *        
 *  4    mpeg      1.3         6/17/99 6:49:46 PM     Senthil Veluswamy 
 *        checkpoint 6/17/99:o
 *        new nv_client_size()
 *        - returns size of client data
 *        removed nv_reallocate:
 *        - size() will be called before an allocation.
 *        
 *  3    mpeg      1.2         6/6/99 10:48:40 PM     Senthil Veluswamy 
 *        checkpoint 6/6/99:h
 *        
 *  2    mpeg      1.1         6/3/99 6:06:44 PM      Senthil Veluswamy 
 *        checkpoint 6/3/99:o
 *        
 *  1    mpeg      1.0         6/3/99 11:17:50 AM     Senthil Veluswamy 
 * $
 * 
 *    Rev 1.31   03 Sep 2002 18:39:04   kortemw
 * SCR(s) 4498 :
 * Remove warnings
 * 
 *    Rev 1.30   25 Apr 2002 12:39:34   velusws
 * SCR(s) 3587 :
 * fixed warning due to re-define of ROM_CONFIG_NUM_CLIENTS
 * 
 *    Rev 1.29   22 Apr 2002 16:14:30   velusws
 * SCR(s) 3587 :
 * Changed the number of CM Clients to 4. Removed the Client defines here. Will be define elsewhere.
 * 
 *    Rev 1.28   06 Feb 2002 12:55:50   velusws
 * SCR(s) 2982 :
 * removed define of DRIVER_INCL_NV_CMSG - debug hack which should not have gone into the archive
 * 
 *    Rev 1.27   01 Feb 2002 16:47:36   velusws
 * SCR(s) 2982 :
 * Fixed #ifdef (typo) for NV_CMSG changes
 ****************************************************************************/

