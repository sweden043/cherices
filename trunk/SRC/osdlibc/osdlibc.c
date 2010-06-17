/****************************************************************************/
/*                   Conexant Proprietary and Confidential                  */
/*                       Software File/Module Header                        */
/*              Conexant Systems Inc. (c) 1998, 1999, 2000, 2001            */
/*                               Austin, TX                                 */
/*                            All Rights Eeserved                           */
/****************************************************************************/
/*
 * Filename:       osdlibw.c
 *
 *
 * Description:    Hardware API for the CX2443x ("Wabash") Display Refresh Module
 *                 This version supports both graphics planes of CX2443x
 *                 Taken from r1.109 of osdlibc.c and modified to support both
 *                 graphics planes
 *
 *
 * Author:         Rob Tilton (modified by Dave Wilson, Steve Glennon,
 *                             Quillian Rutherford, etc)
 *
 ****************************************************************************/
/* $Header: osdlibc.c, 120, 5/14/04 9:49:40 AM, Xin Golden$
 ****************************************************************************/
/* Functions:                                                                */
/***** Exported OSD Functions ************************************************/
/*    OSDLibInit()                                                           */
/*    CreateOSDRgn()                                                         */
/*    DestroyOSDRegion()                                                     */
/*    SetOSDRgnOptions()                                                     */
/*    GetOSDRgnOptions()                                                     */
/*    SwapOSDRegions()                                                       */
/*    SetOSDRgnRect()                                                        */
/*    GetOSDRgnRect()                                                        */
/*    GetOSDRgnBpp()                                                         */
/*    SetOSDRgnPalette()                                                     */
/*    SetOSDRgnAlpha()                                                       */
/*    SetOSDColorKey()                                                       */
/*    GetOSDColorKey()                                                       */
/*    SetOSDBackground()                                                     */
/*    GetOSDBackground()                                                     */
/*    SetDefaultOSDPalette()                                                 */
/*    GetOSDRgnPalEntry()                                                    */
/*    SetOSDRgnPalEntry()                                                    */
/*    ResizeOSDRegion()                                                      */
/*    SetOutputType()                                                        */
/*    RectIntersect()                                                        */
/*    SetOSDBuffer()                                                         */
/***** Exported MPG Functions ************************************************/
/*    UpdateMpgScaling()                                                     */
/*    SetMpgAR()                                                             */
/*    GetMpgAR()                                                             */
/*    SetDisplayAR()                                                         */
/*    GetDisplayAR()                                                         */
/*    SetAspectRatio()                                                       */
/*    GetAspectRatio()                                                       */
/*    GetDisplayOutput()                                                     */
/*    SetDisplayOutput()                                                     */
/***** Internal Functions ****************************************************/
/*    GenerateOSDLinkList()                                                  */
/*    UpdateOSDHeader()                                                      */
/*    RLCopy2Screen()                                                        */
/*    GraphicsInit()                                                         */
/*    FBInit()                                                               */
/***** Exported BitBlit Functions ********************************************/
/*    osd_bitblit_mem2screen()                                               */
/*    osd_bitblit_fill()                                                     */
/*    osd_bitblit_xor_fill()                                                 */
/*    osd_bitblit_rl_fill()                                                  */
/*    osd_bitblit_screen2mem()                                               */
/*****************************************************************************/

/*****************/
/* Include Files */
/*****************/
#include "stbcfg.h"
#include "kal.h"
#define INCL_MPG
#define INCL_DRM
#define INCL_PLL
#define INCL_GPI
#include "gcpdefs.h"
#include "osdlib.h"
#include "osdisrc.h"
#include "vidlib.h"
#include "globals.h"
#include "confmgr.h"
#include "iic.h"
#include "gpioext.h"
#include "vidprvc.h"
#include "gfxlib.h"
#include "retcodes.h"
#include "drmfilter.h"
#include "osdprv.h"
#if defined(DRIVER_INCL_TVENC)
#include "tvenc.h"
#endif

/********************************/
/* Internal Function Prototypes */
/********************************/
void OSDDrvInit(void);
void GraphicsInit(void);

#ifdef OPENTV_12

/* Prototype for OpenTV 1.2 video driver size change callback */
void video_size_change(HVIDRGN hRgn, u_int32 dwWidth, u_int32 dwHeight);

#if CUSTOMER == VENDOR_D
extern bool gbStandbyMode;
#endif

#endif

#if CUSTOMER == VENDOR_C
   extern OSDHANDLE ghOSDRgn;
#endif


#define ERIC_RAYEL_HFILTER_MODE

/* Functions for bitblt. */
/* #define NO_HW 0 */
#define PC_SIM  1

extern void SetupFB(int pFB, int Bpp);

sem_id_t GCP_Sem_ID;

/* Global variables */
u_int32 guserVTapSelect = 0; /* @@@SG  DCS #1150 Changed to 0 as it was making */
                             /* stills look fuzzy in BSkyB tests (some sizes)  */
                             
POSDLIBREGION gpOsdLibList[2]  = {NULL, NULL};   /* Ptr to linked osd lib list, per plane     */
u_int32 gActiveOSDHdr[2]       = {1,1};          /* The currently active list of OSDs         */

bool    gbPendingOSDLLUpdate[2] = {FALSE,FALSE};  /* Update to linked list pending (per plane) */
u_int32 guPendingOSDHdr[2]      = {1,1};          /* Which header is pending (per plane)       */
#define OSD_LL_UPDATE_SAFETY (24)                 /* Number of lines before end of screen      */
                                                  /* that is danger area - we will wait for    */
                                                  /* next field to update linked list          */

u_int32        gdwSrcWidth = 720;
u_int32        gdwSrcHeight = 576;
u_int32        gdwAR = MPG_VID_ASPECT_43;
OSD_AR_MODE    gArMode = OSD_ARM_PANSCAN;
OSD_DISPLAY_AR gDisplayAR = OSD_DISPLAY_AR_43;
int32          gScartType = SCART_TYPE_NONE;
bool gbGammaCorrect = FALSE;

u_int32 gnHBlank;
u_int32 gnVBlank;

int gnOsdMaxHeight = 576;
int gnOsdMaxWidth = 720;
extern u_int32 gOSDPixelModes[];
bool    gbNechesB = TRUE;
extern unsigned long ChipRevID;

#ifdef DEBUG
int    giRgnCount = 0;
#endif

/* A semi-arbitrary line number in approx the middle of the picture. We use this */
/* line as a signal to tell us to write the DRM and MPG registers when the user  */
/* requests a change in aspect ratio handling from pan/scan to letterbox or vice */
/* versa.                                                                        */
int  giOsdNotifyInt5   = 0;
extern void vidPrepareMpgForAspectHandlingChange(u_int32 dwRgnIndex);

extern volatile u_int32 gnFieldCount;

/* This table was generated based on a gamma of 2.2 */
BYTE bGamma[256] = {0, 21, 28, 34, 39, 43, 46, 50, 53, 56, 59, 61, 64, 66, 68, 70,
                   72, 74, 76, 78, 80, 82, 84, 85, 87, 89, 90, 92, 93, 95, 96, 98,
                   99,101,102,103,105,106,107,109,110,111,112,114,115,116,117,118,
                  119,120,122,123,124,125,126,127,128,129,130,131,132,133,134,135,
                  136,137,138,139,140,141,142,143,144,144,145,146,147,148,149,150,
                  151,151,152,153,154,155,156,156,157,158,159,160,160,161,162,163,
                  164,164,165,166,167,167,168,169,170,170,171,172,173,173,174,175,
                  175,176,177,178,178,179,180,180,181,182,182,183,184,184,185,186,
                  186,187,188,188,189,190,190,191,192,192,193,194,194,195,195,196,
                  197,197,198,199,199,200,200,201,202,202,203,203,204,205,205,206,
                  206,207,207,208,209,209,210,210,211,212,212,213,213,214,214,215,
                  215,216,217,217,218,218,219,219,220,220,221,221,222,223,223,224,
                  224,225,225,226,226,227,227,228,228,229,229,230,230,231,231,232,
                  232,233,233,234,234,235,235,236,236,237,237,238,238,239,239,240,
                  240,241,241,242,242,243,243,244,244,245,245,246,246,247,247,248,
                  248,249,249,249,250,250,251,251,252,252,253,253,254,254,255,255};

u_int32 gOSDPixelModes[NUM_OSD_MODES] = {BPP4ARGBPAL, BPP4AYUVPAL
#if MAX_BPP >= 8  /* Add 8bpp */
                                               ,BPP8ARGBPAL, BPP8AYUVPAL
#endif /* MAX_BPP >= 8 */
#if MAX_BPP >= 16  /* Add 16bpp */
                                               ,BPP16ARGBPAL,
                                                BPP16AYUV,
                                                BPP16RGB565PAL,
                                                BPP16YUV655,
                                                BPP16YUV422,
                                                BPP16ARGB1555PAL,
                                                BPP16AYUV2644
#endif /* MAX_BPP == 16 */
#if MAX_BPP == 32  /* Add 32bpp */
                                               ,BPP32ARGB8888PAL,
                                               BPP32AYUV8888
#endif /* MAX_BPP == 32 */
};

#if defined(DRIVER_INCL_TVENC)
/* TV encoder driver handle which can be used in other application */ 
CNXT_TVENC_HANDLE gTvencHandle;
#endif

/*****************************************************************************/
/* Function: OSDInit()                                                       */
/*                                                                           */
/* Parameters: void                                                          */
/*                                                                           */
/* Returns: void                                                             */
/*                                                                           */
/* Description: Initializes internal data items.                             */
/*****************************************************************************/
void OSDInit(void)
{
   #ifdef DRIVER_INCL_CONFMGR
   sabine_config_data *pConfigData;
   #endif /* DRIVER_INCL_CONFMGR */
   HW_DWORD yccBackground;
   u_int16  video_standard = 0;
   u_int16  video_format = 0;
   static bool FirstTime = TRUE;
   #if defined(DRIVER_INCL_TVENC)
   CNXT_TVENC_CAPS tvencCaps;
   CNXT_TVENC_VIDEO_STANDARD Standard;
   CNXT_TVENC_STATUS eRetcode;
   #endif

   if (FirstTime == FALSE)
      return;
   FirstTime = FALSE;
   trace_new(OSD_FUNC_TRACE, "->OSDInit\n");

   /* Setup the scart signalling method. */
   /* BVIDBUG The current config method uses features, SCART_TYPE in this */
   /* particular case.  #ifdef'ing to remain compatible with old config */
   /* system for the time being. */
   #if SCART_TYPE == SCART_TYPE_LEGACY
      gScartType = SCART_TYPE_TDK_AVPRO_5002B;
   #else
      gScartType = SCART_TYPE;
   #endif
   
   #if defined(DRIVER_INCL_CONFMGR) && (VIDEO_OUTPUT_STANDARD_SWITCHABLE == YES)
   /* Get our non-volatile config data */
   pConfigData = config_lock_data();
   if (pConfigData)
   {
      video_standard = pConfigData->video_standard;
      video_format = pConfigData->video_format;
      config_unlock_data(pConfigData);
   }
   #else /* DRIVER_INCL_CONFMGR */
   /* The CONFMGR driver is not available so we use the default video      */
   /* format and standard as defined using software config file parameters */
   video_format   = VIDEO_FORMAT_DEFAULT;
   video_standard = VIDEO_OUTPUT_STANDARD_DEFAULT;
   #endif /* DRIVER_INCL_CONFMGR */

   /* Initialise the DRM filter coefficients */
   cnxt_drm_init_filter_coefficients();
   
   OSDIsrInit(); /* Init the ISR */

   /* Initialize the OSD library */
   OSDLibInit(gnOsdMaxHeight, video_format);

   /* Set the background color */
   CNXT_SET_VAL(&(yccBackground), DRM_COLOR_Y_MASK, BACKGROUND_Y);
   CNXT_SET_VAL(&(yccBackground), DRM_COLOR_CR_MASK, BACKGROUND_CR);
   CNXT_SET_VAL(&(yccBackground), DRM_COLOR_CB_MASK, BACKGROUND_CB);
   SetOSDBackground(yccBackground);

   #if defined(DRIVER_INCL_TVENC)
   /* Initialize the TV encoder driver*/
   cnxt_tvenc_init(NULL);
   tvencCaps.uLength = sizeof(CNXT_TVENC_CAPS);
   /* abtain a handle to the TV encoder driver */
   cnxt_tvenc_get_unit_caps( 0, &tvencCaps );
   cnxt_tvenc_open( &gTvencHandle, &tvencCaps, 0, 0 );

   if( video_standard == NTSC )
   {
      Standard = CNXT_TVENC_NTSC_M;
   }
   else if( (video_standard == PAL )||(video_standard == AUTO_DETECT) )
   {
      Standard = CNXT_TVENC_PAL_B_WEUR;
   }
   else if( video_standard == SECAM )
   {
      Standard = CNXT_TVENC_SECAM_L;
   }
   else
   {
      trace_new(OSD_ERROR_MSG, "OSD: Invalid video standard %d passed.", video_standard);
      error_log(ERROR_WARNING);
      return;
   }
   eRetcode = cnxt_tvenc_set_video_standard(gTvencHandle, Standard);
   if( eRetcode != CNXT_TVENC_OK )
   {
      trace_new(OSD_ERROR_MSG, "OSD: Video standared %d was not set successfully", video_standard);
      error_log(ERROR_WARNING);
      return;
   }
   #else
   SetOutputType(video_standard);
   #endif

	// ÀµÔÆÁ¼add for control color:
	//cnxt_tvenc_set_picture_control(gTvencHandle, CNXT_TVENC_CONTRAST, 20);
   	//cnxt_tvenc_set_picture_control(gTvencHandle, CNXT_TVENC_BRIGHTNESS, -10);
	
   GraphicsInit();

   #ifdef DRIVER_INCL_OSD
   /* Init the OpenTv OSD driver. */
   OSDDrvInit();
   #endif /* DRIVER_INCL_OSD */
   #ifdef DRIVER_INCL_OSD1_2
   /* Init the OpenTv OSD driver. */
   OSDDrvInit();
   #endif /* DRIVER_INCL_OSD1_2 */

   /* For Neches, turn on blending with background color by default. */
   CNXT_SET_VAL(glpDrmControl, DRM_CONTROL_BLEND_BACKGROUND_MASK, 1);

   /* Colorado and Wabash have a scaler filter coefficient problem in the DRM      */
   /* which causes upscaled images to occasionally exhibit an lsb problem. This    */
   /* results in faint but perceptible vertical lines in the image. The following  */
   /* filter settings smooth the luma just enough to get rid of this while keeping */
   /* overall image quality as high as possible.                                   */
   #if ((DRM_SCALER_COEFF_TYPE == DRM_SCALER_COEFF_TYPE_COLORADO) && \
        (DRM_FILTER_PHASE_0_PROBLEM == YES))
   *(LPREG)glpDrmSharp[0] = 0x007017e1;
   *(LPREG)glpDrmSharp[1] = 0x007017e1;
   #elif (DRM_SCALER_COEFF_TYPE == DRM_SCALER_COEFF_TYPE_WABASH)
   if (ChipRevID >= PCI_REV_ID_C_WABASH) {
       *(LPREG)glpDrmSharp[0] = 0x00000010;
       *(LPREG)glpDrmSharp[1] = 0x00000010;
   } else {     
       *(LPREG)glpDrmSharp[0] = 0x007017e1;
       *(LPREG)glpDrmSharp[1] = 0x007017e1;
   }
   #else
   *(LPREG)glpDrmSharp[0] = 0x00000010;
   *(LPREG)glpDrmSharp[1] = 0x00000010;
   #endif


   #if CUSTOMER == VENDOR_C
   SetOSDRgnOptions(ghOSDRgn, OSD_RO_FFENABLE, TRUE);
   #endif

   /* This init is being pulled out of osdInit so the TTX  */
   /* api can be made more generic.  osdTTXInit must now   */
   /* be explicitly initialized ! AFTER ! osdInit();       */
   /* osdTTXInit();                                        */
   trace_new(OSD_FUNC_TRACE, "<-OSDInit\n");

   return;
}


/*****************************************************************************/
/* Function: OSDLibInit()                                                    */
/*                                                                           */
/* Parameters: u_int32 dwOSDMaxHeight - Maximum OSD height                   */
/*             u_int16 uVideoFormat - Pan & scan or letterbox                */
/*                                                                           */
/* Returns: void                                                             */
/*                                                                           */
/* Description: Initializes the OSD hardware.  Called from OSDInit().        */
/*****************************************************************************/
void OSDLibInit(u_int32 dwOSDMaxHeight, u_int16 uVideoFormat)
{
   bool ksPrev;
   OSD_AR_MODE arMode;

   trace_new(OSD_FUNC_TRACE, "->OSDLibInit\n");

   if (uVideoFormat == PANSCAN)
      arMode = OSD_ARM_PANSCAN;
   else
      arMode = OSD_ARM_LETTERBOX;
   SetMpgAR(arMode, FALSE);

   ksPrev = critical_section_begin();
   CNXT_SET_VAL(glpCtrl0, MPG_CONTROL0_ENABLEASPECTCONVERT_MASK, 1);
   CNXT_SET_VAL(glpCtrl0, MPG_CONTROL0_ENABLEPANSCAN_MASK, (uVideoFormat == PANSCAN) ? 1 : 0);
   critical_section_end(ksPrev);

   CNXT_SET_VAL(glpDrmDWIControl1, DRM_DWI_CONTROL_1_MEM_WORD_GROUPS_MASK, 0x8);
   CNXT_SET_VAL(glpDrmDWIControl1, DRM_DWI_CONTROL_1_PAL_WORD_GROUPS_MASK, 0x8);
   CNXT_SET_VAL(glpDrmDWIControl1, DRM_DWI_CONTROL_1_PAL_REQ_ENABLE_MASK, 0);
   #if GRAPHICS_PLANES == GP_DUAL
   CNXT_SET_VAL(glpDrmDWIControl1, DRM_DWI_CONTROL_1_OSD1_CRIT_MASK, 0x20);
   #endif
   CNXT_SET_VAL(glpDrmDWIControl1, DRM_DWI_CONTROL_1_G1_CRIT_MASK, 0x20);
   CNXT_SET_VAL(glpDrmDWIControl1, DRM_DWI_CONTROL_1_G2_CRIT_MASK, 0x20);
   
   CNXT_SET_VAL(glpDrmDWIControl2, DRM_DWI_CONTROL_2_EXT_VID_CRIT_MASK, 32);
   CNXT_SET_VAL(glpDrmDWIControl2, DRM_DWI_CONTROL_2_MPEG_CHROMA_CRIT_MASK, 0x40);
   CNXT_SET_VAL(glpDrmDWIControl2, DRM_DWI_CONTROL_2_MPEG_LUMA1_CRIT_MASK, 0x20);
   CNXT_SET_VAL(glpDrmDWIControl2, DRM_DWI_CONTROL_2_SPLIT_FIFO_LUMA_MASK, 0);
   CNXT_SET_VAL(glpDrmDWIControl2, DRM_DWI_CONTROL_2_SPLIT_FIFO_CHROMA_MASK, 0);
   CNXT_SET_VAL(glpDrmDWIControl2, DRM_DWI_CONTROL_2_MPEG_LUMA2_CRIT_MASK, 0x20);

   CNXT_SET_VAL(glpDrmDWIControl3, DRM_DWI_CONTROL_2_EXT_VID_CRIT_MASK, 32);
   CNXT_SET_VAL(glpDrmDWIControl3, DRM_DWI_CONTROL_2_MPEG_CHROMA_CRIT_MASK, 0x40);
   CNXT_SET_VAL(glpDrmDWIControl3, DRM_DWI_CONTROL_2_MPEG_LUMA1_CRIT_MASK, 0x20);
   CNXT_SET_VAL(glpDrmDWIControl3, DRM_DWI_CONTROL_2_SPLIT_FIFO_LUMA_MASK, 0);
   CNXT_SET_VAL(glpDrmDWIControl3, DRM_DWI_CONTROL_2_SPLIT_FIFO_CHROMA_MASK, 0);
   CNXT_SET_VAL(glpDrmDWIControl3, DRM_DWI_CONTROL_2_MPEG_LUMA2_CRIT_MASK, 0x20);

   CNXT_SET_VAL(glpDrmControl, DRM_CONTROL_VIDEO_TOP_MASK, 1); /* Set the mpeg video as top! */

   vidLibInit();
/*   osdTTXInit(); */

   trace_new(OSD_FUNC_TRACE, "<-OSDLibInit\n");
}

/*****************************************************************************/
/* Function: get_mode_alignment()                                            */
/*                                                                           */
/* Parameters: u_int32 dwPixelMode - Pixel mode of region to create          */
/*                                                                           */
/* Returns: number of bytes if alignment required for the                    */
/*          stride and buffer pointer of an OSD region                       */
/*                                                                           */
/* Description: returns number of bytes if alignment required for the        */
/*          stride and buffer pointer of an OSD region                       */
/*****************************************************************************/
u_int32 get_mode_alignment(u_int32  dwPixelMode)
{
    u_int32       uAlignment;

    switch (dwPixelMode)
    {
        case OSD_MODE_4ARGB:
        case OSD_MODE_4AYUV:
            uAlignment = 4;
            break;
        case OSD_MODE_8ARGB:
        case OSD_MODE_8AYUV:
            uAlignment = 4;
            break;
        case OSD_MODE_16ARGB:
        case OSD_MODE_16AYUV:
        case OSD_MODE_16RGB:
        case OSD_MODE_16YUV655:
        case OSD_MODE_16YUV422:
        case OSD_MODE_16ARGB1555:
        case OSD_MODE_16AYUV2644:
            uAlignment = 8;
            break;
        case OSD_MODE_32ARGB:
        case OSD_MODE_32AYUV:
            uAlignment = 16;
            break;
        default:
            /* should not be here because all the cases should be covered by above */
            uAlignment = 4;  
            error_log(ERROR_WARNING); 
            break;
    }

    return uAlignment;
}
    

