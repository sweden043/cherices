/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*           Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002         */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       BD_CNXT.C
 *
 *
 * Description:    ISA Descriptor tables used with Conexant reference IRDs
 *
 * Author:         Ray Mack           
 *
 * NOTE: Information in this file will likely transition to a hardware 
 *       configuration file at a later date.
 *
 ****************************************************************************/
/* $Header: bd_cnxt.c, 20, 4/22/04 4:20:33 PM, Sunil Cheruvu$
 ****************************************************************************/

/******************/
/* Include Files  */
/******************/
#include "stbcfg.h"
#include "basetype.h"
#include "board.h"
#include "hwconfig.h"

/********************************************/
/*  Local defines                           */
/********************************************/
#define ISA_NO_WAIT_STATES              0
#define ISA_WAIT_UNTIL_READY_0          2
#define ISA_WAIT_UNTIL_READY_1          3

/*************************/
/* ISA Descriptor Values */
/*************************/
/* The ISA_XXX_CARD_INDEX definitions in board.h refer to the   */
/* indicies of the elements in the ISADevices[] array and must  */
/* be maintained together.                                      */

#if EMULATION_LEVEL == PHASE3
/* No devices configured under emulation */
ISA_DEVICE ISADevices[1];
int iNumISADevices = 0;

#else 
ISA_DEVICE ISADevices[] =
{   
#if (PCI_ISA_DESCRIPTOR_SETUP == DESCRIPTOR_SETUP_KLONDIKE)
    /****************************************************************************/
    /* Telegraph Memory Mapped Devices on Klondike, GPIO extender on later IRDs */   
    /****************************************************************************/  
    {
        0,                   /* Is it clocked.               */
        0,                   /* XOEMask                      */
        ISA_CHIP_SELECT_1,   /* ChipSelect                   */
        3,                   /* Descriptor number - determines base address. */
        ISA_NO_WAIT_STATES,  /* Ext wait                     */
        MEMORY,              /* Memory mapped.               */
        OFF,                 /* IOReg signal is not used.    */
        500,                 /* Write access time (ns).      */
        500,                 /* Read access time (ns).       */
        0,                   /* IOReg setup time (ns).       */
        0,                   /* IOReg access time (ns).      */
        100,                 /* Chip select setup time (ns). */
        100,                 /* Chip select hold time (ns).  */
        200,                 /* Control setup time (ns).     */
        100,                 /* Control hold time (ns).      */
    },

#if (GPIO_CONFIG == GPIOM_BRAZOS)
    /****************************************************************************/
    /* Telegraph Memory Mapped Devices on Klondike,                             */   
    /****************************************************************************/  
    {
        0,                   /* Is it clocked.               */
        0,                   /* XOEMask                      */
        ISA_CHIP_SELECT_2,   /* ChipSelect                   */
        2,                   /* Descriptor number - determines base address. */
        ISA_NO_WAIT_STATES,  /* Ext wait                     */
        MEMORY,              /* Memory mapped.               */
        OFF,                 /* IOReg signal is not used.    */
        500,                 /* Write access time (ns).      */
        500,                 /* Read access time (ns).       */
        0,                   /* IOReg setup time (ns).       */
        0,                   /* IOReg access time (ns).      */
        100,                 /* Chip select setup time (ns). */
        100,                 /* Chip select hold time (ns).  */
        200,                 /* Control setup time (ns).     */
        100,                 /* Control hold time (ns).      */
    },
#endif /* #if (GPIO_CONFIG == GPIOM_BRAZOS) */

    /********************************/
    /* Telegraph I/O Mapped Devices */   
    /********************************/  
    {
        0,      /* Is it clocked.               */
       	0,      /* XOEMask for each board       */
        ISA_CHIP_SELECT_2,   /* ChipSelect                   */
        4,      /* Descriptor number - determines base address. */
        ISA_WAIT_UNTIL_READY_1,  /* Ext wait    */
        IO,     /* Use IO control signals.      */
        OFF,    /* IOReg signal is not used.    */
        500,    /* Write access time (ns).      */
        500,    /* Read access time (ns).       */
        0,      /* IOReg setup time (ns).       */
        0,      /* IOReg access time (ns).      */
        100,    /* Chip select setup time (ns). */
        100,    /* Chip select hold time (ns).  */
        200,    /* Control setup time (ns).     */
        100,    /* Control hold time (ns).      */
    },
              
    /***************/
    /* Modem  Card */   
    /***************/  
    {
        0,      /* Is it clocked. */
        0,      /* XOEMask        */
        ISA_CHIP_SELECT_3,   /* ChipSelect                   */
        5,      /* Descriptor number - determines base address. */

#if CABLE_MODEM != NOT_PRESENT /* Cable modem present */
  #if CABLE_MODEM == EXTERNAL_CX24943

        ISA_WAIT_UNTIL_READY_0,  /* Ext wait  -- wait until ISA RDY line goes to 0 */
        MEMORY,                  /* Use MEMORY control signals.                    */
        OFF,                     /* IOReg signal is not used.                      */
        100,                     /* Write access time (ns).                        */
        100,                     /* Read access time (ns).                         */
        0,                       /* IOReg setup time (ns).                         */
        0,                       /* IOReg access time (ns).                        */
        100,                     /* Chip select setup time (ns).                   */
        30,                      /* Chip select hold time (ns).                    */
        300,                     /* Control setup time (ns).                       */
        100                      /* Control hold time (ns).                        */

  #else

        ISA_WAIT_UNTIL_READY_1,  /* Ext wait                          */
    #if GPIO_CONFIG == GPIOM_COLORADO || GPIO_CONFIG == GPIOM_HONDO
        IO,                      /* Use IO control signals.           */
    #else
        MEMORY,                  /* Use MEMORY control signals.       */
    #endif
        OFF,                     /* IOReg signal is not used.         */
        400,                     /* Write access time (ns).           */
        260,                     /* Read access time (ns).            */
        0,                       /* IOReg setup time (ns).            */
        0,                       /* IOReg access time (ns).           */
        20,                      /* Chip select setup time (ns).      */
        60,                      /* Chip select hold time (ns).       */
        60,                      /* Control setup time (ns).          */
        60                       /* Control hold time (ns).           */

  #endif
#else /* Cable modem not present */

        ISA_WAIT_UNTIL_READY_1,  /* Ext wait                          */
  #if GPIO_CONFIG == GPIOM_COLORADO || GPIO_CONFIG == GPIOM_HONDO
        IO,                      /* Use IO control signals.           */
  #else
        MEMORY,                  /* Use MEMORY control signals.       */
  #endif
        OFF,                     /* IOReg signal is not used.         */
        400,                     /* Write access time (ns).           */
        260,                     /* Read access time (ns).            */
        0,                       /* IOReg setup time (ns).            */
        0,                       /* IOReg access time (ns).           */
        20,                      /* Chip select setup time (ns).      */
        60,                      /* Chip select hold time (ns).       */
        60,                      /* Control setup time (ns).          */
        60                       /* Control hold time (ns).           */
#endif

    },
              
    /********************/
    /* Common Interface */
    /********************/  
    {
        0,      /* Is it clocked.               */
        0,      /* XOEMask                      */ 
        ISA_CHIP_SELECT_4,   /* ChipSelect                   */
        7,      /* Descriptor number - determines base address. */
        ISA_NO_WAIT_STATES, /* Ext wait         */
        IO,     /* Use IO control signals.      */
        OFF,    /* IOReg signal is not used.    */
        150,    /* Write access time (ns).      */
        150,    /* Read access time (ns).       */
        0,      /* IOReg setup time (ns).       */
        0,      /* IOReg access time (ns).      */
        60,     /* Chip select setup time (ns). */
        10,     /* Chip select hold time (ns).  */
        30,     /* Control setup time (ns).     */
        125      /* Control hold time (ns).     */

    }

    /**************************/
    /* Internal ATA Subsystem */   
    /**************************/

    /*    Uses ISA descriptor 6     */

    /*
     * The internal ATA subsystem uses one primary and one secondary ISA
     * descriptor.  The setup is done in ConfigATA().  This uses ISA
     * descriptor 6.  DO NOT USE ISA DESCRIPTOR 6 FOR ANYTHING ELSE!!!
     *
     */

#elif (PCI_ISA_DESCRIPTOR_SETUP == DESCRIPTOR_SETUP_MILANO)
    /* Milano has no external (ie. ISA like) buses.  It has the following
     * devices:
     * CS0 - Descriptor #? - ROM0
     * CS1 - Descriptor #? - ROM1
     * CS2 - Descriptor #3 - PIO Expander (using MEM rd/wr signals)
     *       Descriptor #6 - IDE (using HD rd/wr signals)
     * CS3 - Descriptor #7 - POD PC card & data channel CS
     * CS4 - Descriptor #7 - POD extended channel CS
     * CS5 - Descriptor #6 - 1394
     */

	/* PIO expander    */
    {
        0,      /* Is it clocked.               */
        1,      /* XOEMask (not buffered)       */
        2,      /* ChipSelect                   */
        3,      /* Descriptor number - determines base address. */
        ISA_NO_WAIT_STATES,  /* Ext wait    */
        MEMORY, /* Use IO control signals.      */
        OFF,    /* IOReg signal is not used.    */
        500,    /* Write access time (ns).      */
        500,    /* Read access time (ns).       */
        0,      /* IOReg setup time (ns).       */
        0,      /* IOReg access time (ns).      */
        100,    /* Chip select setup time (ns). */
        100,    /* Chip select hold time (ns).  */
        200,    /* Control setup time (ns).     */
        100,    /* Control hold time (ns).      */
    },

	/* POD descriptor for memory (PC Card) mode */
 	/* Note: the PODHI code changes the descriptor registers, but not the XOE mask register */
    {
        0,      /* Is it clocked.               */
        0,      /* XOEMask (buffered)           */
        3,      /* ChipSelect                   */
        7,      /* Descriptor number            */
        ISA_WAIT_UNTIL_READY_1,    /* Ext wait  */
        MEMORY, /* Use IO control signals.      */
        PCCARD,    /* IOReg signal is used.     */
        465,    /* Write access time (ns).      */
        465,    /* Read access time (ns).       */
        0,      /* IOReg setup time (ns).       */
        0,      /* IOReg access time (ns).      */
        0,      /* Chip select setup time (ns). */
        35,     /* Chip select hold time (ns).  */
        100,    /* Control setup time (ns).     */
        35,     /* Control hold time (ns).      */
    },

	/* POD descriptor for I/O extended channel  */
    {
        0,      /* Is it clocked.               */
        0,      /* XOEMask (buffered)           */
        4,      /* ChipSelect                   */
        7,      /* Descriptor number            */
        ISA_WAIT_UNTIL_READY_1,    /* Ext wait  */
        IO,     /* Use IO control signals.      */
        PCCARD,    /* IOReg signal is used.    */
        465,    /* Write access time (ns).      */
        465,    /* Read access time (ns).       */
        0,      /* IOReg setup time (ns).       */
        0,      /* IOReg access time (ns).      */
        0,      /* Chip select setup time (ns). */
        35,     /* Chip select hold time (ns).  */
        100,    /* Control setup time (ns).     */
        35,     /* Control hold time (ns).      */
    },

    /* 1394 Descriptor Definition */
    {
        0,      /* Is it clocked.               */
        1,      /* XOEMask for each board       */
        CNXT_FIREWIRE_CHIP_SELECT,   /* ChipSelect = 5                  */
        CNXT_FIREWIRE_DESCRIPTOR,      /* Descriptor number - determines base address. */
        ISA_WAIT_UNTIL_READY_1,  /* Ext wait    */                                     
        IO, /* Use IO control signals.      */                                         
        PCCARD,    /* IOReg signal is not used.    */
        500,    /* Write access time (ns).      62.5*/
        500,    /* Read access time (ns).       62.5*/
        0,      /* IOReg setup time (ns).       0 */
        0,      /* IOReg access time (ns).      0*/
        120,    /* 120 Chip select setup time (ns). 12.5*/
        56,    /* 56 Chip select hold time (ns).  12.5 */
        112,    /* 112 Control setup time (ns).     25  */
        56,    /* 56 Control hold time (ns).      12.5   */
    }

    /*****************************/
    /* Nand Flash Driver for     */
    /* 1. Milano rev 5 and above */
    /* 2. Bronco                 */
    /*****************************/
    /*    Uses ISA descriptor 2     */
    /*
     * The Nand Flash driver uses one primary and one secondary ISA
     * descriptor.  The setup is done in SetupNandFlash().  This uses ISA
     * descriptor 2.  DO NOT USE ISA DESCRIPTOR 2 FOR ANYTHING ELSE!!!
     *
     */

#else
#error Unknown PCI_ISA_DESCRIPTOR type
#endif
};
int iNumISADevices = sizeof(ISADevices)/sizeof(ISA_DEVICE);
#endif

