/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*           Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002         */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       osdlibc.h
 *
 *
 * Description:    OSD Library Header File
 *
 *
 * Author:         Rob Tilton with major modifications by many others
 *
 ****************************************************************************/
/* $Header: osdlibc.h, 37, 3/5/04 6:59:26 PM, Steve Glennon$
 ****************************************************************************/
 
#ifndef _OSDLIBC_H_
#define _OSDLIBC_H_

typedef u_int32 OSDHANDLE;

#ifndef LPVOID
typedef void * LPVOID;
#endif /* LPVOID */

/* Aspect Ratio Mode Definitions */
typedef enum {
   OSD_ARM_PANSCAN = 1,
   OSD_ARM_LETTERBOX,
   OSD_ARM_NONE
} OSD_AR_MODE;

/* Display Aspect Ratio Definitions */
typedef enum {
   OSD_DISPLAY_AR_43 = 1,
   OSD_DISPLAY_AR_169
} OSD_DISPLAY_AR;


/* Video display output connection type.  These bits can be OR'd together. */
typedef u_int32 OSD_DISPLAY_OUTPUT;
#define OSD_OUTPUT_RGB       1  /* RGB is only available with a SCART connector. */
#define OSD_OUTPUT_COMPOSITE 2
#define OSD_OUTPUT_SVIDEO    4

#define MAKEID(a,b,c,d)                    \
        ((((long)(a)) << 24 ) |            \
         (((long)(b)) << 16 ) |            \
         (((long)(c)) <<  8 ) |            \
         (((long)(d)) <<  0 ))

#ifndef OPENTV_EN2
/* These are already defined in OPENTV_EN2.H */
#define PIXMAP_RL               MAKEID('B','M','R','L')
#define PIXMAP_RAW_RO           MAKEID('R','A','W','R')
#endif

/* Debug messaging functions */
#define OSD_ERROR_MSG                           (TRACE_OSD|TRACE_LEVEL_ALWAYS)
#define OSD_FUNC_TRACE                          (TRACE_OSD|TRACE_LEVEL_2)
#define OSD_MSG                                 (TRACE_OSD|TRACE_LEVEL_2)
#define OSD_FUNC_TRACE1                         (TRACE_OSD|TRACE_LEVEL_1)
#define OSD_MSG1                                (TRACE_OSD|TRACE_LEVEL_1)
#define OSD_ISR_MSG                             (TRACE_OSD|TRACE_LEVEL_1)

#define BOUND(x,top,bot) ((((x>top)?top:x)<bot)?bot:x)

/************************************************************************/
/* These macros convert Gamma RGB to YCrCb values using a scale factor  */
/* of 1024 to avoid floating point and integer division.                */
/* Bounds checking is done for safety but worst case values should      */
/* be within valid ranges. It is possible to generate Cr and Cb values  */
/* of 15 in the worst case due to round off error.                      */
/* If these are not illegal Cr,Cb values for hardware operations        */
/* then bounds checking could be disabled. Tweaking the coefficients    */
/* could also be done to avoid illegal values if a little more error in */
/* the conversion can be tolerated.                                     */
/************************************************************************/

#define RGBTOY(r,g,b)  (BOUND((((263*r+516*g+100*b)>>10)+16),235,16))
#define RGBTOCR(r,g,b) (BOUND((((450*r-377*g-73*b)>>10)+128),240,16))
#define RGBTOCB(r,g,b) (BOUND((((-152*r-298*g+450*b)>>10)+128),240,16))

/*********************************************************************/
/* These macros convert to Gamma RGB from YCrCb values using a scale */
/* factor of 1024 to avoid floating point and integer division.      */
/* Bounds checking is done since values outside the 0 to 255 range   */
/* are common.                                                       */
/*********************************************************************/
           

#define RFROMYCC(y,cr,cb) (BOUND(((1192*(y-16)+1608*(cr-128))>>10),255,0))
#define GFROMYCC(y,cr,cb) (BOUND(((1192*(y-16)-832*(cr-128)-401*(cb-128))>>10),255,0))
#define BFROMYCC(y,cr,cb) (BOUND(((1192*(y-16)+2066*(cb-128))>>10),255,0))

