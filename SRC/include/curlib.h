/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*           Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002         */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       curlib.h
 *
 *
 * Description:    OSD Cursor Library public header file
 *
 *
 * Author:         Rob Tilton (modified by Dave Wilson)
 *
 ****************************************************************************/
/* $Header: curlib.h, 7, 3/26/03 4:28:42 PM, Lucy C Allevato$
 ****************************************************************************/

#ifndef _CURLIB_H_
#define _CURLIB_H_

typedef struct _OSDCURSOR *POSDCURSOR;
typedef POSDCURSOR HOSDCUR;

typedef struct _OSDCURSORPOS {
   short      x;
   short      y;
} OSDCURSORPOS, *POSDCURSORPOS;


/*****************************************************************************/
/* Exported Functions:                                                       */
/*****************************************************************************/
HOSDCUR curCreateCursor(u_int8 *pData, 
                        OSDHANDLE hRgn,
                        u_int8 byWidth, 
                        u_int8 byHeight, 
                        u_int32 byStride,
                        u_int8 byHotX,
                        u_int8 byHotY);
HOSDCUR curCreateCursorNative(u_int8 *pData, 
                              LPREG pPal,
                              u_int8 byHeight, 
                              u_int8 byHotX,
                              u_int8 byHotY,
                              bool   bInvertEnable,
                              u_int8 byInvertColor);
bool curDeleteCursor(HOSDCUR hCur);
HOSDCUR curSetCursor(HOSDCUR hCur);
HOSDCUR curGetCursor(void);
bool curSetPos(POSDCURSORPOS pPos);
bool curGetPos(POSDCURSORPOS pPos);
bool curShowCursor(HOSDCUR hCur, bool bShow);
void curDeleteAllCursors(void);
u_int32 SetCursorInvertColor(DRMPAL rgbColor, bool bInvert);

#endif /* _CURLIB_H_ */

/****************************************************************************
 * Modifications:
 * $Log: 
 *  7    mpeg      1.6         3/26/03 4:28:42 PM     Lucy C Allevato SCR(s) 
 *        5887 :
 *        add one more input parameter stride in curCreateCursor
 *        
 *  6    mpeg      1.5         9/25/02 10:18:18 PM    Carroll Vance   SCR(s) 
 *        3786 :
 *        Removing old DRM and AUD conditional bitfield code.
 *        
 *        
 *  5    mpeg      1.4         5/21/02 1:35:10 PM     Carroll Vance   SCR(s) 
 *        3786 :
 *        Removed DRM bitfields.
 *        
 *  4    mpeg      1.3         1/7/02 2:15:42 PM      Dave Wilson     SCR(s) 
 *        3004 :
 *        Added parameters to curCreateCursorNative to allow transparent pixel 
 *        index
 *        to be specified if required.
 *        
 *  3    mpeg      1.2         1/4/02 4:28:36 PM      Dave Wilson     SCR(s) 
 *        2920 :
 *        Added prototype for curCreateCursorNative.
 *        
 *  2    mpeg      1.1         6/7/00 6:01:20 PM      Lucy C Allevato Changed 
 *        POSITION type name to OSDCURSORPOS to help avoid data type name
 *        collisions.
 *        
 *  1    mpeg      1.0         11/10/99 11:26:58 AM   Rob Tilton      
 * $
 * 
 *    Rev 1.6   26 Mar 2003 16:28:42   goldenx
 * SCR(s) 5887 :
 * add one more input parameter stride in curCreateCursor
 * 
 *    Rev 1.5   25 Sep 2002 21:18:18   vancec
 * SCR(s) 3786 :
 * Removing old DRM and AUD conditional bitfield code.
 * 
 * 
 *    Rev 1.4   21 May 2002 12:35:10   vancec
 * SCR(s) 3786 :
 * Removed DRM bitfields.
 * 
 *    Rev 1.3   07 Jan 2002 14:15:42   dawilson
 * SCR(s) 3004 :
 * Added parameters to curCreateCursorNative to allow transparent pixel index
 * to be specified if required.
 * 
 *    Rev 1.2   04 Jan 2002 16:28:36   dawilson
 * SCR(s) 2920 :
 * Added prototype for curCreateCursorNative.
 *
 ****************************************************************************/

