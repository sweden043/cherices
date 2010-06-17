/****************************************************************************/
/*                   Conexant Proprietary and Confidential                  */
/*                       Software File/Module Header                        */
/*          Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002          */
/*                               Austin, TX                                 */
/*                            All Rights Eeserved                           */
/****************************************************************************/
/*
 * Filename:       GFXOBJS.C
 *
 *
 * Description:    Graphics Hardware Abstraction Object Routines
 *                 Hardware Graphics Objects Manipulation Code
 *                 Bitmap and Pattern Create and Destroy
 *
 *
 * Author:         Eric Ching
 *
 ****************************************************************************
 $Header: gfxobjs.c, 14, 11/24/03 5:02:45 PM, Xin Golden$
 ****************************************************************************/

/***** Exported Functions **************************************************
GfxCreateBitmap()
GfxDeleteBitmap()
GfxCreatePattern()
GfxDeletePattern()
GfxCreateFontBitmap()
GfxDeleteFontBitmap()
***************************************************************************/

#include "stbcfg.h"
#include "kal.h"
#define INCL_DRM
#include "osdlib.h"
#include "gfxtypes.h"
#include "gfxlib.h"
#include "gxagfx.h"
#include "genmac.h"
#include "gxablt.h"
#include "context.h"
#include "gfx_os.h"
#include "gfxutls.h"

static u_int32 GfxTypeToOSDMode(u_int8 Type);
static u_int32 GfxTypeToOSDOptions(u_int8 Type);

/*************************************************************************
 * GfxCreateBitmap
 *
 *************************************************************************/
PGFX_BITMAP GfxCreateBitmap(u_int8 Type,
                            u_int8 Bpp,
                            u_int16 Width,
                            u_int16 Height,
                            void *BitmapBits)
{
PGFX_BITMAP pNewBitmap = NULL;
OSDHANDLE hOSDRgn;
OSDRECT BMRect;
u_int32 Mode, Options;

   /* Allocate a bitmap instance */
   pNewBitmap = (PGFX_BITMAP)mem_nc_malloc(sizeof(GFX_BITMAP));

   if (pNewBitmap != NULL) {
      if (Type != GFX_MONO) {
         /* Setup the bitmap rectangle size */
         BMRect.left = 0;
         BMRect.top = 0;
         BMRect.right = Width;
         BMRect.bottom = Height;

         /* Set the mode for the region from the type */
         Mode = GfxTypeToOSDMode(Type);

         /* Set the options for the region to be created */
         Options = GfxTypeToOSDOptions(Type);

         /* Create the OSD region representing the bitmap object */
         hOSDRgn = CreateOSDRegion(&BMRect, Mode, Options);

         /* If a valid OSD region has been created setup the bitmap object */
         if (hOSDRgn != 0) {
            pNewBitmap->Version  = GFX_BITMAP_CURRENT_VERSION;
            pNewBitmap->VerSize  = sizeof(GFX_BITMAP);
            pNewBitmap->Type     = Type;
            pNewBitmap->Bpp      = Bpp;
            pNewBitmap->Height   = Height;
            pNewBitmap->Width    = Width;
            pNewBitmap->dwRef    = (u_int32)hOSDRgn;
            pNewBitmap->Stride   =
                  GetOSDRgnOptions(hOSDRgn, OSD_RO_FRAMESTRIDE);
            pNewBitmap->pPalette =
                  (void *)GetOSDRgnOptions(hOSDRgn, OSD_RO_PALETTEADDRESS);
            pNewBitmap->pBits = (void *)( ((POSDLIBREGION)hOSDRgn)->pImage );
         } else {
            /* Free the allocated bitmap instance */
            mem_nc_free((void *)pNewBitmap);
            pNewBitmap = NULL;
         }

      } else {
         u_int32 BytesPerLine;

         /* Setup mono bitmap object */
         /* BytesPerLine is Width/8 rounded up to handle partial bytes. */
         BytesPerLine = (Width + 7) >> 3;

         pNewBitmap->pBits = mem_nc_malloc(BytesPerLine*Height);
         if (pNewBitmap->pBits) {
            pNewBitmap->Version  = GFX_BITMAP_CURRENT_VERSION;
            pNewBitmap->VerSize  = sizeof(GFX_BITMAP);
            pNewBitmap->Type     = Type;
            pNewBitmap->Bpp      = Bpp;
            pNewBitmap->Height   = Height;
            pNewBitmap->Width    = Width;
            pNewBitmap->dwRef    = 0;
            pNewBitmap->Stride   = BytesPerLine;
            pNewBitmap->pPalette = 0;
         } else {
            /* Free the allocated bitmap instance */
            mem_nc_free((void *)pNewBitmap);
            pNewBitmap = NULL;
         }
      }
   } /* pNewBitmap != NULL */

   return (pNewBitmap);
}

