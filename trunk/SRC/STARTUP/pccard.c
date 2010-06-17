/****************************************************************************/ 
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*          Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002          */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        pccard.c
 *
 *
 * Description:     PCCard routines
 *
 *
 * Author:          Tim Ross
 *
 ****************************************************************************/
/* $Header: pccard.c, 6, 2/13/03 12:20:56 PM, Matt Korte$
 ****************************************************************************/ 

/******************/
/* Include Files  */
/******************/
#include "stbcfg.h"
#include "basetype.h"
#include "board.h"
#include "pccard.h"

#if CUSTOMER != VENDOR_A && CUSTOMER != VENDOR_D

/**********************/
/* Local Definitions  */
/**********************/

/***************/
/* Global Data */
/***************/


/********************************************************************/
/*  InitPCCardBridge                                                */
/*                                                                  */
/*  PARAMETERS:                                                     */
/*      iPCCardBridgeSlot - slot number of PCCard bridge device.    */
/*                                                                  */
/*  DESCRIPTION:                                                    */
/*      Initializes the PCI to PCCard bridge.                       */
/*                                                                  */
/*  RETURNS:                                                        */
/*      Nothing.                                                    */
/********************************************************************/
void InitPCCardBridge(int iPCCardBridgeSlot)
{
    LPREG pPCICfgAddr = (LPREG)PCI_CFG_ADDR_REG;
    LPREG pPCICfgData = (LPREG)PCI_CFG_DATA_REG;
    u_int32 loc;

    /**********************************************/
    /* Setup PCI - PCCard Bridge if it was found. */
    /**********************************************/
    loc = ((0 << PCI_CFG_ADDR_BUS_SHIFT)      |
           (0 << PCI_CFG_ADDR_FUNCTION_SHIFT) |
           (iPCCardBridgeSlot << PCI_CFG_ADDR_DEVICE_SHIFT));

    /*
     * Set the ExCA registers base address.
     */
    *pPCICfgAddr = loc | (PCI_PCCARD_BASE_ADDR_OFF << PCI_CFG_ADDR_REGISTER_SHIFT);
    *pPCICfgData = PCCARD_BRIDGE_REG_BASE;

    /*
     * Set the PCI latency timer.
     */
    *pPCICfgAddr = loc | (PCI_CLS_LT_HT_BIST_OFF << PCI_CFG_ADDR_REGISTER_SHIFT);
    *pPCICfgData = PCCARD_LATENCY << 8;

    /*
     * Set the PCI Bus #, CardBus bus #, subordinate bus #, and CardBus latency.
     */
    *pPCICfgAddr = loc | (PCI_BUS_NUM_OFF << PCI_CFG_ADDR_REGISTER_SHIFT);
    *pPCICfgData = (CARDBUS_BUS_NUM << 8) | (CARDBUS_SUB_BUS_NUM << 16) |
                   (CARDBUS_LATENCY << 24);

    /*
     * Disable all Cardbus memory and I/O transactions.
     */
    *pPCICfgAddr = loc | (PCI_CARDBUS_MEM_BASE_0_OFF << PCI_CFG_ADDR_REGISTER_SHIFT);
    *pPCICfgData = 0;

    *pPCICfgAddr = loc | (PCI_CARDBUS_MEM_LIMIT_0_OFF << PCI_CFG_ADDR_REGISTER_SHIFT);
    *pPCICfgData = 0;

    *pPCICfgAddr = loc | (PCI_CARDBUS_MEM_BASE_1_OFF << PCI_CFG_ADDR_REGISTER_SHIFT);
    *pPCICfgData = 0;

    *pPCICfgAddr = loc | (PCI_CARDBUS_MEM_LIMIT_1_OFF << PCI_CFG_ADDR_REGISTER_SHIFT);
    *pPCICfgData = 0;

    *pPCICfgAddr = loc | (PCI_CARDBUS_IO_BASE_0_OFF << PCI_CFG_ADDR_REGISTER_SHIFT);
    *pPCICfgData = 0;

    *pPCICfgAddr = loc | (PCI_CARDBUS_IO_LIMIT_0_OFF << PCI_CFG_ADDR_REGISTER_SHIFT);
    *pPCICfgData = 0;

    *pPCICfgAddr = loc | (PCI_CARDBUS_IO_BASE_1_OFF << PCI_CFG_ADDR_REGISTER_SHIFT);
    *pPCICfgData = 0;

    *pPCICfgAddr = loc | (PCI_CARDBUS_IO_LIMIT_1_OFF << PCI_CFG_ADDR_REGISTER_SHIFT);
    *pPCICfgData = 0;

    /*
     * Setup bridge control register to all defaults except CardBus reset
     * which is deasserted.
     */
    *pPCICfgAddr = loc | (PCI_CARDBUS_BRIDGE_CTRL_OFF << PCI_CFG_ADDR_REGISTER_SHIFT);
    *pPCICfgData = 0;

    /*
     * Setup PCCard legacy mode index/data I/O base address.
     */
    *pPCICfgAddr = loc | (PCI_PCCARD_IO_BASE_OFF << PCI_CFG_ADDR_REGISTER_SHIFT);
    *pPCICfgData = PCCARD_BRIDGE_IO_BASE;

    /*
     * Set the system control register to mostly defaults.
     */
    *pPCICfgAddr = loc | (PCI_PCCARD_SYS_CTRL_OFF << PCI_CFG_ADDR_REGISTER_SHIFT);
    *pPCICfgData = 0;

    /*
     * Set the PCI1210's GPIO's to default input state, except MF0 
     * which becomes PCI INTA.
     */
    *pPCICfgAddr = loc | (PCI_PCCARD_GPIO_OFF << PCI_CFG_ADDR_REGISTER_SHIFT);
    *pPCICfgData = 0x2;

    /*
     * Disable the retry counters, use PCI interrupts only, and
     * keep diags register untouched.
     */
    *pPCICfgAddr = loc | (PCI_PCCARD_RTY_DC_CC_DR_OFF << PCI_CFG_ADDR_REGISTER_SHIFT);
    *pPCICfgData = (0x61 << 24);    /* 0x61 is diags register default value */

    /*
     * Disable socket DMA.
     */
    *pPCICfgAddr = loc | (PCI_PCCARD_SKT_DMA_OFF << PCI_CFG_ADDR_REGISTER_SHIFT);
    *pPCICfgData = 0;

    *pPCICfgAddr = loc | (PCI_PCCARD_DDMA_OFF << PCI_CFG_ADDR_REGISTER_SHIFT);
    *pPCICfgData = 0;

    /*
     * Set the CardBus power management.
     */
    *pPCICfgAddr = loc | (PCI_CARDBUS_POWER_OFF << PCI_CFG_ADDR_REGISTER_SHIFT);
    *pPCICfgData = 0;

    /*
     * Disable the GPIO events and zero all GPIO output values.
     */
    *pPCICfgAddr = loc | (PCI_PCCARD_GPE_OFF << PCI_CFG_ADDR_REGISTER_SHIFT);
    *pPCICfgData = 0;

    *pPCICfgAddr = loc | (PCI_PCCARD_GPIO_IO_OFF << PCI_CFG_ADDR_REGISTER_SHIFT);
    *pPCICfgData = 0;
    
    /*
     * Enable memory and I/O accesses
     */
    *pPCICfgAddr = loc | (PCI_CMD_STAT_OFF << PCI_CFG_ADDR_REGISTER_SHIFT);
    *pPCICfgData |= (PCI_CMD_IO_ACCESS_ENABLE | PCI_CMD_MEM_ACCESS_ENABLE);
}

