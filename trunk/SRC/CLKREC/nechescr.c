/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*          Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002          */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        nechescr.c
 *
 *
 * Description:     Source code for system clock recovery,
 *                  Adaptable version
 *
 *
 * Author:          Amy Pratt
 *
 ****************************************************************************/
/* $Id: nechescr.c,v 1.70, 2004-05-18 22:35:43Z, Tim White$
 ****************************************************************************/

/** include files **/
#include <assert.h>
#include <stdlib.h>
#include "stbcfg.h"
#include "kal.h"
#include "retcodes.h"
#include "globals.h"
#include "clkrec.h"
#include "ccrapi.h"
#include "demuxapi.h"

#ifdef DEBUG
#define CR_TRACE
#define TRACE_CR_SUM (TRACE_LEVEL_2 | TRACE_CR)
#endif

extern u_int32 gDemuxInstance;

/* the following definitions refer to the clock recovery algorithm
   used in the function ccrSimpleFilter.  This is the default 
   clock recovery algorithm.

   The algorithm is:
       frequency_new = frequency_nominal + (GAIN * s(i))
   where
       s(i) = (1-RHO)*e(i) + RHO*s(i-1)
       e(i) = PCR(i) - STC(i)
       0<=RHO<=1

   also,    PHI = 1 - RHO
   for more information on the algorithm, see the article
   "Clock Synchronization in Software MPEG-2 Decoder" by
   Victor Ramamoorthy, published in SPIE Vol. 3021 pp.194-210.
*/
#ifdef USING_FLOATING_POINT
#define FREQ_OFFSET_MAX   1350
#define GAIN 0.06125  /* gain applied after the filter */
                /* This value should approximately equal
                   1/(PHI * max time between PTSes)
                   For Transport Streams, this is
                   1/(PHI * .1) which is 10 for
                   PHI = 1 and 50 fo PHI = .5   */

#define FERROR_MAX (1350.0/GAIN)
#define FERROR_MIN (-(FERROR_MAX))

#define PHI_BASE .85        /* PHI
                               becomes PHI_BASE^i, where i is
                               initially 1 and is incremented
                               by one each iteration
                               until PHI_BASE <= PHI_MIN     */
#define PHI_MIN 0.0306125  /* PHI_MIN should be the largest
                       value of PHI that adequately filters the
                       incoming PTSes for jitter.  EuroBox specs
                       state this jitter is 50us, or 4.5 ticks on
                       the 90kHz clock.  I don't know how this
                       translates to PHI, .5 is a first guess */

#else
#define GAIN_NUMERATOR    49
#define GAIN_DENOMINATOR  800

#if PARSER_TYPE==DVB_PARSER
#define FREQ_OFFSET_MAX   1350
#define FERROR_MAX        22041
#define FERROR_MIN       -22041
#else
#define FREQ_OFFSET_MAX   2700
#define FERROR_MAX        44082
#define FERROR_MIN       -44082
#endif

#endif

#ifdef OPENTV
#define MAX_CHANGE 460
#else
#define MAX_CHANGE 460 /*  This is the largest change in the filtered
                            error value.  Values larger than this will
                            cause the clock to change too much too quickly.
                            This value is consistent with
                           previous "BIG JITTER" definition. 
                           A more realistic number is 460,
                           or (ignoring jitter) 
                           max_possible_difference_in_.2_seconds
                           times maximum value of PHI */
#endif


#define NOM_FREQ 27000000
#if 0 /* definition moved to ccrapi.h */
#define CCR_DISCONT_THRESHOLD 5400000  /* if the PTS and STC differ by more
                               than this value, signal that
                               we have a discontinuity
                               Signal discontinuity if difference
                               is more than .2 second             */
#endif /* comment */

#define DISCONT_RETURN_THRESHOLD 540000  /* this value allows for 2ms jitter */
                                     /* if delta is within this
                                        threshold, the STC has
                                        been toggled            */
#define STC_OFFSET 0   /* safe values are between 0 and 4496.0 */
                       /* 0 means there is no offset.  any real 
                          number value will be subtracted from 
                          the incoming PCR value before setting 
                          the STC.  This will allow more data
                          to build up in the encoded buffers   */

#define MAX_AUDIO_STC_OFFSET              0x32a0  /* this is the max possible
                                                     offset, which is two frame 
                                                     times in 90KHz ticks for 
                                                     16kHz Layer II audio */
#define MAX_AUDIO_ADJUSTABLE_STC_OFFSET   900
#define TARGET_AUDIO_STC_OFFSET           450
#define AUDIO_FREQ_ADJUST_FACTOR          5000000

/** external data **/
#ifdef DRIVER_INCL_AUDIO
extern bool first_PTS;
#endif
extern bool SyncOn;
extern bool NewPCRFlag;
extern u_int16 apll_divisor;
extern int32 current_nom_pll_config_frac;
extern LPREG lpPLL_Div1;

extern u_int32 xtal_frequency;
extern u_int32 current_nom_aud_freq;

/** internal functions **/
void clk_rec_task( void * );
int clk_rec_mpeg_isr( u_int32, bool, PFNISR *);
void writePLL( u_int8, int32, int32, u_int32);
void clk_rec_ISRNotifyRegister(u_int32 interrupt_bit,
                               char vendor_id,
                               clk_rec_ISRNotify_t handler);
void clk_rec_PCRNotifyRegister(clk_rec_PCRNotify_t handler);
void clk_rec_PCRFailCBRegister(char vendor_id, clk_rec_PCRFailCB_t handler);
void clk_rec_PCRDiscontinuityCBRegister(char vendor_id, clk_rec_PCRFailCB_t handler);
int32 ccrPcrDifference(void);

#ifdef USING_FLOATING_POINT
double ccrSimpleFilter(int32 delta);
#else
int32 ccrSimpleFilter(int32 delta);
#endif

void ccrSimpleFilterInit(void);


/** public data **/
bool reenable_sync;
bool reenable_aud_sync;
bool SyncBlackout;   /* if we are in the middle of a discontinuity, 
                        DO NOT let anyone enable sync */
u_int32 wait4newSTC;
u_int32 ccrDiscontThreshold = CCR_DISCONT_THRESHOLD;
u_int32 ccrPCRTimerValue = 540000;
bool    ccrPCRTimerEnable = TRUE;
ccrFilterFunc ccrCurrentFilter = ccrSimpleFilter;
ccrFilterInitFunc ccrCurrentFilterInit = ccrSimpleFilterInit;
bool bForceDiscontinuity = FALSE;
bool bClkRecEnabled = FALSE;

/** private data **/
#define     MPG_PSCRS_HI_HIGHBIT_MASK                   0x00000001
#define     MPG_PSCRS_HI_EXTENSION_MASK                 0x000003FE

typedef struct _ClkRef {
    HW_DWORD High;
    u_int32 Low;
} ClkRef;

static clk_rec_ISRNotify_t vb_VIDPTSARRIVED_notify = NULL;
static clk_rec_ISRNotify_t vb_VIDPTSMATURED_notify = NULL;
static clk_rec_PCRNotify_t dmx_PCR_notify = NULL;
static clk_rec_PCRFailCB_t vbPCRFailCallback = NULL;
static clk_rec_PCRFailCB_t vbPCRDiscontinuityCallback = NULL;

u_int32 clk27_postdiv;
#if PLL_TYPE != PLL_TYPE_COLORADO
    u_int32 mpg_pll_prescale;
    u_int32 aud_pll_prescale;
#endif

static int32 audio_tick_diff = 0;

/* ccrSimpleFilter variables */
#ifdef USING_FLOATING_POINT
static float phi, last_ferror;
static float offset;
static double current_PLL_frequency;  /* based on 27MHz clock */
#else
static int32 phi_numerator[]   = {   1,  17, 138, 113,  83,  67,  66,  42, 
                                     3,  41,  38,  33,  28,  11,  15,   9,
                                    13,   6,   8,   9,   5,   3 };
