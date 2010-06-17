/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                   Conexant Systems Inc. (c) 1998-2003                    */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        FLASHRW.C
 *
 *
 * Description:     Low level flash memory access functions
 *
 *
 * Author:          Tim White
 *
 ****************************************************************************/
/* $Header: flashrw.c, 112, 6/29/04 11:00:39 PM, Xiao Guang Yan$
 ****************************************************************************/

/*****************/
/* Include Files */
/*****************/
#include <string.h>
#include "stbcfg.h"
#include "basetype.h"
#ifndef DLOAD
#if RTOS == VXWORKS
    #include <cacheLib.h>
#endif
#include "kal.h"
#endif
#ifdef DLOAD
#include "defs.h"
#endif
#include "startup.h"
#include "retcodes.h"
#include "flashrw.h"

#if DOWNLOAD_SERIAL_SUPPORT==YES
 #include "libfuncs.h"
#endif

#define FLASH_ROMS_COPY_CODE_SIZE  4096       /* 4 K ... current size is about 800 bytes */

/************************/
/* External Definitions */
/************************/
extern int write_amd(void *addr, void *buffer, int size);
extern int erase_amd(void *addr, void (*callback)(void));
extern int erase_amd_bank(void *addr, void (*callback)(void));
extern int write_intel(void *addr, void *buffer, int size);
extern int erase_intel(void *addr, void (*callback)(void));
extern int erase_intel_bank(void *addr, void (*callback)(void));
extern void FCopy(void *dest, const void *src, size_t count);
extern void FFillBytes(void *dest, int value, size_t count);

extern int write_sam_nandflash(void *addr, void *buffer, int size);
extern int read_sam_nandflash(void *addr, void *buffer, int size);
extern int erase_samnand(void *addr, void (*callback)(void));
extern int erase_samnand_bank(void *addr, void (*callback)(void));

extern u_int8 NumROMBanks;

#if DOWNLOAD_SERIAL_SUPPORT==YES
extern u_int8 *FirstCodeAddr;
extern bool   PermissionToWriteBootSector;
extern char   GetUserChoice(int iMax);
#endif

#if !defined(DLOAD) && (RTOS != NOOS)
extern bool bKalUp;
#endif

#ifndef DLOAD
extern int CallFuncByAddrWithIntrsDisabled (u_int8 *func_addr,
				u_int32 d0, u_int32 d1, u_int32 d2);

extern bool FlashDisableDCache( u_int32 fbank, unsigned int addr, unsigned int num_of_megs );
extern void FlashEnableDCache( u_int32 fbank, unsigned int addr, unsigned int num_of_megs );
#endif

/******************************************/
/* Local Definitions and Global Variables */
/******************************************/
bool flash_low_lev_init_done = FALSE;
extern BANK_INFO Bank[];
int CurrentBank;
FLASH_INFO FlashInfo;
int (*write_flash_array[MAX_NUM_FLASH_ROM_BANKS])(void *addr, void *buffer, int size);
int (*read_flash_array[MAX_NUM_FLASH_ROM_BANKS])(void *addr, void *buffer, int size);
int (*erase_flash_array[MAX_NUM_FLASH_ROM_BANKS])(void *addr, void (*callback)(void));
int (*erase_flash_bank[MAX_NUM_FLASH_ROM_BANKS])(void *addr, void (*callback)(void));
#if !defined(DLOAD) && (RTOS != NOOS)
sem_id_t flash_access_sem = (sem_id_t) NULL;
#endif

/**************************/
/* Function Prototypes    */   
/**************************/
int write_flash(void *addr, void *buffer, int size);
static int erase_flash(void *addr, void (*callback)(void));
int CopyAndCallRoutine(u_int8 *func_addr, u_int32 d0, u_int32 d1, u_int32 d2);
int block_erase(void *addr, int block_size);
int block_erase_with_callback(void *addr, int block_size, void (*callback)(void));
int bank_erase(void *addr);
int bank_erase_with_callback(void *addr, void (*callback)(void));
bool block_check(void *addr, int block_size);
bool block_check_nosem(void *addr, int block_size);


/********************************************************************/
/*  FindCurrentBank                                                 */
/*                                                                  */
/*  PARAMETERS:                                                     */
/*      addr - void ptr to a ROM address                            */
/*                                                                  */
/*  DESCRIPTION:                                                    */
/*      Determines the bank containing addr.                        */
/*                                                                  */
/*  RETURNS:                                                        */
/*      TRUE - addr was within one of the flash banks.              */
/*      FALSE - addr was outside the range of the flash banks.      */
/********************************************************************/
bool FindCurrentBank(void *addr)
{
    u_int32 BankBase;

    BankBase = ROM_START_ADDRESS;

    /* If addr is less than ROM start, return FALSE. */
    if (BankBase > (u_int32)addr)
    {
        return(FALSE);
    }
        
    for (CurrentBank = 0; CurrentBank < MAX_NUM_FLASH_ROM_BANKS; CurrentBank++)
    {
        BankBase += Bank[CurrentBank].BankSize;
        if (BankBase > (u_int32)addr)
	{
            break;
	}
    }
    
    /* If addr is higher than the top of ROM, return FALSE. */
    if (BankBase < (u_int32)addr)
    {
        return(FALSE);
    }

    return(TRUE);
}    

/******************************************************************************************/
/* This function is called by users of the flash ROM to obtain information about where    */
/* the different users of the flash ROM live.  This is the central point of flash ROM     */
/* global allocation.  See include/startup.h for accompanying header file.                */
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

