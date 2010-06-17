/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                    Conexant Systems Inc. (c) 1998-2002                   */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       ac3fix.c
 *
 *
 * Description:    Source File for the AC3 Software Fix
 *
 *
 * Author:         Senthil Veluswamy
 *
 ****************************************************************************/
/* $Header: ac3fix.c, 12, 7/16/03 12:43:22 PM, Dave Aerne$
 ****************************************************************************/

/*****************/
/* Include Files */
/*****************/
#include "stbcfg.h"
#include "kal.h"
#include "retcodes.h"
#include "globals.h"
#include "trace.h"
#include "hwlib.h"
#include "aud_error.h"
#include "string.h"

#undef AC3FIX_USE_INTERRUPT_CONTEXT
/* #define AC3FIX_USE_INTERRUPT_CONTEXT */

/* #ifdef DEBUG         */
/* #define AC3FIX_DEBUG */
/* #endif               */

/***********/
/* Aliases */
/***********/
#define AC3_FIX_QU_VALID                  0x1ACE
#define AC3_FIX_QU_PROCESS_END            0xA0
#define AC3_FIX_QU_PROCESS_ALIGN_INT      0xA1
#define AC3_FIX_QU_PROCESS_TIMER_INT      0xA2

#define AC3_FIX_TIMER_PERIOD              6
#define AC3_FIX_MAX_TIMEOUT_COUNT         3

#define AC3_FRAME_SYNC_DETECT_BLOCK_SIZE  128

#define AC3_HEADER_SRATE_CODE_OFFSET      4
#define AC3_HEADER_SRATE_CODE_MASK        0xC0
#define AC3_HEADER_SRATE_CODE_SHIFT       6
#define AC3_SRATE_48KHZ_CODE              0

#define AC3_HEADER_FRSIZ_CODE_OFFSET      4
#define AC3_HEADER_FRSIZ_CODE_MASK        0x3F
#define AC3_HEADER_FRSIZ_CODE_SHIFT       0
#define AC3_FRSIZ_CODE_MAX                0x25

#define AC3_HEADER_BITSTRM_ID_OFFSET      5
#define AC3_HEADER_BITSTRM_ID_MASK        0xF8
#define AC3_HEADER_BITSTRM_ID_SHIFT       3
#define AC3_BITSTRM_ID_MAX                8

/* purpose of address mask is to remove wrap flag, bit[31], and to lop off    */
/* the lower six bits to make the pointer modulo-64. This is required as the  */
/* demux can cache up to three bytes of encoded data (per pid) even though    */
/* the encoded write pointer has already been advanced to reflect these bytes.*/
/* Also avoids hsx coherency problems as encoded write pointer may be updated */
/* before the transaction to write the data bytes to memory is complete.      */
#define AUDIO_ADDRESS_MASK                0x3FFFC0
#define AUDIO_ADDRESS_WRAP_MASK           0x80000000
#define AUDIO_ENCODED_ACCESS_SIZE         64

#define FRAME_SYNC_BYTE1                  0x0b
#define FRAME_SYNC_BYTE2                  0x77

#define AC3_SYNC_LOCATION_OFFSET_REG      0xF700

/***************/
/* Global Data */
/***************/
#ifndef AC3FIX_USE_INTERRUPT_CONTEXT
queue_id_t gAC3FixQu;
#endif /* !AC3FIX_USE_INTERRUPT_CONTEXT */

tick_id_t gAC3FixTimer;
bool gbAC3FixTimerActive;
u_int8 gAC3FixTimeoutCount;

u_int8 *gpAudioBufEnd, *gpAudioBufStart;
u_int16 AC3FrameSizeTable[19] = {64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 
                                 384, 448, 512, 640, 768, 896, 1024, 1152, 1280};

extern u_int32 gDemuxInstance;

#ifdef AC3FIX_DEBUG
u_int8 gMoveBuf[4*1024];
u_int32 gMoveBufIdx;

u_int32 gAC3TimerStart, gAC3TimerStop;

u_int32 gAC3FixIntCb, guCodeCb, guCodeSkips, gAC3FixSkips;
extern u_int32 gbNumMPGAudioErrCbs;
#endif /* AC3FIX_DEBUG */

/***********************/
/* Function prototypes */
/***********************/
bool ac3_fix_init(void);

#ifndef AC3FIX_USE_INTERRUPT_CONTEXT
static void ac3_fix_task(void *Params);
#endif /* !AC3FIX_USE_INTERRUPT_CONTEXT */

static void ac3_fix_align_callback(u_int32 uDSRRegValue);
static void ac3_fix_timer_callback(tick_id_t TickId, void *Param);

/* Helper functions */
static void ac3_fix_align_data(bool NewFrame);
static bool ac3_fix_verify_header(u_int8 *pAC3Header);
static void ac3_fix_move_data(char *pFrame, 
                                    u_int8 FrameAlignOffset, u_int32 NumBytes);
#ifdef AC3FIX_DEBUG
static void ac3_fix_debug_print(u_int8 *pAC3Header);
#endif /* AC3FIX_DEBUG */

extern void register_mpg_audio_error_callback
            (pfnMPGAudioErrorCallback pfnAudioErrorCb, u_int32 uDSRRegValue);

extern void FCopy(char *pDest, char *pSrc, u_int32 len);

