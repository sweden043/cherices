/****************************************************************************/
/*                            Conexant Systems                              */
/****************************************************************************/
/*                                                                          */
/* Filename:       eepriv.h                                                 */
/*                                                                          */
/* Description:    EEPROM Driver Private Header file                        */
/*                                                                          */
/* Author:         Senthil Veluswamy                                        */
/*                                                                          */
/* Date:           02/08/00                                                 */
/*                                                                          */
/* Copyright Conexant Systems, 1999                                         */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/

/****************************************************************************
 * $Header: eepriv.h, 16, 2/13/03 11:46:22 AM, Matt Korte$
 ****************************************************************************/

#ifndef _EEPRIV_H_
#define _EEPRIV_H_

/*****************/
/* Include Files */
/*****************/
#include <string.h>
#include "stbcfg.h"
#include "kal.h"
#include "retcodes.h"
#include "iic.h"
#include "nvramp.h"
#ifdef DRIVER_INCL_NVSTORE
#include "e2p.h"
#endif

/***********/
/* Aliases */
/***********/
// For TRACE statements
#define TRACE_EE            TRACE_NV

// Return Codes
#define RC_EE_ERR           (MOD_NVS + 0x10)

#if EEPROM_TYPE == EEPROM_32KB
#define EE_DEVICE_SIZE      32768
#else /* EEPROM_TYPE == EEPROM_32KB */
#define EE_DEVICE_SIZE      16384
#endif /* EEPROM_TYPE == EEPROM_32KB */

#ifdef DRIVER_INCL_NVSTORE
 #ifdef DRIVER_INCL_NV_CMSG
#define EE_FS_START_ADDR    MAX_EEPROM_SIZE
#define EE_FS_SIZE          0
 #else /* DRIVER_INCL_NV_CMSG */
#define EE_FS_START_ADDR    MAX_EEPROM_SIZE
#define EE_FS_SIZE          (EE_DEVICE_SIZE - NV_PRIV_SIZE - MAX_EEPROM_SIZE)
                                    // Space allocated for the File System.
 #endif /* DRIVER_INCL_NV_CMSG */
#else
#define EE_FS_START_ADDR    0
#define EE_FS_SIZE          (EE_DEVICE_SIZE - NV_PRIV_SIZE)
                                    // Space allocated for the File System.
#endif

#define EE_FS_SECTOR_SIZE   64      // Size of each sector in the File System.
                                    // EE Page Size.

/* NB: Device I2C address(es) are defined in the relevant vendor header file */

#define WRITE_FLAG          0x0     // IIC Write Transaction.
#define READ_FLAG           0x1     // IIC Read Transaction.

#if CUSTOMER == VENDOR_C
#define TRANSFER_SIZE       32      // max #Bytes that can be written to EE at
                                    // one go
#else // CUSTOMER
//319 _32k eeprom
#define TRANSFER_SIZE       32      // max #Bytes that can be written to EE at
                                    // one go
#endif // CUSTOMER

#define EE_WRITE_STIME      10      // EE has a Write Setup time of 10 mS

/***********************/
/* Function prototypes */
/***********************/
// EE File system
u_int16 ee_fs_read(u_int16 address, voidF buffer, u_int16 count, 
                                                            void* private);
u_int16 ee_fs_write(u_int16 address, voidF buffer, u_int16 count, 
                                                            void* private);

// EE lower level Driver
bool ee_present(void);
bool ee_initialized(void);
bool ee_init(void);
u_int16 ee_read(u_int16 address, voidF buffer, u_int16 count, void* private);
u_int16 ee_write(u_int16 address, voidF buffer, u_int16 count, void* private);

#endif // _EEPRIV_H_

/****************************************************************************
 * Modifications: 
 * $Log: 
 *  16   mpeg      1.15        2/13/03 11:46:22 AM    Matt Korte      SCR(s) 
 *        5479 :
 *        Removed old header reference
 *        
 *  15   mpeg      1.14        1/30/02 5:18:14 PM     Senthil Veluswamy SCR(s) 
 *        2982 :
 *        Fixed EE_FS Start Address Error when EEP is used for Wabash CM NV
 *        
 *  14   mpeg      1.13        1/30/02 4:49:36 PM     Senthil Veluswamy SCR(s) 
 *        2982 :
 *        Updated to work with 32K EEProms, to use the whole device for Wabash 
 *        CM NV Store
 *        
 *  13   mpeg      1.12        8/1/00 1:04:42 PM      Dave Wilson     Updated 
 *        to use I2C addresses from relevant vendor header.
 *        
 *  12   mpeg      1.11        7/13/00 1:18:36 PM     Senthil Veluswamy defined
 *         new EE address for Vendor_A
 *        
 *  11   mpeg      1.10        6/14/00 6:51:34 PM     Tim White       
 *        VENDOR_C's Atmel EEPROM device uses 32byte sectors instead of 64byte
 *        sectors like everyone else so far.
 *        
 *  10   mpeg      1.9         4/20/00 1:22:02 PM     Senthil Veluswamy Changed
 *         Read/Write_TRANSFER_SIZE to TRANSFER_SIZE
 *        
 *  9    mpeg      1.8         4/18/00 2:56:36 PM     Senthil Veluswamy changed
 *         to work only with 16k EE
 *        
 *  8    mpeg      1.7         4/13/00 9:08:00 PM     Senthil Veluswamy changed
 *         R/W page sizes for 16K EE
 *        
 *  7    mpeg      1.6         4/11/00 6:36:08 PM     Senthil Veluswamy 
 *        corrected DRV_INCL_NVSTORE  to DRIVER_INCL_NVSTORE
 *        
 *  6    mpeg      1.5         4/11/00 6:13:46 PM     Senthil Veluswamy added 
 *        EE_FS_START_ADDR to enable NVSTORE to work with 
 *        - EE /FS drivers
 *        changed to work with 16K EE.
 *        
 *  5    mpeg      1.4         2/23/00 6:46:06 PM     Senthil Veluswamy moved 
 *        "eeprom.h" and "eefspriv.h" defines here.
 *        Removed extra defines.
 *        
 *  4    mpeg      1.3         2/18/00 3:03:50 PM     Dave Wilson     Added 
 *        fake RAM support to aid integration.
 *        Added wrapper functions to prevent o-code access to private data area
 *        
 *  3    mpeg      1.2         2/10/00 3:04:52 PM     Senthil Veluswamy changed
 *         defines to work with 2404
 *        
 *  2    mpeg      1.1         2/9/00 7:27:14 PM      Senthil Veluswamy Added 
 *        I2C defines for EE.
 *        Think about EE_FS defines - are they needed
 *        
 *  1    mpeg      1.0         2/8/00 6:46:18 PM      Senthil Veluswamy 
 * $
 * 
 *    Rev 1.15   13 Feb 2003 11:46:22   kortemw
 * SCR(s) 5479 :
 * Removed old header reference
 * 
 *    Rev 1.14   30 Jan 2002 17:18:14   velusws
 * SCR(s) 2982 :
 * Fixed EE_FS Start Address Error when EEP is used for Wabash CM NV
 ****************************************************************************/