/*************************************************************************
 * GfxDeleteBitmap
 *
 *************************************************************************/
u_int32 GfxDeleteBitmap(PGFX_BITMAP pBitmap)
{
u_int32 rc = 0;

   if (pBitmap != 0) {
      if (pBitmap->Type != GFX_MONO) {
         /* Free the associated OSD region for this pattern */
         if ( DestroyOSDRegion((OSDHANDLE)pBitmap->dwRef) ) {
            rc = 0;
         } else {
            rc = 1;
            ERRMSG((GXA_ERR_MSG,"GXA ERR: DestroyOSDRegion Failed!\n"));
         }
      } else {
         /* Free the memory for the mono bitmap */
         mem_nc_free(pBitmap->pBits);
      }
      /* Free the memory for this bitmap instance */
      mem_nc_free(pBitmap);
   }

   return (rc);
}

/*************************************************************************
 * GfxCreatePattern
 *
 *************************************************************************/
PGFX_PAT GfxCreatePattern(u_int8 Type,
                          u_int8 Bpp,
                          u_int8 PatSize,
                          void *PatBits)
{
PGFX_PAT pNewPattern = NULL;
OSDHANDLE hOSDRgn;
OSDRECT BMRect;
u_int32 Mode, Options;
u_int32 MonoPatBytes = 0;

   /* Allocate a pattern instance */
   pNewPattern = (PGFX_PAT)mem_nc_malloc(sizeof(GFX_PAT));

   if (pNewPattern != NULL) {
      /* Setup the pattern size */
      BMRect.left = 0;
      BMRect.top = 0;
      switch (PatSize) {
         case GFX_PAT_8x8:
            BMRect.right = GFX_PAT_SIZE_8x8;
            BMRect.bottom = GFX_PAT_SIZE_8x8;
            MonoPatBytes = GFX_PAT_NUM8x8_BYTES;
            break;

         case GFX_PAT_16x16:
            BMRect.right = GFX_PAT_SIZE_16x16;
            BMRect.bottom = GFX_PAT_SIZE_16x16;
            MonoPatBytes = GFX_PAT_NUM16x16_BYTES;
            break;

         case GFX_PAT_32x32:
            BMRect.right = GFX_PAT_SIZE_32x32;
            BMRect.bottom = GFX_PAT_SIZE_32x32;
            MonoPatBytes = GFX_PAT_NUM32x32_BYTES;
            break;
         default:
            ERRMSG((GXA_ERR_MSG,"GXA ERR: Illegal pattern size!\n"));
      }

      if (Type != GFX_MONO) {

         /* Set the mode for the region from the type and bpp */
         Mode = GfxTypeToOSDMode(Type);

         /* Set the options for the region to be created */
         Options = GfxTypeToOSDOptions(Type);

         /* Create the OSD region representing the pattern object */
         hOSDRgn = CreateOSDRegion(&BMRect, Mode, Options);

         /* If a valid OSD region has been created setup the pattern object */
         if (hOSDRgn != 0) {
            pNewPattern->Version  = GFX_PAT_CURRENT_VERSION;
            pNewPattern->VerSize  = sizeof(GFX_BITMAP);
            pNewPattern->Type     = Type;
            pNewPattern->Bpp      = Bpp;
            pNewPattern->PatSize  = PatSize;
            pNewPattern->dwRef    = (u_int32)hOSDRgn;
            pNewPattern->pPalette =
                  (void *)GetOSDRgnOptions(hOSDRgn, OSD_RO_PALETTEADDRESS);
            pNewPattern->pPatBits = (void *)( ((POSDLIBREGION)hOSDRgn)->pImage );
         } else {
            /* Free the allocated pattern instance */
            mem_nc_free((void *)pNewPattern);
            pNewPattern = NULL;
         }

      } else {
         /* Setup mono pattern object */
         pNewPattern->pPatBits = mem_nc_malloc(MonoPatBytes);
         if (pNewPattern->pPatBits) {
            pNewPattern->Version  = GFX_PAT_CURRENT_VERSION;
            pNewPattern->VerSize  = sizeof(GFX_BITMAP);
            pNewPattern->Type     = Type;
            pNewPattern->Bpp      = Bpp;
            pNewPattern->PatSize  = PatSize;
            pNewPattern->dwRef    = 0;
            pNewPattern->pPalette = 0;
         } else {
            /* Free the allocated bitmap instance */
            mem_nc_free((void *)pNewPattern);
            pNewPattern = NULL;
         }
      }
   } /* pNewPattern != NULL */

   return (pNewPattern);

}

