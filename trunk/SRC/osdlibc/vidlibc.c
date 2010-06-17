/****************************************************************************/
/*                   Conexant Proprietary and Confidential                  */
/*                       Software File/Module Header                        */
/*              Conexant Systems Inc. (c) 1998, 1999, 2000, 2001            */
/*                               Austin, TX                                 */
/*                            All Rights Eeserved                           */
/****************************************************************************/
/*
 * Filename:       vidlibc.c
 *
 *
 * Description:    API for programming the video path of the CX2249X's DRM.     
 *
 *
 * Author:         Rob Tilton (modified since by Eric Ching, Dave Wilson 
 *                 and Steve Glennon)
 *
 ****************************************************************************/
/* $Header: vidlibc.c, 105, 6/14/04 10:22:29 AM, Dave Wilson$
 ****************************************************************************/
 
/*****************************************************************************/
/* Functions:                                                                */
/***** Exported Live Video Functions *****************************************/
/*    vidLibInit()                                                           */
/*    vidCreateVideoRgn()                                                    */
/*    vidAllocateVideoRgn()                                                  */
/*    vidDestroyVideoRgn()                                                   */
/*    vidDestroyAllVideoRgn()                                                */
/*    vidQueryVideoRgnSize()                                                 */
/*    vidSetVideoRgnDimensions()                                             */
/*    vidSetOptions()                                                        */
/*    vidGetOptions()                                                        */
/*    vidSetPos()                                                            */
/*    vidGetPos()                                                            */
/*    vidDisplayRegion()                                                     */
/*    vidSetConnection()                                                     */
/*    vidGetConnection()                                                     */
/*    vidUpdateScaling()                                                     */
/*    vidGet420Hardware()                                                    */
/*    vidGetActiveRegion()                                                   */
/*    vidSetActiveRegion()                                                   */
/*    vidSetRegionInput()                                                    */
/*    vidGetTopRegion()                                                      */
/*    vidGetBottomRegion()                                                   */
/*    vidSetAlpha()                                                          */
/*    vidGetAlpha()                                                          */
/*    vidWipeEnable()                                                        */
/*    vidSetWipeRgn()                                                        */
/*    vidGetWipeRgn()                                                        */
/*    vidSetWipeLine()                                                       */
/*    vidGetWipeLine()                                                       */
/*    vidGetSafeStillBuffer()                                                */
/***** Internal Functions ****************************************************/
/*    vidUpdate420Scaling()                                                  */
/*    vidUpdate422Scaling()                                                  */
/*    vidCreateVideoRgnGeneric()                                             */
/*    vidSetHWBuffSize()                                                     */
/*****************************************************************************/
#include "stbcfg.h"
#define INCL_DRM
#define INCL_MPG
#include "kal.h"
#include "globals.h"
#include "retcodes.h"
#include "confmgr.h"
#include "osdlib.h"
#include "vidlib.h"
#include "vidprvc.h"
#include "video.h"


#define USE_GLOBAL_STILL_REGION

/***** Global Variables ******************************************************/
#define FULL_SCALE_VIDEO    0x400

#ifdef OPENTV_12
#define HALF_SCALE_VIDEO    0x200
#define DOUBLE_SCALE_VIDEO  0x800

#define VIDEO_HEAP_SIZE 0x220000

#include "cnxtheap.h"
heap_handle ghVidHeap;
heap_handle ghVidTransientHeap;
bool gbTransientHeapExists = FALSE;
bool gbBFrameBufferAvailable = FALSE;

/* Yes, this array is supposed to be enormous (2MB+) */
unsigned char gpVidHeap[VIDEO_HEAP_SIZE];
#endif

/* Keep a linked list of all of the allocated video regions. */ 
PVIDRGN gpVidRgn = NULL;

/* The hardware will support 2 video planes.  Each video plane will have an */
/* attached video region.                                                   */
PVIDRGN gpActiveVidRgn[VID_MAX_VID_RGN] = {NULL, NULL};

/* Each video region can have an associated wipe video region. */
PVIDRGN gpWipeVidRgn[VID_MAX_VID_RGN] = {NULL, NULL};

/* The hardware can only write into 1 video region at a time.  Better keep a  */
/* copy of the video region pointer that the hardware is connected.           */
PVIDRGN gp420Rgn = NULL;

/* The hardware has a decoded video buffer.  Keep a video region structure */
/* around to define the default video buffer.                              */
VIDRGN gvrMpg;
HVIDRGN ghvrMpg;

/* Keep a video region structure around to define the default */
/* still decode video buffer for use during still no display. */
#ifdef USE_GLOBAL_STILL_REGION
VIDRGN gvrVideoStill;
#endif
HVIDRGN ghVideoStill;

/*  Keep a video region structure around to define an alternate              */
/*  video region for use in attaching a video region to the multiple planes. */
/*  This is done because the video region being attached to the              */
/*  plane may already be attached to another plane. In any case, the         */
/*  region has already been allocated and it needs to be shown on another    */
/*  plane with perhaps different scaling.                                    */
VIDRGN gvrVidRgnMirror;
HVIDRGN ghVidRgnMirror;

extern u_int32 guserVTapSelect;
extern u_int32 gnOsdMaxHeight;
extern u_int32 gnOsdMaxWidth;
extern u_int32 gnHBlank;
extern u_int32 gnVBlank;
extern OSD_DISPLAY_AR gDisplayAR;
extern OSD_AR_MODE    gArMode;
extern u_int32 gdwLastMpgAnchor;
extern u_int32 gdwLastDisplayedAnchor;
extern int PlayState;
extern void video_send_hardware_command(u_int32);

bool vidRelocateTransientRegions(void);

#ifdef ENABLE_DISABLE_FILTER_FIX
bool bUseDRMFilterFix = TRUE;
#endif

#ifdef OPENTV_12
static bool vidCreateBFrameHeap(void);
#endif

static void vidCalculateVideoSrcAndDestination(PVIDRGN  pRgn, 
                                               OSDRECT *prectSrc, 
                                               OSDRECT *prectDest, 
                                               u_int32 *pdwPanOffset,
                                               bool    *pbPanScan, 
                                               bool    *pbLetterBox,
                                               bool    *pbEnableARConv);

/*****************************************************************************/

/***** Exported Functions ****************************************************/
/*****************************************************************************/
/* Function: vidLibInit()                                                    */
/*                                                                           */
/* Type: Exported                                                            */
/*                                                                           */
/* Parameters: void                                                          */
/*                                                                           */
/* Returns: bool - TRUE on success.                                          */
/*                                                                           */
/* Description: Initializes the DRM Video Library.                           */
/*****************************************************************************/
bool vidLibInit(void)
{
   bool bRet = TRUE;

   #ifdef OPENTV_12
   /* OpenTV 1.2 - we create a special heap to use when allocating  */
   /* video regions. This is needed to prevent fragmentation of the */
   /* direct heap.                                                  */
   ghVidHeap = heap_create(gpVidHeap, VIDEO_HEAP_SIZE);
   if(ghVidHeap == (heap_handle)NULL)
     return(FALSE);
   #endif
   
   CNXT_SET_VAL(glpDrmControl, DRM_CONTROL_EXT_VID_BUFF_ENABLE_MASK, 3); /*  Enable the first 2 LV fields. */
   CNXT_SET_VAL(glpDrmControl, DRM_CONTROL_EXT_VID_BUFF_SELECT_MASK, 1); /* Set the first field as top. */
   CNXT_SET_VAL(glpDrmControl, DRM_CONTROL_MPEG_ENABLE_MASK, 0);         /* Disable the mpg path.       */
   CNXT_SET_VAL(glpDrmControl, DRM_CONTROL_EXT_VID_ENABLE_MASK, 0);      /* Disable the evid path.      */
   CNXT_SET_VAL(glpDrmControl, DRM_CONTROL_EXT_VID_FIELD_MAP_MASK, 1);   /* Set the still field order   */
      
   CNXT_SET_VAL(glpDrmControl2, DRM_CONTROL_2_EXT_VID_BUFF_ENABLE_MASK, 3); /* Enable the first 2 LV fields. */
   CNXT_SET_VAL(glpDrmControl2, DRM_CONTROL_2_EXT_VID_BUFF_SELECT_MASK, 1); /* Set the first field as top.   */
   CNXT_SET_VAL(glpDrmControl2, DRM_CONTROL_2_MPEG_ENABLE_MASK, 0);         /* Disable the mpg path.         */
   CNXT_SET_VAL(glpDrmControl2, DRM_CONTROL_2_EXT_VID_FIELD_MAP_MASK, 1);   /* Set the still field order     */

   /* Disable tile and wipe. */
   #if (DRM_TILE_TYPE == DRM_TILE_TYPE_COLORADO)
   CNXT_SET_VAL(glpDrmMpgTileWipe[0], DRM_MPG_TILE_WIPE_CTRL_TILE_ENABLE_MASK, 0);
   CNXT_SET_VAL(glpDrmMpgTileWipe[1], DRM_MPG_TILE_WIPE_CTRL_TILE_ENABLE_MASK, 0);
   #else
   CNXT_SET_VAL(glpDrmMpgTileWipe[0], DRM_MPG_TILE_WIPE_CTRL_TILE_X_ENABLE_MASK, 0);
   CNXT_SET_VAL(glpDrmMpgTileWipe[0], DRM_MPG_TILE_WIPE_CTRL_TILE_Y_ENABLE_MASK, 0);
   CNXT_SET_VAL(glpDrmMpgTileWipe[1], DRM_MPG_TILE_WIPE_CTRL_TILE_X_ENABLE_MASK, 0);
   CNXT_SET_VAL(glpDrmMpgTileWipe[1], DRM_MPG_TILE_WIPE_CTRL_TILE_Y_ENABLE_MASK, 0);
   #endif
   CNXT_SET_VAL(glpDrmMpgTileWipe[0], DRM_MPG_TILE_WIPE_CTRL_WIPE_ENABLE_MASK, 0);
   CNXT_SET_VAL(glpDrmMpgTileWipe[1], DRM_MPG_TILE_WIPE_CTRL_WIPE_ENABLE_MASK, 0);

   /* Set the parameters for the default video region. */
   gvrMpg.pNext = NULL;
   gvrMpg.pPrev = NULL;
   gvrMpg.dwHeight = gnOsdMaxHeight;   /* Buffer height. */
   gvrMpg.dwWidth = 720;               /* Buffer width.  */
   gvrMpg.dwStride = 720;
   gvrMpg.vrType = TYPE_420;
   gvrMpg.pBuffer = (u_int8*)HWBUF_DEC_I_ADDR;
   gvrMpg.dwSize = HWBUF_DEC_I_SIZE;
   gvrMpg.vrConnection = HARDWARE;
   gvrMpg.rcSrc.top = 0;
   gvrMpg.rcSrc.left = 0;
   gvrMpg.rcSrc.right = 720;
   gvrMpg.rcSrc.bottom = gnOsdMaxHeight;
   gvrMpg.rcDst.top = 0;
   gvrMpg.rcDst.left = 0;
   gvrMpg.rcDst.right = 720;
   gvrMpg.rcDst.bottom = gnOsdMaxHeight;
   gvrMpg.arMode = OSD_ARM_NONE;
   gvrMpg.dwAR = MPG_VID_ASPECT_43;
   gvrMpg.dwSrcH = gnOsdMaxHeight;
   gvrMpg.dwSrcW = 720;
   gvrMpg.dwPan = 0;
   gvrMpg.dwScan = 0;
   gvrMpg.dwAutoMode = VID_AUTO_ALL;
   gvrMpg.dwVScale = 0x0400;
   gvrMpg.dwHScale = 0x0400;
   gvrMpg.bTop = TRUE;
   gvrMpg.dwUserInfo = 0;

   /* Init the parameters for a mpeg still decode region.
    * This region will only be used for displaying stills
    * with video play NO DISPLAY. It is expected that live
    * video will have been stopped and that it is safe to
    * use the static B frame buffer for decode which is
    * already big enough to decode a full PAL size still.
    */
   #ifdef USE_GLOBAL_STILL_REGION
   gvrVideoStill.pNext = NULL;
   gvrVideoStill.pPrev = NULL;
   gvrVideoStill.dwHeight = 576;
   gvrVideoStill.dwWidth = 720;
   gvrVideoStill.dwStride = 720;
   gvrVideoStill.vrType = TYPE_420;
   gvrVideoStill.pBuffer = (u_int8*)HWBUF_DEC_B_ADDR;
   gvrVideoStill.dwSize = HWBUF_DEC_B_SIZE;
   gvrVideoStill.vrConnection = HARDWARE;
   gvrVideoStill.rcSrc.top = 0;
   gvrVideoStill.rcSrc.left = 0;
   gvrVideoStill.rcSrc.right = 720;
   gvrVideoStill.rcSrc.bottom = 576;
   gvrVideoStill.rcDst.top = 0;
   gvrVideoStill.rcDst.left = 0;
   gvrVideoStill.rcDst.right = 720;
   gvrVideoStill.rcDst.bottom = 576;
   gvrVideoStill.arMode = OSD_ARM_NONE;
   gvrVideoStill.dwAR = MPG_VID_ASPECT_43;
   gvrVideoStill.dwSrcH = 576;
   gvrVideoStill.dwSrcW = 720;
   gvrVideoStill.dwPan = 0;
   gvrVideoStill.dwScan = 0;
   gvrVideoStill.dwAutoMode = VID_AUTO_ALL;
   gvrVideoStill.dwVScale = 0x0400;
   gvrVideoStill.dwHScale = 0x0400;
   gvrVideoStill.bTop = FALSE;
   ghVideoStill = (HVIDRGN)&gvrVideoStill;
   #else
   /* Make the still region handle just point to the MPEG region. To be perfectly */
   /* honest, I don't have any idea why these 2 region structures are needed and, */
   /* for OpenTV, they actually cause problems.                                   */
   ghVideoStill = (HVIDRGN)&gvrMpg;
   #endif

   ghVidRgnMirror = (HVIDRGN)&gvrVidRgnMirror;

   gp420Rgn = gpVidRgn = &gvrMpg;
   ghvrMpg = (HVIDRGN)&gvrMpg;
   vidSetOptions((HVIDRGN)gpVidRgn, VO_IMAGE_ON_TOP, TRUE, TRUE);
   
   #ifdef DRIVER_INCL_VIDEO
   /* Make sure that we have a valid image decoded so that the DRM does */
   /* not display a screenful of junk as soon as the video plane is     */
   /* enabled.                                                          */
   gen_video_decode_blank();
   #endif
   
   vidDisplayRegion((HVIDRGN)gpVidRgn, TRUE);

   return bRet;
}

/*****************************************************************************/
/* Function: vidQueryVideoRgnSize()                                          */
/*                                                                           */
/* Type: Exported                                                            */
/*                                                                           */
/* Parameters: u_int32 dwWidth   - Width of the requested image              */
/*             u_int32 dwHeight  - Height of the requested image             */
/*             VIDRGNTYPE vrType - Pixel type of image, 422 or 420.          */
/*                                                                           */
/* Returns: u_int32 - number of bytes to allocate for this image.            */
/*                                                                           */
/* Description: This function allows an application to query the amount of   */
/*              memory that must be allocated to store an image of a given   */
/*              size and type. This number may then be passed to the function*/
/*              vidAllocateVideoRgn to create a region capable of holding    */
/*              the image.                                                   */
/*****************************************************************************/
u_int32 vidQueryVideoRgnSize(u_int32 dwWidth, u_int32 dwHeight, VIDRGNTYPE vrType) 
{
   u_int32 dwStride;
   u_int32 dwSize;
   
   /* Calculate the stride.                      */
   /* 422 images are 16 bpp.  Stride = Width * 2 */
   /* 420 images are 12 bpp.  Stride = Width     */
   dwStride = dwWidth;
   if (vrType == TYPE_422)
      dwStride <<= 1;
 
   /* Word align the stride. */
   dwStride += 3;
   dwStride &= 0xFFFFFFFC;
 
   /* Calculate the amount of memory to allocate. */
   dwSize = dwHeight * dwStride;
 
   /* Remember, 4:2:0 is effectively 12bpp ! */
   if(vrType == TYPE_420)
     dwSize = (dwSize * 3)/ 2;
     
   return(dwSize);
}

/*****************************************************************************/
/* Function: vidSetVideoRgnDimensions()                                      */
/*                                                                           */
/* Type: Exported                                                            */
/*                                                                           */
/* Parameters: HVIDRGN hRgn      - The region whose dimensions are to be set */
/*             u_int32 dwWidth   - Width of the requested image              */
/*             u_int32 dwHeight  - Height of the requested image             */
/*                                                                           */
/* Returns:    TRUE on success, FALSE on failure                             */
/*                                                                           */
/* Description: This function is used to set the dimensions of an image      */
/*              created using the vidAllocateVideoRgn function. It is not    */
/*              valid to resize an existing region.                          */
/*****************************************************************************/
bool vidSetVideoRgnDimensions(HVIDRGN hRgn, u_int32 dwWidth, u_int32 dwHeight)
{
   u_int32 dwStride;
   u_int32 dwSize;
   PVIDRGN pRgn;
   bool bRet = FALSE;

   /* First check the video region handle. */
   pRgn = (PVIDRGN)hRgn;
   
   /* Dimensions must be non-zero and handle must be valid */
   if ((pRgn) && (dwWidth) && (dwHeight) && (pRgn != &gvrMpg))
   {
      /* Calculate the stride.                      */
      /* 422 images are 16 bpp.  Stride = Width * 2 */
      /* 420 images are 12 bpp.  Stride = Width     */
      dwStride = dwWidth;
      if (pRgn->vrType == TYPE_422)
         dwStride <<= 1;
 
      /* Word align the stride. */
      dwStride += 3;
      dwStride &= 0xFFFFFFFC;
 
      /* Calculate the amount of memory required */
      dwSize = dwHeight * dwStride;
      
      /* Check that the buffer associated with this video image is large enough */
      if (pRgn->dwSize >= dwSize)
      {
         /* Fill in the appropriate fields */
         pRgn->dwHeight     = dwHeight;
         pRgn->dwWidth      = dwWidth;
         pRgn->dwStride     = dwStride;
         pRgn->dwSrcH       = dwHeight;
         pRgn->dwSrcW       = dwWidth;
         pRgn->rcSrc.top    = 0;
         pRgn->rcSrc.left   = 0;
         pRgn->rcSrc.right  = dwWidth;
         pRgn->rcSrc.bottom = dwHeight;
         pRgn->rcDst.top    = 0;
         pRgn->rcDst.left   = 0;
         pRgn->rcDst.right  = dwWidth;
         pRgn->rcDst.bottom = dwHeight;

         bRet = TRUE;
      }
   }
   
   return bRet;
}

/*****************************************************************************
 * Function: vidCopyVideoRgnSettings()
 *
 * Type: Private exported function
 *
 * Parameters: HVIDRGN hSrcRgn - The source region
 *             HVIDRGN hDstRgn - The destination region
 *
 * Returns:    TRUE on success, FALSE on failure
 *
 * Description: This function is used to copy the settings of a video region
 *              that has already been created.
 *****************************************************************************/
bool vidCopyVideoRgnSettings(HVIDRGN hSrcRgn, HVIDRGN hDstRgn)
{
   PVIDRGN pSrcRgn, pDstRgn;
   bool bRet = FALSE;

   /* First check the video region handles. */
   pSrcRgn = (PVIDRGN)hSrcRgn;
   pDstRgn = (PVIDRGN)hDstRgn;
   if ((pSrcRgn) && (pDstRgn))
   {
      *pDstRgn = *pSrcRgn;
   }
   
   return bRet;
}

/*****************************************************************************/
/* Function: vidCreateVideoRgn()                                             */
/*                                                                           */
/* Type: Internal                                                            */
/*                                                                           */
/* Parameters: POSDRECT pRc      - Ptr describing the size of region to      */
/*                                 create.                                   */
/*             VIDRGNTYPE vrType - Pixel type of region, 422 or 420.         */
/*                                                                           */
/* Returns: HVIDRGN - A valid handle to a DRM Video Region on success.       */
/*                    NULL on failure.                                       */
/*                                                                           */
/* Description: Creates a video region and allocates all of the memory needed*/
/*****************************************************************************/
HVIDRGN vidCreateVideoRgn(POSDRECT pRc, VIDRGNTYPE vrType)
{
   return (vidCreateVideoRgnGeneric(pRc, 0, vrType));
}  

/*****************************************************************************/
/* Function: vidAllocateVideoRgn()                                           */
/*                                                                           */
/* Type: Exported                                                            */
/*                                                                           */
/* Parameters: u_int32 dwSize    - Number of bytes to allocate for buffer    */
/*             VIDRGNTYPE vrType - Pixel type of region, 422 or 420.         */
/*                                                                           */
/* Returns: HVIDRGN - A valid handle to a DRM Video Region on success.       */
/*                    NULL on failure.                                       */
/*                                                                           */
/* Description: Allocates memory for a video region. The size passed to this */
/*              function should be derived from a previous call to           */
/*              vidQueryVideoRgnSize and the image dimensions should be set  */
/*              with a later call to vidSetVideoRgnDimensions. This call is  */
/*              primarily for use by OpenTV 1.2 which has an "allocate first"*/
/*              model for creating video and image screens.                  */
/*****************************************************************************/
HVIDRGN vidAllocateVideoRgn(u_int32 dwSize, VIDRGNTYPE vrType)
{
   return (vidCreateVideoRgnGeneric(NULL, dwSize, vrType));
}

/*****************************************************************************/
/* Function: vidDestroyVideoRgn()                                            */
/*                                                                           */
/* Type: Exported                                                            */
/*                                                                           */
/* Parameters: HVIDRGN hRgn - Handle of video region to destroy.             */
/*                                                                           */
/* Returns: bool - TRUE on successful deletion.                              */
/*                                                                           */
/* Description: Removes the video region from the list of allocated video    */
/*              regions and frees all associated memory.                     */
/*****************************************************************************/
bool vidDestroyVideoRgn(HVIDRGN hRgn)
{
   bool bRet = FALSE;
   PVIDRGN pRgn;
   u_int32 dwActIndex;

   /* First check the video region handle. */
   pRgn = (PVIDRGN)hRgn;
   if ((pRgn) && (pRgn != &gvrMpg))
   {
      /* Now that we have a valid handle, check to see if the region is  */
      /* currently active.                                               */
      if ((gpActiveVidRgn[0] == pRgn) || (gpActiveVidRgn[1] == pRgn))
      {
         /* Get the index to the correct bank of registers. */
         dwActIndex = (gpActiveVidRgn[0] == pRgn) ? 0 : 1;

         /* Remove the active region from the active and disable it in DRM. */
         vidDisplayRegion(hRgn, FALSE);
      }

      /* Remove the region from the linked list. */
      if (pRgn->pNext)
         pRgn->pNext->pPrev = pRgn->pPrev;
      if (pRgn->pPrev)
         pRgn->pPrev->pNext = pRgn->pNext;
      else
         gpVidRgn = pRgn->pNext;

      /* Free the memory associated with the region. */
      if (pRgn->pBuffer)
      {
        #ifdef OPENTV_12
        vidImageBufferFree( (void *)CLEAR_FLAGS(pRgn->pBuffer, NCR_BASE), pRgn->bTransient);
        #else
         mem_nc_free(pRgn->pBuffer);
      
        #endif 
      }
      /* Free the memory holding the structure. */
      mem_free(pRgn);
      pRgn = NULL; /* Just in case. */

      /* Set a successful return code. */
      bRet = TRUE;
   }

   return bRet;
}

/*****************************************************************************/
/* Function: vidDestroyAllVideoRgn()                                         */
/*                                                                           */
/* Type: Exported                                                            */
/*                                                                           */
/* Parameters: void                                                          */
/*                                                                           */
/* Returns: bool - TRUE on successful deletion.                              */
/*                                                                           */
/* Description: Removes all video regions from the list of allocated video   */
/*              regions and frees all associated memory.                     */
/*****************************************************************************/
bool vidDestroyAllVideoRgn(void)
{
   bool bRet = TRUE;
   PVIDRGN pDelete = NULL;
   PVIDRGN pRgn;

   pRgn = gpVidRgn;
   while (pRgn)
   {
      /* Delete every region except for the default 420 HW region. */
      if (pRgn != &gvrMpg)
         pDelete = pRgn;

      pRgn = pRgn->pNext;

      if (pDelete)
         vidDestroyVideoRgn((HVIDRGN)pDelete);
      pDelete = NULL;
   }

   return bRet;
}

