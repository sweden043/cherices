/****************************************************************************/
/*                 Conexant Systems, Inc. - COLORADO                        */
/****************************************************************************/
/*                                                                          */
/* Filename:           PCM.C                                                */
/*                                                                          */
/* Description:        Main source file for colorado pcm driver             */
/*                                                                          */
/* Author:             Matthew W. Korte                                     */
/*                                                                          */
/* Copyright Conexant Systems, Inc., 2001                                   */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/
/*
$Header: Pcm.c, 18, 9/8/03 8:19:04 PM, Dave Aerne$
$Log: 
 18   mpeg      1.17        9/8/03 8:19:04 PM      Dave Aerne      SCR(s) 7434 
       7435 :
       adjust calculation of buf_start to ensure buffer resides within 
       malloc'ed space. Also modify memset to only clear the buffer from start 
       to stop w/o clearing the excess bytes, prevents clearing bytes outside 
       of the buffer space.
       
 17   mpeg      1.16        8/26/03 7:19:24 PM     Dave Aerne      SCR(s) 7381 
       7382 :
       modify malloc of pcm_buffers to produce quad-word aligned buffers,
       as required by arm audio hardware.
       
 16   mpeg      1.15        6/24/03 1:27:52 PM     Miles Bintz     SCR(s) 6822 
       :
       added initialization value to remove warning in release build
       
 15   mpeg      1.14        6/19/03 2:02:08 PM     Bobby Bradford  SCR(s) 6796 
       :
       Add offset of 4 to interrupt address register values to get non-
       quadword alignment (to prevent multiple interrupts from occuring)
       
 14   mpeg      1.13        4/28/03 3:59:24 PM     Senthil Veluswamy SCR(s) 
       6114 :
       Modified to use the Fast En/DeQ routines when SRC Optimization is turned
        on.
       
 13   mpeg      1.12        4/21/03 8:34:02 PM     Senthil Veluswamy SCR(s) 
       6066 6065 :
       Changed PCM_DISABLE_SRC to take values YES and NO instead of just being 
       defined.
       
 12   mpeg      1.11        4/21/03 7:56:42 PM     Senthil Veluswamy SCR(s) 
       6066 6065 :
       Modifications to compile time disable SRC and run time disable Mixing if
        the MPEG level is set to MUTE.
       
 11   mpeg      1.10        2/13/03 12:25:00 PM    Matt Korte      SCR(s) 5479 
       :
       Removed old header reference
       
 10   mpeg      1.9         8/21/02 12:46:24 PM    Ray Mack        SCR(s) 4443 
       :
       fixed another syntax error that broke the build.
       
 9    mpeg      1.8         8/21/02 12:09:00 PM    Ray Mack        SCR(s) 4443 
       :
       Fixed missing else clause in mixing.  If memory mixing didn't happen 
       then the data was garbage for all but STEREO case.  Also fixed 
       questionable C syntax for pointers and signed/unsigned error.  Fixed 
       some of the "//" comments.
       
 8    mpeg      1.7         8/12/02 3:19:56 PM     Lucy C Allevato SCR(s) 4368 
       :
       Deleted #include "string.h", <string.h> already included above.
       
 7    mpeg      1.6         6/20/02 5:14:56 PM     Steve Glennon   SCR(s): 4071
        
       Added code to flush PCMSRCQ and PCMInputQ when issued a stop command
       
 6    mpeg      1.5         6/19/02 5:54:34 PM     Steve Glennon   SCR(s): 4066
        
       Many fixes to original code which did not work correctly at all. 
       Enqueued
       and dequeued data from different queues etc.
       
 5    mpeg      1.4         5/17/02 5:03:54 PM     Steve Glennon   SCR(s): 3822
        
       Removed use of bitfields to access hardware registers.
       
       
 4    mpeg      1.3         5/15/02 3:28:06 AM     Steve Glennon   SCR(s): 2438
        
       Major work to implement full functions required of driver
       
       
 3    mpeg      1.2         4/25/02 5:57:40 PM     Matt Korte      SCR(s) 2438 
       :
       Added more support. Runs for no mixing. The file PcmMix.c
       needs to be modified to handle getting, mixing and sending
       the data buffers to/from the IRQ handlers.
       
       
 2    mpeg      1.1         8/30/01 11:55:12 AM    Matt Korte      SCR(s) 2438 
       :
       DCS #2438
       Intermediate put of Generic PCM driver. Adds the concept
       of buffers and handles. Seperates the interrupt handlers.
       The Capture ISR now moves the buffer handler from the
       Capture list to the PCM list, and set up the next buffer.
       The Play ISR now moves the buffer handler to the Capture
       list. There is a task that moves the buffers from the
       PCM list to the Play list. So the interrupt handlers are
       out of the job of mixing, and are short and quick now.
       In the future, all the mixing and conversions can happen
       in the task.
       
 1    mpeg      1.0         8/8/01 2:47:20 PM      Matt Korte      
$
 * 
 *    Rev 1.17   08 Sep 2003 19:19:04   aernedj
 * SCR(s) 7434 7435 :
 * adjust calculation of buf_start to ensure buffer resides within malloc'ed space. Also modify memset to only clear the buffer from start to stop w/o clearing the excess bytes, prevents clearing bytes outside of the buffer space.
 * 
 *    Rev 1.16   26 Aug 2003 18:19:24   aernedj
 * SCR(s) 7381 7382 :
 * modify malloc of pcm_buffers to produce quad-word aligned buffers,
 * as required by arm audio hardware.
 * 
 *    Rev 1.15   24 Jun 2003 12:27:52   bintzmf
 * SCR(s) 6822 :
 * added initialization value to remove warning in release build
 * 
 *    Rev 1.14   19 Jun 2003 13:02:08   bradforw
 * SCR(s) 6796 :
 * Add offset of 4 to interrupt address register values to get non-
 * quadword alignment (to prevent multiple interrupts from occuring)
 * 
 *    Rev 1.13   28 Apr 2003 14:59:24   velusws
 * SCR(s) 6114 :
 * Modified to use the Fast En/DeQ routines when SRC Optimization is turned on.
 * 
 *    Rev 1.12   21 Apr 2003 19:34:02   velusws
 * SCR(s) 6066 6065 :
 * Changed PCM_DISABLE_SRC to take values YES and NO instead of just being defined.
 * 
 *    Rev 1.11   21 Apr 2003 18:56:42   velusws
 * SCR(s) 6066 6065 :
 * Modifications to compile time disable SRC and run time disable Mixing if the MPEG level is set to MUTE.
 * 
 *    Rev 1.10   13 Feb 2003 12:25:00   kortemw
 * SCR(s) 5479 :
 * Removed old header reference
 * 
 *    Rev 1.9   21 Aug 2002 11:46:24   raymack
 * SCR(s) 4443 :
 * fixed another syntax error that broke the build.
 * 
 *    Rev 1.8   21 Aug 2002 11:09:00   raymack
 * SCR(s) 4443 :
 * Fixed missing else clause in mixing.  If memory mixing didn't happen then the data was garbage for all but STEREO case.  Also fixed questionable C syntax for pointers and signed/unsigned error.  Fixed some of the "//" comments.
 * 
 *    Rev 1.7   Aug 12 2002 15:19:56   allevalc
 * SCR(s) 4368 :
 * Deleted #include "string.h", <string.h> already included above.
 * 
 *    Rev 1.6   Jun 20 2002 17:14:56   glennon
 * SCR(s): 4071 
 * Added code to flush PCMSRCQ and PCMInputQ when issued a stop command
 * 
 *    Rev 1.5   Jun 19 2002 17:54:34   glennon
 * SCR(s): 4066 
 * Many fixes to original code which did not work correctly at all. Enqueued
 * and dequeued data from different queues etc.
 * 
 *    Rev 1.4   17 May 2002 16:03:54   glennon
 * SCR(s): 3822 
 * Removed use of bitfields to access hardware registers.
 * 
 * 
 *    Rev 1.3   15 May 2002 02:28:06   glennon
 * SCR(s): 2438 
 * Major work to implement full functions required of driver
 * 
 * 
 *    Rev 1.2   25 Apr 2002 16:57:40   kortemw
 * SCR(s) 2438 :
 * Added more support. Runs for no mixing. The file PcmMix.c
 * needs to be modified to handle getting, mixing and sending
 * the data buffers to/from the IRQ handlers.
 * 
 * 
 *    Rev 1.1   30 Aug 2001 10:55:12   kortemw
 * SCR(s) 2438 :
 * DCS #2438
 * Intermediate put of Generic PCM driver. Adds the concept
 * of buffers and handles. Seperates the interrupt handlers.
 * The Capture ISR now moves the buffer handler from the
 * Capture list to the PCM list, and set up the next buffer.
 * The Play ISR now moves the buffer handler to the Capture
 * list. There is a task that moves the buffers from the
 * PCM list to the Play list. So the interrupt handlers are
 * out of the job of mixing, and are short and quick now.
 * In the future, all the mixing and conversions can happen
 * in the task.
 * 
 *    Rev 1.0   08 Aug 2001 13:47:20   kortemw
 * SCR(s) 2438 :
 * DCS #2438
 * Initial put of Generic PCM driver. Does not do mixing yet. Does
 * have API interface, interrupt handlers and task. It does use
 * the interrupt handlers for the PCM data instead of hardware.
*/

