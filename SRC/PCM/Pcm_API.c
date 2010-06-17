/****************************************************************************/
/*                 Conexant Systems, Inc. - COLORADO                        */
/****************************************************************************/
/*                                                                          */
/* Filename:           PCM_API.C                                            */
/*                                                                          */
/* Description:        API source file for colorado pcm driver              */
/*                                                                          */
/* Author:             Matthew W. Korte                                     */
/*                                                                          */
/* Copyright Conexant Systems, Inc., 2001                                   */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/
/*
$Header: Pcm_API.c, 7, 4/21/03 8:07:22 PM, $
$Log: 
 7    mpeg      1.6         4/21/03 8:07:22 PM     Senthil Veluswamy SCR(s) 
       6066 6065 :
       Modifications to run time disable the mixing code in PCM if the MPEG 
       level is set to MUTE.
       
 6    mpeg      1.5         2/13/03 12:24:56 PM    Matt Korte      SCR(s) 5479 
       :
       Removed old header reference
       
 5    mpeg      1.4         1/10/03 5:45:18 PM     Senthil Veluswamy SCR(s) 
       5230 :
       Added support for General Event Notification & Data Feed/Stop Events.
       
 4    mpeg      1.3         6/19/02 5:54:34 PM     Steve Glennon   SCR(s): 4066
        
       Many fixes to original code which did not work correctly at all. 
       Enqueued
       and dequeued data from different queues etc.
       
 3    mpeg      1.2         5/15/02 3:28:08 AM     Steve Glennon   SCR(s): 2438
        
       Major work to implement full functions required of driver
       
       
 2    mpeg      1.1         4/25/02 5:57:46 PM     Matt Korte      SCR(s) 2438 
       :
       Added more support. Runs for no mixing. The file PcmMix.c
       needs to be modified to handle getting, mixing and sending
       the data buffers to/from the IRQ handlers.
       
       
 1    mpeg      1.0         8/8/01 2:47:28 PM      Matt Korte      
$
 * 
 *    Rev 1.6   21 Apr 2003 19:07:22   velusws
 * SCR(s) 6066 6065 :
 * Modifications to run time disable the mixing code in PCM if the MPEG level is set to MUTE.
 * 
 *    Rev 1.5   13 Feb 2003 12:24:56   kortemw
 * SCR(s) 5479 :
 * Removed old header reference
 * 
 *    Rev 1.4   10 Jan 2003 17:45:18   velusws
 * SCR(s) 5230 :
 * Added support for General Event Notification & Data Feed/Stop Events.
 * 
 *    Rev 1.3   Jun 19 2002 17:54:34   glennon
 * SCR(s): 4066 
 * Many fixes to original code which did not work correctly at all. Enqueued
 * and dequeued data from different queues etc.
 * 
 *    Rev 1.2   15 May 2002 02:28:08   glennon
 * SCR(s): 2438 
 * Major work to implement full functions required of driver
 * 
 * 
 *    Rev 1.1   25 Apr 2002 16:57:46   kortemw
 * SCR(s) 2438 :
 * Added more support. Runs for no mixing. The file PcmMix.c
 * needs to be modified to handle getting, mixing and sending
 * the data buffers to/from the IRQ handlers.
 * 
 * 
 *    Rev 1.0   08 Aug 2001 13:47:28   kortemw
 * SCR(s) 2438 :
 * DCS #2438
 * Initial put of Generic PCM driver. Does not do mixing yet. Does
 * have API interface, interrupt handlers and task. It does use
 * the interrupt handlers for the PCM data instead of hardware.
*/

/** include files **/
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

int32 cnxt_pcm_attenuation_shift[] = 
{   //   0   1   2   3   4   5   6   7   8   9
        20, 10, 12, 14,  9, 11, 13,  8, 10, 12, // 0
         7,  9, 11,  6,  8, 10,  5,  7,  9,  4, // 10
         6,  8,  3,  5,  7,  2,  4,  6,  1,  3, // 20
         5,  0                                  // 30
};
int32 cnxt_pcm_attenuation_multiply[] = 
{   //   0   1   2   3   4   5   6   7   8   9
         1,  1,  5, 25,  1,  5, 25,  1,  5, 25, // 0
         1,  5, 25,  1,  5, 25,  1,  5, 25,  1, // 10
         5, 25,  1,  5, 25,  1,  5, 25,  1,  5, // 20
        25,  1                                  // 30

};