/*****************************************************************************/
/* Function: vidSetOptions()                                                 */
/*                                                                           */
/* Type: Exported                                                            */
/*                                                                           */
/* Parameters: HVIDRGN hRgn - Handle to the video region to update.          */
/*             VIDOPTION vOption - The option type to change.                */
/*             u_int32 dwVal - New value for the specified option.           */
/*             bool bUpdate - Update DRM registers.                          */
/*                                                                           */
/* Returns: bool - TRUE on success.                                          */
/*                                                                           */
/* Description: Changes a parameter to the given region.                     */
/*****************************************************************************/
bool vidSetOptions(HVIDRGN hRgn, VIDOPTION vOption, u_int32 dwVal, bool bUpdate)
{
   bool bRet = FALSE;
   PVIDRGN pRgn;

   pRgn = (PVIDRGN)hRgn;
   if (pRgn)
   {
      switch (vOption)
      {
      case VO_ASPECT_RATIO_MODE:       
         /* @@@SG Reset pan and scan vectors when mode changed to Letterbox */
         if ((pRgn->arMode != (OSD_AR_MODE)dwVal) && 
             (pRgn->arMode == OSD_ARM_LETTERBOX))
         {
            pRgn->dwPan = 0;
            pRgn->dwScan = 0;
         } 
         pRgn->arMode = (OSD_AR_MODE)dwVal;     
         bRet = TRUE;
         break;
      case VO_ASPECT_RATIO:
         pRgn->dwAR = dwVal;
         bRet = TRUE;
         break;
      case VO_CONNECTION:
         bRet = vidSetConnection(hRgn, (VIDRGNCONNECT)dwVal);
         break;
      case VO_IMAGE_HEIGHT:
         if (pRgn->dwSrcH != dwVal)
         {
            #ifndef USE_OLD_CODE
            /* Rescale the source rectangle appropriately for the new size to keep */
            /* it correct on the screen. Also fix up the scale factors to keep the */
            /* image the same size on screen.                                      */
            if(pRgn->dwHeight && !(pRgn->dwAutoMode & VID_AUTO_SCALE))
            {
              isr_trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: Rescaling source height and scale factor on size change for region 0x%x\n", hRgn, 0);
              isr_trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: Old height %d, scale factor 0x%x\n", pRgn->rcSrc.bottom-pRgn->rcSrc.top, pRgn->dwVScale);
              pRgn->rcSrc.top    = (pRgn->rcSrc.top    * dwVal) / pRgn->dwHeight;
              pRgn->rcSrc.bottom = (pRgn->rcSrc.bottom * dwVal) / pRgn->dwHeight;
              if(dwVal)
                pRgn->dwVScale   = (pRgn->dwVScale * pRgn->dwHeight)/dwVal;
              isr_trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: New height %d, scale factor 0x%x\n", pRgn->rcSrc.bottom-pRgn->rcSrc.top, pRgn->dwVScale);
            }
            else
            {
              pRgn->rcSrc.top    = 0;
              pRgn->rcSrc.bottom = dwVal;
            }
            #else
            /* @@@SG Set Src rectangle to size of video when we get a size change */
            pRgn->rcSrc.top    = 0;
            pRgn->rcSrc.bottom = dwVal;
            #endif
         } 
         pRgn->dwSrcH   = dwVal; 
         pRgn->dwHeight = dwVal; 
         bRet = TRUE;
         break;
      case VO_IMAGE_WIDTH:
         if (pRgn->dwSrcW != dwVal)
         {
            #ifndef USE_OLD_CODE
            /* Rescale the source rectangle appropriately for the new size to keep */
            /* it correct on the screen.                                           */
            if(pRgn->dwWidth && !(pRgn->dwAutoMode & VID_AUTO_SCALE))
            {
              isr_trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: Rescaling source width and scale factor on size change for region 0x%x\n", hRgn, 0);
              isr_trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: Old width %d, scale factor 0x%x\n", pRgn->rcSrc.right-pRgn->rcSrc.left, pRgn->dwHScale);
              pRgn->rcSrc.left  = (pRgn->rcSrc.left  * dwVal) / pRgn->dwWidth;
              pRgn->rcSrc.right = (pRgn->rcSrc.right * dwVal) / pRgn->dwWidth;
              if(dwVal)
                pRgn->dwHScale   = (pRgn->dwHScale * pRgn->dwWidth)/dwVal;
              isr_trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: New width %d, scale factor 0x%x\n", pRgn->rcSrc.right-pRgn->rcSrc.left, pRgn->dwHScale);
            }
            else
            {
              pRgn->rcSrc.left  = 0;
              pRgn->rcSrc.right = dwVal;
            }
            #else
            /* @@@SG Set Src rectangle to size of video when we get a size change */
            pRgn->rcSrc.left  = 0;
            pRgn->rcSrc.right = dwVal;
            #endif
         } 
         pRgn->dwSrcW  = dwVal;
         pRgn->dwWidth = dwVal;
         bRet = TRUE;
         break;
      case VO_HSCALE:
         pRgn->dwHScale = dwVal;
         /* If scaling specified, source rectangle ignored */
         pRgn->bSrcRcSpec = FALSE;
         pRgn->dwAutoMode &= ~VID_AUTO_SCALE;
         bRet = TRUE;
         break;
      case VO_VSCALE:
         #ifdef OPENTV_12
         /* For OpenTV 1.2 we deliberately limit the video Y scaling factors */
         /* to 0.5x, 1.0x and 2.0x to mimic the behaviour of existing IRDs   */
         if (dwVal <= HALF_SCALE_VIDEO)
         {
           dwVal = HALF_SCALE_VIDEO;
         }
         else
         {
           if (dwVal <= FULL_SCALE_VIDEO)
           {
             dwVal = FULL_SCALE_VIDEO;
           }
           else
           {
             dwVal = DOUBLE_SCALE_VIDEO;
           }
         }      
         #endif
         pRgn->dwVScale = dwVal;
         /* If scaling specified, source rectangle ignored */
         pRgn->bSrcRcSpec = FALSE;
         pRgn->dwAutoMode &= ~VID_AUTO_SCALE;
         bRet = TRUE;
         break;
      case VO_PAN_VECTOR:
         pRgn->dwPan = dwVal;
         pRgn->dwAutoMode &= ~VID_AUTO_PANSCAN;
         bRet = TRUE;
         break;
      case VO_SCAN_VECTOR:
         pRgn->dwScan = dwVal;
         pRgn->dwAutoMode &= ~VID_AUTO_PANSCAN;
         bRet = TRUE;
         break;
      case VO_AUTOMODE:
         pRgn->dwAutoMode = dwVal;
         if(dwVal & VID_AUTO_SCALE )
         {
            pRgn->dwHScale = 0x400;
            pRgn->dwVScale = 0x400;
         }
         bRet = TRUE;
         break;
      case VO_IMAGE_ON_TOP:
         pRgn->bTop = (dwVal == 0) ? FALSE : TRUE;
         
         /* Note that '1' in the DRM control register VideoTop field makes video */
         /* plane 0 appear on top and vice versa!!!!                             */
         
         /* Set video plane 0 as required */
         if (pRgn == gpActiveVidRgn[0])
         {
           CNXT_SET_VAL(glpDrmControl, DRM_CONTROL_VIDEO_TOP_MASK, pRgn->bTop);
           
           /* Set the other plane's Top indicator to the opposite of this one's */
           if(gpActiveVidRgn[1])
             gpActiveVidRgn[1]->bTop = !pRgn->bTop;
         }  
         /* Set video plane 1 as required */
         if (pRgn == gpActiveVidRgn[1])
         {
           CNXT_SET_VAL(glpDrmControl, DRM_CONTROL_VIDEO_TOP_MASK, !pRgn->bTop);
           
           /* Set the other plane's Top indicator to the opposite of this one's */
           if(gpActiveVidRgn[0])
             gpActiveVidRgn[0]->bTop = !pRgn->bTop;
         }  
         bRet = TRUE;
         break;
      case VO_BUFFER_PTR:        /* For internal use only! Required by OTVVIDEO */
        trace_new(TRACE_MPG|TRACE_LEVEL_2, 
                  "VIDLIB: Changing 0x%x buffer pointer from 0x%x to 0x%x\n",
                  pRgn, pRgn->pBuffer, dwVal);
        pRgn->pBuffer = (u_int8 *)dwVal;
        bRet = TRUE;
        break;
        
      case VO_USER_INFO:        /* For internal use only! Required by OTVVIDEO */
        trace_new(TRACE_MPG|TRACE_LEVEL_2, 
                  "VIDLIB: Setting 0x%08x user info to 0x%08x\n",
                  pRgn, dwVal);
        pRgn->dwUserInfo = dwVal;
        bRet = TRUE;
        break;

      /* Parsed pan/scan vectors. These are basically just used for storage of any pan/scan   */
      /* vector the video driver parses out of an MPEG still. Setting them has no effect in   */
      /* this driver but the values here may be used at a higher level to correct the display */
      /* position for 16:9 stills displayed on a 4:3 screen, for example.                     */
      case VO_IMAGE_PAN_VECTOR:      
         pRgn->iPanRead = (int16)dwVal;
         break;
         
      case VO_IMAGE_SCAN_VECTOR:     
         pRgn->iScanRead = (int16)dwVal;
         break;
         
      case VO_IMAGE_VECTOR_PRESENT:  
         pRgn->bVectorPresent = (bool)dwVal;
         break;
        
      case VO_PIXEL_TYPE:            /* Read only. */
      case VO_BUFFER_HEIGHT:         /* Read only. */
      case VO_BUFFER_WIDTH:          /* Read only. */
      case VO_BUFFER_STRIDE:         /* Read only. */
      default:
         break;
      }

      /* Update only active regions. */
      if (bUpdate)
      {
         if (pRgn == gpActiveVidRgn[0])
            vidUpdateScaling(0);
         if (pRgn == gpActiveVidRgn[1])
            vidUpdateScaling(1);
      }
   }

   return bRet;
}

/*****************************************************************************/
/* Function: vidGetOptions()                                                 */
/*                                                                           */
/* Type: Exported                                                            */
/*                                                                           */
/* Parameters: HVIDRGN hRgn - Handle of video region to get attribute data.  */
/*             VIDOPTION vOption - The option type to retrieve.              */
/*                                                                           */
/* Returns: u_int32 - The value of the attribute of the related video region.*/
/*                                                                           */
/* Description: Returns the value of a video region attribute.               */
/*****************************************************************************/
u_int32 vidGetOptions(HVIDRGN hRgn, VIDOPTION vOption)
{
   u_int32 dwRet = 0;
   PVIDRGN pRgn;

   pRgn = (PVIDRGN)hRgn;
   if (pRgn)
   {
      switch (vOption)
      {
      case VO_ASPECT_RATIO_MODE:
         dwRet = (u_int32)pRgn->arMode;
         break;
      case VO_ASPECT_RATIO:
         dwRet = (u_int32)pRgn->dwAR;
         break;
      case VO_CONNECTION:
         dwRet = (u_int32)pRgn->vrConnection;
         break;
      case VO_PIXEL_TYPE:
         dwRet = (u_int32)pRgn->vrType;
         break;
      case VO_BUFFER_HEIGHT:
         dwRet = (u_int32)pRgn->dwHeight;
         break;
      case VO_BUFFER_WIDTH:
         dwRet = (u_int32)pRgn->dwWidth;
         break;
      case VO_BUFFER_PTR:
         dwRet = (u_int32)pRgn->pBuffer;
         break;
      case VO_BUFFER_STRIDE:
         dwRet = (u_int32)pRgn->dwStride;
         break;
      case VO_IMAGE_HEIGHT:
         dwRet = pRgn->dwSrcH;
         break;
      case VO_IMAGE_WIDTH:
         dwRet = pRgn->dwSrcW;
         break;
      case VO_HSCALE:
         dwRet = pRgn->dwHScale;
         break;
      case VO_VSCALE:
         dwRet = pRgn->dwVScale;
         break;
      case VO_PAN_VECTOR:
         dwRet = pRgn->dwPan;
         break;
      case VO_SCAN_VECTOR:
         dwRet = pRgn->dwScan;
         break;
      case VO_AUTOMODE:
         dwRet = pRgn->dwAutoMode;
         break;
      case VO_IMAGE_ON_TOP:
         dwRet = (pRgn->bTop) ? 1 : 0;
         break;
         
      case VO_IMAGE_PAN_VECTOR:      
         dwRet = (u_int32)pRgn->iPanRead;
         break;
         
      case VO_IMAGE_SCAN_VECTOR:     
         dwRet = (u_int32)pRgn->iScanRead;
         break;
         
      case VO_IMAGE_VECTOR_PRESENT:  
         dwRet = (u_int32)pRgn->bVectorPresent;
         break;
         
      case VO_USER_INFO:
         dwRet = pRgn->dwUserInfo;
         break;   
      }
   }

   return dwRet;
}
bool  vidForceDisplay(HVIDRGN hRgn, bool bSoftware);
/*****************************************************************************/
/* Function: vidSetPos()                                                     */
/*                                                                           */
/* Type: Exported                                                            */
/*                                                                           */
/* Parameters: HVIDRGN hRgn - Handle to video region.                        */
/*             POSDRECT prcDst - Ptr to rect describing the display area.    */
/*             POSDRECT prcSrc - Ptr to rect describing source cropping area.*/
/*                                                                           */
/* Returns: bool - TRUE on success.                                          */
/*                                                                           */
/* Description: This function controls how the video image is displayed on   */
/*              the screen.  The destination rectangle is expressed in terms */
/*              of screen coordinates and describes the area of the screen   */
/*              where the video will be displayed.  The source rectangle is  */
/*              expressed in terms of the image extents and describes the    */
/*              of the video image which will be displayed.                  */
/*                                                                           */
/*              The 422 images do not support scaling.  The height and width */
/*              of the source rectangle must equal the height and width of   */
/*              the destination rectangle.                                   */
/*                                                                           */
/*              The 420 images do support image scaling.  The area of the    */
/*              source and destination rectangles can differ.                */
/*****************************************************************************/
bool vidSetPos(HVIDRGN hRgn, POSDRECT prcDst, POSDRECT prcSrc)
{
   bool bRet = FALSE;
   u_int32 dwSrcW, dwSrcH, dwDstW, dwDstH;
   u_int32 dwSrcX, dwSrcY, dwDstX, dwDstY;
   u_int32 dwActiveIndex = INVALID_VID_INDEX;
   PVIDRGN pRgn;

   if (hRgn)
   {
     
      pRgn = (PVIDRGN)hRgn;

      /* Find the destination height and width.                                */
      /* If prcDst is NULL, use the full screen size, else use the given rect. */
      dwDstW = (prcDst) ? (prcDst->right - prcDst->left) : OSD_MAX_WIDTH;
      dwDstH = (prcDst) ? (prcDst->bottom - prcDst->top) : gnOsdMaxHeight;
      dwDstX = (prcDst) ? prcDst->left : 0;
      dwDstY = (prcDst) ? prcDst->top : 0;

      /* Find the source height and width.                                     */
      /* If prcSrc is NULL, use the full region size, else use the given rect. */
      dwSrcW = (prcSrc) ? (prcSrc->right - prcSrc->left) : pRgn->dwWidth;  /* OSD_MAX_WIDTH;  */
      dwSrcH = (prcSrc) ? (prcSrc->bottom - prcSrc->top) : pRgn->dwHeight; /* gnOsdMaxHeight; */
      dwSrcX = (prcSrc) ? prcSrc->left : 0;
      dwSrcY = (prcSrc) ? prcSrc->top : 0;        

      isr_trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: Source dimensions %d x %d\n", dwSrcW, dwSrcH);
      isr_trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: Source position  (%d, %d)\n", dwSrcX, dwSrcY);      
      isr_trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: Dest   dimensions %d x %d\n", dwDstW, dwDstH);
      isr_trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: Dest   position  (%d, %d)\n", dwDstX, dwDstY);
      
      /* If source Rectangle Specified, note it since it overrides scale factors */
      if (prcSrc)
      {         
         pRgn->bSrcRcSpec = TRUE;
      } 

      /* Due to hardware limitations we cannot perform scaling on 422 regions. */
      /* Only continue if it is a 420 region or there is no scaling.           */
      if ((pRgn->vrType == TYPE_420) ||
         ((dwSrcW == dwDstW) && (dwSrcH == dwDstH)))
      {
         /* Set the source rectangle. */
         pRgn->rcSrc.top = dwSrcY;
         pRgn->rcSrc.left = dwSrcX;
         pRgn->rcSrc.right = dwSrcX + dwSrcW;
         pRgn->rcSrc.bottom = dwSrcY + dwSrcH;

         /* Set the destination rectangle. */
         pRgn->rcDst.top = dwDstY;
         pRgn->rcDst.left = dwDstX;
         pRgn->rcDst.right = dwDstX + dwDstW;
         pRgn->rcDst.bottom = dwDstY + dwDstH;

         /* If the region is active, update the hardware params. */
         if (pRgn == gpActiveVidRgn[0])
            dwActiveIndex = 0;
         if (pRgn == gpActiveVidRgn[1])
            dwActiveIndex = 1;

         /* If there is an active region, perform an update. */
         if (dwActiveIndex != INVALID_VID_INDEX)
            vidUpdateScaling(dwActiveIndex);

         bRet = TRUE;
      }
   }
	//add for still display guangyiwu 2007.9.19 begin
	   if(vidGet420Hardware()==hRgn)
	   {
	   	vidForceDisplay(hRgn, 0);
		vidForceDisplay(hRgn, 1);
	   	vidForceDisplay(hRgn, 0);
	   }
	   //add for still display guangyiwu 2007.9.19 end
   return bRet;
}

/*****************************************************************************/
/* Function: vidGetPos()                                                     */
/*                                                                           */
/* Type: Exported                                                            */
/*                                                                           */
/* Parameters: HVIDRGN hRgn - Handle to video region.                        */
/*             POSDRECT prcDst - Ptr to rect to receive the display area.    */
/*             POSDRECT prcSrc - Ptr to rect to receive source cropping area.*/
/*                                                                           */
/* Returns: bool - TRUE on success.                                          */
/*                                                                           */
/* Description: Returns the rectangles used for displaying the video region. */
/*              The source rectangle is used for cropping the source image.  */
/*              The destination rectangle is used to describe the area on the*/
/*              screen where the source image will be displayed.             */
/*****************************************************************************/
bool vidGetPos(HVIDRGN hRgn, POSDRECT prcDst, POSDRECT prcSrc)
{
   PVIDRGN pRgn;
   bool bRet = FALSE;

   pRgn = (PVIDRGN)hRgn;

   if ((pRgn) && (prcDst) && (prcSrc))
   {
      prcDst->top = pRgn->rcDst.top;
      prcDst->left = pRgn->rcDst.left;
      prcDst->right = pRgn->rcDst.right;
      prcDst->bottom = pRgn->rcDst.bottom;

      prcSrc->top = pRgn->rcSrc.top;
      prcSrc->left = pRgn->rcSrc.left;
      prcSrc->right = pRgn->rcSrc.right;
      prcSrc->bottom = pRgn->rcSrc.bottom;

      bRet = TRUE;
   }

   return bRet;
}

/*****************************************************************************/
/* Function: vidDisplayRegion()                                              */
/*                                                                           */
/* Type: Exported                                                            */
/*                                                                           */
/* Parameters: HVIDRGN hRgn - Video region handle to show or hide.           */
/*             bool bEnable - Show or hide flag.                             */
/*                                                                           */
/* Returns: bool - TRUE on success.                                          */
/*                                                                           */
/* Description: Shows or hides a video region.  Up to 2 video planes can be  */
/*              shown at the same time.  A failure code will be returned if  */
/*              a region is trying to be displayed while there are no free   */
/*              hardware video planes.  If 2 video planes are already being  */
/*              shown, a third is not possible and results in a failure      */
/*              return code.                                                 */
/*****************************************************************************/
bool vidDisplayRegion(HVIDRGN hRgn, bool bEnable)
{
   bool bRet = FALSE;
   PVIDRGN pRgn;
   u_int32 dwActIndex = INVALID_VID_INDEX;

   pRgn = (PVIDRGN)hRgn;
   if (pRgn)
   {
      if (bEnable)
      {
         /* Find the first inactive video plane. */
         if( gpActiveVidRgn[0] == NULL || gpActiveVidRgn[0] == pRgn )
            dwActIndex = 0;
         else if (gpActiveVidRgn[1] == NULL || gpActiveVidRgn[1] == pRgn )
            dwActIndex = 1;
		 else
			 ;

         /* Only enable a region if there is a free active video plane. */
         if (dwActIndex != INVALID_VID_INDEX)
         {
            bRet = vidSetActiveRegion(hRgn, dwActIndex);
         }
      }
      else /* Disable an active video region. */
      {
         if (gpActiveVidRgn[0] == pRgn)
         {
            CNXT_SET_VAL(glpDrmControl, DRM_CONTROL_MPEG_ENABLE_MASK, 0);
            CNXT_SET_VAL(glpDrmControl, DRM_CONTROL_EXT_VID_ENABLE_MASK, 0);
            gpActiveVidRgn[0] = NULL;
         }
         else if (gpActiveVidRgn[1] == pRgn)
         {
            CNXT_SET_VAL(glpDrmControl2, DRM_CONTROL_2_MPEG_ENABLE_MASK, 0);
            CNXT_SET_VAL(glpDrmControl2, DRM_CONTROL_2_EXT_VID_ENABLE_MASK, 0);
            gpActiveVidRgn[1] = NULL;
         }
         bRet = TRUE;
      }
   }

   return bRet;
}

