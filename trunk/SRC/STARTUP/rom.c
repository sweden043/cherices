/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                   Conexant Systems Inc. (c) 1999-2003                    */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        rom.c
 *
 *
 * Description:     ROM descriptor setup and flash info gathering.
 *
 *
 * Author:          Tim Ross
 *
 ****************************************************************************/
/* $Id: rom.c,v 1.50, 2004-04-25 00:57:48Z, Sunil Cheruvu$
 ****************************************************************************/

/******************/
/* Include Files  */
/******************/
#include <string.h>

#include "stbcfg.h"
#include "basetype.h"
#include "board.h"
#include "startup.h"
#include "hwlib.h"
#ifdef DLOAD
 #include "defs.h"
#endif
#include "mmu.h"

#if !defined(DLOAD) && IMAGE_TYPE != BOOT && IMAGE_TYPE != BOOTEXT
#if (CPU_TYPE==CPU_ARM920T) || (CPU_TYPE==AUTOSENSE)
 extern unsigned int mmuModifyROMCacheability (unsigned int fbank,
                                               unsigned int on_off,
                                               unsigned int addr,
                                               unsigned int num_of_megs );
#endif /* (CPU_TYPE==CPU_ARM920T) || (CPU_TYPE==AUTOSENSE) */
#endif /* !defined(DLOAD) && IMAGE_TYPE != BOOT && IMAGE_TYPE != BOOTEXT */

/**********************/
/* Local Definitions  */
/**********************/
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#define REAL_SETUP_ROMS_COPY_CODE_SIZE  16384       /* 16 K ... current size is about 3K */

#if !defined(DLOAD) && IMAGE_TYPE != BOOT && IMAGE_TYPE != BOOTEXT
bool FlashDisableDCache( u_int32 fbank, unsigned int addr, unsigned int num_of_megs );
void FlashEnableDCache( u_int32 fbank, unsigned int addr, unsigned int num_of_megs );
#endif /* IMAGE_TYPE != BOOT && IMAGE_TYPE != BOOTEXT */

typedef void (*FNPTR) ( void *data );

/***************/
/* Global Data */
/***************/
u_int8 NumROMBanks;
u_int32 UniformBlockSize;
BANK_INFO Bank[MAX_NUM_FLASH_ROM_BANKS];
/* Used to track the presence of a Needmore with NandFlash chip on it*/
bool  gbNeedmoreHasNandFlash = FALSE;

extern u_int32  gboard_type;
extern u_int32  gboard_rev;

/*****************/
/* External Data */
/*****************/
extern FLASH_DESC FlashDescriptors[];
u_int16 NandFlashDeviceID = 0xABCD;
extern NAND_FLASH_DESC NandFlashDescriptors[];
void SetupNandFlash(void);
u_int32 CNXT_NANDFLASH_READY_BUSY_GPIO;
u_int32 CNXT_NANDFLASH_CHIP_ENABLE_GPIO;
u_int32 CNXT_NANDFLASH_CHIP_SELECT;
extern ISA_DEVICE ISADevices[];
extern void RealSetupROMs(FLASH_DESC *pFlashDesc);
extern void FFillBytes(void *dest, int value, size_t count);

#if (RTOS==PSOS25) ||(RTOS==PSOS) || (RTOS==NUP) || (RTOS == UCOS) || (RTOS == UCOS2) || (RTOS==NOOS)
extern u_int32 FreeMemStart;
#else
extern u_int32 end;
#endif /* RTOS */