/*****************************************************************************/
/* Function: CreateOSDRgn()                                                  */
/*                                                                           */
/* Parameters: POSDRECT lpRect - Ptr to rect describing the region           */
/*             u_int32 dwPixelMode - Pixel mode of region to create          */
/*             u_int32 dwOptions - Region options                            */
/*             LPVOID lpImage - Ptr to buffer to use as the image buffer.    */
/*             u_int32 dwStride - Stride in bytes of the buffer of lpImage.  */
/*                                                                           */
/* Returns: OSDHANDLE if successful or false otherwise.                      */
/*                                                                           */
/* Description: Creates and initializes an OSD region.                       */
/*              If lpImage is NULL, an image buffer is allocated.            */
/*              If lpImage is not NULL, then the region will use the buffer  */
/*              pointed to by lpImage, and the library will not allocate an  */
/*              image buffer.                                                */
/*              dwStride is ignored if lpImage is NULL.                      */
/*****************************************************************************/
OSDHANDLE CreateOSDRgn(POSDRECT lpRect,
                       u_int32  dwPixelMode,
                       u_int32  dwOptions,
                       LPVOID   lpImage,
                       u_int32  dwStride)
{
   OSDHANDLE     hRet = (OSDHANDLE)NULL;
   POSDLIBREGION pRgn;
   OSDRECT       rectRegion;
   u_int32       uAlignment;
   u_int32       uByteWidth;
   u_int32       uPlane = 0;      /* OSD Plane - 0 for primary, 1 for secondary */
   
   /* Check for plane - must be 0 on non-dual plane (Wabash/Cabo/Kobuk) devices */
   if (dwOptions & OSD_RO_PLANE1)
   {
      #if GRAPHICS_PLANES == GP_DUAL
      
      /* Second plane does not support flicker filter or Aspect Ratio Conversion */
      /* Obscure plane 1 is only valid on plane 0                                */
      uPlane = 1;
      if (0 != (dwOptions & (OSD_RO_FFENABLE | 
                             OSD_RO_ARCENABLE | 
                             OSD_RO_OBSCUREPLANE1)))
      {
         return hRet;
         
      } /* endif */
      
      #else
      return hRet;
      #endif      
   
   } 
   
   trace_new(OSD_FUNC_TRACE, "->CreateOSDRgn\n");
   if ((pRgn = (POSDLIBREGION)mem_nc_malloc(sizeof(OSDLIBREGION)))!=0)
   {
      pRgn->dwW = lpRect->right - lpRect->left;
      pRgn->dwH = lpRect->bottom - lpRect->top;

      /* Find out the number of bytes if alignment required for the */
      /* stride and buffer pointer                                  */
      uAlignment = get_mode_alignment(dwPixelMode);

      switch (dwPixelMode)
      {
      case OSD_MODE_4ARGB:
      case OSD_MODE_4AYUV:
         uByteWidth = (pRgn->dwW + 1)/2;
         pRgn->dwNumPalEntries = 16;
         break;
      case OSD_MODE_8ARGB:
      case OSD_MODE_8AYUV:
         uByteWidth = pRgn->dwW;
         pRgn->dwNumPalEntries = 256;
         break;
      case OSD_MODE_16ARGB:
         uByteWidth = pRgn->dwW << 1;
         pRgn->dwNumPalEntries = 16;
         break;
      case OSD_MODE_16AYUV:
         uByteWidth = pRgn->dwW << 1;
         pRgn->dwNumPalEntries = 0;
         break;
      case OSD_MODE_16RGB:
         uByteWidth = pRgn->dwW << 1;
         pRgn->dwNumPalEntries = 64;
         break;
      case OSD_MODE_16YUV655:
         uByteWidth = pRgn->dwW << 1;
         pRgn->dwNumPalEntries = 0;
         break;
      case OSD_MODE_16YUV422:
         uByteWidth = pRgn->dwW << 1;
         pRgn->dwNumPalEntries = 0;
         break;
      case OSD_MODE_16ARGB1555:
         uByteWidth = pRgn->dwW << 1;
         pRgn->dwNumPalEntries = 32;
         break;
      case OSD_MODE_16AYUV2644:
         uByteWidth = pRgn->dwW << 1;
         pRgn->dwNumPalEntries = 0;
         break;
      case OSD_MODE_32ARGB:
         uByteWidth = pRgn->dwW << 2;
         pRgn->dwNumPalEntries = 256;
         break;
      case OSD_MODE_32AYUV:
         uByteWidth = pRgn->dwW << 2;
         pRgn->dwNumPalEntries = 0;
         break;
      default:
         return hRet;
      }

      pRgn->dwStride = (uByteWidth + (uAlignment-1)) & (~(uAlignment-1));

      if (lpImage)
         pRgn->dwStride = dwStride;

      hRet = (OSDHANDLE)pRgn;
      pRgn->pPrev = NULL;
      pRgn->pImage = lpImage;
      pRgn->pAllocatedImage = NULL;
      pRgn->pPalette = NULL;
      pRgn->nTop = lpRect->top;
      pRgn->nLeft = lpRect->left;
      pRgn->nRight = lpRect->right;
      pRgn->nBottom = lpRect->bottom;
      pRgn->dwMode = dwPixelMode;
      pRgn->dwOptions = dwOptions;
      pRgn->dwPalIndex = 0;
      pRgn->dwRgnAlpha = MAX_OSD_RGN_ALPHA;
      pRgn->OSDHdr1.pNextOsd = 0;
      pRgn->OSDHdr2.pNextOsd = 0;

      /* Create the memory for the palette, pad for cache alignment,
       * and move to non-cache region.
       */
      if (pRgn->dwNumPalEntries)
         pRgn->pPalette = (PDRMPAL)mem_nc_malloc(
            (pRgn->dwNumPalEntries * sizeof(DRMPAL)));

      /* Create the memory for the image, pad for cache alignment,
       * and move to non-cache region.
       */
      if (lpImage == NULL)
      {
         #ifdef OPENTV_12
         pRgn->pAllocatedImage = (LPVOID)vidImageBufferAlloc((pRgn->dwStride * pRgn->dwH), &(pRgn->bTransient), FALSE);
         if(pRgn->pAllocatedImage)
           pRgn->pAllocatedImage = (LPVOID)SET_FLAGS(pRgn->pAllocatedImage, NCR_BASE);
         pRgn->pImage = pRgn->pAllocatedImage;
         #else
         pRgn->pAllocatedImage = (LPVOID)mem_nc_malloc(
            ((pRgn->dwStride * pRgn->dwH) + (uAlignment-1)));
         pRgn->pImage =(LPVOID)(((u_int32)pRgn->pAllocatedImage + (uAlignment-1)) & ~(uAlignment-1));
         #endif
      }

      /* Make sure that everything has been created successfully.  If anything
       * is missing, free all that has been allocated and return in disgrace.
       */
      if (((pRgn->dwNumPalEntries) && (!pRgn->pPalette)) ||
         (!pRgn->pImage))
      {
         if ((pRgn->dwNumPalEntries) && (!pRgn->pPalette))
            trace_new(OSD_ERROR_MSG, "OSD ERROR: Failed palette alloc!\n");
         if (!pRgn->pImage)
            trace_new(OSD_ERROR_MSG, "OSD ERROR: Failed image alloc!\n");

         if (pRgn->pImage)
         {
            #ifdef OPENTV_12
            vidImageBufferFree((void *)CLEAR_FLAGS(pRgn->pAllocatedImage, NCR_BASE), pRgn->bTransient);
            #else
            mem_nc_free((void *)pRgn->pAllocatedImage);
            #endif
         }
         if (pRgn->pPalette)
            mem_nc_free((void *)pRgn->pPalette);

         mem_nc_free((void *)pRgn);
         hRet = (OSDHANDLE)NULL;
      }
      else /* All is well. */
      {
         if ((pRgn->pNext = gpOsdLibList[uPlane])!=0)
         {
            pRgn->pNext->pPrev = pRgn;
         }   
         gpOsdLibList[uPlane] = pRgn;
         pRgn->uPlane = uPlane;

         /* Need to setup the osd header. */
         UpdateOSDHeader(hRet);

         /* Set the default palette. */
         SetDefaultOSDPalette(hRet);

         /* If the buffer was not allocated here, 
          * or NOCLEARBUFFER specified 
          * don't fill it with blackness.
          */
         if ((lpImage == NULL) && ((dwOptions & OSD_RO_NOCLEARBUFFER) == 0))
         {
            /* Fill the buffer with lots of nothing. */
            rectRegion.left   = 0;
            rectRegion.top    = 0;
            rectRegion.right  = pRgn->dwW;
            rectRegion.bottom = pRgn->dwH;
            osd_bitblit_fill(hRet, 0, &rectRegion);
         }

         /* Add this to the list if it is enabled. */
         if (dwOptions & OSD_RO_ENABLE)
            GenerateOSDLinkList(uPlane);

#ifdef VERBOSE_DEBUG
         trace_new(OSD_MSG, "pRgn = 0x%08X\n", (u_int32)pRgn);
         trace_new(OSD_MSG1, "pRgn->pPrev = 0x%08X\n", pRgn->pPrev);
         trace_new(OSD_MSG1, "pRgn->pNext = 0x%08X\n", pRgn->pNext);
         trace_new(OSD_MSG, "pRgn->nTop = %d\n", pRgn->nTop);
         trace_new(OSD_MSG, "pRgn->nLeft = %d\n", pRgn->nLeft);
         trace_new(OSD_MSG1, "pRgn->nRight = %d\n", pRgn->nRight);
         trace_new(OSD_MSG1, "pRgn->nBottom = %d\n", pRgn->nBottom);
         trace_new(OSD_MSG, "pRgn->dwW = 0x%08X\n", pRgn->dwW);
         trace_new(OSD_MSG, "pRgn->dwH = 0x%08X\n", pRgn->dwH);
         trace_new(OSD_MSG1, "pRgn->dwStride = 0x%08X\n", pRgn->dwStride);
         trace_new(OSD_MSG1, "pRgn->dwNumPalEntries = 0x%08X\n", pRgn->dwNumPalEntries);
         trace_new(OSD_MSG, "pRgn->dwMode = 0x%08X\n", pRgn->dwMode);
         trace_new(OSD_MSG1, "pRgn->dwOptions = 0x%08X\n", pRgn->dwOptions);
         trace_new(OSD_MSG1, "pRgn->dwPalIndex = 0x%08X\n", pRgn->dwPalIndex);
         trace_new(OSD_MSG1, "pRgn->dwRgnAlpha = 0x%08X\n", pRgn->dwRgnAlpha);
         trace_new(OSD_MSG, "pRgn->pPalette = 0x%08X\n", (u_int32)pRgn->pPalette);
         trace_new(OSD_MSG, "pRgn->pImage = 0x%08X\n", (u_int32)pRgn->pImage);
         trace_new(OSD_MSG, "pRgn->pAllocatedImage = 0x%08X\n", (u_int32)pRgn->pAllocatedImage);
#else
         trace_new(OSD_MSG,
                   "%d x %d at (%d, %d). %d colours, stride %d, buffer at 0x%08X/0x%08X, handle 0x%08X\n",
                   pRgn->dwW, pRgn->dwH, pRgn->nLeft, pRgn->nTop,
                   pRgn->dwNumPalEntries, pRgn->dwStride, pRgn->pImage, pRgn->pAllocatedImage, pRgn);
#endif
         #ifdef DEBUG
         giRgnCount++;
         trace_new(OSD_MSG, "%d OSD regions now exist\n", giRgnCount);
         #endif
      }
   }
   else
   {
      trace_new(OSD_ERROR_MSG, "OSD ERROR: Failed OSDLIBREGION alloc!\n");
   }

   trace_new(OSD_FUNC_TRACE, "<-CreateOSDRgn\n");
   return hRet;
}

/*****************************************************************************/
/* Function: DestroyOSDRegion()                                              */
/*                                                                           */
/* Parameters: OSDHANDLE hRgn - Handle to OSD region to be destroyed.        */
/*                                                                           */
/* Returns: bool - TRUE on success.                                          */
/*                                                                           */
/* Description: Destroys an OSD region.                                      */
/*****************************************************************************/
bool DestroyOSDRegion(OSDHANDLE hRgn)
{
   bool bRet = FALSE;
   POSDLIBREGION pRgn;
   int nFieldCount;

   trace_new(OSD_FUNC_TRACE, "->DestroyOSDRegion 0x%08X\n", (u_int32)hRgn);

   if ((pRgn = (POSDLIBREGION)hRgn)!=0)
   {
      /* If the region was enabled, disable it and generate a new list prior
       * to deletetion.
       */
      if (pRgn->dwOptions & OSD_RO_ENABLE)
      {
         pRgn->dwOptions &= ~OSD_RO_ENABLE;
         GenerateOSDLinkList(pRgn->uPlane);
      }
      if (pRgn->pNext)
         pRgn->pNext->pPrev = pRgn->pPrev;

      if (pRgn->pPrev)
         pRgn->pPrev->pNext = pRgn->pNext;
      else
         gpOsdLibList[pRgn->uPlane] = pRgn->pNext;

      /* Wait for the field change to load the DRM registers.
       * We don't want to delete any memory if DRM is still using it.
       */
      #if (CUSTOMER == VENDOR_D) && (defined OPENTV_12)
      /* Vendor D turns off the encoder in standby mode so this code will */
      /* hang here until the encoder is turned back on if we wait for the */
      /* next field while in standby.                                     */
      if (!gbStandbyMode)
      {
        nFieldCount = gnFieldCount;
        while (nFieldCount == gnFieldCount);
      }
      #else
      nFieldCount = gnFieldCount;
      while (nFieldCount == gnFieldCount);
      #endif

      if (pRgn->pAllocatedImage)
      {
        #ifdef OPENTV_12
        vidImageBufferFree((void *)CLEAR_FLAGS(pRgn->pAllocatedImage, NCR_BASE), pRgn->bTransient);
        #else
        mem_nc_free((void *)pRgn->pAllocatedImage);
        #endif
      }
      if (pRgn->pPalette)
         mem_nc_free((void *)pRgn->pPalette);

      mem_nc_free((void *)pRgn);
      bRet = TRUE;

      #ifdef DEBUG
      giRgnCount--;
      trace_new(OSD_MSG, "%d OSD regions now exist\n", giRgnCount);
      #endif
   }
   else
   {
      trace_new(OSD_ERROR_MSG, "ERROR: Invalid handle - 0x%08X\n", hRgn);
   }

   trace_new(OSD_FUNC_TRACE, "<-DestroyOSDRegion\n");
   return bRet;
}

/*****************************************************************************/
/* Function: SetOSDRgnOptions()                                              */
/*                                                                           */
/* Parameters: OSDHANDLE hRgn - Handle to OSD region.                        */
/*             u_int32   dwFlags - The options to change.                    */
/*             u_int32   dwValue - New option value.                         */
/*                                                                           */
/* Returns: bool - TRUE on success.                                          */
/*                                                                           */
/* Description: Changes the options for an OSD region.                       */
/*****************************************************************************/
bool SetOSDRgnOptions(OSDHANDLE hRgn, u_int32 dwFlags, u_int32 dwValue)
{
   bool          bRet = TRUE;
   POSDLIBREGION pRgn;
   u_int32       dwSavedOptions;
   bool          bNeedUpdate = FALSE;

   trace_new(OSD_FUNC_TRACE1, "->SetOSDRgnOptions\n");
   trace_new(OSD_MSG1, "dwFlags = 0x%08X, dwValue = 0x%08X\n", dwFlags, dwValue);

   if ((pRgn = (POSDLIBREGION)hRgn)!=0)
   {
      /* Save the previous options in case one of the flags is invalid */
      /* We will undo the changes if we find an invalid option.        */
      dwSavedOptions = pRgn->dwOptions;

      /* Go through each of the options updating the region */
      if (dwFlags & OSD_RO_LOADPALETTE)
      {
         if (dwValue)
         {
            pRgn->dwOptions |= OSD_RO_LOADPALETTE;
         }
         else
         {
            pRgn->dwOptions &= ~OSD_RO_LOADPALETTE;
         }
         bNeedUpdate = TRUE;
      }
      if (dwFlags & OSD_RO_BSWAPACCESS)
      {
         if (dwValue)
         {
            pRgn->dwOptions |= OSD_RO_BSWAPACCESS;
         }
         else
         {
            pRgn->dwOptions &= ~OSD_RO_BSWAPACCESS;
         }
         bNeedUpdate = TRUE;
      }
      if (dwFlags & OSD_RO_ALPHAENABLE)
      {
         if (dwValue)
         {
            pRgn->dwOptions |= OSD_RO_ALPHAENABLE;
         }
         else
         {
            pRgn->dwOptions &= ~OSD_RO_ALPHAENABLE;
         }
         bNeedUpdate = TRUE;
      }
      if (dwFlags & OSD_RO_COLORKEYENABLE)
      {
         if (dwValue)
         {
            pRgn->dwOptions |= OSD_RO_COLORKEYENABLE;
         }
         else
         {
            pRgn->dwOptions &= ~OSD_RO_COLORKEYENABLE;
         }
         bNeedUpdate = TRUE;
      }
      if (dwFlags & OSD_RO_ALPHATOPVIDEO)
      {
         if (dwValue)
         {
            pRgn->dwOptions |= OSD_RO_ALPHATOPVIDEO;
         }
         else
         {
            pRgn->dwOptions &= ~OSD_RO_ALPHATOPVIDEO;
         }
         bNeedUpdate = TRUE;
      }
      if (dwFlags & OSD_RO_ALPHABOTHVIDEO)
      {
         if (dwValue)
         {
            pRgn->dwOptions |= OSD_RO_ALPHABOTHVIDEO;
         }
         else
         {
            pRgn->dwOptions &= ~OSD_RO_ALPHABOTHVIDEO;
         }
         bNeedUpdate = TRUE;
      }
      if (dwFlags & OSD_RO_FORCEREGIONALPHA)
      {
         if (dwValue)
         {
            pRgn->dwOptions |= OSD_RO_FORCEREGIONALPHA;
         }
         else
         {
            pRgn->dwOptions &= ~OSD_RO_FORCEREGIONALPHA;
         }
         bNeedUpdate = TRUE;
      }
      if (dwFlags & OSD_RO_4BPPPALINDEX)
      {
         if (dwValue > 3)
         {
            pRgn->dwPalIndex = 3;
         }
         else
         {
            pRgn->dwPalIndex = dwValue;
         }
         bNeedUpdate = TRUE;
      }

      if (dwFlags & OSD_RO_FORCEREGIONALPHA)
      {
         if (dwValue)
         {
            pRgn->dwOptions |= OSD_RO_FORCEREGIONALPHA;
         }
         else
         {
            pRgn->dwOptions &= ~OSD_RO_FORCEREGIONALPHA;
         }
         bNeedUpdate = TRUE;
      }
      /* Now do the things which are only valid for plane 0 */
      /* That includes FF, ARC, Blend with plane 1          */
      if (pRgn->uPlane == 0)
      {
         if (dwFlags & OSD_RO_FFENABLE)
         {
            if (dwValue)
            {
               pRgn->dwOptions |= OSD_RO_FFENABLE;
            }
            else
            {
               pRgn->dwOptions &= ~OSD_RO_FFENABLE;
            }
            bNeedUpdate = TRUE;
         }
         if (dwFlags & OSD_RO_ARCENABLE)
         {
            if (dwValue)
            {
               pRgn->dwOptions |= OSD_RO_ARCENABLE;
            }
            else
            {
               pRgn->dwOptions &= ~OSD_RO_ARCENABLE;
            }
            bNeedUpdate = TRUE;
         }
         #if GRAPHICS_PLANES == GP_DUAL
         if (dwFlags & OSD_RO_OBSCUREPLANE1)
         {
            /* OBSCUREPLANE1 only applies to plane 0 in dual plane IC */
            if (dwValue)
            {
               pRgn->dwOptions |= OSD_RO_OBSCUREPLANE1;
            }
            else
            {
               pRgn->dwOptions &= ~OSD_RO_OBSCUREPLANE1;
            }
            bNeedUpdate = TRUE;
         }
         #endif   
         
      } else {
         /* Plane 1: Check that nothing that was specified is invalid.   */
         /* If so, restore the pre-entry options value and fail the call */
         if (dwValue && 
             ((dwFlags & (OSD_RO_FFENABLE | 
                          OSD_RO_ARCENABLE | 
                          OSD_RO_OBSCUREPLANE1)) != 0) )
         {
            pRgn->dwOptions = dwSavedOptions;
            bNeedUpdate = FALSE;
            bRet = FALSE;
         
         } /* endif invalid option for plane 1 specified */
      
      } /* endif */

      /* Check that OSD_RO_PLANEx were not specified as not applicable after create */
      if ((dwFlags & (OSD_RO_PLANE0 | OSD_RO_PLANE1)) != 0)
      {
         pRgn->dwOptions = dwSavedOptions;
         bNeedUpdate = FALSE;
         bRet = FALSE;
      } /* endif */
      
      if (bNeedUpdate)
      {
         bRet = UpdateOSDHeader(hRgn);
      }

      if (bRet && (dwFlags & OSD_RO_ENABLE))
      {
         if (dwValue)
         {
            pRgn->dwOptions |= OSD_RO_ENABLE;
         }
         else
         {
            pRgn->dwOptions &= ~OSD_RO_ENABLE;
         }
         GenerateOSDLinkList(pRgn->uPlane);
      }
   }
   else
   {
      trace_new(OSD_ERROR_MSG, "ERROR: Invalid handle - 0x%08X\n", hRgn);
   }

   trace_new(OSD_FUNC_TRACE1, "<-SetOSDRgnOptions\n");

   return bRet;
}

/*****************************************************************************/
/* Function: GetOSDRgnOptions()                                              */
/*                                                                           */
/* Parameters: OSDHANDLE hRgn - Handle to OSD region.                        */
/*             u_int32   dwFlags - The options flag.                         */
/*                                                                           */
/* Returns: u_int32 - Value of the option.                                   */
/*                                                                           */
/* Description: Returns the options for an OSD region.                       */
/*****************************************************************************/
u_int32 GetOSDRgnOptions(OSDHANDLE hRgn, u_int32 dwFlags)
{
   u_int32 dwRet = 0;
   POSDLIBREGION pRgn;

   trace_new(OSD_FUNC_TRACE1, "->GetOSDRgnOptions\n");

   if ((pRgn = (POSDLIBREGION)hRgn)!=0)
   {
      switch (dwFlags)
      {
      case OSD_RO_LOADPALETTE:
      case OSD_RO_BSWAPACCESS:
      case OSD_RO_FFENABLE:
      case OSD_RO_ALPHAENABLE:
      case OSD_RO_COLORKEYENABLE:
      case OSD_RO_ARCENABLE:
      case OSD_RO_ALPHABOTHVIDEO:
      case OSD_RO_ALPHATOPVIDEO:
      case OSD_RO_FORCEREGIONALPHA:
      case OSD_RO_ENABLE:
      case OSD_RO_OBSCUREPLANE1:
         dwRet = (pRgn->dwOptions & dwFlags) ? TRUE : FALSE;
         break;
      case OSD_RO_4BPPPALINDEX:
         dwRet = pRgn->dwPalIndex;
         break;
      case OSD_RO_FRAMEBUFFER:
         dwRet = (u_int32)pRgn->pImage;
         break;
      case OSD_RO_FRAMESTRIDE:
         dwRet = pRgn->dwStride;
         break;
      case OSD_RO_MODE:
         dwRet = pRgn->dwMode;
         break;
      case OSD_RO_BUFFERHEIGHT:
         dwRet = pRgn->dwH;
         break;
      case OSD_RO_BUFFERWIDTH:
         dwRet = pRgn->dwW;
         break;
      case OSD_RO_PALETTEADDRESS:
         dwRet = (u_int32)pRgn->pPalette;
         break;
      }
   }
   else
   {
      trace_new(OSD_ERROR_MSG, "ERROR: Invalid handle - 0x%08X\n", hRgn);
   }

   trace_new(OSD_FUNC_TRACE1, "<-GetOSDRgnOptions\n");

   return dwRet;
}

