/****************************************************************************/
/*                   Conexant Proprietary and Confidential                  */
/*                       Software File/Module Header                        */
/*          Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002          */
/*                               Austin, TX                                 */
/*                            All Rights Eeserved                           */
/****************************************************************************/
/*
 * Filename:       PALETTE.C
 *
 *
 * Description:    Graphics Hardware Palette Library Routines
 *
 *
 * Author:         Anzhi Chen
 *
 ****************************************************************************
 $Header: palette.c, 8, 9/22/03 11:56:30 AM, Dave Wilson$
 ****************************************************************************/

/***** Exported Functions **************************************************
GfxSetPaletteEntries()
GfxGetPaletteEntries()
****************************************************************************/


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


/**********************************************************************************/
/* Function: GfxSetPaletteEntries()                                               */
/*                                                                                */
/* Parameters: PGFX_BITMAP pBitmap - Pointer to the bitmap whose palette to be set*/
/*             u_int32 Start_Index - Start index of the palette entry to be set   */
/*             u_int32 Num_Entries - Number of entries to be set                  */
/*             PGFX_COLOR pColor - Pointer to palette data in GFX_COLOR format    */
/*                                                                                */
/* Returns: u_int32 - 0 if sucessful                                              */
/*                  - a positive value that defines the reason of failure         */
/*                                                                                */                                                                              
/* Description: Sets the palette for the GFX bitmap                               */
/**********************************************************************************/
u_int32 GfxSetPaletteEntries(PGFX_BITMAP pBitmap, u_int32 Start_Index,
                        u_int32 Num_Entries, PGFX_COLOR pColor)
{
   u_int32 dwRet = 0;
   u_int32 hRgn; 
   POSDLIBREGION pRgn;
   u_int32 dwNumEntries = 0;
   u_int32 dwPalSize = 0;
   PDRMPAL lpDest = NULL;
   u_int32 dwCount;
   bool    bRGB = FALSE;
   u_int8 bSetAlpha = FALSE;

   hRgn = pBitmap->dwRef;
   if ((pRgn = (POSDLIBREGION)hRgn)!=0)
   {
      dwRet = GfxGetPaletteSizeAndColorSpace(pRgn->dwMode, &bRGB, &dwPalSize);

      /* Bail out if we discovered a problem already */
      if(dwRet)
        return(dwRet);
        
      /* Make sure the entries passed fall within the palette for this region */  
      if (Start_Index > (dwPalSize - 1))
         return(ERR_BAD_START_INDEX);
      else
      {
         if ( (Start_Index+Num_Entries) > dwPalSize ) 
            return(ERR_BAD_NUM_ENTRIES);     
         else
         {
            dwNumEntries = Num_Entries;
            if (pRgn->pPalette)
               lpDest = pRgn->pPalette;
            else
              return(ERR_FLAG_NO_PALETTE);
         }      
      }

      /* Check the flag of the first PGFX_COLOR entry  */
      if (!((u_int32)pColor[0].Flags & GFX_CM_TYPE)) 
         dwRet = ERR_BAD_GFX_FLAG;
      else
      {  
         /* Check the flag of the first GFX_COLOR in the array. */
         if (pColor[0].Flags & GFX_CF_ALPHA)
            bSetAlpha = TRUE;             
         
         if (pColor[0].Flags & GFX_CF_RGB)
         {         
            if (bRGB)
            {
               for (dwCount=0; dwCount<dwNumEntries; dwCount++)
               {
                  CNXT_SET_VAL(&(lpDest[Start_Index+dwCount].rgb),DRMRGBPAL_R_MASK,(u_int32)pColor[dwCount].Value.argb.R);
                  CNXT_SET_VAL(&(lpDest[Start_Index+dwCount].rgb),DRMRGBPAL_G_MASK,(u_int32)pColor[dwCount].Value.argb.G);
                  CNXT_SET_VAL(&(lpDest[Start_Index+dwCount].rgb),DRMRGBPAL_B_MASK,(u_int32)pColor[dwCount].Value.argb.B);
                  if (bSetAlpha == TRUE)
                  {
                     CNXT_SET_VAL(&(lpDest[Start_Index+dwCount].rgb),DRMRGBPAL_ALPHA_MASK,(u_int32)pColor[dwCount].Value.argb.A);
                  }   
               }
            }
            else
               dwRet = ERR_FLAG_NOT_MATCH_MODE;
         }
         else if (pColor[0].Flags & GFX_CF_YCC)
         {
            if (!bRGB)
            {         
               for (dwCount=0; dwCount<dwNumEntries; dwCount++)
               {
                  CNXT_SET_VAL(&(lpDest[Start_Index+dwCount].ycc),DRMYCCPAL_Y_MASK,(u_int32)pColor[dwCount].Value.aycc.Y);
                  CNXT_SET_VAL(&(lpDest[Start_Index+dwCount].ycc),DRMYCCPAL_CB_MASK,(u_int32)pColor[dwCount].Value.aycc.Cb);
                  CNXT_SET_VAL(&(lpDest[Start_Index+dwCount].ycc),DRMYCCPAL_CR_MASK,(u_int32)pColor[dwCount].Value.aycc.Cr);
                  if (bSetAlpha == TRUE)
                  {
                     CNXT_SET_VAL(&(lpDest[Start_Index+dwCount].ycc),DRMYCCPAL_ALPHA_MASK,(u_int32)pColor[dwCount].Value.aycc.A);
                  }
               }
            }
            else
               dwRet = ERR_FLAG_NOT_MATCH_MODE;
         }
      }
   }
   else
   {
      ERRMSG((GXA_ERR_MSG, "GXA ERROR: Invalid OSD handle!\n"));
      dwRet = ERR_FLAG_BAD_OSD_RGN;
   }

   return dwRet;
}

