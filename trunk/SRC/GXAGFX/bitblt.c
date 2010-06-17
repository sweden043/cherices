/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*           Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002         */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       bitblt.c
 *
 *
 * Description:    Graphics hardware bitblit library functions
 *
 *
 * Author:         Eric Ching
 *
 ****************************************************************************/
/* $Header: bitblt.c, 27, 3/16/04 10:57:54 AM, Xin Golden$
 ****************************************************************************/

/***** Exported Functions **************************************************
GfxSolidBlt()
GfxPatternBlt()
GfxCopyBlt()
GfxTextBlt()
****************************************************************************/
#include <string.h>
#include "stbcfg.h"
#include "kal.h"
#ifndef PCI_GXA
#define INCL_DRM
#define INCL_GXA
#include "osdlib.h"
#include "osddefs.h"
#endif
#include "gfxtypes.h"
#include "gfxlib.h"
#include "gxagfx.h"
#include "genmac.h"
#include "gxablt.h"
#include "context.h"
#include "gfx_os.h"
#include "gfxutls.h"
#include "retcodes.h"

/* Intermediate grey levels used in greyscale text rendering. Expressed in 256ths */
#define DARK_GREY_TEXT_SCALE 84
#define LIGHT_GREY_TEXT_SCALE 172

#define TEXT_PATTERN_ALIGN_MASK 0x3

extern sem_id_t GXA_Sem_Id;

#ifndef PCI_GXA
/* Private local palette for YCC pixel conversion cases */
static GFX_CVALUE GxaPalette[64];
#endif

/****************************************************/
/* Global variables used in greyscale font blitting */
/****************************************************/
static GFX_BITMAP  sGreyRGBA8Bitmap;
static PGFX_BITMAP pGreyRGBA32Bitmap = NULL;
static u_int8      gpIndices[4];
static bool        bGreyCharColorsSet = FALSE;
static GFX_CVALUE  gpLookupPalette[256];

static u_int32 InternalGreyTextBlt( char * String,
                               PGFX_FONT pFont,
                               PGFX_BITMAP pDstBitmap,
                               PGFX_XY pDstPt, 
                               PGFX_OP pOpcode,
                               u_int8  pTextIndices[]);

static u_int32 InternalCopyBlt(PGFX_BITMAP pSrcBitmap,
                               PGFX_RECT pSrcRect,
                               PGFX_BITMAP pDstBitmap,
                               PGFX_XY pDstPt,
                               PGFX_OP pOpcode);
static u_int32 InternalSolidBlt(PGFX_BITMAP pDstBitmap,
                                PGFX_RECT pDstRect,
                                PGFX_OP pOpcode);

static void InternalBuildTextColourLookup(u_int8 pIndices[], GFX_CVALUE pLookup[]);
static u_int32 InternalColourInterpolate(u_int32 uBack, u_int32 uFront, u_int32 uFactor);

/********************************************************************/
/*  FUNCTION:    GfxGetFontCellSize                                 */
/*                                                                  */
/*  PARAMETERS:  pFont   - Font whose cell size is being queried    */
/*               pWidth  - Storage for returned width               */
/*               pHeight - Storage for returned height              */
/*                                                                  */
/*  DESCRIPTION: Determine the cell size to use for the given font. */
/*               This will depend upon the type and version of the  */
/*               font.                                              */
/*                                                                  */
/*  RETURNS:     TRUE on success, FALSE if passed a bad font or ptr.*/
/*                                                                  */
/*  CONTEXT:     This function may be called in any context         */
/*                                                                  */
/********************************************************************/
bool GfxGetFontCellSize(GFX_FONT *pFont, int *pWidth, int *pHeight)
{
    if(!pFont || !pWidth || !pHeight)
      return(FALSE);
      
    if(pFont->Version >= 0x2)
    {
      if( (pFont->WidthType == GFX_FONTTYPE_AAFIXED) || 
         ((pFont->WidthType == GFX_FONTWIDTH_FIXED) && (pFont->pWidths)))
      {   
        *pWidth = pFont->pWidths[0];
      }  
      else
      {
        *pWidth = pFont->CharWidth;
      }  
        
      *pHeight =  pFont->CharHeight;
    }
    else
    {  
      *pWidth  = pFont->CharWidth;
      *pHeight = pFont->CharHeight;
    }  
    
    if(pFont->Version >= 0x04)
    {
      /* Add the cell padding values */
      *pWidth  += pFont->WidthPad;
      *pHeight += pFont->HeightPad;
    }
    
    return(TRUE);
}           

/********************************************************************/
/*  FUNCTION:    GfxGetFontCharacterSize                            */
/*                                                                  */
/*  PARAMETERS:  pFont   - Font whose cell size is being queried    */
/*               iChar   - ASCII code of character being queried    */
/*               pWidth  - Storage for returned width               */
/*               pHeight - Storage for returned height              */
/*                                                                  */
/*  DESCRIPTION: Determine the width and height of a particular     */
/*               character in the supplied font.                    */
/*                                                                  */
/*  RETURNS:     TRUE on success, FALSE on failure                  */
/*                                                                  */
/*  CONTEXT:     This function may be called in any context         */
/*                                                                  */
/********************************************************************/
bool GfxGetFontCharacterSize(GFX_FONT *pFont, int iChar, int *pWidth, int *pHeight)
{
    int iFontIndex;
    
    if(!pFont || !pWidth || !pHeight)
      return(FALSE);
      
    /* Make sure the character is in the supplied font */
    if((iChar >= (int)pFont->FirstFont) && (iChar <= (int)pFont->LastFont))
    {
      iFontIndex = iChar - pFont->FirstFont;
    }
    else
    {
      /* Character is not in font - set "safe" return values */
      *pWidth = pFont->CharWidth;
      *pHeight = pFont->CharHeight;
      return(FALSE);
    }
    
    switch(pFont->WidthType)
    {
      case GFX_FONTWIDTH_FIXED:
        if((pFont->Version >= 0x02) && pFont->pWidths)
          *pWidth = pFont->pWidths[0];
        else
          *pWidth = pFont->CharWidth;  
        break;
        
      case GFX_FONTWIDTH_VARIABLE:
      case GFX_FONTTYPE_ANTIALIASED:
        *pWidth = pFont->pWidths[iFontIndex];
        break;
        
      case GFX_FONTTYPE_AAFIXED:
        *pWidth = pFont->pWidths[0];
        break;
        
      default:
        /* Bad font! */
        *pWidth = pFont->CharWidth;
        *pHeight = pFont->CharHeight;
        return(FALSE);
    }
    
    *pHeight =  pFont->CharHeight;
    
    if(pFont->Version >= 0x04)
    {
      /* Add the cell padding values */
      *pWidth  += pFont->WidthPad;
      *pHeight += pFont->HeightPad;
    }
    return(TRUE);
}   

/****************************************************************************
 * GfxSolidBlt
 *
 * Parameters: PGFX_BITMAP pDstBitmap - Pointer to structure describing
 *                                      the destination bitmap
 *             PGFX_RECT pDstRect - Pointer to structure describing the
 *                                  rectangular area of memory to fill.
 *             PGFX_OP pOpcode - Pointer to structure describing the
 *                               operation to perform and options.
 * Returns:
 *
 * Description:
 *
 ***************************************************************************/
u_int32 GfxSolidBlt(PGFX_BITMAP pDstBitmap,
                   PGFX_RECT pDstRect,
                   PGFX_OP pOpcode)
{

   u_int32 rc = 0;
   int     iKalRetcode;
   
   /* Get the access semaphore then call our internal version of the function */
   iKalRetcode = sem_get(GXA_Sem_Id, KAL_WAIT_FOREVER);
   if(iKalRetcode == RC_OK)
   {
     rc = InternalSolidBlt(pDstBitmap, pDstRect, pOpcode);
     sem_put(GXA_Sem_Id);
   }
   else
     rc = GFXERR_SYSTEM_ERROR;  

   return (rc);

}

/****************************************************************************
 * InternalSolidBlit
 *
 * Parameters: PGFX_BITMAP pDstBitmap - Pointer to structure describing
 *                                      the destination bitmap
 *             PGFX_RECT pDstRect     - Pointer to structure describing the
 *                                      rectangular area of memory to fill.
 *             PGFX_OP pOpcode        - Pointer to structure describing the
 *                                      operation to perform and options.
 * Returns:
 *
 * Description: Internal version of GfxSolidBlt which does not claim the 
 *              access semaphore. Can be called by other GXAGFX functions
 *              which already hold the semaphore.
 *
 ***************************************************************************/