/*****************************************************************************/
/* Function: SwapOSDRegions()                                                */
/*                                                                           */
/* Parameters: OSDHANDLE hNew - Region to be shown.                          */
/*             OSDHANDLE hOld - Region to be hidden.                         */
/*                                                                           */
/* Returns: bool - TRUE on success.                                          */
/*                                                                           */
/* Description: Disables hOld and enables hNew like the following code:      */
/*              SetOSDRgnOptions(hOld, OSD_RO_ENABLE, FALSE);                */
/*              SetOSDRgnOptions(hNew, OSD_RO_ENABLE, TRUE);                 */
/*              The difference being that GenerateOSDLinkList is only called */
/*              once by SwapOSDRegions, updating both changes to the screen  */
/*              simultaneously and avoiding flicker.                         */
/*                                                                           */
/*****************************************************************************/
bool SwapOSDRegions(OSDHANDLE hNew, OSDHANDLE hOld)
{
   bool          bRet = TRUE;
   POSDLIBREGION pNewRgn, pOldRgn;

   trace_new(OSD_FUNC_TRACE1, "->SwapOSDRegions\n");

   if ( ( (pNewRgn = (POSDLIBREGION)hNew) != 0 ) && ( (pOldRgn = (POSDLIBREGION)hOld) != 0 ) )
   {
         pNewRgn->dwOptions |= OSD_RO_ENABLE;
         pOldRgn->dwOptions &= ~OSD_RO_ENABLE;
         GenerateOSDLinkList(pNewRgn->uPlane); /* problems with > 1 plane? */
                                               /* only allow swapping regions on same plane? */
                                               /* or do one off - one on if different planes? */
   }
   else
   {
      trace_new(OSD_ERROR_MSG, "ERROR: Invalid handle - 0x%08X\n", ((hNew)?hOld:hNew));
   }

   trace_new(OSD_FUNC_TRACE1, "<-SetOSDRgnOptions\n");
   return bRet;
}

/*****************************************************************************/
/* Function: SetOSDRgnRect()                                                 */
/*                                                                           */
/* Parameters: OSDHANDLE hRgn - Handle to OSD region.                        */
/*             POSDRECT lpRect - New rgn rect.                               */
/*                                                                           */
/* Returns: bool - TRUE if the rgn rect was changed.                         */
/*                                                                           */
/* Description: Changes the screen position and size of an OSD region.       */
/*              Sizes larger than the allocated size are not allowed.        */
/*****************************************************************************/
bool SetOSDRgnRect(OSDHANDLE hRgn, POSDRECT lpRect)
{
   bool bRet = FALSE;
   POSDLIBREGION pRgn;

   trace_new(OSD_FUNC_TRACE, "->SetOSDRgnRect\n");

   if (((pRgn = (POSDLIBREGION)hRgn)!=0) && (lpRect))
   {
      if (((u_int32)(lpRect->right - lpRect->left) <= pRgn->dwW) &&
         ((u_int32)(lpRect->bottom - lpRect->top) <= pRgn->dwH))
      {
         pRgn->nTop = lpRect->top;
         pRgn->nLeft = lpRect->left;
         pRgn->nRight = lpRect->right;
         pRgn->nBottom = lpRect->bottom;

         /* Update the osd header. */
         UpdateOSDHeader(hRgn);

         /* Add this to the list if it is enabled. */
         if (pRgn->dwOptions & OSD_RO_ENABLE)
            GenerateOSDLinkList(pRgn->uPlane);

         bRet = TRUE;
      }
   }
   if (!hRgn)
      trace_new(OSD_ERROR_MSG, "ERROR: Invalid handle - 0x%08X\n", hRgn);
   if (!lpRect)
      trace_new(OSD_ERROR_MSG, "ERROR: Invalid lpRect - 0x%08X\n", lpRect);

   trace_new(OSD_FUNC_TRACE, "<-SetOSDRgnRect\n");

   return bRet;
}

/*****************************************************************************/
/* Function: GetOSDRgnRect()                                                 */
/*                                                                           */
/* Parameters: OSDHANDLE hRgn - Handle to OSD region.                        */
/*             POSDRECT lpRect - Ptr for result rect.                        */
/*                                                                           */
/* Returns: bool - TRUE if lpRect contains a valid rect.                     */
/*                                                                           */
/* Description: Returns the rect describing the screen coordinates of the    */
/*              OSD region.                                                  */
/*****************************************************************************/
bool GetOSDRgnRect(OSDHANDLE hRgn, POSDRECT lpRect)
{
   bool bRet = FALSE;
   POSDLIBREGION pRgn;

   trace_new(OSD_FUNC_TRACE, "->GetOSDRgnRect\n");

   if (((pRgn = (POSDLIBREGION)hRgn)!=0) && (lpRect))
   {
      lpRect->top = pRgn->nTop;
      lpRect->left = pRgn->nLeft;
      lpRect->right = pRgn->nRight;
      lpRect->bottom = pRgn->nBottom;

      bRet = TRUE;
   }
   if (!hRgn)
      trace_new(OSD_ERROR_MSG, "ERROR: Invalid handle - 0x%08X\n", hRgn);
   if (!lpRect)
      trace_new(OSD_ERROR_MSG, "ERROR: Invalid lpRect - 0x%08X\n", lpRect);

   trace_new(OSD_FUNC_TRACE, "<-GetOSDRgnRect\n");

   return bRet;
}

/*****************************************************************************/
/* Function: GetOSDRgnBpp()                                                  */
/*                                                                           */
/* Parameters: OSDHANDLE hRgn - Handle to OSD region.                        */
/*                                                                           */
/* Returns: u_int8 - The bits per pixel.                                     */
/*                                                                           */
/* Description: Returns the bpp of the given region.  0 if error.            */
/*****************************************************************************/
u_int8 GetOSDRgnBpp(OSDHANDLE hRgn)
{
   u_int8 uRet = 0;
   POSDLIBREGION pRgn;
   POSDHEADER pOsdHdr;

   trace_new(OSD_FUNC_TRACE1, "->GetOSDRgnBpp\n");

   if ((pRgn = (POSDLIBREGION)hRgn)!=0)
   {
      pOsdHdr = (POSDHEADER)&pRgn->OSDHdr1;
      uRet = 4 << ((pOsdHdr->dwXStartWidth & XSW_BPP_MASK) >> XSW_PTYPE_SHIFT);
   }
   else
   {
      trace_new(OSD_ERROR_MSG, "ERROR: Invalid handle - 0x%08X\n", hRgn);
   }

   trace_new(OSD_FUNC_TRACE1, "<-GetOSDRgnBpp\n");

   return uRet;
}

/*****************************************************************************/
/* Function: SetOSDRgnPalette()                                              */
/*                                                                           */
/* Parameters: OSDHANDLE hRgn - Handle to OSD region.                        */
/*             PDRMPAL pPal   - Pointer to palette data.                     */
/*             bool bLoadAlpha - Load alpha from palette data                */
/*                                                                           */
/* Returns: u_int32 - The number of palette entries set.                     */
/*                                                                           */
/* Description: Sets the palette for the OSD region.                         */
/*****************************************************************************/
u_int32 SetOSDRgnPalette(OSDHANDLE hRgn, PDRMPAL pPal, bool bLoadAlpha)
{
   u_int32 dwRet = 0;
   POSDLIBREGION pRgn;
   u_int32 dwNumEntries = 0;
   PDRMPAL lpDest = NULL;
   u_int32 dwCount;
   bool bGammaCorrect = FALSE;

   trace_new(OSD_FUNC_TRACE, "->SetOSDRgnPalette\n");
   trace_new(OSD_MSG1, "pPal = 0x%08X, bLoadAlpha = %d\n", pPal, bLoadAlpha);

   if ((pRgn = (POSDLIBREGION)hRgn)!=0)
   {
      switch (pRgn->dwMode)
      {
      case OSD_MODE_4ARGB:
         bGammaCorrect = TRUE;
         /* Fall thru. */
      case OSD_MODE_4AYUV:
         dwNumEntries = 16;
         if (pRgn->pPalette)
         {
            lpDest = pRgn->pPalette;
         }
         break;

      case OSD_MODE_8ARGB:
         bGammaCorrect = TRUE;
         /* Fall thru. */
      case OSD_MODE_8AYUV:
         dwNumEntries = 256;
         lpDest = pRgn->pPalette;
         break;

      case OSD_MODE_16ARGB:
         dwNumEntries = 16;
         lpDest = pRgn->pPalette;
         break;

      case OSD_MODE_16RGB:
         dwNumEntries = 64;
         lpDest = pRgn->pPalette;
         break;

      case OSD_MODE_16ARGB1555:
         dwNumEntries = 32;
         lpDest = pRgn->pPalette;
         break;

      case OSD_MODE_32ARGB:
         dwNumEntries = 256;
         lpDest = pRgn->pPalette;
         break;

      /* These formats do not use the hardware palette at all */
      case OSD_MODE_16AYUV2644:
      case OSD_MODE_16AYUV:
      case OSD_MODE_16YUV655:
      case OSD_MODE_16YUV422:
      case OSD_MODE_32AYUV:
         break;
      }

      if ((bGammaCorrect) && (gbGammaCorrect))
         trace_new(OSD_MSG, "Performing gamma correction.\n");

      for (dwCount=0; dwCount<dwNumEntries; dwCount++)
      {
         if ((bGammaCorrect) && (gbGammaCorrect))
         {
            CNXT_SET_VAL(&(lpDest[dwCount].rgb), DRMRGBPAL_R_MASK, bGamma[(u_int32)CNXT_GET_VAL(&pPal[dwCount].rgb, DRMRGBPAL_R_MASK)]);
            CNXT_SET_VAL(&(lpDest[dwCount].rgb), DRMRGBPAL_G_MASK, bGamma[(u_int32)CNXT_GET_VAL(&pPal[dwCount].rgb, DRMRGBPAL_G_MASK)]);
            CNXT_SET_VAL(&(lpDest[dwCount].rgb), DRMRGBPAL_B_MASK, bGamma[(u_int32)CNXT_GET_VAL(&pPal[dwCount].rgb, DRMRGBPAL_B_MASK)]);
            if (bLoadAlpha)
               CNXT_SET_VAL(&(lpDest[dwCount].rgb), DRMRGBPAL_ALPHA_MASK, CNXT_GET_VAL(&pPal[dwCount].rgb, DRMRGBPAL_ALPHA_MASK));
         }
         else
         {
            lpDest[dwCount] = pPal[dwCount];
         }
         if (!bLoadAlpha)
            CNXT_SET_VAL(&(lpDest[dwCount].rgb), DRMRGBPAL_ALPHA_MASK, DEFAULT_PALETTE_ALPHA);
      }
      dwRet = dwNumEntries;
   }
   else
   {
      trace_new(OSD_ERROR_MSG, "ERROR: Invalid handle - 0x%08X\n", hRgn);
   }

   trace_new(OSD_FUNC_TRACE, "<-SetOSDRgnPalette\n");

   return dwRet;
}

/*****************************************************************************/
/* Function: SetOSDRgnAlpha()                                                */
/*                                                                           */
/* Parameters: OSDHANDLE hRgn - Handle to OSD region.                        */
/*             OSDALPHAFLAG Flags - Alpha flags.                             */
/*             u_int32 dwAlpha - New alpha value.                            */
/*             u_int32 dwPalIndex - Pal index for per palette alpha.         */
/*                                                                           */
/* Returns: bool - TRUE on success.                                          */
/*                                                                           */
/* Description: Sets the alpha value for the OSD region.                     */
/*****************************************************************************/
bool SetOSDRgnAlpha(OSDHANDLE hRgn,
                    OSDALPHAFLAG Flags,
                    u_int32 dwAlpha,
                    u_int32 dwPalIndex)
{
   bool bRet = FALSE;
   POSDLIBREGION pRgn;

   trace_new(OSD_FUNC_TRACE, "->SetOSDRgnAlpha\n");
   trace_new(OSD_MSG, "Flags = 0x%08X, dwAlpha = %d, dwPalIndex = %d\n", Flags,
      dwAlpha, dwPalIndex);

   if ((pRgn = (POSDLIBREGION)hRgn)!=0)
   {
      bRet = TRUE;

      switch (Flags)
      {
      case PALETTE_ALPHA:
         if ((dwPalIndex < pRgn->dwNumPalEntries) && (pRgn->pPalette))
         {
            CNXT_SET_VAL(&pRgn->pPalette[dwPalIndex].rgb, DRMRGBPAL_ALPHA_MASK,
               (dwAlpha > MAX_OSD_RGN_ALPHA) ? MAX_OSD_RGN_ALPHA : dwAlpha);
         }
         break;
      case REGION_ALPHA:
         pRgn->dwRgnAlpha =
            (dwAlpha > MAX_OSD_RGN_ALPHA) ? MAX_OSD_RGN_ALPHA : dwAlpha;
         if (pRgn->dwOptions & OSD_RO_ALPHAENABLE)
            UpdateOSDHeader(hRgn);
         break;
      default:
         bRet = FALSE;
      }
   }
   else
   {
      trace_new(OSD_ERROR_MSG, "ERROR: Invalid handle - 0x%08X\n", hRgn);
   }

   trace_new(OSD_FUNC_TRACE, "<-SetOSDRgnAlpha\n");

   return bRet;
}

/*****************************************************************************/
/* Function: SetOSDColorKey()                                                */
/*                                                                           */
/* Parameters: u_int32 dwColorKey - Color key compare value.                 */
/*                                                                           */
/* Returns: bool - TRUE on success.                                          */
/*                                                                           */
/* Description: Sets the OSD color key compare register.                     */
/*****************************************************************************/
bool SetOSDColorKey(u_int32 dwColorKey)
{
   bool ksPrev;

   trace_new(OSD_FUNC_TRACE, "->SetOSDColorKey\n");

   ksPrev = critical_section_begin();
   CNXT_SET_VAL(glpDrmColorKey, DRM_COLOR_KEY_PIXEL_MASK, dwColorKey);
   critical_section_end(ksPrev);

   trace_new(OSD_FUNC_TRACE, "<-SetOSDColorKey\n");

   return TRUE;
}

/*****************************************************************************/
/* Function: GetOSDColorKey()                                                */
/*                                                                           */
/* Parameters: void                                                          */
/*                                                                           */
/* Returns: u_int32 - OSD color key value.                                   */
/*                                                                           */
/* Description: Returns the osd color key compare register.                  */
/*****************************************************************************/
u_int32 GetOSDColorKey(void)
{
   u_int32 dwRet = CNXT_GET_VAL(glpDrmColorKey, DRM_COLOR_KEY_PIXEL_MASK);

   trace_new(OSD_FUNC_TRACE, "<->GetOSDColorKey\n");

   return dwRet;
}

/*****************************************************************************/
/* Function: SetOSDBackground()                                              */
/*                                                                           */
/* Parameters: DRM_COLOR yccBackground - Background color                    */
/*                                                                           */
/* Returns: bool - TRUE on success.                                          */
/*                                                                           */
/* Description: Sets the osd background color register.                      */
/*****************************************************************************/
bool SetOSDBackground(HW_DWORD yccBackground)
{
   bool ksPrev;

   trace_new(OSD_FUNC_TRACE, "->SetOSDBackground\n");

   ksPrev = critical_section_begin();
   *glpDrmBackground = yccBackground;
   critical_section_end(ksPrev);

   trace_new(OSD_FUNC_TRACE, "<-SetOSDBackground\n");

   return TRUE;
}

/*****************************************************************************/
/* Function: GetOSDBackground()                                              */
/*                                                                           */
/* Parameters: void                                                          */
/*                                                                           */
/* Returns: DRM_COLOR - Background color                                     */
/*                                                                           */
/* Description: Gets the osd background color register.                      */
/*****************************************************************************/
HW_DWORD GetOSDBackground(void)
{
   trace_new(OSD_FUNC_TRACE, "<->GetOSDBackground\n");
   return *glpDrmBackground;
}

/*****************************************************************************/
/* Function: SetDefaultOSDPalette()                                          */
/*                                                                           */
/* Parameters: OSDHANDLE hRgn - Handle to OSD region.                        */
/*                                                                           */
/* Returns: u_int32 - The number of entries set.                             */
/*                                                                           */
/* Description: Sets the region palette to default.                          */
/*****************************************************************************/
u_int32 SetDefaultOSDPalette(OSDHANDLE hRgn)
{
   u_int32 dwRet = 0;
   u_int32 dwCount;
   POSDLIBREGION pRgn;
   PDRMPAL lpPal = NULL;
   bool bRGB = FALSE;
   u_int32 dwVal;
   DRMPAL palEntry;
   u_int8 byEntry;

   trace_new(OSD_FUNC_TRACE, "->SetDefaultOSDPalette\n");

   if ((pRgn = (POSDLIBREGION)hRgn)!=0)
   {
      dwRet = pRgn->dwNumPalEntries;

      /* Reset the palette */
      for (dwCount=0; dwCount<pRgn->dwNumPalEntries; dwCount++)
         pRgn->pPalette[dwCount].dwVal = ((u_int32)DEFAULT_PALETTE_ALPHA << 24);

      switch (pRgn->dwMode)
      {
      case OSD_MODE_4ARGB:
         bRGB = TRUE;
         /* Fall thru. */
      case OSD_MODE_4AYUV:
         if (pRgn->pPalette)
         {
            lpPal = pRgn->pPalette;
         }
         break;
      case OSD_MODE_8ARGB:
         bRGB = TRUE;
         /* Fall thru. */
      case OSD_MODE_8AYUV:
         lpPal = pRgn->pPalette;
         break;
      case OSD_MODE_16ARGB:
         for (dwCount=0; dwCount<16; dwCount++)
         {
            HW_DWORD value;
            dwVal = dwCount * 255 / 15;
            value = ((gbGammaCorrect) ? bGamma[dwVal] : dwVal);
            CNXT_SET_VAL(&(pRgn->pPalette[dwCount].rgb), DRMRGBPAL_B_MASK, value);
            CNXT_SET_VAL(&(pRgn->pPalette[dwCount].rgb), DRMRGBPAL_G_MASK, value);
            CNXT_SET_VAL(&(pRgn->pPalette[dwCount].rgb), DRMRGBPAL_R_MASK, value);
         }
         break;
      case OSD_MODE_16RGB:
         for (dwCount=0; dwCount<32; dwCount++)
         {
            HW_DWORD value;
            dwVal = dwCount * 255 / 31;
            value = ((gbGammaCorrect) ? bGamma[dwVal] : dwVal);
            CNXT_SET_VAL(&(pRgn->pPalette[dwCount].rgb), DRMRGBPAL_B_MASK, value);
            CNXT_SET_VAL(&(pRgn->pPalette[dwCount].rgb), DRMRGBPAL_R_MASK, value);
            CNXT_SET_VAL(&(pRgn->pPalette[dwCount].rgb), DRMRGBPAL_ALPHA_MASK, dwVal);
         }
         for (dwCount=0; dwCount<64; dwCount++)
         {
            dwVal = dwCount * 255 / 63;
            CNXT_SET_VAL(&(pRgn->pPalette[dwCount].rgb), DRMRGBPAL_G_MASK,
               ((gbGammaCorrect) ? bGamma[dwVal] : dwVal));
         }
         break;
      case OSD_MODE_16ARGB1555:
         for (dwCount=0; dwCount<32; dwCount++)
         {
            HW_DWORD value;
            dwVal = dwCount * 255 / 31;
            value = ((gbGammaCorrect) ? bGamma[dwVal] : dwVal);
            CNXT_SET_VAL(&(pRgn->pPalette[dwCount].rgb), DRMRGBPAL_B_MASK, value);
            CNXT_SET_VAL(&(pRgn->pPalette[dwCount].rgb), DRMRGBPAL_G_MASK, value);
            CNXT_SET_VAL(&(pRgn->pPalette[dwCount].rgb), DRMRGBPAL_R_MASK, value);
         }
         break;
      case OSD_MODE_32ARGB:
         for (dwCount=0; dwCount<256; dwCount++)
         {
            HW_DWORD val = ((gbGammaCorrect) ? bGamma[dwCount] : dwCount);
            CNXT_SET_VAL(&(pRgn->pPalette[dwCount].rgb), DRMRGBPAL_B_MASK, val);
            CNXT_SET_VAL(&(pRgn->pPalette[dwCount].rgb), DRMRGBPAL_G_MASK, val);
            CNXT_SET_VAL(&(pRgn->pPalette[dwCount].rgb), DRMRGBPAL_R_MASK, val);
            CNXT_SET_VAL(&(pRgn->pPalette[dwCount].rgb), DRMRGBPAL_ALPHA_MASK, dwCount);
         }
         break;
      case OSD_MODE_16AYUV:
      case OSD_MODE_16YUV655:
      case OSD_MODE_16YUV422:
      case OSD_MODE_16AYUV2644:
      case OSD_MODE_32AYUV:
         break;
      }


      if (lpPal)
      {
         for (dwCount=0; dwCount<16; dwCount++)
         {
            /* Create a 16 entry palette keyed from dwCount.
             * The 4 LSB bits in dwCount can be thought as MBGR
             * M = Full intensity 0xFF or half intensity 0x7F
             * B, G, R = Blue, Green, Red on/off with the intensity
             *           based on M.
             */
            byEntry = (dwCount & 8) ? 0xFF : 0x7F;

            CNXT_SET_VAL(&(palEntry.rgb), DRMRGBPAL_R_MASK, (dwCount & 1) ? byEntry : 0);
            CNXT_SET_VAL(&(palEntry.rgb), DRMRGBPAL_G_MASK, (dwCount & 2) ? byEntry : 0);
            CNXT_SET_VAL(&(palEntry.rgb), DRMRGBPAL_B_MASK, (dwCount & 4) ? byEntry : 0);

            if (bRGB)
            {
               CNXT_SET_VAL(&(lpPal[dwCount].rgb), DRMRGBPAL_R_MASK, (gbGammaCorrect) ?
                  bGamma[CNXT_GET_VAL(&(palEntry.rgb), DRMRGBPAL_R_MASK)] : CNXT_GET_VAL(&(palEntry.rgb), DRMRGBPAL_R_MASK));
               CNXT_SET_VAL(&(lpPal[dwCount].rgb), DRMRGBPAL_G_MASK, (gbGammaCorrect) ?
                  bGamma[CNXT_GET_VAL(&(palEntry.rgb), DRMRGBPAL_G_MASK)] : CNXT_GET_VAL(&(palEntry.rgb), DRMRGBPAL_G_MASK));
               CNXT_SET_VAL(&(lpPal[dwCount].rgb), DRMRGBPAL_B_MASK, (gbGammaCorrect) ?
                  bGamma[CNXT_GET_VAL(&(palEntry.rgb), DRMRGBPAL_B_MASK)] : CNXT_GET_VAL(&(palEntry.rgb), DRMRGBPAL_B_MASK));
            }
            else
            {
               CNXT_SET_VAL(&(lpPal[dwCount].ycc), DRMYCCPAL_Y_MASK,
                    RGBTOY(
                        (int)CNXT_GET_VAL(&(palEntry.rgb), DRMRGBPAL_R_MASK),
                        (int)CNXT_GET_VAL(&(palEntry.rgb), DRMRGBPAL_G_MASK),
                        (int)CNXT_GET_VAL(&(palEntry.rgb), DRMRGBPAL_B_MASK)));
               CNXT_SET_VAL(&(lpPal[dwCount].ycc), DRMYCCPAL_CR_MASK,
                    RGBTOCR(
                        (int)CNXT_GET_VAL(&(palEntry.rgb), DRMRGBPAL_R_MASK),
                        (int)CNXT_GET_VAL(&(palEntry.rgb), DRMRGBPAL_G_MASK),
                        (int)CNXT_GET_VAL(&(palEntry.rgb), DRMRGBPAL_B_MASK)));
               CNXT_SET_VAL(&(lpPal[dwCount].ycc), DRMYCCPAL_CB_MASK,
                    RGBTOCB(
                        (int)CNXT_GET_VAL(&(palEntry.rgb), DRMRGBPAL_R_MASK),
                        (int)CNXT_GET_VAL(&(palEntry.rgb), DRMRGBPAL_G_MASK),
                        (int)CNXT_GET_VAL(&(palEntry.rgb), DRMRGBPAL_B_MASK)));
            }
         }
      }
   }
   else
   {
      trace_new(OSD_ERROR_MSG, "ERROR: Invalid handle - 0x%08X\n", hRgn);
   }

   trace_new(OSD_FUNC_TRACE, "<-SetDefaultOSDPalette\n");

   return dwRet;
}

