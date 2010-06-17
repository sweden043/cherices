/****************************************************************************/
/*                   Conexant Proprietary and Confidential                  */
/*                       Software File/Module Header                        */
/*                    Conexant Systems Inc. (c) 1998-2004                   */
/*                               Austin, TX                                 */
/*                            All Rights Eeserved                           */
/****************************************************************************/
/*
 * Filename:       video.h
 *
 *
 * Description:    description
 *
 *
 * Author:         Mustafa Ismail (with alterations by Dave Wilson, Eric Ching, 
 *                 Steve Glennon and Quillian Rutherford)
 *
 ****************************************************************************/
/* $Id: video.h,v 1.54, 2004-05-25 16:00:12Z, Tim White$
 ****************************************************************************/

#ifndef _VIDEO_H_
#define _VIDEO_H_

typedef struct {
    void      *begin;
    int       size;           /* NOTE:It holds at most (size-1)  bytes in queue. */
    int       in;             /* Write  offset from the beginning of the buffer. */
    int       out;            /* Read offset form the beginning of the buffer. */
} gen_farcir_q;

typedef struct {
    u_int8      hour;
    u_int8      minute;
    u_int8      second;
    u_int8      frame;
} gen_time_code_t;

typedef enum _CNXT_VID_STATUS
{
 CNXT_VID_STATUS_OK = 0, 
 CNXT_VID_STATUS_DECODE_1_TIMEOUT,
 CNXT_VID_STATUS_DECODE_2_TIMEOUT,
 CNXT_VID_STATUS_SYNC_TIMEOUT, 
 CNXT_VID_STATUS_DEMUX_DISABLED,
 CNXT_VID_STATUS_NO_DATA,
 CNXT_VID_STATUS_SYS_ERROR,
 CNXT_VID_STATUS_ASYNC,
 CNXT_VID_STATUS_DRIP_INACTIVE = -1,
 CNXT_VID_STATUS_DRIP_Q_FULL   = -2,
 CNXT_VID_STATUS_NULL_LEN_REQ  = -3
} CNXT_VID_STATUS;

#if ((PVR==YES)||(XTV_SUPPORT==YES))

typedef enum _CNXT_ES_BUFFER
{
 CNXT_ES_BUFFER_VIDEO,
 CNXT_ES_BUFFER_AUDIO,
 CNXT_ES_BUFFER_AC3
} CNXT_ES_BUFFER;

typedef enum _CNXT_ES_BUFFER_EVENT
{
 CNXT_ES_BUFFER_EMPTY,
 CNXT_ES_BUFFER_FULL,
 CNXT_ES_BUFFER_LWM
} CNXT_ES_BUFFER_EVENT;

typedef void (*CNXT_ES_BUFFER_CALLBACK)(CNXT_ES_BUFFER Buffer,
                                        CNXT_ES_BUFFER_EVENT Event,
                                        void *pTag);

bool gen_video_set_es_buffer_callback(CNXT_ES_BUFFER_CALLBACK NotifyFn, void *pTag);
bool gen_video_set_es_buffer_lwm(CNXT_ES_BUFFER Buffer, u_int32 lwm);
bool gen_video_get_es_buffer_info(CNXT_ES_BUFFER Buffer, u_int32 *size, u_int32 *free, u_int32 *lwm);

typedef void (*CNXT_VIDEO_UNBLANK_CALLBACK)(void *pTag);

bool gen_video_set_unblank_callback(CNXT_VIDEO_UNBLANK_CALLBACK NotifyFn, void *pTag);
#endif /* (PVR==YES) */

typedef void (*gen_callback_cmd_done_t)( CNXT_VID_STATUS eStatus );
typedef void (*gen_callback_frame_t) ( void );
typedef void (*gen_callback_time_code_notify)(gen_time_code_t *);
typedef void (*gen_callback_audio_error_notify)(u_int32 uDSRRegValue);
typedef void (*gen_callback_user_data_notify)(u_int8 *pData, u_int32 uLen, u_int32 uType, bool bMore);
typedef bool (*gen_callback_handle_msg_func)(u_int32 *pMsg);
typedef void (*DRIP_CALLBACK)(u_int32 DripID);

#if ((PVR==YES)||(XTV_SUPPORT==YES))
#define VIDEO_ISR_BITS (MPG_ERROR_IN_STAT_REG|MPG_VB_FULL|MPG_GOP_TC_RECEIVED|MPG_VIDEO_SEQ_END|MPG_VIDEO_DEC_COMPLETE|MPG_VIDEO_DEC_INTERRUPT6|MPG_VIDEO_DEC_INTERRUPT7|MPG_VB_LOWWATERMARK)
#else