/************************/
/* Function definitions */
/************************/
/*********************************************************************/
/*  ac3_fix_init()                                                   */
/*                                                                   */
/*  PARAMETERS:  None.                                               */
/*                                                                   */
/*  DESCRIPTION: Allocate resources for the fix driver and register  */
/*                the required error & timer callbacks               */
/*                                                                   */
/*  RETURNS:     TRUE - on success, FALSE - on failure.              */
/*********************************************************************/
bool ac3_fix_init(void)
{
   int retcode = TRUE;
#ifndef AC3FIX_USE_INTERRUPT_CONTEXT
   task_id_t ac3_fix_taskid=0;
   u_int32 mesg[4];
#endif /* !AC3FIX_USE_INTERRUPT_CONTEXT */

   /* Allocate the Tick timer */
   gAC3FixTimer = tick_create(ac3_fix_timer_callback, NULL, "AFTI");
   if((tick_id_t)NULL == gAC3FixTimer)
   {
      retcode = FALSE;
      #ifdef AC3FIX_DEBUG
      trace_new(TRACE_AUD | TRACE_LEVEL_ALWAYS, 
         "Unable to Allocate the AC3 Fix Timer!\n");
      #endif /* AC3FIX_DEBUG */
   }
   else
   {
      retcode = TRUE;

      /* Setup the timer */
      retcode = tick_set(gAC3FixTimer, AC3_FIX_TIMER_PERIOD, TRUE);
      if(RC_OK != retcode)
      {
         retcode = FALSE;
         #ifdef AC3FIX_DEBUG
         trace_new(TRACE_AUD | TRACE_LEVEL_ALWAYS, 
            "Unable to Set the AC3 Fix Timer! RC=%x\n", retcode);
         #endif /* AC3FIX_DEBUG */
      }
      else
      {
         retcode = TRUE;
      }
   }

#ifndef AC3FIX_USE_INTERRUPT_CONTEXT
   if(retcode)
   {
      /* Create Fix Queue */
      gAC3FixQu = qu_create(5, "AFQU");
      if((queue_id_t)NULL == gAC3FixQu)
      {
         retcode = FALSE;
         #ifdef AC3FIX_DEBUG
         trace_new(TRACE_AUD | TRACE_LEVEL_ALWAYS, 
            "Unable to Create the AC3 Fix Qu!\n");
         #endif /* AC3FIX_DEBUG */
      }
      else
      {
         retcode = TRUE;

         /* Create the Fix Task */
         ac3_fix_taskid = task_create(ac3_fix_task, NULL, NULL, 
                     AFTK_TASK_STACK_SIZE, AFTK_TASK_PRIORITY, AFTK_TASK_NAME);
         if((task_id_t)NULL == ac3_fix_taskid)
         {
            retcode = FALSE;
            #ifdef AC3FIX_DEBUG
            trace_new(TRACE_AUD | TRACE_LEVEL_ALWAYS, 
               "Unable to Create the AC3 Fix Task!\n");
            #endif /* AC3FIX_DEBUG */
         }
         else
         {
            retcode = TRUE;
         }
      }
   }
#endif /* !AC3FIX_USE_INTERRUPT_CONTEXT */

   if(!retcode)
   {
#ifndef AC3FIX_USE_INTERRUPT_CONTEXT
      if(ac3_fix_taskid)
      {
         mesg[0] = AC3_FIX_QU_VALID;
         mesg[1] = AC3_FIX_QU_PROCESS_END;
         qu_send(gAC3FixQu, &mesg);
      }

      if(gAC3FixQu)
      {
         qu_destroy(gAC3FixQu);
      }
#endif /* !AC3FIX_USE_INTERRUPT_CONTEXT */

      if(gAC3FixTimer)
      {
         tick_destroy(gAC3FixTimer);
      }
   }
   else
   {
#ifdef AC3FIX_DEBUG
      /* Start the RTC */
      *(LPREG)(RTC_DATA_REG) = 0;
      *(LPREG)(RTC_MATCH_REG) = 0xFFFFFFFF;
#endif /* AC3FIX_DEBUG */

      /* Register the AC3 Align Error Callback */
      register_mpg_audio_error_callback(ac3_fix_align_callback,
                                        MPG_AUDIO_AC3_ALIGN);

      /* set gpAudioBufEnd to ENCAUD_ADDR + ENCAUD_SIZE, i.e. video start */
      gpAudioBufEnd = (u_int8 *)
                  ((HWBUF_ENCAUD_ADDR + HWBUF_ENCAUD_SIZE)
                                    | BSWAP_OFFSET | NCR_BASE);

      gpAudioBufStart = (u_int8 *)
                  (*DPS_AUDIO_START_ADDR_EX(gDemuxInstance)
                                    | BSWAP_OFFSET | NCR_BASE);

      #ifdef AC3FIX_DEBUG
      trace_new(TRACE_AUD | TRACE_LEVEL_2, 
            "Init. Audio Buf Start=%x, Buf End=%x.\n", 
                                 gpAudioBufStart, gpAudioBufEnd);
      #endif /* AC3FIX_DEBUG */
   }

   return(retcode);
}

#ifndef AC3FIX_USE_INTERRUPT_CONTEXT
/*********************************************************************/
/*  ac3_fix_task()                                                   */
/*                                                                   */
/*  PARAMETERS:  Params - parameters if any, passed                  */
/*                                                                   */
/*  DESCRIPTION: Wait for a data align message from the Audio AC3    */
/*                Align callback. Locate the sync word/header for the*/
/*                AC3 frame and if the header/frame are not aligned  */
/*                on an 8byte boundary, align the frame. Alignment is*/
/*                done only after the whole frame is rxd.            */
/*               If the whole frame has not been rxd when the data   */
/*                align message is rxd, a wait timer is started to   */
/*                wait for a 8 milliSecond window. The timer timeout */
/*                messages are passsed to this task a well. Data     */
/*                alignment is then reattempted.                     */
/*               If the 8 milliseconds elapse without the complete   */
/*                frame is rxd, then the frame alignment is discarded*/
/*                and all state is discarded.                        */
/*               If in the middle of waiting for the timeout, a new  */
/*                frame message arrives, the timer is stopped, all   */
/*                saved state discarded and the new frame is         */
/*                processed afresh.                                  */
/*                                                                   */
/*  RETURNS:     Nothing.                                            */
/*********************************************************************/
static void ac3_fix_task(void *Params)
{
   int retcode;
   u_int32 mesg[4];
   bool process_messages = TRUE;

   while(process_messages)
   {
      /* Wait for a message */
      retcode = qu_receive(gAC3FixQu, KAL_WAIT_FOREVER, &mesg);
      if(RC_OK != retcode)
      {
         #ifdef AC3FIX_DEBUG
         trace_new(TRACE_AUD | TRACE_LEVEL_ALWAYS, 
            "AC3FTask: Error receiving Qu mesg=%x!\n", retcode);
         #endif /* AC3FIX_DEBUG */
         continue;
      }

      if(AC3_FIX_QU_VALID != mesg[0])
      {
         #ifdef AC3FIX_DEBUG
         trace_new(TRACE_AUD | TRACE_LEVEL_ALWAYS, 
            "AC3FTask: Invalid Qu message Rxd. Valid=%x(%x)!\n", 
               mesg[0], AC3_FIX_QU_VALID);
         #endif /* AC3FIX_DEBUG */
         continue;
      }

      switch(mesg[1])
      {
         case AC3_FIX_QU_PROCESS_ALIGN_INT:
            ac3_fix_align_data(TRUE);
         break;

         case AC3_FIX_QU_PROCESS_TIMER_INT:
            ac3_fix_align_data(FALSE);
         break;

         case AC3_FIX_QU_PROCESS_END:
            process_messages = FALSE;
         break;

         #ifdef AC3FIX_DEBUG
         default:
            trace_new(TRACE_AUD | TRACE_LEVEL_ALWAYS, 
               "AC3FTask: Rxd Unknown Qu Command=%x!\n", mesg[1]);
         #endif /* AC3FIX_DEBUG */
      }
   }

   #ifdef AC3FIX_DEBUG
   trace_new(TRACE_AUD | TRACE_LEVEL_2, 
      "AC3FTask: Process End received. Exiting.\n");
   #endif /* AC3FIX_DEBUG */

   task_terminate();
}
#endif /* !AC3FIX_USE_INTERRUPT_CONTEXT */