static int32 phi_denominator[] = {   1,  20, 191, 184, 159, 151, 175, 131,
                                    11, 177, 193, 197, 197,  91, 146, 103,
                                   175,  95, 149, 197, 129,  91 };
                                   /* phi_numerator[i]/phi_denominator[i] = phi^i */
static u_int32 clk_rec_step = 0, max_changing_step = 21;
static int32 last_ferror, last_ferror_remain;

static int32 offset;
static int32 current_PLL_frequency;  /* based on 27MHz clock */
#endif

/* end filter variables */
static bool PCR_PID_changed;
static PFNISR clk_previous_mpeg_isr;
static u_int32 clk_rec_task_ID;
static sem_id_t clk_rec_sem;
LPREG lpPCRTimer = (LPREG)MPG_PCR_TIMER_INFO_REG;
LPREG lpPLL_MPEG = (LPREG)PLL_VID_CONFIG_REG;
LPREG lpPLL_Aud = (LPREG)PLL_AUD_CONFIG_REG;
int32 MPG_pll_nom_config_int, MPG_pll_nom_config_frac;

#ifdef CR_TRACE
void buffer_monitor_task( void * );
int abuff_full_max, abuff_full_min, abuff_level;
int vbuff_full_max, vbuff_full_min, vbuff_level;
int PCR_arrival_freq;
int PCR_interval;
unsigned int lastPCRHi;
int raw_err_max, raw_err_min;
int abuff_full_avg, vbuff_full_avg, raw_err_avg;
#ifdef USING_FLOATING_POINT
double freq_max, freq_min;
double err_delta_max, err_delta_min;
double freq_avg, freq_delta_avg;
#else
u_int32 freq_max, freq_min;
int32 err_delta_max, err_delta_min;
u_int32 freq_avg, freq_delta_avg;
#endif
bool discont_trace;
int pcr_count;
static u_int32 buffer_monitor_task_ID;

#endif /* CR_TRACE */


/** public functions **/
/*
 ** temp_disable_avsync
 *
 *  FILENAME: D:\build2\latest\CLKREC\clkrec.c
 *
 *  PARAMETERS: none
 *
 *  DESCRIPTION:  Temporarily disables a/v sync (if enabled)
 *                Should be called when a/v sync needs to be
 *                disabled because of error conditions.
 *                (Due to synchronization errors in the incoming
 *                bitstream, channel changes, large PCR jitter, ...)
 *       MAY BE CALLED FROM ISR
 *
 *  RETURNS: none
 *
 */
#ifdef OPENTV
extern void audio_stop_quiet(void);
extern void audio_play_quiet(void);
#endif // OPENTV

#if !defined (USING_FLOATING_POINT)
u_int32 calculate_multiplier ( u_int32 pll_freq, u_int32 xtal_freq )
{
   int32 n = 25;
   u_int32 multiplier = 0;

   while ( ( n > 0 ) && ( ( xtal_freq & 0x1 ) == 0 ) )
   {
      n --;
      xtal_freq = xtal_freq >> 1;
   }

   multiplier = ( pll_freq / xtal_freq ) << n;
   while ( n > 0 )
   {
      pll_freq = pll_freq % xtal_freq;
      while ( ( ( pll_freq & 0x80000000 ) == 0 ) && ( n > 0 ) )
      {
         n --;
         pll_freq = pll_freq << 1;
      }
      multiplier += (( pll_freq / xtal_freq ) << n);
   }

   return multiplier;
}
#endif

void temp_disable_avsync( void )
{
    HW_DWORD     tmpCtrl0;
    HW_DWORD     tmpCtrl1;

#ifdef OPENTV
//    audio_stop_quiet();
#endif // OPENTV
    tmpCtrl0 = *glpCtrl0;
   // Only mess with the video sync if not a PID change
   if (CNXT_GET_VAL(&tmpCtrl0, MPG_CONTROL0_ENABLESYNC_MASK) == 1 && !PCR_PID_changed)
   {
       isr_trace_new(TRACE_LEVEL_3 | TRACE_CR, " disabling sync in temp_disable_avsync\n",0,0);
       CNXT_SET_VAL(&tmpCtrl0, MPG_CONTROL0_ENABLESYNC_MASK, 0);
       *glpCtrl0 = tmpCtrl0;
       reenable_sync = TRUE;
       SyncOn = TRUE;
   }
   tmpCtrl1 = *glpCtrl1;
   if (CNXT_GET_VAL(&tmpCtrl1, MPG_CONTROL1_ENABLEAUDSYNC_MASK) == 1) {
       CNXT_SET_VAL(&tmpCtrl1, MPG_CONTROL1_ENABLEAUDSYNC_MASK, 0);
       *glpCtrl1 = tmpCtrl1;
       reenable_aud_sync = TRUE;
#ifndef WABASH_AVSYNC_CLKREC_DISABLED
       CNXT_SET_VAL(&tmpCtrl1, MPG_CONTROL1_ENABLEAUDSYNC_MASK, 1);  // Turn the audio sync back on.
#endif
       *glpCtrl1 = tmpCtrl1;        // The toggle is enough to reload PTS
   }
   SyncBlackout = TRUE;
}

/*
 ** restart_avsync
 *
 *  FILENAME: D:\build2\latest\CLKREC\clkrec.c
 *
 *  PARAMETERS:  none
 *
 *  DESCRIPTION:  Should be called to re-enable sync after error
 *                condition has cleared.
 *       MAY BE CALLED FROM ISR
 *
 *  RETURNS:  none
 *
 */
void restart_avsync( void )
{
    HW_DWORD     tmpCtrl0;
    HW_DWORD     tmpCtrl1;

    if (reenable_sync == TRUE)
    {
        tmpCtrl0 = *glpCtrl0;
#ifndef WABASH_AVSYNC_CLKREC_DISABLED
        CNXT_SET_VAL(&tmpCtrl0, MPG_CONTROL0_ENABLESYNC_MASK, 1);
#endif
        *glpCtrl0 = tmpCtrl0;
    }
    if (reenable_aud_sync == TRUE) {
        /* glpCtrl1->Mute=1; */
        tmpCtrl1 = *glpCtrl1;
#ifndef WABASH_AVSYNC_CLKREC_DISABLED
        CNXT_SET_VAL(&tmpCtrl1, MPG_CONTROL1_ENABLEAUDSYNC_MASK, 1);
#endif
        *glpCtrl1 = tmpCtrl1;

        /* reset audio freq adjust factor */
        audio_tick_diff = 0;
    }
    SyncBlackout = FALSE;
    reenable_sync = FALSE;
    reenable_aud_sync = FALSE;
#ifdef OPENTV
//    audio_play_quiet();
#endif // OPENTV
    /* process_time_sleep(100); */
    /* glpCtrl1->Mute=0; */
}


/*
 *
 *  FILENAME: D:\build2\sabine\clkrec\clk_recov.c
 *
 *  PARAMETERS: reinit  (if false, this is power-up, do everything)
 *                      (if true, may be called from ISR, be careful)
 *
 *  DESCRIPTION:  function initializes the clock recovery block
 *
 *  RETURNS:  returns true if initialization was successful, false otherwise.
 *
 */