//*****Green screen patch start***************
#define VIDEO_ISR_BITS (MPG_ERROR_IN_STAT_REG|MPG_GOP_TC_RECEIVED|MPG_VIDEO_SEQ_END|MPG_VIDEO_DEC_COMPLETE|MPG_VIDEO_DEC_INTERRUPT6|MPG_VIDEO_DEC_INTERRUPT7)
//*****Green screen patch end***************

#endif /* PVR==YES */

#define NUM_FLUSH_BYTES                   384
#define VIDEO_SEM_WAIT_LENGTH      5000

/* Hold the dimensions and type of a decoded MPEG picture */
typedef struct _MPEG_buffer_image
{
  u_int32 dwWidth;
  u_int32 dwHeight;
  u_int32 dwStartingMode;
} MPEG_buffer_image;

#define DEC_I_BUFFER_INDEX 0
#define DEC_P_BUFFER_INDEX 1

/* Commands to the video task */
#define VIDEO_COMMAND_PLAY         1
#define VIDEO_COMMAND_RESET        2
#define VIDEO_COMMAND_SEND_AV_DONE 3
#define VIDEO_COMMAND_STOP         4
#define VIDEO_COMMAND_PAUSE        5
#define VIDEO_COMMAND_RESUME       6
#define VIDEO_COMMAND_USER_DATA    7
#define VIDEO_COMMAND_SET_VBUFFS   8
#define VIDEO_COMMAND_CONTINUE     9
#define VIDEO_COMMAND_STILL_WAIT   10
#define VIDEO_COMMAND_FLUSHED      11
#define VIDEO_COMMAND_BLANK        12
#define VIDEO_COMMAND_UNBLANK      13
#define VIDEO_COMMAND_FEED_DRIP    14
#define VIDEO_COMMAND_BUF_EMPTY    15
#define VIDEO_COMMAND_FLUSH_VBV    16


#define VIDEO_SEND_COMMAND_TIMEOUT 10000

#define PLAY_OFF_STATE             0
#define PLAY_LIVE_STATE            1
#define PLAY_STILL_STATE           2
#define PLAY_STILL_STATE_NO_DISP   3
#define PLAY_LIVE_STATE_NO_SYNC    4
#define PLAY_DRIP_STATE            5

#define LOCAL_VIDEO_BUFFER_SIZE (50*1024)
#define LOCAL_VIDEO_USER_BUFFER_SIZE (512)

/* Value for video errors for fatal_exit() */
#define VIDEO_VCORE_HUNG	0x80
#define VIDEO_QSEND_FAIL	0x81

/* the size of the seperate buffer in the decoder hardware */
#define DECODER_BUFFER_SIZE 128

/* Function prototypes */
void StartSendPesData(void);
void StopSendPesData(void);

bool gen_video_init(bool ReInit, gen_callback_frame_t video_FrameNotify, gen_callback_time_code_notify time_code_notify);
gen_callback_frame_t gen_video_set_frame_callback(gen_callback_frame_t pfnCallback);
void gen_video_blank(gen_callback_cmd_done_t video_command_done);
void gen_video_unblank(gen_callback_cmd_done_t video_command_done);
CNXT_VID_STATUS gen_video_play(u_int32 starting_state, 
                                  gen_callback_cmd_done_t video_command_done);
bool gen_video_stop(gen_callback_cmd_done_t video_command_done);
bool gen_video_set_still_scaling_mode(bool bUseFrameScaling);
bool gen_video_get_still_scaling_mode(bool *pbUseFrameScaling);
bool gen_video_set_startup_timeouts(u_int32 uTimeout1, u_int32 uTimeout2, u_int32 uTimeout3);
bool gen_video_get_startup_timeouts(u_int32 *pTimeout1, u_int32 *pTimeout2, u_int32 *pTimeout3);
bool gen_video_pause(gen_callback_cmd_done_t video_command_done);
bool gen_video_resume(gen_callback_cmd_done_t video_command_done);
int gen_send_still_data(gen_farcir_q *q);
bool gen_flush_still_data(gen_callback_cmd_done_t video_command_done);
void *gen_video_get_buffer(void);
void gen_video_reset(void);
int gen_video_buffer_size(void);
void gen_video_set_video_buffers(u_int8 *pIBuff, u_int8 *pPBuff, u_int8 *pBBuff);
void gen_video_StartSendPesData(void);
void gen_video_StopSendPesData(void);
void gen_video_register_user_data_callback(gen_callback_user_data_notify pfnUserDataCallback);
void gen_video_register_audio_error_callback(gen_callback_audio_error_notify pfnAudioError);
bool gen_video_get_safe_still_buffer(u_int8 **ppBuffer);
bool gen_video_get_current_display_buffer(u_int8 **ppBuffer);
bool gen_video_get_motion_decode_buffers(u_int8 **ppIBuff, u_int8 ** ppPBuff);
bool gen_video_get_still_panscan_vector(int *pPan, int *pScan, bool *pbFound);
void gen_video_decode_blank(void);
bool gen_video_is_motion_video_playing(void);
int  gen_video_drip_request(char *buptr, int buflen);
void gen_video_stop_drip(void);
void gen_video_register_drip_callbacks(DRIP_CALLBACK DBEmpty, DRIP_CALLBACK VBEmpty);