/** include files **/
#include <string.h>
#include "stbcfg.h"
#include "basetype.h"
#include "confmgr.h"
#include "kal.h"
#include "retcodes.h"
#include "globals.h"
#include "gen_aud.h"
#include "pcm.h"
#include "pcmcq.h"
#include "pcmpriv.h"

/* Start Defines */
#define PCM_DEBUG

#if (PCM_DISABLE_SRC == YES)
 #define INPUT_BUFF_SIZE 32 
#endif /* (PCM_DISABLE_SRC == YES) */


/*End   Defines */
/* Start Declarations */
volatile cnxt_pcm_buf_handle_t *ps_SRCBuff;

/* Debug stats */
volatile u_int32 IdleStateDidNothing = 0;
volatile u_int32 PlayingStateDidNothing = 0;
volatile u_int32 PlayingStateCalledIdle = 0;
volatile u_int32 PCMTaskIterations = 0;
volatile u_int32 SemTimeouts = 0;

/* End   Declarations */

/* Start Prototypes */
/* static void move_pcm_buffer(volatile cnxt_pcm_buf_handle_t *FromHeadPtr,
                            volatile cnxt_pcm_buf_handle_t *FromTailPtr,
                            volatile cnxt_pcm_buf_handle_t *ToHeadPtr,
                            volatile cnxt_pcm_buf_handle_t *ToTailPtr);    */
static void move_pcm_buffer(volatile cnxt_pcm_buf_handle_t **pFromHeadPtr,
                            volatile cnxt_pcm_buf_handle_t **pFromTailPtr,
                            volatile cnxt_pcm_buf_handle_t **pToHeadPtr,
                            volatile cnxt_pcm_buf_handle_t **pToTailPtr);
                            

void pcmDoPlayingState(void);
void pcmTryToStop(void);
                            
extern void RequestRawDataIfNeeded(void);

/* End   Prototypes */

/* Start Extern Prototypes */
/* End   Extern Prototypes */

/********************************************************************/
/* FUNCTION:    cnxt_pcm_task                                       */
/*                                                                  */
/* PARAMETERS:  None.                                               */
/*                                                                  */
/* DESCRIPTION: Main task for controlling the mixing of PCM with    */
/*              Mpeg audio into the PCM output buffers              */
/*                                                                  */
/* RETURNS:     None.                                               */
/********************************************************************/
const unsigned char Beep824HzAt48kHz_data[] = {
 0xff,0x07,0x0f,0x16,0x1e,0x25,0x2b,0x31,0x36,0x3b,0x3f,0x42,0x45,0x47,0x47,0x47
,0x47,0x45,0x42,0x3f,0x3b,0x36,0x31,0x2b,0x25,0x1e,0x17,0x0f,0x07,0x00,0xf8,0xf0
,0xe9,0xe1,0xda,0xd4,0xce,0xc9,0xc4,0xc0,0xbd,0xba,0xb8,0xb8,0xb8,0xb8,0xba,0xbd
,0xc0,0xc4,0xc9,0xce,0xd4,0xda,0xe1,0xe8,0xf0,0xf8                              
};
#ifdef PCM_DEBUG
bool DoBeep = FALSE;
#endif /* def PCM_DEBUG */

