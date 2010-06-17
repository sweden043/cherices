/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                   Conexant Systems Inc. (c) 1998-2003                    */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        startup.h
 *
 *
 * Description:     Header file for startup definitions
 *
 *
 * Author:          Tim Ross
 *
 ****************************************************************************/
/* $Id: startup.h,v 1.44, 2004-04-22 22:10:19Z, Sunil Cheruvu$
 ****************************************************************************/

#ifndef _STARTUP_H
#define _STARTUP_H

/*****************/
/* Include Files */
/*****************/
#include "basetype.h"
#include "hwconfig.h"

/**********************/
/* Local Definitions  */
/**********************/
#define ROM_TYPE_FLASH				1
#define ROM_TYPE_OTP				2
#define ROM_TYPE_EMPTY				3

#define MAX_NUM_FLASH_ROM_BANKS			2
#define MAX_NUM_SECTORS_PER_BANK                160//80

#define FLASH_CHIP_VENDOR_INTEL                 0x89
#define FLASH_CHIP_VENDOR_AMD                   0x01
#define FLASH_CHIP_VENDOR_FUJITSU               0x04
#define FLASH_CHIP_VENDOR_ATMEL                 0x1F
#define FLASH_CHIP_VENDOR_ST                    0x20
#define FLASH_CHIP_VENDOR_SST                   0xBF
#define FLASH_CHIP_VENDOR_TOSHIBA               0x98

#define FLASH_CHIP_VENDOR_MACRONIX              0xC2

#define FLASH_CHIP_VENDOR_SAMSUNG               0xEC
#define FLASH_CHIP_VENDOR_UNKNOWN               0xFF

/*
** Flags definitions used for both the FLASH_DESC and BANK_INFO structures
*/

#define FLASH_FLAGS_BURST_SUPPORTED             (1L << 0)
#define FLASH_FLAGS_UNLOCK_LOCK_SEQ_REQD        (1L << 1)
#define FLASH_FLAGS_UNIFORM_SECTOR_SIZE         (1L << 2)
#define FLASH_FLAGS_TOP_BOOT_BLOCK_LAYOUT       (1L << 3)
#define FLASH_FLAGS_CONSTANT_WRITE_SPEED        (1L << 4)

/*
** FLASH Enter/Exit ID Mode Defines
*/
#define FLASH_ID_ENTER_INTEL            0
#define FLASH_ID_ENTER_AMDX8            1
#define FLASH_ID_ENTER_AMDX16           2
#define FLASH_ID_ENTER_ATMELX16         3

#define FLASH_ID_ENTER_SAMSUNG          0x90

#define FLASH_ID_EXIT_INTEL             0
#define FLASH_ID_EXIT_AMD               1
#define FLASH_ID_EXIT_SAMSUNG           2

/* Nand Flash driver related values */
#define CNXT_NANDFLASH_DESCRIPTOR       2
#define NAND_FLASH_DESC_BASE_ADDRESS (PIO_BASE + (CNXT_NANDFLASH_DESCRIPTOR<<20))
#define NANDCMD  ((volatile u_int32*) (NAND_FLASH_DESC_BASE_ADDRESS+0x200) )
#define NANDADDR ((volatile u_int32*) (NAND_FLASH_DESC_BASE_ADDRESS+0x100) )
#define NANDDATA ((volatile u_int32*) (NAND_FLASH_DESC_BASE_ADDRESS+0x000) )
#define NAND_FLASH_CHIP_ENABLE_VAL         0
#define NAND_FLASH_CHIP_DISABLE_VAL        1


/*********/
/* Flash */
/*********/

/* NOTE:  The SetupROMs() function in STARTUP/ROM.C and CODELDR/ROM.C need to be
** modified if this structure is modified.  ROM.C depends on the fact that there are
** only two function vectors in this structure and that they are the first two entries
** in the structure.  Do not change this or any image built will not be able to run from
** ROM directly.
*/

