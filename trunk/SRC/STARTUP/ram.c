 /****************************************************************************/ 
 /*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
 /*                       SOFTWARE FILE/MODULE HEADER                        */
 /*          Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002          */
 /*                              Austin, TX                                  */
 /*                         All Rights Reserved                              */
 /****************************************************************************/
 /*
  * Filename:    ram.c
  *
  *
  * Description: RAM routines.
  *
  *
  * Author:      Tim Ross
  *
  ****************************************************************************/
 /* $Header: ram.c, 18, 3/3/04 2:00:23 PM, Craig Dry$
  ****************************************************************************/ 

/******************/
/* Include Files  */
/******************/
#include "stbcfg.h"
#include "basetype.h"
#include "ram.h"

/**********************/
/* Local Definitions  */
/**********************/

/***************/
/* Global Data */
/***************/
extern int RAMWidth;


/********************************************************************/
/*  GetRAMSize                                                      */
/*                                                                  */
/*  DESCRIPTION:                                                    */
/*      Query the RAM controller registers to find out how much     */
/*      RAM is installed.                                           */
/*                                                                  */
/*  PARAMETERS:                                                     */
/*      None.                                                       */
/*                                                                  */
/*  RETURNS:                                                        */
/*      Size of physical RAM in bytes.                              */
/********************************************************************/
int GetRAMSize()
{
    int RAMSize = 0;

    LPREG pMemCfg = (LPREG)SDR_MEM_CFG_REG;
    HW_DWORD Config;

    Config = *pMemCfg;

    switch ((CNXT_GET_VAL(&Config, SDR_MEM_CFG_RANK0_SIZE_MSB_MASK) << 2) | 
	    CNXT_GET_VAL(&Config, SDR_MEM_CFG_RANK0_SIZE_MASK))
    {
            case SDR_8MB_64:
                RAMSize += 0x800000;
                break;
            case SDR_4MB_16:
                RAMSize += 0x400000;
                break;
            case SDR_8MB_16:
                RAMSize += 0x800000;
                break;
            case SDR_16MB_64:
                RAMSize += 0x1000000;
                break;
            case SDR_32MB_128:
                RAMSize += 0x2000000;
                break;
            case SDR_64MB_256:
                RAMSize += 0x4000000;
                break;
    }

    if (!CNXT_GET(&Config, SDR_MEM_CFG_RANK1_EMPTY_MASK))
    {
         switch ((CNXT_GET_VAL(&Config, SDR_MEM_CFG_RANK1_SIZE_MSB_MASK) << 2) | 
		 CNXT_GET_VAL(&Config, SDR_MEM_CFG_RANK1_SIZE_MASK))
	    {
                case SDR_8MB_64:
                    RAMSize += 0x800000;
                    break;
                case SDR_4MB_16:
                    RAMSize += 0x400000;
                    break;
                case SDR_8MB_16:
                    RAMSize += 0x800000;
                    break;
                case SDR_16MB_64:
                    RAMSize += 0x1000000;
                    break;
                case SDR_32MB_128:
                    RAMSize += 0x2000000;
                    break;
                case SDR_64MB_256:
                    RAMSize += 0x4000000;
                    break;
            }
    }

    /*
     * Take into account the width of the memory bus.  There are only 2
     * possible widths at the moment:  32 and 16.  If 32, leave the value
     * alone.  If 16, divide by 2.  Anything else is an error...
     */
    switch(RAMWidth)
    {
        /*
         * For 16bit memory bus, divide the size by 2
         */
        case 16:
            RAMSize >>= 1;
            break;

        /*
         * For 32bit memory bus, divide the size by 1
         */
        case 32:
            break;

        /*
         * If RAMWidth indicates anything other than 16 or 32, set
         * the RAMSize to 0 to force an error...
         */
        default:
            RAMSize = 0;
    }


#ifndef DLOAD
    /*
     * A hack.....  What happens is later on when buffers are allocated
     * by calling OTV's direct_mem_malloc() is that we get an address above
     * the 32MB barrier for a buffer which we use as a hardware buffer
     * for PSI.  The pawser microcode cannot handle an address above
     * the 32MB limit. This limitation should go away in newer chips
     * equipped with Pinky.
     */

     if (RAMSize > 0x02000000)
         RAMSize = 0x02000000;
#endif

    return( RAMSize );
}
    
