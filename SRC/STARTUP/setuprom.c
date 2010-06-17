/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                   Conexant Systems Inc. (c) 1999-2003                    */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        setuprom.c
 *
 *
 * Description:     ROM descriptor setup and flash info gathering
 *
 *
 * Author:          Tim White
 *
 ****************************************************************************/
/* $Header: setuprom.c, 48, 10/21/03 9:48:41 AM, Larry Wang$
 ****************************************************************************/


/*
 * !!!!IMPORTANT!!!!   !!!!IMPORTANT!!!!   !!!!IMPORTANT!!!!  
 * !!!!IMPORTANT!!!!   !!!!IMPORTANT!!!!   !!!!IMPORTANT!!!!  
 *                                                              
 * Care must be taken with the SetupROMs(), RealSetupROMs(), and the Enter/Exit
 * ROM ID vector functions.  The order is imporant in this file since all
 * functions up to the Fake ID functions are copied to RAM if the function(s)
 * was called from ROM.
 *
 * It is important that RealSetupROMs() not call any functions except the
 * Enter/Exit ROM ID vector functions since the functions will not be copied to
 * RAM and when the RealSetupROM() function puts ROM into ID mode, ROM is no
 * longer addressible, hence you're hosed.  Any new function calls that need
 * to be added *must* be put in this file between RealSetupROMs() and SetupROMs()
 * and those functions cannot call anything else.  Pay particular attention to
 * not use operators etc which causes any runtime library function from being
 * called.  No printf()'s, no strxxx()'s, no Checkpoint()'s, no nothing, period.
 * An example would be something like: a = b / c where all are int's.  The "/"
 * divide will end being a function call to a realtime math support library.  
 * This cannot be done in RealSetupROMs().                    
 *                                                              
 * The SetupROMs() function also depends on the format of the FLASH_DESC
 * structure defined in INCLUDE/BOARD.H.  If the location of the Enter/Exit
 * ROM ID vector functions change in this structure, SetupROMs() has to be
 * modified!         
 *                                                              
 * !!!!IMPORTANT!!!!   !!!!IMPORTANT!!!!   !!!!IMPORTANT!!!!  
 * !!!!IMPORTANT!!!!   !!!!IMPORTANT!!!!   !!!!IMPORTANT!!!!  
 */

/******************/
/* Include Files  */
/******************/
#include "stbcfg.h"
#include "basetype.h"
#include "board.h"
#include "startup.h"
#ifdef DLOAD
#include "defs.h"
#endif

#if CUSTOMER != VENDOR_A && CUSTOMER != VENDOR_D

/**********************/
/* Local Definitions  */
/**********************/
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

/***************/
/* Global Data */
/***************/

/*****************/
/* External Data */
/*****************/
extern u_int8 NumROMBanks;
extern BANK_INFO Bank[];
extern FLASH_DESC FlashDescriptors[];
extern u_int32 Image$$ZI$$Limit;