static u_int32 InternalSolidBlt(PGFX_BITMAP pDstBitmap,
                                PGFX_RECT pDstRect,
                                PGFX_OP pOpcode)
{
u_int32 rc = 0;
u_int32 Format;
u_int32 BltCmdType = SOLID_BLT_COPY;
u_int32 GxaConfig, BltControl;
u_int32 PitchPixels;
#ifndef PCI_GXA
u_int32 GxaConfig2;
#endif
int16 XStart, YStart;
int16 XSize, YSize;

   if ( (pOpcode->BltControl & GFX_OP_ROP) && (pDstBitmap->Type & GFX_TF_NONRGB) ) {
      ERRMSG((GXA_ERR_MSG,"GXA ERR: ROPs to YCC destinations not supported!\n"));
      rc |= GFXERR_NO_ROP_TO_YCC;
   }

   if ( ((pDstBitmap->Type & GFX_NBPP_MASK) != GFX_4BPP) && !rc ) {

      /* This sets up the BLT command and the number of parameters.
       * There is no check for whether the transparency flag is set
       * because transparency is not supported for solid blts.
       */

      // Operation is a copy if neither ROP or ALPHA is set.
      if ( !(pOpcode->BltControl & (GFX_OP_ROP|GFX_OP_ALPHA)) ) {
            BltCmdType = SOLID_BLT_COPY;
            /* Get the current configuration and
             * clear the alpha control bits
             */
            mGetGXAConfig(GxaConfig)
            GxaConfig = (GxaConfig & 0xFE07FFFF);
            mSetGXAConfig(GxaConfig)
      } else {
         if (pOpcode->BltControl & GFX_OP_ALPHA) {
            if (pDstBitmap->Type & GFX_TF_ALPHA) {
#ifndef PCI_GXA
               BltCmdType = SOLID_BLT_ALPHA;

               /* Set the alpha blending options for this operation
                * Get the current configuration
                * and clear the alpha control bits
                */
               mGetGXAConfig(GxaConfig)
               GxaConfig = (GxaConfig & 0xFE07FFFF);

               /* Check which type of alpha blend to use */
               if (pOpcode->AlphaUse & GFX_BLEND_INSIDE_OUT)
                  GxaConfig |= INSIDE_OUT_ALPHA;

               /* Check if we need to preserve the alpha */
               if (pOpcode->BltControl & GFX_OP_PRESERVE_ALPHA) {
                  /* Preserve alpha for this blt */
                  GxaConfig |= PRESERVE_ALPHA;
                  /* Check which alpha should be preserved */
                  switch ( pOpcode->AlphaUse & GFX_BLEND_STORE_MASK ) {
                     case GFX_BLEND_STORE_DST:
                        GxaConfig |= ALPHA_STORE_DST;
                        break;
                     case GFX_BLEND_STORE_SRC:
                        GxaConfig |= ALPHA_STORE_SRC;
                        break;
                     case GFX_BLEND_STORE_FIXED:
                        GxaConfig |= ALPHA_STORE_FIXED;
                        mGetGXAConfig2(GxaConfig2)
                        GxaConfig2 = (GxaConfig2 & 0xFFFFFF00l) | pOpcode->Alpha;
                        mSetGXAConfig2(GxaConfig2)
                        break;
                     default:
                        rc = GFXERR_BAD_ALPHA_SETUP;
                        ERRMSG((GXA_ERR_MSG,"GXA ERR: Illegal alpha store value!\n"));
                  }
               }

               /* Check which alpha to use for the blend */
               switch ( pOpcode->AlphaUse & GFX_BLEND_USE_MASK ) {
                  case GFX_BLEND_USE_DST:
                     GxaConfig |= USE_DST_ALPHA;
                     break;
                  case GFX_BLEND_USE_SRC:
                     GxaConfig |= USE_SRC_ALPHA;
                     break;
                  case GFX_BLEND_USE_FIXED:
                     GxaConfig |= USE_FIXED_ALPHA;
                     mGetGXAConfig2(GxaConfig2)
                     GxaConfig2 = (GxaConfig2 & 0xFFFFFF00l) | pOpcode->Alpha;
                     mSetGXAConfig2(GxaConfig2)
                     break;
                  default:
                     rc = GFXERR_BAD_ALPHA_SETUP;
                     ERRMSG((GXA_ERR_MSG,"GXA ERR: Illegal alpha use value!\n"));
               }

               mSetGXAConfig(GxaConfig)
#else
               /* If PCI_GXA just do a copy instead of alpha */
               BltCmdType = SOLID_BLT_COPY;
#endif /* PCI_GXA */

            } else {
               ERRMSG((GXA_ERR_MSG,"GXA ERR: Cannot blend to destination!\n"));
               rc |= GFXERR_NO_ALPHA_IN_DST;
            }
         } else {
            /* Set the ROP for this operation */
            mGetGXAConfig(GxaConfig)
            /* Clear ROP field and the preserve alpha setup */
            GxaConfig = (GxaConfig & 0xFE07FF00) | (pOpcode->ROP & 0x0F);

#ifndef PCI_GXA
            /* Check if we need to preserve the alpha */
            /* For 16 and 32 bit formats with alpha only */
            if ( (pDstBitmap->Type & GFX_TF_ALPHA) &&
                 (pOpcode->BltControl & GFX_OP_PRESERVE_ALPHA) ) {
               /* Preserve alpha for this blt */
               GxaConfig |= PRESERVE_ALPHA;
               /* Check which alpha should be preserved */
               switch ( pOpcode->AlphaUse & GFX_BLEND_STORE_MASK ) {
                  case GFX_BLEND_STORE_DST:
                     GxaConfig |= ALPHA_STORE_DST;
                     break;
                  case GFX_BLEND_STORE_SRC:
                     GxaConfig |= ALPHA_STORE_SRC;
                     break;
                  case GFX_BLEND_STORE_FIXED:
                     GxaConfig |= ALPHA_STORE_FIXED;
                     mGetGXAConfig2(GxaConfig2)
                     GxaConfig2 = (GxaConfig2 & 0xFFFFFF00l) | pOpcode->Alpha;
                     mSetGXAConfig2(GxaConfig2)
                     break;
                  default:
                     rc = GFXERR_BAD_ALPHA_SETUP;
                     ERRMSG((GXA_ERR_MSG,"GXA ERR: Illegal alpha store value!\n"));
               }
            }
#endif /* PCI_GXA */
            mSetGXAConfig(GxaConfig)

            BltCmdType = SOLID_BLT_ROP;
         }
      }

      XStart = pDstRect->Left;
      /* Setup the blt control register */
      mGetBltControl(BltControl)
      BltControl &= ~BLT_BACKWARDS;
      if ( pOpcode->BltControl & GFX_OP_REVERSE ) {
         BltControl |= BLT_BACKWARDS;
         YStart = pDstRect->Bottom - 1;
      } else {
         YStart = pDstRect->Top;
      }
      mSetBltControl(BltControl)

      XSize = pDstRect->Right - pDstRect->Left;
      YSize = pDstRect->Bottom - pDstRect->Top;

      /* Translate type to hardware format */
      Format = GfxTranslateTypeToFormat(pDstBitmap->Type);

      /* setup context register 7 for solid fill */
      GxaSetupBitmapContext(7,BM_TYPE_SOLID,Format,0,0);

      /* setup context register 0 for destination */
      PitchPixels = GfxConvertPitchToPixels(pDstBitmap->Stride,pDstBitmap->Bpp);
   #ifdef PCI_GXA
      GxaSetupBitmapContext(0,BM_TYPE_COLOR,Format,PitchPixels,(pDstBitmap->PCIOffset>>2));
   #else
      GxaSetupBitmapContext(0,BM_TYPE_COLOR,Format,PitchPixels,(u_int32)pDstBitmap->pBits);
   #endif

      /* send the blt, source context 7, dest context 0, params 2 */
      mCheckFifo(4)
      mCmd1(GXA_BASE |BltCmdType|BMAP7_SRC|BMAP0_DST,
            mMakeXY(XStart,YStart),
            mMakeXY(XSize,YSize))

   } else if (!rc) {
      if ( pOpcode->BltControl & (GFX_OP_ALPHA|GFX_OP_TRANS|GFX_OP_ROP) ) {
         ERRMSG((GXA_ERR_MSG,"GXA ERR: Illegal 4 bpp fill blt!\n"));
         rc |= GFXERR_ILLEGAL_4BPP_OP;         
      } else {
         /* Software 4 bpp fill is handled here by calling bitblt.s */
      }
   } /* ENDIF NOT 4BPP */

   return (rc);
}

/****************************************************************************
 * GfxPatternBlt
 *
 * Parameters: PGFX_PAT pPattern - Pointer to structure describing the
 *                                 pattern to use.
 *             PGFX_XY pPatXY - XY Offset used to change the alignment of
 *                              the pattern relative to the destination
 *                              bitmap origin.
 *             PGFX_BITMAP pDstBitmap - Pointer to structure describing
 *                                      the destination bitmap.
 *             PGFX_RECT pDstRect - Pointer to structure describing the
 *                                  rectangular area of memory to fill
 *                                  with the pattern.
 *             PGFX_OP pOpcode - Pointer to structure describing the
 *                               operation to perform and options.
 * Returns:
 *
 * Description:
 *
 ***************************************************************************/
