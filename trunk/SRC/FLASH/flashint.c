/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*          Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002          */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        FLASHINT.C
 *
 *
 * Description:     Low level Intel flash memory access functions
 *
 *
 * Author:          Tim White
 *
 ****************************************************************************/
/* $Header: flashint.c, 12, 9/3/03 4:04:18 PM, Tim White$
 ****************************************************************************/

//
// WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
// WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
//
// In order to allow execution from ROM (flash ROM), there has to be a way
// to write and erase the flash ROM since execution requires the update of,
// at the very least, the config area of the flash ROM.  Also the code images
// can be updated via the demux driver although this is not recommended for
// an image running from ROM for several reasons:  1) Care must be taken to
// not overwrite the currently running image, and 2) The amount of time taken
// to erase a sector where all interrupts are disabled is likely too long
// in order to have the download stay operational using the demux driver.
//
// All code (and data) below before the bank_erase function is subject to
// copying to RAM and executed out of RAM if the caller is running from ROM
// space.  During execution of any of the following routines, all interrupts
// must be disabled assuming the interrupt routines access code and/or data
// residing in the ROM which cannot be accessed when writing/erasing.  If
// a scatter load list were used such that interrupt routines and associated
// data could be guaranteed to be in RAM at load time, interrupts would not
// need to be disabled.
//
// If altering or adding any functions to this file, care must be taken
// to ensure the proper order and that nothing in the copy area below
// references anything outside the scope of the following copy area or
// global data R/W area.  Pay particular attention to not referencing
// any runtime library functions such as rt_divide() when a divide "/"
// is used.  Only include the functions that require copying in the copy
// area below.  Currently these are the chip_write() and chip_erase()
// functions.  The chip_erase_bank() functions are only used by the
// download tool which is guaranteed to not be run from ROM.
//
// WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
// WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
//

/*****************/
/* Include Files */
/*****************/
#include <string.h>
#include "stbcfg.h"
#include "basetype.h"
#ifndef DLOAD
#include "kal.h"
#endif
#ifdef DLOAD
#include "defs.h"
#endif
#include "startup.h"
#include "flashrw.h"

/**************************/
/* Flash Command Indecies */
/**************************/
#define FLASH_CMD_READ			0
#define FLASH_CMD_READ_CONF		1
#define FLASH_CMD_READ_QUERY		2
#define FLASH_CMD_READ_STATUS		3
#define FLASH_CMD_CLEAR_STATUS		4
#define FLASH_CMD_PROGRAM		5
#define FLASH_CMD_BLOCK_ERASE_SETUP	6
#define FLASH_CMD_PROGRAM_ERASE_SUSPEND	7
#define FLASH_CMD_BLOCK_ERASE_CONFIRM	8
#define FLASH_CMD_PROGRAM_ERASE_RESUME	8
#define FLASH_CMD_UNLOCK_BLOCK		8
#define FLASH_CMD_LOCK_SETUP		9
#define FLASH_CMD_LOCK_BLOCK		10
#define FLASH_CMD_LOCKDOWN_BLOCK	11
#define FLASH_CMD_PROTECTION_PROGRAM	12

/******************************************/
/* Local Definitions and Global Variables */
/******************************************/
extern BANK_INFO Bank[];
extern int CurrentBank;
extern u_int32 UniformBlockSize;

/**************************/
/* Function Prototypes    */   
/**************************/
int write_intel(void *addr, void *buffer, int size);
int erase_intel(void *addr, void (*callback)(void));
int erase_intel_bank(void *addr, void (*callback)(void));