/*****************************************************************************/
/* Function: vidSetConnection()                                              */
/*                                                                           */
/* Type: Exported                                                            */
/*                                                                           */
/* Parameters: HVIDRGN hRgn - Video region handle for connection change.     */
/*             VIDRGNCONNECT vrConnect - The new connection type.            */
/*                                                                           */
/* Returns: bool - TRUE on success.  FALSE if the hardware already has a     */
/*          video region connected.                                          */
/*                                                                           */
/* Description: Sets the connection to a video region.  The connection states*/
/*              who is putting data into the region's buffer.  The SOFTWARE  */
/*              connection means that an application is putting the video    */
/*              image data into the video region.  The HARDWARE connection   */
/*              means that the video region's buffer is automatically being  */
/*              filled by the corresponding hardware.  The video decoder for */
/*              420 regions and EVID for 422 regions.                        */
/*              A video region can be connected to HARDWARE and does not have*/
/*              to be visible.  The hardware can be filling an off screen    */
/*              region while displaying different regions.                   */
/*****************************************************************************/
bool vidSetConnection(HVIDRGN hRgn, VIDRGNCONNECT vrConnect)
{
   bool bRet = FALSE;
   PVIDRGN pRgn;
   u_int32 dwActIndex = INVALID_VID_INDEX;
   u_int32 dwStride;
   bool    ks;
   bool    bRetcode;
   u_int8 *pIBuff, *pPBuff;

   pRgn = (PVIDRGN)hRgn;
   if (pRgn)
   {
      if (gpActiveVidRgn[0] == pRgn)
         dwActIndex = 0;
      else if (gpActiveVidRgn[1] == pRgn)
         dwActIndex = 1;

      if (vrConnect == HARDWARE)
      {
         /* Only 420 type regions can be filled with hardware.  The hardware */
         /* can fill only a single region at a time.                         */
         if ((gp420Rgn == NULL) && (pRgn->vrType == TYPE_420))
         {
            pRgn->vrConnection = vrConnect;
            gp420Rgn = pRgn;
            
            /* Set the mpg decoder's ptrs to the regions video buffer. */
            if (pRgn == &gvrMpg)
            {
               bRetcode = gen_video_get_motion_decode_buffers(&pIBuff, &pPBuff);
               gen_video_set_video_buffers(pIBuff, pPBuff, 0);
               
            }      
            else
               gen_video_set_video_buffers(pRgn->pBuffer, pRgn->pBuffer, 0);

            bRet = TRUE;
         }
      }
      else /* vrConnect == SOFTWARE */
      {
         /* When the hardware buffer is switched from HARDWARE to SOFTWARE,  */
         /* the pBuffer element will point to the last decoded anchor frame  */
         /* which holds the last complete image.                             */
         if ((pRgn == &gvrMpg) && (pRgn->vrConnection == HARDWARE))
         {
            bRetcode = gen_video_get_current_display_buffer(&pRgn->pBuffer);
            trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDLIB: Forcing DRM to display frame buffer at 0x%08x\n", pRgn->pBuffer);
         }

         if (gp420Rgn == pRgn)
            gp420Rgn = NULL;
            
         pRgn->vrConnection = vrConnect;
         /* Check to see if this is being displayed.  Update the DRM still */
         /* control accordingly.                                           */
         if (dwActIndex != INVALID_VID_INDEX)
         {
            /* We always store images using multiple of 16 byte strides */
            dwStride = (pRgn->dwSrcW + 15) & 0xFFFFFFF0;

            trace_new(TRACE_MPG|TRACE_LEVEL_1, 
                      "****VIDLIB: Writing DRM still display regs for plane %d, address 0x%08x, (%d, %d)\n",
                      dwActIndex,
                      pRgn->pBuffer,
                      pRgn->dwSrcW,
                      pRgn->dwSrcH);
                      
            ks = critical_section_begin();
            
            CNXT_SET_VAL(glpDrmMpgLumaAddr[dwActIndex], DRM_ADDRESS_ADDRESS_MASK, 
               (u_int32)pRgn->pBuffer);
            CNXT_SET_VAL(glpDrmMpgChromaAddr[dwActIndex], DRM_ADDRESS_ADDRESS_MASK, 
               (u_int32)pRgn->pBuffer + (dwStride * pRgn->dwSrcH));

            /* Set the aspect ratio */
            CNXT_SET_VAL(glpDrmMpgStillCtrl[dwActIndex], 
                         DRM_MPG_VIDEO_PARAM_ASPECT_RATIO_MASK, 
                         (pRgn->dwAR == MPG_VID_ASPECT_169) ? 1 : 0);
                         
            /* Set pan/scan letterbox */
            CNXT_SET_VAL(glpDrmMpgStillCtrl[dwActIndex], 
                         DRM_MPG_VIDEO_PARAM_PAN_NOT_LETTERBOX_MASK, 
                         (pRgn->arMode == OSD_ARM_PANSCAN) ? 1 : 0);
                         
            /* Set the field not line to progressive. */
            CNXT_SET_VAL(glpDrmMpgStillCtrl[dwActIndex], 
                DRM_MPG_VIDEO_PARAM_FIELD_NOT_LINE_MASK, 1);
            /* Set the frame not field to frame. */
            CNXT_SET_VAL(glpDrmMpgStillCtrl[dwActIndex], 
                DRM_MPG_VIDEO_PARAM_FRAME_NOT_FIELD_MASK, 1);

            /* Set the image height and width. */
            CNXT_SET_VAL(glpDrmMpgSrcSize[dwActIndex], 
                DRM_SIZE_WIDTH_MASK, dwStride);
            CNXT_SET_VAL(glpDrmMpgSrcSize[dwActIndex], 
                DRM_SIZE_HEIGHT_MASK, pRgn->dwSrcH);
            
            CNXT_SET_VAL(glpDrmMpgStillCtrl[dwActIndex], 
                DRM_MPG_VIDEO_PARAM_STILL_ENABLE_MASK, 1);
            
            critical_section_end(ks);
         }

         bRet = TRUE;
      }
   }

   return bRet;
}

/*****************************************************************************/
/* Function: vidGetConnection()                                              */
/*                                                                           */
/* Type: Exported                                                            */
/*                                                                           */
/* Parameters: HVIDRGN hRgn - Video region handle.                           */
/*             VIDRGNCONNECT *pConnect - Ptr to receive the connection type. */
/*                                                                           */
/* Returns: bool - TRUE if pConnect has been filled with the connection type */
/*          of the given region.                                             */
/*                                                                           */
/* Description: Returns the connection type of the given region.  A HARDWARE */
/*              connection indicates that the hardware is filling the video  */
/*              region with image data.  A SOFTWARE connection indicates that*/
/*              the video region is under software control.  The hardware    */
/*              not change inage data during a SOFTWARE connection.          */
/*****************************************************************************/
bool vidGetConnection(HVIDRGN hRgn, VIDRGNCONNECT *pConnect)
{
   bool bRet = FALSE;
   PVIDRGN pRgn;

   pRgn = (PVIDRGN)hRgn;
   /* Make sure that we have real pointers. */
   if (pRgn && pConnect)
   {
      *pConnect = pRgn->vrConnection;
      bRet = TRUE;
   }

   return bRet;
}

/*****************************************************************************/
/* Function: vidUpdateScaling()                                              */
/*                                                                           */
/* Type: Exported                                                            */
/*                                                                           */
/* Parameters: u_int32 dwRgnIndex - Index into which of the video planes to  */
/*                                  update.                                  */
/*                                                                           */
/* Returns: void                                                             */
/*                                                                           */
/* Description: Updates the DRM video scaling registers.  This function is   */
/*              call from ISRs and tasks.                                    */
/*****************************************************************************/
void vidUpdateScaling(u_int32 dwRgnIndex)
{
   /* Make sure that that the index is in bounds. */
   if (dwRgnIndex < VID_MAX_VID_RGN)
   {
      if (gpActiveVidRgn[dwRgnIndex])
      {
         if (gpActiveVidRgn[dwRgnIndex]->vrType == TYPE_422)
            vidUpdate422Scaling(dwRgnIndex);
         else
            vidUpdate420Scaling(dwRgnIndex);
      }
   }
}

/*****************************************************************************/
/* Function: vidGet420Hardware()                                             */
/*                                                                           */
/* Type: External                                                            */
/*                                                                           */
/* Parameters: void                                                          */
/*                                                                           */
/* Returns: HVIDRGN - Handle to the video region that is currently being     */
/*          filled by the Mpg video hardware.  Otherwise NULL.               */
/*                                                                           */
/* Description: Returns a handle to the video region that is currently       */
/*              connected to the mpg video hardware.                         */
/*****************************************************************************/
HVIDRGN vidGet420Hardware(void)
{
   return (HVIDRGN)gp420Rgn;
}

/*****************************************************************************/
/* Function: vidGetActiveRegion()                                            */
/*                                                                           */
/* Type: Exported                                                            */
/*                                                                           */
/* Parameters: u_int32 dwRgnIndex - The zero based index of the active video */
/*                                  region.                                  */
/*                                                                           */
/* Returns: HVIDRGN - Handle to an active video region.                      */
/*                                                                           */
/* Description: The DRM hardware can support 2 simultaneously active video   */
/*              regions.  This function will return the handle of a video    */
/*              region if it is active, otherwise it will retun NULL.        */
/*****************************************************************************/
HVIDRGN vidGetActiveRegion(u_int32 dwRgnIndex)
{
   HVIDRGN hRet = (HVIDRGN)NULL;

   if (dwRgnIndex < VID_MAX_VID_RGN)
      hRet = (HVIDRGN)gpActiveVidRgn[dwRgnIndex];

   return hRet;
}


/********************************************************************/
/*  FUNCTION:    vidGetVideoRegion                                  */
/*                                                                  */
/*  PARAMETERS:  None                                               */
/*                                                                  */
/*  DESCRIPTION: Return a handle to the global video region,        */
/*               regardless of whether it is enabled or not.        */
/*                                                                  */
/*  RETURNS:     Handle to the global video region.                 */
/*                                                                  */
/*  CONTEXT:     May be called from any context.                    */
/*                                                                  */
/********************************************************************/
HVIDRGN vidGetVideoRegion(void)
{
  return(ghvrMpg);
}

/*****************************************************************************/
/* Function: vidSetRegionInput()                                             */
/*                                                                           */
/* Type: Exported                                                            */
/*                                                                           */
/* Parameters: HVIDRGN hRgn - Video region handle.                           */
/*             u_int32 dwActIndex - The zero based index of the video        */
/*                                  plane.                                   */
/*                                                                           */
/* Returns: BOOL - TRUE if the video region is hooked to the plane.          */
/*                 FALSE if it was unable to the region to the plane         */
/*                                                                           */
/* Description: The DRM hardware can support 2 simultaneously active video   */
/*              planes.  This function will individually set the input       */
/*              source for each plane.                                       */ 
/*                                                                           */
/* Context:     This function is called from task and interrupt context.     */
/*****************************************************************************/
bool vidSetRegionInput(HVIDRGN hRgn, u_int32 dwActIndex)
{
   bool bRet = FALSE;
   PVIDRGN pRgn = (PVIDRGN)hRgn;
   u_int32 dwStride;
   bool    ks;

   if ((dwActIndex < VID_MAX_VID_RGN) && (pRgn))
   {
         gpActiveVidRgn[dwActIndex] = pRgn;

         /* Set the buffer pointers for software connection.  The 422 buffer */
         /* pointers are set as a part of the scaling.                       */
         if (pRgn->vrType == TYPE_420)
         {
            ks = critical_section_begin();
            
            if (pRgn->vrConnection == SOFTWARE)
            {
               /* Create the DWORD aligned stride for the 420 buffer. */
               dwStride = (pRgn->dwSrcW + 3) & 0xFFFFFFFC;

               CNXT_SET_VAL(glpDrmMpgLumaAddr[dwActIndex], 
                   DRM_ADDRESS_ADDRESS_MASK, (u_int32)pRgn->pBuffer);
               CNXT_SET_VAL(glpDrmMpgChromaAddr[dwActIndex], 
                   DRM_ADDRESS_ADDRESS_MASK,  
                  (u_int32)pRgn->pBuffer + (dwStride * pRgn->dwSrcH));

               /* Enable the still image control. */
               CNXT_SET_VAL(glpDrmMpgStillCtrl[dwActIndex], 
                   DRM_MPG_VIDEO_PARAM_STILL_ENABLE_MASK, 1);
               /* Set the aspect ratio to 4:3. */
               CNXT_SET_VAL(glpDrmMpgStillCtrl[dwActIndex], 
                   DRM_MPG_VIDEO_PARAM_ASPECT_RATIO_MASK, 0);
               /* Set the field not line to progressive. */
               CNXT_SET_VAL(glpDrmMpgStillCtrl[dwActIndex], 
                   DRM_MPG_VIDEO_PARAM_FIELD_NOT_LINE_MASK, 1);
               /* Set the frame not field to frame. */
               CNXT_SET_VAL(glpDrmMpgStillCtrl[dwActIndex], 
                   DRM_MPG_VIDEO_PARAM_FRAME_NOT_FIELD_MASK, 1);

               /* Set the image height and width. */
               CNXT_SET_VAL(glpDrmMpgSrcSize[dwActIndex], DRM_SIZE_WIDTH_MASK, pRgn->dwSrcW);
               CNXT_SET_VAL(glpDrmMpgSrcSize[dwActIndex], DRM_SIZE_HEIGHT_MASK, pRgn->dwSrcH);
            }
            else /* pRgn->vrConnection == HARDWARE */
            {
               /* Set the upper 3 bits of the image buffer pointer. */
               CNXT_SET_VAL(glpDrmMpgStillCtrl[dwActIndex], 
                   DRM_MPG_VIDEO_PARAM_MEM_EXTENSION_MASK,  
                  ((u_int32)pRgn->pBuffer) >> 24);

               /* Enable the hardware image control. */
               CNXT_SET_VAL(glpDrmMpgStillCtrl[dwActIndex], 
                   DRM_MPG_VIDEO_PARAM_STILL_ENABLE_MASK, 0);
               
            }
            
            critical_section_end(ks);
         }

         vidUpdateScaling(dwActIndex);

         bRet = TRUE;
   }

   return bRet;
}

/*****************************************************************************/
/* Function: vidSetActiveRegion()                                            */
/*                                                                           */
/* Type: Exported                                                            */
/*                                                                           */
/* Parameters: HVIDRGN hRgn - Video region handle.                           */
/*             u_int32 dwActIndex - The zero based index of the active video */
/*                                  region.                                  */
/*                                                                           */
/* Returns: BOOL - TRUE if the video region is set as the active region.     */
/*                 FALSE if it was unable to set the requested region as     */
/*                 active.  This could be due to the video plane already has */
/*                 an active video region.                                   */
/*                                                                           */
/* Description: The DRM hardware can support 2 simultaneously active video   */
/*              regions.  This function will individually set and enable an  */
/*              active video region.  It calls vidSetRegionInput to hook a   */
/*              region to a plan and then activates it.  These probably      */
/*              would be better as two totally separate calls, but that      */
/*              would break things that already use vidSetActiveRegion       */ 
/*****************************************************************************/
bool vidSetActiveRegion(HVIDRGN hRgn, u_int32 dwActIndex)
{
   bool bRet = FALSE;
   PVIDRGN pRgn = (PVIDRGN)hRgn;

   if ((vidSetRegionInput(hRgn,dwActIndex)) && (dwActIndex < VID_MAX_VID_RGN) && (pRgn))
   {
         gpActiveVidRgn[dwActIndex] = pRgn;
         if (pRgn->bTop)
            CNXT_SET_VAL(glpDrmControl, DRM_CONTROL_VIDEO_TOP_MASK, 
                (dwActIndex == 0) ? 1 : 0);
         else
            CNXT_SET_VAL(glpDrmControl, DRM_CONTROL_VIDEO_TOP_MASK, 
            (dwActIndex == 0) ? 0 : 1);

         if (dwActIndex == 0)
         {
            if (pRgn->vrType == TYPE_422)
            {
               CNXT_SET_VAL(glpDrmControl, DRM_CONTROL_MPEG_ENABLE_MASK, 0);
               CNXT_SET_VAL(glpDrmControl, DRM_CONTROL_EXT_VID_ENABLE_MASK, 1);
            }
            else /* pRgn->vrType == TYPE_420 */
            {
               CNXT_SET_VAL(glpDrmControl, DRM_CONTROL_EXT_VID_ENABLE_MASK, 0);
               CNXT_SET_VAL(glpDrmControl, DRM_CONTROL_MPEG_ENABLE_MASK, 1);
            }
         }
         else
         {
            if (pRgn->vrType == TYPE_422)
            {
               CNXT_SET_VAL(glpDrmControl2, DRM_CONTROL_2_MPEG_ENABLE_MASK, 0);
               CNXT_SET_VAL(glpDrmControl2, DRM_CONTROL_2_EXT_VID_ENABLE_MASK, 1);
            }
            else /* pRgn->vrType == TYPE_420 */
            {
               CNXT_SET_VAL(glpDrmControl2, DRM_CONTROL_2_EXT_VID_ENABLE_MASK, 0);
               CNXT_SET_VAL(glpDrmControl2, DRM_CONTROL_2_MPEG_ENABLE_MASK, 1);
            }
         }


         bRet = TRUE;
   }

   return bRet;
}

/*****************************************************************************/
/* Function: vidGetTopRegion()                                               */
/*                                                                           */
/* Type: Exported                                                            */
/*                                                                           */
/* Parameters: void                                                          */
/*                                                                           */
/* Returns: HVIDRGN - Handle to the video region that is topmost.            */
/*                                                                           */
/* Description: Returns the video region that DRM has marked as the topmost. */
/*****************************************************************************/
HVIDRGN vidGetTopRegion(void)
{
   HVIDRGN hRet = (HVIDRGN)NULL;

   if (CNXT_GET_VAL(glpDrmControl, DRM_CONTROL_VIDEO_TOP_MASK) == 1)
      hRet = (HVIDRGN)gpActiveVidRgn[0];
   else
      hRet = (HVIDRGN)gpActiveVidRgn[1];

   return hRet;
}

/*****************************************************************************/
/* Function: vidGetBottomRegion()                                            */
/*                                                                           */
/* Type: Exported                                                            */
/*                                                                           */
/* Parameters: void                                                          */
/*                                                                           */
/* Returns: HVIDRGN - Handle to the video region that is on bottom.          */
/*                                                                           */
/* Description: Returns the video region that DRM has marked as bottom.      */
/*****************************************************************************/
HVIDRGN vidGetBottomRegion(void)
{
   HVIDRGN hRet = (HVIDRGN)NULL;

   if (CNXT_GET_VAL(glpDrmControl, DRM_CONTROL_VIDEO_TOP_MASK) == 0)
      hRet = (HVIDRGN)gpActiveVidRgn[0];
   else
      hRet = (HVIDRGN)gpActiveVidRgn[1];

   return hRet;
}

/*****************************************************************************/
/* Function: vidSetAlpha()                                                   */
/*                                                                           */
/* Type: Exported                                                            */
/*                                                                           */
/* Parameters: u_int32 dwRgnIndex - The zero based index of the active video */
/*                                  region.                                  */
/*             u_int8 byAlpha - Video blend alpha.                           */
/*                                                                           */
/* Returns: bool - TRUE on success.                                          */
/*                                                                           */
/* Description: Each of the 2 DRM video planes can blend with the background */
/*              color.  This function will set the corresponding video plane */
/*              alpha.                                                       */
/*****************************************************************************/
bool vidSetAlpha(u_int32 dwRgnIndex, u_int8 byAlpha)
{
   bool bRet = FALSE;

   if (dwRgnIndex == 0)
   {
      CNXT_SET_VAL(glpDrmVideoAlpha, DRM_VIDEO_ALPHA_VIDEO0_ALPHA_MASK, byAlpha);
      bRet = TRUE;
   }
   else if (dwRgnIndex == 1)
   {
      CNXT_SET_VAL(glpDrmVideoAlpha, DRM_VIDEO_ALPHA_VIDEO1_ALPHA_MASK, byAlpha);
      bRet = TRUE;
   }

   return bRet;  
}

/*****************************************************************************/
/* Function: vidGetAlpha()                                                   */
/*                                                                           */
/* Type: Exported                                                            */
/*                                                                           */
/* Parameters: u_int32 dwRgnIndex - The zero based index of the active video */
/*                                  region.                                  */
/*             u_int8 *pbyAlpha - Ptr to return the video blend alpha.       */
/*                                                                           */
/* Returns: bool - TRUE on success.                                          */
/*                                                                           */
/* Description: Each of the 2 DRM video planes can blend with the background */
/*              color.  This function returns the corresponding video plane  */
/*              alpha.                                                       */
/*****************************************************************************/
bool vidGetAlpha(u_int32 dwRgnIndex, u_int8 *pbyAlpha)
{
   bool bRet = FALSE;

   if (pbyAlpha)
   {
      if (dwRgnIndex == 0)
      {
         *pbyAlpha = CNXT_GET_VAL(glpDrmVideoAlpha, 
             DRM_VIDEO_ALPHA_VIDEO0_ALPHA_MASK);
         bRet = TRUE;
      }
      else if (dwRgnIndex == 1)
      {
         *pbyAlpha = CNXT_GET_VAL(glpDrmVideoAlpha, 
             DRM_VIDEO_ALPHA_VIDEO1_ALPHA_MASK);
         bRet = TRUE;
      }
   }

   return bRet;
}

/*****************************************************************************/
/* Function: vidWipeEnable()                                                 */
/*                                                                           */
/* Type: Exported                                                            */
/*                                                                           */
/* Parameters: u_int32 dwRgnIndex - Zero based video plane index. (0 or 1)   */
/*             bool bEnable - Flag to enable/disable video wiping.           */
/*                                                                           */
/* Returns: bool - TRUE on success.                                          */
/*                                                                           */
/* Description: Enables/disables video wiping for the selected video plane.  */
/*              Before enabling video wipe, a wipe region must be set using  */
/*              vidSetWipeRgn().  It is also recommended to set the wipe line*/
/*              number using vidSetWipeLine().                               */
/*****************************************************************************/
bool vidWipeEnable(u_int32 dwRgnIndex, bool bEnable)
{
   bool bRet = FALSE;

   if (dwRgnIndex < VID_MAX_VID_RGN)
   {
      CNXT_SET_VAL(glpDrmMpgTileWipe[dwRgnIndex], 
          DRM_MPG_TILE_WIPE_CTRL_WIPE_ENABLE_MASK, (bEnable) ? 1 : 0);
          
      #if DRM_NEEDS_SPLIT_FIFO_ON_WIPE == YES
      CNXT_SET_VAL(glpDrmDWIControl[dwRgnIndex],
          DRM_DWI_CONTROL_2_SPLIT_FIFO_CHROMA_MASK, (bEnable) ? 1 : 0);
      #endif        
      
      bRet = TRUE;
      
      #if DRM_HAS_UPSCALED_PAL_STILL_BUG == YES
      if (!bEnable)
      {
        /* Turn off the PAL bit in the DRM to ensure still scaling looks as good as possible */
        /* There is a bug in CX22490/1/6 that makes upscaled stills look bad in PAL mode     */
        /* unless this bit is clear but the bit must be set correctly while motion video is  */
        /* playing or when doing still to still wipes.                                       */
        if((PlayState != PLAY_LIVE_STATE) && (PlayState != PLAY_LIVE_STATE_NO_SYNC))
        {
          CNXT_SET_VAL(glpDrmControl, DRM_CONTROL_OUTPUT_FORMAT_MASK, 0);
        }
      }
      else
      {
        /* To ensure that the wipe chroma looks correct, we must set the OutputFormat bit */
        /* correctly. Currently we leave it set to 0 when motion video is disabled since  */
        /* this works around a hardware upscaling bug. We must set it correctly for wipes */
        /* though to ensure that we don't see the chroma misalignment problem.            */
        if(gnOsdMaxHeight == 576)
          CNXT_SET_VAL(glpDrmControl, DRM_CONTROL_OUTPUT_FORMAT_MASK, 1); /* PAL */
        else
          CNXT_SET_VAL(glpDrmControl, DRM_CONTROL_OUTPUT_FORMAT_MASK, 0); /* NTSC */
      }  
      #endif       
   }

   return bRet;
}

/*****************************************************************************/
/* Function: vidSetWipeRgn()                                                 */
/*                                                                           */
/* Type: Exported                                                            */
/*                                                                           */
/* Parameters: u_int32 dwRgnIndex - Zero based video plane index. (0 or 1)   */
/*             HVIDRGN hRgn - Video region handle for the wipe region.       */
/*                                                                           */
/* Returns: bool - TRUE on success.                                          */
/*                                                                           */
/* Description: Video wiping will switch from displaying an active video     */
/*              region to a wipe video region at a display line set from     */
/*              vidSetWipeLine().  The wipe video region is set from         */
/*              vidSetWipeRgn().                                             */
/*****************************************************************************/
bool vidSetWipeRgn(u_int32 dwRgnIndex, HVIDRGN hRgn)
{
   bool bRet = FALSE;
   PVIDRGN pRgn = (PVIDRGN)hRgn;
   u_int8 *pLuma;
   u_int8 *pChroma;
   u_int32 dwStride;

   if ((dwRgnIndex < VID_MAX_VID_RGN) && pRgn)
   {
      dwStride = (pRgn->dwSrcW + 3) & ~3;
      
      /* To be completed - remove for Rev B of CX2249X                          */
      /* For Colorado rev A, there is a minor hardware bug that causes a 2 line */
      /* difference positioning of the wipe image. We correct for this hear.    */
      #ifndef COLORADO_REV_A_WIPE_BUG
      pLuma = pRgn->pBuffer;
      pChroma = pLuma + (dwStride * pRgn->dwSrcH);
      #else
      pLuma = pRgn->pBuffer + 2*dwStride;
      pChroma = pLuma + (dwStride * (pRgn->dwSrcH+1));
      #endif

      CNXT_SET_VAL(glpDrmWipeLumaAddr[dwRgnIndex], DRM_ADDRESS_ADDRESS_MASK, 
          (u_int32)pLuma);
      CNXT_SET_VAL(glpDrmWipeChromaAddr[dwRgnIndex], DRM_ADDRESS_ADDRESS_MASK, 
          (u_int32)pChroma);

      gpWipeVidRgn[dwRgnIndex] = pRgn;

      bRet = TRUE;
   }

   return bRet;
}

/*****************************************************************************/
/* Function: vidGetWipeRgn()                                                 */
/*                                                                           */
/* Type: Exported                                                            */
/*                                                                           */
/* Parameters: u_int32 dwRgnIndex - Zero based video plane index. (0 or 1)   */
/*                                                                           */
/* Returns: HVIDRGN - The video region handle for the wipe region.  Or NULL. */
/*                                                                           */
/* Description: Returns the wipe video region handle associated with each    */
/*              video plane.                                                 */
/*****************************************************************************/
HVIDRGN vidGetWipeRgn(u_int32 dwRgnIndex)
{
   HVIDRGN hRgn = (HVIDRGN)NULL;

   if (dwRgnIndex < VID_MAX_VID_RGN)
   {
      hRgn = (HVIDRGN)gpWipeVidRgn[dwRgnIndex];
   }

   return hRgn;
}