/****************************************************************************
 * Modifications:
 * $Log: 
 *  20   mpeg      1.19        4/22/04 4:20:33 PM     Sunil Cheruvu   CR(s) 
 *        8870 8871 : Added the Nand Flash support for Wabash(Milano rev 5 and 
 *        above) and Brazos(Bronco).
 *  19   mpeg      1.18        1/27/04 12:38:45 PM    Bobby Bradford  CR(s) 
 *        8278 : Add descriptor (Brazos-only) to support Electra Ethernet 
 *        operation on Bronco IRD.  As far as I can tell, this descriptor is 
 *        not used anywhere else.
 *  18   mpeg      1.17        8/27/03 9:28:04 PM     Moshe Yehushua  SCR(s): 
 *        7173 7174 
 *        Added POD descriptor
 *        
 *  17   mpeg      1.16        8/26/03 5:05:04 PM     Sahil Bansal    SCR(s): 
 *        7378 
 *        Changed XOEMask Value from 0 to 1 for 1394 on Milano.
 *        
 *  16   mpeg      1.15        7/24/03 5:14:22 PM     Sahil Bansal    SCR(s): 
 *        7038 
 *        Added 1394 descriptor to descriptor array
 *        
 *  15   mpeg      1.14        7/8/03 2:36:12 PM      Miles Bintz     SCR(s) 
 *        6807 :
 *        changed descriptor from ISA_DESCRIPTOR_2 to just 2
 *        
 *  14   mpeg      1.13        7/7/03 11:54:28 AM     Miles Bintz     SCR(s) 
 *        6807 :
 *        changed descriptor configuration for milano boards
 *        
 *  13   mpeg      1.12        2/13/03 11:18:34 AM    Matt Korte      SCR(s) 
 *        5479 :
 *        Removed old header reference
 *        
 *  12   mpeg      1.11        1/22/03 11:42:02 AM    Dave Wilson     SCR(s) 
 *        5099 :
 *        Used ISA_CHIP_SELECT_n overrides from hardware config file rather 
 *        than
 *        hardcoded numbers for each device's chip select. This allows us to 
 *        tailor
 *        these via the config file rather than having to edit the code.
 *        
 *  11   mpeg      1.10        12/20/02 1:40:58 PM    Tim White       SCR(s) 
 *        5068 :
 *        Removed telegraph card when using the emulator.
 *        
 *        
 *  10   mpeg      1.9         12/12/02 5:25:40 PM    Tim White       SCR(s) 
 *        5157 :
 *        Fixed bug where if GPIO_CONFIG was not Colorado or Wabash, it would 
 *        build break.
 *        
 *        
 *  9    mpeg      1.8         3/26/02 4:33:52 PM     Carroll Vance   SCR(s) 
 *        3451 :
 *        Changed CABLE_DEVICE to CABLE_MODEM.  Added NOT_PRESENT case.
 *        
 *        
 *  8    mpeg      1.7         1/14/02 10:59:30 AM    Dave Moore      SCR(s) 
 *        3006 :
 *        Added treatment of external cx24943 cable modem.
 *        
 *        
 *  7    mpeg      1.6         11/20/01 11:35:46 AM   Quillian Rutherford 
 *        SCR(s) 2754 :
 *        Updated for emulation
 *        
 *  6    mpeg      1.5         8/7/01 3:43:40 PM      Angela          SCR(s) 
 *        2437 :
 *        Changed descriptor 5 to Memory mode if Hondo
 *        
 *  5    mpeg      1.4         7/3/01 11:07:00 AM     Tim White       SCR(s) 
 *        2178 2179 2180 :
 *        Merge branched Hondo specific code back into the mainstream source 
 *        database.
 *        
 *        
 *  4    mpeg      1.3         11/7/00 10:54:48 AM    Ray Mack        Picked up
 *         Descriptor 3 for Telegraph Memory mapped devices
 *        
 *  3    mpeg      1.2         10/30/00 6:02:54 PM    Joe Kroesche    fixed up 
 *        descriptors and timings
 *        
 *  2    mpeg      1.1         10/2/00 3:47:48 PM     Miles Bintz     changed 
 *        timings in decriptor 7 so that telegraph works when running from ROM
 *        
 *  1    mpeg      1.0         9/23/00 5:54:58 PM     Joe Kroesche    
 * $
 * 
 *    Rev 1.17   27 Aug 2003 20:28:04   yehushm
 * SCR(s): 7173 7174 
 * Added POD descriptor
 * 
 *    Rev 1.16   26 Aug 2003 16:05:04   bansals
 * SCR(s): 7378 
 * Changed XOEMask Value from 0 to 1 for 1394 on Milano.
 * 
 *    Rev 1.15   24 Jul 2003 16:14:22   bansals
 * SCR(s): 7038 
 * Added 1394 descriptor to descriptor array
 * 
 *    Rev 1.14   08 Jul 2003 13:36:12   bintzmf
 * SCR(s) 6807 :
 * changed descriptor from ISA_DESCRIPTOR_2 to just 2
 * 
 *    Rev 1.13   07 Jul 2003 10:54:28   bintzmf
 * SCR(s) 6807 :
 * changed descriptor configuration for milano boards
 * 
 *    Rev 1.12   13 Feb 2003 11:18:34   kortemw
 * SCR(s) 5479 :
 * Removed old header reference
 * 
 *    Rev 1.11   22 Jan 2003 11:42:02   dawilson
 * SCR(s) 5099 :
 * Used ISA_CHIP_SELECT_n overrides from hardware config file rather than
 * hardcoded numbers for each device's chip select. This allows us to tailor
 * these via the config file rather than having to edit the code.
 *
 ****************************************************************************/

