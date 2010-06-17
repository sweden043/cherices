/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                   Conexant Systems Inc. (c) 1998 - 2003                  */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       vidlibc.h
 *
 *
 * Description:    Video plane scaling/positioning API
 *
 *
 * Author:         Rob Tilton, Dave Wilson and others
 *
 ****************************************************************************/
/* $Header: vidlibc.h, 16, 9/29/03 2:02:48 PM, Dave Wilson$
 ****************************************************************************/
#ifndef _VIDLIBC_H_
#define _VIDLIBC_H_

typedef u_int32 HVIDRGN;
typedef enum {
   TYPE_422,   // Live video data type.
   TYPE_420    // Mpg video data type.
} VIDRGNTYPE;

typedef enum {
   SOFTWARE,   // Software is used to fill the video buffer.
   HARDWARE    // Hardware is used to fill the video buffer.
} VIDRGNCONNECT;

typedef enum {
   VO_ASPECT_RATIO_MODE,
   VO_ASPECT_RATIO,
   VO_CONNECTION,
   VO_PIXEL_TYPE,
   VO_BUFFER_HEIGHT,
   VO_BUFFER_WIDTH,
   VO_BUFFER_PTR,
   VO_BUFFER_STRIDE,
   VO_IMAGE_HEIGHT,
   VO_IMAGE_WIDTH,
   VO_HSCALE,
   VO_VSCALE,
   VO_PAN_VECTOR,
   VO_SCAN_VECTOR,
   VO_AUTOMODE,
   VO_IMAGE_ON_TOP,
   VO_IMAGE_PAN_VECTOR,
   VO_IMAGE_SCAN_VECTOR,
   VO_IMAGE_VECTOR_PRESENT,
   VO_USER_INFO
} VIDOPTION;

#define VID_AUTO_SCALE     0x00000001
#define VID_AUTO_PANSCAN   0x00000002
#define VID_AUTO_WINDOW    0x00000004
#define VID_AUTO_ALL       0x00000007

// Exportes function prototypes.
bool    vidLibInit(void);
HVIDRGN vidCreateVideoRgn(POSDRECT pRc, VIDRGNTYPE vrType);
bool    vidDestroyVideoRgn(HVIDRGN hRgn);
bool    vidDestroyAllVideoRgn(void);
bool    vidSetOptions(HVIDRGN hRgn, VIDOPTION vOption, u_int32 dwVal, bool bUpdate);
u_int32 vidGetOptions(HVIDRGN hRgn, VIDOPTION vOption);
bool    vidSetPos(HVIDRGN hRgn, POSDRECT prcDst, POSDRECT prcSrc);
bool    vidGetPos(HVIDRGN hRgn, POSDRECT prcDst, POSDRECT prcSrc);
bool    vidDisplayRegion(HVIDRGN hRgn, bool bEnable);
bool    vidSetConnection(HVIDRGN hRgn, VIDRGNCONNECT vrConnect);
bool    vidGetConnection(HVIDRGN hRgn, VIDRGNCONNECT *pConnect);
void    vidUpdateScaling(u_int32 dwRgnIndex);
HVIDRGN vidGet420Hardware(void);
HVIDRGN vidGetActiveRegion(u_int32 dwRgnIndex);
HVIDRGN vidGetVideoRegion(void);
bool    vidSetRegionInput(HVIDRGN hRgn, u_int32 dwActIndex);
bool    vidSetActiveRegion(HVIDRGN hRgn, u_int32 dwActIndex);
HVIDRGN vidGetTopRegion(void);
HVIDRGN vidGetBottomRegion(void);
HVIDRGN vidAllocateVideoRgn(u_int32 dwSize, VIDRGNTYPE sType);
u_int32 vidQueryVideoRgnSize(u_int32 dwWidth, u_int32 dwHeight, VIDRGNTYPE sType);
bool    vidSetVideoRgnDimensions(HVIDRGN hRgn, u_int32 dwWidth, u_int32 dwHeight);
bool    vidSetAlpha(u_int32 dwRgnIndex, u_int8 byAlpha);
bool    vidGetAlpha(u_int32 dwRgnIndex, u_int8 *pbyAlpha);
bool    vidWipeEnable(u_int32 dwRgnIndex, bool bEnable);
bool    vidSetWipeRgn(u_int32 dwRgnIndex, HVIDRGN hRgn);
HVIDRGN vidGetWipeRgn(u_int32 dwRgnIndex);
bool    vidSetWipeLine(u_int32 dwRgnIndex, u_int32 dwLine);
bool    vidGetWipeLine(u_int32 dwRgnIndex, u_int32 *pdwLine);