/********************/
/* Flash Descriptor */
/********************/
typedef struct _FLASH_DESC  {                                   /* See above note! */
    u_int32     EnterIDMode;                                    /* See above note! */
    u_int32     ExitIDMode;                                     /* See above note! */
    u_int8      MfrID;
    u_int32     DeviceID;
    u_int32     ChipSize;
    u_int16     ReadAccess;
    u_int8      ReadCSSetup;
    u_int8      ReadCSHold;
    u_int8      ReadCtrlSetup;
    u_int8      ReadAddrHold;
    u_int16     WriteAccess;
    u_int8      WriteCSSetup;
    u_int8      WriteCSHold;
    u_int8      WriteCtrlSetup;
    u_int8      WriteAddrHold;
    u_int8      BurstTime;
    u_int8      PageSize;
    u_int32     Flags;
    u_int8      ChipNumSectors;
    const u_int16 *ChipSectorSize;
} FLASH_DESC, *LPFLASH_DESC;

extern FLASH_DESC FlashDescriptors[];
extern int NumFlashTypes;

typedef struct _NAND_FLASH_DESC  {                                   /* See above note! */
    /* Nand Flash specific settings */
    u_int32     EnterIDMode;                                    /* See above note! */
    u_int32     ExitIDMode;                                     /* See above note! */
    u_int8      MfrID;
    u_int32     DeviceID;
    u_int32     ChipSize;
    u_int16     ReadAccess;
    u_int16     WriteAccess;
    u_int8      CSSetup;
    u_int8      CSHold;
    u_int8      CtrlSetup;
    u_int8      AddrHold;
    u_int32     Flags;
    u_int8      NandFlashOrg; /* x8 or x16 */
    u_int16     NandFlashPageSize;
    u_int32     NandFlashPagesPerBlock;
    u_int32     NandFlashNumOfBlocks;
    u_int8      NandFlashSpareAreaSize;
}NAND_FLASH_DESC, *LPNAND_FLASH_DESC;

extern NAND_FLASH_DESC NandFlashDescriptors[];
extern int NumNandFlashTypes;

/********************/
/* Bank Informction */
/********************/
typedef struct _BANK_INFO
{
    u_int32             BankBase;
    u_int32             BankSize;
    u_int8              ROMType;
    u_int8              BankWidth;
    u_int8              ChipWidth;
    u_int32             Flags;
    u_int32             ReadTimings;
    u_int32             WriteTimings;
    u_int32             WriteModeTimings;
    LPFLASH_DESC        pFlashDesc;
    LPNAND_FLASH_DESC    pNandFlashDesc;
} BANK_INFO, *LPBANK_INFO;

extern BANK_INFO Bank[];
extern u_int8 NumROMBanks;
extern u_int32 UniformBlockSize;

/******************************************************************************************/
/*                                                                                        */
/* Bottom Boot Block Format:                                                              */
/* ========================                                                               */
/*                                                                                        */
/*                     +----------------------------------------+ <- TOP of Bank 0        */
/*                     |                                        |                         */
/*                     +----------------------------------------+                         */
/*                     |                                        |                         */
/* FsBlockStart     -> +----------------------------------------+                         */
/*                     |                                        |                         */
/*                     +----------------------------------------+                         */
/*                     |                                        |                         */
/*                     +----------------------------------------+                         */
/*                     |                                        |                         */
/*                     +----------------------------------------+                         */
/*                     |                                        |                         */
/*                     +----------------------------------------+                         */
/*                     |                                        |                         */
/*                     +----------------------------------------+                         */
/*                     |                                        |                         */
/*                     +----------------------------------------+                         */
/*                     +----------------------------------------+                         */
/*                     +----------------------------------------+                         */
/* CodeBlockStart   -> +----------------------------------------+                         */
/*                     +----------------------------------------+                         */
/* ConfigBlockStart -> +----------------------------------------+                         */
/* BootBlockStart   -> +----------------------------------------+ <- BOTTOM of Bank 0     */
/*                                                                   (0x20000000)         */
/*                                                                                        */
/* Top Boot Block Format:                                                                 */
/* =====================                                                                  */
/*                                                                                        */
/*                     +----------------------------------------+ <- TOP of Bank 0        */
/*                     +----------------------------------------+                         */
/* ConfigBlockStart -> +----------------------------------------+                         */
/*                     +----------------------------------------+                         */
/*                     +----------------------------------------+                         */
/*                     +----------------------------------------+                         */
/*                     +----------------------------------------+                         */
/*                     |                                        |                         */
/*                     +----------------------------------------+                         */
/*                     |                                        |                         */
/* FsBlockStart     -> +----------------------------------------+                         */
/*                     |                                        |                         */
/*                     +----------------------------------------+                         */
/*                     |                                        |                         */
/*                     +----------------------------------------+                         */
/*                     |                                        |                         */
/*                     +----------------------------------------+                         */
/*                     |                                        |                         */
/*                     +----------------------------------------+                         */
/*                     |                                        |                         */
/* CodeBlockStart   -> +----------------------------------------+                         */
/*                     |                                        |                         */
/* BootBlockStart   -> +----------------------------------------+ <- BOTTOM of Bank 0     */
/*                                                                   (0x20000000)         */
/*                                                                                        */
/******************************************************************************************/