/********************************************************************/
/* FUNCTION:    cnxt_pcm_get_status                                 */
/*                                                                  */
/* PARAMETERS:  ps_status   Pointer to structure where status will  */
/*                          be stored                               */
/*                                                                  */
/* DESCRIPTION: Returns the current PCM driver status.              */
/*                                                                  */
/* RETURNS:     PCM_ERROR_OK       No error                         */
/********************************************************************/
PCM_STATUS cnxt_pcm_get_status ( cnxt_pcm_status_t *ps_status)
{
    bool kstate;                        // State of interrupts
    /* Give the user a copy of the status structure */
    kstate = critical_section_begin();  // Turn off interrupts
    *ps_status = s_cnxt_pcm_status;
    critical_section_end(kstate);       // Turn on interrupts if they were on
    return( PCM_ERROR_OK);
}   // end cnxt_pcm_status()

/********************************************************************/
/* FUNCTION:    cnxt_pcm_set_volume                                 */
/*                                                                  */
/* PARAMETERS:  ui8_volume_left     PCM volume for left             */
/*              ui8_volume_right    PCM volume for right            */
/*                                                                  */
/* DESCRIPTION: Sets the PCM volume.                                */
/*                                                                  */
/* RETURNS:     PCM_ERROR_OK       No error                         */
/*              PCM_ERROR_VOLUME   parameter out of range           */
/********************************************************************/
PCM_STATUS cnxt_pcm_set_volume (    u_int8 ui8_volume_left, 
                                    u_int8 ui8_volume_right)
{
    PCM_STATUS ret_val = PCM_ERROR_OK;
    bool       kstate;                        // State of interrupts
    
    /* Make sure the values are valid */
    kstate = critical_section_begin();  // Turn off interrupts
    if (ui8_volume_left > CNXT_PCM_MAX_VOLUME) {
        ui8_volume_left = CNXT_PCM_MAX_VOLUME;
        ret_val = s_cnxt_pcm_status.e_last_error = PCM_ERROR_VOLUME;
    }
    if (ui8_volume_right > CNXT_PCM_MAX_VOLUME) {
        ui8_volume_right = CNXT_PCM_MAX_VOLUME;
        ret_val = s_cnxt_pcm_status.e_last_error = PCM_ERROR_VOLUME;
    }
    /* Set the volume values in the status structure. */
    s_cnxt_pcm_status.ui8_pcm_vol_left  = ui8_volume_left;
    s_cnxt_pcm_status.ui8_pcm_vol_right = ui8_volume_right;

    critical_section_end(kstate);       // Turn on interrupts if they were on

    return( ret_val);
}   // end cnxt_pcm_set_volume()

/********************************************************************/
/* FUNCTION:    cnxt_pcm_get_volume                                 */
/*                                                                  */
/* PARAMETERS:  pui8_volume_left    pointer for PCM volume for left */
/*              pui8_volume_right   pointer for PCM volume for right*/
/*                                                                  */
/* DESCRIPTION: returns the PCM volume.                             */
/*                                                                  */
/* RETURNS:     PCM_ERROR_OK       No error                         */
/********************************************************************/
PCM_STATUS cnxt_pcm_get_volume (    u_int8 *pui8_volume_left, 
                                    u_int8 *pui8_volume_right)
{
    /* Get the volume data from the status structure */
    *pui8_volume_left   = s_cnxt_pcm_status.ui8_pcm_vol_left;
    *pui8_volume_right  = s_cnxt_pcm_status.ui8_pcm_vol_right;
    return( PCM_ERROR_OK);
}   // end cnxt_pcm_get_volume()