bool clock_init(bool reinit)
{
    static bool FirstTime = TRUE;
    HW_DWORD     tmpCtrl1;
    PFNISR clk_rec_isr_handle;
    LPREG pPostDiv2Reg = (LPREG)PLL_DIV2_REG;

    trace_new(TRACE_LEVEL_2 | TRACE_CR, "Inside Clock Init\n");

    if (FirstTime) /* first time only */{
        FirstTime = FALSE;
        /* register ISR */
        clk_rec_isr_handle = (PFNISR)&clk_rec_mpeg_isr;
        int_register_isr( INT_MPEGSUB, clk_rec_isr_handle,
                      0, 0, &clk_previous_mpeg_isr );

        /* set up tasks and semaphores*/
        clk_rec_sem = sem_create(0, "CLKS");
        clk_rec_task_ID = task_create( clk_rec_task, NULL, NULL,
                  CLKT_TASK_STACK_SIZE, CLKT_TASK_PRIORITY, CLKT_TASK_NAME);

#ifdef CR_TRACE
        buffer_monitor_task_ID = task_create( buffer_monitor_task, NULL, NULL,
                  BUFT_TASK_STACK_SIZE, BUFT_TASK_PRIORITY, BUFT_TASK_NAME);
        lastPCRHi = 0;
        PCR_interval = 9000/8;
#endif /* CR_TRACE */

        /* unmask needed interrupt bits */
#if (PVR==YES)
        *glpIntMask |= MPG_PCR_RECEIVED | MPG_PCR_DISC;
#else
        *glpIntMask |= MPG_VID_PTS_MATURED | MPG_VID_PTS_ARRIVED | MPG_PCR_RECEIVED | MPG_PCR_DISC;
#endif

        /* initialize PCR timer */
        CNXT_SET_VAL(lpPCRTimer, MPG_PCR_TIMER_INFO_TIMERVALUE_MASK, ccrPCRTimerValue);

        /* XXXAMY I assume that the divider will be set correctly *
         * and will not change after initialization               */
        #if PLL_TYPE == PLL_TYPE_COLORADO
            clk27_postdiv = (*pPostDiv2Reg & PLL_DIV2_CLK27_MASK) >> PLL_DIV2_CLK27_SHIFT;
          #ifdef USING_FLOATING_POINT
            MPG_pll_nom_config_int = (clk27_postdiv*NOM_FREQ)/xtal_frequency;
            MPG_pll_nom_config_frac = (u_int32)(((clk27_postdiv * NOM_FREQ)/(double)xtal_frequency) 
                                               * (1<<25)) & 0x1FFFFFF;
          #else
            MPG_pll_nom_config_int = calculate_multiplier ( clk27_postdiv*NOM_FREQ, xtal_frequency );
            MPG_pll_nom_config_frac = MPG_pll_nom_config_int & 0x1FFFFFF;
            MPG_pll_nom_config_int = MPG_pll_nom_config_int >> 25;
          #endif
        #else 
          #if PLL_TYPE == PLL_TYPE_WABASH
            clk27_postdiv = (*pPostDiv2Reg & PLL_DIV2_CLK27_MASK) >> PLL_DIV2_CLK27_SHIFT;
            mpg_pll_prescale = PLL_PRESCALE((*((LPREG)PLL_PRESCALE_REG) &
                                            PLL_PRESCALE_MPG_MASK) >> PLL_PRESCALE_MPG_SHIFT);
            aud_pll_prescale = PLL_PRESCALE((*((LPREG)PLL_PRESCALE_REG) &
                                            PLL_PRESCALE_AUD_MASK) >> PLL_PRESCALE_AUD_SHIFT);
            #ifdef USING_FLOATING_POINT
            MPG_pll_nom_config_int = (NOM_FREQ * mpg_pll_prescale * clk27_postdiv * 2)
                                               / (double)xtal_frequency;
            MPG_pll_nom_config_frac = (u_int32)(((NOM_FREQ * mpg_pll_prescale * clk27_postdiv * 2)
                                               / (double)xtal_frequency) * (1<<25)) & 0x1FFFFFF;
            #else
            MPG_pll_nom_config_int = calculate_multiplier ( NOM_FREQ * mpg_pll_prescale * clk27_postdiv * 2,
                                                            xtal_frequency );
            MPG_pll_nom_config_frac = MPG_pll_nom_config_int & 0x1FFFFFF;
            MPG_pll_nom_config_int = MPG_pll_nom_config_int >> 25;
            #endif
          #else /* PLL_TYPE_BRAZOS and beyond */                                     
            clk27_postdiv = (*pPostDiv2Reg & PLL_DIV2_CLK27_MASK) >> PLL_DIV2_CLK27_SHIFT;
            mpg_pll_prescale = PLL_PRESCALE((*((LPREG)PLL_PRESCALE_REG) &
                                            PLL_PRESCALE_MPG_MASK) >> PLL_PRESCALE_MPG_SHIFT);
            aud_pll_prescale = PLL_PRESCALE((*((LPREG)PLL_PRESCALE_REG) &
                                            PLL_PRESCALE_AUD_MASK) >> PLL_PRESCALE_AUD_SHIFT);
            #ifdef USING_FLOATING_POINT
            MPG_pll_nom_config_int = (NOM_FREQ * mpg_pll_prescale * clk27_postdiv)
                                               / (double)xtal_frequency;
            MPG_pll_nom_config_frac = (u_int32)(((NOM_FREQ * mpg_pll_prescale * clk27_postdiv)
                                               / (double)xtal_frequency) * (1<<25)) & 0x1FFFFFF;
            #else
            MPG_pll_nom_config_int = calculate_multiplier ( NOM_FREQ * mpg_pll_prescale * clk27_postdiv,
                                                            xtal_frequency );
            MPG_pll_nom_config_frac = MPG_pll_nom_config_int & 0x1FFFFFF;
            MPG_pll_nom_config_int = MPG_pll_nom_config_int >> 25;
            #endif
          #endif                                     
        #endif
        
        trace_new(TRACE_LEVEL_1 | TRACE_CR,
            "Setting MPG PLL nominal Int/Frac %x/%x\n",
            MPG_pll_nom_config_int, MPG_pll_nom_config_frac);
       /* set system clock to initial frequency */
       writePLL(PLL_LOCK_MPEG, -1, MPG_pll_nom_config_int, MPG_pll_nom_config_frac);

    }
    /* filter parameter init */
    ccrCurrentFilterInit();
    offset = 0;
    reenable_sync = FALSE;
    reenable_aud_sync = FALSE;
    SyncBlackout = FALSE;
    wait4newSTC = 1;
    PCR_PID_changed = FALSE;

    /* enable PCR timer */
    CNXT_SET_VAL(lpPCRTimer, MPG_PCR_TIMER_INFO_ENABLETIMER_MASK, ccrPCRTimerEnable);

    /* XXXAMY move to appropriate place */
    tmpCtrl1 = *glpCtrl1;
    CNXT_SET_VAL(&tmpCtrl1, MPG_CONTROL1_ENCVIDEOENABLE_MASK, 1);
    CNXT_SET_VAL(&tmpCtrl1, MPG_CONTROL1_ENCAUDIOENABLE_MASK, 1);
#if PARSER_TYPE==DTV_PARSER
    CNXT_SET_VAL(&tmpCtrl1, MPG_CONTROL1_DSSMODE_MASK, 1);
#endif
    *glpCtrl1 = tmpCtrl1;
    return TRUE;
}

void clk_rec_ISRNotifyRegister(u_int32 interrupt_bit, char vendor_id,
                               clk_rec_ISRNotify_t handler)
{
    /* Vendor ID is now ignored. This allows anyone to register callbacks */
    /* for these interrupts. The parameter is left in place to keep the   */
    /* API from changing.                                                 */
    
    vendor_id = vendor_id; /* Prevent a warning */
    
    if (interrupt_bit == MPG_VID_PTS_MATURED)
        vb_VIDPTSMATURED_notify = handler;
    if (interrupt_bit == MPG_VID_PTS_ARRIVED)
        vb_VIDPTSARRIVED_notify = handler;
}

void clk_rec_PCRNotifyRegister(clk_rec_PCRNotify_t handler)
{
    dmx_PCR_notify = handler;
}

void clk_rec_PCRFailCBRegister( char vendor_id, clk_rec_PCRFailCB_t handler)
{
    /* Vendor ID is now ignored. This allows anyone to register callbacks */
    /* for these interrupts. The parameter is left in place to keep the   */
    /* API from changing.                                                 */
    
    vendor_id = vendor_id; /* Prevent a warning */
    
    vbPCRFailCallback = handler;
}

