/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*          Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002          */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        aud_comn.c
 *
 *
 * Description:     Main source file for common audio driver functions
 *                  Contents formerly contained in audio.c
 *
 *
 * Author:          Amy C. Pratt
 *
 ****************************************************************************/
/* $Id: aud_comn.c,v 1.115, 2004-06-28 03:56:54Z, Xiao Guang Yan$
 ****************************************************************************/

/** include files **/
#include "stbcfg.h"
#include "basetype.h"
#include "confmgr.h"
#include "kal.h"
#include "retcodes.h"
#include "globals.h"
#ifdef DRIVER_INCL_GENDMXC
#include "gendemux.h"
#else
#include "demuxapi.h"
#endif
#include "aud_comn.h"
#include "aud_api.h"

/** local definitions **/
/* The Various modules capable of Audio Attenutation */
typedef enum _AUD_ATTN_MODULE
{
   AUD_ATTN_MODULE_PCM_OUTPUT,
   AUD_ATTN_MODULE_DECODER_INPUT,
   AUD_ATTN_MODULE_RFMOD_OUTPUT
} AUD_ATTN_MODULE;

/* default settings */

/** external functions **/
extern void PCMInit(void);
extern bool LoadMicrocode(bool, void *, void *);
extern void StartPCMCaptureAndPlayback(void);
extern void StopPCMCaptureAndPlayback(void);
#ifdef DRIVER_INCL_GENDMXC
extern bool gen_dmx_channel_enabled(u_int32 chid);
#else
extern bool gen_dmx_channel_enabled(u_int32 dmxid, u_int32 chid);
#endif

#if SOFTWARE_AC3_SPDIF_FIX == YES && AUDIO_MICROCODE == UCODE_COLORADO
extern bool ac3_fix_init(void);
#endif /* SOFTWARE_AC3_SPDIF_FIX */

#if CANAL_AUDIO_HANDLING == YES
extern bool datadry_fix_init(void);
#endif /* CANAL_AUDIO_HANDLING */

#if (USE_BRAZOS16_PCM_SW_WORKAROUND == YES)
extern bool Brazos16PCMInit(void);
#endif /* (USE_BRAZOS16_PCM_SW_WORKAROUND == YES) */

/** external data **/
extern unsigned char MPEG_AUDIO_UCODE[];
extern u_int32 MPEG_AUDIO_UCODE_SIZE;
#ifdef DRIVER_INCL_AC3
extern unsigned char AC3_AUDIO_UCODE[];
extern u_int32 AC3_AUDIO_UCODE_SIZE;
#endif
#if AUDIO_MICROCODE == UCODE_BRAZOS
extern unsigned char AOC_AUDIO_UCODE[];
extern u_int32 AOC_AUDIO_UCODE_SIZE;
#endif

extern u_int32 xtal_frequency;

extern bool LastAudPTSMatured;

/** internal functions **/
static void arm_audio_gen_init( bool reinit);
u_int32 cnxt_audio_get_audio_info( void );
static void audio_set_volume_internal(int percentage, AUD_ATTN_MODULE module);
void hw_aud_stop(bool bClearBuffers);
void hw_aud_start(void);

/** public data **/
LPREG lpleft_atten = (LPREG) AUD_LEFT_ATTEN_REG;
LPREG lpright_atten = (LPREG) AUD_RIGHT_ATTEN_REG;
int32 pll_config_48kHz_int, pll_config_44kHz_int, pll_config_32kHz_int,
      pll_config_24kHz_int, pll_config_22kHz_int, pll_config_16kHz_int;
int32 pll_config_48kHz_frac, pll_config_44kHz_frac, pll_config_32kHz_frac,
      pll_config_24kHz_frac, pll_config_22kHz_frac, pll_config_16kHz_frac;
int32 pll_48kHz_divider, pll_44kHz_divider, pll_32kHz_divider,
      pll_24kHz_divider, pll_22kHz_divider, pll_16kHz_divider;
LPREG lpPLL_Div1 = (LPREG)PLL_DIV1_REG;
int32 current_nom_pll_config_int, current_nom_pll_config_frac;
LPMPG_ADDRESS lpAudReadPtr;
LPMPG_ADDRESS lpAudWritePtr;
LPREG lpPTSHi = (LPREG) MPG_PTS_HI_REG;
LPREG lpPTSLo = (LPREG) MPG_PTS_LO_REG;

int16 apll_divisor;
u_int32 current_nom_aud_freq;
u_int32 aud_frametime;
PFNISR previous_mpeg_isr;

/* Audio Output (PCM) module volume */
u_int16 current_volume; /* as a percentage, 0 volume is attenuated by 96dB */
#if ((PCM_AUDIO_TYPE != PCM_AUDIO_COLORADO) && (PCM_AUDIO_TYPE != PCM_AUDIO_WABASH))
/* Audio Decode Input module volume */
u_int16 auddecoder_current_volume;
 #if (CPU_TYPE!=CPU_ARM940T)
 /* RF Modulator Ouput module volume */
 u_int16 rfmod_current_volume;
 #endif /* (CPU_TYPE!=CPU_ARM940T) */
#endif /* PCM_AUDIO_TYPE != PCM_AUDIO_COLORADO && PCM_AUDIO_WABASH */

bool SyncOn = FALSE;
mixer_volume_func_t MPEGVolumeFunc = NULL;
mixer_volume_func_t G729VolumeFunc = NULL;
bool gbRestoreAudioAfterReset = FALSE;
bool cnxt_audio_mono = FALSE;       /* Flag for Mono vs Stereo (default) */
bool gbAudioDroppedOut = FALSE;
u_int32 guDropoutTime = 0;
u_int32 guiAudioInfo = 0;

/* Minimum time to wait after muting on a data dropout before unmuting */
#define AUTO_UNMUTE_HOLDOFF_PERIOD 200

extern u_int32 gDemuxInstance;

extern int RAMWidth;

u_int32 ab_full_cnt = 0;
u_int32 ab_empty_cnt = 0;

/** private data **/
    /* attenuation coefficient values are not evenly spaced.  Use this
       table to look them up */
static u_int8 atten_coeff_tbl[7] = { 0x0, 0xF, 0xD, 0xC, 0xB, 0xA, 0x9 };
static gen_aud_ISRNotify_t vb_ABFULL_notify = NULL;
static gen_aud_ISRNotify_t vb_NEWPTS_notify = NULL;
static gen_aud_ISRNotify_t vb_PTSMATURED_notify = NULL;
static gen_aud_ISRNotify_t va_ABEMPTY_notify = NULL;
static gen_aud_ASRCNotify_t ASRCNotify = NULL;
static bool gen_aud_init_1stTime = TRUE;
static bool bAudioInitCalled = FALSE;/*Check on whether init has been called*/
void audio_ramp_vol_up( void );
#if (CANAL_AUDIO_HANDLING==YES)
static u_int32 volume_ramp_task_ID;
static u_int32 volume_ramp_target_tick;
sem_id_t aud_ramp_up_sem;
void volume_ramp_task( void * );
bool audio_ramp_vol_down(u_int32 min_time_down);
void cnxt_audio_drop_out(void);
extern bool SyncBlackout;  /* if TRUE, sync should not be enabled
                              because we are in the middle of a discontinuity */
u_int8 user_mute = 0;
u_int8 user_volume = 100;
extern bool bCheckForAudioDry;

static LPREG  lpDpsEventStatus = (LPREG)DPS_EVENT_STATUS_REG_EX(0);
static LPREG  lpDpsEventEnable = (LPREG)DPS_EVENT_ENABLE_REG_EX(0);  

#define WAIT_VALID_TO_UNMUTE      240
#define WAIT_UNMUTE_TO_INT_ENABLE 100

#endif /* (CANAL_AUDIO_HANDLING==YES) */

/** public functions **/

/*
 *               gen_audio_init
 *
 *  FILENAME: paceav\genaud.c
 *
 *  PARAMETERS:  
 *       microcode_type     specifies microcode to be downloaded
 *
 *  DESCRIPTION:  This function handles audio hardware and software
 *                initialization that is common to all customer configurations.
 *                This includes:
 *                     initializing common driver variables
 *                     initializing audio hardware registers
 *                     downloading audio microcode
 *                     initializing audio PLL
 *                     registering the audio interrupt handler
 *
 *  RETURNS:  TRUE if successful, FALSE otherwise
 *
 */
bool gen_audio_init(bool useAC3ucode, bool reinit)
{
    PFNISR audio_mpeg_isr_handle;
    bool ret;  /* return code */
#if PLL_TYPE != PLL_TYPE_COLORADO
    double tempmath;
    int32 prescale;
#endif

    bAudioInitCalled=TRUE;
    
    current_nom_aud_freq = 48000 * 512;
    aud_frametime = 1152;
    lpAudReadPtr  = (LPMPG_ADDRESS) DPS_AUDIO_READ_PTR_EX(gDemuxInstance);
    lpAudWritePtr = (LPMPG_ADDRESS) DPS_AUDIO_WRITE_PTR_EX(gDemuxInstance);

    reinit = (reinit || !gen_aud_init_1stTime);
    /* constants used by driver, only computed once */
    if (reinit == FALSE) {
        /* divider values must fit into 6 bits, ie 1<=DIV<40 */
        /* target frequency for PLL is 200 to 500 MHz, before division */
        /* pre-divide by 2 is back */
        pll_48kHz_divider = 6;
        pll_44kHz_divider = 6;
        pll_32kHz_divider = 8;
        pll_24kHz_divider = 2 * pll_48kHz_divider;
        pll_22kHz_divider = 2 * pll_44kHz_divider;
        pll_16kHz_divider = 2 * pll_32kHz_divider;

#if PLL_TYPE == PLL_TYPE_COLORADO

        pll_config_48kHz_int = (48000 * 512 * pll_48kHz_divider * 2) / xtal_frequency;
        pll_config_44kHz_int = (44100 * 512 * pll_44kHz_divider * 2) / xtal_frequency;
        pll_config_32kHz_int = (32000 * 512 * pll_32kHz_divider * 2) / xtal_frequency;

        /* Frac is a 25 bit field */
        pll_config_48kHz_frac = (u_int32)((double)((48000.0 * 512.0 * 2 *
                 pll_48kHz_divider) / (double)xtal_frequency) * (1<<25)) & 0x1ffffff;
        pll_config_44kHz_frac = (u_int32)((double)((44100.0 * 512.0 * 2 *
                 pll_44kHz_divider) / (double)xtal_frequency) * (1<<25)) & 0x1ffffff;
        pll_config_32kHz_frac = (u_int32)((double)((32000.0 * 512.0 * 2 *
                 pll_32kHz_divider) / (double)xtal_frequency) * (1<<25)) & 0x1ffffff;

#else
#if PLL_TYPE == PLL_TYPE_WABASH

        /*
         * For Wabash (and future chips), there is an additional programmable pre-divide
         * which defaults to PLL_PRESCALE (e.g. 3 for Wabash, 1 for Brazos).
         * Also, for the audio PLL, there is an extra divide by 2.  See codeldr\pll.s
         * for a complete description
         */ 

        prescale = PLL_PRESCALE((*((LPREG)PLL_PRESCALE_REG) &
                             PLL_PRESCALE_AUD_MASK) >> PLL_PRESCALE_AUD_SHIFT);

        pll_config_48kHz_int = (48000 * 512 * pll_48kHz_divider * 2 * 2) /
                 ((double)xtal_frequency / (double)prescale);
        pll_config_44kHz_int = (44100 * 512 * pll_44kHz_divider * 2 * 2) /
                 ((double)xtal_frequency / (double)prescale);
        pll_config_32kHz_int = (32000 * 512 * pll_32kHz_divider * 2 * 2) /
                 ((double)xtal_frequency / (double)prescale);

        /* Frac is a 25 bit field */
        tempmath = (48000.0 * 512.0 * pll_48kHz_divider * 2 * 2);
        tempmath /= ((double)xtal_frequency / (double)prescale);
        tempmath -= pll_config_48kHz_int;
        tempmath *= (1<<25);
        pll_config_48kHz_frac = (int)tempmath & 0x1ffffff;
        
        tempmath = (44100.0 * 512.0 * pll_44kHz_divider * 2 * 2);
        tempmath /= ((double)xtal_frequency / (double)prescale);
        tempmath -= pll_config_48kHz_int;
        tempmath *= (1<<25);
        pll_config_44kHz_frac = (int)tempmath & 0x1ffffff;

        tempmath = (32000.0 * 512.0 * pll_32kHz_divider * 2 * 2);
        tempmath /= ((double)xtal_frequency / (double)prescale);
        tempmath -= pll_config_48kHz_int;
        tempmath *= (1<<25);
        pll_config_32kHz_frac = (int)tempmath & 0x1ffffff;

#else

        /*
         * For Brazos there is a single non-programmable pre-divide by 2!
         */ 

        prescale = PLL_PRESCALE((*((LPREG)PLL_PRESCALE_REG) &
                             PLL_PRESCALE_AUD_MASK) >> PLL_PRESCALE_AUD_SHIFT);

        pll_config_48kHz_int = (48000 * 512 * pll_48kHz_divider * 2) /
                 ((double)xtal_frequency / (double)prescale);
        pll_config_44kHz_int = (44100 * 512 * pll_44kHz_divider * 2) /
                 ((double)xtal_frequency / (double)prescale);
        pll_config_32kHz_int = (32000 * 512 * pll_32kHz_divider * 2) /
                 ((double)xtal_frequency / (double)prescale);

        /* Frac is a 25 bit field */
        tempmath = (48000.0 * 512.0 * pll_48kHz_divider * 2);
        tempmath /= ((double)xtal_frequency / (double)prescale);
        tempmath -= pll_config_48kHz_int;
        tempmath *= (1<<25);
        pll_config_48kHz_frac = (int)tempmath & 0x1ffffff;
        
        tempmath = (44100.0 * 512.0 * pll_44kHz_divider * 2);
        tempmath /= ((double)xtal_frequency / (double)prescale);
        tempmath -= pll_config_48kHz_int;
        tempmath *= (1<<25);
        pll_config_44kHz_frac = (int)tempmath & 0x1ffffff;

        tempmath = (32000.0 * 512.0 * pll_32kHz_divider * 2);
        tempmath /= ((double)xtal_frequency / (double)prescale);
        tempmath -= pll_config_48kHz_int;
        tempmath *= (1<<25);
        pll_config_32kHz_frac = (int)tempmath & 0x1ffffff;

#endif
#endif

        pll_config_24kHz_int = pll_config_48kHz_int;
        pll_config_24kHz_frac = pll_config_48kHz_frac;
        pll_config_22kHz_int = pll_config_44kHz_int;
        pll_config_22kHz_frac = pll_config_44kHz_frac;
        pll_config_16kHz_int = pll_config_32kHz_int;
        pll_config_16kHz_frac = pll_config_32kHz_frac;

        trace_new(TRACE_LEVEL_2 | TRACE_AUD, "Downloading audio microcode.\n");

        /* download AOC (Audio Output Controller) microcode if Brazos rev B or later */
#if AUDIO_MICROCODE == UCODE_BRAZOS
        if(!ISBRAZOSREVA) {
           ret = LoadMicrocode(0, AOC_AUDIO_UCODE, (AOC_AUDIO_UCODE + AOC_AUDIO_UCODE_SIZE));
           if (ret == FALSE) {
              trace_new(TRACE_LEVEL_ALWAYS | TRACE_AUD, 
                  "audio aoc microcode download failed.\n");
               error_log(MOD_AUD & 0x1);
               return FALSE;
           }
        }
#endif

        /* download MPEG microcode */
        if (useAC3ucode == TRUE)
        {
#ifdef DRIVER_INCL_AC3
            ret = LoadMicrocode(0, AC3_AUDIO_UCODE, (AC3_AUDIO_UCODE + AC3_AUDIO_UCODE_SIZE));
#else /* Not including AC3 driver, so fail. */
            ret = FALSE;
            trace_new(TRACE_LEVEL_ALWAYS | TRACE_AUD, "Trying to download AC3 when driver not included.\n");
#endif /* AC3 */
        }
        else /* Mpeg audio microcode */
        {
            ret = LoadMicrocode(0, MPEG_AUDIO_UCODE, (MPEG_AUDIO_UCODE + MPEG_AUDIO_UCODE_SIZE));
        }
        if (ret == FALSE) {
            trace_new(TRACE_LEVEL_ALWAYS | TRACE_AUD, 
                  "audio microcode download failed.\n");
            error_log(MOD_AUD & 0x1);
            return FALSE;
        }

        /* Audio Info Register Init */
        cnxt_audio_get_audio_info(); 

        /* ISRs */
        audio_mpeg_isr_handle = (PFNISR)&audio_mpeg_isr;

        /* MPEG block interrupt handler.  Handles interrupts for
         * FSCODE, vb_full, ab_full, ab_empty, PTS */
        int_register_isr( INT_MPEGSUB, audio_mpeg_isr_handle,
                          0, 0, &previous_mpeg_isr );

        /* Initialize audio registers */

        /* initialize these before setting AudFrameStart */
        *lpAudWritePtr = (MPG_ADDRESS)*lpAudReadPtr;

        CNXT_SET_VAL(glpCtrl0, MPG_CONTROL0_MPEGACTIVE_MASK, 1);

#if ((PCM_AUDIO_TYPE==PCM_AUDIO_BRAZOS) && (USE_BRAZOS16_PCM_SW_WORKAROUND != YES))
   if(ISBRAZOSREVA && RAMWidth == 16) {
     /* If Brazos Rev A and RAMWidth is 16, do following audio workaround */
     CNXT_SET_VAL(glpCtrl1, MPG_CONTROL1_OUTPUTLEFTJUSTIFIED_MASK, 1);
     CNXT_SET_VAL((LPREG)AUD_CHANNEL_CONTROL_REG,
             AUD_CHANNEL_CONTROL_SIX_CHAN_AUDIO_MASK, 1);
        } else
#endif
     CNXT_SET_VAL(glpCtrl1, MPG_CONTROL1_OUTPUTLEFTJUSTIFIED_MASK, 0);
   
        /*    EnableDownmix */
        CNXT_SET_VAL(glpCtrl1, MPG_CONTROL1_DISABLEDIALNORM_MASK, 0);
        CNXT_SET_VAL(glpCtrl1, MPG_CONTROL1_SPDIFISAC3_MASK, 0);
        CNXT_SET_VAL(glpCtrl1, MPG_CONTROL1_MUTE_MASK, 0);
        CNXT_SET_VAL(glpCtrl1, MPG_CONTROL1_LROUTPUTCNTRL_MASK, MPG_LRCONTROL_NORMAL);
        CNXT_SET_VAL(glpCtrl1, MPG_CONTROL1_ENABLEAUDSYNC_MASK, 0);
        CNXT_SET_VAL(glpCtrl1, MPG_CONTROL1_AUDIOPARMCHANGE_MASK, 0);
#ifdef DRIVER_INCL_AC3
        if (useAC3ucode == TRUE) {
            CNXT_SET_VAL(glpCtrl1, MPG_CONTROL1_ENABLEDOWNMIX_MASK, 1);
#ifdef MPG_AUDIOMODE_AC3
            CNXT_SET_VAL(glpCtrl1, MPG_CONTROL1_AUDIOMODE_MASK, MPG_AUDIOMODE_AC3);
#endif
        }
#endif
#if (CANAL_AUDIO_HANDLING==YES)
        aud_ramp_up_sem = sem_create( 0, "RMPS" );
        volume_ramp_task_ID = task_create( volume_ramp_task, NULL, NULL,
                   RMPT_TASK_STACK_SIZE, RMPT_TASK_PRIORITY, RMPT_TASK_NAME);
#endif /* (CANAL_AUDIO_HANDLING==YES) */
   if (!cnxt_audio_init(useAC3ucode))
     return FALSE;
    }

    configure_PLL(MPG_SAMPFREQ_48000);
    CNXT_SET_VAL(glpCtrl1, MPG_CONTROL1_AUDFRAMESTART_MASK, 1); /* Need x clocks before arm_capture_enable,
                                    so careful when optimizing */

    arm_audio_gen_init(reinit);

#if SOFTWARE_AC3_SPDIF_FIX == YES && AUDIO_MICROCODE == UCODE_COLORADO
    ac3_fix_init();
#endif /* SOFTWARE_AC3_SPDIF_FIX == YES */

#if CANAL_AUDIO_HANDLING == YES
    datadry_fix_init();
#endif /* CANAL_AUDIO_HANDLING == YES */

#if (USE_BRAZOS16_PCM_SW_WORKAROUND == YES)
     if(ISBRAZOSREVA && RAMWidth == 16) {
        /* If Brazos Rev A and RAMWidth is 16, do following software audio workaround */
       Brazos16PCMInit();
    }
#endif /* (USE_BRAZOS16_PCM_SW_WORKAROUND == YES) */

    *glpIntMask |= MPG_AB_FULL | MPG_NEW_PTS_RECEIVED | MPG_FSCODE_CHANGE;

    gen_aud_init_1stTime = FALSE;
    
    /* Set volume to 100% by default */
    audio_set_volume(100);

    /* Initialize the SpDif port to Mpeg PCM data */
    cnxt_ac3_passthru(FALSE);

#ifdef HWBUF_ENCAC3_ADDR
    /* Initialize encoded AC3 buffer */
    *DPS_AC3_START_ADDR_EX(gDemuxInstance) = HWBUF_ENCAC3_ADDR;
    *DPS_AC3_END_ADDR_EX(gDemuxInstance) = HWBUF_ENCAC3_ADDR + HWBUF_ENCAUD_SIZE - 1;

    *DPS_AC3_READ_PTR_EX(gDemuxInstance) = HWBUF_ENCAC3_ADDR;
    *DPS_AC3_WRITE_PTR_EX(gDemuxInstance) = HWBUF_ENCAC3_ADDR;
#endif

    return TRUE;
}