/********************************************************************/
/* FUNCTION:    cnxt_pcm_set_volume_mpeg                            */
/*                                                                  */
/* PARAMETERS:  ui8_volume_left     PCM volume for left             */
/*              ui8_volume_right    PCM volume for right            */
/*                                                                  */
/* DESCRIPTION: Sets the PCM volume.                                */
/*                                                                  */
/* RETURNS:     PCM_ERROR_OK       No error                         */
/*              PCM_ERROR_VOLUME   parameter out of range           */
/********************************************************************/
PCM_STATUS cnxt_pcm_set_volume_mpeg (   u_int8 ui8_volume_left, 
                                        u_int8 ui8_volume_right)
{
    PCM_STATUS ret_val = PCM_ERROR_OK;
    bool kstate;                        // State of interrupts
    /* Make sure the values are valid */
    kstate = critical_section_begin();  // Turn off interrupts
    if (ui8_volume_left > CNXT_PCM_MAX_VOLUME) {
        ui8_volume_left = CNXT_PCM_MAX_VOLUME;
        ret_val = s_cnxt_pcm_status.e_last_error = PCM_ERROR_VOLUME;
    }
    if (ui8_volume_right > CNXT_PCM_MAX_VOLUME) {
        ui8_volume_right = CNXT_PCM_MAX_VOLUME;
        ret_val = s_cnxt_pcm_status.e_last_error = PCM_ERROR_VOLUME;
    }
    /* Set the volume values in the status structure. */
    s_cnxt_pcm_status.ui8_mpeg_vol_left     = ui8_volume_left;
    s_cnxt_pcm_status.ui8_mpeg_vol_right    = ui8_volume_right;
    // For each channel, get a multiplication factor and a shift value
    // that will result in the proper attenuation to step the volume
    // down by 2dB each step.
    MPEGLeftAttenShift  = cnxt_pcm_attenuation_shift[ui8_volume_left];
    MPEGRightAttenShift = cnxt_pcm_attenuation_shift[ui8_volume_right];
    MPEGLeftAttenMulti  = cnxt_pcm_attenuation_multiply[ui8_volume_left];
    MPEGRightAttenMulti = cnxt_pcm_attenuation_multiply[ui8_volume_right];
    critical_section_end(kstate);       // Turn on interrupts if they were on
    return( ret_val);
}   // end cnxt_pcm_set_volume_mpeg()

/********************************************************************/
/* FUNCTION:    cnxt_pcm_get_volume_mpeg                            */
/*                                                                  */
/* PARAMETERS:  pui8_volume_left    pointer for PCM volume for left */
/*              pui8_volume_right   pointer for PCM volume for right*/
/*                                                                  */
/* DESCRIPTION: returns the PCM volume.                             */
/*                                                                  */
/* RETURNS:     PCM_ERROR_OK       No error                         */
/********************************************************************/
PCM_STATUS cnxt_pcm_get_volume_mpeg (   u_int8 *pui8_volume_left, 
                                        u_int8 *pui8_volume_right)
{
    /* Get the volume data from the status structure */
    *pui8_volume_left   = s_cnxt_pcm_status.ui8_mpeg_vol_left;
    *pui8_volume_right  = s_cnxt_pcm_status.ui8_mpeg_vol_right;
    return( PCM_ERROR_OK);
}   // end cnxt_pcm_get_volume_mpeg()

/********************************************************************/
/* FUNCTION:    SetMPEGVolume                                       */
/*                                                                  */
/* PARAMETERS:  left        Mpeg volume for left channel            */
/*              right       Mpeg volume for right channel           */
/*                                                                  */
/* DESCRIPTION: Sets the volume for the Mpeg audio before it is     */
/*              mixed with the generic PCM data.                    */
/*              Note that values of 0-31 are supported, where 0     */
/*              is mute. Each step is ~-2dB.                        */
/*                                                                  */
/* RETURNS:     TRUE if done correctly.                             */
/********************************************************************/
bool SetMPEGVolume(unsigned char left, unsigned char right)
{
    bool ret_val = TRUE;
    // Ensure that value in range of 0-31
    if (left > CNXT_PCM_MAX_VOLUME)  left    = CNXT_PCM_MAX_VOLUME;
    if (right > CNXT_PCM_MAX_VOLUME) right   = CNXT_PCM_MAX_VOLUME;

    if (cnxt_pcm_set_volume_mpeg(left, right) != PCM_ERROR_OK) {
        ret_val = FALSE;
    }
#ifdef DEBUG
   trace_new(TRACE_LEVEL_2 | TRACE_AUD, 
    "SetMPEGVolume: left=%d, right=%d, lsh=%d, rsh=%d lmult=%d rmult=%d\n", 
    left, right, MPEGLeftAttenShift, MPEGRightAttenShift, MPEGLeftAttenMulti, MPEGRightAttenMulti);
#endif //def DEBUG

   return ret_val;
}