/********************************************************************/
/*  GetRAMWidth                                                     */
/*                                                                  */
/*  DESCRIPTION:                                                    */
/*      Determine the width of the memory bus.                      */
/*                                                                  */
/*  PARAMETERS:                                                     */
/*      None.                                                       */
/*                                                                  */
/*  RETURNS:                                                        */
/*      Width of the memory bus.                                    */
/********************************************************************/
int GetRAMWidth()
{
    int RAMWidth = 0;

#if ((RAM_BUS_WIDTH == RAM_BUS_32BITS_WIDE) || (RAM_BUS_WIDTH == RAM_BUS_32BITS_WIDE_FORCED))
    RAMWidth = 32;
#elif ((RAM_BUS_WIDTH == RAM_BUS_16BITS_WIDE) || (RAM_BUS_WIDTH == RAM_BUS_16BITS_WIDE_FORCED))
    RAMWidth = 16;
#else /* RAM_BUS_WIDTH == AUTOSENSE */
    u_int32 width;
    width = CNXT_GET(PLL_CONFIG0_REG, PLL_CONFIG0_MEMORY_BUS_WIDTH_MASK);
    if(width == PLL_CONFIG0_MEMORY_BUS_32BITS_WIDE)
    {
        RAMWidth = 32;
    }
    else
    {
        RAMWidth = 16;
    }
#endif

    return (RAMWidth);
}

 /****************************************************************************
 * Modifications:
 * $Log: 
 *  18   mpeg      1.17        3/3/04 2:00:23 PM      Craig Dry       CR(s) 
 *        8495 : Create 2 new RAM_BUS_WIDTH settings allowing customer to force
 *         memory controller into 16 or 32 bit mode, regardless of board jumper
 *         settings.
 *  17   mpeg      1.16        2/13/03 12:21:06 PM    Matt Korte      SCR(s) 
 *        5479 :
 *        Removed old header reference
 *        
 *  16   mpeg      1.15        1/23/03 4:31:58 PM     Tim White       SCR(s) 
 *        5298 :
 *        Added new function GetRAMWidth() which returns either 16 or 32 for 
 *        the width
 *        (in bits) of the memory bus.  This is determined by lookin at the
 *        RAM_BUS_WIDTH configuration option.  If it's set to 
 *        RAM_BUS_32BITS_WIDE (the default),
 *        the returned value is 32.  If it's set to RAM_BUS_16BITS_WIDE, the 
 *        returned
 *        value is 16.  If it's set to AUTOSENSE, it reads the CONFIG0 
 *        strapping 4 on
 *        the Bronco IRD to determine the width.
 *        The GetRAMSize() function needs to know the width in order to 
 *        determine the
 *        amount of memory available.
 *        
 *        
 *  15   mpeg      1.14        5/10/02 3:17:40 PM     Craig Dry       SCR(s) 
 *        3754 :
 *        Run Watchtv
 *        
 *  14   mpeg      1.13        1/11/02 3:46:00 PM     Dave Moore      SCR(s) 
 *        3006 :
 *        added SDR_64MB_256
 *        
 *        
 *  13   mpeg      1.12        9/19/01 4:57:46 PM     Tim White       SCR(s) 
 *        2659 :
 *        Cannot use Vendor_ID and Board_ID in download build.
 *        
 *        
 *  12   mpeg      1.11        9/6/01 3:31:40 PM      Tim White       SCR(s) 
 *        2592 :
 *        Artificially limit the amount of RAM in the system to 32MB since
 *        no Pawser buffer allocations can be above the 32MB boundary.
 *        
 *        
 *  11   mpeg      1.10        4/11/01 8:12:34 PM     Amy Pratt       DCS914 
 *        Removed Neches support.
 *        
 *  10   mpeg      1.9         9/11/00 4:37:10 PM     Ray Mack        Remove 
 *        hack for when Colorado would not report correct RAM size
 *        
 *  9    mpeg      1.8         8/30/00 11:55:18 AM    Tim White       Fixed 
 *        VxWorks compilation warnings.
 *        
 *  8    mpeg      1.7         6/7/00 2:24:28 PM      Tim White       
 *        Implemented the Eric Deal MC workaround in the codeldr.
 *        
 *  7    mpeg      1.6         5/9/00 2:43:30 PM      Tim White       Added 
 *        Colorado HW workaround for RAMSize in startup code.  Was missing...
 *        
 *  6    mpeg      1.5         1/12/00 5:06:14 PM     Tim Ross        Added 
 *        compilation switches for Colorado unique feature.
 *        
 *  5    mpeg      1.4         1/11/00 6:23:06 PM     Tim Ross        Added 
 *        SDRAM changes for Colorado.
 *        
 *  4    mpeg      1.3         11/18/99 8:09:48 PM    Dave Wilson     Changed 
 *        register access slightly to make it safe for vxWorks
 *        
 *  3    mpeg      1.2         10/19/99 3:29:36 PM    Tim Ross        Removed 
 *        SABINE and PID board dependencies.
 *        Made compatible with CN8600 CFG file.
 *        
 *  2    mpeg      1.1         9/30/99 5:36:46 PM     Tim Ross        made file
 *         conditionally set params for non-arm boards
 *        
 *  1    mpeg      1.0         7/1/99 1:05:54 PM      Tim Ross        
 * $
 * 
 *    Rev 1.16   13 Feb 2003 12:21:06   kortemw
 * SCR(s) 5479 :
 * Removed old header reference
 * 
 *    Rev 1.15   23 Jan 2003 16:31:58   whiteth
 * SCR(s) 5298 :
 * Added new function GetRAMWidth() which returns either 16 or 32 for the width
 * (in bits) of the memory bus.  This is determined by lookin at the
 * RAM_BUS_WIDTH configuration option.  If it's set to RAM_BUS_32BITS_WIDE (the default),
 * the returned value is 32.  If it's set to RAM_BUS_16BITS_WIDE, the returned
 * value is 16.  If it's set to AUTOSENSE, it reads the CONFIG0 strapping 4 on
 * the Bronco IRD to determine the width.
 * The GetRAMSize() function needs to know the width in order to determine the
 * amount of memory available.
 * 
 * 
 *    Rev 1.14   10 May 2002 14:17:40   dryd
 * SCR(s) 3754 :
 * Run Watchtv
 *
 ****************************************************************************/ 