void clk_rec_PCRDiscontinuityCBRegister(char vendor_id, clk_rec_PCRFailCB_t handler)
{
    /* Vendor ID is now ignored. This allows anyone to register callbacks */
    /* for these interrupts. The parameter is left in place to keep the   */
    /* API from changing.                                                 */
    
    vendor_id = vendor_id; /* Prevent a warning */
    
    vbPCRDiscontinuityCallback = handler;
}

/** private functions **/

#ifdef USING_FLOATING_POINT
double ccrSimpleFilter(int32 delta)
{
    double ferror;
    double frequency;

    /* if delta is larger than MAX_JITTER + MAX_DIFFERENCE in .2 seconds,
       reload the STC to avoid buffer underflows and overflows.  */
    /* MAX_JITTER for this algorithm is 50us.  MAX_DIFFERENCE is
       .2 seconds * (27000810 - 26999190), however I have loosened
       this reqirement considerably.     */
    if (abs(delta) > (2000000)) {
        CNXT_SET_VAL(glpCtrl0, MPG_CONTROL0_LOADBACKUPSTC_MASK, 1);
        trace_new(TRACE_LEVEL_3|TRACE_CR, "Reloading STC to avoid under/overflows\n");
    }

    if (phi > PHI_MIN) phi = phi * PHI_BASE;
    else phi = PHI_MIN;
    ferror = (phi * delta) + ((1-phi) * last_ferror);

#ifdef CR_TRACE
    if ((ferror - last_ferror) > err_delta_max) err_delta_max = ferror - last_ferror;
    if ((ferror - last_ferror) < err_delta_min) err_delta_min = ferror - last_ferror;
#endif /* CR_TRACE */

    if (abs(ferror - last_ferror) > MAX_CHANGE) {
        /* algorithm is not sufficient for this jitter */
        /* Don't change clock too much */
        if (ferror > last_ferror)
            ferror = last_ferror + MAX_CHANGE;
        else ferror = last_ferror - MAX_CHANGE;
    }

    /* Don't let clock wander more than 50 ppm from nominal */
    if (ferror > FERROR_MAX) {
        ferror = FERROR_MAX;
        /* reload the STC to avoid buffer overflows */
        //CNXT_SET_VAL(glpCtrl0, MPG_CONTROL0_LOADBACKUPSTC_MASK, 1);
    }
    if (ferror < FERROR_MIN) {
        ferror = FERROR_MIN;
        /* reload the STC to avoid buffer underflows */
        //CNXT_SET_VAL(glpCtrl0, MPG_CONTROL0_LOADBACKUPSTC_MASK, 1);
    }
    
    last_ferror = ferror;
    frequency = NOM_FREQ + (GAIN * ferror);

    return frequency;
}

void ccrSimpleFilterInit(void)
{
    phi = 1.0;
    last_ferror = 0.0;
}
#else
int32 ccrSimpleFilter(int32 delta)
{
   int32 ferror, ferror_remain, frequency;

   /* if delta is larger than MAX_JITTER + MAX_DIFFERENCE in .2 seconds,
      reload the STC to avoid buffer underflows and overflows.  */
   /* MAX_JITTER for this algorithm is 50us.  MAX_DIFFERENCE is
      .2 seconds * (27000810 - 26999190), however I have loosened
      this reqirement considerably.     */
   if ( abs ( delta ) > 2000000 )
   {
      CNXT_SET_VAL(glpCtrl0, MPG_CONTROL0_LOADBACKUPSTC_MASK, 1);
      trace_new(TRACE_LEVEL_3|TRACE_CR, "Reloading STC to avoid under/overflows\n");
   }

   if ( clk_rec_step < max_changing_step )
   {
      clk_rec_step ++;
      ferror = phi_denominator[clk_rec_step-1] * ( phi_numerator[clk_rec_step] * delta 
               + ( phi_denominator[clk_rec_step] - phi_numerator[clk_rec_step] ) * last_ferror )
               + phi_denominator[clk_rec_step] * last_ferror_remain;
      ferror = ferror / ( phi_denominator[clk_rec_step] * phi_denominator[clk_rec_step-1] );
      ferror_remain = ferror % ( phi_denominator[clk_rec_step] * phi_denominator[clk_rec_step-1] );
   }
   else
   {
      ferror = phi_numerator[clk_rec_step] * delta 
               + ( phi_denominator[clk_rec_step] - phi_numerator[clk_rec_step] ) * last_ferror
               + last_ferror_remain;
      ferror = ferror / phi_denominator[clk_rec_step];
      ferror_remain = ferror % phi_denominator[clk_rec_step];
   }


#ifdef CR_TRACE
   if ((ferror - last_ferror) > err_delta_max) err_delta_max = ferror - last_ferror;
   if ((ferror - last_ferror) < err_delta_min) err_delta_min = ferror - last_ferror;
#endif /* CR_TRACE */

   if (abs(ferror - last_ferror) > MAX_CHANGE)
   {
      /* algorithm is not sufficient for this jitter */
      /* Don't change clock too much */
      if (ferror > last_ferror)
      {
         ferror = last_ferror + MAX_CHANGE;
      }
      else
      {
         ferror = last_ferror - MAX_CHANGE;
      }
   }

   /* Don't let clock wander more than 50 ppm from nominal */
   if (ferror > FERROR_MAX)
   {
      ferror = FERROR_MAX;
      ferror_remain = 0;
      /* reload the STC to avoid buffer overflows */
      //CNXT_SET_VAL(glpCtrl0, MPG_CONTROL0_LOADBACKUPSTC_MASK, 1);
   }
   if (ferror < FERROR_MIN)
   {
      ferror = FERROR_MIN;
      ferror_remain = 0;
      /* reload the STC to avoid buffer underflows */
      //CNXT_SET_VAL(glpCtrl0, MPG_CONTROL0_LOADBACKUPSTC_MASK, 1);
   }
    
   last_ferror = ferror;
   last_ferror_remain = ferror_remain;
   frequency = NOM_FREQ + ferror * GAIN_NUMERATOR / GAIN_DENOMINATOR;

   return frequency;
}

void ccrSimpleFilterInit(void)
{
   clk_rec_step = 0;
   last_ferror = 0;
   last_ferror_remain = 0;
}
#endif

static int32 aud_stc_pts_diff ( void )
{
#if PARSER_TYPE == DVB_PARSER
   LPREG pAudStcPtsDiffReg = (LPREG)0x1000f300;
   int32 stc_pts_diff;

   /* read out STC-PTS */
   stc_pts_diff = *pAudStcPtsDiffReg;
   *pAudStcPtsDiffReg = 0;

   /* check the valid bit of STC-PTS */
   if ( ( stc_pts_diff & 0x10 ) == 0 )
   {
      /* STC-PTS not valid */
      return audio_tick_diff;
   }

   stc_pts_diff = ( stc_pts_diff >> 8 ) & 0x1fffff;


   if ( stc_pts_diff > MAX_AUDIO_STC_OFFSET )
   {
      /* STC-PTS exceeds its max possible value, can't be valid */
      trace_new ( TRACE_LEVEL_3 | TRACE_CR, "stc_pts_diff: %d\n", stc_pts_diff );
      return audio_tick_diff;
   }

   if ( stc_pts_diff > MAX_AUDIO_ADJUSTABLE_STC_OFFSET )
   {
      stc_pts_diff = MAX_AUDIO_ADJUSTABLE_STC_OFFSET;
   }

   audio_tick_diff = stc_pts_diff - TARGET_AUDIO_STC_OFFSET;

   return audio_tick_diff;
#else
   return 0;    /* no audio clk recover for DirecTV for now */
#endif
}