/*****************************/
/* Intel Command Code Table  */   
/*****************************/
u_int32 IntelCode[2][13] = 
{
    /* 8-bit chips */
    {   /* 0 - Read */     
        0xFFFFFFFF,
        /* 1 - Read Configuration */ 
        0x90909090,
	/* 2 - Read Query */
	0x98989898,
	/* 3 - Read Status */
	0x70707070,
	/* 4 - Clear Status */
	0x50505050,
	/* 5 - Program */
	0x40404040,
	/* 6 - Block Erase & Block Erase Confirm */
	0x20202020,
	/* 7 - Program Erase/Suspend */
	0xB0B0B0B0,
	/* 8 - Program Erase/Resume & Unlock Block */
	0xD0D0D0D0,
	/* 9 - Lock Setup */
	0x60606060,
	/* 10 - Lock Block */
	0x01010101,
	/* 11 - Lockdown Block */
	0x2F2F2F2F,
	/* 12 - Protection Program */
	0xC0C0C0C0
    },
    /* 16-bit chips */
    {   /* 0 - Read */     
        0xFFFFFFFF,
        /* 1 - Read Configuration */ 
        0x00900090,
	/* 2 - Read Query */
	0x00980098,
	/* 3 - Read Status */
	0x00700070,
	/* 4 - Clear Status */
	0x00500050,
	/* 5 - Program */
	0x00400040,
	/* 6 - Block Erase & Block Erase Confirm */
	0x00200020,
	/* 7 - Program Erase/Suspend */
	0x00B000B0,
	/* 8 - Program Erase/Resume & Unlock Block */
	0x00D000D0,
	/* 9 - Lock Setup */
	0x00600060,
	/* 10 - Lock Block */
	0x00010001,
	/* 11 - Lockdown Block */
	0x002F002F,
	/* 12 - Protection Program */
	0x00C000C0
    }
};                                                  

/**********************************/
/* Intel Status Byte Definitions  */   
/**********************************/
#define WSM_READY       0x80
#define ERASE_SUSPENDED 0X40
#define ERASE_ERROR     0x30
#define PROGRAM_ERROR   0x10
#define LOW_VPP         0x08           