/*********************************************************************/
/*  ac3_fix_align_data()                                             */
/*                                                                   */
/*  PARAMETERS:  NewAC3Frame - New AC3 Frame alignment request.      */
/*                                                                   */
/*  DESCRIPTION: Calculate the AC3 Sync location. Check if the       */
/*                sync word is aligned on an 8-byte boundary. If not,*/
/*                the encoded AC3 frame needs to be re-aligned.      */
/*                If complete AC3 frame has been rxd when interrupt  */
/*                is processed then proceed to align the data.       */
/*                Else copy the bytes of the encoded frame which have*/
/*                been received and then set a timer to return later.*/
/*                When the timer expires check to see if remaining   */
/*                bytes of frame have been received. If yes then     */
/*                copy the remaining bytes and the frame is complete,*/
/*                else don't copy any bytes and reset timer to       */
/*                return later. Will come back a maximum of          */
/*                AC3_FIX_MAX_TIMEOUT_COUNT times before bailing out */
/*                on the current frame.                              */
/*                                                                   */
/*  RETURNS:     Nothing.                                            */
/*********************************************************************/
static void ac3_fix_align_data(bool NewAC3Frame)
{
   int retcode;
   static volatile u_int8 *spAC3FramePtrOffset = 
                        (u_int8 *)(AC3_SYNC_LOCATION_OFFSET_REG | NCR_BASE);
   static volatile u_int8 *spAC3uCodeFlag = 
                        (u_int8 *)((AC3_SYNC_LOCATION_OFFSET_REG + 1) | NCR_BASE);
   static u_int32 ac3_align_offset;
   static u_int32 num_bytes_copied;
   static u_int8 *ac3_frame_ptr;
   u_int8 *audio_read_ptr;
   u_int8 *audio_write_ptr;
   u_int32 num_bytes_rxd=0;
   int frame_size_code;
   static u_int32 frame_size;
   bool audio_read_ptr_wrap_flag;
   bool frame_sync_at_offset = TRUE;

   /* Turn off the timer */
   if(gbAC3FixTimerActive)
   {
      retcode = tick_stop(gAC3FixTimer);
      #ifdef AC3FIX_DEBUG
      if(RC_OK == retcode)
      {
         isr_trace_new(TRACE_AUD | TRACE_LEVEL_1, 
            "AC3FAlign: New Frame=%x. Stopped Timer.\n", NewAC3Frame, 0);
      }
      #endif /* AC3FIX_DEBUG */
      gbAC3FixTimerActive = FALSE;
   }

   /* assert startdsp bit within DSR as handshaking signal for audio ucode */
   CNXT_SET_VAL(MPG_DECODE_STATUS_REG,
                MPG_DECODE_STATUS_TESTSTARTDSPWRITE_MASK,
                TRUE);

   /* Have we been called with a new Frame in? */
   if(NewAC3Frame)
   {
      /* Reset the timeout count */
      gAC3FixTimeoutCount = 0;

      /* Initialize the Read pointer */
      audio_read_ptr = (u_int8 *)    /* Get the Address. Chop off other bits*/
            ((*DPS_AUDIO_READ_PTR_EX(gDemuxInstance) & AUDIO_ADDRESS_MASK) | 
                                BSWAP_OFFSET | NCR_BASE);

      /* Initialize the Read pointer wrap flag, bit [31]*/
      audio_read_ptr_wrap_flag = (bool)  /* Get the wrap flag. Chop off other bits*/
            (*DPS_AUDIO_READ_PTR_EX(gDemuxInstance) >> 31);
   
      /* Calculate the frame pointer location */
      if((AC3_FRAME_SYNC_DETECT_BLOCK_SIZE - *spAC3FramePtrOffset) > 
         (audio_read_ptr - gpAudioBufStart))
      {
         ac3_frame_ptr = gpAudioBufEnd - 
                       (AC3_FRAME_SYNC_DETECT_BLOCK_SIZE - *spAC3FramePtrOffset
                       - (audio_read_ptr - gpAudioBufStart));
      }
      else
      {
         ac3_frame_ptr = audio_read_ptr - 
                    (AC3_FRAME_SYNC_DETECT_BLOCK_SIZE - *spAC3FramePtrOffset);
      }
   
      #ifdef AC3FIX_DEBUG
      isr_trace_new(TRACE_AUD | TRACE_LEVEL_1, 
         "AC3FAlign: Read ptr=%x, FramePtr=%x\n", 
            (u_int32)audio_read_ptr, (u_int32)ac3_frame_ptr);
      isr_trace_new(TRACE_AUD | TRACE_LEVEL_1, 
         "AC3FAlign: uCode offset=%x, \n", 
            *spAC3FramePtrOffset, 0);
   
      /* Is the Frame Ptr in the expected range */
      if((ac3_frame_ptr >= gpAudioBufEnd) || (ac3_frame_ptr < gpAudioBufStart))
      {
         isr_trace_new(TRACE_AUD | TRACE_LEVEL_ALWAYS, 
            "AC3Align: Illegal pFrame=%x, Bounds Missed=Buf %s\n", 
               (u_int32)(ac3_frame_ptr), 
                  (u_int32)((ac3_frame_ptr < gpAudioBufStart) ? "Start" : "End"));
      }
      #endif /* AC3FIX_DEBUG */
   
      /* Do we have a valid Sync ptr? */
      frame_sync_at_offset = ac3_fix_verify_header(ac3_frame_ptr);
      if(!frame_sync_at_offset)
      {
         #ifdef AC3FIX_DEBUG
         isr_trace_new(TRACE_AUD | TRACE_LEVEL_2, 
         "AC3FAlign: uCode offset=%x, \n", 
            *spAC3FramePtrOffset, 0);
         ac3_fix_debug_print(ac3_frame_ptr);
         #endif /* AC3FIX_DEBUG */
   
         /* Fix up SPDIF's "expected" location */
         ac3_frame_ptr = (u_int8 *)((u_int32)ac3_frame_ptr & ~0x7);
         /* SPDIF expects a frame sync here. */
         /* ac3_frame_ptr masked to be on an 8-byte boundary so do not 
          * need to check for buffer wrap when writing 2-byte syncword */
         *ac3_frame_ptr = FRAME_SYNC_BYTE1;
         *(ac3_frame_ptr+1) = FRAME_SYNC_BYTE2;
         #ifdef AC3FIX_DEBUG
         isr_trace_new(TRACE_AUD | TRACE_LEVEL_2, 
            "AC3FAlign: Frame Sync not found! Ptr=%x, sync_found=%x.\n", 
               (u_int32)ac3_frame_ptr, frame_sync_at_offset);
         #endif /* AC3FIX_DEBUG */
      }
   
      /* Is Align/Move necessary? */
      ac3_align_offset = (u_int32)ac3_frame_ptr & 0x7;

      if(ac3_align_offset==0)
      {
         isr_trace_new(TRACE_AUD | TRACE_LEVEL_1, 
            "AC3FAlign: Frame already aligned. Ptr=%x.\n", 
               (u_int32)ac3_frame_ptr, 0);
   
         /* deassert startdsp bit within DSR. Used as handshaking signal within audio ucode */
         CNXT_SET_VAL(MPG_DECODE_STATUS_REG, 
                      MPG_DECODE_STATUS_TESTSTARTDSPWRITE_MASK,
                      0);
         return;
      }
      else 
      /* offset will be in range 0 < ac3_align_offset < 8. */
      {
         #ifdef AC3FIX_DEBUG
         isr_trace_new(TRACE_AUD | TRACE_LEVEL_1, 
            "AC3FAlign: Frame Ptr=%x. Needs to be aligned by=%d.\n", 
               (u_int32)ac3_frame_ptr, ac3_align_offset);
         #endif /* AC3FIX_DEBUG */
   
         /* if AC3 ucode flag then this implies ac3 decode and passthrough */ 
         /* of same and need to adjust audio encoded read pointer */
         if (*spAC3uCodeFlag != 0)
         {
            /* offset is invalid as it is out of range */
            if ((*spAC3FramePtrOffset < 0x3a) || (*spAC3FramePtrOffset > 0x79))
            {
               #ifdef AC3FIX_DEBUG
               isr_trace_new(TRACE_AUD | TRACE_LEVEL_ALWAYS, 
                  "AC3FAlign: Illegal Offset of %x.\n", 
                     (u_int32)*spAC3FramePtrOffset, 0);
               #endif /* AC3FIX_DEBUG */
               ;
            }
            /* for 0x3a <= offset <= 0x3f, rewind read pointer by 128-bytes */
            /* and take wrap flag into account */
            else if (*spAC3FramePtrOffset < 0x40)
            {
               audio_read_ptr -= AUDIO_ENCODED_ACCESS_SIZE<<1;
               if (audio_read_ptr < gpAudioBufStart)
               {
                  audio_read_ptr = audio_read_ptr - gpAudioBufStart + gpAudioBufEnd;
                  audio_read_ptr_wrap_flag = audio_read_ptr_wrap_flag ^ 1;
               }
               *DPS_AUDIO_READ_PTR_EX(gDemuxInstance) = (((u_int32) audio_read_ptr & 
                  AUDIO_ADDRESS_MASK) | (u_int32) (audio_read_ptr_wrap_flag << 31));
            }
            /* for 0x40 <= offset <= 0x79, rewind read pointer by 64-bytes */
            /* and take wrap flag into account */
            else 
            {
               audio_read_ptr -= AUDIO_ENCODED_ACCESS_SIZE;
               if (audio_read_ptr < gpAudioBufStart)
               {
                  audio_read_ptr = audio_read_ptr - gpAudioBufStart + gpAudioBufEnd;
                  audio_read_ptr_wrap_flag = audio_read_ptr_wrap_flag ^ 1;
               }
               *DPS_AUDIO_READ_PTR_EX(gDemuxInstance) = (((u_int32) audio_read_ptr & 
                  AUDIO_ADDRESS_MASK) | (u_int32) (audio_read_ptr_wrap_flag << 31));
            }
         } /* if (*spAC3uCodeFlag != 0) */
   
         /* Determine the frame size (in bytes) */
         if((ac3_frame_ptr + AC3_HEADER_FRSIZ_CODE_OFFSET) >= gpAudioBufEnd)
         {
            frame_size_code = (*(gpAudioBufStart + (ac3_frame_ptr + AC3_HEADER_FRSIZ_CODE_OFFSET - 
                                  gpAudioBufEnd)) & 
                               AC3_HEADER_FRSIZ_CODE_MASK) >>
                              AC3_HEADER_FRSIZ_CODE_SHIFT;
         }
         else
         {
            frame_size_code = (*(ac3_frame_ptr+AC3_HEADER_FRSIZ_CODE_OFFSET) & 
                               AC3_HEADER_FRSIZ_CODE_MASK) >>
                              AC3_HEADER_FRSIZ_CODE_SHIFT;
         }
         frame_size = AC3FrameSizeTable[frame_size_code>>1] << 1;  /* in bytes */

      }  /* offset != 0 */
   }  /* NewAC3Frame */

   /**************************************************************************
    * always go through the following code every time fix_align_data is called
    * and alignment is necessary. Will be either a new frame requiring 
    * alignment or an iteration for copying remainder of current frame. 
    **************************************************************************/

   /* Get the Audio Write address */
   audio_write_ptr = (u_int8 *)   /* Get the Address. Chop off other bits*/
            ((*DPS_AUDIO_WRITE_PTR_EX(gDemuxInstance) & AUDIO_ADDRESS_MASK) | 
                              BSWAP_OFFSET | NCR_BASE);
   /* Get the number of bytes rxd */
   if(audio_write_ptr < ac3_frame_ptr)
   {
      /* Write ptr wrapped around */
      num_bytes_rxd = (audio_write_ptr - gpAudioBufStart) +
                      (gpAudioBufEnd - ac3_frame_ptr);
   }
   else
   {
      num_bytes_rxd = (audio_write_ptr - ac3_frame_ptr);
   }

   #ifdef AC3FIX_DEBUG
   isr_trace_new(TRACE_AUD | TRACE_LEVEL_1, 
      "AC3FAlign: Frame size=%x, #Bytes Rxd=%x\n", 
         frame_size, num_bytes_rxd);
   isr_trace_new(TRACE_AUD | TRACE_LEVEL_1, 
      "AC3FAlign: Frame Ptr=%x, Audio Write Ptr=%x\n", 
         (u_int32)ac3_frame_ptr, (u_int32)audio_write_ptr);
   #endif /* AC3FIX_DEBUG */

   /* Has the complete frame been rxd? */
   if(num_bytes_rxd < frame_size)
   { /* No */
      #ifdef AC3FIX_DEBUG
       isr_trace_new(TRACE_AUD | TRACE_LEVEL_1, 
         "AC3FAlign: Incomplete Frame. NumRxd=%x\n", 
            num_bytes_rxd, 0);
      #endif /* AC3FIX_DEBUG */

      if (NewAC3Frame) /* if initial time for new frame (when count is 0) */
      {
         /* Copy and Align as many bytes as possible */
         ac3_fix_move_data((char *)ac3_frame_ptr, 
                       ac3_align_offset, 
                       num_bytes_rxd);
         num_bytes_copied = num_bytes_rxd;
         #ifdef AC3FIX_DEBUG
         isr_trace_new(TRACE_AUD | TRACE_LEVEL_1, 
            "AC3FAlign: Inc Frame. NumRxd/Cpd=%x. ac3_align_offset=%x.\n", 
               num_bytes_rxd, ac3_align_offset);
         #endif /* AC3FIX_DEBUG */
      }
      /* else don't copy any additional data at this time*/
      /* Start a timer and wait a little */
      if(AC3_FIX_MAX_TIMEOUT_COUNT > gAC3FixTimeoutCount)
      {
         retcode = tick_start(gAC3FixTimer);
         if(RC_OK == retcode)
         {
            gbAC3FixTimerActive = TRUE;
            #ifdef AC3FIX_DEBUG
            isr_trace_new(TRACE_AUD | TRACE_LEVEL_1, 
              "AC3FAlign: Inc Frame. NumRxd=%x, ac3_align_offset=%x. Started Timer.\n", 
                  num_bytes_rxd, ac3_align_offset);
            #endif /* AC3FIX_DEBUG */
         }
      }
   }
   else
   { /* Yes. Whole frame has been rxd */
      /* Align frame Back to the nearest 8-byte boundary */
      if(!NewAC3Frame) { /* adjust num_bytes_rxd count */
         frame_size -= num_bytes_copied;
         ac3_frame_ptr = ac3_frame_ptr + num_bytes_copied;
         if(gpAudioBufEnd < ac3_frame_ptr) {
           ac3_frame_ptr = ac3_frame_ptr - (gpAudioBufEnd - gpAudioBufStart);
         }
      } /* if(!NewAC3Frame) */
      ac3_fix_move_data((char *)(ac3_frame_ptr), 
                    ac3_align_offset, 
                    frame_size);
      #ifdef AC3FIX_DEBUG
      isr_trace_new(TRACE_AUD | TRACE_LEVEL_2, 
      "AC3FAlign: Comp Frame. ac3_frame_ptr=%x, NumCpd=%x.\n", 
          (u_int32)(ac3_frame_ptr), frame_size);
         #endif /* AC3FIX_DEBUG */
   }

   #ifdef AC3FIX_DEBUG
   isr_trace_new(TRACE_AUD | TRACE_LEVEL_1, 
      "AC3FAlign: Alignment Complete. Returning.\n", 0, 0);
   #endif /* AC3FIX_DEBUG */

   /* deassert startdsp bit within DSR as handshaking signal for audio ucode */
   CNXT_SET_VAL(MPG_DECODE_STATUS_REG,
                MPG_DECODE_STATUS_TESTSTARTDSPWRITE_MASK,
                FALSE);
   return;
}


