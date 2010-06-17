/****************************************************************************/
/*                 Conexant Systems, Inc. - COLORADO                        */
/****************************************************************************/
/*                                                                          */
/* Filename:           PCM.H                                                */
/*                                                                          */
/* Description:        Public header file for colorado pcm driver           */
/*                                                                          */
/* Author:             Matthew W. Korte                                     */
/*                                                                          */
/* Copyright Conexant Systems, Inc., 2001                                   */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/
/*
$Header: pcm.h, 6, 2/13/03 12:27:04 PM, Matt Korte$
$Log: 
 6    mpeg      1.5         2/13/03 12:27:04 PM    Matt Korte      SCR(s) 5479 
       :
       Removed old header reference
       
 5    mpeg      1.4         1/10/03 5:46:40 PM     Senthil Veluswamy SCR(s) 
       5230 :
       Added support for General Event Notification & Data Feed/Stop Events.
       
 4    mpeg      1.3         9/3/02 7:41:42 PM      Matt Korte      SCR(s) 4498 
       :
       Remove warnings
       
 3    mpeg      1.2         5/15/02 3:20:24 AM     Steve Glennon   SCR(s): 2438
        
       Significant changes to the API and structures to accommodate an easier 
       design
       
       
 2    mpeg      1.1         4/25/02 5:54:24 PM     Matt Korte      SCR(s) 2438 
       :
       Added WAIT_FOR_BUF state.
       
       
 1    mpeg      1.0         8/8/01 2:50:12 PM      Matt Korte      
$
 * 
 *    Rev 1.5   13 Feb 2003 12:27:04   kortemw
 * SCR(s) 5479 :
 * Removed old header reference
 * 
 *    Rev 1.4   10 Jan 2003 17:46:40   velusws
 * SCR(s) 5230 :
 * Added support for General Event Notification & Data Feed/Stop Events.
 * 
 *    Rev 1.3   03 Sep 2002 18:41:42   kortemw
 * SCR(s) 4498 :
 * Remove warnings
 * 
 *    Rev 1.2   15 May 2002 02:20:24   glennon
 * SCR(s): 2438 
 * Significant changes to the API and structures to accommodate an easier design
 * 
 * 
 *    Rev 1.1   25 Apr 2002 16:54:24   kortemw
 * SCR(s) 2438 :
 * Added WAIT_FOR_BUF state.
 * 
 * 
 *    Rev 1.0   08 Aug 2001 13:50:12   kortemw
 * SCR(s) 2438 :
 * DCS #2438
 * Initial put of Generic PCM driver. Does not do mixing yet. Does
 * have API interface, interrupt handlers and task. It does use
 * the interrupt handlers for the PCM data instead of hardware.
*/

#ifndef _PCM_H_
#define _PCM_H_

/* pcm defines START */
#define CNXT_PCM_MAX_VOLUME         (31)
/* pcm defines END */

/* pcm type definitions START */
typedef enum
{
    NULL_STATE = 0,         // Unintialized state
    IDLE,                   // Not processing, buffers released, doing MPEG
    PLAY,                   // Doing SRC until we meet threshold.
    PLAYING,                // Have enough SRC data, do mix of PCM with MPEG
    PAUSED,                 // Temp. stop of playing, still has buffers.
    STOPPING,               // Stop has been signalled, not idle yet
    ERROR                   // Error state.
} cnxt_pcm_state_t;

typedef enum
{
    SAMPLE_RATE_8K = 0,     // 8KHz
    SAMPLE_RATE_11K,        // 11.025KHz
    SAMPLE_RATE_12K,        // 12KHz
    SAMPLE_RATE_16K,        // 16KHz
    SAMPLE_RATE_22K,        // 22.05KHz
    SAMPLE_RATE_24K,        // 24KHz
    SAMPLE_RATE_32K,        // 32KHz
    SAMPLE_RATE_44K,        // 44.1KHz
    SAMPLE_RATE_48K,        // 48KHz
    SAMPLE_RATE_INVALID     // Illegal Sample Rate
} cnxt_pcm_sample_rate_t;

typedef enum
{
    BITS_PER_SAMPLE_8 = 0,  // 8 bits per sample
    BITS_PER_SAMPLE_12,     // 12 bits per sample
    BITS_PER_SAMPLE_16,     // 16 bits per sample
    BITS_PER_SAMPLE_20,     // 20 bits per sample
    BITS_PER_SAMPLE_24,     // 24 bits per sample
    BITS_PER_SAMPLE_INVALID // Illegal bits per sample
} cnxt_pcm_bits_t;

typedef enum
{
    MPEG_LEVEL_MUTE = 0,    // Mute the Mpeg audio before mixing
    MPEG_LEVEL_HALF,        // Attenuate in half the Mpeg audio before mixing
    MPEG_LEVEL_QTR,         // Attenuate 1/4 the Mpeg audio before mixing
    MPEG_LEVEL_INVALID      // Illegal Mpeg attenuation level
} cnxt_pcm_mpeg_mix_level_t;

typedef enum
{
    PCM_CM_STEREO = 0,      // Play Stereo channels as-is
    PCM_CM_STEREO_SWAP,     // Swap left and right stereo channels
    PCM_CM_MONO_MIX,        // Mix left and right channels and output on both
    PCM_CM_MONO_LEFT,       // Output left on left and right
    PCM_CM_MONO_RIGHT,      // Output right on left and right
    PCM_CM_INVALID          // Illegal channel mode
} cnxt_pcm_channel_mode_t;