int write_intel(void *addr, void *buffer, int size)
{
   int i;
   volatile u_int32 *dest, *src;
   u_int32 wsm_mask, err_mask;
   int result;               
   u_int32 buf;

   dest = (u_int32 *)addr;
   src = (u_int32 *)buffer;
   result = size;

   /* Set up status checking masks. */ 
   wsm_mask = err_mask = 0;
   for (i = 0;i < Bank[CurrentBank].BankWidth ;i += Bank[CurrentBank].ChipWidth )
   {                       
      wsm_mask |= (WSM_READY << i);
      err_mask |= ((PROGRAM_ERROR | LOW_VPP) << i);
   }

   /* Put ROM controller into flash program mode. */
#if !defined(DLOAD) && CPU_TYPE == CPU_ARM920T && RTOS != VXWORKS
   if((u_int32)addr & 0x08000000)
   {
      ENTER_WRITE_MODE_2();
   }
   else
#endif /* !defined(DLOAD) && CPU_TYPE == CPU_ARM920T && RTOS != VXWORKS */
   {
      ENTER_WRITE_MODE();                              
   }
   
   //
   // If flash type is by default locked, unlock first
   //
   if(Bank[CurrentBank].Flags & FLASH_FLAGS_UNLOCK_LOCK_SEQ_REQD)
   {
      //
      // Send unlock command sequence
      //
      *dest = IntelCode[Bank[CurrentBank].ChipWidth >> 4][FLASH_CMD_LOCK_SETUP];   
      *dest = IntelCode[Bank[CurrentBank].ChipWidth >> 4][FLASH_CMD_UNLOCK_BLOCK];   

      /* Send read status command. */
      *dest = IntelCode[Bank[CurrentBank].ChipWidth >> 4][FLASH_CMD_READ_STATUS];  

      /* Wait for write to complete. */
      while((*dest & wsm_mask) != wsm_mask) 
	;

      //
      // Return to read array mode
      //
      *dest = IntelCode[Bank[CurrentBank].ChipWidth >> 4][FLASH_CMD_READ];
   }

   //
   // Program flash
   //
   for (i = size; i > 0; i -= (Bank[CurrentBank].BankWidth >> 3))
   {
      /* Send write command. */
      *dest = IntelCode[Bank[CurrentBank].ChipWidth >> 4][FLASH_CMD_PROGRAM];   
      
      /* Write data. */ 
      switch (Bank[CurrentBank].BankWidth)
      {                
        case 8: 
            *dest = *src;
            break;
        case 16:
           /* We always read the source buffer one byte at a time */
           /* to avoid misalignment read errors. buf will always  */
           /* be aligned.                                         */
           *(u_int8 *)&buf = *((u_int8 *)src);
           *((u_int8 *)&buf + 1) = *((u_int8 *)src + 1);
           *dest = buf;
           break;
        case 32:
           /* We always read the source buffer one byte at a time */
           /* to avoid misalignment read errors. buf will always  */
           /* be aligned.                                         */
           *(u_int8 *)&buf = *(u_int8 *)src;
           *((u_int8 *)&buf + 1) = *((u_int8 *)src + 1);
           *((u_int8 *)&buf + 2) = *((u_int8 *)src + 2);
           *((u_int8 *)&buf + 3) = *((u_int8 *)src + 3);
           *dest = buf;
           break;
     }
      
      /* Send read status command. */
      *dest = IntelCode[Bank[CurrentBank].ChipWidth >> 4][FLASH_CMD_READ_STATUS];  

      /* Wait for write to complete. */
      while((*dest & wsm_mask) != wsm_mask) 
	;

      /* Check for errors. */
      if(*dest & err_mask)
      {
          /* If there was an error, indicate 0 bytes were written */
          /* and clear the status byte.                           */
          *dest = IntelCode[Bank[CurrentBank].ChipWidth >> 4][FLASH_CMD_CLEAR_STATUS];             
          result = 0;                      
          i = 0;
          break;
      }           

      /* Increment ptrs to next data element. */
      dest = (u_int32 *)((u_int8 *)dest + (Bank[CurrentBank].BankWidth>>3));
      src = (u_int32 *)((u_int8 *)src + (Bank[CurrentBank].BankWidth>>3));
   }

   //
   // Ensure *dest is on the same ROM bank since we could have crossed a
   // ROM bank boundary here...
   //
   dest = (u_int32 *)addr;

   //
   // If flash type is by default locked, unlock first
   //
   if(Bank[CurrentBank].Flags & FLASH_FLAGS_UNLOCK_LOCK_SEQ_REQD)
   {
      //
      // Send (re)lock command sequence
      //
      *dest = IntelCode[Bank[CurrentBank].ChipWidth >> 4][FLASH_CMD_LOCK_SETUP];
      *dest = IntelCode[Bank[CurrentBank].ChipWidth >> 4][FLASH_CMD_LOCK_BLOCK];

      /* Send read status command. */
      *dest = IntelCode[Bank[CurrentBank].ChipWidth >> 4][FLASH_CMD_READ_STATUS];

      /* Wait for write to complete. */
      while((*dest & wsm_mask) != wsm_mask)
        ;
   }

   //
   // Return to read array mode
   //
   *dest = IntelCode[Bank[CurrentBank].ChipWidth >> 4][FLASH_CMD_READ];
      
   //
   // Return ROM controller to normal read mode
   //
#if !defined(DLOAD) && CPU_TYPE == CPU_ARM920T && RTOS != VXWORKS
   if(!((u_int32)addr & 0x08000000))
#endif /* !defined(DLOAD) && CPU_TYPE == CPU_ARM920T && RTOS != VXWORKS */
   {
      ENTER_READ_MODE();
   }

   return result;
}