/********************************************************************/
/*  FUNCTION:    audio_set_volume                                   */
/*                                                                  */
/*  PARAMETERS:  percentage - percentage of full volume to set      */
/*                                                                  */
/*  DESCRIPTION: Set the audio volume as a percentage.              */
/*                                                                  */
/*  RETURNS:     The volume level set prior to this call            */
/*                                                                  */
/*  CONTEXT:     May be called in any context                       */
/*                                                                  */
/********************************************************************/
int audio_set_volume(int percentage)
{
   int old_volume = current_volume;

   /* Clamp to accepted range */
   if(percentage > 100)
   {
      percentage = 100;
   }
   else if(percentage < 0)
   {
      percentage = 0;
   }

   current_volume = percentage;
   audio_set_volume_internal(percentage, AUD_ATTN_MODULE_PCM_OUTPUT);
   return(old_volume);
}

#if ((PCM_AUDIO_TYPE != PCM_AUDIO_COLORADO) && (PCM_AUDIO_TYPE != PCM_AUDIO_WABASH))

/********************************************************************/
/*  FUNCTION:    adecoder_set_volume                                */
/*                                                                  */
/*  PARAMETERS:  percentage - percentage of full volume to set      */
/*                                                                  */
/*  DESCRIPTION: Set the audio volume as a percentage.              */
/*                                                                  */
/*  RETURNS:     The volume level set prior to this call            */
/*                                                                  */
/*  CONTEXT:     May be called in any context                       */
/*                                                                  */
/********************************************************************/
int auddecoder_set_volume(int percentage)
{
   int old_auddecoder_volume = auddecoder_current_volume;

   /* Clamp to accepted range */
   if(percentage > 100)
   {
      percentage = 100;
   }
   else if(percentage < 0)
   {
      percentage = 0;
   }

   auddecoder_current_volume = percentage;
   audio_set_volume_internal(percentage, AUD_ATTN_MODULE_DECODER_INPUT);
   return(old_auddecoder_volume);
}

 #if (CPU_TYPE!=CPU_ARM940T)

/********************************************************************/
/*  FUNCTION:    rfmod_set_volume                                   */
/*                                                                  */
/*  PARAMETERS:  percentage - percentage of full volume to set      */
/*                                                                  */
/*  DESCRIPTION: Set the audio volume as a percentage.              */
/*                                                                  */
/*  RETURNS:     The volume level set prior to this call            */
/*                                                                  */
/*  CONTEXT:     May be called in any context                       */
/*                                                                  */
/********************************************************************/
int rfmod_set_volume(int percentage)
{
   int rfmod_old_volume = rfmod_current_volume;

   /* Check for rev b */
    #if CPU_TYPE == AUTOSENSE
    if(GetCPUType() == CPU_ARM940T)
    {
        return(0);
    }
    #endif /* CPU_TYPE == AUTOSENSE */

   /* Clamp to accepted range */
   if(percentage > 100)
   {
      percentage = 100;
   }
   else if(percentage < 0)
   {
      percentage = 0;
   }

   rfmod_current_volume = percentage;
   audio_set_volume_internal(percentage, AUD_ATTN_MODULE_RFMOD_OUTPUT);
   return(rfmod_old_volume);
}

 #endif /* (CPU_TYPE!=CPU_ARM940T) */

#endif /* PCM_AUDIO_TYPE != PCM_AUDIO_COLORADO && PCM_AUDIO_WABASH */

/**************************************************************/
/* audio_set_volume_internal                                  */
/*                                                            */
/* Set the audio volume level and return the previous setting */
/**************************************************************/
static void audio_set_volume_internal(int percentage, AUD_ATTN_MODULE module)
{
   int Coefficient=0, Shift=0;

   /* Clamp to accepted range */
   if(percentage > 100)
   {
      percentage = 100;
   }
   else if(percentage < 0)
   {
      percentage = 0;
   }

   Coefficient = atten_coeff_tbl[(100-percentage) % 7];
   Shift = (100-percentage) / 7;
   
   #if (PUDONG_AUDIO_DAC == AUDIO_DAC_HT82V731)    /* For AUDIO_DAC_HT82V731 */
   Shift = Shift + 2;   /* 4 bit attenuation to avoid bad sound. */
   if (Shift >= 15)
   {
      Shift = 15;
   }
   #endif
   
   isr_trace_new(TRACE_LEVEL_2 | TRACE_AUD, 
      "audio_set_volume_internal: percentage is %d for Module:%d\n", 
         percentage, module);
   isr_trace_new(TRACE_LEVEL_1 | TRACE_AUD, 
      "audio_set_volume_internal: Setting atten shift/coeff to %x/%x\n",
         Shift, Coefficient);

   switch(module)
   {
   case AUD_ATTN_MODULE_PCM_OUTPUT:
      CNXT_SET_VAL(lpleft_atten, AUD_ATTEN_COEFFICIENT_MASK,  Coefficient); 
      CNXT_SET_VAL(lpleft_atten, AUD_ATTEN_SHIFT_MASK,  Shift); 
      CNXT_SET_VAL(lpright_atten, AUD_ATTEN_COEFFICIENT_MASK, Coefficient); 
      CNXT_SET_VAL(lpright_atten, AUD_ATTEN_SHIFT_MASK, Shift); 
   break;

#if ((PCM_AUDIO_TYPE != PCM_AUDIO_COLORADO) && (PCM_AUDIO_TYPE != PCM_AUDIO_WABASH))

   case AUD_ATTN_MODULE_DECODER_INPUT:
      CNXT_SET_VAL(lpleft_atten, AUD_MPEG_CAPT_ATTEN_COEFFICIENT_MASK,  Coefficient); 
      CNXT_SET_VAL(lpleft_atten, AUD_MPEG_CAPT_ATTEN_SHIFT_MASK,  Shift); 
      CNXT_SET_VAL(lpright_atten, AUD_MPEG_CAPT_ATTEN_COEFFICIENT_MASK, Coefficient); 
      CNXT_SET_VAL(lpright_atten, AUD_MPEG_CAPT_ATTEN_SHIFT_MASK, Shift); 
   break;

 #if (CPU_TYPE!=CPU_ARM940T)

   case AUD_ATTN_MODULE_RFMOD_OUTPUT:
      CNXT_SET_VAL(lpleft_atten, AUD_RFMOD_ATTEN_COEFFICIENT_MASK,  Coefficient); 
      CNXT_SET_VAL(lpleft_atten, AUD_RFMOD_ATTEN_SHIFT_MASK,  Shift); 
      CNXT_SET_VAL(lpright_atten, AUD_RFMOD_ATTEN_COEFFICIENT_MASK, Coefficient); 
      CNXT_SET_VAL(lpright_atten, AUD_RFMOD_ATTEN_SHIFT_MASK, Shift); 
   break;

 #endif /* (CPU_TYPE!=CPU_ARM940T) */

#endif /* PCM_AUDIO_TYPE != PCM_AUDIO_COLORADO && PCM_AUDIO_WABASH */

   default:
      isr_trace_new(TRACE_LEVEL_ALWAYS | TRACE_AUD, 
         "audio_set_volume_internal: Unknown Module!!\n", module, 0);
   break;
   }

   return;
}

/****************************************************************/
/* audio_get_volume                                             */
/*                                                              */
/* Return the currently set audio volume level as a percentage  */
/****************************************************************/
int audio_get_volume(void)
{
    return(current_volume);
}

#if ((PCM_AUDIO_TYPE != PCM_AUDIO_COLORADO) && (PCM_AUDIO_TYPE != PCM_AUDIO_WABASH))

/****************************************************************/
/* auddecoder_get_volume                                        */
/*                                                              */
/* Return the currently set audio decoder volume level as a     */
/* percentage                                                   */
/****************************************************************/
int auddecoder_get_volume(void)
{
    return(auddecoder_current_volume);
}

 #if (CPU_TYPE!=CPU_ARM940T)
/****************************************************************/
/* rfmod_get_volume                                             */
/*                                                              */
/* Return the currently set RF Modulator audio volume level as a*/
/* percentage                                                   */
/****************************************************************/
int rfmod_get_volume(void)
{
    #if CPU_TYPE == AUTOSENSE
    if(GetCPUType() == CPU_ARM940T)
    {
        return(0);
    }
    #endif /* CPU_TYPE == AUTOSENSE */

    return(rfmod_current_volume);
}

 #endif /* (CPU_TYPE!=CPU_ARM940T) */

#endif /* PCM_AUDIO_TYPE != PCM_AUDIO_COLORADO && PCM_AUDIO_WABASH */

u_int32 markerloops = 0x00010000;
void aud_marker(void)
{
   u_int32 i;
   hw_set_aud_mute(TRUE);
   for (i = 0;i < markerloops ; i++)
   {
      /* do nothing */
   } /* endfor */
   hw_set_aud_mute(FALSE);
   
}
void hw_set_aud_mute(bool enableMute)
{
   bool cs;
   static u_int32 hw_mute_count=0;

   if(enableMute)
   {
      cs = critical_section_begin();
      hw_mute_count++;
      CNXT_SET_VAL(glpCtrl1, MPG_CONTROL1_MUTE_MASK, enableMute);
      critical_section_end(cs);
   }
   else
   {
      cs = critical_section_begin();
      if(hw_mute_count)
      {
         hw_mute_count--;
      }

      if(hw_mute_count==0)
      {
        CNXT_SET_VAL(glpCtrl1, MPG_CONTROL1_MUTE_MASK, enableMute);
      }
      critical_section_end(cs);
   }

   #if (CANAL_AUDIO_HANDLING == YES)
   user_mute = enableMute;
   #endif
}

void hw_set_aud_sync(bool enableSync)
{
#ifndef WABASH_AVSYNC_CLKREC_DISABLED
    CNXT_SET_VAL(glpCtrl1, MPG_CONTROL1_ENABLEAUDSYNC_MASK, enableSync);
#endif
}

/********************************************************************/
/*  FUNCTION:    hw_aud_stop                                        */
/*                                                                  */
/*  PARAMETERS:  bClearBuffers Clear the encoded data buffers if    */
/*                             TRUE. The demux must have been       */
/*                             halted from sending audio data if    */
/*                             this is specified.                   */
/*                                                                  */
/*  DESCRIPTION: Stops the audio decoder and waits for it to show   */
/*               that it has halted with a timeout. Optionally      */
/*               clears the audio encoder data read/write pointers  */
/*                                                                  */
/*  RETURNS:     Nothing.                                           */
/*                                                                  */
/*  CONTEXT:     May be called in any context, but may wait         */
/*               forever if called with interrupts disabled and if  */
/*               the audio ucode hangs and never shows "halted"     */
/*                                                                  */
/********************************************************************/
void hw_aud_stop(bool bClearBuffers)
{
    u_int32 uiStartTime;
    bool    bStopped = FALSE;

    /* Signal the halt decode bit (works for MPEG and AC3 audio) */
    /* Spec indicates HaltDecode only works for AC3.             */
    /* Undocumented that it now works for both MPEG and AC3      */
    CNXT_SET_VAL(glpCtrl1, MPG_CONTROL1_HALTDECODE_MASK, 1);
    
    /* Wait for the stopped bit to be asserted (with a timeout) */
    uiStartTime = get_system_time();
    while (!bStopped && ((get_system_time() - uiStartTime) <= AUDIO_STOP_MAX_WAIT))
    {
      /* Not documented in version 101080p6 of the chip spec     */
      /* bit 13 of MPG_DECODE_STATUS_REG will be set when halted */
      if ((*(LPREG)MPG_DECODE_STATUS_REG) & MPG_TEST_DSP_DONE)
      {
         bStopped = TRUE;
      } 
    } /* endwhile not stopped and no timeout */
    
    /* Mow go and set the audio read/write pointers to the same */
    /* (The start of the audio PES buffer */
    if (bClearBuffers)
    {
      *(LPREG)lpAudWritePtr = *(LPREG)lpAudReadPtr;
    } /* endif clear buffers */

    return;
    
} /* hw_aud_stop */

