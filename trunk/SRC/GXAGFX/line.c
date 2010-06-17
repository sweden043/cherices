/***************************************************************************
                                                                          
   Filename: LINE.C

   Description: Graphics Hardware Line Library Routine

   Created: 1/10/2000, Eric Ching

   Copyright Conexant Systems, 2000
   All Rights Reserved.

****************************************************************************/

/***** Exported Functions **************************************************
GfxPolyLine()
****************************************************************************/

/***************************************************************************
                     PVCS Version Control Information
$Header: line.c, 9, 2/13/03 12:05:54 PM, Matt Korte$
$Log: 
 9    mpeg      1.8         2/13/03 12:05:54 PM    Matt Korte      SCR(s) 5479 
       :
       Removed old header reference
       
 8    mpeg      1.7         8/28/00 6:42:14 PM     Lucy C Allevato Fixes for 
       strict compiler checking of possible usage of variables before
       they are initialized.
       
 7    mpeg      1.6         8/23/00 3:00:30 PM     Lucy C Allevato Removed 
       relative include paths.
       
 6    mpeg      1.5         6/28/00 4:33:46 PM     Lucy C Allevato Added usage 
       of GXA semaphore to serialize accesses that program HW regs.
       
 5    mpeg      1.4         6/6/00 6:07:06 PM      Lucy C Allevato Fixed 
       comment style to not use C++ line comment.
       
 4    mpeg      1.3         6/5/00 2:06:50 PM      Lucy C Allevato Fixed latent
        bug in GxaConfig setup that allowed a previous GxaConfig setup
       to affect the current operation. Now alpha preserve, alpha store and 
       alpha
       use fileds are cleared for all operations.
       
 3    mpeg      1.2         5/24/00 8:33:18 PM     Lucy C Allevato Updated 
       Pitch setup to use the Stride from the bitmap recalculated to the
       pitch in pixels.
       
 2    mpeg      1.1         3/31/00 4:50:10 PM     Lucy C Allevato Updates to 
       code for name changes in colorado.h to match up with spec.
       
 1    mpeg      1.0         3/23/00 3:53:58 PM     Lucy C Allevato 
$
 * 
 *    Rev 1.8   13 Feb 2003 12:05:54   kortemw
 * SCR(s) 5479 :
 * Removed old header reference
 * 
 *    Rev 1.7   28 Aug 2000 17:42:14   eching
 * Fixes for strict compiler checking of possible usage of variables before
 * they are initialized.
 * 
 *    Rev 1.6   23 Aug 2000 14:00:30   eching
 * Removed relative include paths.
 * 
 *    Rev 1.5   28 Jun 2000 15:33:46   eching
 * Added usage of GXA semaphore to serialize accesses that program HW regs.
 * 
 *    Rev 1.4   06 Jun 2000 17:07:06   eching
 * Fixed comment style to not use C++ line comment.
 * 
 *    Rev 1.3   05 Jun 2000 13:06:50   eching
 * Fixed latent bug in GxaConfig setup that allowed a previous GxaConfig setup
 * to affect the current operation. Now alpha preserve, alpha store and alpha
 * use fileds are cleared for all operations.
 * 
 *    Rev 1.2   24 May 2000 19:33:18   eching
 * Updated Pitch setup to use the Stride from the bitmap recalculated to the
 * pitch in pixels.
 * 
 *    Rev 1.1   31 Mar 2000 16:50:10   eching
 * Updates to code for name changes in colorado.h to match up with spec.
 * 
 *    Rev 1.0   23 Mar 2000 15:53:58   eching
 * Initial revision.
****************************************************************************/

#include "stbcfg.h"
#include "kal.h"
#include "gfxtypes.h"
#include "gxagfx.h"
#include "genmac.h"
#include "gxablt.h"
#include "context.h"
#include "gfx_os.h"
#include "gfxutls.h"

extern sem_id_t GXA_Sem_Id;

/****************************************************************************
 * GfxPolyLine
 ***************************************************************************/
