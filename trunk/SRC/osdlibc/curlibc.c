/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*           Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002         */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       CURLIBC.C
 *
 *
 * Description:    OSD Cusor control library implementation
 *
 *
 * Author:         Rob Tilton, Eric Ching and Dave Wilson
 *
 ****************************************************************************/
 /* $Id: curlibc.c,v 1.23, 2003-11-24 21:55:55Z, Dave Wilson$
 ****************************************************************************/

/*****************************************************************************/
/* Functions:                                                                */
/***** Exported Cursor Functions *********************************************/
/*    curCreateCursor()                                                      */
/*    curCreateCursorNative()                                                */
/*    curDeleteCursor()                                                      */
/*    curSetCursor()                                                         */
/*    curGetCursor()                                                         */
/*    curSetPos()                                                            */
/*    curGetPos()                                                            */
/*    curShowCursor()                                                        */
/*    curDeleteAllCursors()                                                  */
/*    SetCursorInvertColor()                                                 */
/*    curRGBtoYCrCb()                                                        */
/***** Internal Functions ****************************************************/
/*    MoveCursor()                                                           */
/*    NumCursorColors()                                                      */
/*    CreateColorCursor()                                                    */
/*    CreateBWGCursor()                                                      */
/***** Internal Debug Functions **********************************************/
/*    IsCursorHandle()                                                       */
/*****************************************************************************/

/*****************/
/* Include Files */
/*****************/

#include <string.h>
#include "stbcfg.h"
#include "kal.h"
#define INCL_DRM
#include "globals.h"
#include "osdlib.h"
#include "curlib.h"
#include "curprvc.h"

/******************************************/
/* Local Definitions and Global Variables */
/******************************************/

static POSDCURSOR   gpCursors = NULL;
static OSDCURSORPOS gposCursor = {0, 0};  // Active area cursor position
static HOSDCUR      ghCursorCurrent = NULL;

#if (DRM_CURSOR_FETCH_TYPE == DRM_CURSOR_FETCH_TYPE_COLORADO)
/* On Colorado and Wabash chips, the DRM cursor logic can only access the */
/* bottom 16MB of DRAM. This causes problems since the heap from which    */
/* cursors are allocated usually occupies the top of DRAM so, on boxes    */
/* with more than 16MB of SDRAM the cursor is frequently corrupted. To    */
/* work round this, we declare a static buffer which is used to hold the  */
/* current cursor. This will appear in the code image's RW segment which  */
/* is a lot more likely to be in low memory.                              */

static u_int8 gpCursorBitmapArray[CURSOR_IMAGE_SIZE+CURSOR_ALIGNMENT-1];
static u_int8* gpCursorBitmapBuffer;

#endif /* DRM_CURSOR_FETCH_TYPE == DRM_CURSOR_FETCH_TYPE_COLORADO */

/*****************************************************************************/
/* Exported Functions:                                                       */
/*****************************************************************************/

/*****************************************************************************/
/* Function: curCreateCursor()                                               */
/*                                                                           */
/* Parameters: u_int8 *pData   - Pointer to cursor image data.               */
/*             OSDHANDLE hRgn  - Handle to related OSD region.               */
/*             u_int8 byWidth  - Width of cursor image.                      */
/*             u_int8 byHeight - Height of cursor image.                     */
/*             u_int8 byHotX   - Hot spot x coord.                           */
/*             u_int8 byHotY   - Hot spot y coord.                           */
/*                                                                           */
/* Returns: HOSDCUR - A handle to the cursor on success or NULL on failure.  */
/*                                                                           */
/* Description: Returns a handle to an allocated cursor structure.  The      */
/*              format for the cursor image data is expected to be in the    */
/*              same format as the related OSD region.  The palette and the  */
/*              pixel format are taken from the related region.              */
/*****************************************************************************/
HOSDCUR curCreateCursor(u_int8 *pData, 
                        OSDHANDLE hRgn,
                        u_int8 byWidth, 
                        u_int8 byHeight, 
                        u_int32 byStride,
                        u_int8 byHotX,
                        u_int8 byHotY)
{
   POSDCURSOR pCursor = NULL;
   u_int32 *pAllocMem;

   if (hRgn != (OSDHANDLE)NULL)
   {
      if ((byWidth <= MAX_CURSOR_WIDTH) && (byHeight <= MAX_CURSOR_HEIGHT) &&
         ((pAllocMem = (u_int32 *)mem_nc_malloc(sizeof(OSDCURSOR) + CURSOR_ALIGNMENT-1))!=0))
      {
         /* cursor needs to be aligned to 16 bytes so page breaks occurs only at line boundary */
         pCursor = (POSDCURSOR)(((u_int32)pAllocMem + (CURSOR_ALIGNMENT-1)) & ~(CURSOR_ALIGNMENT-1));;
         pCursor->dwUnAlignedPtr = (u_int32)pAllocMem;
         pCursor->byHotX = byHotX;
         pCursor->byHotY = byHotY;
         pCursor->byW = byWidth;
         pCursor->byH = byHeight;
         pCursor->bInvertEnabled = FALSE;
         pCursor->byInvertIndex = (u_int8)0;

         if (NumCursorColors(hRgn, pData, byWidth, byHeight, byStride) <= 
            NUM_HW_CURSOR_COLORS)
         {
            /* There are few enough color used to allow for a hw cursor */
            CreateColorCursor(hRgn, pData, pCursor, byStride);
         }
         else
         {  /* There are too many colors for a color cursor.              */
            /* We will approximate by doing a black white and gray cursor */
            CreateBWGCursor(hRgn, pData, pCursor, byStride);
         }
 
         /* Add the cursor to the linked list. */
         pCursor->pNext = gpCursors;
         pCursor->pPrev = NULL;
         gpCursors->pPrev = pCursor;
         gpCursors        = pCursor;
      }
   }

   return (HOSDCUR)pCursor;
}