bool GetFlashInfo(LPFLASH_INFO pFlashInfo)
{
   int	NumChips, i;
   int	size;
   unsigned int  j;
   bool rc = TRUE;

   NumChips =  Bank[0].BankWidth / Bank[0].ChipWidth;

   if(Bank[0].Flags & FLASH_FLAGS_UNIFORM_SECTOR_SIZE)
   {
      pFlashInfo->BootBlockSize		= ((u_int32) Bank[0].pFlashDesc->ChipSectorSize<<12) * NumChips;
   }
   else
   {
      pFlashInfo->BootBlockSize		= ((u_int32) Bank[0].pFlashDesc->ChipSectorSize[0]<<12) * NumChips;
   }
   pFlashInfo->BootBlockStart		= (u_int8 *) Bank[0].BankBase;

   pFlashInfo->NumBanks			= (u_int32)  NumROMBanks;
   pFlashInfo->RomBase			= (u_int8 *) ROM_START_ADDRESS;
   pFlashInfo->RomSize			= (u_int32)  0x0;

   pFlashInfo->RunningFromBank		= 0;

   for(i=0; i<NumROMBanks; i++)
   {
     pFlashInfo->RomSize		+= (u_int32) Bank[i].BankSize;
   }
   
   if(Bank[0].Flags & FLASH_FLAGS_UNIFORM_SECTOR_SIZE)
   {
       pFlashInfo->ConfigBlockSize	= (((u_int32) Bank[0].pFlashDesc->ChipSectorSize<<12) * NumChips) << 1;
       pFlashInfo->ConfigBlockStart	= (u_int8 *) pFlashInfo->BootBlockStart +
							pFlashInfo->BootBlockSize;
       pFlashInfo->FsBlockSize		= (((u_int32) Bank[0].pFlashDesc->ChipSectorSize<<12) * NumChips) << 2;
       pFlashInfo->FsBlockStart		= (u_int8 *) ((u_int32) Bank[0].BankBase + Bank[0].BankSize -
		((((u_int32)Bank[0].pFlashDesc->ChipSectorSize<<12) * NumChips)<<2));
       pFlashInfo->CodeBlockStart       = (u_int8 *) pFlashInfo->ConfigBlockStart +
                                                        pFlashInfo->ConfigBlockSize;
   }
   else
   {

   if(Bank[0].pFlashDesc->ChipSectorSize[0] <
	Bank[0].pFlashDesc->ChipSectorSize[Bank[0].pFlashDesc->ChipNumSectors-1])
   {
     /* Bottom Boot Block Flash ROM Parts */
     if(Bank[0].pFlashDesc->ChipSectorSize[1] == Bank[0].pFlashDesc->ChipSectorSize[2])
     {
       #if (FLASH_CONFIG_DATA_SIZE == 0)
       pFlashInfo->ConfigBlockSize	= (((u_int32) Bank[0].pFlashDesc->ChipSectorSize[1]<<12)
						* NumChips) << 1;
		 #else
		 pFlashInfo->ConfigBlockSize	= FLASH_CONFIG_DATA_SIZE;
		 #endif
		 
		 #if (FLASH_CONFIG_DATA_START_ADDR == 0)
       pFlashInfo->ConfigBlockStart	= (u_int8 *) pFlashInfo->BootBlockStart +
							pFlashInfo->BootBlockSize;
		 #else
		 pFlashInfo->ConfigBlockStart	= (u_int8 *)FLASH_CONFIG_DATA_START_ADDR;
		 #endif
     }
     else
     {
       pFlashInfo->ConfigBlockSize	= (u_int32)  0x0;
       pFlashInfo->ConfigBlockStart	= (u_int8 *) 0x0;
       rc = FALSE;
     }
     if(Bank[0].pFlashDesc->ChipSectorSize[Bank[0].pFlashDesc->ChipNumSectors-1] ==
		Bank[0].pFlashDesc->ChipSectorSize[Bank[0].pFlashDesc->ChipNumSectors-2])
     {
       pFlashInfo->FsBlockSize		=
	(((u_int32) Bank[0].pFlashDesc->ChipSectorSize[Bank[0].pFlashDesc->ChipNumSectors-1]<<12)
						* NumChips) << 1;
       pFlashInfo->FsBlockStart		= (u_int8 *) ((u_int32) Bank[0].BankBase +
		Bank[0].BankSize -
		((((u_int32)Bank[0].pFlashDesc->ChipSectorSize[Bank[0].pFlashDesc->ChipNumSectors-1]<<12) * NumChips)<<1));
     }
     else
     {
       pFlashInfo->FsBlockSize		= (u_int32)  0x0;
       pFlashInfo->FsBlockStart		= (u_int8 *) 0x0;
       rc = FALSE;
     }
     pFlashInfo->CodeBlockStart		= (u_int8 *) pFlashInfo->ConfigBlockStart +
							pFlashInfo->ConfigBlockSize;
   }
   else
   {
     /* Top Boot Block Flash ROM Parts */
     if(Bank[0].pFlashDesc->ChipSectorSize[Bank[0].pFlashDesc->ChipNumSectors-1] ==
		Bank[0].pFlashDesc->ChipSectorSize[Bank[0].pFlashDesc->ChipNumSectors-2])
     {
       pFlashInfo->ConfigBlockSize	= (((u_int32) Bank[0].pFlashDesc->ChipSectorSize[Bank[0].pFlashDesc->ChipNumSectors-1]<<12) * NumChips) << 1;
       pFlashInfo->ConfigBlockStart	= (u_int8 *) ((u_int32) Bank[0].BankBase +
		Bank[0].BankSize -
		((((u_int32)Bank[0].pFlashDesc->ChipSectorSize[Bank[0].pFlashDesc->ChipNumSectors-1]<<12) * NumChips)<<1));
     }
     else if ((Bank[0].pFlashDesc->DeviceID==0x22B9) || (Bank[0].pFlashDesc->DeviceID==0x22D7))
     {
       #if (FLASH_CONFIG_DATA_SIZE == 0)
       pFlashInfo->ConfigBlockSize	= (((u_int32) Bank[0].pFlashDesc->ChipSectorSize[Bank[0].pFlashDesc->ChipNumSectors-1]<<12) * NumChips) << 1;
       #else
		 pFlashInfo->ConfigBlockSize	= FLASH_CONFIG_DATA_SIZE;
		 #endif
		 
		 #if (FLASH_CONFIG_DATA_START_ADDR == 0)
       pFlashInfo->ConfigBlockStart	= (u_int8 *) ((u_int32) Bank[0].BankBase + Bank[0].BankSize -
		 ((((u_int32)Bank[0].pFlashDesc->ChipSectorSize[Bank[0].pFlashDesc->ChipNumSectors-1]<<12) * NumChips)<<1));
       #else
		 pFlashInfo->ConfigBlockStart	= (u_int8 *)FLASH_CONFIG_DATA_START_ADDR;
		 #endif
     }
     else
     {
       pFlashInfo->ConfigBlockSize	= (u_int32)  0x0;
       pFlashInfo->ConfigBlockStart	= (u_int8 *) 0x0;
       rc = FALSE;
     }
     for(i=Bank[0].pFlashDesc->ChipNumSectors-1, size=0; i; i--)
     {
       size += ((u_int32)Bank[0].pFlashDesc->ChipSectorSize[i]<<12);
       if(Bank[0].pFlashDesc->ChipSectorSize[i] == Bank[0].pFlashDesc->ChipSectorSize[0])
         break;
     }

     if((i>3)&&(Bank[0].pFlashDesc->ChipSectorSize[i] == Bank[0].pFlashDesc->ChipSectorSize[i-1]))
     {
       pFlashInfo->FsBlockSize		= (((u_int32)Bank[0].pFlashDesc->ChipSectorSize[0]<<12)
						* NumChips) << 1;
       pFlashInfo->FsBlockStart		= (u_int8 *) ((u_int32) Bank[0].BankBase +
		Bank[0].BankSize -
		(size + ((((u_int32)Bank[0].pFlashDesc->ChipSectorSize[i]<<12) * NumChips) << 1)));
     }
     else
     {
       pFlashInfo->FsBlockSize		= (u_int32)  0x0;
       pFlashInfo->FsBlockStart		= (u_int8 *) 0x0;
       rc = FALSE;
     }
     pFlashInfo->CodeBlockStart		= (u_int8 *) pFlashInfo->BootBlockStart +
							pFlashInfo->BootBlockSize;
   }

   }

   /* Ensure CodeBlockStart is on a 64K boundary or codeldr will not load the image! */
   pFlashInfo->CodeBlockStart		= (u_int8 *)
	(((u_int32)pFlashInfo->CodeBlockStart+0x10000-1)&(~(0x10000-1)));
   pFlashInfo->CodeBlockSize		= (u_int32) pFlashInfo->FsBlockStart -
							(u_int32) pFlashInfo->CodeBlockStart;

   if(!rc)
   {
     return (rc);
   }

   /* Setup FlashInfo.RomInfo array */
   for(i=0; i<NumROMBanks; i++)
   {
     pFlashInfo->RomInfo[i].Flags      = Bank[i].Flags;
     pFlashInfo->RomInfo[i].BankBase   = (u_int8 *)Bank[i].BankBase;
     pFlashInfo->RomInfo[i].BankSize   = Bank[i].BankSize;
     pFlashInfo->RomInfo[i].NumSectors = Bank[i].pFlashDesc->ChipNumSectors;
     if(pFlashInfo->RomInfo[i].NumSectors>MAX_NUM_SECTORS_PER_BANK)
     {
       return (FALSE);
     }
     if(Bank[0].Flags & FLASH_FLAGS_UNIFORM_SECTOR_SIZE)
     {
         pFlashInfo->RomInfo[i].Sector[0].Start = pFlashInfo->RomInfo[i].BankBase;
         pFlashInfo->RomInfo[i].Sector[0].Length = (u_int32) Bank[0].pFlashDesc->ChipSectorSize << 12;
         pFlashInfo->RomInfo[i].Sector[1].Start = 0;
         pFlashInfo->RomInfo[i].Sector[1].Length = 0;
         pFlashInfo->RomInfo[i].NumSectors = Bank[i].BankSize /
			((u_int32) Bank[i].pFlashDesc->ChipSectorSize << 12);
     }
     else
     {

     for(j=0; j<pFlashInfo->RomInfo[i].NumSectors; j++)
     {
       if(j)
       {
         pFlashInfo->RomInfo[i].Sector[j].Start = pFlashInfo->RomInfo[i].Sector[j-1].Start +
                                      pFlashInfo->RomInfo[i].Sector[j-1].Length;
       }
       else
       {
         pFlashInfo->RomInfo[i].Sector[j].Start = pFlashInfo->RomInfo[i].BankBase;
       }
       pFlashInfo->RomInfo[i].Sector[j].Length =
		((u_int32)Bank[i].pFlashDesc->ChipSectorSize[j]<<12) *
        	(Bank[i].BankWidth / Bank[i].ChipWidth);
     }
     }
     if((!pFlashInfo->RunningFromBank) &&
        (((u_int32)&write_flash >= Bank[i].BankBase) &&
         ((u_int32)&write_flash < (Bank[i].BankBase + Bank[i].BankSize))))
     {
       pFlashInfo->RunningFromBank = (i+1);
     }
   }
   return (rc);
}