/********************************************************************/
/*  SetupROMs                                                       */
/*                                                                  */
/*  PARAMETERS:                                                     */
/*      None.                                                       */
/*                                                                  */
/*  DESCRIPTION:                                                    */
/*      When running from RAM, simply calls the RealSetupROMs()     */
/*      function at the top of the file.                            */
/*                                                                  */
/*      When running from ROM, copies the above functions in this   */
/*      file starting with RealSetupROMs() to R/W memory (RAM)      */
/*      starting at the Image$$ZI$$Limit (end) address + 4          */
/*      bytes.  Then it makes a copy of the global FLashDescriptors */
/*      array in RAM and modifies the flash ROM Enter/Exit function */
/*      vectors to point to the copy of the above functions.        */
/*      Lastly it calls an assembly function in STARTUP/STARTUP.S   */
/*      which simply makes the "long" bl to RealSetupROMs() in RAM. */
/*                                                                  */
/*      All this ensures that any image whether it's running out of */
/*      RAM or ROM can successfully obtain the ROM information      */
/*      which forces the flash ROM chips into ID mode.  Once in ID  */
/*      mode, the ROM memory is not accessible.                     */
/*                                                                  */
/*      This could have been done using the I-Cache lockdown        */
/*      feature but the code required to do that is just as nasty   */
/*      since the function setting up the I-Cache must be running   */
/*      from non-cached memory and the area being setup must be     */
/*      under I-Cache control with the I-Cache enabled.  A separate */
/*      temporary Data Protection Region would need to be setup     */
/*      in order to override the default ROM I-Cache setting and    */
/*      this area where the setup function would live needs to be at*/
/*      least 4K away from the function(s) being locked down.       */
/*      This forces some rather nasty ordering of .obj files in     */
/*      the link step of the build process (i.e. nasty).            */
/*                                                                  */
/*  RETURNS:                                                        */
/*      Nothing                                                     */
/*                                                                  */
/********************************************************************/
void SetupROMs(void)
{
  int       BankNum, NumChips, BytesPerChip;

#if EMULATION_LEVEL == FINAL_HARDWARE
#if !defined(DLOAD)

  u_int8    *code_area, *data_area, *src, *dst;
  u_int32   code_size, i;
  FNPTR     setup_rom_fnptr;

  FFillBytes(Bank, 0, sizeof(Bank));
  /*
   * Check if being run out of ROM
   */
  if((u_int32)&SetupROMs > ROM_START_ADDRESS)
  {

    /*
     * We're running out of ROM, perform the copy
     * Copy RealSetupROMs() function to RAM
     */
    code_size = REAL_SETUP_ROMS_COPY_CODE_SIZE;
#if (RTOS==PSOS25) || (RTOS==PSOS) || (RTOS==NUP) || (RTOS == UCOS) || (RTOS == UCOS2)  || (RTOS == NOOS)
    code_area = dst = (u_int8 *)((FreeMemStart + 7) & (~(0x3)));
#else
    code_area = dst = (u_int8 *)((u_int32)((u_int32)&end + 7) & (~(0x3)));
#endif /* RTOS */
    src = (u_int8 *)((u_int32)&RealSetupROMs & ~0x1);
    for(i=0;i<code_size;i++)
    {
       *dst++ = *src++;
    }

    /*
     * Copy FlashDescriptors to RAM (make a copy at least)
     */
    data_area = dst = (u_int8 *)((u_int32)(code_area + code_size + 7) & (~(0x3)));
    src = (u_int8 *)FlashDescriptors;
    while((*(src+8)!=0xFF)&&(*(src+12)!=0xFF))
    {
       for(i=0;i<sizeof(FLASH_DESC);i++)
       {
          *dst++ = *src++;
       }
    }

    /*
     * Invalidate the I cache to be sure the code is picked up.
     */
    FlushICache();

    /*
     * Call the real deal...
     */
#if ARM_TOOLKIT == ADS || ARM_TOOLKIT == SDT
    setup_rom_fnptr = (FNPTR)((u_int32)code_area | 1);   /* Thumb Execution */
#else
    setup_rom_fnptr = (FNPTR)code_area;                  /* ARM Execution */
#endif
    setup_rom_fnptr ( (u_int8 *)data_area);
  }
  else
#endif /* !defined(DLOAD) */
  {

    /*
     * Running out of RAM already, so we're safe, just call the function
     */
    RealSetupROMs(FlashDescriptors);
  }

   /* Setup the Nand Flash devices. (Inculdes teh ISA descriptor setup and 
   bank population */
   SetupNandFlash();

   /*
    * Setup PCI_ROM_MODE_REG
    */
   *((LPREG) PCI_ROM_MODE_REG) |= PCI_ROM_PREFETCH_ENABLE;

#ifdef PCI_ROM_MODE_SUPPORTS_PCI_SYNC_BITS
   /*
    * On newer chips (Wabash and beyond), need to set the PCI sync bits
    */
   *((LPREG)PCI_ROM_MODE_REG) |= (( 2UL << PCI_ROM_REQ0_SYNC_SHIFT ) |
                                  ( 2UL << PCI_ROM_REQ1_SYNC_SHIFT ) |
                                  ( 2UL << PCI_ROM_REQ2_SYNC_SHIFT ));
#endif /* PCI_ROM_MODE_SUPPORTS_PCI_SYNC_BITS */

#else /* EMULATION_LEVEL != FINAL_HARDWARE (i.e. emulation) */
  {
    /*
     * Hardcode for emulation since there's no real flash ROM out there
     */
    NumROMBanks = 2;
    Bank[0].pFlashDesc = Bank[1].pFlashDesc = (LPFLASH_DESC) &FlashDescriptors[0];
    Bank[0].BankWidth = Bank[1].BankWidth = 0x10;
    Bank[0].ChipWidth = Bank[1].ChipWidth = 0x10;
    Bank[0].ReadTimings = Bank[1].ReadTimings = 0x00;
    Bank[0].WriteTimings = Bank[1].WriteTimings = 0x80;
    Bank[0].WriteModeTimings = Bank[1].WriteModeTimings = 0;
    *((LPREG)PCI_ROM_DESC0_REG)=(*((LPREG)PCI_ROM_DESC0_REG)&0x700)|0x090C1020;
    *((LPREG)PCI_ROM_DESC1_REG)=(*((LPREG)PCI_ROM_DESC1_REG)&0x700)|0x090C1020;
    *((LPREG)PCI_ROM0_MAP_REG)=0xFE0;
    *((LPREG)PCI_ROM1_MAP_REG)=0x200FE0;
    *((LPREG)PCI_ROM_MODE_REG)=0x2;
    *((LPREG)PCI_ROM_XOE_MASK_REG)|=0x3;
    *((LPREG)PCI_PAGE_BOUNDARY_REG)=0x0;
    *((LPREG)PCI_ROM_DESC0_REG2)=0x0;
    *((LPREG)PCI_ROM_DESC1_REG2)=0x0;
  }
#endif

  /*
   * Fill in the array of BANK_INFO elements which
   * describes the ROM banks.
   */
  for (BankNum = 0; BankNum < MAX_NUM_FLASH_ROM_BANKS; BankNum++)
  {

    /* 
     * If the bank is empty, we need to fill in the info as such.   
     */
    if (BankNum < NumROMBanks)
    {
      Bank[BankNum].ROMType = ROM_TYPE_FLASH;
      BytesPerChip = Bank[BankNum].pFlashDesc->ChipSize / 8;
      NumChips = Bank[BankNum].BankWidth / Bank[BankNum].ChipWidth;
      Bank[BankNum].BankSize = BytesPerChip * NumChips;
      if (BankNum == 0)
      {
        Bank[BankNum].BankBase = ROM_START_ADDRESS;
      }
      else
      {
        Bank[BankNum].BankBase = Bank[BankNum-1].BankBase +
                 Bank[BankNum-1].BankSize;
      }
      Bank[BankNum].Flags = Bank[BankNum].pFlashDesc->Flags;

      /*
       * Calculate the interface information which uses a uniform block size
       * and eliminates the boot block area.
       */
      if(Bank[0].Flags & FLASH_FLAGS_UNIFORM_SECTOR_SIZE)
      {
          UniformBlockSize = ((u_int32)Bank[0].pFlashDesc->ChipSectorSize<<12)*NumChips;
      }
      else if(Bank[0].Flags & FLASH_FLAGS_TOP_BOOT_BLOCK_LAYOUT)
      {
          UniformBlockSize = ((u_int32)Bank[0].pFlashDesc->ChipSectorSize[0]<<12)*NumChips;
      }
      else
      {
         UniformBlockSize = ((u_int32)Bank[0].pFlashDesc->ChipSectorSize[Bank[0].pFlashDesc->ChipNumSectors-1]<<12)*NumChips;
      }
    }
    else
    {

      /*
       * The bank is empty.
       */
      Bank[BankNum].ROMType = ROM_TYPE_EMPTY;
      Bank[BankNum].BankSize = 0;
      Bank[BankNum].BankBase = Bank[BankNum-1].BankBase;
      Bank[BankNum].Flags = 0;
      Bank[BankNum].ReadTimings = 0;
      Bank[BankNum].WriteTimings = 0;
      Bank[BankNum].WriteModeTimings = 0;
    }
  }
}