/* Maximum number of pixels in each direction that        */
/* SetDisplayPosition will allow you to move horizontally */
/* and vertically                                         */
#define MAX_SCREEN_POSITION_X_OFFSET 16
#define MAX_SCREEN_POSITION_Y_OFFSET 16

/**************************************************************/
/* Timing information defines.                                */
/* These are encoder specific and need to be addressed!       */
/* Changing HBLANK values can affect the alignment of color   */
/* components. So values may need to be fixed from calculated */
/* ones to get correct colors.                                */
/**************************************************************/
#define HBLANK_COUNT                            0xF3   // 20
#define VBLANK_COUNT                            0x10   // 10
#if (CUSTOMER == VENDOR_D)
#define BT865_PAL_HBLANK                        0x100
#else
#define BT865_PAL_HBLANK                        0x120  // 0xF4 //(864-720)
#endif
#define BT865_NTSC_HBLANK                       0xF4   // 0xF4 //(858-720)
#define BT865_PAL_VBLANK                        0x17   // 16   //(625-576)
#define BT865_NTSC_VBLANK                       0x13   // 16   //(525-480)

/*****************************************************************/
/* HBLANK and VBLANK values for PAL 861 changed from commented   */
/* values to get full 720x576 display of video. 865 values above */
/* may also need to be changed in a similar fashion but the spec */
/* for the 865 doesn't have the same register set so values will */
/* need to be tweaked for correct behavior.                      */
/*****************************************************************/

#define BT861_PAL_HBLANK                        0x108    
#define BT861_NTSC_HBLANK                       0x108    
#define BT861_PAL_VBLANK                        0x17     
#define BT861_NTSC_VBLANK                       0x13     
#define BT861_PAL_HSYNC                         0
#define BT861_NTSC_HSYNC                        0

/**************************************************************/
/*  The mode number is also used to determine the pixel type. */
/*     gnMode = 0 <==> 4bpp ARGB (ABGR)                       */
/*              1 <==> 4bpp AYUV (ACrCbY)                     */
/*              2 <==> 8bpp ARGB                              */
/*              3 <==> 8bpp AYUV                              */
/*              4 <==> 16bpp ARGB palette (4444)              */
/*              5 <==> 16bpp AYUV direct (4444)               */
/*              6 <==> 16bpp RGB palette (565)                */
/*              7 <==> 16bpp YUV direct (655)                 */
/*              8 <==> 16bpp YUV direct (4:2:2)               */
/*              9 <==> 16bpp ARGB palette (1555)              */
/*             10 <==> 16bpp YUV direct (2644)                */
/*             11 <==> 32bpp ARGB palette (8888)              */
/*             12 <==> 32bpp YUV direct (8888)                */
/**************************************************************/
#define OSD_MODE_4ARGB                          0
#define OSD_MODE_4AYUV                          1
#define OSD_MODE_8ARGB                          2
#define OSD_MODE_8AYUV                          3
#define OSD_MODE_16ARGB                         4
#define OSD_MODE_16AYUV                         5
#define OSD_MODE_16RGB                          6
#define OSD_MODE_16YUV655                       7
#define OSD_MODE_16YUV422                       8
#define OSD_MODE_16ARGB1555                     9
#define OSD_MODE_16AYUV2644                    10
#define OSD_MODE_32ARGB                        11
#define OSD_MODE_32AYUV                        12

#define OSD_HEADER_FIELD_STRIDE                 (OSD_BUF_FIELD_STRIDE |   \
                                                (OSD_BUF_FIELD_STRIDE << 16))

/*Background color defines */
#define BACKGROUND_Y                RGBTOY(0,0,0)
#define BACKGROUND_CR               RGBTOCR(0,0,0)
#define BACKGROUND_CB               RGBTOCB(0,0,0)

/* OSD driver type definitions */
/* MAX_BPP controls the mode table, palette, frame buffer, ... */
/* Valid entries are 4, 8, 16, or 32                           */
#define MAX_BPP                                 32