/*****************************************************************************/
/* Function: curCreateCursorNative()                                         */
/*                                                                           */
/* Parameters: pData    - Pointer to cursor image data.                      */
/*             pPalette - Pointer to array of 3 palette structs describing   */
/*                        colors for pixel values 01b, 10b and 11b.          */
/*             byHeight - Height of cursor image.                            */
/*             byHotX   - Hot spot x coord.                                  */
/*             byHotY   - Hot spot y coord.                                  */
/*             bInvertEnabled - TRUE if one cursor color acts as pixel invert*/
/*                              FALSE if all 3 colors are used normally      */
/*             byInvertIndex  - Cursor palette index for invert colour       */
/*                                                                           */
/* Returns: HOSDCUR - A handle to the cursor on success or NULL on failure.  */
/*                                                                           */
/* Description: Returns a handle to an allocated cursor structure.  The      */
/*              format for the cursor image data is expected to be in the    */
/*              native hardware format (2bpp, leftmost in lsb). The supplied */
/*              image data must be 64 pixels (4 words) wide. If less than    */
/*              64 lines high, undefined lines will be filled with           */
/*              transparency.                                                */
/*****************************************************************************/
HOSDCUR curCreateCursorNative(u_int8 *pData, 
                              LPREG pPal,
                              u_int8 byHeight, 
                              u_int8 byHotX,
                              u_int8 byHotY,
                              bool   bInvertEnabled,
                              u_int8 byInvertIndex)
{
   POSDCURSOR pCursor = NULL;
   int        iLineLoop;
   int        iByteLoop;
   int        iOutputIndex = 0;
   u_int32    *pAllocMem;
   
   if (pData && pPal)
   {
     if ((byHeight <= MAX_CURSOR_HEIGHT) &&
        ((pAllocMem = (u_int32 *)mem_nc_malloc(sizeof(OSDCURSOR) + CURSOR_ALIGNMENT-1))!=0))
     {
        /* cursor needs to be aligned to 16 bytes so page breaks occurs only at line boundary */
        pCursor = (POSDCURSOR)(((u_int32)pAllocMem + (CURSOR_ALIGNMENT-1)) & ~(CURSOR_ALIGNMENT-1));;
        pCursor->dwUnAlignedPtr = (u_int32)pAllocMem;
        /* Copy cursor dimensions and hotspot details */
        pCursor->byHotX = byHotX;
        pCursor->byHotY = byHotY;
        pCursor->byW = MAX_CURSOR_WIDTH;
        pCursor->byH = byHeight;
        pCursor->bInvertEnabled = bInvertEnabled;
        pCursor->byInvertIndex  = byInvertIndex;
        
        /* Copy the cursor palette */
        pCursor->yuvCursorPal[0] = pPal[0];
        pCursor->yuvCursorPal[1] = pPal[1];
        pCursor->yuvCursorPal[2] = pPal[2];

        /* Copy the cursor pixels, padding undefined lines with transparency */
        for (iLineLoop = 0; iLineLoop < MAX_CURSOR_HEIGHT; iLineLoop++)
        {
          for (iByteLoop = 0; iByteLoop < CURSOR_BYTES_PER_LINE; iByteLoop++)
          {
            if (iLineLoop < byHeight)
            {
              pCursor->byImage[iOutputIndex] = pData[iOutputIndex];
            }
            else
            {
              pCursor->byImage[iOutputIndex] = 0;
            }  
            iOutputIndex++;
          }  
        }
      
        /* Add the cursor to the linked list */
        pCursor->pNext = gpCursors;
        pCursor->pPrev = NULL;
        gpCursors->pPrev = pCursor;
        gpCursors        = pCursor;
     }
   }
   return (HOSDCUR)pCursor;
}

/*****************************************************************************/
/* Function: curDeleteCursor()                                               */
/*                                                                           */
/* Parameters: HOSDCUR hCur - Handle of a cursor to be deleted.              */
/*                                                                           */
/* Returns: bool - TRUE on success.                                          */
/*                                                                           */
/* Description: Deletes the cursor and frees the memory.                     */
/*****************************************************************************/
bool curDeleteCursor(HOSDCUR hCur)
{
   bool bRet = FALSE;
   POSDCURSOR pCursor;

   DEBUG_VERIFY_CURSOR(hCur, bRet);

   pCursor = (POSDCURSOR)hCur;
   
   if (pCursor != 0)
   {
      if (hCur == ghCursorCurrent)
      {  /* Delete and disable the current cursor.              */
         /* This probably shouldn't happen, but you never know! */
         CNXT_SET_VAL(glpDrmCursorControl, DRM_CURSOR_CONTROL_CURSOR_ENABLE_MASK, 0);
         CNXT_SET_VAL(glpDrmCursorStoreAddr, DRM_CURSOR_ADDRESS_ADDRESS_MASK, NULL);
         ghCursorCurrent = (HOSDCUR)NULL;
      }

      /* Update the cursor linked list */
      if (pCursor->pNext)
         pCursor->pNext->pPrev = pCursor->pPrev;
      if (pCursor->pPrev)
         pCursor->pPrev->pNext = pCursor->pNext;
      else
         gpCursors = pCursor->pNext;

      /*  Free the cursor data */
      mem_nc_free((void *)pCursor->dwUnAlignedPtr);

      bRet = TRUE;
   }

   return bRet;
}