u_int32 GfxPatternBlt(PGFX_PAT pPattern,
                      PGFX_XY pPatXY,
                      PGFX_BITMAP pDstBitmap,
                      PGFX_RECT pDstRect,
                      PGFX_OP pOpcode)
{
u_int32 rc = 0;
#ifndef PCI_GXA
u_int8 bLoadPalette = 0;
#endif
u_int32 Format;
u_int32 BltCmdType = BLT_COPY; /* Init to copy by default */
u_int32 GxaConfig, BltControl;
u_int32 PitchPixels;
#ifndef PCI_GXA
u_int32 GxaConfig2;
#endif
int16 XStart, YStart;
int16 XSize, YSize;
int16 PatX, PatY;
int16 PatWidth;
u_int32 PatType;

   if ( (pOpcode->BltControl & GFX_OP_ROP) && (pDstBitmap->Type & GFX_TF_NONRGB) ) {
      ERRMSG((GXA_ERR_MSG,"GXA ERR: ROPs to YCC destinations not supported!\n"));
      rc |= GFXERR_NO_ROP_TO_YCC;
   }

   if ( ((pDstBitmap->Type & GFX_NBPP_MASK) != GFX_4BPP) && !rc ) {

      sem_get(GXA_Sem_Id, KAL_WAIT_FOREVER);

#ifndef PCI_GXA
      /* Check Bpp to determine if format conversion will occur */
      if ( (pPattern->Type != pDstBitmap->Type) &&
         (pPattern->Type != GFX_MONO) )
      {
         if ( !(pDstBitmap->Type & GFX_TF_NONRGB) && 
              (pPattern->Type & GFX_TF_NONRGB) ) {
            ERRMSG((GXA_ERR_MSG,"GXA ERR: Conversion from YCC src to RGB dst not supported!\n"));
            rc |= GFXERR_CANNOT_CONVERT;
         } else {
            /* Check Bpp to determine if operation needs to load a palette */
            /* to perform color space conversion */
            if ( (pPattern->Bpp < pDstBitmap->Bpp) ||
                  ((pPattern->Bpp == 32) && (pDstBitmap->Bpp == 32)) ) {
               bLoadPalette = 1;
            } else if ( (pPattern->Bpp == 16) && (pDstBitmap->Bpp == 16) ) {
               bLoadPalette = 1;
            } else {
               /* Conversion blt to smaller Bpp format not supported */
               ERRMSG((GXA_ERR_MSG,"GXA ERR: Conversion to smaller pixel format not supported!\n"));
               rc |= GFXERR_CANNOT_CONVERT;
            }
         }
      }
#endif

      /* This sets up the BLT command and the number of parameters. */

      /* Operation is a copy if neither ROP or ALPHA is set. */
      if ( !(pOpcode->BltControl & (GFX_OP_ROP|GFX_OP_ALPHA)) ) {
         BltCmdType = (pOpcode->BltControl & GFX_OP_TRANS) ? BLT_COPY_T : BLT_COPY;
         /* Get the current configuration and clear the alpha control bits */
         mGetGXAConfig(GxaConfig)
         GxaConfig = (GxaConfig & 0xFE07FFFF);
         mSetGXAConfig(GxaConfig)
      } else {
         if (pOpcode->BltControl & GFX_OP_ALPHA) {
            if (pDstBitmap->Type & GFX_TF_ALPHA) {
#ifndef PCI_GXA
               BltCmdType = (pOpcode->BltControl & GFX_OP_TRANS) ? BLT_ALPHA_T : BLT_ALPHA;

               /* Set the alpha blending options for this operation */
               /* Get the current configuration
                * and clear the alpha control bits
                */
               mGetGXAConfig(GxaConfig)
               GxaConfig = (GxaConfig & 0xFE07FFFF);

               /* Check which type of alpha blend to use */
               if (pOpcode->AlphaUse & GFX_BLEND_INSIDE_OUT)
                  GxaConfig |= INSIDE_OUT_ALPHA;

               /* Check if we need to preserve the alpha */
               if (pOpcode->BltControl & GFX_OP_PRESERVE_ALPHA) {
                  GxaConfig |= PRESERVE_ALPHA; // Preserve alpha for this blt
                  /* Check which alpha should be preserved */
                  switch ( pOpcode->AlphaUse & GFX_BLEND_STORE_MASK ) {
                     case GFX_BLEND_STORE_DST:
                        GxaConfig |= ALPHA_STORE_DST;
                        break;
                     case GFX_BLEND_STORE_SRC:
                        GxaConfig |= ALPHA_STORE_SRC;
                        break;
                     case GFX_BLEND_STORE_FIXED:
                        GxaConfig |= ALPHA_STORE_FIXED;
                        mGetGXAConfig2(GxaConfig2)
                        GxaConfig2 = (GxaConfig2 & 0xFFFFFF00l) | pOpcode->Alpha;
                        mSetGXAConfig2(GxaConfig2)
                        break;
                     default:
                        rc = GFXERR_BAD_ALPHA_SETUP;
                        ERRMSG((GXA_ERR_MSG,"GXA ERR: Illegal alpha store value!\n"));
                  }
               }

               /* Check which alpha to use for the blend */
               switch ( pOpcode->AlphaUse & GFX_BLEND_USE_MASK ) {
                  case GFX_BLEND_USE_DST:
                     GxaConfig |= USE_DST_ALPHA;
                     break;
                  case GFX_BLEND_USE_SRC:
                     GxaConfig |= USE_SRC_ALPHA;
                     break;
                  case GFX_BLEND_USE_FIXED:
                     GxaConfig |= USE_FIXED_ALPHA;
                     mGetGXAConfig2(GxaConfig2)
                     GxaConfig2 = (GxaConfig2 & 0xFFFFFF00l) | pOpcode->Alpha;
                     mSetGXAConfig2(GxaConfig2)
                     break;
                  default:
                     rc = GFXERR_BAD_ALPHA_SETUP;
                     ERRMSG((GXA_ERR_MSG,"GXA ERR: Illegal alpha use value!\n"));
               }

               mSetGXAConfig(GxaConfig)
#else
         /* If PCI_GXA just do a copy instead of alpha */
         BltCmdType = (pOpcode->BltControl & GFX_OP_TRANS) ? BLT_COPY_T : BLT_COPY;
#endif /* PCI_GXA */

            } else {
               ERRMSG((GXA_ERR_MSG,"GXA ERR: Cannot blend to destination!\n"));
               rc |= GFXERR_NO_ALPHA_IN_DST;
            }
         } else {
            /* Set the ROP for this operation */
            mGetGXAConfig(GxaConfig)
            /* Clear ROP field and the preserve alpha setup */
            GxaConfig = (GxaConfig & 0xFE07FF00) | (pOpcode->ROP & 0x0F);

#ifndef PCI_GXA
            /* Check if we need to preserve the alpha */
            if (pOpcode->BltControl & GFX_OP_PRESERVE_ALPHA) {
                /* Preserve alpha for this blt */
               GxaConfig |= PRESERVE_ALPHA;
               /* Check which alpha should be preserved */
               switch ( pOpcode->AlphaUse & GFX_BLEND_STORE_MASK ) {
                  case GFX_BLEND_STORE_DST:
                     GxaConfig |= ALPHA_STORE_DST;
                     break;
                  case GFX_BLEND_STORE_SRC:
                     GxaConfig |= ALPHA_STORE_SRC;
                     break;
                  case GFX_BLEND_STORE_FIXED:
                     GxaConfig |= ALPHA_STORE_FIXED;
                     mGetGXAConfig2(GxaConfig2)
                     GxaConfig2 = (GxaConfig2 & 0xFFFFFF00l) | pOpcode->Alpha;
                     mSetGXAConfig2(GxaConfig2)
                     break;
                  default:
                     rc = GFXERR_BAD_ALPHA_SETUP;
                     ERRMSG((GXA_ERR_MSG,"GXA ERR: Illegal alpha store value!\n"));
               }
            }
#endif /* PCI_GXA */
            mSetGXAConfig(GxaConfig)

            BltCmdType = (pOpcode->BltControl & GFX_OP_TRANS) ? BLT_ROP_T : BLT_ROP;
         }
      }

      /* Set the blt direction to forwards */
      mGetBltControl(BltControl)
      BltControl &= ~BLT_BACKWARDS;
      mSetBltControl(BltControl)

      XStart = pDstRect->Left;
      YStart = pDstRect->Top;

      XSize = pDstRect->Right - pDstRect->Left;
      YSize = pDstRect->Bottom - pDstRect->Top;

      switch (pPattern->PatSize) {
         case GFX_PAT_8x8:
            /* Limit the pattern xy offset to valid value */
            PatX = pPatXY->X & 0x07;
            PatY = pPatXY->Y & 0x07;
            PatWidth = GFX_PAT_SIZE_8x8;
            PatType = BM_TYPE_PATT | BM_TYPE_PATSZ8;
            break;
         case GFX_PAT_16x16:
            /* Limit the pattern xy offset to valid value */
            PatX = pPatXY->X & 0x0F;
            PatY = pPatXY->Y & 0x0F;
            PatWidth = GFX_PAT_SIZE_16x16;
            PatType = BM_TYPE_PATT | BM_TYPE_PATSZ16;
            break;
         case GFX_PAT_32x32:
            /* Limit the pattern xy offset to valid value */
            PatX = pPatXY->X & 0x1F;
            PatY = pPatXY->Y & 0x1F;
            PatWidth = GFX_PAT_SIZE_32x32;
            PatType = BM_TYPE_PATT | BM_TYPE_PATSZ32;
            break;
         case GFX_PAT_2x2:
         case GFX_PAT_4x4:
         default:
            PatX = pPatXY->X;
            PatY = pPatXY->Y;
            PatWidth = 0;
            PatType = BM_TYPE_PATT;
            ERRMSG((GXA_ERR_MSG,"GXA ERR: Unsupported pattern width!\n"));
            rc = GFXERR_PAT_WIDTH;
            break;
      }

      PatType |= (pPattern->Type == GFX_MONO) ? BM_TYPE_MONO : BM_TYPE_COLOR;

      if (PatType & BM_TYPE_MONO) {
         /* If mono source translate the destination type to get format */
         Format = GfxTranslateTypeToFormat(pDstBitmap->Type);
      } else {
         /* Translate the source type to a format for the bitmap context */
         Format = GfxTranslateTypeToFormat(pPattern->Type);
      }

      /* setup context register 3 for pattern  */
#ifdef PCI_GXA
      GxaSetupBitmapContext(3,PatType,Format,PatWidth,(pPattern->PCIOffset>>2));
#else
      GxaSetupBitmapContext(3,PatType,Format,PatWidth,(u_int32)pPattern->pPatBits);
#endif

#ifndef PCI_GXA
      if (bLoadPalette) {
         POSDLIBREGION pOsdRgn;
         u_int32 PaletteAddr = (u_int32)GxaPalette, dwVal, dwCount;

         TRACE((GXA_TRACE_MSG,"GXA: Loading palette for this operation.\n"));
         if (pPattern->pPalette != 0) {
            PaletteAddr = (u_int32)pPattern->pPalette;
         } else if (pPattern->dwRef != 0) {
            pOsdRgn = (POSDLIBREGION)(pPattern->dwRef);
            if (pOsdRgn->dwNumPalEntries != 0) {
               PaletteAddr = (u_int32)pOsdRgn->pPalette;
            } else {
               /* This should be one of the 16 bpp YCC formats */
               /* Create a linear palette for the GXA to use */
               switch (pPattern->Type) {
                  case GFX_AYCC16_4444:
                     for (dwCount=0; dwCount<16; dwCount++)
                     {
                        dwVal = (dwCount << 4);
                        dwVal = BOUND(dwVal,240,16);
                        GxaPalette[dwCount].aycc.Cb =
                        GxaPalette[dwCount].aycc.Cr = dwVal;
                        dwVal = BOUND(dwVal,235,16);
                        GxaPalette[dwCount].aycc.Y = dwVal;
                        GxaPalette[dwCount].aycc.A = 0xFF;
                     }
                     break;
                  case GFX_AYCC16_2644:
                     for (dwCount=0; dwCount<16; dwCount++)
                     {
                        dwVal = (dwCount << 4);
                        dwVal = BOUND(dwVal,240,16);
                        GxaPalette[dwCount].aycc.Cb =
                        GxaPalette[dwCount].aycc.Cr = dwVal;
                     }
                     for (dwCount=0; dwCount<64; dwCount++)
                     {
                        dwVal = (dwCount << 2);
                        dwVal = BOUND(dwVal,235,16);
                        GxaPalette[dwCount].aycc.Y = dwVal;
                        GxaPalette[dwCount].aycc.A = 0xFF;
                     }
                     break;
                  case GFX_AYCC16_0655:
                     for (dwCount=0; dwCount<32; dwCount++)
                     {
                        dwVal = (dwCount << 3);
                        dwVal = BOUND(dwVal,240,16);
                        GxaPalette[dwCount].aycc.Cb =
                        GxaPalette[dwCount].aycc.Cr = dwVal;
                     }
                     for (dwCount=0; dwCount<64; dwCount++)
                     {
                        dwVal = (dwCount << 2);
                        dwVal = BOUND(dwVal,235,16);
                        GxaPalette[dwCount].aycc.Y = dwVal;
                        GxaPalette[dwCount].aycc.A = 0xFF;
                     }
                     break;
               }
               PaletteAddr = (u_int32)GxaPalette;
            }
         } else {
            ERRMSG((GXA_ERR_MSG,"GXA ERR: No palette available for conversion!\n"));
         }
         mLoadPalette(BMAP3_SRC,PaletteAddr);
      }
#endif

      /* Translate the destination type to a format for the bitmap context */
      Format = GfxTranslateTypeToFormat(pDstBitmap->Type);

      /* setup context register 1 for destination */
      PitchPixels = GfxConvertPitchToPixels(pDstBitmap->Stride,pDstBitmap->Bpp);
#ifdef PCI_GXA
      GxaSetupBitmapContext(1,BM_TYPE_COLOR,Format,
                        PitchPixels,(pDstBitmap->PCIOffset>>2));
#else
      GxaSetupBitmapContext(1,BM_TYPE_COLOR,Format,
                        PitchPixels,(u_int32)pDstBitmap->pBits);
#endif

      /* send the blt, source context 1, dest context 0, params 3 */
      mCheckFifo(4)
      mCmd2(GXA_BASE|BltCmdType|BMAP3_SRC|BMAP1_DST,
            mMakeXY(XStart,YStart),
            mMakeXY(XSize,YSize),
            mMakeXY(PatX,PatY))

      sem_put(GXA_Sem_Id);

   } else if (!rc) {
      ERRMSG((GXA_ERR_MSG,"GXA ERR: Illegal 4 bpp pattern blt!\n"));
      rc |= GFXERR_ILLEGAL_4BPP_OP;         
   } /* ENDIF NOT 4BPP */

   return (rc);
}

/****************************************************************************
 * GfxCopyBlt
 *
 * Parameters: PGFX_BITMAP pSrcBitmap - Pointer to structure describing
 *                                      the source bitmap.
 *             PGFX_RECT pSrcRect - Pointer to structure describing the
 *                                  rectangular extents.
 *             PGFX_BITMAP pDstBitmap - Pointer to structure describing
 *                                      the destination bitmap.
 *             PGFX_XY pDstPt - Pointer to structure describing the
 *                              starting XY point in the destination.
 *             PGFX_OP pOpcode - Pointer to structure describing the
 *                               operation to perform and options.
 * Returns:
 *
 * Description:
 *
 ***************************************************************************/
u_int32 GfxCopyBlt(PGFX_BITMAP pSrcBitmap,
                   PGFX_RECT pSrcRect,
                   PGFX_BITMAP pDstBitmap,
                   PGFX_XY pDstPt,
                   PGFX_OP pOpcode)
{
   u_int32 rc = 0;
   int     iKalRetcode;
   
   /* Get the access semaphore then call our internal version of the function */
   iKalRetcode = sem_get(GXA_Sem_Id, KAL_WAIT_FOREVER);
   if(iKalRetcode == RC_OK)
   {
     rc = InternalCopyBlt(pSrcBitmap, pSrcRect, pDstBitmap, pDstPt, pOpcode);
     sem_put(GXA_Sem_Id);
   }
   else
     rc = GFXERR_SYSTEM_ERROR;  

   return (rc);
}

/****************************************************************************
 * InternalCopyBlt
 *
 * Parameters: PGFX_BITMAP pSrcBitmap - Pointer to structure describing
 *                                      the source bitmap.
 *             PGFX_RECT pSrcRect - Pointer to structure describing the
 *                                  rectangular extents.
 *             PGFX_BITMAP pDstBitmap - Pointer to structure describing
 *                                      the destination bitmap.
 *             PGFX_XY pDstPt - Pointer to structure describing the
 *                              starting XY point in the destination.
 *             PGFX_OP pOpcode - Pointer to structure describing the
 *                               operation to perform and options.
 * Returns:
 *
 * Description: Internal version of GfxCopyBlt which does not claim the 
 *              access semaphore. Can be called by other GXAGFX functions
 *              which already hold the semaphore.
 *
 ***************************************************************************/