/********************************************************************/
/* FUNCTION:    cnxt_pcm_set_channel_mode_mpeg                      */
/*                                                                  */
/* PARAMETERS:  mode  New left/right channel mode to apply          */
/*                                                                  */
/* DESCRIPTION: Set the MPEG left/right channel mix mode            */
/*                                                                  */
/* RETURNS:     PCM_ERROR_OK       No error                         */
/*              PCM_ERROR_INVALID  parameter out of range           */
/********************************************************************/
PCM_STATUS cnxt_pcm_set_channel_mode_mpeg ( cnxt_pcm_channel_mode_t mode)
{
    PCM_STATUS ret_val = PCM_ERROR_OK;
    bool kstate;                        // State of interrupts
    
    /* Make sure the values are valid */
    kstate = critical_section_begin();  // Turn off interrupts
    if (mode >= PCM_CM_INVALID) {
        ret_val = s_cnxt_pcm_status.e_last_error = PCM_ERROR_INVALID;
    }
    
    /* Set the status structure */
    s_cnxt_pcm_status.echannel_mode = mode;
    critical_section_end(kstate);       // Turn on interrupts if they were on

    return( ret_val);

}   // end cnxt_pcm_set_channel_mode_mpeg()

/********************************************************************/
/* FUNCTION:    cnxt_pcm_get_channel_mode_mpeg                      */
/*                                                                  */
/* PARAMETERS:  pmode  Place to return current mode                 */
/*                                                                  */
/* DESCRIPTION: Get the MPEG left/right channel mix mode            */
/*              Does not check for a null pointer.                  */
/*                                                                  */
/* RETURNS:     PCM_ERROR_OK       No error                         */
/********************************************************************/
PCM_STATUS cnxt_pcm_get_channel_mode_mpeg ( cnxt_pcm_channel_mode_t *pmode)
{
    /* return the current setting */
    *pmode = s_cnxt_pcm_status.echannel_mode;    
    return( PCM_ERROR_OK );

}   // end cnxt_pcm_get_channel_mode_mpeg()

/********************************************************************/
/* FUNCTION:    cnxt_pcm_start                                      */
/*                                                                  */
/* PARAMETERS:  ps_format   Pointer to structure containing format  */
/*                          of the audio to be played.              */
/*                                                                  */
/* DESCRIPTION: Tells the PCM Driver to start playing audio.        */
/*                                                                  */
/* RETURNS:     PCM_ERROR_OK        No error.                       */
/*              PCM_ERROR_BUSY      Not in the IDLE state.          */
/*              PCM_ERROR_NODATA    Data Req. Fcn not initialized.  */
/*              PCM_ERROR_NORELEASE Buf Rel. Fcn not initialized.   */
/*              PCM_ERROR_BAD_FORMAT    Format not correct.         */
/********************************************************************/
PCM_STATUS cnxt_pcm_start ( cnxt_pcm_format_t *ps_format)
{
    bool kstate;                        // State of interrupts
    /* We can only start playing from the IDLE state. Any other state
    *  is an error. */
    if (s_cnxt_pcm_status.e_state != IDLE) {
        return( s_cnxt_pcm_status.e_last_error = PCM_ERROR_BUSY);
    }
    /* If the data request function/callback has not been initialized,
    *  return an error. */
    if((s_cnxt_pcm_status.pfn_data_request == NULL) &&
       (s_cnxt_pcm_status.pfn_event_callback == NULL))
    {
       return( s_cnxt_pcm_status.e_last_error = PCM_ERROR_NODATA);
    }
    /* If any of the format parameters are not valid,
    *  return an error.*/
    /* NOTE: Will this catch a negative value? */
    if (    (ps_format->e_rate          >= SAMPLE_RATE_INVALID)     ||
            (ps_format->e_bps           >= BITS_PER_SAMPLE_INVALID) ||
            (ps_format->e_mpeg_level    >= MPEG_LEVEL_INVALID) )    {
        return( s_cnxt_pcm_status.e_last_error = PCM_ERROR_BAD_FORMAT);
    }

    /* Do we have MPEG ? */
    if(ps_format->e_mpeg_level == MPEG_LEVEL_MUTE)
    {
       gbPCMPlayNoMpeg = TRUE;

       hw_aud_stop(TRUE);

       /* Turn off the internal audio source */
       CNXT_SET_VAL(AUD_CLOCK_CONTROL_REG, 
                    AUD_CLOCK_CONTROL_SOURCE_SELECT_MASK, 0);

       hw_aud_start();
    }

    kstate = critical_section_begin();  // Turn off interrupts
    /* Copy the format into the status structure. */
    s_cnxt_pcm_status.s_format = *ps_format;
    /* Go to the PLAY state because there is no data yet. */
    /* When sufficient data has arrived we will transfer  */
    /* into the PLAYING state.                            */
    s_cnxt_pcm_status.e_state = PLAY;
    critical_section_end(kstate);       // Turn on interrupts if they were on
    return( PCM_ERROR_OK);
}   // end cnxt_pcm_start()

