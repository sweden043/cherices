/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                    Conexant Systems Inc. (c) 1998-2003                   */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       PCMINIT.C
 *
 *
 * Description:    Initialization and utility source file for 
 *                 colorado pcm driver
 *
 *
 * Author:         Matthew W. Korte
 *
 ****************************************************************************/
/* $Header: PcmInit.c, 15, 6/19/03 2:02:16 PM, Bobby Bradford$
 ****************************************************************************/

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
#define PCM_PRIMARY
#include "pcmpriv.h"

/* Start Defines */
/* End   Defines */

/* Start Declarations */
/* End   Declarations */

/* Start Prototypes */
/* End   Prototypes */

/* Start Extern Prototypes */
/* End   Extern Prototypes */

/*******************************************************************************

   SetUpPCM_FIFOs()

   Description:
      Configures the PCM capture and PCM playback FIFO thresholds.

   Parameters
      None

   Return Value
      None

   Notes
      None

*******************************************************************************/
void SetUpPCM_FIFOs(void)
{
   LPREG lpplay_control = (LPREG)AUD_PLAY_CONTROL_REG;
   LPREG lpcapt_control = (LPREG)AUD_CAPT_CONTROL_REG;
   u_int32 play_fifo_ctrl;
   u_int32 capt_fifo_ctrl;

   /* Just create values to write to registers, no need to preserve prior */
   
   /* Set normal threshold to 6/16 empty */   
   play_fifo_ctrl = SET_FIELD(0,
                              10,      /* 10 = 6/16 empty */
                              AUD_FIFO_CONTROL_NORM_THRESHOLD_MASK, 
                              AUD_FIFO_CONTROL_NORM_THRESHOLD_SHIFT);
                              
   /* Set critical threshold to 8/16 empty */   
   play_fifo_ctrl = SET_FIELD(play_fifo_ctrl,
                              8,      /* 8 = 8/16 empty */
                              AUD_FIFO_CONTROL_CRIT_THRESHOLD_MASK, 
                              AUD_FIFO_CONTROL_CRIT_THRESHOLD_SHIFT);

   /* Set normal threshold to 9/16 full */   
   capt_fifo_ctrl = SET_FIELD(0,
                              9,      /* 9 = 9/16 full */
                              AUD_FIFO_CONTROL_NORM_THRESHOLD_MASK, 
                              AUD_FIFO_CONTROL_NORM_THRESHOLD_SHIFT);
                              
   /* Set critical threshold to 11/16 full */   
   capt_fifo_ctrl = SET_FIELD(capt_fifo_ctrl,
                              11,     /* 11 = 11/16 full */
                              AUD_FIFO_CONTROL_CRIT_THRESHOLD_MASK, 
                              AUD_FIFO_CONTROL_CRIT_THRESHOLD_SHIFT);

   /* Beware:  reading FIFO control register will reset the
   * system memory normal event and system memory critical event registers */
   *lpplay_control = play_fifo_ctrl;
   *lpcapt_control = capt_fifo_ctrl;
   
} /* SetUpPCM_FIFOs */

