/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*           Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002         */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       osdisrc.c
 *
 *
 * Description:    OSD Interrupt Handling Functions
 *
 *
 * Author:         Rob Tilton with major modifications by many others
 *
 ****************************************************************************/
/* $Header: osdisrc.c, 45, 5/14/04 9:49:53 AM, Xin Golden$
 ****************************************************************************/
#include "stbcfg.h"
#include "basetype.h"
#define INCL_DRM
#define INCL_MPG
#include "kal.h"
#include "osdlib.h"
#include "osdprv.h"
#include "osdisrc.h"
#include "globals.h"
#include "retcodes.h"
#include "vidlibc.h"
#include "video.h"

/* Must be set both here and in VIDEO.C */
#if (defined OPENTV_12) || (ENABLE_DEFERRED_VIDEO_UNBLANK == YES)
#define DEFERRED_UNBLANK
#endif

/* The TTX_TESTING define is used only with OSDTEST for testing TTX.*/
/*#define TTX_TESTING*/

/* The DRM_LINE_INT_TESTING define is used only with OSDTEST for testing 
 the line interrupts.*/
/*#define DRM_LINE_INT_TESTING*/

#ifdef TTX_TESTING
extern void SetTTXAddress(void);
#endif /* TTX_TESTING */

#ifdef DRM_LINE_INT_TESTING
extern void LineIntNotify(void);
#endif /* DRM_LINE_INT_TESTING */

PFNISR gpfnOsdChain = NULL;
PFNISR gpfnMpgChain = NULL;

extern u_int32 gdwSrcWidth;
extern u_int32 gdwSrcHeight;
extern u_int32 gdwAR;

static bool gbMpgScale;
static u_int32 gdwMpgScaleDelay;
static u_int32 gdwMpgScaleCount;

#ifdef DEBUG
sem_id_t semidMPGSizeChange;
#endif
queue_id_t  gquidLineNotify;
u_int32     glinenotifymsg[4] = {0, 0, 0, 0};

PLINECALLBACK gpLineList = NULL; /*  List of callbacks for the line int API.*/
u_int32 gdwTTXIntLine;
u_int32 gnFieldCount;
u_int32 gdwDrmIsrFlags;
PFNOSDISR gpfnIFrameNotify = (PFNOSDISR)NULL;
PFNOSDISR gpfnARNotify     = (PFNOSDISR)NULL;
PFNOSDISR gpfnSizeNotify   = (PFNOSDISR)NULL;
u_int32 gdwLastARNotified = 0xFFFFFFFF;     /* Initialize to unused AR value */
u_int32 gdwLastWidthNotified = 0;           /* Initialize to invalid width   */
u_int32 gdwLastHeightNotified = 0;          /* Initialize to invalid height  */
bool    gbDeferredEnable = FALSE;

/* Shadow copies of the video decoder I, P and B decode buffer pointers */
/* defined in video.c                                                   */
extern u_int8 *gpIBuffer;
extern u_int8 *gpPBuffer;
extern u_int8 *gpBBuffer;
extern u_int8 *gpVideoDisplayBuffer;
extern bool    gbPendingOSDLLUpdate[2]; /* Update to linked list pending (per plane) */


extern void gen_video_frame_displayed_signal(u_int32 uBufferPtr, int iBufferIndex);

#ifdef DEFERRED_UNBLANK
extern void gen_video_unblank_internal(void);
extern MPEG_buffer_image gsDecodedImages[];
extern bool              gbDeferredUnblank;
#endif

#define OSD_MPG_ISR_FLAGS    (MPG_IMAGE_RES_CHANGE | MPG_ASPECT_RATIO_CHANGE | MPG_VIDEO_DEC_INTERRUPT5)
#ifdef DRM_LINE_INT_TESTING
#define OSD_DRM_ISR_FLAGS    (DRM_STATUS_LAST_PIXEL | DRM_STATUS_DISPLAY_LINE_ACTIVE)
#else /* DRM_LINE_INT_TESTING */
#define OSD_DRM_ISR_FLAGS    (DRM_STATUS_LAST_PIXEL)
#endif /* DRM_LINE_INT_TESTING */

/******************************************************************************
 SAR Test is used to verify that the SAR register can be cleared and used.
 The test code has been left in, but SAR_TEST_CODE must be defined.
#define SAR_TEST_CODE*/
#ifdef SAR_TEST_CODE
#undef OSD_DRM_ISR_FLAGS
#define OSD_DRM_ISR_FLAGS    (DRM_STATUS_LAST_PIXEL)
typedef enum {
   SAR_ENABLE = 0,
   SAR_DISABLE,
   SAR_READ
} SAR_STATE;

#define SAR_CLEAR                    0xFFFFFFFF

SAR_STATE gSarState;
#endif /* SAR_TEST_CODE */