/*********************************************************************/
/*  ac3_fix_verify_header()                                          */
/*                                                                   */
/*  PARAMETERS:  pAC3Header - pointer to the AC3 Frame header.       */
/*                                                                   */
/*  DESCRIPTION: Test various header fields to determine if the      */
/*                passed header is a valid AC3 header.               */
/*                                                                   */
/*  RETURNS:     TRUE - to indicate that a valid header has been     */
/*                detected.                                          */
/*               FALSE - to indicate that a header was not found.    */
/*********************************************************************/
static bool ac3_fix_verify_header(u_int8 *pAC3Header)
{
   u_int8 data;
   #ifdef AC3FIX_DEBUG
   bool wrap = FALSE;
   #endif /* AC3FIX_DEBUG */

   #ifdef AC3FIX_DEBUG
   isr_trace_new(TRACE_AUD | TRACE_LEVEL_1, 
      "VerifyAC3Header: Sync Ptr=%x.\n", (u_int32)pAC3Header, 0);
   #endif /* AC3FIX_DEBUG */

   /* Is the frame sync present? */
   data = *pAC3Header;
   if((u_int8)FRAME_SYNC_BYTE1 != data)
   {
      /* Invalid! */
      #ifdef AC3FIX_DEBUG
      isr_trace_new(TRACE_AUD | TRACE_LEVEL_2, 
         "VerifyAC3Header: Invalid Sync1=%x, wrap=%d!\n", 
            data, wrap);
      #endif /* AC3FIX_DEBUG */
      return(FALSE);
   }

   /* frame sync 2 */
   if(gpAudioBufEnd <= (pAC3Header+1))
   {
      data = *gpAudioBufStart;
   }
   else
   {
      data = *(pAC3Header+1);
   }
   if((u_int8)FRAME_SYNC_BYTE2 != data)
   {
      /* Invalid! */
      #ifdef AC3FIX_DEBUG
      isr_trace_new(TRACE_AUD | TRACE_LEVEL_2, 
         "VerifyAC3Header: Invalid Sync2=%x, wrap=%d!\n", 
            data, wrap);
      #endif /* AC3FIX_DEBUG */
      return(FALSE);
   }

   /* Get the Sample Rate Code */
   if((pAC3Header + AC3_HEADER_SRATE_CODE_OFFSET) >= gpAudioBufEnd)
   {
      data = (*(gpAudioBufStart + 
               (AC3_HEADER_SRATE_CODE_OFFSET + (pAC3Header - gpAudioBufEnd))) &
              AC3_HEADER_SRATE_CODE_MASK) >>
             AC3_HEADER_SRATE_CODE_SHIFT;
      #ifdef AC3FIX_DEBUG
      wrap = TRUE;
      #endif /* AC3FIX_DEBUG */
   }
   else
   {
      data = (*(pAC3Header + AC3_HEADER_SRATE_CODE_OFFSET) & 
              AC3_HEADER_SRATE_CODE_MASK) >>
             AC3_HEADER_SRATE_CODE_SHIFT;
   }

   /* Valid Sample Rate Code ? */
   if((int)AC3_SRATE_48KHZ_CODE != data)
   {
      /* Invalid! */
      #ifdef AC3FIX_DEBUG
      isr_trace_new(TRACE_AUD | TRACE_LEVEL_2, 
         "VerifyAC3Header: Invalid Sample Rate Code =%d, wrap=%d!\n", 
            data, wrap);
      #endif /* AC3FIX_DEBUG */
      return(FALSE);
   }

   /* Check the Frame Size Code */
   if((pAC3Header + AC3_HEADER_FRSIZ_CODE_OFFSET) >= gpAudioBufEnd)
   {
      data = (*(gpAudioBufStart +
               (AC3_HEADER_FRSIZ_CODE_OFFSET + (pAC3Header - gpAudioBufEnd)))
              & AC3_HEADER_FRSIZ_CODE_MASK) >>
             AC3_HEADER_FRSIZ_CODE_SHIFT;
      #ifdef AC3FIX_DEBUG
      wrap = TRUE;
      #endif /* AC3FIX_DEBUG */
   }
   else
   {
      data = (*(pAC3Header + AC3_HEADER_FRSIZ_CODE_OFFSET) & 
                           AC3_HEADER_FRSIZ_CODE_MASK) >>
             AC3_HEADER_FRSIZ_CODE_SHIFT;
   }

   /* Valid Frame Size Code ? */
   if((int)AC3_FRSIZ_CODE_MAX < data)
   {
      /* Invalid! */
      #ifdef AC3FIX_DEBUG
      isr_trace_new(TRACE_AUD | TRACE_LEVEL_2, 
         "VerifyAC3Header: Invalid Frame Size Code =%d, wrap=%d!\n", 
            data, wrap);
      #endif /* AC3FIX_DEBUG */
      return(FALSE);
   }

   /* Final Check - Bit stream Id */
   if((pAC3Header + AC3_HEADER_BITSTRM_ID_OFFSET) >= gpAudioBufEnd)
   {
      data = (*(gpAudioBufStart +
               (AC3_HEADER_BITSTRM_ID_OFFSET + (pAC3Header- gpAudioBufEnd)))
              & AC3_HEADER_BITSTRM_ID_MASK) >>
             AC3_HEADER_BITSTRM_ID_SHIFT;
      #ifdef AC3FIX_DEBUG
      wrap = TRUE;
      #endif /* AC3FIX_DEBUG */
   }
   else
   {
      data = (*(pAC3Header + AC3_HEADER_BITSTRM_ID_OFFSET) & 
                           AC3_HEADER_BITSTRM_ID_MASK) >>
             AC3_HEADER_BITSTRM_ID_SHIFT;
   }

   /* Valid Bit stream Id ? */
   if((int)AC3_BITSTRM_ID_MAX < data)
   {
      /* Invalid! */
      #ifdef AC3FIX_DEBUG
      isr_trace_new(TRACE_AUD | TRACE_LEVEL_2, 
         "VerifyAC3Header: Invalid Bit stream Id =%d, wrap=%d!\n", data, wrap);
      #endif /* AC3FIX_DEBUG */
      return(FALSE);
   }

   return(TRUE);
}