/*************************************/
/* ROM information array definitions */
/*************************************/

typedef struct _ROM_SECTOR
{
  u_int8        *Start;
  u_int32       Length;
} ROM_SECTOR;

typedef struct _ROM_INFO
{
  u_int8        *BankBase;
  u_int32       BankSize;
  u_int32       NumSectors;
  u_int32       Flags;
  ROM_SECTOR    Sector[MAX_NUM_SECTORS_PER_BANK];
} ROM_INFO;

typedef struct
{
   u_int32	RunningFromBank;
   u_int32      BootBlockSize;
   u_int8       *BootBlockStart;
   u_int32      ConfigBlockSize;
   u_int8       *ConfigBlockStart;
   u_int32      CodeBlockSize;
   u_int8       *CodeBlockStart;
   u_int32      FsBlockSize;
   u_int8       *FsBlockStart;
   u_int32      NumBanks;
   u_int8       *RomBase;
   u_int32      RomSize;
   ROM_INFO     RomInfo[MAX_NUM_FLASH_ROM_BANKS];
} FLASH_INFO, *LPFLASH_INFO;

extern FLASH_INFO FlashInfo;

/***********************/
/* Fatal Exit          */
/***********************/
#define C_ERR_CODE          0x80

#define ERR_BAD_BOARD_ID    0x00 | C_ERR_CODE
#define ERR_NOT_ENOUGH_RAM  0x01 | C_ERR_CODE

void StartupFatalExit(char *);

/***********************/
/* Function Prototypes */
/***********************/
u_int32 ReadCfgEeprom(u_int32 uSize, u_int32 uOffset);
void Checkpoint(char *);
void GetDefaultBSPCfg(void **pBSPCfgInfo);
void WriteLEDs(u_int32);
void ClearLEDs(void);
void WriteSerial(char *);
void BspMmuTransOff(void);
#if CPU_TYPE == AUTOSENSE
   extern int GetCPUType();
#endif
void SetupROMs(void);

typedef enum _PLL_SOURCE
{
    ARM_PLL_SOURCE = 0,
    MEM_PLL_SOURCE
} PLL_SOURCE;

void CalcClkFreqAndPeriod (u_int32 *frequency, u_int32 *period, PLL_SOURCE pll_source, u_int32 xtal_freq, bool raw);

/***********************/
/* Flash ROM           */
/***********************/
bool GetFlashInfo(LPFLASH_INFO pFlashInfo);
void SetupROMMemoryRegions(void);
void SetMemoryRegion(u_int32 Region, u_int32 base, u_int32 size);
void EnableDCacheForRegion(u_int32 fbank);
bool DisableDCacheForRegion(u_int32 fbank);

/***********************/
/* MEM PLL Speed       */
/***********************/
extern u_int32 MemClkPeriod32;

#endif