/*****************************************************************************/
/* Function: curSetCursor()                                                  */
/*                                                                           */
/* Parameters: HOSDCUR hCur - The handle of the cursor to become active.     */
/*                                                                           */
/* Returns: HOSDCUR - Handle to the previously active cursor.                */
/*                                                                           */
/* Description: Installs the specified cursor as the current cursor.  This   */
/*              does not change the current show state.                      */
/*****************************************************************************/
HOSDCUR curSetCursor(HOSDCUR hCur)
{
   HOSDCUR hPrev = NULL;
   POSDCURSOR pCursor;

   DEBUG_VERIFY_CURSOR(hCur, hPrev);

   hPrev = ghCursorCurrent;

   /* Set the new cursor */
   pCursor = (POSDCURSOR)hCur;
   if (pCursor != 0)
   {
      *(u_int32 *)glpDrmCursorPal0 = *(u_int32 *)&pCursor->yuvCursorPal[0];
      *(u_int32 *)glpDrmCursorPal1 = *(u_int32 *)&pCursor->yuvCursorPal[1];
      *(u_int32 *)glpDrmCursorPal2 = *(u_int32 *)&pCursor->yuvCursorPal[2];
      #if (DRM_CURSOR_FETCH_TYPE == DRM_CURSOR_FETCH_TYPE_COLORADO)
      /* We need to copy the cursor bitmap to our static buffer to make sure that it */
      /* is in low memory.                                                           */
      /* gpCursorBitmapBuffer needs to be aligned to 16 bytes */
      gpCursorBitmapBuffer = (u_int8*)(((u_int32)gpCursorBitmapArray+(CURSOR_ALIGNMENT-1)) & ~(CURSOR_ALIGNMENT-1)); 
      memcpy(gpCursorBitmapBuffer, pCursor->byImage, CURSOR_IMAGE_SIZE);
      CNXT_SET_VAL(glpDrmCursorStoreAddr, DRM_CURSOR_ADDRESS_ADDRESS_MASK, (u_int32)gpCursorBitmapBuffer );
      CNXT_SET_VAL(glpDrmCursorStoreAddr, DRM_CURSOR_ADDRESS_FETCH_SIZE_MASK, pCursor->byH); 
      #else
      CNXT_SET_VAL(glpDrmCursorStoreAddr, DRM_CURSOR_ADDRESS_ADDRESS_MASK, (u_int32)pCursor);
      CNXT_SET_VAL(DRM_CURSOR_FETCH_SIZE_REG, DRM_CURSOR_FETCH_SIZE_LINES_MASK, pCursor->byH); 
      #endif

      /* Set the invert pixel index if this cursor needs it */
      CNXT_SET_VAL(glpDrmCursorControl, DRM_CURSOR_CONTROL_INVERT_MASK, pCursor->bInvertEnabled ? (pCursor->byInvertIndex+1) : 0);
      
      ghCursorCurrent = (HOSDCUR)pCursor;

      MoveCursor(gposCursor.x, gposCursor.y);
   }

   
   /* Return the previous cursor handle. */
   return hPrev;
}

/*****************************************************************************/
/* Function: curGetCursor()                                                  */
/*                                                                           */
/* Parameters: void                                                          */
/*                                                                           */
/* Returns: HOSDCUR - The handle of the current cursor on suceess.           */
/*                    NULL on error.                                         */
/*                                                                           */
/* Description: Returns a handle to the current cursor.                      */
/*****************************************************************************/
HOSDCUR curGetCursor(void)
{
   return (ghCursorCurrent);
}

/*****************************************************************************/
/* Function: curSetPos()                                                     */
/*                                                                           */
/* Parameters: POSDCURSORPOS pPos - Pointer to a OSDCURSORPOS structure.     */
/*                                                                           */
/* Returns: bool - TRUE on success.                                          */
/*                                                                           */
/* Description: Changes the location of the current cursor.                  */
/*****************************************************************************/
bool curSetPos(POSDCURSORPOS pPos)
{
   bool bRet = TRUE;

   gposCursor.x = pPos->x;
   gposCursor.y = pPos->y;

   MoveCursor(gposCursor.x, gposCursor.y);

   return bRet;
}

/*****************************************************************************/
/* Function: curGetPos()                                                     */
/*                                                                           */
/* Parameters: POSDCURSORPOS pPos - Pointer to OSDCURSORPOS to receive       */
/*             cursor coord.                                                 */
/*                                                                           */
/* Returns: bool - TRUE on success.                                          */
/*                                                                           */
/* Description: Returns the current coordinates of the OSD cursor.           */
/*****************************************************************************/
bool curGetPos(POSDCURSORPOS pPos)
{
   bool bRet = FALSE;

   if (pPos)
   {
      pPos->x = gposCursor.x;
      pPos->y = gposCursor.y;

      bRet = TRUE;
   }

   return bRet;
}

/*****************************************************************************/
/* Function: curShowCursor()                                                 */
/*                                                                           */
/* Parameters: HOSDCUR hCur - Handle of cursor to show.                      */
/*             bool bShow   - Show/hide flag.                                */
/*                                                                           */
/* Returns: bool - TRUE on success.                                          */
/*                                                                           */
/* Description: Shows or hides the cursor.  If the cursor to be shown is not */
/*              the active cursor, then the cursor to be shown becomes the   */
/*              the active cursor.  Hiding a cursor that is not visible does */
/*              not generate an error.                                       */
/*****************************************************************************/
bool curShowCursor(HOSDCUR hCur, bool bShow)
{
   bool bRet = FALSE;

   if (hCur)
   {
      if (bShow)
      {
         /* If the cursor to show is not the current, then make it the current. */
         if (hCur != curGetCursor())
         {
            curSetCursor(hCur);
         }

         /* Enable the cursor. */
         if (curGetCursor())
         {
            CNXT_SET_VAL(glpDrmCursorControl, 
                DRM_CURSOR_CONTROL_CURSOR_ENABLE_MASK, 1);
            bRet = TRUE;
         }
      }
      else
      {
         /* Disable the cursor only if it is the current cursor. */
         if (hCur == curGetCursor())
         {
            CNXT_SET_VAL(glpDrmCursorControl, 
                DRM_CURSOR_CONTROL_CURSOR_ENABLE_MASK, 0);
         }

         /*  It is not an error to disable a disabled cursor.  Return success. */
         bRet = TRUE;
      }
   }

   return bRet;
}

/*****************************************************************************/
/* Function: curDeleteAllCursors()                                           */
/*                                                                           */
/* Parameters: void                                                          */
/*                                                                           */
/* Returns: void                                                             */
/*                                                                           */
/* Description: Frees all memory associated with the cursor list.            */
/*****************************************************************************/
void curDeleteAllCursors(void)
{
   POSDCURSOR pCursor = gpCursors;
   POSDCURSOR pDelete;

   curShowCursor(curGetCursor(), FALSE);
   while (pCursor)
   {
      pDelete = pCursor;
      pCursor = pCursor->pNext;
      mem_nc_free((void *)pDelete->dwUnAlignedPtr);
   }
   CNXT_SET_VAL(glpDrmCursorStoreAddr, DRM_CURSOR_ADDRESS_ADDRESS_MASK, NULL);
   ghCursorCurrent = NULL;
   gpCursors = NULL;
}