/********************************************************************/
/* FUNCTION:    cnxt_pcm_stop                                       */
/*                                                                  */
/* PARAMETERS:  NONE.                                               */
/*                                                                  */
/* DESCRIPTION: Tells the PCM Driver to stop playing audio.         */
/*                                                                  */
/* RETURNS:     PCM_ERROR_OK       No error                         */
/********************************************************************/
PCM_STATUS cnxt_pcm_stop ( void)
{
    bool kstate;                        // State of interrupts

    /* Do we have MPEG ? */
    if(gbPCMPlayNoMpeg == TRUE)
    {
       gbPCMPlayNoMpeg = FALSE;

       hw_aud_stop(TRUE);

       /* Turn off the internal audio source */
       CNXT_SET_VAL(AUD_CLOCK_CONTROL_REG, 
                    AUD_CLOCK_CONTROL_SOURCE_SELECT_MASK, 1);

       hw_aud_start();
    }

    kstate = critical_section_begin();  // Turn off interrupts
    /* Tell the driver to shut down and go to the IDLE state. */
    s_cnxt_pcm_status.e_state = STOPPING;
    critical_section_end(kstate);       // Turn on interrupts if they were on
    return( PCM_ERROR_OK);
}   // end cnxt_pcm_stop()

/********************************************************************/
/* FUNCTION:    cnxt_pcm_pause                                      */
/*                                                                  */
/* PARAMETERS:  NONE.                                               */
/*                                                                  */
/* DESCRIPTION: Tells the PCM Driver to temporarily stop            */
/*              playing audio.                                      */
/*                                                                  */
/* RETURNS:     PCM_ERROR_OK        No error                        */
/*              PCM_ERROR_STATE     Can only pause if playing.      */
/********************************************************************/
PCM_STATUS cnxt_pcm_pause ( void)
{
    bool kstate;                        // State of interrupts
    PCM_STATUS ret_val = PCM_ERROR_OK;  // Assume no error
    kstate = critical_section_begin();  // Turn off interrupts
    /* We can only pause playing from the PLAY or PLAYING state.
    *  Any other state is an error. */
    if ((s_cnxt_pcm_status.e_state != PLAY) &&
        (s_cnxt_pcm_status.e_state != PLAYING))
    {
        ret_val = s_cnxt_pcm_status.e_last_error = PCM_ERROR_STATE;
    } else {  // Must be in correct state
        /* Tell the driver to pause temporarily. */
        s_cnxt_pcm_status.e_state = PAUSED;
    }
    critical_section_end(kstate);       // Turn on interrupts if they were on
    return( ret_val);
}   // end cnxt_pcm_pause()

/********************************************************************/
/* FUNCTION:    cnxt_pcm_resume                                     */
/*                                                                  */
/* PARAMETERS:  NONE.                                               */
/*                                                                  */
/* DESCRIPTION: Tells the PCM Driver to resume playing.             */
/*                                                                  */
/* RETURNS:     PCM_ERROR_OK        No error                        */
/*              PCM_ERROR_STATE     Can only resume if paused.      */
/********************************************************************/
PCM_STATUS cnxt_pcm_resume( void )
{
    bool kstate;                        // State of interrupts
    /* We can only resume playing from the PAUSE state. Any 
    *  other state is an error. */
    if ( s_cnxt_pcm_status.e_state != PAUSED) {
        return( s_cnxt_pcm_status.e_last_error = PCM_ERROR_STATE);
    }
    kstate = critical_section_begin();  // Turn off interrupts
    /* Tell the driver to resume playing. */
    s_cnxt_pcm_status.e_state = PLAYING;
    critical_section_end(kstate);       // Turn on interrupts if they were on
    return( PCM_ERROR_OK);
}   // end cnxt_pcm_resume()