/*************************************************************************
 * GfxDeletePattern
 *
 *************************************************************************/
u_int32 GfxDeletePattern(PGFX_PAT pPattern)
{
u_int32 rc = 0;

   if (pPattern != 0) {
      if (pPattern->Type != GFX_MONO) {
         /* Free the associated OSD region for this pattern */
         if ( DestroyOSDRegion((OSDHANDLE)pPattern->dwRef) ) {
            rc = 0;
         } else {
            rc = 1;
            ERRMSG((GXA_ERR_MSG,"GXA ERR: DestroyOSDRegion Failed!\n"));
         }
      } else {
         /* Free the memory for the mono pattern */
         mem_nc_free(pPattern->pPatBits);
      }
      /* Free the memory for this pattern instance */
      mem_nc_free(pPattern);
   }

   return (rc);
}

/*************************************************************************
 * GfxCreateFontBitmap - create a fixed pitch font bitmap.
 *
 *************************************************************************/
PGFX_FONT GfxCreateFontBitmap(u_int8 WidthType,
                            u_int16 CharWidth,  
                            u_int16 CharHeight,
                            u_int8 FirstChar,
                            u_int8 LastChar,
                            void *pBits)
{
PGFX_FONT pFONT = NULL;
u_int32 BytesPerChar, BytesFontTable;

   /* Allocate a bitmap instance */
   pFONT = (PGFX_FONT)mem_nc_malloc(sizeof(GFX_FONT));

   if (pFONT != NULL) {
      if (WidthType == GFX_FONTWIDTH_FIXED)
      {
         /* BytesPerLine is Width/8 */
         BytesPerChar =  (CharWidth + 7) / 8 * CharHeight;         
         /* Calculate total bytes requied to contain all the font chars */
         BytesFontTable = BytesPerChar * (LastChar - FirstChar + 1);        
         pFONT->pBits = mem_nc_malloc(BytesFontTable);
         if (pFONT->pBits) {
            pFONT->Version    = 1;
            pFONT->VerSize    = sizeof(GFX_FONT);
            pFONT->WidthType  = WidthType;
            pFONT->CharWidth  = CharWidth;
            pFONT->CharHeight = CharHeight;
            pFONT->FirstFont  = FirstChar;
            pFONT->LastFont   = LastChar;
         }
         else
         {
            mem_nc_free((void *)pFONT);
            pFONT = NULL;
         }
      }
      else  /* if NON-fixed width fonts */
      {
         ERRMSG((GXA_ERR_MSG,"GXA ERR: Proportionally spaced fonts can't be created using GfxCreateFontBitmap!\n"));
         mem_nc_free((void *)pFONT);
         pFONT = NULL;
      }
   }
   return (pFONT);
}

/*************************************************************************
 * GfxDeleteFontBitmap
 *
 *************************************************************************/
void GfxDeleteFontBitmap(PGFX_FONT pFont)
{
   if (pFont != 0) {
      mem_nc_free(pFont->pBits);
   }
   /* Free the memory for this bitmap instance */
   mem_nc_free(pFont);
}

/*************************************************************************
 * GfxTypeToOSDMode
 *
 *************************************************************************/

