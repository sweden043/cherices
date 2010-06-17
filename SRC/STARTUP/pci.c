/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                    Conexant Systems Inc. (c) 1998-2003                   */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        pci.c
 *
 *
 * Description:     PCI specific routines
 *
 *
 * Author:          Tim Ross
 *
 ****************************************************************************/
/* $Header: pci.c, 19, 5/5/03 5:06:18 PM, Tim White$
 ****************************************************************************/

/******************/
/* Include Files  */
/******************/
#include "stbcfg.h"
#include "basetype.h"
#include "board.h"
#include "startup.h"
#include "startuppci.h"

/**********************/
/* Local Definitions  */
/**********************/
u_int32 pciIoSpaceSize, pciMemSpaceSize;

#define ROUND_UP(x, align)	(((int) (x) + (align - 1)) & ~(align - 1))

/***************/
/* Global Data */
/***************/

/***************/
/* Externs     */
/***************/
extern PCI_CONFIG PCIDevices[];
extern int iNumPCIDevices;
/********************************************************************/
/*  InitPCI                                                         */
/*                                                                  */
/*  PARAMETERS:                                                     */
/*      None.                                                       */
/*                                                                  */
/*  DESCRIPTION:                                                    */
/*      Initializes PCI bridge and devices (except other bridges).  */
/*                                                                  */
/*  RETURNS:                                                        */
/*      nothing                                                     */
/********************************************************************/
void InitPCI( void )
{
   LPREG pPCICfgAddr = (LPREG)PCI_CFG_ADDR_REG;
   LPREG pPCICfgData = (LPREG)PCI_CFG_DATA_REG;
   u_int32 pci_io_addr = PCI_IO_BASE;
   u_int32 pci_mem_addr = PCI_MEM_BASE;
   u_int32 pc_block_size;
   u_int32 loc, dev_ven;
   int     found_config, i, j, k;
   
   /***************************************************/
   /* Take PCI out of reset an setup ARM-PCI bridge. */
   /***************************************************/
   *((LPREG)PCI_RESET_REG) &= ~PCI_RESET_ASSERTED;

   /*
    * Set bridge's memory space to non-cacheable system memory
    */
   *pPCICfgAddr = (PCI_BASE_ADDR_0_OFF << PCI_CFG_ADDR_REGISTER_SHIFT);
   *pPCICfgData = NCR_BASE;

   /*
    * Turn on bridge's mastering and memory accessibility
    */
   *pPCICfgAddr = (PCI_CMD_STAT_OFF << PCI_CFG_ADDR_REGISTER_SHIFT);
   *pPCICfgData |= (PCI_CMD_MASTER_ENABLE | PCI_CMD_MEM_ACCESS_ENABLE);

   /**********************/
   /* Setup PCI devices. */
   /**********************/
   /*
    * Scan all slots looking for known devices
    */
   for ( i = 1 ; i < 31 ; i ++ )
   {
      loc = ( ( 0 << PCI_CFG_ADDR_BUS_SHIFT )      |
              ( 0 << PCI_CFG_ADDR_FUNCTION_SHIFT ) |
              ( i << PCI_CFG_ADDR_DEVICE_SHIFT ) );
      *pPCICfgAddr = loc | ( PCI_ID_OFF << PCI_CFG_ADDR_REGISTER_SHIFT );

      if ( ( dev_ven=*pPCICfgData ) != -1 ) 
      {
         /*
          * Somebody is at home ...
          */
         found_config = FALSE;

         for (j=0; j<iNumPCIDevices; j++) 
         {
            /*
             * When we find a device we know about, jam in config values from table...
             */
            if (dev_ven == PCIDevices[j].ID) 
            {
               found_config = TRUE;

               for ( k = 0 ; k < 6 ; k ++ )
               {
                  if ( PCIDevices[j].SizeOnBar[k] )
                  {
                     pc_block_size = PCIDevices[j].SizeOnBar[k] & 0x7fffffff;

                     if ( PCIDevices[j].SizeOnBar[k] & 0x80000000 )
                     {
                        /* I/O space */
                        pci_io_addr = ROUND_UP ( pci_io_addr, pc_block_size );
                        *pPCICfgAddr = loc | ( ( PCI_BASE_ADDR_0_OFF + k ) << PCI_CFG_ADDR_REGISTER_SHIFT );
                        *pPCICfgData = pci_io_addr;
                        pci_io_addr += pc_block_size;
                     }
                     else
                     {
                        /* memory space */
                        pci_mem_addr = ROUND_UP ( pci_mem_addr, pc_block_size );
                        *pPCICfgAddr = loc | ( ( PCI_BASE_ADDR_0_OFF + k ) << PCI_CFG_ADDR_REGISTER_SHIFT );
                        *pPCICfgData = pci_mem_addr;
                        pci_mem_addr += pc_block_size;
                     }
                  }
               }

               *pPCICfgAddr = loc | (PCI_CLS_LT_HT_BIST_OFF << PCI_CFG_ADDR_REGISTER_SHIFT);
               *pPCICfgData = PCIDevices[j].Latency_Cache;

               /*
                * Enable Decoders
                */
               *pPCICfgAddr = loc | (PCI_CMD_STAT_OFF << PCI_CFG_ADDR_REGISTER_SHIFT);
               *pPCICfgData = PCIDevices[j].IOAccess |
                              (PCIDevices[j].MemoryAccess << 1) |
                              (PCIDevices[j].BusMastering << 2);

               break;
            }
         }

         if (!found_config)
         {
            Checkpoint("InitPCI: unknown device found..\n\r");
         }
      }
   }

   /*determine PCI I/O and memory space size */
   pciIoSpaceSize  = pci_io_addr - PCI_IO_BASE;
   pciMemSpaceSize = pci_mem_addr - PCI_MEM_BASE;
}