/*******************************************************************************

   StartPCMCaptureAndPlayback()

   Description:
      Enables PCM capture and PCM playback.

   Parameters
      None

   Return Value
      None

   Notes
      The weird stuff at the end is to make sure that the capture buffer index
      and the playback buffer index are at the same relative offset within their
      respective buffers -- this is essential for the way the ISRs are coded.

*******************************************************************************/
void StartPCMCaptureAndPlayback(void)
{
   LPREG lpchan_ctrl = (LPREG) AUD_CHANNEL_CONTROL_REG;
   u_int32 chan_ctrl = 0;
   bool kstate;

   kstate = critical_section_begin();
   /* make sure we're really starting from zero */
   *lpchan_ctrl = chan_ctrl;  /* set CaptEnable, SerialEnable, PlayEnable to 0 */

#if (PCM_AUDIO_TYPE!=PCM_AUDIO_COLORADO)
   /* if feature exists then enable pcm audio mpeg capt (iadatain) atten path */
   /* this allows for hardware atten on capture side of pcm input from decoder */
   /* chan_ctrl = SET_BIT(chan_ctrl, AUD_CHANNEL_CONTROL_MPEG_CAPT_ATTEN_ENABLE); */
   /* *lpchan_ctrl = chan_ctrl; */
#endif

   /* enable capture, play and serial */
   chan_ctrl = SET_BIT(chan_ctrl,AUD_CHANNEL_CONTROL_CAPT_ENABLE);
   chan_ctrl = SET_BIT(chan_ctrl,AUD_CHANNEL_CONTROL_PLAY_ENABLE);
   chan_ctrl = SET_BIT(chan_ctrl,AUD_CHANNEL_CONTROL_SERIAL_ENABLE);

   *lpchan_ctrl = chan_ctrl;

#if defined(FILL_ZEROS)
   StartPCMCnt++;
   /* wait for play to start */
   while(( GET_AUD_ADDRESS(lpplay_status) - 
           GET_AUD_ADDRESS(lpplay_a_start) == 0x00)
      ;
   PlayDifa = GET_PCM_OFFSET(lpplay_a_start,lpplay_status);
   CaptDifa = GET_PCM_OFFSET(lpcapt_a_start,lpcapt_status);
   PCMDifa = CaptDifa - PlayDifa;
   if (PCMDifa > 0x20) {
        BadStartCnt++;
   }
#endif /* defined(FILL_ZEROS) */

   cnxt_force_mix = FORCE_MIX_INIT_VAL; /* Have to start with this many mixes. */
   critical_section_end(kstate);
}   /* end StartPCMCaptureAndPlayback() */

/********************************************************************/
/* FUNCTION:    StopPCMCaptureAndPlayback                           */
/*                                                                  */
/* PARAMETERS:  None.                                               */
/*                                                                  */
/* DESCRIPTION: Disables PCM capture and PCM playback.              */
/*                                                                  */
/* RETURNS:     None.                                               */
/********************************************************************/
void StopPCMCaptureAndPlayback(void)
{
   LPREG lpchan_ctrl =(LPREG)AUD_CHANNEL_CONTROL_REG;

   /* Stop everything (Capture, Serial and Play enables to 0) */
   *lpchan_ctrl = 0;
}

/*******************************************************************************

   SetUpPCM_CaptureAndPlaybackBuffers()

   Description:
      Stores the PCM capture and PCM playback buffer addresses in the
      appropriate registers.

   Parameters
      None

   Return Value
      None

   Notes
      None

*******************************************************************************/
void SetUpPCM_CaptureAndPlaybackBuffers(void)
{
    int i;
    volatile cnxt_pcm_buf_handle_t *ps_ptr;
    /* Get the head of the capture buffers. Then */
    /* get and build the rest of the cature buffers */
    ps_buf_handle_capt_head = ps_ptr = GetNewPcmBuffer();
    for (i = 1; i < INIT_NUM_BUFS; i++) {
        ps_ptr->ps_next_buf_handle = GetNewPcmBuffer();
        ps_ptr = ps_ptr->ps_next_buf_handle;
    } /* end for */
    ps_buf_handle_capt_tail = ps_ptr;
    /* Set up the A capture buffer */
    ps_ptr =  ps_buf_handle_capt_head;
    *lpcapt_a_start = MAKE_AUD_ADDRESS((int)ps_ptr->pui32_buf_start);
    *lpcapt_a_stop  = MAKE_AUD_ADDRESS((int)ps_ptr->pui32_buf_stop);
    ps_ptr->b_aregs_used = TRUE;
    /* Set up the B capture buffer */
    ps_ptr = ps_ptr->ps_next_buf_handle;
    *lpcapt_b_start =  MAKE_AUD_ADDRESS((int)ps_ptr->pui32_buf_start);
    *lpcapt_b_stop  =  MAKE_AUD_ADDRESS((int)ps_ptr->pui32_buf_stop);
    ps_ptr->b_aregs_used = FALSE;

    /* Get the head of the play buffers and 2 more */
    ps_buf_handle_play_head = ps_ptr  = GetNewPcmBuffer();
    ps_ptr->ps_next_buf_handle        = GetNewPcmBuffer();
    ps_ptr->ps_next_buf_handle->ps_next_buf_handle 
                                      = GetNewPcmBuffer();
    ps_buf_handle_play_tail = ps_ptr->ps_next_buf_handle->ps_next_buf_handle->ps_next_buf_handle 
                                      = GetNewPcmBuffer();
    /* Set up the A play buffer */
    *lpplay_a_start =  MAKE_AUD_ADDRESS((int)ps_ptr->pui32_buf_start);
    *lpplay_a_stop  =  MAKE_AUD_ADDRESS((int)ps_ptr->pui32_buf_stop);
    ps_ptr->b_aregs_used = TRUE;
    /* Set up the B play buffer */
    ps_ptr = ps_ptr->ps_next_buf_handle;
    *lpplay_b_start =  MAKE_AUD_ADDRESS((int)ps_ptr->pui32_buf_start);
    *lpplay_b_stop  =  MAKE_AUD_ADDRESS((int)ps_ptr->pui32_buf_stop);
    ps_ptr->b_aregs_used = FALSE;

}   /* end SetUpPCM_CaptureAndPlaybackBuffers() */

/*******************************************************************************

   SetUpAndRegisterPCM_ISRs()

   Description:
      Configures the capture ISR to occur when PCM capture buffer A is full and
      the playback ISR to occur when PCM capture buffer B is full.

   Parameters
      None

   Return Value
      None

   Notes
      None

*******************************************************************************/
void SetUpAndRegisterPCM_ISRs(void)
{
   PFNISR arm_audio_capt_isr_handle, arm_audio_play_isr_handle;

    /* Set the capture and play interrupt addresses as the first byte  */
    /* of the B buffer, which is just after the A buffer is done. */
    *lpcapt_int_addr = 0x04 + MAKE_AUD_ADDRESS((int)(ps_buf_handle_capt_head->ps_next_buf_handle->pui32_buf_start));
    /* Note: add 4 (one word) to get non-quadword alignment */
    bCaptABuf = TRUE;
    *lpplay_int_addr = 0x04 + MAKE_AUD_ADDRESS((int)(ps_buf_handle_play_head->ps_next_buf_handle->pui32_buf_start));
    /* Note: add 4 (one word) to get non-quadword alignment */
    bPlayABuf = TRUE;

   /* register interrupt handlers */
   arm_audio_capt_isr_handle = (PFNISR)arm_audio_capt_isr;
   arm_audio_play_isr_handle = (PFNISR)arm_audio_play_isr;

   /* ARM audio interrupt handler. */
   int_register_isr( INT_AUDIO_CAPT, arm_audio_capt_isr_handle,
      0, 0, &previous_arm_audio_capt_isr );
   int_register_isr( INT_AUDIO_PLAY, arm_audio_play_isr_handle,
      0, 0, &previous_arm_audio_play_isr );

   int_enable(INT_AUDIO_CAPT);
   int_enable(INT_AUDIO_PLAY);
}   /* end SetUpAndRegisterPCM_ISRs() */

/********************************************************************/
/* FUNCTION:    PCMInit                                             */
/*                                                                  */
/* PARAMETERS:  NONE                                                */
/*                                                                  */
/* DESCRIPTION: Sets up the function pointer used to return buffers */
/*              to the user after the driver is finished with them. */
/*              Once a buffer is submitted to the driver, it should */
/*              not be used, destroyed, or modified again until it  */
/*              has been released back to the user.                 */
/*                                                                  */
/* RETURNS:     NONE                                                */
/********************************************************************/
void PCMInit(void)
{
    static bool AlreadyDoneThis = FALSE;
    static u_int32 cnxt_pcm_task_id = 0;    /* Local static for tracking PID */
    #if (PCM_DISABLE_SRC != YES)
    static u_int32 cnxt_src_task_id = 0;    /* Local static for tracking PID */
    #endif /* (PCM_DISABLE_SRC != YES) */
    bool bOK;                              

    if (!AlreadyDoneThis)  {
        pcmStatusInit();                    /* Init the state structure */
        SetUpPCM_FIFOs();
        SetUpPCM_CaptureAndPlaybackBuffers();
        SetUpAndRegisterPCM_ISRs();
        StartPCMCaptureAndPlayback();

        AudMPGVolumeFuncRegister( SetMPEGVolume );
 
        /* Create the PCM Input sample circular queue. 
         * This queue is used to amalgamate the input samples after converting
         * them to the PCM 20 bit format.
         */
        bOK = PCM20_CQ_CreateQ(&PCMInputQ, PCM_INPUTQ_SAMPLES);
        if (!bOK)
        {
           trace_new(TRACE_LEVEL_2 | TRACE_AUD, "PCMInit ERROR: PCMInputQ creation failure\n");
        } 
        else 
        {

           #if (PCM_DISABLE_SRC != YES)
           /* Create circular queue to receive SRC'ed samples */
           /* Worst case is it needs 6 times the number of input samples */
           bOK = PCM20_CQ_CreateQ(&PCMSRCQ, (6 * PCM_INPUTQ_SAMPLES));
           if (!bOK)
           {
              trace_new(TRACE_LEVEL_2 | TRACE_AUD, "PCMInit ERROR: PCMSRCQ creation failure\n");
           } 
           else 
           #endif /* (PCM_DISABLE_SRC != YES) */
           {
              /* Create the mix and merge semaphore */
              cnxt_pcm_sem_id = sem_create(0,"PCMS");
              if (cnxt_pcm_sem_id == 0)
              {
                 trace_new(TRACE_LEVEL_2 | TRACE_AUD, "PCMInit ERROR: cnxt_pcm_sem_id sem creation failure\n");
              } 
              else 
              {
                 /* Create the mix and merge task */
                 cnxt_pcm_task_id = task_create((PFNTASK)cnxt_pcm_task, 
                                                (void *)NULL, 
                                                (void *)NULL, 
                                                PCMM_TASK_STACK_SIZE, 
                                                PCMM_TASK_PRIORITY, 
                                                PCMM_TASK_NAME);
                 if (cnxt_pcm_task_id == 0)  
                 {   
                     trace_new(TRACE_LEVEL_2 | TRACE_AUD, "PCMInit ERROR: cnxt_pcm_task task creation failure\n");
                 }
                 #if (PCM_DISABLE_SRC != YES)
                 else 
                 {
                    /* Create the mix and merge semaphore */
                    cnxt_src_sem_id = sem_create(0,"SRCS");
                    if (cnxt_src_sem_id == 0)
                    {
                       trace_new(TRACE_LEVEL_2 | TRACE_AUD, "PCMInit ERROR: cnxt_src_sem_id sem creation failure\n");
                    } 
                    else 
                    {
                       /* Create the sample rate convert task */
                       cnxt_src_task_id = task_create((PFNTASK)cnxt_src_task, 
                                                      (void *)NULL, 
                                                      (void *)NULL, 
                                                      SSRC_TASK_STACK_SIZE, 
                                                      SSRC_TASK_PRIORITY, 
                                                      SSRC_TASK_NAME);
                       if (cnxt_src_task_id == 0)  
                       {   
                           trace_new(TRACE_LEVEL_2 | TRACE_AUD, "PCMInit ERROR: cnxt_src_task task creation failure\n");
                       } /* endif SRC task created OK */
                    } /* endif SRC Sem created OK */
                 } /* endif PCM task created OK */
                 #endif /* (PCM_DISABLE_SRC != YES) */
              } /* endif mix and merge semaphore OK */
           } /* endif SRC queue created OK */
        } /* endif input queue created OK */
        
        AlreadyDoneThis = TRUE;
    } /* endif !AlreadyDoneThis */

    return;
    
}   /* PCMInit */

void pcmStatusInit()
{
    s_cnxt_pcm_status.e_state                = IDLE;
    s_cnxt_pcm_status.ui8_pcm_vol_left       = CNXT_PCM_MAX_VOLUME;
    s_cnxt_pcm_status.ui8_pcm_vol_right      = CNXT_PCM_MAX_VOLUME;
    s_cnxt_pcm_status.ui8_mpeg_vol_left      = CNXT_PCM_MAX_VOLUME;
    s_cnxt_pcm_status.ui8_mpeg_vol_right     = CNXT_PCM_MAX_VOLUME;
    s_cnxt_pcm_status.echannel_mode          = PCM_CM_STEREO;
   // s_cnxt_pcm_status.echannel_mode          = PCM_CM_MONO_MIX;
    s_cnxt_pcm_status.s_format.b_signed      = TRUE;
    s_cnxt_pcm_status.s_format.e_rate        = SAMPLE_RATE_48K;
    s_cnxt_pcm_status.s_format.b_stereo      = TRUE;
    s_cnxt_pcm_status.s_format.e_bps         = BITS_PER_SAMPLE_8;
    s_cnxt_pcm_status.s_format.e_mpeg_level  = MPEG_LEVEL_HALF;
    s_cnxt_pcm_status.ui32_play_threshold    = 0;
    s_cnxt_pcm_status.ui32_request_threshold = 1;
    s_cnxt_pcm_status.pfn_data_request       = NULL;
    s_cnxt_pcm_status.pfn_event_callback     = NULL;
    s_cnxt_pcm_status.p_event_info           = NULL;
    s_cnxt_pcm_status.e_last_error           = PCM_ERROR_OK;
}

/*
$Log: 
 15   mpeg      1.14        6/19/03 2:02:16 PM     Bobby Bradford  SCR(s) 6796 
       :
       Add offset of 4 to interrupt address register values to get non-
       quadword alignment (to prevent multiple interrupts from occuring)
       
 14   mpeg      1.13        6/2/03 3:32:08 PM      Senthil Veluswamy SCR(s) 
       6666 :
       1) Updated File header (Copyright info, etc)
       2) Move $Log to bottom of file
       3) Removed // comments
       
 13   mpeg      1.12        6/2/03 3:23:22 PM      Senthil Veluswamy SCR(s) 
       6666 :
       Changed the Play FIFO Threshold values as per Dave A
       
 12   mpeg      1.11        4/21/03 8:34:06 PM     Senthil Veluswamy SCR(s) 
       6066 6065 :
       Changed PCM_DISABLE_SRC to take values YES and NO instead of just being 
       defined.
       
 11   mpeg      1.10        4/21/03 7:59:00 PM     Senthil Veluswamy SCR(s) 
       6066 6065 :
       Modifications to compile time disable SRC and run time disable Mixing if
        the MPEG level is set to MUTE.
       
 10   mpeg      1.9         2/14/03 11:31:38 AM    Dave Aerne      SCR(s) 5510 
       :
       comment out, for now, code to enable mpeg capt atten path
       
 9    mpeg      1.8         2/13/03 2:24:22 PM     Dave Aerne      SCR(s) 5492 
       :
       added code to enable mpeg capt atten path for non-colorado devices
       through setting of pcm channel control bit
       
 8    mpeg      1.7         2/13/03 12:24:54 PM    Matt Korte      SCR(s) 5479 
       :
       Removed old header reference
       
 7    mpeg      1.6         1/10/03 5:41:18 PM     Senthil Veluswamy SCR(s) 
       5230 :
       Added support for General Event Notification & Data Feed/Stop Events.
       
 6    mpeg      1.5         6/20/02 5:53:32 PM     Steve Glennon   SCR(s): 4072
        
       Moved PCMInitStatus to start of PCMInit instead of end.
       
 5    mpeg      1.4         5/17/02 5:03:54 PM     Steve Glennon   SCR(s): 3822
        
       Removed use of bitfields to access hardware registers.
       
       
 4    mpeg      1.3         5/15/02 3:28:10 AM     Steve Glennon   SCR(s): 2438
        
       Major work to implement full functions required of driver
       
       
 3    mpeg      1.2         4/25/02 5:57:50 PM     Matt Korte      SCR(s) 2438 
       :
       Added more support. Runs for no mixing. The file PcmMix.c
       needs to be modified to handle getting, mixing and sending
       the data buffers to/from the IRQ handlers.
       
       
 2    mpeg      1.1         8/30/01 11:55:20 AM    Matt Korte      SCR(s) 2438 
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
       
 1    mpeg      1.0         8/8/01 2:47:30 PM      Matt Korte      
$
 * 
 *    Rev 1.14   19 Jun 2003 13:02:16   bradforw
 * SCR(s) 6796 :
 * Add offset of 4 to interrupt address register values to get non-
 * quadword alignment (to prevent multiple interrupts from occuring)
 * 
 *    Rev 1.13   02 Jun 2003 14:32:08   velusws
 * SCR(s) 6666 :
 * 1) Updated File header (Copyright info, etc)
 * 2) Move $Log to bottom of file
 * 3) Removed // comments
 * 
 *    Rev 1.12   02 Jun 2003 14:23:22   velusws
 * SCR(s) 6666 :
 * Changed the Play FIFO Threshold values as per Dave A
 * 
 *    Rev 1.11   21 Apr 2003 19:34:06   velusws
 * SCR(s) 6066 6065 :
 * Changed PCM_DISABLE_SRC to take values YES and NO instead of just being defined.
 * 
 *    Rev 1.10   21 Apr 2003 18:59:00   velusws
 * SCR(s) 6066 6065 :
 * Modifications to compile time disable SRC and run time disable Mixing if the MPEG level is set to MUTE.
 * 
 *    Rev 1.9   14 Feb 2003 11:31:38   aernedj
 * SCR(s) 5510 :
 * comment out, for now, code to enable mpeg capt atten path
 * 
 *    Rev 1.8   13 Feb 2003 14:24:22   aernedj
 * SCR(s) 5492 :
 * added code to enable mpeg capt atten path for non-colorado devices
 * through setting of pcm channel control bit
 * 
 *    Rev 1.7   13 Feb 2003 12:24:54   kortemw
 * SCR(s) 5479 :
 * Removed old header reference
 * 
 *    Rev 1.6   10 Jan 2003 17:41:18   velusws
 * SCR(s) 5230 :
 * Added support for General Event Notification & Data Feed/Stop Events.
 * 
 *    Rev 1.5   Jun 20 2002 17:53:32   glennon
 * SCR(s): 4072 
 * Moved PCMInitStatus to start of PCMInit instead of end.
 * 
 *    Rev 1.4   17 May 2002 16:03:54   glennon
 * SCR(s): 3822 
 * Removed use of bitfields to access hardware registers.
 * 
 * 
 *    Rev 1.3   15 May 2002 02:28:10   glennon
 * SCR(s): 2438 
 * Major work to implement full functions required of driver
 * 
 * 
 *    Rev 1.2   25 Apr 2002 16:57:50   kortemw
 * SCR(s) 2438 :
 * Added more support. Runs for no mixing. The file PcmMix.c
 * needs to be modified to handle getting, mixing and sending
 * the data buffers to/from the IRQ handlers.
 * 
 * 
 *    Rev 1.1   30 Aug 2001 10:55:20   kortemw
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
 *    Rev 1.0   08 Aug 2001 13:47:30   kortemw
 * SCR(s) 2438 :
 * DCS #2438
 * Initial put of Generic PCM driver. Does not do mixing yet. Does
 * have API interface, interrupt handlers and task. It does use
 * the interrupt handlers for the PCM data instead of hardware.
*/