static u_int32 GfxTypeToOSDMode(u_int8 Type)
{
u_int32 Mode = 0;

   switch (Type) {
      case GFX_ARGB4:
         Mode = OSD_MODE_4ARGB;
         break;
      case GFX_AYCC4:
         Mode = OSD_MODE_4AYUV;
         break;
      case GFX_ARGB8:
         Mode = OSD_MODE_8ARGB;
         break;
      case GFX_AYCC8:
         Mode = OSD_MODE_8AYUV;
         break;
      case GFX_ARGB16_0565:
         Mode = OSD_MODE_16RGB;
         break;
      case GFX_ARGB16_4444:
         Mode = OSD_MODE_16ARGB;
         break;
      case GFX_AYCC16_4444:
         Mode = OSD_MODE_16AYUV;
         break;
      case GFX_AYCC16_0655:
         Mode = OSD_MODE_16YUV655;
         break;
#ifndef PCI_GXA
      case GFX_AYCC16_2644:
         Mode = OSD_MODE_16AYUV2644;
         break;
      case GFX_ARGB16_1555:
         Mode = OSD_MODE_16ARGB1555;
         break;
      case GFX_ARGB32:
         Mode = OSD_MODE_32ARGB;
         break;
      case GFX_AYCC32:
         Mode = OSD_MODE_32AYUV;
         break;
#else
      case GFX_AYCC16_2644:
      case GFX_ARGB16_1555:
      case GFX_ARGB32:
      case GFX_AYCC32:
         ERRMSG((GXA_ERR_MSG,"GXA ERR: Unsupported type\n"));
         Mode = OSD_MODE_16RGB;
         break;
#endif
      default:
         ERRMSG((GXA_ERR_MSG,"GXA ERR: Illegal type\n"));
   }
   return (Mode);
}

/*************************************************************************
 * GfxTypeToOSDOptions
 *
 *************************************************************************/

static u_int32 GfxTypeToOSDOptions(u_int8 Type)
{
u_int32 Options = 0;

   switch (Type) {
      case GFX_ARGB4:
      case GFX_ARGB8:
      case GFX_ARGB16_4444:
      case GFX_ARGB16_1555:
      case GFX_ARGB16_0565:
      case GFX_ARGB32:
         Options = OSD_RO_LOADPALETTE | OSD_RO_ALPHAENABLE |
                   OSD_RO_ALPHABOTHVIDEO;
         break;
      case GFX_AYCC4:
      case GFX_AYCC8:
      case GFX_AYCC16_4444:
      case GFX_AYCC16_2644:
      case GFX_AYCC16_0655:
      case GFX_AYCC32:
         Options = OSD_RO_LOADPALETTE | OSD_RO_ALPHAENABLE |
                   OSD_RO_ALPHABOTHVIDEO;
         break;

      default:
         ERRMSG((GXA_ERR_MSG,"GXA ERR: Illegal Type\n"));
   }
   return (Options);
}

