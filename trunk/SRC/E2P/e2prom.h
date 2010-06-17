/****************************************************************************/
/*                            Conexant Systems                              */
/****************************************************************************/
/*                                                                          */
/* Filename:       e2prom.h                                                 */
/*                                                                          */
/* Description:    E2PROM private header file.                              */
/*                                                                          */
/* Author:         Senthil Veluswamy                                        */
/*                                                                          */
/* Date:           05/28/99                                                 */
/*                                                                          */
/* Copyright Conexant Systems, 1999                                         */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/

#ifndef _E2PROM_H_
#define _E2PROM_H_

/******************/
/* Include Files  */
/******************/
#include <string.h>
#include "stbcfg.h"
#include "kal.h"
#include "retcodes.h"
#include "e2p.h"
#include "iic.h"
#ifdef DRIVER_INCL_EEPROM
#include "..\eeprom\eepriv.h"
#endif

/***********/
/* aliases */
/***********/

/**** I2C address definitions can now be found in the revelant ****/
/**** vendor header file.                                      ****/

#define HEADER_BASE_ADD     0x0     // Header starts at the bottom.
#define E2P_MAGIC_NUM       0xFAC1E9B0
                                    // Sequence to check presence of valid
                                    // data in EEPROM.
#define E2P_ERASE_MAGIC_NUM 0xABCDACBD
                                    // Sequence which will be used to invalidate
                                    // all config data in e2p.
#define MAGIC_NUM_SIZE      0x4     // Uses 4 bytes
/*
#define CLIENT_NUMB_SIZE    0x4     // Uses 4 bytes
#define CLIENT_CKS_SIZE     0x4     // Uses 4 byte
*/
#define WRITE_FLAG          0x0     // IIC Write Transaction.
#define READ_FLAG           0x1     // IIC Read Transaction.
#define READ_TRANSFER_SIZE  32      // #Bytes that will be read at a time.
#define WRITE_PAGE_SIZE     16      // max #Bytes that can be written to E2P at
                                    // one go
#define E2P_WRITE_STIME     10      // E2P has a Write Setup time of 10 mS

/******************/
/* New Data Types */
/******************/
// E2P Header data
// The sizes of the data members used to access Header Info has been increased
// to 32bits to workaround VxWork's predilection for 16 bit aligned addresses
// and to prevent any further trouble due to different compiler's interpretation
// of "packing", etc

typedef struct _E2P_Header_Client_Info{ // data for each client
    u_int32 Num_Bytes;              // Number of bytes used by client.
    u_int32 CheckSum;               // CheckSum to verify client data.
} E2P_Header_Client_Info;

typedef E2P_Header_Client_Info *p_E2P_Header_Client_Info;

typedef struct _E2P_Header_Info{    // Complete header info.
    u_int32 Magic_Number;           // To Check validity of E2P data. This is
                                    // the first byte in E2P header.
    E2P_Header_Client_Info Client_Info[ROM_CONFIG_NUM_CLIENTS];
} E2P_Header_Info, *p_E2P_Header_Info;

/***********************/
/* Function prototypes */
/***********************/
void e2p_task(void *Params);
int32 read_data(u_int16 Address, u_int16 Num_Bytes, u_int8 *Read_Buffer);
int32 write_data(u_int16 Address, u_int16 Num_Bytes, u_int8 *Write_Buffer);
u_int8 get_checksum(u_int16 Address, u_int16 Num_Bytes, bool COMPLEMENT);
void identify_board_vendor(void);

#endif // _E2PROM_H_

