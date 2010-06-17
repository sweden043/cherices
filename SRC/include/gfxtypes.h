/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*           Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002         */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       gfxtypes.h
 *
 *
 * Description:    Type definitions used by GFXLIB and GXAGFX modules
 *
 *
 * Author:         Eric Ching
 *
 ****************************************************************************/
/* $Header: gfxtypes.h, 11, 4/8/03 10:56:40 AM, Dave Wilson$
 ****************************************************************************/
 
#ifndef _GFXTYPES_H_
#define _GFXTYPES_H_

#include "kal.h"

typedef struct _GFX_XY {
   int16 X;
   int16 Y;
} GFX_XY, *PGFX_XY;

typedef GFX_XY GFX_PT;
typedef PGFX_XY PGFX_PT;

typedef struct _GFX_ARGB {
   u_int8 R;
   u_int8 G;
   u_int8 B;
   u_int8 A;
} GFX_ARGB, *PGFX_ARGB;

typedef struct _GFX_AYCC {
   u_int8 Y;
   u_int8 Cb;
   u_int8 Cr;
   u_int8 A;
} GFX_AYCC, *PGFX_AYCC;

typedef union _GFX_CVALUE {
         GFX_ARGB      argb;
         GFX_AYCC      aycc;
         u_int32       dwVal;
} GFX_CVALUE, *PGFX_CVALUE;

typedef struct _GFX_COLOR {
         GFX_CVALUE  Value;
         u_int8      Flags;
} GFX_COLOR, *PGFX_COLOR;

#define GFX_CF_RGB   0x01
#define GFX_CF_YCC   0x02
#define GFX_CM_TYPE  0x03
#define GFX_CF_ALPHA 0x80

typedef struct _GFX_RECT {
   int16 Left;
   int16 Top;
   int16 Right;
   int16 Bottom;
} GFX_RECT, *PGFX_RECT;

#define GFX_BITMAP_CURRENT_VERSION 0

typedef struct _GFX_BITMAP {
   u_int16 Version; /* Set to GFX_BITMAP_CURRENT_VERSION */
   u_int16 VerSize;
   u_int8  Type;
   u_int8  Bpp;
   u_int16 Height;
   u_int16 Width;
   u_int16 Stride;
   u_int32 dwRef;
#ifdef PCI_GXA
   u_int32 PCIOffset;
#endif
   void * pPalette;
   void * pBits;
} GFX_BITMAP, *PGFX_BITMAP;

// Use bit 8 as a flag to indicate whether type contains alpha channel
// Use bit 7 as a flag to indicate whether type is 1BPP special case
// Use bit 3 as a flag to indicate whether type is YCC/NONRGB
// If the type is a color format then the BPP is given by bits 1 and 0.
// 4 BPP is currently not supported by the hardware and is assigned the
// hardware reserved value of 00.
// The entire hardware encoding to be used in the format field of the HW
// context register is given by bits 4 to 0.
#define GFX_TF_ALPHA       0x80
#define GFX_TF_NONRGB      0x04
#define GFX_1BPP           0x40
#define GFX_4BPP           0x00
#define GFX_8BPP           0x01
#define GFX_16BPP          0x02
#define GFX_32BPP          0x03
#define GFX_NBPP_MASK      0x03
#define GFX_HWFMT_MASK     0x1F

#define GFX_MONO           0x40
#define GFX_ARGB4          0x00
#define GFX_ARGB8          0x01
#define GFX_ARGB16_4444    0x82
#define GFX_ARGB16_1555    0x92
#define GFX_ARGB16_0565    0x0A
#define GFX_ARGB32         0x83
#define GFX_AYCC4          0x04
#define GFX_AYCC8          0x05
#define GFX_AYCC16_4444    0x86
#define GFX_AYCC16_2644    0x96
#define GFX_AYCC16_0655    0x0E
#define GFX_AYCC32         0x87

#define GFX_PAT_CURRENT_VERSION 0