/********************************************************
 * RETURNS:  returns the difference between stc and pts
 *           in ticks of the 27MHz clock.
 *           If the returned value is negative, the STC
 *           is greater than the PCR.  
 *           The range of the return value is -0x40000000
 *           to +0x40000000.  A return value of 0x40000000
 *           indicates that the difference was 0x40000000
 *           or greater.  A return value of -0x40000000
 *           indicates that the difference was -0x40000000
 *           or less.
 ********************************************************/


int32 ccrPcrDifference(void)
{
    int32 difference;
    int8 sign;

#if PARSER_TYPE == DVB_PARSER
    int32 HiStc, HiPcr;
    int32 LoStc, LoPcr;
    int32 HiDiff;
    ClkRef stc, pcr;

    stc.High = *glpSTCSnapHi;
    pcr.High = *glpPCRHigh;
    stc.Low = *glpSTCSnapLo;
    pcr.Low = *glpPCRLow;

    
    LoStc = CNXT_GET_VAL(&stc.High, MPG_PSCRS_HI_EXTENSION_MASK) + (300 * (stc.Low & 0x7));
    LoPcr = CNXT_GET_VAL(&pcr.High, MPG_PSCRS_HI_EXTENSION_MASK) + (300 * (pcr.Low & 0x7));

    HiStc = (stc.Low / 8) + (CNXT_GET_VAL(&stc.High, MPG_PSCRS_HI_HIGHBIT_MASK) << 29);
    HiPcr = (pcr.Low / 8) + (CNXT_GET_VAL(&pcr.High, MPG_PSCRS_HI_HIGHBIT_MASK) << 29);

    if (HiPcr >= HiStc) {
        sign = 1;
        HiDiff = HiPcr - HiStc;
    } else {
        sign = -1;
        HiDiff = HiStc - HiPcr;
    }

#ifdef TRACE_CR
    if(HiStc > 0x3FFFFC00)
      trace_new(TRACE_LEVEL_3 | TRACE_CR, 
		"Most signficant bits of STC are 0x%08x, rollover imminent!!!\n", HiStc);
    if(HiStc < 0x00000200)
      trace_new(TRACE_LEVEL_3 | TRACE_CR, 
		"Most signficant bits of STC are 0x%08x, just rolled over!!!\n", HiStc);
#endif

    /* if the difference is greater than half of the PCR range
      (approximately 13 hours) use the *other* distance.  It
      will be smaller.  [the shortest distance between 1 and
      0x1FFFFFFFF is 2 since the STC wraps back to 0.] */
    if (HiDiff & 0x20000000) {
        sign = sign * -1;
        HiDiff = (0x3FFFFFFF - HiDiff) + 1;
    }

    if (HiDiff >= (0x40000000/300)/8)
        return (sign * 0x40000000);

    difference = (HiDiff * sign * 300 * 8) + LoPcr - LoStc;

#else  /* PARSER_TYPE == DTV_PARSER */

    u_int32 pcr32, stc32, pcrStcDiff;

    stc32 = *glpSTCSnapLo;
    pcr32 = *glpPCRLow;
    
    if ( pcr32 > stc32 )
    {
        pcrStcDiff = pcr32 - stc32;
        sign = 1;
    }
    else
    {
        pcrStcDiff = stc32 - pcr32;
        sign = -1;
    }

    if ( pcrStcDiff & 0x80000000 )
    {
        pcrStcDiff = 0xffffffff - pcrStcDiff;
        sign = -sign;
    }

    difference = sign * (int32)pcrStcDiff;

#endif
        
    return difference;
}


void clk_rec_suspend(void)
{
   bool bCSState;
   
   bCSState = critical_section_begin();
   writePLL(PLL_LOCK_MPEG, -1, MPG_pll_nom_config_int, MPG_pll_nom_config_frac);
   writePLL(PLL_LOCK_AUDIO, -1, -1, current_nom_pll_config_frac);
   bClkRecEnabled = FALSE;   
   critical_section_end(bCSState);
}

void clk_rec_resume(void)
{
   bool bCSState;
   
   bCSState = critical_section_begin();
   bClkRecEnabled = TRUE;
   bForceDiscontinuity = TRUE;
   critical_section_end(bCSState);
}

/*
 ** writePLL
 *
 *  FILENAME: C:\Builds\latest\CLKREC\nechescr.c
 *
 *  PARAMETERS:
 *
 *  DESCRIPTION:
 *
 *  RETURNS:
 *
 */

void writePLL( u_int8 whichPLL, int32 divider, int32 integral, u_int32 fractional)
{
    
    bool kstate;

#ifndef WABASH_AVSYNC_CLKREC_DISABLED
    if (bClkRecEnabled)
    {
        if (!((whichPLL==PLL_LOCK_MPEG) || (whichPLL==PLL_LOCK_AUDIO))) return;

        kstate = critical_section_begin();
        PLL_UNLOCK()
        *((LPREG)PLL_LOCK_STAT_REG) &= ~whichPLL;

        if (whichPLL==PLL_LOCK_MPEG)
        {
            /*
             * UPDATE VIDEO PLL
             */

            /*
             * Write either just the fractional value or both fractional and integral
             */
            if (integral >= 0)
            {
                *lpPLL_MPEG = ((integral << PLL_VID_CONFIG_INT_SHIFT) & PLL_VID_CONFIG_INT_MASK) |
                              ((fractional << PLL_VID_CONFIG_FRAC_SHIFT) & PLL_VID_CONFIG_FRAC_MASK);
            }
            else
            {
                *lpPLL_MPEG = (*lpPLL_MPEG & ~PLL_VID_CONFIG_FRAC_MASK) |
                              ((fractional << PLL_VID_CONFIG_FRAC_SHIFT) & PLL_VID_CONFIG_FRAC_MASK);
            }
        }
        else
        {
            /*
             * UPDATE AUDIO PLL
             */

            /*
             * Write to separate divider register
             */
            if (divider >= 0)
            {
                *((LPREG)PLL_LOCK_STAT_REG) &= ~PLL_LOCK_ALLDIVIDERS;
                /* write divider first if frequency is decreasing, otherwise
                   write int & frac first */
                *lpPLL_Div1 = (*lpPLL_Div1 & ~PLL_DIV1_ASPL_CLK_MASK) |
                              (((u_int32)divider << PLL_DIV1_ASPL_CLK_SHIFT) & PLL_DIV1_ASPL_CLK_MASK);
                *((LPREG)PLL_LOCK_STAT_REG) |= PLL_LOCK_ALLDIVIDERS;
            }

            /*
             * Write either just the fractional value or both fractional and integral
             */
            if (integral >= 0)
            {
                *lpPLL_Aud = ((integral << PLL_AUD_CONFIG_INT_SHIFT) & PLL_AUD_CONFIG_INT_MASK) |
                             ((fractional << PLL_AUD_CONFIG_FRAC_SHIFT) & PLL_AUD_CONFIG_FRAC_MASK);
            }
            else
            {
                *lpPLL_Aud = (*lpPLL_Aud & ~PLL_AUD_CONFIG_FRAC_MASK) |
                             ((fractional << PLL_AUD_CONFIG_FRAC_SHIFT) & PLL_AUD_CONFIG_FRAC_MASK);
            }
        }
      

        *((LPREG)PLL_LOCK_STAT_REG) |= whichPLL;
        PLL_LOCK()


        critical_section_end(kstate);
    }
#endif
    return;
}


/*
 ** clk_rec_mpeg_isr
 *
 *  FILENAME: D:\build2\sabine\clkrec\clk_recov.c
 *
 *  PARAMETERS:
 *
 *  DESCRIPTION:
 *
 *  RETURNS:
 *
 */

