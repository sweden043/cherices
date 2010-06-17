/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*        Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002, 2003      */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       iic.h
 *
 *
 * Description:    I2C Driver public header file
 *
 *
 * Author:         Rob Tilton
 *
 ****************************************************************************/
/* $Header: iic.h, 12, 4/2/03 10:52:02 AM, Billy Jackman$
 ****************************************************************************/

#ifndef _IIC_H_
#define _IIC_H_

#include "basetype.h"

#ifndef BYTE
#define BYTE   u_int8
#endif /* BYTE */
#ifndef DWORD
#define DWORD  u_int32
#endif /* DWORD */

/* Indicates the bus to which a device is attached. The I2C_ labels       */
/* are defined in the hardware configuration file. I2C_BUS_NONE indicates */
/* that a device is not populated on the target board and will result in  */
/* a warning from the IIC driver that some caller needs to compile out    */
/* a piece of code.                                                       */

typedef u_int8 IICBUS;
/* Valid values are I2C_BUS_NONE, I2C_BUS_0, I2C_BUS_1. I can't use an enum */
/* here due to the way the config file works - sorry.                       */

/* Previous IICBUS members defined for backwards compatibility */
#define DDC I2C_BUS_NONE
#define IIC I2C_BUS_0

typedef struct _tagIICTRANS{
   DWORD    dwCount;                    /* Number of items in Cmd buffer */
   BYTE*    pData;                      /* Pointer to data buffer */
   BYTE*    pCmd;                       /* Pointer to command buffer */
   DWORD    dwError;                    /* Error code if iicTransaction fails */
} IICTRANS, * PIICTRANS;

#define IIC_ERROR_NOERR              0  /* No Error */
#define IIC_ERROR_NOACK              1  /* Ack not received after byte write */
#define IIC_ERROR_NOADDRESSACK       2  /* Device not responding to address */
#define IIC_ERROR_INVALIDDATA        3  /* Invalid data pointer */
#define IIC_ERROR_INVALIDCMD         4  /* Invalid cmd pointer */
#define IIC_ERROR_INVALIDADDR        5  /* Invalid address */

#define IIC_DATA                     0x01  
#define IIC_START                    0x02
#define IIC_STOP                     0x04
#define IIC_ACK                      0x08
#define IIC_NO_DEATH                 0x10 /* Used to test for addresses. */
#define IIC_PARTIAL                  0x20

/* Function Prototypes */
/*-------------------- */
void IICInit(void);
bool iicTransaction(PIICTRANS pTrans, IICBUS bus);
bool iicAddressTest(BYTE byAddr, IICBUS bus, bool bDeath);
bool iicWriteReg(BYTE byAddr, BYTE byReg, IICBUS bus);
bool iicWriteIndexedReg(BYTE byAddr, BYTE byIndex, BYTE byReg, IICBUS bus);
bool iicReadReg(BYTE byAddr, BYTE *pReg, IICBUS bus);
bool iicReadIndexedReg(BYTE byAddr, BYTE byIndex, BYTE *pReg, IICBUS bus);

/**********************************************************/
/* GPIO Extender Functions (semaphore protected wrappers) */
/**********************************************************/
u_int8 read_gpio_extender(void);
u_int8 read_second_gpio_extender(void);
u_int8 write_gpio_extender(u_int8 bit_mask, u_int8 value);
u_int8 write_second_gpio_extender(u_int8 bit_mask, u_int8 value);

#endif /* _IIC_H_ */