typedef struct _GFX_PAT {
   u_int16 Version; /* Set to GFX_PAT_CURRENT_VERSION */
   u_int16 VerSize;
   u_int8 Type;
   u_int8 Bpp;
   u_int8 PatSize;
   u_int32 dwRef;
#ifdef PCI_GXA
   u_int32 PCIOffset;
#endif
   void * pPalette;
   void * pPatBits;
} GFX_PAT, *PGFX_PAT;

#define GetBitmapOSDHANDLE(screen) (OSDHANDLE)(screen->dwRef)

#define GFX_PAT_NONE    0
#define GFX_PAT_2x2     2
#define GFX_PAT_4x4     4
#define GFX_PAT_8x8     8
#define GFX_PAT_16x16   16
#define GFX_PAT_32x32   32

#define GFX_PAT_SIZE_8x8        8
#define GFX_PAT_SIZE_16x16      16
#define GFX_PAT_SIZE_32x32      32
#define GFX_PAT_NUM8x8_BYTES    8
#define GFX_PAT_NUM16x16_BYTES  32
#define GFX_PAT_NUM32x32_BYTES  128

#define GFX_FONT_CURRENT_VERSION 4

typedef struct _GFX_FONT
{
   u_int16 Version;     /* Set to GFX_FONT_CURRENT_VERSION */
   u_int16 VerSize;
   u_int8  WidthType;
   u_int16 CharWidth;   
   u_int16 CharHeight;
   u_int8  FirstFont;
   u_int8  LastFont;
   #ifdef PCI_GXA
   u_int32 PCIOffset;
   #endif
   void   *pBits;
   u_int8 *pWidths;     /* Added in version 2 - pointer to width table      */
   u_int8  BitDepth;    /* Added in version 2 - 1 for mono, 2 for greyscale */
   u_int16 MapHeight;   /* Added in version 3 - Height of glyph bitmap cell */
   u_int16 HeightPad;   /* Added in version 4 - Padding space between vertically adjacent lines of text */
   u_int16 WidthPad;    /* Added in version 4 - Padding space between rendered characters */
   u_int8  FactCharWidth;    /*add for chinese the space between chars*/
} GFX_FONT, *PGFX_FONT;

#define GFX_FONTWIDTH_FIXED      0x01
#define GFX_FONTWIDTH_VARIABLE   0x02
#define GFX_FONTTYPE_ANTIALIASED 0x03
#define GFX_FONTTYPE_AAFIXED     0x04

typedef struct _GFX_OP {
u_int8 ROP;
u_int8 BltControl;
u_int8 AlphaUse;
u_int8 Alpha;
} GFX_OP, *PGFX_OP;

// ROP Defines
#define GFX_ROP_COPY        0x00
#define GFX_ROP_AND         0x01
#define GFX_ROP_AND_NOTDST  0x02
#define GFX_ROP_BLACKNESS   0x03
#define GFX_ROP_OR_NOTDST   0x04
#define GFX_ROP_XNOR        0x05
#define GFX_ROP_NOTDST      0x06
#define GFX_ROP_NOR         0x07
#define GFX_ROP_OR          0x08
#define GFX_ROP_NOP         0x09
#define GFX_ROP_XOR         0x0A
#define GFX_ROP_NOTSRC_AND  0x0B
#define GFX_ROP_WHITENESS   0x0C
#define GFX_ROP_NOTSRC_OR   0x0D
#define GFX_ROP_NAND        0x0E
#define GFX_ROP_NOTSRC      0x0F

// BltControl Defines
#define GFX_OP_ROP            0x01
#define GFX_OP_TRANS          0x02
#define GFX_OP_ALPHA          0x04
#define GFX_OP_REVERSE        0x08
#define GFX_OP_PRESERVE_ALPHA 0x10
#define GFX_OP_COPY           0x00
#define GFX_OP_COPY_TRANS     0x02
#define GFX_OP_ROP_TRANS      0x03
#define GFX_OP_ALPHA_TRANS    0x06