static u_int32 InternalCopyBlt(PGFX_BITMAP pSrcBitmap,
                               PGFX_RECT pSrcRect,
                               PGFX_BITMAP pDstBitmap,
                               PGFX_XY pDstPt,
                               PGFX_OP pOpcode)
{
u_int32 rc = 0;
#ifndef PCI_GXA
u_int8 bLoadPalette = 0;
#endif
u_int32 Format;
u_int32 GxaConfig;
#ifndef PCI_GXA
u_int32 GxaConfig2;
#endif
u_int32 BltCmdType = BLT_COPY, BltControl;
u_int32 PitchPixels;
int16 SrcX, SrcY;
int16 DstX, DstY;
int16 XSize, YSize;
u_int32 BMType;

   if ( (pOpcode->BltControl & GFX_OP_ROP) && (pDstBitmap->Type & GFX_TF_NONRGB) ) {
      ERRMSG((GXA_ERR_MSG,"GXA ERR: ROPs to YCC destinations not supported!\n"));
      rc |= GFXERR_NO_ROP_TO_YCC;
   }

   if ( ((pDstBitmap->Type & GFX_NBPP_MASK) != GFX_4BPP) && !rc ) {

#ifndef PCI_GXA
      // Check Bpp to determine if format conversion will occur
      if ( (pSrcBitmap->Type != pDstBitmap->Type) &&
         (pSrcBitmap->Type != GFX_MONO) )
      {
         if ( !(pDstBitmap->Type & GFX_TF_NONRGB) && 
              (pSrcBitmap->Type & GFX_TF_NONRGB) ) {
            ERRMSG((GXA_ERR_MSG,"GXA ERR: Conversion from YCC src to RGB dst not supported!\n"));
            rc |= GFXERR_CANNOT_CONVERT;
         } else {
            /* Check Bpp to determine if operation needs to load a palette
             * to perform color space conversion
             */
            if ( (pSrcBitmap->Bpp < pDstBitmap->Bpp) ||
                 ((pSrcBitmap->Bpp == 32) && (pDstBitmap->Bpp == 32)) ) {
               bLoadPalette = 1;
            } else if ( (pSrcBitmap->Bpp == 16) && (pDstBitmap->Bpp == 16) ) {
               bLoadPalette = 1;
            } else {
               /* Conversion blt to smaller Bpp format not supported */
               ERRMSG((GXA_ERR_MSG,"GXA ERR: Conversion to smaller pixel format not supported!\n"));
               rc |= GFXERR_CANNOT_CONVERT;
            }
         }
      }
#endif

      /* This sets up the BLT command and the number of parameters. */

      /* Operation is a copy if neither ROP or ALPHA is set. */
      if ( !(pOpcode->BltControl & (GFX_OP_ROP|GFX_OP_ALPHA)) ) {
         BltCmdType = (pOpcode->BltControl & GFX_OP_TRANS) ? BLT_COPY_T : BLT_COPY;
         /* Get the current configuration and clear the alpha control bits */
         mGetGXAConfig(GxaConfig)
         GxaConfig = (GxaConfig & 0xFE07FFFF);
         mSetGXAConfig(GxaConfig)
      } else {
         if (pOpcode->BltControl & GFX_OP_ALPHA) {
            if (pDstBitmap->Type & GFX_TF_ALPHA) {
#ifndef PCI_GXA
               BltCmdType = (pOpcode->BltControl & GFX_OP_TRANS) ? BLT_ALPHA_T : BLT_ALPHA;

               /* Set the alpha blending options for this operation */
               /* Get the current configuration
                * and clear the alpha control bits
                */
               mGetGXAConfig(GxaConfig)
               GxaConfig = (GxaConfig & 0xFE07FFFF);

               /* Check which type of alpha blend to use */
               if (pOpcode->AlphaUse & GFX_BLEND_INSIDE_OUT)
                  GxaConfig |= INSIDE_OUT_ALPHA;

               /* Check if we need to preserve the alpha */
               if (pOpcode->BltControl & GFX_OP_PRESERVE_ALPHA) {
                  /* Preserve alpha for this blt */
                  GxaConfig |= PRESERVE_ALPHA;
                  /* Check which alpha should be preserved */
                  switch ( pOpcode->AlphaUse & GFX_BLEND_STORE_MASK ) {
                     case GFX_BLEND_STORE_DST:
                        GxaConfig |= ALPHA_STORE_DST;
                        break;
                     case GFX_BLEND_STORE_SRC:
                        GxaConfig |= ALPHA_STORE_SRC;
                        break;
                     case GFX_BLEND_STORE_FIXED:
                        GxaConfig |= ALPHA_STORE_FIXED;
                        mGetGXAConfig2(GxaConfig2)
                        GxaConfig2 = (GxaConfig2 & 0xFFFFFF00l) | pOpcode->Alpha;
                        mSetGXAConfig2(GxaConfig2)
                        break;
                     default:
                        rc = GFXERR_BAD_ALPHA_SETUP;
                        ERRMSG((GXA_ERR_MSG,"GXA ERR: Illegal alpha store value!\n"));
                  }
               }

               /* Check which alpha to use for the blend */
               switch ( pOpcode->AlphaUse & GFX_BLEND_USE_MASK ) {
                  case GFX_BLEND_USE_DST:
                     GxaConfig |= USE_DST_ALPHA;
                     break;
                  case GFX_BLEND_USE_SRC:
                     GxaConfig |= USE_SRC_ALPHA;
                     break;
                  case GFX_BLEND_USE_FIXED:
                     GxaConfig |= USE_FIXED_ALPHA;
                     mGetGXAConfig2(GxaConfig2)
                     GxaConfig2 = (GxaConfig2 & 0xFFFFFF00l) | pOpcode->Alpha;
                     mSetGXAConfig2(GxaConfig2)
                     break;
                  default:
                     rc = GFXERR_BAD_ALPHA_SETUP;
                     ERRMSG((GXA_ERR_MSG,"GXA ERR: Illegal alpha use value!\n"));
               }

               mSetGXAConfig(GxaConfig)
#else
         /* If PCI_GXA just do a copy instead of alpha */
         BltCmdType = (pOpcode->BltControl & GFX_OP_TRANS) ? BLT_COPY_T : BLT_COPY;
#endif /* PCI_GXA */
            } else {
               ERRMSG((GXA_ERR_MSG,"GXA ERR: Cannot blend to destination!\n"));
               rc |= GFXERR_NO_ALPHA_IN_DST;
            }
         } else {
            /* Set the ROP for this operation */
            mGetGXAConfig(GxaConfig)
            /* Clear ROP field and the preserve alpha setup */
            GxaConfig = (GxaConfig & 0xFE07FF00) | (pOpcode->ROP & 0x0F);
#ifndef PCI_GXA
            /* Check if we need to preserve the alpha */
            if (pOpcode->BltControl & GFX_OP_PRESERVE_ALPHA) {
               /* Preserve alpha for this blt */
               GxaConfig |= PRESERVE_ALPHA;
               /* Check which alpha should be preserved */
               switch ( pOpcode->AlphaUse & GFX_BLEND_STORE_MASK ) {
                  case GFX_BLEND_STORE_DST:
                     GxaConfig |= ALPHA_STORE_DST;
                     break;
                  case GFX_BLEND_STORE_SRC:
                     GxaConfig |= ALPHA_STORE_SRC;
                     break;
                  case GFX_BLEND_STORE_FIXED:
                     GxaConfig |= ALPHA_STORE_FIXED;
                     mGetGXAConfig2(GxaConfig2)
                     GxaConfig2 = (GxaConfig2 & 0xFFFFFF00l) | pOpcode->Alpha;
                     mSetGXAConfig2(GxaConfig2)
                     break;
                  default:
                     rc = GFXERR_BAD_ALPHA_SETUP;
                     ERRMSG((GXA_ERR_MSG,"GXA ERR: Illegal alpha store value!\n"));
               }
            }
#endif /* PCI_GXA */
            mSetGXAConfig(GxaConfig)

            BltCmdType = (pOpcode->BltControl & GFX_OP_TRANS) ? BLT_ROP_T : BLT_ROP;
         }
      }

      XSize = pSrcRect->Right - pSrcRect->Left;
      YSize = pSrcRect->Bottom - pSrcRect->Top;

      SrcX = pSrcRect->Left;
      DstX = pDstPt->X;
      /* Setup the blt control register */
      mGetBltControl(BltControl)
      BltControl &= ~BLT_BACKWARDS;
      if ( pOpcode->BltControl & GFX_OP_REVERSE ) {
         BltControl |= BLT_BACKWARDS;
         SrcY = pSrcRect->Bottom - 1;
         DstY = pDstPt->Y + YSize - 1;
      } else {
         SrcY = pSrcRect->Top;
         DstY = pDstPt->Y;
      }
      mSetBltControl(BltControl)

      BMType = (pSrcBitmap->Type == GFX_MONO) ? BM_TYPE_MONO : BM_TYPE_COLOR;

      if (BMType != BM_TYPE_MONO) {
         /* Translate the source type to a format for the bitmap context */
         Format = GfxTranslateTypeToFormat(pSrcBitmap->Type);
      } else {
         /* If mono source translate the destination type to get format */
         Format = GfxTranslateTypeToFormat(pDstBitmap->Type);
      }

      /* setup context register 2 for source  */
      PitchPixels = GfxConvertPitchToPixels(pSrcBitmap->Stride,pSrcBitmap->Bpp);
#ifdef PCI_GXA
      GxaSetupBitmapContext(2,BMType,Format,
                        PitchPixels,(pSrcBitmap->PCIOffset>>2));
#else
      GxaSetupBitmapContext(2,BMType,Format,
                        PitchPixels,(u_int32)pSrcBitmap->pBits);
#endif

#ifndef PCI_GXA
      if (bLoadPalette) {
         POSDLIBREGION pOsdRgn;
         u_int32 PaletteAddr = (u_int32)GxaPalette, dwCount, dwVal;

         TRACE((GXA_TRACE_MSG,"GXA: Loading palette for this operation.\n"));
         if (pSrcBitmap->pPalette != 0) {
            PaletteAddr = (u_int32)pSrcBitmap->pPalette;
         } else if (pSrcBitmap->dwRef != 0) {
            pOsdRgn = (POSDLIBREGION)(pSrcBitmap->dwRef);
            if (pOsdRgn->dwNumPalEntries != 0) {
               PaletteAddr = (u_int32)pOsdRgn->pPalette;
            } else {
               /* This should be one of the 16 bpp YCC formats
                * Create a linear palette for the GXA to use
                */
               switch (pSrcBitmap->Type) {
                  case GFX_AYCC16_4444:
                     for (dwCount=0; dwCount<16; dwCount++)
                     {
                        dwVal = (dwCount << 4);

                        dwVal = BOUND(dwVal,240,16);
                        GxaPalette[dwCount].aycc.Cb =
                        GxaPalette[dwCount].aycc.Cr = dwVal;
                        dwVal = BOUND(dwVal,235,16);
                        GxaPalette[dwCount].aycc.Y = dwVal;
                        GxaPalette[dwCount].aycc.A = 0xFF;
                     }
                     break;
                  case GFX_AYCC16_2644:
                     for (dwCount=0; dwCount<16; dwCount++)
                     {
                        dwVal = (dwCount << 4);
                        dwVal = BOUND(dwVal,240,16);
                        GxaPalette[dwCount].aycc.Cb =
                        GxaPalette[dwCount].aycc.Cr = dwVal;
                     }
                     for (dwCount=0; dwCount<64; dwCount++)
                     {
                        dwVal = (dwCount << 2);
                        dwVal = BOUND(dwVal,235,16);
                        GxaPalette[dwCount].aycc.Y = dwVal;
                        GxaPalette[dwCount].aycc.A = 0xFF;
                     }
                     break;
                  case GFX_AYCC16_0655:
                     for (dwCount=0; dwCount<32; dwCount++)
                     {
                        dwVal = (dwCount << 3);
                        dwVal = BOUND(dwVal,240,16);
                        GxaPalette[dwCount].aycc.Cb =
                        GxaPalette[dwCount].aycc.Cr = dwVal;
                     }
                     for (dwCount=0; dwCount<64; dwCount++)
                     {
                        dwVal = (dwCount << 2);
                        dwVal = BOUND(dwVal,235,16);
                        GxaPalette[dwCount].aycc.Y = dwVal;
                        GxaPalette[dwCount].aycc.A = 0xFF;
                     }
                     break;
               }
               PaletteAddr = (u_int32)GxaPalette;
            }
         } else {
            ERRMSG((GXA_ERR_MSG,"GXA ERR: No palette available for conversion!\n"));
         }
         mLoadPalette(BMAP2_SRC,PaletteAddr);
      }
#endif

      /* Translate the destination type to a format for the bitmap context */
      Format = GfxTranslateTypeToFormat(pDstBitmap->Type);

      /* setup context register 1 for destination */
      PitchPixels = GfxConvertPitchToPixels(pDstBitmap->Stride,pDstBitmap->Bpp);
#ifdef PCI_GXA
      GxaSetupBitmapContext(1,BM_TYPE_COLOR, Format,
                        PitchPixels,(pDstBitmap->PCIOffset>>2));
#else
      GxaSetupBitmapContext(1,BM_TYPE_COLOR, Format,
                        PitchPixels,(u_int32)pDstBitmap->pBits);
#endif

      /* send the blt, source context 1, dest context 0, params 3 */
      mCheckFifo(4)
      mCmd2(GXA_BASE|BltCmdType|BMAP2_SRC|BMAP1_DST,
            mMakeXY(DstX,DstY),
            mMakeXY(XSize,YSize),
            mMakeXY(SrcX,SrcY))

   } else if (!rc) {
      if ( pOpcode->BltControl & (GFX_OP_ALPHA|GFX_OP_TRANS|GFX_OP_ROP) ) {
         ERRMSG((GXA_ERR_MSG,"GXA ERR: Illegal 4 bpp copy blt!\n"));
         rc |= GFXERR_ILLEGAL_4BPP_OP;         
      } else {
         /* Software 4 bpp copy is handled here by calling bitblt.s */
      }
   } /* ENDIF NOT 4BPP */

   return (rc);
}