void cnxt_pcm_task(void)
{
#ifdef PCM_DEBUG
    int i,j;
    volatile cnxt_pcm_buf_handle_t *ps_ptr;
    volatile cnxt_pcm_buf_handle_t *ps_myptr;
#endif
    bool kstate;
    int rc;

    ps_SRCBuff = GetNewPcmBuffer();
    
#ifdef PCM_DEBUG
    ps_myptr = GetNewPcmBuffer();
    for (i = 0, j = 0; i < ARM_AUD_QUART_BUFF_SIZE;) {
      ps_myptr->pui32_buf_start[i++] = (Beep824HzAt48kHz_data[j] << 10);
      ps_myptr->pui32_buf_start[i++] = (Beep824HzAt48kHz_data[j++] << 10) | 0x80000000;
      if (j >= sizeof(Beep824HzAt48kHz_data)) {
        j = 0;
      }
    }
#endif /* def PCM_DEBUG */
    
    ps_done_head_ptr = ps_done_tail_ptr = NULL;
    while (TRUE) {
      /* Wait here on a semaphore, with a timeout                */
      /* The capture interrupt will signal the sem to wake us up */
      rc = sem_get(cnxt_pcm_sem_id, PCM_SEM_WAIT_TIME);
      if (rc == RC_KAL_TIMEOUT)
      {
         SemTimeouts++;
      } 
      else if (rc != RC_OK)
      {
          trace_new(TRACE_LEVEL_2 | TRACE_AUD, 
                    "PCM Task: Error from sem_get, rc= %d!!!\n", 
                    rc);
      } /* endif */
      PCMTaskIterations++;
      switch (s_cnxt_pcm_status.e_state) {
        /* IDLE,PLAY,PAUSED are the same state from the view of the PCM MIX task */
        /* PLAY just means we are starting play - SRC is to take place        */
        /* When there is enough SRC'ed data to mix (above threshold) then we  */
        /* transition into PLAYING                                            */
        case IDLE:
        #if (PCM_DISABLE_SRC != YES)
        case PLAY:
        #endif /* (PCM_DISABLE_SRC != YES) */
        case PAUSED:
          pcmDoIdleState();
          break; /* end case IDLE/PLAY/PAUSED: */
        #if (PCM_DISABLE_SRC == YES)
        case PLAY:
          pcmDoIdleState();
          if(PCM20_CQ_UsedElements(&PCMInputQ) >= INPUT_BUFF_SIZE)
          {
             s_cnxt_pcm_status.e_state = PLAYING;
          }
          break; /* end case PLAY: */
        #endif /* (PCM_DISABLE_SRC == YES) */
        case PLAYING:
          pcmDoPlayingState();
          break; /* end case PLAYING: */
        case STOPPING:
          pcmDoIdleState();
          pcmTryToStop();
          break; /* end case STOPPING */
        default:
          trace_new(TRACE_LEVEL_2 | TRACE_AUD, "PCM Task: Illegal Status value=%d!!!\n", (int)(s_cnxt_pcm_status.e_state));
          s_cnxt_pcm_status.e_state = IDLE;
          break;
      } /* end switch */
      kstate = critical_section_begin();
      if (ps_done_head_ptr != NULL) {
#ifdef PCM_DEBUG
        if (DoBeep) {
          for (i = 0; i < ARM_AUD_QUART_BUFF_SIZE; i++) {
            ps_buf_handle_pcm_head->pui32_buf_start[i] = ps_myptr->pui32_buf_start[i];
          }
        }
#endif /* def PCM_DEBUG */

        move_pcm_buffer(&ps_done_head_ptr,&ps_done_tail_ptr,&ps_buf_handle_play_head,&ps_buf_handle_play_tail);
        
      } /* end if (ps_done_head_ptr != NULL) */
      critical_section_end(kstate);
#ifdef PCM_DEBUG
      kstate = critical_section_begin();
      for (ps_ptr = ps_buf_handle_capt_head; ps_ptr != ps_buf_handle_capt_tail;
            ps_ptr = ps_ptr->ps_next_buf_handle) {
        if (!(((int)ps_ptr->pui32_buf_start) & 0x10000000)) {
          trace_new(TRACE_LEVEL_2 | TRACE_AUD, "PCM Task: Bad capt buf value\n");
          fatal_exit(ERROR_FATAL | RC_AUD_BADPTR);
        }
      }
      critical_section_end(kstate);
#endif /* def PCM_DEBUG */
      #if (PCM_DISABLE_SRC == YES)
      /* Decide whether we are in a state where we should request data */
      if ((s_cnxt_pcm_status.e_state == PLAY)    ||
          (s_cnxt_pcm_status.e_state == PLAYING))
      {
         RequestRawDataIfNeeded();
      } /* endif PLAY or PLAYING */
      #endif /* (PCM_DISABLE_SRC == YES) */
/*      task_time_sleep(20); */
    } /* end while */
}   /* end cnxt_pcm_task() */