u_int32 GfxPolyLine(PGFX_BITMAP pDstBitmap,
                    PGFX_XY pPoints,
                    u_int32 NumPoints,
                    PGFX_OP pOpcode)
{
u_int32 rc = 0;
u_int32 PtIdx = 0;
u_int32 Format;
u_int32 LineCmdType = LINE_COPY_CMD, PLineCmdType;
u_int32 GxaConfig;
u_int32 PitchPixels;
#ifndef PCI_GXA
u_int32 GxaConfig2;
#endif
int16  XStart = 0, YStart = 0, XNext = 0, YNext = 0;
int16  DeltaX, DeltaY;

   if (NumPoints > 1) {
      XStart = pPoints[PtIdx].X;
      YStart = pPoints[PtIdx].Y;
      PtIdx++;
      XNext = pPoints[PtIdx].X;
      YNext = pPoints[PtIdx].Y;
   } else {
      ERRMSG((GXA_ERR_MSG,"GXA ERR: Not enough points in PolyLine!\n"));
      rc = GFXERR_NUM_POINTS;
   }

   if ( (pOpcode->BltControl & GFX_OP_ROP) && (pDstBitmap->Type & GFX_TF_NONRGB) ) {
      ERRMSG((GXA_ERR_MSG,"GXA ERR: ROPs to YCC destinations not supported!\n"));
      rc = GFXERR_NO_ROP_TO_YCC;
   }

   if ( ((pDstBitmap->Type & GFX_NBPP_MASK) != GFX_4BPP) && !rc ) {

      sem_get(GXA_Sem_Id, KAL_WAIT_FOREVER);

      /* Translate type to hardware format */
      Format = GfxTranslateTypeToFormat(pDstBitmap->Type);

      /* setup context register 0 for destination */
      PitchPixels = GfxConvertPitchToPixels(pDstBitmap->Stride,pDstBitmap->Bpp);
#ifdef PCI_GXA
      GxaSetupBitmapContext(0,BM_TYPE_COLOR,Format,PitchPixels,(pDstBitmap->PCIOffset>>2));
#else
      GxaSetupBitmapContext(0,BM_TYPE_COLOR,Format,PitchPixels,(u_int32)pDstBitmap->pBits);
#endif

      /* This sets up the LINE command and the number of parameters
       * and the POLYLINE command and the number of parameters
       * Operation is a copy if neither ROP or ALPHA is set.
       */
      if ( !(pOpcode->BltControl & (GFX_OP_ROP|GFX_OP_ALPHA)) ) {
         LineCmdType = (pOpcode->BltControl & GFX_OP_TRANS) ?
                        LINE_COPY_T_CMD : LINE_COPY_CMD;
         /* Get the current configuration and clear the alpha control bits */
         mGetGXAConfig(GxaConfig)
         GxaConfig = (GxaConfig & 0xFE07FFFF);
         mSetGXAConfig(GxaConfig)
      } else {
         if (pOpcode->BltControl & GFX_OP_ALPHA) {
            if (pDstBitmap->Type & GFX_TF_ALPHA) {
#ifndef PCI_GXA
               LineCmdType = (pOpcode->BltControl & GFX_OP_TRANS) ?
                              LINE_ALPHA_T_CMD : LINE_ALPHA_CMD;

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
               LineCmdType = (pOpcode->BltControl & GFX_OP_TRANS) ?
                              LINE_COPY_T_CMD : LINE_COPY_CMD;
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
#endif /* PCI_GXA */
            mSetGXAConfig(GxaConfig)

            LineCmdType = (pOpcode->BltControl & GFX_OP_TRANS) ?
                           LINE_ROP_T_CMD : LINE_ROP_CMD;
         }
      }

      PLineCmdType = LineCmdType | ONE_PARAM_CMD;
      LineCmdType |= TWO_PARAMS_CMD;

      DeltaX = XNext - XStart;
      DeltaY = YNext - YStart;

      /* send the line, source context 0, dest context 0, params 2 */
      mCheckFifo(3)
      mCmd1(GXA_BASE|LineCmdType|BMAP0_SRC|BMAP0_DST,
            mMakeXY(XNext,YNext),
            mMakeXY(XStart,YStart))

      while (++PtIdx < NumPoints) {
         DeltaX = pPoints[PtIdx].X - XNext;
         DeltaY = pPoints[PtIdx].Y - YNext;

         XNext = pPoints[PtIdx].X;
         YNext = pPoints[PtIdx].Y;
         mCheckFifo(1)
         mCmd0(GXA_BASE|PLineCmdType|BMAP0_SRC|BMAP0_DST,
               mMakeXY(XNext,YNext))

      }

      sem_put(GXA_Sem_Id);

   } else if (!rc) {
      ERRMSG((GXA_ERR_MSG,"GXA ERR: Illegal 4 bpp line draw!\n"));
      rc = GFXERR_ILLEGAL_4BPP_OP;         
   } /* ENDIF NOT 4BPP */

   return (rc);
}