/*****************************************************************************/
/* Function: SetCursorInvertColor                                            */
/*                                                                           */
/* Parameters: DRM_COLOR rgbColor - Color whose palette index is to be used  */
/*                                  as the cursor invert color               */
/*             bool bInvert       - TRUE - set cursor to invert pixels       */
/*                                  FALSE - disable cursor invert index      */
/*                                                                           */
/* Returns: Cursor pixel index found to contain color rgbColor. O if no      */
/*          match is found.                                                  */
/*                                                                           */
/* Description: Enables or disables cursor inversion. The rbgColor passed is */
/*              used to find a match in the current cursor palette and, if   */
/*              bInvert is TRUE, this index is set as the cursor invert      */
/*              index.                                                       */
/*****************************************************************************/
u_int32 SetCursorInvertColor(DRMPAL rgbColor, bool bInvert)
{
  int i;
  DRMPAL yuvToFind;
  LPREG lpPal;
  HW_DWORD   tmpPAL;
  HOSDCUR hCur;
  POSDCURSOR pCursor;


  CNXT_SET_VAL(&(yuvToFind.rgb), DRMRGBPAL_R_MASK, CNXT_GET_VAL(&(rgbColor.rgb), DRMRGBPAL_R_MASK));
  CNXT_SET_VAL(&(yuvToFind.rgb), DRMRGBPAL_G_MASK, CNXT_GET_VAL(&(rgbColor.rgb), DRMRGBPAL_G_MASK));
  CNXT_SET_VAL(&(yuvToFind.rgb), DRMRGBPAL_B_MASK, CNXT_GET_VAL(&(rgbColor.rgb), DRMRGBPAL_B_MASK));
  
  /* Convert the RGB color passed to YUV as used in the cursor palette */
  curRGBtoYCrCb(&yuvToFind);

  /* Look through existing cursor palette to find a match */
  lpPal = (LPREG)glpDrmCursorPal0;
  
  for (i=0; i<3; i++)
  {
    tmpPAL = *lpPal;
    if((CNXT_GET_VAL(&tmpPAL, DRMYCCPAL_Y_MASK) == CNXT_GET_VAL(&(yuvToFind.ycc), DRMYCCPAL_Y_MASK) ) &&
       (CNXT_GET_VAL(&tmpPAL, DRMYCCPAL_CR_MASK) == CNXT_GET_VAL(&(yuvToFind.ycc), DRMYCCPAL_CR_MASK)) &&
       (CNXT_GET_VAL(&tmpPAL, DRMYCCPAL_CB_MASK) == CNXT_GET_VAL(&(yuvToFind.ycc), DRMYCCPAL_CB_MASK)))
       break;
    lpPal++;   
  }
  
  /* If we fell out the end of the loop, we didn't find the correct value */
  if (i == 3)
     return(0);
  
  /* Set this information in the cursor object so that it is maintained */
  /* if the cursor is changed then changed back later                   */
  hCur = curGetCursor();
  
  if (hCur)
  {
    pCursor = (POSDCURSOR)hCur;
    pCursor->bInvertEnabled = bInvert;
    pCursor->byInvertIndex = i;
  }
  
  /* Otherwise, set the cursor to invert pixels of the index found */
  CNXT_SET_VAL(glpDrmCursorControl, DRM_CURSOR_CONTROL_INVERT_MASK, 
      bInvert ? (i+1) : 0);
    
  return(i+1);
}

/*****************************************************************************/
/* Internal Functions:                                                       */
/*****************************************************************************/
/*****************************************************************************/
/* Function: MoveCursor()                                                    */
/*                                                                           */
/* Parameters: short x - New cursor x position                               */
/*             short y - New cursor y position                               */
/*                                                                           */
/* Returns: void                                                             */
/*                                                                           */
/* Description: Moves the cursor to the new position in active screen coords.*/
/*****************************************************************************/
void MoveCursor(short x, short y)
{
   POSDCURSOR pCursor;
   short StartX = x;
   short StartY = y;

   if (ghCursorCurrent)
   {
      pCursor = (POSDCURSOR)ghCursorCurrent;
      StartX -= (short)pCursor->byHotX;
      StartY -= (short)pCursor->byHotY;
   }

   CNXT_SET_VAL(glpDrmCursorControl, DRM_CURSOR_CONTROL_X_MASK, (StartX*2) 
       + CNXT_GET_VAL(glpDrmScreenStart, DRM_SCREEN_START_FIRST_ACTIVE_PIXEL_MASK));
   CNXT_SET_VAL(glpDrmCursorControl, DRM_CURSOR_CONTROL_Y_MASK, (StartY/2) 
       + CNXT_GET_VAL(glpDrmScreenStart, DRM_SCREEN_START_FIELD1_ACTIVE_LINE_MASK));
   CNXT_SET_VAL(glpDrmCursorControl, DRM_CURSOR_CONTROL_FIELD_START_MASK, StartY % 2);
}

/*****************************************************************************/
/* Function: NumCursorColors()                                               */
/*                                                                           */
/* Parameters: OSDHANDLE hRgn - Handle to related OSD region.                */
/*             u_int8 *pData  - Pointer to cursor image data.                */
/*             u_int8 width   - Width of the cursor data.                    */
/*             u_int8 height  - Height of the cursor data.                   */
/*                                                                           */
/* Returns: u_int32 - The number of non-transparent colors used.             */
/*                                                                           */
/* Description: Parses the cursor data and calculates the number of non-     */
/*              transparent colors in the image.                             */
/*****************************************************************************/
u_int32 NumCursorColors(OSDHANDLE hRgn, 
                        u_int8 *pData, 
                        u_int8 width, 
                        u_int8 height,
                        u_int32 stride)
{
   u_int8  *pPixel;
   u_int32 nBpp;
   u_int32 x, y;
   u_int32 dwPixelMask;
   u_int32 dwPixelBank[8] = {0, 0, 0, 0, 0, 0, 0, 0};
   u_int32 dwPixelBankIndex;
   u_int32 dwColorCount = 0;
   u_int32 dwPixel;
   u_int32 dwPixelFlag;

   nBpp = GetOSDRgnBpp(hRgn);

   switch (nBpp)
   {
   case 4:
      dwPixelMask = 0xF;
      break;
   case 8:
      dwPixelMask = 0xFF;
      break;
   default:
      return 0xFFFFFFFF;
   }

   for (y=0; y<height; y++)
   {
      pPixel = pData + (y * stride);
      for (x=0; x<width; x++)
      {
         dwPixel = (u_int32)*(pPixel + (x * nBpp / 8));
         if ((nBpp == 4) && (!(x & 1)))
            dwPixel >>= 4;
         dwPixel &= dwPixelMask;
         dwPixelBankIndex = (dwPixel >> 5) & 7;
         dwPixelFlag = 1 << (dwPixel & 0x1F);
         if ((dwPixel) && (!(dwPixelBank[dwPixelBankIndex] & dwPixelFlag)))
         {
            dwColorCount ++;
            dwPixelBank[dwPixelBankIndex] |= dwPixelFlag;
         }
      }
   }

   return dwColorCount;
}