/********************************************************************/
/* FUNCTION:    arm_audio_capt_isr                                  */
/*                                                                  */
/* PARAMETERS:  interrupt_ID : Not used                             */
/*              isFIQ        : Not used                             */
/*              previous     : Not used                             */
/*                                                                  */
/* DESCRIPTION: This is the ISR that happens when the Capture       */
/*              A buffer is full (So the Play A buffer is empty     */
/*              as well). This Capture A buffer needs to be mixed   */
/*              with PCM data and moved to the Play A buffer.       */
/*                                                                  */
/* RETURNS:     None.                                               */
/********************************************************************/
u_int32 pcmtimenow, pcmtimelast, pcmtimedif, pcmtimedifmax = 0, pcmtimedifmin = 0xffffff;
int arm_audio_capt_isr( u_int32 interrupt_ID, bool isFIQ, PFNISR *previous )
{

   LPREG lpcapt_start_done;
   LPREG lpcapt_start_now;
   LPREG lpcapt_stop_done;
   LPREG lpcapt_stop_now;

#ifdef DEBUG
   pcm_capt_isr_cnt++;
   pcmtimenow = get_system_time();
   if (pcm_capt_isr_cnt > 10) {
     pcmtimedif = pcmtimenow - pcmtimelast;
     if (pcmtimedif > pcmtimedifmax) pcmtimedifmax = pcmtimedif;
     if (pcmtimedif < pcmtimedifmin) pcmtimedifmin = pcmtimedif;
   }
   pcmtimelast = pcmtimenow;
#endif 
   /* Is this the Address interrupt? */
   if(*AudCaptIntStatus & 0x01) 
   {  

      /* Don't clear the interrupt bit here as we do things in here that */
      /* can cause it to become set (adjusting buffer pointers?)         */
   
      if (bCaptABuf) 
      {  /* Just finished capturing in the A buffer */
         lpcapt_start_done = (LPREG)AUD_CAPT_A_START_REG;
         lpcapt_stop_done  = (LPREG)AUD_CAPT_A_STOP_REG;
         lpcapt_start_now = (LPREG)AUD_CAPT_B_START_REG;
         lpcapt_stop_now  = (LPREG)AUD_CAPT_B_STOP_REG;
      } 
      else 
      {  /* Just finished capturing in the B buffer */
         lpcapt_start_done = (LPREG)AUD_CAPT_B_START_REG;
         lpcapt_stop_done  = (LPREG)AUD_CAPT_B_STOP_REG;
         lpcapt_start_now = (LPREG)AUD_CAPT_A_START_REG;
         lpcapt_stop_now  = (LPREG)AUD_CAPT_A_STOP_REG;
      } /* end else capturing in the B buffer */
      if (!(BETWEEN(lpcapt_start_now,lpcapt_status,lpcapt_stop_now,NORMAL_SAFETY))) 
      {
         /* Can't change start/stop if processing in wrong buffer */
         PcmCaptWrngBuf++;
         isr_trace_new(TRACE_LEVEL_2 | TRACE_AUD, "CAPT ISR: Pointer in wrong buffer, cnt=%d\n", PcmCaptWrngBuf, 0);
         /* Not sure of recovery processing here. */
      } 
      else if(ps_buf_handle_capt_head->ps_next_buf_handle == NULL)
      {
         /* Correct buffer, but not a spare buffer to use. */
         PcmCaptNoSpare++;
         isr_trace_new(TRACE_LEVEL_2 | TRACE_AUD, "CAPT ISR: First one is corrupt? cnt=%d\n", PcmCaptNoSpare, 0);
         /* Not sure of recovery precessing here. */
      } 
      else if (ps_buf_handle_capt_head->ps_next_buf_handle->ps_next_buf_handle == NULL) 
      {
         /* Correct buffer, but not a spare buffer to use. */
         PcmCaptNoSpare++;
         isr_trace_new(TRACE_LEVEL_2 | TRACE_AUD, "CAPT ISR: No spare buffer to replace filled one, cnt=%d\n", PcmCaptNoSpare, 0);
         /* Not sure of recovery precessing here. */
      } 
      else 
      {  /* it is in the correct buffer, and we have a spare to use.
          * Move capt head buffer to pcm tail. 
          */
         move_pcm_buffer(&ps_buf_handle_capt_head,
                         &ps_buf_handle_capt_tail,
                         &ps_buf_handle_pcm_head,
                         &ps_buf_handle_pcm_tail); 
                        
         if (((int)(ps_buf_handle_capt_head->ps_next_buf_handle->pui32_buf_start)) < 0x10000000) {
           isr_trace_new(TRACE_LEVEL_2 | TRACE_AUD, 
                         "CAPT ISR: Writing small value of %8.8x\n", 
                         (int)(ps_buf_handle_capt_head->ps_next_buf_handle->pui32_buf_start), 
                         0);
         }

         /* At this point, ps_buf_handle_capt_head points to the buffer
          * descriptor for the buffer currently being captured into.
          * Replace the hardware pointers for the buffer we just
          * captured to point to the start of the buffer after the one
          * pointed to by the head. Set the interrupt pointer to 
          * the same place so it will fire after the current buffer
          * has been captured. 
          */
/*         lpcapt_start_done->Address = (int)(ps_buf_handle_capt_head->ps_next_buf_handle->pui32_buf_start);
           lpcapt_stop_done->Address  = (int)(ps_buf_handle_capt_head->ps_next_buf_handle->pui32_buf_stop);
           lpcapt_int_addr->Address   = (int)(ps_buf_handle_capt_head->ps_next_buf_handle->pui32_buf_start);
*/
         *lpcapt_start_done = MAKE_AUD_ADDRESS((int)(ps_buf_handle_capt_head->ps_next_buf_handle->pui32_buf_start));
         *lpcapt_stop_done  = MAKE_AUD_ADDRESS((int)(ps_buf_handle_capt_head->ps_next_buf_handle->pui32_buf_stop));
         *lpcapt_int_addr   = 0x04 + MAKE_AUD_ADDRESS((int)(ps_buf_handle_capt_head->ps_next_buf_handle->pui32_buf_start));
         /* Note: add 4 (one word) to get non-quadword alignment */
         /* Note which set of regs were used */
         ps_buf_handle_capt_head->ps_next_buf_handle->b_aregs_used = bCaptABuf;
         /* Toggle flag for next IRQ */
         bCaptABuf = !bCaptABuf;   
         /* wake up the PCM mix task */
         sem_put(cnxt_pcm_sem_id);
      
      } 
  
      /* Now clear the capture address interrupt bit */
      /* and read it back to ensure it is flushed to the hardware before exiting the ISR */
      *AudCaptIntStatus = 0x01;
      if (*AudCaptIntStatus & 0x01)
      {
         *AudCaptIntStatus = 0x01; /* should never happen               */
                                   /* if statement is to flush register */
      } 

   } 


    if(*AudCaptIntStatus & 0x02) {  
        NOTIFY_UNEXPECTED_ISR("Capture FIFO Overrun", CaptFIFOOverrun);
        *AudCaptIntStatus = 0x02;   
    }
    if(*AudCaptIntStatus & 0x04) {  
        NOTIFY_UNEXPECTED_ISR("Capture FIFO Underrun", CaptFIFOUnderrun);
        *AudCaptIntStatus = 0x04;   
    }
    if(*AudCaptIntStatus & 0xFFFFFFF8) {  
        NOTIFY_UNEXPECTED_ISR("Capture Unknown Interrupt", CaptInvalidInt);
        *AudCaptIntStatus = 0xFFFFFFF8;   
    }
    
   return RC_ISR_HANDLED;
}

