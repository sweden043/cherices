/***************************************************************************
                                                                          
   Filename: GFXLIB.H

   Description: CN49XX Graphics Library Include

   Created: 3/3/2000 by Eric Ching

   Copyright Conexant Systems, 2000
   All Rights Reserved.

****************************************************************************/

/***************************************************************************
                     PVCS Version Control Information
$Header: gfxlib.h, 5, 7/15/03 11:52:30 AM, Dave Wilson$
$Log: 
 5    mpeg      1.4         7/15/03 11:52:30 AM    Dave Wilson     SCR(s) 6914 
       :
       Added prototype for GfxPrepareBufferForGXA. This function is used to 
       clean
       and flush any image data from the D-cache if a buffer is being shared 
       between
       the CPU and the hardware blitter.
       
 4    mpeg      1.3         1/3/02 12:00:16 PM     Quillian Rutherford SCR(s) 
       2992 :
       Added prototype for GfxIsGXACmdDone
       
 3    mpeg      1.2         3/23/00 3:50:14 PM     Lucy C Allevato Added 
       drawing attribute functions to library.
       
 2    mpeg      1.1         3/7/00 6:14:48 PM      Lucy C Allevato Changed 
       function name to GfxFillMem to match name in library.
       
 1    mpeg      1.0         3/6/00 1:21:22 PM      Lucy C Allevato 
$
 * 
 *    Rev 1.4   15 Jul 2003 10:52:30   dawilson
 * SCR(s) 6914 :
 * Added prototype for GfxPrepareBufferForGXA. This function is used to clean
 * and flush any image data from the D-cache if a buffer is being shared between
 * the CPU and the hardware blitter.
 * 
 *    Rev 1.3   03 Jan 2002 12:00:16   rutherq
 * SCR(s) 2992 :
 * Added prototype for GfxIsGXACmdDone
 * 
 *    Rev 1.2   23 Mar 2000 15:50:14   eching
 * Added drawing attribute functions to library.
 * 
 *    Rev 1.1   07 Mar 2000 18:14:48   eching
 * Changed function name to GfxFillMem to match name in library.
 * 
 *    Rev 1.0   06 Mar 2000 13:21:22   eching
 * Initial revision.
****************************************************************************/

#ifndef _GFXLIB_H_
#define _GFXLIB_H_

/****************************************************************************
 * Software Only Functions
 ***************************************************************************/
void SWSolidFill(int pDest, int Pitch, int ExtX, int ExtY, int Color, int X, int Y, int Bpp);
void SWXORFill(int pDest, int Pitch, int ExtX, int ExtY, int Color, int X, int Y, int Bpp);
void SWCopy2Screen(int pDest, int pSrc, int DstPitch, int SrcPitch, int ExtX, int ExtY, int DstX, int DstY, int Bpp);
void SWCopy2Memory(int pDest, int pSrc, int DstPitch, int SrcPitch, int ExtX, int ExtY, int SrcX, int SrcY, int Bpp);
void FillBytes(int pDest, int fillbyte, int Count);
int RepColor(int color, int Bpp);

/***************************************************************************
 Function to check if gxa engine is idle (indicates previous command done)
***************************************************************************/
int GfxIsGXACmdDone (u_int32 iWaitTime);

/***************************************************************************
 Function to ensure all data written to a buffer by the CPU us flushed 
 through the D-cache into SDRAM.
***************************************************************************/
void GfxPrepareBufferForGXA (u_int8 *addr, u_int32 size);

/***************************************************************************
 Init Function
***************************************************************************/
void GfxInit (u_int8 Bpp);

/***************************************************************************
 Low Level Bitblt Functions
***************************************************************************/
void GfxFillMem(u_int32 *pDest,
                int Pitch,
                int ExtX,
                int ExtY,
                u_int32 FillColor,
                int X,
                int Y,
                int Bpp,
                u_int8 Rop);

void GfxCopyRectMem(u_int32 *pDest,
                u_int32 *pSrc,
                int DstPitch,
                int SrcPitch,
                int ExtX,
                int ExtY,
                int SrcX,
                int SrcY,
                int DstX,
                int DstY,
                int Bpp);

void GfxCopy2Screen(u_int32 *pDest,
                    u_int32 *pSrc,
                    int DstPitch,
                    int SrcPitch,
                    int ExtX,
                    int ExtY,
                    int DstX,
                    int DstY,
                    int Bpp);

void GfxCopy2Mem(u_int32 *pDest,
                 u_int32 *pSrc,
                 int DstPitch,
                 int SrcPitch,
                 int ExtX,
                 int ExtY,
                 int SrcX,
                 int SrcY,
                 int Bpp);

/***************************************************************************
 Low Level Drawing Attribute Functions
***************************************************************************/
void GfxSetFGColor(u_int32 Color);

u_int32 GfxGetFGColor(void);

void GfxSetBGColor(u_int32 Color);

u_int32 GfxGetBGColor(void);

void GfxSetLinePattern(u_int32 Color);

u_int32 GfxGetLinePattern(void);

#endif // _GFXLIB_H_

