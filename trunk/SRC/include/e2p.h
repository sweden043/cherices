/****************************************************************************/
/*                            Conexant Systems                              */
/****************************************************************************/
/*                                                                          */
/* Filename:       e2p.h                                                    */
/*                                                                          */
/* Description:    E2PROM Header file.                                      */
/*                                                                          */
/* Author:         Senthil Veluswamy                                        */
/*                                                                          */
/* Date:           5/28/99                                                  */
/*                                                                          */
/* Copyright Conexant Systems, 1999                                         */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/

/****************************************************************************
 * $Header: e2p.h, 27, 5/6/04 5:23:20 PM, Miles Bintz$
 ****************************************************************************/

#ifndef _E2P_H_
#define _E2P_H_

/*****************/
/* Include Files */
/*****************/
#include "nvstore.h"

/***********/
/* Aliases */
/***********/

#ifdef OPENTV
  #define MAX_EEPROM_SIZE    512
#else

 #if EEPROM_TYPE == EEPROM_32KB
#define MAX_EEPROM_SIZE     32768     // Max E2P size.
 #elif EEPROM_TYPE == EEPROM_16KB
#define MAX_EEPROM_SIZE     16384     // Max E2P size.
 #else /* EEPROM_TYPE == EEPROM_32KB */
#define MAX_EEPROM_SIZE     512     // Max E2P size.
 #endif

#endif

#define TRACE_E2P TRACE_NV

// Return Codes
#define RC_E2P_ERR          (MOD_NVS + 0x10)

/***********************/
/* Function prototypes */
/***********************/
bool e2p_present(void);
bool e2p_init(void);
u_int32 e2p_allocate(u_int8 Client_Id, u_int32 Num_Bytes);
u_int32 e2p_client_size(u_int8 Client_Id, u_int32 *Client_Data_Size);
u_int32 e2p_read(u_int8 Client_Id, void *Read_Buf, u_int32 Num_Bytes_to_Read, 
                        u_int32 Read_Offset, u_int32 *Num_Bytes_Actually_Read);
u_int32 e2p_write(u_int8 Client_Id, void *Write_Buf, 
                        u_int32 Num_Bytes_to_Write, u_int32 Write_Offset, 
                                        u_int32 *Num_Bytes_Actually_Written);
u_int32 e2p_erase(void);

#endif // _E2P_H_