/********************************************************************/
/* FUNCTION:    arm_audio_play_isr                                  */
/*                                                                  */
/* PARAMETERS:  interrupt_ID : Not used                             */
/*              isFIQ        : Not used                             */
/*              previous     : Not used                             */
/*                                                                  */
/* DESCRIPTION: This is the ISR that happens when the Play          */
/*              B buffer is empty (So the Capture B buffer is full  */
/*              as well). The Capture B buffer needs to be mixed    */
/*              with PCM data and moved to the Play B buffer.       */
/*                                                                  */
/* RETURNS:     None.                                               */
/********************************************************************/
int arm_audio_play_isr( u_int32 interrupt_ID, bool isFIQ, PFNISR *previous )
{

    LPREG lpplay_start_done;
    LPREG lpplay_start_now;
    LPREG lpplay_stop_done;
    LPREG lpplay_stop_now;

#ifdef DEBUG
    pcm_play_isr_cnt++;
#endif 

    /* Is this the Address interrupt? */
    if(*AudPlayIntStatus & 0x01) {  

      /* Don't clear the interrupt bit here as we do things in here that */
      /* can cause it to become set (adjusting buffer pointers?)         */
   
      if (bPlayABuf) {  
        lpplay_start_done = (LPREG)AUD_PLAY_A_START_REG;
        lpplay_stop_done  = (LPREG)AUD_PLAY_A_STOP_REG;
        lpplay_start_now = (LPREG)AUD_PLAY_B_START_REG;
        lpplay_stop_now  = (LPREG)AUD_PLAY_B_STOP_REG;
      } else {          
        lpplay_start_done = (LPREG)AUD_PLAY_B_START_REG;
        lpplay_stop_done  = (LPREG)AUD_PLAY_B_STOP_REG;
        lpplay_start_now = (LPREG)AUD_PLAY_A_START_REG;
        lpplay_stop_now  = (LPREG)AUD_PLAY_A_STOP_REG;
      } 
      
      if (!(BETWEEN(lpplay_start_now,lpplay_status,lpplay_stop_now,NORMAL_SAFETY))) {
        /* Can't change start/stop if processing in wrong buffer */
        PcmPlayWrngBuf++;
        isr_trace_new(TRACE_LEVEL_2 | TRACE_AUD, "PLAY ISR: Pointer in wrong buffer, cnt=%d\n", PcmPlayWrngBuf, 0);
        /* Not sure of recovery processing here. */
      } else if (ps_buf_handle_play_head->ps_next_buf_handle->ps_next_buf_handle == NULL) {
        /* Correct buffer, but not a spare buffer to use.*/
        PcmPlayNoSpare++;
        isr_trace_new(TRACE_LEVEL_2 | TRACE_AUD, "PLAY ISR: No spare buffer to replace filled one, cnt=%d\n", PcmPlayNoSpare, 0);
        /* Not sure of recovery precessing here. */
      } else { /* it is in the correct buffer, and we have a spare to use.
                  Move buffer from play head to capture tail.*/
        move_pcm_buffer(&ps_buf_handle_play_head, 
                        &ps_buf_handle_play_tail,
                        &ps_buf_handle_capt_head,
                        &ps_buf_handle_capt_tail);
        /* now set up start, stop and irq, and a/b of 2nd.*/
        *lpplay_start_done = MAKE_AUD_ADDRESS((int)(ps_buf_handle_play_head->ps_next_buf_handle->pui32_buf_start));
        *lpplay_int_addr = *lpplay_start_done + 0x04 ;
        /* Note: add 4 (one word) to get non-quadword alignment */
        *lpplay_stop_done  = MAKE_AUD_ADDRESS((int)(ps_buf_handle_play_head->ps_next_buf_handle->pui32_buf_stop));
        ps_buf_handle_play_head->ps_next_buf_handle->b_aregs_used = bPlayABuf;
        bPlayABuf = !bPlayABuf;   /* toggle flag for next irq*/
      }
      
      /* Now clear the play address interrupt bit */
      /* and read it back to ensure it is flushed to the hardware before exiting the ISR */
      *AudPlayIntStatus = 0x01;
      if (*AudPlayIntStatus & 0x01)
      {
         *AudPlayIntStatus = 0x01; /* should never happen               */
                                   /* if statement is to flush register */
      } 

    } 


    if(*AudPlayIntStatus & 0x02) {  
        NOTIFY_UNEXPECTED_ISR("Play FIFO Overrun", PlayFIFOOverrun);
        *AudPlayIntStatus = 0x02; 
    }
    if(*AudPlayIntStatus & 0x04) { 
        NOTIFY_UNEXPECTED_ISR("Play FIFO Underrun", PlayFIFOUnderrun);
        *AudPlayIntStatus = 0x04;   
    }
    if(*AudPlayIntStatus & 0xFFFFFFF8) {    
        NOTIFY_UNEXPECTED_ISR("Play Unknown Interrupt", PlayInvalidInt);
        *AudPlayIntStatus = 0xFFFFFFF8;     
    }
    
   return RC_ISR_HANDLED;
}   
/********************************************************************/
/* FUNCTION:    SuperMix                                            */
/*                                                                  */
/* PARAMETERS:  ps_MPEG     Buffer Descriptor for MPEG audio        */
/*              ps_UserPCM  Buffer Descriptor for PCM SRC data      */
/*                          May be NULL if nothing to mix           */
/*              ps_Dest     Destination buffer descriptor           */
/*                                                                  */
/* DESCRIPTION: This routine merge the SRC and MPEG data into a     */
/*              destination buffer, taking into account all outside */
/*              influences (MPEG Channel Mode, MPEG mix level, MPEG */
/*              volume)                                             */
/*                                                                  */
/* RETURNS:     None.                                               */
/********************************************************************/
void SuperMix(volatile cnxt_pcm_buf_handle_t *ps_MPEG, 
              volatile cnxt_pcm_buf_handle_t *ps_UserPCM,
              volatile cnxt_pcm_buf_handle_t *ps_Dest)
{
    int32 Channel;
    int32 OData1, OData2;
    int32 PCMData1, PCMData2;    // MPEG: process left and right channel at same time
    int32 UPCMDataL, UPCMDataR;  // User PCM left and right channel
    int32 CurrentAttenShift1, CurrentAttenShift2; // Shift for both channels
    int32 CurrentAttenMulti1, CurrentAttenMulti2; // Multiply for both channels
    int32 *pStartMPEG, *pStartPCM = 0, *pDest;
    bool  bMixPCM = FALSE;
    int i;

    /* Get data pointers for MPEG and destination */
    pStartMPEG = (int32 *)(ps_MPEG->pui32_buf_start);
    pDest      = (int32 *)(ps_Dest->pui32_buf_start);

    /* Determine if there is a PCM SRC buffer to mix with */
    if (ps_UserPCM)
    {
       bMixPCM = TRUE;
       pStartPCM  = (int32 *)(ps_UserPCM->pui32_buf_start);
    } /* endif */
    
    
#ifdef DEBUG
    MoveAndMixCnt++;
#endif // DEBUG
    if (cnxt_force_mix > 0) {
      cnxt_force_mix--;
    }

    // Is the first one right channel
    Channel = (*pStartMPEG & (int32)0x80000000);

    // Get the left and right channel shift and mult values
    if(Channel) { // 1st one is right channel
      CurrentAttenShift1 = MPEGRightAttenShift; 
      CurrentAttenShift2 = MPEGLeftAttenShift;
      CurrentAttenMulti1 = MPEGRightAttenMulti; 
      CurrentAttenMulti2 = MPEGLeftAttenMulti;
    } else { // !Channel    // 2nd one is right channel
      CurrentAttenShift2 = MPEGRightAttenShift; 
      CurrentAttenShift1 = MPEGLeftAttenShift;
      CurrentAttenMulti2 = MPEGRightAttenMulti; 
      CurrentAttenMulti1 = MPEGLeftAttenMulti;
    }
    
    /* loop through all stereo samples */
    for (i = 0; i < (ARM_AUD_BUFF_SIZE >> 3) ; i++)
    {

      // Read both channel values, advance the read pointer twice.
      PCMData1 = *pStartMPEG & 0x000FFFFF; /* get 20 bit value */
      pStartMPEG++;
      PCMData2 = *pStartMPEG & 0x000FFFFF; /* get 20 bit value */
      pStartMPEG++;


      // Shift and mult the data, or set to 0 if shift is large
      if (CurrentAttenShift1 < 20) {
          if (PCMData1 & 0x00080000)
              PCMData1 |= (uint32)0xFFF00000; /* sign extend to 32 bits */
          PCMData1 *= CurrentAttenMulti1; 
          PCMData1 >>= CurrentAttenShift1; 
          PCMData1 &= 0x000FFFFF;
      } else { // (CurrentAttenShift1 >= 20)
          PCMData1 = 0;    // So neg values go to 0 instead of -1
      } // (CurrentAttenShift1 >= 20)
      if (CurrentAttenShift2 < 20) {
          if (PCMData2 & 0x00080000)
              PCMData2 |= (uint32)0xFFF00000; /* sign extend to 32 bits */
          PCMData2 *= CurrentAttenMulti2; 
          PCMData2 >>= CurrentAttenShift2; 
          PCMData2 &= 0x000FFFFF;
      } else { // (CurrentAttenShift2 >= 20)
          PCMData2 = 0;    // So neg values go to 0 instead of -1
      } // (CurrentAttenShift2 >= 20)

      /* If the MPEG Channel mode is non-zero (not just plain stereo) */
      /* then do extra processing                                     */
      if (s_cnxt_pcm_status.echannel_mode)
      {
         switch (s_cnxt_pcm_status.echannel_mode)
         {
            int32 tmp;
            case PCM_CM_STEREO_SWAP:
               tmp = PCMData1;
               PCMData1 = PCMData2;
               PCMData2 = tmp;
               break;
            case PCM_CM_MONO_MIX:
               if (PCMData1 & 0x00080000)
                   PCMData1 |= (int32)0xFFF00000; /* sign extend to 32 bits */
               if (PCMData2 & 0x00080000)
                   PCMData2 |= (int32)0xFFF00000; /* sign extend to 32 bits */
               PCMData1 = ((PCMData1 + PCMData2) >> 1) & 0xfffff;
               PCMData2 = PCMData1;
               break;
            case PCM_CM_MONO_LEFT:
               if (Channel) /* 1st one is right */
               {
                  PCMData1 = PCMData2;
               } else {
                  PCMData2 = PCMData1;
               } /* endif */
               break;
            case PCM_CM_MONO_RIGHT:
               if (Channel) /* 1st one is right */
               {
                  PCMData2 = PCMData1;
               } else {
                  PCMData1 = PCMData2;
               } /* endif */
               break;
            default:   
               break;
         } /* endswitch on channel mode */
      
      } /* endif not just plain stereo */

      /* Now mix with the PCM buffer if present */
      if (bMixPCM)
      {
         /* Prepare MPEG samples for some math */
         if (PCMData1 & 0x00080000)
             PCMData1 |= (uint32)0xFFF00000; /* sign extend to 32 bits */
         if (PCMData2 & 0x00080000)
             PCMData2 |= (uint32)0xFFF00000; /* sign extend to 32 bits */

         switch (s_cnxt_pcm_status.s_format.e_mpeg_level)
         {
            case MPEG_LEVEL_MUTE:
               PCMData1 = PCMData2 = 0;
               break;
            case MPEG_LEVEL_QTR:
               PCMData1 >>= 2;
               PCMData2 >>= 2;
               break;
            case MPEG_LEVEL_HALF:
               PCMData1 >>= 1;
               PCMData2 >>= 1;
               break;
            default:
               break;
         } /* endswitch */

         /* Get left and right samples to mix */
         UPCMDataL = *(pStartPCM++);
         if (s_cnxt_pcm_status.s_format.b_stereo)
         {
            UPCMDataR = *(pStartPCM++);
         } 
         else 
         {
            UPCMDataR = UPCMDataL;
         } /* endif */
      
         /* Now add in the user PCM data */
         /* Note that this is not a saturated add, and it will sound */
         /* terrible if it is overdriven */
         if (Channel) /* 1st one is right */
         {
            OData1 = PCMData1 + UPCMDataR;
            OData2 = PCMData2 + UPCMDataL;
         } 
         else 
         {
            OData1 = PCMData1 + UPCMDataL;
            OData2 = PCMData2 + UPCMDataR;
         } /* endif Channel */
        
      }
      else
      {
         OData1 = PCMData1;
         OData2 = PCMData2;
      } /* endif bMixPCM */
      
      // Get the sign and upper bits correct.
      OData1 &= 0x000FFFFF;
      OData2 &= 0x000FFFFF;
      if(OData1 &  0x00080000) {
         OData1 |= 0x7FF00000;
      }
      if(OData2 &  0x00080000) {
         OData2 |= 0x7FF00000;
      }
      
      // If the Channel bit was set on the first value,
      // set it again on that first value.
      // Otherwise, set the channel bit on the second value.
      if (Channel)  {
        OData1 |= Channel;
      } else {
        OData2 |= (int32)0x80000000;
      }

      // Write both values to the output buffer, and advance the
      // write pointer twice.
      *pDest = OData1;
      pDest++;
      *pDest = OData2;
      pDest++;
    } /* endfor all stereo samples */
    return;
    
} /* SuperMix */