#endif

/****************************************************************************
 * Modifications:
 * $Log: 
 *  6    mpeg      1.5         2/13/03 12:20:56 PM    Matt Korte      SCR(s) 
 *        5479 :
 *        Removed old header reference
 *        
 *  5    mpeg      1.4         4/10/02 5:11:16 PM     Tim White       SCR(s) 
 *        3509 :
 *        Eradicate external bus (PCI) bitfield usage.
 *        
 *        
 *  4    mpeg      1.3         4/5/02 11:51:50 AM     Tim White       SCR(s) 
 *        3510 :
 *        Backout DCS #3468
 *        
 *        
 *  3    mpeg      1.2         3/28/02 2:50:54 PM     Quillian Rutherford 
 *        SCR(s) 3468 :
 *        Removed bit fields
 *        
 *        
 *  2    mpeg      1.1         1/31/01 2:30:12 PM     Tim White       DCS#0988:
 *         Reclaim footprint space.
 *        
 *  1    mpeg      1.0         6/8/99 12:50:52 PM     Tim Ross        
 * $
 * 
 *    Rev 1.5   13 Feb 2003 12:20:56   kortemw
 * SCR(s) 5479 :
 * Removed old header reference
 * 
 *    Rev 1.4   10 Apr 2002 16:11:16   whiteth
 * SCR(s) 3509 :
 * Eradicate external bus (PCI) bitfield usage.
 * 
 *
 ****************************************************************************/ 