/*********************************************************************/
/*  ac3_fix_move_data()                                              */
/*                                                                   */
/*  PARAMETERS:  pFrame - Pointer to Frame Header.                   */
/*               FrameAlignOffset - Offset to move back the header by*/
/*               NumBytes - Number of Bytes to move                  */
/*                                                                   */
/*  DESCRIPTION: Move back the specified number of bytes accounting  */
/*                for audio buffer wrap arounds.                     */
/*                                                                   */
/*  RETURNS:     Nothing.                                            */
/*********************************************************************/
static void ac3_fix_move_data(char *pFrame, 
                                    u_int8 FrameAlignOffset, u_int32 NumBytes)
{
   bool cs;

#ifdef AC3FIX_DEBUG
   int retcode;
   u_int8 *audio_read_ptr, *audio_write_ptr;

   isr_trace_new(TRACE_AUD | TRACE_LEVEL_2, 
      "AC3Move: Dest=%x, #Bytes=%x\n", (u_int32)pFrame, NumBytes);

   memset(gMoveBuf, 0, sizeof(gMoveBuf));
   gMoveBufIdx = 0;
   isr_trace_new(TRACE_AUD | TRACE_LEVEL_1, 
      "AC3Move: Memcpy Dest=%x, #Src=%x\n", (u_int32)(gMoveBuf+gMoveBufIdx), 
                        (u_int32)(pFrame - FrameAlignOffset));
   /* Initialize the Read pointer */
   audio_read_ptr = (u_int8 *)    /* Get the Address. Chop off other bits*/
         ((*DPS_AUDIO_READ_PTR_EX(gDemuxInstance) & AUDIO_ADDRESS_MASK) | 
                                BSWAP_OFFSET | NCR_BASE);
   /* Get the Audio Write address */
   audio_write_ptr = (u_int8 *)   /* Get the Address. Chop off other bits*/
            ((*DPS_AUDIO_WRITE_PTR_EX(gDemuxInstance) & AUDIO_ADDRESS_MASK) | 
                              BSWAP_OFFSET | NCR_BASE);
#endif /* AC3FIX_DEBUG */

   /* Make sure than the Frame is valid */
   if(pFrame > (char *)gpAudioBufEnd)
   {
      #ifdef AC3FIX_DEBUG
      isr_trace_new(TRACE_AUD | TRACE_LEVEL_ALWAYS, 
         "AC3Move: Illegal Move. pFrame=%x, AudBufEnd=%x\n", 
            (u_int32)(pFrame), (u_int32)(gpAudioBufEnd));
      #endif /* AC3FIX_DEBUG */
      return;
   }

   cs = critical_section_begin();

   /* Check for Wrap around */
   if((char *)gpAudioBufEnd < (pFrame+NumBytes))
   {
#ifdef AC3FIX_DEBUG
      FCopy((char *)gMoveBuf, pFrame, ((char *)gpAudioBufEnd-pFrame));
      FCopy((char *)gMoveBuf+((char *)gpAudioBufEnd-pFrame), (char *)gpAudioBufStart, 
            (NumBytes-(u_int32)((char *)gpAudioBufEnd-pFrame)));
#endif /* AC3FIX_DEBUG */

      FCopy((char *)(pFrame - FrameAlignOffset), 
            (char *)pFrame, 
            (u_int32)((char *)gpAudioBufEnd - pFrame));
      /* Update the remaining number of bytes to copy */
      NumBytes -= ((char *)gpAudioBufEnd - pFrame);
      FCopy((char *)(gpAudioBufEnd - FrameAlignOffset), 
            (char *)gpAudioBufStart, 
            FrameAlignOffset);
      /* Update the remaining number of bytes to copy */
      NumBytes -= FrameAlignOffset;
      FCopy((char *)gpAudioBufStart, 
            (char *)(gpAudioBufStart + FrameAlignOffset), 
            (u_int32)NumBytes);

#ifdef AC3FIX_DEBUG
      /* Check move */
      retcode = memcmp(gMoveBuf, (pFrame-FrameAlignOffset), 
                     (u_int32)((char *)gpAudioBufEnd - pFrame + FrameAlignOffset));
      if(retcode != 0)
      {
         isr_trace_new(TRACE_AUD | TRACE_LEVEL_ALWAYS, 
            "AC3Move: Memcmp Miss1 ReadPtr=%x, WritePtr=%x!\n", 
               (u_int32)audio_read_ptr, (u_int32)audio_write_ptr);
      }
      retcode = memcmp(gMoveBuf+(u_int32)((char *)gpAudioBufEnd - pFrame + FrameAlignOffset),
                   gpAudioBufStart, NumBytes);
      if(retcode != 0)
      {
         isr_trace_new(TRACE_AUD | TRACE_LEVEL_ALWAYS, 
            "AC3Move: Memcmp Miss2 ReadPtr=%x, WritePtr=%x!\n", 
               (u_int32)audio_read_ptr, (u_int32)audio_write_ptr);
      }
#endif /* AC3FIX_DEBUG */
   }
   else
   {
#ifdef AC3FIX_DEBUG
      FCopy((char *)gMoveBuf, (char *)pFrame, NumBytes);
#endif /* AC3FIX_DEBUG */

      FCopy((char *)(pFrame - FrameAlignOffset), (char *)pFrame, NumBytes);

#ifdef AC3FIX_DEBUG
      retcode = memcmp(gMoveBuf, (pFrame-FrameAlignOffset), NumBytes);
      if(retcode != 0)
      {
         isr_trace_new(TRACE_AUD | TRACE_LEVEL_ALWAYS, 
            "AC3Move: Memcmp Miss ReadPtr=%x, WritePtr=%x!\n", 
               (u_int32)audio_read_ptr, (u_int32)audio_write_ptr);
         isr_trace_new(TRACE_AUD | TRACE_LEVEL_ALWAYS, 
            "AC3Move: Memcmp Miss FrameAlignOffset=%x, NumBytes=%x!\n", 
               FrameAlignOffset, NumBytes);
      }
#endif /* AC3FIX_DEBUG */
   }

   critical_section_end(cs);

   return;
}

