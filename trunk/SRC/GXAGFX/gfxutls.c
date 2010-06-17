/***************************************************************************
                                                                          
   Filename: GFXUTLS.C

   Description: Graphics Hardware Utility Routines

   Created: 1/19/2000 by Eric Ching

   Copyright Conexant Systems, 2000
   All Rights Reserved.

****************************************************************************/

/***************************************************************************
                     PVCS Version Control Information
$Header: gfxutls.c, 8, 2/13/03 11:55:54 AM, Matt Korte$
$Log: 
 8    mpeg      1.7         2/13/03 11:55:54 AM    Matt Korte      SCR(s) 5479 
       :
       Removed old header reference
       
 7    mpeg      1.6         8/23/01 1:16:58 PM     Quillian Rutherford SCR(s) 
       2525 :
       Moved the gxa idle check function out of here into gfxlib/copymem.c
       
       
 6    mpeg      1.5         8/21/01 6:00:46 PM     Quillian Rutherford SCR(s) 
       2525 :
       Added gxa idle api function
       
       
 5    mpeg      1.4         8/28/00 6:42:50 PM     Lucy C Allevato Fixes for 
       strict compiler checking of possible usage of variables before
       they are initialized.
       
 4    mpeg      1.3         8/23/00 3:00:46 PM     Lucy C Allevato Removed 
       relative include paths.
       
 3    mpeg      1.2         6/6/00 6:07:10 PM      Lucy C Allevato Fixed 
       comment style to not use C++ line comment.
       
 2    mpeg      1.1         5/24/00 8:35:18 PM     Lucy C Allevato Added 
       GfxConvertPitchToPixels to convert pitch in bytes to pixels.
       
 1    mpeg      1.0         3/23/00 3:54:00 PM     Lucy C Allevato 
$
 * 
 *    Rev 1.7   13 Feb 2003 11:55:54   kortemw
 * SCR(s) 5479 :
 * Removed old header reference
 * 
 *    Rev 1.6   23 Aug 2001 12:16:58   rutherq
 * SCR(s) 2525 :
 * Moved the gxa idle check function out of here into gfxlib/copymem.c
 * 
 * 
 *    Rev 1.4   28 Aug 2000 17:42:50   eching
 * Fixes for strict compiler checking of possible usage of variables before
 * they are initialized.
 * 
 *    Rev 1.3   23 Aug 2000 14:00:46   eching
 * Removed relative include paths.
 * 
 *    Rev 1.2   06 Jun 2000 17:07:10   eching
 * Fixed comment style to not use C++ line comment.
 * 
 *    Rev 1.1   24 May 2000 19:35:18   eching
 * Added GfxConvertPitchToPixels to convert pitch in bytes to pixels.
 * 
 *    Rev 1.0   23 Mar 2000 15:54:00   eching
 * Initial revision.
****************************************************************************/
#include "kal.h"
#include "gfxtypes.h"
#include "gfxlib.h"
#include "gxagfx.h"
#include "genmac.h"
#include "context.h"
#include "gfx_os.h"
#include "gfxutls.h"

/****************************************************************************
 * GfxTranslateTypeToFormat
 ***************************************************************************/
u_int32 GfxTranslateTypeToFormat(u_int8 Type)
{
u_int32 Format;

   switch (Type) {
      case GFX_MONO:
         ERRMSG((GXA_ERR_MSG,"GXA ERR: Can't translate MONO to a HW format\n"));
         Format = BM_FMT_RGB8;
         break;
      case GFX_ARGB8:
         Format = BM_FMT_RGB8;
         break;
      case GFX_AYCC8:
         Format = BM_FMT_YCC8;
         break;
      case GFX_ARGB16_0565:
         Format = BM_FMT_ARGB0565;
         break;
      case GFX_AYCC16_2644:
         Format = BM_FMT_AYCC2644;
         break;
      case GFX_AYCC16_0655:
         Format = BM_FMT_AYCC0655;
         break;
      case GFX_ARGB16_1555:
         Format = BM_FMT_ARGB1555;
         break;
      case GFX_ARGB16_4444:
         Format = BM_FMT_ARGB4444;
         break;
      case GFX_AYCC16_4444:
         Format = BM_FMT_AYCC4444;
         break;
      case GFX_ARGB32:
         Format = BM_FMT_ARGB32;
         break;
      case GFX_AYCC32:
         Format = BM_FMT_AYCC32;
         break;
      default:
         ERRMSG((GXA_ERR_MSG,"GXA ERR: Illegal color\n"));
         Format = BM_FMT_RGB8;
   }

   return (Format);
}