/******************************************************************************
 * GfxTextBlt
 *
 * Parameters: char * String - Pointer to the string to render
 *             PGFX_FONT pFont - Pointer to the structure describing the
 *                               font to use.
 *             PGFX_BITMAP pDstBitmap - Pointer to structure describing
 *                                      the destination bitmap
 *             PGFX_XY pDstPt - Pointer to structure describing the
 *                              starting XY point in the destination.
 *             PGFX_OP pOpcode - Pointer to structure describing the
 *                               operation to perform and options.
 * Returns:
 *
 * Description:
 *
 *****************************************************************************/
u_int32 GfxTextBlt( char * String,
                    PGFX_FONT pFont,
                    PGFX_BITMAP pDstBitmap,
                    PGFX_XY pDstPt, 
                    PGFX_OP pOpcode)
{
u_int32 rc = 0;
u_int32 Format;
u_int32 BltCmdType = TEXT_ALPHA, GxaConfig;
u_int32 PitchPixels;
#ifndef PCI_GXA
u_int32 GxaConfig2;
#endif
u_int8 * pCharBits;
#ifdef PCI_GXA
u_int32 CharOffset, CharPCIOffset;
#else
u_int32 CharOffset;
#endif
int StrLength;
u_int16 FontW, FontH, FontWBytes;
u_int16 FontStartIndex;
u_int16 DstX, DstY;
u_int16 CharWidth;
u_int16 AdjustX;
int i;

   /* This function does not handle greyscale fonts. Use GfxTextBltEx instead */
   if((pFont->Version >= 3) && 
     ((pFont->WidthType == GFX_FONTTYPE_ANTIALIASED) || (pFont->WidthType == GFX_FONTTYPE_AAFIXED)))
   {
      ERRMSG((GXA_ERR_MSG,"GXA ERR: Must call GfxTextBltEx for anti-aliased fonts!\n"));
      rc = GFXERR_NOT_SUPPORTED;
      return(rc);
   }
   
   if ( (pOpcode->BltControl & GFX_OP_ROP) && (pDstBitmap->Type & GFX_TF_NONRGB) ) {
      ERRMSG((GXA_ERR_MSG,"GXA ERR: ROPs to YCC destinations not supported!\n"));
      rc = GFXERR_NO_ROP_TO_YCC;
   } else {

      sem_get(GXA_Sem_Id, KAL_WAIT_FOREVER);

      StrLength = strlen(String);
   
      /* This sets up the BLT command and the number of parameters. */

      /* Operation is a copy if neither ROP or ALPHA is set. */
      if ( !(pOpcode->BltControl & (GFX_OP_ROP|GFX_OP_ALPHA)) ) {
         BltCmdType = (pOpcode->BltControl & GFX_OP_TRANS) ? TEXT_COPY_T : TEXT_COPY;
         /* Get the current configuration and clear the alpha control bits */
         mGetGXAConfig(GxaConfig)
         GxaConfig = (GxaConfig & 0xFE07FFFF);
         mSetGXAConfig(GxaConfig)
      } else {
         if (pOpcode->BltControl & GFX_OP_ALPHA) {
            if (pDstBitmap->Type & GFX_TF_ALPHA) {
#ifndef PCI_GXA
               BltCmdType = (pOpcode->BltControl & GFX_OP_TRANS) ? TEXT_ALPHA_T : TEXT_ALPHA;

               /* Set the alpha blending options for this operation */
               /* Get the current configuration
                * and clear the alpha control bits
                */
               mGetGXAConfig(GxaConfig)
               GxaConfig = (GxaConfig & 0xFE07FFFF);

               /* Check which type of alpha blend to use */
               if (pOpcode->AlphaUse & GFX_BLEND_INSIDE_OUT)
                  GxaConfig |= INSIDE_OUT_ALPHA;

               /* Check if we need to preserve the alpha */
               if (pOpcode->BltControl & GFX_OP_PRESERVE_ALPHA) {
                  /* Preserve alpha for this blt */
                  GxaConfig |= PRESERVE_ALPHA;
                  /* Check which alpha should be preserved */
                  switch ( pOpcode->AlphaUse & GFX_BLEND_STORE_MASK ) {
                     case GFX_BLEND_STORE_DST:
                        GxaConfig |= ALPHA_STORE_DST;
                        break;
                     case GFX_BLEND_STORE_SRC:
                        GxaConfig |= ALPHA_STORE_SRC;
                        break;
                     case GFX_BLEND_STORE_FIXED:
                        GxaConfig |= ALPHA_STORE_FIXED;
                        mGetGXAConfig2(GxaConfig2)
                        GxaConfig2 = (GxaConfig2 & 0xFFFFFF00l) | pOpcode->Alpha;
                        mSetGXAConfig2(GxaConfig2)
                        break;
                     default:
                        rc = GFXERR_BAD_ALPHA_SETUP;
                        ERRMSG((GXA_ERR_MSG,"GXA ERR: Illegal alpha store value!\n"));
                  }
               }

               /* Check which alpha to use for the blend */
               switch ( pOpcode->AlphaUse & GFX_BLEND_USE_MASK ) {
                  case GFX_BLEND_USE_DST:
                     GxaConfig |= USE_DST_ALPHA;
                     break;
                  case GFX_BLEND_USE_SRC:
                     GxaConfig |= USE_SRC_ALPHA;
                     break;
                  case GFX_BLEND_USE_FIXED:
                     GxaConfig |= USE_FIXED_ALPHA;
                     mGetGXAConfig2(GxaConfig2)
                     GxaConfig2 = (GxaConfig2 & 0xFFFFFF00l) | pOpcode->Alpha;
                     mSetGXAConfig2(GxaConfig2)
                     break;
                  default:
                     rc = GFXERR_BAD_ALPHA_SETUP;
                     ERRMSG((GXA_ERR_MSG,"GXA ERR: Illegal alpha use value!\n"));
               }

               mSetGXAConfig(GxaConfig)
#else
         /* If PCI_GXA just do a copy instead of alpha */
         BltCmdType = (pOpcode->BltControl & GFX_OP_TRANS) ? TEXT_COPY_T : TEXT_COPY;
#endif /* PCI_GXA */
            } else {
               ERRMSG((GXA_ERR_MSG,"GXA ERR: Cannot blend to destination!\n"));
               rc = GFXERR_NO_ALPHA_IN_DST;
            }
         } else {
            /* Set the ROP for this operation */
            mGetGXAConfig(GxaConfig)
            /* Clear ROP field and the preserve alpha setup */
            GxaConfig = (GxaConfig & 0xFE07FF00) | (pOpcode->ROP & 0x0F);
#ifndef PCI_GXA
            /* Check if we need to preserve the alpha */
            if (pOpcode->BltControl & GFX_OP_PRESERVE_ALPHA) {
               /* Preserve alpha for this blt */
               GxaConfig |= PRESERVE_ALPHA;
               /* Check which alpha should be preserved */
               switch ( pOpcode->AlphaUse & GFX_BLEND_STORE_MASK ) {
                  case GFX_BLEND_STORE_DST:
                     GxaConfig |= ALPHA_STORE_DST;
                     break;
                  case GFX_BLEND_STORE_SRC:
                     GxaConfig |= ALPHA_STORE_SRC;
                     break;
                  case GFX_BLEND_STORE_FIXED:
                     GxaConfig |= ALPHA_STORE_FIXED;
                     mGetGXAConfig2(GxaConfig2)
                     GxaConfig2 = (GxaConfig2 & 0xFFFFFF00l) | pOpcode->Alpha;
                     mSetGXAConfig2(GxaConfig2)
                     break;
                  default:
                     rc = GFXERR_BAD_ALPHA_SETUP;
                     ERRMSG((GXA_ERR_MSG,"GXA ERR: Illegal alpha store value!\n"));
               }
            }
#endif /* PCI_GXA */
            mSetGXAConfig(GxaConfig)

            BltCmdType = (pOpcode->BltControl & GFX_OP_TRANS) ? TEXT_ROP_T : TEXT_ROP;
         }
      }

      /* Translate the destination type to a format for the bitmap context */
      Format = GfxTranslateTypeToFormat(pDstBitmap->Type);
   
      /* setup context register 1 for destination */
      PitchPixels = GfxConvertPitchToPixels(pDstBitmap->Stride,pDstBitmap->Bpp);
      #ifdef PCI_GXA
      GxaSetupBitmapContext(1,BM_TYPE_COLOR,Format,
                     PitchPixels,(pDstBitmap->PCIOffset>>2));
      #else
      GxaSetupBitmapContext(1,BM_TYPE_COLOR,Format,
                     PitchPixels,(u_int32)pDstBitmap->pBits);
      #endif

      /* This is the width of the cell containing each glyph bitmap */
      FontW = pFont->CharWidth;
        
      if(pFont->Version >= 0x03)
      {
        FontH = pFont->MapHeight;
      }
      else  
      {
        FontH = pFont->CharHeight;
      }
      
      FontStartIndex = pFont->FirstFont;
      DstX = (u_int16)pDstPt->X;
      DstY = (u_int16)pDstPt->Y;
      
      /* Since source is MONO, translate the destination type to get format */
      Format = GfxTranslateTypeToFormat(pDstBitmap->Type);
   
      for (i=0; i< StrLength; i++)
      {
         FontWBytes = (FontW + 7)/8;
         CharOffset = ( (u_int16)*(unsigned char *)(String+i) - FontStartIndex) * FontWBytes * (FontH); 
         pCharBits = (u_int8 *)pFont->pBits + CharOffset;   
         #ifdef PCI_GXA
         CharPCIOffset = pFont->PCIOffset + CharOffset;
         #endif
         
         /* Pattern pointer MUST be word aligned. We can't guarantee this (the font is passed */
         /* to us) so we must fix it up and apply a dummy X offset if it is not correctly     */
         /* aligned.                                                                          */
         AdjustX = (u_int16)((u_int32)pCharBits & TEXT_PATTERN_ALIGN_MASK) * 8;
         pCharBits = (u_int8 *)((u_int32)pCharBits & ~TEXT_PATTERN_ALIGN_MASK);
         
         /* For variable width fonts, look up the width of the character in the     */
         /* font width table. The actual font patterns are stored in fixed width    */
         /* cells but this determines where the "cursor" moves after each character */
         /* is blitted.                                                             */
         
         if(pFont->Version >= 0x02)
         {
           if(pFont->WidthType == GFX_FONTWIDTH_VARIABLE)
           {
             CharWidth = (u_int16)(pFont->pWidths[(u_int16)((unsigned char)*(String+i)) - FontStartIndex]);
           }  
           else  
           {
             if(pFont->WidthType == GFX_FONTWIDTH_FIXED)
               CharWidth = (u_int16)pFont->pWidths[0];
             else 
               CharWidth = FontW;
           }    
         }
         else
           CharWidth = FontW;
           
         
         /* setup context register 2 for source (font bitmap)  */
         /* source is mono font bitmap */
         #ifdef PCI_GXA
         GxaSetupBitmapContext(2,BM_TYPE_MONO,Format, FontW, (CharPCIOffset>>2)); 
         #else                          
         GxaSetupBitmapContext(2,BM_TYPE_MONO,Format, FontW, (u_int32)pCharBits);
         #endif
   
         /* send the blt, source context 2, dest context 1, params 3 */
         mCheckFifo(4)
         mCmd2(GXA_BASE|BltCmdType|BMAP2_SRC|BMAP1_DST,
               mMakeXY(DstX, DstY),
               mMakeXY(CharWidth, pFont->CharHeight),
               mMakeXY(AdjustX, 0));
   
         /* Move to the next character position across the line */
         DstX = DstX + CharWidth;
         
         /* Version 4 of the GFXFONT structure added a width padding field */
         if(pFont->Version >= 0x04)
           DstX += pFont->WidthPad;
      }

      sem_put(GXA_Sem_Id);

   }     

   return (rc);
}