#if MAX_BPP == 4
#define NUM_OSD_MODES                           2
#elif MAX_BPP == 8
#define NUM_OSD_MODES                           4
#elif MAX_BPP == 16
#define NUM_OSD_MODES                           11
#elif MAX_BPP == 32
#define NUM_OSD_MODES                           13
#endif

#define OSD_MAX_HEIGHT                          576
#define OSD_MAX_WIDTH                           720

#define OSD_BUF_STRIDE                          (OSD_MAX_WIDTH*MAX_BPP/8)
#define OSD_BUF_FIELD_STRIDE                    (OSD_BUF_STRIDE*2)


#define MAX_OSD_RGN_ALPHA                 0xFF
#define DEFAULT_PALETTE_ALPHA             MAX_OSD_RGN_ALPHA

/* OSD Region Options - dwOptions */
#define OSD_RO_LOADPALETTE                0x00000001
#define OSD_RO_BSWAPACCESS                0x00000002
#define OSD_RO_FFENABLE                   0x00000004 /* Only valid for plane 0 */
#define OSD_RO_ALPHAENABLE                0x00000008
#define OSD_RO_COLORKEYENABLE             0x00000010
#define OSD_RO_ARCENABLE                  0x00000020 /* Only valid for plane 0 */
#define OSD_RO_ALPHATOPVIDEO              0x00000040
#define OSD_RO_FORCEREGIONALPHA           0x00000080
#define OSD_RO_4BPPPALINDEX               0x00000100
#define OSD_RO_ENABLE                     0x00000200
#define OSD_RO_FRAMEBUFFER                0x00000400 /* Read only */
#define OSD_RO_FRAMESTRIDE                0x00000800 /* Read only */
#define OSD_RO_MODE                       0x00001000 /* Read only */
#define OSD_RO_ALPHABOTHVIDEO             0x00002000
#define OSD_RO_BUFFERHEIGHT               0x00004000 /* Read only */
#define OSD_RO_BUFFERWIDTH                0x00008000 /* Read only */
#define OSD_RO_PALETTEADDRESS             0x00010000 /* Read only */
#define OSD_RO_PLANE0                     0x00000000 /* Set on Create only */
#define OSD_RO_PLANE1                     0x00020000 /* Set on Create only */  
#define OSD_RO_OBSCUREPLANE1              0x00040000 /* Only valid for plane 0 */                   
#define OSD_RO_NOCLEARBUFFER              0x00080000 /* Used on creation only */

typedef struct _OSDRECT {
   short    left;
   short    top;
   short    right;
   short    bottom;
} OSDRECT, *POSDRECT;

typedef struct _OSDLIBREGION *POSDLIBREGION;
typedef struct _OSDLIBREGION {
   POSDLIBREGION  pPrev;
   POSDLIBREGION  pNext;
   int            nTop;       /* Screen pos and size        */
   int            nLeft;      /* Screen pos and size        */
   int            nRight;     /* Screen pos and size        */
   int            nBottom;    /* Screen pos and size        */
   u_int32        dwW;        /* Width of allocated buffer  */
   u_int32        dwH;        /* Height of allocated buffer */
   u_int32        dwStride;   /* Stride of allocated buffer */
   u_int32        dwNumPalEntries;
   u_int32        dwMode;     /* Index into mode table */
   u_int32        dwOptions;
   u_int32        dwPalIndex; /* Used only in 4bpp */
   u_int32        dwRgnAlpha;
   u_int32        dwUnAlignedHdr; /* Unaligned ptr from alloc (cache alignment) */
   u_int32        dwUnAlignedPal;
   u_int32        dwUnAlignedImage;

   OSDHEADER      OSDHdr1;
   OSDHEADER      OSDHdr2;
   PDRMPAL        pPalette;
   LPVOID         pImage;
   LPVOID         pAllocatedImage;
   bool           bTransient;
   u_int32        uPlane;     /* OSD Plane - 0 for Colorado, 0,1 for Wabash */
} OSDLIBREGION;

typedef enum  {
   PALETTE_ALPHA = 1,
   REGION_ALPHA = 2
} OSDALPHAFLAG;

typedef enum {
   MPG_VIDEO = 1,
   LIVE_VIDEO = 2
} VIDEOPLANE;