/********************************************************************/
/*  FUNCTION:    hw_aud_start                                       */
/*                                                                  */
/*  PARAMETERS:  None                                               */
/*                                                                  */
/*  DESCRIPTION: Takes the audio decoder out of halt mode.          */
/*               Assumes all else is well - microcode loaded etc.   */
/*                                                                  */
/*  RETURNS:     Nothing                                            */
/*                                                                  */
/*  CONTEXT:     May be called in any context.                      */
/*                                                                  */
/********************************************************************/
void hw_aud_start(void)
{
    CNXT_SET_VAL(glpCtrl1, MPG_CONTROL1_HALTDECODE_MASK, 0);
    return;

} /* hw_aud_start */


void arm_audio_gen_init(bool reinit)
{
    /* function should
     *                 initialize arm audio registers
     *                 anything else?
     */

    LPREG lpBoardOpt = (LPREG) PLL_CONFIG0_REG;

    /* initialize arm audio registers */

    LPREG lpoutp_ctrl = (LPREG) AUD_OUTP_CONTROL_REG;
    LPREG lpclock_ctrl = (LPREG) AUD_CLOCK_CONTROL_REG;
    LPREG lpfrm_prot = (LPREG) AUD_FRAME_PROTOCOL_REG;
    LPREG lpaud_null = (LPREG) AUD_NULL_REG;

    HW_DWORD outp_ctrl = *lpoutp_ctrl;
    HW_DWORD clock_ctrl = *lpclock_ctrl;
    HW_DWORD frm_prot = *lpfrm_prot;
    HW_DWORD aud_null = *lpaud_null;
    trace_new(TRACE_LEVEL_2 | TRACE_AUD, "Inside ARM Audio init.\n");

    if (!reinit) {                                             
      
        #if PLL_PIN_GPIO_MUX1_REG_DEFAULT == NOT_DETERMINED
        /* Set GPIO Muxing to control Deemphasis if necessary */
        if (AUD_DEEMP_ON_GPIO) {
           *((LPREG) PLL_PIN_GPIO_MUX1_REG) |= PLL_PIN_GPIO_MUX1_PIO_41;
        } else {
           *((LPREG) PLL_PIN_GPIO_MUX1_REG) &= ~PLL_PIN_GPIO_MUX1_PIO_41;
        } 
        #endif
        
        /* Output control register */
        CNXT_SET_VAL(&outp_ctrl, AUD_OUTP_CONTROL_ENABLE_BITCK_MASK,    TRUE);
        CNXT_SET_VAL(&outp_ctrl, AUD_OUTP_CONTROL_BITCK_DEFAULT_MASK,   0);
        CNXT_SET_VAL(&outp_ctrl, AUD_OUTP_CONTROL_ENABLE_LRCK_MASK,      TRUE);
        CNXT_SET_VAL(&outp_ctrl, AUD_OUTP_CONTROL_LRCK_DEFAULT_MASK,     0);
        CNXT_SET_VAL(&outp_ctrl, AUD_OUTP_CONTROL_ENABLE_DATAOUT_MASK,  TRUE);
        CNXT_SET_VAL(&outp_ctrl, AUD_OUTP_CONTROL_DATAOUT_DEFAULT_MASK, 0);
        CNXT_SET_VAL(&outp_ctrl, AUD_OUTP_CONTROL_ENABLE_DEEMP_MASK,     AUD_DEEMP_ENABLE_DEFAULT);
        CNXT_SET_VAL(&outp_ctrl, AUD_OUTP_CONTROL_DEEMP_DEFAULT_MASK,    AUD_DEEMP_VALUE_DEFAULT);
        CNXT_SET_VAL(&outp_ctrl, AUD_OUTP_CONTROL_DEEMP_STANDARD_MASK,   AUD_DEEMP_MODE_DEFAULT);
        CNXT_SET_VAL(&outp_ctrl, AUD_OUTP_CONTROL_DEEMP_NEG_ASSERT_MASK, AUD_DEEMP_ACTIVE_LOW_DEFAULT);

        /* Clock control register. */
        CNXT_SET_VAL(&clock_ctrl, AUD_CLOCK_CONTROL_SOURCE_SELECT_MASK,  1);  /* use Raven */
        CNXT_SET_VAL(&clock_ctrl, AUD_CLOCK_CONTROL_BIT_CK32X_MASK,      0);     /* 64 x sample frequency */
        CNXT_SET_VAL(&clock_ctrl, AUD_CLOCK_CONTROL_ENABLE_OSCK1_MASK,  TRUE);
        CNXT_SET_VAL(&clock_ctrl, AUD_CLOCK_CONTROL_OSCK1_DEFAULT_MASK, 0);
        CNXT_SET_VAL(&clock_ctrl, AUD_CLOCK_CONTROL_ENABLE_OSCK2_MASK,  FALSE);
        CNXT_SET_VAL(&clock_ctrl, AUD_CLOCK_CONTROL_OSCK2_DEFAULT_MASK, 0);
        CNXT_SET_VAL(&clock_ctrl, AUD_CLOCK_CONTROL_SELECT_OSCK1_MASK,  0);  /* audio clock */
        CNXT_SET_VAL(&clock_ctrl, AUD_CLOCK_CONTROL_OSCK1_DIVISOR_MASK, 1); /* divide clock by 2^1, 256 * s.f. */
        CNXT_SET_VAL(&clock_ctrl, AUD_CLOCK_CONTROL_SELECT_OSCK2_MASK,  1);  /* MPEG clock, for CR testing */
        CNXT_SET_VAL(&clock_ctrl, AUD_CLOCK_CONTROL_OSCK2_DIVISOR_MASK, 0);

        /* Frame Protocol Register. */
        CNXT_SET_VAL(&frm_prot, AUD_FRAME_PROTOCOL_AUD_DATA_LEN_MASK,  4);  /* twenty bits */
        CNXT_SET_VAL(&frm_prot, AUD_FRAME_PROTOCOL_ENABLE_STEREO_MASK,  TRUE);
        #if DIGITAL_DISPLAY == NOT_PRESENT
           CNXT_SET(&frm_prot, AUD_FRAME_PROTOCOL_OP_FORMAT_SELECT_MASK,  AUD_FRAME_PROTOCOL_OP_FORMAT_LEFTJUST);
        #else
           /* This is crosby, we use a different audio DAC that expects I2S */
           CNXT_SET(&frm_prot, AUD_FRAME_PROTOCOL_OP_FORMAT_SELECT_MASK,  AUD_FRAME_PROTOCOL_OP_FORMAT_I2S);
        #endif
        
        #if (PUDONG_AUDIO_DAC == AUDIO_DAC_CX4334)          /* For AUDIO_DAC_CX4334 */
           CNXT_SET(PLL_PIN_GPIO_MUX1_REG, PLL_PIN_GPIO_MUX1_PIO_42, 1<<10L);
           CNXT_SET(&frm_prot, AUD_FRAME_PROTOCOL_OP_FORMAT_SELECT_MASK,  AUD_FRAME_PROTOCOL_OP_FORMAT_I2S);
        #elif (PUDONG_AUDIO_DAC == AUDIO_DAC_HT82V731)   /* For AUDIO_DAC_HT82V731 */
           CNXT_SET(PLL_PIN_GPIO_MUX1_REG, PLL_PIN_GPIO_MUX1_PIO_42, 1<<10L);
           CNXT_SET(&frm_prot, AUD_FRAME_PROTOCOL_OP_FORMAT_SELECT_MASK,  AUD_FRAME_PROTOCOL_OP_FORMAT_RIGHTJUST);
        #endif
        
        CNXT_SET(&frm_prot, AUD_FRAME_PROTOCOL_IP_FORMAT_SELECT_MASK,  AUD_FRAME_PROTOCOL_IP_FORMAT_I2S);      /* from Raven or A/D */
        CNXT_SET_VAL(&frm_prot, AUD_FRAME_PROTOCOL_UNUSED_BIT_STATE_MASK,  0);

#if CUSTOMER == VENDOR_A
        CNXT_SET_VAL(&frm_prot, AUD_FRAME_PROTOCOL_AUD_DATA_LEN_MASK,  2);  /* 18 bits */
        CNXT_SET_VAL(&frm_prot, AUD_FRAME_PROTOCOL_OP_FORMAT_SELECT_MASK,  AUD_FORMAT_RIGHTJUST);
#endif
        /* determine the DAC/ADC/codec configuration */
        if ((*lpBoardOpt & RSO_6_CHAN_AUDIO) != 0) {
            /* ERROR -- software not configured for 6 channel audio */
        } else {  /* 2 channel audio */
#if 0 /* old code for THUNDER, comments still good */
            if ((*lpBoardOpt & RSO_AUDIO_ID) ==
               (RSO_AUDIO_ID_THUNDER01 << RSO_AUDIO_ID_SHIFT)) {
               /* most common case, using thunder 0001, or thunder for Colorado(Twitty) */
               /* internal SCLK mode, deemphasis disabled, output left justified */

               /* deemphasis is disabled for D/A if the DEM/SCLK pin is
                      high for 5 consecutive falling edges of LRCK. */
                /*    outp_ctrl.DeempDefault = 1; */

                /* XXXAMY BITCK is not driving D/A.  Should it be disabled? */
                /*    outp_ctrl.EnableBitCk = TRUE; */
                /* output is left justified */
                /*     frm_prot.OPFormatSelect = AUD_FORMAT_LEFTJUST; */

            } else if ((*lpBoardOpt & RSO_AUDIO_ID) ==
               (RSO_AUDIO_ID_THUNDER00 << RSO_AUDIO_ID_SHIFT)) {
                /* should only happen internally, configure for thunder 0000 */
                /* external SCLK mode (deemphasis disabled), output IIS */

                /* deemphasis automatically disabled */
                /* external SCLK mode is entered if 16 low to high transitions
                   are detected on the DEM/SCLK pin during any phase of the
                   LCLK period.  This should happen automatically if BITCLK,
                   connected to SCLK, is enabled */
                /*    outp_ctrl.EnableBitCk = TRUE; */

                /* output format is IIS */
                CNXT_SET_VAL(&frm_prot, AUD_FRAME_PROTOCOL_OP_FORMAT_SELECT_MASK,  AUD_FORMAT_I2S);

            } else {
            /* ERROR, unrecognized configuration */
            }
#endif /* COMMENT */
        }

    }

#if 0
    /* check config bits */
      if (source internal) {
          /* audio source is Raven */
          CNXT_SET_VAL(&clock_ctrl, AUD_CLOCK_CONTROL_SOURCE_SELECT_MASK,  1);  /* use Raven */
          CNXT_SET_VAL(&frm_prot, AUD_FRAME_PROTOCOL_AUD_DATA_LEN_MASK,  4);  /* twenty bits */

      } else {
        /* audio source is external */
          CNXT_SET_VAL(&clock_ctrl, AUD_CLOCK_CONTROL_SOURCE_SELECT_MASK,  0);  /* generate clock */
          CNXT_SET_VAL(&frm_prot, AUD_FRAME_PROTOCOL_AUD_DATA_LEN_MASK,  2);    /* eighteen bits */
      }
#endif /*COMMENT*/


    *lpoutp_ctrl = outp_ctrl;
    *lpclock_ctrl = clock_ctrl;
    *lpfrm_prot = frm_prot;



    /* Audio NULL Register.
     * This should be in the format of the Audio Data Word.  */
    CNXT_SET_VAL(&aud_null, AUD_NULL_NULL_WORD_MASK, 0);
    *lpaud_null = aud_null;

    /* Attenuation Register
     * Zero coefficient is special case that implies no attenuation */
    audio_set_volume(0);

    PCMInit();

}

int configure_PLL(int rate)
{
    bool kstate;
    int16 new_divisor;
    static int iCurrentRate = MPG_SAMPFREQ_48000;
    int  iTemp;

    iTemp = iCurrentRate;
    iCurrentRate = rate;

    switch (rate) {
    case MPG_SAMPFREQ_48000:
        isr_trace_new(TRACE_LEVEL_4 | TRACE_AUD, "Inside configure_PLL(), rate is 48k\n", 0, 0);
        current_nom_aud_freq = 48000 * 512;
        current_nom_pll_config_int = pll_config_48kHz_int;
        current_nom_pll_config_frac = pll_config_48kHz_frac;
        new_divisor = pll_48kHz_divider;
        break;
    case MPG_SAMPFREQ_44100:
        isr_trace_new(TRACE_LEVEL_4 | TRACE_AUD, "Inside configure_PLL(), rate is 44k\n", 0, 0);
        current_nom_aud_freq = 44100 * 512;
        current_nom_pll_config_int = pll_config_44kHz_int;
        current_nom_pll_config_frac = pll_config_44kHz_frac;
        new_divisor = pll_44kHz_divider;
        break;
    case MPG_SAMPFREQ_32000:
    isr_trace_new(TRACE_LEVEL_4 | TRACE_AUD, "Inside configure_PLL(), rate is 32k\n", 0, 0);
        current_nom_aud_freq = 32000 * 512;
        current_nom_pll_config_int = pll_config_32kHz_int;
        current_nom_pll_config_frac = pll_config_32kHz_frac;
        new_divisor = pll_32kHz_divider;
        break;
    case MPG_SAMPFREQ_24000:
    isr_trace_new(TRACE_LEVEL_4 | TRACE_AUD, "Inside configure_PLL(), rate is 24k\n", 0, 0);
        current_nom_aud_freq = 24000 * 512;
        current_nom_pll_config_int = pll_config_24kHz_int;
        current_nom_pll_config_frac = pll_config_24kHz_frac;
        new_divisor = pll_24kHz_divider;
        break;
    case MPG_SAMPFREQ_22050:
    isr_trace_new(TRACE_LEVEL_4 | TRACE_AUD, "Inside configure_PLL(), rate is 22k\n", 0, 0);
        current_nom_aud_freq = 22050 * 512;
        current_nom_pll_config_int = pll_config_22kHz_int;
        current_nom_pll_config_frac = pll_config_22kHz_frac;
        new_divisor = pll_22kHz_divider;
        break;
    case MPG_SAMPFREQ_16000:
    isr_trace_new(TRACE_LEVEL_4 | TRACE_AUD, "Inside configure_PLL(), rate is 16k\n", 0, 0);
        current_nom_aud_freq = 16000 * 512;
        current_nom_pll_config_int = pll_config_16kHz_int;
        current_nom_pll_config_frac = pll_config_16kHz_frac;
        new_divisor = pll_16kHz_divider;
        break;
    default:
        /* error -- assume 48kHz */
        isr_trace_new(TRACE_LEVEL_4 | TRACE_AUD,
               "ERROR: INSIDE CONFIGURE_PLL() -- UNKNOWN RATE -- ASSUMING 48K\n", 0, 0);
        current_nom_aud_freq = 48000 * 512;
        current_nom_pll_config_int = pll_config_48kHz_int;
        current_nom_pll_config_frac = pll_config_48kHz_frac;
        new_divisor = pll_48kHz_divider;
        iCurrentRate = MPG_SAMPFREQ_48000;
        break;
    }

    kstate = critical_section_begin();
    if ( *(LPREG)PLL_LOCK_CMD_REG != PLL_LOCK_CMD_VALUE) {
        cs_trace(TRACE_AUD|TRACE_LEVEL_3, "PLL already unlocked.\n", 0, 0);
        PLL_LOCK()
    }
    PLL_UNLOCK()
    *((LPREG)PLL_LOCK_STAT_REG) &= ~PLL_LOCK_ALLDIVIDERS;
    if (new_divisor > apll_divisor) {
        /* write divider first if frequency is decreasing */
        *lpPLL_Div1 = (*lpPLL_Div1 & ~PLL_DIV1_ASPL_CLK_MASK) |
                      ((new_divisor << PLL_DIV1_ASPL_CLK_SHIFT) & PLL_DIV1_ASPL_CLK_MASK);
    }

    *((LPREG)PLL_LOCK_STAT_REG) &= ~PLL_LOCK_AUDIO;
    *((LPREG)PLL_AUD_CONFIG_REG) =
        (((current_nom_pll_config_int << PLL_AUD_CONFIG_INT_SHIFT) & PLL_AUD_CONFIG_INT_MASK) |
         ((current_nom_pll_config_frac << PLL_AUD_CONFIG_FRAC_SHIFT) & PLL_AUD_CONFIG_FRAC_MASK));
    *((LPREG)PLL_LOCK_STAT_REG) |= PLL_LOCK_AUDIO;

    if (new_divisor < apll_divisor) {
        /* write divider last if frequency is increasing */
        *lpPLL_Div1 = (*lpPLL_Div1 & ~PLL_DIV1_ASPL_CLK_MASK) |
                      ((new_divisor << PLL_DIV1_ASPL_CLK_SHIFT) & PLL_DIV1_ASPL_CLK_MASK);
    }
    *((LPREG)PLL_LOCK_STAT_REG) |= PLL_LOCK_ALLDIVIDERS;
    PLL_LOCK()
    critical_section_end(kstate);

    /* let PCM driver know that audio sample rate has changed */
    if (ASRCNotify != NULL) (ASRCNotify)(current_nom_aud_freq/512);

    apll_divisor = new_divisor;

    return(iTemp);
}