/****************************************************/
/* Generic function to read flash memory, any type. */
/****************************************************/
int read_flash(void *addr, int count, void *buffer)
{
   extern bool  gbNeedmoreHasNandFlash;
   if(gbNeedmoreHasNandFlash && (CurrentBank==1))
   {
      int rc = 0;
      /* Need to do it differently here for Nand Flash */
      if( Bank[CurrentBank].pNandFlashDesc)
      {
         rc = read_flash_array[CurrentBank](addr, buffer, count);
         if(rc)
         {   
            count = 0;
         }
      }
   }
   else
   {
      /* For NOR Flash */
      ENTER_READ_MODE();
      FCopy(buffer, addr, count);
   }
   return count;
}

/***************************************************************************************/
/* This function finds out the width of the ROM and the type of the flash that's used. */
/* Returns: TRUE if successful, FALSE if failed                                        */
/***************************************************************************************/
bool InitFlashInfo(void)
{                
    int BankNum;

    if(!flash_low_lev_init_done)
    {
      for (BankNum = 0; BankNum < MAX_NUM_FLASH_ROM_BANKS; BankNum++)
      {  
        if (Bank[BankNum].BankWidth != 0)                                     
        {
            switch (Bank[BankNum].pFlashDesc->MfrID)
            {                                   
                /*
                 ST uses both types... :-(
                */
                case FLASH_CHIP_VENDOR_ST:
                    switch (Bank[BankNum].pFlashDesc->DeviceID)
                    {
                        case 0x0016:
                            write_flash_array[BankNum] = write_intel;
                            erase_flash_array[BankNum] = erase_intel;
                            erase_flash_bank[BankNum]  = erase_intel_bank;
                            break;
        
                        case 0x2249:
                        default:
                            write_flash_array[BankNum] = write_amd;
                            erase_flash_array[BankNum] = erase_amd;
                            erase_flash_bank[BankNum]  = erase_amd_bank;
                            break;
                    }
                    break;

                /*
                 Intel has a different write/erase sequence
                 than everyone else in the industry...
                */
                case FLASH_CHIP_VENDOR_INTEL:
                    write_flash_array[BankNum] = write_intel;
                    erase_flash_array[BankNum] = erase_intel;
                    erase_flash_bank[BankNum]  = erase_intel_bank;
                    break;

                /*
                 Everyone else uses the "AMD" write/erase
                 sequence...
                */
                case FLASH_CHIP_VENDOR_AMD:
                case FLASH_CHIP_VENDOR_FUJITSU:
                case FLASH_CHIP_VENDOR_ATMEL:
                case FLASH_CHIP_VENDOR_SST:
                default:
                    write_flash_array[BankNum] = write_amd;
                    erase_flash_array[BankNum] = erase_amd;
                    erase_flash_bank[BankNum]  = erase_amd_bank;
                    break;

            }
            if(Bank[BankNum].pNandFlashDesc)
            {
               /* Handle the Nand Flash devices here */
               switch (Bank[BankNum].pNandFlashDesc->MfrID)
               {
                   case FLASH_CHIP_VENDOR_SAMSUNG:
                       write_flash_array[BankNum] = write_sam_nandflash;
                       read_flash_array[BankNum] = read_sam_nandflash;
                       erase_flash_array[BankNum] = erase_samnand;
                       erase_flash_bank[BankNum]  = erase_samnand_bank;
                       break;
                   default:
                       write_flash_array[BankNum] = write_sam_nandflash;
                       read_flash_array[BankNum] = read_sam_nandflash;
                       erase_flash_array[BankNum] = erase_samnand;
                       erase_flash_bank[BankNum]  = erase_samnand_bank;
                       break;
               } /* end of switch */
            } /* end of if */
         }
      }
    
      /* Setup global FlashInfo structure */
      GetFlashInfo(&FlashInfo);

      /* Set flag indicating low level init has been done. */
      flash_low_lev_init_done = TRUE;
    }
    
    return(TRUE);
}

/*****************************************************************************************/
/* This is the low level flash read function.                                            */
/* Return value:     number of bytes copied in case of success (should be equal to count */
/*                   other value in case of failure                                      */
/*****************************************************************************************/
int data_read(void *addr, int count, void *buffer)
{
    int result_count;

#if !defined(DLOAD) && (RTOS != NOOS)
    if(bKalUp)
    {
        if(!flash_access_sem)
        {
            if((flash_access_sem = sem_create(1, "FLSM")) == 0)
            {
                return (0);
            }
        }
        if((sem_get(flash_access_sem, KAL_WAIT_FOREVER))!=RC_OK)
        {
            return(0);
        }
    }
#endif
    
    /* Set the current bank according to the addr. */
    FindCurrentBank(addr);

    result_count = read_flash(addr, count, buffer);

#if !defined(DLOAD) && (RTOS != NOOS)
    if (bKalUp)
    {
        sem_put(flash_access_sem);
    }
#endif

    return result_count;       
}

#if DOWNLOAD_SERIAL_SUPPORT==YES
/*****************************************************************************************/
/*  This procedure informs the user that the boot sector is about to be overwritten.     */
/*  Then it asks for confirmation of the user's intent and returns the result as         */
/*    TRUE  - permission granted                                                         */
/*    FALSE - permission denied                                                          */
/*****************************************************************************************/
bool GetPermissionToWriteBootSector(void)
{
   char cChoice;

   PermissionToWriteBootSector = FALSE;

   printf("\n\nDANGER!! If you continue, you will overwrite the boot block sector.\n");
   printf("What do you want to do?\n");
   printf("   1. Quit     - This will preserve your boot block as it is.\n");
   printf("   2. Continue - This will overwrite the current boot block contents.\n");
   
   /* Wait for the user to type a valid number */
   cChoice = GetUserChoice(8);
   
   if (cChoice == '2')
   {
      printf("\n\nYou have chosen to overwrite the boot block contents.\n");
      printf("This will destroy the current codeldr program.\n");
      printf("Are you sure you want to do this?\n");
      printf("  1. YES\n");
      printf("  2. NO\n");
      
      /* Wait for the user to type a valid number */
      cChoice = GetUserChoice(8);
      
      if (cChoice == '1')
      {
         PermissionToWriteBootSector = TRUE;
      }
   }
   return PermissionToWriteBootSector;
}
#endif