typedef enum
{
    PCM_ERROR_OK = 0,       // No error. Normal processing.
    PCM_ERROR_STATUS,       // Problem getting status.
    PCM_ERROR_VOLUME,       // Volume value out of range.
    PCM_ERROR_BUSY,         // Attempting to start when not in the idle state.
    PCM_ERROR_NODATA,       // Data Request function not initialized.
    PCM_ERROR_NORELEASE,    // Buffer Release function not initialized.
    PCM_ERROR_BAD_FORMAT,   // Unknown or unsupported audio format.
    PCM_ERROR_DATABUF,      // Error occurred when releasing the buffer.
    PCM_ERROR_STATE,        // Attempting to pause when not in PLAY state.
    PCM_ERROR_NOROOM,       // No room for this buffer.
    PCM_ERROR_BAD_BUFFER,   // Buffer or length is not right.
    PCM_ERROR_INVALID       // Invalid parameter passed
} PCM_STATUS;

/* Callback function to request more PCM data */
typedef PCM_STATUS (*cnxt_pcm_data_req_fcn_t)(void);

/* pcm clip playing events */
typedef u_int32 PCM_EVENT;
#define EV_PCM_EMPTY 1
#define EV_PCM_FEED  2

/* Callback function for event notification. Can be used for multiple event, 
   including asking for more PCM data (as above) */
typedef PCM_STATUS (*cnxt_pcm_callback_t)(PCM_EVENT event, 
                                           void *pevdata, 
                                           void *pinfo );

typedef struct _cnxt_pcm_format_t
{
    bool                    b_signed;           // Signed or unsigned audio?
    cnxt_pcm_sample_rate_t  e_rate;             // sample rate of the audio.
    bool                    b_stereo;           // Stereo or mono?
    cnxt_pcm_bits_t         e_bps;              // Bits per sample
    cnxt_pcm_mpeg_mix_level_t   e_mpeg_level;   // How mix the mpeg with this?
} cnxt_pcm_format_t;

typedef struct _cnxt_pcm_status_t
{
    cnxt_pcm_state_t        e_state;            // State of the PCM driver.
    u_int8                  ui8_pcm_vol_left;   // PCM Vol for lft chan (0-31)
    u_int8                  ui8_pcm_vol_right;  // PCM Vol for rht chan (0-31)
    u_int8                  ui8_mpeg_vol_left;  // MPEG Vol for lft chan (0-31)
    u_int8                  ui8_mpeg_vol_right; // MPEG Vol for rht chan (0-31)
    cnxt_pcm_channel_mode_t echannel_mode;      // MPEG channel mode
    cnxt_pcm_format_t       s_format;           // Format for audio data.
    u_int32                 ui32_play_threshold;// Samples to convert before starting
    u_int32                 ui32_request_threshold; // Number of sample spaces available 
                                                    // before requesting more data
    cnxt_pcm_data_req_fcn_t pfn_data_request;   // Ptr to fcn for req. buffer
    cnxt_pcm_callback_t     pfn_event_callback; /* Ptr to fn for Event Callback */
    void                    *p_event_info;      /* Ptr to User info. Can be used */
                                                /* to pass instance data, etc.   */
    PCM_STATUS              e_last_error;       // Last error encountered
} cnxt_pcm_status_t;
/* pcm type definitions END */

/* pcm function prototypes START */
PCM_STATUS cnxt_pcm_get_status(            cnxt_pcm_status_t *ps_status );
PCM_STATUS cnxt_pcm_set_volume(            u_int8 ui8_volume_percent_left, 
                                           u_int8 ui8_volume_percent_right );
PCM_STATUS cnxt_pcm_get_volume(            u_int8 *pui8_volume_percent_left, 
                                           u_int8 *pui8_volume_percent_right );
PCM_STATUS cnxt_pcm_set_volume_mpeg(       u_int8 ui8_volume_percent_left, 
                                           u_int8 ui8_volume_percent_right );
PCM_STATUS cnxt_pcm_get_volume_mpeg(       u_int8 *pui8_volume_percent_left, 
                                           u_int8 *pui8_volume_percent_right );
PCM_STATUS cnxt_pcm_set_channel_mode_mpeg( cnxt_pcm_channel_mode_t mode );
PCM_STATUS cnxt_pcm_get_channel_mode_mpeg( cnxt_pcm_channel_mode_t *pmode );
PCM_STATUS cnxt_pcm_start(                 cnxt_pcm_format_t *ps_format );
PCM_STATUS cnxt_pcm_stop( void );
PCM_STATUS cnxt_pcm_pause( void );
PCM_STATUS cnxt_pcm_resume( void );
PCM_STATUS cnxt_pcm_set_state(cnxt_pcm_state_t state);
PCM_STATUS cnxt_pcm_set_thresholds(        u_int32 ui32_play_threshold, 
                                           u_int32 ui32_request_threshold);
PCM_STATUS cnxt_pcm_data_req_fcn_register( cnxt_pcm_data_req_fcn_t pfn_data_req );
PCM_STATUS cnxt_pcm_register_callback(cnxt_pcm_callback_t pfn_callback,
                                      void *pinfo);
PCM_STATUS cnxt_pcm_write_data( void *p_data_buf, u_int32 ui32_length, u_int32 *pui32_consumed);
/* pcm function prototypes END */

/* pcm extern START */
/* pcm extern END */

#endif // !defined( _PCM_H_)


// Bias and endian considerations on the format?

