/****************************************************************************/
/*                            Conexant Systems                              */
/****************************************************************************/
/*                                                                          */
/* Filename:       board.h                                                  */
/*                                                                          */
/* Description:    Board hardware definition.                               */
/*                                                                          */
/* Author:         Tim Ross                                                 */
/*                                                                          */
/* Copyright Conexant Systems, 1999                                         */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/
/***************************************************************************
$Header: board.h, 22, 2/13/03 11:21:04 AM, Matt Korte$
****************************************************************************/

#ifndef _BOARD_H
#define _BOARD_H

/*****************/
/* Include Files */
/*****************/

/***********************/
/* Function Prototypes */
/***********************/

/**********************/
/* Local Definitions  */
/**********************/
typedef volatile u_int8 *LPREGB;
typedef volatile u_int16 *LPREGH;

/******************/
/* Register Bases */
/******************/
#define ROM_BASE     0x20000000
#define PCI_IO_BASE  0x31000000
#define PCI_MEM_BASE 0x40000000

/***********************************/
/* Board Type Specific Definitions */
/***********************************/

typedef struct _BOARD_TYPES
{
  u_int8        BoardID;            /* Board Identifier                      */
  u_int8        DeviceConfigIndex;  /* Index into ISA_DEVICE internal arrays */
} BOARD_TYPES, *LPBOARD_TYPES;

#define MAX_DEVICE_CONFIGS  4

/* Definitions of actual board and vendor IDs are to be */
/* found in HWCONFIG.H                                  */

/***************/
/* ISA Devices */
/***************/
typedef enum _IO_REG_MODE {OFF, PCCARD, PROGRAMMABLE} IO_REG_MODE;
typedef enum _IO_MEMORY {MEMORY, IO} IO_MEMORY;
typedef struct _ISA_DEVICE  {
    bool        bClocked;
    bool        XOEMask;
    u_int8      ChipSelect;
    u_int8      DescNum;
    u_int8      ExtWait;
    IO_MEMORY   IOMemory;
    IO_REG_MODE IORegMode;
    u_int16     WriteAccessTime;
    u_int16     ReadAccessTime;
    u_int16     IORegSetupTime;
    u_int16     IORegAccessTime;
    u_int16     CSSetupTime;
    u_int16     CSHoldTime;
    u_int16     CtrlSetupTime;
    u_int16     CtrlHoldTime;
} ISA_DEVICE, *LPISA_DEVICE;
extern ISA_DEVICE ISADevices[];
extern int iNumISADevices;

/* The ISA_XXX_CARD_INDEX definitions refer to the indicies of the  */
/* elements in the ISADevices[] array in boarddata.c and must be    */
/* maintained together.                                             */
#define ISA_SERIAL_CARD_INDEX       0
#define ISA_MODEM_CARD_INDEX        1
#define ISA_CI_CARD_INDEX           2
#define ISA_8900_ETHERNET_CARD_INDEX 3

#if CUSTOMER != VENDOR_A && CUSTOMER != VENDOR_D

/***************/
/* PCI Devices */
/***************/
#define PCCARD_PCI_DEVICE_NUM   1
#define PCI_ID_TI_1210          0xac1a104c
#define PCI_ID_TI_1211          0xac1e104c
#define PCI_ID_ACCES_COM2S      0x10d0494f

/* PCI - Rockwell RS56-PCI BASIC2/20410 Modem configuration. */
#define PCI_ID_RS56_PCI  0x1002127A
#define RS56_PCI_BASE    0x4fff0000  // 64K at top of PCI Memory

/* PCI - PCCard Bridge configuration.  */
#define PCCARD_BRIDGE_REG_BASE  0x40000000
#define PCCARD_BRIDGE_IO_BASE   0xfeec
#define PCCARD_LATENCY          0x20
#define CARDBUS_BUS_NUM         1
#define CARDBUS_SUB_BUS_NUM     0
#define CARDBUS_LATENCY         0x40

/*****************/
/* PCCard Bridge */
/*****************/
#define PCCARD_EXCA_BASE        (PCCARD_BRIDGE_REG_BASE + 0x800)
#define PCCARD_EXCA_IO_BASE     (PIO_BASE + PCCARD_BRIDGE_IO_BASE)