/*****************************************************************************/
/* Function: vidSetWipeLine()                                                */
/*                                                                           */
/* Type: Exported                                                            */
/*                                                                           */
/* Parameters: u_int32 dwRgnIndex - Zero based video plane index. (0 or 1)   */
/*             u_int32 dwLine - Display line number for wipe region switch.  */
/*                              1 to 480 NTSC, 1 to 576 PAL                  */
/*                                                                           */
/* Returns: bool - TRUE on success.                                          */
/*                                                                           */
/* Description: Sets the wipe line display number.  When DRM is outputing a  */
/*              display line less than the wipe line number, the associated  */
/*              video region is displayed.  When DRM outputs a line greater  */
/*              than the wipe line number, the display is switched to the    */
/*              video plane's wipe region. A logical wipe line of 0 should   */
/*              not be used.                                                 */
/*****************************************************************************/
bool vidSetWipeLine(u_int32 dwRgnIndex, u_int32 dwLine)
{
   bool bRet = FALSE;
   u_int32 dwWipeLine;

   if ((dwRgnIndex < VID_MAX_VID_RGN) && (dwLine < gnOsdMaxHeight))
   {
      dwWipeLine = (dwLine / 2) + gnVBlank;
      CNXT_SET_VAL(glpDrmMpgStillCtrl[dwRgnIndex], 
          DRM_MPG_VIDEO_PARAM_WIPE_LINE_NUMBER_MASK, dwWipeLine);
      bRet = TRUE;
   }

   return bRet;
}

/*****************************************************************************/
/* Function: vidGetWipeLine()                                                */
/*                                                                           */
/* Type: Exported                                                            */
/*                                                                           */
/* Parameters: u_int32 dwRgnIndex - Zero based video plane index. (0 or 1)   */
/*             u_int32 *pdwLine - Ptr to receive the wipe line number.       */
/*                                                                           */
/* Returns: bool - TRUE on success.                                          */
/*                                                                           */
/* Description: Returns the actual wipe line number used to switch between   */
/*              a video region and a wipe video region.  This number may     */
/*              differ from the line that was set with vidSetWipeLine().     */
/*****************************************************************************/
bool vidGetWipeLine(u_int32 dwRgnIndex, u_int32 *pdwLine)
{
   bool bRet = FALSE;
   u_int32 dwWipeLine;

   if ((dwRgnIndex < VID_MAX_VID_RGN) && (pdwLine))
   {
      dwWipeLine = CNXT_GET_VAL(glpDrmMpgStillCtrl[dwRgnIndex], 
          DRM_MPG_VIDEO_PARAM_WIPE_LINE_NUMBER_MASK) - gnVBlank;
      dwWipeLine <<= 1;

      *pdwLine = dwWipeLine;
      bRet = TRUE;
   }

   return bRet;
}

/***** End Exported Functions ************************************************/

/***** Internal Functions ****************************************************/
/*****************************************************************************/
/* Function: vidUpdate420Scaling()                                           */
/*                                                                           */
/* Type: Internal                                                            */
/*                                                                           */
/* Parameters: u_int32 dwRgnIndex - Index into which of the video planes to  */
/*                                  update.                                  */
/*                                                                           */
/* Returns: void                                                             */
/*                                                                           */
/* Description: Updates the 420 Mpg DRM video scaling registers.  This       */
/*              function is call from ISRs and tasks.                        */
/*                                                                           */
/* Context:     This function is called under interrupt context              */
/*                                                                           */
/*****************************************************************************/
void vidUpdate420Scaling(u_int32 dwRgnIndex)
{
   PVIDRGN pRgn;
   OSDRECT rectSrc, rectDest;
   u_int32 dwSrcX, dwSrcY, dwSrcH, dwSrcW;
   u_int32 dwDstX, dwDstY, dwDstH, dwDstW;
   u_int32 dwPanOffset;
   u_int32 dwFetchWidth;
   u_int32 dwIncXScale, dwIncYScale;
   u_int32 dwHFilter, dwVfilter;
   u_int32 dwAddrOffset;
   LPREG pDWIControl2_3;
   bool bPanScan = FALSE;
   bool bLetterBox = FALSE;
   bool bSplitFIFO = FALSE;
   bool bEnableAspectConv = FALSE;
   bool ksPrev;
   bool bYDecEnable, bXDecEnable;
   #if (DRM_FILTER_PHASE_0_PROBLEM == YES)
   u_int32 dwTemp;
   #endif

   isr_trace_new(TRACE_LEVEL_2|TRACE_MPG, "VIDEO: DRM video scaling update for plane %d\n", dwRgnIndex, 0);
   
   /* There are currently a maximum of 2 Video Regions */
   if (dwRgnIndex < VID_MAX_VID_RGN)
   {
      pRgn = gpActiveVidRgn[dwRgnIndex];
      if (pRgn) /* If the video region is active this will not be NULL */
      {
         /* The control registers DWI2 and DWI3 are the same, but represent
          * video planes 0/1 respectively.
          */
         pDWIControl2_3 = (dwRgnIndex==0)?glpDrmDWIControl2:glpDrmDWIControl3;

         /* Calculate the "real" source and destination rectangles for the  */
         /* video window taking into account any auto modes set and the     */
         /* source and destination specified by the application. Also       */
         /* adjust as required to handle aspect ratio of stream and display */
         vidCalculateVideoSrcAndDestination(pRgn, &rectSrc, &rectDest, &dwPanOffset, &bPanScan, &bLetterBox, &bEnableAspectConv);

         dwDstH = rectDest.bottom - rectDest.top;
         dwDstW = rectDest.right  - rectDest.left;
         dwDstX = rectDest.left;
         dwDstY = rectDest.top;
         
         dwSrcH = rectSrc.bottom - rectSrc.top;
         dwSrcW = rectSrc.right  - rectSrc.left;
         dwSrcX = rectSrc.left;
         dwSrcY = rectSrc.top; 
         
         dwFetchWidth = dwSrcW;

         /***************************************************************************/
         /* All source and destination rectangle calculations and adjustments have  */
         /* been completed at this point. Now go off and write the DRM registers to */
         /* reflect the desired position/scale.                                     */
         /***************************************************************************/
         
         /* Program DRM registers to set the MPEG size, position,
          * and field start for displaying source MPEG data.
          */
         isr_trace_new(TRACE_MPG|TRACE_LEVEL_1, 
                   "****VIDLIB: Writing DRM MPEG size and position for plane %d\n", dwRgnIndex, 0);
         isr_trace_new(TRACE_MPG|TRACE_LEVEL_1, "****VIDLIB: Position (%d, %d)\n",
                   dwDstX,
                   dwDstY);
         isr_trace_new(TRACE_MPG|TRACE_LEVEL_1, "****VIDLIB: Size (%d, %d)\n",
                   dwDstW,
                   dwDstH);
         
         /* Calculate the address offset value */
         dwAddrOffset = dwSrcX;

         /* Calculate the DRM scaling, filtering, and fifo settings.
          * The Src and Dst height and width may have been modified
          * before this calculation based on whether pan-scan or
          * letterbox is active for the video region.
          */

         /* Calculate the Y scale increment value to use.
          * This corresponds to the input MPEG fetch height
          * divided by the output height. If this is greater
          * than 0x1000 it means that downscaling in Y will
          * occur to fit the the MPEG source data into the
          * output screen area.
          */
         dwIncYScale = (dwSrcH << 12) / dwDstH;

         /* This is a bug that was fixed in Hondo and beyond,     */
         /* There is no plan to fix it in Colorado at the moment. */
        #ifdef COLORADO_CHROMA_WIPE_BUG
        /* If wipe is enabled. Check if scaling is equal to 0x1000
         * and fix up the DRM vertical scaling factor to work
         * around a wipe bug when cropping and not scaling.
         */
         if ( (CNXT_GET_VAL(glpDrmMpgTileWipe[dwRgnIndex], 
             DRM_MPG_TILE_WIPE_CTRL_WIPE_ENABLE_MASK)) &&
              (dwIncYScale == 0x1000) )
         {
            if ( ( dwAddrOffset != 0 ) || 
                 ((dwSrcY >> 1) != 0 )) /* VerticalOffset */ 
               dwIncYScale++;
         }
         #endif
         
         if (dwIncYScale > 0xFFFF)
           dwIncYScale = 0xFFFF;

         /* Calculate the X scale increment value to use.
          * This corresponds to the input MPEG fetch width
          * divided by the output width. If this is greater
          * than 0x1000 it means that downscaling in Y will
          * occur to fit the input MPEG source data into the
          * output screen area.
          */
         dwIncXScale = (dwFetchWidth << 12) / dwDstW;
         if(dwIncXScale > 0xFFFF)
           dwIncXScale = 0xFFFF;
           
         /* Enable SplitFIFO if X screen resolution of the
          * output image is greater than 360 and downscaling in Y.
          * NOTE: SplitFIFOChroma should be set if SplitFIFOLuma is
          * is set per specification.
          */
         if ( (dwDstW > 360) && (dwDstH < dwSrcH) )  
           bSplitFIFO = TRUE;

         /* Work out the horizontal filter setting to use depending upon source */
         /* and destination size.                                               */
         #ifndef ERIC_RAYEL_HFILTER_MODE
         if ((dwSrcW / dwDstW) == 2)
            dwHFilter = 1;
         else if ((dwSrcW / dwDstW) > 2)
            dwHFilter = 2;
         else
            dwHFilter = 0;
         #else /* ERIC_RAYEL_HFILTER_MODE */
         /* Eric Rayel wants the HFilterSelect to be set to 2 if not at full screen. */
         if (dwDstW != 720)
            dwHFilter = 2;
         else
            dwHFilter = 0;
         #endif /* ERIC_RAYEL_HFILTER_MODE */

         /* Turn on the video lowpass filter if scaling down */
         if (dwSrcH > dwDstH)
         {
           dwVfilter = 1;
         }
         else
         {
           dwVfilter = 0;
         }
         
         #if (DRM_FILTER_PHASE_0_PROBLEM == YES)
         /* If scaling 704 wide source to 720, we tweak the values to get round   */
         /* a phase 0 problem in the DRM scaler for Colorado and Wabash rev A. In */  
         /* this solution we scale the image slightly wider and crop it to ensure */
         /* that any vertical lines introduced by the problem are at most 1 pixel */
         /* wide and can be smoothed by the sharpening filter.                    */
         
         isr_trace_new(TRACE_MPG|TRACE_LEVEL_1, 
                 "VIDLIB: Original dwIncXScale 0x%08x, dwAddrOffset 0x%08x\n", dwIncXScale, dwAddrOffset);
         isr_trace_new(TRACE_MPG|TRACE_LEVEL_1, 
                 "VIDLIB:          dwFetchWidth 0x%08x\n", dwFetchWidth, 0);
                 
         #ifdef ENABLE_DISABLE_FILTER_FIX        
         if(bUseDRMFilterFix)
         {
         #endif
           dwTemp = dwIncXScale & 0xFFF;
          
           if(((dwTemp > 0xf80) || (dwTemp && (dwTemp < 0x80))) && (dwIncXScale < 0x4000) && (dwIncXScale > 0xf80))
           {
             isr_trace_new(TRACE_MPG|TRACE_LEVEL_2, 
                     "VIDLIB: Hit special case scaling!\n", 0, 0);

             dwIncXScale = (dwTemp > 0xF80) ? ((dwIncXScale & 0xF000) | 0xF80) : (dwIncXScale & 0xF000);
             dwTemp = (dwDstW * dwIncXScale)/4096;
             dwAddrOffset += (dwFetchWidth - dwTemp)/2;
             dwFetchWidth = dwTemp;
             isr_trace_new(TRACE_MPG|TRACE_LEVEL_2, 
                     "VIDLIB: Fixed up dwIncXScale 0x%08x, dwAddrOffset 0x%08x\n", dwIncXScale, dwAddrOffset);
             isr_trace_new(TRACE_MPG|TRACE_LEVEL_2, 
                     "VIDLIB:          dwFetchWidth 0x%08x\n", dwFetchWidth, 0);
           } 
         #ifdef ENABLE_DISABLE_FILTER_FIX        
         }  
         #endif
         #endif /* DRM_FILTER_PHASE_0_PROBLEM == YES */
           
         /*************************************************************************/
         /* By this point, we have precalculated everything we need to update     */
         /* the appropriate DRM and MPG registers. Do all the writes in a block   */
         /* protected by a critical section to ensure that they all get completed */
         /* as quickly as possible.                                               */
         /*************************************************************************/
         
          ksPrev = critical_section_begin();
          
         /* If the video region data is being filled by the hardware
          * program the MPEG host interface control register to
          * indicate whether aspect ratio conversion is enabled.
          * If aspect ratio conversion is enabled the pan-scan or
          * letter box control bit will be looked at by the VCORE
          * hardware and scaling of the data will occur.
          * NOTE: It was recommended by the chip guys that letterbox mode
          * not be used here, and to perform any decoded picture sizing
          * and scaling in this code using only the DRM.
          * This is not the current behavior of the code. Right now
          * the B_YInc is programmed differently if in letterbox to
          * handle the difference in size for B frame decodes.
          * To be completed. AR does not apply if not doing all automatic
          * scaling, windowing, offset.
          */
         if (pRgn->vrConnection == HARDWARE)
         {
            CNXT_SET_VAL(glpCtrl0, MPG_CONTROL0_ENABLEASPECTCONVERT_MASK, (bEnableAspectConv) ? 1 : 0);
            CNXT_SET_VAL(glpCtrl0, MPG_CONTROL0_ENABLEPANSCAN_MASK, (bPanScan) ? 1 : 0);
         }

         /* If aspect ratio conversion is not enabled in the MPEG
          * control register set panscan and letterbox to FALSE for
          * the rest of this routine.
          */
         if (!bEnableAspectConv)
         {
            bPanScan = FALSE;
            bLetterBox = FALSE;
         }
         
         CNXT_SET_VAL(glpDrmMpegSize[dwRgnIndex], DRM_SIZE_WIDTH_MASK, 
             dwDstW);
         CNXT_SET_VAL(glpDrmMpegSize[dwRgnIndex], DRM_SIZE_HEIGHT_MASK, 
             dwDstH);
         CNXT_SET_VAL(glpDrmMpegPos[dwRgnIndex], DRM_POSITION_X_MASK, 
             (dwDstX << 1) + gnHBlank);
         CNXT_SET_VAL(glpDrmMpegPos[dwRgnIndex], DRM_POSITION_FIELD_START_MASK, 
             dwDstY % 2);
         CNXT_SET_VAL(glpDrmMpegPos[dwRgnIndex], DRM_POSITION_Y_MASK, 
             (dwDstY / 2) + gnVBlank);
         
         CNXT_SET_VAL(glpDrmMpegOffsetWidth[dwRgnIndex], DRM_MPEG_OFFSET_WIDTH_PAN_SCAN_OFFSET_MASK, dwPanOffset);
         CNXT_SET_VAL(glpDrmMpegOffsetWidth[dwRgnIndex], DRM_MPEG_OFFSET_WIDTH_FETCH_WIDTH_MASK, dwFetchWidth);
         CNXT_SET_VAL(glpDrmMpegOffsetWidth[dwRgnIndex], DRM_MPEG_OFFSET_WIDTH_ADDR_OFFSET_MASK, dwAddrOffset);
         CNXT_SET_VAL(glpDrmMpegVoffset[dwRgnIndex], DRM_MPEG_VOFFSET_VERTICAL_OFFSET_MASK, dwSrcY >> 1);
         CNXT_SET_VAL(glpDrmMpegVoffset[dwRgnIndex], DRM_MPEG_VOFFSET_FETCH_HEIGHT_MASK, dwSrcH);
          
         /* Program the DRM Y scaling register values */
         CNXT_SET_VAL(glpDrmMpegYInc[dwRgnIndex], DRM_MPEG_Y_INC_IP_Y_INC_MASK, dwIncYScale);

         CNXT_SET_VAL(glpDrmMpegYInc[dwRgnIndex], DRM_MPEG_Y_INC_B_Y_INC_MASK, (!bLetterBox) ? 
            CNXT_GET_VAL(glpDrmMpegYInc[dwRgnIndex], DRM_MPEG_Y_INC_IP_Y_INC_MASK) :
            (CNXT_GET_VAL(glpDrmMpegYInc[dwRgnIndex], DRM_MPEG_Y_INC_IP_Y_INC_MASK) * 3 / 4));

         if(DRM_HAS_WIPE_CHROMA_BUG == YES)
         {
           /* To get round a problem with wiping when the increments are set to 0x1000, fudge them slightly here */
           /* For some reason, if increment is set to 0x1000, if you try to wipe between video regions, you end  */
           /* up wiping the luma under the chroma of the top region. Changing the factor slightly results in     */
           /* correct operation.                                                                                 */
           if(CNXT_GET_VAL(glpDrmMpegYInc[dwRgnIndex], DRM_MPEG_Y_INC_IP_Y_INC_MASK) 
               == 0x1000)
             CNXT_SET_VAL(glpDrmMpegYInc[dwRgnIndex], DRM_MPEG_Y_INC_IP_Y_INC_MASK, 
               0x0FFF);
           if(CNXT_GET_VAL(glpDrmMpegYInc[dwRgnIndex], DRM_MPEG_Y_INC_B_Y_INC_MASK) 
               == 0x1000)
             CNXT_SET_VAL(glpDrmMpegYInc[dwRgnIndex], DRM_MPEG_Y_INC_B_Y_INC_MASK, 
               0x0FFF);
         } /* DRM_HAS_WIPE_CHROMA_BUG */
         
         /* Program the DRM X scaling register value */
         CNXT_SET_VAL(glpDrmMpegXInc[dwRgnIndex], DRM_MPEG_X_INC_X_INC_MASK, dwIncXScale);

         /* Program the split fifo mode */
         CNXT_SET_VAL(pDWIControl2_3, DRM_DWI_CONTROL_2_SPLIT_FIFO_LUMA_MASK, bSplitFIFO);
         
         #if DRM_NEEDS_SPLIT_FIFO_ON_WIPE == YES
         if(CNXT_GET(glpDrmMpgTileWipe[dwRgnIndex], DRM_MPG_TILE_WIPE_CTRL_WIPE_ENABLE_MASK))
         { 
           CNXT_SET_VAL(pDWIControl2_3, DRM_DWI_CONTROL_2_SPLIT_FIFO_CHROMA_MASK, TRUE);
         }
         else
         {
           CNXT_SET_VAL(pDWIControl2_3, DRM_DWI_CONTROL_2_SPLIT_FIFO_CHROMA_MASK, bSplitFIFO);
         }
         #else    
         CNXT_SET_VAL(pDWIControl2_3, DRM_DWI_CONTROL_2_SPLIT_FIFO_CHROMA_MASK, bSplitFIFO);
         #endif        
         
         /* Program the DRM filter selection
          * The VFilterSelect must be set to a 2 tap filter
          * if SplitFIFO mode is currently selected or if the X screen
          * resolution of the output image is greater than 240 pixels.
          */
         CNXT_SET_VAL(glpDrmMpegXInc[dwRgnIndex], DRM_MPEG_X_INC_V_FILTER_SELECT_MASK, 
            ((CNXT_GET_VAL(pDWIControl2_3, DRM_DWI_CONTROL_2_SPLIT_FIFO_LUMA_MASK)) ||
            (dwDstW > 240)) ? 0 : guserVTapSelect);

         /* Always enable frame scale since DRM will ignore it
          * when it is getting decoded MPEG. See DRM specification.
          * Useful when handling stills and doesn't hurt anything
          * if always set.
          */
         CNXT_SET_VAL(glpDrmMpegXInc[dwRgnIndex], DRM_MPEG_X_INC_LUMA_FRAME_SCALE_MASK, 1);
         CNXT_SET_VAL(glpDrmMpegXInc[dwRgnIndex], DRM_MPEG_X_INC_CHROMA_FRAME_SCALE_MASK, 1);

         /* 
          * Set the vertical lowpass filter depending on the scaling value from logic above.
          */
         CNXT_SET_VAL(glpDrmMpegXInc[dwRgnIndex], DRM_MPEG_X_INC_V_FILTER_MODE_MASK, dwVfilter);

         /* Determine whether X and Y decimation should occur */
         bYDecEnable = bXDecEnable = FALSE;

         /* The following uses a fixed point increment value to
          * program the DRM hardware DDA scaling. Bits 15:12
          * represent the integer part and bits 11:0 represent
          * the fractional part.
          */

         /* If IP_YInc is greater than 4 decimate in Y
          * The input MPEG source size is at least 4 times
          * larger than the output size.
          */
         if (CNXT_GET_VAL(glpDrmMpegYInc[dwRgnIndex], DRM_MPEG_Y_INC_IP_Y_INC_MASK) > 0x4000)
            bYDecEnable = TRUE;

         /* If XInc is greater than 4 decimate in X
          * The input MPEG source size is at least 4 times
          * larger than the output size.
          */
         if (CNXT_GET_VAL(glpDrmMpegXInc[dwRgnIndex], DRM_MPEG_X_INC_X_INC_MASK) > 0x4000)
         {
            bXDecEnable = TRUE;
            /* If SplitFIFOLuma is enabled decimate in Y also */
            if (CNXT_GET_VAL(pDWIControl2_3, DRM_DWI_CONTROL_2_SPLIT_FIFO_LUMA_MASK) == 1)
            {
               bYDecEnable = TRUE;
            }
         }

         /* If decimating in Y disable SplitFIFOChroma and SplitFIFOLuma
          * and set VFilterSelect to 0 for 2 tap filtering.
          */
         if (bYDecEnable)
         {
            CNXT_SET_VAL(pDWIControl2_3, DRM_DWI_CONTROL_2_SPLIT_FIFO_CHROMA_MASK, 0);
            CNXT_SET_VAL(pDWIControl2_3, DRM_DWI_CONTROL_2_SPLIT_FIFO_LUMA_MASK, 0);
            CNXT_SET_VAL(glpDrmMpegXInc[dwRgnIndex], DRM_MPEG_X_INC_V_FILTER_SELECT_MASK, 0);
         }

         /* Program the DRM settings for horizontal chroma and luma
          * decimation based on whether X decimation is enabled
          */
         CNXT_SET_VAL(glpDrmMpegXInc[dwRgnIndex], DRM_MPEG_X_INC_DEC_HORZ_CHROMA_MASK, (bXDecEnable) ? 1 : 0);
         CNXT_SET_VAL(glpDrmMpegXInc[dwRgnIndex], DRM_MPEG_X_INC_DEC_HORZ_LUMA_MASK, (bXDecEnable) ? 1 : 0);

         /* Program the DRM settings for vertical chroma and luma
          * decimation based on whether Y decimation is enabled
          */
         CNXT_SET_VAL(glpDrmMpegXInc[dwRgnIndex], DRM_MPEG_X_INC_DEC_VERT_CHROMA_MASK, (bYDecEnable) ? 1 : 0);
         CNXT_SET_VAL(glpDrmMpegXInc[dwRgnIndex], DRM_MPEG_X_INC_DEC_VERT_LUMA_MASK, (bYDecEnable) ? 1 : 0);

         /* Set the horizontal filter based on the horizontal downscale. */
         CNXT_SET_VAL(glpDrmMpegXInc[dwRgnIndex], DRM_MPEG_X_INC_H_FILTER_SELECT_MASK, dwHFilter);

         /* Disable the DRM error interrupt */
         CNXT_SET_VAL(glpDrmInterrupt, DRM_INTERRUPT_ENABLE_DRM_ERROR_MASK_MASK, 0);

         critical_section_end(ksPrev);
      }
   }
}