PCM_STATUS cnxt_pcm_set_state(cnxt_pcm_state_t state)
{
    bool kstate;                        // State of interrupts
    kstate = critical_section_begin();  // Turn off interrupts
    s_cnxt_pcm_status.e_state= state;
    critical_section_end(kstate);       // Turn on interrupts if they were on
    return( PCM_ERROR_OK);
}   // end cnxt_pcm_set_req_threshold()

/********************************************************************/
/* FUNCTION:    cnxt_pcm_set_req_threshold                          */
/*                                                                  */
/* PARAMETERS:  ui32_threshold  Threshold value.                    */
/*                                                                  */
/* DESCRIPTION: When playing audio, and the number of bytes         */
/*              remaining in the buffer is at this threshold or     */
/*              less, then another buffer will be requested. If     */
/*              a value of 0 is used, then one buffer will be       */
/*              completely drained before the next is requested.    */
/*              On the other hand, if this value is larger than     */
/*              the number of bytes in the buffer to begin with,    */
/*              then as soon as the driver starts processing that   */
/*              buffer, it will request another buffer, which could */
/*              be useful in making sure the data is pipelined and  */
/*              continuous.                                         */
/*                                                                  */
/* RETURNS:     PCM_ERROR_OK       No error                         */
/********************************************************************/
PCM_STATUS cnxt_pcm_set_thresholds( u_int32 ui32_play_threshold,
                                    u_int32 ui32_request_threshold )
{
    bool kstate;                        // State of interrupts
    kstate = critical_section_begin();  // Turn off interrupts
    /* Set the threshold at which a new data buffer will be
    *  requested */
    s_cnxt_pcm_status.ui32_play_threshold = ui32_play_threshold;
    s_cnxt_pcm_status.ui32_request_threshold = ui32_request_threshold;
    critical_section_end(kstate);       // Turn on interrupts if they were on
    return( PCM_ERROR_OK);
}   // end cnxt_pcm_set_req_threshold()

/********************************************************************/
/* FUNCTION:    cnxt_pcm_data_req_fcn_register                      */
/*                                                                  */
/* PARAMETERS:  pfn_data_req    Pointer to function that the PCM    */
/*                              driver is to use to request more    */
/*                              data from the user                  */
/*                                                                  */
/* DESCRIPTION: Sets up the function pointer that the driver uses.  */
/*                                                                  */
/* RETURNS:     PCM_ERROR_OK       No error                         */
/*              PCM_ERROR_INVALID  The General Callback was already */
/*                                  defined. So Data Callback set is*/
/*                                  not allowed.                    */
/********************************************************************/
PCM_STATUS cnxt_pcm_data_req_fcn_register ( cnxt_pcm_data_req_fcn_t pfn_data_req)
{
    PCM_STATUS retcode = PCM_ERROR_OK;
    bool kstate;                        /* State of interrupts */
    kstate = critical_section_begin();  /* Turn off interrupts */
    /* Set the pointer to the function the driver is to use
    *  to request more data. */
    if(s_cnxt_pcm_status.pfn_event_callback == NULL)
    {
       s_cnxt_pcm_status.pfn_data_request = pfn_data_req;
    }
    else
    {
       retcode = PCM_ERROR_INVALID;
    }

    critical_section_end(kstate);       /* Turn on interrupts if they were on */
    return(retcode);
}   /* end cnxt_pcm_data_req_fcn_register() */

