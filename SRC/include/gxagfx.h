/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                   Conexant Systems Inc. (c) 1998 - 2003                  */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       gxagfx.h
 *
 *
 * Description:    GXA graphics accelerator driver top level header file
 *
 *
 * Author:         Eric Ching and others
 *
 ****************************************************************************/
/* $Header: gxagfx.h, 8, 11/13/03 3:15:46 PM, Dave Wilson$
 ****************************************************************************/

#ifndef _GXAGFX_H_
#define _GXAGFX_H_

/*****************/
/* Include Files */
/*****************/
#include "stbcfg.h"
#include "kal.h"
#include "gfxtypes.h"
#include "gfxlib.h"

/****************************************************************************
 * RGB to YCrCb color space conversion macros
 ***************************************************************************/
#ifndef BOUND 
#define BOUND(x,top,bot) ( (x > top) ? top : ( (x < bot) ? bot : x) )
#endif

// These macros convert Gamma RGB to YCrCb values using a scale factor
// of 1024 to avoid floating point and integer division.
// Bounds checking is done for safety but worst case values should
// be within valid ranges. It is possible to generate Cr and Cb values
// of 15 in the worst case due to round off error.
// If these are not illegal Cr,Cb values for hardware operations
// then bounds checking could be disabled. Tweaking the coefficients
// could also be done to avoid illegal values if a little more error in
// the conversion can be tolerated.

#ifndef RGBTOY
#define RGBTOY(r,g,b)  (BOUND((((263*r+516*g+100*b)>>10)+16),235,16))
#endif
#ifndef RGBTOCR
#define RGBTOCR(r,g,b) (BOUND((((450*r-377*g-73*b)>>10)+128),240,16))
#endif
#ifndef RGBTOCB
#define RGBTOCB(r,g,b) (BOUND((((-152*r-298*g+450*b)>>10)+128),240,16))
#endif

/****************************************************************************
 * Graphics Blt Functions
 ***************************************************************************/

u_int32 GfxSolidBlt(PGFX_BITMAP pDstBitmap,
                   PGFX_RECT pDstRect,
                   PGFX_OP pOpcode);

u_int32 GfxPatternBlt(PGFX_PAT pPattern,
                      PGFX_XY pPatXY,
                      PGFX_BITMAP pDstBitmap,
                      PGFX_RECT pDstRect,
                      PGFX_OP pOpcode);

u_int32 GfxCopyBlt(PGFX_BITMAP pSrcBitmap,
                   PGFX_RECT pSrcRect,
                   PGFX_BITMAP pDstBitmap,
                   PGFX_XY pDstPt,
                   PGFX_OP pOpcode);

u_int32 GfxTextBlt( char * String,
                    PGFX_FONT pFont,
                    PGFX_BITMAP pDstBitmap,
                    PGFX_XY pDstPt, 
                    PGFX_OP pOpcode);
                    
u_int32 GfxTextBltEx( char * String,
                      PGFX_FONT pFont,
                      PGFX_BITMAP pDstBitmap,
                      PGFX_XY pDstPt, 
                      PGFX_OP pOpcode,
                      u_int8  pIndices[]); 
                    
/****************************************************************************
 * Graphics Line Functions
 ***************************************************************************/
u_int32 GfxPolyLine(PGFX_BITMAP pDstBitmap,
                    PGFX_XY pPoints,
                    u_int32 NumPoints,
                    PGFX_OP pOpcode);

/*************************************/
/* Higher Level Graphics Functions   */
/*************************************/

/****************************************************************************
 * Graphics Object Functions
 * Used to create higher level graphics objects/resources.
 * Not required if a resource has already been created by another subsystem.
 ***************************************************************************/
PGFX_BITMAP GfxCreateBitmap(u_int8 Type,
                            u_int8 Bpp,
                            u_int16 Width,
                            u_int16 Height,
                            void *BitmapBits);

u_int32 GfxDeleteBitmap(PGFX_BITMAP pBitmap);

PGFX_PAT GfxCreatePattern(u_int8 Type,
                          u_int8 Bpp,
                          u_int8 PatSize,
                          void *PatBits);

u_int32 GfxDeletePattern(PGFX_PAT pPattern);

PGFX_FONT GfxCreateFontBitmap(u_int8 WidthType,
                            u_int16 CharWidth,  
                            u_int16 CharHeight,
                            u_int8 FirstChar,
                            u_int8 LastChar,
                            void *pBits);

void GfxDeleteFontBitmap(PGFX_FONT pFont);

bool GfxGetFontCharacterSize(GFX_FONT *pFont, 
                             int iChar, 
                             int *pWidth, 
                             int *pHeight);
                            
bool GfxGetFontCellSize(GFX_FONT *pFont, 
                        int *pWidth, 
                        int *pHeight);
                            