#ifdef USE_OLD_RECTANGLE_CALCULATION_CODE
/********************************************************************/
/*  FUNCTION:    vidCalculateVideoSrcAndDestination                 */
/*                                                                  */
/*  PARAMETERS:  pRgn           - Video region for which rectangles */
/*                                are to be calculated.             */
/*               prectSrc       - Storage for returned source       */
/*                                rectangle (image coordinates)     */
/*               prectDst       - Storage for returned destination  */
/*                                rectangle (screen coordinates)    */
/*               pbPanScan      - Storage for returned value        */
/*                                indicating whether or not         */
/*                                pan/scan is to be used.           */
/*               pbLetterBox    - Storage for returned value        */
/*                                indicating whether or not         */
/*                                letterboxing is to be used.       */
/*               pbEnableARConv - Storage for returned value        */
/*                                indicating whether or not to      */
/*                                enable AR conversion in hardware. */
/*                                                                  */
/*  DESCRIPTION: This function calculates rectangles for source and */
/*               destination to allow the passed video region to    */
/*               be displayed as expected given the various auto    */
/*               modes and aspect ratio handling options set.       */
/*                                                                  */
/*  RETURNS:     Nothing                                            */
/*                                                                  */
/*  CONTEXT:     May be called in any context                       */
/*                                                                  */
/********************************************************************/
static void vidCalculateVideoSrcAndDestination(PVIDRGN  pRgn, 
                                               OSDRECT *prectSrc, 
                                               OSDRECT *prectDest, 
                                               u_int32 *pdwPanCentreOffset,
                                               bool    *pbPanScan, 
                                               bool    *pbLetterBox,
                                               bool    *pbEnableARConv)
{
   u_int32 dwSrcH, dwSrcW;
   u_int32 dwDstH, dwDstW;
   u_int32 dwScaledH, dwScaledW;
   u_int32 dwAR;
   u_int32 dwScanOffset;
  
   /* Set default conditions */
   *pbPanScan = FALSE;
   *pbLetterBox = FALSE;
   *pbEnableARConv = FALSE;
   
   /* Get the screen destination rectangle width and height */
   dwDstH = pRgn->rcDst.bottom - pRgn->rcDst.top;
   dwDstW = pRgn->rcDst.right - pRgn->rcDst.left;

   /* pRgn->rcSrc points to a source rectangle specified in
    * terms of the desired output MPEG image. This gets the
    * initial values for prectSrc->left, prectSrc->top, dwSrcH, and dwSrcW
    * in terms of the source selection rectangle on the decoded
    * input MPEG image. The scale factors used are the decoded
    * input size divided by the allocated buffer size as follows:
    *    dwMpgHScale = pRgn->dwSrcW / pRgn->dwWidth;
    *    dwMpgVScale = pRgn->dwSrcH / pRgn->dwHeight;
    * where the multiplication is performed before the division
    * to avoid integer arithmetic errors when the input size is
    * smaller than the allocated buffer size.
    */   
   /* @@@SG It seems that for automode, rcSrc is always the whole
    * of the source image, so dwSrcH and the rcSrc(b-t) are
    * equivalent.
    */

   if ( (pRgn->dwAutoMode & VID_AUTO_SCALE) != 0 )
   {
      prectSrc->left = (pRgn->rcSrc.left * pRgn->dwSrcW) / pRgn->dwWidth;
      prectSrc->left &= 0xFFFFFFFE;
      /* Default to full source image for auto mode */
      dwSrcW = (pRgn->rcSrc.right - pRgn->rcSrc.left);
      
      /* DW - removed this check for now. Used to be OPENTV_12 */ 

      #ifdef OPENTV_12_LIMITED_SCALING
      /* For OpenTV 1.2 we limit all video height scaling to values */
      /* 0.5x, 1.0x and 2.0x to mimic existing BSkyB IRDs           */
      {
         u_int32 dwVScale;    /*  Vertical Scale Factor */
                          
         /* Calculate a raw scale factor of the Src to Dest */                                               
         dwVScale = (FULL_SCALE_VIDEO * (pRgn->dwSrcH)) / dwDstH; 
        
         /* Now make it match one of the fixed allowed values */
         if (dwVScale >= DOUBLE_SCALE_VIDEO)
         {                  
            dwVScale = DOUBLE_SCALE_VIDEO;
         } else if (dwVScale >= FULL_SCALE_VIDEO) {
                                      
            dwVScale = FULL_SCALE_VIDEO;
         } else {          
                            
            dwVScale = HALF_SCALE_VIDEO;
         } 
                                                                
         /* Given the scaling, calculate the height in the source
          * based on the destination rectangle height 
          */                                        
         dwSrcH = ((u_int32)(pRgn->rcDst.bottom - pRgn->rcDst.top) *
                     dwVScale) / FULL_SCALE_VIDEO;
         /* Make sure we stay within the source image */
         if (dwSrcH > pRgn->dwSrcH)
         {
            dwSrcH = pRgn->dwSrcH;
         }                                                          
         /* @@@SG This used to be 
          * dwSrcY = ((u_int32)(pRgn->rcDst.top) * dwVScale) / 0x400;
          * But we want to center within the source image 
          */
          
          /* DW: If we don't round this down to the next lower multiple  */
          /*     of 16, we end up positioning video slightly differently */
          /*     than the existing ST IRDs.                              */
         prectSrc->top = ((pRgn->dwSrcH - dwSrcH) >> 1) & ~15; 
      }
      #else
      /* Arbitrary scaling case */
      prectSrc->top = (pRgn->rcSrc.top * pRgn->dwSrcH) / pRgn->dwHeight;
      dwSrcH = ((pRgn->rcSrc.bottom - pRgn->rcSrc.top) *
                  pRgn->dwSrcH) / pRgn->dwHeight;
      #endif            
   }
   else
   {                
      /* If source rectangle specified use it, else calculate it */
      if (pRgn->bSrcRcSpec)
      {
         /* Auto scaling is not enabled, so the given source
          * rectangle is used. This will be checked against
          * the decoded size later and fix up if necessary.
          */
         prectSrc->left = pRgn->rcSrc.left;
         prectSrc->left &= 0xFFFFFFFE;
         dwSrcW = pRgn->rcSrc.right - pRgn->rcSrc.left;

         prectSrc->top = pRgn->rcSrc.top;
         dwSrcH = pRgn->rcSrc.bottom - pRgn->rcSrc.top;
      
      } else { 
         /* Destination and scale factor determine source rect */
         prectSrc->left = 0;
         prectSrc->top = 0;
         dwSrcW = (dwDstW * FULL_SCALE_VIDEO) / pRgn->dwHScale;
         dwSrcH = (dwDstH * FULL_SCALE_VIDEO) / pRgn->dwVScale;
         /* If calculated Src W/H less than actual src W/H, center */
         if (dwSrcW < pRgn->dwSrcW)
         {                        
            prectSrc->left = ((pRgn->dwSrcW - dwSrcW) >> 1) & 0xFFFFFFFE;
         } 
         if (dwSrcH < pRgn->dwSrcH)
         {                        
            prectSrc->top = ((pRgn->dwSrcH - dwSrcH) >> 1) & 0xFFFFFFFE;
         } 
        
      } /* endif Src Rectangle Specified */

   }

   /* Make sure that the clipping src rectangle is inside
    * the input decoded MPEG video image.
    */
   if (dwSrcW > pRgn->dwSrcW)
   {
      dwSrcW = pRgn->dwSrcW;
   }
   if (dwSrcH > pRgn->dwSrcH)
   {
      dwSrcH = pRgn->dwSrcH;
   }

   /* Get the current aspect ratio of the video stream as set
    * by the MPEG decoder on the AR change interrupt via
    * UpdateMpgScaling.
    */
   dwAR = pRgn->dwAR;
   /* Reset any invalid Aspect ratios to MPG_VID_ASPECT_43. */
   if ((dwAR != MPG_VID_ASPECT_43) &&
      #ifdef USE_14_9
      (dwAR != MPG_VID_ASPECT_149) &&
      #endif /* USE_14_9 */
      (dwAR != MPG_VID_ASPECT_169) &&
      (dwAR != MPG_VID_ASPECT_209))
      dwAR = MPG_VID_ASPECT_43;

   /* If AR mode for the video region is not none/don't care
    * check if we need to perform PANSCAN or LETTERBOX
    * when the aspect ratio of video and display do not match.
    */
   /* NOTE: This essentially behaves as an automatic scaling
    * and should check to see if AR conversion should be performed
    * before setting these flags for the rest of the routine.
    */
   if ( (pRgn->arMode != OSD_ARM_NONE) &&
        (pRgn->dwAutoMode & VID_AUTO_WINDOW) )
        /* (pRgn->dwAutoMode == VID_AUTO_ALL) ) - old version */
   {
      if (((dwAR != MPG_VID_ASPECT_43) && (gDisplayAR == OSD_DISPLAY_AR_43))||
         ((dwAR != MPG_VID_ASPECT_169) && (gDisplayAR == OSD_DISPLAY_AR_169)))
      {
         if (pRgn->arMode == OSD_ARM_PANSCAN)
            *pbPanScan = TRUE;
         if (pRgn->arMode == OSD_ARM_LETTERBOX)
            *pbLetterBox = TRUE;
      }
   }

   /* If we are letterboxing modify the destination
    * width or height as appropriate.
    */

   /* NOTE: Enabling aspect ratio conversion here may also
    * set certain behaviors that imply automode behavior.
    * May need to check automode and change behavior here
    * and in the letterbox scaling setup.
    */
   if (*pbLetterBox)
   {
      /* The display has either aspect ratio 4:3 or 16:9 */
      if (gDisplayAR == OSD_DISPLAY_AR_43)
      {
       #ifdef DECODER_LETTERBOX
         /* Always do aspect conversion if 4:3 aspect display
          * and video aspect is different when letterboxing
          */
         *pbEnableARConv = TRUE;
       #endif
         switch (dwAR)
         {
         case MPG_VID_ASPECT_43: /* Shouldn't get here! */
            break;
         case MPG_VID_ASPECT_169:
            dwDstH = dwDstH * 3 / 4;
            break;
         case MPG_VID_ASPECT_209:
            dwDstH = dwDstH * 3 / 5;
            break;
         #ifdef USE_14_9
         case MPG_VID_ASPECT_149:
            dwDstH = dwDstH * 6 / 7;
            break;
         #endif /* USE_14_9 */
         }
      }
      else /* gDisplayAR == OSD_DISPLAY_AR_169 */
      {
         switch (dwAR)
         {
         case MPG_VID_ASPECT_43:
            dwDstW = dwDstW * 3 / 4;
            break;
         case MPG_VID_ASPECT_169: /* Shouldn't get here! */
            break;
         case MPG_VID_ASPECT_209:
          #ifdef DECODER_LETTERBOX
            /* Do aspect conversion if 16:9 aspect display
             * and video aspect is 20:9.
             */
            *pbEnableARConv = TRUE;
          #endif
            dwDstH = dwDstH * 4 / 5;
            break;
         #ifdef USE_14_9
         case MPG_VID_ASPECT_149:
            dwDstW = dwDstW * 7 / 8;
            break;
         #endif /* USE_14_9 */
         }
      }
   } /* end if *pbLetterBox */

   /* If we are in panscan mode set up the panscan offset and
    * data fetch width values based on the current source
    * width and height. These are used to program the DRM registers.
    */
   /* NOTE: Enabling aspect ratio conversion here may also
    * set certain behaviors that imply automode behavior.
    * May need to check automode and change behavior here.
    */
   if (*pbPanScan)
   {
      if (gDisplayAR == OSD_DISPLAY_AR_43)
      {
         /* Always do aspect conversion if 4:3 aspect display
          * and video aspect is different when in pan/scan
          */
         *pbEnableARConv = TRUE;
         switch (dwAR)
         {
         case MPG_VID_ASPECT_169:
            *pdwPanCentreOffset = dwSrcW >> 3;
            dwSrcW = dwSrcW * 3 / 4;
            break;
         case MPG_VID_ASPECT_209:
            *pdwPanCentreOffset = dwSrcW / 10;
            dwSrcW = dwSrcW * 3 / 5;
            break;
         #ifdef USE_14_9
         case MPG_VID_ASPECT_149:
            *pdwPanCentreOffset = dwSrcW / 14;
            dwSrcW = dwSrcW * 6 / 7;
            break;
         #endif /* USE_14_9 */
         case MPG_VID_ASPECT_43: /* Shouldn't get here! */
         default:
            *pdwPanCentreOffset = 0;
            break;
         }
        
         /* If setting an explicit pan/scan vector, we update the source rectangle   */
         /* manually. This is needed since, in this mode (at least for OpenTV cases) */
         /* we are generally displaying an MPEG still. The DRM's automatic pan/scan  */
         /* function relies upon signals from the video decoder and, if these are    */
         /* not present, the offset is not applied. Hence it is not possible to use  */
         /* the pan offset hardware to pan a still.                                  */
         if ((pRgn->dwAutoMode & VID_AUTO_PANSCAN) == 0)
           prectSrc->left += *pdwPanCentreOffset;
      }
      else /* gDisplayAR == OSD_DISPLAY_AR_169 */
      {
         switch (dwAR)
         {
         case MPG_VID_ASPECT_43:
            *pdwPanCentreOffset = 0;
            dwScanOffset = dwSrcH >> 3;
            dwSrcH = (dwSrcH * 3) >> 2;
            break;
         case MPG_VID_ASPECT_209:
            /* Do aspect conversion if 16:9 aspect display
             * and video aspect is 20:9.
             */
            *pbEnableARConv = TRUE;
            dwScanOffset = 0;
            *pdwPanCentreOffset = dwSrcW / 10;
            dwSrcW = dwSrcW * 4 / 5;
            break;
         #ifdef USE_14_9
         case MPG_VID_ASPECT_149:
            *pdwPanCentreOffset = 0;
            dwScanOffset = dwSrcH >> 4;
            dwSrcH = (dwSrcH * 7) >> 3;
            break;
         #endif /* USE_14_9 */
         case MPG_VID_ASPECT_169: /* Shouldn't get here! */
         default:
            dwScanOffset = 0;
            *pdwPanCentreOffset = 0;
            break;
         }
         prectSrc->top += dwScanOffset;
      }
   }
   else /* if not doing panscan */
   {
      /* Use pan/scan vectors to set *pdwPanCentreOffset? */
      *pdwPanCentreOffset = 0;
   } /* end of if *pbPanScan */

   /* Fixup the SrcX and SrcY if needed to create a centered image */
   if ((prectSrc->top + dwSrcH) > pRgn->dwSrcH)
   {
      prectSrc->top = (pRgn->dwSrcH - dwSrcH) >> 1;
   }                 
   if ((prectSrc->left + dwSrcW) > pRgn->dwSrcW)
   {
      prectSrc->left = (pRgn->dwSrcW - dwSrcW) >> 1; 
   }                   
  
   /* If not set to automatic pan/scan mode apply the */
   /* pan/scan vectors to SrcX and SrcY.              */
  
   if ((pRgn->dwAutoMode & VID_AUTO_PANSCAN) == 0)
   {
      if (((int)prectSrc->left + (int)pRgn->dwPan + (int)*pdwPanCentreOffset) > 0)
      {
         prectSrc->left = (u_int32)((int)prectSrc->left + (int)pRgn->dwPan + (int)*pdwPanCentreOffset);
         prectSrc->left = min(prectSrc->left, pRgn->dwWidth-dwSrcW);
      }
      else
      {
         prectSrc->left = 0;
      }
      
      if (((int)prectSrc->top + (int)pRgn->dwScan) > 0)
      {
         prectSrc->top = (u_int32)((int)prectSrc->top + (int)pRgn->dwScan);
         prectSrc->top = min(prectSrc->top, pRgn->dwHeight-dwSrcH);
      }
      else
      {
         prectSrc->top = 0;
      }
      
      /* Fix up the pan/scan vectors to take into account cases where */
      /* they reach their maximum or minimum values.                  */
      pRgn->dwPan  = (u_int32)((int)*pdwPanCentreOffset  - ((int)pRgn->dwWidth - (int)dwSrcW)/2);
      pRgn->dwScan = (u_int32)((int)dwScanOffset - ((int)pRgn->dwHeight - (int)dwSrcH)/2);
   }

   /* Adjust the destination X and Y to compensate for any changes */
   /* to DstW and DstH above.                                      */

   /* Adjust the starting screen Y coordinate */
   prectDst->top = pRgn->rcDst.top + 
      (((pRgn->rcDst.bottom - pRgn->rcDst.top) - dwDstH) / 2);

   /* Adjust the starting screen X coordinate */
   prectDst->left = pRgn->rcDst.left + 
      (((pRgn->rcDst.right - pRgn->rcDst.left) - dwDstW) / 2);

   /* Make sure that the X coordinate is on an even pixel */
   prectDst->left &= 0xFFFFFFFE;

   if ( (pRgn->dwAutoMode & VID_AUTO_SCALE) == 0 )
   {
      /* Fixup the destination rectangle to match current
      * scaling if required. This is necessary since
      * scaling is sometimes set before a decode to
      * the video plane and the correct destination
      * rectangle will not have been set.
      */
      dwScaledH = (dwSrcH * pRgn->dwVScale)/0x0400;
      if ( (dwDstH != dwScaledH) && (dwScaledH != 0) )
      {
         dwDstH = dwScaledH;
         if ( (prectDst->top + dwDstH) > gnOsdMaxHeight)
         {
            dwDstH -= ((prectDst->top + dwDstH) - gnOsdMaxHeight);
         }
      }

      dwScaledW = (dwSrcW * pRgn->dwHScale)/0x0400;
      if ( (dwDstW != dwScaledW) && (dwScaledW != 0) )
      {
         dwDstW = dwScaledW;
         if ( (prectDst->left + dwDstW) > gnOsdMaxWidth)
         {
            dwDstW -= ((prectDst->left + dwDstW) - gnOsdMaxWidth);
         }
      }
   }
   
   /* Set bottom right rectangle coordinates */
   prectSrc->bottom = prectSrc->top  + dwSrcH;
   prectSrc->right  = prectSrc->left + dwSrcW;
   prectDst->bottom = prectDst->top  + dwDstH;
   prectDst->right  = prectDst->left + dwDstW;
}
#else
/********************************************************************/
/*  FUNCTION:    vidCalculateVideoSrcAndDestination                 */
/*               *** NEW VERSION ***                                */
/*                                                                  */
/*  PARAMETERS:  pRgn           - Video region for which rectangles */
/*                                are to be calculated.             */
/*               prectSrc       - Storage for returned source       */
/*                                rectangle (image coordinates)     */
/*               prectDst       - Storage for returned destination  */
/*                                rectangle (screen coordinates)    */
/*               pbPanScan      - Storage for returned value        */
/*                                indicating whether or not         */
/*                                pan/scan is to be used.           */
/*               pbLetterBox    - Storage for returned value        */
/*                                indicating whether or not         */
/*                                letterboxing is to be used.       */
/*               pbEnableARConv - Storage for returned value        */
/*                                indicating whether or not to      */
/*                                enable AR conversion in hardware. */
/*                                                                  */
/*  DESCRIPTION: This function calculates rectangles for source and */
/*               destination to allow the passed video region to    */
/*               be displayed as expected given the various auto    */
/*               modes and aspect ratio handling options set.       */
/*                                                                  */
/*  RETURNS:     Nothing                                            */
/*                                                                  */
/*  CONTEXT:     May be called in any context                       */
/*                                                                  */
/********************************************************************/
static void vidCalculateVideoSrcAndDestination(PVIDRGN  pRgn, 
                                               OSDRECT *prectSrc, 
                                               OSDRECT *prectDst, 
                                               u_int32 *pdwPanCentreOffset,
                                               bool    *pbPanScan, 
                                               bool    *pbLetterBox,
                                               bool    *pbEnableARConv)
{
   u_int32 dwSrcH, dwSrcW;
   u_int32 dwDstH, dwDstW;
   u_int32 dwScaledH, dwScaledW;
   u_int32 dwAR;
   u_int32 dwScanCentreOffset = 0;
  
   /* Set default conditions */
   *pbPanScan      = FALSE;
   *pbLetterBox    = FALSE;
   *pbEnableARConv = FALSE;
   
   /* Get the screen destination rectangle width and height */
   dwDstH = pRgn->rcDst.bottom - pRgn->rcDst.top;
   dwDstW = pRgn->rcDst.right - pRgn->rcDst.left;

   /* pRgn->rcSrc is a source rectangle specified in the   */
   /* MPEG image coordinate space.                         */
   /* pRgn->rcDst is a destination rectangle specified in  */
   /* screen pixel coordinates.                            */
   
   /******************************************************************/
   /* Determine the basic source rectangle to use based upon the     */
   /* scaling mode selected and either the supplied source rectangle */
   /* or scaling parameters.                                         */
   /******************************************************************/
   if ( (pRgn->dwAutoMode & VID_AUTO_SCALE) != 0 )
   {
      /* It seems that for automode, rcSrc is always the whole */
      /* of the source image, so dwSrcH and the rcSrc(b-t) are */
      /* equivalent.                                           */
      prectSrc->left = pRgn->rcSrc.left;
      prectSrc->top  = pRgn->rcSrc.top;
      prectSrc->left &= 0xFFFFFFFE;
      
      dwSrcW = pRgn->rcSrc.right - pRgn->rcSrc.left;
      dwSrcH = pRgn->rcSrc.bottom - pRgn->rcSrc.top;
   }
   else
   {                
      /* If source rectangle specified use it, else calculate it */
      if (pRgn->bSrcRcSpec)
      {
         /* Use the supplied source rectangle (we will fix this up later if needed) */
         prectSrc->left = pRgn->rcSrc.left;
         prectSrc->top  = pRgn->rcSrc.top;
         prectSrc->left &= 0xFFFFFFFE;
         
         dwSrcW = pRgn->rcSrc.right - pRgn->rcSrc.left;
         dwSrcH = pRgn->rcSrc.bottom - pRgn->rcSrc.top;
      }
      else
      { 
         /* Destination and scale factors determine source rect (pan/scan handled later) */
         prectSrc->left = 0;
         prectSrc->top = 0;
         dwSrcW = (dwDstW * FULL_SCALE_VIDEO) / pRgn->dwHScale;
         dwSrcH = (dwDstH * FULL_SCALE_VIDEO) / pRgn->dwVScale;
         
         /* If calculated Src W/H less than actual src W/H, center */ 
         /* if auto pan scan is enabled                            */
         
         /* DW: This fix is slightly questionable and may need to be  */
         /*     revisited later. The behaviour when the destination   */
         /*     is smaller than the scaled source is really something */
         /*     that is network-dependent.                            */
         
         if(pRgn->dwAutoMode & VID_AUTO_PANSCAN)
         {
           if (dwSrcW < pRgn->dwSrcW)
           {                        
              prectSrc->left = ((pRgn->dwSrcW - dwSrcW) >> 1) & 0xFFFFFFFE;
           } 
           if (dwSrcH < pRgn->dwSrcH)
           {                        
              prectSrc->top = ((pRgn->dwSrcH - dwSrcH) >> 1) & 0xFFFFFFFE;
           } 
         }
      } /* endif Src Rectangle Specified */
   }

   /* Make sure that the src rectangle is inside */
   /* the input decoded MPEG video image.        */
   if (dwSrcW > pRgn->dwSrcW)
   {
      dwSrcW = pRgn->dwSrcW;
   }
   if (dwSrcH > pRgn->dwSrcH)
   {
      dwSrcH = pRgn->dwSrcH;
   }

   /*******************************************************************/
   /* Now handle alterations required to deal with mismatching stream */
   /* and display aspect ratios.                                      */
   /*******************************************************************/
   
   /* Get the aspect ratio of the image in the current video region */
   dwAR = pRgn->dwAR;
   
   /* Reset any invalid aspect ratios to MPG_VID_ASPECT_43. */
   if ((dwAR != MPG_VID_ASPECT_43)  &&
       (dwAR != MPG_VID_ASPECT_169) &&
       (dwAR != MPG_VID_ASPECT_209))
      dwAR = MPG_VID_ASPECT_43;

   /* If AR mode for the video region is not none/don't care   */
   /* check if we need to perform PANSCAN or LETTERBOX         */
   /* when the aspect ratio of video and display do not match. */
   if ( (pRgn->arMode != OSD_ARM_NONE) &&
        (pRgn->dwAutoMode & VID_AUTO_WINDOW) )
   {
      /* If the aspect ratios do not match... */
      if (((dwAR != MPG_VID_ASPECT_43)  && (gDisplayAR == OSD_DISPLAY_AR_43))||
          ((dwAR != MPG_VID_ASPECT_169) && (gDisplayAR == OSD_DISPLAY_AR_169)))
      {
         /* ...decide whether to do pan/scan or letterbox */
         if (pRgn->arMode == OSD_ARM_PANSCAN)
            *pbPanScan = TRUE;
         if (pRgn->arMode == OSD_ARM_LETTERBOX)
            *pbLetterBox = TRUE;
      }
   }

   /* If we are letterboxing modify the destination */
   /* width or height as appropriate.               */

   if (*pbLetterBox)
   {
      /* The display has either aspect ratio 4:3 or 16:9 */
      if (gDisplayAR == OSD_DISPLAY_AR_43)
      {
         switch (dwAR)
         {
         case MPG_VID_ASPECT_43: /* Shouldn't get here! */
            break;
         case MPG_VID_ASPECT_169:
            dwDstH = dwDstH * 3 / 4;
            break;
         case MPG_VID_ASPECT_209:
            dwDstH = dwDstH * 3 / 5;
            break;
         }
      }
      else /* gDisplayAR == OSD_DISPLAY_AR_169 */
      {
         switch (dwAR)
         {
         case MPG_VID_ASPECT_43:
            dwDstW = dwDstW * 3 / 4;
            break;
         case MPG_VID_ASPECT_169: /* Shouldn't get here! */
            break;
         case MPG_VID_ASPECT_209:
            dwDstH = dwDstH * 4 / 5;
            break;
         }
      }
   } /* end if *pbLetterBox */

   /* If we are in panscan mode set up the panscan centre offset */
   /* value based on the current source width and height.        */
   if (*pbPanScan)
   {
      if (gDisplayAR == OSD_DISPLAY_AR_43)
      {
         /* We always do hardware aspect conversion with a 4:3 aspect ratio */
         /* display and a different video aspect ratio when in pan/scan     */
         *pbEnableARConv = TRUE;
         
         switch (dwAR)
         {
         case MPG_VID_ASPECT_169:
            *pdwPanCentreOffset = dwSrcW >> 3;
            dwSrcW = dwSrcW * 3 / 4;
            break;
         case MPG_VID_ASPECT_209:
            *pdwPanCentreOffset = dwSrcW / 10;
            dwSrcW = dwSrcW * 3 / 5;
            break;
         case MPG_VID_ASPECT_43: /* Shouldn't get here! */
         default:
            *pdwPanCentreOffset = 0;
            break;
         }       
      }
      else /* gDisplayAR == OSD_DISPLAY_AR_169 */
      {
         switch (dwAR)
         {
         case MPG_VID_ASPECT_43:
            *pdwPanCentreOffset  = 0;
            dwScanCentreOffset = dwSrcH >> 3;
            dwSrcH = (dwSrcH * 3) >> 2;
            break;
         case MPG_VID_ASPECT_209:
            /* Do aspect conversion if 16:9 aspect display
             * and video aspect is 20:9.
             */
            *pbEnableARConv    = TRUE;
            dwScanCentreOffset = 0;
            *pdwPanCentreOffset  = dwSrcW / 10;
            dwSrcW = dwSrcW * 4 / 5;
            break;
         case MPG_VID_ASPECT_169: /* Shouldn't get here! */
         default:
            dwScanCentreOffset = 0;
            *pdwPanCentreOffset  = 0;
            break;
         }
         prectSrc->top += dwScanCentreOffset;
      }
   }
   else /* if not doing panscan */
   {
      *pdwPanCentreOffset = 0;
   } /* end of if *pbPanScan */

   /* If not set to automatic pan/scan mode apply the */
   /* pan/scan vectors to SrcX and SrcY.              */
  
   if ((pRgn->dwAutoMode & VID_AUTO_PANSCAN) == 0)
   {
      prectSrc->left = (short)((int)prectSrc->left + (int)pRgn->dwPan + (int)*pdwPanCentreOffset);
      prectSrc->left = min(prectSrc->left, (short)pRgn->dwWidth-(short)dwSrcW);
      prectSrc->left = max(prectSrc->left, 0);
      
      prectSrc->top = (short)((int)prectSrc->top + (int)pRgn->dwScan);
      prectSrc->top = min(prectSrc->top, (short)pRgn->dwHeight-(short)dwSrcH);
      prectSrc->top = max(prectSrc->top, 0);
      
      /* Fix up the pan/scan vectors to take into account cases where */
      /* they reach their maximum or minimum values.                  */
      pRgn->dwPan  = (u_int32)((int)prectSrc->left - (int)*pdwPanCentreOffset);
      pRgn->dwScan = (u_int32)((int)prectSrc->top - (int)dwScanCentreOffset);
   }

   /* Adjust the destination X and Y to compensate for any changes */
   /* to DstW and DstH above.                                      */

   /* Adjust the starting screen Y coordinate */
   prectDst->top = pRgn->rcDst.top + 
      (((pRgn->rcDst.bottom - pRgn->rcDst.top) - dwDstH) / 2);

   /* Adjust the starting screen X coordinate */
   prectDst->left = pRgn->rcDst.left + 
      (((pRgn->rcDst.right - pRgn->rcDst.left) - dwDstW) / 2);

   /* Make sure that the X coordinate is on an even pixel */
   prectDst->left &= 0xFFFFFFFE;

   if ( (pRgn->dwAutoMode & VID_AUTO_SCALE) == 0 )
   {
      /* Fixup the destination rectangle to match current */
      /* scaling if required. This is necessary since     */
      /* scaling is sometimes set before a decode to      */
      /* the video plane and the correct destination      */
      /* rectangle will not have been set.                */
      dwScaledH = (dwSrcH * pRgn->dwVScale)/0x0400;
      if ( (dwDstH != dwScaledH) && (dwScaledH != 0) )
      {
         dwDstH = dwScaledH;
         if ( (prectDst->top + dwDstH) > gnOsdMaxHeight)
         {
            dwDstH -= ((prectDst->top + dwDstH) - gnOsdMaxHeight);
         }
      }

      dwScaledW = (dwSrcW * pRgn->dwHScale)/0x0400;
      if ( (dwDstW != dwScaledW) && (dwScaledW != 0) )
      {
         dwDstW = dwScaledW;
         if ( (prectDst->left + dwDstW) > gnOsdMaxWidth)
         {
            dwDstW -= ((prectDst->left + dwDstW) - gnOsdMaxWidth);
         }
      }
   }
   
   /* Set bottom right rectangle coordinates */
   prectSrc->bottom = prectSrc->top  + dwSrcH;
   prectSrc->right  = prectSrc->left + dwSrcW;
   prectDst->bottom = prectDst->top  + dwDstH;
   prectDst->right  = prectDst->left + dwDstW;
}
#endif /* USE_OLD_RECTANGLE_CALCULATION_CODE */