/**********************************************************************************/
/* Function: GfxGetPaletteEntries()                                               */
/*                                                                                */
/* Parameters: PGFX_BITMAP pBitmap - Pointer to the bitmap whose palette to get   */
/*             u_int32 Start_Index - Start index of the palette entry to get      */
/*             u_int32 Num_Entries - Number of entries to get                     */
/*             PGFX_COLOR pColor - Pointer to GFX_COLOR array that will hold the  */
/*                                 palette entries                                */
/*                                                                                */
/* Returns: u_int32 - 0 if sucessful                                              */
/*                  - a positive value that defines the reason of failure         */
/*                                                                                */
/* Description: Gets the palette entries associated with the indexes              */
/**********************************************************************************/
u_int32 GfxGetPaletteEntries(PGFX_BITMAP pBitmap, u_int32 Start_Index,
                        u_int32 Num_Entries, PGFX_COLOR pColor)
{
   u_int32 dwRet = 0;
   u_int32 hRgn; 
   POSDLIBREGION pRgn;
   u_int32 dwNumEntries = 0;
   u_int32 dwPalSize;
   PDRMPAL lpSrc = NULL;
   u_int32 dwCount;
   bool    bRGB = FALSE;
   u_int8 bGetAlpha = FALSE;
   
   hRgn = pBitmap->dwRef;
   if ((pRgn = (POSDLIBREGION)hRgn)!=0)
   {
      dwRet = GfxGetPaletteSizeAndColorSpace(pRgn->dwMode, &bRGB, &dwPalSize);

      /* Bail out if we discovered a problem already */
      if(dwRet)
        return(dwRet);
        
      /* Make sure the entries passed fall within the palette for this region */  
      if (Start_Index > (dwPalSize - 1))
         return(ERR_BAD_START_INDEX);
      else
      {
         if ( (Start_Index+Num_Entries) > dwPalSize ) 
            return(ERR_BAD_NUM_ENTRIES);     
         else
         {
            dwNumEntries = Num_Entries;
            if (pRgn->pPalette)
               lpSrc = pRgn->pPalette;
            else
              return(ERR_FLAG_NO_PALETTE);
         }      
      }

      /* Check the flag of the first PGFX_COLOR entry */
      if (!((u_int32)pColor[0].Flags & GFX_CM_TYPE)) 
         dwRet = ERR_BAD_GFX_FLAG;
      else
      {  
         if (pColor[0].Flags & GFX_CF_ALPHA)
            bGetAlpha = TRUE;
                         
         if (pColor[0].Flags & GFX_CF_RGB)
         {         
            if (bRGB)
            {
               for (dwCount=0; dwCount<dwNumEntries; dwCount++)
               {
                  pColor[dwCount].Value.argb.R = CNXT_GET_VAL(&lpSrc[Start_Index+dwCount].rgb, DRMRGBPAL_R_MASK);
                  pColor[dwCount].Value.argb.G = CNXT_GET_VAL(&lpSrc[Start_Index+dwCount].rgb, DRMRGBPAL_G_MASK);
                  pColor[dwCount].Value.argb.B = CNXT_GET_VAL(&lpSrc[Start_Index+dwCount].rgb, DRMRGBPAL_B_MASK);
                  if (bGetAlpha == TRUE)
                  {
                     pColor[dwCount].Value.argb.A = CNXT_GET_VAL(&lpSrc[Start_Index+dwCount].rgb, DRMRGBPAL_ALPHA_MASK);
                  }
               }
            }
            else
               dwRet = ERR_FLAG_NOT_MATCH_MODE;
         }
         else if (pColor[0].Flags & GFX_CF_YCC)
         {
            if (!bRGB)
            {         
               for (dwCount=0; dwCount<dwNumEntries; dwCount++)
               {
                  pColor[dwCount].Value.aycc.Y = CNXT_GET_VAL(&lpSrc[Start_Index+dwCount].ycc, DRMYCCPAL_Y_MASK);
                  pColor[dwCount].Value.aycc.Cb = CNXT_GET_VAL(&lpSrc[Start_Index+dwCount].ycc, DRMYCCPAL_CB_MASK);
                  pColor[dwCount].Value.aycc.Cr = CNXT_GET_VAL(&lpSrc[Start_Index+dwCount].ycc, DRMYCCPAL_CR_MASK);
                  if (bGetAlpha == TRUE)
                  {
                     pColor[dwCount].Value.aycc.A = CNXT_GET_VAL(&lpSrc[Start_Index+dwCount].ycc, DRMYCCPAL_ALPHA_MASK);
                  }
               }
            }
            else
               dwRet = ERR_FLAG_NOT_MATCH_MODE;
         }
      }
   }
   else
   {
      ERRMSG((GXA_ERR_MSG, "GXA ERROR: Invalid OSD handle!\n"));
      dwRet = ERR_FLAG_BAD_OSD_RGN;
   }

   return dwRet;
}