/********************************************************************/
/*  FlashDisableDCache                                              */
/*                                                                  */
/*  PARAMETERS:                                                     */
/*      fbank       - the flash bank                                */
/*      addr        - the address of the area to disable            */
/*      num_of_megs - the size of the area to disable               */
/*                                                                  */
/*  DESCRIPTION:                                                    */
/*      Based on what CPU is used, call DisableDCacheForRegion or   */
/*      mmuModifyROMCacheability.                                   */
/*                                                                  */
/*  RETURNS:                                                        */
/*      The previous state of caching for the specified region,     */
/*      TRUE==was enabled, FALSE==was disabled.                     */
/*                                                                  */
/********************************************************************/
bool FlashDisableDCache( u_int32 fbank, unsigned int addr, unsigned int num_of_megs )
{
    bool retval = FALSE;
#if !defined(DLOAD) && IMAGE_TYPE != BOOT && IMAGE_TYPE != BOOTEXT
    unsigned int state;

    /*
     * If the protection unit is enabled, disable the D cache
     */
    state = mmuGetState();
    if(state & 0x1)
    {

      #if (CPU_TYPE==CPU_ARM940T) || (CPU_TYPE==AUTOSENSE)

        #if CPU_TYPE == AUTOSENSE
            if ( GetCPUType() == CPU_ARM940T )
            {
        #endif

                retval = DisableDCacheForRegion( (fbank+3) );

        #if CPU_TYPE == AUTOSENSE
            }
        #endif

      #endif /* (CPU_TYPE==CPU_ARM940T) || (CPU_TYPE==AUTOSENSE) */

      #if (CPU_TYPE==CPU_ARM920T) || (CPU_TYPE==AUTOSENSE)

        #if CPU_TYPE == AUTOSENSE
            if ( GetCPUType() == CPU_ARM920T )
            {
        #endif

                retval = mmuModifyROMCacheability( fbank, CACHEING_OFF, addr, num_of_megs );

        #if CPU_TYPE == AUTOSENSE
            }
        #endif

      #endif /* (CPU_TYPE==CPU_ARM920T) || (CPU_TYPE==AUTOSENSE) */

    }

#endif /* !defined(DLOAD) && IMAGE_TYPE != BOOT && IMAGE_TYPE != BOOTEXT */

    return retval;
}


