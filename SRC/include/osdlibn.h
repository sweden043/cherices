/*****************************************************************************/
/* File: osdlibn.h                                                           */
/*                                                                           */
/* Module: CN86XX OSD resource library.                                      */
/*                                                                           */
/* Description:  Hardware API for the Display Refresh Module.                */
/*****************************************************************************/
/*****************************************************************************
$Header: osdlibn.h, 9, 4/5/02 11:44:26 AM, Tim White$
$Log: 
 9    mpeg      1.8         4/5/02 11:44:26 AM     Tim White       SCR(s) 3510 
       :
       Backout DCS #3468
       
       
 8    mpeg      1.7         3/28/02 2:28:26 PM     Quillian Rutherford SCR(s) 
       3468 :
       Removed DRM_COLOR and replaced with HW_DWORD
       
       
 7    mpeg      1.6         11/20/01 11:20:56 AM   Quillian Rutherford SCR(s) 
       2754 :
       Removed an enum for the SCART type to comply with new build system.
       
       
 6    mpeg      1.5         9/28/00 10:35:28 AM    Dave Wilson     Removed I2C 
       address definitions. These are now in the relevant vendor's 
       header file.
       
 5    mpeg      1.4         4/18/00 10:58:28 AM    Dave Wilson     Added new 
       SCART type definition for Klondike boards
       
 4    mpeg      1.3         2/28/00 3:10:58 PM     Rob Tilton      Added new 
       ISR Line Callback API.
       
 3    mpeg      1.2         1/21/00 1:22:04 PM     Rob Tilton      Added 
       defines which were needed for the OpenTv 1.2 driver.
       
 2    mpeg      1.1         1/20/00 5:29:56 PM     Rob Tilton      Updates for 
       OpenTv 1.2.
       
 1    mpeg      1.0         1/5/00 10:19:14 AM     Rob Tilton      
$
 * 
 *    Rev 1.8   05 Apr 2002 11:44:26   whiteth
 * SCR(s) 3510 :
 * Backout DCS #3468
 * 
 * 
 *    Rev 1.6   20 Nov 2001 11:20:56   rutherq
 * SCR(s) 2754 :
 * Removed an enum for the SCART type to comply with new build system.
 * 
 * 
 *    Rev 1.5   28 Sep 2000 09:35:28   dawilson
 * Removed I2C address definitions. These are now in the relevant vendor's 
 * header file.
 * 
 *    Rev 1.4   18 Apr 2000 09:58:28   dawilson
 * Added new SCART type definition for Klondike boards
 * 
 *    Rev 1.3   28 Feb 2000 15:10:58   rtilton
 * Added new ISR Line Callback API.
 * 
 *    Rev 1.2   21 Jan 2000 13:22:04   rtilton
 * Added defines which were needed for the OpenTv 1.2 driver.
 * 
 *    Rev 1.1   20 Jan 2000 17:29:56   rtilton
 * Updates for OpenTv 1.2.
 * 
 *    Rev 1.0   05 Jan 2000 10:19:14   rtilton
 * Initial revision.
 * 
*****************************************************************************/
#ifndef _OSDLIBN_H_
#define _OSDLIBN_H_

#ifndef OPENTV
#ifndef direct_memory_alloc
#define direct_memory_alloc malloc
#endif /* direct_memory_alloc */
#ifndef direct_memory_free
#define direct_memory_free  free
#endif /* direct_memory_free */
#endif /* OPENTV */

typedef u_int32 OSDHANDLE;

#ifndef LPVOID
typedef void * LPVOID;
#endif /* LPVOID */

//#ifndef BOOL
//typedef u_int32 BOOL;
//#endif /* BOOL */

#ifndef LPBYTE
typedef char * LPBYTE;
#endif /* LPBYTE */

#ifndef BYTE
typedef u_int8 BYTE;
#endif /* BYTE */

// Aspect Ratio Mode Definitions
typedef enum {
   OSD_ARM_PANSCAN = 1,
   OSD_ARM_LETTERBOX,
   OSD_ARM_NONE
} OSD_AR_MODE;

// Display Aspect Ratio Definitions
typedef enum {
   OSD_DISPLAY_AR_43 = 1,
   OSD_DISPLAY_AR_169
} OSD_DISPLAY_AR;