int audio_mpeg_isr( u_int32 interrupt_ID, bool isFIQ, PFNISR *previous )
{
    /* this ISR handles the following interrupt fields in the MPEG block:
     *      (audio) PTS_RECEIVED         (bit 27)
     *      FSCODE changed               (bit 26)
     *      (audio) PTS_MATURED          (bit 17)
     *      audio buffer empty           (bit 12)
     *      audio buffer full            (bit 11)
     *      audio buffer lowwatermark    (bit  8)
     */
#ifdef OPENTV_12
    extern bool audio_ramp_vol_down(u_int32 min_time_down);
    extern sem_id_t aud_ramp_up_sem;
#endif  /* OPENTV_12 */

    u_int32 IRQReason = *glpIntStatus & (MPG_AB_FULL | MPG_AB_EMPTY
                         | MPG_AB_LOWWATERMARK
                         | MPG_FSCODE_CHANGE | MPG_PTS_MATURED
                         | MPG_NEW_PTS_RECEIVED);

    /***********  AUDIO_PTS_MATURED  ***********/
    if ((IRQReason & MPG_PTS_MATURED) != 0) {
        *glpIntStatus = MPG_PTS_MATURED; 
        isr_trace_new(TRACE_LEVEL_1 | TRACE_AUD, "Latest audio PTS has matured.\n", 0, 0);
        LastAudPTSMatured = TRUE;

        if (vb_PTSMATURED_notify != NULL) (vb_PTSMATURED_notify)();
    }

    /***********  AUDIO_PTS_RECEIVED  ***********/
    if ((IRQReason & MPG_NEW_PTS_RECEIVED) != 0) {
        *glpIntStatus = MPG_NEW_PTS_RECEIVED; 
        /* Enable buffer full and empty interrupts iff audio decoder enabled */
        if (CNXT_GET(glpCtrl1, MPG_CONTROL1_ENCAUDIOENABLE_MASK)) {
           *glpIntMask |= MPG_AB_FULL | MPG_AB_EMPTY;  /* Enable buffer full/empty interrupts */
        }
        cnxt_audio_pts_received_int(0);
        if (vb_NEWPTS_notify != NULL) (vb_NEWPTS_notify)();
    }

    /***********  MPG_AB_FULL  ***********/
    if ((IRQReason & MPG_AB_FULL ) != 0) {
        *glpIntMask   &= ~MPG_AB_FULL;         /* Disable audio buffer full interrupt  */
        *glpIntStatus = MPG_AB_FULL; 
        ++ab_full_cnt;
        /* if colorado and rev C (and thus not rev F), then
        *  if the SpdifIsAC3 bit is set, then
        *  ignore these interrupts, since SpdifisAC3 mode
        *  will cause MPG_AB_FULL interrupts. 
        *  Rev F has been modified to prevent SpdifisAC3 mode 
        *  from causing MPG_AB_FULL interrupts.
        *  Wabash and later chips have added a new and seperate 
        *  Audio SPDIF buffer full interrupt bit within ISR for
        *  interrupts caused by SpdifisAC3 mode */
#ifdef ISCOLORADOREVC
        if (!(ISCOLORADOREVC && CNXT_GET(glpCtrl1, MPG_CONTROL1_SPDIFISAC3_MASK))) {
#endif
#ifdef DEBUG
            isr_trace_new(TRACE_LEVEL_ALWAYS | TRACE_AUD, 
               "Audio Buffer FULL!!!!!!!!!!!!!!!!!!!!!!!!!!! ab_full_cnt=%d\n", 
               ab_full_cnt, 0);
#endif /* defined( DEBUG) */
#if 1   
/* Change the way we handle this buffer overflow. 
        If the audio sync was on, then we want to toggle
        it, so that the HW will reload clock recovery
        information.
        Then reset the buffer pointers into the encoded
        audio buffer, so that we don't get another AB Full
        interrupt immediately, to give the system time to
        correct what ever situation lead to this condition.*/
        
            if (CNXT_GET(glpCtrl1, MPG_CONTROL1_ENABLEAUDSYNC_MASK)) {
                CNXT_SET_VAL(glpCtrl1, MPG_CONTROL1_ENABLEAUDSYNC_MASK, 0);
                *lpAudWritePtr = (MPG_ADDRESS)HWBUF_ENCAUD_ADDR;
                *lpAudReadPtr = (MPG_ADDRESS)HWBUF_ENCAUD_ADDR;
                *DPS_INFO_CHANGE_REG_EX(gDemuxInstance) |= (1 << AUDIO_CHANNEL);
#ifndef WABASH_AVSYNC_CLKREC_DISABLED
                CNXT_SET_VAL(glpCtrl1, MPG_CONTROL1_ENABLEAUDSYNC_MASK, 1);
#endif
            } else {
#if 0            
                *lpAudWritePtr = (MPG_ADDRESS)HWBUF_ENCAUD_ADDR;
                *lpAudReadPtr = (MPG_ADDRESS)HWBUF_ENCAUD_ADDR;
                *DPS_INFO_CHANGE_REG_EX(gDemuxInstance) |= (1 << AUDIO_CHANNEL);
#else
		hw_aud_stop(TRUE);
		hw_aud_start();
#endif
            }
#else   /* Do nothing for buffer overflow for now. */

            /* disable pid, disable decoding, stop arm block, reset buffer pointers */
            /* This also sets gbRestoreAudioAfterReset if audio PID was enabled     */
            /* on entry  */
            aud_before_reset();

            /* if audio sync is enabled, reload the STC to prevent the overflow 
               from happening again and again */
            if (CNXT_GET_VAL(glpCtrl1, MPG_CONTROL1_ENABLEAUDSYNC_MASK) == 1) {
                CNXT_SET_VAL(glpCtrl0, MPG_CONTROL0_LOADBACKUPSTC_MASK, 1);
            }

            /* enable decoding bit */
            CNXT_SET_VAL(glpCtrl1, MPG_CONTROL1_ENCAUDIOENABLE_MASK, 1);

            /* Only reenable the audio pid if it was enabled on entry to aud_before_reset */
            if (gbRestoreAudioAfterReset)
            {
               /* enable audio PID */
#ifdef DRIVER_INCL_GENDMXC
               gen_dmx_control_av_channel((u_int32) 0, 
                      (u_int32) AUDIO_CHANNEL, (gencontrol_channel_t) GEN_DEMUX_ENABLE);
#else
          if (cnxt_dmx_channel_control(gDemuxInstance, (u_int32) AUDIO_CHANNEL, 
                    (gencontrol_channel_t) GEN_DEMUX_ENABLE) != DMX_STATUS_OK) {
        trace_new(DPS_ERROR_MSG,"DEMUX:control_channel failed\n");
          }
#endif
               gbRestoreAudioAfterReset = FALSE;    
            } 

            /* restart ARM audio */
            StartPCMCaptureAndPlayback();
#endif  /* Do nothing for buffer overflow for now. */
#ifdef ISCOLORADOREVC
        } /* endif (!(ISCOLORADOREVC && CNXT_GET(glpCtrl1, MPG_CONTROL1_SPDIFISAC3_MASK))) */
#endif

        /* let application know about error */
        /* callback is safe from interrupt context */
        if (vb_ABFULL_notify != NULL) {
           (vb_ABFULL_notify)();
        }
    }

    /***********  MPG_AB_EMPTY  ***********/
    if ((IRQReason & MPG_AB_EMPTY) != 0 ) {
      *glpIntMask   &= ~MPG_AB_EMPTY;         /* Disable audio buffer empty interrupt  */
      *glpIntStatus = MPG_AB_EMPTY; 
      if(cnxt_audio_is_valid()) {
        ++ab_empty_cnt;
      }
#ifdef DEBUG
           isr_trace_new(TRACE_LEVEL_2 | TRACE_AUD, 
               "Audio Buffer EMPTY!!!!!!!!!!!!!!!!!!!!!!!!!!! ab_empty_cnt=%d\n", 
               ab_empty_cnt, 0);
#endif /* defined( DEBUG) */
      cnxt_audio_empty_int(0);
      if (va_ABEMPTY_notify != NULL) {
         (va_ABEMPTY_notify)();
      }
    }

    /***********  MPG_FSCODE_CHANGE  ***********/
    if ((IRQReason & MPG_FSCODE_CHANGE) != 0) {
        *glpIntStatus = MPG_FSCODE_CHANGE; 
#ifdef OPENTV_12
        isr_trace_new(TRACE_LEVEL_2 | TRACE_AUD, "Ramping volume down\n",0,0);
        if (audio_ramp_vol_down(300)) {
          sem_put(aud_ramp_up_sem);
        }
#endif  /* OPENTV_12 */
        isr_trace_new(TRACE_LEVEL_4 | TRACE_AUD, 
                                          "FSCODE_CHANGE.  \n", 0, 0);
        /* sample rate changed.  program the audio PLL to match. */
        configure_PLL(CNXT_GET_VAL(glpCtrl1, MPG_CONTROL1_SAMPFREQ_MASK));
    }

#if 0 /* Interrupt removed in Colorado, may be added to pawser if needed */
    /***********  MPG_AUD_DATA_ACCEPTED  ***********/
    if ((IRQReason & MPG_AUD_DATA_ACCEPTED) != 0) {
        *glpIntStatus = MPG_AUD_DATA_ACCEPTED; 
        isr_trace_new(TRACE_LEVEL_2 | TRACE_AUD, 
                             "New audio data in encoded buffer\n", 0,0);
        *glpIntMask |= MPG_AB_EMPTY;
        *glpIntMask &= ~MPG_AUD_DATA_ACCEPTED;
    }
#endif /* COMMENT */

    /***********  MPG_AB_LOWWATERMARK  ***********/
    if (IRQReason & MPG_AB_LOWWATERMARK) {
      *glpIntStatus = MPG_AB_LOWWATERMARK; 
      cnxt_audio_lowwater_int(0);
    }

    if((*glpIntStatus) == 0)
    {
        return RC_ISR_HANDLED;
    }
    else {
        *previous = previous_mpeg_isr;
        return RC_ISR_NOTHANDLED;
    }

}

void aud_before_reset(void)
{
    bool kstate;

    if (bAudioInitCalled==FALSE)
    {
      return;
    }

#ifdef DRIVER_INCL_GENDMXC
    gbRestoreAudioAfterReset = gen_dmx_channel_enabled((u_int32)AUDIO_CHANNEL);
#else
    gbRestoreAudioAfterReset = gen_dmx_channel_enabled(gDemuxInstance, (u_int32)AUDIO_CHANNEL);
#endif
    /* disable audio PID */
    if (gbRestoreAudioAfterReset)
    {
#ifdef DRIVER_INCL_GENDMXC
       gen_dmx_control_av_channel((u_int32) 0, 
            (u_int32) AUDIO_CHANNEL, (gencontrol_channel_t) GEN_DEMUX_DISABLE);
#else
       if (cnxt_dmx_channel_control(gDemuxInstance, (u_int32) AUDIO_CHANNEL, 
            (gencontrol_channel_t) GEN_DEMUX_DISABLE) != DMX_STATUS_OK) {
        trace_new(DPS_ERROR_MSG,"DEMUX:control_channel failed\n");
       }
#endif
    } 

    /* stop ARM audio */
    StopPCMCaptureAndPlayback();

    /* disable audio decoding */
    CNXT_SET_VAL(glpCtrl1, MPG_CONTROL1_ENCAUDIOENABLE_MASK, 0);

    /* reset buffer pointers */
    kstate = critical_section_begin();
    *lpAudWritePtr = (MPG_ADDRESS)HWBUF_ENCAUD_ADDR;
    *lpAudReadPtr = (MPG_ADDRESS)HWBUF_ENCAUD_ADDR;
    *DPS_INFO_CHANGE_REG_EX(gDemuxInstance) |= (1 << AUDIO_CHANNEL);    
    critical_section_end(kstate);

    /* mute? disable audio sync? */
}

void aud_after_reset(void)
{
    if (bAudioInitCalled==FALSE)
    {
      return;
    }

    /* set PLL for 48kHz */
    configure_PLL(MPG_SAMPFREQ_48000);

    /* enable decoding bit */
    CNXT_SET_VAL(glpCtrl1, MPG_CONTROL1_ENCAUDIOENABLE_MASK, 1);

    if (gbRestoreAudioAfterReset)
    {
       /* enable PID */
#ifdef DRIVER_INCL_GENDMXC
       gen_dmx_control_av_channel((u_int32) 0, 
            (u_int32) AUDIO_CHANNEL, (gencontrol_channel_t) GEN_DEMUX_ENABLE);
#else
       if (cnxt_dmx_channel_control(gDemuxInstance, (u_int32) AUDIO_CHANNEL, 
            (gencontrol_channel_t) GEN_DEMUX_ENABLE) != DMX_STATUS_OK) {
        trace_new(DPS_ERROR_MSG,"DEMUX:control_channel failed\n");
       }
#endif

       gbRestoreAudioAfterReset = FALSE;
    }


    /* start ARM audio */
    StartPCMCaptureAndPlayback();

    /* unmute?  re-enable audio sync? */
}

/*
 *            gen_aud_ISRNotifyRegister
 *
 *  FILENAME: aud_comn\aud_comn.c
 *
 *  PARAMETERS:  
 *            interrupt_bit    the notify function will be called 
 *                                 when this interrupt occurs
 *            vendor_id        which flavor of the audio driver
 *                                 wants to be notified
 *            handler          pointer to the notify function to be called
 *
 *  DESCRIPTION:  This function registers a notify/interrupt handling
 *                function with the common audio driver.
 *
 *  RETURNS:  Old Handler.
 *
 */
gen_aud_ISRNotify_t gen_aud_ISRNotifyRegister(u_int32 interrupt_bit,
                               gen_aud_ISRNotify_t handler)
{
    gen_aud_ISRNotify_t old_handler = NULL;
    if (interrupt_bit == MPG_AB_FULL) {
      old_handler = vb_ABFULL_notify;
      vb_ABFULL_notify = handler;
    } else if (interrupt_bit == MPG_AB_EMPTY) {
      old_handler = va_ABEMPTY_notify;
      va_ABEMPTY_notify = handler;
    } else if (interrupt_bit == MPG_NEW_PTS_RECEIVED) {
      old_handler = vb_NEWPTS_notify;
      vb_NEWPTS_notify = handler;
    }else if (interrupt_bit == MPG_PTS_MATURED) {
      old_handler = vb_PTSMATURED_notify;
      vb_PTSMATURED_notify = handler;
    } else {
      old_handler = NULL;
    }
    return(old_handler);
}
        
/*
 *             gen_aud_ASRCNotifyRegister
 *
 *  FILENAME: aud_comn\aud_comn.c
 *
 *  PARAMETERS:   ASRChangeNotify       pointer to the notify function
 *                                      to be called when the audio 
 *                                      sample rate changes
 *
 *  DESCRIPTION:  Allows PCM driver to register a function that
 *                will be called when the audio sample clock is 
 *                changed.  Only one notify function can be registered
 *                at a time.  The notify function will be called
 *                at interrupt time.
 *
 *  RETURNS: None.
 */
void gen_aud_ASRCNotifyRegister(gen_aud_ASRCNotify_t ASRChangeNotify)
{
    ASRCNotify = ASRChangeNotify;
}