/****************************************************************************
 * Palette Functions
 * Used to set and get palette entries for bitmap resources created using
 * GfxCreateBitmap. Changing the palette for pattern resources is not
 * necessary since a palette for a pattern is set by changing the palette
 * for the bitmap resource into which the pattern will be drawn.
 * Only useful for indexed pixel formats.
 ***************************************************************************/
u_int32 GfxSetPaletteEntries(PGFX_BITMAP pBitmap, u_int32 Start_Index,
                        u_int32 Num_Entries, PGFX_COLOR pColor);

u_int32 GfxGetPaletteEntries(PGFX_BITMAP pBitmap, u_int32 Start_Index,
                        u_int32 Num_Entries, PGFX_COLOR pColor);

u_int32 GfxGetPaletteSizeAndColorSpace(u_int32 dwOsdMode, bool *pbRGB, 
                        u_int32 *pPalSize);

/* Definitions for palette loading error conditions. */
#define ERR_BAD_START_INDEX     1
#define ERR_BAD_NUM_ENTRIES     2
#define ERR_BAD_GFX_FLAG        3
#define ERR_BAD_OSD_MODE        4
#define ERR_FLAG_NOT_MATCH_MODE 5
#define ERR_FLAG_NO_PALETTE     6
#define ERR_FLAG_BAD_OSD_RGN    7
#define ERR_FLAG_BAD_OSD_MODE   8
#define ERR_FLAG_BAD_PTR        9

#endif // _GXAGFX_H_

/****************************************************************************
 * Modifications:
 * $Log: 
 *  8    mpeg      1.7         11/13/03 3:15:46 PM    Dave Wilson     CR(s): 
 *        7939 7940 Added prototypes for new functions GfxGetFontCharacterSize 
 *        and GfxGetFontCellSize.
 *  7    mpeg      1.6         9/22/03 11:58:26 AM    Dave Wilson     SCR(s) 
 *        7006 :
 *        Added several new return codes which are used by the Get and Set 
 *        palette
 *        functions.
 *        Added prototype for new function GfxGetPaletteSizeAndColorSpace.
 *        
 *  6    mpeg      1.5         2/13/03 7:30:54 PM     Lucy C Allevato SCR(s) 
 *        5503 :
 *        add back the include files which were missing from the previous 
 *        version.  The include files are stbcfg.h, kal.h, gfxtypes.h and 
 *        gfxlib.h 
 *        
 *  5    mpeg      1.4         2/13/03 12:27:06 PM    Matt Korte      SCR(s) 
 *        5479 :
 *        Removed old header reference
 *        
 *  4    mpeg      1.3         10/2/02 12:27:58 PM    Dave Wilson     SCR(s) 
 *        4621 :
 *        Added code and defintions to support greyscale font rendering.
 *        
 *  3    mpeg      1.2         11/13/01 2:05:12 PM    Dave Wilson     SCR(s) 
 *        2893 :
 *        Previous version could not be included alongside OSDLIBC.H due to 
 *        multiple
 *        definitions of BOUND. Changed the #ifdef to ensure that it is not 
 *        dependent
 *        on the name of the other header.
 *        
 *  2    mpeg      1.1         1/23/01 10:20:34 AM    Quillian Rutherford New 
 *        (fixed) BOUND macro
 *        
 *  1    mpeg      1.0         3/23/00 3:51:12 PM     Lucy C Allevato 
 * $
 * 
 *    Rev 1.6   22 Sep 2003 10:58:26   dawilson
 * SCR(s) 7006 :
 * Added several new return codes which are used by the Get and Set palette
 * functions.
 * Added prototype for new function GfxGetPaletteSizeAndColorSpace.
 *
 * 
 *    Rev 1.5   13 Feb 2003 19:30:54   goldenx
 * SCR(s) 5503 :
 * add back the include files which were missing from the previous version.  The include files are stbcfg.h, kal.h, gfxtypes.h and gfxlib.h 
 * 
 *    Rev 1.4   13 Feb 2003 12:27:06   kortemw
 * SCR(s) 5479 :
 * Removed old header reference
 * 
 *    Rev 1.3   02 Oct 2002 11:27:58   dawilson
 * SCR(s) 4621 :
 * Added code and defintions to support greyscale font rendering.
 * 
 *    Rev 1.2   13 Nov 2001 14:05:12   dawilson
 * SCR(s) 2893 :
 * Previous version could not be included alongside OSDLIBC.H due to multiple
 * definitions of BOUND. Changed the #ifdef to ensure that it is not dependent
 * on the name of the other header.
 * 
 *    Rev 1.1   23 Jan 2001 10:20:34   rutherq
 * New (fixed) BOUND macro
 * 
 *    Rev 1.0   23 Mar 2000 15:51:12   eching
 * Initial revision.
****************************************************************************/