/******************************************************************************
 * GfxTextBltEx
 *
 * Parameters: char * String - Pointer to the string to render
 *             PGFX_FONT pFont - Pointer to the structure describing the
 *                               font to use.
 *             PGFX_BITMAP pDstBitmap - Pointer to structure describing
 *                                      the destination bitmap
 *             PGFX_XY pDstPt - Pointer to structure describing the
 *                              starting XY point in the destination.
 *             PGFX_OP pOpcode - Pointer to structure describing the
 *                               operation to perform and options.
 *             u_int8 pTextIndices[] - Pointer to an array of 4 palette
 *                               indices. [0] holds background colour,
 *                                        [1] holds 33% foreground
 *                                        [2] holds 66% foreground
 *                                        [3] holds foreground.
 *
 * Returns:
 *
 * Description:
 *
 *****************************************************************************/
u_int32 GfxTextBltEx( char * String,
                      PGFX_FONT pFont,
                      PGFX_BITMAP pDstBitmap,
                      PGFX_XY pDstPt, 
                      PGFX_OP pOpcode,
                      u_int8  pTextIndices[])
{
   u_int32 rc = 0;
   
   /* Determine whether we need to route this call to the greyscale or mono */
   /* text blitting function.                                               */
   if((pFont->Version >= 3) && 
     ((pFont->WidthType == GFX_FONTTYPE_ANTIALIASED) || (pFont->WidthType == GFX_FONTTYPE_AAFIXED)))
   {
      rc = InternalGreyTextBlt(String, pFont, pDstBitmap, pDstPt, pOpcode, pTextIndices);
   }
   else
   {
      rc = GfxTextBlt(String, pFont, pDstBitmap, pDstPt, pOpcode);
   }
   return(rc);
}

/******************************************************************************
 * GfxGreyTextBlt
 *
 * Parameters: char * String   - Pointer to the string to render
 *             PGFX_FONT pFont - Pointer to the structure describing the
 *                               font to use.
 *             PGFX_BITMAP pDstBitmap - Pointer to structure describing
 *                                      the destination bitmap
 *             PGFX_XY pDstPt  - Pointer to structure describing the
 *                               starting XY point in the destination.
 *             PGFX_OP pOpcode - Pointer to structure describing the
 *                               operation to perform and options.
 * Returns:
 *
 * Description: This function handles the special case of blitting a greyscale
 *              font. This is only currently supported for 2bpp fonts into
 *              8bpp or greater surfaces. In this case, we use a 2 pass blit to 
 *              draw the string.
 *
 *****************************************************************************/
static u_int32 InternalGreyTextBlt( char * String,
                                    PGFX_FONT pFont,
                                    PGFX_BITMAP pDstBitmap,
                                    PGFX_XY pDstPt, 
                                    PGFX_OP pOpcode,
                                    u_int8  pTextIndices[])
{
   u_int32    rc = 0;
   GFX_BITMAP sGlyphBitmap;
   GFX_XY     sTempDstPt;
   GFX_XY     sInterPt;
   GFX_OP     sInterOpcode;
   GFX_RECT   rectSource;
   int        StrLength;
   int        iLoop;
   int        iKalRetcode;
   bool       bNewIndices;
   u_int32    uForeground, uBackground;
   u_int32    uCharWidth;
   u_int32    uIntermediateWidth;
   u_int32    uIntermediateHeight;
   u_int32    uSavedBackground;
   u_int32    uClearColor;
   u_int32    uRetcode;
   u_int32    uPtrCorrect;
   GFX_COLOR  colColour;

   /* This function only handles 2bpp greyscale fonts into 8bpp and higher surfaces */
   if(((pFont->WidthType != GFX_FONTTYPE_ANTIALIASED) && (pFont->WidthType != GFX_FONTTYPE_AAFIXED)) ||
      (pFont->BitDepth != 2) || (pDstBitmap->Bpp < 8))
   {
     rc = GFXERR_CANNOT_CONVERT;
     ERRMSG((GXA_ERR_MSG,"GXA ERR: 2bpp greyscale fonts only supported into bitmaps with bpp >= 8!\n"));
     return(rc);
   }  
   
   if ( (pOpcode->BltControl & GFX_OP_ROP) && (pDstBitmap->Type & GFX_TF_NONRGB) )
   {
      ERRMSG((GXA_ERR_MSG,"GXA ERR: ROPs to YCC destinations not supported!\n"));
      rc = GFXERR_NO_ROP_TO_YCC;
   }
   else
   {
      /* How wide must the intermediate bitmap be?                                        */
      /* We include height and width padding in this bitmap so that when we blit we erase */
      /* everything underneath the character cell and don't leave stray pixels between    */
      /* characters.                                                                      */
      uIntermediateWidth  = pFont->CharWidth + ((pFont->Version >= 4) ? pFont->WidthPad : 0);
      uIntermediateHeight = pFont->MapHeight + ((pFont->Version >= 4) ? pFont->HeightPad : 0);
      
      /* If we have already allocated our intermediate bitmap but it is the */
      /* wrong size for this font, free it up.                              */
      if((pGreyRGBA32Bitmap != NULL) && 
         ((pGreyRGBA32Bitmap->Height != uIntermediateHeight) || (pGreyRGBA32Bitmap->Width != (((uIntermediateWidth/4)+3)&(~3)))))
      {
        GfxDeleteBitmap(pGreyRGBA32Bitmap);
        pGreyRGBA32Bitmap = NULL;
      }
         
      /* If we need to allocate a new intermediate bitmap, do it here */   
      if(pGreyRGBA32Bitmap == NULL)
      {
        /* We allocate the 32bpp bitmap then alias the 8bpp version on top of it since the 32bpp   */
        /* region has a larger pointer constraint (16 bytes rather than 4 for the 8bpp region). If */
        /* we do it the other way around, we end up generating some 32bpp alias bitmaps that have  */
        /* invalid bitmap pointers.                                                                */
        pGreyRGBA32Bitmap = GfxCreateBitmap(GFX_ARGB32, 32, ((uIntermediateWidth/4)+3)&(~3), uIntermediateHeight, NULL);
        if(!pGreyRGBA32Bitmap)
        {
          rc = GFXERR_OUT_OF_MEMORY;
          ERRMSG((GXA_ERR_MSG,"GXA ERR: Unable to allocate intermediate bitmap!\n"));
          return(rc);
        }
        
        /* Create a shadow copy of the intermediate bitmap and mark it as 8bpp RGB */
        sGreyRGBA8Bitmap.Version  = GFX_BITMAP_CURRENT_VERSION;
        sGreyRGBA8Bitmap.VerSize  = sizeof(GFX_BITMAP);
        sGreyRGBA8Bitmap.Bpp      = 8;
        sGreyRGBA8Bitmap.dwRef    = pGreyRGBA32Bitmap->dwRef;
        sGreyRGBA8Bitmap.Height   = pGreyRGBA32Bitmap->Height;
        sGreyRGBA8Bitmap.Width    = pGreyRGBA32Bitmap->Width*4;
        sGreyRGBA8Bitmap.Stride   = pGreyRGBA32Bitmap->Stride;
        sGreyRGBA8Bitmap.Type     = GFX_ARGB8;
        sGreyRGBA8Bitmap.pPalette = pGreyRGBA32Bitmap->pPalette;
        sGreyRGBA8Bitmap.pBits    = pGreyRGBA32Bitmap->pBits;
      }
      
      /* Sort out the palette indices to use in our intermediate step */
      if(!bGreyCharColorsSet)
        bNewIndices = TRUE;
      else    
        bNewIndices = FALSE;
      
      if(pTextIndices)
      {
        /* The caller provided the indices so use these */
        for(iLoop = 0; iLoop < 4; iLoop++)
        {
          if(gpIndices[iLoop] != pTextIndices[iLoop])
          {
            bNewIndices = TRUE;
            gpIndices[iLoop] = pTextIndices[iLoop];  
          }  
        }  
      }
      else
      {
        /* The caller failed to provide any indices so just use the first */
        /* four. We'll map these to real colors later if necessary.       */
        for(iLoop = 0; iLoop < 4; iLoop++)
        {
          if(gpIndices[iLoop] != iLoop)
          {
            bNewIndices = TRUE;
            gpIndices[iLoop] = iLoop;  
          }  
        }  
      }  
      
      /* Build the lookup table we will use to render into the intermediate buffer */
      if(bNewIndices)
      {
        InternalBuildTextColourLookup(gpIndices, gpLookupPalette);
        
        /* The lookup palette is in cached memory but we are about to pass it to the */
        /* hardware so we MUST ensure that all the values written to it have made it */
        /* out to the real SDRAM. Hence we flush this section of the data cache.     */
        FlushAndInvalDCacheRegion((u_int8 *)gpLookupPalette, sizeof(gpLookupPalette));
      }
      
      /* Populate another bitmap to represent the source glyphs (we'll muck with this in */
      /* the drawing loop but there's no point in doing the static fields more than we   */
      /* need to).                                                                       */
      sGlyphBitmap.Version  = GFX_BITMAP_CURRENT_VERSION;
      sGlyphBitmap.VerSize  = sizeof(GFX_BITMAP);
      sGlyphBitmap.Bpp      = 8;
      sGlyphBitmap.dwRef    = 0;
      sGlyphBitmap.Height   = pFont->CharHeight;
      sGlyphBitmap.Width    = pFont->CharWidth/4;
      sGlyphBitmap.Stride   = pFont->CharWidth/4;
      sGlyphBitmap.Type     = GFX_ARGB8;
      sGlyphBitmap.pPalette = (void *)SET_FLAGS(gpLookupPalette, NCR_BASE);
      
      /* If the destination format is something other than 8bpp, we need to fix up */
      /* the palette of our intermediate 8bpp bitmap to contain the real colours   */
      /* that are expected.                                                        */
      colColour.Flags = GFX_CF_ALPHA | GFX_CF_RGB;
      
      if(pDstBitmap->Bpp != 8)
      {
        uForeground = GfxGetFGColor();
        uBackground = GfxGetBGColor();  
        colColour.Value.dwVal = uBackground;
        GfxSetPaletteEntries(&sGreyRGBA8Bitmap, gpIndices[0], 1, &colColour);
        colColour.Value.dwVal = InternalColourInterpolate(uBackground, uForeground, DARK_GREY_TEXT_SCALE);
        GfxSetPaletteEntries(&sGreyRGBA8Bitmap, gpIndices[1], 1, &colColour);
        colColour.Value.dwVal = InternalColourInterpolate(uBackground, uForeground, LIGHT_GREY_TEXT_SCALE);
        GfxSetPaletteEntries(&sGreyRGBA8Bitmap, gpIndices[2], 1, &colColour);
        colColour.Value.dwVal = uForeground;
        GfxSetPaletteEntries(&sGreyRGBA8Bitmap, gpIndices[3], 1, &colColour);
      }
      else
      {
        /* If we are dealing with an 8bpp destination, we need to set the appropriate    */
        /* intermediate colours in the 4 palette locations provided for drawing the text */
        GfxGetPaletteEntries(pDstBitmap, gpIndices[0], 1, &colColour);
        uBackground = colColour.Value.dwVal;
        
        GfxGetPaletteEntries(pDstBitmap, gpIndices[3], 1, &colColour);
        uForeground = colColour.Value.dwVal;
        
        colColour.Value.dwVal = InternalColourInterpolate(uBackground, uForeground, DARK_GREY_TEXT_SCALE);
        GfxSetPaletteEntries(pDstBitmap, gpIndices[1], 1, &colColour);
        colColour.Value.dwVal = InternalColourInterpolate(uBackground, uForeground, LIGHT_GREY_TEXT_SCALE);
        GfxSetPaletteEntries(pDstBitmap, gpIndices[2], 1, &colColour);
      }
      
      /* Do some blit setup that is only needed once */
      sInterPt.X = 0;
      sInterPt.Y = 0;
      
      sTempDstPt.X = pDstPt->X;
      sTempDstPt.Y = pDstPt->Y;
      
      sInterOpcode.ROP        = GFX_ROP_COPY;
      sInterOpcode.BltControl = GFX_OP_COPY;
      sInterOpcode.Alpha      = 0;
      sInterOpcode.AlphaUse   = 0;

      /* Get the current background color and set up the color we will need */
      /* if we have to clear the intermediate bitmap.                       */
      mRepColor8(gpIndices[0], uClearColor);
      uSavedBackground = GfxGetBGColor();
      
      /* How long is the string we have to render? */
      StrLength = strlen(String);

      iKalRetcode = sem_get(GXA_Sem_Id, KAL_WAIT_FOREVER);
      if(iKalRetcode == RC_OK)
      {
        /* Draw each character in turn */
        for(iLoop = 0; iLoop < StrLength; iLoop++)
        {
          /* How wide is this character? */
          if(pFont->pWidths)
          {
            if(pFont->WidthType == GFX_FONTTYPE_AAFIXED)
              uCharWidth = (u_int32)(pFont->pWidths[0]);
            else
              uCharWidth = (u_int32)(pFont->pWidths[(u_int8)String[iLoop]-pFont->FirstFont]);
          }  
          else
            uCharWidth = pFont->CharWidth;  
        
          if(uCharWidth)
          {
            /* If the font has width and/or height padding, we need to clear the intermediate */
            /* bitmap first. This ensures that there are no stray pixels from the last glyph  */
            /* in the padding area and prevents garbage between characters in the output      */
            if((pFont->Version >= 4) && (pFont->HeightPad || pFont->WidthPad))
            {
              rectSource.Left   = 0;
              rectSource.Top    = 0;
              rectSource.Bottom = sGreyRGBA8Bitmap.Height;
              rectSource.Right  = sGreyRGBA8Bitmap.Width; 
              
              /* Set the required background color (don't call GfxSetBGColor - it claims the semaphore!) */
              mCheckFifo(1)
              mLoadReg(GXA_BG_COLOR_REG, uClearColor)
              
              uRetcode = InternalSolidBlt(&sGreyRGBA8Bitmap, &rectSource, &sInterOpcode);  
              
              /* Revert to original background color */
              mCheckFifo(1)
              mLoadReg(GXA_BG_COLOR_REG, uSavedBackground)
            }
            
            /* First, blit the appropriate glyph from the 2bpp font into our intermediate  */
            /* character buffer using the colour expansion hardware to convert from 2bpp   */
            /* to 8bpp (we do 4 pixels at a time using the palette as a lookup and telling */
            /* the hardware that it is doing an 8bpp to 32bpp expansion).                  */
        
            sGlyphBitmap.pBits = (void *)((u_int8 *)pFont->pBits + 
                                 pFont->CharWidth/4*pFont->MapHeight*(int)((u_int8)String[iLoop]-pFont->FirstFont));

            /* Source bitmap ptr must be word aligned so we muck with the pointer and */
            /* coordinates to ensure that this is the case.                           */
            uPtrCorrect = (u_int32)sGlyphBitmap.pBits % 4;
            sGlyphBitmap.pBits = (void *)((u_int32)sGlyphBitmap.pBits & ~3);
            
            rectSource.Left   = (short)uPtrCorrect;
            rectSource.Top    = 0;
            rectSource.Right  = pFont->CharWidth/4 + (short)uPtrCorrect;
            rectSource.Bottom = pFont->CharHeight;
                              
            uRetcode = InternalCopyBlt(&sGlyphBitmap,
                                       &rectSource,
                                       pGreyRGBA32Bitmap,
                                       &sInterPt,
                                       &sInterOpcode);
                                              
            /* Next, blit the glyph from the intermediate buffer to the destination and */
            /* apply any special features, ROPs, etc that were specified.               */
            rectSource.Left   = 0;
            rectSource.Right  = uCharWidth;
            if(pFont->Version >= 4)
            {
              /* Pad height and width if the font has padding defined */
              rectSource.Right += pFont->WidthPad;
              rectSource.Bottom += pFont->HeightPad;
            }  
            
            uRetcode = InternalCopyBlt(&sGreyRGBA8Bitmap,
                                       &rectSource,
                                       pDstBitmap,
                                       &sTempDstPt,
                                       pOpcode);
                                      
            sTempDstPt.X += uCharWidth;
            if(pFont->Version >= 0x04) 
              sTempDstPt.X += pFont->WidthPad;
          }
        }
        sem_put(GXA_Sem_Id);
      }  
   }     

   return (rc);
}