/**********************************************************************************/
/* Function: GfxGetPaletteSizeAndColorSpace                                       */
/*                                                                                */
/* Parameters: u_int32  dwMode   - Bitmap mode (from OSDLIBC.H)                   */
/*             bool    *pbRGB    - Returned TRUE if RGB bitmap, FALSE if YUV      */
/*             u_int32 *pPalSize - Returned number of entries in bitmap palette   */
/*                                                                                */
/* Returns: u_int32 - 0 if sucessful                                              */
/*                  - a positive value that defines the reason of failure         */
/*                                                                                */
/* Description: Returns the number of palette entries and the color space for a   */
/*              given pixel mode.                                                 */
/**********************************************************************************/
u_int32 GfxGetPaletteSizeAndColorSpace(u_int32 dwMode, bool *pbRGB, u_int32 *pPalSize)
{
   u_int32 dwRet = 0;
  
  if(!pbRGB || !pPalSize)
    return(ERR_FLAG_BAD_PTR);
    
  switch (dwMode)
  {
    case OSD_MODE_4ARGB:
       *pbRGB    = TRUE;
       *pPalSize = 16;
       break;
      
       /* Fall thru */
    case OSD_MODE_4AYUV:
       *pbRGB    = FALSE;
       *pPalSize = 16;
       break;
       
    case OSD_MODE_8ARGB:
       *pbRGB    = TRUE;
       *pPalSize = 256;
       break;
       
       /* Fall thru */
    case OSD_MODE_8AYUV:
       *pbRGB    = FALSE;
       *pPalSize = 256;
       break;
      
    case OSD_MODE_16ARGB: /* 4:4:4:4 */
       *pbRGB    = TRUE;
       *pPalSize = 16;
       break;
       
       break;
    case OSD_MODE_16RGB: /* 0:5:6:5 */
       *pbRGB    = TRUE;
       *pPalSize = 64;
       break;
       
    case OSD_MODE_16ARGB1555: /* 1:5:5:5 */
       *pbRGB    = TRUE;
       *pPalSize = 32;
       break;
  
  
    case OSD_MODE_32ARGB: /* 8:8:8:8 */
       *pbRGB    = TRUE;
       *pPalSize = 256;
       break;
       
    /* OSD modes which have no palette associated with them */
    case OSD_MODE_16AYUV:    
    case OSD_MODE_16YUV655:  
    case OSD_MODE_16YUV422:  
    case OSD_MODE_16AYUV2644:
    case OSD_MODE_32AYUV:    
       *pbRGB    = FALSE;
       *pPalSize = 0;
       dwRet = ERR_FLAG_NO_PALETTE;
       break;
       
    /* Illegal OSD modes */
    default:
       dwRet = ERR_FLAG_BAD_OSD_MODE;
       break;
  }           
  
  return(dwRet);
}
/***************************************************************************
                     PVCS Version Control Information
$Log: 
 8    mpeg      1.7         9/22/03 11:56:30 AM    Dave Wilson     SCR(s) 7006 
       :
       Fixed and reworked GfxGetPaletteEntries and GfxSetPaletteEntries. 
       Previous
       versions would not allow the palette of a 16 or 32bpp RGB bitmap to be 
       set.
       Also added GfxGetPaletteSizeAndColorSpace function to remove chunk of 
       code
       which was duplicated in the original get and set functions. This is made
       public since it may be useful to applications as well as internal 
       functions.
       
 7    mpeg      1.6         2/13/03 12:05:50 PM    Matt Korte      SCR(s) 5479 
       :
       Removed old header reference
       
 6    mpeg      1.5         10/2/02 12:30:46 PM    Dave Wilson     SCR(s) 4621 
       :
       Removed the #ifdef GFX_HI_LVL_FUNCS since some functions here are used
       in GfxTextBltEx which is not in the old HI_LVL_FUNCS group.
       
 5    mpeg      1.4         9/23/02 6:08:28 PM     Lucy C Allevato SCR(s) 4640 
       :
       remove DRM bitfields conditionally from palette.c
       
 4    mpeg      1.3         3/21/02 10:15:50 AM    Dave Wilson     SCR(s) 3416 
       :
       Removed a compiler warning when code is compiled to nothing.
       
 3    mpeg      1.2         8/23/00 3:01:20 PM     Lucy C Allevato Removed 
       relative include paths.
       
 2    mpeg      1.1         6/6/00 6:07:14 PM      Lucy C Allevato Fixed 
       comment style to not use C++ line comment.
       
 1    mpeg      1.0         3/23/00 3:54:04 PM     Lucy C Allevato 
$
 * 
 *    Rev 1.7   22 Sep 2003 10:56:30   dawilson
 * SCR(s) 7006 :
 * Fixed and reworked GfxGetPaletteEntries and GfxSetPaletteEntries. Previous
 * versions would not allow the palette of a 16 or 32bpp RGB bitmap to be set.
 * Also added GfxGetPaletteSizeAndColorSpace function to remove chunk of code
 * which was duplicated in the original get and set functions. This is made
 * public since it may be useful to applications as well as internal functions.
 * 
 *    Rev 1.6   13 Feb 2003 12:05:50   kortemw
 * SCR(s) 5479 :
 * Removed old header reference
 * 
 *    Rev 1.5   02 Oct 2002 11:30:46   dawilson
 * SCR(s) 4621 :
 * Removed the #ifdef GFX_HI_LVL_FUNCS since some functions here are used
 * in GfxTextBltEx which is not in the old HI_LVL_FUNCS group.
 * 
 *    Rev 1.4   23 Sep 2002 17:08:28   goldenx
 * SCR(s) 4640 :
 * remove DRM bitfields conditionally from palette.c
 * 
 *    Rev 1.3   21 Mar 2002 10:15:50   dawilson
 * SCR(s) 3416 :
 * Removed a compiler warning when code is compiled to nothing.
 * 
 *    Rev 1.2   23 Aug 2000 14:01:20   eching
 * Removed relative include paths.
 * 
 *    Rev 1.1   06 Jun 2000 17:07:14   eching
 * Fixed comment style to not use C++ line comment.
 * 
 *    Rev 1.0   23 Mar 2000 15:54:04   eching
 * Initial revision.
****************************************************************************/