/*****************************************************************************/
/* Function: CreateColorCursor()                                             */
/*                                                                           */
/* Parameters: OSDHANDLE hRgn     - Handle to related OSD region.            */
/*             u_int8 *pData      - Pointer to original cursor data          */
/*             POSDCURSOR pCursor - Pointer to OSD Cursor structure to hold  */
/*                                  the 2bpp cursor image                    */
/*                                                                           */
/* Returns: void                                                             */
/*                                                                           */
/* Description: Creates a color OSD cursor from a 4 or 8 bpp OpenTv cursor.  */
/*****************************************************************************/
void CreateColorCursor(OSDHANDLE hRgn, u_int8 *pData, POSDCURSOR pCursor, u_int32 dwStride)
{
   u_int32 *pDst;
   u_int8 *pSrc;
   u_int32 dwOSDPixel;
   u_int32 dwPalIndex;
   u_int32 dwBpp;
   u_int32 x, y, i;
   DRMPAL rgbCursorPal[3];
   DRMPAL rgbPixel;
   u_int32 dwDstIndex;
   u_int32 dwPixelType;

   dwBpp = GetOSDRgnBpp(hRgn);
   dwPixelType = GetOSDRgnOptions(hRgn, OSD_RO_MODE);

   if ((pData) && (pCursor) && (pCursor->byW) && (pCursor->byH) && (dwBpp))
   {
      pDst = (u_int32 *)SET_FLAGS(pCursor, NCR_BASE); 
      pSrc = pData;
      /* Zero out the cursor */
      for (y=0; y<NUM_CURSOR_WORDS; y++)
         pDst[y] = 0x00000000;

      for (i=0; i<3; i++)
      {
         CNXT_SET_VAL(&(rgbCursorPal[i].rgb), DRMRGBPAL_ALPHA_MASK, 0x0A);
         CNXT_SET_VAL(&(rgbCursorPal[i].rgb), DRMRGBPAL_R_MASK, 0);
         CNXT_SET_VAL(&(rgbCursorPal[i].rgb), DRMRGBPAL_G_MASK, 0);
         CNXT_SET_VAL(&(rgbCursorPal[i].rgb), DRMRGBPAL_B_MASK, 0);
      }
      
      for (y=0; y<pCursor->byH; y++)
      {
         for (x=0; x<pCursor->byW; x++)
         {
            switch(dwBpp)
            {
            case 4:
               dwPalIndex = (u_int32)*(pSrc + (x/2));
               if (x % 1)
                  dwPalIndex >>= 4;
               dwPalIndex &= 0x0000000F;
               break;
            case 8:
               dwPalIndex = (u_int32)*(pSrc + x);
               dwPalIndex &= 0x000000FF;
               break;
            case 16:
               dwPalIndex = (u_int32)*(pSrc + (x * 2) + 1);
               dwPalIndex <<= 8;
               dwPalIndex |= ((u_int32)*(pSrc + (x * 2))) & 0x000000FF;
               dwPalIndex &= 0x0000FFFF;
               break;
            default: /* Shouldn't get here use 8 bpp default */
               dwPalIndex = (u_int32)*(pSrc + x);
               dwPalIndex &= 0x000000FF;
            }

            if ((dwOSDPixel = dwPalIndex) != 0)
            {
               rgbPixel = GetOSDRgnPalEntry(hRgn, dwPalIndex);
               CNXT_SET_VAL(&(rgbPixel.rgb), DRMRGBPAL_ALPHA_MASK, 0x80);

               for (i=0; i<3; i++)
               {
                  /* If this pixel already has an entry in the cursor palette, use it. */
                  if ((CNXT_GET_VAL(&(rgbPixel.rgb), DRMRGBPAL_R_MASK) == CNXT_GET_VAL(&(rgbCursorPal[i].rgb), DRMRGBPAL_R_MASK)) &&
                     (CNXT_GET_VAL(&(rgbPixel.rgb), DRMRGBPAL_G_MASK) == CNXT_GET_VAL(&(rgbCursorPal[i].rgb), DRMRGBPAL_G_MASK)) &&
                     (CNXT_GET_VAL(&(rgbPixel.rgb), DRMRGBPAL_B_MASK) == CNXT_GET_VAL(&(rgbCursorPal[i].rgb), DRMRGBPAL_B_MASK)))
                  {
                     dwOSDPixel = i + 1;
                     i = 3;
                  }
                  else if (CNXT_GET_VAL(&(rgbCursorPal[i].rgb), DRMRGBPAL_ALPHA_MASK) == 0x0A)
                  {
                     /* If cursor palette entry is free (indicated by marker in */
                     /* the unused Alpha field), write the new RGB value to it. */
                     rgbCursorPal[i] = rgbPixel;
                     dwOSDPixel = i + 1;
                     i = 3;
                  }
               }
            }
            dwDstIndex = (y*4)+(x/16);
            pDst[dwDstIndex] |= dwOSDPixel << ((x & 0xF) << 1);
         }
         pSrc += dwStride;
      }

      /* The Image is filled at this point.  Set the cursor palette. */
      switch (dwPixelType)
      {  /* Convert RGB space to YCrCb space. */
      case OSD_MODE_4ARGB:
      case OSD_MODE_8ARGB:
      case OSD_MODE_16ARGB:
      case OSD_MODE_16RGB:
         for (i=0; i<3; i++)
            curRGBtoYCrCb(&rgbCursorPal[i]);
         break;
      }

      /* load the hardware palette.*/
      for (i=0; i<3; i++)
      {
         CNXT_SET_VAL(&(pCursor->yuvCursorPal[i]), DRM_COLOR_Y_MASK, CNXT_GET_VAL(&(rgbCursorPal[i].rgb), DRMRGBPAL_R_MASK));
         CNXT_SET_VAL(&(pCursor->yuvCursorPal[i]), DRM_COLOR_CB_MASK, CNXT_GET_VAL(&(rgbCursorPal[i].rgb), DRMRGBPAL_G_MASK));
         CNXT_SET_VAL(&(pCursor->yuvCursorPal[i]), DRM_COLOR_CR_MASK, CNXT_GET_VAL(&(rgbCursorPal[i].rgb), DRMRGBPAL_B_MASK));
      }
   }
}