/*****************************************************************************/
/* Function: OSDIsrInit()                                                    */
/*                                                                           */
/* Parameters: void                                                          */
/*                                                                           */
/* Returns: void                                                             */
/*                                                                           */
/* Description: Initializes the OSD interrupt service routine. This is called*/
/*              from the OSDInit function.                                   */
/*****************************************************************************/
void OSDIsrInit(void)
{
   int nRC;
   bool ksPrev;

   ksPrev = critical_section_begin();
   CNXT_SET_VAL(glpCtrl0, MPG_CONTROL0_ENABLEINT_MASK, 1);             /* Enable the MPG interrupts*/
   critical_section_end(ksPrev);

  /* These may have been set already so DO NOT CLEAR THEM HERE!
   gpfnIFrameNotify = NULL;    Function ptr for I-Frame decode complete.
   gpfnARNotify = NULL;        Function ptr for aspect ratio change.*/

   CNXT_SET_VAL(glpDrmControl, DRM_CONTROL_PROG_LINE_INT_MASK, 100);
     /* generic number for testing*/
   CNXT_SET_VAL(glpDrmControl, DRM_CONTROL_PROG_FIELD_INT_MASK, 0);
   CNXT_SET_VAL(glpDrmControl, DRM_CONTROL_PROG_LINE_FIELD_ENABLE_MASK, 0);

   gbMpgScale = FALSE;
   gdwMpgScaleDelay = 2;

#ifdef SAR_TEST_CODE
   glpDrmSar->Sar = SAR_CLEAR;
   gSarState = SAR_ENABLE;
#endif /* SAR_TEST_CODE */

   gdwDrmIsrFlags = OSD_DRM_ISR_FLAGS;
   *(LPREG)glpDrmInterrupt = gdwDrmIsrFlags;
   gquidLineNotify = (queue_id_t)NULL;

   #ifdef DEBUG
   /*  Create a semaphore to signal the MPG AR/Size change task in debug builds*/
   semidMPGSizeChange = sem_create(1, "OSD1");
   task_create(MPGSizeARChange, (void *)NULL, (void *)NULL,
            OSD2_TASK_STACK_SIZE, OSD2_TASK_PRIORITY, OSD2_TASK_NAME);
   #endif         

   /* Register the MPEG interrupt to look for aspect ratio or size changes.*/
   nRC = int_register_isr(INT_MPEGSUB, (PFNISR)OSDIsr, FALSE, FALSE,
       &gpfnMpgChain);
   if (RC_OK == nRC)
   {
      nRC = int_enable(INT_MPEGSUB);
      if (RC_OK != nRC)
      {
         trace_new(OSD_ERROR_MSG, "int_enable(INT_MPEGSUB) returned ");
         switch (nRC)
         {
         case RC_KAL_INVALID:
            trace_new(OSD_ERROR_MSG, "RC_KAL_INVALID\n");
            break;
         case RC_KAL_NOTHOOKED:
            trace_new(OSD_ERROR_MSG, "RC_KAL_NOTHOOKED\n");
            break;
         }
      }
      else
      {
         /* Enable MPG resolution aspect ratio change interrupt*/
         ksPrev = critical_section_begin();
         *glpIntMask |= OSD_MPG_ISR_FLAGS; /*(MPG_IMAGE_RES_CHANGE | MPG_ASPECT_RATIO_CHANGE);*/
         critical_section_end(ksPrev);
      }
   }
   else
   {
      trace_new(OSD_ERROR_MSG, "int_register_isr(INT_MPEGSUB, ...) returned ");
      switch (nRC)
      {
      case RC_KAL_INVALID:
         trace_new(OSD_ERROR_MSG, "RC_KAL_INVALID\n");
         break;
      case RC_KAL_BADPTR:
         trace_new(OSD_ERROR_MSG, "RC_KAL_BADPTR\n");
         break;
      case RC_KAL_PRIVATE:
         trace_new(OSD_ERROR_MSG, "RC_KAL_PRIVATE\n");
         break;
      }
   }

   /* Register the DRM interrup to look for error messages.*/
   nRC = int_register_isr(INT_DRM, (PFNISR)OSDIsr, FALSE, FALSE, &gpfnOsdChain);
   if (RC_OK == nRC)
   {
      nRC = int_enable(INT_DRM);
      if (RC_OK != nRC)
      {
         trace_new(OSD_ERROR_MSG, "int_enable(INT_DRM) returned ");
         switch (nRC)
         {
         case RC_KAL_INVALID:
            trace_new(OSD_ERROR_MSG, "RC_KAL_INVALID\n");
            break;
         case RC_KAL_NOTHOOKED:
            trace_new(OSD_ERROR_MSG, "RC_KAL_NOTHOOKED\n");
            break;
         }
      }
      else
      {
         *(LPREG)glpIntStatus |= DRM_STATUS_DISPLAY_LINE_ACTIVE |
                                     DRM_STATUS_LAST_PIXEL |
                                     DRM_STATUS_FIRST_FIELD |
                                     DRM_STATUS_DRM_ERROR;
      }
   }
   else
   {
      trace_new(OSD_ERROR_MSG, "int_register_isr(INT_DRM, ...) returned ");
      switch (nRC)
      {
      case RC_KAL_INVALID:
         trace_new(OSD_ERROR_MSG, "RC_KAL_INVALID\n");
         break;
      case RC_KAL_BADPTR:
         trace_new(OSD_ERROR_MSG, "RC_KAL_BADPTR\n");
         break;
      case RC_KAL_PRIVATE:
         trace_new(OSD_ERROR_MSG, "RC_KAL_PRIVATE\n");
         break;
      }
   }
}