int erase_intel(void *addr, void (*callback)(void))
{
   int i;
   volatile u_int32 *dest;
   u_int32 wsm_mask, err_mask;
   int result = 0;

   dest = (u_int32 *)addr;

   /* Set up status checking masks. */ 
   wsm_mask = err_mask = 0;
   for (i = 0;i < Bank[CurrentBank].BankWidth ;i += Bank[CurrentBank].ChipWidth )
   {                       
      wsm_mask |= (WSM_READY << i);
      err_mask |= ((ERASE_ERROR | LOW_VPP) << i);
   }

   /* Put ROM controller into flash program mode. */
   ENTER_WRITE_MODE();                              
   
   //
   // If flash type is by default locked, unlock first
   //
   if(Bank[CurrentBank].Flags & FLASH_FLAGS_UNLOCK_LOCK_SEQ_REQD)
   {
      //
      // Send unlock command sequence
      //
      *dest = IntelCode[Bank[CurrentBank].ChipWidth >> 4][FLASH_CMD_LOCK_SETUP];
      *dest = IntelCode[Bank[CurrentBank].ChipWidth >> 4][FLASH_CMD_UNLOCK_BLOCK];

      /* Send read status command. */
      *dest = IntelCode[Bank[CurrentBank].ChipWidth >> 4][FLASH_CMD_READ_STATUS];

      /* Wait for write to complete. */
      while((*dest & wsm_mask) != wsm_mask)
        ;
   }

   /* Send erase setup command. */
   *dest = IntelCode[Bank[CurrentBank].ChipWidth >> 4][FLASH_CMD_BLOCK_ERASE_SETUP];
   
   /* Send erase confirm command. */
   *dest = IntelCode[Bank[CurrentBank].ChipWidth >> 4][FLASH_CMD_BLOCK_ERASE_CONFIRM];
   
   /* Send read status command. */
   *dest = IntelCode[Bank[CurrentBank].ChipWidth >> 4][FLASH_CMD_READ_STATUS];  

   /* Wait for erase to complete. */
   while((*dest & wsm_mask) != wsm_mask) 
      if(callback)
         callback();

   /* Check for errors. */
   if(*dest & err_mask)
   {
      if ((*dest & err_mask) != 0) 
      {
         /* If there was an error, indicate an erase failure */
         /* and clear the status byte.                           */
         *dest = IntelCode[Bank[CurrentBank].ChipWidth >> 4][FLASH_CMD_CLEAR_STATUS];             
         result = -1;
      }
   }           

   //
   // If flash type is by default locked, unlock first
   //
   if(Bank[CurrentBank].Flags & FLASH_FLAGS_UNLOCK_LOCK_SEQ_REQD)
   {
      //
      // Send (re)lock command sequence
      //
      *dest = IntelCode[Bank[CurrentBank].ChipWidth >> 4][FLASH_CMD_LOCK_SETUP];
      *dest = IntelCode[Bank[CurrentBank].ChipWidth >> 4][FLASH_CMD_LOCK_BLOCK];

      /* Send read status command. */
      *dest = IntelCode[Bank[CurrentBank].ChipWidth >> 4][FLASH_CMD_READ_STATUS];

      /* Wait for write to complete. */
      while((*dest & wsm_mask) != wsm_mask)
        ;
   }

   /* Return to read array mode. */
   *dest = IntelCode[Bank[CurrentBank].ChipWidth >> 4][FLASH_CMD_READ];
      
   /* Return ROM controller to normal read mode. */
   ENTER_READ_MODE();

   return result;
}
 
//
// Nothing after this point is copied to RAM.  The address of the
// erase_xxxx_bank() function is used in CopyAndCallRoutine()
// (flash/flashrw.c) for the end of the copy area.
//

int erase_intel_bank(void *addr, void (*callback)(void))
{
   unsigned int i, result = 0;
   u_int32 size, dest = Bank[CurrentBank].BankBase, size_left;

   // Intel chips do not have an ERASE_ALL (chip erase) command.  We must
   // erase sector by sector calling erase_intel()

   if(Bank[CurrentBank].Flags & FLASH_FLAGS_UNIFORM_SECTOR_SIZE)
   {
      size = UniformBlockSize;
      size_left = Bank[CurrentBank].BankSize;
      while(size_left)
      {
         result = erase_intel((void *)dest, callback);
         dest += size;
         if(result != 0)
            break;
         size_left -= UniformBlockSize;
      }
   }
   else
   {
      for(i=0;i<Bank[CurrentBank].pFlashDesc->ChipNumSectors;i++)
      {
         size = ((u_int32) Bank[CurrentBank].pFlashDesc->ChipSectorSize[i]<<12) *
                    (Bank[CurrentBank].BankWidth / Bank[CurrentBank].ChipWidth);
         result = erase_intel((void *)dest, callback);
         dest += size;
         if(result != 0)
            break;
      }
   }
   return (result);
}