/*********************************************************************/
/*  ac3_fix_isr()                                                    */
/*                                                                   */
/*  PARAMETERS:  uDSRRegValue - Value of the MPEG DSR register.      */
/*                                                                   */
/*  DESCRIPTION: Verify that an Audio AC3 align error has occoured   */
/*                and send a message to the fix task to check and    */
/*                align a new AC3 frame if required.                 */
/*                                                                   */
/*  RETURNS:     Nothing.                                            */
/*********************************************************************/
static void ac3_fix_align_callback(u_int32 uDSRRegValue)
{
#ifndef AC3FIX_USE_INTERRUPT_CONTEXT
   u_int32 mesg[4];
   int retcode;
#endif /* !AC3FIX_USE_INTERRUPT_CONTEXT */

   /* Is this a valid callback? */
   if((uDSRRegValue&MPG_DECODE_STATUS_AUDIOERROR_MASK) != MPG_AUDIO_AC3_ALIGN)
   {
      /* Error! Invalid call. */
      #ifdef AC3FIX_DEBUG
      isr_trace_new(TRACE_AUD | TRACE_LEVEL_ALWAYS, 
         "AC3FIsr: Error! AC3 Align not set! DSR=%x.\n", uDSRRegValue, 0);
      #endif /* AC3FIX_DEBUG */
      return;
   }

#ifdef AC3FIX_DEBUG
   gAC3TimerStart = *(LPREG)(0x30490000);
   if((gAC3TimerStart - gAC3TimerStop) > 35)
   {
      gAC3FixSkips++;
      isr_trace_new(TRACE_AUD | TRACE_LEVEL_ALWAYS, 
         "AC3FIsr: Timer Start=%x. Prev Stop Diff=%d\n", gAC3TimerStart, 
                  gAC3TimerStart - gAC3TimerStop);
   }
#endif /* AC3FIX_DEBUG */

#ifndef AC3FIX_USE_INTERRUPT_CONTEXT
   /* Wake up the Fix task */
   mesg[0] = AC3_FIX_QU_VALID;
   mesg[1] = AC3_FIX_QU_PROCESS_ALIGN_INT;
   retcode = qu_send(gAC3FixQu, &mesg);
   #ifdef AC3FIX_DEBUG
   if(RC_OK != retcode)
   {
      isr_trace_new(TRACE_AUD | TRACE_LEVEL_ALWAYS, 
         "AC3FIsr: Error! Qu Send returned RC=%x.\n", retcode, 0);
   }
   #endif /* AC3FIX_DEBUG */
#else /* AC3FIX_USE_INTERRUPT_CONTEXT */
   /* Do the processing directly */
   ac3_fix_align_data(TRUE);
#endif /* !AC3FIX_USE_INTERRUPT_CONTEXT */

#ifdef AC3FIX_DEBUG
   gAC3TimerStop = *(LPREG)(0x30490000);
   isr_trace_new(TRACE_AUD | TRACE_LEVEL_1, 
         "AC3FIsr: Timer Stop=%x. Diff=%d\n", gAC3TimerStop, 
            gAC3TimerStop-gAC3TimerStart);
#endif /* AC3FIX_DEBUG */

   return;
}