/*****************************************************************************************/
/* This is the low level flash write function.  If the flash is 16-bit wide,             */
/* the destination address has to be half-word-aligned.  If the flash is 32-bit wide,    */
/* the destination address will have to word-aligned. So the function parameter addr has */
/* to be checked for hword or word alignment if necessary, before flash write happens.   */
/* Return value:      number of bytes written to flash ( should be equal to count )      */
/*                    if return value not equal to count, it indicates failure           */  
/*****************************************************************************************/
int data_write(void *addr, int count, void *buffer)
{                     
    int result, i, num_end_bytes, bytes_to_write, bytes_written;
    u_int8 start_data[4], end_data[4], *ptr;
    u_int32 offset_mask;
#ifndef DLOAD
    bool rom_data_cache_enabled;
#endif

#if DOWNLOAD_SERIAL_SUPPORT==YES
    if (!PermissionToWriteBootSector && ((u_int32)addr < (u_int32)FirstCodeAddr))
    {
       if (!GetPermissionToWriteBootSector())
          return 0;
    }
#endif

#if !defined(DLOAD) && (RTOS != NOOS)
    if (bKalUp)
    {
        if(!flash_access_sem)
        {
            if((flash_access_sem = sem_create(1, "FLSM")) == 0)
            {
                return (0);
            }
        }
        if((sem_get(flash_access_sem, KAL_WAIT_FOREVER))!=RC_OK)
        {
            return(0);
        }
    }
#endif

    /* Initialize some variables. */
    bytes_written = 0;
    FFillBytes(start_data, 0xff, 4);
    FFillBytes(end_data, 0xff, 4);
    ptr = (u_int8 *) buffer;

    /* Set the current bank according to the addr. */
    FindCurrentBank(addr);          
    
#ifndef DLOAD
    /*
     Must ensure the ROM Data Cache is disabled!!!
    */
    rom_data_cache_enabled = FlashDisableDCache(CurrentBank,
            Bank[CurrentBank].BankBase,
            (unsigned int)(Bank[CurrentBank].BankSize>>20) );
#endif

    /* Set up a mask we will use to determine the misalignment. */
    offset_mask = 0xf >> (4 - (Bank[CurrentBank].BankWidth >> 4));
    
    /* Fill the start_data buffer with the current flash data from the */
    /* last alignment to the next alignment.                           */
    read_flash((void *)((u_int32)addr & ~offset_mask), 
               Bank[CurrentBank].BankWidth >> 3, 
               (void *)start_data);

    /* Fill the start_data buffer beginning at the misalignment to the */
    /* next alignment with the data to be written to flash.            */
    for (i = (u_int32)addr & offset_mask; 
         i < (Bank[CurrentBank].BankWidth >> 3) && i != 0 && bytes_written < count;
         i++, bytes_written++) 
    {
        start_data[i] = *ptr++;
    }
    bytes_to_write = (u_int32)addr & offset_mask ? 
                     (Bank[CurrentBank].BankWidth >> 3) :
                     0;

    /* Write the start_data buffer to flash and advance the flash */
    /* address ptr to the next alignment.                         */
    result = write_flash((void *)((u_int32)addr & ~offset_mask),
                             (void *)start_data,
                             bytes_to_write);
    if(result != bytes_to_write)
    {
	goto data_write_bail;
    }
    addr = (void *)(((u_int32)addr & ~offset_mask) + bytes_to_write);
    bytes_to_write = count - bytes_written;
    
    /* Now write the contents of buffer to the subsequent aligned   */
    /* addresses in the flash until we no longer have enough data   */
    /* in buffer to complete an aligned write to flash.             */
    num_end_bytes = bytes_to_write & offset_mask;
    result = write_flash(addr, ptr, (bytes_to_write - num_end_bytes));
    if(result != (bytes_to_write - num_end_bytes))
    {
	goto data_write_bail;
    }
    bytes_written += result;
    ptr += (bytes_to_write-num_end_bytes);
    
    if(bytes_written < count)
    {
        /* Fill the end_data buffer with the current flash data from the */
        /* current alignment to the next alignment.                      */
        read_flash((void *)((int)addr + bytes_to_write - num_end_bytes), 
                   Bank[CurrentBank].BankWidth >> 3, 
                   (void *)end_data);
        
        /* Fill the end_data buffer beginning at the alignment to the */
        /* end of the data to be written to flash.                    */
        addr = (void *)((int)addr + (bytes_to_write - num_end_bytes));
        for (i = 0; i < num_end_bytes; i++) 
        {
            end_data[i] = *ptr++;
        }
        /* Write the last bytes of unaligned data to flash. */
        if (num_end_bytes != 0)
        {
            result = write_flash(addr, end_data, (Bank[CurrentBank].BankWidth >> 3));
            if(result != (Bank[CurrentBank].BankWidth >> 3))
	    {
	        goto data_write_bail;
	    }
            bytes_written += result;
        }
    }
    
data_write_bail:

#ifndef DLOAD
    /*
     Re-enable the ROM Data Cache if it was enabled upon entry
    */
    if(rom_data_cache_enabled)
    {
        FlashEnableDCache(CurrentBank,
                Bank[CurrentBank].BankBase,
                (unsigned int)(Bank[CurrentBank].BankSize>>20) );
    }
#endif

#if !defined(DLOAD) && (RTOS != NOOS)
    if (bKalUp)
    {
        sem_put(flash_access_sem);
    }
#endif

    return (bytes_written >= count) ? count : bytes_written;
    
}

/*****************************************************************************************/
/* This is the low level flash erase sector function.                                    */
/*****************************************************************************************/
int block_erase(void *addr, int block_size)
{
    int result=0;
#ifndef DLOAD
    bool rom_data_cache_enabled;
#endif
    
#if !defined(DLOAD) && (RTOS != NOOS)
    /*
     If running OS, ensure exclusive access
    */
    if (bKalUp)
    {
        if(!flash_access_sem)
        {
            if((flash_access_sem = sem_create(1, "FLSM")) == 0)
            {
                return (0);
            }
        }
        if((sem_get(flash_access_sem, KAL_WAIT_FOREVER))!=RC_OK)
        {
            return(-1);
        }
    }
#endif

    /*
     Set the current bank according to the addr
    */
    FindCurrentBank(addr);
     
    /*
     If the area is already erased, don't erase it again
    */
    if(block_check_nosem(addr, block_size)==FALSE)
    {

#ifndef DLOAD
        /*
         Must ensure the ROM Data Cache is disabled!!!
        */
        rom_data_cache_enabled = FlashDisableDCache(CurrentBank,
                Bank[CurrentBank].BankBase,
                (unsigned int)(Bank[CurrentBank].BankSize>>20) );
#endif

        /*
         Erase the sector containing addr
        */
        result = erase_flash(addr, NULL);  

#ifndef DLOAD
        /*
         Re-enable the ROM Data Cache if it was enabled upon entry
        */
        if(rom_data_cache_enabled)
        {
            FlashEnableDCache(CurrentBank,
                    Bank[CurrentBank].BankBase,
                    (unsigned int)(Bank[CurrentBank].BankSize>>20) );
        }
#endif

    }

#if !defined(DLOAD) && (RTOS != NOOS)
    /*
     If running OS, release exclusive access
    */
    if (bKalUp)
    {
        sem_put(flash_access_sem);
    }
#endif

   return result;
}

/*****************************************************************************************/
/* This is the low level flash erase sector function with callback function.             */
/*****************************************************************************************/
int block_erase_with_callback(void *addr, int block_size, void (*callback)(void))
{
    int result=0;
#ifndef DLOAD
    bool rom_data_cache_enabled;
#endif
    
#if DOWNLOAD_SERIAL_SUPPORT==YES
    if (!PermissionToWriteBootSector && ((u_int32)addr < (u_int32)FirstCodeAddr))
    {
       if (!GetPermissionToWriteBootSector())
          return FAILURE;
    }
#endif

#if !defined(DLOAD) && (RTOS != NOOS)
    /*
     If running OS, ensure exclusive access
    */
    if (bKalUp)
    {
        if(!flash_access_sem)
        {
            if((flash_access_sem = sem_create(1, "FLSM")) == 0)
            {
                return (0);
            }
        }
        if((sem_get(flash_access_sem, KAL_WAIT_FOREVER))!=RC_OK)
        {
            return(-1);
        }
    }
#endif

    /*
     Set the current bank according to the addr
    */
    FindCurrentBank(addr);
     
    /*
     If the area is already erased, don't erase it again
    */
    if(block_check_nosem(addr, block_size)==FALSE)
    {
#ifndef DLOAD
        /*
         Must ensure the ROM Data Cache is disabled!!!
        */
        rom_data_cache_enabled = FlashDisableDCache(CurrentBank,
                Bank[CurrentBank].BankBase,
                (unsigned int)(Bank[CurrentBank].BankSize>>20) );
#endif

        /*
         Erase the sector containing addr
        */
        result = erase_flash(addr, callback);  

#ifndef DLOAD
        /*
         Re-enable the ROM Data Cache if it was enabled upon entry
        */
        if(rom_data_cache_enabled)
        {
            FlashEnableDCache(CurrentBank,
                    Bank[CurrentBank].BankBase,
                    (unsigned int)(Bank[CurrentBank].BankSize>>20) );
        }
#endif
    }

#if !defined(DLOAD) && (RTOS != NOOS)
    /*
     If running OS, release exclusive access
    */
    if (bKalUp)
    {
        sem_put(flash_access_sem);
    }
#endif
    
   return result;
}

