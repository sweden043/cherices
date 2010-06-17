/*                 Conexant Systems, Inc. - COLORADO                        */
/****************************************************************************/
/*                                                                          */
/* Filename:           PcmPriv.h                                            */
/*                                                                          */
/* Description:        Private definitions for the PCM Driver               */
/*                                                                          */
/* Author:             Matthew W. Korte                                     */
/*                                                                          */
/* Copyright Conexant Systems, Inc., 2001                                   */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/
/*
$Header: PcmPriv.h, 11, 9/8/03 8:19:16 PM, Dave Aerne$
$Log: 
 11   mpeg      1.10        9/8/03 8:19:16 PM      Dave Aerne      SCR(s) 7434 
       7435 :
       adjust calculation of buf_start to ensure buffer resides within 
       malloc'ed space. Also modify memset to only clear the buffer from start 
       to stop w/o clearing the excess bytes, prevents clearing bytes outside 
       of the buffer space.
       
 10   mpeg      1.9         8/26/03 7:19:06 PM     Dave Aerne      SCR(s) 7381 
       7382 :
       modify malloc of pcm_buffers to produce quad-word aligned buffers,
       as required by arm audio hardware.
       
 9    mpeg      1.8         4/28/03 3:44:42 PM     Senthil Veluswamy SCR(s) 
       6114 :
       Reduced memory size used for SRC buffers. Added FCopy function prototype
        to use in this module.
       
 8    mpeg      1.7         4/21/03 8:02:46 PM     Senthil Veluswamy SCR(s) 
       6066 6065 :
       Modifications to compile time disable SRC and run time disable Mixing if
        the MPEG level is set to MUTE.
       
 7    mpeg      1.6         9/25/02 10:07:12 AM    Lucy C Allevato SCR(s) 4650 
       :
       Added bool storage identifier to variables bCaptABuf and bPlayABuf.
       
 6    mpeg      1.5         6/19/02 5:54:34 PM     Steve Glennon   SCR(s): 4066
        
       Many fixes to original code which did not work correctly at all. 
       Enqueued
       and dequeued data from different queues etc.
       
 5    mpeg      1.4         5/17/02 5:03:54 PM     Steve Glennon   SCR(s): 3822
        
       Removed use of bitfields to access hardware registers.
       
       
 4    mpeg      1.3         5/15/02 3:28:10 AM     Steve Glennon   SCR(s): 2438
        
       Major work to implement full functions required of driver
       
       
 3    mpeg      1.2         4/25/02 5:58:32 PM     Matt Korte      SCR(s) 2438 
       :
       Added more support. Runs for no mixing. The file PcmMix.c
       needs to be modified to handle getting, mixing and sending
       the data buffers to/from the IRQ handlers.
       
       
 2    mpeg      1.1         8/30/01 11:55:08 AM    Matt Korte      SCR(s) 2438 
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
       
 1    mpeg      1.0         8/8/01 2:47:36 PM      Matt Korte      
$
 * 
 *    Rev 1.10   08 Sep 2003 19:19:16   aernedj
 * SCR(s) 7434 7435 :
 * adjust calculation of buf_start to ensure buffer resides within malloc'ed space. Also modify memset to only clear the buffer from start to stop w/o clearing the excess bytes, prevents clearing bytes outside of the buffer space.
 * 
 *    Rev 1.9   26 Aug 2003 18:19:06   aernedj
 * SCR(s) 7381 7382 :
 * modify malloc of pcm_buffers to produce quad-word aligned buffers,
 * as required by arm audio hardware.
 * 
 *    Rev 1.8   28 Apr 2003 14:44:42   velusws
 * SCR(s) 6114 :
 * Reduced memory size used for SRC buffers. Added FCopy function prototype to use in this module.
 * 
 *    Rev 1.7   21 Apr 2003 19:02:46   velusws
 * SCR(s) 6066 6065 :
 * Modifications to compile time disable SRC and run time disable Mixing if the MPEG level is set to MUTE.
 * 
 *    Rev 1.6   Sep 25 2002 10:07:12   allevalc
 * SCR(s) 4650 :
 * Added bool storage identifier to variables bCaptABuf and bPlayABuf.
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
 *    Rev 1.3   15 May 2002 02:28:10   glennon
 * SCR(s): 2438 
 * Major work to implement full functions required of driver
 * 
 * 
 *    Rev 1.2   25 Apr 2002 16:58:32   kortemw
 * SCR(s) 2438 :
 * Added more support. Runs for no mixing. The file PcmMix.c
 * needs to be modified to handle getting, mixing and sending
 * the data buffers to/from the IRQ handlers.
 * 
 * 
 *    Rev 1.1   30 Aug 2001 10:55:08   kortemw
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
 *    Rev 1.0   08 Aug 2001 13:47:36   kortemw
 * SCR(s) 2438 :
 * DCS #2438
 * Initial put of Generic PCM driver. Does not do mixing yet. Does
 * have API interface, interrupt handlers and task. It does use
 * the interrupt handlers for the PCM data instead of hardware.
*/