/****************************************************************************
 * Modifications : 
 * $Log: 
 *  27   mpeg      1.26        5/6/04 5:23:20 PM      Miles Bintz     CR(s) 
 *        8897 8898 : for opentv, max_eeprom_size must be 512 for legacy hacks
 *  26   mpeg      1.25        4/16/04 3:55:40 AM     Xiao Guang Yan  CR(s) 
 *        8822 8823 : Changed definitions of MAX_EEPROM_SIZE to reflex real 
 *        EEPROM size. 
 *  25   mpeg      1.24        2/4/02 10:19:00 AM     Senthil Veluswamy SCR(s) 
 *        2982 :
 *        Fixed typo - in check for inclusion of NV_CMSG
 *        
 *  24   mpeg      1.23        1/30/02 5:00:38 PM     Senthil Veluswamy SCR(s) 
 *        2982 :
 *        corrected EEPROM_SIZE error for 16K EEProm
 *        
 *  23   mpeg      1.22        1/30/02 4:57:42 PM     Senthil Veluswamy SCR(s) 
 *        2982 :
 *        Modified to work with 32KB EEProm
 *        
 *  22   mpeg      1.21        1/30/02 4:25:40 PM     Senthil Veluswamy SCR(s) 
 *        2982 :
 *        Increased MAX_EEPROM_SIZE for Wabash CM NV.
 *        
 *  21   mpeg      1.20        10/19/99 4:45:52 PM    Senthil Veluswamy added 
 *        interface e2p_erase to erase all config data
 *        
 *  20   mpeg      1.19        10/6/99 7:26:16 PM     Senthil Veluswamy moved 
 *        MAX_EEPROM_SIZE from cfg file to here. can potentially change for 
 *        - each Customer build.
 *        
 *  19   mpeg      1.18        8/2/99 5:54:56 PM      Senthil Veluswamy o:
 *        Added public interface e2p_present. used to check for e2p before
 *        - KAL is up
 *        
 *  18   mpeg      1.17        7/28/99 7:23:16 PM     Senthil Veluswamy o:
 *        modified read/write interfaces to work with a void* buffer
 *        
 *  17   mpeg      1.16        7/21/99 6:58:56 PM     Senthil Veluswamy o:
 *        removed public interface: e2p_present(). Functionality has been 
 *        - made inherent in the e2p driver
 *        changed return value pointer types to match those in nvstore.h
 *        
 *  16   mpeg      1.15        7/21/99 10:39:16 AM    Senthil Veluswamy o:
 *        code review changes:
 *        - moved return code values to nv header
 *        - changed function prototypes to match nv functions
 *        
 *  15   mpeg      1.14        7/16/99 11:35:18 AM    Senthil Veluswamy o:
 *        codereview done on this version
 *        
 *  14   mpeg      1.13        7/9/99 7:40:58 PM      Senthil Veluswamy o:
 *        removed public interface: e2p_raw_read()
 *        
 *  13   mpeg      1.12        7/7/99 6:32:42 PM      Senthil Veluswamy o:
 *        changed/assigned return codes and error log values
 *        changed return types of interfaces to int32
 *        
 *  12   mpeg      1.11        7/6/99 6:30:42 PM      Senthil Veluswamy O:
 *        changed return types to int
 *        defined return codes in terms of Module return code startup value
 *        
 *  11   mpeg      1.10        7/2/99 5:17:06 PM      Senthil Veluswamy o:
 *        
 *  10   mpeg      1.9         6/30/99 7:01:10 PM     Senthil Veluswamy o:
 *        
 *  9    mpeg      1.8         6/21/99 12:12:54 AM    Senthil Veluswamy 
 *        checkpoint 6/20/99:h
 *        new return code: RC_E2P_PARAM_ERR 
 *        changed return type of e2p_allocate from bool -> int16
 *        
 *  8    mpeg      1.7         6/18/99 6:27:46 PM     Senthil Veluswamy 
 *        checkpoint 6/18/99:o
 *        changed header file names
 *        moved return codes here
 *        
 *  7    mpeg      1.6         6/17/99 6:41:18 PM     Senthil Veluswamy 
 *        checkpoint 6/17/99:o
 *        bug fix:
 *        e2p_client_size(*,*,Client Id) -> Client_Id
 *        
 *  6    mpeg      1.5         6/16/99 6:44:30 PM     Senthil Veluswamy 
 *        checkpoint 6/16/99:o
 *        
 *  5    mpeg      1.4         6/10/99 9:19:40 PM     Senthil Veluswamy 
 *        checkpoint 6/10/99:h
 *        
 *  4    mpeg      1.3         6/7/99 5:45:34 PM      Senthil Veluswamy 
 *        checkpoint 6/7/99:o
 *        
 *  3    mpeg      1.2         6/3/99 6:06:52 PM      Senthil Veluswamy 
 *        checkpoint 6/3/99:o
 *        
 *  2    mpeg      1.1         5/28/99 6:19:20 PM     Senthil Veluswamy 
 *        checkpoint 5/28/99:o
 *        
 *  1    mpeg      1.0         5/28/99 4:32:40 PM     Senthil Veluswamy 
 * $
 * 
 *    Rev 1.24   04 Feb 2002 10:19:00   velusws
 * SCR(s) 2982 :
 * Fixed typo - in check for inclusion of NV_CMSG
 * 
 *    Rev 1.23   30 Jan 2002 17:00:38   velusws
 * SCR(s) 2982 :
 * corrected EEPROM_SIZE error for 16K EEProm
 * 
 *    Rev 1.22   30 Jan 2002 16:57:42   velusws
 * SCR(s) 2982 :
 * Modified to work with 32KB EEProm
 *
 ****************************************************************************/