/********************************************************************/
/* FUNCTION:    GetNewPcmBuffer                                     */
/*                                                                  */
/* PARAMETERS:  NONE.                                               */
/*                                                                  */
/* DESCRIPTION: This routine alloc a cnxt_pcm_buf_handle_t and      */
/*              a PCM buffer, and initialize the values of the      */
/*              handle.                                             */
/*                                                                  */
/* RETURNS:     Pointer to the new cnxt_pcm_buf_handle_t            */
/********************************************************************/
#ifdef PCM_DEBUG
int PcmBufList[16];
int PcmBufListIndex = 0;
#endif // def PCM_DEBUG
cnxt_pcm_buf_handle_t *GetNewPcmBuffer(void)
{
    cnxt_pcm_buf_handle_t *ps_return_val;

    // allocate and check the buffer handler
    if ((ps_return_val = (cnxt_pcm_buf_handle_t *)(mem_malloc(sizeof(cnxt_pcm_buf_handle_t)))) == NULL) {
        fatal_exit(ERROR_FATAL | RC_AUD_MEMORY);
        return(NULL);
    }
#ifdef PCM_DEBUG
    PcmBufList[PcmBufListIndex++] = (int)ps_return_val;
#endif // def PCM_DEBUG
    // allocate and check the pcm buffer. Note that this buffer
    // has to be non-cached, since the hw and arm both access the
    // data in it.
    if ((ps_return_val->pui32_buf_start_malloc = (u_int32 *)(mem_nc_malloc(ARM_AUD_BUFF_SIZE+ARM_AUD_BUFF_EXCESS))) == NULL) {
        fatal_exit(ERROR_FATAL | RC_AUD_MEMORY);
        return(NULL);
    }
    
    ps_return_val->pui32_buf_start      = (u_int32 *) (((u_int32)ps_return_val->pui32_buf_start_malloc 
                                           + ARM_AUD_BUFF_EXCESS) & 0xfffffff0); /* quad word aligned */
    ps_return_val->pui32_buf_stop       = ps_return_val->pui32_buf_start + (ARM_AUD_BUFF_SIZE >> 2);
    ps_return_val->ui32_size_in_bytes   = ARM_AUD_BUFF_SIZE;
    ps_return_val->b_customer_buf       = FALSE;
    ps_return_val->b_aregs_used         = FALSE;
    ps_return_val->ps_next_buf_handle   = NULL;
    memset(ps_return_val->pui32_buf_start, 0, ARM_AUD_BUFF_SIZE);   /* zero out buffer, ignoring excess bytes. */
    
#ifdef PCM_DEBUG
    trace_new(TRACE_LEVEL_2 | TRACE_AUD, "GetNewPcmBuffer: Handle=%8.8x, Start=%8.8x, Stop=%8.8x\n", ps_return_val, ps_return_val->pui32_buf_start, ps_return_val->pui32_buf_stop);
#endif // def PCM_DEBUG

    return(ps_return_val);
}   // end GetNewPcmBuffer()

