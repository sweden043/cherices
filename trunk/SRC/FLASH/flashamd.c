/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*          Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002          */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        FLASHAMD.C
 *
 *
 * Description:     Low level AMD flash memory access functions
 *
 *
 * Author:          Tim White
 *
 ****************************************************************************/
/* $Header: flashamd.c, 14, 9/3/03 4:04:16 PM, Tim White$
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
#define FLASH_CMD_ID            	0
#define FLASH_CMD_READ          	1
#define FLASH_CMD_UNLOCK1       	1
#define FLASH_CMD_WRITE         	2
#define FLASH_CMD_ERASE_SETUP   	3
#define FLASH_CMD_ERASE_CONFIRM 	4
#define FLASH_CMD_ERASE_RESUME  	4
#define FLASH_CMD_ERASE_SUSPEND 	5
#define FLASH_CMD_STATUS        	6
#define FLASH_CMD_UNLOCK2       	6
#define FLASH_CMD_ERASE_CHIP    	7
#define FLASH_CMD_CFI_QUERY     	8
#define FLASH_CMD_UNLOCK_BYPASS		9
#define FLASH_CMD_TEMP_UNPROTECT_SETUP	10
#define FLASH_CMD_BLOCK_ERASE_CONFIRM 	11
#define FLASH_CMD_RESET           12

/******************************************/
/* Local Definitions and Global Variables */
/******************************************/
extern BANK_INFO Bank[];
extern int CurrentBank;

/**************************/
/* Function Prototypes    */   
/**************************/
int write_amd(void *addr, void *buffer, int size);
int erase_amd(void *addr, void (*callback)(void));
int erase_amd_bank(void *addr, void (*callback)(void));

/***************************/
/* AMD Command Code Table  */   
/***************************/
u_int32 AMDCode[2][13] = 
{
    /* 8-bit chips */
    {   /* 0 - ID */     
        0x90909090,
        /* 1 - Read & Unlock Code 1 */ 
        0xaaaaaaaa,
        /* 2 - Write */        
        0xa0a0a0a0,
        /* 3 - Erase Setup */ 
        0x80808080,
        /* 4 - Erase Confirm/Resume */ 
        0x30303030,
        /* 5 - Erase Suspend */
        0xb0b0b0b0,
        /* 6 - Status & Unlock Code 2 */  
        0x55555555,
        /* 7 - Erase Chip */
        0x10101010,
        /* 8 - CFI Query */
        0x98989898,
        /* 9 - Unlock Bypass */
        0x20202020,
        /* 10 - Temporary Unprotect Setup */
        0xE0E0E0E0,
        /* 11 - Erase Block (SST) */
        0x50505050,
        /* 12 - Reset Chip */
        0xF0F0F0F0
    },
    
    /* 16-bit chips */
    {   /* 0 - ID */     
        0x00900090,
        /* 1 - Read & Unlock Code 1 */ 
        0x00aa00aa,
        /* 2 - Write */        
        0x00a000a0,
        /* 3 - Erase Setup */ 
        0x00800080,
        /* 4 - Erase Confirm/Resume */ 
        0x00300030,
        /* 5 - Erase Suspend */
        0x00b000b0,
        /* 6 - Status & Unlock Code 2 */  
        0x00550055,
        /* 7 - Erase Chip */
        0x00100010,
        /* 8 - CFI Query */
        0x00980098,
        /* 9 - Unlock Bypass */
        0x00200020,
        /* 10 - Temporary Unprotect Setup */
        0x00E000E0,
        /* 11 - Erase Block (SST) */
        0x00500050,
        /* 12 - Reset Chip */
        0xF0F0F0F0
    }
};               

/********************************/
/* AMD Status Byte Definitions  */   
/********************************/
#define EA_READY    0x80
#define EA_TOGGLE1  0x40
#define EA_TOGGLE2  0x02
#define EA_TIMEOUT  0x20
#define EA_ERASING  0x08

void delay30ms(u_int32 nTimes)
{
   u_int32 pollDelay, toDo;
 
   for(toDo=0; toDo<nTimes; toDo++)
   {
     for(pollDelay=0; pollDelay<0x7FFF; pollDelay++)
     {
       /* takes 30 ms at 166/133MHz */
     }
   }
}