#ifdef OPENTV_12
void   *vidImageBufferAlloc(u_int32 dwSize, bool *pbTransient, bool bPermanent);
bool    vidImageBufferFree(void *lpBuffer, bool bTransient);
bool    vidDecodeBuffersAvail(void);
bool    vidDecodeBuffersNeeded(void);
bool    vidForceDisplay(HVIDRGN hRgn, bool bSoftware);
#endif

#endif /* _VIDLIBC_H_ */

/****************************************************************************
 * Modifications:
 * $Log: 
 *  16   mpeg      1.15        9/29/03 2:02:48 PM     Dave Wilson     SCR(s) 
 *        7571 7570 :
 *        Added option IDs to allow access to the parsed MPEG still pan/scan 
 *        offset
 *        information now held in the VIDRGN structure.
 *        
 *  15   mpeg      1.14        8/21/01 1:53:22 PM     Dave Wilson     SCR(s) 
 *        2504 2505 :
 *        Added prototype for vidGetVideoRegion function.
 *        
 *  14   mpeg      1.13        7/5/01 2:05:18 PM      Dave Wilson     SCR(s) 
 *        2194 2195 :
 *        Added bool return code from vidForceDisplay to allow caller to 
 *        determine
 *        whether display was being controlled by vcore or direct DRM 
 *        programming when
 *        the call is made.
 *        
 *  13   mpeg      1.12        6/29/01 6:08:22 PM     Dave Wilson     SCR(s) 
 *        2002 1914 2003 1915 1916 :
 *        Added VO_USER_INFO. This field, intended for client use, will be 
 *        needed in
 *        the OpenTV video driver to reduce transients on unblank.
 *        
 *  12   mpeg      1.11        6/28/01 3:00:38 PM     Dave Wilson     SCR(s) 
 *        2105 2106 :
 *        New microcode correctly reports buffer which is being displayed so 
 *        reworked
 *        these files to track this properly. Now decode of P frame stills will
 *         difference
 *        with the correct buffer and they will end up looking correct.
 *        
 *  11   mpeg      1.10        6/11/01 10:12:18 AM    Dave Wilson     SCR(s) 
 *        2066 2067 :
 *        
 *        AQdded vidForceDisplay to force DRM to display a video region or 
 *        revert to hardware control. Similar to vidSetConnection without 
 *        mucking with decoder buffers.
 *        
 *  10   mpeg      1.9         5/31/01 6:47:54 PM     Dave Wilson     SCR(s) 
 *        1897 1898 1826 1827 1670 1389 1390 :
 *        Added prototype for vidGetSafeStillDecodeBuffer.
 *        
 *  9    mpeg      1.8         4/20/01 12:13:52 PM    Dave Wilson     DCS1124: 
 *        Major changes to video memory management to get Sky Text app running
 *        
 *  8    mpeg      1.7         2/12/01 2:55:46 PM     Quillian Rutherford Added
 *         vidSetRegionInput
 *        
 *  7    mpeg      1.6         11/24/00 7:41:44 PM    Lucy C Allevato Added new
 *         options to track HSCALE and VSCALE parameters for vidrgn.
 *        
 *  6    mpeg      1.5         7/12/00 11:58:50 AM    Lucy C Allevato Added 
 *        video options for pan vector, scan vector, and automode along with
 *        automode defines.
 *        
 *  5    mpeg      1.4         3/22/00 4:12:06 PM     Rob Tilton      Added 
 *        video wipe functions.
 *        
 *  4    mpeg      1.3         3/2/00 4:31:44 PM      Rob Tilton      Added 
 *        Get/Set Alpha.
 *        
 *  3    mpeg      1.2         2/9/00 1:20:16 PM      Rob Tilton      Added 
 *        vidSetActiveRegion().
 *        
 *  2    mpeg      1.1         2/1/00 5:30:44 PM      Dave Wilson     Changes 
 *        for OpenTV 1.2 support
 *        
 *  1    mpeg      1.0         1/5/00 10:20:42 AM     Rob Tilton      
 * $
 * 
 *    Rev 1.15   29 Sep 2003 13:02:48   dawilson
 * SCR(s) 7571 7570 :
 * Added option IDs to allow access to the parsed MPEG still pan/scan offset
 * information now held in the VIDRGN structure.
 * 
 *    Rev 1.14   21 Aug 2001 12:53:22   dawilson
 * SCR(s) 2504 2505 :
 * Added prototype for vidGetVideoRegion function.
 * 
 *    Rev 1.13   05 Jul 2001 13:05:18   dawilson
 * SCR(s) 2194 2195 :
 * Added bool return code from vidForceDisplay to allow caller to determine
 * whether display was being controlled by vcore or direct DRM programming when
 * the call is made.
 * 
 *    Rev 1.12   29 Jun 2001 17:08:22   dawilson
 * SCR(s) 2002 1914 2003 1915 1916 :
 * Added VO_USER_INFO. This field, intended for client use, will be needed in
 * the OpenTV video driver to reduce transients on unblank.
 * 
 *    Rev 1.11   28 Jun 2001 14:00:38   dawilson
 * SCR(s) 2105 2106 :
 * New microcode correctly reports buffer which is being displayed so reworked
 * these files to track this properly. Now decode of P frame stills will difference
 * with the correct buffer and they will end up looking correct.
 * 
 *    Rev 1.10   11 Jun 2001 09:12:18   dawilson
 * SCR(s) 2066 2067 :
 * 
 * AQdded vidForceDisplay to force DRM to display a video region or revert to hardware control. Similar to vidSetConnection without mucking with decoder buffers.
 * 
 *    Rev 1.9   31 May 2001 17:47:54   dawilson
 * SCR(s) 1897 1898 1826 1827 1670 1389 1390 :
 * Added prototype for vidGetSafeStillDecodeBuffer.
 * 
 *    Rev 1.8   20 Apr 2001 11:13:52   dawilson
 * DCS1124: Major changes to video memory management to get Sky Text app running
 * 
 *    Rev 1.7   12 Feb 2001 14:55:46   rutherq
 * Added vidSetRegionInput
 * 
 *    Rev 1.6   24 Nov 2000 19:41:44   eching
 * Added new options to track HSCALE and VSCALE parameters for vidrgn.
 * 
 *    Rev 1.5   12 Jul 2000 10:58:50   eching
 * Added video options for pan vector, scan vector, and automode along with
 * automode defines.
 * 
 *    Rev 1.4   22 Mar 2000 16:12:06   rtilton
 * Added video wipe functions.
 * 
 *    Rev 1.3   02 Mar 2000 16:31:44   rtilton
 * Added Get/Set Alpha.
 * 
 *    Rev 1.2   09 Feb 2000 13:20:16   rtilton
 * Added vidSetActiveRegion().
 * 
 *    Rev 1.1   01 Feb 2000 17:30:44   dawilson
 * Changes for OpenTV 1.2 support
 * 
 *    Rev 1.0   05 Jan 2000 10:20:42   rtilton
 * Initial revision.
 * 
 ****************************************************************************/