/********************************************************************/
/* FUNCTION:    pcmDoIdleState                                      */
/*                                                                  */
/* PARAMETERS:  None                                                */
/*                                                                  */
/* DESCRIPTION: Moves a buffer from the head of the PCM pool to the */
/*              tail of the "done" pool without mixing user PCM.    */
/*                                                                  */
/* RETURNS:     None.                                               */
/********************************************************************/
void pcmDoIdleState(void)
{
    bool kstate;
    /* Check we have a buffer to do */
    if (ps_buf_handle_pcm_head != NULL) 
    {
       /* Check to see if we have to change the MPEG samples */
       if ((s_cnxt_pcm_status.ui8_mpeg_vol_left != CNXT_PCM_MAX_VOLUME) ||
           (s_cnxt_pcm_status.ui8_mpeg_vol_right != CNXT_PCM_MAX_VOLUME) ||
           (s_cnxt_pcm_status.echannel_mode != PCM_CM_STEREO) )
       {
          /* Perform volume/channel adjustment without PCM mix */
          SuperMix(ps_buf_handle_pcm_head,   /* MPEG        */
                   NULL,                     /* No User PCM */
                   ps_buf_handle_pcm_head);  /* Destination */                     
                   
       } /* endif have to change MPEG samples */
       
       /* move buffer from pcm head to done tail (moves pointers only) */
       kstate = critical_section_begin();
       move_pcm_buffer(&ps_buf_handle_pcm_head,
                       &ps_buf_handle_pcm_tail,
                       &ps_done_head_ptr,
                       &ps_done_tail_ptr);
       critical_section_end(kstate);
       
    }
    else 
    {
       IdleStateDidNothing++;
    } /* endif we have a pcm buffer to process */
    
    return;
    
} /* pcmDoIdleState */