/********************************************************************/
/*  FUNCTION:    InternalColourInterpolate                          */
/*                                                                  */
/*  PARAMETERS:  uBack   - Background colour                        */
/*               uFront  - Foreground colour                        */
/*               uFactor - Mix factor in 256ths                     */
/*                                                                  */
/*  DESCRIPTION: Given foreground and background colours and a mix  */
/*               factor, return the appropriate colour which falls  */
/*               mix/256 between foreground and background.         */
/*                                                                  */
/*  RETURNS:     Interpolated colour                                */
/*                                                                  */
/*  CONTEXT:     May be called from any context                     */
/*                                                                  */
/********************************************************************/
static u_int32 InternalColourInterpolate(u_int32 uBack, u_int32 uFront, u_int32 uFactor)
{
  int iLoop;
  u_int32 uRet = 0;
  
  for(iLoop = 0; iLoop < 4; iLoop++)
  {
    uRet |= (((uBack & 0xFF) * (256-uFactor) + (uFront & 0xFF) * uFactor)/256) << (iLoop * 8);
    uBack  >>= 8;
    uFront >>= 8;
  }  
  return(uRet);
}

/********************************************************************/
/*  FUNCTION:    InternalBuildTextColorLookup                       */
/*                                                                  */
/*  PARAMETERS:  pIndices[] - array of 4 indices to use in output   */
/*                            lookup table.                         */
/*               pLookup[]  - array of 256 colours which will be    */
/*                            written with the lookup table values. */
/*                                                                  */
/*  DESCRIPTION: Create a lookup table allowing a source expand     */
/*               bit blit to be used to accelerate rendering of 2bpp*/
/*               greyscale fonts.                                   */
/*                                                                  */
/*  RETURNS:     Nothing                                            */
/*                                                                  */
/*  CONTEXT:     May be called in any context.                      */
/*                                                                  */
/********************************************************************/
static void InternalBuildTextColourLookup(u_int8 pIndices[], GFX_CVALUE pLookup[])
{
  int iLoop;
  
  for(iLoop = 0; iLoop < 256; iLoop++)
  {
    pLookup[iLoop].argb.A = pIndices[iLoop      % 4];               
    pLookup[iLoop].argb.B = pIndices[(iLoop/4)  % 4];
    pLookup[iLoop].argb.G = pIndices[(iLoop/16) % 4];
    pLookup[iLoop].argb.R = pIndices[(iLoop/64) % 4];
  }
}