int clk_rec_mpeg_isr( u_int32 interrupt_ID, bool isFIQ, PFNISR *previous )
{
    u_int32 IRQReason = *glpIntStatus & (MPG_PCR_DISC | MPG_PCR_RECEIVED
                                         | MPG_VID_PTS_ARRIVED | MPG_VID_PTS_MATURED);

    *glpIntStatus = IRQReason;

    if ((IRQReason & MPG_PCR_DISC) != 0) {
        isr_trace_new(TRACE_LEVEL_4 | TRACE_CR, "ISR:  MPEG discontinuity detected!\n", 0, 0);

        /* treat as if there is no discontinuity */
        /* used to reinitialize filter variables here.  Don't do that anymore */
#ifdef DRIVER_INCL_AUDIO
        first_PTS = TRUE;
#endif
        /* wait for the decoder to reach the new PCR and toggle the STC */
        wait4newSTC = 1;
        /* disable timer here if used */
        CNXT_SET_VAL(lpPCRTimer, MPG_PCR_TIMER_INFO_ENABLETIMER_MASK, 0);
#ifdef CR_TRACE
        discont_trace = TRUE;
#endif

    }

    if ((IRQReason & MPG_PCR_RECEIVED) != 0) {
        isr_trace_new(TRACE_LEVEL_1 | TRACE_CR, "New PCR has arrived.  Starting clock recovery task.\n", 0, 0);
        /* is this a new PCR PID?? */
        if ((NewPCRFlag == FALSE) && (wait4newSTC == 0)) {
            isr_trace_new(TRACE_LEVEL_4 | TRACE_CR, "PCR PID has changed.\n", 0, 0);
            PCR_PID_changed = TRUE;
        }
        /* start the system clock recovery task */
        sem_put( clk_rec_sem );

        if (dmx_PCR_notify != NULL) {
            dmx_PCR_notify(*glpPCRHigh, *glpPCRLow);
        }
          
#ifdef CR_TRACE
        pcr_count++;
#endif
    }

    if ((IRQReason & MPG_VID_PTS_ARRIVED) != 0) {
        isr_trace_new(TRACE_LEVEL_1 | TRACE_CR, "NEW_VID_PTS_RECEIVED. PTS_HI %x PTS_LO %x \n", *glpVPTSHi, *glpVPTSLo);

        if (vb_VIDPTSARRIVED_notify != NULL) vb_VIDPTSARRIVED_notify();

    }

    if ((IRQReason & MPG_VID_PTS_MATURED) != 0) {
        
        if (vb_VIDPTSMATURED_notify != NULL) vb_VIDPTSMATURED_notify();

    }

#if 0 /* bit removed in Colorado, may reappear in pawser if needed */
    if ((IRQReason & MPG_VID_DATA_ACCEPTED) != 0) {
        isr_trace_new(TRACE_LEVEL_2 | TRACE_CR, "New VIDEO data has arrived.\n", 0, 0);
        /* XXXAMY need to catch MPG_VB_EMPTY to oppose this */
        *glpIntMask &= ~MPG_VID_DATA_ACCEPTED;

    }
#endif /* COMMENT */

    if (*glpIntStatus == 0)
        return RC_ISR_HANDLED;
    else {
        *previous = clk_previous_mpeg_isr;
        return RC_ISR_NOTHANDLED;
    }
}


/*
 ** clk_rec_task
 *
 *  FILENAME: D:\build2\sabine\clkrec\clk_recov.c
 *
 *  PARAMETERS:
 *
 *  DESCRIPTION:
 *
 *  RETURNS:
 *
 */