typedef enum {
   OSD_NORMAL = 1,
   OSD_RLE    = 2
} OSDPMTYPE;

typedef struct _OSDPIXMAP {
   OSDRECT     rcBounds;
   u_int8      *pData;
   OSDPMTYPE   pmType;
   u_int16     wStride;
   u_int16     *pRLOffsets;
} OSDPIXMAP, *POSDPIXMAP;

// OSD Line Interrupt Callback API Types.
typedef enum {
   TOP,
   BOTTOM,
   BOTH
} OSDFIELD;

typedef void (*POSDLINEISR)(u_int32 dwLine, OSDFIELD oField);
typedef void (*PFNOSDISR)(u_int32 dwParam1, u_int32 dwParam2);

/************************/
/*                      */
/*  Function prototypes */
/*                      */
/************************/

/************************************/
/* Exported OSD function prototypes */
/************************************/
void OSDLibInit(u_int32 dwOSDMaxHeight, u_int16 uVideoFormat);
#define CreateOSDRegion(lpRect, dwPixelMode, dwOptions) CreateOSDRgn(lpRect, dwPixelMode, dwOptions, NULL, 0)
OSDHANDLE CreateOSDRgn(POSDRECT lpRect, u_int32 dwPixelMode, u_int32 dwOptions, LPVOID lpImage, u_int32  dwStride);
OSDHANDLE CreateOSDRgnX(POSDRECT lpRect, u_int32 dwPixelMode, u_int32 dwOptions, LPVOID lpImage, u_int32  dwStride);
bool DestroyOSDRegion(OSDHANDLE hRgn);
bool SetOSDRgnOptions(OSDHANDLE hRgn, u_int32 dwFlags, u_int32 dwValue);
u_int32 GetOSDRgnOptions(OSDHANDLE hRgn, u_int32 dwFlags);
bool SwapOSDRegions(OSDHANDLE hNew, OSDHANDLE hOld);
bool SetOSDRgnRect(OSDHANDLE hRgn, POSDRECT lpRect);
bool GetOSDRgnRect(OSDHANDLE hRgn, POSDRECT lpRect);
u_int8 GetOSDRgnBpp(OSDHANDLE hRgn);
u_int32 SetOSDRgnPalette(OSDHANDLE hRgn, PDRMPAL pPal, bool bLoadAlpha);
bool SetOSDRgnAlpha(OSDHANDLE hRgn, OSDALPHAFLAG Flags, u_int32 dwAlpha, u_int32 dwPalIndex);
bool SetOSDColorKey(u_int32 dwColorKey);
u_int32 GetOSDColorKey(void);
bool SetOSDBackground(HW_DWORD yccBackground);
HW_DWORD GetOSDBackground(void);
u_int32 SetDefaultOSDPalette(OSDHANDLE hRgn);
DRMPAL GetOSDRgnPalEntry(OSDHANDLE hRgn, u_int32 dwIndex);
bool SetOSDRgnPalEntry(OSDHANDLE hRgn, u_int32 dwIndex, DRMPAL drmPal);
u_int32 SetCursorInvertColor(DRMPAL rgbColor, bool bInvert);
bool ResizeOSDRegion(OSDHANDLE hRgn, POSDRECT lpRect);
void SetOutputType(u_int16 video_standard);
bool RectIntersect(POSDRECT prc1, POSDRECT prc2);
bool osdRegisterLineISR(POSDLINEISR pfnCallback, u_int32 dwLine, OSDFIELD oSignal);
void osdUnRegisterLineISR(POSDLINEISR pfnCallback, u_int32 dwLine);
bool osdRegIFrameComplete(PFNOSDISR pfnCallback);
bool osdRegAspectRatio(PFNOSDISR pfnCallback);
void osdResetAspectRatioCallback(void);
bool SetOSDBuffer(OSDHANDLE hRgn, LPVOID pBuffer, u_int32 dwStride);