#define NEWPCM
/****************************************************************************
* If PCM_PRIMARY is defined, then global declarations are suppose to
* instantiate the variable, and possibly give an initial value. Otherwise,
* the global declarations are extern statements with no instantiations.
* This allows one declaration 'statement' that can be used by all files
* that reference the variable.
****************************************************************************/
#ifdef PCM_PRIMARY
#define BASE_DEF 
#define INIT_VAL(a) = a
#define INIT_ARRAY4(A,B,C,D) = { A, B, C, D}
#else  // !PCM_PRIMARY
#define BASE_DEF extern
#define INIT_VAL(a)
#define INIT_ARRAY4(A,B,C,D) 
#endif // !PCM_PRIMARY

// Start Defines
#ifdef NEWPCM
#define ARM_AUD_BUFF_SIZE           0x1000
#else   // !NEWPCM
#define ARM_AUD_BUFF_SIZE           0x1000
#endif  // !NEWPCM

/* extra bytes which will be useful in generation
   of quad-word aligned addresses */
#define ARM_AUD_BUFF_EXCESS         0x0f

// fatal_exit codes
#define RC_AUD_MEMORY               (MOD_AUD    + 0x01)
#define RC_AUD_BADPTR               (MOD_AUD    + 0x02)

#define ARM_AUD_HALF_BUFF_SIZE      (ARM_AUD_BUFF_SIZE >> 1)
#define ARM_AUD_QUART_BUFF_SIZE     (ARM_AUD_BUFF_SIZE >> 2)
#define ARM_AUD_EIGHTH_BUFF_SIZE    (ARM_AUD_BUFF_SIZE >> 3)
#define ARM_AUD_SIXTEENTH_BUFF_SIZE (ARM_AUD_BUFF_SIZE >> 4)

#define INT_AUDIO_CAPT INT_AUDIORX
#define INT_AUDIO_PLAY INT_AUDIOTX

// Values for mixing/conversion
#define TAPS 8
#define HALF_TAPS (TAPS >> 1)

#ifdef DEBUG
#define PCM_ISR_MASK 0xfff
#endif // DEBUG

#define PCM_PTR_DIF_LIMIT           (0x40)

/* Macro to get the offset between the current pointer and the start pointer */
/* given register pointers to a pair of start and status audio regs          */
#define GET_PCM_OFFSET(START,CURRENT) (GET_AUD_ADDRESS(CURRENT) - GET_AUD_ADDRESS(START))

#ifdef PCM_DEBUG
#define NOTIFY_UNEXPECTED_ISR(String, Count)                            \
    do {                                                                \
        isr_trace_new(TRACE_LEVEL_4 | TRACE_AUD, "%s, count=%d\n",String,++Count); \
    } while(0) // NOTIFY_UNEXPECTED_ISR
#else  // !PCM_DEBUG
#define NOTIFY_UNEXPECTED_ISR(String, Count)
#endif // !PCM_DEBUG

#define PCM_SWAP(A,B,C)                                                 \
    do {                                                                \
        C = *A;                                                         \
        *A = *B;                                                        \
        *B = C;                                                         \
    } while(0) // PCM_SWAP