/*
 *             AudMPGVolumeFuncRegister
 *
 *  FILENAME: aud_comn\aud_comn.c
 *
 *  PARAMETERS:  volume_function    pointer to the function
 *                                  that will execute the change
 *                                  in the MPEG audio volume.
 *
 *  DESCRIPTION:  Allows audio sample rate conversion and Mixing
 *                (PCM) driver to register a function that will
 *                be called when the MPEG volume to the mixer
 *                should be changed.  The registered function will
 *                be called at task time.
 *
 *  RETURNS: None.
 */
void AudMPGVolumeFuncRegister(mixer_volume_func_t volume_function)
{
    MPEGVolumeFunc = volume_function;
}

/*
 *             AudPCMVolumeFuncRegister
 *
 *  FILENAME: aud_comn\aud_comn.c
 *
 *  PARAMETERS:  volume_function    pointer to the function
 *                                  that will execute the change
 *                                  in the G.729 audio volume.
 *
 *  DESCRIPTION:  Allows audio sample rate conversion and Mixing
 *                (PCM) driver to register a function that will
 *                be called when the G.729 volume to the mixer
 *                should be changed.  The registered function will
 *                be called at task time.
 *
 *  RETURNS: None.
 */
void AudPCMVolumeFuncRegister(mixer_volume_func_t volume_function)
{
    G729VolumeFunc = volume_function;
}


/*
 *            gen_audio_play
 *
 *  FILENAME:    aud_comn\aud_comn.c
 *
 *  PARAMETERS:   SyncWithPCR    TRUE if audio stream should be 
 *                               played in sync with a PCR PID.
 *
 *  DESCRIPTION:  Enables the audio PID and sets audio sync to
 *                value requested.
 *
 *  RETURNS: none.
 */
void gen_audio_play(bool SyncWithPCR)
{
    SyncOn = SyncWithPCR;
    /* enable PID */

    /* Ensure that the DSP is out of halt mode */
    hw_aud_start();
    
    /* enable the audio PID */
#ifdef DRIVER_INCL_GENDMXC
    gen_dmx_control_av_channel((u_int32) 0, 
            (u_int32) AUDIO_CHANNEL, (gencontrol_channel_t) GEN_DEMUX_ENABLE);
#else
    if (cnxt_dmx_channel_control(gDemuxInstance, (u_int32) AUDIO_CHANNEL, 
            (gencontrol_channel_t) GEN_DEMUX_ENABLE) != DMX_STATUS_OK) {
        trace_new(DPS_ERROR_MSG,"DEMUX:control_channel failed\n");
    }
#endif

    /* Turn A/V sync on if specified */
    hw_set_aud_sync(SyncWithPCR);
    return;
    
} /* gen_audio_play */

/*
 *            gen_audio_stop
 *
 *  FILENAME: aud_comn\aud_comn.c
 *
 *  PARAMETERS: none.
 *
 *  DESCRIPTION:  Disables the audio PID and disables audio sync.
 *                Audio should be muted before this point as glitches may occur.
 *                Stops the DSP and flushes the encoded audio buffers
 *
 *  RETURNS: none.
 */
void gen_audio_stop(void)
{
    SyncOn = FALSE;
    /* disable audio PID */
#ifdef DRIVER_INCL_GENDMXC
    gen_dmx_control_av_channel((u_int32) 0, 
            (u_int32) AUDIO_CHANNEL, (gencontrol_channel_t) GEN_DEMUX_DISABLE);
#else
    if (cnxt_dmx_channel_control(gDemuxInstance, (u_int32) AUDIO_CHANNEL, 
            (gencontrol_channel_t) GEN_DEMUX_DISABLE) != DMX_STATUS_OK) {
        trace_new(DPS_ERROR_MSG,"DEMUX:control_channel failed\n");
    }
#endif
    /* Turn sync off, stop the DSP and flush demux buffers,  */
    hw_set_aud_sync(FALSE);
    hw_aud_stop(TRUE);         /* Stop and TRUE=FLUSH */
    hw_aud_start();            /* Restart the DSP so it will see resets */
                               /* from the audio display unit (AOC) to  */
                               /* make it ready to do next channel      */
    return;
    
} /* gen_audio_stop */

/*
 *  cnxt_download_ucode
 *
 *  FILENAME: aud_comn\aud_comn.c
 *
 *  PARAMETERS:     bool useAC3ucode    TRUE for AC3
 *                                      FALSE for MPEG
 *
 *  DESCRIPTION:  This function should download the audio microcode
 *                for audio data of the format specified.  Currently
 *                only MPEG and AC3 are supported.
 *
 */
bool cnxt_download_ucode( bool useAC3ucode )
{
    bool ret;
    if (useAC3ucode) {
#ifdef DRIVER_INCL_AC3
        ret = LoadMicrocode(0, AC3_AUDIO_UCODE, (AC3_AUDIO_UCODE + AC3_AUDIO_UCODE_SIZE));
        CNXT_SET_VAL(glpCtrl1, MPG_CONTROL1_ENABLEDOWNMIX_MASK, 1);
#ifdef MPG_AUDIOMODE_AC3
        CNXT_SET_VAL(glpCtrl1, MPG_CONTROL1_AUDIOMODE_MASK, MPG_AUDIOMODE_AC3);
#endif
#else /* Not including AC3 driver, so fail. */
        ret = FALSE;
        trace_new(TRACE_LEVEL_ALWAYS | TRACE_AUD, "Trying to download AC3 when driver not included.\n");
#endif /* AC3 */
    } else
    {   /* Must be MPEG Audio */
        ret = LoadMicrocode(0, MPEG_AUDIO_UCODE, (MPEG_AUDIO_UCODE + MPEG_AUDIO_UCODE_SIZE));
        CNXT_SET_VAL(glpCtrl1, MPG_CONTROL1_ENABLEDOWNMIX_MASK, 0);
    }
    if (ret == FALSE) {
        trace_new(TRACE_LEVEL_ALWAYS | TRACE_AUD, "audio microcode download failed.\n");
        error_log(MOD_AUD & 0x1);
    }
    return(ret);
}
/*
 *  cnxt_ac3_passthru
 *
 *  FILENAME:     aud_comn\aud_comn.c
 *
 *  PARAMETERS:   bool TurnAc3On      TRUE for AC3 pass thru to Sp/Dif port
 *                                    FALSE for PCM   to Sp/Dif port
 *
 *  DESCRIPTION:  This function will set up the Sp/Dif port and pawser for
 *                either AC3 pass thru or PCM and Mpeg. In either 
 *                case, the Sp/Dif port is set up to be audio output.
 *
 *  RETURNS:      TRUE if successful
 *                FALSE if unsuccessful.
 */
bool cnxt_ac3_passthru( bool TurnAc3On )
{
#if SOFTWARE_AC3_SPDIF_FIX != YES && AUDIO_MICROCODE == UCODE_COLORADO
    bool kstate;
#endif /* SOFTWARE_AC3_SPDIF_FIX */
    bool RetStatus = TRUE;

    #if PLL_PIN_GPIO_MUX1_REG_DEFAULT == NOT_DETERMINED
    /*
    * Enable output on the Sp/Dif port
    */
    *((LPREG) PLL_PIN_GPIO_MUX1_REG) |= PLL_PIN_GPIO_MUX1_AUD_SPDIF;
    #endif

    /*
    * Are we trying to turn it on or off? If on, make sure the
    * pawser supports it.
    */
    if (TurnAc3On) {
#if AUDIO_MICROCODE == UCODE_COLORADO
 #if SOFTWARE_AC3_SPDIF_FIX == YES
        *glpIntMask &= ~MPG_AB_FULL;  /* Turn off the Full interrupts */
        CNXT_SET_VAL(glpCtrl1, MPG_CONTROL1_SPDIFISAC3_MASK, TRUE);  /* Set the port to AC3 */
 #else /* SOFTWARE_AC3_SPDIF_FIX == NO */
      if ((*((LPREG)DPS_CAPABILITIES_EX(gDemuxInstance))) & DPS_CAPABILITIES_AC3_SPDIF_FIX) {
        *glpIntMask &= ~MPG_AB_FULL;  /* Turn off the Full interrupts */

        /* Tell pawser to re-read audio encoded write pointer */
        kstate = critical_section_begin();
        *lpAudWritePtr = (MPG_ADDRESS)HWBUF_ENCAUD_ADDR;
        *lpAudReadPtr = (MPG_ADDRESS)HWBUF_ENCAUD_ADDR;
        *DPS_INFO_CHANGE_REG_EX(gDemuxInstance) |= (1 << AUDIO_CHANNEL);    
        critical_section_end(kstate);

        CNXT_SET_VAL(glpCtrl1, MPG_CONTROL1_SPDIFISAC3_MASK, TRUE);  /* Set the port to AC3 */
        *DPS_PES_PID_REG_EX(gDemuxInstance) |= 4;  /* turn on bit 2 for pawser */
      } else {  /* ERROR: The pawser does not support AC3. */
        RetStatus = FALSE;
        trace_new(TRACE_LEVEL_1 | TRACE_AUD, "cnxt_ac3_passthru: Error!!!!!!!!!!!\n");
        trace_new(TRACE_LEVEL_1 | TRACE_AUD, "Pawser doesn't support AC3: Caps are %x\n", (*((LPREG)DPS_CAPABILITIES_EX(gDemuxInstance))));
      }
 #endif /* SOFTWARE_AC3_SPDIF_FIX */
#else /* AUDIO_MICROCODE != UCODE_COLORADO */
      CNXT_SET_VAL(glpCtrl1, MPG_CONTROL1_SPDIFISAC3_MASK, TRUE);  /* Set the port to AC3 */
#endif /* AUDIO_MICROCODE == UCODE_COLORADO */
    } else { /* !TurnAc3On */
      CNXT_SET_VAL(glpCtrl1, MPG_CONTROL1_SPDIFISAC3_MASK, FALSE);   /* Set the port to PCM */
#if AUDIO_MICROCODE == UCODE_COLORADO
      *glpIntMask |= MPG_AB_FULL;     /* Turn on the Full interrupts */
 #if SOFTWARE_AC3_SPDIF_FIX != YES
      *DPS_PES_PID_REG_EX(gDemuxInstance) &= ~4;  /* turn off bit 2 for pawser */
 #endif /* SOFTWARE_AC3_SPDIF_FIX != YES */
#endif /* AUDIO_MICROCODE == UCODE_COLORADO */
    }
    return(RetStatus);
}

#ifdef HWBUF_ENCAC3_ADDR
/*
 *  cnxt_MpgDecAc3Pt
 *
 *  FILENAME:     aud_comn\aud_comn.c
 *
 *  PARAMETERS:   bool EnableDecAndPt TRUE to select MPEG decode + AC3 pass thru to Sp/Dif port
 *                                    FALSE to select single stream operations
 *
 *  DESCRIPTION:  This function is used to enable/select 1.5 decode, that is handling
 *                two audio pids. One of the pids is required to be MPEG pid while the
 *                other needs to be AC3. 
 *                MPEG pid will be decodd and presented out as PCM while the AC3
 *                encoded stream will be passed through to the SPDIF port.
 *
 *  RETURNS:      TRUE
 */
 
bool cnxt_MpgDecAc3Pt( bool EnableDecAndPt )
{
      CNXT_SET_VAL(glpDecoderStatusReg, MPG_DECODE_STATUS_AUD1P5DECENABLE_MASK, EnableDecAndPt);
      return(TRUE);
}
#endif /* #ifdef HWBUF_ENCAC3_ADDR */

/*
 *  cnxt_audio_is_valid
 *
 *  FILENAME:     aud_comn\aud_comn.c
 *
 *  PARAMETERS:   none.
 *
 *  DESCRIPTION:  This function will return TRUE if the audio microcode
 *                thinks it is decoding valid audio, FALSE if the audio
 *                microcode thinks it is not decoding valid audio. See
 *                the notes on the routine cnxt_audio_get_audio_info().
 *
 *  RETURNS:      TRUE if decoding valid audio.
 *                FALSE if not decoding valid audio.
 */
bool cnxt_audio_is_valid( void )
{
    return ((cnxt_audio_get_audio_info() & AUDIO_INFO_VALID_MASK) == AUDIO_INFO_VALID_MASK);
}

/*
 *  cnxt_audio_get_header
 *
 *  FILENAME:     aud_comn\aud_comn.c
 *
 *  PARAMETERS:   none.
 *
 *  DESCRIPTION:  This function will return the mpeg audio header
 *                from the audio microcode.
 *                See the notes on the routine cnxt_audio_get_audio_info().
 *
 *  RETURNS:      Mpeg audio header (will be all 0's if audio not valid)
 */
u_int32 cnxt_audio_get_header( void )
{
    return (cnxt_audio_get_audio_info() & AUDIO_INFO_HEADER_MASK);
}
/*
 *  cnxt_audio_get_audio_info
 *
 *  FILENAME:     aud_comn\aud_comn.c
 *
 *  PARAMETERS:   none.
 *
 *  DESCRIPTION:  This function will return the audio information that
 *                is supplied by the audio microcode. Currently, that
 *                includes the mpeg audio header and a valid indicator.
 *                Note that the audio microcode updates this value at most
 *                once per audio frame, so we return the same value until
 *                enough time has elapsed that it can be updated.
 *
 *  RETURNS:      32 bit audio info value from the audio microcode.
 */
u_int32 cnxt_audio_get_audio_info( void )
{
    static u_int32 uiLastTime = 0;
    static bool bInitialized = FALSE;
    static u_int32 uiMinTimeLimit = AUDIO_INFO_MIN_TIME; /* may be dynamic later */
    bool kstate;
    LPREG lpAudioInfo = (LPREG) ( AUDIO_INFO_REG | NCR_BASE );
    
    /* 
    * if the microcode has not had enough time to update the
    * value since the last time, then don't update the value.
    */
    if ((!bInitialized) || (get_system_time() - uiLastTime) >= uiMinTimeLimit)  {
      /* 
      * Enough time has elapsed.
      * Read the current information, update the timer, and clear
      * the current information.
      */
      kstate = critical_section_begin();
      guiAudioInfo = *lpAudioInfo;
      uiLastTime = get_system_time();
      *lpAudioInfo = 0;
      critical_section_end(kstate);
      bInitialized = TRUE;
    }

    return (guiAudioInfo);
}

#if (CANAL_AUDIO_HANDLING==YES)
/********************************************************************/
/*  FUNCTION:    volume_ramp_task                                   */
/*                                                                  */
/*  PARAMETERS:  None (arg is ignored)                              */
/*                                                                  */
/*  DESCRIPTION: Ramp the audio volume back up on request from      */
/*               other parts of the driver.                         */
/*                                                                  */
/*  RETURNS:     Does not return.                                   */
/*                                                                  */
/*  CONTEXT:     This is a task main function.                      */
/*                                                                  */
/********************************************************************/
void volume_ramp_task( void *arg )
{
    int     i;
    u_int32 frametime = 24; /* frametime in ms of MPEG layer II, 48 kHz audio */
    bool    ks;
    u_int32 uDSRValue;

    while(1)
    {
        /* Wait to be told to look for valid audio and unmute */
        sem_get( aud_ramp_up_sem, KAL_WAIT_FOREVER );
        
        if (volume_ramp_target_tick > get_system_time())
        {
          trace_new(TRACE_LEVEL_2|TRACE_TST, "AUDIO: Sleeping till end of ramp up holdoff time\n");
          task_time_sleep((volume_ramp_target_tick - get_system_time()));
        }
        
        trace_new(TRACE_LEVEL_2|TRACE_TST, "AUDIO: Waiting for valid audio before ramping volume up\n");
        
        while(!cnxt_audio_is_valid())
        {
            /* Sleep a couple of frame times then check for valid again */
            task_time_sleep(2*frametime);
        }

        /* Wait a bit before we unmute. Without this, we sometimes get minor glitches */
        /* as audio restarts.                                                         */
        task_time_sleep(WAIT_VALID_TO_UNMUTE);
            
        trace_new(TRACE_LEVEL_2|TRACE_TST, "AUDIO: Ramping volume back up\n");
            
        ks = critical_section_begin();
            
        /* Disable decoder mute unless user has enabled it */
        CNXT_SET_VAL(glpCtrl1, MPG_CONTROL1_MUTE_MASK, (0 || user_mute));
        
        /* Ramp volume up quickly */
        for (i=5; i<current_volume; i+=5)
            audio_set_volume_internal(i, AUD_ATTN_MODULE_PCM_OUTPUT);
            
        /* In case the user set a volume level which is not a */
        /* multiple of 5...                                   */
        audio_set_volume_internal(current_volume, AUD_ATTN_MODULE_PCM_OUTPUT);
        
        critical_section_end(ks);

        /* Wait a bit before we start looking for problems again. If we don't do this, */
        /* we occasionally see spurious "data dried up" signals as the audio gets back */
        /* up and running and this causes audible artifacts.                           */
        task_time_sleep(WAIT_UNMUTE_TO_INT_ENABLE);

        /* Set things up to look for the next dropout */
        *lpDpsEventStatus = DPS_BAD_PES_HEADER;  /* Clear any existing bad PES event */
        *lpDpsEventEnable |= DPS_BAD_PES_HEADER; /* then reenable the interrupt from this event */
        
        gbAudioDroppedOut = FALSE;
      
        /* Clear any pending decoder errors to prevent us being interrupted for an  */
        /* old error once we start looking for dry-up conditions again. Reading the */
        /* DSR register has the effect of clearing it (nice, eh?).                  */
        uDSRValue = *(LPREG)glpDecoderStatusReg; 
      
        /* Tell the video driver to start looking for cases where the */
        /* audio data dries up (status register is shared with video  */
        /* decoder) assuming audio is currently valid.                */
        bCheckForAudioDry = TRUE;      

    }
}
#endif /* (CANAL_AUDIO_HANDLING==YES) */