/****************************************************************************
 * Modifications:
 * $Log: 
 *  45   mpeg      1.44        4/22/04 5:10:19 PM     Sunil Cheruvu   CR(s) 
 *        8870 8871 : Added the Nand Flash support for Wabash(Milano rev 5 and 
 *        above) and Brazos(Bronco).
 *  44   mpeg      1.43        3/19/04 1:52:01 AM     Xiao Guang Yan  CR(s) 
 *        8595 : Added vendor ID for Macronix flash. 
 *  43   mpeg      1.42        11/10/03 5:22:37 PM    Tim White       CR(s): 
 *        7900 7901 Add support to get the raw VCO freq and period.
 *        
 *  42   mpeg      1.41        11/1/03 2:48:59 PM     Tim Ross        CR(s): 
 *        7719 7762 Added ReadCfgEeprom() prototype.
 *  41   mpeg      1.40        10/30/03 4:04:36 PM    Tim White       CR(s): 
 *        7756 7757 Remove CalcMemClkPeriod() function and replace with 
 *        CalcFreqAndPeriod() function
 *        which works on any PLL and returns both the frequency and the clock 
 *        period.
 *        
 *  40   mpeg      1.39        10/17/03 10:06:00 AM   Larry Wang      CR(s): 
 *        7673 Remove the global MemClkPeriod.
 *  39   mpeg      1.38        7/9/03 3:28:24 PM      Tim White       SCR(s) 
 *        6901 :
 *        Phase 3 codeldrext drop.
 *        
 *        
 *  38   mpeg      1.37        6/24/03 6:35:24 PM     Tim White       SCR(s) 
 *        6831 :
 *        Add flash, hsdp, demux, OSD, and demod support to codeldrext
 *        
 *        
 *  37   mpeg      1.36        6/5/03 5:29:30 PM      Tim White       SCR(s) 
 *        6660 6661 :
 *        Flash banks separately controlled by the 920 MMU using hardware 
 *        virtual pagemapping.
 *        
 *        
 *  36   mpeg      1.35        5/14/03 4:11:50 PM     Tim White       SCR(s) 
 *        6346 6347 :
 *        Added CalcMemClkPeriod() function.
 *        
 *        
 *  35   mpeg      1.34        5/5/03 4:02:32 PM      Tim White       SCR(s) 
 *        6172 :
 *        Removed LED arg to Checkpoint and StartupFatalExit functions.
 *        
 *        
 *  34   mpeg      1.33        4/30/03 5:30:26 PM     Billy Jackman   SCR(s) 
 *        6113 :
 *        Removed some vendor-specific code.
 *        Added CPUTypeAddr, the location in parser memory of the CPU type 
 *        detected at
 *        runtime.
 *        Eliminated double slash comments.
 *        Modified conditional compilation directives to include external 
 *        references
 *        when CPU_TYPE=AUTOSENSE.
 *        
 *  33   mpeg      1.32        4/4/03 5:35:16 PM      Craig Dry       SCR(s) 
 *        5956 :
 *        Define EEPROM Configuration offsets to allow access of 
 *        ConfigurationValid
 *        from pawser memory.
 *        
 *  32   mpeg      1.31        1/31/03 5:40:02 PM     Dave Moore      SCR(s) 
 *        5375 :
 *        Removed prototypes for several unused functions that were
 *        in su_cache.s.
 *        
 *        
 *  31   mpeg      1.30        1/22/03 10:42:14 AM    Tim White       SCR(s) 
 *        5099 :
 *        Set Maximum Number of Flash ROM banks to 2 from 4.  Cannot use ~cs2 
 *        on Brazos.
 *        For a long time, we have only supported the use of 2 banks anyway, so
 *         this should
 *        have been changed for Colorado chips long ago...
 *        
 *        
 *  30   mpeg      1.29        12/12/02 5:16:22 PM    Tim White       SCR(s) 
 *        5157 :
 *        Removed the ChipID and ChipRevID externs which were leftovers from 
 *        the chip type
 *        macros living in this file.
 *        
 *        
 *  29   mpeg      1.28        8/15/02 6:55:34 PM     Tim Ross        SCR(s) 
 *        4409 :
 *        Moved chip identification macros from here to respective chip header 
 *        files.
 *        
 *  28   mpeg      1.27        4/30/02 12:57:44 PM    Billy Jackman   SCR(s) 
 *        3656 :
 *        Added extern declarations for Enable/DisableDCacheForRegion.
 *        
 *  27   mpeg      1.26        4/29/02 9:37:24 AM     Bobby Bradford  SCR(s) 
 *        3580 :
 *        Modified the FLASH_DESC structure, to support changes in
 *        the STARTUP code that detects the ROM(FLASH) type.
 *        
 *  26   mpeg      1.25        4/26/02 3:20:00 PM     Tim White       SCR(s) 
 *        3562 :
 *        Add support for Colorado Rev_F
 *        
 *        
 *  25   mpeg      1.24        4/10/02 5:15:24 PM     Tim White       SCR(s) 
 *        3509 :
 *        Eradicate external bus (PCI) bitfield usage.
 *        
 *        
 *  24   mpeg      1.23        11/15/01 9:02:44 AM    Bobby Bradford  SCR(s) 
 *        2878 :
 *        Added new macros, ISHONDO and ISWABASH, using newly defined Hondo and
 *         Wabash
 *        PCI Device ID information
 *        
 *  23   mpeg      1.22        6/22/01 3:11:42 PM     Tim White       SCR(s) 
 *        2150 2155 :
 *        Set WriteWait to 0 in order to force Intel Flash ROM parts
 *        to ignore all writes when it's in READ_MODE.
 *        
 *        
 *  22   mpeg      1.21        2/15/01 5:00:54 PM     Tim White       DCS#1215:
 *         Merge Vendor_D changes back into the mainline code.
 *        
 *  21   mpeg      1.20        2/15/01 4:48:46 PM     Tim White       DCS#1230:
 *         Globalize MemClkPeriod for use by the Soft Modem code.
 *        
 *  20   mpeg      1.19        9/26/00 6:05:30 PM     Tim White       Added 
 *        switchable secondary descriptor capability to low-level flash 
 *        function.
 *        
 *  19   mpeg      1.18        9/6/00 6:05:04 PM      Tim White       Added 
 *        Toshiba Flash ROM support.
 *        
 *  18   mpeg      1.17        6/21/00 5:32:08 PM     Tim White       Added 
 *        Colorado Rev_C changes (pin muxing changes).
 *        
 *  17   mpeg      1.16        6/15/00 1:51:20 PM     Tim White       Added an 
 *        ISCOLORADOREVA macro.
 *        
 *  16   mpeg      1.15        6/9/00 11:57:20 AM     Tim White       Added 
 *        hwconfig.h prereq #include for CUSTOMER #ifdef's to work correctly.
 *        
 *  15   mpeg      1.14        5/17/00 12:21:34 PM    Tim White       Added 
 *        Flags paramater.
 *        
 *  14   mpeg      1.13        5/4/00 5:48:50 PM      Tim White       Vendor_B 
 *        must not mess with ~cs2 since it can be connected to SRAM
 *        which can cause ~cs2 to drop which can cause data loss on SRAM 
 *        depending
 *        on memrd memwr signals during SetupROMs().
 *        
 *  13   mpeg      1.12        4/24/00 6:48:48 PM     Tim White       Removed 
 *        prototypes which are no longer in startup/cache.s
 *        
 *  12   mpeg      1.11        4/24/00 6:02:30 PM     Senthil Veluswamy No 
 *        change.
 *        
 *  11   mpeg      1.10        4/10/00 5:55:36 PM     Tim White       Updated 
 *        for corrected WRITE (program) speed values.
 *        
 *  10   mpeg      1.9         4/10/00 4:24:00 PM     Tim White       Added 
 *        several flash ROM types to main codebase (startup).
 *        
 *  9    mpeg      1.8         3/8/00 5:18:50 PM      Tim White       
 *        Restructured the BANK_INFO array and added support for new Intel 
 *        Flash ROM.
 *        
 *  8    mpeg      1.7         3/2/00 5:28:12 PM      Tim White       Added ROM
 *         per bank support for Colorado.
 *        
 *  7    mpeg      1.6         12/8/99 5:10:54 PM     Tim Ross        Added 
 *        changes for Colorado.
 *        
 *  6    mpeg      1.5         11/1/99 6:28:18 PM     Tim Ross        Added 
 *        GetDefaultBSPCfg() prototype.
 *        
 *  5    mpeg      1.4         10/28/99 12:55:08 PM   Dave Wilson     Added 
 *        prototypes for GetChipID and GetChipRev
 *        
 *  4    mpeg      1.3         10/13/99 4:52:20 PM    Tim White       Added the
 *         FlashInfo structure for use with interfacing to the low-level
 *        flash driver (flash/flashrw.c).
 *        
 *  3    mpeg      1.2         9/7/99 10:31:26 PM     Tim Ross        Added 
 *        BANK_INFO structure and moved FLASH_DESCRIPTOR 
 *        structure to ROM.H in STARTUP directory.
 *        
 *  2    mpeg      1.1         8/11/99 5:23:06 PM     Dave Wilson     Replaced 
 *        kal.h with basetype.h to allow non-KAL apps to use the header.
 *        
 *  1    mpeg      1.0         6/8/99 12:55:00 PM     Tim Ross        
 * $
 ****************************************************************************/