/*****************************************************************************************/
/* This is the low level flash erase bank function.                                      */
/*****************************************************************************************/
int bank_erase(void *addr)
{
    int result=0;
#ifndef DLOAD
    bool rom_data_cache_enabled;
#endif
    
#if !defined(DLOAD) && (RTOS != NOOS)
    /*
     If running OS, ensure exclusive access
    */
    if (bKalUp)
    {
        if(!flash_access_sem)
        {
            if((flash_access_sem = sem_create(1, "FLSM")) == 0)
            {
                return (0);
            }
        }
        if((sem_get(flash_access_sem, KAL_WAIT_FOREVER))!=RC_OK)
        {
            return(-1);
        }
    }
#endif

    /*
     Set the current bank according to the addr
    */
    FindCurrentBank(addr);
     
    /*
     If the area is already erased, don't erase it again
    */
    if(block_check_nosem(addr, (int)Bank[CurrentBank].BankSize)==FALSE)
    {
#ifndef DLOAD
        /*
         Must ensure the ROM Data Cache is disabled!!!
        */
        rom_data_cache_enabled = FlashDisableDCache(CurrentBank,
                Bank[CurrentBank].BankBase,
                (unsigned int)(Bank[CurrentBank].BankSize>>20) );
#endif

        /*
         Erase the sector containing addr
        */
        result = erase_flash_bank[CurrentBank]((void *)Bank[CurrentBank].BankBase, NULL);

#ifndef DLOAD
        /*
         Re-enable the ROM Data Cache if it was enabled upon entry
        */
        if(rom_data_cache_enabled)
        {
            FlashEnableDCache(CurrentBank,
                    Bank[CurrentBank].BankBase,
                    (unsigned int)(Bank[CurrentBank].BankSize>>20) );
        }
#endif
    }

#if !defined(DLOAD) && (RTOS != NOOS)
    /*
     If running OS, release exclusive access
    */
    if (bKalUp)
    {
        sem_put(flash_access_sem);
    }
#endif
    
   return result;
}

/*****************************************************************************************/
/* This is the low level flash erase bank function with callback function.               */
/*****************************************************************************************/
int bank_erase_with_callback(void *addr, void (*callback)(void))
{
    int result=0;
#ifndef DLOAD
    bool rom_data_cache_enabled;
#endif
    
#if DOWNLOAD_SERIAL_SUPPORT==YES
    if (!PermissionToWriteBootSector && ((u_int32)addr < (u_int32)FirstCodeAddr))
    {
       if (!GetPermissionToWriteBootSector())
          return FAILURE;
    }
#endif

#if !defined(DLOAD) && (RTOS != NOOS)
    /*
     If running OS, ensure exclusive access
    */
    if (bKalUp)
    {
        if(!flash_access_sem)
        {
            if((flash_access_sem = sem_create(1, "FLSM")) == 0)
            {
                return (0);
            }
        }
        if((sem_get(flash_access_sem, KAL_WAIT_FOREVER))!=RC_OK)
        {
            return(-1);
        }
    }
#endif

    /*
     Set the current bank according to the addr
    */
    FindCurrentBank(addr);
     
    /*
     If the area is already erased, don't erase it again
    */
    if(block_check_nosem(addr, (int)Bank[CurrentBank].BankSize)==FALSE)
    {
#ifndef DLOAD
        /*
         Must ensure the ROM Data Cache is disabled!!!
        */
        rom_data_cache_enabled = FlashDisableDCache(CurrentBank,
                Bank[CurrentBank].BankBase,
                (unsigned int)(Bank[CurrentBank].BankSize>>20) );
#endif

        /*
         Erase the sector containing addr
        */
        result = erase_flash_bank[CurrentBank]((void *)Bank[CurrentBank].BankBase, callback);

#ifndef DLOAD
        /*
         Re-enable the ROM Data Cache if it was enabled upon entry
        */
        if(rom_data_cache_enabled)
        {
          FlashEnableDCache(CurrentBank,
                Bank[CurrentBank].BankBase,
                (unsigned int)(Bank[CurrentBank].BankSize>>20) );
        }
#endif

    }

#if !defined(DLOAD) && (RTOS != NOOS)
    /*
     If running OS, release exclusive access
    */
    if (bKalUp)
    {
        sem_put(flash_access_sem);
    }
#endif
    
   return result;
}

/*********************************************************************/
/* This function checks to see if the flash block is all ff's or not */
/* Return Value:        TRUE if block is all ff's                    */
/*                      FALSE if otherwise                           */
/*********************************************************************/
bool block_check_nosem(void *addr, int block_size)
{
    int i;
    bool erased_flag = TRUE;
    volatile u_int8 *dest;

    for(i = 0, dest = (u_int8 *)addr; i < block_size; i++, dest++)
    {       
        if(*dest != 0xff)
        {
            erased_flag = FALSE;                            
            break;
        }
    }

    return (erased_flag);       
}

/*********************************************************************/
/* This is a wrapper for block_check_nosem that claims the driver    */
/* access semaphore before checking the block. This is required since*/
/* block_check function is needed both from other functions at the   */
/* same level (eg. block_erase) and from higher level APIs.          */
/*********************************************************************/
bool block_check(void *addr, int block_size)
{
    bool erased_flag = TRUE;

#if !defined(DLOAD) && (RTOS != NOOS)
    if (bKalUp)
    {
        if(!flash_access_sem)
        {
            if((flash_access_sem = sem_create(1, "FLSM")) == 0)
            {
                return (FALSE);
            }
        }
        if((sem_get(flash_access_sem, KAL_WAIT_FOREVER))!=RC_OK)
        {
            return(FALSE);
        }
    }
#endif

    erased_flag = block_check_nosem(addr, block_size);
     
#if !defined(DLOAD) && (RTOS != NOOS)
    if (bKalUp)
    {
        sem_put(flash_access_sem);
    }
#endif

    return (erased_flag);       
}

int write_flash(void *addr, void *buffer, int size)
{
    int	rc;

#ifndef DLOAD
    /* Are we running from ROM and the same bank we're flashing? */
    if(((u_int32)&write_flash >= Bank[CurrentBank].BankBase) &&
       ((u_int32)&write_flash < (Bank[CurrentBank].BankBase +
                                 Bank[CurrentBank].BankSize)))
    {
        /* Running from ROM and the same bank we're flashing to */
        rc = CopyAndCallRoutine((u_int8 *)write_flash_array[CurrentBank],
		(u_int32) addr, (u_int32) buffer, (u_int32) size);
    }
    else
    {
        /* Either running from RAM or not running from the same ROM
          bank we're flashing to
		*/
        rc = write_flash_array[CurrentBank](addr, buffer, size);
    }
#else /* DLOAD */
    /* For the flash download utility, we just call the function */
    rc = write_flash_array[CurrentBank](addr, buffer, size);
#endif /* !DLOAD */
    return (rc);
}

static int erase_flash(void *addr, void (*callback)(void))
{
    int	rc;

#ifndef DLOAD
    /* Are we running from ROM and the same bank we're flashing? */
    if(((u_int32)&write_flash >= Bank[CurrentBank].BankBase) &&
       ((u_int32)&write_flash < (Bank[CurrentBank].BankBase +
                                 Bank[CurrentBank].BankSize)))
    {
        /* Running from ROM and the same bank we're flashing to */
        rc = CopyAndCallRoutine ((u_int8 *)erase_flash_array[CurrentBank],
                                 (u_int32) addr, (u_int32) callback, (u_int32) NULL);
    }
    else
    {
        /* Either running from RAM or not running from the same ROM */
        /* bank we're flashing to                                   */
        rc = erase_flash_array[CurrentBank](addr, callback);
    }
#else /* DLOAD */
    /* For the flash download utility, we just call the function */
    rc = erase_flash_array[CurrentBank](addr, callback);
#endif /* !DLOAD */
    return (rc);
}

