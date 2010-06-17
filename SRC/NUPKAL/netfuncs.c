/****************************************************************************/
/*                 Rockwell Semiconductor Systems - SABINE                  */
/****************************************************************************/
/*                                                                          */
/* Filename:           NETFUNCS.C                                            */
/*                                                                          */
/* Description:        Network command processing                           */
/*                                                                          */
/* Author:             Mustafa Ismail                                       */
/*                                                                          */
/* Copyright Conexant Systems 1999.                                         */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/
/*
$Header: netfuncs.c, 6, 2/13/03 1:52:42 PM, Bobby Bradford$
$Log: 
 6    mpeg      1.5         2/13/03 1:52:42 PM     Bobby Bradford  SCR(s) 5479 
       :
       Fixed warnings ... moved "stbcfg.h" to beginning of list
       also changed <nup.h> to "nup.h"
       
 5    mpeg      1.4         2/13/03 12:44:46 PM    Matt Korte      SCR(s) 5479 
       :
       Removed old header reference
       
 4    mpeg      1.3         1/31/01 2:40:20 PM     Tim White       DCS#0988: 
       Reclaim footprint space.
       
 3    mpeg      1.2         1/17/01 1:02:18 PM     Miles Bintz     tracker 
       #714.  Cleaned up warnings
       
 2    mpeg      1.1         8/29/00 8:46:18 PM     Miles Bintz     fixed things
       
 1    mpeg      1.0         8/29/00 5:39:56 PM     Ismail Mustafa  
$
 * 
 *    Rev 1.5   13 Feb 2003 13:52:42   bradforw
 * SCR(s) 5479 :
 * Fixed warnings ... moved "stbcfg.h" to beginning of list
 * also changed <nup.h> to "nup.h"
 * 
 *    Rev 1.4   13 Feb 2003 12:44:46   kortemw
 * SCR(s) 5479 :
 * Removed old header reference
 * 
 *    Rev 1.3   31 Jan 2001 14:40:20   whiteth
 * DCS#0988: Reclaim footprint space.
 * 
 *    Rev 1.2   17 Jan 2001 13:02:18   bintzmf
 * tracker #714.  Cleaned up warnings
 * 
 *    Rev 1.1   29 Aug 2000 19:46:18   bintzmf
 * fixed things
 * 
 *    Rev 1.0   29 Aug 2000 16:39:56   mustafa
 * Initial revision.
 *
 *
*/
#include "stbcfg.h"
#include "nup.h"
#include <stdio.h>
#include "kal.h"
#define INSTANTIATE_TABLES
#include "kalint.h"
#include "hwlib.h"

#include "globals.h"
#include "retcodes.h"
#include "fpleds.h"
#include "bldver.h"
#include "iic.h"
#include "startup.h"
#ifdef DRIVER_INCL_OSDLIB
#include "osdttx.h"
#endif /* DRIVER_INCL_OSDLIB */

extern size_t data_write(u_int8 *addr, size_t count, voidF buffer);
extern int block_erase(u_int8 *start_addr, size_t block_size);
extern size_t data_read(u_int8 *addr, size_t count, voidF buffer);
extern int demod_setsource(int iNewSource);
extern bool block_check(u_int8 *addr, size_t block_size);
extern int EraseBlock(u_int8 *pBlock, u_int32 Size);

extern FLASH_INFO FlashInfo;

#ifdef DRIVER_INCL_PROFILER
extern u_int32 gpdwProfilerPCBuffer;
extern u_int32 gpdwProfWriteAddr;
extern u_int32 gpdwProfEndAddr;
extern u_int32 gpdwProfReadAddr;
#endif /* DRIVER_INCL_PROFILER */

u_int32 my_recv(int ChildSock, char buff[], u_int32 ReadSize);
extern u_int32 uTraceFlags;

u_int32 netfuncs_flash_offset, netfuncs_erase_offset, netfuncs_bank, netfuncs_sector;
u_int8 *RomFlashImageAddr;
char szStringNet[20];

void FlashInit()
{
    // Reset offsets
    netfuncs_flash_offset = netfuncs_erase_offset = 0;
    return;
}