/*****************************************************************************/
/* Function: OSDIsr()                                                        */
/*                                                                           */
/* Parameters: u_int32 dwIntID  - Interrupt that the ISR is being asked to   */
/*                                service.                                   */
/*             bool    bFIQ     - If TRUE, the routine is running as a result*/
/*                                of a FIQ, else an IRQ.                     */
/*             PFNISR* pfnChain - A pointer to storage for any ISR to be     */
/*                                called after this function completes.      */
/*                                                                           */
/* Returns: int - RC_ISR_HANDLED - Interrupt fully handled by this routine.  */
/*                                 Do not chain.                             */
/*                RC_ISR_NOTHANDLED - Interrupt not handled by this function.*/
/*                                    KAL should chain to the function whose */
/*                                    pointer is stored in pfnChain          */
/*                                                                           */
/* Description:  Interrupt service routine for the OSD driver.               */
/*****************************************************************************/
int OSDIsr(u_int32 dwIntID, bool bFIQ, PFNISR *pfnChain)
{
   int nRet = RC_ISR_HANDLED;
   u_int32 nStatus;
   u_int32 nEnable;
   u_int32 uLastFrame;
   u_int32 dwLastWidth;
   u_int32 dwLastHeight;
   u_int32 dwLastAR;
   static bool bNotifyAspectChange = FALSE;
   #ifdef SAR_TEST_CODE
   u_int32 nField;
   u_int32 nSAR;
   u_int32 nSARClear;
   #endif /* SAR_TEST_CODE */
   int giDecodedIndex;

   isr_trace_new(OSD_ISR_MSG, "->OSDIsr\n",0,0);

   switch (dwIntID)
   {
   case INT_MPEGSUB:
      nStatus = *(LPREG)glpIntStatus;
      if (nStatus & OSD_MPG_ISR_FLAGS)
      {
         /**************************************************************************/
         /* Handle Video Core Interrupts relating to image size or DRM interaction */
         /**************************************************************************/
         
         /* Clear the flags we will process during this interrupt */
         *(LPREG)glpIntStatus = (nStatus & OSD_MPG_ISR_FLAGS); 

         /* The MPEG image size has changed */
         if (nStatus & MPG_IMAGE_RES_CHANGE)
         {
            /* In BSkyB Open apps, some video is not a multiple of 16 high   */
            /* Although not valid MPEG, we need to deal with it.             */
            /* Bottom 3 bits of height are used to indicate which buffer is  */
            /* being displayed so we need to mask this off. As a result, we  */
            /* cannot deal with any case where the height is not a multiple  */
            /* of 8.                                                         */
            /* Width is also problematic as video ucode puts other info in   */
            /* bottom 3 bits of width (Top field first, I/P/B decode etc)    */
            
            dwLastWidth  = gdwSrcWidth;
            dwLastHeight = gdwSrcHeight;
          
            gdwSrcWidth = CNXT_GET_VAL(glpMpgPicSize, MPG_VID_SIZE_WIDTH_MASK) & 0xFFFFFFF8;
            gdwSrcHeight = CNXT_GET_VAL(glpMpgPicSize, MPG_VID_SIZE_HEIGHT_MASK) & 0xFFFFFFF8;
            
            isr_trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: MPG size changed to 0x%x by 0x%x.\n", gdwSrcWidth, gdwSrcHeight);
            
            gbMpgScale = TRUE;
            gdwMpgScaleCount = gdwMpgScaleDelay;

            isr_trace_new(OSD_ISR_MSG, "OSD_ISR:Handling MPG Res Change.\n", 0, 0);
         }
         
         /* An image aspect ratio change has occurred */
         
         if (nStatus & MPG_ASPECT_RATIO_CHANGE)
         {
            dwLastAR = gdwAR;
            
            gdwAR = CNXT_GET_VAL(glpMpgPicSize, MPG_VID_SIZE_ASPECTRATIO_MASK);
            if (gdwAR != dwLastAR)
            {
              isr_trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: MPG AR changed to 0x%x.\n", gdwAR, 0);
              
              bNotifyAspectChange = TRUE;
              
              /* NB: Strictly speaking, this should be done on the next interrupt 5     */
              /*     but, for some unknown reason, the pan scan offset portion of       */
              /*     the DRM offset/width register seems to take longer to take effect  */
              /*     than the other registers. If we rescale during int 5, we sometimes */
              /*     see a "bounce" when changing from a 4:3 to a 16:9 channel with     */
              /*     letterbox disabled.                                                */
              UpdateMpgScaling();
            }  

            isr_trace_new(OSD_ISR_MSG, "OSD_ISR:Handling MPG AR Change.\n", 0, 0);
         }
         
         /* This interrupt indicates that the VCore has sent a display command to */
         /* the DRM                                                               */
         if (nStatus & MPG_VIDEO_DEC_INTERRUPT5)
         {
            /* If required, set the region input back to hardware. This used to be    */
            /* done at the end of video_play but this caused the display address to   */
            /* swap in some cases resulting in decodes of possibly incorrectly scaled */
            /* images into the visible buffer.                                        */
            if (gbDeferredEnable)
            {
              isr_trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: Setting video region input during Int 5\n", 0, 0);
              vidSetRegionInput(vidGet420Hardware(),0);
              gbDeferredEnable = FALSE;
            }  

            /* Used in deferred MPEG size/aspect ratio updates */
            osdMpegInt5Received();

            /* Perform immediate mpg scaling if needed.*/
            if (gbMpgScale && (gdwMpgScaleCount == 0))
            {  /* Perform the scaling now.*/
            
               isr_trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: Updating DRM video scaling.\n", 0, 0);
              
               UpdateMpgScaling();
               
               /* Do we need to send a notification as a result of a size change? */
               if (gpfnARNotify) 
               {
                  if ((gdwAR        != gdwLastARNotified) ||
                      (gdwSrcWidth  != gdwLastWidthNotified) ||
                      (gdwSrcHeight != gdwLastHeightNotified))
                  {
                    bNotifyAspectChange = TRUE;
                  }
               }
               gbMpgScale = FALSE;
            }

            /* Send a notification to a registered aspect ratio/size callback if the aspect */
            /* ratio and/or stream dimensions have changed.                                 */
            if(bNotifyAspectChange && gpfnARNotify)
            {
              (gpfnARNotify)(gdwAR, (gdwSrcHeight << 16) | gdwSrcWidth);
            
              gdwLastARNotified     = gdwAR;
              gdwLastWidthNotified  = gdwSrcWidth;
              gdwLastHeightNotified = gdwSrcHeight;
              
              bNotifyAspectChange = FALSE;
            }
            
            /* Notify the IFrame callback of a complete I frame.
              The bottom 2 bits of the MPEG width is used to store flags telling us which
              picture type has just been decoded.*/
            uLastFrame = CNXT_GET(glpMpgPicSize, MPG_VID_SIZE_WIDTH_MASK) & MPG_VID_MASK_COMPLETE;
            if (gpfnIFrameNotify)
               (gpfnIFrameNotify)(uLastFrame, 
                  (uLastFrame == MPG_VID_I_COMPLETE) ? (int)gpIBuffer : (int)gpPBuffer);

            /* Keep track of the buffer that the DRM is being told to display. */
            #if MPG_SYNC_BIT_POSITION == MPG_SYNC_BIT_POSITION_COLORADO
            switch (CNXT_GET(glpMpgPicSize, MPG_VID_MASK_DISPLAYED))
            #else
            switch (CNXT_GET(glpMpgAddrExt, MPG_VID_MASK_DISPLAYED))
            #endif
            {
              case MPG_VID_I_DISPLAYED:       gpVideoDisplayBuffer = gpIBuffer; break;
              case MPG_VID_P_DISPLAYED:       gpVideoDisplayBuffer = gpPBuffer; break;
              case MPG_VID_B_FRAME_DISPLAYED: 
              case MPG_VID_B_FIELD_DISPLAYED: gpVideoDisplayBuffer = gpBBuffer; break;
            }  

            /* Which buffer is the vcore currently trying to display? */
            switch ((u_int32)gpVideoDisplayBuffer)
            {
              case HWBUF_DEC_P_ADDR:
                giDecodedIndex = DEC_P_BUFFER_INDEX;
                break;
            
              case HWBUF_DEC_I_ADDR:
                giDecodedIndex = DEC_I_BUFFER_INDEX;
                break;
            
              default:
                giDecodedIndex = -1;
                break;
            }      
                
            /* Let the video driver know that we got an INT5 */    
            gen_video_frame_displayed_signal((u_int32)gpVideoDisplayBuffer, giDecodedIndex);
            isr_trace_new(OSD_ISR_MSG, "OSD_ISR:Handling MPG Interrupt 5.\n", 0, 0);
         }

         #ifdef DEBUG
         /* This is only used for debug messages.*/
         if (((nStatus & MPG_ASPECT_RATIO_CHANGE) || 
            (nStatus & MPG_IMAGE_RES_CHANGE)) && 
            (semidMPGSizeChange))
            sem_put(semidMPGSizeChange);
         #endif   

         if ((*(LPREG)glpIntStatus) && ((*pfnChain = gpfnMpgChain)!=0))
             nRet = RC_ISR_NOTHANDLED;
      }
      else if ((*pfnChain = gpfnMpgChain)!=0)
      {
         isr_trace_new(OSD_ISR_MSG, "Not Handling MPG Interrupt\n",0,0);
         nRet = RC_ISR_NOTHANDLED;
      }
      break;
   case INT_DRM:
      nStatus = *(LPREG)glpDrmStatus;
      nEnable = *(LPREG)glpDrmInterrupt;
      *(LPREG)glpDrmStatus = gdwDrmIsrFlags | 0x0F000000; /* Clear my flags*/
      /**(LPREG)glpDrmStatus = 0x0F000000; Clear these error bits that don't
                                            do anything.  Neches problem.*/
      if (nStatus & 0x0F000000)
      {
         isr_trace_new(OSD_ISR_MSG, "DRM Error bits set 0x%08X\n", 
            nStatus & 0x0F000000, 0);
      }
      if (nStatus & nEnable)
      {
#ifdef SAR_TEST_CODE
         /* This needs to come from nStatus;*/
         nField = glpDrmStatus->Field;
#endif /* SAR_TEST_CODE */

         isr_trace_new(OSD_ISR_MSG, "Handling DRM Interrupt\n",0,0);
         if (nStatus & DRM_STATUS_LAST_PIXEL & nEnable)
         {
            isr_trace_new(OSD_ISR_MSG, "DRM_STATUS_LAST_PIXEL\n",0,0);

            /* advance field count and indicate any pending */
            /* OSD linked list updates have become active   */
            gnFieldCount ++;
            gbPendingOSDLLUpdate[0] = FALSE;
            gbPendingOSDLLUpdate[1] = FALSE;

/* Do this on MPEG interrupt 5 since this is timed to ensure that the update 
   will not occur too early.

             Perform delayed mpg scaling.
            if (gbMpgScale)
            {
               if (gdwMpgScaleCount != 0)
                  gdwMpgScaleCount --;

               if (gdwMpgScaleCount == 0)
               {
                  isr_trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: Updating DRM video scaling in retrace.\n", 0, 0);
                  
                  UpdateMpgScaling();
                  if (gpfnARNotify) Notify the AR callback of the change.
                  if (gdwAR != gdwLastARNotified)
                  {
                     (gpfnARNotify)(gdwAR, (gdwSrcHeight << 16) | gdwSrcWidth);
                     gdwLastARNotified = gdwAR;
                  }
                  gbMpgScale = FALSE;
               }
            }*/
#ifdef TTX_TESTING
            SetTTXAddress();
#endif /* TTX_TESTING */
#ifdef SAR_TEST_CODE
            switch (gSarState)
            {
            case SAR_ENABLE:
               if (nField == 0)
               {
                  glpDrmControl->SAREnable = 1;
                  gSarState = SAR_DISABLE;
               }
               break;
            case SAR_DISABLE:
               if (nField == 0)
               {
                  glpDrmControl->SAREnable = 0;
                  gSarState = SAR_READ;
               }
               break;
            case SAR_READ:
               if (nField == 1)
               {
                  nSAR = glpDrmSar->Sar;
                  isr_trace_new(OSD_ISR_MSG, "SAR Read 0x%08X\n", nSAR, 0);
                  glpDrmSar->Sar = SAR_CLEAR;
                  nSARClear = glpDrmSar->Sar;
                  isr_trace_new(OSD_ISR_MSG, "SAR Read 0x%08X\n", nSARClear, 0);
                  gSarState = SAR_ENABLE;
               }
               break;
            }
#endif /* SAR_TEST_CODE */
         }

         if (nStatus & DRM_STATUS_DRM_ERROR & nEnable)
         {
            *(LPREG)glpDrmStatus = 0x0F000000;
            isr_trace_new(OSD_ISR_MSG, "DRM_STATUS_DRM_ERROR 0x%08X\n", nStatus, 0);
         }

         if (nStatus & DRM_STATUS_DISPLAY_LINE_ACTIVE & nEnable)
         {
            isr_trace_new(OSD_ISR_MSG, "DRM_STATUS_DISPLAY_LINE_ACTIVE\n",0,0);
            osdLineISR(CNXT_GET_VAL(glpDrmControl, DRM_CONTROL_PROG_LINE_INT_MASK), 
               (OSDFIELD)(CNXT_GET_VAL(glpDrmStatus, DRM_STATUS_FIELD_MASK) ? BOTTOM : TOP));
#ifdef DRM_LINE_INT_TESTING
            LineIntNotify();
#endif /* DRM_LINE_INT_TESTING */
         }

         if (nStatus & DRM_STATUS_FIRST_FIELD & nEnable)
            isr_trace_new(OSD_ISR_MSG, "DRM_STATUS_FIRST_FIELD\n",0,0);

         if ((*(LPREG)glpDrmStatus) && ((*pfnChain = gpfnOsdChain)!=0))
            nRet = RC_ISR_NOTHANDLED;
      }
      else if ((*pfnChain = gpfnOsdChain)!=0)
      {
         isr_trace_new(OSD_ISR_MSG, "Not Handling DRM Interrupt\n",0,0);
         nRet = RC_ISR_NOTHANDLED;
      }
      break;
   }

   isr_trace_new(OSD_ISR_MSG, "<-OSDIsr\n",0,0);
   return nRet;
}

