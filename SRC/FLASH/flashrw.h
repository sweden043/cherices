/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*          Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002          */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        flashrw.h
 *
 *
 * Description:     Mode macros
 *
 *
 * Author:          Tim White
 *
 ****************************************************************************/
/* $Header: flashrw.h, 10, 9/3/03 4:04:08 PM, Tim White$
 ****************************************************************************/

#ifndef _FLASHRW_H
#define _FLASHRW_H

/*****************/
/* Include Files */
/*****************/
#include "basetype.h"

/***************************************************/
/* Macro used to put flash into read or write mode */
/***************************************************/
#if CUSTOMER == VENDOR_A || CUSTOMER == VENDOR_D
#define ENTER_WRITE_MODE()                                                 \
{                                                                          \
        LPREG pPrimary   = (LPREG)PCI_DESC_BASE  + CurrentBank;            \
        LPREG pSecondary = (LPREG)PCI_DESC2_BASE + CurrentBank;            \
        *pPrimary |= PCI_ROM_DESC_FLASH_PROGRAM_MODE;                      \
        *pPrimary &= ~PCI_ROM_DESC_BURST_ENABLE;                           \
        *pSecondary = Bank[CurrentBank].WriteTimings;                      \
}
#define ENTER_READ_MODE()                                                  \
{                                                                          \
        LPREG pPrimary   = (LPREG)PCI_DESC_BASE  + CurrentBank;            \
        LPREG pSecondary = (LPREG)PCI_DESC2_BASE + CurrentBank;            \
        *pPrimary &= ~PCI_ROM_DESC_FLASH_PROGRAM_MODE;                     \
        *pSecondary = Bank[CurrentBank].ReadTimings;                       \
}
#else
#define ENTER_WRITE_MODE()                                                 \
{                                                                          \
        LPREG pPrimary   = (LPREG)PCI_DESC_BASE  + CurrentBank;            \
        LPREG pSecondary = (LPREG)PCI_DESC2_BASE + CurrentBank;            \
        *pPrimary |= PCI_ROM_DESC_FLASH_PROGRAM_MODE;                      \
        *pPrimary &= ~PCI_ROM_DESC_BURST_ENABLE;                           \
        if(Bank[CurrentBank].pFlashDesc->MfrID == FLASH_CHIP_VENDOR_INTEL) \
        {                                                                  \
            *pPrimary = ((*pPrimary & ~PCI_ROM_DESC_WRITE_WAIT_MASK) |     \
                         (Bank[CurrentBank].WriteModeTimings <<            \
                          PCI_ROM_DESC_WRITE_WAIT_SHIFT));                 \
        }                                                                  \
        *pSecondary = Bank[CurrentBank].WriteTimings;                      \
}

#define ENTER_READ_MODE()                                                  \
{                                                                          \
        LPREG pPrimary   = (LPREG)PCI_DESC_BASE  + CurrentBank;            \
        LPREG pSecondary = (LPREG)PCI_DESC2_BASE + CurrentBank;            \
        *pPrimary &= ~PCI_ROM_DESC_FLASH_PROGRAM_MODE;                     \
        if(Bank[CurrentBank].pFlashDesc->Flags & FLASH_FLAGS_BURST_SUPPORTED) \
        {                                                                  \
            *pPrimary |= PCI_ROM_DESC_BURST_ENABLE;                        \
        }                                                                  \
        if(Bank[CurrentBank].pFlashDesc->MfrID == FLASH_CHIP_VENDOR_INTEL) \
        {                                                                  \
            *pPrimary &= ~PCI_ROM_DESC_WRITE_WAIT_MASK;                    \
        }                                                                  \
        *pSecondary = Bank[CurrentBank].ReadTimings;                       \
}
 #if !defined(DLOAD) && CPU_TYPE == CPU_ARM920T && RTOS != VXWORKS