/********************************************************************/
/* FUNCTION:    pcmDoPlayingState                                   */
/*                                                                  */
/* PARAMETERS:  None                                                */
/*                                                                  */
/* DESCRIPTION: Does all the processing associated with the play    */
/*              state, including volume, sample rate convert and    */
/*              mixing                                              */
/*                                                                  */
/* RETURNS:     None.                                               */
/********************************************************************/
void pcmDoPlayingState(void)
{
   u_int32 SamplesToDeQ;
   bool kstate;
   
   /* If there is a destination buffer available to us */
   if (ps_buf_handle_pcm_head != NULL)
   {
      /* If there is any SRC data available to mix */
      #if (PCM_DISABLE_SRC == YES)
      if (!PCM20_CQ_EmptyQ(&PCMInputQ))
      #else /* (PCM_DISABLE_SRC != YES) */
      if (!PCM20_CQ_EmptyQ(&PCMSRCQ))
      #endif /* (PCM_DISABLE_SRC == YES) */
      {
         /* Samples to DeQueue is whole buffer if stereo, half if mono */
         SamplesToDeQ = ARM_AUD_BUFF_SIZE >> 2;
         if (!s_cnxt_pcm_status.s_format.b_stereo)
         {
            /* Half the samples if mono */
            SamplesToDeQ >>= 1;
         } 

         if(gbPCMPlayNoMpeg)
         {
            /* Dequeue a buffer to mix into MPEG */
#if USE_UNOPTIMIZED_PCM_SRC == YES
            PCM20_CQ_DequeueBuffer(
#else /* !USE_UNOPTIMIZED_PCM_SRC */
            PCM20_CQ_FastDequeueBuffer(
#endif /* USE_UNOPTIMIZED_PCM_SRC */
                                   #if (PCM_DISABLE_SRC == YES)
                                   &PCMInputQ,
                                   #else /* !PCM_DISABLE_SRC */
                                   &PCMSRCQ,
                                   #endif /* PCM_DISABLE_SRC */
                                   (PCM20 *)
                                    (ps_buf_handle_pcm_head->pui32_buf_start),
                                   SamplesToDeQ);
         }
         else
         {
            /* Dequeue a buffer to mix into MPEG */
#if USE_UNOPTIMIZED_PCM_SRC == YES
            PCM20_CQ_DequeueBuffer(
#else /* !USE_UNOPTIMIZED_PCM_SRC */
            PCM20_CQ_FastDequeueBuffer(
#endif /* USE_UNOPTIMIZED_PCM_SRC */
                                   #if (PCM_DISABLE_SRC == YES)
                                   &PCMInputQ,
                                   #else /* (PCM_DISABLE_SRC != YES) */
                                   &PCMSRCQ,
                                   #endif /* (PCM_DISABLE_SRC == YES) */
                                   (PCM20 *)(ps_SRCBuff->pui32_buf_start), 
                                   SamplesToDeQ);

            /* Mix SRC samples back into MPEG(PCM) buffer */
            SuperMix(ps_buf_handle_pcm_head,   /* MPEG        */
                     ps_SRCBuff,               /* User PCM    */
                     ps_buf_handle_pcm_head);  /* Destination */                     
         }

         /* move buffer from pcm head to done tail (moves pointers only) */
         kstate = critical_section_begin();
         move_pcm_buffer(&ps_buf_handle_pcm_head,
                         &ps_buf_handle_pcm_tail,
                         &ps_done_head_ptr,
                         &ps_done_tail_ptr);
         critical_section_end(kstate);
                              
      } else {
         /* No SRC data available */
         PlayingStateCalledIdle++;
         pcmDoIdleState();
      } /* endif SRC data available */     
   } 
   else 
   {
      PlayingStateDidNothing++;
   } /* endif destination buffer available */
   
// Start the conversion of the buffer.
// when full buffer of converted stuff, mix w pcm stuff
// when done mixing, put in play buffer
// Meanwhile, send pcm to play as per idle
// keep in mind asking for more data when data left below threshold
// and where/when do we release used buffer.
// the conversion needs to change to the pcm format at the
// proper sample rate and then adjust the volume before
// the mixing. The mixing needs to take into account
// the mixing level. If the level is no Mpeg, then
// can we use pointer copy instead of full copy.
// How transition to pause and idle and play_done states.

} /* pcmDoPlayState */

/********************************************************************/
/* FUNCTION:    pcmTryToStop                                        */
/*                                                                  */
/* PARAMETERS:  None                                                */
/*                                                                  */
/* DESCRIPTION: Attempts to transition into the IDLE state if the   */
/*              SRC task is IDLE. If not, flush the SRC Q           */
/*                                                                  */
/* RETURNS:     None.                                               */
/********************************************************************/
void pcmTryToStop(void)
{
   bool kstate;
   
   #if (PCM_DISABLE_SRC == YES)
   kstate = critical_section_begin();
   s_cnxt_pcm_status.e_state = IDLE;
   critical_section_end(kstate);
   PCM20_CQ_ClearQ(&PCMInputQ);
   #else /* (PCM_DISABLE_SRC != YES) */
   if (eSRCTaskState == IDLE)
   {
      kstate = critical_section_begin();
      s_cnxt_pcm_status.e_state = IDLE;
      critical_section_end(kstate);
      PCM20_CQ_FlushQ(&PCMSRCQ);
   } 
   else 
   {
      PCM20_CQ_FlushQ(&PCMSRCQ);
      
   } /* endif SRC Task IDLE */
   #endif /* (PCM_DISABLE_SRC == YES) */
}

/********************************************************************/
/* FUNCTION:    move_pcm_buffer                                     */
/*                                                                  */
/* PARAMETERS:  FromHeadPtr - Buffer descriptor pointer for head of */
/*                            "From" pool. Must NOT be NULL         */
/*              FromTailPtr - Buffer descriptor pointer for tail of */
/*                            "From" pool                           */
/*              ToHeadPtr -   Buffer des riptor pointer for head of */
/*                            "To" pool. May be NULL if empty.      */
/*              ToTailPtr -   Buffer descriptor pointer for tail of */
/*                            "To" pool. May not be NULL if         */
/*                            ToHeadPtr is not NULL                 */
/*                                                                  */
/* DESCRIPTION: Moves a buffer from the head of one set of buffers  */
/*              to the tail of the other set of buffers. If the     */
/*              "To" buffer chain is empty, set both head and tail  */
/*              to point to the buffer. If the result of taking the */
/*              "from" buffer is an empty "From" list, fix up head  */
/*              and tail to be NULL.                                */
/*                                                                  */
/* RETURNS:     None.                                               */
/********************************************************************/
static void move_pcm_buffer(volatile cnxt_pcm_buf_handle_t **pFromHeadPtr,
                            volatile cnxt_pcm_buf_handle_t **pFromTailPtr,
                            volatile cnxt_pcm_buf_handle_t **pToHeadPtr,
                            volatile cnxt_pcm_buf_handle_t **pToTailPtr)
{                    
    if (*pToHeadPtr == NULL) 
    {                                              
      *pToHeadPtr = *pToTailPtr = *pFromHeadPtr;
    } else {
      *pToTailPtr = ((*pToTailPtr)->ps_next_buf_handle = *pFromHeadPtr);
    }
    if ((*pFromHeadPtr = (*pFromHeadPtr)->ps_next_buf_handle) == NULL) {
      *pFromTailPtr = NULL;
    }
    (*pToTailPtr)->ps_next_buf_handle = NULL;
    return;
} /* move_pcm */