/********************************************************************/
/*  FlashEnableDCache                                               */
/*                                                                  */
/*  PARAMETERS:                                                     */
/*      fbank       - the flash bank                                */
/*      addr        - the address of the area to enable             */
/*      num_of_megs - the size of the area to enable                */
/*                                                                  */
/*  DESCRIPTION:                                                    */
/*      Based on what CPU is used, call EnableDCacheForRegion or    */
/*      mmuModifyROMCacheability.                                   */
/*                                                                  */
/*  RETURNS:                                                        */
/*      Nothing                                                     */
/*                                                                  */
/********************************************************************/
void FlashEnableDCache( u_int32 fbank, unsigned int addr, unsigned int num_of_megs )
{
#if !defined(DLOAD) && IMAGE_TYPE != BOOT && IMAGE_TYPE != BOOTEXT
    unsigned int state;
    
    /*
     * If the protection unit is enabled, disable the D cache
     */
    state = mmuGetState();
    if(state & 0x1)
    {

      #if (CPU_TYPE==CPU_ARM940T) || (CPU_TYPE==AUTOSENSE)

        #if CPU_TYPE == AUTOSENSE
            if ( GetCPUType() == CPU_ARM940T )
            {
        #endif

                EnableDCacheForRegion( (fbank+3) );

        #if CPU_TYPE == AUTOSENSE
            }
        #endif

      #endif /* (CPU_TYPE==CPU_ARM940T) || (CPU_TYPE==AUTOSENSE) */

      #if (CPU_TYPE==CPU_ARM920T) || (CPU_TYPE==AUTOSENSE)

        #if CPU_TYPE == AUTOSENSE
            if ( GetCPUType() == CPU_ARM920T )
            {
        #endif

                (void)mmuModifyROMCacheability( fbank, CACHEING_ON, addr, num_of_megs );

        #if CPU_TYPE == AUTOSENSE
            }
        #endif

      #endif /* (CPU_TYPE==CPU_ARM920T) || (CPU_TYPE==AUTOSENSE) */

    }

#endif /* !defined(DLOAD) && IMAGE_TYPE != BOOT && IMAGE_TYPE != BOOTEXT */

}