/*****************************************************************************/
/* Function: GetOSDRgnPalEntry()                                             */
/*                                                                           */
/* Parameters: OSDHANDLE hRgn - Handle to OSD region.                        */
/*             u_int32 dwIndex - Palette index to retrieve.                  */
/*                                                                           */
/* Returns: DRMPAL - Palette entry.                                          */
/*                                                                           */
/* Description: Returns the palette entry associated with dwIndex.           */
/*****************************************************************************/
DRMPAL GetOSDRgnPalEntry(OSDHANDLE hRgn, u_int32 dwIndex)
{
   DRMPAL palRet;
   POSDLIBREGION pRgn;

   trace_new(OSD_FUNC_TRACE, "<-GetOSDRgnPalEntry\n");

   palRet.dwVal = 0;

   if ((pRgn = (POSDLIBREGION)hRgn)!=0)
   {
      if ((pRgn->pPalette) && (dwIndex < pRgn->dwNumPalEntries))
      {
         palRet = pRgn->pPalette[dwIndex];
      }
   }
   else
   {
      trace_new(OSD_ERROR_MSG, "ERROR: Invalid handle - 0x%08X\n", hRgn);
   }

   trace_new(OSD_FUNC_TRACE, "<-GetOSDRgnPalEntry\n");

   return palRet;
}

/*****************************************************************************/
/* Function: SetOSDRgnPalEntry()                                             */
/*                                                                           */
/* Parameters: OSDHANDLE hRgn - Handle to OSD region.                        */
/*             u_int32 dwIndex - Palette index to set.                       */
/*             DRMPAL drmPal - Color to load in the palette.                 */
/*                                                                           */
/* Returns: bool - TRUE on success.                                          */
/*                                                                           */
/* Description: Sets the palette entry associated with the index.  4 bpp     */
/*              indexes will always be 0 to 15 no matter where the 4bpp is   */
/*              mapped to the 8bpp palette.  This will also write the alpha  */
/*              value in the palette.                                        */
/*****************************************************************************/
bool SetOSDRgnPalEntry(OSDHANDLE hRgn, u_int32 dwIndex, DRMPAL drmPal)
{
   POSDLIBREGION pRgn;
   bool bRet = FALSE;
   u_int32 dwPalSize;

   pRgn = (POSDLIBREGION)hRgn;
   if (pRgn)
   {
      switch (GetOSDRgnOptions(hRgn, OSD_RO_MODE))
      {
      case OSD_MODE_4ARGB:
      case OSD_MODE_4AYUV:
      case OSD_MODE_16ARGB:
         dwPalSize = 16;
         break;
      case OSD_MODE_8ARGB:
      case OSD_MODE_8AYUV:
         dwPalSize = 256;
         break;
      case OSD_MODE_16RGB:
         dwPalSize = 64;
         break;
      case OSD_MODE_16ARGB1555:
         dwPalSize = 32;
         break;
      case OSD_MODE_32ARGB:
         dwPalSize = 256;
         break;
      case OSD_MODE_16AYUV:
      case OSD_MODE_16YUV655:
      case OSD_MODE_16YUV422:
      case OSD_MODE_16AYUV2644:
      case OSD_MODE_32AYUV:
      default:
         dwPalSize = 0;
         break;
      }

      if (dwIndex < dwPalSize)
      {
         pRgn->pPalette[dwIndex] = drmPal;
         bRet = TRUE;
      }
   }

   return bRet;
}

/*****************************************************************************/
/* Function: ResizeOSDRegion()                                               */
/*                                                                           */
/* Parameters: OSDHANDLE hRgn - Handle to OSD region.                        */
/*             POSDRECT lpRect - Ptr to rect describing the region           */
/*                                                                           */
/* Returns: bool - TRUE on successful resizing of the OSD region.            */
/*                                                                           */
/* Description: Resizes the memory buffer associated with the region.  None  */
/*              of the options are changed, however the image buffer is      */
/*              cleared upon successful reallocation.  If the new image      */
/*              buffer was not created, the original image buffer is left    */
/*              unchanged.                                                   */
/*****************************************************************************/
bool ResizeOSDRegion(OSDHANDLE hRgn, POSDRECT lpRect)
{
   bool bRet = FALSE;
   u_int32 dwHeight, dwWidth, dwStride;
   POSDLIBREGION pRgn;
   LPVOID pImage;
   bool bEnabled = FALSE;
   OSDRECT rcClean;
   u_int32       uAlignment;
   u_int32       uByteWidth;


   pRgn = (POSDLIBREGION)hRgn;
   if (pRgn && lpRect)
   {
      dwWidth = lpRect->right - lpRect->left;
      dwHeight = lpRect->bottom - lpRect->top;

      /* Find out the number of bytes if alignment required for the */
      /* stride and buffer pointer                                  */
      uAlignment = get_mode_alignment(pRgn->dwMode);

      switch (pRgn->dwMode)
      {
      case OSD_MODE_4ARGB:
      case OSD_MODE_4AYUV:
         uByteWidth = (dwWidth + 1)/2;
         break;
      case OSD_MODE_8ARGB:
      case OSD_MODE_8AYUV:
         uByteWidth = dwWidth;
         break;
      case OSD_MODE_16ARGB:
      case OSD_MODE_16AYUV:
      case OSD_MODE_16RGB:
      case OSD_MODE_16YUV655:
      case OSD_MODE_16YUV422:
      case OSD_MODE_16ARGB1555:
      case OSD_MODE_16AYUV2644:
         uByteWidth = dwWidth << 1;
         break;
      case OSD_MODE_32ARGB:
      case OSD_MODE_32AYUV:
         uByteWidth = dwWidth << 2;
         break;
      default:
         /* above should cover all the cases */
         uByteWidth = dwWidth;
         error_log(ERROR_WARNING);
         break;
      }

      dwStride = (uByteWidth + (uAlignment-1)) & (~(uAlignment-1));

      /* Create the memory for the image, pad for cache alignment,
       * and move to non-cache region.
       */
      #ifdef OPENTV_12
      pImage = (LPVOID)vidImageBufferAlloc(dwStride * dwHeight, &(pRgn->bTransient), FALSE);
      if(pImage)
        pImage = (LPVOID)SET_FLAGS(pImage, NCR_BASE);
      #else
        pImage = (LPVOID)mem_nc_malloc(
            ((dwStride * dwHeight) + (uAlignment-1)));
      #endif

      if (pImage)
      {
         /* Check to see if the region is enabled, if so, disable it. */
         bEnabled = (bool)GetOSDRgnOptions(hRgn, OSD_RO_ENABLE);
         if (bEnabled)
            SetOSDRgnOptions(hRgn, OSD_RO_ENABLE, FALSE);

         /* Free the old image buffer. */
         if (pRgn->pAllocatedImage)
         {
            #ifdef OPENTV_12
            vidImageBufferFree((void *)CLEAR_FLAGS(pRgn->pAllocatedImage, NCR_BASE), pRgn->bTransient);
            #else
            mem_nc_free((void *)pRgn->pAllocatedImage);
            #endif
         }

         pRgn->nTop = lpRect->top;
         pRgn->nLeft = lpRect->left;
         pRgn->nRight = lpRect->right;
         pRgn->nBottom = lpRect->bottom;
         pRgn->dwW = dwWidth;
         pRgn->dwH = dwHeight;
         pRgn->pAllocatedImage = pImage;
         pRgn->pImage =(LPVOID)(((u_int32)pRgn->pAllocatedImage + (uAlignment-1)) & ~(uAlignment-1));
         pRgn->dwStride = dwStride;

         UpdateOSDHeader(hRgn);

         /* Fill the buffer with lots of nothing. */
         rcClean.top = 0;
         rcClean.left = 0;
         rcClean.right = dwWidth;
         rcClean.bottom = dwHeight;
         osd_bitblit_fill(hRgn, 0, &rcClean);

         /* Enable the region if needed. */
         if (bEnabled)
            SetOSDRgnOptions(hRgn, OSD_RO_ENABLE, TRUE);

         bRet = TRUE;
      }
   }

   return bRet;
}

/*****************************************************************************/
/* Function: RectIntersect()                                                 */
/*                                                                           */
/* Parameters: POSDRECT prc1 - Ptr to first rectangle.                       */
/*             POSDRECT prc1 - Ptr to second rectangle.                      */
/*                                                                           */
/* Returns: BOOL - TRUE if the rectangles intersect.  FALSE otherwise.       */
/*                                                                           */
/* Description: Check to see if 2 rectangles intersect each other.           */
/*****************************************************************************/
bool RectIntersect(POSDRECT prc1, POSDRECT prc2)
{
   bool bRet = FALSE;
   bool bYOverlap = FALSE;

   /* Check the pointers.  Don't use NULL. */
   if ((prc1) && (prc2))
   {
      /* Check in the y direction. */
      if ((prc1->top >= prc2->top) && (prc1->top < prc2->bottom))
         bYOverlap = TRUE;

      if ((prc2->top >= prc1->top) && (prc2->top < prc1->bottom))
         bYOverlap = TRUE;

      /* Don't bother checking for X overlap if the Y doesn't overlap. */
      if (bYOverlap)
      {
         if ((prc1->left >= prc2->left) && (prc1->left < prc2->right))
            bRet = TRUE;

         if ((prc2->left >= prc1->left) && (prc2->left < prc1->right))
            bRet = TRUE;
      }
   }

   return bRet;
}

/*****************************************************************************/
/* Function: SetOSDBuffer()                                                  */
/*                                                                           */
/* Parameters: OSDHANDLE hRgn - Handle to OSD region.                        */
/*             LPVOID pBuffer- Ptr to image buffer.                          */
/*             u_int32 dwStride - Stride of the image buffer.                */
/*                                                                           */
/* Returns: BOOL - TRUE if the osd region is now using the new buffer.       */
/*                                                                           */
/* Description: Replaces the region buffer with a new buffer.  If the new    */
/*              image is a different size from the original image, it is     */
/*              assumed that the region will be disabled before switching    */
/*              buffers.  It is also assumed that the new height and width   */
/*              will be set after changing buffers and before enabling the   */
/*              region.                                                      */
/*****************************************************************************/
bool SetOSDBuffer(OSDHANDLE hRgn, LPVOID pBuffer, u_int32 dwStride)
{
   bool bRet = FALSE;
   POSDLIBREGION pRgn;

   trace_new(OSD_FUNC_TRACE, "->SetOSDBuffer\n");

   pRgn = (POSDLIBREGION)hRgn;

   if (pRgn && pBuffer && dwStride)
   {
      /* Free any image buffer that we have allocated. */
      if (pRgn->pAllocatedImage)
      {
         #ifdef OPENTV_12
         vidImageBufferFree((void *)CLEAR_FLAGS(pRgn->pAllocatedImage, NCR_BASE), pRgn->bTransient);
         #else
         mem_nc_free((void *)pRgn->pAllocatedImage);
        #endif
      }
      pRgn->pAllocatedImage = NULL;

      /* Set the image pointer to the new buffer. */
      pRgn->pImage = pBuffer;

      /* Set the new stride value. */
      pRgn->dwStride = dwStride;

      bRet = UpdateOSDHeader(hRgn);
   }

   trace_new(OSD_FUNC_TRACE, "<-SetOSDBuffer\n");

   return bRet;
}

/*****************************************************************************/
/* Exported Mpeg Functions ***************************************************/
/*****************************************************************************/
/*****************************************************************************/
/* Function: UpdateMpgScaling()                                              */
/*                                                                           */
/* Parameters: void                                                          */
/*                                                                           */
/* Returns: void                                                             */
/*                                                                           */
/* Description: Sets the Mpeg video position, size, and scaling registers.   */
/*****************************************************************************/
void UpdateMpgScaling(void)
{
   HVIDRGN hRgn = vidGet420Hardware();

   if (hRgn)
   {
      /* Update the video region height, width and aspect ratio. */
      vidSetOptions(hRgn, VO_ASPECT_RATIO_MODE, gArMode, FALSE);
      vidSetOptions(hRgn, VO_ASPECT_RATIO, gdwAR, FALSE);
      vidSetOptions(hRgn, VO_IMAGE_WIDTH, gdwSrcWidth, FALSE);
      #ifdef OPENTV_12
      vidSetOptions(hRgn, VO_IMAGE_HEIGHT, gdwSrcHeight, FALSE);
      video_size_change(hRgn, gdwSrcWidth, gdwSrcHeight);
      #else
      vidSetOptions(hRgn, VO_IMAGE_HEIGHT, gdwSrcHeight, TRUE);
      #endif
   }
}

/*****************************************************************************/
/* Function: SetMpgAR()                                                      */
/*                                                                           */
/* Parameters: OSD_AR_MODE arMode - Aspect ratio mode.                       */
/*             bool bUpdate - Update the Mpg scaling.                        */
/*                                                                           */
/* Returns: bool - TRUE on success.                                          */
/*                                                                           */
/* Description: Sets the aspect ratio mode for the mpeg display.             */
/*****************************************************************************/
bool SetMpgAR(OSD_AR_MODE arMode, bool bUpdate)
{
   bool    bRet = TRUE;
   HVIDRGN hRgn = vidGet420Hardware();

   switch (arMode)
   {
      case OSD_ARM_PANSCAN:
      case OSD_ARM_LETTERBOX:
      case OSD_ARM_NONE:
         /* Update out global aspect ratio variable and start the ball    */
         /* rolling that will eventually update the DRM and MPG registers */
         /* to set the correct aspect ratio mode.                         */

         /**************************************************************************/
         /* This is a tricky business since the sequencing of writes to the MPEG   */
         /* decoder and DRM registers is vital to prevent "bouncing" artifacts on  */
         /* the transition. The sequence is as follows:                            */
         /*                                                                        */
         /* 1. First, update the target video region with the new aspect ratio     */
         /*    handling mode. DO NOT set the bUpdate parameter!!!!                 */
         /* 2. Wait for the next MPEG interrupt 5 and set the aspect ratio         */
         /*    conversion and letterbox control bits in the MPEG decoder           */
         /*    according to the mode being selected, the screen and video aspect   */
         /*    ratios.                                                             */
         /* 3. Wait for the interrupt 5 after this and apply video scaling changes */
         /*    by writing all the DRM registers.                                   */
         /*                                                                        */
         /* If you decide to do this any other way, all bets are off!              */
         /**************************************************************************/

         trace_new(TRACE_MPG|TRACE_LEVEL_2, "OSD: Switching aspect ratio handling to mode %d\n", (u_int32)arMode);

         gArMode = arMode;
         vidSetOptions(hRgn, VO_ASPECT_RATIO_MODE, gArMode, FALSE);

         if (bUpdate)
         {
           /* Wait for the next Int 5 from OSDISRC.C */
           giOsdNotifyInt5 = 1;
         }
         break;
      default:
         bRet = FALSE;
   }

   return bRet;
}

/********************************************************************/
/*  FUNCTION:    osdMpegInt5Received                                */
/*                                                                  */
/*  PARAMETERS:  None                                               */
/*                                                                  */
/*  DESCRIPTION: This function is called from OSDISRC.C whenever an */
/*               MPEG interrupt 5 (picture sent to DRM for display) */
/*               is received. We use this in processing pan/scan-   */
/*               letterbox changes since the timing of writing the  */
/*               DRM and MPEG regs is important here if we want to  */
/*               avoid artifacts.                                   */
/*                                                                  */
/*  RETURNS:     Nothing                                            */
/*                                                                  */
/*  CONTEXT:     This function is called under interrupt context    */
/*                                                                  */
/********************************************************************/
void osdMpegInt5Received(void)
{
   switch(giOsdNotifyInt5)
   {
     case 1:
       /* We received an Int 5 so now we tell the MPEG decoder to prepare */
       /* for a panscan/letterbox switch.                                 */
       vidPrepareMpgForAspectHandlingChange(0);
       giOsdNotifyInt5++;
       isr_trace_new(TRACE_MPG|TRACE_LEVEL_2, "OSD: Update MPEG panscan/letterbox registers on int 5\n", 0, 0);
       break;

     case 2:
       /* On the second int 5 we update the DRM registers */
       UpdateMpgScaling();
       isr_trace_new(TRACE_MPG|TRACE_LEVEL_2, "OSD: Video window size updated after second int 5\n", 0, 0);
       giOsdNotifyInt5 = 0;
       break;
  }
}

/*****************************************************************************/
/* Function: GetMpgAR()                                                      */
/*                                                                           */
/* Parameters: void                                                          */
/*                                                                           */
/* Returns: OSD_AR_MODE - The current aspect ratio mode.                     */
/*                                                                           */
/* Description: Returns the aspect ratio mode for the mpeg display.          */
/*****************************************************************************/
OSD_AR_MODE GetMpgAR(void)
{
   return gArMode;
}

/*****************************************************************************/
/* Function: SetDisplayAR()                                                  */
/*                                                                           */
/* Parameters: OSD_DISPLAY_AR arDisplay - Aspect ratio.                      */
/*             bool bUpdate - Update the Mpg scaling.                        */
/*                                                                           */
/* Returns: bool - TRUE on success.                                          */
/*                                                                           */
/* Description: Sets the aspect ratio mode for the display.                  */
/*****************************************************************************/
bool SetDisplayAR(OSD_DISPLAY_AR arDisplay, bool bUpdate)
{
   bool bRet = TRUE;

   if ((arDisplay == OSD_DISPLAY_AR_43) || (arDisplay == OSD_DISPLAY_AR_169))
   {
     trace_new(OSD_FUNC_TRACE, "SetDisplayAR: Display type set to %s\n",
               (arDisplay == OSD_DISPLAY_AR_43) ? "4:3" : "16:9");
     gDisplayAR = arDisplay;
     if(bUpdate)
      UpdateMpgScaling();
     bRet = TRUE;
   }
   else
   {
     trace_new(OSD_FUNC_TRACE, "SetDisplayAR: Invalid aspect ratio %d passed!\n",
               (int)arDisplay);
     bRet = FALSE;
   }

   return bRet;
}

/*****************************************************************************/
/* Function: GetDisplayAR()                                                  */
/*                                                                           */
/* Parameters: void                                                          */
/*                                                                           */
/* Returns: OSD_DISPLAY_AR - The current display aspect ratio.               */
/*                                                                           */
/* Description: Returns the aspect ratio mode for the display.               */
/*****************************************************************************/
OSD_DISPLAY_AR GetDisplayAR(void)
{
   return gDisplayAR;
}

/*****************************************************************************/
/* Function: SetAspectRatio()                                                */
/*                                                                           */
/* Parameters: OSD_DISPLAY_AR arDisplay - Display aspect ratio.              */
/*             OSD_AR_MODE arMode - Aspect ratio mode.                       */
/*                                                                           */
/* Returns: bool - TRUE on success.                                          */
/*                                                                           */
/* Description: Sets the aspect ratio and mode.                              */
/*****************************************************************************/
bool SetAspectRatio(OSD_DISPLAY_AR arDisplay, OSD_AR_MODE arMode)
{
   bool bRet = FALSE;

   if (SetDisplayAR(arDisplay, FALSE))
      bRet = SetMpgAR(arMode, TRUE);

   return bRet;
}

/*****************************************************************************/
/* Function: GetAspectRatio()                                                */
/*                                                                           */
/* Parameters: OSD_DISPLAY_AR *parDisplay - Ptr to display aspect ratio.     */
/*             OSD_AR_MODE *parMode - Ptr to aspect ratio mode.              */
/*                                                                           */
/* Returns: bool - TRUE on success.                                          */
/*                                                                           */
/* Description: Returns the aspect ratio and mode.                           */
/*****************************************************************************/
bool GetAspectRatio(OSD_DISPLAY_AR *parDisplay, OSD_AR_MODE *parMode)
{
   bool bRet = FALSE;

   if (parDisplay && parMode)
   {
      *parMode = GetMpgAR();
      *parDisplay = GetDisplayAR();
      bRet = TRUE;
   }

   return bRet;
}