#ifndef DLOAD

#define TMP_STACK_SIZE		0x0100

int CopyAndCallRoutine(u_int8 *func_addr, u_int32 d0, u_int32 d1, u_int32 d2)
{
  u_int8        *area, *src, *dst, *addr;
  u_int32       size, i;
#if RTOS!=VXWORKS && RTOS!=NOOS
  u_int32       cs;
#endif /* RTOS!=VXWORKS */
  int           rc;   /* THIS HAS TO BE THE LAST LOCAL VARIABLE */

  addr = (u_int8*)((u_int32)func_addr & ~0x1);
  size = FLASH_ROMS_COPY_CODE_SIZE;
  area = (u_int8 *) &rc;	/* Get current stack ptr...  SEE NOTE ABOVE */
  area = dst = (u_int8 *)((u_int32)area - size - TMP_STACK_SIZE - 4);
  src = (u_int8 *)addr;
  for(i=0;i<size;i++)
  {
     *dst++ = *src++;
  }

#if RTOS==VXWORKS
  /* Warning: it is not clear from VxW docs if interrupts are locked out... */
  cacheFlush(DATA_CACHE, NULL, ENTIRE_CACHE);
  cachePipeFlush();
  cacheInvalidate(DATA_CACHE, NULL, ENTIRE_CACHE);
  cacheInvalidate(INSTRUCTION_CACHE, NULL, ENTIRE_CACHE);
#else
 #if RTOS!=NOOS
  cs = critical_section_begin( );
 #endif
  CleanDCache();      /* Flush DCache contents to Memory */
  DrainWriteBuffer();
  FlushDCache();      /* Invalidate both caches */
  FlushICache();
 #if RTOS!=NOOS
  critical_section_end( cs );
 #endif
#endif

  rc = CallFuncByAddrWithIntrsDisabled (area, d0, d1, d2);
  return (rc);
}


#endif /* !DLOAD */