/****************************************************************************
 * Modifications:
 * $Log: 
 *  12   mpeg      1.11        9/3/03 4:04:18 PM      Tim White       SCR(s) 
 *        7424 :
 *        Added support for the dual flash aperture.
 *        
 *        
 *  11   mpeg      1.10        3/5/03 5:19:30 PM      Tim White       SCR(s) 
 *        5681 5682 :
 *        Add support for ST M58LW032D flash type.
 *        
 *        
 *  10   mpeg      1.9         2/13/03 11:48:56 AM    Matt Korte      SCR(s) 
 *        5479 :
 *        Removed old header reference
 *        
 *  9    mpeg      1.8         4/10/02 5:05:24 PM     Tim White       SCR(s) 
 *        3509 :
 *        Eradicate external bus (PCI) bitfield usage.
 *        
 *        
 *  8    mpeg      1.7         1/31/01 2:33:42 PM     Tim White       DCS#0988:
 *         Reclaim footprint space.
 *        
 *  7    mpeg      1.6         4/10/00 4:25:40 PM     Tim White       Added 
 *        several flash types to main codebase.
 *        
 *  6    mpeg      1.5         4/6/00 9:59:52 AM      Ray Mack        fixes to 
 *        remove warnings
 *        
 *  5    mpeg      1.4         3/8/00 5:18:06 PM      Tim White       
 *        Restructured the BANK_INFO array and added support for new Intel 
 *        Flash ROM.
 *        
 *  4    mpeg      1.3         3/2/00 5:44:16 PM      Tim White       Put back 
 *        the calls to the macros since this function *cannot* make
 *        external function references.  The macros have been modified.  See 
 *        the
 *        flashrw.h header file for more information.
 *        
 *  3    mpeg      1.2         3/1/00 1:31:40 PM      Tim Ross        Replaced 
 *        ENTER_READ/WRITE_MODE macros with function
 *        calls toi EnterRead/WriteMode().
 *        
 *  2    mpeg      1.1         10/26/99 11:22:04 AM   Senthil Veluswamy 
 *        Hardcoded array parameter to workaround Arm2.5 bug in estimating 
 *        array sizes.
 *        
 *  1    mpeg      1.0         10/13/99 4:46:08 PM    Tim White       
 * $
 * 
 *    Rev 1.11   03 Sep 2003 15:04:18   whiteth
 * SCR(s) 7424 :
 * Added support for the dual flash aperture.
 * 
 * 
 *    Rev 1.10   05 Mar 2003 17:19:30   whiteth
 * SCR(s) 5681 5682 :
 * Add support for ST M58LW032D flash type.
 * 
 * 
 *    Rev 1.9   13 Feb 2003 11:48:56   kortemw
 * SCR(s) 5479 :
 * Removed old header reference
 * 
 *    Rev 1.8   10 Apr 2002 16:05:24   whiteth
 * SCR(s) 3509 :
 * Eradicate external bus (PCI) bitfield usage.
 * 
 *
 *    Rev 1.7   31 Jan 2001 14:33:42   whiteth
 * DCS#0988: Reclaim footprint space.
 *
 *    Rev 1.6   10 Apr 2000 15:25:40   whiteth
 * Added several flash types to main codebase.
 *
 *    Rev 1.5   06 Apr 2000 08:59:52   raymack
 * fixes to remove warnings
 *
 *    Rev 1.4   08 Mar 2000 17:18:06   whiteth
 * Restructured the BANK_INFO array and added support for new Intel Flash ROM.
 *
 *    Rev 1.3   02 Mar 2000 17:44:16   whiteth
 * Put back the calls to the macros since this function *cannot* make
 * external function references.  The macros have been modified.  See the
 * flashrw.h header file for more information.
 *
 *    Rev 1.2   01 Mar 2000 13:31:40   rossst
 * Replaced ENTER_READ/WRITE_MODE macros with function
 * calls toi EnterRead/WriteMode().
 *
 *    Rev 1.1   26 Oct 1999 10:22:04   velusws
 * Hardcoded array parameter to workaround Arm2.5 bug in estimating array sizes.
 *
 *    Rev 1.0   13 Oct 1999 15:46:08   whiteth
 * Initial revision.
 *
 ****************************************************************************/