/****************************************************************************
 * Modifications:
 * $Log: 
 *  12   mpeg      1.11        4/2/03 10:52:02 AM     Billy Jackman   SCR(s) 
 *        5939 5940 :
 *        Added flag IIC_PARTIAL to allow operations to be broken up into 
 *        several calls
 *        to iicTransaction().
 *        Eradicated double slash comments.
 *        
 *  11   mpeg      1.10        12/4/02 1:40:58 PM     Dave Wilson     SCR(s) 
 *        5059 :
 *        Renamed I2C_BUS_x labels to use indices 0 and 1 rather than 1 and 2 
 *        so that
 *        the software definition matches the spec and app note.
 *        
 *  10   mpeg      1.9         11/27/02 4:14:30 PM    Lucy C Allevato SCR(s) 
 *        5035 :
 *        Changed definitions of IIC and DDC from IIC_blah to I2C_blah.
 *        
 *  9    mpeg      1.8         11/27/02 9:53:58 AM    Dave Wilson     SCR(s) 
 *        4902 :
 *        Replaced IICBUS type definition. The old enum was generating warnings
 *         when
 *        using the new config file definitions with SDT builds (but not ADS 
 *        oddly
 *        enough).
 *        
 *  8    mpeg      1.7         11/26/02 3:59:02 PM    Dave Wilson     SCR(s) 
 *        4902 :
 *        Changed IICBUS enum to use values from HWCONFIG.CFG (now that the 
 *        config file
 *        includes a label for each device's bus as well as its address).
 *        
 *  7    mpeg      1.6         7/29/02 5:00:00 PM     Lucy C Allevato SCR(s) 
 *        4298 :
 *        Added iic error code: IIC_ERROR_INVALIDADDR to take care of cases 
 *        such as whenthe iic gets a request to write to address 0.
 *        
 *  6    mpeg      1.5         4/13/00 10:06:22 AM    Dave Wilson     Moved 
 *        GPIO extender functions here from KAL
 *        
 *  5    mpeg      1.4         3/28/00 5:58:54 PM     Dave Wilson     Added 
 *        functions to access second GPIO extender on Klondike
 *        
 *  4    mpeg      1.3         10/29/99 10:59:06 AM   Dave Wilson     Moved 
 *        GPIO extender APIs from HWLIB to IIC.
 *        
 *  3    mpeg      1.2         8/10/98 6:16:24 PM     Rob Tilton      Added 
 *        death to iicAddressTest().
 *        
 *  2    mpeg      1.1         8/7/98 3:23:24 PM      Rob Tilton      Added 
 *        API's.
 *        
 *  1    mpeg      1.0         7/30/98 4:13:38 PM     Rob Tilton      
 * $
 * 
 *    Rev 1.11   02 Apr 2003 10:52:02   jackmaw
 * SCR(s) 5939 5940 :
 * Added flag IIC_PARTIAL to allow operations to be broken up into several calls
 * to iicTransaction().
 * Eradicated double slash comments.
 * 
 *    Rev 1.10   04 Dec 2002 13:40:58   dawilson
 * SCR(s) 5059 :
 * Renamed I2C_BUS_x labels to use indices 0 and 1 rather than 1 and 2 so that
 * the software definition matches the spec and app note.
 * 
 *    Rev 1.9   27 Nov 2002 16:14:30   allevalc
 * SCR(s) 5035 :
 * Changed definitions of IIC and DDC from IIC_blah to I2C_blah.
 * 
 *    Rev 1.8   27 Nov 2002 09:53:58   dawilson
 * SCR(s) 4902 :
 * Replaced IICBUS type definition. The old enum was generating warnings when
 * using the new config file definitions with SDT builds (but not ADS oddly
 * enough).
 * 
 *    Rev 1.7   26 Nov 2002 15:59:02   dawilson
 * SCR(s) 4902 :
 * Changed IICBUS enum to use values from HWCONFIG.CFG (now that the config file
 * includes a label for each device's bus as well as its address).
 * 
 *    Rev 1.6   Jul 29 2002 17:00:00   allevalc
 * SCR(s) 4298 :
 * Added iic error code: IIC_ERROR_INVALIDADDR to take care of cases such as whenthe iic gets a request to write to address 0.
 * 
 *    Rev 1.5   13 Apr 2000 09:06:22   dawilson
 * Moved GPIO extender functions here from KAL
 * 
 *    Rev 1.4   28 Mar 2000 17:58:54   dawilson
 * Added functions to access second GPIO extender on Klondike
 * 
 *    Rev 1.3   29 Oct 1999 09:59:06   dawilson
 * Moved GPIO extender APIs from HWLIB to IIC.
 * 
 *    Rev 1.2   10 Aug 1998 17:16:24   rtilton
 * Added death to iicAddressTest().
 * 
 *    Rev 1.1   07 Aug 1998 14:23:24   rtilton
 * Added API's.
 * 
 *    Rev 1.0   30 Jul 1998 15:13:38   rtilton
 * Initial revision.
 * 
 *****************************************************************************/