/*****************************************************************************/
/* Function: vidPrepareMpgForAspectHandlingChange                            */
/*                                                                           */
/* Type: Internal (but called from OSDLIBC)                                  */
/*                                                                           */
/* Parameters: u_int32 dwRgnIndex - Index into which of the video planes to  */
/*                                  update.                                  */
/*                                                                           */
/* Returns: void                                                             */
/*                                                                           */
/* Description: Sets the MPEG decoder aspect ratio convert and panscan/      */
/*              letterbox control only. This is done before the DRM updates  */
/*              to ensure that the change does not include nasty "bouncing"  */
/*              in the transition.                                           */
/*                                                                           */
/* Context:     This function is called under interrupt context              */
/*                                                                           */
/*****************************************************************************/
void vidPrepareMpgForAspectHandlingChange(u_int32 dwRgnIndex)
{
   PVIDRGN pRgn;

   u_int32 dwAR;
   u_int32 dwEnableAspectConv = 0;
   bool bPanScan = FALSE;
   bool bLetterBox = FALSE;
   bool ksPrev;

   isr_trace_new(TRACE_LEVEL_2|TRACE_MPG, "VIDEO: DRM video scaling update for plane %d\n", dwRgnIndex, 0);
   
   /* There are currently a maximum of 2 Video Regions */
   if (dwRgnIndex < VID_MAX_VID_RGN)
   {
      pRgn = gpActiveVidRgn[dwRgnIndex];
      if (pRgn) /* If the video region is active this will not be NULL */
      {
         /* Get the current aspect ratio of the video stream as set
          * by the MPEG decoder on the AR change interrupt via
          * UpdateMpgScaling.
          */
         dwAR = pRgn->dwAR;
         /* Reset any invalid Aspect ratios to MPG_VID_ASPECT_43. */
         if ((dwAR != MPG_VID_ASPECT_43) &&
            #ifdef USE_14_9
            (dwAR != MPG_VID_ASPECT_149) &&
            #endif /* USE_14_9 */
            (dwAR != MPG_VID_ASPECT_169) &&
            (dwAR != MPG_VID_ASPECT_209))
            dwAR = MPG_VID_ASPECT_43;

         /* If AR mode for the video region is not none/don't care
          * check if we need to perform PANSCAN or LETTERBOX
          * when the aspect ratio of video and display do not match.
          */
         /* NOTE: This essentially behaves as an automatic scaling
          * and should check to see if AR conversion should be performed
          * before setting these flags for the rest of the routine.
          */
         if ( (pRgn->arMode != OSD_ARM_NONE) &&
              (pRgn->dwAutoMode == VID_AUTO_ALL) )
         {
            if (((dwAR != MPG_VID_ASPECT_43) && (gDisplayAR == OSD_DISPLAY_AR_43))||
               ((dwAR != MPG_VID_ASPECT_169) && (gDisplayAR == OSD_DISPLAY_AR_169)))
            {
               if (pRgn->arMode == OSD_ARM_PANSCAN)
                  bPanScan = TRUE;
               if (pRgn->arMode == OSD_ARM_LETTERBOX)
                  bLetterBox = TRUE;
            }
         }

         /* If we are letterboxing modify the destination
          * width or height as appropriate.
          */

         /* NOTE: Enabling aspect ratio conversion here may also
          * set certain behaviors that imply automode behavior.
          * May need to check automode and change behavior here
          * and in the letterbox scaling setup.
          */
         if (bLetterBox)
         {
            /* The display has either aspect ratio 4:3 or 16:9 */
            if (gDisplayAR == OSD_DISPLAY_AR_43)
            {
              #ifdef DECODER_LETTERBOX
              /* Always do aspect conversion if 4:3 aspect display
               * and video aspect is different when letterboxing
               */
              dwEnableAspectConv = 1;
              #endif
            }
            else /* gDisplayAR == OSD_DISPLAY_AR_169 */
            {
                #ifdef DECODER_LETTERBOX
                if(dwAR == MPG_VID_ASPECT_209)
                {
                  /* Do aspect conversion if 16:9 aspect display
                   * and video aspect is 20:9.
                   */
                  dwEnableAspectConv = 1;
                }  
                #endif
            }
         } /* end if bLetterBox */

       
         /* If we are in panscan mode set up the panscan offset and
          * data fetch width values based on the current source
          * width and height. These are used to program the DRM registers.
          */
         /* NOTE: Enabling aspect ratio conversion here may also
          * set certain behaviors that imply automode behavior.
          * May need to check automode and change behavior here.
          */
         if (bPanScan)
         {
            if (gDisplayAR == OSD_DISPLAY_AR_43)
            {
               /* Always do aspect conversion if 4:3 aspect display
                * and video aspect is different when in pan/scan
                */
               dwEnableAspectConv = 1;
            }
            else /* gDisplayAR == OSD_DISPLAY_AR_169 */
            {
               if(dwAR == MPG_VID_ASPECT_209)
                 dwEnableAspectConv = 1;
            }
         }
        
         ksPrev = critical_section_begin();
          
         if (pRgn->vrConnection == HARDWARE)
         {
            CNXT_SET_VAL(glpCtrl0, MPG_CONTROL0_ENABLEASPECTCONVERT_MASK, dwEnableAspectConv);
            CNXT_SET_VAL(glpCtrl0, MPG_CONTROL0_ENABLEPANSCAN_MASK, (bPanScan) ? 1 : 0);
         }

         critical_section_end(ksPrev);
      }
   }
}

/*****************************************************************************/
/* Function: vidUpdate422Scaling()                                           */
/*                                                                           */
/* Type: Internal                                                            */
/*                                                                           */
/* Parameters: u_int32 dwRgnIndex - Index into which of the video planes to  */
/*                                  update.                                  */
/*                                                                           */
/* Returns: void                                                             */
/*                                                                           */
/* Description: Updates the 422 DRM video scaling registers.  This           */
/*              function is call from ISRs and tasks.                        */
/*****************************************************************************/
void vidUpdate422Scaling(u_int32 dwRgnIndex)
{
   PVIDRGN pRgn;
   u_int32 dwSrcX, dwSrcY, dwDstX, dwDstY, dwHeight, dwWidth;

   if (dwRgnIndex < VID_MAX_VID_RGN)
   {
      pRgn = gpActiveVidRgn[dwRgnIndex];
      if (pRgn)
      {
         dwSrcX = pRgn->rcSrc.left & 0xFFFFFFFE;
         dwSrcY = pRgn->rcSrc.top;
         dwHeight = pRgn->rcDst.bottom - pRgn->rcDst.top;
         dwWidth = pRgn->rcDst.right - pRgn->rcDst.left;
         dwDstX = pRgn->rcDst.left & 0xFFFFFFFE;
         dwDstY = pRgn->rcDst.top;

         if ((dwSrcX < pRgn->dwWidth) && (dwSrcY < pRgn->dwHeight) &&
            (dwDstY < gnOsdMaxHeight) && (dwDstY < OSD_MAX_WIDTH))
         {
            if ((dwSrcX + dwWidth) > pRgn->dwWidth)
               dwWidth = pRgn->dwWidth - dwSrcX;
            if ((dwSrcY + dwHeight) > pRgn->dwHeight)
               dwHeight = pRgn->dwHeight - dwSrcY;

            CNXT_SET_VAL(glpDrmVidBuf0[dwRgnIndex], DRM_ADDRESS_ADDRESS_MASK, (u_int32)pRgn->pBuffer + 
               (pRgn->dwStride * dwSrcY) + (dwSrcX << 1));
            CNXT_SET_VAL(glpDrmVidBuf1[dwRgnIndex], DRM_ADDRESS_ADDRESS_MASK, (u_int32)pRgn->pBuffer + 
               (pRgn->dwStride * (dwSrcY + 1)) + (dwSrcX << 1));
            CNXT_SET_VAL(glpDrmVidPos[dwRgnIndex], DRM_POSITION_X_MASK, (dwDstX << 1) + gnHBlank);
            CNXT_SET_VAL(glpDrmVidPos[dwRgnIndex], DRM_POSITION_Y_MASK, (dwDstY >> 1) + gnVBlank);
            CNXT_SET_VAL(glpDrmVidPos[dwRgnIndex], DRM_POSITION_FIELD_START_MASK, dwDstY & 1);
            CNXT_SET_VAL(glpDrmVidSize[dwRgnIndex], DRM_VID_SIZE_WIDTH_MASK, dwWidth);
            CNXT_SET_VAL(glpDrmVidSize[dwRgnIndex], DRM_VID_SIZE_HEIGHT_MASK, dwHeight);
            CNXT_SET_VAL(glpDrmVidStride[dwRgnIndex], DRM_VID_STRIDE_STRIDE_MASK, pRgn->dwStride * 2);
         }
      }
   }
}

/*****************************************************************************/
/* Function: vidCreateVideoRgnGeneric()                                      */
/*                                                                           */
/* Type: Internal                                                            */
/*                                                                           */
/* Parameters: POSDRECT pRc      - Ptr describing the size of region to      */
/*                                 create or NULL if dimensions are not      */
/*                                 specified.                                */
/*             u_int32 dwSize    - Number of bytes to allocate for region or */
/*                                 0 if dimensions supplied in pRc.          */
/*             VIDRGNTYPE vrType - Pixel type of region, 422 or 420.         */
/*                                                                           */
/* Returns: HVIDRGN - A valid handle to a DRM Video Region on success.       */
/*                    NULL on failure.                                       */
/*                                                                           */
/* Description: Creates a video region and allocates all of the memory needed*/
/*              This function supports both the vidCreateVideoRgn and        */
/*              vidAllocateVideoRgn APIs.                                    */
/*****************************************************************************/
HVIDRGN vidCreateVideoRgnGeneric(POSDRECT pRc, u_int32 dwSize, VIDRGNTYPE vrType)
{
   HVIDRGN hRgn = (HVIDRGN)NULL;
   u_int32 dwHeight, dwWidth, dwStride;
   PVIDRGN pRgn;

   trace_new(TRACE_LEVEL_2 | TRACE_OSD, "OSD: vidCreateVideoRgnGeneric\n");

   /* Are the basic parameters valid? We can allow neither both pRc and  */
   /* dwSize zero nor both non-zero. */
   if((!pRc && (dwSize == 0)) ||
      ( pRc && (dwSize != 0)))
      return((HVIDRGN)NULL);
      
      
   /* Allocate the video region structure. */
   pRgn = (PVIDRGN)mem_malloc(sizeof(VIDRGN));

   trace_new(TRACE_LEVEL_2 | TRACE_OSD, "OSD: video region structure at 0x%08x\n", pRgn);

   if (pRgn)
   {
      /* Two cases are supported here - either we are passed a rectangle */
      /* describing the region size or we are passed a number of bytes */
      /* to allocate. */
      if (pRc)
      {
         /* Calculate the height and width from the supplied rectangle */
         dwHeight = pRc->bottom - pRc->top;
         dwWidth = pRc->right - pRc->left;

         /* Calculate the stride. */
         /* 422 images are 16 bpp.  Stride = Width * 2 */
         /* 420 images are 12 bpp.  Stride = Width */
         dwStride = dwWidth;
         if (vrType == TYPE_422)
            dwStride <<= 1;

         /* Word align the stride. */
         dwStride += 3;
         dwStride &= 0xFFFFFFFC;

         /* Calculate the amount of memory to allocate. */
         dwSize = dwStride * (dwHeight + ((dwHeight + 1) / 2));
      }
      else
      {
         /* We were passed a size only. Clear the dimension values. These */
         /* will be set later using a call to vidSetVideoRgnDimensions. */
         dwHeight = 0;
         dwWidth  = 0;
         dwStride = 0;
      }
      
      /* Allocate the video memory needed for the region. */
      #ifdef OPENTV_12
      pRgn->pBuffer = (u_int8 *)vidImageBufferAlloc(dwSize, &(pRgn->bTransient), FALSE);
      if(pRgn->pBuffer)
        pRgn->pBuffer = (u_int8 *)SET_FLAGS(pRgn->pBuffer, NCR_BASE);
      #else
      pRgn->pBuffer = (u_int8 *)mem_nc_malloc(dwSize);
      #endif
        
      /* TBD this video buffer cannot span over a 16Meg boundary. */

      trace_new(TRACE_LEVEL_2 | TRACE_OSD, "OSD: region pointer is 0x%08x\n", pRgn->pBuffer);

      /* If the memory has been allocated correctly, load the structure. */
      if (pRgn->pBuffer)
      {
         pRgn->dwSize         = dwSize;
         pRgn->dwHeight       = dwHeight;
         pRgn->dwWidth        = dwWidth;
         pRgn->dwStride       = dwStride;
         pRgn->vrType         = vrType;
         pRgn->vrConnection   = SOFTWARE;
         pRgn->arMode         = gArMode;
         pRgn->dwAR           = MPG_VID_ASPECT_43;
         pRgn->dwSrcH         = dwHeight;
         pRgn->dwSrcW         = dwWidth;
         pRgn->dwVScale       = 0x0400;
         pRgn->dwHScale       = 0x0400;
         pRgn->dwPan          = 0;
         pRgn->dwScan         = 0;
         pRgn->dwAutoMode     = VID_AUTO_ALL;
         pRgn->bTop           = TRUE;
         pRgn->iPanRead       = 0;
         pRgn->iScanRead      = 0;
         pRgn->bVectorPresent = FALSE;
         pRgn->dwUserInfo     = 0;
         
         /* Set the source rectangle to the full size of the region. */
         pRgn->rcSrc.top    = 0;
         pRgn->rcSrc.left   = 0;
         pRgn->rcSrc.right  = dwWidth;
         pRgn->rcSrc.bottom = dwHeight;
         pRgn->bSrcRcSpec   = FALSE;

         /* Set the destination rectangle to the full size of the region. */
         if(pRc)
         {
           pRgn->rcDst.top    = pRc->top;
           pRgn->rcDst.left   = pRc->left;
           pRgn->rcDst.right  = pRc->right;
           pRgn->rcDst.bottom = pRc->bottom;
         }
         else
         {
           pRgn->rcDst.top    = 0;
           pRgn->rcDst.left   = 0;
           pRgn->rcDst.right  = dwWidth;
           pRgn->rcDst.bottom = dwHeight;
         }

         /* Add the region to the linked list. */
         pRgn->pPrev = NULL;
         pRgn->pNext = gpVidRgn;
         if (gpVidRgn)
            gpVidRgn->pPrev = pRgn;

         gpVidRgn = pRgn;
         hRgn = (HVIDRGN)pRgn;
      }
      else
      {
         /* Something went wrong.  Houston, we have a problem! */
         /* Free the video region structure and exit in disgrace. */
         trace_new(TRACE_LEVEL_2 | TRACE_OSD, "OSD: Error in region creation\n");
         mem_free(pRgn);
      }
   }

   return hRgn;
}

/*****************************************************************************/
/* Function: vidSetHWBuffSize()                                              */
/*                                                                           */
/* Type: Internal                                                            */
/*                                                                           */
/* Parameters: void                                                          */
/*                                                                           */
/* Returns: void                                                             */
/*                                                                           */
/* Description: This is used to notify the video library that the output     */
/*              screen size is changing and that the default MPG hardware    */
/*              buffer size is changing.                                     */
/*****************************************************************************/
void vidSetHWBuffSize(void)
{
   u_int32 dwIndex = INVALID_VID_INDEX;

   /* Check to see if the buffer is changing size. */
   if (gvrMpg.dwHeight != gnOsdMaxHeight)
   {
      #ifdef NEVER
      /* Scale the source rectangle in terms of the new height. */
      gvrMpg.rcSrc.top = gvrMpg.rcSrc.top * gnOsdMaxHeight / gvrMpg.dwHeight;
      gvrMpg.rcSrc.bottom = gvrMpg.rcSrc.bottom * gnOsdMaxHeight / 
         gvrMpg.dwHeight;

      /* Scale the destination rectangle in terms of the new height. */
      gvrMpg.rcDst.top = gvrMpg.rcDst.top * gnOsdMaxHeight / gvrMpg.dwHeight;
      gvrMpg.rcDst.bottom = gvrMpg.rcDst.bottom * gnOsdMaxHeight / 
         gvrMpg.dwHeight;
      #else
      /* Old case fails if we decode anything that is not the same dimension    */
      /* as the screen before calling this function. We now make the assumption */
      /* that changing the video standard in use will reset the output to full  */
      /* screen.                                                                */
      gvrMpg.rcDst.top = 0;
      gvrMpg.rcDst.bottom = gnOsdMaxHeight;  
      #endif   

      /* Set the new buffer height. */
      gvrMpg.dwHeight = gnOsdMaxHeight;   /* Buffer height. */

      /* Update scaling if needed. */
      if (gpActiveVidRgn[0] == (PVIDRGN)&gvrMpg)
         dwIndex = 0;
      if (gpActiveVidRgn[1] == (PVIDRGN)&gvrMpg)
         dwIndex = 1;

      /* Update the video scaling if the default MPG buffer is visible. */
      if (dwIndex != INVALID_VID_INDEX)
         vidUpdateScaling(dwIndex);
   }
}

#if 1//OPENTV_12