/********************************************************************/
/*  FUNCTION:    audio_ramp_vol_up                                  */
/*                                                                  */
/*  PARAMETERS:  none                                               */
/*                                                                  */
/*  DESCRIPTION: Ramp the audio PCM output level back to previous   */
/*               level.                                             */
/*                                                                  */
/*  RETURNS:     nothing                                            */
/*                                                                  */
/*  CONTEXT:     May be called in any context                       */
/*                                                                  */
/********************************************************************/
void audio_ramp_vol_up( void )
{
  int     i;
  bool ks;
  
  ks = critical_section_begin();
  
  /* Turn mute off */
  CNXT_SET_VAL(glpCtrl1, MPG_CONTROL1_MUTE_MASK, 0);
  
  /* Ramp volume up quickly */
  for (i=5; i<current_volume; i+=5)
    audio_set_volume_internal(i, AUD_ATTN_MODULE_PCM_OUTPUT);
  
  /* In case the user set a volume level which is not a */
  /* multiple of 5...                                   */
  audio_set_volume_internal(current_volume, AUD_ATTN_MODULE_PCM_OUTPUT);
  
  critical_section_end(ks);
}

#if (CANAL_AUDIO_HANDLING==YES)
/********************************************************************/
/*  FUNCTION:    audio_ramp_vol_down                                */
/*                                                                  */
/*  PARAMETERS:  min_time_down - number of milliseconds for which   */
/*                               audio must remain muted before any */
/*                               attempt will be made to unmute it  */
/*                               by the volume_ramp_task.           */
/*                                                                  */
/*  DESCRIPTION: Ramp the audio PCM output level to 0 then mute the */
/*               decoder output.                                    */
/*                                                                  */
/*  RETURNS:     TRUE (return code maintained to keep the prototype */
/*               the same as the equivalent function used when      */
/*               building an OpenTV image)                          */
/*                                                                  */
/*  CONTEXT:     May be called in any context                       */
/*                                                                  */
/********************************************************************/
bool audio_ramp_vol_down(u_int32 min_time_down) 
{
    int i;
    bool ret_val = TRUE;
    bool ks;

    isr_trace_new(TRACE_TST|TRACE_LEVEL_2, "AUDIO: Ramping volume down. Holdoff time %dmS\n", min_time_down, 0);

    /* What will the system time be when we allow unmute requests to */
    /* be serviced?                                                  */
    volume_ramp_target_tick = get_system_time() + min_time_down;
    
    ks = critical_section_begin();
 
    /* Ramp the PCM volume down to 0 */
    for (i=audio_get_volume(); i>=0; i-=5) 
      audio_set_volume_internal(i);
      
    /* Now mute the decoder output */  
    CNXT_SET_VAL(glpCtrl1, MPG_CONTROL1_MUTE_MASK, 1);
      
    critical_section_end(ks);
    
    return(ret_val);
}
#endif /* (CANAL_AUDIO_HANDLING==YES) */

#if (CANAL_AUDIO_HANDLING==YES)
void cnxt_audio_drop_out(void)
{
    u_int32 uDummy;
    bool    bSynced = FALSE;
    
    /* Stop looking for the interrupt signalling data dried up */
    bCheckForAudioDry = FALSE;
    /* Clear any pending decoder errors to prevent us being interrupted for an */
    /* old error once we unmute.                                               */
    uDummy = *(LPREG)glpDecoderStatusReg;    
    
    /* Mute the decoder output */
    CNXT_SET_VAL(glpCtrl1, MPG_CONTROL1_MUTE_MASK, 1);
    
    /* Ramp down the PCM output */
    audio_ramp_vol_down(300);
    
    isr_trace_new( TRACE_TST|TRACE_LEVEL_2, "Audio drop out detected\n", 0, 0);
    gbAudioDroppedOut = TRUE;

    /* Mark the audio as invalid */
    guiAudioInfo = 0;
    
    /* Disable sync if it is enabled */
    if (CNXT_GET(glpCtrl1, MPG_CONTROL1_ENABLEAUDSYNC_MASK)) 
    {
        CNXT_SET_VAL(glpCtrl1, MPG_CONTROL1_ENABLEAUDSYNC_MASK, 0);
        bSynced = TRUE;
     }   
         
    /* Flush the encoded audio buffer, making sure we disable the decoder while we do this */
    /* to make sure that we don't have a fight when trying to modify the register.         */
    CNXT_SET_VAL(glpCtrl1, MPG_CONTROL1_ENCAUDIOENABLE_MASK, 0);
    uDummy = *(LPREG)glpCtrl1;
    *lpAudReadPtr = *lpAudWritePtr;  
    CNXT_SET_VAL(glpCtrl1, MPG_CONTROL1_ENCAUDIOENABLE_MASK, 1);
    
    *DPS_INFO_CHANGE_REG_EX(gDemuxInstance) |= (1 << AUDIO_CHANNEL);
    #ifndef WABASH_AVSYNC_CLKREC_DISABLED
    if(bSynced)
      CNXT_SET_VAL(glpCtrl1, MPG_CONTROL1_ENABLEAUDSYNC_MASK, 1);
    #endif
        
    /* Tell the volume_ramp_task to start looking for valid audio again */
    
    /* Clear the signal semaphore to make sure we only post it once */
    do {
      uDummy = sem_get(aud_ramp_up_sem, 0);
    } while (uDummy == RC_OK);
    sem_put(aud_ramp_up_sem);  
     
    //*glpIntMask |= MPG_NEW_PTS_RECEIVED;   /* Turn interrupt on so we know good audio */
}
#endif /* (CANAL_AUDIO_HANDLING==YES) */
/** private functions **/

/********************************************************************/
/*  FUNCTION:    cnxt_audio_get_buffer_errors                       */
/*                                                                  */
/*  PARAMETERS:  pOverflow - pointer to audio buffer overflow cnt   */
/*               pUnderflow - pointer to audio buffer underflow cnt */
/*                                                                  */
/*  DESCRIPTION: Updates audio buffer full, empty counts            */
/*                                                                  */
/*  RETURNS:     nothing                                            */
/*                                                                  */
/*  CONTEXT:     May be called in any context                       */
/*                                                                  */
/********************************************************************/
void cnxt_audio_get_buffer_errors( u_int32 *pOverflow, u_int32 *pUnderflow)
{
    *pOverflow = ab_full_cnt;
    *pUnderflow = ab_empty_cnt;
}

#if ((PCM_AUDIO_TYPE != PCM_AUDIO_COLORADO) && (PCM_AUDIO_TYPE != PCM_AUDIO_WABASH))

 #if (CPU_TYPE!=CPU_ARM940T)

/*********************************************************************/
/*  cnxt_audio_spdif_headerbits_set()                                */
/*                                                                   */
/*  PARAMETERS: AUDIO_SPDIF_HEADER_PARAM eParam                      */
/*              u_int32 ui32Value                                    */
/*                                                                   */
/*  DESCRIPTION:Set specified bits in the SPDIF header.              */
/*                                                                   */
/*  RETURNS: RC_OK if success                                        */
/*           Appropriate failure return code otherwise               */
/*********************************************************************/
int cnxt_audio_spdif_headerbits_set(AUDIO_SPDIF_HEADER_PARAM eParam, 
                                    u_int32 ui32Value)
{
   int iRetcode = RC_OK;

   #if CPU_TYPE == AUTOSENSE
   if(GetCPUType() == CPU_ARM940T)
   {
      return((MOD_AUD + 0x0001));
   }
   #endif /* CPU_TYPE == AUTOSENSE */

   switch(eParam)
   {
      case AUDIO_SPDIF_HEADER_PARAM_CONSUMER:
         CNXT_SET_VAL(AUD_CHAN_STAT_HEADER_REG, 
                      AUD_CHAN_STAT_HEADER_CONSUMER_MASK,
                      ui32Value);
         trace_new(TRACE_AUD | TRACE_LEVEL_2, 
            "SPDIFSet: Set Consumer Flag to : %d\n", ui32Value);
      break;

      case AUDIO_SPDIF_HEADER_PARAM_DATA_AUDIO:
         CNXT_SET_VAL(AUD_CHAN_STAT_HEADER_REG, 
                      AUD_CHAN_STAT_HEADER_AUDIO_FLAG_MASK,
                      ui32Value);
         trace_new(TRACE_AUD | TRACE_LEVEL_2, 
            "SPDIFSet: Set Audio Flag to : %d\n", ui32Value);
      break;

      case AUDIO_SPDIF_HEADER_PARAM_COPYRIGHT:
         CNXT_SET_VAL(AUD_CHAN_STAT_HEADER_REG, 
                      AUD_CHAN_STAT_HEADER_COPYRIGHT_MASK,
                      ui32Value);
         trace_new(TRACE_AUD | TRACE_LEVEL_2, 
            "SPDIFSet: Set Copyright Flag to : %d\n", ui32Value);
      break;

      case AUDIO_SPDIF_HEADER_PARAM_EMPHASIS:
         CNXT_SET_VAL(AUD_CHAN_STAT_HEADER_REG, 
                      AUD_CHAN_STAT_HEADER_EMPHASIS_MASK,
                      ui32Value);
         trace_new(TRACE_AUD | TRACE_LEVEL_2, 
            "SPDIFSet: Set Emphasis to : %d\n", ui32Value);
      break;

      case AUDIO_SPDIF_HEADER_PARAM_MODE:
         CNXT_SET_VAL(AUD_CHAN_STAT_HEADER_REG, 
                      AUD_CHAN_STAT_HEADER_MODE_MASK,
                      ui32Value);
         trace_new(TRACE_AUD | TRACE_LEVEL_2, 
            "SPDIFSet: Set Mode to : %d\n", ui32Value);
      break;

      case AUDIO_SPDIF_HEADER_PARAM_MODE00_CATEGORY_CODE:
         CNXT_SET_VAL(AUD_CHAN_STAT_HEADER_REG, 
                      AUD_CHAN_STAT_HEADER_MODE00_CATEGORY_CODE_MASK,
                      ui32Value);
         trace_new(TRACE_AUD | TRACE_LEVEL_2, 
            "SPDIFSet: Set Mode 00 Category Code to : %d\n", ui32Value);
      break;

      case AUDIO_SPDIF_HEADER_PARAM_MODE00_GENERATION_STATUS:
         CNXT_SET_VAL(AUD_CHAN_STAT_HEADER_REG, 
                      AUD_CHAN_STAT_HEADER_MODE00_GENERATION_STAT_MASK,
                      ui32Value);
         trace_new(TRACE_AUD | TRACE_LEVEL_2, 
            "SPDIFSet: Set Mode 00 Generation Status to : %d\n", ui32Value);
      break;

      case AUDIO_SPDIF_HEADER_PARAM_MODE00_SOURCE_NUM:
         CNXT_SET_VAL(AUD_CHAN_STAT_HEADER_REG, 
                      AUD_CHAN_STAT_HEADER_MODE00_SOURCE_NUM_MASK,
                      ui32Value);
         trace_new(TRACE_AUD | TRACE_LEVEL_2, 
            "SPDIFSet: Set Mode 00 Source Num to : %d\n", ui32Value);
      break;

      case AUDIO_SPDIF_HEADER_PARAM_MODE00_CHANNEL_NUM:
         CNXT_SET_VAL(AUD_CHAN_STAT_HEADER_REG, 
                      AUD_CHAN_STAT_HEADER_MODE00_CHANNEL_NUM_MASK,
                      ui32Value);
         trace_new(TRACE_AUD | TRACE_LEVEL_2, 
            "SPDIFSet: Set Mode 00 Channel Num to : %d\n", ui32Value);
      break;

      case AUDIO_SPDIF_HEADER_PARAM_MODE00_SAMPLE_FREQUENCY:
         CNXT_SET_VAL(AUD_CHAN_STAT_HEADER_REG, 
                      AUD_CHAN_STAT_HEADER_MODE00_SAMPLE_FREQUENCY_MASK,
                      ui32Value);
         trace_new(TRACE_AUD | TRACE_LEVEL_2, 
            "SPDIFSet: Set Mode 00 Sample Frequency to : %d\n", ui32Value);
      break;

      case AUDIO_SPDIF_HEADER_PARAM_MODE00_CLK_ACCURACY:
         CNXT_SET_VAL(AUD_CHAN_STAT_HEADER_REG, 
                      AUD_CHAN_STAT_HEADER_MODE00_CLK_ACCURACY_MASK,
                      ui32Value);
         trace_new(TRACE_AUD | TRACE_LEVEL_2, 
            "SPDIFSet: Set Mode 00 Clock Accuracy to : %d\n", ui32Value);
      break;

      default :
         iRetcode = (MOD_AUD + 0x0003); /* Invalid */
         trace_new(TRACE_AUD | TRACE_LEVEL_ALWAYS, 
            TRACE_FG_LIGHT_RED
              "SPDIFSet: Invalid Option : %d\n"
            TRACE_FG_NORMAL, eParam);
      break;
   }

   return(iRetcode);
}