#endif

#endif
/***************************************************************************
                     PVCS Version Control Information
$Log: 
 22   mpeg      1.21        2/13/03 11:21:04 AM    Matt Korte      SCR(s) 5479 
       :
       Removed old header reference
       
 21   mpeg      1.20        9/23/02 3:19:00 PM     Larry Wang      SCR(s) 4638 
       :
       Define PCI_MEM_BASE as 0x40000000.
       
 20   mpeg      1.19        11/29/01 1:00:20 PM    Miles Bintz     SCR(s) 2936 
       :
       removed dependencies on cnxtpci.h
       
       
 19   mpeg      1.18        11/20/01 3:16:02 PM    Quillian Rutherford SCR(s) 
       2754 :
       Minor changes to the ISA devices definitions (changed an array to an 
       int)
       Affect startupc.c and bd_cnxt.c in the startup directory.
       
       
 18   mpeg      1.17        7/3/01 1:32:46 PM      Tim White       SCR(s) 2178 
       2179 2180 :
       > Fixed board ID issue.  Now have separate MAX_NUM_BOARDS with
       separate vendor lists.
       
       
 17   mpeg      1.16        7/3/01 10:43:12 AM     Tim White       SCR(s) 2178 
       2179 2180 :
       Merge branched Hondo specific code back into the mainstream source 
       database.
       
       
 16   mpeg      1.15        1/31/01 2:27:42 PM     Tim White       DCS#0988: 
       Reclaim footprint space.
       
 15   mpeg      1.14        1/29/01 2:44:20 PM     Tim White       Changed 
       pci.h to cnxtpci.h.
       
 14   mpeg      1.13        7/18/00 5:52:18 PM     Tim White       Merge in 
       PACE changes to ISA tables.
       
 13   mpeg      1.12        5/17/00 5:44:22 PM     Ray Mack        added 
       Ethernet card for ISA Bus
       
 12   mpeg      1.11        4/7/00 2:46:34 PM      Dave Wilson     Changes to 
       allow CODELDR and STARTUP to work on both Thor and Klondike
       
 11   mpeg      1.10        3/22/00 4:27:12 PM     Tim White       Removed old 
       IOCHRDY bits and replaced with ExtWait:2 which is valid for both
       Colorado and all Neches (A&B) chips according to the specs.
       
 10   mpeg      1.9         3/8/00 5:18:52 PM      Tim White       Restructured
        the BANK_INFO array and added support for new Intel Flash ROM.
       
 9    mpeg      1.8         12/13/99 2:17:34 PM    Tim Ross        Corrected 
       definition of enum IO_MEMORY for ISA descriptors.
       
 8    mpeg      1.7         12/10/99 5:37:42 PM    Tim Ross        Added some 
       index definitions fro the ISA cards.
       
 7    mpeg      1.6         12/8/99 5:12:42 PM     Tim Ross        Added 
       changes for Colorado.
       
 6    mpeg      1.5         10/13/99 4:50:12 PM    Tim White       Added 
       comments regarding the dependency between the FLASH_DESC structure
       and the flash/flashrw.c file.
       
 5    mpeg      1.4         9/12/99 9:29:40 PM     Tim Ross        Added TI 
       PCI1211 PCI-PCCard bridge ID.
       
 4    mpeg      1.3         9/9/99 11:13:26 AM     Tim Ross        Moved 
       FLASH_DESC from startup.h to here.
       
 3    mpeg      1.2         8/11/99 5:24:24 PM     Dave Wilson     Replaced 
       kal.h with basetype.h to allow non-KAL apps to use the header.
       
 2    mpeg      1.1         8/6/99 11:42:32 AM     Tim Ross        Added THOR2 
       rev X1 US and EURO boards to list of supported 
       boards.
       
 1    mpeg      1.0         6/8/99 12:57:50 PM     Tim Ross        
$
 * 
 *    Rev 1.21   13 Feb 2003 11:21:04   kortemw
 * SCR(s) 5479 :
 * Removed old header reference
****************************************************************************/