/********************************************************************/
/* FUNCTION:    cnxt_pcm_register_callback                          */
/*                                                                  */
/* PARAMETERS:  pfnEventCallback Ptr to function that the PCM driver*/
/*                               is to use to notify an event       */
/*                               occourence to the user             */
/*              pEventInfo       Ptr to User info. Can be used to   */
/*                               instance data, etc                 */
/*                                                                  */
/* DESCRIPTION: Sets up a function pointer for general Event        */
/*              Callback for all Events that the driver supports.   */
/*              Repeated calls will cause the previously registered */
/*              callbacks to be overwritten                         */
/*                                                                  */
/* RETURNS:     PCM_ERROR_OK       No error                         */
/*              PCM_ERROR_INVALID  The Data request Callback was    */
/*                                  already set. So Data Callback   */
/*                                  set is not allowed.             */
/********************************************************************/
PCM_STATUS cnxt_pcm_register_callback(cnxt_pcm_callback_t pfnEventCallback,
                                      void *pEventInfo)
{
   PCM_STATUS retcode = PCM_ERROR_OK;
   bool kstate;                        /* State of interrupts */
   kstate = critical_section_begin();  /* Turn off interrupts */

   /* Has the Data request callback already been set? */
   if(s_cnxt_pcm_status.pfn_data_request != NULL)
   {
      retcode = PCM_ERROR_INVALID;
   }
   else
   {
      /* Set the Callback */
      s_cnxt_pcm_status.pfn_event_callback = pfnEventCallback;
      s_cnxt_pcm_status.p_event_info       = pEventInfo;
   }

   critical_section_end(kstate);       /* Restore Interrupt State */
   return(retcode);
}

/********************************************************************/
/* FUNCTION:    cnxt_pcm_write_data                                 */
/*                                                                  */
/* PARAMETERS:  p_data_buf  Pointer to buffer containing audio.     */
/*              ui32_length Number of bytes of data in this buffer. */
/*              ui32_consumed (out)Number of bytes of data taken    */
/*                                                                  */
/* DESCRIPTION: Passes data buffer to the PCM driver to be played.  */
/*              This function takes as much data off the input buff */
/*              as it can, and returns information to the caller as */
/*              to how much it actually took.                       */
/*                                                                  */
/*              It copies the samples, converted to 20 bit PCM,     */
/*              into the PCMInputQ circular queue.                  */
/*                                                                  */
/* RETURNS:     PCM_ERROR_OK            No error                    */
/*              PCM_ERROR_BAD_BUFFER    The buffer is not right     */
/*              PCM_ERROR_NOROOM        There is no room left,      */
/*                                      we have 2 buffers already.  */
/********************************************************************/
PCM_STATUS cnxt_pcm_write_data( void *p_data_buf, u_int32 ui32_length, u_int32 *pui32_consumed)
{
    PCM_STATUS rc = PCM_ERROR_OK;       /* Procedure return code */
    u_int8     BytesPerSample;          /* Input data format     */
    u_int32    BytesConsumed;           /* Bytes consumed from input */
    
    /* Check the buffer for correctness. */
    if ((p_data_buf == NULL) || (ui32_length == 0)) {
        return( s_cnxt_pcm_status.e_last_error = PCM_ERROR_BAD_BUFFER);
    }
    /* work out numerically the input bytes per sample */
    switch (s_cnxt_pcm_status.s_format.e_bps)
    {
       case BITS_PER_SAMPLE_8:  BytesPerSample = 1; break;
       case BITS_PER_SAMPLE_12: BytesPerSample = 2; break;
       case BITS_PER_SAMPLE_16: BytesPerSample = 2; break;
       case BITS_PER_SAMPLE_20: BytesPerSample = 3; break;
       case BITS_PER_SAMPLE_24: BytesPerSample = 3; break;
       default:
         return( s_cnxt_pcm_status.e_last_error = PCM_ERROR_BAD_FORMAT);
         break;  
    } /* endswitch */
    
    /* if successfully got bytes per sample */
    if (BytesPerSample)
    {
       /* Convert input samples to PCM20 and enqueue on a circular buffer */
       /* Returns Samples Consumed */
       BytesConsumed = PCM20_CQ_ConvertAndEnqueueBuffer(&PCMInputQ,
                                                       (u_int8 *)p_data_buf,
                                                       ui32_length,
                                                       BytesPerSample,
                                                       s_cnxt_pcm_status.s_format.b_signed);
       BytesConsumed *= BytesPerSample; 
       
       /* report the number of bytes taken by this function */                                               
       *pui32_consumed = BytesConsumed;
       
    } /* endif no error */
    return(rc);
    
} /* cnxt_pcm_pass_data_buffer() */