/****************************************************************************
 * Modifications:
 * $Log: 
 *  112  mpeg      1.111       6/29/04 11:00:39 PM    Xiao Guang Yan  CR(s) 
 *        9622 9623 : Changed flash config data start address and size to 
 *        configurable.
 *  111  mpeg      1.110       4/22/04 4:16:59 PM     Sunil Cheruvu   CR(s) 
 *        8870 8871 : Added the Nand Flash support for Wabash(Milano rev 5 and 
 *        above) and Brazos.
 *  110  mpeg      1.109       10/17/03 9:35:21 AM    Larry Wang      CR(s): 
 *        7673 Remove double log.
 *  109  mpeg      1.108       10/17/03 9:28:41 AM    Larry Wang      CR(s): 
 *        7673 Replace memcpy and memset with FCopy and FFillBytes.
 *  108  mpeg      1.107       9/23/03 6:23:26 PM     Miles Bintz     SCR(s) 
 *        7523 :
 *        added support for serial and lauterbach flash downloads
 *        
 *  107  mpeg      1.106       9/22/03 6:24:14 PM     Miles Bintz     SCR(s) 
 *        7523 :
 *        changed SERIAL_DOWNLOAD_SUPPORT to new SWCONFIG defintino of 
 *        DOWNLOAD_SERIAL_SUPPRT
 *        
 *        
 *  106  mpeg      1.105       7/9/03 3:27:06 PM      Tim White       SCR(s) 
 *        6901 :
 *        Phase 3 codeldrext drop.
 *        
 *        
 *  105  mpeg      1.104       6/24/03 6:34:10 PM     Tim White       SCR(s) 
 *        6831 :
 *        Add flash, hsdp, demux, OSD, and demod support to codeldrext
 *        
 *        
 *  104  mpeg      1.103       6/5/03 5:23:16 PM      Tim White       SCR(s) 
 *        6660 6661 :
 *        Flash banks separately controlled by the 920 MMU using hardware 
 *        virtual pagemapping.
 *        
 *        
 *  103  mpeg      1.102       5/14/03 4:12:52 PM     Tim White       SCR(s) 
 *        6346 6347 :
 *        Moved Bank[] array into startup/rom.c.
 *        
 *        
 *  102  mpeg      1.101       4/30/03 5:56:56 PM     Billy Jackman   SCR(s) 
 *        6113 :
 *        Modified code to call FlashDisable/EnableDCache (defined in 
 *        startupc.c) to
 *        eliminate the need for many blocks of conditional code.  Removed 
 *        references
 *        that are no longer needed because of removing the conditional code.
 *        Removed some vendor-specific code.
 *        Eliminated double slash comments.
 *        
 *  101  mpeg      1.100       4/4/03 9:21:50 AM      Craig Dry       SCR(s) 
 *        5908 :
 *        Warn user multiple times before overwriting the flash boot area
 *        
 *  100  mpeg      1.99        3/5/03 5:19:24 PM      Tim White       SCR(s) 
 *        5681 5682 :
 *        Add support for ST M58LW032D flash type.
 *        
 *        
 *  99   mpeg      1.98        2/13/03 11:48:50 AM    Matt Korte      SCR(s) 
 *        5479 :
 *        Removed old header reference
 *        
 *  98   mpeg      1.97        2/5/03 6:21:22 PM      Tim Ross        SCR(s) 
 *        5227 :
 *        Removed a #error statement that was temporarily used for debug that 
 *        was breaking all VXWORKS builds.
 *        
 *  97   mpeg      1.96        2/5/03 5:55:08 PM      Miles Bintz     SCR(s) 
 *        5227 :
 *        Added stbcfg.h before other includes so RTOS is defined correctly
 *        
 *        
 *  96   mpeg      1.95        2/5/03 12:37:44 PM     Miles Bintz     SCR(s) 
 *        5227 :
 *        changed order of include files so that max isn't redefined
 *        
 *  95   mpeg      1.94        1/31/03 5:33:50 PM     Dave Moore      SCR(s) 
 *        5375 :
 *        Removed FlushCachesC() and replaced with critical section
 *        around the equiv Flush / Drain / Invalidate routines. Replaced
 *        all the double slash comments with std 'C' slash star comments.
 *        
 *        
 *  94   mpeg      1.93        4/10/02 5:05:18 PM     Tim White       SCR(s) 
 *        3509 :
 *        Eradicate external bus (PCI) bitfield usage.
 *        
 *        
 *  93   mpeg      1.92        4/5/02 6:24:56 PM      Dave Moore      SCR(s) 
 *        3458 :
 *        minor 920t cleanup
 *        
 *        
 *  92   mpeg      1.91        4/1/02 11:34:58 AM     Dave Moore      SCR(s) 
 *        3457 :
 *        Added MMU_CACHE_DISABLE check to all ARM920 ifdefs
 *        
 *        
 *  91   mpeg      1.90        4/1/02 8:43:06 AM      Dave Moore      SCR(s) 
 *        3457 :
 *        added 920T support
 *        
 *        
 *  90   mpeg      1.89        6/26/01 1:09:16 PM     Bobby Bradford  SCR(s) 
 *        2160 :
 *        See Tracker #2160 for details.
 *        
 *  89   mpeg      1.88        4/12/01 7:24:20 PM     Amy Pratt       DCS914 
 *        Removed Neches support.
 *        
 *  88   mpeg      1.87        4/4/01 6:54:56 PM      Joe Kroesche    #1636 - 
 *        changed local function erase_flash to static to prevent namespace
 *        collision with new opentv en2 libs.  This function was not referenced
 *         from
 *        any other source that I could see.
 *        
 *  87   mpeg      1.86        2/16/01 5:22:08 PM     Tim White       DCS#1248:
 *         Fixed problem with bank_erase for VENDOR_A & VENDOR_D.
 *        
 *  86   mpeg      1.85        2/15/01 5:36:46 PM     Tim White       DCS#1230:
 *         Globalize MemClkPeriod for use by the Soft Modem code.
 *        
 *  85   mpeg      1.84        2/12/01 2:28:58 PM     Tim White       DCS#1187:
 *         Build problem with download tool for VENDOR_A && VENDOR_D.
 *        
 *  84   mpeg      1.83        2/1/01 5:42:40 PM      Tim White       DCS#1115:
 *         Data abort for prereading past the end of the data buffer in 
 *        data_write().
 *        
 *  83   mpeg      1.82        1/31/01 2:33:40 PM     Tim White       DCS#0988:
 *         Reclaim footprint space.
 *        
 *  82   mpeg      1.81        8/30/00 11:54:40 AM    Tim White       Fixed 
 *        VxWorks compilation warnings.
 *        
 *  81   mpeg      1.80        6/21/00 1:45:06 PM     Dave Wilson     Fixed 
 *        semaphoring on block_check to prevent other block_xxx APIs which call
 *        the function from hanging.
 *        
 *  80   mpeg      1.79        6/21/00 10:18:02 AM    Ismail Mustafa  Fixed 
 *        syntax error (extra bracket removed).
 *        
 *  79   mpeg      1.78        6/20/00 5:27:32 PM     Tim White       Fix 
 *        VxWorks compiler problem.
 *        
 *  78   mpeg      1.77        6/12/00 9:34:58 AM     Ray Mack        undo 
 *        error in version 1.76
 *        
 *  77   mpeg      1.76        6/11/00 6:22:36 PM     Ray Mack        changes 
 *        to eliminate compiler warnings
 *        
 *  76   mpeg      1.75        5/17/00 12:22:40 PM    Tim White       Added 
 *        support for SST ROM's.
 *        
 *  75   mpeg      1.74        4/10/00 4:25:50 PM     Tim White       Added 
 *        several flash types to main codebase.
 *        
 *  74   mpeg      1.73        4/6/00 10:00:12 AM     Ray Mack        fixes to 
 *        remove warnings
 *        
 *  73   mpeg      1.72        4/3/00 6:49:16 PM      Tim White       Fixed bug
 *         with non-aligned writes that has been there since day 1.  The 
 *        minidriver
 *        testing for Samsung found this bug.
 *        
 *  72   mpeg      1.71        3/8/00 5:18:06 PM      Tim White       
 *        Restructured the BANK_INFO array and added support for new Intel 
 *        Flash ROM.
 *        
 *  71   mpeg      1.70        3/2/00 5:47:10 PM      Tim White       Allow the
 *         low-level flash functions to work on Neches and Colorado taking
 *        into account the difference in ROM bank cache settings and whether to
 *         copy
 *        to RAM before running if running from ROM depending on which bank is 
 *        being
 *        flashed, which bank (if running from ROM) of ROM the code is running 
 *        from
 *        and whether it's a Neches-based chip or Colorado (and follow-ons) 
 *        based.
 *        
 *  70   mpeg      1.69        3/1/00 1:35:18 PM      Tim Ross        Enabled 
 *        flashing while executing from ROM w/out first doing a copy
 *        of the flashing code to RAM.
 *        
 *        Replaced ENTER_READ/WRITE_MODE macros with function
 *        calls toi EnterRead/WriteMode().
 *        
 *  69   mpeg      1.68        12/9/99 4:03:06 PM     Tim White       Reenable 
 *        ROM Data Cache.
 *        
 *  68   mpeg      1.67        12/8/99 7:04:44 PM     Tim White       Moved 
 *        flash_access_sem from finit.c to flashrw.c where it belongs.
 *        
 *  67   mpeg      1.66        11/11/99 2:57:36 PM    Tim White       Since ROM
 *         data caching is no longer enabled (only instruction caching is
 *        enabled), there is no longer any need to call DisableROMDataCache() 
 *        and
 *        EnableROMDataCache().
 *        
 *  66   mpeg      1.65        10/30/99 12:47:52 PM   Tim White       Removed 
 *        the need for the __asm inline for VxWorks port.
 *        
 *  65   mpeg      1.64        10/27/99 4:57:00 PM    Dave Wilson     Changed 
 *        WAIT_FOREVER to KAL_WAIT_FOREVER
 *        
 *  64   mpeg      1.63        10/19/99 2:21:24 PM    Tim White       Fixed 
 *        minor bug with not having a 64K aligned FlashInfo->CodeBlockStart 
 *        address
 *        which is required for CodeLdr to load an image from flash.
 *        
 *  63   mpeg      1.62        10/13/99 4:43:42 PM    Tim White       Added 
 *        FlashInfo into low-level routines.  Run-from-ROM code can now
 *        use the low-level flashrw() routines.
 *        
 *  62   mpeg      1.61        9/24/99 4:46:12 PM     Tim White       Added 
 *        block_erase_with_callback() and bank_erase_with_callback() in order 
 *        to leave
 *        block_erase() and bank_erase() intact.
 *        
 *  61   mpeg      1.60        9/24/99 1:51:30 PM     Tim White       Added 
 *        erase by bank, fixed several bugs, added callback function to erase 
 *        functions
 *        
 *  60   mpeg      1.59        9/10/99 6:36:00 PM     Tim Ross        Modified 
 *        write_amd(), write_intel(), and data_write to correct write 
 *        alignment problems.
 *        
 *  59   mpeg      1.58        9/8/99 6:18:34 PM      Tim Ross        Corrected
 *         number of bytes written that is returned by data_write.
 *        
 *  58   mpeg      1.57        9/8/99 3:33:42 PM      Tim Ross        Changed 
 *        FLASH_READ/WRITE to FLASH_CMD_READ/WRITE
 *        to correct build problem. 
 *        Added init flag to ensure data is retrieved from STARTUP when
 *        low-level operation calls are made.
 *        
 *  57   mpeg      1.56        9/7/99 10:56:22 PM     Tim Ross        Rewrote 
 *        almost entire file to be able to properly handle all 
 *        combinations of 8, 16, and 32-bit banks as well as 8, and 16-bit 
 *        flash chips widths.
 *        
 *  56   mpeg      1.55        8/19/99 4:14:54 PM     Tim Ross        Corrected
 *         write_bankwidth_xx algorithm that checks when flash
 *        chips are done writing. 
 *        
 *  55   mpeg      1.54        8/16/99 4:31:16 PM     Anzhi Chen      Fixed the
 *         warnings.
 *        
 *  54   mpeg      1.53        8/16/99 4:23:18 PM     Anzhi Chen      Include 
 *        "kal.h" only for non-DLOAD build.
 *        
 *  53   mpeg      1.52        8/16/99 4:20:14 PM     Dave Wilson     Added 
 *        lpCurrentFlash to allow external modules to access the flash 
 *        descriptor for the device currently in use.
 *        
 *  52   mpeg      1.51        8/11/99 5:59:52 PM     Dave Wilson     Changed 
 *        all KAL calls to use new API.
 *        
 *  51   mpeg      1.50        8/11/99 5:21:48 PM     Tim Ross        Removed  
 *        incorrect write of PLL_ROM_MODE_REG at end of data_write
 *        and data_read.
 *        
 *  50   mpeg      1.49        8/10/99 3:03:18 PM     Dave Wilson     Made sure
 *         that all exit paths from data_write cleared the write mode
 *        bit in the PCI_ROM_MODE_REG.
 *        
 *  49   mpeg      1.48        8/6/99 11:36:20 AM     Tim Ross        Rewrote 
 *        status check algorithm in all erase_bankwidth_xx and 
 *        write_bankwidth_xx routines.
 *        Corrected data_write() routine to properly turn on flash program 
 *        bit in ROM Mode register.
 *        
 *  48   mpeg      1.47        6/18/99 12:39:50 PM    Anzhi Chen      Passed 
 *        more info in GetFlashInfo().
 *        
 *  47   mpeg      1.46        6/1/99 1:59:00 PM      Anzhi Chen      Fixed a 
 *        bug that defined BandStartAddr as u_int32 * when should be u_int8 *.
 *        
 *  46   mpeg      1.45        5/28/99 11:27:34 AM    Anzhi Chen      Added 
 *        code into write and erase functions to support 2nd bank of flash.
 *        
 *  45   mpeg      1.44        5/26/99 5:37:46 PM     Anzhi Chen      
 *        Simplified the read functions to simply calling memcpy().
 *        
 *  44   mpeg      1.43        5/25/99 6:12:36 PM     Tim Ross        Replaced 
 *        #include of opentvx.h w/ kal.h.
 *        
 *  43   mpeg      1.42        5/20/99 3:35:18 PM     Anzhi Chen      Changed 
 *        the reference to hwaddr.h to defs.h.
 *        
 *  42   mpeg      1.41        5/19/99 11:08:00 AM    Anzhi Chen      Fixed the
 *         order of the include files.
 *        
 *  41   mpeg      1.40        5/18/99 6:10:12 PM     Anzhi Chen      Fixed a 
 *        bug in block_check() that failed to increment to checking address.
 *        Added lots of DLOAD flag so this file can be shared by the dload 
 *        directory.
 *        
 *  40   mpeg      1.39        5/13/99 6:39:04 PM     Anzhi Chen      Added 
 *        code at the begin of flash writes to check if the destination is 
 *        writtable.  Returns error if not.
 *        
 *  39   mpeg      1.38        5/6/99 3:50:14 PM      Anzhi Chen      Added 
 *        rom_identify and GetFlashInfo.  Added code support for 16-bit AMD
 *        and INTEL flashes.
 *        
 *  38   mpeg      1.37        4/9/99 6:37:26 PM      Anzhi Chen      Added the
 *         check for AMD flash at places that checks the Rom_Chip_Id.
 *        
 *  37   mpeg      1.36        4/9/99 4:24:16 PM      Tim Ross        Removed 
 *        id_mode_XXXX and id_XXXX_exit functions asnd 
 *        put in board.c in BSP.
 *        
 *  36   mpeg      1.35        3/29/99 4:41:12 PM     Anzhi Chen      Syntax 
 *        change.
 *        
 *  35   mpeg      1.34        3/17/99 2:05:54 PM     Tim Ross        Added 
 *        check for bKalUp to allow BSP to retrieve config params.
 *        
 *  34   mpeg      1.33        3/11/99 2:54:20 PM     Anzhi Chen      Fixed a 
 *        bug in block_erase() which wrote 0xffffffff to a u_int8 pointer.
 *        
 *  33   mpeg      1.32        2/19/99 2:10:00 PM     Anzhi Chen      Fixed a 
 *        bug in block_erase() that writes to an initalized pointer.
 *        
 *  32   mpeg      1.31        2/4/99 3:59:54 PM      Anzhi Chen      Added 
 *        id_amd_exit() and id_intel_exit().  Removed the reset command call
 *        from write operations.  
 *        
 *  31   mpeg      1.30        1/28/99 9:58:22 AM     Anzhi Chen      Changed 
 *        code to get rid of the warnings.
 *        
 *  30   mpeg      1.29        1/20/99 4:44:22 PM     Anzhi Chen      Added a 
 *        reset command before all the write and erase operations.  This seems 
 *        to
 *        fix the problem that hung the erase operation.
 *        
 *  29   mpeg      1.28        1/15/99 3:48:16 PM     Anzhi Chen      Fixed a 
 *        bug in data_write() that did not call semaphore_signal() before some 
 *        returns.
 *        
 *  28   mpeg      1.27        1/15/99 3:36:44 PM     Anzhi Chen      Fixed a 
 *        bug in data_write() where still called data_read().
 *        
 *  27   mpeg      1.26        1/15/99 9:55:58 AM     Anzhi Chen      Added 
 *        read_byte(), read_hword() and read_word() functions.  This was done 
 *        for
 *        the optimization of the flash driver.  Inside data_write(), there 
 *        were calls to
 *        data_read().  This screwed things up when semaphore were added at 
 *        begining of
 *        data_write() and data_read().
 *        
 *  26   mpeg      1.25        11/20/98 11:09:26 AM   Anzhi Chen      Fixed the
 *         problem that caused programming to fail sometimes due to erasing 
 *        falilure.
 *        
 *  25   mpeg      1.24        11/16/98 4:04:32 PM    Anzhi Chen      Did the 
 *        same modification for erase casees as last revision.
 *        
 *  24   mpeg      1.23        11/16/98 3:50:46 PM    Anzhi Chen      In 
 *        write_word(), changed the code that checking DQ5.  Any one DQ5 is 1 
 *        indicates
 *        time limit exceeded.
 *        
 *  23   mpeg      1.22        9/25/98 5:01:56 PM     Anzhi Chen      Changed 
 *        back to revision 1.20.
 *        
 *  22   mpeg      1.21        9/25/98 11:21:22 AM    Anzhi Chen      Modified 
 *        id_identify_amd(). 
 *        
 *  21   mpeg      1.20        9/24/98 5:35:28 PM     Anzhi Chen      Fixed 2 
 *        bugs in erase_16_block() and erase_32_block().
 *        
 *  20   mpeg      1.19        9/24/98 4:21:20 PM     Anzhi Chen      Added 
 *        parathese around shift in id_mode_amd().
 *        
 *  19   mpeg      1.18        9/24/98 11:42:48 AM    Anzhi Chen      Changed 
 *        names for rom_shit_size etc.
 *        
 *  18   mpeg      1.17        9/18/98 2:16:32 PM     Anzhi Chen      
 *        ROM_CHIP_ID were defined as external variables instead of definitions
 *         in .cfg
 *        file.
 *        
 *  17   mpeg      1.16        9/3/98 2:44:54 PM      Anzhi Chen      Fixed 
 *        several bugs in 32-bit ROM case.
 *        
 *  16   mpeg      1.15        8/28/98 6:23:56 PM     Anzhi Chen      Modefied 
 *        code that handles 16-bit ROM.
 *        
 *  15   mpeg      1.14        8/27/98 11:02:24 AM    Anzhi Chen      Changed 
 *        type of pointer to memory to LPREG or volatile types in places.
 *        
 *  14   mpeg      1.13        8/26/98 3:08:00 PM     Anzhi Chen      Fixed a 
 *        bug in write_hword() where checked 32-bit status data.
 *        
 *  13   mpeg      1.12        8/21/98 4:51:50 PM     Anzhi Chen      Bug 
 *        fixed.
 *        
 *  12   mpeg      1.11        8/19/98 5:58:54 PM     Anzhi Chen      Fixed 
 *        lots of bugs.
 *        
 *  11   mpeg      1.10        8/17/98 2:48:48 PM     Anzhi Chen      Removed 
 *        the variable timeout.
 *        
 *  10   mpeg      1.9         8/17/98 2:41:44 PM     Anzhi Chen      Fixed a 
 *        lot of bug in flash write case. Added code support for source address
 *        non-dword-aligned cases.
 *        
 *  9    mpeg      1.8         8/13/98 2:00:46 PM     Anzhi Chen      Fixed an 
 *        error due to last change.
 *        
 *  8    mpeg      1.7         8/13/98 1:54:22 PM     Anzhi Chen      Added new
 *         function block_check().
 *        
 *  7    mpeg      1.6         8/13/98 12:59:06 PM    Anzhi Chen      Fixed a 
 *        bug in write_data() when calculating aligned address.
 *        
 *  6    mpeg      1.5         8/12/98 5:00:08 PM     Anzhi Chen      Fixed a 
 *        bug in clear_bits().
 *        
 *  5    mpeg      1.4         8/12/98 11:41:00 AM    Anzhi Chen      Added the
 *         read status register command for INTEL flash.
 *        
 *  4    mpeg      1.3         8/10/98 11:48:16 AM    Anzhi Chen      Fixed 
 *        several compiling errors.
 *        
 *  3    mpeg      1.2         8/10/98 11:30:20 AM    Anzhi Chen      Fixed a 
 *        lot of compiling errors.
 *        
 *  2    mpeg      1.1         8/4/98 10:52:04 AM     Anzhi Chen      Moved 
 *        functions from fsdrv.c to this file.
 *        
 *  1    mpeg      1.0         6/19/98 3:38:10 PM     Anzhi Chen      
 * $
 ****************************************************************************/