/********************************************************************/
/*  FUNCTION:    vidForceDisplay                                    */
/*                                                                  */
/*  PARAMETERS:  hRgn - region whose buffer is to be displayed      */
/*               bSoftware - TRUE force DRM to display this static  */
/*                                buffer                            */
/*                           FALSE revert to vcore-controlled       */
/*                                display                           */
/*                                                                  */
/*  DESCRIPTION: Similar to vidSetConnection, this call sets either */
/*               software or hardware controlled display of the     */
/*               video region whose handle is passed. Unlike        */
/*               vidSetConnection, it is safe to make this call     */
/*               while motion video is running - it will not stop   */
/*               decode.                                            */
/*                                                                  */
/*  RETURNS:     TRUE if hardware was displaying the video region,  */
/*               FALSE if software had set DRM to display the       */
/*               buffer directly.                                   */
/*                                                                  */
/*  CONTEXT:     Must be called from a non-interrupt context        */
/*                                                                  */
/********************************************************************/
bool  vidForceDisplay(HVIDRGN hRgn, bool bSoftware)
{

   PVIDRGN pRgn;
   u_int32 dwActIndex = INVALID_VID_INDEX;
   u_int32 dwStride;
   bool    ks;
   bool    bRetcode;

   pRgn = (PVIDRGN)hRgn;

   if (pRgn)
   {
      if (gpActiveVidRgn[0] == pRgn)
         dwActIndex = 0;
      else if (gpActiveVidRgn[1] == pRgn)
         dwActIndex = 1;
      else return(FALSE);
         
      bRetcode = CNXT_GET_VAL(glpDrmMpgStillCtrl[dwActIndex], 
          DRM_MPG_VIDEO_PARAM_STILL_ENABLE_MASK) ? FALSE : TRUE;
      
      if (bSoftware)
      {
         /* Create the 16-byte aligned stride for the 420 buffer. */
         dwStride = (pRgn->dwSrcW + 15) & 0xFFFFFFF0;
                  
         ks = critical_section_begin();
        
         CNXT_SET_VAL(glpDrmMpgLumaAddr[dwActIndex], 
             DRM_ADDRESS_ADDRESS_MASK,  (u_int32)pRgn->pBuffer);
         CNXT_SET_VAL(glpDrmMpgChromaAddr[dwActIndex], 
             DRM_ADDRESS_ADDRESS_MASK,  
             (u_int32)pRgn->pBuffer + (dwStride * pRgn->dwSrcH));

         /* Set the aspect ratio to 4:3. */
         CNXT_SET_VAL(glpDrmMpgStillCtrl[dwActIndex], 
             DRM_MPG_VIDEO_PARAM_ASPECT_RATIO_MASK, 0);
         /* Set the field not line to progressive. */
         CNXT_SET_VAL(glpDrmMpgStillCtrl[dwActIndex], 
             DRM_MPG_VIDEO_PARAM_FIELD_NOT_LINE_MASK, 1);
         /* Set the frame not field to frame. */
         CNXT_SET_VAL(glpDrmMpgStillCtrl[dwActIndex], 
             DRM_MPG_VIDEO_PARAM_FRAME_NOT_FIELD_MASK, 1);

         /* Set the image height and width. */
         CNXT_SET_VAL(glpDrmMpgSrcSize[dwActIndex], 
             DRM_SIZE_WIDTH_MASK, dwStride);
         CNXT_SET_VAL(glpDrmMpgSrcSize[dwActIndex], 
             DRM_SIZE_HEIGHT_MASK, pRgn->dwSrcH);
        
         CNXT_SET_VAL(glpDrmMpgStillCtrl[dwActIndex], 
             DRM_MPG_VIDEO_PARAM_STILL_ENABLE_MASK, 1);
        
         critical_section_end(ks);
      }
      else
      {
         ks = critical_section_begin();
        
         CNXT_SET_VAL(glpDrmMpgStillCtrl[dwActIndex], 
             DRM_MPG_VIDEO_PARAM_STILL_ENABLE_MASK, 0);
        
         critical_section_end(ks);
      }   
   }      
   
   return(bRetcode);
}
#endif

/*****************************************************************/
/*****************************************************************/
/**                                                             **/
/**  Functions handling allocation and freeing of image buffers **/
/**  (specific to OpenTV 1.2 just now but may prove useful in   **/
/**  EN2 and other environments later).                         **/
/**                                                             **/
/*****************************************************************/
/*****************************************************************/


#ifdef OPENTV_12

/********************************************************************/
/*  FUNCTION:    vidImageBufferAlloc                                */
/*                                                                  */
/*  PARAMETERS:  dwSize      - the number of bytes to allocate from */
/*                             the video heap for this object.      */
/*               pbTransient - pointer to storage which will be set */
/*                             to indicate whether the object has   */
/*                             been allocated in a permanent or     */
/*                             transient section of the video heap. */
/*               bPermanent  - Force allocation only from permanent */
/*                             video heap and fail call if RAM is   */
/*                             not available here.                  */
/*                                                                  */
/*  DESCRIPTION: This function tries first to allocate a block of   */
/*               memory of the desired size from the fixed video    */
/*               object heap. If this fails and a secondary heap    */
/*               is available, it attempts to allocate from this,   */
/*               transient heap (this memory is overlaid on the     */
/*               motion video I and P decode buffers and will be    */
/*               destroyed when motion video is restarted. It is    */
/*               only used when no motion video is in use). The     */
/*               value of *pbTransient is set to TRUE if the RAM    */
/*               allocated is in the transient part of the heap or  */
/*               FALSE if in the permanent heap.                    */
/*                                                                  */
/*  RETURNS:     Pointer to allocated buffer or NULL if no memory   */
/*               is available in the video/OSD object heap.         */
/*                                                                  */
/*  CONTEXT:     May be called on any non-interrupt context         */
/*                                                                  */
/********************************************************************/
void   *vidImageBufferAlloc(u_int32 dwSize, bool *pbTransient, bool bPermanent)
{
  void *pBuff;
  
  #ifdef DEBUG
  not_interrupt_safe();
  #endif
  
  /* Try to allocate object from the permanent heap */
  pBuff = heap_alloc(ghVidHeap, dwSize);
  
  /* If we go it, return the pointer */
  if (pBuff)
  {
    *pbTransient = FALSE;
  }  
  else
  {
    if (bPermanent)
    {
      /* We have been asked to allocate only from the permanent heap but */
      /* we can't so fail the call.                                      */
      pBuff = NULL;
      trace_new(TRACE_LEVEL_4|TRACE_MPG, "VIDEO: Can't allocate %d bytes in permanent video heap!\n", dwSize);
      *pbTransient = FALSE;
    }
    else
    {
      /* If the B frame heap does not exist, try to create it */
      if(gbBFrameBufferAvailable && !gbTransientHeapExists)
        vidCreateBFrameHeap();
        
      /* Try allocating from the transient heap if it exists */
      if (gbTransientHeapExists)
      {
        #ifdef DEBUG
        if (heap_check(ghVidTransientHeap))
        {
          trace_new(TRACE_LEVEL_ALWAYS|TRACE_MPG,"VIDEO: Transient heap still seems OK\n");
        }  
        else
        {
          trace_new(TRACE_LEVEL_ALWAYS|TRACE_MPG,"VIDEO: ***** Transient heap has been corrupted!! *****\n");
        }  
        #endif /* DEBUG */
        
        pBuff = heap_alloc(ghVidTransientHeap, dwSize);
        if (pBuff == NULL)
        {
          trace_new(TRACE_LEVEL_4|TRACE_MPG, "VIDEO: Can't allocate %d bytes in transient heap!\n", dwSize);
          *pbTransient = FALSE;
          #ifdef DEBUG
          heap_dump(ghVidTransientHeap);
          heap_dump(ghVidHeap);
          #endif /* DEBUG */
        }
        else
        {
          /* We got the memory from the transient heap. Tell the caller */
          *pbTransient = TRUE;
          trace_new(TRACE_LEVEL_2|TRACE_MPG, "VIDEO: Allocated %d (0x%x) bytes in transient heap at 0x%08x\n", dwSize, dwSize, pBuff);
        }  
      }
      else
      {
        /* Transient heap does not exist */
        pBuff = NULL;
        trace_new(TRACE_LEVEL_4|TRACE_MPG, "VIDEO: Transient heap not available. No permanent space for %d bytes\n", dwSize);
        *pbTransient = FALSE;
      } 
    }   
  }
  
  return(pBuff);  
}

/********************************************************************/
/*  FUNCTION:    vidImageBufferFree                                 */
/*                                                                  */
/*  PARAMETERS:  lpBuffer   - Pointer to the image buffer to be     */
/*                            freed.                                */
/*               bTransient - The value passed back in pbTransient  */
/*                            when the buffer was originally        */
/*                            allocated.                            */
/*                                                                  */
/*  DESCRIPTION: Free an image buffer previously allocated using a  */
/*               call to vidImageBufferAlloc. This function takes   */
/*               into account the heap that the buffer was          */
/*               allocated from and, if this was from the transient */
/*               section of the heap, checks to ensure that the     */
/*               heap still exists before freeing the buffer.       */
/*                                                                  */
/*  RETURNS:     TRUE if successful, FALSE if transient heap has    */
/*               been destroyed and object no longer exists.        */
/*                                                                  */
/*  CONTEXT:     May be called on any non-interrupt context         */
/*                                                                  */
/********************************************************************/
bool vidImageBufferFree(void *lpBuffer, bool bTransient)
{
  bool bRetcode;
  
  #ifdef DEBUG
  not_interrupt_safe();
  #endif
  
  if (!bTransient)
  {
    /* Free the buffer from the permanent video/osd heap */
    heap_free(ghVidHeap, lpBuffer);
    bRetcode = TRUE;
  }  
  else
  {
    /* Free the object from the transient heap if it exists */
    if (gbTransientHeapExists)
    {
      trace_new(TRACE_LEVEL_2|TRACE_MPG, "VIDEO: Freeing 0x%08x from transient heap\n", lpBuffer);
      
      #ifdef DEBUG
      if (heap_check(ghVidTransientHeap))
      {
        trace_new(TRACE_LEVEL_ALWAYS|TRACE_MPG,"VIDEO: Transient heap still seems OK\n");
      }  
      else
      {
        trace_new(TRACE_LEVEL_ALWAYS|TRACE_MPG,"VIDEO: ***** Transient heap has been corrupted!! *****\n");
      }  
      #endif /* DEBUG */
      
      heap_free(ghVidTransientHeap, lpBuffer);
      bRetcode = TRUE;
    }
    else
    {
      /* The heap has been destroyed since this object was allocated! */
      trace_new(TRACE_LEVEL_2|TRACE_MPG, "VIDEO: Attempt to free transient object at 0x%x after heap destroyed\n", lpBuffer);
      bRetcode = FALSE;
    }
  } 
  return(bRetcode);
}

/********************************************************************/
/*  FUNCTION:    vidDecodeBuffersAvail                              */
/*                                                                  */
/*  PARAMETERS:  None                                               */
/*                                                                  */
/*  DESCRIPTION: This function is called by the video driver and    */
/*               informs us that motion video is not using the      */
/*               hardware decoded B frame buffer and that this area */
/*               is available for use by other video and OSD        */
/*               objects. In this case, we create a new heap in     */
/*               this area of memory and make it (the transient     */
/*               heap) available to vidImageBufferAlloc.            */
/*                                                                  */
/*  RETURNS:     TRUE if successful, FALSE otherwise                */
/*                                                                  */
/*  CONTEXT:     May be called on any non-interrupt context         */
/*                                                                  */
/********************************************************************/
bool vidDecodeBuffersAvail(void)
{
  #ifdef DEBUG
  not_interrupt_safe();
  #endif

  trace_new(TRACE_LEVEL_2|TRACE_MPG, "VIDEO: Decode buffers available for video/OSD objects\n");
  
  gbBFrameBufferAvailable = TRUE;
  
  return(TRUE);
}

#ifdef OPENTV_12
/********************************************************************/
/*  FUNCTION:    vidCreateBFrameHeap                                */
/*                                                                  */
/*  PARAMETERS:  None                                               */
/*                                                                  */
/*  DESCRIPTION: If we have been told that the video driver is not  */
/*               using motion video, we can reclaim the B frame     */
/*               buffer for use as a general OSD/video heap. This   */
/*               function sets up that heap.                        */
/*                                                                  */
/*  RETURNS:     TRUE if successful, FALSE otherwise                */
/*                                                                  */
/*  CONTEXT:     May be called on any non-interrupt context         */
/*                                                                  */
/********************************************************************/
static bool vidCreateBFrameHeap(void)
{
  #ifdef DEBUG
  not_interrupt_safe();
  #endif
  
  if (gbBFrameBufferAvailable)
  {
    trace_new(TRACE_LEVEL_2|TRACE_MPG, "VIDEO: Setting up B frame memory heap\n");
    
    if (!gbTransientHeapExists)
    {
      /* Create the transient heap in the B decode buffer memory */
      ghVidTransientHeap = heap_create((void *)HWBUF_DEC_B_ADDR, HWBUF_DEC_B_SIZE);
      if (ghVidTransientHeap)
      {
        trace_new(TRACE_LEVEL_2|TRACE_MPG, "VIDEO: Created transient heap at 0x%x\n", ghVidTransientHeap);
        gbTransientHeapExists = TRUE;
        return(TRUE);
      }
      else
      {
        trace_new(TRACE_LEVEL_ALWAYS|TRACE_MPG, "VIDEO: Unable to created transient heap!\n");
        gbTransientHeapExists = FALSE;
        return(FALSE);
      }
    }    
    else
    {
      /* The transient heap already exists! */
      trace_new(TRACE_LEVEL_ALWAYS|TRACE_MPG, "VIDEO: Transient heap already exists!\n");
    
      #ifdef DEBUG
      if (heap_check(ghVidTransientHeap))
      {
        trace_new(TRACE_LEVEL_ALWAYS|TRACE_MPG,"VIDEO: Transient heap still seems OK\n");
      }  
      else
      {
        trace_new(TRACE_LEVEL_ALWAYS|TRACE_MPG,"VIDEO: ***** Transient heap has been corrupted!! *****\n");
      }  
      #endif
    
      return(TRUE);
    }
  }
  else
  {
    trace_new(TRACE_LEVEL_2|TRACE_MPG, "VIDEO: Request to set up  B frame heap when not available!!\n");
    gbTransientHeapExists = FALSE;
    return(FALSE);
  }
}
#endif /* OPENTV_12 */

/********************************************************************/
/*  FUNCTION:    vidDecodeBuffersNeeded                             */
/*                                                                  */
/*  PARAMETERS:  None                                               */
/*                                                                  */
/*  DESCRIPTION: This function is called by the video driver and    */
/*               informs us that the hardware decoded B frame       */
/*               buffer is once again required for motion video.    */
/*               We attempt to relocate all existing objects which  */
/*               are in the transient heap to the permanent heap    */
/*               then destroy the heap before returning. If it is   */
/*               not possible to relocate all objects, we return    */
/*               FALSE to the video driver but it is expected that  */
/*               the buffer will then be overwritten by the hardware*/
/*               and their contents lost.                           */
/*                                                                  */
/*  RETURNS:     TRUE if successful, FALSE otherwise                */
/*                                                                  */
/*  CONTEXT:     May be called on any non-interrupt context         */
/*                                                                  */
/********************************************************************/
bool vidDecodeBuffersNeeded(void)
{
  bool bRetcode;
  bool bVidRelocated;
  bool bOsdRelocated;
  
  #ifdef DEBUG
  not_interrupt_safe();
  #endif
  
  if (gbTransientHeapExists)
  {
    trace_new(TRACE_LEVEL_2|TRACE_MPG, "VIDEO: Decode buffers being reclaimed\n");

    #ifdef DEBUG
    if (heap_check(ghVidTransientHeap))
    {
      trace_new(TRACE_LEVEL_ALWAYS|TRACE_MPG,"VIDEO: Transient heap still seems OK\n");
    }  
    else
    {
      trace_new(TRACE_LEVEL_ALWAYS|TRACE_MPG,"VIDEO: ***** Transient heap has been corrupted!! *****\n");
    }  
    #endif /* DEBUG */
    
    /* Call clients telling them to tidy up any objects in the */
    /* transient heap before we destroy it                     */
    bVidRelocated = vidRelocateTransientRegions();
    bOsdRelocated = osdRelocateTransientRegions();
  
    /* Actually destroy the heap */
    gbTransientHeapExists = FALSE;
    bRetcode = heap_destroy(ghVidTransientHeap);
  
    if (!bRetcode)
    {
      trace_new(TRACE_LEVEL_2|TRACE_MPG, "VIDEO: Failed to destroy transient heap!\n");
    }
  
    /* If the object relocation failed, return FALSE too */
    if(!bVidRelocated || !bOsdRelocated)
      bRetcode = FALSE;
  }
  else
  {
    trace_new(TRACE_LEVEL_ALWAYS|TRACE_MPG, "VIDEO: Request to free transient heap when it doesn't exist!\n");
    bRetcode = TRUE;
  }
    
  gbBFrameBufferAvailable = FALSE;
    
  return(bRetcode);
}

/********************************************************************/
/*  FUNCTION:    vidRelocateTransientRegions                        */
/*                                                                  */
/*  PARAMETERS:  None                                               */
/*                                                                  */
/*  DESCRIPTION: Relocate any video regions currently stored in     */
/*               the transient heap to the permanent heap assuming  */
/*               space is available.                                */
/*                                                                  */
/*  RETURNS:     TRUE if all objects were successfully relocated,   */
/*               FALSE if some could not be relocated.              */
/*                                                                  */
/*  CONTEXT:     Must be called from a non-interrupt context        */
/*                                                                  */
/********************************************************************/
bool vidRelocateTransientRegions(void)
{
  #ifdef DEBUG
  not_interrupt_safe();
  #endif
  
  trace_new(TRACE_LEVEL_2|TRACE_MPG, "VIDEO: Relocating transient memory regions\n");
 
  /* To be completed. For the time being, we just leave any affected */
  /* regions to be trashed by the video hardware when it starts up. */
  
  return(TRUE);
}
#endif


/***** End Internal Functions ************************************************/

/*****************************************************************************/
/* Function:                                                                 */
/*                                                                           */
/* Type:                                                                     */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Returns:                                                                  */
/*                                                                           */
/* Description:                                                              */
/*****************************************************************************/