#if !defined(DLOAD) && (RTOS != VXWORKS) && IMAGE_TYPE != BOOT && IMAGE_TYPE != BOOTEXT
/********************************************************************/
/*  SetupROMMemoryRegions                                           */
/*                                                                  */
/*  DESCRIPTION:                                                    */
/*      Setup memory regions per ROM bank.                          */
/*                                                                  */
/*  PARAMETERS:                                                     */
/*      None.                                                       */
/*                                                                  */
/*  RETURNS:                                                        */
/*      Nothing.                                                    */
/********************************************************************/
void SetupROMMemoryRegions(void)
{
#if MMU_CACHE_DISABLE == 0
    u_int32 i;
    #if (CPU_TYPE==CPU_ARM940T) || (CPU_TYPE==AUTOSENSE)
    u_int32 state, cs;
    #endif

    #if (CPU_TYPE==CPU_ARM940T) || (CPU_TYPE==AUTOSENSE)

        #if CPU_TYPE == AUTOSENSE
            if ( GetCPUType() == CPU_ARM940T )
            {
        #endif

                /* block interrupts */
                cs = critical_section_begin();

                state = mmuGetState( );      /* Get CP15 Reg 1 */

                mmuSetState( state & ~(BOTH_CACHES | MMU_ENABLE) ); /* Both Caches and MMU off */

                /* Need to Flush/Invalidate the DCache */
                CleanDCache( );
                DrainWriteBuffer( );
                FlushDCache( );

                /* And flush the ICache.. */
                FlushICache();

                for(i=0;i<NumROMBanks;i++)
                {
                    SetMemoryRegion((i+3), Bank[i].BankBase, Bank[i].BankSize);
                    EnableDCacheForRegion((i+3));
                    EnableICacheForRegion((i+3));
                }

                mmuSetState( state ); /* Caches / MMU back to entry state */

                critical_section_end( cs );

        #if CPU_TYPE == AUTOSENSE
            }
        #endif

    #endif /* (CPU_TYPE==CPU_ARM940T) || (CPU_TYPE==AUTOSENSE) */

    #if (CPU_TYPE==CPU_ARM920T) || (CPU_TYPE==AUTOSENSE)

        #if CPU_TYPE == AUTOSENSE
            if ( GetCPUType() == CPU_ARM920T )
            {
        #endif

                for(i=0;i<NumROMBanks;i++)
                {
                    mmuModifyROMCacheability (i,
                                              CACHEING_ON,
                                              Bank[i].BankBase,
                                              Bank[i].BankSize>>20);
                }

        #if CPU_TYPE == AUTOSENSE
            }
        #endif

    #endif /* (CPU_TYPE==CPU_ARM920T) || (CPU_TYPE==AUTOSENSE) */

#endif /* MMU_CACHE_DISABLE == 0 */
}
#endif /* !defined(DLOAD) && (RTOS != VXWORKS) && IMAGE_TYPE != BOOT && IMAGE_TYPE != BOOTEXT */