#define ENTER_WRITE_MODE_2()                                               \
{                                                                          \
        LPREG pPrimary   = (LPREG)PCI_ISAROM_DESC2_REG;                    \
        LPREG pSecondary = (LPREG)PCI_ISAROM_DESC2_REG2;                   \
        if(CurrentBank)                                                    \
        {                                                                  \
            *pPrimary   = *(LPREG)PCI_ROM_DESC1_REG;                       \
            *pSecondary = *(LPREG)PCI_ROM_DESC1_REG2;                      \
        }                                                                  \
        else                                                               \
        {                                                                  \
            *pPrimary   = *(LPREG)PCI_ROM_DESC0_REG;                       \
            *pSecondary = *(LPREG)PCI_ROM_DESC0_REG2;                      \
        }                                                                  \
        *pPrimary |= PCI_ROM_DESC_FLASH_PROGRAM_MODE;                      \
        *pPrimary &= ~PCI_ROM_DESC_BURST_ENABLE;                           \
        if(Bank[CurrentBank].pFlashDesc->MfrID == FLASH_CHIP_VENDOR_INTEL) \
        {                                                                  \
            *pPrimary = ((*pPrimary & ~PCI_ROM_DESC_WRITE_WAIT_MASK) |     \
                         (Bank[CurrentBank].WriteModeTimings <<            \
                          PCI_ROM_DESC_WRITE_WAIT_SHIFT));                 \
        }                                                                  \
        *pSecondary = Bank[CurrentBank].WriteTimings;                      \
}

/*
 * Defines the maximum size write for using the quick write method (in bytes)
 */
#define FLASH_QUICK_WRITE_LIMIT                        512

 #endif
#endif

#endif // _FLASHRW_H

/****************************************************************************
 * Modifications:
 * $Log: 
 *  10   mpeg      1.9         9/3/03 4:04:08 PM      Tim White       SCR(s) 
 *        7424 :
 *        Added support for the dual flash aperture.
 *        
 *        
 *  9    mpeg      1.8         4/10/02 5:05:12 PM     Tim White       SCR(s) 
 *        3509 :
 *        Eradicate external bus (PCI) bitfield usage.
 *        
 *        
 *  8    mpeg      1.7         6/22/01 3:12:32 PM     Tim White       SCR(s) 
 *        2150 2155 :
 *        > Set WriteWait to 0 in order to force Intel Flash ROM parts
 *        to ignore all writes when it's in READ_MODE.
 *        
 *        
 *  7    mpeg      1.6         4/12/01 7:24:04 PM     Amy Pratt       DCS914 
 *        Removed Neches support.
 *        
 *  6    mpeg      1.5         1/31/01 2:33:38 PM     Tim White       DCS#0988:
 *         Reclaim footprint space.
 *        
 *  5    mpeg      1.4         9/26/00 6:10:08 PM     Tim White       Added 
 *        switchable secondary descriptor capability to low-level flash 
 *        function.
 *        
 *  4    mpeg      1.3         3/8/00 5:18:08 PM      Tim White       
 *        Restructured the BANK_INFO array and added support for new Intel 
 *        Flash ROM.
 *        
 *  3    mpeg      1.2         3/2/00 5:40:08 PM      Tim White       Modified 
 *        the ENTER_READ_MODE() and ENTER_WRITE_MODE() macros to test for
 *        Neches chip and use the global bit set or use the per ROM desc 
 *        setting
 *        for Colorado (and follow-ons).
 *        
 *  2    mpeg      1.1         3/1/00 1:36:02 PM      Tim Ross        Replaced 
 *        ENTER_READ/WRITE_MODE macros with function
 *        calls EnterRead/WriteMode().
 *        
 *  1    mpeg      1.0         10/13/99 4:46:52 PM    Tim White       
 * $
 * 
 *    Rev 1.9   03 Sep 2003 15:04:08   whiteth
 * SCR(s) 7424 :
 * Added support for the dual flash aperture.
 * 
 * 
 *    Rev 1.8   10 Apr 2002 16:05:12   whiteth
 * SCR(s) 3509 :
 * Eradicate external bus (PCI) bitfield usage.
 * 
 *
 ****************************************************************************/