/*****************************************************************************/
/* Function: CreateBWGCursor()                                               */
/*                                                                           */
/* Parameters: OSDHANDLE hRgn     - Handle to related OSD region.            */
/*             u_int8 *pData      - Pointer to original cursor data          */
/*             POSDCURSOR pCursor - Pointer to OSD Cursor structure to hold  */
/*                                  the 2bpp cursor image                    */
/*                                                                           */
/* Returns: void                                                             */
/*                                                                           */
/* Description: Creates a black, white, and gray OSD cursor from a 4 or 8 bpp*/
/*              OpenTv cursor.                                               */
/*****************************************************************************/
void CreateBWGCursor(OSDHANDLE hRgn, u_int8 *pData, POSDCURSOR pCursor, u_int32 dwStride)
{
   u_int32 *pDst;
   u_int8 *pSrc;
   u_int32 dwOSDPixel;
   u_int32 dwBpp;
   u_int32 x, y;
   DRMPAL rgbPixel;
   u_int32 dwPixelType;
   u_int32 dwLuma;
   u_int32 dwDstIndex;
   bool bTransparent;

   dwBpp = GetOSDRgnBpp(hRgn);
   dwPixelType = GetOSDRgnOptions(hRgn, OSD_RO_MODE);

   if ((pData) && (pCursor) && (pCursor->byW) && (pCursor->byH) && (dwBpp))
   {
      pDst = (u_int32 *)SET_FLAGS(pCursor, NCR_BASE); 
      pSrc = pData;
      /* Zero out the cursor */
      for (y=0; y<NUM_CURSOR_WORDS; y++)
         pDst[y] = 0x00000000;

      for (y=0; y<pCursor->byH; y++)
      {
         for (x=0; x<pCursor->byW; x++)
         {
            switch(dwBpp)
            {
            case 4:
               dwOSDPixel = (u_int32)*(pSrc + (x/2));
               if (x % 1)
                  dwOSDPixel >>= 4;
               dwOSDPixel &= 0x0000000F;
               break;
            case 8:
               dwOSDPixel = (u_int32)*(pSrc + x);
               dwOSDPixel &= 0x000000FF;
               break;
            case 16:
               dwOSDPixel = (u_int32)*(pSrc + (x * 2) + 1);
               dwOSDPixel <<= 8;
               dwOSDPixel |= ((u_int32)*(pSrc + (x * 2))) & 0x000000FF;
               dwOSDPixel &= 0x0000FFFF;
               break;
            default: /* Shouldn't get here use 8 bpp default */
               dwOSDPixel = (u_int32)*(pSrc + x);
               dwOSDPixel &= 0x000000FF;
            }
            
            bTransparent = FALSE;
            switch (dwPixelType)
            {
            case OSD_MODE_4ARGB:
            case OSD_MODE_8ARGB:
               if (!dwOSDPixel)
                  bTransparent = TRUE;
               rgbPixel = GetOSDRgnPalEntry(hRgn, dwOSDPixel);
               dwLuma = LUMA(CNXT_GET_VAL(&(rgbPixel.rgb), DRMRGBPAL_R_MASK), 
                   CNXT_GET_VAL(&(rgbPixel.rgb), DRMRGBPAL_G_MASK), 
                   CNXT_GET_VAL(&(rgbPixel.rgb), DRMRGBPAL_B_MASK));
               break;
            case OSD_MODE_4AYUV:
            case OSD_MODE_8AYUV:
               if (!dwOSDPixel)
                  bTransparent = TRUE;
               rgbPixel = GetOSDRgnPalEntry(hRgn, dwOSDPixel);
               dwLuma = CNXT_GET_VAL(&(rgbPixel.ycc), DRMYCCPAL_Y_MASK); 
               break;
            case OSD_MODE_16ARGB:
               if (!(dwOSDPixel & 0x00000FFF))
                  bTransparent = TRUE;
               CNXT_SET_VAL(&(rgbPixel.rgb), DRMRGBPAL_R_MASK, ((dwOSDPixel & 0xF)*(u_int32)255/(u_int32)15));
               CNXT_SET_VAL(&(rgbPixel.rgb), DRMRGBPAL_G_MASK, (((dwOSDPixel >> 4) & 0xF) *
                                 (u_int32)255 / (u_int32)15));
               CNXT_SET_VAL(&(rgbPixel.rgb), DRMRGBPAL_B_MASK, (((dwOSDPixel >> 8) & 0xF) *
                                 (u_int32)255 / (u_int32)15));
               dwLuma = LUMA(CNXT_GET_VAL(&(rgbPixel.rgb), DRMRGBPAL_R_MASK), 
                   CNXT_GET_VAL(&(rgbPixel.rgb), DRMRGBPAL_G_MASK), 
                   CNXT_GET_VAL(&(rgbPixel.rgb), DRMRGBPAL_B_MASK));
               break;
            case OSD_MODE_16AYUV:
               if ((dwOSDPixel & 0x00000FFF) == 0x00000881)
                  bTransparent = TRUE;
               dwLuma = (dwOSDPixel & 0xF) * 255 / 0xF;
               break;
            case OSD_MODE_16RGB:
               if (!dwOSDPixel)
                  bTransparent = TRUE;
               CNXT_SET_VAL(&(rgbPixel.rgb), DRMRGBPAL_R_MASK, ((dwOSDPixel & 0x1F)*(u_int32)255/(u_int32)31));
               CNXT_SET_VAL(&(rgbPixel.rgb), DRMRGBPAL_G_MASK, (((dwOSDPixel >> 5) & 0x3F) *
                                 (u_int32)255 / (u_int32)63));
               CNXT_SET_VAL(&(rgbPixel.rgb), DRMRGBPAL_B_MASK, (((dwOSDPixel >> 11) & 0x1F) *
                                 (u_int32)255 / (u_int32)31));
               dwLuma = LUMA(
                   CNXT_GET_VAL(&(rgbPixel.rgb), DRMRGBPAL_R_MASK), 
                   CNXT_GET_VAL(&(rgbPixel.rgb), DRMRGBPAL_G_MASK), 
                   CNXT_GET_VAL(&(rgbPixel.rgb), DRMRGBPAL_B_MASK));
               break;
            case OSD_MODE_16YUV655:
               if (dwOSDPixel == 0x00008404)
                  bTransparent = TRUE;
               dwLuma = (dwOSDPixel & 0x3F) * 255 / 0x3F;
               break;
            default:
               dwLuma = 0;
               break;
            }

            if (!bTransparent)
            {
               if (dwLuma < GRAY_THRESHOLD)
                  dwOSDPixel = CURSOR_BLACK; /* Black is less than gray */
               else if (dwLuma < WHITE_THRESHOLD)
                  dwOSDPixel = CURSOR_GRAY; /* Gray is less than white */
               else
                  dwOSDPixel = CURSOR_WHITE; /* Not black or gray, must be white */

               dwDstIndex = (y*4)+(x/16);
               pDst[dwDstIndex] |= dwOSDPixel << ((x & 0xF) << 1);
            }
         }
         pSrc += dwStride;
      }
      /* The Image is filled at this point.  Set the cursor palette. */
      CNXT_SET_VAL(&(pCursor->yuvCursorPal[CURSOR_BLACK-1]), DRM_COLOR_Y_MASK, CURSOR_BLACK_Y);
      CNXT_SET_VAL(&(pCursor->yuvCursorPal[CURSOR_BLACK-1]), DRM_COLOR_CB_MASK, CURSOR_BLACK_CB);
      CNXT_SET_VAL(&(pCursor->yuvCursorPal[CURSOR_BLACK-1]), DRM_COLOR_CR_MASK, CURSOR_BLACK_CR);

      CNXT_SET_VAL(&(pCursor->yuvCursorPal[CURSOR_GRAY-1]), DRM_COLOR_Y_MASK, CURSOR_GRAY_Y);
      CNXT_SET_VAL(&(pCursor->yuvCursorPal[CURSOR_GRAY-1]), DRM_COLOR_CB_MASK, CURSOR_GRAY_CB);
      CNXT_SET_VAL(&(pCursor->yuvCursorPal[CURSOR_GRAY-1]), DRM_COLOR_CR_MASK, CURSOR_GRAY_CR);

      CNXT_SET_VAL(&(pCursor->yuvCursorPal[CURSOR_WHITE-1]), DRM_COLOR_Y_MASK, CURSOR_WHITE_Y);
      CNXT_SET_VAL(&(pCursor->yuvCursorPal[CURSOR_WHITE-1]), DRM_COLOR_CB_MASK, CURSOR_WHITE_CB);
      CNXT_SET_VAL(&(pCursor->yuvCursorPal[CURSOR_WHITE-1]), DRM_COLOR_CR_MASK, CURSOR_WHITE_CR);
   }
}