#define NUM_DSR_ERROR_CODES 32
void gen_video_get_error_stats(u_int32 *pOverflowCount, 
                               u_int32 *pUnderflowCount,
                               u_int32 pDSRErrors[]);

/* Function to mark the current display buffer as invalid in cases where         */
/* the DEFERRED_UNBLANK build option is specified. This causes gen_video_unblank */
/* to keep the video plane blanked until the next decode occurs.                 */
#ifdef DEFERRED_UNBLANK
void gen_video_invalidate_video_display_buffer(void);
#endif

/* Special variant of gen_video_pause which pauses on the next I picture. Required by Canal+ */
#if VIDEO_UCODE_TYPE == VMC_EXTRA_ERROR_RECOVERY
bool gen_video_pause_on_next_I(gen_callback_cmd_done_t video_command_done);
#endif

/* For backwards compatibility */
#define gen_flush_data gen_flush_still_data

typedef struct
{
   bool bInSync;
} CNXT_VIDEO_STATUS;

bool gen_video_get_status(CNXT_VIDEO_STATUS *VideoStatus);

void ipanel_get_vpts(u_int32 *pts_value);

#endif /* _VIDEO_H_ */

/*******************************************************************************
 * Modifications:
 * $Log: 
 *  55   mpeg      1.54        5/25/04 11:00:12 AM    Tim White       CR(s) 
 *        9296 9297 : Add video unblank callback function.
 *        
 *  54   mpeg      1.53        5/25/04 9:43:50 AM     Billy Jackman   CR(s) 
 *        9285 9286 : Reinstate XTV changes for CRs 9000 and 9001.
 *  53   mpeg      1.52        5/25/04 9:38:50 AM     Billy Jackman   CR(s) 
 *        9285 9286 : Added structure definition and function prototype for 
 *        gen_video_get_status.
 *        Momentarily remove NDS changes from CRs 9000 and 9001.
 *  52   mpeg      1.51        4/29/04 9:38:36 AM     Larry Wang      CR(s) 
 *        9000 9001 : Enable video buffer low watermark for XTV.
 *  51   mpeg      1.50        3/15/04 4:58:49 PM     Craig Dry       CR(s) 
 *        8526 : Video Drip prototypes added.
 *  50   mpeg      1.49        3/12/04 9:53:57 AM     Tim White       CR(s) 
 *        8545 : Add ES buffer event notification for PVR.
 *        
 *  49   mpeg      1.48        3/2/04 7:46:44 PM      Matt Korte      CR(s) 
 *        8492 : Replace VIDEO_STATUS with CNXT_VID_STATUS
 *  48   mpeg      1.47        12/5/03 2:46:25 PM     Xin Golden      CR(s) 
 *        8105 8106 : increased the padded 0s from 256 bytes to 384 bytes to 
 *        make sure the decoder finish decoding the last byte of the still.
 *  47   mpeg      1.46        12/4/03 10:57:43 AM    Xin Golden      CR(s): 
 *        8039 8040 Defined a macro for the size of the seperate buffer in the 
 *        decoder hardware.
 *  46   mpeg      1.45        10/28/03 3:09:33 PM    Dave Wilson     CR(s): 
 *        7709 Added prototype for new function 
 *        gen_video_is_motion_video_playing.
 *        
 *  45   mpeg      1.44        9/29/03 2:00:50 PM     Dave Wilson     SCR(s) 
 *        7552 3160 :
 *        Added prototype for gen_video_get_still_panscan_vector.
 *        
 *  44   mpeg      1.43        7/1/03 12:34:32 PM     Dave Wilson     SCR(s) 
 *        6363 :
 *        Added prototype for gen_video_set_frame_callback.
 *        
 *  43   mpeg      1.42        6/3/03 3:14:54 PM      Dave Wilson     SCR(s) 
 *        6653 :
 *        Added VIDEO_COMMAND_FLUSH_VBV
 *        
 *  42   mpeg      1.41        4/22/03 4:01:52 PM     Dave Wilson     SCR(s) 
 *        6069 :
 *        Added prototypes for gen_video_get/set_startup_timeouts.
 *        
 *  41   mpeg      1.40        4/22/03 2:04:24 PM     Dave Wilson     SCR(s) 
 *        6057 :
 *        Added prototypes for gen_video_set/get_still_scaling_mode
 *        
 *  40   mpeg      1.39        3/19/03 4:00:54 PM     Dave Wilson     SCR(s) 
 *        5135 5827 :
 *        Added prototype for new function gen_video_get_error_stats.
 *        
 *  39   mpeg      1.38        2/21/03 3:47:32 PM     Dave Wilson     SCR(s) 
 *        5580 :
 *        Added Canal+ specific function gen_video_pause_on_next_I
 *        
 *  38   mpeg      1.37        10/11/02 5:51:24 PM    Craig Dry       SCR(s) 
 *        4716 :
 *        Add asynchronous features to Video Drip API
 *        
 *  37   mpeg      1.36        10/11/02 4:14:44 PM    Dave Wilson     SCR(s) 
 *        4746 :
 *        Added prototype for gen_video_register_audio_error_callback
 *        
 *  36   mpeg      1.35        9/30/02 1:33:44 PM     Craig Dry       SCR(s) 
 *        4716 :
 *        Add Pause/Resume Capability to Video Drip Mode  
 *        
 *        
 *  35   mpeg      1.34        9/27/02 2:47:36 PM     Dave Wilson     SCR(s) 
 *        1513 :
 *        Renamed gen_flush_data to gen_flush_still_data.
 *        
 *  34   mpeg      1.33        8/15/02 1:58:24 PM     Dave Wilson     SCR(s) 
 *        4377 :
 *        Added VIDEO_STATUS_SYNC_TIMEOUT return code from 
 *        WaitForLiveVideoToStart.
 *        This indicates that, although decoding is happening, the video 
 *        decoder took
 *        longer than 2.5s to report that AV sync has been achieved
 *        
 *  33   mpeg      1.32        8/8/02 1:36:10 PM      Bob Van Gulick  SCR(s) 
 *        4350 :
 *        Add support for MHP Video Drips
 *        
 *        
 *  32   mpeg      1.31        6/12/02 3:32:52 PM     Dave Wilson     SCR(s) 
 *        3216 :
 *        Added VIDEO_STATUS enum containing return codes for gen_video_play.
 *        Modified gen_video_play prototype to include new return codes.
 *        Modified function callback type to include new status parameter.
 *        
 *  31   mpeg      1.30        6/6/02 3:17:16 PM      Dave Wilson     SCR(s) 
 *        3931 :
 *        Changed the user data callback prototype to include parameters 
 *        informing
 *        the client of the last picture type decoded and whether or not more 
 *        data
 *        will be sent back from this picture cycle.
 *        
 *  30   mpeg      1.29        2/22/02 10:27:34 AM    Dave Wilson     SCR(s) 
 *        3206 3207 3208 :
 *        Added prototype for gen_video_decode_blank API.
 *        
 *  29   mpeg      1.28        8/28/01 1:00:20 PM     Dave Wilson     SCR(s) 
 *        2542 2543 :
 *        Moved definitions required for DEFERRED_UNBLANK processing to VIDEO.H
 *         from
 *        VIDEO.C so that OSDISRC.C can access them.
 *        
 *  28   mpeg      1.27        7/12/01 4:48:16 PM     Dave Wilson     SCR(s) 
 *        2256 2255 :
 *        Removed gen_video_deferred_unblank API.
 *        
 *  27   mpeg      1.26        6/28/01 3:00:30 PM     Dave Wilson     SCR(s) 
 *        2105 2106 :
 *        New microcode correctly reports buffer which is being displayed so 
 *        reworked
 *        these files to track this properly. Now decode of P frame stills will
 *         difference
 *        with the correct buffer and they will end up looking correct.
 *        
 *  26   mpeg      1.25        6/8/01 8:33:38 AM      Dave Wilson     SCR(s) 
 *        1926 1927 :
 *        Added video commands for blank and unblank
 *        
 *  25   mpeg      1.24        4/20/01 12:13:28 PM    Dave Wilson     DCS1124: 
 *        Major changes to video memory management to get Sky Text app running
 *        
 *  24   mpeg      1.23        4/11/01 1:53:44 PM     Steve Glennon   DCS 1675 
 *        (put by DW): Added new video commands to allow some new operations
 *        to be passed to the video task rather than being handled on a 
 *        different 
 *        context.
 *        
 *  23   mpeg      1.22        3/15/01 2:23:02 PM     Lucy C Allevato Added 
 *        VIDEO_SEM_WAIT_LENGTH for use by sem_get() in video.c when getting a
 *        semaphore from the local command done function.
 *        
 *  22   mpeg      1.21        3/14/01 2:53:08 PM     Lucy C Allevato Changes 
 *        for PVCS 1386, changed some prototypes to bool from void.
 *        Make sure to use video.c revision 1.111 or later.
 *        
 *  21   mpeg      1.20        2/28/01 3:10:56 PM     Lucy C Allevato Added a 
 *        define for the length of time our hwtimer waits for still
 *        decodes.
 *        
 *  20   mpeg      1.19        2/22/01 3:30:52 PM     Lucy C Allevato Added a 
 *        define for a video hung case, where vcore is hung.  I am expecting
 *        any new error codes to follow this pattern I have started at 0x80.  
 *        ie:
 *        0x80-0x89 will be video failure codes so we can tell what area the 
 *        fatal_exit
 *        has come from.
 *        
 *  19   mpeg      1.18        2/2/01 4:18:02 PM      Angela          Merged 
 *        Vendor_c changes into the code(mostly #if CUSTOMER==VENDOR_C blocks)
 *        see DCS#1049
 *        
 *  18   mpeg      1.17        11/6/00 2:19:48 PM     Ismail Mustafa  Added new
 *         prototype for the handle message function.
 *        
 *  17   mpeg      1.16        8/8/00 5:56:00 PM      Ismail Mustafa  Added 
 *        PLAY_STILL_STATE_NO_DISP.
 *        
 *  16   mpeg      1.15        8/3/00 2:05:58 PM      Ismail Mustafa  Added 
 *        PLAY_LIVE_STATE_NO_SYNC.
 *        
 *  15   mpeg      1.14        5/15/00 5:15:52 PM     Ismail Mustafa  Enabled 
 *        bitstream error interrupts.
 *        
 *  14   mpeg      1.13        4/11/00 7:16:28 PM     Ismail Mustafa  Reduced 
 *        User Data buffer size.
 *        
 *  13   mpeg      1.12        4/7/00 2:33:46 PM      Ismail Mustafa  Added 
 *        prototype for gen_video_register_user_data_callback.
 *        
 *  12   mpeg      1.11        4/5/00 12:00:48 PM     Ismail Mustafa  Remoced 
 *        Interrupt 7 from the ISR define.
 *        
 *  11   mpeg      1.10        4/4/00 8:53:24 PM      Ismail Mustafa  Added API
 *         for User data.
 *        
 *  10   mpeg      1.9         2/18/00 5:20:38 PM     Ismail Mustafa  Added 
 *        video interrupt 7 for lock up detection.
 *        
 *  9    mpeg      1.8         1/31/00 4:05:48 PM     Ismail Mustafa  Cleanup.
 *        
 *  8    mpeg      1.7         12/7/99 3:54:46 PM     Ismail Mustafa  Added 
 *        more prototypes.
 *        
 *  7    mpeg      1.6         10/26/99 5:56:10 PM    Ismail Mustafa  Added 
 *        gen_video_StartSendPesData().
 *        
 *  6    mpeg      1.5         10/7/99 2:19:12 PM     Ismail Mustafa  New for 
 *        generic video driver.
 *        
 *  5    mpeg      1.4         7/14/99 4:52:16 PM     Ismail Mustafa  Added 
 *        StartSendPesData prototypes.
 *        
 *  4    mpeg      1.3         6/24/99 8:05:50 PM     Ismail Mustafa  Changed 
 *        NECHES flush bytes to 256.
 *        
 *  3    mpeg      1.2         4/27/99 11:37:26 AM    Ismail Mustafa  Added new
 *         command defines.
 *        
 *  2    mpeg      1.1         4/19/99 11:36:12 AM    Ismail Mustafa  Changed 
 *        NUM_FLUSH_BYTES for NECHES.
 *        
 *  1    mpeg      1.0         1/27/99 4:05:08 PM     Steve Glennon   
 * $
 *
 ******************************************************************************/