/*********************************************************************/
/*  cnxt_audio_spdif_headerbits_get()                                */
/*                                                                   */
/*  PARAMETERS: AUDIO_SPDIF_HEADER_PARAM eParam                      */
/*              u_int32 *pui32Value                                  */
/*                                                                   */
/*  DESCRIPTION:Get specified bits in the SPDIF header.              */
/*                                                                   */
/*  RETURNS: RC_OK if success                                        */
/*           Appropriate failure return code otherwise               */
/*********************************************************************/
int cnxt_audio_spdif_headerbits_get(AUDIO_SPDIF_HEADER_PARAM eParam, 
                                    u_int32 *pui32Value)
{
   int iRetcode = RC_OK;

   #if CPU_TYPE == AUTOSENSE
   if(GetCPUType() == CPU_ARM940T)
   {
      return((MOD_AUD + 0x0001));
   }
   #endif /* CPU_TYPE == AUTOSENSE */

   switch(eParam)
   {
      case AUDIO_SPDIF_HEADER_PARAM_CONSUMER:
         *pui32Value = CNXT_GET_VAL(AUD_CHAN_STAT_HEADER_REG, 
                                    AUD_CHAN_STAT_HEADER_CONSUMER_MASK);
         trace_new(TRACE_AUD | TRACE_LEVEL_2, 
            "SPDIFGet: Consumer Flag Val : %d\n", *pui32Value);
      break;

      case AUDIO_SPDIF_HEADER_PARAM_DATA_AUDIO:
         *pui32Value = CNXT_GET_VAL(AUD_CHAN_STAT_HEADER_REG, 
                                    AUD_CHAN_STAT_HEADER_AUDIO_FLAG_MASK);
         trace_new(TRACE_AUD | TRACE_LEVEL_2, 
            "SPDIFGet: Audio Flag Val : %d\n", *pui32Value);
      break;

      case AUDIO_SPDIF_HEADER_PARAM_COPYRIGHT:
         *pui32Value = CNXT_GET_VAL(AUD_CHAN_STAT_HEADER_REG, 
                                    AUD_CHAN_STAT_HEADER_COPYRIGHT_MASK);
         trace_new(TRACE_AUD | TRACE_LEVEL_2, 
            "SPDIFGet: Copyright Flag Val : %d\n", *pui32Value);
      break;

      case AUDIO_SPDIF_HEADER_PARAM_EMPHASIS:
         *pui32Value = CNXT_GET_VAL(AUD_CHAN_STAT_HEADER_REG, 
                                    AUD_CHAN_STAT_HEADER_EMPHASIS_MASK);
         trace_new(TRACE_AUD | TRACE_LEVEL_2, 
            "SPDIFGet: Emphasis Val : %d\n", *pui32Value);
      break;

      case AUDIO_SPDIF_HEADER_PARAM_MODE:
         *pui32Value = CNXT_GET_VAL(AUD_CHAN_STAT_HEADER_REG, 
                                    AUD_CHAN_STAT_HEADER_MODE_MASK);
         trace_new(TRACE_AUD | TRACE_LEVEL_2, 
            "SPDIFGet: Mode Val : %d\n", *pui32Value);
      break;

      case AUDIO_SPDIF_HEADER_PARAM_MODE00_CATEGORY_CODE:
         *pui32Value = CNXT_GET_VAL(AUD_CHAN_STAT_HEADER_REG, 
                                    AUD_CHAN_STAT_HEADER_MODE00_CATEGORY_CODE_MASK);
         trace_new(TRACE_AUD | TRACE_LEVEL_2, 
            "SPDIFGet: Mode 00 Category Code Val : %d\n", *pui32Value);
      break;

      case AUDIO_SPDIF_HEADER_PARAM_MODE00_GENERATION_STATUS:
         *pui32Value = CNXT_GET_VAL(AUD_CHAN_STAT_HEADER_REG, 
                                    AUD_CHAN_STAT_HEADER_MODE00_GENERATION_STAT_MASK);
         trace_new(TRACE_AUD | TRACE_LEVEL_2, 
            "SPDIFGet: Mode 00 Generation Status Val : %d\n", *pui32Value);
      break;

      case AUDIO_SPDIF_HEADER_PARAM_MODE00_SOURCE_NUM:
         *pui32Value = CNXT_GET_VAL(AUD_CHAN_STAT_HEADER_REG, 
                                    AUD_CHAN_STAT_HEADER_MODE00_SOURCE_NUM_MASK);
         trace_new(TRACE_AUD | TRACE_LEVEL_2, 
            "SPDIFGet: Mode 00 Source Num Val : %d\n", *pui32Value);
      break;

      case AUDIO_SPDIF_HEADER_PARAM_MODE00_CHANNEL_NUM:
         *pui32Value = CNXT_GET_VAL(AUD_CHAN_STAT_HEADER_REG, 
                                    AUD_CHAN_STAT_HEADER_MODE00_CHANNEL_NUM_MASK);
         trace_new(TRACE_AUD | TRACE_LEVEL_2, 
            "SPDIFGet: Mode 00 Channel Num Val : %d\n", *pui32Value);
      break;

      case AUDIO_SPDIF_HEADER_PARAM_MODE00_SAMPLE_FREQUENCY:
         *pui32Value = CNXT_GET_VAL(AUD_CHAN_STAT_HEADER_REG, 
                                    AUD_CHAN_STAT_HEADER_MODE00_SAMPLE_FREQUENCY_MASK);
         trace_new(TRACE_AUD | TRACE_LEVEL_2, 
            "SPDIFGet: Mode 00 Sample Frequency Val : %d\n", *pui32Value);
      break;

      case AUDIO_SPDIF_HEADER_PARAM_MODE00_CLK_ACCURACY:
         *pui32Value = CNXT_GET_VAL(AUD_CHAN_STAT_HEADER_REG, 
                                    AUD_CHAN_STAT_HEADER_MODE00_CLK_ACCURACY_MASK);
         trace_new(TRACE_AUD | TRACE_LEVEL_2, 
            "SPDIFGet: Mode 00 Clock Accuracy Val : %d\n", *pui32Value);
      break;

      default :
         iRetcode = (MOD_AUD + 0x0003); /* Invalid */
         trace_new(TRACE_AUD | TRACE_LEVEL_ALWAYS, 
            TRACE_FG_LIGHT_RED 
              "SPDIFGet: Invalid Option : %d\n"
            TRACE_FG_NORMAL, eParam);
      break;
   }

   return(iRetcode);
}

 #endif /* (CPU_TYPE!=CPU_ARM940T) */

#endif /* PCM_AUDIO_TYPE != PCM_AUDIO_COLORADO && PCM_AUDIO_WABASH */