/****************************************************************************
 * Modifications:
 * $Log: 
 *  105  mpeg      1.104       6/14/04 10:22:29 AM    Dave Wilson     CR(s) 
 *        9391 9392 : vidSetConnection now correctly takes into account the 
 *        aspect ratio of the image when switching to software display. Prior 
 *        to this fix, if you were displaying 16:9 video on a 4:3 screen using 
 *        pan/scan, an attempt to decode a still in the background would cause 
 *        the 16:9 image to shift to the right.
 *  104  mpeg      1.103       4/14/04 10:22:30 AM    Ray Mack        CR(s) 
 *        8855 8856 : Changed logic to have vertical filter turned on any time 
 *        the image is down scaled in vertical direction.
 *  103  mpeg      1.102       4/7/04 4:08:51 PM      Ray Mack        CR(s) 
 *        8798 8801 : Added a test for scaling between 1:1 and 2:1 and set 
 *        vertical lowpass filtering on.  For some reason, old default was 
 *        vertical filter always off.  Now it is only off if upscaling or if 
 *        scaling > 2:1.
 *  102  mpeg      1.101       2/11/04 8:48:51 AM     Steven Jones    CR(s) 
 *        8382 : Repair 14:9 aspect ratio code which became obsolete in its 
 *        time of non-use.
 *  101  mpeg      1.100       2/4/04 2:58:11 PM      Dave Wilson     CR(s) 
 *        8319 8318 : Added a workaround for a Brazos rev B hardware bug which 
 *        causes image plane wipes to show bad chroma in some cases. This code 
 *        is included if the label DRM_NEEDS_SPLIT_FIFO_ON_WIPE is defined as 
 *        YES.
 *        
 *  100  mpeg      1.99        9/29/03 2:42:16 PM     Dave Wilson     SCR(s) 
 *        7571 7570 :
 *        Added code to allow access to new VIDRGN fields holding information 
 *        on
 *        MPEG still image pan/scan vectors.
 *        
 *  99   mpeg      1.98        9/18/03 4:38:58 PM     Dave Wilson     SCR(s) 
 *        7450 7449 :
 *        Changed vidUpdate420Scaling to only centre a scaled image in the 
 *        window if
 *        auto pan/scan is enabled. This is slightly questionable since the 
 *        behaviour
 *        in this case is really something that the network needs to specify 
 *        but...
 *        
 *  98   mpeg      1.97        7/18/03 2:36:56 PM     Dave Wilson     SCR(s) 
 *        5530 :
 *        Reworked vidUpdate420Scaling to make it clearer and to enable manual
 *        pan/scan vectors (which were inadvertantly broken in a previous 
 *        edit).
 *        
 *  97   mpeg      1.96        6/6/03 1:08:48 PM      Dave Wilson     SCR(s) 
 *        6659 :
 *        Removed all // comments.
 *        Made vidSetVideoRgnDimensions return an error if either the height or
 *         width 
 *        passed is 0.
 *        
 *  96   mpeg      1.95        5/20/03 3:13:30 PM     Dave Wilson     SCR(s) 
 *        6485 6486 6487 :
 *        Ensured that DRM chroma/luma wipe workaround code is no longer used 
 *        if run
 *        on Brazos Rev B (where the bug is fixed).
 *        
 *  95   mpeg      1.94        4/23/03 11:10:28 AM    Dave Wilson     SCR(s) 
 *        5862 5863 :
 *        Changed initialisation of glpDrmMpgTileWipe registers to use correct 
 *        format
 *        for old and new style layout (layout changed for Brazos rev B).
 *        
 *  94   mpeg      1.93        2/19/03 10:53:02 AM    Dave Wilson     SCR(s) 
 *        5528 5543 :
 *        Used 2 new chip features to build in workarounds for BSkyB stills 
 *        test
 *        problems with video wipe. New features are DRM_HASWIPE_CHROMA_BUG and
 *        DRM_HAS_UPSCALED_PAL_STILL_BUG.
 *        
 *  93   mpeg      1.92        2/13/03 12:16:48 PM    Matt Korte      SCR(s) 
 *        5479 :
 *        Removed old header reference
 *        
 *  92   mpeg      1.91        9/25/02 10:09:00 PM    Carroll Vance   SCR(s) 
 *        3786 :
 *        Removing old DRM and AUD conditional bitfield code.
 *        
 *        
 *  91   mpeg      1.90        9/19/02 5:01:14 PM     Dave Wilson     SCR(s) 
 *        4381 :
 *        Corrected handling of VO_IMAGE_ON_TOP. The value passed to 
 *        vidSetOptions
 *        for this case is now used to determine whether the video region 
 *        should be 
 *        brought to the top or sent to the back. Sending 0 puts the plane to 
 *        the back
 *        whereas sending anything else pulls it to the top.
 *        
 *  90   mpeg      1.89        9/16/02 3:29:18 PM     Dave Wilson     SCR(s) 
 *        4567 :
 *        Change of image size now also fixes up the current scale factors and 
 *        source
 *        rectangle if auto-scaling is disabled. This was needed for some Sky 
 *        apps after
 *        the Avago scaling fix to OTVVID12.C
 *        
 *  89   mpeg      1.88        9/6/02 4:21:44 PM      Matt Korte      SCR(s) 
 *        4498 :
 *        Fixed Warnings
 *        
 *  88   mpeg      1.87        7/15/02 5:27:56 PM     Dave Wilson     SCR(s) 
 *        4086 :
 *        Changed vidWipeEnable to ensure that it doesn't change the state of 
 *        the DRM
 *        PAL/NTSC format bit if motion video is being played. The previous 
 *        code caused
 *        video to be very jerky on exit from the Sky News Active app on BSkyB.
 *        
 *  87   mpeg      1.86        6/26/02 8:40:36 AM     Dave Wilson     SCR(s): 
 *        4088 
 *        Special scaling case code in vidUpdate420Scaling was rounding down 
 *        the size
 *        of very small stills to 0 which caused crud to be seen rather than 
 *        the image.
 *        This was seen especially clearly in cases where the small black still
 *         is
 *        decoded on startup. Added another check to ensure that this does not 
 *        happen.
 *        
 *  86   mpeg      1.85        6/21/02 1:52:00 PM     Dave Wilson     SCR(s) 
 *        4061 :
 *        Added a special case to ensure that the DRM scaler problem (where 
 *        faint
 *        vertical lines are introduced when some particular scale factors are 
 *        used)
 *        is not seen. Code in vidUpdate420Scaling now checks for offending 
 *        scale
 *        factors and tweaks them slightly if found to move them out of the 
 *        range which
 *        causes the problem.
 *        
 *  85   mpeg      1.84        6/12/02 7:03:44 PM     Carroll Vance   SCR(s) 
 *        3786 :
 *        Removed DRM bitfields.
 *        
 *        
 *  84   mpeg      1.83        6/12/02 12:24:54 PM    Carroll Vance   SCR(s) 
 *        3786 :
 *        Removed DRM bitfields.
 *        
 *  83   mpeg      1.82        6/10/02 2:54:24 PM     Craig Dry       SCR(s) 
 *        3923 :
 *        Remove MPG bitfields unconditionally - Step 2
 *        
 *  82   mpeg      1.81        6/6/02 5:52:14 PM      Craig Dry       SCR(s) 
 *        3923 :
 *        Remove MPG bitfields conditionally - Step 1
 *        
 *  81   mpeg      1.80        5/28/02 6:18:18 PM     Carroll Vance   SCR(s) 
 *        3786 :
 *        Removed DRM bitfields.
 *        
 *  80   mpeg      1.79        5/21/02 12:48:40 PM    Dave Wilson     SCR(s) 
 *        3609 :
 *        Tidied up vidUpdate420Scaling to ensure that all register writes are 
 *        carried
 *        out in a block within a critical section.
 *        Added new function vidPrepareMpgForAspectHandlingChange which sets 
 *        only the
 *        MPEG decoder registers which affect panscan/letterbox handling.
 *        
 *  79   mpeg      1.78        5/14/02 3:50:52 PM     Dave Wilson     SCR(s) 
 *        3777 :
 *        Recent fix to workaround MPEG upscaling problems in PAL mode resulted
 *         in a
 *        break in still to still wipes (chroma was misaligned - basically a 
 *        previous
 *        fix was nullified by the new one). Reworked vidWipeEnable to correct 
 *        the case
 *        and allow both the upscaling fix and the vipe chroma fix to work 
 *        correctly.
 *        
 *  78   mpeg      1.77        4/5/02 11:47:32 AM     Tim White       SCR(s) 
 *        3510 :
 *        Backout DCS #3468
 *        
 *        
 *  77   mpeg      1.76        3/28/02 2:46:34 PM     Quillian Rutherford 
 *        SCR(s) 3468 :
 *        Removed bit fields
 *        
 *        
 *  76   mpeg      1.75        3/20/02 4:58:04 PM     Dave Wilson     SCR(s) 
 *        3416 :
 *        Removed a few compiler warnings.
 *        
 *  75   mpeg      1.74        2/25/02 11:18:18 AM    Dave Wilson     SCR(s) 
 *        3241 :
 *        vidSetHWBuffSize used to mess the video scaling up unless an image 
 *        the 
 *        same size as the OSD buffer had been decoded prior to it being 
 *        called. We now
 *        assume that changing the video standard (the only thing that causes 
 *        the function
 *        to be called today) will result in the video window being reset to 
 *        the full
 *        screen and this cures the problem.
 *        
 *  74   mpeg      1.73        2/22/02 10:46:46 AM    Dave Wilson     SCR(s) 
 *        3199 :
 *        Added a call to gen_video_decode_blank inside the initialisation 
 *        function
 *        to make sure that the video and DRM are initialised with a black 
 *        image just
 *        before the video plane is enabled. This prevents nasty screenfulls of
 *         crud
 *        being seen after initialisation and before any video has been 
 *        decoded.
 *        
 *  73   mpeg      1.72        1/8/02 4:15:58 PM      Dave Wilson     SCR(s) 
 *        2993 :
 *        vidWipeEnable now subtracts 1 from each of the Y increment fields to 
 *        prevent
 *        a chroma problem when displaying NTSC. Hardware folks are still 
 *        investigating
 *        why this is required but it cures the problem (visually) so I'm 
 *        puting this
 *        fix back.
 *        
 *  72   mpeg      1.71        12/18/01 5:32:00 PM    Quillian Rutherford 
 *        SCR(s) 2933 :
 *        Merged wabash code
 *        
 *  71   mpeg      1.70        11/26/01 11:18:34 AM   Dave Wilson     SCR(s) 
 *        2912 2913 :
 *        Fix provided by Vendor D: Turn off auto scaling or auto panscan if 
 *        the 
 *        client sets explicit scale factors or panscan vector.
 *        
 *  70   mpeg      1.69        11/5/01 12:33:44 PM    Dave Wilson     SCR(s) 
 *        2834 2835 :
 *        Setting auto scaling now resets the horizontal and vertical scale 
 *        factors
 *        to the default values. This is done since we have seen at least one 
 *        app on
 *        the BSkyB network that turns auto-scaling off then does not set the 
 *        scale
 *        factors to be used. Without this change, it would use whatever 
 *        scaling was
 *        set up before the app started resulting in intermittent scaling 
 *        weirdnesses.
 *        
 *  69   mpeg      1.68        10/5/01 4:51:00 PM     Dave Wilson     SCR(s) 
 *        2719 2720 :
 *        Removed previous workaround for Sky Active (channel 269) video 
 *        scaling
 *        problem and fixed root cause. OTV video driver was miscalculating 
 *        scale
 *        factors since the dwWidth and dwHeight fields of the video region 
 *        were not
 *        being update on an MPEG size change - only the dwSrcH and dwSrcW 
 *        fields got
 *        changed.
 *        
 *  68   mpeg      1.67        10/5/01 4:00:00 PM     Dave Wilson     SCR(s) 
 *        2721 2722 :
 *        Replaced a trace_new in vidUpdate420Scaling with isr_trace_new which 
 *        is 
 *        interrupt-safe.
 *        
 *  67   mpeg      1.66        10/5/01 1:06:36 PM     Dave Wilson     SCR(s) 
 *        2688 2689 :
 *        
 *        vidUpdate420Scaling now includes a workaround for a Sky video scaling
 *        problem. If scaling into a window with auto scaling disabled and the 
 *        end 
 *        result will be to display an image smaller than the window, turn auto
 *         scaling
 *        back on and ensure that the window is filled. This works for the 
 *        problems we
 *        have seen in Sky Active but is likely to be an interim fix until we 
 *        hear from
 *        Sky what the actual required behaviour is in these cases.
 *        
 *  66   mpeg      1.65        9/10/01 8:45:02 AM     Dave Wilson     SCR(s) 
 *        2626 2627 :
 *        Apparently the ST box needs to fix up source Y coordinate to a 
 *        multiple of
 *        16 when auto-scaling into a window. Adding this assumption to the 
 *        code makes the video and graphics
 *        align correctly in the Open... HSBC application without breaking 
 *        anything else
 *        that uses windoed video (or, at least, anything else I ran).
 *        
 *  65   mpeg      1.64        8/21/01 1:55:36 PM     Dave Wilson     SCR(s) 
 *        2504 2505 :
 *        Added vidGetVideoRegion function to return handle to global video 
 *        region.
 *        
 *  64   mpeg      1.63        8/9/01 1:49:06 PM      Dave Wilson     SCR(s) 
 *        2311 2312 :
 *        Removed gvrVideoStill and pointed ghVideoStill to the gvrMpeg 
 *        structure.
 *        I can see no reason to have 2 different structures used here and, 
 *        before this,
 *        we had problems in OpenTV 1.2 where some window parameters were set 
 *        in the 
 *        wrong structure resulting in incorrect video position and size.
 *        
 *  63   mpeg      1.62        7/10/01 6:36:46 PM     Dave Wilson     SCR(s) 
 *        2238 2237 :
 *        Changes to support cases where video of width (2n+1)*8 is decoded. 
 *        Vcore
 *        reports size of original image but we need to round the stride up to 
 *        the next
 *        16-byte boundary.
 *        
 *  62   mpeg      1.61        7/9/01 11:28:08 AM     Dave Wilson     SCR(s) 
 *        2196 2197 :
 *        Previous code was enabling automatic display of the video plane at 
 *        the end of video play rather than waiting until the image was decoded
 *         into the plane. As a result, you could see some crap before the 
 *        decode completed. Now the switch from manual to
 *        automatic display is done when the still is finished decoding.
 *        
 *  61   mpeg      1.60        7/5/01 2:06:34 PM      Dave Wilson     SCR(s) 
 *        2194 2195 :
 *        Added bool return code to vidForceDisplay to allow caller to 
 *        determine whether vcore or direct DRM programming was controlling 
 *        display of the video region when the call is made.
 *        
 *  60   mpeg      1.59        7/3/01 4:46:24 PM      Dave Wilson     SCR(s) 
 *        2190 2191 :
 *        Changed vidForceDisplay so that it doesn't look at the DRM still 
 *        enable bit
 *        before deciding to do something. When using this function to force an
 *         update
 *        with no incoming video, this was causing the region to be left in DRM
 *         display
 *        mode rather than reverting to hardware control by the MPEG core.
 *        
 *  59   mpeg      1.58        6/29/01 6:06:40 PM     Dave Wilson     SCR(s) 
 *        2002 1914 2003 1915 1916 :
 *        Changed OSD MPEG interrupt handling so that UpdateMpgScaling is only 
 *        called
 *        when required rather than on every decode interrupt 5.
 *        Additional changes to reduce/remove some transients in Open.. app 
 *        startup and shutdown.
 *        
 *  58   mpeg      1.57        6/28/01 3:06:08 PM     Dave Wilson     SCR(s) 
 *        2105 2106 :
 *        Various video transient fixes. Code modified now that the video 
 *        microcode has been fixed to correctly report the buffer being 
 *        displayed.
 *        
 *  57   mpeg      1.56        6/11/01 10:32:14 AM    Dave Wilson     SCR(s) 
 *        2066 2067 :
 *        Added vidForceDisplay call to allow us to force DRM to display a 
 *        given
 *        bufferwithout making any video core buffer changes.
 *        
 *  56   mpeg      1.55        5/31/01 6:50:44 PM     Dave Wilson     SCR(s) 
 *        1897 1898 :
 *        Hardware/microcode bugs mean that we can't assume the displayed 
 *        buffer
 *        based upon the last decoded anchor frame after all. New code assumes 
 *        that 
 *        we always display the P buffer when stopped (all display stills are 
 *        decoded
 *        here and all non-display are decoded into the I buffer).
 *        
 *  55   mpeg      1.54        4/25/01 6:38:08 AM     Dave Wilson     DCS775: 
 *        Reduced Vendor A video heap size back to 0x220000 bytes
 *        
 *  54   mpeg      1.53        4/25/01 5:36:24 AM     Dave Wilson     DCS1772: 
 *        B frame memory heap is now only created when an allocation from
 *        the main video heap fails. This prevents some of the magenta flashes 
 *        that
 *        were seen when the heap was created whenever video was stopped.
 *        
 *  53   mpeg      1.52        4/23/01 6:06:06 PM     Steve Glennon   SCR 1124:
 *        [SG] - made calls to heap_dump conditional on #ifdef debug
 *        This was breaking the release build where heap_dump gets compiled 
 *        out.
 *        
 *        
 *  52   mpeg      1.51        4/20/01 12:11:28 PM    Dave Wilson     DCS1124: 
 *        Major changes to video memory management to get Sky Text app running
 *        
 *  51   mpeg      1.50        4/11/01 1:58:10 PM     Steve Glennon   DCS1675 
 *        (put by DW): Moved code that writes the I,P & B buffer addresses
 *        to the hardware into the VIDEO driver so that it is taken care of on 
 *        the 
 *        main video task.
 *        
 *  50   mpeg      1.49        3/29/01 3:29:46 PM     Steve Glennon   Fix for 
 *        DCS#1499 - sky news app wrong video position if 4:3 letterbox
 *        Fixed vidSetOptions so that it only resets the pan/scan vectors if it
 *         gets 
 *        a new aspect ratio mode AND the aspect ratio mode is different to 
 *        previous.
 *        I was resetting the pan scan vectors every time the mode was 
 *        LETTERBOX
 *        before.
 *        
 *  49   mpeg      1.48        3/29/01 11:11:38 AM    Dave Wilson     DCS1495: 
 *        Removed infinite loops polling MPEG core CmdValid bit. Now call a 
 *        central function in VIDEO to send commands to the MPG decoder.
 *        
 *  48   mpeg      1.47        2/22/01 2:02:30 PM     QA - Roger Taylor Moved 
 *        definition of FULL_SCALE_VIDEO to out of the #ifdef OPENTV_12
 *        as it is now used in general case rather than hard coded 0x400
 *        Continued part of fix to DCS#1278/1279. Fixed a compile problem for 
 *        OPENTV_12 not being defined (eg for NDSTESTS and/or EN2)
 *        
 *  47   mpeg      1.46        2/22/01 1:15:16 AM     Steve Glennon   Major 
 *        changes to recognize that source rectangle is redundant if 
 *        destination
 *        and scale factor are known.
 *        Built more of the scaling knowledge into vidUpdate420Scaling
 *        so that it can respond to resolution changes on decode complete.
 *        Part of a fix for DCS#1278 in conjunction with otvvid12.c r 1.46
 *        and vidprvc.h r 1.6
 *        
 *  46   mpeg      1.45        2/13/01 5:33:12 PM     Steve Glennon   Fix for 
 *        DCS#1207 - video scaled incorrectly in automode.
 *        Changed to default to entire source rectangle for automode rather 
 *        than the 
 *        funky "source is in terms of destination" scaling
 *        
 *  45   mpeg      1.44        2/12/01 8:45:04 PM     Steve Glennon   Modified 
 *        vidSetOptions to reset the source rectangle to the whole image 
 *        if the VO_IMAGE_HEIGHT or VO_IMAGE_WIDTH is modified (actually checks
 *         that
 *        the new value is different from previous, or all heck breaks loose).
 *        THis is to fix DCS#1193. Previous fixes had caused the bahavior to be
 *         
 *        wrong in certain stills subtests (subtests 1,2,9) with images wrapped
 *        horizontally and incorrectly positioned vertically with green/purple 
 *        junk
 *        below them for the vertical case.
 *        This only happened when the previous tests were run sequentially 
 *        first.
 *        THe cause was that we were not being told the image dimensions, and 
 *        only
 *        discovered them on the MPEG decode ISR - by then the source rectangle
 *         was 
 *        messed up. Now, if the image size changes, we reset the source 
 *        rectangle and
 *        that fixes the case in question.
 *        
 *  44   mpeg      1.43        2/12/01 2:56:28 PM     Quillian Rutherford Add 
 *        vidSetRegionInput and changed vidSetActiveRegion to call it
 *        
 *  43   mpeg      1.42        2/9/01 12:47:38 PM     Steve Glennon   Fix for 
 *        DCS#1151 - made centering conditional on Automode
 *        code changed in vidUpdate420Scaling
 *        
 *  42   mpeg      1.41        2/8/01 6:19:42 PM      Steve Glennon   Fix for 
 *        DCS #1014
 *        Also fix for DCS #1145
 *        Limit scaling to 0.5, 1.0 and 2.0 for OTV 1.2 only to match existing 
 *        BSkyB 
 *        box behavior.
 *        Fixed vidUpdate420Scaling to check scaling and correct
 *        Fixed vidSetOptions to massage vertical scaling to match one of the 
 *        allowed
 *        scale factors.
 *        
 *        NOTE: Requirement is to limit vertical scale factors ONLY!!!
 *        
 *  41   mpeg      1.40        2/5/01 2:23:32 PM      Dave Wilson     DCS1126: 
 *        Backed out change done last week to cure green line problem at
 *        bottom of some video windows. This broke all cases where the video 
 *        window
 *        was intended to be taller than the last decoded MPEG still object.
 *        
 *  40   mpeg      1.39        2/1/01 5:36:06 PM      Steve Glennon   Fix for 
 *        DCS#1065 - Pan Scan not working correctly
 *        Failure was that in stills test, numbers were not being reset when 
 *        you
 *        exited one still and went back into it in subtest 7
 *        Fix was to reset the pan and scan vectors when the ARMODE is set to 
 *        OSD_ARM_LETTERBOX
 *        
 *  39   mpeg      1.38        2/1/01 9:50:14 AM      Quillian Rutherford Fixed
 *         comparison bug that would occur if using 422 hardware with no 
 *        scaling
 *        
 *  38   mpeg      1.37        1/29/01 5:06:50 PM     Dave Wilson     
 *        DCS1061/1062: Added code to remove green line in cases where video 
 *        windows
 *        are placed on odd-Y boundaries.
 *        
 *  37   mpeg      1.36        1/18/01 4:34:14 PM     Lucy C Allevato DCS 876 
 *        Fix for pan/scan not working. Moved pan/scan setup code in
 *        vidUpdate420Scaling to below the changes for clipping and centering 
 *        so
 *        that the pan/scan adjustments would apply.
 *        
 *  36   mpeg      1.35        1/9/01 4:35:56 PM      Lucy C Allevato Fix for 
 *        DCS 874 Still Test 2 MPEG 3/4 clipping delta for x=540, y=144
 *        and x=180, y=432 cases. vidUpdate420Scaling changed to center the
 *        image when the selected input src x and width exceed the decoded 
 *        buffer
 *        width and clipping is needed for both the horizontal and vertical 
 *        cases.
 *        
 *  35   mpeg      1.34        12/11/00 10:21:40 AM   Miles Bintz     changed 
 *        #include "heap.h" to #include "cnxtheap.h"
 *        
 *  34   mpeg      1.33        11/24/00 7:45:26 PM    Lucy C Allevato Added 
 *        code to support dwHScale and dwVScale parameters when doing manual
 *        scaling to scale properly when the scale gets set before any decode 
 *        occurs
 *        on the video plane.
 *        
 *  33   mpeg      1.32        11/20/00 2:57:28 PM    Dave Wilson     Increased
 *         video heap size to 0x220000 bytes since this allows all the 
 *        combination tests to run without problems.
 *        
 *  32   mpeg      1.31        11/16/00 1:41:32 PM    Dave Wilson     Now uses 
 *        a separate heap for all image allocation.
 *        
 *  31   mpeg      1.30        11/15/00 3:22:32 PM    Dave Wilson     Changed 
 *        memory management under OpenTV 1.2 to use a separate heap for the 
 *        video region image buffers.
 *        
 *  30   mpeg      1.29        11/13/00 6:39:38 PM    Lucy C Allevato Added 
 *        code change to work around hardware chroma wipe bug in
 *        vidUpdate420Scaling.
 *        
 *  29   mpeg      1.28        11/8/00 5:18:14 PM     Lucy C Allevato Changed 
 *        vidUpdate420Scaling to use DRM letterbox scaling only unless
 *        DECODER_LETTERBOX is defined. This is currently not used because 
 *        changing
 *        between pan/scan and letterbox on the fly can cause problems in the
 *        decoder(a hang) unless the decoder is paused first. This solution 
 *        allows
 *        on the fly changes to occur.
 *        
 *  28   mpeg      1.27        10/8/00 3:35:52 PM     Lucy C Allevato Added a 
 *        video region structure to handle attaching/mirroring a video
 *        region on more than one plane. Added a copy region settings function 
 *        to
 *        support this. Removed the setting of StillEnable in vidSetConnection 
 *        when
 *        switching back to a hardware connection. This can be done other ways 
 *        and
 *        causes an artifact when switching from still no display to hardware.
 *        
 *  27   mpeg      1.26        10/3/00 6:20:26 PM     Dave Wilson     Aspect 
 *        ratio handling changes to get SYSCVID VTS test working
 *        
 *  26   mpeg      1.25        9/26/00 5:59:10 PM     Lucy C Allevato Added a 
 *        fix for pan and scan limits for starting x and y when doing
 *        manual pan and scan.
 *        
 *  25   mpeg      1.24        9/25/00 5:11:08 PM     Lucy C Allevato Changed 
 *        vidSetPos to use buffer width and height of source region if passed
 *        a NULL rectangle for the src. Added initial code to support pan and 
 *        scan
 *        vectors in the 420 scaling code.
 *        
 *  24   mpeg      1.23        9/19/00 11:02:24 PM    Lucy C Allevato Switched 
 *        rectangle setup in vidSetVideoRgnDimensions back to the given
 *        width and height for defaults. This can cause scaling weirdness if 
 *        rectangles
 *        set bigger than buffer width or height. Really only makes sense for 
 *        the
 *        global mpeg decode buffer to use rectangles that are full screen size
 *         by
 *        default.
 *        
 *  23   mpeg      1.22        9/15/00 4:19:06 PM     Lucy C Allevato Changed 
 *        to using new commented version of vidUpdate420Scaling. Added init
 *        of gvrVideoStill and ghVideoStill for using the B frame buffer for
 *        decoding stills in no display mode.
 *        
 *  22   mpeg      1.21        8/31/00 5:28:36 PM     Lucy C Allevato Changed 
 *        vidSetVideoRgnDimensions to initialize the src and dst rectangles to
 *        be full screen values.
 *        
 *  21   mpeg      1.20        8/28/00 6:12:56 PM     Lucy C Allevato Fixes for
 *         strict level compiler checking for possible usage of variables
 *        before they are initialized.
 *        
 *  20   mpeg      1.19        8/8/00 8:08:04 PM      Lucy C Allevato Replaced 
 *        original vidUpdate420Scaling to fix downscaling problem
 *        in demo until it can be determined what is causing the problem in the
 *        cleaned up version.
 *        
 *  19   mpeg      1.18        8/3/00 11:58:42 AM     Lucy C Allevato Ignore 
 *        aspect ratio considerations if not VID_AUTO_ALL. Added initialization
 *        of Pan/Scan and AutoMode when region created.
 *        
 *  18   mpeg      1.17        7/12/00 12:00:50 PM    Lucy C Allevato Added 
 *        video options for pan vector, scan vector, and automode to set/get
 *        options. Added commented version and slightly reorganized version of
 *        Update420Scaling.
 *        
 *  17   mpeg      1.16        5/22/00 10:47:26 AM    Dave Wilson     Changed 
 *        vidSetActiveRegion to replace any region currently using the 
 *        hardware required. Previously the call failed if some other region 
 *        was using
 *        the hardware.
 *        Added a workaround in vidSetWipeRegion to correct for a 2 line 
 *        positioning
 *        error in Colorado Rev A.
 *        
 *  16   mpeg      1.15        5/15/00 8:38:40 PM     Dave Wilson     Fixed 
 *        vidQueryVideoRgnSize - YUV 4:2:0 IS 12bpp and not 8bpp.
 *        
 *  15   mpeg      1.14        5/15/00 5:50:00 PM     Dave Wilson     Fixed a 
 *        pointer which was not being correctly initialised
 *        
 *  14   mpeg      1.13        5/3/00 10:52:56 AM     Rob Tilton      Fixed the
 *         return code for set pos.
 *        
 *  13   mpeg      1.12        4/28/00 6:20:46 PM     Rob Tilton      The 420 
 *        scaling was selecting the wrong register.
 *        
 *  12   mpeg      1.11        4/26/00 5:22:58 PM     Dave Wilson     Corrected
 *         field ordering for stills (spec error)
 *        Still planes are now created at the position specified rather than 
 *        top left
 *        
 *  11   mpeg      1.10        4/18/00 11:53:08 AM    Dave Wilson     Changed 
 *        calls to mem_nc_malloc/free after removal of an unused parameter
 *        
 *  10   mpeg      1.9         4/14/00 10:28:26 AM    Rob Tilton      Added 
 *        ghvrMpg.
 *        
 *  9    mpeg      1.8         4/13/00 5:49:34 PM     Rob Tilton      Changed 
 *        how the mpg decode buffers are set.
 *        
 *  8    mpeg      1.7         3/22/00 4:11:34 PM     Rob Tilton      Added 
 *        video wipe functions.
 *        
 *  7    mpeg      1.6         3/20/00 2:59:36 PM     Rob Tilton      Corrected
 *         memory allocation and stride for 420 regions.  Added DRM DWI 
 *        Control 3 reg.
 *        
 *  6    mpeg      1.5         3/2/00 4:30:54 PM      Rob Tilton      Added 
 *        Get/Set Alpha.
 *        
 *  5    mpeg      1.4         2/9/00 3:42:10 PM      Rob Tilton      Added 
 *        internal fucntion vidSetHWBuffSize().
 *        
 *  4    mpeg      1.3         2/9/00 1:18:46 PM      Rob Tilton      Added 
 *        vidSetActiveRegion().
 *        
 *  3    mpeg      1.2         2/1/00 5:31:40 PM      Dave Wilson     Changes 
 *        for OpenTV 1.2 support
 *        
 *  2    mpeg      1.1         1/6/00 1:25:46 PM      Rob Tilton      Corrected
 *         the init sequence and disabled DRM error interrupt in the scale
 *        update.
 *        
 *  1    mpeg      1.0         1/5/00 10:24:14 AM     Rob Tilton      
 * $
 * 
 *****************************************************************************/

