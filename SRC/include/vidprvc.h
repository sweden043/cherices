/*****************************************************************************/
/* File: vidprv.h                                                            */
/*                                                                           */
/* Module: Video resource library.                                           */
/*                                                                           */
/* Description: Private include file for the video path of Colorado.         */
/*****************************************************************************/
/*****************************************************************************
$Header: vidprvc.h, 5, 9/29/03 2:03:48 PM, Dave Wilson$
$Log: 
 5    mpeg      1.4         9/29/03 2:03:48 PM     Dave Wilson     SCR(s) 7571 
       7570 :
       Added iPanRead, iScanRead and bVectorPresent fields to the VIDRGN 
       structure
       to allow storage of any pan/scan vector parsed from an MPEG still. We 
       need to
       hold this information alongside the image that requires it, hence we put
        it
       inside the region structure.
       
 4    mpeg      1.3         6/29/01 6:08:10 PM     Dave Wilson     SCR(s) 2002 
       1914 2003 1915 1916 :
       Added VO_USER_INFO. This field, intended for client use, will be needed 
       in
       the OpenTV video driver to reduce transients on unblank.
       
 3    mpeg      1.2         4/20/01 12:13:48 PM    Dave Wilson     DCS1124: 
       Major changes to video memory management to get Sky Text app running
       
 2    mpeg      1.1         2/22/01 11:44:04 AM    Steve Glennon   Fix for 
       DCS#1278 - added bSrcRcSpec to signal when source rectangle has
       been specified - false if scaling provided, true if src rect provided
       
 1    mpeg      1.0         2/12/01 11:06:42 AM    Quillian Rutherford 
$
 * 
 *    Rev 1.4   29 Sep 2003 13:03:48   dawilson
 * SCR(s) 7571 7570 :
 * Added iPanRead, iScanRead and bVectorPresent fields to the VIDRGN structure
 * to allow storage of any pan/scan vector parsed from an MPEG still. We need to
 * hold this information alongside the image that requires it, hence we put it
 * inside the region structure.
 * 
 *    Rev 1.3   29 Jun 2001 17:08:10   dawilson
 * SCR(s) 2002 1914 2003 1915 1916 :
 * Added VO_USER_INFO. This field, intended for client use, will be needed in
 * the OpenTV video driver to reduce transients on unblank.
 * 
 *    Rev 1.2   20 Apr 2001 11:13:48   dawilson
 * DCS1124: Major changes to video memory management to get Sky Text app running
 * 
 *    Rev 1.1   22 Feb 2001 11:44:04   glennon
 * Fix for DCS#1278 - added bSrcRcSpec to signal when source rectangle has
 * been specified - false if scaling provided, true if src rect provided
 * 
 *    Rev 1.6   22 Feb 2001 01:17:34   glennon
 * Added field to indicate that source rectangle has been specified 
 * (bSrcRcSpec) to VIDRGN structure.
 * Part of a fix to DCS#1278 along with vidlibc.c r 1.46 and otvvid12.c r 1.46
 * 
 *    Rev 1.5   24 Nov 2000 19:42:34   eching
 * Added dwHScale and dwVScale values to vidrgn to track the manul scale
 * settings. These are used when the scale is being set before any decode occurs
 * so that the scale will be sett properly when decode does occur.
 * 
 *    Rev 1.4   12 Jul 2000 11:03:32   eching
 * Added dwPan, dwScan, and dwAutoMode to VIDRGN struct. for auto vs. manual
 * scaling, windowing, and offset functionality.
 * 
 *    Rev 1.3   13 Apr 2000 16:50:24   rtilton
 * Added extra param when setting the decode buffers.
 * 
 *    Rev 1.2   09 Feb 2000 15:42:48   rtilton
 * Added internal fucntion vidSetHWBuffSize().
 * 
 *    Rev 1.1   01 Feb 2000 17:31:50   dawilson
 * Changes for OpenTV 1.2 support
 * 
 *    Rev 1.0   05 Jan 2000 10:24:50   rtilton
 * Initial revision.
 * 
*****************************************************************************/
#ifndef _VIDPRV_H_
#define _VIDPRV_H_

typedef struct _VIDRGN *PVIDRGN;
typedef struct _VIDRGN {
   PVIDRGN        pNext;         /* Ptr to next video region in the list. */
   PVIDRGN        pPrev;         /* Ptr to the previous region in the list. */
   u_int32        dwHeight;      /* Height in pixels of the allocated buffer. */
   u_int32        dwWidth;       /* Width in pixels of the allocated buffer. */
   u_int32        dwStride;      /* Stride in bytes of the allocated buffer. */
   VIDRGNTYPE     vrType;        /* Pixel type, 420 or 422. */
   u_int8         *pBuffer;      /* Pointer to allocated video buffer. */
   VIDRGNCONNECT  vrConnection;  /* Source connection, hardware or software. */
   OSDRECT        rcSrc;         /* Source rect used for clipping. */
   OSDRECT        rcDst;         /* Screen placement rectangle. */
   OSD_AR_MODE    arMode;        /* Aspect ratio mode, letterbox, panscan, none. */
   u_int32        dwAR;          /* Aspect ratio of this video region. */
   u_int32        dwSrcH;        /* The height of the image in the buffer. */
   u_int32        dwSrcW;        /* The width of the image in the buffer. */
   u_int32        dwSize;        /* Size of buffer linked to this region. */
   u_int32        dwVScale;      /* Manual vert. scale factor for region */
   u_int32        dwHScale;      /* Manual horz. scale factor for region */
   int32          dwPan;         /* Storage for manual pan vector */
   int32          dwScan;        /* Storage for manual scan vector */
   u_int32        dwAutoMode;    /* Automode flags */
   bool           bTop;          /* TRUE if this plane is the topmost plane. */
   bool           bSrcRcSpec;    /* TRUE if rcSrc has been set by a vidSetPos */
   bool           bTransient;    /* TRUE if allocated in the transient part of the heap */
   int16          iPanRead;      /* Pan vector read for this image (if a still) */
   int16          iScanRead;     /* Scan vector read for this image (if a still)*/
   bool           bVectorPresent;/* TRUE if a vector was read, FALSE if none was found */
   u_int32        dwUserInfo;    /* Free space for application to use for eg. instance hook */
} VIDRGN;

#define VID_MAX_VID_RGN       2
#define INVALID_VID_INDEX     0xFFFFFFFF

// Internal function prototypes.
void vidUpdate420Scaling(u_int32 dwRgnIndex);
void vidUpdate422Scaling(u_int32 dwRgnIndex);
void vidSetMpgDecodeAddr(u_int8 *pIBuffer, u_int8 *pPBuffer);
HVIDRGN vidCreateVideoRgnGeneric(POSDRECT pRc, u_int32 dwSize, VIDRGNTYPE vrType);
void vidSetHWBuffSize(void);

#endif /* _VIDPRV_H_ */