/****************************************************************************
 * Modifications:
 * $Log: 
 *  116  mpeg      1.115       6/27/04 10:56:54 PM    Xiao Guang Yan  CR(s) 
 *        9598 9599 : Set PIN MUX for PIO42 for Pudong audio DAC.
 *  115  mpeg      1.114       5/3/04 9:33:08 PM      Matt Korte      CR(s) 
 *        8974 8973 : if attempting to download ac3 and don't have ac3 driver, 
 *        then fail download
 *  114  mpeg      1.113       4/6/04 1:37:37 AM      Xiao Guang Yan  CR(s) 
 *        8774 8775 : Changed Audio DAC settings for Pudong board. 
 *  113  mpeg      1.112       4/1/04 4:43:21 PM      Dave Aerne      CR(s) 
 *        8729 8730 : added support an additional mode of ac3 passthrough 
 *        within mpeg ucode.
 *        Passthrough is now supported w/o decode of mpeg in addition to still 
 *        supporting
 *        1.5 decode mode, which is decode mpeg and passthrough ac3.
 *  112  mpeg      1.111       3/9/04 12:32:24 PM     Steven Jones    CR(s) 
 *        8540 : Change aud_before_reset and aud_after_reset so that they just 
 *        return if
 *        called before gen_audio_init. This stops an undocumented dependancy 
 *        that
 *        audio must be initialised before video (these functions are called 
 *        from
 *        gen_video_init).
 *  111  mpeg      1.110       1/29/04 12:01:25 PM    Miles Bintz     CR(s) 
 *        8305 : added new line to end of character
 *  110  mpeg      1.109       12/23/03 8:45:26 AM    Ian Mitchell    CR(s) 
 *        7739 : Change the output format to I2S for the Crosby platform.
 *        
 *  109  mpeg      1.108       9/26/03 5:54:38 PM     Miles Bintz     SCR(s) 
 *        7559 :
 *        added a trace_fg_normal after an error string to reset color back to 
 *        white
 *        
 *  108  mpeg      1.107       7/31/03 2:22:40 PM     Steven Jones    SCR(s): 
 *        7093 
 *        Back-out interrupt-checking change of 15th July.
 *        
 *  107  mpeg      1.106       7/24/03 5:32:40 PM     Dave Aerne      SCR(s) 
 *        7042 :
 *        corrected code to only set/reset the pawser pes pid reg when NOT 
 *        using 
 *        SOFTWARE_AC3_SPDIF_FIX and to not modify the reg otherwise. Fix only 
 *        applies 
 *        to colorado builds throughs use of ifdefs.
 *        
 *  106  mpeg      1.105       7/24/03 2:59:18 PM     Senthil Veluswamy SCR(s) 
 *        3674 :
 *        Changed hw_set_aud_mute() to keep track of number of mutes and 
 *        unmutes
 *        
 *  105  mpeg      1.104       7/22/03 12:27:20 PM    Larry Wang      SCR(s) 
 *        6976 :
 *        Remove floating variable a2s_ratio, which will not be used in clock 
 *        recovery any more.
 *        
 *  104  mpeg      1.103       7/14/03 6:10:28 PM     Senthil Veluswamy SCR(s) 
 *        2692 :
 *        Modified audio_mpeg_isr to only check for isr conditions that caused 
 *        the isr call.
 *        
 *  103  mpeg      1.102       7/9/03 6:16:52 PM      Senthil Veluswamy SCR(s) 
 *        6922 :
 *        Added interfaces to get and set the SPDIF header bits on Bronco B+.
 *        
 *  102  mpeg      1.101       6/23/03 8:44:40 PM     Senthil Veluswamy SCR(s) 
 *        6641 :
 *        Corrected number of parameters to aud vol set internal in 
 *        volume_ramp_task
 *        
 *  101  mpeg      1.100       6/23/03 8:19:44 PM     Senthil Veluswamy SCR(s) 
 *        6641 :
 *        Added audio volume get/set interfaces for rf modulator and audio 
 *        decoder. Fixed Low RF modulator bug.
 *        
 *  100  mpeg      1.99        6/3/03 10:22:12 AM     Larry Wang      SCR(s) 
 *        6667 :
 *        Initialize AC3 buffer for parser if the secondary audio channel is 
 *        defined.  Don't turn primary audio channel off when calling 
 *        cnxt_ac3_passthru(FALSE).
 *        
 *  99   mpeg      1.98        5/20/03 4:21:34 PM     Dave Aerne      SCR(s) 
 *        6492 :
 *        changed new_divisor setting within 24kHz case from pll_32kHz_divisor 
 *        to
 *        pll_24kHz_divisor.
 *        
 *  98   mpeg      1.97        5/15/03 7:43:50 PM     Dave Aerne      SCR(s) 
 *        6261 6291 :
 *        added support for download of aocucode upon init.
 *        
 *  97   mpeg      1.96        5/1/03 6:04:50 PM      Craig Dry       SCR(s) 
 *        5521 :
 *        Conditionally remove GPIO Pin Mux and Alt Func register accesses.
 *        
 *  96   mpeg      1.95        3/28/03 4:23:02 PM     Tim White       SCR(s) 
 *        5909 :
 *        Replace CRYSTAL_FREQUENCY #define with xtal_frequency global.
 *        
 *        
 *  95   mpeg      1.94        3/27/03 4:59:30 PM     Dave Aerne      SCR(s) 
 *        5898 :
 *        modifed trace_level to prevent message appearing for default debug 
 *        builds
 *        
 *  94   mpeg      1.93        3/21/03 5:00:48 PM     Dave Aerne      SCR(s) 
 *        5136 :
 *        additional comments.
 *        
 *  93   mpeg      1.92        3/21/03 3:39:58 PM     Dave Aerne      SCR(s) 
 *        5136 :
 *        added code and support for cnxt_audio_get_buffer_errors() routine 
 *        which
 *        returns the current values of the running audio buffer full and empty
 *         counters.
 *        Enabled MPG_PTS_RECEIVED interrupt to facilitate same.
 *        Replaced any existing tabs with spaces.
 *        
 *  92   mpeg      1.91        3/5/03 3:10:40 PM      Craig Dry       SCR(s) 
 *        5457 :
 *        Remove needles #ifdefs surrounding audio extension hooks
 *        
 *  91   mpeg      1.90        2/28/03 2:53:00 PM     Lucy C Allevato SCR(s) 
 *        5624 :
 *        Moved call to notify function outside the upper most if statement for
 *         MPG_AB_FULL.
 *        
 *  90   mpeg      1.89        2/27/03 6:16:52 PM     Dave Aerne      SCR(s) 
 *        4672 :
 *        added code to support AUTO_DETECT for live stream in which correct 
 *        audio
 *        microcode is determined dynamically once stream feed has begun and is
 *         downloaded
 *        
 *        
 *  89   mpeg      1.88        2/21/03 9:28:56 AM     Lucy C Allevato SCR(s) 
 *        5577 :
 *        Added boundry check to audio_set_volume.
 *        
 *  88   mpeg      1.87        2/12/03 6:08:32 PM     Dave Aerne      SCR(s) 
 *        5262 :
 *        removed ifdef CANAL_AUDIO_HANDLING around aud_ramp_vol_up function 
 *        and now use that function in generic fashion
 *        
 *  87   mpeg      1.86        2/10/03 11:46:16 AM    Senthil Veluswamy SCR(s) 
 *        5455 :
 *        fixed build warning.
 *        
 *  86   mpeg      1.85        2/10/03 11:13:18 AM    Senthil Veluswamy SCR(s) 
 *        5452 :
 *        Fixed bug. Enabled SPDIF AC3 path for non-colorado chips.
 *        
 *  85   mpeg      1.84        2/7/03 6:06:54 PM      Dave Aerne      SCR(s) 
 *        5450 :
 *        qualified software workaround to only be activated when memory 
 *        interface is 16-bit. Else interface is 32-bit and PCM audio
 *        transfers work as expected.
 *        
 *  84   mpeg      1.83        2/4/03 2:47:28 PM      Senthil Veluswamy SCR(s) 
 *        5401 :
 *        Added support for the Brazos 16bit memory, pcm playback sw 
 *        workaround. Use of this workaround will disable the hw workaround for
 *         the same problem.
 *        
 *  83   mpeg      1.82        1/24/03 7:03:00 PM     Brendan Donahe  SCR(s) 
 *        5315 :
 *        Fixed up 16-bit MC workaround using PCM_AUDIO_TYPE.
 *        
 *        
 *  82   mpeg      1.81        1/24/03 4:16:14 PM     Brendan Donahe  SCR(s) 
 *        5315 :
 *        Added workaround for Brazos A 16-bit memory controller mode.
 *        
 *        
 *  81   mpeg      1.80        1/22/03 9:18:34 PM     Brendan Donahe  SCR(s) 
 *        5288 :
 *        Added Brazos audio PLL programming by removing extra * 2 (there is no
 *         extra
 *        / 2 on Brazos as there is on Wabash) with significant input from 
 *        DaveA.
 *        
 *        
 *  80   mpeg      1.79        1/13/03 5:43:16 PM     Joe Kroesche    SCR(s) 
 *        5239 :
 *        OR'd register read in "get_info" function with NCR_BASE
 *        
 *  79   mpeg      1.78        12/18/02 1:33:46 PM    Dave Wilson     SCR(s) 
 *        5185 :
 *        Removed some redundant emulation code.
 *        
 *  78   mpeg      1.77        12/16/02 3:43:02 PM    Tim White       SCR(s) 
 *        5169 :
 *        Allow future chips to use Wabash code by default instead of the 
 *        Colorado code.
 *        The prescale is read from the chip and is different on Brazos (the 
 *        default) than
 *        on Wabash.
 *        
 *        
 *  77   mpeg      1.76        12/3/02 3:47:58 PM     Craig Dry       SCR(s) 
 *        4991 :
 *        Add audio_ramp_vol_up and make other routines visible to aud_api.c
 *        
 *  76   mpeg      1.75        11/25/02 12:34:50 PM   Craig Dry       SCR(s) 
 *        4991 :
 *        put PES structures/data here temporarily until consolidation
 *        into a single *.h file can be accomplished which does not break
 *        openTV
 *        
 *  75   mpeg      1.74        11/20/02 5:53:46 PM    Craig Dry       SCR(s) 
 *        4991 :
 *        Added hooks to call routines in AUD_API.C
 *        
 *  74   mpeg      1.73        11/20/02 3:26:42 PM    Craig Dry       SCR(s) 
 *        4991 :
 *        include audio.h for PES_global_data type declaration
 *        
 *  73   mpeg      1.72        11/20/02 3:07:16 PM    Craig Dry       SCR(s) 
 *        4991 :
 *        Move aud_PES_global_data to aud_comn.c to share with OpenTV and 
 *        AUD_API.C
 *        
 *  72   mpeg      1.71        11/15/02 4:03:36 PM    Senthil Veluswamy SCR(s) 
 *        4965 :
 *        Added hooks for Data Dry error handling
 *        Removed // Comments
 *        
 *  71   mpeg      1.70        11/15/02 12:51:30 PM   Senthil Veluswamy SCR(s) 
 *        4935 :
 *        Fixed a variable exluded in earlier wrap.
 *        
 *  70   mpeg      1.69        11/15/02 12:36:44 PM   Senthil Veluswamy SCR(s) 
 *        4935 :
 *        Wrapped SOFTWARE_AC3_SPDIF_FIX to be used only for Colorado
 *        
 *  69   mpeg      1.68        10/21/02 8:27:50 PM    Senthil Veluswamy SCR(s) 
 *        4790 :
 *        Added hooks for the Software AC3 SPDIF workaround
 *        
 *  68   mpeg      1.67        9/25/02 10:27:38 PM    Carroll Vance   SCR(s) 
 *        3786 :
 *        Removing old DRM and AUD conditional bitfield code.
 *        
 *        
 *  67   mpeg      1.66        8/29/02 5:02:14 PM     Matt Korte      SCR(s) 
 *        4493 :
 *        Add call to cnxt_ac3_passthru() to initialize the Sp/Dif port to be
 *        enabled to Mpeg PCM data.
 *        
 *        
 *  66   mpeg      1.65        8/5/02 12:52:56 PM     Ray Mack        SCR(s) 
 *        4212 :
 *        Fixed error in call to isr_trace_new that could make a reference 
 *        through a pointer on a non-word aligned boundary for a 32 bit value. 
 *         THis used a bitfield replacement macro incorrectly.
 *        
 *  65   mpeg      1.64        7/1/02 5:09:38 PM      Matt Korte      SCR(s) 
 *        4120 :
 *        Change gen_aud_ISRNotifyRegister() to be more generic
 *        
 *        
 *  64   mpeg      1.63        6/10/02 3:21:48 PM     Craig Dry       SCR(s) 
 *        3923 :
 *        Remove MPG bitfields unconditionally - Step 2
 *        
 *  63   mpeg      1.62        6/6/02 5:47:18 PM      Craig Dry       SCR(s) 
 *        3923 :
 *        Remove MPG bitfields conditionally - Step 1
 *        
 *  62   mpeg      1.61        6/3/02 12:37:36 PM     Carroll Vance   SCR(s) 
 *        3918 :
 *        Changed PLL_LOCK_CMD to PLL_LOCK_CMD_VALUE to avoid conflict with 
 *        bitfield type.
 *        
 *  61   mpeg      1.60        5/20/02 12:11:12 AM    Steve Glennon   SCR(s): 
 *        3823 
 *        Added static functions hw_aud_stop and hw_aud_stop to help 
 *        gen_audio_stop
 *        do a cleaner job of stopping the audio on channel change. hw_aud_stop
 *         issues 
 *        the halt bit in the CONTROL1 register, then turns off the demux and 
 *        then 
 *        empties the compressed audio buffer, before re-starting the DSP. This
 *         is to 
 *        flush out any remaining data from the old channel to avoid squawks.
 *        hw_aud_start just clears the HALT bit in CONTROL1.
 *        
 *        
 *  60   mpeg      1.59        5/16/02 2:48:14 PM     Dave Wilson     SCR(s) 
 *        3799 :
 *        Created local copies of the parser event status and enable register 
 *        pointers.
 *        These are used in the Canal+ audio handling code but the previous 
 *        globals have
 *        been removed from GLOBALS.H.
 *        
 *  59   mpeg      1.58        5/13/02 12:22:12 PM    Tim White       SCR(s) 
 *        3760 :
 *        DPS_ bitfields changes, remove DPS_ HSDP definitions, legacy gendmxc 
 *        globals...
 *        
 *        
 *  58   mpeg      1.57        5/6/02 4:21:36 PM      Dave Wilson     SCR(s) 
 *        3702 :
 *        Method of unmuting audio is now gated solely by the audio valid 
 *        indicator
 *        and not on PTS arrival or maturation. This prevents problems when 
 *        running
 *        audio with no PCR set and sync disabled (previously a dropout in this
 *         case
 *        could cause the audio to be muted and remain that way until sync was 
 *        reenabled).
 *        
 *  57   mpeg      1.56        5/3/02 5:51:54 PM      Dave Wilson     SCR(s) 
 *        3669 :
 *        Added signalling to tell video driver to act upon or ignore new DSR 
 *        interrupt from audio decoder informing us of a data dry-up.
 *        
 *  56   mpeg      1.55        5/2/02 4:56:32 PM      Dave Wilson     SCR(s) 
 *        3687 :
 *        
 *        cnxt_audio_get_audio_info now does not clear the info register after 
 *        reading
 *        it. Current microcode sometimes fails to rewrite the value within 
 *        200mS so we
 *        were reading occasional spurious 0s. Microcode investigation 
 *        continues but, for
 *        now, this software workaround cures the problem.
 *        
 *  55   mpeg      1.54        5/2/02 9:09:38 AM      Carroll Vance   SCR(s) 
 *        3659 :
 *        Removed old bitfield type names.  Using LPREG and HW_DWORD types 
 *        instead.
 *        
 *  54   mpeg      1.53        5/1/02 3:20:46 PM      Tim White       SCR(s) 
 *        3673 :
 *        Remove PLL_ bitfields usage from codebase.
 *        
 *        
 *  53   mpeg      1.52        4/30/02 8:50:50 PM     Carroll Vance   SCR(s) 
 *        3659 :
 *        Remove bitfields from audio
 *        
 *  52   mpeg      1.51        4/30/02 6:00:06 PM     Joe Kroesche    SCR(s) 
 *        3243 :
 *        added code to support muting and ramp down of audio when bad pes 
 *        header is detected by parser.  This is specific to a Canal+ test 
 *        pculiarity
 *        
 *  51   mpeg      1.50        4/5/02 11:59:16 AM     Tim White       SCR(s) 
 *        3510 :
 *        Backout DCS #3468
 *        
 *        
 *  50   mpeg      1.49        3/28/02 3:33:50 PM     Quillian Rutherford 
 *        SCR(s) 3468 :
 *        Bug introducted by last revision for ATHENS builds fixed.
 *        
 *        
 *  49   mpeg      1.48        3/28/02 2:58:22 PM     Quillian Rutherford 
 *        SCR(s) 3468 :
 *        Removed bit fields
 *        
 *        
 *  48   mpeg      1.47        3/14/02 3:50:58 PM     Matt Korte      SCR(s) 
 *        3375 :
 *        Added aud_comn.h, moved some things there.
 *        
 *        
 *  47   mpeg      1.46        3/12/02 4:57:00 PM     Bob Van Gulick  SCR(s) 
 *        3359 :
 *        Add multi-instance demux support
 *        
 *        
 *  46   mpeg      1.45        3/8/02 1:45:56 PM      Bob Van Gulick  SCR(s) 
 *        3335 :
 *        Remove demux warnings
 *        
 *        
 *  45   mpeg      1.44        2/27/02 4:34:48 PM     Matt Korte      SCR(s) 
 *        3262 :
 *        Added support for cnxt_audio_is_valid(), which allows the audio
 *        microcode to inform the software of the current state of the decoder.
 *        
 *        
 *  44   mpeg      1.43        2/26/02 7:20:10 PM     Tim Ross        SCR(s) 
 *        3255 :
 *        Updated Wabash microcode to latest version and fixed download source 
 *        to 
 *        reference correct data structure labels.
 *        
 *  43   mpeg      1.42        2/14/02 1:50:22 PM     Tim White       SCR(s) 
 *        3183 :
 *        CRYSTAL_FREQUENCY has to be cast to a double for the calculation.
 *        
 *        
 *  42   mpeg      1.41        2/11/02 3:57:08 PM     Miles Bintz     SCR(s) 
 *        3172 :
 *        Defined double tempmath at top of gen_audio_init
 *        
 *        
 *  41   mpeg      1.40        2/8/02 11:22:40 AM     Miles Bintz     SCR(s) 
 *        3153 :
 *        removed hardcode values of 14318180 and substituded CRYSTAL_FREQUENCY
 *         as defined in 
 *        config file or hwconfig.
 *        
 *        
 *  40   mpeg      1.39        2/6/02 4:21:18 PM      Bob Van Gulick  SCR(s) 
 *        3143 :
 *        Conditionally compile for DEMUX or GENDMXC
 *        
 *        
 *  39   mpeg      1.38        1/31/02 9:04:48 AM     Tim White       SCR(s) 
 *        3106 :
 *        Disable AVSync for Wabash for the time being.
 *        
 *        
 *  38   mpeg      1.37        1/30/02 11:04:20 AM    Tim White       SCR(s) 
 *        3097 :
 *        Dropped fix for Dave Wilson, see defect #3097 for more information.
 *        
 *        
 *  37   mpeg      1.36        1/22/02 11:17:38 AM    Matt Korte      SCR(s) 
 *        3073 2981 :
 *        Disable the AB_FULL interrupt during AC3 pass-thru operation.
 *        
 *        
 *  36   mpeg      1.35        12/19/01 2:28:10 PM    Bob Van Gulick  SCR(s) 
 *        2977 :
 *        Backout multi-instance demux code
 *        
 *        
 *  35   mpeg      1.34        12/18/01 5:24:08 PM    Quillian Rutherford 
 *        SCR(s) 2933 :
 *        Merged wabash code
 *        
 *        
 *  34   mpeg      1.33        12/17/01 5:44:32 PM    Bob Van Gulick  SCR(s) 
 *        2977 :
 *        Make use of the multi-instance demux driver
 *        
 *        
 *  33   mpeg      1.32        12/6/01 2:54:56 PM     Matt Korte      SCR(s) 
 *        2949 :
 *        Added some comments and type cast accesses to register 
 *        DPS_CAPABILITIES.
 *        
 *        
 *  32   mpeg      1.31        12/5/01 5:08:40 PM     Matt Korte      SCR(s) 
 *        2949 :
 *        Added routine cnxt_ac3_passthru() which will turn AC3
 *        pass-thru either on or off based on the parameter passed in.
 *        
 *        
 *  31   mpeg      1.30        10/16/01 3:47:32 PM    Matt Korte      SCR(s) 
 *        2787 2369 :
 *        Tip version of 1.26.1.0 that uses the .c microcode instead
 *        of the .s version. See note for that fix.
 *        
 *        
 *  30   mpeg      1.29        9/24/01 4:06:00 PM     Quillian Rutherford 
 *        SCR(s) 2649 :
 *        Minor fix to make sure the right ucode is loaded for hondo
 *        
 *        
 *  29   mpeg      1.28        9/13/01 3:15:56 PM     Quillian Rutherford 
 *        SCR(s) 2633 2634 2635 2636 2637 :
 *        Changed to use the C structure for ucode instead of an assembly 
 *        structure;
 *        
 *        
 *  28   mpeg      1.27        8/8/01 2:49:12 PM      Matt Korte      SCR(s) 
 *        2438 :
 *        DCS #2438
 *        Initial put of Generic PCM driver. Does not do mixing yet. Does
 *        have API interface, interrupt handlers and task. It does use
 *        the interrupt handlers for the PCM data instead of hardware.
 *        
 *  27   mpeg      1.26        6/27/01 11:00:02 AM    Matt Korte      SCR(s) 
 *        2163 :
 *        Replace genDemuxReInitHW with audio read/write pointer resets
 *        like it was before.
 *        
 *        
 *  26   mpeg      1.25        6/27/01 9:22:16 AM     Matt Korte      SCR(s) 
 *        2163 :
 *        The audio microcode download no longer needs a ResetVCore.
 *        
 *        
 *  25   mpeg      1.24        5/9/01 6:16:46 PM      Matt Korte      DCS 
 *        #1874, #1875
 *        Some channels were losing audio in the EPG build at the
 *        channel wrap or where there were errors in the stream.
 *        The clock recovery code was getting messed up, plus the
 *        audio buffer was overflowing. This code will clear the
 *        audio encode buffer when the overflow happens by setting
 *        the read and write pointers to the beginning. This will
 *        allow some processing time before the buffer overflows
 *        again, in order to remedy the condition causing the overflow.
 *        Also, if the Audio sync is on, it will be toggled, so
 *        that the HW will reset the clock recovery values and
 *        start fresh.
 *        
 *  24   mpeg      1.23        5/4/01 3:03:06 PM      Tim White       DCS#1822,
 *         DCS#1824, DCS31825 -- Critical Section Overhaul
 *        
 *  23   mpeg      1.22        4/18/01 6:47:16 PM     Amy Pratt       DCS1642 
 *        Removed unnecessary references to obsolete fields AudioMode,
 *        HaltDecode, and KaraokeMode1.
 *        
 *  22   mpeg      1.21        4/17/01 4:17:46 PM     Matt Korte      DCS #1724
 *        When processing slides and apps, audio_stop and audio_play
 *        get called, which conditionally disable/enable the audio PID.
 *        They can also cause an audio buffer full interrupt. This
 *        had also been conditionally disabling/enabling audio PID,
 *        and the two (interrupt level and non-interrupt level) were
 *        confusing each other, such that sometimes the audio PID was
 *        left disabled when it should have been enabled. We rarely
 *        get an audio buffer full interrupt, and this condition is the
 *        most likely cause of it. There is no real benefit or necessity
 *        to clearing and resetting all the audio for this condition, so
 *        this will not be done anymore. However, some audio data has
 *        been lost, and since that may have to do with clock recovery,
 *        the audio buffer full interrupt will now force the clock
 *        recovery to act as if a discontinuity was received in order
 *        to force the reloading of the STC.
 *        
 *  21   mpeg      1.20        4/6/01 5:02:48 PM      Amy Pratt       DCS914 
 *        Removed old neches code and references to CHIP_NAME
 *        
 *  20   mpeg      1.19        4/2/01 7:55:00 PM      Steve Glennon   DCS# 
 *        1603/1604
 *        Fixed aud_before_reset and aud_after_reset and audio_mpeg_isr to 
 *        only reenable the audio if it was enabled when the disable occurred.
 *        Made use of global gbRestoreAudioAfterReset
 *        Was unconditionally enabling audio after a channel change when there 
 *        was no
 *        valid audio PID in the new program.
 *        
 *  19   mpeg      1.18        3/3/01 6:16:46 PM      Matt Korte      DCS 
 *        #1367, 1368.
 *        Mpeg Audio Microcode version is now 1.11.
 *        Sync check now adds one frame to STC prior to Repeat check to 
 *        enlarge Play window by one frame time. Allows for proper (clean 
 *        though invalid) sync operation of BSkyB PcrAudv10.trp
 *        Added debug code in support of finding this type of problem
 *        more readily in the future.
 *        
 *  18   mpeg      1.17        2/27/01 11:56:10 AM    Tim Ross        DCS966. 
 *        Had to change audio clock divider values for emulating hondo.
 *        
 *  17   mpeg      1.16        2/26/01 2:23:12 PM     Steve Glennon   Fix for 
 *        DCS#s 1312, 1313, 1314, 1315 - setting audio deemphasis based on
 *        customer, and fixing "tinny" sounding audio for BSkyB. New defaults 
 *        are
 *        in vendor_blah.h and conexant.h - defaults are now to give active low
 *         out
 *        on deepmhasis pin, and ignore stream. Previous behavior was deemph 
 *        pin was
 *        set to a GPIO input by mistake.
 *        
 *  16   mpeg      1.15        1/29/01 9:46:32 AM     Matt Korte      DCS# 
 *        1066, ignore AB_FULL interrupts if bit 11 of Control1 set
 *        
 *  15   mpeg      1.14        12/8/00 12:02:34 PM    Matt Korte      Moved 
 *        cnxt_download_ucode here from avutil.c
 *        
 *  14   mpeg      1.13        11/14/00 8:51:50 AM    Matt Korte      Moved 
 *        AudFrameStart to before arm_audio_gen_init so that the
 *        determination of left vs right channel is done correctly.
 *        
 *  13   mpeg      1.12        8/30/00 3:48:48 PM     Chris Chapman   Changed 
 *        "\N" to "\n".
 *        
 *  12   mpeg      1.11        8/30/00 12:51:46 PM    Chris Chapman   Changed 
 *        default case of configure_PLL() to assume 48kHZ.
 *        This clears up a warning message in the build process.
 *        
 *  11   mpeg      1.10        8/29/00 7:06:30 PM     Amy Pratt       Load new 
 *        STC at audio buffer overflow in attempt to prevent further overflows
 *        
 *  10   mpeg      1.9         7/11/00 7:56:02 PM     Amy Pratt       
 *        Initialize audio output format for VENDOR_A.
 *        
 *  9    mpeg      1.8         6/20/00 5:33:46 PM     Amy Pratt       must 
 *        clear MPG_NEW_PTS_RECEIVED interrupt in ISR
 *        
 *  8    mpeg      1.7         6/15/00 8:39:32 PM     Amy Pratt       Added ISR
 *         callbacks for PTS interrupts.
 *        
 *  7    mpeg      1.6         6/6/00 1:30:26 PM      Amy Pratt       Removed 
 *        all C++ style comments
 *        Corrected header information
 *        
 *  6    mpeg      1.5         6/1/00 2:49:32 PM      Amy Pratt       Added 
 *        registration function for MPEG and PCM volume callbacks.
 *        
 *  5    mpeg      1.4         5/31/00 4:41:28 PM     Amy Pratt       Added 
 *        callback and callback registration function for Audio sample rate
 *        changes.  This is needed by the PCM driver.
 *        
 *  4    mpeg      1.3         5/30/00 1:56:22 PM     Amy Pratt       Removed 
 *        SyncOn from hw_set_aud_sync().  This was causing OTV
 *        audio-from-memory to fail.
 *        
 *  3    mpeg      1.2         5/26/00 6:56:26 PM     Amy Pratt       Added 
 *        gen_audio_play and gen_audio_stop.
 *        Also make changes to link without old audio driver,
 *        removed #ifdef DEBUG from around trace calls.
 *        
 *  2    mpeg      1.1         5/25/00 3:09:34 PM     Amy Pratt       
 *        current_volume must be shared with OTV audio driver.
 *        
 *  1    mpeg      1.0         5/24/00 9:53:48 PM     Amy Pratt       
 * $
 * 
 ****************************************************************************/