/********************************************************************/
/*  RealSetupROMs  (SetupROMs)                                      */
/*                                                                  */
/*  PARAMETERS:                                                     */
/*      Ptr to FlashDescriptors.                                    */
/*                                                                  */
/*  DESCRIPTION:                                                    */
/*      Determines the ROM configuration and programs the ROM       */
/*      descriptors, and mapping registers.                         */
/*                                                                  */
/*  RETURNS:                                                        */
/*      Number of populated ROM banks.                              */
/*                                                                  */
/*  Note:  When used with the DOWNLOAD/DOWNLOAD.C (flash.li)        */
/*         utility, this function takes no parameters and uses      */
/*         the global FlashDescriptors array.                       */
/*                                                                  */
/*         When used with a generic image, this is a subfunction    */
/*         called RealSetupROMs() which takes a pointer to the      */
/*         global FlashDescriptors array in the case where the      */
/*         image is run out of RAM or takes a pointer to a copy     */
/*         of the FlashDescriptors (with vectors patched) in RAM.   */
/*         The real SetupROMs() function which called this function */
/*         is located at the bottom of this file (order dependent!) */
/*                                                                  */
/********************************************************************/
void RealSetupROMs(FLASH_DESC *pFlashDesc)
{
    bool            FlashFound;
    u_int8          i, BankWidth=0, ChipWidth=0, ROMBank, NumChips=0, cs;
    u_int32         BankSize, RealBankStartAddr, BankStartAddr;
    u_int32         MfrCode=FLASH_CHIP_VENDOR_UNKNOWN, DeviceCode8, DeviceCode16;
    u_int32         FlashType, data, val, div, rd_div;
    LPREG           pPrimary, pSecondary, pMap, pXOEMask;

    pXOEMask = (LPREG) PCI_ROM_XOE_MASK_REG;

    BankStartAddr = RealBankStartAddr = ROM_BASE;

    /****************************************/
    /* If ptr passed in is NULL, use global */
    /****************************************/
    if(!pFlashDesc)
    {
        pFlashDesc = (FLASH_DESC *) FlashDescriptors;
    }

    /*********************************/
    /* Initialize ROM variables.     */   
    /*********************************/
    NumROMBanks = 0;

    /**********************************************/
    /* Calculate ROM bank size for each ROM bank. */
    /**********************************************/
    for (ROMBank = 0; ROMBank < MAX_NUM_FLASH_ROM_BANKS; ROMBank++)
    {
        pPrimary   = (LPREG) PCI_ROM_DESC_BASE + ROMBank;
        pSecondary = (LPREG) PCI_DESC2_BASE    + ROMBank;
        pMap       = (LPREG) PCI_ROM_MAP_BASE  + ROMBank;

        /**********************************/
        /* Initialize flag indicating we  */
        /* found a recognized flash type. */
        /**********************************/
        FlashFound = FALSE;

        /**********************************/
        /* Initialize ROM descriptor and  */
        /* ROM Map registers to values    */
        /* that will allow us to access   */
        /* flash devices and calculate    */
        /* proper values.                 */
        /**********************************/
        cs = (*pPrimary & PCI_ROM_DESC_CS_MASK) >> PCI_ROM_DESC_CS_SHIFT;

        *pPrimary = ((3 << PCI_ROM_DESC_HOLD_TIME_SHIFT)                  |
                     (3 << PCI_ROM_DESC_SETUP_TIME_SHIFT)                 |
                     PCI_ROM_DESC_WIDTH_32                                |
#ifdef DLOAD
                     (ROMBank << PCI_ROM_DESC_CS_SHIFT)                   |
#else
                     (cs << PCI_ROM_DESC_CS_SHIFT)                        |
#endif
                     (15 << PCI_ROM_DESC_BURST_WAIT_SHIFT)                |
                     (20 << PCI_ROM_DESC_READ_WAIT_SHIFT)                 |
                     (20 << PCI_ROM_DESC_WRITE_WAIT_SHIFT)                |
                     PCI_ROM_DESC_TYPE_ROM                                |
                     PCI_ROM_DESC_BURST_DISABLE);


        /*
         * Enable flash program mode
         */
        *pPrimary |= PCI_ROM_DESC_FLASH_PROGRAM_MODE;
        *pMap = ((((~((0x200000>>16)-1)) << PCI_ROM_MAP_SIZE_SHIFT) & PCI_ROM_MAP_SIZE_MASK) |
                 ((BankStartAddr>>16) << PCI_ROM_MAP_OFFSET_SHIFT));
#ifndef DLOAD
        *pSecondary = 0x0;
#endif

        /******************************************************/
        /* Determine chip width, bank width,  and flash type. */
        /******************************************************/
        for (FlashType = (u_int32)0; FlashType < (u_int32)NumFlashTypes; FlashType++)
        {
            /**************************************/
            /* Try setting XOE mask both ways for */
            /* each flash type we try.            */
            /**************************************/
            for (*pXOEMask |= (1 << cs), i = 0; i < 2; *pXOEMask &= ~(1 << cs), i++)
            {
                /*******************************/
                /* Put flash into ID mode and  */
                /* capture mfr and device IDs. */
                /*******************************/
                if ( pFlashDesc[FlashType].EnterIDMode == FLASH_ID_ENTER_INTEL )
                {
                    *((volatile u_int32 *)(BankStartAddr)) = 0x90909090;
                }
                else if ( pFlashDesc[FlashType].EnterIDMode == FLASH_ID_ENTER_AMDX8 )
                {
                    *((volatile u_int32 *)(BankStartAddr + (0xaaa<<2))) = 0xaaaaaaaa;
                    *((volatile u_int32 *)(BankStartAddr + (0x555<<2))) = 0x55555555;
                    *((volatile u_int32 *)(BankStartAddr + (0xaaa<<2))) = 0x90909090;
                }
                else if ( pFlashDesc[FlashType].EnterIDMode == FLASH_ID_ENTER_AMDX16 )
                {
                    *((volatile u_int32 *)(BankStartAddr + (0x555<<2))) = 0xaaaaaaaa;
                    *((volatile u_int32 *)(BankStartAddr + (0x2aa<<2))) = 0x55555555;
                    *((volatile u_int32 *)(BankStartAddr + (0x555<<2))) = 0x90909090;
                }
                else if ( pFlashDesc[FlashType].EnterIDMode == FLASH_ID_ENTER_ATMELX16 )
                {
                    *((volatile u_int32 *)(BankStartAddr + (0x5555<<2))) = 0xaaaaaaaa;
                    *((volatile u_int32 *)(BankStartAddr + (0x2aaa<<2))) = 0x55555555;
                    *((volatile u_int32 *)(BankStartAddr + (0x5555<<2))) = 0x90909090;
                }

                MfrCode = *((volatile u_int8 *)BankStartAddr);
                DeviceCode8 = *((volatile u_int32 *)(BankStartAddr + 8));
                DeviceCode16 = *((volatile u_int32 *)(BankStartAddr + 4));
                /* DeviceCode32 = *((volatile u_int32 *)(BankStartAddr + ??? )); */
                if ( pFlashDesc[FlashType].ExitIDMode == FLASH_ID_EXIT_INTEL )
                {
                    *(volatile u_int32 *)(BankStartAddr) = 0xffffffff;
                }
                else if ( pFlashDesc[FlashType].ExitIDMode == FLASH_ID_EXIT_AMD )
                {
                    *(volatile u_int32 *)(BankStartAddr) = 0xf0f0f0f0;
                }
                
                /****************************/
                /* Did we find a flash type */
                /* we recognize?            */
                /****************************/
                if (MfrCode == pFlashDesc[FlashType].MfrID)
                {
                    /*
                     * NOTE: Order is important! Must try largest chip sizes first.
                     */
                    
                    /***************************/
                    /* How wide is the device? */
                    /***************************/
                    if ((u_int16)DeviceCode16 == (u_int16)pFlashDesc[FlashType].DeviceID)
                    {
                        FlashFound = TRUE;
                        
                        /*********************************/
                        /* How many devices in the bank? */
                        /*********************************/
                        if ((u_int16)(DeviceCode16 >> 16) ==
                            (u_int16)pFlashDesc[FlashType].DeviceID)
                        {
                            BankWidth = 32;
                        }
                        else
                        {
                            BankWidth = 16;    
                        }
                        ChipWidth = 16;
                        NumChips = BankWidth >> 4;
                        break;
                    }
                    else if ((u_int8)DeviceCode8 == (u_int8)pFlashDesc[FlashType].DeviceID)
                    {
                        FlashFound = TRUE;

                        /*********************************/
                        /* How many devices in the bank? */
                        /*********************************/
                        if ((u_int8)(DeviceCode8 >> 16) ==
                            (u_int8)pFlashDesc[FlashType].DeviceID)
                        {
                            BankWidth = 32;
                        }
                        else if ((u_int8)(DeviceCode8 >> 8) ==
                                 (u_int8)pFlashDesc[FlashType].DeviceID)
                        {
                            BankWidth = 16;
                        }
                        else
                        {
                            BankWidth = 8;    
                        }
                        ChipWidth = 8;
                        NumChips = BankWidth >> 3;
                        break;
                    }
                }
            }
            if (FlashFound)
            {
                break;
            }
        }
        
        /******************************/
        /* Keep track of how many ROM */
        /* banks are populated.       */
        /******************************/
        if (FlashFound)
	     {
            NumROMBanks++;

            /***********************************/
            /* Calculate bank size and program */
            /* ROM size register.              */
            /***********************************/
            BankSize = (NumChips) * (pFlashDesc[FlashType].ChipSize >> 3);
            *pMap = ((((~((BankSize>>16)-1)) << PCI_ROM_MAP_SIZE_SHIFT) & PCI_ROM_MAP_SIZE_MASK) |
                     ((RealBankStartAddr>>16) << PCI_ROM_MAP_OFFSET_SHIFT));
            RealBankStartAddr += BankSize;
            BankStartAddr = RealBankStartAddr;
            if(RealBankStartAddr & 0x003FFFFF)
            {
                BankStartAddr &= 0xFFC00000;
                BankStartAddr += 0x00400000;
            }
    
            /***********************************/
            /* Calculate the Read & Write      */
            /* timings for the secondary       */
            /* descriptor                      */
            /***********************************/
            Bank[ROMBank].ReadTimings  = 0x0;

            if(pFlashDesc[FlashType].ReadCSSetup)
            {
                data = pFlashDesc[FlashType].ReadCSSetup * 100;
                val = div = 0;
                while(val < data)
                {
                    ++div;
                    val += MemClkPeriod32;
                }
                Bank[ROMBank].ReadTimings |= ((div&0x7f)<<17);
            }

            if(pFlashDesc[FlashType].ReadCSHold)
            {
                data = pFlashDesc[FlashType].ReadCSHold * 100;
                val = div = 0;
                while(val < data)
                {
                    ++div;
                    val += MemClkPeriod32;
                }
                Bank[ROMBank].ReadTimings |= ((div&0x1f)<<12);
            }

            if(pFlashDesc[FlashType].ReadCtrlSetup)
            {
                data = pFlashDesc[FlashType].ReadCtrlSetup * 100;
                val = div = 0;
                while(val < data)
                {
                    ++div;
                    val += MemClkPeriod32;
                }
                Bank[ROMBank].ReadTimings |= ((div&0x7f)<<5);
            }

            if(pFlashDesc[FlashType].ReadAddrHold)
            {
                data = pFlashDesc[FlashType].ReadAddrHold * 100;
                val = div = 0;
                while(val < data)
                {
                    ++div;
                    val += MemClkPeriod32;
                }
                Bank[ROMBank].ReadTimings |= ((div&0x1f)<<0);
            }

            Bank[ROMBank].WriteTimings = 0x0;

            if(pFlashDesc[FlashType].WriteCSSetup)
            {
                data = pFlashDesc[FlashType].WriteCSSetup * 100;
                val = div = 0;
                while(val < data)
                {
                    ++div;
                    val += MemClkPeriod32;
                }
                Bank[ROMBank].WriteTimings |= ((div&0x7f)<<17);
            }

            if(pFlashDesc[FlashType].WriteCSHold)
            {
                data = pFlashDesc[FlashType].WriteCSHold * 100;
                val = div = 0;
                while(val < data)
                {
                    ++div;
                    val += MemClkPeriod32;
                }
                Bank[ROMBank].WriteTimings |= ((div&0x1f)<<12);
            }

            if(pFlashDesc[FlashType].WriteCtrlSetup)
            {
                data = pFlashDesc[FlashType].WriteCtrlSetup * 100;
                val = div = 0;
                while(val < data)
                {
                    ++div;
                    val += MemClkPeriod32;
                }
                Bank[ROMBank].WriteTimings |= ((div&0x7f)<<5);
            }

            if(pFlashDesc[FlashType].WriteAddrHold)
            {
                data = pFlashDesc[FlashType].WriteAddrHold * 100;
                val = div = 0;
                while(val < data)
                {
                    ++div;
                    val += MemClkPeriod32;
                }
                Bank[ROMBank].WriteTimings |= ((div&0x1f)<<0);
            }

            /***********************************/
            /* Program ROM descriptor register */
            /* with proper settings for this   */
            /* flash type.                     */
            /***********************************/

            /*
             * Always use secondary descriptor if setup/hold values req'd
             */
            *pPrimary &= ~(PCI_ROM_DESC_SETUP_TIME_MASK | PCI_ROM_DESC_HOLD_TIME_MASK);

            /*
             * Set secondary descriptor for reads
             */
            *pSecondary = Bank[ROMBank].ReadTimings;

            if ( BankWidth == 8 )
            {
                *pPrimary = ((*pPrimary & ~PCI_ROM_DESC_WIDTH_MASK) | PCI_ROM_DESC_WIDTH_8);
            }
            else if ( BankWidth == 16 )
            {
                *pPrimary = ((*pPrimary & ~PCI_ROM_DESC_WIDTH_MASK) | PCI_ROM_DESC_WIDTH_16);
            }
            else if ( BankWidth == 32 )
            {
                *pPrimary = ((*pPrimary & ~PCI_ROM_DESC_WIDTH_MASK) | PCI_ROM_DESC_WIDTH_32);
            }

            /*
             * Set ReadAccess time from table
             */
            data = FlashDescriptors[FlashType].ReadAccess * 100 + 500;
            val = div = 0;
            while(val < data)
            {
                ++div;
                val += MemClkPeriod32;
            }
            *pPrimary = ((*pPrimary & ~PCI_ROM_DESC_READ_WAIT_MASK) |
                         (div << PCI_ROM_DESC_READ_WAIT_SHIFT));
            rd_div = div;

            /*
             * Set WriteAccess time from table
             */
            data = FlashDescriptors[FlashType].WriteAccess * 100;
            val = div = 0;
            while(val < data)
            {
                ++div;
                val += MemClkPeriod32;
            }
            *pPrimary = ((*pPrimary & ~PCI_ROM_DESC_WRITE_WAIT_MASK) |
                         (div << PCI_ROM_DESC_WRITE_WAIT_SHIFT));

            Bank[ROMBank].WriteModeTimings = (*pPrimary & PCI_ROM_DESC_WRITE_WAIT_MASK) >>
                                                          PCI_ROM_DESC_WRITE_WAIT_SHIFT;
            if(MfrCode == FLASH_CHIP_VENDOR_INTEL)
            {
                /*
                 * Fix for Intel flash leaving READ_ARRAY
                 * mode with data pattern match on writes
                 */
                *pPrimary &= ~PCI_ROM_DESC_WRITE_WAIT_MASK;
            }

            /*
             * Set bursting if supported by the flash ROM
             */
            if(FlashDescriptors[FlashType].Flags & FLASH_FLAGS_BURST_SUPPORTED)
            {
                /*
                 * Set BurstAccess time
                 */
                data = FlashDescriptors[FlashType].BurstTime * 100;
                val = div = 0;
                while(val < data)
                {
                    ++div;
                    val += MemClkPeriod32;
                }
                *pPrimary = ((*pPrimary & ~PCI_ROM_DESC_BURST_WAIT_MASK) |
                             (div << PCI_ROM_DESC_BURST_WAIT_SHIFT));
                *pPrimary |= PCI_ROM_DESC_BURST_ENABLE;

                *((LPREG)PCI_PAGE_BOUNDARY_REG) =
                             (((u_int32)FlashDescriptors[FlashType].PageSize-4) <<
                             PCI_PAGE_BOUNDARY_PAGE_SIZE_SHIFT);
            }
            else
            {
                /*
                 * Irrelevant
                 */
                *pPrimary = ((*pPrimary & ~PCI_ROM_DESC_BURST_WAIT_MASK) |
                             (1 << PCI_ROM_DESC_BURST_WAIT_SHIFT));
                *pPrimary &= ~PCI_ROM_DESC_BURST_ENABLE;
            }

            /*
             * Disable flash program mode
             */
            *pPrimary &= ~PCI_ROM_DESC_FLASH_PROGRAM_MODE;

            /**************************************************/
            /* Store flash info for this bank so that our API */
            /* clients can get it later.                      */
            /* Ensure the actual global FlashDescriptor array */
            /* is used here, not the copy!                    */
            /**************************************************/
            Bank[ROMBank].pFlashDesc = (LPFLASH_DESC) &FlashDescriptors[FlashType];
            Bank[ROMBank].BankWidth = BankWidth;
            Bank[ROMBank].ChipWidth = ChipWidth;        
	}
        else
	{
            /*
             * Disable flash program mode
             */
            *pPrimary &= ~PCI_ROM_DESC_FLASH_PROGRAM_MODE;

	    /*
             * Must be the last bank, quit
             */
	    break;
	}
    }
}