/***************************************************************************
                     PVCS Version Control Information
$Log: 
 14   mpeg      1.13        11/24/03 5:02:45 PM    Xin Golden      CR(s): 7988 
       8016 change the font version to 1 when the font is created in 
       GfxCreateFontBitmap
       because some fields in the GFX_FONT structure are not defined in 
       GfxCreateFontBitmap.
 13   mpeg      1.12        6/24/03 2:25:04 PM     Miles Bintz     SCR(s) 6822 
       :
       added initialization value to remove warning in release build
       
 12   mpeg      1.11        2/28/03 2:31:16 PM     Dave Wilson     SCR(s) 5616 
       :
       GfxCreateBitmap, GfxCreatePattern and GfxCreateFontBitmap now all 
       correctly
       set the Version and VerSize fields of their respective structures.
       
 11   mpeg      1.10        2/27/03 1:53:14 PM     Lucy C Allevato SCR(s) 5565 
       :
       bitmap and pattern should be created from non-cached memory instead of 
       cached memory 
       
 10   mpeg      1.9         2/13/03 11:55:52 AM    Matt Korte      SCR(s) 5479 
       :
       Removed old header reference
       
 9    mpeg      1.8         12/19/02 3:25:24 PM    Lucy C Allevato SCR(s) 5196 
       :
       change return code when type is not mono on GfxDeleteBitmap and 
       GfxDeletePattern to match the API.
       
 8    mpeg      1.7         11/21/02 2:42:54 PM    Lucy C Allevato SCR(s) 4973 
       :
       disable Flicker Filtering and Aspect Ration Conversion when create 
       bitmap or pattern
       
 7    mpeg      1.6         10/2/02 12:30:32 PM    Dave Wilson     SCR(s) 4621 
       :
       Removed the #ifdef GFX_HI_LVL_FUNCS since some functions here are used
       in GfxTextBltEx which is not in the old HI_LVL_FUNCS group.
       
 6    mpeg      1.5         9/23/02 5:44:20 PM     Lucy C Allevato SCR(s) 4639 
       :
       Ensure all fields in GFX_FONT are initialized correctly inside 
       GfxCreateFontBitmap 
       
 5    mpeg      1.4         3/21/02 10:15:56 AM    Dave Wilson     SCR(s) 3416 
       :
       Removed a compiler warning when code is compiled to nothing.
       
 4    mpeg      1.3         8/23/00 3:01:04 PM     Lucy C Allevato Removed 
       relative include paths.
       
 3    mpeg      1.2         6/6/00 6:07:12 PM      Lucy C Allevato Fixed 
       comment style to not use C++ line comment.
       
 2    mpeg      1.1         5/24/00 8:37:10 PM     Lucy C Allevato Added setup 
       of Stride and pPalette in BITMAP structure. Added setup of
       pPalette in GFX_PAT structure and fixed allocation size to GFX_PAT from
       GFX_BITMAP. Changed the default region options to make more sense.
       
 1    mpeg      1.0         3/23/00 3:54:02 PM     Lucy C Allevato 
$
 * 
 *    Rev 1.12   24 Jun 2003 13:25:04   bintzmf
 * SCR(s) 6822 :
 * added initialization value to remove warning in release build
 * 
 *    Rev 1.11   28 Feb 2003 14:31:16   dawilson
 * SCR(s) 5616 :
 * GfxCreateBitmap, GfxCreatePattern and GfxCreateFontBitmap now all correctly
 * set the Version and VerSize fields of their respective structures.
 * 
 *    Rev 1.10   27 Feb 2003 13:53:14   goldenx
 * SCR(s) 5565 :
 * bitmap and pattern should be created from non-cached memory instead of cached memory 
 * 
 *    Rev 1.9   13 Feb 2003 11:55:52   kortemw
 * SCR(s) 5479 :
 * Removed old header reference
 * 
 *    Rev 1.8   19 Dec 2002 15:25:24   goldenx
 * SCR(s) 5196 :
 * change return code when type is not mono on GfxDeleteBitmap and GfxDeletePattern to match the API.
 * 
 *    Rev 1.7   21 Nov 2002 14:42:54   goldenx
 * SCR(s) 4973 :
 * disable Flicker Filtering and Aspect Ration Conversion when create bitmap or pattern
 * 
 *    Rev 1.6   02 Oct 2002 11:30:32   dawilson
 * SCR(s) 4621 :
 * Removed the #ifdef GFX_HI_LVL_FUNCS since some functions here are used
 * in GfxTextBltEx which is not in the old HI_LVL_FUNCS group.
 * 
 *    Rev 1.5   23 Sep 2002 16:44:20   goldenx
 * SCR(s) 4639 :
 * Ensure all fields in GFX_FONT are initialized correctly inside 
 * GfxCreateFontBitmap 
 * 
 *    Rev 1.4   21 Mar 2002 10:15:56   dawilson
 * SCR(s) 3416 :
 * Removed a compiler warning when code is compiled to nothing.
 * 
 *    Rev 1.3   23 Aug 2000 14:01:04   eching
 * Removed relative include paths.
 * 
 *    Rev 1.2   06 Jun 2000 17:07:12   eching
 * Fixed comment style to not use C++ line comment.
 * 
 *    Rev 1.1   24 May 2000 19:37:10   eching
 * Added setup of Stride and pPalette in BITMAP structure. Added setup of
 * pPalette in GFX_PAT structure and fixed allocation size to GFX_PAT from
 * GFX_BITMAP. Changed the default region options to make more sense.
 * 
 *    Rev 1.0   23 Mar 2000 15:54:02   eching
 * Initial revision.
****************************************************************************/