/*********************************************************************/
/*  ac3_fix_timer_callback()                                         */
/*                                                                   */
/*  PARAMETERS:  TickId - Id of the Tick timer used.                 */
/*               Param - Other parameters if any, passed             */
/*                                                                   */
/*  DESCRIPTION: Send a timer timeout message to the fix task to     */
/*                indicate that the wait time has elapsed.           */
/*                                                                   */
/*  RETURNS:     Nothing.                                            */
/*********************************************************************/
static void ac3_fix_timer_callback(tick_id_t TickId, void *Param)
{
#ifndef AC3FIX_USE_INTERRUPT_CONTEXT
   u_int32 mesg[4];
   int retcode;
#endif /* !AC3FIX_USE_INTERRUPT_CONTEXT */

   if(TickId != gAC3FixTimer)
   {
      /* Error! Invalid call. */
      #ifdef AC3FIX_DEBUG
      isr_trace_new(TRACE_AUD | TRACE_LEVEL_ALWAYS, 
         "AC3FTimer: Error! Invalid Timer Id=%x(%x).\n", TickId, gAC3FixTimer);
      #endif /* AC3FIX_DEBUG */
      return;
   }

#ifndef AC3FIX_USE_INTERRUPT_CONTEXT
   /* Wake up the Fix task */
   mesg[0] = AC3_FIX_QU_VALID;
   mesg[1] = AC3_FIX_QU_PROCESS_TIMER_INT;
   retcode = qu_send(gAC3FixQu, &mesg);
   #ifdef AC3FIX_DEBUG
   if(RC_OK != retcode)
   {
      isr_trace_new(TRACE_AUD | TRACE_LEVEL_ALWAYS, 
         "AC3FTimer: Error! Qu Send returned RC=%x.\n", retcode, 0);
   }
   #endif /* AC3FIX_DEBUG */
#else /* AC3FIX_USE_INTERRUPT_CONTEXT */
   /* Do the processing directly */
   ac3_fix_align_data(FALSE);
#endif /* !AC3FIX_USE_INTERRUPT_CONTEXT */

   /* Update the timer state */
   gAC3FixTimeoutCount++;

   return;
}