// Video display output connection type.  These bits can be OR'd together.
typedef u_int32 OSD_DISPLAY_OUTPUT;
#define OSD_OUTPUT_RGB       1  // RGB is only available with a SCART connector.
#define OSD_OUTPUT_COMPOSITE 2
#define OSD_OUTPUT_SVIDEO    4

#define MAKEID(a,b,c,d)                    \
        ((((long)(a)) << 24 ) |            \
         (((long)(b)) << 16 ) |            \
         (((long)(c)) <<  8 ) |            \
         (((long)(d)) <<  0 ))

#define PIXMAP_RL               MAKEID('B','M','R','L')
#define PIXMAP_RAW_RO           MAKEID('R','A','W','R')

// Debug messaging functions
#define OSD_ERROR_MSG                           (TRACE_OSD|TRACE_LEVEL_ALWAYS)
#define OSD_FUNC_TRACE                          (TRACE_OSD|TRACE_LEVEL_2)
#define OSD_MSG                                 (TRACE_OSD|TRACE_LEVEL_2)
#define OSD_ISR_MSG                             (TRACE_OSD|TRACE_LEVEL_1)

#define BOUND(x,top,bot) ((((x>top)?top:x)<bot)?bot:x)

// These macros convert Gamma RGB to YCrCb values using a scale factor
// of 1024 to avoid floating point and integer division.
// Bounds checking is done for safety but worst case values should
// be within valid ranges. It is possible to generate Cr and Cb values
// of 15 in the worst case due to round off error.
// If these are not illegal Cr,Cb values for hardware operations
// then bounds checking could be disabled. Tweaking the coefficients
// could also be done to avoid illegal values if a little more error in
// the conversion can be tolerated.

#define RGBTOY(r,g,b)  (BOUND((((263*r+516*g+100*b)>>10)+16),235,16))
#define RGBTOCR(r,g,b) (BOUND((((450*r-377*g-73*b)>>10)+128),240,16))
#define RGBTOCB(r,g,b) (BOUND((((-152*r-298*g+450*b)>>10)+128),240,16))

// These macros convert to Gamma RGB from YCrCb values using a scale
// factor of 1024 to avoid floating point and integer division.
// Bounds checking is done since values outside the 0 to 255 range
// are common.

#define RFROMYCC(y,cr,cb) (BOUND(((1192*(y-16)+1608*(cr-128))>>10),255,0))
#define GFROMYCC(y,cr,cb) (BOUND(((1192*(y-16)-832*(cr-128)-401*(cb-128))>>10),255,0))
#define BFROMYCC(y,cr,cb) (BOUND(((1192*(y-16)+2066*(cb-128))>>10),255,0))

// I2C Device defines
// *** These have now been moved to each vendor's header file ***
//#define BT865_IIC_ADDRESS                    0x8A
//#define BT861_IIC_ADDRESS                    0x8A

// Timing information defines.  These are encoder specific and need to be addressed! TBD
#define HBLANK_COUNT                            0xF3   // 20
#define VBLANK_COUNT                            0x10   // 10
#define BT865_PAL_HBLANK                        0x120  // 0xF4 //(864-720)
#define BT865_NTSC_HBLANK                       0xF4   // 0xF4 //(858-720)
#define BT865_PAL_VBLANK                        0x17   // 16   //(625-576)
#define BT865_NTSC_VBLANK                       0x13   // 16   //(525-480)

#define BT861_PAL_HBLANK                        0x120    // 0xF4
#define BT861_NTSC_HBLANK                       0xF2     // 0xF2
#define BT861_PAL_VBLANK                        0x17     // 16
#define BT861_NTSC_VBLANK                       0x13     // 16

// The mode number is also used to determine the pixel type.
//    gnMode = 0 <==> 4bpp ARGB (ABGR)
//             1 <==> 4bpp AYUV (ACrCbY)
//             2 <==> 8bpp ARGB
//             3 <==> 8bpp AYUV
//             4 <==> 16bpp ARGB palette (4444)
//             5 <==> 16bpp AYUV direct (4444)
//             6 <==> 16bpp RGB palette (565)
//             7 <==> 16bpp YUV direct (655)
//             8 <==> 16bpp YUV direct (4:2:2)
#define OSD_MODE_4ARGB                          0
#define OSD_MODE_4AYUV                          1
#define OSD_MODE_8ARGB                          2
#define OSD_MODE_8AYUV                          3
#define OSD_MODE_16ARGB                         4
#define OSD_MODE_16AYUV                         5
#define OSD_MODE_16RGB                          6
#define OSD_MODE_16YUV655                       7
#define OSD_MODE_16YUV422                       8