#ifdef DEBUG
/*****************************************************************************/
/* Function: MPGSizeARChange()                                               */
/*                                                                           */
/* Parameters: void* pParam - Pointer passed from process_create()           */
/*                                                                           */
/* Returns: void                                                             */
/*                                                                           */
/* Description: MPG Size / Aspect Ratio change process.                      */
/*****************************************************************************/
void MPGSizeARChange(void *pParam)
{
/*   u_int32 dwSrcWidth, dwSrcHeight, dwDstWidth, dwDstHeight, dwAR;*/
/*   bool bPanScan, bLetterBox;*/

   while (1)
   {
      /* Wait for the semaphore to signal an Aspect Ratio or size change*/
      trace_new(OSD_MSG, "Waiting for MPG Size or AR change\n");
      sem_get(semidMPGSizeChange, KAL_WAIT_FOREVER);
      trace_new(OSD_MSG, "MPG Size or AR change detected\n");

      trace_new(OSD_MSG, "MPG Width = %d\n", gdwSrcWidth);
      trace_new(OSD_MSG, "MPG Height = %d\n", gdwSrcHeight);
      trace_new(OSD_MSG, "MPG AR = %d\n", gdwAR);
   }
}
#endif

/*****************************************************************************/
/* Function: osdLineISR()                                                    */
/*                                                                           */
/* Parameters: u_int32 dwLine - Line number for interrupt.                   */
/*             OSDFIELD oField - Indicating which field is being displayed.  */
/*                                                                           */
/* Returns: void                                                             */
/*                                                                           */
/* Description: Line interrupt service routine.  Any registered line callback*/
/*              is notified from here.  The interrupt line number is updated */
/*              in the DRM Control Register.                                 */
/*****************************************************************************/
void osdLineISR(u_int32 dwLine, OSDFIELD oField)
{
   PLINECALLBACK pList = gpLineList;
   u_int32 dwNextLine = dwLine;

   /* Walk through the linked list of callbacks and perform notifications.*/
   while (pList)
   {
      /* When the lines match, update the next line and check for notification.*/
      if (pList->dwLine == dwLine)
      {
         /* Perform notification.*/
         if ((pList->pfnCallback) &&
            ((pList->oField == BOTH) || (pList->oField == oField)))
            (pList->pfnCallback)(dwLine, oField);

         if (pList->pNext)
            dwNextLine = pList->pNext->dwLine;
         else
            dwNextLine = gpLineList->dwLine;
      }

      pList = pList->pNext;
   }

   /* Don't let the line int be set past the end of the screen.*/
   if (dwNextLine >= CNXT_GET_VAL(glpDrmScreenEnd, 
      DRM_SCREEN_END_FIELD1_LAST_LINE_MASK))
   {
      /* Since the list is ordered in ascending order, grab the first line*/
      /* number from the list.*/
      if (gpLineList)
      {
         if (gpLineList->dwLine < CNXT_GET_VAL(glpDrmScreenEnd, 
            DRM_SCREEN_END_FIELD1_LAST_LINE_MASK))
         {
            dwNextLine = gpLineList->dwLine;
         }
         else
         {
            /* Really strange if this happens, every line in the list is past*/
            /* the end of the displayable lines.*/
            dwNextLine = CNXT_GET_VAL(glpDrmScreenEnd, 
               DRM_SCREEN_END_FIELD1_LAST_LINE_MASK) - 1;
         }
      }
   }

   /* Set the Line number for the interrupt.*/
   CNXT_SET_VAL(glpDrmControl, DRM_CONTROL_PROG_LINE_INT_MASK, dwNextLine);
}