/*****************************************************************************/
/***** Internal Functions ****************************************************/
/*****************************************************************************/
/*****************************************************************************/
/* Function: UpdateOSDHeader()                                               */
/*                                                                           */
/* Parameters: OSDHANDLE hRgn - Handle to OSD region to be updated.          */
/*                                                                           */
/* Returns: bool - TRUE on success                                           */
/*                                                                           */
/* Description: Load the OSD header with the current mode from the mode table*/
/*****************************************************************************/
bool UpdateOSDHeader(OSDHANDLE hRgn)
{
   u_int32 nWidth;
   u_int32 nLines;
   u_int32 nF1YStart;
   u_int32 nF2YStart;
   u_int32 nF1YEnd;
   u_int32 nF2YEnd;
   u_int32 nF1Height;
   u_int32 nF2Height;
   POSDLIBREGION pRgn;
   POSDHEADER pOsd;
   POSDHEADER pNextOsd;
   bool    ks;
   int     nFieldCount;     /* Field count local copy      */
   int32   nLineNum;        /* Current line of display     */
   int     uHdrToUse;       /* Which headers to use for LL */
   u_int32 uPlane;          /* Plane of this region        */
   

   if ((pRgn = (POSDLIBREGION)hRgn)==0)
      return FALSE;

   uPlane = pRgn->uPlane;

   ks = critical_section_begin();

   /* Decide whether to update now, or wait for vertical retrace           */
   /* If we are closer to retrace than (n) lines, wait until after retrace */ 
   nLineNum = ((*glpDrmStatus) & DRM_STATUS_LINE_MASK) >> DRM_STATUS_LINE_SHIFT;
   if (nLineNum > ((gnOsdMaxHeight/2) - (OSD_LL_UPDATE_SAFETY)))
   {
   
      /* Wait for the field change to load the DRM registers.             */
      /* We don't want to mess up any structures if DRM is still using it.*/
      /* We must be outside critical section while waiting for field      */
      /* to change, or we will hang forever.                              */
      
      #if (CUSTOMER == VENDOR_D) && (defined OPENTV_12)
      /* Vendor D turns off the encoder in standby mode so this code will */
      /* hang here until the encoder is turned back on if we wait for the */
      /* next field while in standby.                                     */
      if (!gbStandbyMode)
      {
         critical_section_end(ks);
         nFieldCount = gnFieldCount;
         while (nFieldCount == gnFieldCount);
         ks = critical_section_begin();
      }
      #else
      critical_section_end(ks);
      nFieldCount = gnFieldCount;
      while (nFieldCount == gnFieldCount);
      ks = critical_section_begin();
      #endif

   } /* endif */

   /* Now we are in a critical section to do the linked list generation */
   /* First, decide which set of headers to use                         */
   /* If pending, use the header set which is marked as pending since   */
   /* it is not active (quite) yet.                                     */
   /* If not pending, use the header set marked "active" which is a     */
   /* somewhat confusing legacy of the old code - it actually means     */
   /* which one is inactive.                                            */

   if (gbPendingOSDLLUpdate[uPlane])
   {
      uHdrToUse = guPendingOSDHdr[uPlane];
   } 
   else 
   {
      uHdrToUse = gActiveOSDHdr[uPlane];
      guPendingOSDHdr[uPlane] = uHdrToUse;
      gbPendingOSDLLUpdate[uPlane] = TRUE;
      
      /* Advance which header is active for after retrace */
      if (gActiveOSDHdr[uPlane] == 1)
      {
         gActiveOSDHdr[uPlane] = 2;
      }
      else
      {
         gActiveOSDHdr[uPlane] = 1;
      }
   } /* endif pending OSD Linked list update */
   
   if (uHdrToUse == 1)
   {
      pOsd = &pRgn->OSDHdr1;
   }
   else
   {
      pOsd = &pRgn->OSDHdr2;
   }

   nWidth = pRgn->nRight - pRgn->nLeft;
   nLines = pRgn->nBottom - pRgn->nTop;

   /* New algorithm scientifically proven to (maybe) be correct using */
   /* a highly complex Excel spreadsheet.                             */
   nF1YStart = gnVBlank + (pRgn->nTop/2) + (pRgn->nTop % 2);
   nF2YStart = gnVBlank + (pRgn->nTop/2);

   if (nLines %2)
   {
     /* Odd height - apply the extra line to the correct field */
     nF2Height = nLines/2 + (pRgn->nTop % 2);
     nF1Height = nLines - nF2Height;
   }
   else
   {
     /* Even height - both fields have same number of lines */
     nF1Height = nLines/2;
     nF2Height = nLines/2;
   }

   nF1YEnd = nF1YStart+nF1Height-1; /* End lines are drawn so subtract 1 */
   nF2YEnd = nF2YStart+nF2Height-1;

   /* Set the OSD header */
/*   pOsd->pNextOsd = 0; */
   pOsd->dwXStartWidth = MAKE_XSTARTWIDTH(
      XSTART(pRgn->nLeft, gnHBlank),      /* x */
      nWidth,                             /* width */
      gOSDPixelModes[pRgn->dwMode],       /* pixel type */
      pRgn->dwPalIndex,                   /* pal index */
      (pRgn->dwOptions & OSD_RO_LOADPALETTE) ? TRUE : FALSE); /* load pal */

   if (pRgn->dwOptions & OSD_RO_BSWAPACCESS) {
      pOsd->dwXStartWidth |= XSW_BSWAP_FLAG;
   }

   if (pRgn->nTop % 2)
   {
     /* Top line of OSD is in ODD field (field 2) */
     pOsd->dwAddrField2 = (u_int32)CLEAR_FLAGS(pRgn->pImage, NCR_BASE);
     pOsd->dwAddrField1 = pOsd->dwAddrField2 + pRgn->dwStride;
   }
   else
   {
     /* Top line of OSD is in EVEN field (field 1) */
     pOsd->dwAddrField1 = (u_int32)CLEAR_FLAGS(pRgn->pImage, NCR_BASE);
     pOsd->dwAddrField2 = pOsd->dwAddrField1 + pRgn->dwStride;
   }

   /* The upper 3 address bits (26-24) are located in the MSB of address 2. */
   /* They tried to make this as ugly as possible! */
   pOsd->dwAddrField2 |= (pOsd->dwAddrField1 & 0x07000000) << 5;
   pOsd->dwAddrField1 &= FA_ADDRESS_MASK;
   pOsd->dwAddrField1 |=
      ((pRgn->dwOptions & OSD_RO_FFENABLE) ? FA_FLICKER_FILTER_ENABLE : 0) |
      ((pRgn->dwOptions & OSD_RO_ALPHAENABLE) ? FA_ALPHA_BLEND_ENABLE : 0) |
      ((pRgn->dwOptions & OSD_RO_COLORKEYENABLE) ? FA_COLOR_KEY_ENABLE : 0) |
      ((pRgn->dwOptions & OSD_RO_ARCENABLE) ? FA_ASPECT_RATIO_CONV_ENABLE : 0) |
      ((pRgn->dwOptions & OSD_RO_ALPHABOTHVIDEO) ? FA_ALPHA_BOTH_VIDEO : 0) |
      ((pRgn->dwOptions & OSD_RO_ALPHATOPVIDEO) ? FA_ALPHA_TOP_VIDEO : 0) |
      ((pRgn->dwOptions & OSD_RO_FORCEREGIONALPHA) ? FA_FORCE_REGION_ALPHA : 0);
      
   #if GRAPHICS_PLANES == GP_DUAL
   /* Turning on blending of plane 0 and 1 is specified in plane 0 only */
   if (pRgn->uPlane == 0)
   {
      pOsd->dwAddrField1 |=
         ((pRgn->dwOptions & OSD_RO_OBSCUREPLANE1) ? 0 : FA_BLEND_PLANE1);
   } /* endif */
   #endif   

   pOsd->dwFieldStride = (pRgn->dwStride << 1) | (pRgn->dwStride << 17);
   pOsd->dwField1Pos = MAKE_FIELDPOS(nF1YStart, nF1YEnd, pRgn->dwRgnAlpha);
   pOsd->dwField2Pos = MAKE_FIELDPOS(nF2YStart, nF2YEnd, 0);
   
   /* Set the palette pointer. */
   pOsd->dwPaletteAddr = (u_int32)pRgn->pPalette;

   /* Copy the new OSD settings into the active header */
   /* This is a little dangerous:                      */
   /*    We may have modified the X/Y and made the     */
   /*    linked list become in the wrong order         */
   /*    There is a non-CS protected window before the */
   /*    code calling us updates the linked list.      */ 
   /* This is only likely to be problem for a large    */
   /* number of OSD's on the screen, as the DRM        */
   /* preloads the first 4 rgn headers on retrace      */
   if (uHdrToUse == 1)
   {
      /* Save the "next" pointer in the inactive hdr  */
      pNextOsd               = pRgn->OSDHdr1.pNextOsd;
      
      /* Overwrite it with the value from active hdr  */
      pRgn->OSDHdr1.pNextOsd = pRgn->OSDHdr2.pNextOsd;
      
      /* Then copy the inactive header into active    */
      pRgn->OSDHdr2          = pRgn->OSDHdr1;
      
      /* Now restore the next ptr in the inactive hdr */
      pRgn->OSDHdr1.pNextOsd = pNextOsd;

   }
   else
   {

      /* Save the "next" pointer in the inactive hdr  */
      pNextOsd               = pRgn->OSDHdr2.pNextOsd;
      
      /* Overwrite it with the value from active hdr  */
      pRgn->OSDHdr2.pNextOsd = pRgn->OSDHdr1.pNextOsd;
      
      /* Then copy the inactive header into active    */
      pRgn->OSDHdr1          = pRgn->OSDHdr2;
      
      /* Now restore the next ptr in the inactive hdr */
      pRgn->OSDHdr2.pNextOsd = pNextOsd;

   } /* endif header to use is 1 */
   
   critical_section_end(ks);


   return TRUE;
}

/*****************************************************************************/
/* Function: GenerateOSDLinkList()                                           */
/*                                                                           */
/* Parameters: void                                                          */
/*                                                                           */
/* Returns: void                                                             */
/*                                                                           */
/* Description: Creates the linked list of OSD regions from the list of      */
/*              OSDLIBREGIONs                                                */
/*****************************************************************************/
void GenerateOSDLinkList(u_int32 uPlane)
{
   u_int32 dwLastX = 0;
   u_int32 dwLastY = 0;
   u_int32 dwNewY = 0x7FFFFFFF;
   u_int32 dwNewX = 0x7FFFFFFF;
   u_int32 dwX, dwY;
   POSDLIBREGION pRgn;
   POSDLIBREGION pNewRgn;
   POSDLIBREGION pRgnCount;
   POSDHEADER pOSDList = NULL;
   POSDHEADER pOSDListPrev = NULL;
   bool ksPrev;
   int nFieldCount;               /* Field count local copy      */
   int32 nLineNum;                /* Current line of display     */
   int uHdrToUse;                 /* Which headers to use for LL */
   
   ksPrev = critical_section_begin();

   /* Decide whether to update now, or wait for vertical retrace           */
   /* If we are closer to retrace than (n) lines, wait until after retrace */ 
   nLineNum = ((*glpDrmStatus) & DRM_STATUS_LINE_MASK) >> DRM_STATUS_LINE_SHIFT;
   if (nLineNum > ((gnOsdMaxHeight/2) - (OSD_LL_UPDATE_SAFETY)))
   {
   
      /* Wait for the field change to load the DRM registers.             */
      /* We don't want to delete any memory if DRM is still using it.     */
      /* We must be outside critical section while waiting for field      */
      /* to change, or we will hang forever.                              */
      
      #if (CUSTOMER == VENDOR_D) && (defined OPENTV_12)
      /* Vendor D turns off the encoder in standby mode so this code will */
      /* hang here until the encoder is turned back on if we wait for the */
      /* next field while in standby.                                     */
      if (!gbStandbyMode)
      {
         critical_section_end(ksPrev);
         nFieldCount = gnFieldCount;
         while (nFieldCount == gnFieldCount);
         ksPrev = critical_section_begin();
      }
      #else
      critical_section_end(ksPrev);
      nFieldCount = gnFieldCount;
      while (nFieldCount == gnFieldCount);
      ksPrev = critical_section_begin();
      #endif

   } /* endif */

   /* Now we are in a critical section to do the linked list generation */
   /* First, decide which set of headers to use                         */
   /* If pending, use the header set which is marked as pending since   */
   /* it is not active (quite) yet.                                     */
   /* If not pending, use the header set marked "active" which is a     */
   /* somewhat confusing legacy of the old code - it actually means     */
   /* which one is inactive.                                            */

   if (gbPendingOSDLLUpdate[uPlane])
   {
      uHdrToUse = guPendingOSDHdr[uPlane];
   } 
   else 
   {
      uHdrToUse = gActiveOSDHdr[uPlane];
      guPendingOSDHdr[uPlane] = uHdrToUse;
      gbPendingOSDLLUpdate[uPlane] = TRUE;
      
      /* Advance which header is active for after retrace */
      if (gActiveOSDHdr[uPlane] == 1)
      {
         gActiveOSDHdr[uPlane] = 2;
      }
      else
      {
         gActiveOSDHdr[uPlane] = 1;
      }
   } /* endif pending OSD Linked list update */

   /* OK, now we can go and do the update itself */
   pRgnCount = gpOsdLibList[uPlane];
   while (pRgnCount)
   {
      pRgn = gpOsdLibList[uPlane];
      pNewRgn = NULL;

      while (pRgn) /* Search the list of library regions for the next */
      {            /* logical x-y positioned library. */
         if (pRgn->dwOptions & OSD_RO_ENABLE)
         {
            if (uHdrToUse == 1)
            {
               dwX = pRgn->OSDHdr1.dwXStartWidth & XSW_XSTART_MASK;
               dwY = pRgn->OSDHdr1.dwField1Pos & FP_START_MASK;
               if (dwY > (pRgn->OSDHdr1.dwField2Pos & FP_START_MASK))
                  dwY = pRgn->OSDHdr1.dwField2Pos & FP_START_MASK;
            }
            else
            {
               dwX = pRgn->OSDHdr2.dwXStartWidth & XSW_XSTART_MASK;
               dwY = pRgn->OSDHdr2.dwField1Pos & FP_START_MASK;
               if (dwY > (pRgn->OSDHdr2.dwField2Pos & FP_START_MASK))
                  dwY = pRgn->OSDHdr2.dwField2Pos & FP_START_MASK;
            }

            /* Is this region on the same line as the last one we linked? */
            if (dwY == dwLastY)
            {
               /* Yes. Is it right of the last region? */
               if (dwX > dwLastX)
               {
                  /* Yes. Is it left of any other region we have found on this line so far? */
                  if(((dwNewY == dwLastY) && (dwX < dwNewX)) || (dwNewY > dwY))
                  {
                    /* Yes - pick it as the next best choice */
                    dwNewX = dwX;
                    dwNewY = dwY;
                    pNewRgn = pRgn;
                  }  
               }
            }
            else if ((dwY > dwLastY) && (dwY <= dwNewY))
            {
               if (dwY == dwNewY)
               {
                  if (dwX < dwNewX)
/* EMC                  if ((dwX > dwLastX) && (dwX < dwNewX)) */
                  {
                     dwNewX = dwX;
                     dwNewY = dwY;
                     pNewRgn = pRgn;
                  }
               }
               else
               {
                  dwNewX = dwX;
                  dwNewY = dwY;
                  pNewRgn = pRgn;
               }
            }
         }
         pRgn = pRgn->pNext;
      }  /* Found the next region in the list. */

      /* pNewRgn points to the next region. */
      /* Update the OSDHEADER.pNextOsd pointer. */
      if (pNewRgn)
      {
         dwLastX = dwNewX;
         dwLastY = dwNewY;

         dwNewY = 0x7FFFFFFF;
         dwNewX = 0x7FFFFFFF;
         
         /* Now link it into the list */
         if (pOSDListPrev)
         {
            if (uHdrToUse == 1)
            {
               pNewRgn->OSDHdr1.pNextOsd = NULL;  /* Mark as last before append */
               pOSDListPrev->pNextOsd = &pNewRgn->OSDHdr1;
            }
            else
            {
               pNewRgn->OSDHdr2.pNextOsd = NULL;  /* Mark as last before append */
               pOSDListPrev->pNextOsd = &pNewRgn->OSDHdr2;
            }
         }
         else
         {
            if (uHdrToUse == 1)
            {
               pNewRgn->OSDHdr1.pNextOsd = NULL;  /* Mark as last before append */
               pOSDList = &pNewRgn->OSDHdr1;
            }
            else
            {
               pNewRgn->OSDHdr2.pNextOsd = NULL;  /* Mark as last before append */
               pOSDList = &pNewRgn->OSDHdr2;
            }
         }

         if (uHdrToUse == 1)
         {
            pOSDListPrev = &pNewRgn->OSDHdr1;
         }
         else
         {
            pOSDListPrev = &pNewRgn->OSDHdr2;
         }
      }

      pRgnCount = pRgnCount->pNext;
   }
   
   /* Update the hardware ptr to the linked list */
   if (uPlane == 0)
   {
      CNXT_SET_VAL(DRM_OSD_POINTER_REG, DRM_ADDRESS_ADDRESS_MASK, (u_int32)pOSDList);
   #if GRAPHICS_PLANES == GP_DUAL      
   } else {
      CNXT_SET_VAL(DRM_OSD1_POINTER_REG, DRM_ADDRESS_ADDRESS_MASK, (u_int32)pOSDList);
   #endif
   } /* endif */
   
   critical_section_end(ksPrev);
}

/*****************************************************************************/
/* Function: RLCopy2Screen()                                                 */
/*                                                                           */
/* Parameters: a lot                                                         */
/*                                                                           */
/* Returns: void                                                             */
/*                                                                           */
/* Description: RLE copy.                                                    */
/*****************************************************************************/
void RLCopy2Screen(u_int8 *pDst, u_int8 *pSrcData,
                   unsigned short *RunLengthOffsets,
                   int DstX, int DstY, int ExtX, int ExtY, int SrcX, int SrcY,
                   LPVOID lpBuffer, u_int32 dwStride, u_int8 byBpp)
{
   u_int8 *pSrcLine, *pDstData;
   int i, j;
   u_int8 *pTmp;
   signed char c;
   int     Count;
   int SkipX;

   trace_new(OSD_FUNC_TRACE1, "OSD:RLCopy2Screen srcX=%d, srcY=%d, dstX=%d, dstY=%d, extX=%d, extY=%d\n",
      SrcX, SrcY, DstX, DstY, ExtX, ExtY);
   #ifdef NO_HW
   pDstData = (u_int8 *)lpBuffer + DstY * dwStride + ((DstX << byBpp) >> 3);
   #else
   pDstData = (u_int8 *)lpBuffer + DstY * dwStride + ((DstX * byBpp) >> 3);
   #endif
   for (i = 0; i < ExtY; ++i)
   {
      /* get the pointer to the line */
      pSrcLine = pSrcData + RunLengthOffsets[SrcY++];
      pTmp =pDstData;
      #ifdef NO_HW
      j = (ExtX << byBpp) >> 3;
      SkipX = ((SrcX << byBpp) >> 3) & 0x03;
      #else
      j = (ExtX * byBpp) >> 3;
      SkipX = ((SrcX * byBpp) >> 3) & 0x03;
      #endif
      while (j)
      {
         if ( (c = *pSrcLine++) > 0)
            Count = (int) c;
         else
            Count = (int) 0 - (int) c;

         if (Count > j)
            Count = j;

         j -= Count;

         if (c > 0)
         {
            while (Count && SkipX)
            {
               --Count;
               --SkipX;
            }
            if (SkipX == 0)
            {
               /*FillBytes((int) pTmp, (int) *pSrcLine, (int) Count); */
               /* pTmp += Count; */
               while (Count--)
                  *pTmp++ = *pSrcLine;
            }
            ++pSrcLine;
         }
         else
         {
            while (Count && SkipX)
            {
               --Count;
               --SkipX;
               pSrcLine++;
            }
            if (SkipX == 0)
            {
               while (Count--)
                  *pTmp++ = *pSrcLine++;
            }
         }
      }
      pDstData += dwStride;
   }
}

/*****************************************************************************/
/* Function: GraphicsInit()                                                  */
/*                                                                           */
/* Parameters: void                                                          */
/*                                                                           */
/* Returns: void                                                             */
/*                                                                           */
/* Description: Init for the blit engine.                                    */
/*****************************************************************************/
void GraphicsInit()
{
   if ((GCP_Sem_ID = sem_create(1, "OSD3")) == 0)
   {
      trace_new(OSD_ERROR_MSG,"Graphics:GCP Sem Create Failed\n");
      return;
   }
#ifndef NO_HW
   GfxInit(8);
#endif
}

/*****************************************************************************/
/* Function: FBInit()                                                        */
/*                                                                           */
/* Parameters: void                                                          */
/*                                                                           */
/* Returns: void                                                             */
/*                                                                           */
/* Description: Unused.  Left in for old testcase compatibilty.              */
/*****************************************************************************/
void FBInit()
{
#ifdef PC_SIM
/*        SetupFB((int) gOSDBuffer, (int) Bpp); */
#endif
}

/*****************************************************************************/
/* Exported BitBlit Functions ************************************************/
/*****************************************************************************/
/*****************************************************************************/
/* Function: osd_bitblit_mem2screen()                                        */
/*                                                                           */
/* Parameters: OSDHANDLE hOSDRgn - Handle to destination OSD Region.         */
/*             u_int8 *data - Ptr to source image data.                      */
/*             u_int16 bytes_per_line - Stride of the source image.          */
/*             POSDRECT dst - Ptr to OSDRECT describing the blt area.        */
/*                                                                           */
/* Returns: void                                                             */
/*                                                                           */
/* Description: Memory to OSD Region bitblit.                                */
/*****************************************************************************/
void osd_bitblit_mem2screen(OSDHANDLE hOSDRgn, u_int8 *data, u_int16 bytes_per_line, POSDRECT dst)
{
   int ExtX, ExtY;
   LPVOID lpBuffer;
   u_int32 dwStride;
   u_int8 byBpp;

   lpBuffer = (LPVOID)GetOSDRgnOptions(hOSDRgn, OSD_RO_FRAMEBUFFER);
   dwStride = GetOSDRgnOptions(hOSDRgn, OSD_RO_FRAMESTRIDE);
#ifdef NO_HW
   switch (GetOSDRgnBpp(hOSDRgn))
   {
    case 4:
       byBpp = GCP_4BPP;
       break;
    case 8:
       byBpp = GCP_8BPP;
       break;
    case 16:
       byBpp = GCP_16BPP;
       break;
    case 32:
       byBpp = GCP_32BPP;
       break;
    default:
       byBpp = GCP_4BPP;
   }
#else
   byBpp = GetOSDRgnBpp(hOSDRgn);
#endif

   ExtX = dst->right - dst->left;
   ExtY = dst->bottom - dst->top;
   trace_new(OSD_FUNC_TRACE1, "OSD:bitblt_mem2screen dstX=%d, dstY=%d, extX=%d, extY=%d\n",
      dst->right, dst->top, ExtX, ExtY);

   if ((ExtX>0) && (ExtY>0))
   {
     #ifdef NO_HW
     SWCopy2Screen((int)lpBuffer, (int)data, dwStride, (int)bytes_per_line,
        ExtX, ExtY, dst->left, dst->top, byBpp);
     #else
     sem_get(GCP_Sem_ID, KAL_WAIT_FOREVER);
     GfxCopy2Screen((u_int32 *)lpBuffer, (u_int32 *)data, dwStride,
            (int)bytes_per_line, ExtX, ExtY, dst->left, dst->top, byBpp);
#if 0
     GcpStart();
     HWCopy2Screen((int)lpBuffer, (int)data, dwStride, (int)bytes_per_line,
        ExtX, ExtY, dst->left, dst->top, byBpp);
     GcpStop();
#endif
     sem_put(GCP_Sem_ID);
     #endif
   }
}

/*****************************************************************************/
/* Function: osd_bitblit_fill()                                              */
/*                                                                           */
/* Parameters: OSDHANDLE hOSDRgn - Handle to destination OSD Region.         */
/*             u_int32 quad - The source data to fill.                       */
/*             POSDRECT dst - Ptr to OSDRECT describing the blt area.        */
/*                                                                           */
/* Returns: void                                                             */
/*                                                                           */
/* Description: OSD Region fill.                                             */
/*****************************************************************************/
void osd_bitblit_fill(OSDHANDLE hOSDRgn, u_int32 quad, POSDRECT dst)
{
   int ExtX, ExtY;
   LPVOID lpBuffer;
   u_int32 dwStride;
   u_int8 byBpp;

   lpBuffer = (LPVOID)GetOSDRgnOptions(hOSDRgn, OSD_RO_FRAMEBUFFER);
   dwStride = GetOSDRgnOptions(hOSDRgn, OSD_RO_FRAMESTRIDE);
#ifdef NO_HW
   switch (GetOSDRgnBpp(hOSDRgn))
   {
    case 4:
       byBpp = GCP_4BPP;
       break;
    case 8:
       byBpp = GCP_8BPP;
       break;
    case 16:
       byBpp = GCP_16BPP;
       break;
    case 32:
       byBpp = GCP_32BPP;
       break;
    default:
       byBpp = GCP_4BPP;
   }
#else
   byBpp = GetOSDRgnBpp(hOSDRgn);
#endif

   ExtX = dst->right - dst->left;
   ExtY = dst->bottom - dst->top;
   trace_new(OSD_FUNC_TRACE1, "OSD:bitblt_fill color=%x, dstX=%d, dstY=%d, extX=%d, extY=%d\n",
      quad, dst->right, dst->top, ExtX, ExtY);

   if ((ExtX>0) && (ExtY>0))
   {
     #ifdef NO_HW
     SWSolidFill((int) lpBuffer, dwStride, ExtX, ExtY, quad,
        dst->left, dst->top, byBpp);
     #else
     sem_get(GCP_Sem_ID, KAL_WAIT_FOREVER);
     GfxFillMem((u_int32 *)lpBuffer, dwStride, ExtX, ExtY, quad,
                dst->left, dst->top, byBpp, 0);
#if 0
     GcpStart();
     HWSolidFill((int)lpBuffer, dwStride, ExtX, ExtY, quad,
        dst->left, dst->top, byBpp);
     GcpStop();
#endif
     sem_put(GCP_Sem_ID);
     #endif
   }
}