/********************************************/
/*  Local defines                           */
/********************************************/
#define ISA_NO_WAIT_STATES              0
#define ISA_WAIT_UNTIL_READY_0          2
#define ISA_WAIT_UNTIL_READY_1          3


/* Define the Nand Flash decsriptor for Needmore unconditionally here */
ISA_DEVICE NandFlashDesc = 
/* NAnd Flash Descriptor Definition */
{
  0,      /* Is it clocked.               */
  1,      /* XOEMask for each board       */
  1,   /* ChipSelect                  */
  CNXT_NANDFLASH_DESCRIPTOR, /* Descriptor number - determines base address. */
  ISA_WAIT_UNTIL_READY_1,  /* Ext wait    */                                     
  MEMORY, /* Use memory control signals.      */                                         
  PCCARD,    /* IOReg signal is not used.    */
  700,    /* Write access time (ns).      62.5*/
  300,    /* Read access time (ns).       62.5*/
  0,      /* IOReg setup time (ns).       0 */
  0,      /* IOReg access time (ns).      0*/
  208,    /*  Chip select setup time (ns). 26*/
  24,    /* Chip select hold time (ns).  3 */
  224,   /* Control setup time (ns).     28  */
  112,    /* Control hold time (ns).      14   */
};

void SetupNandFlash(void)
{
  /* SUNIL: Detect the Nand Flash device here, might have to do it differently 
      than NOR Flash */

   #define SEND_CMD(CMD) (*NANDCMD = CMD)
   #define CALC_MEMCLK(realtime)                      \
               if(realtime)                           \
               {                                      \
                   data = realtime * 100;             \
                   val = div = 0;                     \
                   while(val < data)                  \
                   {                                  \
                       ++div;                         \
                       val += MemClkPeriod32;         \
                   }                                  \
                   time = div;                        \
               }


   /* Nand Flash related */
   u_int32 deviceid = 0;
   u_int8 device_code = 0, maker_code = 0;
   LPREG  pPrimary, pSecondary, pReg, pXOEMask;
   bool UsingSyncIO = FALSE; 
   u_int16 time = 0;
   u_int32 FlashType, data, val, div;
   bool    FlashFound;
   u_int32 temp = 0;

   /* Start with no nand flash support */
   gbNeedmoreHasNandFlash = FALSE;

   /* check if bank 1 is taken by NOR Flash, if so don't try for
      nand flash in bank 1 */
   if(NumROMBanks == (MAX_NUM_FLASH_ROM_BANKS))
   {
      return;
   }

   /* Nand Flash is supported currently only on Wabash and Brazos chips */
   if( (ISWABASH) || (ISBRAZOS) )
   {
      switch(gboard_type)
      {
         case BOARD_CONFIG_BOARD_TYPE_MILANO:
            {
               if(gboard_rev > 3)
               {
                  CNXT_NANDFLASH_READY_BUSY_GPIO = 52;
                  CNXT_NANDFLASH_CHIP_ENABLE_GPIO = 69;
                  CNXT_NANDFLASH_CHIP_SELECT = 1;                  
                  /* reset the GPIO pins */
                  CNXT_SET(PLL_PIN_GPIO_MUX2_REG, PLL_PIN_GPIO_MUX2_PIO_69, 0);
                  CNXT_SET(PLL_PIN_GPIO_MUX1_REG, PLL_PIN_GPIO_MUX1_PIO_52, 0);
               }
               else
               {
                  /* Nand Flash is not supported on this rev of Milano */
                  return;
               }
            }
         break;
         case BOARD_CONFIG_BOARD_TYPE_BRONCO:
            {
               CNXT_NANDFLASH_READY_BUSY_GPIO = 27;
               CNXT_NANDFLASH_CHIP_ENABLE_GPIO = 42;                  
               CNXT_NANDFLASH_CHIP_SELECT = 1;                  
               /* reset the GPIO pins */
               CNXT_SET(PLL_PIN_GPIO_MUX0_REG, PLL_PIN_GPIO_MUX0_PIO_27, 0);
               CNXT_SET(PLL_PIN_GPIO_MUX1_REG, PLL_PIN_GPIO_MUX1_PIO_42, 0);
            }
         break;
         default:
            /* Nand Flash is not supported on this Board type */
            return;
         break;
      }
   }
   else
   {
      /* Nand Flash is not supported on this Chip type */
      return;
   }

   /* Enumerate all the known Nand Flash parts */
   for (FlashType = 0; FlashType < (u_int32)NumNandFlashTypes; FlashType++)
   {
       /***********************************/
       /* Setup ISA descriptor for Nand Flash. */
       /***********************************/
        pPrimary   = (LPREG) PCI_ISA_DESC_BASE + CNXT_NANDFLASH_DESCRIPTOR;
        pSecondary = (LPREG) PCI_DESC2_BASE    + CNXT_NANDFLASH_DESCRIPTOR;

        /* Setup the descriptor designated in the card's ISADevice */
        /* structure with the other parameters in the ISADevice    */
        /* structure.                                              */

        *pPrimary = (PCI_ISA_DESC_TYPE_ISA                                     |
                     (NandFlashDesc.ExtWait << PCI_ISA_DESC_EXT_WAIT_SHIFT) |
                     (NandFlashDesc.IOMemory << PCI_ISA_DESC_IO_TYPE_SHIFT) |
                     (NandFlashDesc.ChipSelect << PCI_ISA_DESC_CS_SHIFT));

            switch (NandFlashDesc.IORegMode)
            {
                case PCCARD:
                    *pPrimary |= PCI_NEEDS_ISA_REG;
                    *pPrimary &= ~PCI_ISA_DESC_REG_IS_PROG;
                    break;
                case PROGRAMMABLE:
                    *pPrimary |= PCI_NEEDS_ISA_REG;
                    *pPrimary |= PCI_ISA_DESC_REG_IS_PROG;
                    break;
                case OFF:
                default:
                    *pPrimary &= ~PCI_NEEDS_ISA_REG;
                    *pPrimary &= ~PCI_ISA_DESC_REG_IS_PROG;
            }

        /* Set syncIO mode if required */
        *pPrimary = ((*pPrimary & ~PCI_ISA_DESC_SYNC_IO_ENABLE_MASK) |
                     (NandFlashDesc.bClocked << PCI_ISA_DESC_SYNC_IO_ENABLE_SHIFT));
        if(NandFlashDesc.bClocked)
        {
            UsingSyncIO = TRUE;
        }

        CALC_MEMCLK(NandFlashDescriptors[FlashType].WriteAccess)

        *pPrimary = ( (*pPrimary & ~PCI_ISA_DESC_WRITE_WAIT_MASK) | 
                     time << PCI_ISA_DESC_WRITE_WAIT_SHIFT);

        CALC_MEMCLK(NandFlashDescriptors[FlashType].ReadAccess)

        *pPrimary = ( (*pPrimary & ~PCI_ISA_DESC_READ_WAIT_MASK) | 
                     time << PCI_ISA_DESC_READ_WAIT_SHIFT);

        *pPrimary &= ~(PCI_ISA_DESC_SETUP_TIME_MASK | PCI_ISA_DESC_HOLD_TIME_MASK);

         CALC_MEMCLK(NandFlashDesc.IORegSetupTime)
        *pSecondary = (time  << PCI_DESC2_REG_SETUP_TIME_SHIFT);
         CALC_MEMCLK(NandFlashDesc.IORegAccessTime)
        *pSecondary |= (time << PCI_DESC2_REG_ACCESS_TIME_SHIFT);
         CALC_MEMCLK(NandFlashDesc.CSSetupTime)
        *pSecondary |= (time << PCI_DESC2_CS_SETUP_TIME_SHIFT);
         CALC_MEMCLK(NandFlashDesc.CSHoldTime)
        *pSecondary |= (time << PCI_DESC2_CS_HOLD_TIME_SHIFT);
         CALC_MEMCLK(NandFlashDesc.CtrlSetupTime)
        *pSecondary |= (time << PCI_DESC2_CTRL_SETUP_TIME_SHIFT);
         CALC_MEMCLK(NandFlashDesc.CtrlHoldTime)
        *pSecondary |= (time << PCI_DESC2_ADDR_HOLD_TIME_SHIFT);
        
        pXOEMask = (LPREG)PCI_ROM_XOE_MASK_REG;

        *pXOEMask = *pXOEMask |
            (NandFlashDesc.XOEMask << NandFlashDesc.ChipSelect);

         temp = (u_int32)*pXOEMask;

       /* If anything is using SyncIO, enable it */
       if(UsingSyncIO)
       {
           /*****************************************
            * Enable Program synchronous I/O clock  *
            *****************************************/
            pReg = (LPREG)PCI_ROM_MODE_REG;
            *pReg = *pReg | 0x400;
       }     
       /* We are done with the ISA desciptor setup, Now read the device ID */

      MAKE_GPIO_INPUT_BANK((u_int8)(CNXT_NANDFLASH_READY_BUSY_GPIO/32), 
                           CNXT_NANDFLASH_READY_BUSY_GPIO%32);

      /* Chip Enable*/
      SET_GPIO_PIN_BANK((u_int8)(CNXT_NANDFLASH_CHIP_ENABLE_GPIO/32), 
                        CNXT_NANDFLASH_CHIP_ENABLE_GPIO%32, 
                        NAND_FLASH_CHIP_ENABLE_VAL);   

      SEND_CMD(NandFlashDescriptors[FlashType].EnterIDMode); // read id
      *NANDADDR = 0x00000000;  // address
      /* make sure that the type casting is present to get a BYTE value */
      maker_code = (u_int8)*NANDDATA;
      device_code = (u_int8)*NANDDATA;
      /* dSIABLE CHIP */
      SET_GPIO_PIN_BANK((u_int8)(CNXT_NANDFLASH_CHIP_ENABLE_GPIO/32), 
                        CNXT_NANDFLASH_CHIP_ENABLE_GPIO%32, 
                        NAND_FLASH_CHIP_DISABLE_VAL);

      if( (maker_code == NandFlashDescriptors[FlashType].MfrID) &&
          (device_code == NandFlashDescriptors[FlashType].DeviceID) )
      {
         FlashFound = TRUE;
         /* Populate the Flash descr info into Bank */
         Bank[1].pFlashDesc = 0;
         Bank[1].pNandFlashDesc = (LPNAND_FLASH_DESC)&NandFlashDescriptors[FlashType];
         Bank[1].BankWidth = NandFlashDescriptors[FlashType].NandFlashOrg;
         Bank[1].ChipWidth = NandFlashDescriptors[FlashType].NandFlashOrg;
         Bank[1].BankSize =  NandFlashDescriptors[FlashType].ChipSize >> 3;
         Bank[1].BankBase =  Bank[0].BankSize+1;
         gbNeedmoreHasNandFlash = TRUE;
         //Bank[1].ROMType = 
         break;
      }

   }/* for (FlashType = (u_int32)0; FlashType < (u_int32)NumNandFlashTypes; FlashType++) */

   deviceid = (maker_code<<8) | device_code;
   NandFlashDeviceID = deviceid;
}

/****************************************************************************
 * Modifications:
 * $Log:
 ****************************************************************************/