#define OSD_HEADER_FIELD_STRIDE                 (OSD_BUF_FIELD_STRIDE |   \
                                                (OSD_BUF_FIELD_STRIDE << 16))

// Background color defines
//#ifdef DEBUG
#define BACKGROUND_Y                RGBTOY(0,0,255)
#define BACKGROUND_CR               RGBTOCR(0,0,255)
#define BACKGROUND_CB               RGBTOCB(0,0,255)
//#else /* DEBUG */
//#define BACKGROUND_Y                RGBTOY(0,0,0)
//#define BACKGROUND_CR               RGBTOCR(0,0,0)
//#define BACKGROUND_CB               RGBTOCB(0,0,0)
//#endif /* DEBUG */

// OSD driver type definitions
// MAX_BPP controls the mode table, palette, frame buffer, ...
// Valid entries are 4, 8, or 16
#define MAX_BPP                                 16

#if MAX_BPP == 4
#define NUM_OSD_MODES                           2
#elif MAX_BPP == 8
#define NUM_OSD_MODES                           4
#elif MAX_BPP == 16
#define NUM_OSD_MODES                           9
#endif

#define OSD_MAX_HEIGHT                          576
#define OSD_MAX_WIDTH                           720

#define OSD_BUF_STRIDE                          (OSD_MAX_WIDTH*MAX_BPP/8)
#define OSD_BUF_FIELD_STRIDE                    (OSD_BUF_STRIDE*2)


#define MAX_OSD_RGN_ALPHA                 0x3F
#define DEFAULT_PALETTE_ALPHA             MAX_OSD_RGN_ALPHA

// OSD Region Options - dwOptions
#define OSD_RO_LOADPALETTE                0x00000001
#define OSD_RO_BSWAPACCESS                0x00000002
#define OSD_RO_FFENABLE                   0x00000004
#define OSD_RO_ALPHAENABLE                0x00000008
#define OSD_RO_COLORKEYENABLE             0x00000010
#define OSD_RO_ARCENABLE                  0x00000020
#define OSD_RO_ALPHATOPVIDEO              0x00000040
#define OSD_RO_FORCEREGIONALPHA           0x00000080
#define OSD_RO_4BPPPALINDEX               0x00000100
#define OSD_RO_ENABLE                     0x00000200
#define OSD_RO_FRAMEBUFFER                0x00000400 // Read only
#define OSD_RO_FRAMESTRIDE                0x00000800 // Read only
#define OSD_RO_MODE                       0x00001000 // Read only
#define OSD_RO_ALPHABOTHVIDEO             0x00002000
#define OSD_RO_BUFFERHEIGHT               0x00004000 // Read only
#define OSD_RO_BUFFERWIDTH                0x00008000 // Read only
#define OSD_RO_PALETTEADDRESS             0x00010000 // Read only

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
   int            nTop;       // Screen pos and size
   int            nLeft;      // Screen pos and size
   int            nRight;     // Screen pos and size
   int            nBottom;    // Screen pos and size
   u_int32        dwW;        // Width of allocated buffer
   u_int32        dwH;        // Height of allocated buffer
   u_int32        dwStride;   // Stride of allocated buffer
   u_int32        dwNumPalEntries;
   u_int32        dwMode;     // Index into mode table
   u_int32        dwOptions;
   u_int32        dwPalIndex; // Used only in 4bpp
   u_int32        dwRgnAlpha;
   u_int32        dwUnAlignedHdr; // Unaligned ptr from alloc (cache alignment)
   u_int32        dwUnAlignedPal;
   u_int32        dwUnAlignedImage;

   OSDHEADER      OSDHdr;
   PDRMPAL        pPalette;
   LPVOID         pImage;
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