/************************************/
/* Exported MPG function prototypes */
/************************************/                               
void UpdateMpgScaling(void);
bool SetMpgAR(OSD_AR_MODE arMode, bool bUpdate);
OSD_AR_MODE GetMpgAR(void);
bool SetDisplayAR(OSD_DISPLAY_AR arDisplay, bool bUpdate);
OSD_DISPLAY_AR GetDisplayAR(void);
bool SetAspectRatio(OSD_DISPLAY_AR arDisplay, OSD_AR_MODE arMode);
bool GetAspectRatio(OSD_DISPLAY_AR *parDisplay, OSD_AR_MODE *parMode);
bool SetDisplayOutput(OSD_DISPLAY_OUTPUT odoType);
OSD_DISPLAY_OUTPUT GetDisplayOutput(void);
bool SetDisplayPosition(int iXoffset, int iYoffset);
bool GetDisplayPosition(int *piXStart, int *piYStart);


/********************************/
/* Internal function prototypes */
/********************************/                           
bool UpdateOSDHeader(OSDHANDLE hRgn);
void GenerateOSDLinkList(u_int32 uPlane);
void OSDInit(void);
void EncoderInit(u_int16 video_standard);
void RLCopy2Screen(u_int8 *pDst, u_int8 *pSrcData, 
                   unsigned short *RunLengthOffsets,
                   int DstX, int DstY, int ExtX, int ExtY, 
                   int SrcX, int SrcY, LPVOID lpBuffer, 
                   u_int32 dwStride, u_int8 byBpp);
void GraphicsInit(void);
int GraphicsShutdown(void);

/***************************************/
/* Exported BitBlt function prototypes */
/***************************************/
void osd_bitblit_mem2screen(OSDHANDLE hOSDRgn, u_int8 *data, u_int16 bytes_per_line, POSDRECT dst);
void osd_bitblit_fill(OSDHANDLE hOSDRgn, u_int32 quad, POSDRECT dst);
void osd_bitblit_xor_fill(OSDHANDLE hOSDRgn, u_int32 quad, POSDRECT dst);
int osd_bitblit_rl_fill(OSDHANDLE hOSDRgn, POSDPIXMAP pix, POSDRECT dst);
void osd_bitblit_screen2mem(OSDHANDLE hOSDRgn, u_int8 *data, u_int16 bytes_per_row, POSDRECT src);

#ifdef OPENTV_12
/***************************************************/
/* Functions relating to transient heap management */
/***************************************************/
bool osdRelocateTransientRegions(void);
#endif /* OPENTV_12 */

#endif /* _OSDLIBC_H_ */