// AlphaUse Defines
#define GFX_BLEND_INSIDE_OUT    0x80
#define GFX_BLEND_STORE_MASK    0x07
#define GFX_BLEND_STORE_DST     0x01
#define GFX_BLEND_STORE_SRC     0x02
#define GFX_BLEND_STORE_FIXED   0x04
#define GFX_BLEND_USE_MASK      0x70
#define GFX_BLEND_USE_DST       0x10
#define GFX_BLEND_USE_SRC       0x20
#define GFX_BLEND_USE_FIXED     0x40

#define GFXERR_NONE              0
#define GFXERR_NO_ROP_TO_YCC     1
#define GFXERR_CANNOT_CONVERT    2
#define GFXERR_BAD_ALPHA_SETUP   3
#define GFXERR_NO_ALPHA_IN_DST   4
#define GFXERR_ILLEGAL_4BPP_OP   5
#define GFXERR_PAT_WIDTH         6
#define GFXERR_NUM_POINTS        7
#define GFXERR_OUT_OF_MEMORY     8
#define GFXERR_NOT_SUPPORTED     9
#define GFXERR_SYSTEM_ERROR     10

// Defines for Blt Flags
typedef u_int32 GFX_BLTFLAGS;

#define GFX_BF_TRANS    0x01
#define GFX_BF_ROP      0x02
#define GFX_BF_REVERSE  0x04

#endif // _GFXTYPES_H_