#ifdef AC3FIX_DEBUG
static void ac3_fix_debug_print(u_int8 *pAC3Header)
{
   u_int8 *PrintPtr;
   int i;
   u_int8 *audio_read_ptr, *audio_write_ptr;

   /* Initialize the Read pointer */
   audio_read_ptr = (u_int8 *)    /* Get the Address. Chop off other bits*/
         ((*DPS_AUDIO_READ_PTR_EX(gDemuxInstance) & AUDIO_ADDRESS_MASK) | 
                                BSWAP_OFFSET | NCR_BASE);
   /* Get the Audio Write address */
   audio_write_ptr = (u_int8 *)   /* Get the Address. Chop off other bits*/
            ((*DPS_AUDIO_WRITE_PTR_EX(gDemuxInstance) & AUDIO_ADDRESS_MASK) | 
                              BSWAP_OFFSET | NCR_BASE);
   isr_trace_new(TRACE_AUD | TRACE_LEVEL_ALWAYS, 
      "VerifyAC3Header: ReadPtr=%x, WritePtr=%x!\n", 
         (u_int32)audio_read_ptr, (u_int32)audio_write_ptr);

   if(pAC3Header>=gpAudioBufStart+8)
   {
      PrintPtr = (u_int8 *)((u_int32)pAC3Header & ~0xf);
   }
   else
   {
      PrintPtr = (u_int8 *)(((u_int32)gpAudioBufEnd - 
                  (7 - (u_int32)pAC3Header - (u_int32)gpAudioBufStart)) & ~0xf);
   }

   isr_trace_new(TRACE_AUD | TRACE_LEVEL_2, 
      "VerifyAC3Header: Invalid Frame Ptr=%x Print Ptr=%x!\n", 
         (u_int32)pAC3Header, (u_int32)PrintPtr);

   for(i=0;i<16;i++, PrintPtr+=4)
   {
      if(PrintPtr >= gpAudioBufEnd)
      {
         PrintPtr = gpAudioBufStart;
      }
      isr_trace_new(TRACE_AUD | TRACE_LEVEL_2, 
         "VerifyAC3Header: Invalid Frame Data=%x, %x!\n", 
            *(u_int32 *)PrintPtr, *((u_int32 *)PrintPtr+1));
   }

   return;
}
#endif /* AC3FIX_DEBUG */
/****************************************************************************
 * Modifications:
 * $Log: 
 *  12   mpeg      1.11        7/16/03 12:43:22 PM    Dave Aerne      SCR(s) 
 *        6987 :
 *        previously this file only worked with mpeg ucode in support of ac3 
 *        encoded
 *        passthrough. Have added hooks to enable it to now work with ac3 ucode
 *         in 
 *        support of simultaneous decode and passthrough of ac3 stream.
 *        Added flag to discern between which ucode (mpeg/ac3) is in use and
 *        also removed code to re-verify frame header for each iteration.
 *        Rather do it once for each new frame and store the data for any 
 *        future
 *        iterations. Also made a fix to ac3_fix_move_data regarding buffer 
 *        boundary
 *        condition.
 *        
 *        
 *  11   mpeg      1.10        6/17/03 1:19:32 PM     Dave Aerne      SCR(s) 
 *        6776 :
 *        Included COMPLETION_TIMER to handle case where entire stream has not 
 *        been
 *        received at time of initial call. Modified audio_address_mask to mask
 *         off lower
 *        two bits thus making address modulo-4 and avoiding any stale data due
 *         to demux
 *        caching of encoded data. Modified definition of audio_buf_end to 
 *        handle audio
 *        encoded buffer sizes other than default 11k bytes. Adjusted timer 
 *        period from
 *        5 to 6. And modified code to handle multiple passese through the copy
 *         routine.
 *        
 *  10   mpeg      1.9         2/13/03 3:55:34 PM     Bobby Bradford  SCR(s) 
 *        5479 :
 *        Remove "hwconfig.h" and "sabine.h", replace with "stbcfg.h"
 *        
 *  9    mpeg      1.8         11/25/02 3:41:42 PM    Senthil Veluswamy SCR(s) 
 *        5018 :
 *        Fixed build warning. Added defines for Task details to taskprio.h
 *        
 *  8    mpeg      1.7         11/15/02 3:57:54 PM    Senthil Veluswamy SCR(s) 
 *        4965 :
 *        Removed a non-printing character that was causing a warning
 *        
 *  7    mpeg      1.6         11/14/02 5:27:28 PM    Senthil Veluswamy SCR(s) 
 *        4935 :
 *        Removed // comments. Added some debug statements
 *        
 *  6    mpeg      1.5         11/7/02 6:24:28 PM     Senthil Veluswamy SCR(s) 
 *        4790 :
 *        1) removed the search component. Has been moved to the uCode.
 *        2) Wrapped all trace for debug
 *        3) renamed functions to be of form ac3_XXX_XXX
 *        4) Timer called 3 * 4mSec for incomplete frames
 *        
 *  5    mpeg      1.4         11/4/02 2:31:20 PM     Senthil Veluswamy SCR(s) 
 *        4790 :
 *        Modified search not to go back beyond last search location
 *        
 *  4    mpeg      1.3         11/1/02 1:22:38 PM     Senthil Veluswamy SCR(s) 
 *        4790 :
 *        1) Included the Frame Incomplete Callback timer.
 *        2) Changed allowed distance between expected Frame pointer and Audio 
 *        Read pointer to be upto 1/2 frame.
 *        3) Lots of debug trace on/off.
 *        
 *        
 *  3    mpeg      1.2         10/25/02 5:33:40 PM    Senthil Veluswamy SCR(s) 
 *        4790 :
 *        Added code to determine if Interrupts are being missed, to count 
 *        skips, etc
 *        
 *  2    mpeg      1.1         10/21/02 4:55:10 PM    Senthil Veluswamy SCR(s) 
 *        4790 :
 *        Bug fixes, Compile time option to use Completion Timer, Interrupt 
 *        context
 *        
 *  1    mpeg      1.0         10/11/02 6:14:46 PM    Senthil Veluswamy 
 * $
 * 
 *    Rev 1.11   16 Jul 2003 11:43:22   aernedj
 * SCR(s) 6987 :
 * previously this file only worked with mpeg ucode in support of ac3 encoded
 * passthrough. Have added hooks to enable it to now work with ac3 ucode in 
 * support of simultaneous decode and passthrough of ac3 stream.
 * Added flag to discern between which ucode (mpeg/ac3) is in use and
 * also removed code to re-verify frame header for each iteration.
 * Rather do it once for each new frame and store the data for any future
 * iterations. Also made a fix to ac3_fix_move_data regarding buffer boundary
 * condition.
 * 
 * 
 *    Rev 1.10   17 Jun 2003 12:19:32   aernedj
 * SCR(s) 6776 :
 * Included COMPLETION_TIMER to handle case where entire stream has not been
 * received at time of initial call. Modified audio_address_mask to mask off lower
 * two bits thus making address modulo-4 and avoiding any stale data due to demux
 * caching of encoded data. Modified definition of audio_buf_end to handle audio
 * encoded buffer sizes other than default 11k bytes. Adjusted timer period from
 * 5 to 6. And modified code to handle multiple passese through the copy routine.
 * 
 *    Rev 1.9   13 Feb 2003 15:55:34   bradforw
 * SCR(s) 5479 :
 * Remove "hwconfig.h" and "sabine.h", replace with "stbcfg.h"
 * 
 *    Rev 1.8   25 Nov 2002 15:41:42   velusws
 * SCR(s) 5018 :
 * Fixed build warning. Added defines for Task details to taskprio.h
 * 
 *    Rev 1.7   15 Nov 2002 15:57:54   velusws
 * SCR(s) 4965 :
 * Removed a non-printing character that was causing a warning
 * 
 *    Rev 1.6   14 Nov 2002 17:27:28   velusws
 * SCR(s) 4935 :
 * Removed // comments. Added some debug statements
 * 
 * 
 *    Rev 1.5   07 Nov 2002 18:24:28   velusws
 * SCR(s) 4790 :
 * 1) removed the search component. Has been moved to the uCode.
 * 2) Wrapped all trace for debug
 * 3) renamed functions to be of form ac3_XXX_XXX
 * 4) Timer called 3 * 4mSec for incomplete frames
 * 
 *    Rev 1.4   04 Nov 2002 14:31:20   velusws
 * SCR(s) 4790 :
 * Modified search not to go back beyond last search location
 * 
 *    Rev 1.3   01 Nov 2002 13:22:38   velusws
 * SCR(s) 4790 :
 * 1) Included the Frame Incomplete Callback timer.
 * 2) Changed allowed distance between expected Frame pointer and Audio Read pointer to be upto 1/2 frame.
 * 3) Lots of debug trace on/off.
 * 
 * 
 *    Rev 1.2   25 Oct 2002 16:33:40   velusws
 * SCR(s) 4790 :
 * Added code to determine if Interrupts are being missed, to count skips, etc
 * 
 *    Rev 1.1   21 Oct 2002 15:55:10   velusws
 * SCR(s) 4790 :
 * Bug fixes, Compile time option to use Completion Timer, Interrupt context
 * 
 *    Rev 1.0   11 Oct 2002 17:14:46   velusws
 * SCR(s) 4790 :
 * Source File for the AC3 Software Fix
 *
 ****************************************************************************/