/*****************************************************************************/
/* Function: curRGBtoYCrCb()                                                 */
/*                                                                           */
/* Parameters: DRMPAL *pEntry - Pointer to RGB data to be converted.         */
/*                                                                           */
/* Returns: void                                                             */
/*                                                                           */
/* Description: Converts 24 bit RGB to 24 bit YCrCb.                         */
/*****************************************************************************/
void curRGBtoYCrCb(DRMPAL *pEntry)
{
   int r, g, b;
   int Y, Cr, Cb;

   r = CNXT_GET_VAL(&(pEntry->rgb), DRMRGBPAL_R_MASK);
   b = CNXT_GET_VAL(&(pEntry->rgb), DRMRGBPAL_B_MASK);
   g = CNXT_GET_VAL(&(pEntry->rgb), DRMRGBPAL_G_MASK);

   Y  = ((( 66 * r) + (129 * g)) + ((25 * b) + 4224)) >> 8;
   Cr = (((112 * r) + 32896) - ((94 * g) + (18 * b))) >> 8;
   Cb = (((112 * b) + 32896) - ((38 * r) + (74 * g))) >> 8;

   Y = min(max(16, Y), 235);
   Cr = min(max(16, Cr), 240);
   Cb = min(max(16, Cb), 240);

   CNXT_SET_VAL(&(pEntry->ycc), DRMYCCPAL_Y_MASK, (u_int8)Y);
   CNXT_SET_VAL(&(pEntry->ycc), DRMYCCPAL_CB_MASK, (u_int8)Cb);
   CNXT_SET_VAL(&(pEntry->ycc), DRMYCCPAL_CR_MASK, (u_int8)Cr);
}

/*****************************************************************************/
/* Internal Debug Functions:                                                 */
/*****************************************************************************/
/*****************************************************************************/
/* Function: IsCursorHandle()                                                */
/*                                                                           */
/* Parameters: HOSDCUR hCursor - OSD Cursor Handle.                          */
/*                                                                           */
/* Returns: bool - TRUE if hCursor is in the linked list of cursors.         */
/*                                                                           */
/* Description: Debug function to verify a valid cursor handle.              */
/*****************************************************************************/
#ifdef DEBUG
bool IsCursorHandle(HOSDCUR hCursor)
{
   bool bRet = FALSE;
   POSDCURSOR pCursor = gpCursors;

   while (pCursor && !bRet)
   {
      if ((HOSDCUR)pCursor == hCursor)
         bRet = TRUE;

      pCursor = pCursor->pNext;
   }

   if (!bRet)
      trace_new(OSD_ERROR_MSG, "CURSOR: Invalid cursor handle");

   return bRet;
}
#endif /* DEBUG */

