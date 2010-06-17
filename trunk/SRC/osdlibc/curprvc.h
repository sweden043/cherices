/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*           Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002         */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/*****************************************************************************/
/* File: curprv.h                                                            */
/*                                                                           */
/* Module: OSD Cursor Library                                                */
/*                                                                           */
/* Description: OSD Cursor library private include file.                     */
/*****************************************************************************/
/*----------------------------------------------------------------------
 *$Id: curprvc.h,v 1.7, 2003-12-19 16:30:44Z, Lucy C Allevato$
 *---------------------------------------------------------------------- */
#ifndef _CURPRV_H_
#define _CURPRV_H_

#define LINES_PER_CURSOR             64
#define CURSOR_PIXELS_PER_LINE       64
#define CURSOR_BPP                   2
#define CURSOR_BYTES_PER_LINE        (CURSOR_PIXELS_PER_LINE * CURSOR_BPP / 8)
#define CURSOR_IMAGE_SIZE            (CURSOR_BYTES_PER_LINE*LINES_PER_CURSOR)
#define MAX_CURSOR_WIDTH             CURSOR_PIXELS_PER_LINE
#define MAX_CURSOR_HEIGHT            LINES_PER_CURSOR
#define NUM_HW_CURSOR_COLORS         3  /* This does not include transparency   */
#define NUM_CURSOR_WORDS             256
/* the number of bytes that alignment required for cursor structure */
#define CURSOR_ALIGNMENT             16

#define LUMA(r,g,b)                  (((257*r + 504*g + 98*b) / 1000) + 16)

#define GRAY_THRESHOLD               89
#define WHITE_THRESHOLD              162

#define CURSOR_TRANSPARENT           0  /* Cursor pal index for transparency  */
#define CURSOR_BLACK                 1  /* Cursor pal index for black         */
#define CURSOR_GRAY                  2  /* Cursor pal index for gray          */
#define CURSOR_WHITE                 3  /* Cursor pal index for white         */

/* Cursor palette settings for the black, white, and gray cursor  */
#define CURSOR_BLACK_Y               0x10
#define CURSOR_BLACK_CB              0x80
#define CURSOR_BLACK_CR              0x80

#define CURSOR_GRAY_Y                0x7E
#define CURSOR_GRAY_CB               0x80
#define CURSOR_GRAY_CR               0x80

#define CURSOR_WHITE_Y               0xEB
#define CURSOR_WHITE_CB              0x80
#define CURSOR_WHITE_CR              0x80

typedef struct _OSDCURSOR {
              /* Keep the image data first in the struct.  The ptr to the       */
              /* struct is also the ptr to the cursor data, which is also used  */
              /* in the Cursor Store Address Register, and also the cursor      */
              /* handle.                                                        */
   u_int8     byImage[CURSOR_IMAGE_SIZE]; /* This puppy must be 32-bit aligned  */
   POSDCURSOR pPrev, pNext;
   u_int8     byHotX, byHotY, byW, byH;
   HW_DWORD  yuvCursorPal[3];
   u_int32    dwUnAlignedPtr; /* Original non cache aligned ptr.                */
   bool       bInvertEnabled;
   u_int8     byInvertIndex;
} OSDCURSOR;

/*****************************************************************************/
/* Internal Functions:                                                       */
/*****************************************************************************/
void MoveCursor(short x, short y);
u_int32 NumCursorColors(OSDHANDLE hRgn, 
                        u_int8 *pData, 
                        u_int8 width, 
                        u_int8 height,
                        u_int32 stride);
void CreateColorCursor(OSDHANDLE hRgn, u_int8 *pData, POSDCURSOR pCursor, u_int32 dwStride);
void CreateBWGCursor(OSDHANDLE hRgn, u_int8 *pData, POSDCURSOR pCursor, u_int32 dwStride);
void curRGBtoYCrCb(DRMPAL *pEntry);

/* Debug Function Prototypes  */
#ifdef DEBUG
bool IsCursorHandle(HOSDCUR hCursor);
#endif /* DEBUG */

#ifdef DEBUG
#define DEBUG_VERIFY_CURSOR(hCursor,ret) if (!IsCursorHandle(hCursor)) return ret
#else /* DEBUG */
#define DEBUG_VERIFY_CURSOR(hCursor,ret) if (0)
#endif /* DEBUG */

#endif /* _CURPRV_H_ */

/*-------------------------------------------------------------------------
 *$Log: 
 * 8    mpeg      1.7         12/19/03 10:30:44 AM   Lucy C Allevato CR(s) 8154
 *        : Fixed Header -> Id and Log keywords
 * 7    mpeg      1.6         3/26/03 4:23:44 PM     Lucy C Allevato SCR(s) 
 *       5887 :
 *       add stride in the curCreateCursor input parameters 
 *       
 * 6    mpeg      1.5         3/26/03 2:56:22 PM     Lucy C Allevato SCR(s) 
 *       5865 :
 *       define the number bytes that alignment required for cursor and also 
 *       replace comments // to c-style
 *       
 * 5    mpeg      1.4         9/25/02 10:00:38 PM    Carroll Vance   SCR(s) 
 *       3786 :
 *       Removing old DRM and AUD conditional bitfield code.
 *       
 *       
 * 4    mpeg      1.3         5/28/02 6:15:40 PM     Carroll Vance   SCR(s) 
 *       3786 :
 *       Removing DRM bitfields.
 *       
 * 3    mpeg      1.2         1/7/02 2:18:52 PM      Dave Wilson     SCR(s) 
 *       3004 :
 *       The invert index and whether or not invert is enabled are now pieces 
 *       of 
 *       information that are tagged to the cursor object and not global 
 *       settings. The
 *       code sets the correct invert information when you enable a cursor now.
 *        Changes
 *       made using a call to SetCursorInvertColor are now tagged to the cursor
 *        that is
 *       currently enabled.
 *       
 * 2    mpeg      1.1         6/7/00 5:55:54 PM      Lucy C Allevato Removed 
 *       old references to cursor data types which are now located in curlib.h
 *       
 * 1    mpeg      1.0         1/5/00 10:24:52 AM     Rob Tilton      
 *$
 *---------------------------------------------------------------------- */