/*****************************************************************************/
/* Function: osd_bitblit_xor_fill()                                          */
/*                                                                           */
/* Parameters: OSDHANDLE hOSDRgn - Handle to destination OSD Region.         */
/*             u_int32 quad - The source data to fill.                       */
/*             POSDRECT dst - Ptr to OSDRECT describing the blt area.        */
/*                                                                           */
/* Returns: void                                                             */
/*                                                                           */
/* Description: OSD Region xor fill.                                         */
/*****************************************************************************/
void osd_bitblit_xor_fill(OSDHANDLE hOSDRgn, u_int32 quad, POSDRECT dst)
{
   int ExtX, ExtY;
   LPVOID lpBuffer;
   u_int32 dwStride;
   u_int8 byBpp;

   lpBuffer = (LPVOID)GetOSDRgnOptions(hOSDRgn, OSD_RO_FRAMEBUFFER);
   dwStride = GetOSDRgnOptions(hOSDRgn, OSD_RO_FRAMESTRIDE);
#ifdef NO_HW
   switch (GetOSDRgnBpp(hOSDRgn))
   {
    case 4:
       byBpp = GCP_4BPP;
       break;
    case 8:
       byBpp = GCP_8BPP;
       break;
    case 16:
       byBpp = GCP_16BPP;
       break;
    case 32:
       byBpp = GCP_32BPP;
       break;
    default:
       byBpp = GCP_4BPP;
   }
#else
   byBpp = GetOSDRgnBpp(hOSDRgn);
#endif

   ExtX = dst->right - dst->left;
   ExtY = dst->bottom - dst->top;
   trace_new(OSD_FUNC_TRACE1, "OSD:bitblt_fill color=%x, dstX=%d, dstY=%d, extX=%d, extY=%d\n",
      quad, dst->right, dst->top, ExtX, ExtY);

   if ((ExtX>0) && (ExtY>0))
   {
     #ifdef NO_HW
     SWXORFill((int) lpBuffer, dwStride, ExtX, ExtY, quad,
        dst->left, dst->top, byBpp);
     #else
     sem_get(GCP_Sem_ID, KAL_WAIT_FOREVER);
     GfxFillMem((u_int32 *)lpBuffer, dwStride, ExtX, ExtY, quad,
                dst->left, dst->top, byBpp, 0x0A);
#if 0
     GcpStart();
     HWSolidFill((int)lpBuffer, dwStride, ExtX, ExtY, quad,
        dst->left, dst->top, byBpp);
     GcpStop();
#endif
     sem_put(GCP_Sem_ID);
     #endif
   }
}

/*****************************************************************************/
/* Function: osd_bitblit_rl_fill()                                           */
/*                                                                           */
/* Parameters: OSDHANDLE hOSDRgn - Handle to destination OSD Region.         */
/*             POSDPIXMAP pix - Ptr to OSDPIXMAP header.                     */
/*             POSDRECT dst - Ptr to OSDRECT describing the blt area.        */
/*                                                                           */
/* Returns: void                                                             */
/*                                                                           */
/* Description: RLE expansion to an OSD region.                              */
/*****************************************************************************/
int osd_bitblit_rl_fill(OSDHANDLE hOSDRgn, POSDPIXMAP pix, POSDRECT dst)
{
   unsigned short pix_width, pix_height, pixmap_width, pixmap_height;
   unsigned int SrcX, SrcY, DstX, DstY, ExtX, ExtY, ExtY1, ExtY2;
   u_int8 *pData;
   u_int8 *pDstData;
   LPVOID lpBuffer;
   u_int32 dwStride;
   u_int8 byBpp;

   lpBuffer = (LPVOID)GetOSDRgnOptions(hOSDRgn, OSD_RO_FRAMEBUFFER);
   dwStride = GetOSDRgnOptions(hOSDRgn, OSD_RO_FRAMESTRIDE);
#ifdef NO_HW
   switch (GetOSDRgnBpp(hOSDRgn))
   {
   case 4:
      byBpp = GCP_4BPP;
      break;
   case 8:
      byBpp = GCP_8BPP;
      break;
   case 16:
      byBpp = GCP_16BPP;
      break;
    case 32:
       byBpp = GCP_32BPP;
       break;
    default:
       byBpp = GCP_4BPP;
   }
#else
   byBpp = GetOSDRgnBpp(hOSDRgn);
#endif

   /* First clip the pixmap rectangle to the destination rectangle */
   DstX = dst->left;
   DstY = dst->top;
   SrcX = pix->rcBounds.left;
   SrcY = pix->rcBounds.top;
   pix_width  = pixmap_width  = pix->rcBounds.right - SrcX;
   pix_height = pixmap_height = pix->rcBounds.bottom - SrcY;
   if (pix_width > (dst->right - dst->left))
      pix_width = dst->right - dst->left;
   if (pix_height > (dst->bottom - dst->top))
      pix_height = dst->bottom - dst->top;
   ExtX = pix_width;
   ExtY = pix_height;
   trace_new(OSD_FUNC_TRACE1,
      "OSD:bitblt_rl_fill dstX=%d, dstY=%d, extX=%d, extY=%d\n",
      dst->right, dst->top, ExtX, ExtY);

   if ((ExtX>0) && (ExtY>0))
   {
     do
     {
        #ifdef NO_HW
        pDstData = (u_int8 *)lpBuffer + DstY * dwStride + ((DstX << byBpp) >> 3);
        #else
        pDstData = (u_int8 *)lpBuffer + DstY * dwStride + ((DstX * byBpp) >> 3);
        #endif
        do
        {
           if (pix->pmType == OSD_RLE)
           {
              SrcY = DstY % pixmap_height;
              ExtY1 = pixmap_height - SrcY;
              if (ExtY1 > ExtY)
                 ExtY1 = ExtY;
              ExtY2 = ExtY - ExtY1;
              RLCopy2Screen(pDstData, pix->pData, pix->pRLOffsets,
                 DstX, DstY, ExtX, ExtY1, SrcX, SrcY, lpBuffer, dwStride, byBpp);
              if (ExtY2)
              {
                 if (ExtY2)
                    SrcY = 0;
                 RLCopy2Screen(pDstData, pix->pData, pix->pRLOffsets,
                    DstX, DstY+ExtY1, ExtX, ExtY2, SrcX, SrcY, lpBuffer,
                    dwStride, byBpp);
              }
           }
           else
           {
              pData = (u_int8 *)pix->pData + (SrcY * pix->wStride);
              #ifdef NO_HW
              pData += ((SrcX << byBpp) >> 3);
              SWCopy2Screen((int)lpBuffer, (int)pData, dwStride,
                 (int)pix->wStride, ExtX, ExtY, DstX, DstY, byBpp);
              #else
              pData += ((SrcX * byBpp) >> 3);
              sem_get(GCP_Sem_ID, KAL_WAIT_FOREVER);
              GfxCopy2Screen((u_int32 *)lpBuffer, (u_int32 *)pData, dwStride,
                  (int)pix->wStride, ExtX, ExtY, DstX, DstY, byBpp);
#if 0
              GcpStart();
              HWCopy2Screen((int)lpBuffer, (int)pData, dwStride,
                 (int)pix->wStride, ExtX, ExtY, DstX, DstY, byBpp);
              GcpStop();
#endif
              sem_put(GCP_Sem_ID);
              #endif
           }
           DstX += ExtX;
           if ( (DstX + ExtX) > (u_int32)(dst->right) )
              ExtX = dst->right - DstX;
        } while (DstX < (u_int32)(dst->right));

        DstX = dst->left;
        DstY += ExtY;
        ExtX = pix_width;
        if ( (DstY + ExtY) > (u_int32)(dst->bottom) )
           ExtY = (DstY + ExtY) - dst->bottom;
     } while (DstY < (u_int32)(dst->bottom));
   }
   return 1;
}

/*****************************************************************************/
/* Function: osd_bitblit_screen2mem()                                        */
/*                                                                           */
/* Parameters: OSDHANDLE hOSDRgn - Handle to source OSD Region.              */
/*             u_int8 *data - Ptr to destination buffer.                     */
/*             u_int16 bytes_per_row - Destination buffer stride.            */
/*             POSDRECT src - Ptr to OSDRECT describing the blt area.        */
/*                                                                           */
/* Returns: void                                                             */
/*                                                                           */
/* Description: OSD region to memory blit.                                   */
/*****************************************************************************/
void osd_bitblit_screen2mem(OSDHANDLE hOSDRgn, u_int8 *data, u_int16 bytes_per_row, POSDRECT src)
{
   int ExtX, ExtY;
   LPVOID lpBuffer;
   u_int32 dwStride;
   u_int8 byBpp;

   lpBuffer = (LPVOID)GetOSDRgnOptions(hOSDRgn, OSD_RO_FRAMEBUFFER);
   dwStride = GetOSDRgnOptions(hOSDRgn, OSD_RO_FRAMESTRIDE);
#ifdef NO_HW
   switch (GetOSDRgnBpp(hOSDRgn))
   {
    case 4:
       byBpp = GCP_4BPP;
       break;
    case 8:
       byBpp = GCP_8BPP;
       break;
    case 16:
       byBpp = GCP_16BPP;
       break;
    case 32:
       byBpp = GCP_32BPP;
       break;
    default:
       byBpp = GCP_4BPP;
   }
#else
   byBpp = GetOSDRgnBpp(hOSDRgn);
#endif

   ExtX = src->right - src->left;
   ExtY = src->bottom - src->top;
   if ((ExtX>0) && (ExtY>0))
   {
#ifdef NO_HW
     SWCopy2Memory((int)data, (int)lpBuffer, (int)bytes_per_row, dwStride,
        ExtX, ExtY, src->left, src->top, byBpp);
#else
     sem_get(GCP_Sem_ID, KAL_WAIT_FOREVER);
     GfxCopy2Mem((u_int32 *)data,(u_int32 *)lpBuffer, (int)bytes_per_row,
            dwStride, ExtX, ExtY, src->left, src->top, byBpp);
#if 0
     GcpStart();
     SWCopy2Memory((int)data, (int)lpBuffer, (int)bytes_per_row, dwStride,
        ExtX, ExtY, src->left, src->top, byBpp);
     GcpStop();
#endif
     sem_put(GCP_Sem_ID);
#endif
   }
}

/******************************************************/
/******************************************************/
/**                                                  **/
/**  OpenTV 1.2 Transient Heap Management Functions  **/
/**                                                  **/
/******************************************************/
/******************************************************/
#ifdef OPENTV_12
/********************************************************************/
/*  FUNCTION:    osdRelocateTransientRegions                        */
/*                                                                  */
/*  PARAMETERS:  None                                               */
/*                                                                  */
/*  DESCRIPTION: Relocate any OSD regions currently stored in the   */
/*               transient heap to the permanent heap assuming      */
/*               space is available.                                */
/*                                                                  */
/*  RETURNS:     TRUE if all objects were successfully relocated,   */
/*               FALSE if some could not be relocated.              */
/*                                                                  */
/*  CONTEXT:     Must be called from a non-interrupt context        */
/*                                                                  */
/********************************************************************/
bool osdRelocateTransientRegions(void)
{
  #ifdef DEBUG
  not_interrupt_safe();
  #endif

  trace_new(TRACE_LEVEL_2|TRACE_OSD, "OSD: Relocating transient memory regions\n");

  /* To be completed. For the time being, we just leave any affected  */
  /* regions to be trashed by the video hardware when it starts up.   */

  return(TRUE);
}
#endif /* OPENTV_12 */