//#define PCM_VAL(A) (int32 *)SET_FLAGS(A->Address, NCR_BASE)
#define SET_FIELD(A,FIELDVAL,MASK,SHIFT) (((A) & (~(MASK))) | (((FIELDVAL) << (SHIFT)) & (MASK)))
#define GET_FIELD(A,MASK,SHIFT) (((A) & (MASK)) >> SHIFT)
#define SET_BIT(A,VAL) ((A) | (VAL))
#define CLEAR_BIT(A,VAL) ((A) & (~(VAL)))
#define GET_AUD_ADDRESS(A) (((*(A)) & AUD_ADDRESS_ADDRESS_MASK) >> AUD_ADDRESS_ADDRESS_SHIFT)
#define MAKE_AUD_ADDRESS(A) (((A) << AUD_ADDRESS_ADDRESS_SHIFT) & AUD_ADDRESS_ADDRESS_MASK)
#define NORMAL_SAFETY   0x100
#define NO_SAFETY       0x0
#define BETWEEN(LOW,VAL,HI,SAFETY) ((GET_AUD_ADDRESS(VAL) >= GET_AUD_ADDRESS(LOW)) && (GET_AUD_ADDRESS(VAL) <= (GET_AUD_ADDRESS(HI)-(SAFETY))))

#define STRINGIT(a) # a
//#define PCMDEBUG
#ifdef PCMDEBUG
#define SHOW_PCM(FCN,POS,BUF)                                                   \
  do {                                                                          \
    if ((pcm_ ## FCN ## _isr_cnt & PCM_ISR_MASK) == 0) {                        \
      isr_trace_new(TRACE_LEVEL_2 | TRACE_AUD,                                  \
        STRINGIT(FCN) " ISR " STRINGIT(POS) " SWAP: Capt start=%x, stop=%x\n",  \
        *(int *)lpcapt_ ## BUF ## _start, *(int *)lpcapt_ ## BUF ## _stop);     \
      isr_trace_new(TRACE_LEVEL_2 | TRACE_AUD,                                  \
        STRINGIT(FCN) " ISR " STRINGIT(POS) " SWAP: Play start=%x, stop=%x\n",  \
        *(int *)lpplay_ ## BUF ## _start, *(int *)lpplay_ ## BUF ## _stop);     \
      isr_trace_new(TRACE_LEVEL_2 | TRACE_AUD,                                  \
        STRINGIT(FCN) " ISR " STRINGIT(POS) " SWAP: Capt is %x, Play is %x\n",  \
        *(int *)lpcapt_status, *(int *)lpplay_status);                          \
    }                                                                           \
  } while(0) // SHOW_PCM
#else  // !PCMDEBUG
#define SHOW_PCM(FCN,POS,BUF)
#endif // !PCMDEBUG

// Have to call the MoveAndMix() this many times at the beginning to get
// everything initialized properly.
#define FORCE_MIX_INIT_VAL  4

#define INIT_NUM_BUFS    4      // Number of pcm bufs to alloc

/* Size of PCM input queue */
//#define PCM_INPUTQ_SAMPLES    16384 
#define PCM_INPUTQ_SAMPLES    ARM_AUD_BUFF_SIZE    /* 4 * Audio Buf size */
//#define PCM_INPUTQ_SAMPLES    ARM_AUD_BUFF_SIZE/2  /* 2 * Audio Buf size */

#define PCM_SEM_WAIT_TIME     20    /* default task wait time in ms */
#define SRC_SEM_WAIT_TIME     5     /* default task wait time in ms */
#define MAX_SRC_TIMEOUT       1000  /* Max timeout waiting for data stored */
// End   Defines

// pcm type definitions START
typedef struct _cnxt_pcm_buf_handle_t
{
    u_int32                 *pui32_buf_start_malloc;
    u_int32                 *pui32_buf_start;
    u_int32                 *pui32_buf_stop;
    u_int32                 ui32_size_in_bytes;
    bool                    b_customer_buf;
    bool                    b_aregs_used;
    volatile struct _cnxt_pcm_buf_handle_t   *ps_next_buf_handle;
} cnxt_pcm_buf_handle_t;

typedef struct _cnxt_pcm_src_t  // Sample Rate Conversion (SRC)
{
    u_int32   Interpolator;
    u_int32   Decimator;
    int32     *Filter;
} cnxt_pcm_src_t;
// pcm type definitions END

// Start Declarations
BASE_DEF volatile cnxt_pcm_buf_handle_t   *ps_buf_handle_capt_head INIT_VAL(NULL);
BASE_DEF volatile cnxt_pcm_buf_handle_t   *ps_buf_handle_capt_tail INIT_VAL(NULL);
BASE_DEF volatile cnxt_pcm_buf_handle_t   *ps_buf_handle_play_head INIT_VAL(NULL);
BASE_DEF volatile cnxt_pcm_buf_handle_t   *ps_buf_handle_play_tail INIT_VAL(NULL);
BASE_DEF volatile cnxt_pcm_buf_handle_t   *ps_buf_handle_pcm_head INIT_VAL(NULL);
BASE_DEF volatile cnxt_pcm_buf_handle_t   *ps_buf_handle_pcm_tail INIT_VAL(NULL);
BASE_DEF volatile cnxt_pcm_buf_handle_t   *ps_done_head_ptr INIT_VAL(NULL);
BASE_DEF volatile cnxt_pcm_buf_handle_t   *ps_done_tail_ptr INIT_VAL(NULL);


BASE_DEF volatile bool bCaptABuf INIT_VAL(TRUE);
BASE_DEF volatile bool bPlayABuf INIT_VAL(TRUE);

BASE_DEF unsigned char arm_aud_capt_buff[ARM_AUD_BUFF_SIZE+0xf];
BASE_DEF unsigned char arm_aud_play_buff[ARM_AUD_BUFF_SIZE+0xf];

BASE_DEF PFNISR previous_arm_audio_capt_isr, previous_arm_audio_play_isr;

BASE_DEF volatile u_int32 *AudCaptIntStatus INIT_VAL((u_int32 *)AUD_CAPT_INT_STAT_REG);
BASE_DEF volatile u_int32 *AudPlayIntStatus INIT_VAL((u_int32 *)AUD_PLAY_INT_STAT_REG);

BASE_DEF LPREG lpcapt_a_start  INIT_VAL((LPREG) AUD_CAPT_A_START_REG);
BASE_DEF LPREG lpcapt_b_start  INIT_VAL((LPREG) AUD_CAPT_B_START_REG);
BASE_DEF LPREG lpcapt_status   INIT_VAL((LPREG)  AUD_CAPT_STATUS_REG);
BASE_DEF LPREG lpcapt_a_stop   INIT_VAL((LPREG)  AUD_CAPT_A_STOP_REG);
BASE_DEF LPREG lpcapt_b_stop   INIT_VAL((LPREG)  AUD_CAPT_B_STOP_REG);
BASE_DEF LPREG lpplay_a_start  INIT_VAL((LPREG) AUD_PLAY_A_START_REG);
BASE_DEF LPREG lpplay_b_start  INIT_VAL((LPREG) AUD_PLAY_B_START_REG);
BASE_DEF LPREG lpplay_status   INIT_VAL((LPREG)  AUD_PLAY_STATUS_REG);
BASE_DEF LPREG lpplay_a_stop   INIT_VAL((LPREG)  AUD_PLAY_A_STOP_REG);
BASE_DEF LPREG lpplay_b_stop   INIT_VAL((LPREG)  AUD_PLAY_B_STOP_REG);
BASE_DEF LPREG lpplay_int_addr INIT_VAL((LPREG) AUD_PLAY_INT_ADDR_REG);
BASE_DEF LPREG lpcapt_int_addr INIT_VAL((LPREG) AUD_CAPT_INT_ADDR_REG);
BASE_DEF int PcmCaptNoSpare INIT_VAL(0);
BASE_DEF int PcmPlayNoSpare INIT_VAL(0);
BASE_DEF int PcmCaptWrngBuf INIT_VAL(0);
BASE_DEF int PcmPlayWrngBuf INIT_VAL(0);
BASE_DEF int PcmDidNothingA INIT_VAL(0);
BASE_DEF int PcmDidNothingB INIT_VAL(0);
BASE_DEF int PcmBadPointerA INIT_VAL(0);
BASE_DEF int PcmBadPointerB INIT_VAL(0);

BASE_DEF int CaptDifa;
BASE_DEF int CaptDifb;
BASE_DEF int PlayDifa;
BASE_DEF int PlayDifb;
BASE_DEF int PCMDifa;
BASE_DEF int PCMDifb;
BASE_DEF int ReStartCnt INIT_VAL(0);
BASE_DEF int PCMSkipReset INIT_VAL(0);

BASE_DEF int cnxt_force_mix INIT_VAL(FORCE_MIX_INIT_VAL); // Force to mix this many times

BASE_DEF volatile cnxt_pcm_status_t s_cnxt_pcm_status;

extern int32 cnxt_pcm_attenuation_shift[];
extern int32 cnxt_pcm_attenuation_multiply[];

BASE_DEF volatile int32 MPEGLeftAttenShift  INIT_VAL(0); /* start out at full volume */
BASE_DEF volatile int32 MPEGRightAttenShift INIT_VAL(0); /* start out at full volume */
BASE_DEF volatile int32 MPEGLeftAttenMulti  INIT_VAL(1); /* start out at full volume */
BASE_DEF volatile int32 MPEGRightAttenMulti INIT_VAL(1); /* start out at full volume */

// Values for mixing/conversion
BASE_DEF int32 FIR_Wn_01_02[HALF_TAPS] INIT_ARRAY4(-11, -47, 198, 884);
BASE_DEF int32 FIR_Wn_01_03[HALF_TAPS] INIT_ARRAY4( -8,  36, 308, 688);
BASE_DEF int32 FIR_Wn_01_04[HALF_TAPS] INIT_ARRAY4(  7,  78, 330, 609);
BASE_DEF int32 FIR_Wn_01_06[HALF_TAPS] INIT_ARRAY4( 25, 109, 338, 552);
BASE_DEF int32 FIR_Wn_01_08[HALF_TAPS] INIT_ARRAY4( 32, 120, 340, 532);
BASE_DEF int32 FIR_Wn_01_11[HALF_TAPS] INIT_ARRAY4( 37, 127, 340, 520);
BASE_DEF int32 FIR_Wn_01_12[HALF_TAPS] INIT_ARRAY4( 38, 128, 340, 518);
BASE_DEF int32 FIR_Wn_01_13[HALF_TAPS] INIT_ARRAY4( 38, 129, 341, 516);
BASE_DEF int32 FIR_Wn_01_26[HALF_TAPS] INIT_ARRAY4( 41, 133, 341, 509);

BASE_DEF cnxt_pcm_src_t a_cnxt_pcm_src_array[8][9] INIT_VAL(/**/)
#ifdef PCM_PRIMARY
{ /* To: / From: 8kHz                 11kHz                 12kHz                 16kHz                 22kHz                 24kHz                 32kHz                 44kHz                 48kHz */
/*48kHz*/ { {6, 1, FIR_Wn_01_06}, {13,3, FIR_Wn_01_13}, {4, 1, FIR_Wn_01_04}, {3, 1, FIR_Wn_01_03}, {13,6, FIR_Wn_01_13}, {2, 1, FIR_Wn_01_02}, {3, 2, NULL        }, {12,11,NULL        }, {1, 1, NULL        } },
/*44kHz*/ { {11,2, FIR_Wn_01_11}, {4, 1, FIR_Wn_01_04}, {11,3, FIR_Wn_01_11}, {11,4, FIR_Wn_01_11}, {2, 1, FIR_Wn_01_02}, {11,6, FIR_Wn_01_11}, {11,8, NULL        }, {1, 1, NULL        }, {11,12,NULL        } },
/*32kHz*/ { {4, 1, FIR_Wn_01_04}, {26,9, FIR_Wn_01_26}, {8, 3, FIR_Wn_01_08}, {2, 1, FIR_Wn_01_02}, {13,9, FIR_Wn_01_13}, {4, 3, FIR_Wn_01_04}, {1, 1, NULL        }, {8,11, NULL        }, {2, 3, NULL        } },
/*RSRVD*/ { {1, 1, NULL        }, {1, 1, NULL        }, {1, 1, NULL        }, {1, 1, NULL        }, {1, 1, NULL        }, {1, 1, NULL        }, {1, 1, NULL        }, {1, 1, NULL        }, {1, 1, NULL        } },
/*24kHz*/ { {3, 1, FIR_Wn_01_03}, {13,6, FIR_Wn_01_13}, {2, 1, FIR_Wn_01_02}, {3, 2, FIR_Wn_01_03}, {12,11,FIR_Wn_01_12}, {1, 1, NULL        }, {3, 4, NULL        }, {6,11, NULL        }, {2, 4, NULL        } },
/*22kHz*/ { {11,4, FIR_Wn_01_11}, {2, 1, FIR_Wn_01_02}, {11,6, FIR_Wn_01_11}, {11,8, FIR_Wn_01_11}, {1, 1, NULL        }, {11,12,FIR_Wn_01_11}, {11,16,NULL        }, {2, 4, NULL        }, {11,24,NULL        } },
/*16kHz*/ { {2, 1, FIR_Wn_01_02}, {13,9, FIR_Wn_01_13}, {4, 3, FIR_Wn_01_04}, {1, 1, NULL        }, {8,11, FIR_Wn_01_08}, {2, 3, FIR_Wn_01_02}, {2, 4, NULL        }, {4,11, NULL        }, {2, 6, NULL        } },
/*RSRVD*/ { {1, 1, NULL        }, {1, 1, NULL        }, {1, 1, NULL        }, {1, 1, NULL        }, {1, 1, NULL        }, {1, 1, NULL        }, {1, 1, NULL        }, {1, 1, NULL        }, {1, 1, NULL        } },
};
#else // !defined(PCM_PRIMARY)
;
#endif

BASE_DEF int32 *PCMFilterKernel INIT_VAL(NULL);

BASE_DEF PCM20_CQ PCMInputQ;               /* Input queue for PCM audio
                                            * samples from user, converted
                                            * into PCM 20 bits per sample
                                            */
BASE_DEF PCM20_CQ PCMSRCQ;                             /* Holds sample rate converted data */
BASE_DEF cnxt_pcm_state_t eSRCTaskState INIT_VAL(NULL_STATE);/* State of SRC task */
BASE_DEF sem_id_t cnxt_pcm_sem_id;
BASE_DEF sem_id_t cnxt_src_sem_id;


#ifdef DEBUG
BASE_DEF int pcm_capt_isr_cnt   INIT_VAL(0);
BASE_DEF int pcm_play_isr_cnt   INIT_VAL(0);
BASE_DEF int MoveAndMixCnt      INIT_VAL(0);
#endif // DEBUG

/* To turn off mixing */
BASE_DEF bool gbPCMPlayNoMpeg   INIT_VAL(FALSE);

// End   Declarations

// Start Extern Prototypes
extern void hw_aud_start();
extern void hw_aud_stop(bool bClearBuffers);
extern cnxt_pcm_buf_handle_t *GetNewPcmBuffer(void);
extern void    MoveAndMix(int32 *pStart, int32 *pStop, int32 *pDest);
extern void    StartPCMCaptureAndPlayback(void);
extern int     arm_audio_capt_isr( u_int32, bool, PFNISR * );
extern int     arm_audio_play_isr( u_int32, bool, PFNISR * );
extern bool    SetMPEGVolume(unsigned char left, unsigned char right);
extern void    AudMPGVolumeFuncRegister(mixer_volume_func_t volume_function);
extern void    cnxt_pcm_task(void);
extern void    pcmStatusInit(void);
extern void    pcmDoIdleState(void);

extern void ChooseSRCParameters( cnxt_pcm_sample_rate_t PCMSampleRate,
                                 u_int32 *pInterpolator, 
                                 u_int32 *pDecimator,
                                 int32  **pPCMFilterKernel);
extern u_int32 SampleRateConvert(u_int32 Interpolator, 
                                 u_int32 Decimator,
                                 u_int32 InputSamples, 
                                 PCM20 * InputBuffer, 
                                 PCM20 * OutputBuffer,
                                 bool bStereo,
                                 int32 *PCMFilterKernel);
extern void    MonoLinearInterpolate(  int32 * InputBuffer, 
                                       int32 * OutputBuffer,
                                       int InputSamples, 
                                       int Interpolator);
extern void    StereoLinearInterpolate(int32 * InputBuffer, 
                                       int32 * OutputBuffer,
                                       int InputSamples, 
                                       int Interpolator);
extern void    MonoFilterAndDecimate(  int32 * InputPtr, 
                                       int32 * OutputPtr,
                                       int InputSamples, 
                                       int Decimator,
                                       int32 *PCMFilterKernel);
extern void    MonoDecimate(           int32 * InputPtr, 
                                       int32 * OutputPtr,
                                       int InputSamples, 
                                       int Decimator);
extern void    StereoFilterAndDecimate(int32 * InputPtr, 
                                       int32 * OutputPtr,
                                       int InputSamples, 
                                       int Decimator,
                                       int32 *PCMFilterKernel);
extern void    StereoDecimate(         int32 * InputPtr, 
                                       int32 * OutputPtr,
                                       int InputSamples, 
                                       int Decimator);
extern void    cnxt_src_task( void );

extern void AttenuateSamples(PCM20 *Samples, 
                             int    SampleCount,
                             bool   bStereo,
                             u_int8 ui8LeftVol,
                             u_int8 ui8RightVol);

/* For the Fast En/DeQ routines */
extern void FCopy(char *pDest, char *pSrc, u_int32 len);
                                       
// End   Extern Prototypes

// Start Extern Declarations
extern bool cnxt_audio_mono;
// End   Extern Declarations