/****************************************************************************
 * GfxTranslateColor
 ***************************************************************************/
u_int32 GfxTranslateColor(PGFX_COLOR pColor, u_int8 Bpp, u_int8 Type)
{
u_int32 Color = 0, TempColor = 0;

   if (Bpp == 8) {
      /* The value passed is an 8 bit color index
       * replicate the index into 32 value
       */
      mRepColor8(pColor->Value.dwVal,Color)
   } else if (Bpp == 16) {
      /* The value passed is a logical color value
       * convert each color component field to a hardware
       * representation and replicate into 32 bit value
       */
      switch (Type) {
         case GFX_ARGB16_0565:
            TempColor = ((pColor->Value.argb.B & 0xF8) << 8) |
                        ((pColor->Value.argb.G & 0xFC) << 3) |
                        ((pColor->Value.argb.R & 0xF8) >> 3);
            break;
         case GFX_AYCC16_2644:
            TempColor = ((pColor->Value.aycc.Cr & 0xF0) << 6) |
                        ((pColor->Value.aycc.Cb & 0xF0) << 2 ) |
                        ((pColor->Value.aycc.Y & 0xFC) >> 2) |
                        ((pColor->Value.argb.A & 0xC0) << 8);
            break;
         case GFX_AYCC16_0655:
            TempColor = ((pColor->Value.aycc.Cr & 0xF8) << 8) |
                        ((pColor->Value.aycc.Cb & 0xF8) << 3) |
                        ((pColor->Value.aycc.Y & 0xFC) >> 2);
            break;
         case GFX_ARGB16_1555:
            TempColor = ((pColor->Value.argb.B & 0xF8) << 7) |
                        ((pColor->Value.argb.G & 0xF8) << 2) |
                        ((pColor->Value.argb.R & 0xF8) >> 3) |
                        ((pColor->Value.argb.A & 0x80) << 8);
            break;
         case GFX_ARGB16_4444:
            TempColor = ((pColor->Value.argb.B & 0xF0) << 4) |
                        ((pColor->Value.argb.G & 0xF0) ) |
                        ((pColor->Value.argb.R & 0xF0) >> 4) |
                        ((pColor->Value.argb.A & 0xF0) << 8);
            break;
         case GFX_AYCC16_4444:
            TempColor = ((pColor->Value.aycc.Cr & 0xF0) << 4) |
                        ((pColor->Value.aycc.Cb & 0xF0) ) |
                        ((pColor->Value.aycc.Y & 0xF0) >> 4) |
                        ((pColor->Value.aycc.A & 0xF0) << 8);
            break;
         default:
            ERRMSG((GXA_ERR_MSG,"GXA ERR: Illegal color\n"));
      }
      mRepColor16(TempColor,Color)

   } else if (Bpp == 32) {
      /* The value passed is a logical color value
       * can just use the full 32 bit value
       */
      Color = pColor->Value.dwVal;
   }

   return (Color);
}
/****************************************************************************
 * GfxConvertPitchToPixels
 ***************************************************************************/
u_int32 GfxConvertPitchToPixels(u_int32 PitchBytes, u_int8 Bpp)
{
u_int32 PixelPitch = 0;

   if (Bpp == 8) {
      PixelPitch = PitchBytes;
   } else if (Bpp == 16) {
      PixelPitch = (PitchBytes+1) >> 1;
   } else if (Bpp == 32) {
      PixelPitch = (PitchBytes+3) >> 2;
   } else if (Bpp == 1) {
      PixelPitch = PitchBytes << 3;
   }

   return (PixelPitch);
}