/*****************************************************************************/
/* Function: osdRegisterLineISR()                                            */
/*                                                                           */
/* Parameters: POSDLINEISR pfnCallback - Ptr to OSDLINEISR callback funtion. */
/*             u_int32 dwLine - Line number for interrupt.                   */
/*             OSDFIELD oSignal - Flag indicating on which field to notify   */
/*                                the callback. TOP, BOTTOM, or BOTH         */
/*                                                                           */
/* Returns: bool - TRUE on success.                                          */
/*                                                                           */
/* Description: Registers a callback function which is called at interrupt   */
/*              time when the requested scan line is displayed. Since the    */
/*              callback function is called in an interrupt context, it is   */
/*              expected that the callback function be used solely for       */
/*              notification or signalling and that it returns as quickly as */
/*              possible.                                                    */
/*                                                                           */
/*              The display line numbers are field based. Each field starts  */
/*              the display line counter at 0.                               */
/*****************************************************************************/
bool osdRegisterLineISR(POSDLINEISR pfnCallback, 
                        u_int32 dwLine, 
                        OSDFIELD oSignal)
{
   bool bRet = FALSE;
   PLINECALLBACK pLine;
   PLINECALLBACK pList, pInsert;

   if ((pfnCallback) && (dwLine < CNXT_GET_VAL(glpDrmScreenEnd, 
      DRM_SCREEN_END_FIELD1_LAST_LINE_MASK)))
   {
      /* Allocate the memory for the line callback structure.*/
      pLine = (PLINECALLBACK)mem_malloc(sizeof(LINECALLBACK));
      if (pLine)
      {
         /* Fill in the line callback structure.*/
         pLine->dwLine = dwLine;
         pLine->pfnCallback = pfnCallback;
         pLine->oField = oSignal;

         /* Insert the callback into the list.*/
         pList = gpLineList;
         pInsert = NULL;
         while (pList)
         {
            if (pList->dwLine < dwLine)
            {
               pInsert = pList; /* Insert after this item.*/
               pList = pList->pNext;
            }
            else
            {
               pList = NULL;
            }
         }

         if (pInsert)
         {
            /* Insert the item in the middle or end of the table.*/
            pLine->pNext = pInsert->pNext;
            pLine->pPrev = pInsert;
            if (pInsert->pNext)
               pInsert->pNext->pPrev = pLine;
            pInsert->pNext = pLine;
         }
         else
         {
            /* Insert the item at the beginning of the table.*/
            pLine->pNext = gpLineList;
            pLine->pPrev = NULL;
            if (gpLineList)
               gpLineList->pPrev = pLine;
            gpLineList = pLine;
            CNXT_SET_VAL(glpDrmControl, DRM_CONTROL_PROG_LINE_INT_MASK, dwLine);
         }

         /* Enable the interrupt.*/
         gdwDrmIsrFlags |= DRM_STATUS_DISPLAY_LINE_ACTIVE;
         *(LPREG)glpDrmInterrupt = gdwDrmIsrFlags;

         bRet = TRUE;
      }
   }

   return bRet;
}

/*****************************************************************************/
/* Function: osdUnRegisterLineISR()                                          */
/*                                                                           */
/* Parameters: POSDLINEISR pfnCallback - Ptr to OSDLINEISR callback funtion. */
/*             u_int32 dwLine - Line number for interrupt.                   */
/*                                                                           */
/* Returns: void                                                             */
/*                                                                           */
/* Description: Unregisters the callback function which was registered with  */
/*              osdRegisterLineISR. The callback function, once it is        */
/*              unregistered, will no longer be notified when the requested  */
/*              display line is active.                                      */
/*****************************************************************************/
void osdUnRegisterLineISR(POSDLINEISR pfnCallback, u_int32 dwLine)
{
   PLINECALLBACK pList = gpLineList;

   /* Walk through the linked list of line callbacks and find a match.*/
   while (pList)
   {
      /* If we have a match, remove it from the list and delete the line item.*/
      if ((pList->pfnCallback == pfnCallback) && (pList->dwLine == dwLine))
      {
         if (pList->pPrev)
            pList->pPrev->pNext = pList->pNext;
         else
            gpLineList = pList->pNext;

         if (pList->pNext)
            pList->pNext->pPrev = pList->pPrev;

         mem_free(pList);

         pList = NULL;
      }
      else
        pList = pList->pNext;
   }

   /* If there are no more items in the line list, disable the line int.*/
   if (gpLineList == NULL)
   {
      /* Clear the line int mask.*/
      gdwDrmIsrFlags &= ~DRM_STATUS_DISPLAY_LINE_ACTIVE;

      /* Disable the interrupt.*/
      *(LPREG)glpDrmInterrupt = gdwDrmIsrFlags;
   }
}

/*****************************************************************************/
/* Function: osdSetMpgScaleDelay()                                           */
/*                                                                           */
/* Parameters: u_int32 dwDelay - MPG delay for scaling in field times.       */
/*                                                                           */
/* Returns: bool - TRUE on success.                                          */
/*                                                                           */
/* Description: Sets the delay used to sync the DRM scaling with the decoded */
/*              mpg image.                                                   */
/*****************************************************************************/
bool osdSetMpgScaleDelay(u_int32 dwDelay)
{
   gdwMpgScaleDelay = dwDelay;

   return TRUE;
}

/*****************************************************************************/
/* Function: osdGetMpgScaleDelay()                                           */
/*                                                                           */
/* Parameters: void                                                          */
/*                                                                           */
/* Returns: u_int32 - The delay value used to sync the scaling of the mpg.   */
/*                                                                           */
/* Description: Returns the delay value used to sync the scaling of the mpg. */
/*****************************************************************************/
u_int32 osdGetMpgScaleDelay(void)
{
   return gdwMpgScaleDelay;
}

/*****************************************************************************/
/* Function: osdRegIFrameComplete()                                          */
/*                                                                           */
/* Parameters: PFNOSDISR pfnCallback - Ptr to OSD ISR callback funtion.      */
/*                                                                           */
/* Returns: bool - TRUE on success.  FALSE if there is already a callback    */
/*                 registered.                                               */
/*                                                                           */
/* Description: Registers a callback function which is called at interrupt   */
/*              time when the video decoder has completed decoding an I or P */
/*              frame.  Our decoded I and P frames are the same.  It is      */
/*              expected that the callback function be used solely for       */
/*              notification or signalling and that it returns as quickly as */
/*              possible.                                                    */
/*                                                                           */
/*              The callback is unregistered by setting pfnCallback to NULL. */
/*****************************************************************************/
bool osdRegIFrameComplete(PFNOSDISR pfnCallback)
{
   bool bRet = FALSE;

   if ((gpfnIFrameNotify == NULL) || (pfnCallback == NULL))
   {
      gpfnIFrameNotify = pfnCallback;
      bRet = TRUE;
   }

   return bRet;
}

/*****************************************************************************/
/* Function: osdRegAspectRatio()                                             */
/*                                                                           */
/* Parameters: PFNOSDISR pfnCallback - Ptr to OSD ISR callback funtion.      */
/*                                                                           */
/* Returns: bool - TRUE on success.  FALSE if there is already a callback    */
/*                 registered.                                               */
/*                                                                           */
/* Description: Registers a callback function which is called at interrupt   */
/*              time when the video decoder has received a different image   */
/*              size or aspect ratio.                                        */
/*                                                                           */
/*              It is expected that the callback function be used solely for */
/*              notification or signalling and that it returns as quickly as */
/*              possible.                                                    */
/*                                                                           */
/*              The callback is unregistered by setting pfnCallback to NULL. */
/*****************************************************************************/
bool osdRegAspectRatio(PFNOSDISR pfnCallback)
{
   bool bRet = FALSE;

   if ((gpfnARNotify == NULL) || (pfnCallback == NULL))
   {
      gpfnARNotify = pfnCallback;
      bRet = TRUE;
   }

   return bRet;
}