void clk_rec_task( void *arg )
{
    HW_DWORD     tmpCtrl0;
    int32 delta;
#ifdef USING_FLOATING_POINT     
    double frequency_new;
    double aud_freq_new;
#else
    int32 frequency_new;
    int32 aud_freq_new;
    int32 aud_stc_pts_offset;
#endif
    u_int32 s_fraction, a_fraction, s_integer;
#if STC_OFFSET != 0
    u_int32 temp, tempHi;
    bool readNewOffset = FALSE;
#endif
    int sem_return;
    u_int32 timeout = 300;  /* if more than .3 seconds elapses 
                               between PCRs, we have lost the stream. */

    while (1) {
        sem_return = sem_get (clk_rec_sem, timeout);
        if (sem_return == RC_KAL_TIMEOUT) {
            /* no PCR in .3 seconds.  PCR playout has failed */
            if (vbPCRFailCallback != NULL) vbPCRFailCallback();
            timeout = 300;
            continue;
        }
        if (sem_return != RC_OK) continue;
        
        /* we have a valid PCR.  If more than .3 seconds elapses before
           the next one, it is an error */
        timeout = 300;

        delta = ccrPcrDifference();

#ifdef CR_TRACE
        if (delta > raw_err_max) raw_err_max = delta;
        if (delta < raw_err_min) raw_err_min = delta;
#endif /* CR_TRACE */
#if (STC_OFFSET!=0)           
        /* Is this the first PCR after an offset has been added? */
        if (readNewOffset == TRUE) {
            if (abs(delta-offset) < DISCONT_RETURN_THRESHOLD) offset = delta;
            readNewOffset = FALSE;
        }
#endif /* STC_OFFSET*/

        /* Is this the first good PCR following a discontinuity? */
        if (wait4newSTC >= 1) {
            if (abs(delta) > DISCONT_RETURN_THRESHOLD) {
               trace_new(TRACE_LEVEL_3 | TRACE_CR, "Waiting for new STC\n");
               wait4newSTC++;

               if (wait4newSTC > 5) {
                   wait4newSTC = 1;
                   trace_new(TRACE_LEVEL_3 | TRACE_CR, 
                     "WARNING:  Expected new STC never loaded. Will load manually.\n");
                   trace_new(TRACE_LEVEL_3 | TRACE_CR, " delta is %x, offset is %f \n", delta, offset);
                   tmpCtrl0 = *glpCtrl0;
                   CNXT_SET_VAL(&tmpCtrl0, MPG_CONTROL0_LOADBACKUPSTC_MASK, 1); 
                   *glpCtrl0 = tmpCtrl0;
               }
               /* In the middle of a discontinuity.  Leave clocks alone.
                  Wait for next PCR */
               continue;
            }
            /* 1st new PCR following discontinuity/startup */
            /* if using offset, call function here */
#if (STC_OFFSET!=0)           
            /* STC -= offset */
            temp = *glpSTCLo;
            tempHi = *glpSTCHi;
            if (temp < STC_OFFSET) continue;  /* wait for next round.  STC will be 
                            big enough to subtract STC_OFFSET without underflow. */
            trace_new(TRACE_LEVEL_3 | TRACE_CR, "STCLo was %x\n", temp);
            offset = STC_OFFSET;
            temp = temp - offset;
            *glpSTCLo = temp;
            *glpSTCHi = tempHi;
            trace_new(TRACE_LEVEL_3 | TRACE_CR, "STCLo should become %x\n", temp);
            trace_new(TRACE_LEVEL_3 | TRACE_CR, "STCLo is now %x\n", *glpSTCLo);
            delta = offset;
            readNewOffset=TRUE;
#endif /* STC_OFFSET*/
            wait4newSTC = 0;
            PCR_PID_changed = FALSE;
            /* reenable timer here if needed */
            CNXT_SET_VAL(lpPCRTimer, MPG_PCR_TIMER_INFO_ENABLETIMER_MASK, ccrPCRTimerEnable);
            restart_avsync();
        }

        if (((u_int32)abs(delta) > ccrDiscontThreshold) || PCR_PID_changed || bForceDiscontinuity) {
            /* handle discontinuity */
            /* disable sync */
            temp_disable_avsync();
            /* set clocks to nominal */
            writePLL(PLL_LOCK_MPEG, -1, MPG_pll_nom_config_int, MPG_pll_nom_config_frac);
            writePLL(PLL_LOCK_AUDIO, -1, -1, current_nom_pll_config_frac);
            current_PLL_frequency = NOM_FREQ;
            /* reinit filter vars */
            ccrCurrentFilterInit();
            /* notify application */
            if (!PCR_PID_changed && (vbPCRDiscontinuityCallback != NULL)) vbPCRDiscontinuityCallback();
            /* concept of BIG_JITTER no longer exists */
            tmpCtrl0 = *glpCtrl0;
            CNXT_SET_VAL(&tmpCtrl0, MPG_CONTROL0_LOADBACKUPSTC_MASK, 1);
            *glpCtrl0 = tmpCtrl0;
            wait4newSTC = 1;
            bForceDiscontinuity = FALSE;
            continue;
        }

        frequency_new = ccrCurrentFilter(delta);

        if ( abs ( frequency_new - NOM_FREQ ) > FREQ_OFFSET_MAX ) 
        {
            /* recovered frequency is outside allowable range */
            trace_new(TRACE_LEVEL_3 | TRACE_CR, "WARNING:  RECOVERED CLOCK FREQUENCY IS OUT OF RANGE.\n");
            trace_new(TRACE_LEVEL_2 | TRACE_CR, "If stream is not live, verify that it is being fed at the correct bit rate.\n");
            trace_new(TRACE_LEVEL_2 | TRACE_CR, "If stream is live, verify that the crystal frequency is set corretly in the source code.\n");
            trace_new(TRACE_LEVEL_3 | TRACE_CR, "Recovered frequency is: %lf\n", frequency_new);

            /* consider this a discontinuity, of sorts */
            /* disable sync */
            temp_disable_avsync();
            /* don't set clocks to nominal */
            /* if we set clocks to nominal, it will cause flashing in the OSD
               on PAL systems, due to the single large change in the clock */

            /* Filter should perform any necessary re-initialization */

            tmpCtrl0 = *glpCtrl0;
            CNXT_SET_VAL(&tmpCtrl0, MPG_CONTROL0_LOADBACKUPSTC_MASK, 1);
            *glpCtrl0 = tmpCtrl0;
            wait4newSTC = 1;

            continue;
        }

        /* compute audio PLL frequency */
#ifdef USING_FLOATING_POINT
        aud_freq_new = (double)current_nom_aud_freq * ( frequency_new / (double)NOM_FREQ )
                         * ( 1.0 + (double)aud_stc_pts_diff () / (double)AUDIO_FREQ_ADJUST_FACTOR );
#else
        aud_stc_pts_offset = aud_stc_pts_diff ();
        aud_freq_new = current_nom_aud_freq 
                        + ( frequency_new - NOM_FREQ ) * ( current_nom_aud_freq >> 6 ) / ( NOM_FREQ >> 6 );
        aud_freq_new += aud_stc_pts_offset * ( aud_freq_new / AUDIO_FREQ_ADJUST_FACTOR )
                          + aud_stc_pts_offset * ( aud_freq_new % AUDIO_FREQ_ADJUST_FACTOR )
                                / AUDIO_FREQ_ADJUST_FACTOR;
#endif         

        /* program the system and audio PLLs */
        /* reprogram just the 25 bit fractional part of the PLL registers */
#ifdef USING_FLOATING_POINT
        #if PLL_TYPE == PLL_TYPE_COLORADO
            s_fraction = (u_int32)(((frequency_new * clk27_postdiv)
                                  / (double)xtal_frequency) * (1<<25)) & 0x1FFFFFF;
            a_fraction = (u_int32)((double)((aud_freq_new * apll_divisor * 2)
                                  / (double)xtal_frequency) * (1<<25)) & 0x1FFFFFF;
        #else 
          #if PLL_TYPE == PLL_TYPE_WABASH
            s_fraction = (u_int32)(((frequency_new * mpg_pll_prescale * clk27_postdiv * 2)
                                  / (double)xtal_frequency) * (1<<25)) & 0x1FFFFFF;
            a_fraction = (u_int32)((double)((aud_freq_new * mpg_pll_prescale * apll_divisor * 2 * 2)
                                  / (double)xtal_frequency) * (1<<25)) & 0x1FFFFFF;
           #else /* PLL_TYPE_BRAZOS and beyond */
            s_fraction = (u_int32)(((frequency_new * mpg_pll_prescale * clk27_postdiv)
                                  / (double)xtal_frequency) * (1<<25)) & 0x1FFFFFF;
            a_fraction = (u_int32)((double)((aud_freq_new * mpg_pll_prescale * apll_divisor * 2)
                                  / (double)xtal_frequency) * (1<<25)) & 0x1FFFFFF;
           #endif                       
        #endif
#else
        #if PLL_TYPE == PLL_TYPE_COLORADO
            s_fraction = calculate_multiplier ( frequency_new * clk27_postdiv,
                                                xtal_frequency ) & 0x1FFFFFF;
            a_fraction = calculate_multiplier ( aud_freq_new * apll_divisor * 2,
                                                xtal_frequency ) & 0x1FFFFFF;
        #else 
          #if PLL_TYPE == PLL_TYPE_WABASH
            s_fraction = calculate_multiplier ( frequency_new * mpg_pll_prescale * clk27_postdiv * 2,
                                                xtal_frequency ) & 0x1FFFFFF;
            a_fraction = calculate_multiplier ( aud_freq_new * mpg_pll_prescale * apll_divisor * 2 * 2,
                                                xtal_frequency ) & 0x1FFFFFF;
           #else /* PLL_TYPE_BRAZOS and beyond */
            s_fraction = calculate_multiplier ( frequency_new * mpg_pll_prescale * clk27_postdiv,
                                                xtal_frequency ) & 0x1FFFFFF;
            s_integer = calculate_multiplier ( frequency_new * mpg_pll_prescale * clk27_postdiv,
                                                xtal_frequency ) & 0x7E000000;
						s_integer = (s_integer >> 25);
            a_fraction = calculate_multiplier ( aud_freq_new * mpg_pll_prescale * apll_divisor * 2,
                                                xtal_frequency ) & 0x1FFFFFF;
           #endif                       
        #endif
#endif

        writePLL(PLL_LOCK_MPEG, -1, s_integer, s_fraction);
        writePLL(PLL_LOCK_AUDIO, -1, -1, a_fraction);

        current_PLL_frequency = frequency_new;

#ifdef CR_TRACE
        if (frequency_new > freq_max) freq_max = frequency_new;
        else if (frequency_new < freq_min) freq_min = frequency_new;
        if (frequency_new > 27000810) trace_new(TRACE_LEVEL_3 | TRACE_CR, 
                "WARNING: recovered frequency greater than 27000810 %lf\n", frequency_new);
#if CUSTOMER != VENDOR_C
        if (frequency_new < 26999190) trace_new(TRACE_LEVEL_3 | TRACE_CR, 
                "WARNING: recovered frequency less than 26999190 %lf\n", frequency_new);
#endif /* !VENDOR_C */
#endif /* CR_TRACE */
    } /* while 1 */
    task_terminate();
}

#ifdef CR_TRACE