/****************************************************************************
 * Modifications:
 * $Log: 
 *  19   mpeg      1.18        5/5/03 5:06:18 PM      Tim White       SCR(s) 
 *        6172 :
 *        Remove duplicate low-level boot support code and use startup 
 *        directory for building
 *        codeldr.  Remove 7 segment LED support.
 *        
 *        
 *  18   mpeg      1.17        2/13/03 12:21:00 PM    Matt Korte      SCR(s) 
 *        5479 :
 *        Removed old header reference
 *        
 *  17   mpeg      1.16        10/1/02 12:20:36 PM    Miles Bintz     SCR(s) 
 *        4644 :
 *        Removed pci_get_bar and moved it to hwlib.c
 *        
 *        
 *  16   mpeg      1.15        9/23/02 3:21:50 PM     Larry Wang      SCR(s) 
 *        4638 :
 *        (1) In InitPCI(), assign memory space according to the size defined 
 *        in PCIDevices table; (2) In pci_get_bar(), read the base address from
 *         PCI config registers.
 *        
 *  15   mpeg      1.14        7/16/02 8:03:02 PM     Matt Korte      SCR(s) 
 *        4215 :
 *        Clean up warnings.
 *        
 *        
 *  14   mpeg      1.13        4/25/02 1:02:46 AM     Tim Ross        SCR(s) 
 *        3620 :
 *        Changed PCIDevices declaration to add a [] on the end, making the 
 *        compiler 
 *        recognize it properly.
 *        
 *  13   mpeg      1.12        4/23/02 11:37:08 AM    Dave Moore      SCR(s) 
 *        3587 :
 *        added pci_get_bar()
 *        
 *        
 *  12   mpeg      1.11        4/10/02 5:11:20 PM     Tim White       SCR(s) 
 *        3509 :
 *        Eradicate external bus (PCI) bitfield usage.
 *        
 *        
 *  11   mpeg      1.10        4/5/02 11:51:40 AM     Tim White       SCR(s) 
 *        3510 :
 *        Backout DCS #3468
 *        
 *        
 *  10   mpeg      1.9         3/28/02 2:49:36 PM     Quillian Rutherford 
 *        SCR(s) 3468 :
 *        Removed bit fields
 *        
 *        
 *  9    mpeg      1.8         11/29/01 1:03:06 PM    Miles Bintz     SCR(s) 
 *        2936 :
 *        Moved definitions that were originally in include\cnxtpci.h to 
 *        startup\startuppci.h
 *        
 *  8    mpeg      1.7         6/12/01 4:53:30 PM     Dave Moore      SCR(s) 
 *        2082 :
 *        Updated for Hondo.
 *        
 *        
 *  7    mpeg      1.6         1/31/01 2:30:24 PM     Tim White       DCS#0988:
 *         Reclaim footprint space.
 *        
 *  6    mpeg      1.5         1/29/01 2:45:50 PM     Tim White       Changed 
 *        pci.h to cnxtpci.h.
 *        
 *  5    mpeg      1.4         4/5/00 6:24:16 PM      Ray Mack        fixes to 
 *        remove warnings
 *        
 *  4    mpeg      1.3         3/16/00 1:08:56 PM     Ray Mack        fixes for
 *         vxworks initial release
 *        
 *  3    mpeg      1.2         9/12/99 9:30:00 PM     Tim Ross        Added TI 
 *        PCI1211 PCI-PCCard bridge ID.
 *        
 *  2    mpeg      1.1         9/9/99 12:22:30 PM     Lucy C Allevato Fixed bug
 *         with setting up PCI memory space. It is defined as 0 or 1 in
 *        the structure so it needs to be shifted left by one to enable the 
 *        memory
 *        space.
 *        
 *  1    mpeg      1.0         6/8/99 12:53:58 PM     Tim Ross        
 * $
 * 
 *    Rev 1.18   05 May 2003 16:06:18   whiteth
 * SCR(s) 6172 :
 * Remove duplicate low-level boot support code and use startup directory for building
 * codeldr.  Remove 7 segment LED support.
 * 
 * 
 *    Rev 1.17   13 Feb 2003 12:21:00   kortemw
 * SCR(s) 5479 :
 * Removed old header reference
 * 
 *    Rev 1.16   01 Oct 2002 11:20:36   bintzmf
 * SCR(s) 4644 :
 * Removed pci_get_bar and moved it to hwlib.c
 * 
 * 
 *    Rev 1.15   23 Sep 2002 14:21:50   wangl2
 * SCR(s) 4638 :
 * (1) In InitPCI(), assign memory space according to the size defined in PCIDevices table; (2) In pci_get_bar(), read the base address from PCI config registers.
 * 
 *    Rev 1.14   16 Jul 2002 19:03:02   kortemw
 * SCR(s) 4215 :
 * Clean up warnings.
 * 
 * 
 *    Rev 1.13   25 Apr 2002 00:02:46   rossst
 * SCR(s) 3620 :
 * Changed PCIDevices declaration to add a [] on the end, making the compiler 
 * recognize it properly.
 * 
 *    Rev 1.12   23 Apr 2002 10:37:08   mooreda
 * SCR(s) 3587 :
 * added pci_get_bar()
 * 
 * 
 *    Rev 1.11   10 Apr 2002 16:11:20   whiteth
 * SCR(s) 3509 :
 * Eradicate external bus (PCI) bitfield usage.
 * 
 *
 ****************************************************************************/