/****************************************************************************
 * Modifications:
 * $Log: 
 *  11   mpeg      1.10        4/8/03 10:56:40 AM     Dave Wilson     SCR(s) 
 *        5974 :
 *        Added GFX_FONTTYPE_AAFIXED
 *        
 *  10   mpeg      1.9         2/28/03 2:19:16 PM     Dave Wilson     SCR(s) 
 *        5616 :
 *        Added current version numbers for various gfx objects.
 *        
 *  9    mpeg      1.8         12/10/02 10:48:30 AM   Lucy C Allevato SCR(s) 
 *        4973 :
 *        add a macro to get the osd handle from a bitmap
 *        
 *  8    mpeg      1.7         10/7/02 5:44:24 PM     Dave Wilson     SCR(s) 
 *        4750 :
 *        Bumped GFX_FONT version to 4 and added a couple of fields to allow 
 *        width
 *        and height padding.
 *        
 *  7    mpeg      1.6         10/2/02 12:27:52 PM    Dave Wilson     SCR(s) 
 *        4621 :
 *        Added code and defintions to support greyscale font rendering.
 *        
 *  6    mpeg      1.5         8/22/02 4:07:46 PM     Dave Wilson     SCR(s) 
 *        4453 :
 *        Previous version had ;s at the end of a couple of #defines. I 
 *        replaced
 *        these #defines with typedefs. Code using the types now builds!
 *        
 *  5    mpeg      1.4         8/2/02 1:10:00 PM      Dave Wilson     SCR(s) 
 *        4321 :
 *        Updated GFX_FONT to allow for cases where the font height is not a 
 *        multiple
 *        of 4. The blitter requires bitmap pointers to be word aligned and 
 *        this had
 *        caused some problems with previous versions of the 8x14 VGA font. The
 *         structure
 *        now allows you to define the character bitmap cell height 
 *        independently from
 *        the actual character height, for example, storing a 14 pixel high 
 *        character
 *        in a 16 pixel high cell.
 *        
 *  4    mpeg      1.3         7/31/02 5:44:28 PM     Dave Wilson     SCR(s) 
 *        3044 :
 *        Updated GFX_FONT structure to allow variable width fonts to be used. 
 *        New
 *        structure version number is 2. Existing code has been modified to 
 *        understand
 *        both version 1 and 2 of the structure.
 *        
 *  3    mpeg      1.2         5/24/00 6:41:28 PM     Lucy C Allevato Added 
 *        pPalette pointer to BITMAP and PATTERN structures.
 *        
 *  2    mpeg      1.1         3/30/00 10:42:28 AM    Lucy C Allevato Updated 
 *        types to use new GFX_OP and related field defines.
 *        
 *  1    mpeg      1.0         2/8/00 4:58:18 PM      Lucy C Allevato 
 * $
 * 
 *    Rev 1.10   08 Apr 2003 09:56:40   dawilson
 * SCR(s) 5974 :
 * Added GFX_FONTTYPE_AAFIXED
 * 
 *    Rev 1.9   28 Feb 2003 14:19:16   dawilson
 * SCR(s) 5616 :
 * Added current version numbers for various gfx objects.
 * 
 *    Rev 1.8   10 Dec 2002 10:48:30   goldenx
 * SCR(s) 4973 :
 * add a macro to get the osd handle from a bitmap
 * 
 *    Rev 1.7   07 Oct 2002 16:44:24   dawilson
 * SCR(s) 4750 :
 * Bumped GFX_FONT version to 4 and added a couple of fields to allow width
 * and height padding.
 * 
 *    Rev 1.6   02 Oct 2002 11:27:52   dawilson
 * SCR(s) 4621 :
 * Added code and defintions to support greyscale font rendering.
 * 
 *    Rev 1.5   22 Aug 2002 15:07:46   dawilson
 * SCR(s) 4453 :
 * Previous version had ;s at the end of a couple of #defines. I replaced
 * these #defines with typedefs. Code using the types now builds!
 * 
 *    Rev 1.4   02 Aug 2002 12:10:00   dawilson
 * SCR(s) 4321 :
 * Updated GFX_FONT to allow for cases where the font height is not a multiple
 * of 4. The blitter requires bitmap pointers to be word aligned and this had
 * caused some problems with previous versions of the 8x14 VGA font. The structure
 * now allows you to define the character bitmap cell height independently from
 * the actual character height, for example, storing a 14 pixel high character
 * in a 16 pixel high cell.
 * 
 *    Rev 1.3   31 Jul 2002 16:44:28   dawilson
 * SCR(s) 3044 :
 * Updated GFX_FONT structure to allow variable width fonts to be used. New
 * structure version number is 2. Existing code has been modified to understand
 * both version 1 and 2 of the structure.
 * 
 *    Rev 1.2   24 May 2000 17:41:28   eching
 * Added pPalette pointer to BITMAP and PATTERN structures.
 * 
 *    Rev 1.1   30 Mar 2000 10:42:28   eching
 * Updated types to use new GFX_OP and related field defines.
 * 
 *    Rev 1.0   08 Feb 2000 16:58:18   eching
 * Initial revision.
 * 
 *    Rev 1.7   07 Feb 2000 18:23:40   eching
 * Updated defines for bitmap pixel types/formats to match up with the HW GXA
 * definitions. Added version and version size fields to some of the structures
 * for future upgrade capability.
 * 
 *    Rev 1.6   10 Jan 2000 17:15:42   eching
 * Added LINEFLAGS type and defines to support line drawing.
 * 
 *    Rev 1.5   06 Jan 2000 17:59:08   eching
 * Changed the GFX_ALPHA data type to be a structure with flags and a value
 * for doing alpha blt code. Added GFX_BF_REVERSE for future support of
 * blitting backwards.
 * 
 *    Rev 1.4   28 Dec 1999 15:59:40   eching
 * Changed definition of GFX_MONO to pass as RGB for ROP blts.
 * 
 *    Rev 1.3   03 Dec 1999 14:23:58   achen
 * Removed FONTCHAR structure.
 * 
 *    Rev 1.2   01 Dec 1999 16:23:18   achen
 * Added GFX_FONT AND GFX_FONTCHAR structures for text.
 * 
 *    Rev 1.1   12 Nov 1999 12:04:54   eching
 * Changed YCC ordering to match up with DRM/OSD YCC ordering of bytes.
 * Removed index from color flags for now.
 * 
 *    Rev 1.0   21 Oct 1999 16:27:06   eching
 * Initial revision.
****************************************************************************/