/****************************************************************************
 * Modifications:
 * $Log: 
 *  24   mpeg      1.23        11/24/03 3:55:55 PM    Dave Wilson     CR(s): 
 *        7512 8015 Fixed a problem in curSetCursor which caused the position 
 *        to be set incorrectly in cases where the new cursor had a different 
 *        hotspot from the current one.
 *        
 *  23   mpeg      1.22        3/26/03 4:27:44 PM     Lucy C Allevato SCR(s) 
 *        5887 :
 *        add stride in the curCreateCursor input parameters 
 *        
 *  22   mpeg      1.21        3/26/03 4:11:02 PM     Lucy C Allevato SCR(s) 
 *        5865 :
 *        correct a mistake made in last version: accidently delete the line to
 *         copy cursor data to gpCursorBitmapBuffer 
 *        
 *  21   mpeg      1.20        3/26/03 2:57:48 PM     Lucy C Allevato SCR(s) 
 *        5865 :
 *        align cursor to 16 bytes so page breaks will occur only on line 
 *        boundary instead of pixel boundary
 *        
 *  20   mpeg      1.19        3/18/03 3:57:36 PM     Lucy C Allevato SCR(s) 
 *        5806 :
 *        CNXT_SET_VAL in curRGBtoYCrCb should take a mask in the second 
 *        parameter instead of a value 
 *        
 *  19   mpeg      1.18        2/19/03 1:23:40 PM     Dave Wilson     SCR(s) 
 *        5534 :
 *        Oops - last edit didn't finish the job. The previous version read the
 *         cursor
 *        bitmap address from the hardware register and used this to derive the
 *         current
 *        cursor handle. Now that I am using a shadow copy of the bitmap in 
 *        Colorado and
 *        Wabash cases, this was resulting in bogus handles being passed back 
 *        on calls
 *        to curGetCursor and causing curSetCursor to fail when it should not.
 *        
 *  18   mpeg      1.17        2/19/03 10:54:56 AM    Dave Wilson     SCR(s) 
 *        5534 :
 *        Fixed a long standing problem on Wabash and Colorado. The cursor must
 *         be in
 *        the bottom 16MB of RAM but, on 32MB IRDs, normally it was above this 
 *        boundary
 *        and displayed as a corrupted block. To get round this, I now keep a 
 *        static
 *        buffer in the image data segment (rather than on the heap) and copy 
 *        the current
 *        cursor bitmap into this rather than using it directly from the cursor
 *         structure.
 *        
 *  17   mpeg      1.16        2/14/03 3:04:00 PM     Dave Wilson     SCR(s) 
 *        5513 :
 *        Fixed up curSetCursor to do the right thing for Brazos. The cursor 
 *        fetch
 *        size moved from the address register to its own register and this had
 *         not been
 *        reflected in the code. As a result, cursors were always very much 
 *        "higher" than
 *        they should have been and the lower portions were filled with 
 *        garbage.
 *        
 *  16   mpeg      1.15        2/13/03 11:31:00 AM    Matt Korte      SCR(s) 
 *        5479 :
 *        Removed old header reference
 *        
 *  15   mpeg      1.14        9/25/02 9:59:20 PM     Carroll Vance   SCR(s) 
 *        3786 :
 *        Removing old DRM and AUD conditional bitfield code.
 *        
 *        
 *  14   mpeg      1.13        9/6/02 4:21:52 PM      Matt Korte      SCR(s) 
 *        4498 :
 *        Fixed Warnings
 *        
 *  13   mpeg      1.12        6/14/02 12:39:14 PM    Dave Wilson     SCR(s) 
 *        4012 :
 *        Fixed a problem in CreateColorCursor which had been introduced as a 
 *        result
 *        of the bitfields removal. The level of indirection of a value passed 
 *        as a 
 *        pointer to CNXT_SET_VAL was wrong.
 *        
 *  12   mpeg      1.11        5/28/02 6:11:18 PM     Carroll Vance   SCR(s) 
 *        3786 :
 *        Removing DRM bitfields.
 *        
 *  11   mpeg      1.10        5/17/02 11:25:04 AM    Carroll Vance   SCR(s) 
 *        3786 :
 *        Removed DRM bitfields.
 *        
 *  10   mpeg      1.9         4/5/02 11:47:24 AM     Tim White       SCR(s) 
 *        3510 :
 *        Backout DCS #3468
 *        
 *        
 *  9    mpeg      1.8         3/28/02 2:43:48 PM     Quillian Rutherford 
 *        SCR(s) 3468 :
 *        Removed bit fields
 *        
 *        
 *  8    mpeg      1.7         1/7/02 2:18:44 PM      Dave Wilson     SCR(s) 
 *        3004 :
 *        The invert index and whether or not invert is enabled are now pieces 
 *        of 
 *        information that are tagged to the cursor object and not global 
 *        settings. The
 *        code sets the correct invert information when you enable a cursor 
 *        now. Changes
 *        made using a call to SetCursorInvertColor are now tagged to the 
 *        cursor that is
 *        currently enabled.
 *        
 *  7    mpeg      1.6         1/4/02 4:27:22 PM      Dave Wilson     SCR(s) 
 *        2920 :
 *        Added new API curCreateCursorNative to allow creation of cursors in 
 *        the 
 *        hardware's native 2bpp format.
 *        
 *  6    mpeg      1.5         8/28/00 6:12:16 PM     Lucy C Allevato Fixes for
 *         strict level compiler checking for possible usage of variables
 *        before they are initialized.
 *        
 *  5    mpeg      1.4         6/7/00 6:15:00 PM      Lucy C Allevato Fixed 
 *        comment updates that were missed in the previous change.
 *        
 *  4    mpeg      1.3         6/7/00 5:54:42 PM      Lucy C Allevato Changed 
 *        POSITION data type name references to OSDCURSORPOS references to
 *        indicate new name that is less likely to collide with other data 
 *        types.
 *        
 *  3    mpeg      1.2         4/18/00 11:53:06 AM    Dave Wilson     Changed 
 *        calls to mem_nc_malloc/free after removal of an unused parameter
 *        
 *  2    mpeg      1.1         3/29/00 8:40:24 AM     Ray Mack        changes 
 *        to delete bit fields
 *        
 *  1    mpeg      1.0         1/5/00 10:24:18 AM     Rob Tilton      
 * $:
 * 
 *****************************************************************************/