////////////////////////////////////////////////////////////////////////////////
// Function prototypes
////////////////////////////////////////////////////////////////////////////////
// Exported OSD function prototypes
void OSDLibInit(u_int32 dwOSDMaxHeight, u_int16 uVideoFormat);
OSDHANDLE CreateOSDRegion(POSDRECT lpRect, u_int32 dwPixelMode, u_int32 dwOptions);
bool DestroyOSDRegion(OSDHANDLE hRgn);
bool SetOSDRgnOptions(OSDHANDLE hRgn, u_int32 dwFlags, u_int32 dwValue);
u_int32 GetOSDRgnOptions(OSDHANDLE hRgn, u_int32 dwFlags);
bool SetOSDRgnRect(OSDHANDLE hRgn, POSDRECT lpRect);
bool GetOSDRgnRect(OSDHANDLE hRgn, POSDRECT lpRect);
u_int8 GetOSDRgnBpp(OSDHANDLE hRgn);
u_int32 SetOSDRgnPalette(OSDHANDLE hRgn, PDRMPAL pPal, bool bLoadAlpha);
bool SetOSDRgnAlpha(OSDHANDLE hRgn, OSDALPHAFLAG Flags, u_int32 dwAlpha, u_int32 dwPalIndex);
bool SetOSDColorKey(u_int32 dwColorKey);
u_int32 GetOSDColorKey(void);
bool SetOSDBackground(DRM_COLOR yccBackground);
DRM_COLOR GetOSDBackground(void);
u_int32 SetDefaultOSDPalette(OSDHANDLE hRgn);
DRMPAL GetOSDRgnPalEntry(OSDHANDLE hRgn, u_int32 dwIndex);
bool SetOSDRgnPalEntry(OSDHANDLE hRgn, u_int32 dwIndex, DRMPAL drmPal);
u_int32 SetCursorInvertColor(DRMPAL rgbColor, bool bInvert);
void SetMpgRectClipped(POSDRECT prcSrc, POSDRECT prcDst);
void GetMpgRectClipped(POSDRECT prcSrc, POSDRECT prcDst);
bool ResizeOSDRegion(OSDHANDLE hRgn, POSDRECT lpRect);
void SetOutputType(u_int16 video_standard);
bool RectIntersect(POSDRECT prc1, POSDRECT prc2);
bool osdRegisterLineISR(POSDLINEISR pfnCallback, u_int32 dwLine, OSDFIELD oSignal);
void osdUnRegisterLineISR(POSDLINEISR pfnCallback, u_int32 dwLine);

////////////////////////////////////////////////////////////////////////////////
// Exported MPG function prototypes
void SetMpgRect(POSDRECT pRc);
void GetMpgRect(POSDRECT pRc);
void UpdateMpgScaling(void);
bool SetTopVideo(VIDEOPLANE video);
VIDEOPLANE GetTopVideo(void);
bool SetMpgAR(OSD_AR_MODE arMode, bool bUpdate);
OSD_AR_MODE GetMpgAR(void);
bool SetDisplayAR(OSD_DISPLAY_AR arDisplay, bool bUpdate);
OSD_DISPLAY_AR GetDisplayAR(void);
bool SetAspectRatio(OSD_DISPLAY_AR arDisplay, OSD_AR_MODE arMode);
bool GetAspectRatio(OSD_DISPLAY_AR *parDisplay, OSD_AR_MODE *parMode);
bool SetDisplayOutput(OSD_DISPLAY_OUTPUT odoType);
OSD_DISPLAY_OUTPUT GetDisplayOutput(void);

////////////////////////////////////////////////////////////////////////////////
// Internal function prototypes
bool UpdateOSDHeader(OSDHANDLE hRgn);
void GenerateOSDLinkList(void);
void OSDInit(void);
void EncoderInit(u_int16 video_standard);
void RLCopy2Screen(u_int8 *pDst, u_int8 *pSrcData, 
                   unsigned short *RunLengthOffsets,
                   int DstX, int DstY, int ExtX, int ExtY, 
                   int SrcX, int SrcY, LPVOID lpBuffer, 
                   u_int32 dwStride, u_int8 byBpp);
void GraphicsInit(void);

////////////////////////////////////////////////////////////////////////////////
// Exported BitBlt function prototypes
void osd_bitblit_mem2screen(OSDHANDLE hOSDRgn, u_int8 *data, u_int16 bytes_per_line, POSDRECT dst);
void osd_bitblit_fill(OSDHANDLE hOSDRgn, u_int32 quad, POSDRECT dst);
void osd_bitblit_xor_fill(OSDHANDLE hOSDRgn, u_int32 quad, POSDRECT dst);
int osd_bitblit_rl_fill(OSDHANDLE hOSDRgn, POSDPIXMAP pix, POSDRECT dst);
void osd_bitblit_screen2mem(OSDHANDLE hOSDRgn, u_int8 *data, u_int16 bytes_per_row, POSDRECT src);


#endif /* _OSDLIBN_H_ */