/****************************************************************************
 * Modifications:
 * $Log: 
 *  120  mpeg      1.119       5/14/04 9:49:40 AM     Xin Golden      CR(s) 
 *        9200 9201 : checked in the changes made by SteveG so osdlib doesn't 
 *        wait for vertical retrace for every change in the OSD position.
 *  119  mpeg      1.118       3/5/04 6:56:53 PM      Steve Glennon   CR(s) 
 *        8473 : Added support for OSD_RO_NOCLEARBUFFER (from API spec) into 
 *        CreateOSDRgn
 *        
 *  118  mpeg      1.117       10/24/03 4:19:35 PM    Sahil Bansal    CR(s): 
 *        7665 Added support for higher pll/clock speeds on for wabash revc1 
 *        parts.  now recv1
 *        will run mpeg arm/fclk at 250, cm fclk=240, mpeg sdram clk=142, and 
 *        cm blck=142.
 *  117  mpeg      1.116       9/19/03 4:08:02 PM     Lucy C Allevato SCR(s) 
 *        5519 :
 *        updated OSDInit to use the new TV encoder driver if it's included.
 *        
 *  116  mpeg      1.115       9/2/03 2:10:00 PM      Angela Swartz   SCR(s) 
 *        7202 :
 *        Initialized OSD1_CRIT bits in DWI control1 register
 *        
 *  115  mpeg      1.114       7/31/03 3:56:14 PM     Steven Jones    SCR(s): 
 *        5046 
 *        Add swap region functionality for flicker-free buffer switching.
 *        
 *  114  mpeg      1.113       7/22/03 7:33:30 PM     Angela Swartz   SCR(s) 
 *        6958 :
 *        modified the code to use the new/changed software config keys 
 *        VIDEO_FORMAT_DEFAULT & VIDEO_OUTPUT_STANDARD_DEFAULT
 *        
 *  113  mpeg      1.112       6/25/03 11:11:30 AM    Miles Bintz     SCR(s) 
 *        6822 :
 *        added initialization value to remove warning in release build
 *        
 *  112  mpeg      1.111       5/16/03 12:49:24 AM    Steve Glennon   SCR(s): 
 *        6390 6391 
 *        Fixed parentheses around clause on line 957 to remove warning
 *        
 *        
 *  111  mpeg      1.110       5/9/03 7:12:48 PM      Steve Glennon   SCR(s): 
 *        6224 6225 6190 6179 
 *        Added Dual Plane OSD support for Wabash class devices. Also fixed a 
 *        testh failure due to sharpening coefficients for Wabash. Now use 
 *        OSD_RO_PLANE1 on CreateOSDRgn to create the region in the secondary 
 *        plane. Default if not specified or if specifying OSD_RO_PLANE0 is to 
 *        be in the primary plane. It is not possible to change the plane after
 *         creation using SetOSDRgnOptions, and attempts will return an error. 
 *        Note that the secondary plane does not support Flicker Filtering or 
 *        Aspect Ratio Conversion, and attempts to set these at creation or 
 *        through SetOSDRgnOptions will cause a failure. Also, default is for 
 *        Plane0 to blend with plane1. To have Plane0 always be over plane1, 
 *        use the OSD_RO_OBSCUREPLANE1 setting on the region which is in 
 *        plane0.
 *        
 *        
 *  110  mpeg      1.109       3/3/03 4:38:34 PM      Lucy C Allevato SCR(s) 
 *        5636 :
 *        4bpp mode should have only 16 palette entries instead of 256.  As 
 *        long as the Palette Index is set correctly for a 4bpp mode, the 
 *        hardware will make sure to access the correct palette entry out of 
 *        the 256 total palette RAM locations.
 *        
 *  109  mpeg      1.108       2/13/03 12:16:58 PM    Matt Korte      SCR(s) 
 *        5479 :
 *        Removed old header reference
 *        
 *  108  mpeg      1.107       1/27/03 5:12:48 PM     Lucy C Allevato SCR(s) 
 *        5290 :
 *        make sure the stride and memory buffer are aligned
 *        
 *  107  mpeg      1.106       1/27/03 3:24:36 PM     Dave Wilson     SCR(s) 
 *        5320 :
 *        Split all video encoder-related functions out into the new file 
 *        encoder.c
 *        
 *  106  mpeg      1.105       1/22/03 5:23:24 PM     Dave Wilson     SCR(s) 
 *        5099 :
 *        Correctly initialised DRM_SHARP_REG in OSDInit. Although this had 
 *        been 
 *        working fine on other chips (where, presumably, the register 
 *        defaulted to a
 *        good value), Brazos broke without this code. Initialising these 
 *        registers 
 *        causes the video window colours to be correct.
 *        
 *  105  mpeg      1.104       1/16/03 2:00:14 PM     Dave Wilson     SCR(s) 
 *        5253 :
 *        In cases where the driver is included in a build without CONFMGR, the
 *         code
 *        now uses the VIDEO_OUTPUT_STANDARD_DEFAULT and 
 *        VIDEO_FORMAT_PANSCAN_DEFAULT
 *        swconfig parameters to set the video format and default 
 *        panscan/letterbox
 *        behaviour.
 *        
 *  104  mpeg      1.103       12/20/02 1:48:52 PM    Billy Jackman   SCR(s) 
 *        5072 :
 *        Made internal encoder setup table driven.  Made selection of DAC 
 *        output mode
 *        dependent upon config variable INTERNAL_ENCODER_OUTMODE which 
 *        defaults to a
 *        value suitable for Wabash and is overridden in Bronco config files 
 *        for Brazos.
 *        
 *  103  mpeg      1.102       12/17/02 3:36:54 PM    Dave Wilson     SCR(s) 
 *        5073 :
 *        Added call to cnxt_drm_init_filter_coefficients to initialise the 
 *        scaler on
 *        Brazos chips.
 *        
 *  102  mpeg      1.101       11/26/02 4:07:14 PM    Dave Wilson     SCR(s) 
 *        4902 :
 *        Changed to use label from config file to define which I2C bus device 
 *        is on.
 *        
 *  101  mpeg      1.100       9/25/02 10:06:42 PM    Carroll Vance   SCR(s) 
 *        3786 :
 *        Removing old DRM and AUD conditional bitfield code.
 *        
 *        
 *  100  mpeg      1.99        9/25/02 5:40:02 PM     Dave Wilson     SCR(s) 
 *        4653 :
 *        Fixed a bug in the sorting logic in GenerateOSDLinkList. In some 
 *        cases,
 *        where there were multiple OSD regions on the same line and others on 
 *        lowe
 *        lines further to the right of the screen, the logic messed up and you
 *         ended
 *        up loosing some regions.
 *        
 *  99   mpeg      1.98        9/25/02 1:22:38 PM     Dave Wilson     SCR(s) 
 *        4367 :
 *        Fixed UpdateOSDHeader and SetOSDRgnPalEntry to correctly handle 
 *        palette
 *        partitioning for 4bpp surfaces.
 *        
 *  98   mpeg      1.97        9/16/02 3:30:24 PM     Dave Wilson     SCR(s) 
 *        4567 :
 *        Added a call to video_set_size from UpdateMpgScaling to allow the 
 *        OpenTV 1.2
 *        driver to recalculate video window size and position if the MPEG size
 *         changes
 *        after the app has set scaling and positioning parameters (this 
 *        happens quite
 *        frequently - for example, see Sky Movies Active application startup).
 *        
 *  97   mpeg      1.96        9/6/02 4:21:36 PM      Matt Korte      SCR(s) 
 *        4498 :
 *        Fixed Warnings
 *        
 *  96   mpeg      1.95        8/5/02 11:55:30 AM     Ganesh Banghar  SCR(s): 
 *        4329 
 *        The bt861 device will have dacs b anc c outputs disabled when used 
 *        for PAL setup for the Brady platform to test to see if the heating of
 *         this device can be reduced.
 *        
 *  95   mpeg      1.94        7/12/02 8:19:46 AM     Steven Jones    SCR(s): 
 *        4176 
 *        Support Brady and other IRDs without a SCART switch.
 *        
 *  94   mpeg      1.93        7/11/02 9:31:48 AM     Billy Jackman   SCR(s) 
 *        4172 :
 *        Changed SetDisplayPosition API to take video output mode into account
 *         when
 *        limiting the output position of video to make sure that all video is 
 *        actually
 *        sent out by the DRM, allowing the last pixel interrupt to work at all
 *         display
 *        positions.
 *        
 *  93   mpeg      1.92        6/17/02 5:18:32 PM     Dave Wilson     SCR(s) 
 *        4030 :
 *        Previously, SetMpgAR only updated the mode if it was different from 
 *        the 
 *        current setting. Unfortunately, this caused problems with OpenTV's 
 *        SYSCAV
 *        test which causes SetAspectRatio to be called. If the display AR is 
 *        being
 *        changed, the code assumes that the call to SetMpgAR (which is made 
 *        second)
 *        will update the display. Unless panscan/letterbox was changed, 
 *        however, no
 *        update would be done. To fix this, I removed the check in SetMpgAR 
 *        and
 *        unconditionally update the scaling even if the new mode is the same 
 *        as it
 *        was before.
 *        
 *  92   mpeg      1.91        6/12/02 12:21:56 PM    Carroll Vance   SCR(s) 
 *        3786 :
 *        Removing DRM bitfields.
 *        
 *  91   mpeg      1.90        6/10/02 2:54:32 PM     Craig Dry       SCR(s) 
 *        3923 :
 *        Remove MPG bitfields unconditionally - Step 2
 *        
 *  90   mpeg      1.89        6/6/02 5:52:20 PM      Craig Dry       SCR(s) 
 *        3923 :
 *        Remove MPG bitfields conditionally - Step 1
 *        
 *  89   mpeg      1.88        6/5/02 12:43:42 PM     Dave Wilson     SCR(s) 
 *        3676 :
 *        Fixed a nasty bug in RLCopy2Screen which was interpreting the byBpp 
 *        parameter wrongly when NO_HW was undefined. This was causing RLE 
 *        blits to
 *        write outside the bounds supplied and was corrupting the OSD heap.
 *        
 *  88   mpeg      1.87        5/28/02 6:58:34 PM     Carroll Vance   SCR(s) 
 *        3786 :
 *        Removing DRM bitfields.
 *        
 *  87   mpeg      1.86        5/24/02 4:29:18 PM     Dave Wilson     SCR(s) 
 *        3842 :
 *        Fixed up Bt861 encoder and DRM settings to ensure we display the 
 *        maximun
 *        possible picture size for both NTSC and PAL. In case of NTSC, this is
 *         719 pixels
 *        by 480 lines. For PAL, this is 719 pixels by 575 lines (awaiting 
 *        input from
 *        San Diego on how to show full 720 pixels across each line). Origin 
 *        has been
 *        set to make sure that the bottom and right edges of the 720x576 image
 *         are 
 *        visible for PAL since BSkyB tests the case where a 1 pixel video 
 *        window is 
 *        positioned at these edges and we must be able to show this.
 *        
 *  86   mpeg      1.85        5/22/02 11:23:44 AM    Dave Wilson     SCR(s) 
 *        3849 :
 *        Switched the order of the SetDisplayAR and SetMpgAR calls inside 
 *        SetAspectRatio to ensure that SetMpgAR is called with the update 
 *        parameter
 *        set to TRUE. In this case, the new code to correctly sequence 
 *        letterbox to
 *        pan/scan switches will be used when the change is made via 
 *        SetAspectRatio too.
 *        
 *  85   mpeg      1.84        5/21/02 12:50:04 PM    Dave Wilson     SCR(s) 
 *        3609 :
 *        Reworked SetMpgAR to update MPG and DRM in a synchronised way and get
 *         rid of
 *        the nasty "bounce" on transitions from letterbox to panscan display.
 *        
 *  84   mpeg      1.83        5/20/02 4:32:46 PM     Dave Wilson     SCR(s) 
 *        3829 :
 *        Set up the DRM sharpness filters to mask a problem seen where 
 *        upscaled
 *        MPEG images exhibit lsb anomalies resulting in visible vertical lines
 *         in
 *        large, flat coloured areas.
 *        
 *  83   mpeg      1.82        5/20/02 3:31:24 PM     Dave Wilson     SCR(s) 
 *        3828 :
 *        Recent fix mistakenly changed the number of active pixels per line in
 *         PAL from
 *        720 to 704. This has now been corrected!
 *        
 *  82   mpeg      1.81        5/17/02 11:40:38 AM    Carroll Vance   SCR(s) 
 *        3786 :
 *        Removed DRM bitfields.
 *        
 *  81   mpeg      1.80        5/7/02 5:07:26 PM      Tim White       SCR(s) 
 *        3721 :
 *        Remove ENC_ bitfield (and hardcoded) value usage.
 *        
 *        
 *  80   mpeg      1.79        4/23/02 4:45:08 PM     Billy Jackman   SCR(s) 
 *        3604 :
 *        Modified to support single pixel horizontal adjustment.
 *        
 *  79   mpeg      1.78        4/5/02 1:53:18 PM      Dave Wilson     SCR(s) 
 *        3510 :
 *        Reimplement fixes lost as a result of moving the file back to 
 *        pre-bitfield-
 *        fix version.
 *        
 *  78   mpeg      1.77        4/5/02 1:00:00 PM      Dave Wilson     SCR(s) 
 *        3510 :
 *        
 *        
 *        Backed out changes for bitfields removal. Equivalent to 1.74
 *        
 *  77   mpeg      1.76        4/2/02 12:22:12 PM     Dave Wilson     SCR(s) 
 *        3488 :
 *        Reworked SetOutputType to more cleanly initialise the video encoders.
 *         Bt861
 *        is complete but some additional work is required for Bt865 and Wabash
 *         internal
 *        encoders (the code here works but is still a bit messy). New work 
 *        will be
 *        covered by Tracker #3489.
 *        
 *  76   mpeg      1.75        3/28/02 2:45:56 PM     Quillian Rutherford 
 *        SCR(s) 3468 :
 *        Removed bit fields
 *        
 *        
 *  75   mpeg      1.74        3/27/02 5:12:12 PM     Dave Wilson     SCR(s) 
 *        3455 :
 *        Added SetDisplayPosition and GetDisplayPosition to allow customers to
 *         
 *        move the display origin by +-16 pixels horizontally or vertically to 
 *        correct
 *        for any middleware positioning requirements.
 *        
 *  74   mpeg      1.73        3/20/02 11:19:10 AM    Dave Wilson     SCR(s) 
 *        3412 :
 *        To work around a bug in the CX2249x hardware involving upscaling of 
 *        4:2:0
 *        images when in PAL mode, we disable the DRM PAL control bit in OpenTV
 *         builds.
 *        This cures the problem in cases when motion video is not being 
 *        displayed
 *        along with the still image (VIDEO.C has been updated to turn this 
 *        control bit
 *        on and off when video is started and stopped since it affects AV 
 *        sync).
 *        
 *  73   mpeg      1.72        3/19/02 3:01:48 PM     Dave Wilson     SCR(s) 
 *        3409 3410 :
 *        Moved GraphicsInit above OSDDrvInit so that the GCP access semaphore 
 *        is
 *        created before OSDDrvInit does its first blit. This gets rid of a KAL
 *         warning
 *        during OpenTV EN2 initialisation.
 *        
 *  72   mpeg      1.71        2/22/02 10:45:42 AM    Dave Wilson     SCR(s) 
 *        3199 :
 *        Reordered some things in the OSDInit function and left the video 
 *        plane
 *        disabled. This is one aspect in preventing flashes of garbage on the 
 *        screen
 *        during initialisation.
 *        
 *  71   mpeg      1.70        2/19/02 2:11:52 PM     Dave Wilson     SCR(s) 
 *        3209 :
 *        Fixed SetDisplayAR. For non-OpenTV builds it used to do absolutely 
 *        nothing
 *        resulting in scaling failures for some combinations of display/mpeg 
 *        aspect
 *        ratio and panscan/letterbox selection.
 *        
 *  70   mpeg      1.69        1/21/02 1:59:24 PM     Dave Wilson     SCR(s) 
 *        3065 :
 *        
 *        
 *        
 *        UpdateOSDHeader used to trash the linked list. When used in a 
 *        multi-OSD
 *        environment, this could cause regions to mysteriously disappear or 
 *        reappear
 *        when APIs were called to affect the properties of totally different 
 *        regions.
 *        
 *  69   mpeg      1.68        1/2/02 4:50:14 PM      Dave Wilson     SCR(s) 
 *        2991 :
 *        SetOSDRegionPalette used to disallow attempts to set the palette for 
 *        16- or
 *        32-bpp RGB regions. Display of these regions is, however, via the 
 *        palette so
 *        this made life somewhat difficult. Notably the alpha values are 
 *        required when
 *        doing colour expansion blits using GXA and these were wrong as a 
 *        result of this
 *        limitation. Code now allows the relevant number of palette entries to
 *         be set
 *        for all RGB and RGBA modes.
 *        
 *  68   mpeg      1.67        12/12/01 2:22:16 PM    Billy Jackman   SCR(s) 
 *        2967 :
 *        When using RGBA16 modes, the palette set had 0xFF in all alpha 
 *        entries.  This
 *        had the effect of disabling per-pixel alpha for the surface since all
 *         alpha
 *        values in pixels were mapped through the palette to 0xFF, fully 
 *        opaque.  Changed
 *        the palette to use linearly increasing alpha values and, hence, allow
 *         the
 *        alpha values specified via foreground and background colors to be 
 *        seen as
 *        expected.
 *        
 *  67   mpeg      1.66        11/21/01 5:18:28 PM    Dave Wilson     SCR(s) 
 *        2917 :
 *        When using RGBA32 modes, the palette set had 0xFF in all alpha 
 *        entries.This
 *        had the effect of disabling per-pixel alpha for the surface since all
 *         alpha
 *        values in pixels were mapped through the palette to 0xFF, fully 
 *        opaque. Changed
 *        the palette to use linearly increasing alpha values and, hence, allow
 *         the 
 *        alpha values specified via foreground and background colours to be 
 *        seen as
 *        expected.
 *        
 *  66   mpeg      1.65        11/20/01 11:17:58 AM   Quillian Rutherford 
 *        SCR(s) 2754 :
 *        Updates for new build system and Wabash
 *        
 *  65   mpeg      1.64        10/29/01 1:29:30 PM    Dave Wilson     SCR(s) 
 *        2839 2840 2841 :
 *        Removed definition of NO_HW so that hardware acceleration will be 
 *        used by
 *        the driver.
 *        
 *  64   mpeg      1.63        10/19/01 6:11:32 PM    Angela          SCR(s) 
 *        2770 2785 :
 *        changed the init values of global gdwSrcWidth and gdwSrcHeight from 0
 *         to default values( 720, 576 ) in order to fix banner build break 
 *        problem
 *        
 *  63   mpeg      1.62        7/3/01 11:03:24 AM     Tim White       SCR(s) 
 *        2178 2179 2180 :
 *        Merge branched Hondo specific code back into the mainstream source 
 *        database.
 *        
 *        
 *  62   mpeg      1.61        4/20/01 12:11:08 PM    Dave Wilson     DCS1124: 
 *        Major changes to video memory management to get Sky Text app running
 *        
 *  61   mpeg      1.60        3/21/01 12:36:10 PM    Dave Wilson     DCS1398: 
 *        Merge Vendor D source file changes
 *        
 *  60   mpeg      1.59        3/5/01 4:19:56 PM      Senthil Veluswamy 
 *        "enable_rgb_outputs" to be called only for Vendor_D OTV 1.2 builds
 *        
 *  59   mpeg      1.58        2/16/01 5:17:58 PM     Dave Wilson     DCS1217: 
 *        Changes for Vendor D rev 7 IRD
 *        
 *  58   mpeg      1.57        2/11/01 11:57:00 PM    Steve Glennon   Changed 
 *        programming of BT861 registers 0B/0C (HBLANK) to be value
 *        0x112 rather than the 0x106 from before, conditional on OPENTV_12
 *        This is part of a fix for DCS#1173 to match horizontal positioning
 *        of existing ST box.
 *        
 *  57   mpeg      1.56        2/10/01 9:33:56 PM     Steve Glennon   Fix for 
 *        DCS #1150 - some images in BSkyB Stills tests look blurry.
 *        Changed guserVTapSelect to 0 (so code now selects 2 tap vfilter in 
 *        all
 *        cases). Previously was selecting 3 tap filtering in cases where it 
 *        was
 *        possible, and it looks crummy (and inconsistent between different 
 *        sizes of
 *        images)
 *        
 *  56   mpeg      1.55        2/2/01 5:05:04 PM      Tim Ross        DCS966.
 *        More tweaks to the video encoder settings for Hondo's internal
 *        encoder.
 *        
 *  55   mpeg      1.54        2/2/01 3:55:50 PM      Angela          Merged 
 *        Vendor_C changes to the code(mostly #if CUSTOMER==VENDOR_C clauses)
 *        see DCS#1049
 *        
 *  54   mpeg      1.53        2/1/01 1:38:40 PM      Tim Ross        DCS966.
 *        Modified settings for Hondo internal encoder to work in emulation.
 *        
 *  53   mpeg      1.52        1/26/01 11:28:46 AM    Dave Wilson     DCS1055: 
 *        Code no longer calls enable_rgb_outputs if built into an app which
 *        does not include the VENDOR_D driver.
 *        
 *  52   mpeg      1.51        1/26/01 11:17:38 AM    Tim Ross        DCS966. 
 *        Added internal encoder support for Hondo.
 *        
 *  51   mpeg      1.50        1/25/01 2:18:20 PM     Dave Wilson     DCS1010: 
 *        Changed SetDisplayOutput to call the SCART driver for Vendor D to
 *        handle turning on and off the RGB outputs.
 *        
 *  50   mpeg      1.49        1/17/01 5:42:18 AM     Dave Wilson     DCS968: 
 *        Added code to prevent hangs in OSD functions when the encoder is
 *        disabled on vendor D's IRD
 *        
 *  49   mpeg      1.48        1/9/01 1:40:22 PM      Tim Ross        DCS836 & 
 *        DCS914
 *        
 *  48   mpeg      1.47        1/4/01 8:13:56 PM      Lucy C Allevato Changes 
 *        in SetOutputType for 861 and PAL encoder programming to adjust
 *        blanking and active values. This fix applies to DCS 887 relating to
 *        video regions of 1 pixel width or height at 719 or line 575 not 
 *        displaying
 *        in the BSkyB video test 7.
 *        
 *  47   mpeg      1.46        12/11/00 10:21:24 AM   Miles Bintz     changed 
 *        #include "heap.h" to #include "cnxtheap.h"
 *        
 *  46   mpeg      1.45        12/7/00 12:48:56 PM    Dave Wilson     Fixed 
 *        SetDisplayOutput to enable/disable RGB DACs
 *        
 *  45   mpeg      1.44        12/6/00 12:13:18 PM    Dave Wilson     Corrected
 *         problems in UpdateOSDHeader that caused missing lines in some cases
 *        
 *  44   mpeg      1.43        11/27/00 4:17:42 AM    Dave Wilson     Added 
 *        code to check for installed encoder
 *        
 *  43   mpeg      1.42        11/16/00 1:41:50 PM    Dave Wilson     Now uses 
 *        a separate heap for all image allocation.
 *        
 *  42   mpeg      1.41        10/13/00 4:05:12 PM    Miles Bintz     removed 
 *        #include "osdttx.h"
 *        
 *  41   mpeg      1.40        10/12/00 2:56:36 PM    Dave Wilson     Fixed 
 *        video output types (RGB/YC/CVBS) for vendor D board
 *        
 *  40   mpeg      1.39        10/3/00 6:20:42 PM     Dave Wilson     Aspect 
 *        ratio handling changes to get SYSCVID VTS test working
 *        
 *  39   mpeg      1.38        10/3/00 3:44:16 PM     Lucy C Allevato Updated 
 *        UpdateOSDHeader for positioning OSDs on the screen.
 *        
 *  38   mpeg      1.37        10/2/00 9:08:10 PM     Lucy C Allevato Added 
 *        double buffered OSDHEADER support to prevent flicker when updating
 *        the linked list of active regions.
 *        
 *  37   mpeg      1.36        9/28/00 1:58:22 PM     Lucy C Allevato Fixed a 
 *        bug in UpdateOSDHeader that could cause flickering OSDs in some
 *        cases.
 *        
 *  36   mpeg      1.35        9/27/00 7:18:22 PM     Dave Wilson     Corrected
 *         a problem where gVideoOutput was being set incorrectly and 
 *        messing up the encoder SQ&C driver
 *        
 *  35   mpeg      1.34        9/27/00 12:44:28 PM    Dave Wilson     Changed 
 *        all I2C address #defines to the ones in the vendor header files.
 *        Fixed up encoder outputs correctly for Vendor D board.
 *        
 *  34   mpeg      1.33        9/25/00 12:43:00 PM    Lucy C Allevato Fixed up 
 *        i2C addresses to pull addresses from vendor specific headers.
 *        Changed some of the trace message levels to be less verbose unless
 *        trace level is set to be verbose. See osdlibc.h changes.
 *        
 *  33   mpeg      1.32        8/28/00 6:04:24 PM     Lucy C Allevato Fixes for
 *         warnings about possible uninitialized variable use. Code was fine
 *        but strict checking created complaints.
 *        
 *  32   mpeg      1.31        8/17/00 3:21:24 PM     Ismail Mustafa  Added 
 *        check in OSDInit for alreday initialized.
 *        
 *  31   mpeg      1.30        8/11/00 2:14:24 PM     Miles Bintz     pulled 
 *        osdTTXInit() out of osdInit().
 *        osdTTXInit must still be called ! AFTER ! osdInit()
 *        
 *  30   mpeg      1.29        6/8/00 11:57:12 AM     Lucy C Allevato Changes 
 *        to commenting style. Removed C++ commenting style. Fixes for some
 *        unsigned to signed conversion warnings.
 *        
 *  29   mpeg      1.28        5/11/00 12:08:38 PM    Lucy C Allevato Merged 
 *        some fixes needed to use hardware acceleration. Not enabled yet
 *        because some other fixes are still required. Added GCP_32BPP to the
 *        software cases so that 32BPP regions can be used.
 *        
 *  28   mpeg      1.27        5/8/00 5:21:34 PM      Rob Tilton      Moved the
 *         osdttxinit to the last init in the osd init sequence.
 *        
 *  27   mpeg      1.26        5/2/00 8:26:02 PM      Dave Wilson     Set scart
 *         type to unknown for boards that are not recognised.
 *        
 *  26   mpeg      1.25        4/27/00 4:18:04 PM     Rob Tilton      Fixed 
 *        region resize.
 *        
 *  25   mpeg      1.24        4/27/00 11:18:08 AM    Rob Tilton      Fixed the
 *         return code for set default palette.
 *        
 *  24   mpeg      1.23        4/27/00 9:27:10 AM     Rob Tilton      Added 
 *        SetOSDBuffer().
 *        
 *  23   mpeg      1.22        4/26/00 7:19:44 PM     Rob Tilton      During 
 *        region creation, the buffer is not filled with blackness if the
 *        buffer is passed in during the creation and not allocated.
 *        
 *  22   mpeg      1.21        4/25/00 11:18:12 PM    Dave Wilson     Removed 
 *        DDC references to get rid of hangs during OSDInit
 *        
 *  21   mpeg      1.20        4/25/00 10:58:48 PM    Dave Wilson     Added 
 *        SCART case for Klondike boards. Currently incomplete but at least
 *        the code doesn't trash other stuff now.
 *        
 *  20   mpeg      1.19        4/25/00 11:26:52 AM    Rob Tilton      Now 
 *        reading the board ID from the API instead of from the register.
 *        
 *  19   mpeg      1.18        4/24/00 6:43:36 PM     Lucy C Allevato Updates 
 *        to SetOutputType to fix compile error with new CONFIG0 register
 *        changes for various boards and vendors.
 *        
 *  18   mpeg      1.17        4/18/00 11:52:46 AM    Dave Wilson     Changed 
 *        calls to mem_nc_malloc/free after removal of an unused parameter
 *        
 *  17   mpeg      1.16        3/20/00 2:58:56 PM     Rob Tilton      Added DRM
 *         DWI Control 3 register init.
 *        
 *  16   mpeg      1.15        3/6/00 4:30:58 PM      Lucy C Allevato Updated 
 *        file to use gfxlib directory and global header gfxlib.h
 *        
 *  15   mpeg      1.14        3/3/00 2:36:46 PM      Rob Tilton      Renamed 
 *        CreateOSDRegion to CreateOSDRgn and added the parameters needed to 
 *        create a region with a pre-allocated buffer.  A macro has been 
 *        created in
 *        OSDLIBC.H to allow the continued usage of the call to CreateOSDRegion
 *         with 
 *        the orginal parameters.
 *        
 *  14   mpeg      1.13        2/29/00 5:16:38 PM     Lucy C Allevato Added 
 *        code for colorado graphics hardware. Still building for software
 *        only for now. Moved SetOutputType call to after the call to 
 *        OSDLibInit to
 *        fix a divide by zero problem.
 *        
 *  13   mpeg      1.12        2/28/00 3:41:36 PM     Rob Tilton      Added 
 *        call to vidSetHwBuffSize() in SetOutputType().
 *        
 *  12   mpeg      1.11        2/18/00 3:28:34 PM     Lucy C Allevato Updated 
 *        to add new 32 BPP formats.
 *        
 *  11   mpeg      1.10        2/17/00 2:24:38 PM     Lucy C Allevato Added 
 *        support for the 2 new 16 bpp formats in colorado.
 *        
 *  10   mpeg      1.9         2/15/00 2:19:04 PM     Dave Wilson     Corrected
 *         a buf in clearing new OSD regions that caused memory outside the
 *        image buffer to be corrupted.
 *        
 *  9    mpeg      1.8         2/14/00 12:31:08 PM    Dave Wilson     Added 
 *        checking for zero height/width rects in blit calls
 *        
 *  8    mpeg      1.7         2/1/00 5:31:48 PM      Dave Wilson     Changes 
 *        for OpenTV 1.2 support
 *        
 *  7    mpeg      1.6         1/20/00 5:25:08 PM     Rob Tilton      Fixed 
 *        typo with gnOsdMaxHeight and Width.
 *        
 *  6    mpeg      1.5         1/20/00 4:54:56 PM     Lucy C Allevato Changed 
 *        SetOSDRgnPalette to use DEFAULT_PALETTE_ALPHA define if
 *        bLoadAlpha is FALSE.
 *        
 *  5    mpeg      1.4         1/20/00 3:10:16 PM     Rob Tilton      The upper
 *         3 bits of the OSD F1 address have been moved the MSB of F2 address.
 *        
 *  4    mpeg      1.3         1/20/00 2:34:46 PM     Rob Tilton      Changes 
 *        for the OpenTv 1.2 driver.
 *        
 *  3    mpeg      1.2         1/19/00 5:45:48 PM     Rob Tilton      Additions
 *         for OpenTv 1.2
 *        
 *  2    mpeg      1.1         1/5/00 3:52:52 PM      Rob Tilton      Added the
 *         ifdef Final Hardware around encoderinit.
 *        
 *  1    mpeg      1.0         1/5/00 10:24:16 AM     Rob Tilton      
 * $
 * 
 *    Rev 1.116   19 Sep 2003 15:08:02   goldenx
 * SCR(s) 5519 :
 * updated OSDInit to use the new TV encoder driver if it's included.
 * 
 *    Rev 1.115   02 Sep 2003 13:10:00   swartzwg
 * SCR(s) 7202 :
 * Initialized OSD1_CRIT bits in DWI control1 register
 * 
 *    Rev 1.114   31 Jul 2003 14:56:14   joness
 * SCR(s): 5046 
 * Add swap region functionality for flicker-free buffer switching.
 * 
 *    Rev 1.113   22 Jul 2003 18:33:30   swartzwg
 * SCR(s) 6958 :
 * modified the code to use the new/changed software config keys VIDEO_FORMAT_DEFAULT & VIDEO_OUTPUT_STANDARD_DEFAULT
 * 
 *    Rev 1.112   25 Jun 2003 10:11:30   bintzmf
 * SCR(s) 6822 :
 * added initialization value to remove warning in release build
 * 
 *    Rev 1.111   15 May 2003 23:49:24   glennon
 * SCR(s): 6390 6391 
 * Fixed parentheses around clause on line 957 to remove warning
 * 
 * 
 *    Rev 1.110   09 May 2003 18:12:48   glennon
 * SCR(s): 6224 6225 6190 6179 
 * Added Dual Plane OSD support for Wabash class devices. Also fixed a testh failure due to sharpening coefficients for Wabash. Now use OSD_RO_PLANE1 on CreateOSDRgn to create the region in the secondary plane. Default if not specified or if specifying OSD_RO_PLANE0 is to be in the primary plane. It is not possible to change the plane after creation using SetOSDRgnOptions, and attempts will return an error. Note that the secondary plane does not support Flicker Filtering or Aspect Ratio Conversion, and attempts to set these at creation or through SetOSDRgnOptions will cause a failure. Also, default is for Plane0 to blend with plane1. To have Plane0 always be over plane1, use the OSD_RO_OBSCUREPLANE1 setting on the region which is in plane0.
 * 
 * 
 *    Rev 1.109   03 Mar 2003 16:38:34   goldenx
 * SCR(s) 5636 :
 * 4bpp mode should have only 16 palette entries instead of 256.  As long as the Palette Index is set correctly for a 4bpp mode, the hardware will make sure to access the correct palette entry out of the 256 total palette RAM locations.
 * 
 *    Rev 1.108   13 Feb 2003 12:16:58   kortemw
 * SCR(s) 5479 :
 * Removed old header reference
 * 
 *    Rev 1.107   27 Jan 2003 17:12:48   goldenx
 * SCR(s) 5290 :
 * make sure the stride and memory buffer are aligned
 * 
 *    Rev 1.106   27 Jan 2003 15:24:36   dawilson
 * SCR(s) 5320 :
 * Split all video encoder-related functions out into the new file encoder.c
 * 
 *    Rev 1.105   22 Jan 2003 17:23:24   dawilson
 * SCR(s) 5099 :
 * Correctly initialised DRM_SHARP_REG in OSDInit. Although this had been 
 * working fine on other chips (where, presumably, the register defaulted to a
 * good value), Brazos broke without this code. Initialising these registers 
 * causes the video window colours to be correct.
 * 
 *    Rev 1.104   16 Jan 2003 14:00:14   dawilson
 * SCR(s) 5253 :
 * In cases where the driver is included in a build without CONFMGR, the code
 * now uses the VIDEO_OUTPUT_STANDARD_DEFAULT and VIDEO_FORMAT_PANSCAN_DEFAULT
 * swconfig parameters to set the video format and default panscan/letterbox
 * behaviour.
 * 
 *    Rev 1.103   20 Dec 2002 13:48:52   jackmaw
 * SCR(s) 5072 :
 * Made internal encoder setup table driven.  Made selection of DAC output mode
 * dependent upon config variable INTERNAL_ENCODER_OUTMODE which defaults to a
 * value suitable for Wabash and is overridden in Bronco config files for Brazos.
 * 
 *    Rev 1.102   17 Dec 2002 15:36:54   dawilson
 * SCR(s) 5073 :
 * Added call to cnxt_drm_init_filter_coefficients to initialise the scaler on
 * Brazos chips.
 * 
 *    Rev 1.101   26 Nov 2002 16:07:14   dawilson
 * SCR(s) 4902 :
 * Changed to use label from config file to define which I2C bus device is on.
 * 
 *    Rev 1.100   25 Sep 2002 21:06:42   vancec
 * SCR(s) 3786 :
 * Removing old DRM and AUD conditional bitfield code.
 * 
 * 
 *    Rev 1.99   25 Sep 2002 16:40:02   dawilson
 * SCR(s) 4653 :
 * Fixed a bug in the sorting logic in GenerateOSDLinkList. In some cases,
 * where there were multiple OSD regions on the same line and others on lowe
 * lines further to the right of the screen, the logic messed up and you ended
 * up loosing some regions.
 * 
 *    Rev 1.98   25 Sep 2002 12:22:38   dawilson
 * SCR(s) 4367 :
 * Fixed UpdateOSDHeader and SetOSDRgnPalEntry to correctly handle palette
 * partitioning for 4bpp surfaces.
 * 
 *    Rev 1.97   16 Sep 2002 14:30:24   dawilson
 * SCR(s) 4567 :
 * Added a call to video_set_size from UpdateMpgScaling to allow the OpenTV 1.2
 * driver to recalculate video window size and position if the MPEG size changes
 * after the app has set scaling and positioning parameters (this happens quite
 * frequently - for example, see Sky Movies Active application startup).
 * 
 *    Rev 1.96   06 Sep 2002 15:21:36   kortemw
 * SCR(s) 4498 :
 * Fixed Warnings
 * 
 *    Rev 1.95   05 Aug 2002 10:55:30   banghag
 * SCR(s): 4329 
 * The bt861 device will have dacs b anc c outputs disabled when used for PAL setup for the Brady platform to test to see if the heating of this device can be reduced.
 *
 *    Rev 1.94   12 Jul 2002 07:19:46   joness
 * SCR(s): 4176
 * Support Brady and other IRDs without a SCART switch.
 *
 *    Rev 1.93   11 Jul 2002 08:31:48   jackmaw
 * SCR(s) 4172 :
 * Changed SetDisplayPosition API to take video output mode into account when
 * limiting the output position of video to make sure that all video is actually
 * sent out by the DRM, allowing the last pixel interrupt to work at all display
 * positions.
 *
 *    Rev 1.92   17 Jun 2002 16:18:32   dawilson
 * SCR(s) 4030 :
 * Previously, SetMpgAR only updated the mode if it was different from the
 * current setting. Unfortunately, this caused problems with OpenTV's SYSCAV
 * test which causes SetAspectRatio to be called. If the display AR is being
 * changed, the code assumes that the call to SetMpgAR (which is made second)
 * will update the display. Unless panscan/letterbox was changed, however, no
 * update would be done. To fix this, I removed the check in SetMpgAR and
 * unconditionally update the scaling even if the new mode is the same as it
 * was before.
 *
 *    Rev 1.91   12 Jun 2002 11:21:56   vancec
 * SCR(s) 3786 :
 * Removing DRM bitfields.
 *
 *    Rev 1.90   10 Jun 2002 13:54:32   dryd
 * SCR(s) 3923 :
 * Remove MPG bitfields unconditionally - Step 2
 *
 *    Rev 1.89   06 Jun 2002 16:52:20   dryd
 * SCR(s) 3923 :
 * Remove MPG bitfields conditionally - Step 1
 *
 *    Rev 1.88   05 Jun 2002 11:43:42   dawilson
 * SCR(s) 3676 :
 * Fixed a nasty bug in RLCopy2Screen which was interpreting the byBpp
 * parameter wrongly when NO_HW was undefined. This was causing RLE blits to
 * write outside the bounds supplied and was corrupting the OSD heap.
 *
 *    Rev 1.87   28 May 2002 17:58:34   vancec
 * SCR(s) 3786 :
 * Removing DRM bitfields.
 *
 *    Rev 1.86   24 May 2002 15:29:18   dawilson
 * SCR(s) 3842 :
 * Fixed up Bt861 encoder and DRM settings to ensure we display the maximun
 * possible picture size for both NTSC and PAL. In case of NTSC, this is 719 pixels
 * by 480 lines. For PAL, this is 719 pixels by 575 lines (awaiting input from
 * San Diego on how to show full 720 pixels across each line). Origin has been
 * set to make sure that the bottom and right edges of the 720x576 image are
 * visible for PAL since BSkyB tests the case where a 1 pixel video window is
 * positioned at these edges and we must be able to show this.
 *
 *    Rev 1.85   22 May 2002 10:23:44   dawilson
 * SCR(s) 3849 :
 * Switched the order of the SetDisplayAR and SetMpgAR calls inside
 * SetAspectRatio to ensure that SetMpgAR is called with the update parameter
 * set to TRUE. In this case, the new code to correctly sequence letterbox to
 * pan/scan switches will be used when the change is made via SetAspectRatio too.
 *
 *    Rev 1.84   21 May 2002 11:50:04   dawilson
 * SCR(s) 3609 :
 * Reworked SetMpgAR to update MPG and DRM in a synchronised way and get rid of
 * the nasty "bounce" on transitions from letterbox to panscan display.
 *
 *    Rev 1.83   20 May 2002 15:32:46   dawilson
 * SCR(s) 3829 :
 * Set up the DRM sharpness filters to mask a problem seen where upscaled
 * MPEG images exhibit lsb anomalies resulting in visible vertical lines in
 * large, flat coloured areas.
 *
 *    Rev 1.82   20 May 2002 14:31:24   dawilson
 * SCR(s) 3828 :
 * Recent fix mistakenly changed the number of active pixels per line in PAL from
 * 720 to 704. This has now been corrected!
 *
 *    Rev 1.81   17 May 2002 10:40:38   vancec
 * SCR(s) 3786 :
 * Removed DRM bitfields.
 *
 *    Rev 1.80   07 May 2002 16:07:26   whiteth
 * SCR(s) 3721 :
 * Remove ENC_ bitfield (and hardcoded) value usage.
 *
 *
 *    Rev 1.79   23 Apr 2002 15:45:08   jackmaw
 * SCR(s) 3604 :
 * Modified to support single pixel horizontal adjustment.
 *
 *    Rev 1.78   05 Apr 2002 13:53:18   dawilson
 * SCR(s) 3510 :
 * Reimplement fixes lost as a result of moving the file back to pre-bitfield-
 * fix version.
 *
 *    Rev 1.77   05 Apr 2002 13:00:00   dawilson
 * SCR(s) 3510 :
 * Backed out changes for bitfields removal. Equivalent to 1.74
 *
 * ------ Change comments removed as a result of backing out to rev 1.74 ----
 *
 *    Rev 1.76   02 Apr 2002 12:22:12   dawilson
 * SCR(s) 3488 :
 * Reworked SetOutputType to more cleanly initialise the video encoders. Bt861
 * is complete but some additional work is required for Bt865 and Wabash internal
 * encoders (the code here works but is still a bit messy). New work will be
 * covered by Tracker #3489.
 *
 *    Rev 1.75   28 Mar 2002 14:45:56   rutherq
 * SCR(s) 3468 :
 * Removed bit fields
 *
 * ------ End of snipped comments ------
 *
 *    Rev 1.74   27 Mar 2002 17:12:12   dawilson
 * SCR(s) 3455 :
 * Added SetDisplayPosition and GetDisplayPosition to allow customers to
 * move the display origin by +-16 pixels horizontally or vertically to correct
 * for any middleware positioning requirements.
 *
 *    Rev 1.73   20 Mar 2002 11:19:10   dawilson
 * SCR(s) 3412 :
 * To work around a bug in the CX2249x hardware involving upscaling of 4:2:0
 * images when in PAL mode, we disable the DRM PAL control bit in OpenTV builds.
 * This cures the problem in cases when motion video is not being displayed
 * along with the still image (VIDEO.C has been updated to turn this control bit
 * on and off when video is started and stopped since it affects AV sync).
 *
 *    Rev 1.72   19 Mar 2002 15:01:48   dawilson
 * SCR(s) 3409 3410 :
 * Moved GraphicsInit above OSDDrvInit so that the GCP access semaphore is
 * created before OSDDrvInit does its first blit. This gets rid of a KAL warning
 * during OpenTV EN2 initialisation.
 *
 *    Rev 1.71   22 Feb 2002 10:45:42   dawilson
 * SCR(s) 3199 :
 * Reordered some things in the OSDInit function and left the video plane
 * disabled. This is one aspect in preventing flashes of garbage on the screen
 * during initialisation.
 *
 *    Rev 1.70   19 Feb 2002 14:11:52   dawilson
 * SCR(s) 3209 :
 * Fixed SetDisplayAR. For non-OpenTV builds it used to do absolutely nothing
 * resulting in scaling failures for some combinations of display/mpeg aspect
 * ratio and panscan/letterbox selection.
 *
 *    Rev 1.69   21 Jan 2002 13:59:24   dawilson
 * SCR(s) 3065 :
 *
 *
 *
 * UpdateOSDHeader used to trash the linked list. When used in a multi-OSD
 * environment, this could cause regions to mysteriously disappear or reappear
 * when APIs were called to affect the properties of totally different regions.
 *
 *    Rev 1.68   02 Jan 2002 16:50:14   dawilson
 * SCR(s) 2991 :
 * SetOSDRegionPalette used to disallow attempts to set the palette for 16- or
 * 32-bpp RGB regions. Display of these regions is, however, via the palette so
 * this made life somewhat difficult. Notably the alpha values are required when
 * doing colour expansion blits using GXA and these were wrong as a result of this
 * limitation. Code now allows the relevant number of palette entries to be set
 * for all RGB and RGBA modes.
 *
 *    Rev 1.67   12 Dec 2001 14:22:16   jackmaw
 * SCR(s) 2967 :
 * When using RGBA16 modes, the palette set had 0xFF in all alpha entries.  This
 * had the effect of disabling per-pixel alpha for the surface since all alpha
 * values in pixels were mapped through the palette to 0xFF, fully opaque.  Changed
 * the palette to use linearly increasing alpha values and, hence, allow the
 * alpha values specified via foreground and background colors to be seen as
 * expected.
 *
 *    Rev 1.66   21 Nov 2001 17:18:28   dawilson
 * SCR(s) 2917 :
 * When using RGBA32 modes, the palette set had 0xFF in all alpha entries.This
 * had the effect of disabling per-pixel alpha for the surface since all alpha
 * values in pixels were mapped through the palette to 0xFF, fully opaque. Changed
 * the palette to use linearly increasing alpha values and, hence, allow the
 * alpha values specified via foreground and background colours to be seen as
 * expected.
 *
 *    Rev 1.65   20 Nov 2001 11:17:58   rutherq
 * SCR(s) 2754 :
 * Updates for new build system and Wabash
 *
 *    Rev 1.64   29 Oct 2001 13:29:30   dawilson
 * SCR(s) 2839 2840 2841 :
 * Removed definition of NO_HW so that hardware acceleration will be used by
 * the driver.
 *
 *    Rev 1.63   19 Oct 2001 17:11:32   angela
 * SCR(s) 2770 2785 :
 * changed the init values of global gdwSrcWidth and gdwSrcHeight from 0 to default values( 720, 576 ) in order to fix banner build break problem
 *
 *    Rev 1.62   03 Jul 2001 10:03:24   whiteth
 * SCR(s) 2178 2179 2180 :
 * Merge branched Hondo specific code back into the mainstream source database.
 *
 *
 *    Rev 1.61.1.3   22 May 2001 14:33:08   prattac
 * DCS1642 removed references to DCS966
 *
 *    Rev 1.61.1.2   07 May 2001 12:47:08   prattac
 * Removed support for THOR.  Cleaned a warning.
 *
 *    Rev 1.61.1.1   07 May 2001 11:43:28   prattac
 * Removed THOR cases from scart code.
 *
 *    Rev 1.61.1.0   03 May 2001 18:37:14   prattac
 * Corrected use of PinAltFunc.
 *
 *    Rev 1.61   20 Apr 2001 11:11:08   dawilson
 * DCS1124: Major changes to video memory management to get Sky Text app running
 *
 *    Rev 1.60   21 Mar 2001 12:36:10   dawilson
 * DCS1398: Merge Vendor D source file changes
 *
 *    Rev 1.59   05 Mar 2001 16:19:56   velusws
 * "enable_rgb_outputs" to be called only for Vendor_D OTV 1.2 builds
 *
 *    Rev 1.58   16 Feb 2001 17:17:58   dawilson
 * DCS1217: Changes for Vendor D rev 7 IRD
 *
 *    Rev 1.57   11 Feb 2001 23:57:00   glennon
 * Changed programming of BT861 registers 0B/0C (HBLANK) to be value
 * 0x112 rather than the 0x106 from before, conditional on OPENTV_12
 * This is part of a fix for DCS#1173 to match horizontal positioning
 * of existing ST box.
 *
 *    Rev 1.56   10 Feb 2001 21:33:56   glennon
 * Fix for DCS #1150 - some images in BSkyB Stills tests look blurry.
 * Changed guserVTapSelect to 0 (so code now selects 2 tap vfilter in all
 * cases). Previously was selecting 3 tap filtering in cases where it was
 * possible, and it looks crummy (and inconsistent between different sizes of
 * images)
 *
 *    Rev 1.55   02 Feb 2001 17:05:04   rossst
 * DCS966.
 * More tweaks to the video encoder settings for Hondo's internal
 * encoder.
 *
 *    Rev 1.54   02 Feb 2001 15:55:50   angela
 * Merged Vendor_C changes to the code(mostly #if CUSTOMER==VENDOR_C clauses)
 * see DCS#1049
 *
 *    Rev 1.53   01 Feb 2001 13:38:40   rossst
 * DCS966.
 * Modified settings for Hondo internal encoder to work in emulation.
 *
 *    Rev 1.52   26 Jan 2001 11:28:46   dawilson
 * DCS1055: Code no longer calls enable_rgb_outputs if built into an app which
 * does not include the VENDOR_D driver.
 *
 *    Rev 1.51   26 Jan 2001 11:17:38   rossst
 * DCS966. Added internal encoder support for Hondo.
 *
 *    Rev 1.50   25 Jan 2001 14:18:20   dawilson
 * DCS1010: Changed SetDisplayOutput to call the SCART driver for Vendor D to
 * handle turning on and off the RGB outputs.
 *
 *    Rev 1.49   17 Jan 2001 05:42:18   dawilson
 * DCS968: Added code to prevent hangs in OSD functions when the encoder is
 * disabled on vendor D's IRD
 *
 *    Rev 1.48   09 Jan 2001 13:40:22   rossst
 * DCS836 & DCS914
 *
 *    Rev 1.47   04 Jan 2001 20:13:56   eching
 * Changes in SetOutputType for 861 and PAL encoder programming to adjust
 * blanking and active values. This fix applies to DCS 887 relating to
 * video regions of 1 pixel width or height at 719 or line 575 not displaying
 * in the BSkyB video test 7.
 *
 *    Rev 1.46   11 Dec 2000 10:21:24   bintzmf
 * changed #include "heap.h" to #include "cnxtheap.h"
 *
 *    Rev 1.45   07 Dec 2000 12:48:56   dawilson
 * Fixed SetDisplayOutput to enable/disable RGB DACs
 *
 *    Rev 1.44   06 Dec 2000 12:13:18   dawilson
 * Corrected problems in UpdateOSDHeader that caused missing lines in some cases
 *
 *    Rev 1.43   27 Nov 2000 04:17:42   dawilson
 * Added code to check for installed encoder
 *
 *    Rev 1.42   16 Nov 2000 13:41:50   dawilson
 * Now uses a separate heap for all image allocation.
 *
 *    Rev 1.41   13 Oct 2000 15:05:12   bintzmf
 * removed #include "osdttx.h"
 *
 *    Rev 1.40   12 Oct 2000 13:56:36   dawilson
 * Fixed video output types (RGB/YC/CVBS) for vendor D board
 *
 *    Rev 1.39   03 Oct 2000 17:20:42   dawilson
 * Aspect ratio handling changes to get SYSCVID VTS test working
 *
 *    Rev 1.38   03 Oct 2000 14:44:16   eching
 * Updated UpdateOSDHeader for positioning OSDs on the screen.
 *
 *    Rev 1.37   02 Oct 2000 20:08:10   eching
 * Added double buffered OSDHEADER support to prevent flicker when updating
 * the linked list of active regions.
 *
 *    Rev 1.36   28 Sep 2000 12:58:22   eching
 * Fixed a bug in UpdateOSDHeader that could cause flickering OSDs in some
 * cases.
 *
 *    Rev 1.35   27 Sep 2000 18:18:22   dawilson
 * Corrected a problem where gVideoOutput was being set incorrectly and
 * messing up the encoder SQ&C driver
 *
 *    Rev 1.34   27 Sep 2000 11:44:28   dawilson
 * Changed all I2C address #defines to the ones in the vendor header files.
 * Fixed up encoder outputs correctly for Vendor D board.
 *
 *    Rev 1.33   25 Sep 2000 11:43:00   eching
 * Fixed up i2C addresses to pull addresses from vendor specific headers.
 * Changed some of the trace message levels to be less verbose unless
 * trace level is set to be verbose. See osdlibc.h changes.
 *
 *    Rev 1.32   28 Aug 2000 17:04:24   eching
 * Fixes for warnings about possible uninitialized variable use. Code was fine
 * but strict checking created complaints.
 *
 *    Rev 1.31   17 Aug 2000 14:21:24   mustafa
 * Added check in OSDInit for alreday initialized.
 *
 *    Rev 1.30   11 Aug 2000 13:14:24   bintzmf
 * pulled osdTTXInit() out of osdInit().
 * osdTTXInit must still be called ! AFTER ! osdInit()
 *
 *    Rev 1.29   08 Jun 2000 10:57:12   eching
 * Changes to commenting style. Removed C++ commenting style. Fixes for some
 * unsigned to signed conversion warnings.
 *
 *    Rev 1.28   11 May 2000 11:08:38   eching
 * Merged some fixes needed to use hardware acceleration. Not enabled yet
 * because some other fixes are still required. Added GCP_32BPP to the
 * software cases so that 32BPP regions can be used.
 *
 *    Rev 1.27   08 May 2000 16:21:34   rtilton
 * Moved the osdttxinit to the last init in the osd init sequence.
 *
 *    Rev 1.26   02 May 2000 19:26:02   dawilson
 * Set scart type to unknown for boards that are not recognised.
 *
 *    Rev 1.25   27 Apr 2000 15:18:04   rtilton
 * Fixed region resize.
 *
 *    Rev 1.24   27 Apr 2000 10:18:08   rtilton
 * Fixed the return code for set default palette.
 *
 *    Rev 1.23   27 Apr 2000 08:27:10   rtilton
 * Added SetOSDBuffer().
 *
 *    Rev 1.22   26 Apr 2000 18:19:44   rtilton
 * During region creation, the buffer is not filled with blackness if the
 * buffer is passed in during the creation and not allocated.
 *
 *    Rev 1.21   25 Apr 2000 22:18:12   dawilson
 * Removed DDC references to get rid of hangs during OSDInit
 *
 *    Rev 1.20   25 Apr 2000 21:58:48   dawilson
 * Added SCART case for Klondike boards. Currently incomplete but at least
 * the code doesn't trash other stuff now.
 *
 *    Rev 1.19   25 Apr 2000 10:26:52   rtilton
 * Now reading the board ID from the API instead of from the register.
 *
 *    Rev 1.18   24 Apr 2000 17:43:36   eching
 * Updates to SetOutputType to fix compile error with new CONFIG0 register
 * changes for various boards and vendors.
 *
 *    Rev 1.17   18 Apr 2000 10:52:46   dawilson
 * Changed calls to mem_nc_malloc/free after removal of an unused parameter
 *
 *    Rev 1.16   20 Mar 2000 14:58:56   rtilton
 * Added DRM DWI Control 3 register init.
 *
 *    Rev 1.15   06 Mar 2000 16:30:58   eching
 * Updated file to use gfxlib directory and global header gfxlib.h
 *
 *    Rev 1.14   03 Mar 2000 14:36:46   rtilton
 * Renamed CreateOSDRegion to CreateOSDRgn and added the parameters needed to
 * create a region with a pre-allocated buffer.  A macro has been created in
 * OSDLIBC.H to allow the continued usage of the call to CreateOSDRegion with
 * the orginal parameters.
 *
 *    Rev 1.13   29 Feb 2000 17:16:38   eching
 * Added code for colorado graphics hardware. Still building for software
 * only for now. Moved SetOutputType call to after the call to OSDLibInit to
 * fix a divide by zero problem.
 *
 *    Rev 1.12   28 Feb 2000 15:41:36   rtilton
 * Added call to vidSetHwBuffSize() in SetOutputType().
 *
 *    Rev 1.11   18 Feb 2000 15:28:34   eching
 * Updated to add new 32 BPP formats.
 *
 *    Rev 1.10   17 Feb 2000 14:24:38   eching
 * Added support for the 2 new 16 bpp formats in colorado.
 *
 *    Rev 1.9   15 Feb 2000 14:19:04   dawilson
 * Corrected a buf in clearing new OSD regions that caused memory outside the
 * image buffer to be corrupted.
 *
 *    Rev 1.8   14 Feb 2000 12:31:08   dawilson
 * Added checking for zero height/width rects in blit calls
 *
 *    Rev 1.7   01 Feb 2000 17:31:48   dawilson
 * Changes for OpenTV 1.2 support
 *
 *    Rev 1.6   20 Jan 2000 17:25:08   rtilton
 * Fixed typo with gnOsdMaxHeight and Width.
 *
 *    Rev 1.5   20 Jan 2000 16:54:56   eching
 * Changed SetOSDRgnPalette to use DEFAULT_PALETTE_ALPHA define if
 * bLoadAlpha is FALSE.
 *
 *    Rev 1.4   20 Jan 2000 15:10:16   rtilton
 * The upper 3 bits of the OSD F1 address have been moved the MSB of F2 address.
 *
 *    Rev 1.3   20 Jan 2000 14:34:46   rtilton
 * Changes for the OpenTv 1.2 driver.
 *
 *    Rev 1.2   19 Jan 2000 17:45:48   rtilton
 * Additions for OpenTv 1.2
 *
 *    Rev 1.1   05 Jan 2000 15:52:52   rtilton
 * Added the ifdef Final Hardware around encoderinit.
 *
 *    Rev 1.0   05 Jan 2000 10:24:16   rtilton
 * Initial revision.
 *
*****************************************************************************/