void clk_rec_print_report( void )
{
    HW_DWORD     tmpCtrl0;
    HW_DWORD     tmpCtrl1;


    LPREG aud_frame = (LPREG)MPG_AUDIO_DROP_CNT_REG;
    LPREG vid_frame = (LPREG)MPG_FRAME_DROP_CNT_REG;

    trace_new(TRACE_CR_SUM, "\n------Clock Recovery Statistics------\n");
    trace_new(TRACE_CR_SUM, "Audio Buffer Fullness:  current    High    Low     Average\n");
    trace_new(TRACE_CR_SUM, "\t\t\t%d\t%d", abuff_level, abuff_full_max);
    trace_new(TRACE_CR_SUM, "\t%d\t%d\n", abuff_full_min, abuff_full_avg);
    trace_new(TRACE_CR_SUM, "Video Buffer Fullness:  current    High    Low     Average\n");
    trace_new(TRACE_CR_SUM, "\t\t\t%d\t%d", vbuff_level, vbuff_full_max);
    trace_new(TRACE_CR_SUM, "\t%d\t%d\n", vbuff_full_min, vbuff_full_avg);
    trace_new(TRACE_CR_SUM, "System Clock Frequency:  current   Max             Min\n");
    trace_new(TRACE_CR_SUM, "\t\t\t%f\t%f", current_PLL_frequency, freq_max);
    trace_new(TRACE_CR_SUM, "\t%f\n", freq_min );
    trace_new(TRACE_CR_SUM, "Filtered Error Change:  Max                Min             \n");
    trace_new(TRACE_CR_SUM, "\t\t\t%f\t%f\n", err_delta_max, err_delta_min);
    trace_new(TRACE_CR_SUM, "PCR:  Arrival Frequency    Max Raw Error   Min Raw Error\n");
    trace_new(TRACE_CR_SUM, "\t\t\t%d\t%d", PCR_arrival_freq, raw_err_max);
    trace_new(TRACE_CR_SUM, "\t%d\n", raw_err_min);
    trace_new(TRACE_CR_SUM, "Frame drop/repeat counters:    Audio           Video  \n");
    trace_new(TRACE_CR_SUM, "\t\t\t\t%d/%d", 
	      CNXT_GET_VAL(aud_frame, MPG_FRAME_DROP_CNT_DROP_MASK), 
	      CNXT_GET_VAL(aud_frame, MPG_FRAME_DROP_CNT_REPEAT_MASK));
    trace_new(TRACE_CR_SUM, "\t\t%d/%d\n", 
	      CNXT_GET_VAL(vid_frame, MPG_FRAME_DROP_CNT_DROP_MASK), 
	      CNXT_GET_VAL(vid_frame, MPG_FRAME_DROP_CNT_REPEAT_MASK));
    tmpCtrl0 = *glpCtrl0;
    tmpCtrl1 = *glpCtrl1;
    trace_new(TRACE_CR_SUM, "Audio sync enabled:  %d  Video sync enabled:  %d\n", 
	      CNXT_GET_VAL(&tmpCtrl1, MPG_CONTROL1_ENABLEAUDSYNC_MASK), 
	      CNXT_GET_VAL(&tmpCtrl0, MPG_CONTROL0_ENABLESYNC_MASK));
}

void buffer_monitor_task( void *arg)
{
    char * read_ptr;  /* hardware encoded audio/video buffer read pointer */
    char * write_ptr; /* hardware encoded audio/video buffer write pointer */
    int bytes_left;
    int count = 0;
    int last_abuff_level = 0;
    int last_vbuff_level = 0;
    int abuff_level_sum = 0;
    int vbuff_level_sum = 0;
    int sleep_time = 100;  /* 100 ms */

    /* Initialize */
    abuff_full_max = vbuff_full_max = 0;
    abuff_full_min = vbuff_full_min = 100;
    freq_max = freq_min = NOM_FREQ;
    err_delta_max = err_delta_min = 0;
    freq_delta_avg = 0;
    raw_err_max = raw_err_min = 0;
    pcr_count = 0;
    discont_trace = FALSE;

    while(1) {
        task_time_sleep(sleep_time);

        count++;

	if(cnxt_dmx_query_status(gDemuxInstance) == DMX_STATUS_OK) {
	  /*  get buffer pointers */
	  read_ptr = (char *)((u_int32)*DPS_AUDIO_READ_PTR_EX(gDemuxInstance) & ~0x80000000);
	  write_ptr = (char *)((u_int32)*DPS_AUDIO_WRITE_PTR_EX(gDemuxInstance) & ~0x80000000);
	  /*  find fullness level */
	  if (read_ptr <= write_ptr) bytes_left = write_ptr - read_ptr;
	  else bytes_left = write_ptr + HWBUF_ENCAUD_SIZE - read_ptr;
	  abuff_level = (bytes_left * 100) / HWBUF_ENCAUD_SIZE;
	  /*  get buffer pointers */
	  read_ptr = (char *)((u_int32)*DPS_VIDEO_READ_PTR_EX(gDemuxInstance) & ~0x80000000);
	  write_ptr = (char *)((u_int32)*DPS_VIDEO_WRITE_PTR_EX(gDemuxInstance) & ~0x80000000);
	  /*  find fullness level */
	  if (read_ptr <= write_ptr) bytes_left = write_ptr - read_ptr;
	  else bytes_left = write_ptr + HWBUF_ENCVID_SIZE - read_ptr;
	  vbuff_level = (bytes_left * 100) / HWBUF_ENCVID_SIZE;
	  /* 
	     if bigger than max,
  	        max gets new
	     if less than min,
	        min gets new
	  */
	  if (abuff_level > abuff_full_max) abuff_full_max = abuff_level;
	  if (abuff_level < abuff_full_min) abuff_full_min = abuff_level;
	  if (vbuff_level > vbuff_full_max) vbuff_full_max = vbuff_level;
	  if (vbuff_level < vbuff_full_min) vbuff_full_min = vbuff_level;
	  /*  sum += new */
	  abuff_level_sum += abuff_level;
	  vbuff_level_sum += vbuff_level;
	  /*  if buffer almost full and buffer fuller than last time warning message */
#if (PVR!=YES)
	  if (abuff_level > 80) trace_new(TRACE_LEVEL_4 | TRACE_CR, " AUDIO BUFFER DANGEROUSLY FULL (over 80 percent)\n");
	  if ((abuff_level > 66) && (abuff_level == abuff_full_max))
            trace_new(TRACE_LEVEL_4 | TRACE_CR, " Audio buffer FILLING, %d percent full\n", abuff_level);
	  if (vbuff_level > 90) trace_new(TRACE_LEVEL_4 | TRACE_CR, " VIDEO BUFFER DANGEROUSLY FULL (over 90 percent)\n");
	  if ((vbuff_level > 85) && (vbuff_level == vbuff_full_max))
            trace_new(TRACE_LEVEL_4 | TRACE_CR, " Video buffer FILLING, %d percent full\n", vbuff_level);
#endif /* (PVR!=YES) */
	  /*  if count = ?? */
	  if ((count >= 300) || (discont_trace == TRUE)) {
            /* avg = sum/count */
            if (count > 0) {
	      abuff_full_avg = abuff_level_sum/count;
	      vbuff_full_avg = vbuff_level_sum/count;
            }
            /* calculate PCR arrival rate */
            if ((sleep_time * count) > 1000) {
	      PCR_arrival_freq = pcr_count / ((sleep_time * count)/1000);
	      /*
		PCR_interval is really 90000/PCR_arrival_freq, but increase it to 
		180000/PCR_arrival_freq
		to allow for jitter.
		The '/8' is added to get rid of the lowest 3 bits.
	      */
	      if (PCR_arrival_freq > 0) 
		PCR_interval = (180000/PCR_arrival_freq) / 8;
	      else PCR_interval = 9000/8;  /* this is 90000/10 /8 */
	      if (PCR_interval > 9000/8) PCR_interval = 9000/8;
            }
            else {
	      PCR_arrival_freq = pcr_count;
	      PCR_interval = 9000/8;
            }
            /*  print report */
            clk_rec_print_report();
            /*  reset variables */
            count = 0;
            abuff_level_sum = 0;
            vbuff_level_sum = 0;
            abuff_full_max = 0;
            abuff_full_min = 100;
            vbuff_full_max = 0;
            vbuff_full_min = 100;
            discont_trace = FALSE;
            pcr_count = raw_err_max;
            raw_err_max = raw_err_min;
            raw_err_min = pcr_count;
            pcr_count = 0;
            freq_max = freq_min = current_PLL_frequency;
            err_delta_max = err_delta_min = 0;
	  }
	  last_abuff_level = abuff_level;
	  last_vbuff_level = vbuff_level;
	}
    }
    task_terminate();
}

#endif /* CR_TRACE */

/****************************************************************************
 * Modifications:
 * $Log:
 ****************************************************************************/