/****************************************************************************
 * Modifications:
 * $Log: 
 *  27   mpeg      1.26        3/16/04 10:57:54 AM    Xin Golden      CR(s) 
 *        8567 : initialized CharWidth in GfxTextBlt in all cases to fix 
 *        warnings when build in release.
 *  26   mpeg      1.25        11/13/03 3:13:57 PM    Dave Wilson     CR(s): 
 *        7939 7940 Added GfxGetFontCharacterSize and GfxGetFontCellSize 
 *        functions.
 *        Updated GfxTextBlt to support fixed pitch mono fonts where the width 
 *        is not
 *        dictated by the glyph cell width but by entry 0 of the width table 
 *        (like th
 *        case of AAFIXED type fonts).
 *        
 *  25   mpeg      1.24        11/6/03 4:43:48 PM     Dave Wilson     CR(s): 
 *        7857 7856 Fixed GfxTextBlt to handle the case where a font bitmap 
 *        array is not aligned
 *        on a word boundary.
 *  24   mpeg      1.23        6/18/03 2:03:40 PM     Dave Wilson     SCR(s): 
 *        4821 
 *        InternalGreyTextBlt now blits the full width of the character 
 *        including any
 *        padding specified in the font. This prevents lines of old pixels 
 *        being left
 *        on the screen between characters in the padding lines/columns.
 *        
 *  23   mpeg      1.22        6/6/03 5:28:06 PM      Dave Wilson     SCR(s) 
 *        6142 :
 *        On a CPU running with write back cache enabled, we were seeing messed
 *         up
 *        colours when using greyscale text. This turned out to be due to the 
 *        fact that
 *        the lookup table was generated in cached memory so that the copy used
 *         by the
 *        
 *        GXA hardware was not the same as the one the CPU just wrote. Adding a
 *         cache
 *        flush after generating the lookup cures the problem (though 
 *        performance on a
 *        940 could suffer as a result).
 *        
 *  22   mpeg      1.21        4/8/03 10:57:14 AM     Dave Wilson     SCR(s) 
 *        5974 :
 *        Added support for GFX_FONTTYPE_AAFIXED. These are fixed width 
 *        anti-aliased
 *        fonts.
 *        
 *  21   mpeg      1.20        2/28/03 2:14:24 PM     Dave Wilson     SCR(s) 
 *        5620 :
 *        Fixed a problem in InternalGreyTextBlt which caused glyphs to be 
 *        corrupted in
 *        some cases. The 32bpp and 8bpp bitmaps used in the greyscale 
 *        expansion have
 *        different memory alignment constraints now and, as a result, the 
 *        expansion was
 *        failing in cases where the 8bpp region was created at an address 
 *        which was not
 *        a multiple of 16 bytes. To cure this, I now create the 32bpp region 
 *        using
 *        GfxCreateBitmap and alias my 8bpp region to it (rather than the other
 *         way
 *        round) since the 32bpp region has the more strict alignment 
 *        constraint.
 *        
 *  20   mpeg      1.19        2/28/03 10:57:54 AM    Dave Wilson     SCR(s) 
 *        5617 :
 *        Corrected a problem in anti-aliased text blitting where characters 
 *        with
 *        ASCII code >= 0x80 were corrupted. This was caused by a couple of 
 *        signed/
 *        unsigned mismatches in some arithmetic.
 *        
 *  19   mpeg      1.18        2/13/03 11:19:30 AM    Matt Korte      SCR(s) 
 *        5479 :
 *        Removed old header reference
 *        
 *  18   mpeg      1.17        10/7/02 5:47:34 PM     Dave Wilson     SCR(s) 
 *        4750 :
 *        Fixed things up so that fonts whose glyph cell is not a multiple of 
 *        16 will
 *        still display correctly. The problem was that in some cases the 
 *        character
 *        bitmap pointer would not be word aligned. This is worked around by 
 *        aligning
 *        the pointer than adjusting the start x and width for the initial 
 *        blit.
 *        
 *  17   mpeg      1.16        10/2/02 12:29:26 PM    Dave Wilson     SCR(s) 
 *        4621 :
 *        Added GfxTextBltEx and InternalGreyTextBlt to support rendering of
 *        2bpp greyscale fonts. Also reworked some internals to allow various 
 *        functions to be called from functions which already hold the access
 *        semaphore.
 *        
 *  16   mpeg      1.15        8/5/02 3:13:12 PM      Dave Wilson     SCR(s) 
 *        4331 :
 *        GfxTextBlt now uses the character rather than cell width to determine
 *         the 
 *        number of pixels to blit horizontally for each character. This 
 *        prevents 
 *        overdrawing at the right end of a string.
 *        
 *  15   mpeg      1.14        8/2/02 6:09:34 PM      Dave Wilson     SCR(s) 
 *        4321 :
 *        Oops - last edit contained one remaining reference to the wrong 
 *        height. This
 *        resulted in the fill character cell height being blitted in 
 *        GfxTextBlt rather
 *        than merely the valid character height.
 *        
 *  14   mpeg      1.13        8/2/02 1:11:12 PM      Dave Wilson     SCR(s) 
 *        4321 :
 *        GfxTextBlt now uses independent character and cell widths to allow, 
 *        for
 *        example, a 14 pixel high character to be described in a 16 pixel high
 *         cell.
 *        This is needed to maintain the requirement that all bitmap pointers 
 *        be 
 *        word aligned.
 *        
 *  13   mpeg      1.12        8/2/02 11:03:14 AM     Dave Wilson     SCR(s) 
 *        4318 :
 *        A signed/unsigned problem in GfxTextBlt caused the function to 
 *        corrupt
 *        all characters with ASCII values >= 0x80. This has now been fixed.
 *        
 *  12   mpeg      1.11        7/31/02 5:42:02 PM     Dave Wilson     SCR(s) 
 *        3044 :
 *        GfxTextBlt now supports variable width fonts (GFX_FONT structure 
 *        version 2).
 *        
 *  11   mpeg      1.10        8/28/00 6:41:16 PM     Lucy C Allevato Fixes for
 *         strict compiler checking of possible usage of variables before
 *        they are initialized.
 *        
 *  10   mpeg      1.9         8/23/00 3:00:00 PM     Lucy C Allevato Removed 
 *        relative include paths.
 *        
 *  9    mpeg      1.8         6/28/00 4:33:18 PM     Lucy C Allevato Added 
 *        usage of GXA semaphore to serialize accesses that program HW regs.
 *        
 *  8    mpeg      1.7         6/6/00 6:06:50 PM      Lucy C Allevato Fixed 
 *        comment style to not use C++ line comment.
 *        
 *  7    mpeg      1.6         6/5/00 2:03:52 PM      Lucy C Allevato Fixed 
 *        latent bug in setup of GxaConfig register for all blits. Clearing of
 *        the alpha preserve, alpha store, and alpha use is now done for all 
 *        blit
 *        types to prevent side effects from a previous blit setup.
 *        
 *  6    mpeg      1.5         5/24/00 8:34:30 PM     Lucy C Allevato Updated 
 *        Pitch setup to use the Stride from the bitmap recalculated to the
 *        pitch in pixels.
 *        
 *  5    mpeg      1.4         5/24/00 6:40:28 PM     Lucy C Allevato Added 
 *        support for new palette field in BITMAP structure. Also added an
 *        additional check of dwRef for non-zero and an error message if trying
 *        to do a conversion without a palette.
 *        
 *  4    mpeg      1.3         5/24/00 6:24:30 PM     Lucy C Allevato Fixed old
 *         reference to GFX_BF_REVERSE in GfxCopyBlt
 *        
 *  3    mpeg      1.2         5/19/00 3:58:10 PM     Lucy C Allevato Added 
 *        support for 16BPP to 16BPP conversions for new chip feature. Fixed
 *        a palette quantization problem that was causing chroma distortion in
 *        16BPP YCC to 16BPP YCC conversions.
 *        
 *  2    mpeg      1.1         3/31/00 4:49:34 PM     Lucy C Allevato Updates 
 *        to code for name changes in colorado.h to match up with spec.
 *        
 *  1    mpeg      1.0         3/23/00 3:53:56 PM     Lucy C Allevato 
 * $
 * 
 *    Rev 1.23   18 Jun 2003 13:03:40   dawilson
 * SCR(s): 4821 
 * InternalGreyTextBlt now blits the full width of the character including any
 * padding specified in the font. This prevents lines of old pixels being left
 * on the screen between characters in the padding lines/columns.
 * 
 *    Rev 1.22   06 Jun 2003 16:28:06   dawilson
 * SCR(s) 6142 :
 * On a CPU running with write back cache enabled, we were seeing messed up
 * colours when using greyscale text. This turned out to be due to the fact that
 * the lookup table was generated in cached memory so that the copy used by the
 * 
 * GXA hardware was not the same as the one the CPU just wrote. Adding a cache
 * flush after generating the lookup cures the problem (though performance on a
 * 940 could suffer as a result).
 * 
 *    Rev 1.21   08 Apr 2003 09:57:14   dawilson
 * SCR(s) 5974 :
 * Added support for GFX_FONTTYPE_AAFIXED. These are fixed width anti-aliased
 * fonts.
 * 
 *    Rev 1.20   28 Feb 2003 14:14:24   dawilson
 * SCR(s) 5620 :
 * Fixed a problem in InternalGreyTextBlt which caused glyphs to be corrupted in
 * some cases. The 32bpp and 8bpp bitmaps used in the greyscale expansion have
 * different memory alignment constraints now and, as a result, the expansion was
 * failing in cases where the 8bpp region was created at an address which was not
 * a multiple of 16 bytes. To cure this, I now create the 32bpp region using
 * GfxCreateBitmap and alias my 8bpp region to it (rather than the other way
 * round) since the 32bpp region has the more strict alignment constraint.
 * 
 *    Rev 1.19   28 Feb 2003 10:57:54   dawilson
 * SCR(s) 5617 :
 * Corrected a problem in anti-aliased text blitting where characters with
 * ASCII code >= 0x80 were corrupted. This was caused by a couple of signed/
 * unsigned mismatches in some arithmetic.
 * 
 *    Rev 1.18   13 Feb 2003 11:19:30   kortemw
 * SCR(s) 5479 :
 * Removed old header reference
 * 
 *    Rev 1.17   07 Oct 2002 16:47:34   dawilson
 * SCR(s) 4750 :
 * Fixed things up so that fonts whose glyph cell is not a multiple of 16 will
 * still display correctly. The problem was that in some cases the character
 * bitmap pointer would not be word aligned. This is worked around by aligning
 * the pointer than adjusting the start x and width for the initial blit.
 * 
 *    Rev 1.16   02 Oct 2002 11:29:26   dawilson
 * SCR(s) 4621 :
 * Added GfxTextBltEx and InternalGreyTextBlt to support rendering of
 * 2bpp greyscale fonts. Also reworked some internals to allow various 
 * functions to be called from functions which already hold the access
 * semaphore.
 * 
 *    Rev 1.15   05 Aug 2002 14:13:12   dawilson
 * SCR(s) 4331 :
 * GfxTextBlt now uses the character rather than cell width to determine the 
 * number of pixels to blit horizontally for each character. This prevents 
 * overdrawing at the right end of a string.
 * 
 *    Rev 1.14   02 Aug 2002 17:09:34   dawilson
 * SCR(s) 4321 :
 * Oops - last edit contained one remaining reference to the wrong height. This
 * resulted in the fill character cell height being blitted in GfxTextBlt rather
 * than merely the valid character height.
 * 
 *    Rev 1.13   02 Aug 2002 12:11:12   dawilson
 * SCR(s) 4321 :
 * GfxTextBlt now uses independent character and cell widths to allow, for
 * example, a 14 pixel high character to be described in a 16 pixel high cell.
 * This is needed to maintain the requirement that all bitmap pointers be 
 * word aligned.
 * 
 *    Rev 1.12   02 Aug 2002 10:03:14   dawilson
 * SCR(s) 4318 :
 * A signed/unsigned problem in GfxTextBlt caused the function to corrupt
 * all characters with ASCII values >= 0x80. This has now been fixed.
 * 
 *    Rev 1.11   31 Jul 2002 16:42:02   dawilson
 * SCR(s) 3044 :
 * GfxTextBlt now supports variable width fonts (GFX_FONT structure version 2).
 * 
 *    Rev 1.10   28 Aug 2000 17:41:16   eching
 * Fixes for strict compiler checking of possible usage of variables before
 * they are initialized.
 * 
 *    Rev 1.9   23 Aug 2000 14:00:00   eching
 * Removed relative include paths.
 * 
 *    Rev 1.8   28 Jun 2000 15:33:18   eching
 * Added usage of GXA semaphore to serialize accesses that program HW regs.
 * 
 *    Rev 1.7   06 Jun 2000 17:06:50   eching
 * Fixed comment style to not use C++ line comment.
 * 
 *    Rev 1.6   05 Jun 2000 13:03:52   eching
 * Fixed latent bug in setup of GxaConfig register for all blits. Clearing of
 * the alpha preserve, alpha store, and alpha use is now done for all blit
 * types to prevent side effects from a previous blit setup.
 * 
 *    Rev 1.5   24 May 2000 19:34:30   eching
 * Updated Pitch setup to use the Stride from the bitmap recalculated to the
 * pitch in pixels.
 * 
 *    Rev 1.4   24 May 2000 17:40:28   eching
 * Added support for new palette field in BITMAP structure. Also added an
 * additional check of dwRef for non-zero and an error message if trying
 * to do a conversion without a palette.
 * 
 *    Rev 1.3   24 May 2000 17:24:30   eching
 * Fixed old reference to GFX_BF_REVERSE in GfxCopyBlt
 * 
 *    Rev 1.2   19 May 2000 14:58:10   eching
 * Added support for 16BPP to 16BPP conversions for new chip feature. Fixed
 * a palette quantization problem that was causing chroma distortion in
 * 16BPP YCC to 16BPP YCC conversions.
 * 
 *    Rev 1.1   31 Mar 2000 16:49:34   eching
 * Updates to code for name changes in colorado.h to match up with spec.
 * 
 *    Rev 1.0   23 Mar 2000 15:53:56   eching
 * Initial revision.
 *
 ****************************************************************************/