/****************************************************************************
 * Modifications:
 * $Log: 
 *  37   mpeg      1.36        3/5/04 6:59:26 PM      Steve Glennon   CR(s) 
 *        8473 : Added #define of OSD_RO_NOCLEARBUFFER to match API spec.
 *        
 *  36   mpeg      1.35        7/31/03 3:52:38 PM     Steven Jones    SCR(s): 
 *        5046 
 *        Add swap region function for flicker-free buffer switching.
 *        
 *  35   mpeg      1.34        5/9/03 7:00:48 PM      Steve Glennon   SCR(s): 
 *        6224 6225 6190 6179 
 *        Changes to support second plane on dual plane capable devices (Wabash
 *         etc)
 *        
 *        
 *  34   mpeg      1.33        9/25/02 10:21:44 PM    Carroll Vance   SCR(s) 
 *        3786 :
 *        Removing old DRM and AUD conditional bitfield code.
 *        
 *        
 *  33   mpeg      1.32        5/24/02 4:25:16 PM     Dave Wilson     SCR(s) 
 *        3842 :
 *        Tweaked blanking and sync values to get maximum possible display area
 *         for
 *        both PAL and NTSC.
 *        
 *  32   mpeg      1.31        5/21/02 1:37:54 PM     Carroll Vance   SCR(s) 
 *        3786 :
 *        Removed DRM bitfields.
 *        
 *  31   mpeg      1.30        5/15/02 12:38:38 PM    Dave Wilson     SCR(s) 
 *        3788 :
 *        Added API osdResetAspectRatioCallback to allow an app to force the 
 *        driver to
 *        call back on the next size or aspect ratio interrupt regardless of 
 *        the last
 *        aspect ratio, height or width reported.
 *        
 *  30   mpeg      1.29        4/23/02 4:43:26 PM     Billy Jackman   SCR(s) 
 *        3604 :
 *        Fixed #define for BT861_NTSC_HBLANK; added #defines for 
 *        BT861_PAL_HSYNC and
 *        BT861_NTSC_HSYNC to support single pixel horizontal adjustment.
 *        
 *  29   mpeg      1.28        4/5/02 11:44:22 AM     Tim White       SCR(s) 
 *        3510 :
 *        Backout DCS #3468
 *        
 *        
 *  28   mpeg      1.27        3/28/02 2:26:48 PM     Quillian Rutherford 
 *        SCR(s) 3468 :
 *        Changed referenence to DRM_COLOR, a bit field, to HW_DWORD.
 *        
 *        
 *  27   mpeg      1.26        3/27/02 5:14:48 PM     Dave Wilson     SCR(s) 
 *        3455 :
 *        Added prototypes for new SetDisplayPosition and GetDisplayPosition 
 *        APIs
 *        
 *  26   mpeg      1.25        11/20/01 11:19:48 AM   Quillian Rutherford 
 *        SCR(s) 2754 :
 *        Removed an enum for the SCART types to comply with new build system
 *        
 *  25   mpeg      1.24        4/20/01 12:13:50 PM    Dave Wilson     DCS1124: 
 *        Major changes to video memory management to get Sky Text app running
 *        
 *  24   mpeg      1.23        2/16/01 5:14:58 PM     Dave Wilson     DCS1217: 
 *        Changes for Vendor D rev 7 board.
 *        
 *  23   mpeg      1.22        2/11/01 11:56:06 PM    Steve Glennon   
 *        Additional adjustment for DCS#1173 - changed BT861_HBLANK to 0x114 
 *        and made it conditional on OPENTV_12
 *        
 *  22   mpeg      1.21        2/10/01 11:41:30 PM    Steve Glennon   Fix for 
 *        DCS #1173 - showing less at left of stills than ST box
 *        Changed the BT861 HBLANK value from 0x108 to ox118 to match section 
 *        of image 
 *        shown by ST box.
 *        
 *  21   mpeg      1.20        1/4/01 8:01:44 PM      Lucy C Allevato Changed 
 *        hblank and vblank defines for 861 as part of fix for DCS 887
 *        where BSkyB video test 7 was not displaying 1 pixel wide or 1 pixel 
 *        high
 *        video regions.
 *        
 *  20   mpeg      1.19        1/3/01 11:06:00 AM     Dave Wilson     DCS859 - 
 *        Changed default background to black
 *        
 *  19   mpeg      1.18        11/6/00 10:09:06 AM    Dave Wilson     OpenTVX.H
 *         from latest EN2 HPK already defines PIXMAP_RL and PIXMAP_RAW_RO
 *        so added conditional compilation stuff to keep the header working on 
 *        EN2.,
 *        
 *  18   mpeg      1.17        10/2/00 9:09:22 PM     Lucy C Allevato Added 
 *        support for 2 OSDHEADERs to allow for double buffering to prevent
 *        flicker while rebuilding the OSD header linked list.
 *        
 *  17   mpeg      1.16        9/26/00 12:14:06 PM    Dave Wilson     Commented
 *         out I2C address definitions. These are now in the vendor header
 *        
 *  16   mpeg      1.15        9/25/00 12:42:34 PM    Lucy C Allevato Added 
 *        some additional trace level defines for selective tracing in OSD.
 *        
 *  15   mpeg      1.14        8/10/00 8:02:52 PM     Tim Ross        Rolled in
 *         VENDOR_B changes.
 *        
 *  14   mpeg      1.13        8/8/00 5:19:24 PM      Lucy C Allevato Fixed 
 *        name typo osdRegAspectRetio now osdRegAspectRatio.
 *        
 *  13   mpeg      1.12        7/11/00 8:13:10 PM     Dave Wilson     Changed 
 *        I2C addresses for vendor_a board
 *        
 *  12   mpeg      1.11        4/27/00 9:27:56 AM     Rob Tilton      Added 
 *        SetOSDBuffer().
 *        
 *  11   mpeg      1.10        4/18/00 10:58:18 AM    Dave Wilson     Added new
 *         SCART type definition for Klondike boards
 *        
 *  10   mpeg      1.9         4/7/00 5:44:18 PM      Rob Tilton      Added OSD
 *         ISR callback registration functions.
 *        
 *  9    mpeg      1.8         3/3/00 2:40:36 PM      Rob Tilton      Changed 
 *        the name of CreateOSDRegion to CreateOSDRgn to allow for the new
 *        parameters to CreateOSDRgn.  Then added a macro for CreateOSDRegion 
 *        for 
 *        backward compatability.
 *        
 *  8    mpeg      1.7         3/2/00 5:04:18 PM      Anzhi Chen      Changed 
 *        MAX_BPP to 32.
 *        
 *  7    mpeg      1.6         2/28/00 3:32:28 PM     Rob Tilton      Added new
 *         ISR Line Callback API.
 *        
 *  6    mpeg      1.5         2/18/00 3:30:52 PM     Lucy C Allevato Changed 
 *        names of new 32 BPP modes to reflect that they contain alpha.
 *        
 *  5    mpeg      1.4         2/17/00 2:25:10 PM     Lucy C Allevato Added new
 *         colorado OSD format definitions.
 *        
 *  4    mpeg      1.3         1/20/00 4:52:26 PM     Lucy C Allevato Updated 
 *        OSD_MAX_RGN_ALPHA to 0xFF.
 *        
 *  3    mpeg      1.2         1/19/00 5:45:12 PM     Rob Tilton      Additions
 *         for OpenTv 1.2 driver.
 *        
 *  2    mpeg      1.1         1/6/00 1:28:02 PM      Rob Tilton      Added the
 *         LPVOID definition back.
 *        
 *  1    mpeg      1.0         1/5/00 10:18:42 AM     Rob Tilton      
 * $
 * 
 *    Rev 1.35   31 Jul 2003 14:52:38   joness
 * SCR(s): 5046 
 * Add swap region function for flicker-free buffer switching.
 * 
 *    Rev 1.34   09 May 2003 18:00:48   glennon
 * SCR(s): 6224 6225 6190 6179 
 * Changes to support second plane on dual plane capable devices (Wabash etc)
 * 
 * 
 *    Rev 1.33   25 Sep 2002 21:21:44   vancec
 * SCR(s) 3786 :
 * Removing old DRM and AUD conditional bitfield code.
 * 
 * 
 *    Rev 1.32   24 May 2002 15:25:16   dawilson
 * SCR(s) 3842 :
 * Tweaked blanking and sync values to get maximum possible display area for
 * both PAL and NTSC.
 * 
 *    Rev 1.31   21 May 2002 12:37:54   vancec
 * SCR(s) 3786 :
 * Removed DRM bitfields.
 * 
 *    Rev 1.30   15 May 2002 11:38:38   dawilson
 * SCR(s) 3788 :
 * Added API osdResetAspectRatioCallback to allow an app to force the driver to
 * call back on the next size or aspect ratio interrupt regardless of the last
 * aspect ratio, height or width reported.
 * 
 *    Rev 1.29   23 Apr 2002 15:43:26   jackmaw
 * SCR(s) 3604 :
 * Fixed #define for BT861_NTSC_HBLANK; added #defines for BT861_PAL_HSYNC and
 * BT861_NTSC_HSYNC to support single pixel horizontal adjustment.
 * 
 *    Rev 1.28   05 Apr 2002 11:44:22   whiteth
 * SCR(s) 3510 :
 * Backout DCS #3468
 * 
 * 
 *    Rev 1.26   27 Mar 2002 17:14:48   dawilson
 * SCR(s) 3455 :
 * Added prototypes for new SetDisplayPosition and GetDisplayPosition APIs
 * 
 *    Rev 1.25   20 Nov 2001 11:19:48   rutherq
 * SCR(s) 2754 :
 * Removed an enum for the SCART types to comply with new build system
 * 
 *    Rev 1.24   20 Apr 2001 11:13:50   dawilson
 * DCS1124: Major changes to video memory management to get Sky Text app running
 * 
 *    Rev 1.23   16 Feb 2001 17:14:58   dawilson
 * DCS1217: Changes for Vendor D rev 7 board.
 * 
 *    Rev 1.22   11 Feb 2001 23:56:06   glennon
 * Additional adjustment for DCS#1173 - changed BT861_HBLANK to 0x114 
 * and made it conditional on OPENTV_12
 * 
 *    Rev 1.21   10 Feb 2001 23:41:30   glennon
 * Fix for DCS #1173 - showing less at left of stills than ST box
 * Changed the BT861 HBLANK value from 0x108 to ox118 to match section of image 
 * shown by ST box.
 * 
 *    Rev 1.20   04 Jan 2001 20:01:44   eching
 * Changed hblank and vblank defines for 861 as part of fix for DCS 887
 * where BSkyB video test 7 was not displaying 1 pixel wide or 1 pixel high
 * video regions.
 * 
 *    Rev 1.19   03 Jan 2001 11:06:00   dawilson
 * DCS859 - Changed default background to black
 * 
 *    Rev 1.18   06 Nov 2000 10:09:06   dawilson
 * OpenTVX.H from latest EN2 HPK already defines PIXMAP_RL and PIXMAP_RAW_RO
 * so added conditional compilation stuff to keep the header working on EN2.,
 * 
 *    Rev 1.17   02 Oct 2000 20:09:22   eching
 * Added support for 2 OSDHEADERs to allow for double buffering to prevent
 * flicker while rebuilding the OSD header linked list.
 * 
 *    Rev 1.16   26 Sep 2000 11:14:06   dawilson
 * Commented out I2C address definitions. These are now in the vendor header
 * 
 *    Rev 1.15   25 Sep 2000 11:42:34   eching
 * Added some additional trace level defines for selective tracing in OSD.
 * 
 *    Rev 1.14   10 Aug 2000 19:02:52   rossst
 * Rolled in VENDOR_B changes.
 * 
 *    Rev 1.13   08 Aug 2000 16:19:24   eching
 * Fixed name typo osdRegAspectRetio now osdRegAspectRatio.
 * 
 *    Rev 1.12   11 Jul 2000 19:13:10   dawilson
 * Changed I2C addresses for vendor_a board
 * 
 *    Rev 1.11   27 Apr 2000 08:27:56   rtilton
 * Added SetOSDBuffer().
 * 
 *    Rev 1.10   18 Apr 2000 09:58:18   dawilson
 * Added new SCART type definition for Klondike boards
 * 
 *    Rev 1.9   07 Apr 2000 16:44:18   rtilton
 * Added OSD ISR callback registration functions.
 * 
 *    Rev 1.8   03 Mar 2000 14:40:36   rtilton
 * Changed the name of CreateOSDRegion to CreateOSDRgn to allow for the new
 * parameters to CreateOSDRgn.  Then added a macro for CreateOSDRegion for 
 * backward compatability.
 * 
 *    Rev 1.7   02 Mar 2000 17:04:18   achen
 * Changed MAX_BPP to 32.
 * 
 *    Rev 1.6   28 Feb 2000 15:32:28   rtilton
 * Added new ISR Line Callback API.
 * 
 *    Rev 1.5   18 Feb 2000 15:30:52   eching
 * Changed names of new 32 BPP modes to reflect that they contain alpha.
 * 
 *    Rev 1.4   17 Feb 2000 14:25:10   eching
 * Added new colorado OSD format definitions.
 * 
 *    Rev 1.3   20 Jan 2000 16:52:26   eching
 * Updated OSD_MAX_RGN_ALPHA to 0xFF.
 * 
 *    Rev 1.2   19 Jan 2000 17:45:12   rtilton
 * Additions for OpenTv 1.2 driver.
 * 
 *    Rev 1.1   06 Jan 2000 13:28:02   rtilton
 * Added the LPVOID definition back.
 * 
 *    Rev 1.0   05 Jan 2000 10:18:42   rtilton
 * Initial revision.
 * 
 *****************************************************************************/