#endif

/****************************************************************************
 * Modifications:
 * $Log: 
 *  48   mpeg      1.47        10/21/03 9:48:41 AM    Larry Wang      CR(s): 
 *        7673 Replace switch()... with if()... else if()... to avoid absolute 
 *        address branch generated by GCC.
 *  47   mpeg      1.46        9/2/03 6:58:10 PM      Joe Kroesche    SCR(s) 
 *        7415 :
 *        removed unneeded header files that were causing PSOS warnings
 *        
 *  46   mpeg      1.45        5/14/03 4:36:20 PM     Tim White       SCR(s) 
 *        6346 6347 :
 *        Use pll.c (CalcMemClkPeriod) for calculating MemClkPeriod & 
 *        MemClkPeriod32.
 *        
 *        
 *  45   mpeg      1.44        2/21/03 9:04:34 AM     Matt Korte      SCR(s) 
 *        5538 :
 *        Fixed warning
 *        
 *  44   mpeg      1.43        2/13/03 12:20:48 PM    Matt Korte      SCR(s) 
 *        5479 :
 *        Removed old header reference
 *        
 *  43   mpeg      1.42        12/12/02 5:28:20 PM    Tim White       SCR(s) 
 *        5157 :
 *        No longer need to turn ROM prefetch on and off according to E.D.
 *        
 *        
 *  42   mpeg      1.41        9/3/02 7:46:08 PM      Matt Korte      SCR(s) 
 *        4498 :
 *        Remove warnings
 *        
 *  41   mpeg      1.40        8/22/02 4:12:10 PM     Larry Wang      SCR(s) 
 *        4459 :
 *        Do not touch bit 1 of PCI_ROM_MODE_REG for Wabash Rev B.
 *        
 *  40   mpeg      1.39        4/29/02 9:51:46 AM     Bobby Bradford  SCR(s) 
 *        3580 :
 *        Removed all functions from this file, except for RealSetupROMs().
 *        Embedded the Enter/Exit ID mode functionality inline (using a 
 *        switch/case statement) rather than using the function pointer.
 *        Now, only this function has to be copied to RAM during the
 *        STARTUP detection of FLASH type.
 *        
 *  39   mpeg      1.38        4/10/02 5:11:22 PM     Tim White       SCR(s) 
 *        3509 :
 *        Eradicate external bus (PCI) bitfield usage.
 *        
 *        
 *  38   mpeg      1.37        4/5/02 11:51:46 AM     Tim White       SCR(s) 
 *        3510 :
 *        Backout DCS #3468
 *        
 *        
 *  37   mpeg      1.36        3/28/02 2:50:16 PM     Quillian Rutherford 
 *        SCR(s) 3468 :
 *        Removed bit fields
 *        
 *        
 *  36   mpeg      1.35        6/22/01 3:12:08 PM     Tim White       SCR(s) 
 *        2150 2155 :
 *        > Set WriteWait to 0 in order to force Intel Flash ROM parts
 *        to ignore all writes when it's in READ_MODE.
 *        
 *        
 *  35   mpeg      1.34        4/12/01 7:26:46 PM     Amy Pratt       DCS914 
 *        Removed Neches support.
 *        
 *  34   mpeg      1.33        2/23/01 3:27:26 PM     Tim White       DCS#1296:
 *         Fix XOE settings and ROM timings for Vendor_A.
 *        
 *  33   mpeg      1.32        1/31/01 2:30:28 PM     Tim White       DCS#0988:
 *         Reclaim footprint space.
 *        
 *  32   mpeg      1.31        12/14/00 2:34:02 PM    Tim White       Ensure 
 *        XOE setting works off ~cs not descriptor!!!!
 *        Also allow download tool (flash.li) to override (i.e. set) the ~cs
 *        (ChipSel) value in the descritor.
 *        
 *  31   mpeg      1.30        12/12/00 3:55:22 PM    Tim White       Removed 
 *        hackorama for Vendor_A (previous change).
 *        
 *  30   mpeg      1.29        12/12/00 1:42:42 AM    Tim White       Hackorama
 *         for Vendor_A.
 *        
 *  29   mpeg      1.28        12/8/00 3:02:12 PM     Tim White       Do not 
 *        set the chipselect values since the minidriver could already
 *        have them reversed for switch banked load.... OTV 1.2
 *        
 *  28   mpeg      1.27        10/2/00 12:20:46 PM    Tim White       FIxed 
 *        minor VxWorks build problem.
 *        
 *  27   mpeg      1.26        9/26/00 6:09:12 PM     Tim White       Added 
 *        switchable secondary descriptor capability to low-level flash 
 *        function.
 *        
 *  26   mpeg      1.25        8/30/00 11:55:20 AM    Tim White       Fixed 
 *        VxWorks compilation warnings.
 *        
 *  25   mpeg      1.24        7/18/00 3:43:22 PM     Tim White       Fixed 
 *        problem dealing with SST flash with second bank.  The problem
 *        stems from the fact that when using a 2MB offset base (BankStartAddr)
 *        for detecting the second bank along with a size of 2MB, what appears
 *        to be happening is that the bit (0x200000) is somehow shifted into 
 *        the
 *        mask before the comparison is done which ends up as A19 which goes
 *        to the wrong address in the SST parts since they care about A19.  
 *        This
 *        is wrong behavior but is happening.  The solution is to only look at
 *        flash ID'
 *        s on a 4MB boundary then set the base correctly after detection.  
 *        This
 *        needs to be looked at by hardware.
 *        
 *  24   mpeg      1.23        6/20/00 9:38:48 AM     Ray Mack        fix 
 *        warning
 *        
 *  23   mpeg      1.22        6/12/00 4:13:22 PM     Tim White       Fixed 
 *        minor compiler warnings.
 *        
 *  22   mpeg      1.21        5/25/00 6:11:54 PM     Tim White       Fixed 
 *        problem where on Neches chips you can't write to the
 *        Secondary descriptors since the address where they live on
 *        Colorado is mapped over the primary descriptor space on
 *        Neches.  The run-time chip independent download tool which
 *        has CHIP_NAME=COLORADO would not operate on a Neches because
 *        of this.
 *        
 *  21   mpeg      1.20        5/17/00 4:44:14 PM     Tim White       Fixed 
 *        build break with CN8600 from previous fix.
 *        
 *  20   mpeg      1.19        5/17/00 1:50:46 PM     Tim White       Ensure 
 *        the secondary descriptors are zero'ed due to Colorado HW bug.
 *        
 *  19   mpeg      1.18        5/17/00 12:20:58 PM    Tim White       Fixed 
 *        problem with SST ROM's.
 *        
 *  18   mpeg      1.17        5/1/00 11:00:18 PM     Tim White       Added 
 *        bringup #if's for Vendor_B.
 *        
 *  17   mpeg      1.16        5/1/00 7:11:12 PM      Senthil Veluswamy Moved 
 *        original declaration of MemClkPeriod32 here.
 *        
 *  16   mpeg      1.15        4/30/00 4:08:12 PM     Ray Mack        changes 
 *        to get it to compile for Gnu
 *        
 *  15   mpeg      1.14        4/28/00 2:01:26 PM     Tim White       Fixed the
 *         timing calculations and allow it to function for
 *        any of the three (3) chips currently supported:  Colorado-A,
 *        Neches-B, and Neches-A so that it will work in the download
 *        utility.
 *        
 *  14   mpeg      1.13        4/27/00 4:20:26 PM     Tim White       Fixed bug
 *         in board definitions.
 *        
 *  13   mpeg      1.12        4/24/00 11:21:12 AM    Tim White       Moved 
 *        strapping CONFIG0 from chip header file(s) to board header file(s).
 *        
 *  12   mpeg      1.11        4/12/00 3:52:26 PM     Dave Wilson     Changed 
 *        PLL_CONFIG0 register accesses to use new union type
 *        
 *  11   mpeg      1.10        4/10/00 6:21:52 PM     Tim White       Added the
 *         correct Write (program) values, added several new Flash ROM types.
 *        
 *  10   mpeg      1.9         3/14/00 3:15:04 PM     Tim White       Fixed 
 *        typo from previous rev...
 *        
 *  9    mpeg      1.8         3/14/00 3:08:48 PM     Tim White       Removed 
 *        the global set ROM in writemode from non-Neches chips.
 *        
 *  8    mpeg      1.7         3/8/00 5:20:12 PM      Tim White       
 *        Restructured the BANK_INFO array and added support for new Intel 
 *        Flash ROM.
 *        
 *  7    mpeg      1.6         3/2/00 6:07:26 PM      Tim Ross        Changed 
 *        OS switch to RTOS switch to avoid conflict w/ Windows NT
 *        build environment.
 *        
 *        Added support for per-descriptor flash program bits.
 *        
 *  6    mpeg      1.5         1/25/00 2:40:34 PM     Tim White       
 *        CSWriteDelay is gone, remove it.
 *        
 *  5    mpeg      1.4         11/18/99 8:10:14 PM    Dave Wilson     Added 
 *        special case for vxWorks which prevents it's brain-damaged compiler
 *        from generating byte accesses to registers
 *        
 *  4    mpeg      1.3         10/22/99 5:30:34 PM    Tim White       Added 
 *        runtime check for burst enable (or not enable rather for Neches-A).
 *        
 *  3    mpeg      1.2         10/20/99 3:32:34 PM    Tim White       Use the 
 *        real FlashDescriptorsp[] array instead of the temporary copy
 *        for the ROMInfo setup array.
 *        
 *  2    mpeg      1.1         10/13/99 6:10:56 PM    Tim White       Remove 
 *        calls to Checkpoint().
 *        
 *  1    mpeg      1.0         10/13/99 4:58:26 PM    Tim White       
 * $
 ****************************************************************************/