void delay10us(u_int32 nTimes)
{
   u_int32 pollDelay, toDo;

   for(toDo=0; toDo<nTimes; toDo++)
   {
     for(pollDelay=0; pollDelay<0x0F; pollDelay++)
     {
       /* takes 30 ms at 166/133MHz */
     }
   }
}

int write_amd(void *addr, void *buffer, int size)
{
   int i,  isST;
   volatile u_int32 *dest, *src, *cmd1, *cmd2;
   u_int32 rdy_mask;
   int result;
   u_int32 buf;
      u_int32 pollDelay, toDo;

   dest = (u_int32 *)addr;
   src = (u_int32 *)buffer;
   result = size;
   
   isST = (Bank[CurrentBank].pFlashDesc->MfrID == FLASH_CHIP_VENDOR_ST) ? 1 : 0;
   /* If using STM29W800DT, clear the isST flag. */
   if (Bank[CurrentBank].pFlashDesc->DeviceID == 0x22D7)
   {
      isST = 0;
   }
   
   /* Set up status checking masks. */
   rdy_mask = 0;
   for (i = 0; i < Bank[CurrentBank].BankWidth; i += Bank[CurrentBank].ChipWidth)
   {                       
      rdy_mask |= (EA_READY << i);
   }                                        
   
#if CUSTOMER != VENDOR_A && CUSTOMER != VENDOR_D
   /* Set up command addresses. */
   if((Bank[CurrentBank].pFlashDesc->MfrID == FLASH_CHIP_VENDOR_AMD) ||
      (Bank[CurrentBank].pFlashDesc->MfrID == FLASH_CHIP_VENDOR_ST) ||
      (Bank[CurrentBank].pFlashDesc->MfrID == FLASH_CHIP_VENDOR_TOSHIBA))
   {
#endif
     cmd1 = (u_int32 *)(((0xaaa >> (Bank[CurrentBank].ChipWidth >> 4)) 
          << (Bank[CurrentBank].BankWidth >> 4)) + Bank[CurrentBank].BankBase);
     cmd2 = (u_int32 *)(((0x555 >> (Bank[CurrentBank].ChipWidth >> 4)) 
          << (Bank[CurrentBank].BankWidth >> 4)) + Bank[CurrentBank].BankBase);
#if CUSTOMER != VENDOR_A && CUSTOMER != VENDOR_D
   }
   else
   {
     cmd1 = (u_int32 *)(((0xaaaa >> (Bank[CurrentBank].ChipWidth >> 4)) 
          << (Bank[CurrentBank].BankWidth >> 4)) + Bank[CurrentBank].BankBase);
     cmd2 = (u_int32 *)(((0x5555 >> (Bank[CurrentBank].ChipWidth >> 4)) 
          << (Bank[CurrentBank].BankWidth >> 4)) + Bank[CurrentBank].BankBase);
   }
#endif

   /* Put ROM controller into flash program mode. */
#if !defined(DLOAD) && CPU_TYPE == CPU_ARM920T && RTOS != VXWORKS
   if((u_int32)addr & 0x08000000)
   {
      cmd1 = (volatile u_int32 *)((u_int32)cmd1 | 0x08000000);
      cmd2 = (volatile u_int32 *)((u_int32)cmd2 | 0x08000000);
      ENTER_WRITE_MODE_2();
   }
   else
#endif /* !defined(DLOAD) && CPU_TYPE == CPU_ARM920T && RTOS != VXWORKS */
   {
      ENTER_WRITE_MODE();                              
   }
   
   /* Program flash. */
   for (i = size; i > 0; i -= (Bank[CurrentBank].BankWidth >> 3))
   {
      /* Send unlock sequence. */
      *cmd1 = AMDCode[Bank[CurrentBank].ChipWidth >> 4][FLASH_CMD_UNLOCK1];   
      *cmd2 = AMDCode[Bank[CurrentBank].ChipWidth >> 4][FLASH_CMD_UNLOCK2];   
      
      /* Send write command. */
      *cmd1 = AMDCode[Bank[CurrentBank].ChipWidth >> 4][FLASH_CMD_WRITE];   
      
#if CUSTOMER != VENDOR_A && CUSTOMER != VENDOR_D
      /* Write data. */ 
      switch (Bank[CurrentBank].BankWidth)
      {                
         case 8:
            *dest = buf = *src;
            break;
         case 16:
#endif
            /* We always read the source buffer one byte at a time */
            /* to avoid misalignment read errors. buf will always  */
            /* be aligned.                                         */
            *(u_int8 *)&buf = *((u_int8 *)src);
            *((u_int8 *)&buf + 1) = *((u_int8 *)src + 1);
            *dest = buf;
#if CUSTOMER != VENDOR_A && CUSTOMER != VENDOR_D
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
      
      /* wait for ST Microelectronic protos */
      if(isST)
      {
       // delay10us(1);
          for(toDo=0; toDo<1; toDo++)
	   {
	     for(pollDelay=0; pollDelay<0x0F; pollDelay++)
	     {
	       /* takes 30 ms at 166/133MHz */
	     }
  	  }
      }
#endif

      while((*dest & rdy_mask) != (buf&rdy_mask));

      /* Increment ptrs to next data element. */
      dest = (u_int32 *)((u_int8 *)dest + (Bank[CurrentBank].BankWidth>>3));
      src = (u_int32 *)((u_int8 *)src + (Bank[CurrentBank].BankWidth>>3));
   }

   /* for ST Microelectrics */
   /* Send soft reset command to put flash in read mode. */
   if(isST)
   {
     *cmd1 = AMDCode[Bank[CurrentBank].ChipWidth >> 4][FLASH_CMD_RESET];   
   }

   /* Return ROM controller to normal read mode. */
#if !defined(DLOAD) && CPU_TYPE == CPU_ARM920T && RTOS != VXWORKS
   if(!((u_int32)addr & 0x08000000))
#endif /* !defined(DLOAD) && CPU_TYPE == CPU_ARM920T && RTOS != VXWORKS */
   {
      ENTER_READ_MODE();
   }

   return result;
}

int erase_amd(void *addr, void (*callback)(void))
{
   int i, isST;
   volatile u_int32 *dest, *cmd1, *cmd2;
   u_int32 rdy_mask;
   int result = 0;
   u_int32 pollDelay, toDo;

   dest = (u_int32 *)addr;
   
   isST = (Bank[CurrentBank].pFlashDesc->MfrID == FLASH_CHIP_VENDOR_ST) ? 1 : 0;
   /* If using STM29W800DT, clear the isST flag. */
   if (Bank[CurrentBank].pFlashDesc->DeviceID == 0x22D7)
   {
      isST = 0;
   }
   
   /* Set up status checking masks. */
   rdy_mask = 0;
   for (i = 0;i < Bank[CurrentBank].BankWidth ;i += Bank[CurrentBank].ChipWidth )
   {                       
      rdy_mask |= (EA_READY << i);
   }                                        
   
   /* Put ROM controller into flash program mode. */
   ENTER_WRITE_MODE();                              
   
#if CUSTOMER != VENDOR_A && CUSTOMER != VENDOR_D
   /* Set up command addresses. */
   if((Bank[CurrentBank].pFlashDesc->MfrID == FLASH_CHIP_VENDOR_AMD) ||
      (Bank[CurrentBank].pFlashDesc->MfrID == FLASH_CHIP_VENDOR_ST) ||
      (Bank[CurrentBank].pFlashDesc->MfrID == FLASH_CHIP_VENDOR_TOSHIBA))
   {
#endif
     cmd1 = (u_int32 *)(((0xaaa >> (Bank[CurrentBank].ChipWidth >> 4)) 
          << (Bank[CurrentBank].BankWidth >> 4)) + Bank[CurrentBank].BankBase);
     cmd2 = (u_int32 *)(((0x555 >> (Bank[CurrentBank].ChipWidth >> 4)) 
          << (Bank[CurrentBank].BankWidth >> 4)) + Bank[CurrentBank].BankBase);
#if CUSTOMER != VENDOR_A && CUSTOMER != VENDOR_D
   }
   else
   {
     cmd1 = (u_int32 *)(((0xaaaa >> (Bank[CurrentBank].ChipWidth >> 4)) 
          << (Bank[CurrentBank].BankWidth >> 4)) + Bank[CurrentBank].BankBase);
     cmd2 = (u_int32 *)(((0x5555 >> (Bank[CurrentBank].ChipWidth >> 4)) 
          << (Bank[CurrentBank].BankWidth >> 4)) + Bank[CurrentBank].BankBase);
   }
#endif

   /* Send unlock sequence. */
   *cmd1 = AMDCode[Bank[CurrentBank].ChipWidth >> 4][FLASH_CMD_UNLOCK1];   
   *cmd2 = AMDCode[Bank[CurrentBank].ChipWidth >> 4][FLASH_CMD_UNLOCK2];   
   
   /* Send erase setup command. */
   *cmd1 = AMDCode[Bank[CurrentBank].ChipWidth >> 4][FLASH_CMD_ERASE_SETUP];   
   
   /* Send unlock sequence again. */
   *cmd1 = AMDCode[Bank[CurrentBank].ChipWidth >> 4][FLASH_CMD_UNLOCK1];   
   *cmd2 = AMDCode[Bank[CurrentBank].ChipWidth >> 4][FLASH_CMD_UNLOCK2];   
   
   /* Send erase confirm command. */
   if(Bank[CurrentBank].pFlashDesc->MfrID == FLASH_CHIP_VENDOR_SST)
   {
     *dest = AMDCode[Bank[CurrentBank].ChipWidth >> 4][FLASH_CMD_ERASE_CONFIRM];
   }
   else
   {
     *dest = AMDCode[Bank[CurrentBank].ChipWidth >> 4][FLASH_CMD_ERASE_CONFIRM];   
   }
   
   /* For ST protos wait before polling */
   if(isST)
   {
  //   delay30ms(7); /* 7 * 30ms = 210 ms */
 //  }
   	
	    for(toDo=0; toDo<7; toDo++)
	   {
		     for(pollDelay=0; pollDelay<0x7FFF; pollDelay++)
		     {
		       /* takes 30 ms at 166/133MHz */
		     }
	    }
   }
   
   /* Wait for erase to complete. */
   while((*dest & rdy_mask) != rdy_mask)
   {
     /* For ST protos wait before polling */
     if(isST)
     {
  //     delay30ms(7); /* 7 * 30ms = 210 ms */
  	    for(toDo=0; toDo<7; toDo++)
	   {
		     for(pollDelay=0; pollDelay<0x7FFF; pollDelay++)
		     {
		       /* takes 30 ms at 166/133MHz */
		     }
	    }
     }
     
      if(callback)
  	      callback();
   }
   
   /* for ST Microelectrics */
   /* Send soft reset command to put flash in read mode. */
   if (isST)
   {
     *cmd1 = AMDCode[Bank[CurrentBank].ChipWidth >> 4][FLASH_CMD_RESET];
   }
   
   /* Return ROM controller to normal read mode. */
   ENTER_READ_MODE();

   return result;
}

//
// Nothing after this point is copied to RAM.  The address of the
// erase_xxxx_bank() function is used in CopyAndCallRoutine()
// (flash/flashrw.c) for the end of the copy area.
//

int erase_amd_bank(void *addr, void (*callback)(void))
{
   int i, isST;
   volatile u_int32 *dest, *cmd1, *cmd2;
   u_int32 rdy_mask;
   int result = 0;

   dest = (u_int32 *)addr;
   
   isST = (Bank[CurrentBank].pFlashDesc->MfrID == FLASH_CHIP_VENDOR_ST) ? 1 : 0;
   /* If using STM29W800DT, clear the isST flag. */
   if (Bank[CurrentBank].pFlashDesc->DeviceID == 0x22D7)
   {
      isST = 0;
   }
   
   /* Set up status checking masks. */
   rdy_mask = 0;
   for (i = 0;i < Bank[CurrentBank].BankWidth ;i += Bank[CurrentBank].ChipWidth )
   {                       
      rdy_mask |= (EA_READY << i);
   }                                        
   
   /* Put ROM controller into flash program mode. */
   ENTER_WRITE_MODE();                              
   
#if CUSTOMER != VENDOR_A && CUSTOMER != VENDOR_D
   /* Set up command addresses. */
   if((Bank[CurrentBank].pFlashDesc->MfrID == FLASH_CHIP_VENDOR_AMD) ||
      (Bank[CurrentBank].pFlashDesc->MfrID == FLASH_CHIP_VENDOR_ST) ||
      (Bank[CurrentBank].pFlashDesc->MfrID == FLASH_CHIP_VENDOR_TOSHIBA))
   {
#endif
     cmd1 = (u_int32 *)(((0xaaa >> (Bank[CurrentBank].ChipWidth >> 4)) 
          << (Bank[CurrentBank].BankWidth >> 4)) + Bank[CurrentBank].BankBase);
     cmd2 = (u_int32 *)(((0x555 >> (Bank[CurrentBank].ChipWidth >> 4)) 
          << (Bank[CurrentBank].BankWidth >> 4)) + Bank[CurrentBank].BankBase);
#if CUSTOMER != VENDOR_A && CUSTOMER != VENDOR_D
   }
   else
   {
     cmd1 = (u_int32 *)(((0xaaaa >> (Bank[CurrentBank].ChipWidth >> 4)) 
          << (Bank[CurrentBank].BankWidth >> 4)) + Bank[CurrentBank].BankBase);
     cmd2 = (u_int32 *)(((0x5555 >> (Bank[CurrentBank].ChipWidth >> 4)) 
          << (Bank[CurrentBank].BankWidth >> 4)) + Bank[CurrentBank].BankBase);
   }
#endif

   /* Send unlock sequence. */
   *cmd1 = AMDCode[Bank[CurrentBank].ChipWidth >> 4][FLASH_CMD_UNLOCK1];   
   *cmd2 = AMDCode[Bank[CurrentBank].ChipWidth >> 4][FLASH_CMD_UNLOCK2];   
   
   /* Send erase setup command. */
   *cmd1 = AMDCode[Bank[CurrentBank].ChipWidth >> 4][FLASH_CMD_ERASE_SETUP];   
   
   /* Send unlock sequence again. */
   *cmd1 = AMDCode[Bank[CurrentBank].ChipWidth >> 4][FLASH_CMD_UNLOCK1];   
   *cmd2 = AMDCode[Bank[CurrentBank].ChipWidth >> 4][FLASH_CMD_UNLOCK2];   
   
   /* Send erase confirm command. */
   *cmd1 = AMDCode[Bank[CurrentBank].ChipWidth >> 4][FLASH_CMD_ERASE_CHIP];   
   
   /* For ST protos wait before polling */
   if(isST)
   {
     delay30ms(7); /* 7 * 30ms = 210 ms */
   }
   
   /* Wait for erase to complete. */
   while((*dest & rdy_mask) != rdy_mask)
   {
     /* For ST protos wait before polling */
     if(isST)
     {
       delay30ms(7); /* 7 * 30ms = 210 ms */
     }

      if(callback)
         callback();
   }
   
   /* for ST Microelectrics */
   /* Send soft reset command to put flash in read mode. */
   if (isST)
   {
     *cmd1 = AMDCode[Bank[CurrentBank].ChipWidth >> 4][FLASH_CMD_RESET];   
   }
   
   /* Return ROM controller to normal read mode. */
   ENTER_READ_MODE();

   return result;
}

/****************************************************************************
 * Modifications:
 * $Log: 
 *  14   mpeg      1.13        9/3/03 4:04:16 PM      Tim White       SCR(s) 
 *        7424 :
 *        Added support for the dual flash aperture.
 *        
 *        
 *  13   mpeg      1.12        2/13/03 11:48:58 AM    Matt Korte      SCR(s) 
 *        5479 :
 *        Removed old header reference
 *        
 *  12   mpeg      1.11        4/10/02 5:05:20 PM     Tim White       SCR(s) 
 *        3509 :
 *        Eradicate external bus (PCI) bitfield usage.
 *        
 *        
 *  11   mpeg      1.10        2/16/01 5:22:04 PM     Tim White       DCS#1248:
 *         Fixed problem with bank_erase for VENDOR_A & VENDOR_D.
 *        
 *  10   mpeg      1.9         2/15/01 5:36:42 PM     Tim White       DCS#1230:
 *         Globalize MemClkPeriod for use by the Soft Modem code.
 *        
 *  9    mpeg      1.8         1/31/01 2:33:40 PM     Tim White       DCS#0988:
 *         Reclaim footprint space.
 *        
 *  8    mpeg      1.7         9/20/00 11:23:10 AM    Tim White       Added 
 *        Toshiba flash ROM capability.
 *        
 *  7    mpeg      1.6         5/17/00 12:22:36 PM    Tim White       Added 
 *        support for SST ROM's.
 *        
 *  6    mpeg      1.5         5/1/00 11:00:40 PM     Tim White       Added 
 *        Atmel flash support code.
 *        
 *  5    mpeg      1.4         3/8/00 5:18:00 PM      Tim White       
 *        Restructured the BANK_INFO array and added support for new Intel 
 *        Flash ROM.
 *        
 *  4    mpeg      1.3         3/2/00 5:43:38 PM      Tim White       Put back 
 *        the calls to the macros since this function *cannot* make
 *        external function references.  The macros have been modified.  See 
 *        the
 *        flashrw.h header file for more information.
 *        
 *  3    mpeg      1.2         3/1/00 1:31:16 PM      Tim Ross        Replaced 
 *        ENTER_READ/WRITE_MODE macros with function
 *        calls toi EnterRead/WriteMode().
 *        
 *  2    mpeg      1.1         10/26/99 11:19:08 AM   Senthil Veluswamy 
 *        Hardcoded array parameter to workaround Arm2.5 array sizing bug.
 *        
 *  1    mpeg      1.0         10/13/99 4:45:36 PM    Tim White       
 * $
 * 
 *    Rev 1.13   03 Sep 2003 15:04:16   whiteth
 * SCR(s) 7424 :
 * Added support for the dual flash aperture.
 * 
 * 
 *    Rev 1.12   13 Feb 2003 11:48:58   kortemw
 * SCR(s) 5479 :
 * Removed old header reference
 * 
 *    Rev 1.11   10 Apr 2002 16:05:20   whiteth
 * SCR(s) 3509 :
 * Eradicate external bus (PCI) bitfield usage.
 * 
 *
 *    Rev 1.10   16 Feb 2001 17:22:04   whiteth
 * DCS#1248: Fixed problem with bank_erase for VENDOR_A & VENDOR_D.
 *
 *    Rev 1.9   15 Feb 2001 17:36:42   whiteth
 * DCS#1230: Globalize MemClkPeriod for use by the Soft Modem code.
 *
 *    Rev 1.8   31 Jan 2001 14:33:40   whiteth
 * DCS#0988: Reclaim footprint space.
 *
 *    Rev 1.7   20 Sep 2000 10:23:10   whiteth
 * Added Toshiba flash ROM capability.
 *
 *    Rev 1.6   17 May 2000 11:22:36   whiteth
 * Added support for SST ROM's.
 *
 *    Rev 1.5   01 May 2000 22:00:40   whiteth
 * Added Atmel flash support code.
 *
 *    Rev 1.4   08 Mar 2000 17:18:00   whiteth
 * Restructured the BANK_INFO array and added support for new Intel Flash ROM.
 *
 *    Rev 1.3   02 Mar 2000 17:43:38   whiteth
 * Put back the calls to the macros since this function *cannot* make
 * external function references.  The macros have been modified.  See the
 * flashrw.h header file for more information.
 *
 *    Rev 1.2   01 Mar 2000 13:31:16   rossst
 * Replaced ENTER_READ/WRITE_MODE macros with function
 * calls toi EnterRead/WriteMode().
 *
 *    Rev 1.1   26 Oct 1999 10:19:08   velusws
 * Hardcoded array parameter to workaround Arm2.5 array sizing bug.
 *
 *    Rev 1.0   13 Oct 1999 15:45:36   whiteth
 * Initial revision.
 *
 ****************************************************************************/