/*****************************************************************************/
/* Function: osdResetAspectRatioCallback                                     */
/*                                                                           */
/* Parameters: None                                                          */
/*                                                                           */
/* Returns:    Nothing                                                       */
/*                                                                           */
/* Description: The aspect ratio and size change callback is only made when  */
/*              the ISR notices that the size or aspect ratio has changed    */
/*              since the last callback was made. In some applications,      */
/*              however, it may be helpful to be able to reset this check    */
/*              to ensure that a new callback is received on, for example,   */
/*              each channel change. This API allows an application to       */
/*              reset the check and receive a new callback the next time     */
/*              the video driver receives any aspect ratio or size change.   */
/*                                                                           */
/*****************************************************************************/
void osdResetAspectRatioCallback(void)
{
  bool ks;
  
  ks = critical_section_begin();
  gdwLastARNotified = 0xFFFFFFFF;
  gdwLastHeightNotified = 0;
  gdwLastWidthNotified  = 0;
  
  critical_section_end(ks);
}

/****************************************************************************
 * Modifications:
 * $Log: 
 *  45   mpeg      1.44        5/14/04 9:49:53 AM     Xin Golden      CR(s) 
 *        9200 9201 : checked in the changes made by SteveG so osdlib doesn't 
 *        wait for vertical retrace for every change in the OSD position.
 *  44   mpeg      1.43        2/3/04 11:10:30 AM     Dave Wilson     CR(s) 
 *        8265 : Reworked MPEG interrupt 5 handling. Rather than doing a bunch 
 *        of video-specific work in the OSD driver, the handler now calls back 
 *        to the video driver to do this.
 *        
 *  43   mpeg      1.42        1/26/04 3:40:23 PM     Mark Thissen    CR(s) 
 *        8269 8270 : added new global bool, gbDRMDisplayPicture, for fast 
 *        channel change.
 *        
 *  42   mpeg      1.41        2/13/03 12:17:02 PM    Matt Korte      SCR(s) 
 *        5479 :
 *        Removed old header reference
 *        
 *  41   mpeg      1.40        2/11/03 1:33:14 PM     Dave Wilson     SCR(s) 
 *        5137 :
 *        Added code to make use of new ENABLE_DEFERRED_VIDEO_UNBLANK config 
 *        option.
 *        
 *  40   mpeg      1.39        1/27/03 3:26:04 PM     Dave Wilson     SCR(s) 
 *        5320 :
 *        Switched the order of 2 headers to ensure OSDPRV.H would include 
 *        without
 *        generating errors.
 *        
 *  39   mpeg      1.38        12/4/02 12:55:42 PM    Lucy C Allevato SCR(s) 
 *        5065 :
 *        Wrapped register used to access the frame being displayed 
 *        conditionally for Wabash (glpMpgAddrExt) and for Colorado 
 *        (glpMpgPicSize) in OSDIsr function.
 *        Modified all // comments.
 *        
 *  38   mpeg      1.37        9/25/02 10:04:48 PM    Carroll Vance   SCR(s) 
 *        3786 :
 *        Removing old DRM and AUD conditional bitfield code.
 *        
 *        
 *  37   mpeg      1.36        9/6/02 4:21:26 PM      Matt Korte      SCR(s) 
 *        4498 :
 *        Fixed Warnings
 *        
 *  36   mpeg      1.35        7/15/02 5:31:30 PM     Dave Wilson     SCR(s) 
 *        3627 :
 *        Tidied up the usage of the various fields in the MPG_VID_SIZE_REG 
 *        where
 *        flags are stored in the low bits of the height and width definitions.
 *         Although
 *        I had seen no particular problems, the wrong masks were being used in
 *         various
 *        operations so I made sure that these were correct.
 *        
 *  35   mpeg      1.34        6/10/02 2:54:08 PM     Craig Dry       SCR(s) 
 *        3923 :
 *        Remove MPG bitfields unconditionally - Step 2
 *        
 *  34   mpeg      1.33        6/6/02 5:51:56 PM      Craig Dry       SCR(s) 
 *        3923 :
 *        Remove MPG bitfields conditionally - Step 1
 *        
 *  33   mpeg      1.32        5/30/02 1:13:14 PM     Dave Wilson     SCR(s) 
 *        2914 :
 *        Previous code would not call the aspect change/size change 
 *        notification 
 *        callback if the aspect ratio of the stream changed but the size did 
 *        not. This
 *        has been rectified.
 *        
 *  32   mpeg      1.31        5/21/02 1:44:46 PM     Carroll Vance   SCR(s) 
 *        3786 :
 *        Removed DRM bitfields.
 *        
 *  31   mpeg      1.30        5/21/02 12:49:14 PM    Dave Wilson     SCR(s) 
 *        3609 :
 *        Added callback from Int 5 handler to OSDLIBC to allow deferred MPG 
 *        and DRM
 *        updates during panscan/letterbox switching.
 *        
 *  30   mpeg      1.29        5/15/02 12:41:40 PM    Dave Wilson     SCR(s) 
 *        3788 :
 *        Added API osdResetAspectRatioCallback to allow an app to force a new 
 *        callback
 *        on the next size or aspect ratio change regardless of whether the new
 *         size or
 *        aspect ratio is the same as the last notified.
 *        Corrected the aspect ratio callback behaviour to make the callback if
 *         the
 *        aspect ratio or picture dimensions change. In the previous case, the 
 *        callback
 *        would not be made if the picture size changed but the aspect ratio 
 *        remained the
 *        same.
 *        
 *  29   mpeg      1.28        4/5/02 11:47:28 AM     Tim White       SCR(s) 
 *        3510 :
 *        Backout DCS #3468
 *        
 *        
 *  28   mpeg      1.27        3/28/02 2:44:50 PM     Quillian Rutherford 
 *        SCR(s) 3468 :
 *        Removed bit fields
 *        
 *        
 *  27   mpeg      1.26        3/26/02 3:44:36 PM     Dave Wilson     SCR(s) 
 *        3449 :
 *        osdUnRegisterLineISR used to hang if called with a function pointer 
 *        and line
 *        number that did not correspond to the entry at the head of the linked
 *         list. The
 *        code to walk to the next list member was missing!
 *        
 *  26   mpeg      1.25        10/5/01 2:50:56 PM     Dave Wilson     SCR(s) 
 *        2684 2685 :
 *        Changed aspect ratio change handler to call UpdateMpgScaling 
 *        immediately
 *        rather than waiting for next int 5. Turns out that the pan scan 
 *        offset takes
 *        longer to take effect than is available if we wait till later.
 *        
 *  25   mpeg      1.24        8/28/01 1:03:10 PM     Dave Wilson     SCR(s) 
 *        2542 2543 :
 *        Moved deferred unblank handler from video decode complete interrupt 
 *        to
 *        Interrupt 5 handler. This closes a 1 field window where it was 
 *        possible
 *        to see garbage during video startup.
 *        
 *  24   mpeg      1.23        8/24/01 4:43:42 PM     Quillian Rutherford 
 *        SCR(s) 2525 2544 2545 :
 *        Back to previous version, moved GXA interrupt handler to gfxlib 
 *        module
 *        
 *        
 *  23   mpeg      1.22        8/21/01 5:36:58 PM     Quillian Rutherford 
 *        SCR(s) 2525 :
 *        Added gxa interrupt handler to be used by gxa idle api
 *        
 *        
 *  22   mpeg      1.21        8/14/01 6:15:18 PM     Dave Wilson     SCR(s) 
 *        1341 2483 :
 *        Ensured that the size and aspect ratio monitor task is only included 
 *        in
 *        debug builds. This frees a task, a couple of semaphores and a few K 
 *        of RAM in
 *        the release build.
 *        
 *  21   mpeg      1.20        7/10/01 6:37:04 PM     Dave Wilson     SCR(s) 
 *        2238 2237 :
 *        Changes to support cases where video of width (2n+1)*8 is decoded. 
 *        Vcore
 *        reports size of original image but we need to round the stride up to 
 *        the next
 *        16-byte boundary.
 *        
 *  20   mpeg      1.19        7/9/01 11:28:16 AM     Dave Wilson     SCR(s) 
 *        2196 2197 :
 *        Previous code was enabling automatic display of the video plane at 
 *        the end of video play rather than waiting until the image was decoded
 *         into the plane. As a result, you could see some crap before the 
 *        decode completed. Now the switch from manual to
 *        automatic display is done when the still is finished decoding.
 *        
 *  19   mpeg      1.18        7/3/01 6:01:16 PM      Dave Wilson     SCR(s) 
 *        2188 2189 :
 *        Fixed a problem introduced earlier today where I was clearing a bunch
 *         of
 *        interrupt status bits that had not been processed. This gets rid of 
 *        both the 
 *        cases I had been seeing where WaitForLiveVideoToStart was timing out.
 *        
 *  18   mpeg      1.17        7/3/01 12:06:12 PM     Dave Wilson     SCR(s) 
 *        2183 2184 :
 *        Previous code didn't update the MPEG scaling if a size change 
 *        interrupt occurred and the new size was the same as the old one. This
 *         breaks if the last image was decoded for use on the image plane, 
 *        though, since the video plane scaling
 *        is not updated correctly. Now we update the MPEG scaling on every 
 *        size change interrupt regardless of whether the new size is the same 
 *        as the old one or not.
 *        
 *  17   mpeg      1.16        6/29/01 6:06:54 PM     Dave Wilson     SCR(s) 
 *        2002 1914 2003 1915 1916 :
 *        Changed OSD MPEG interrupt handling so that UpdateMpgScaling is only 
 *        called
 *        when required rather than on every decode interrupt 5.
 *        Additional changes to reduce/remove some transients in Open.. app 
 *        startup and shutdown.
 *        
 *  16   mpeg      1.15        6/28/01 3:06:14 PM     Dave Wilson     SCR(s) 
 *        2105 2106 :
 *        Various video transient fixes. Code modified now that the video 
 *        microcode has been fixed to correctly report the buffer being 
 *        displayed.
 *        
 *  15   mpeg      1.14        5/31/01 6:50:52 PM     Dave Wilson     SCR(s) 
 *        1897 1898 :
 *        Hardware/microcode bugs mean that we can't assume the displayed 
 *        buffer
 *        based upon the last decoded anchor frame after all. New code assumes 
 *        that 
 *        we always display the P buffer when stopped (all display stills are 
 *        decoded
 *        here and all non-display are decoded into the I buffer).
 *        
 *  14   mpeg      1.13        4/11/01 6:58:52 PM     Amy Pratt       DCS914 
 *        Removed Sabine support.
 *        
 *  13   mpeg      1.12        3/2/01 4:24:18 PM      Tim White       DCS#1365:
 *         Globally decrease stack memory uasge.
 *        DCS#1366: Globally decrease stack memory usage.
 *        
 *  12   mpeg      1.11        2/26/01 5:45:58 PM     Steve Glennon   Removed 
 *        masking of lower nibble on vertical size read from hardware.
 *        Fix for DCS#1223 (along with new video microcode)
 *        
 *  11   mpeg      1.10        2/6/01 11:32:28 AM     Dave Wilson     
 *        DCS1035\1035: Fixed up aspect ratio signalling. Previous version 
 *        turned off
 *        the callback since init was called after the callback pointer had 
 *        been
 *        set.
 *        
 *  10   mpeg      1.9         10/23/00 3:06:34 PM    Miles Bintz     removed 
 *        TTX interrupt handlers
 *        
 *  9    mpeg      1.8         8/16/00 3:30:30 PM     Miles Bintz     changed 
 *        the line interrupt functions to send queue messages instead of
 *        semaphores
 *        
 *  8    mpeg      1.7         8/8/00 5:20:46 PM      Lucy C Allevato Fixed 
 *        naming typo osdRegAspectRetio now osdRegAspectRatio. Added some
 *        callback checks to callback only when aspect ratio changes.
 *        
 *  7    mpeg      1.6         5/8/00 5:07:08 PM      Rob Tilton      
 *        Initialized the line int for the lfirst item in the list.
 *        
 *  6    mpeg      1.5         4/13/00 5:50:58 PM     Rob Tilton      Added 
 *        global variable indicating the last anchor frame decoded.
 *        
 *  5    mpeg      1.4         4/11/00 4:45:32 PM     Rob Tilton      Switched 
 *        from using hardcoded numbers to using defines for last decoded 
 *        buffer.
 *        
 *  4    mpeg      1.3         4/7/00 5:45:10 PM      Rob Tilton      Added OSD
 *         ISR callback registration functions.
 *        
 *  3    mpeg      1.2         2/28/00 3:30:54 PM     Rob Tilton      Added new
 *         ISR Line Callback API.
 *        
 *  2    mpeg      1.1         2/8/00 5:19:24 PM      Rob Tilton      Disabled 
 *        the DRM error bit in the DRM interrupt enable register.
 *        
 *  1    mpeg      1.0         1/5/00 10:24:14 AM     Rob Tilton      
 * $
 * 
 *    Rev 1.41   13 Feb 2003 12:17:02   kortemw
 * SCR(s) 5479 :
 * Removed old header reference
 * 
 *    Rev 1.40   11 Feb 2003 13:33:14   dawilson
 * SCR(s) 5137 :
 * Added code to make use of new ENABLE_DEFERRED_VIDEO_UNBLANK config option.
 * 
 *    Rev 1.39   27 Jan 2003 15:26:04   dawilson
 * SCR(s) 5320 :
 * Switched the order of 2 headers to ensure OSDPRV.H would include without
 * generating errors.
 * 
 *    Rev 1.38   04 Dec 2002 12:55:42   allevalc
 * SCR(s) 5065 :
 * Wrapped register used to access the frame being displayed conditionally for Wabash (glpMpgAddrExt) and for Colorado (glpMpgPicSize) in OSDIsr function.
 * Modified all // comments.
 * 
 *    Rev 1.37   25 Sep 2002 21:04:48   vancec
 * SCR(s) 3786 :
 * Removing old DRM and AUD conditional bitfield code.
 * 
 * 
 *    Rev 1.36   06 Sep 2002 15:21:26   kortemw
 * SCR(s) 4498 :
 * Fixed Warnings
 * 
 *    Rev 1.35   15 Jul 2002 16:31:30   dawilson
 * SCR(s) 3627 :
 * Tidied up the usage of the various fields in the MPG_VID_SIZE_REG where
 * flags are stored in the low bits of the height and width definitions. Although
 * I had seen no particular problems, the wrong masks were being used in various
 * operations so I made sure that these were correct.
 * 
 *    Rev 1.34   10 Jun 2002 13:54:08   dryd
 * SCR(s) 3923 :
 * Remove MPG bitfields unconditionally - Step 2
 * 
 *    Rev 1.33   06 Jun 2002 16:51:56   dryd
 * SCR(s) 3923 :
 * Remove MPG bitfields conditionally - Step 1
 * 
 *    Rev 1.32   30 May 2002 12:13:14   dawilson
 * SCR(s) 2914 :
 * Previous code would not call the aspect change/size change notification 
 * callback if the aspect ratio of the stream changed but the size did not. This
 * has been rectified.
 * 
 *    Rev 1.31   21 May 2002 12:44:46   vancec
 * SCR(s) 3786 :
 * Removed DRM bitfields.
 * 
 *    Rev 1.30   21 May 2002 11:49:14   dawilson
 * SCR(s) 3609 :
 * Added callback from Int 5 handler to OSDLIBC to allow deferred MPG and DRM
 * updates during panscan/letterbox switching.
 * 
 *    Rev 1.29   15 May 2002 11:41:40   dawilson
 * SCR(s) 3788 :
 * Added API osdResetAspectRatioCallback to allow an app to force a new callback
 * on the next size or aspect ratio change regardless of whether the new size or
 * aspect ratio is the same as the last notified.
 * Corrected the aspect ratio callback behaviour to make the callback if the
 * aspect ratio or picture dimensions change. In the previous case, the callback
 * would not be made if the picture size changed but the aspect ratio remained the
 * same.
 * 
 *    Rev 1.28   05 Apr 2002 11:47:28   whiteth
 * SCR(s) 3510 :
 * Backout DCS #3468
 * 
 * 
 *    Rev 1.26   26 Mar 2002 15:44:36   dawilson
 * SCR(s) 3449 :
 * osdUnRegisterLineISR used to hang if called with a function pointer and line
 * number that did not correspond to the entry at the head of the linked list. The
 * code to walk to the next list member was missing!
 * 
 *    Rev 1.25   05 Oct 2001 13:50:56   dawilson
 * SCR(s) 2684 2685 :
 * Changed aspect ratio change handler to call UpdateMpgScaling immediately
 * rather than waiting for next int 5. Turns out that the pan scan offset takes
 * longer to take effect than is available if we wait till later.
 * 
 *    Rev 1.24   28 Aug 2001 12:03:10   dawilson
 * SCR(s) 2542 2543 :
 * Moved deferred unblank handler from video decode complete interrupt to
 * Interrupt 5 handler. This closes a 1 field window where it was possible
 * to see garbage during video startup.
 * 
 *    Rev 1.23   24 Aug 2001 15:43:42   rutherq
 * SCR(s) 2525 2544 2545 :
 * Back to previous version, moved GXA interrupt handler to gfxlib module
 * 
 * 
 *    Rev 1.21   14 Aug 2001 17:15:18   dawilson
 * SCR(s) 1341 2483 :
 * Ensured that the size and aspect ratio monitor task is only included in
 * debug builds. This frees a task, a couple of semaphores and a few K of RAM in
 * the release build.
 * 
 *    Rev 1.20   10 Jul 2001 17:37:04   dawilson
 * SCR(s) 2238 2237 :
 * Changes to support cases where video of width (2n+1)*8 is decoded. Vcore
 * reports size of original image but we need to round the stride up to the next
 * 16-byte boundary.
 * 
 *    Rev 1.19   09 Jul 2001 10:28:16   dawilson
 * SCR(s) 2196 2197 :
 * Previous code was enabling automatic display of the video plane at the end of video play rather than waiting until the image was decoded into the plane. As a result, you could see some crap before the decode completed. Now the switch from manual to
 * automatic display is done when the still is finished decoding.
 * 
 *    Rev 1.18   03 Jul 2001 17:01:16   dawilson
 * SCR(s) 2188 2189 :
 * Fixed a problem introduced earlier today where I was clearing a bunch of
 * interrupt status bits that had not been processed. This gets rid of both the 
 * cases I had been seeing where WaitForLiveVideoToStart was timing out.
 * 
 *    Rev 1.17   03 Jul 2001 11:06:12   dawilson
 * SCR(s) 2183 2184 :
 * Previous code didn't update the MPEG scaling if a size change interrupt occurred and the new size was the same as the old one. This breaks if the last image was decoded for use on the image plane, though, since the video plane scaling
 * is not updated correctly. Now we update the MPEG scaling on every size change interrupt regardless of whether the new size is the same as the old one or not.
 * 
 *    Rev 1.16   29 Jun 2001 17:06:54   dawilson
 * SCR(s) 2002 1914 2003 1915 1916 :
 * Changed OSD MPEG interrupt handling so that UpdateMpgScaling is only called
 * when required rather than on every decode interrupt 5.
 * Additional changes to reduce/remove some transients in Open.. app startup and shutdown.
 * 
 *    Rev 1.15   28 Jun 2001 14:06:14   dawilson
 * SCR(s) 2105 2106 :
 * Various video transient fixes. Code modified now that the video microcode has been fixed to correctly report the buffer being displayed.
 * 
 *    Rev 1.14   31 May 2001 17:50:52   dawilson
 * SCR(s) 1897 1898 :
 * Hardware/microcode bugs mean that we can't assume the displayed buffer
 * based upon the last decoded anchor frame after all. New code assumes that 
 * we always display the P buffer when stopped (all display stills are decoded
 * here and all non-display are decoded into the I buffer).
 * 
 *    Rev 1.13   11 Apr 2001 17:58:52   prattac
 * DCS914 Removed Sabine support.
 * 
 *    Rev 1.12   02 Mar 2001 16:24:18   whiteth
 * DCS#1365: Globally decrease stack memory uasge.
 * DCS#1366: Globally decrease stack memory usage.
 * 
 *    Rev 1.11   26 Feb 2001 17:45:58   glennon
 * Removed masking of lower nibble on vertical size read from hardware.
 * Fix for DCS#1223 (along with new video microcode)
 * 
 *    Rev 1.10   06 Feb 2001 11:32:28   dawilson
 * DCS1035\1035: Fixed up aspect ratio signalling. Previous version turned off
 * the callback since init was called after the callback pointer had been
 * set.
 * 
 *    Rev 1.9   23 Oct 2000 14:06:34   bintzmf
 * removed TTX interrupt handlers
 * 
 *    Rev 1.8   16 Aug 2000 14:30:30   bintzmf
 * changed the line interrupt functions to send queue messages instead of
 * semaphores
 * 
 *    Rev 1.7   08 Aug 2000 16:20:46   eching
 * Fixed naming typo osdRegAspectRetio now osdRegAspectRatio. Added some
 * callback checks to callback only when aspect ratio changes.
 * 
 *    Rev 1.6   08 May 2000 16:07:08   rtilton
 * Initialized the line int for the lfirst item in the list.
 * 
 *    Rev 1.5   13 Apr 2000 16:50:58   rtilton
 * Added global variable indicating the last anchor frame decoded.
 * 
 *    Rev 1.4   11 Apr 2000 15:45:32   rtilton
 * Switched from using hardcoded numbers to using defines for last decoded 
 * buffer.
 * 
 *    Rev 1.3   07 Apr 2000 16:45:10   rtilton
 * Added OSD ISR callback registration functions.
 * 
 *    Rev 1.2   28 Feb 2000 15:30:54   rtilton
 * Added new ISR Line Callback API.
 * 
 *    Rev 1.1   08 Feb 2000 17:19:24   rtilton
 * Disabled the DRM error bit in the DRM interrupt enable register.
 * 
 *    Rev 1.0   05 Jan 2000 10:24:14   rtilton
 * Initial revision.
 * 
 ****************************************************************************/