bool FlashIt(u_int8 *pData, u_int32 len)
{
    size_t retcount;
    u_int8 *pSrc, *pDst, *start;
    u_int32 length;
    int i;

    // If this is the first time FlashIt() has been called since the
    // last call to FlashInit(), initialize the starting bank and sector
    if ( (!netfuncs_flash_offset) && (!netfuncs_erase_offset) )
    {
      // Ensure a valid RomFlashImageAddr value has been setup first
      if((RomFlashImageAddr < FlashInfo.RomBase) ||
                (RomFlashImageAddr > (FlashInfo.RomBase + FlashInfo.RomSize)))
      {
         trace("FlashIt() failed, invalid RomFlashImageAddr=0x%x\n", RomFlashImageAddr);
         #ifdef DRIVER_INCL_FPLEDS
         LEDString("Err");
         #endif
         return FALSE;
      }

      // First, find the flash ROM bank number (netfuncs_bank)
      for(netfuncs_bank=0;netfuncs_bank<FlashInfo.NumBanks;netfuncs_bank++)
      {
        if(RomFlashImageAddr<(FlashInfo.RomInfo[netfuncs_bank].BankBase+
                        FlashInfo.RomInfo[netfuncs_bank].BankSize))
          break;
      }

      // Second, find the flash ROM sector number (netfuncs_sector)
      for(netfuncs_sector=0;netfuncs_sector<FlashInfo.RomInfo[netfuncs_bank].NumSectors;
                                                        netfuncs_sector++)
      {
        if(FlashInfo.RomInfo[netfuncs_bank].Flags & FLASH_FLAGS_UNIFORM_SECTOR_SIZE)
        {
          if(RomFlashImageAddr < ((FlashInfo.RomInfo[netfuncs_bank].Sector[0].Start +
                FlashInfo.RomInfo[netfuncs_bank].Sector[0].Length * netfuncs_sector) +
                FlashInfo.RomInfo[netfuncs_bank].Sector[0].Length))
          {
             break;
          }
        }
        else
        {
          if(RomFlashImageAddr<(FlashInfo.RomInfo[netfuncs_bank].Sector[netfuncs_sector].Start+
                FlashInfo.RomInfo[netfuncs_bank].Sector[netfuncs_sector].Length))
          {
             break;
          }
        }
      }
    }

    // Erase next sector if we need to
    //
    // NOTE:  If the image address (RomFlashImageAddr) from the demux code
    // does not start on a Flash ROM sector boundary, everything in the sector
    // will be waxed!!!!!  This could be the boot block, config space, etc...
    // Without the ability to mallocate large chunks (several MB's) of memory
    // or have the ability to relocate an image on the fly, this has to be
    // known by the user of PSIPACK and the build tools (armlink -Base).  Care
    // must be taken here...
    //
    if ( (netfuncs_flash_offset + len) > netfuncs_erase_offset)
    {
        if(FlashInfo.RomInfo[netfuncs_bank].Flags & FLASH_FLAGS_UNIFORM_SECTOR_SIZE)
        {
           start  = FlashInfo.RomInfo[netfuncs_bank].Sector[0].Start +
                    FlashInfo.RomInfo[netfuncs_bank].Sector[0].Length * netfuncs_sector;
           length = FlashInfo.RomInfo[netfuncs_bank].Sector[0].Length;
        }
        else
        {
           start  = FlashInfo.RomInfo[netfuncs_bank].Sector[netfuncs_sector].Start;
           length = FlashInfo.RomInfo[netfuncs_bank].Sector[netfuncs_sector].Length;
        }
        retcount = EraseBlock((u_int8 *)start, length);
        if (retcount != 0)
           return FALSE;
        netfuncs_erase_offset += (length - ((netfuncs_erase_offset +
                                  (u_int32)RomFlashImageAddr) - (u_int32)start));
        if((netfuncs_sector+1)==FlashInfo.RomInfo[netfuncs_bank].NumSectors)
        {
          if((netfuncs_bank+1)==FlashInfo.NumBanks)
          {
            trace("FlashIt() failed, image spilling over the end of the ROM space=0x%x\n",
                                (u_int32)(RomFlashImageAddr + netfuncs_erase_offset));
            #ifdef DRIVER_INCL_FPLEDS
            LEDString("Err");
            #endif
            return FALSE;
          }
          else
          {
            ++netfuncs_bank;
            netfuncs_sector=0;
          }
        }
        else
        {
          ++netfuncs_sector;
        }
    }

    // Write data to flash ROM
    retcount = data_write( (u_int8 *)  (RomFlashImageAddr + netfuncs_flash_offset),
                        (size_t) len, pData);
    if (retcount != len){
        trace("flash data_write failed. Returned 0x%x instead of %d\n", retcount, len);
        trace("Flash write Address=0x%x\n", RomFlashImageAddr + netfuncs_flash_offset);
        #ifdef DRIVER_INCL_FPLEDS
        LEDString("Err");
        #endif
        return FALSE;
    }

    // Verify data written to flash ROM
    for(i=netfuncs_flash_offset,pSrc=pData,pDst=(RomFlashImageAddr+netfuncs_flash_offset);
                                i<(netfuncs_flash_offset+len);i++)
    {
      if(*pSrc++!=*pDst++)
      {
            trace("FlashIt() failed, data write verification failed!!! at 0x%x\n",
                                (u_int32)(RomFlashImageAddr + netfuncs_flash_offset));
            #ifdef DRIVER_INCL_FPLEDS
            LEDString("Err");
            #endif
            return FALSE;
      }
    }

    netfuncs_flash_offset += len;
    return TRUE;
}



int EraseBlock(u_int8 *pBlock, u_int32 Size)
{
    int retcount;

    // Check for need to erase (not all 1's)
    if (!block_check(pBlock, Size))
    {

        // Erase sector
        retcount = block_erase(pBlock, Size);
        if (retcount != 0)
        {
              trace("block_erase failed on address 0x%x, size 0x%08x\n",
                        (u_int32)pBlock, Size);
              #ifdef DRIVER_INCL_FPLEDS
              LEDString("Err");
              #endif
              return -1;
        }

        // Verify the erase
        if (!block_check(pBlock, Size))
        {
              trace("block was not erased block address=0x%x, size 0x%08x!!!!\n",
                        (u_int32)pBlock, Size);
              #ifdef DRIVER_INCL_FPLEDS
              LEDString("Err");
              #endif
              return -1;
        }
    }
    return 0;
}

