/****************************************************************************/
/*                   Conexant Proprietary and Confidential                  */
/*                       Software File/Module Header                        */
/*                   Conexant Systems Inc. (c) 1998-2004                    */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       video.c
 *
 *
 * Description:    CX2249X Generic Video Driver
 *
 *
 * Author:         Mustafa Ismail (extensively modified since by Eric Ching, 
 *                 Dave Wilson, Steve Glennon and Quillian Rutherford)
 *
 ****************************************************************************/
/* $Id: video.c,v 1.278, 2004-06-01 21:52:53Z, Billy Jackman$
 ****************************************************************************/
#include "stbcfg.h"
#include "basetype.h"
#include "kal.h"
#include "confmgr.h"
#include "video.h"
#include "retcodes.h"
#include "globals.h"

#ifdef DRIVER_INCL_GENDMXC
#include "gendemux.h"
#else
#include "demuxapi.h"
#endif

#include "osdlib.h"
#include "vidlib.h"
#include "startup.h"
#include "aud_comn.h"
#if (defined OPENTV_12) || (defined BLANK_BEFORE_ALL_STILLS)
#include "gfxlib.h"
#endif

/* Set this to 1 to enable use of the hardware byte swapping aperture */
/* (recommended). Set it to 0 to do byte swapping in software.        */
#define SWAP_COPY   1

/* Define PERIODIC_VIDEO_TRACE to generate a trace every 0.5s   */
/* which shows video-related info - read/write ptrs, skip/rpt   */
/* counts, PTS, PCR, STC, etc.                                  */
/* #define PERIODIC_VIDEO_TRACE */

/* We need to flush a few stills through the video decoder on   */
/* startup to ensure that it is in a known good state. This     */
/* value defines the number of stills we send. 3 seems to be    */
/* the correct number for CX2249x                               */
#define NUM_STILLS_FOR_INIT_FLUSH 3

/* When we restart video, we try to set the read and write pointers */
/* this far apart in the encoded video buffer.                      */
#define MPEG_BUFFER_RESTART_TARGET (200*1024)

/* Define DEFERRED_UNBLANK to enable deferred unblanking where the video */
/* plane will remain blanked after a call to gen_unblank_video unless it */
/* contains an image which was originally decoded for display. If no     */
/* displayable image is available, the code waits until one is before    */
/* unblanking. This is required for OpenTV 1.2 where many apps unblank   */
/* the video before decoding something into it thus causing a blast of   */
/* video crud if we don't defer the unblank. It is also required if      */
/* using FAST_RETURN_FROM_PLAY since this uses deferred unblank to       */
/* ensure that video is not unblanked until it is safe to be displayed.  */
#if (defined OPENTV_12) || (FAST_RETURN_FROM_PLAY == YES) || (ENABLE_DEFERRED_VIDEO_UNBLANK == YES)
#define DEFERRED_UNBLANK
#endif

/* Colour we use to hide artifacts during video_play startup if         */
/* BLANK_ON_VIDEO_START is defined                                      */
#define EPG_BACKGROUND_COLOUR 0x0080A014

/* Number of decode complete interrupts to wait before deciding that live */
/* video has started correctly. Minimum is 2 unless fast video unblank    */
/* is being used.                                                         */
#if (BRAZOS_WABASH_FAST_UNBLANK == YES)
  #define VIDEO_START_DECODE_DELAY 1
#else
  #define VIDEO_START_DECODE_DELAY 2
#endif

/* Maximum time (ms) to wait for live video to start */
#define MAX_VIDEO_START_TIMEOUT 3500

/* Time to wait for video data to start appearing from demux after */
/* enabling the channel.                                           */
#define MAX_VIDEO_DATA_FLOWING_TIMEOUT 750

/* Time to wait for first decode complete before assuming stream is scrambled */
#define MAX_VIDEO_PICTURE_1_TIMEOUT 2150

/* Granularity (ms) of polling loop used in waiting for video to start */
#define VIDEO_START_LOOP_DELAY  40

/* Time to wait for a still decode to complete before instigating error recovery */
#define VIDEO_STILL_DECODE_TIMEOUT 300

/* Number of decoded pictures after starting video during which skip/repeat info is dumped */
#define DEBUG_INFO_MAX_PICTURES 12

/* The number of pictures between each skip/repeat debug message */
#define DEBUG_INFO_RATE 2

/* Black 16 x 16 pixel MPEG still data */
const unsigned char black16_image[] = { 
0x00,0x00,0x01,0xb3,0x01,0x00,0x10,0x23,0x00,0x00,0xe3,0x80,0x00,0x00,0x01,0xb5
,0x14,0x8a,0x00,0x01,0x00,0x00,0x00,0x00,0x01,0xb8,0x00,0x08,0x00,0x40,0x00,0x00
,0x01,0x00,0x00,0x0c,0xe4,0xe8,0x00,0x00,0x01,0xb5,0x8f,0xff,0xf3,0x49,0x80,0x00
,0x00,0x01,0x01,0xbb,0xf8,0x7b,0x46,0x8d,0x18,0x61,0x80,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x01,0xb7                                                       
}; 
bool gen_video_decode_still_image(voidF *pImage, u_int32 uSize);

/* Number of times to poll MPEG command valid bit before assuming hardware is stuck */
/* This is the number of times to go round the polling loop. Multiply by 20mS to work */
/* out the actual timeout.                                                            */
#define MPEG_COMMAND_TIMEOUT_LOOPS 5

/* Number of milliseconds to wait for an idle interrupt after telling the video core */
/* to stop.                                                                          */
#define MPEG_IDLE_LOOP_TIMEOUT 200//120
  
/*
   This structure is a copy of the one in audio.h. audio.h cannot be included here
   since it includes opentvx.h. So until audio is made generic this should remain here.
*/

enum { FIND_PREFIX, PARSING_HEADER, SENDING_DATA
};
enum { PREFIX_FOUND, LAST_BYTE_NONZERO, LAST_BYTE_ZERO, LAST_TWO_BYTES_ZERO,
    SKIP_STREAM_ID, GET_PACKET_LENGTH1, GET_PACKET_LENGTH2,
    GET_MPEG1_MPEG2, SKIPPING_DATA_BITS, SKIPPING_STUFFING_BYTES,
    SKIP_SECOND_BYTE_OF_STD, GET_PTS_MPEG1, PTS12, PTS13, PTS14,
    FIFTH_BYTE_OF_PTS, SKIP_DTS, SKIP_DTS2, SKIP_DTS3, SKIP_DTS4,
    END_OF_DTS, SKIPPING_HEADER, READING_PTS_BITS, GET_HEADER_LENGTH,
    GET_PTS_MPEG2, PTS22, PTS23, PTS24, FIFTH_BYTE_OF_MPEG2_PTS, HAS_PTS
};

typedef struct  _PES_global_data
{
    int  where_in_PES;
    int  progress;
    int  bytes_left_in_packet;
    int  bytes_left_in_header;
    int  pts_present;
    char pts_array[5];
    bool PesLenTBD;
    int  InitialPacketLen;
} PES_global_data;

/* Hold the dimensions and type of the image stored in a given */
/* decoded MPEG anchor pciture buffer                          */
MPEG_buffer_image gsDecodedImages[2];
bool              gbDeferredUnblank = FALSE;
  
extern int handle_PES( gen_farcir_q *q, PES_global_data *perm_data, bool isAudio );
extern int copy_data( gen_farcir_q *q, int bytes_to_copy, bool isAudio );
bool LoadMicrocode(bool Video, void *pStart, void *pEnd);
static void gen_video_blank_internal(void);
       void gen_video_unblank_internal(void);
static CNXT_VID_STATUS StartLiveVideo(u_int32 uState, bool *pbStillDecoded);
 void ReadWritePtrReset(void);
static bool WaitForVideoIdle(u_int32 uTimeoutMs);
static CNXT_VID_STATUS WaitForStillDecodeToComplete(u_int32 uTimeout);
static void PadEncodedVideoBuffer(int iNumBytes);
static bool SendPaddingAndStartCode(u_int8 uCode, int iPosition);
static u_int32 GetVideoBufferFullness(void);
static void video_send_hardware_stop_command(void);
static bool SafeToUnblankVideo(int iDisplayCount, int iDisplayIndex, int iState);

sem_id_t semVideoPlay;
static bool fix_mpeg_still_problems(gen_farcir_q *q, bool bReset);
extern void av_command_done(void);

extern bool osdSetMpgScaleDelay(u_int32 delay);
extern bool gbRestoreAudioAfterReset;


/*************************************/
/* Video Microcode variant selection */
/*************************************/
#if VIDEO_MICROCODE == UCODE_COLORADO
  /************/
  /* Colorado */
  /************/
  #if VIDEO_UCODE_TYPE == VMC_EXTRA_ERROR_RECOVERY
    extern unsigned char col_video_ucode_err[];
    extern u_int32 col_video_ucode_err_size;
    #define SELECTED_VIDEO_MICROCODE      col_video_ucode_err
    #define SELECTED_VIDEO_MICROCODE_SIZE col_video_ucode_err_size
    /* Extra pause command only available in Canal+ microcode */
    #define MPG_COMMAND_PAUSE_ON_I 0x1060
    #define MPEG_I_IDLE_TIMEOUT 1200
  #elif VIDEO_UCODE_TYPE == VMC_MACROBLOCK_ERROR_RECOVERY
    extern unsigned char col_macroblock_video_ucode[];
    extern u_int32 col_macroblock_video_ucode_size;
    #define SELECTED_VIDEO_MICROCODE      col_macroblock_video_ucode
    #define SELECTED_VIDEO_MICROCODE_SIZE col_macroblock_video_ucode_size
  #elif VIDEO_UCODE_TYPE == VMC_PICTURE_ERROR_RECOVERY
    extern unsigned char col_picture_video_ucode[];
    extern u_int32 col_picture_video_ucode_size;
    #define SELECTED_VIDEO_MICROCODE      col_picture_video_ucode
    #define SELECTED_VIDEO_MICROCODE_SIZE col_picture_video_ucode_size
  #else
    #error "Unsupported Colorado video microcode type defined in VIDEO_UCODE_TYPE"  
  #endif /* VIDEO_UCODE_TYPE */
#elif VIDEO_MICROCODE == UCODE_HONDO
  /*********/
  /* Hondo */
  /*********/
  extern unsigned char hond_video_ucode[];
  extern u_int32 hond_video_ucode_size;
  #define SELECTED_VIDEO_MICROCODE      hond_video_ucode
  #define SELECTED_VIDEO_MICROCODE_SIZE hond_video_ucode_size
#elif VIDEO_MICROCODE == UCODE_WABASH
  /**********/
  /* Wabash */
  /**********/
  #if VIDEO_UCODE_TYPE == VMC_MACROBLOCK_ERROR_RECOVERY
    extern unsigned char wabash_macro_video_ucode[]; 
    extern u_int32 wabash_macro_video_ucode_size;
    #define SELECTED_VIDEO_MICROCODE      wabash_macro_video_ucode
    #define SELECTED_VIDEO_MICROCODE_SIZE wabash_macro_video_ucode_size
  #elif VIDEO_UCODE_TYPE == VMC_PICTURE_ERROR_RECOVERY
    extern unsigned char wabash_video_ucode[];
    extern u_int32 wabash_video_ucode_size;
    #define SELECTED_VIDEO_MICROCODE      wabash_video_ucode
    #define SELECTED_VIDEO_MICROCODE_SIZE wabash_video_ucode_size
  #else  
    #error "Unsupported Wabash video microcode type defined in VIDEO_UCODE_TYPE"  
  #endif  
#elif VIDEO_MICROCODE == UCODE_BRAZOS
  #if VIDEO_UCODE_TYPE == VMC_MACROBLOCK_ERROR_RECOVERY
    extern unsigned char brazos_macro_video_ucode[];
    extern u_int32 brazos_macro_video_ucode_size;
    #define SELECTED_VIDEO_MICROCODE      brazos_macro_video_ucode
    #define SELECTED_VIDEO_MICROCODE_SIZE brazos_macro_video_ucode_size
    
  #elif VIDEO_UCODE_TYPE == VMC_MACRO_BRAZOS_NTSC_PAL
    extern unsigned char brazos_video_ucode[];
    extern u_int32 col_video_ucode_size;
    #define SELECTED_VIDEO_MICROCODE      brazos_video_ucode
    #define SELECTED_VIDEO_MICROCODE_SIZE col_video_ucode_size
        
  #else  
    #error "Unsupported Brazos video microcode type defined in VIDEO_UCODE_TYPE"  
  #endif  
#else
  #error "Unsupported chip type" 
#endif

extern void InitMPEGVideo(void);
extern void VideoReInitHW(void);

extern LPMPG_ADDRESS lpAudReadPtr;
extern LPMPG_ADDRESS lpAudWritePtr;
extern char leftover[2][4];
extern volatile u_int32 gnFieldCount;
volatile u_int32 *pgnFieldCount;

extern bool VideoPIDEnabled;

extern LPMPG_ADDRESS glpTSReadPtr;
extern LPMPG_ADDRESS glpTSWritePtr;

extern int gnOsdMaxHeight;

/* Shadow copies of the currently set I, P and B buffer addresses */
u_int8 *gpIBuffer = NULL;
u_int8 *gpPBuffer = NULL;
u_int8 *gpBBuffer = NULL;
u_int8 *gpVideoDisplayBuffer = (u_int8*)HWBUF_DEC_P_ADDR;
u_int32 gdwLastDecodedAnchor;
int     giDRMDisplayPicture = 0;

extern u_int32 gdwSrcWidth;
extern u_int32 gdwSrcHeight;

u_int32 gDemuxInstance =0;

/* Video Drip */
tick_id_t VideoDripTick;
void VideoDripTickCallback(tick_id_t hTick, void *pUser);
gen_farcir_q VideoDripQ;
void tick_timer_drip_callback(tick_id_t tickTimer, void *pUserData);
void copy_drip_data(void);
static int DripIn;
static int DripOut;
static int StartDrip;
static DRIP_CALLBACK VideoBufEmpty;
static DRIP_CALLBACK DripBufEmpty;
#define VDREQ_ARRAY_SIZE 8  //Must be a power of 2!!!
static gen_farcir_q  vdreqs[VDREQ_ARRAY_SIZE];

#define DRIP_TIME_DELAY   1000

#ifdef DEBUG

bool gbWaiting = FALSE;

#if (LEGACY_DVR==YES)

/*
 * Legacy DVR
 */

extern LPMPG_ADDRESS glpDVRAudio1ReadPtr;
extern LPMPG_ADDRESS glpDVRAudio1WritePtr;
extern LPMPG_ADDRESS glpDVRAudio2ReadPtr;
extern LPMPG_ADDRESS glpDVRAudio2WritePtr;

extern LPDPS_ADDRESS glpDVREventStart;
extern LPDPS_ADDRESS glpDVREventEnd;
extern LPDPS_ADDRESS glpDVREventReadPtr;
extern LPDPS_ADDRESS glpDVREventWritePtr;

extern LPDPS_ADDRESS glpDVRVideoStart;
extern LPDPS_ADDRESS glpDVRVideoEnd;
extern LPDPS_ADDRESS glpDVRVideoReadPtr;
extern LPDPS_ADDRESS glpDVRVideoWritePtr;

extern LPDPS_ADDRESS glpDVRAudioStart;
extern LPDPS_ADDRESS glpDVRAudioEnd;
extern LPDPS_ADDRESS glpDVRAudioReadPtr;
extern LPDPS_ADDRESS glpDVRAudioWritePtr;

#endif
#endif

#if ((PVR==YES)||(XTV_SUPPORT==YES))
static CNXT_ES_BUFFER_CALLBACK Video_ES_Buffer_Callback_Function;
static void *Video_ES_Buffer_Callback_Tag;
static CNXT_VIDEO_UNBLANK_CALLBACK Video_Unblank_Callback_Function;
static void *Video_Unblank_Callback_Tag;
#endif /* PVR==YES */

bool VideoEnableSync(void);
bool VideoDisableSync(void);
void VideoEnableDec(void);
void VideoDisableDec(void);


void video_process(void *arg);
void video_process_msg(u_int32 *message);
void video_process_set_vbuffs(u_int8 *IAddr, u_int8 *PAddr, u_int8 *BAddr);
CNXT_VID_STATUS send_video_command(u_int32 *msg);
void send_video_command_done(u_int32 *msg, CNXT_VID_STATUS eStatus);
void post_video_command(u_int32 *msg, gen_callback_cmd_done_t video_command_done);
void post_video_command_done(u_int32 *msg, CNXT_VID_STATUS eStatus);

#ifdef DEBUG
static void DumpVideoMessage(u_int32 *message);
#ifdef PERIODIC_VIDEO_TRACE
static char *GetStateDescription(int iState);
#endif
#endif /* DEBUG */

#ifdef BLANK_BEFORE_ALL_STILLS
static void fill_video_color(u_int16 uWidth, u_int16 uHeight, u_int32 *pImage, u_int8 Y, u_int8 Cb, u_int8 Cr);
static bool bStillBlank = TRUE;
#define VIDEO_BLACK_Y  0x00
#define VIDEO_BLACK_CB 0x80
#define VIDEO_BLACK_CR 0x80
#endif

#ifdef DEBUG
char *strVideoStart[] =
{
  "CNXT_VID_STATUS_OK",
  "CNXT_VID_STATUS_DECODE_1_TIMEOUT",
  "CNXT_VID_STATUS_DECODE_2_TIMEOUT",
  "CNXT_VID_STATUS_SYNC_TIMEOUT", 
  "CNXT_VID_STATUS_DEMUX_DISABLED",
  "CNXT_VID_STATUS_NO_DATA",
  "CNXT_VID_STATUS_SYS_ERROR",
  "CNXT_VID_STATUS_ASYNC"
};
#endif

CNXT_VID_STATUS WaitForLiveVideoToStart(bool bSync);
static  u_int32 SetReadPtr(u_int32 uTarget, bool bOverflowed);

int video_IRQHandler(u_int32 IntID, bool Fiq, PFNISR *pfnChain);

task_id_t VideoProcessID = 0;
task_id_t VideoDebugProcessID = 0;
queue_id_t VideoQueueID = 0;

bool gStoppingVideo = FALSE;
gen_time_code_t gen_current_time_code;

#if (VIDEO_UCODE_TYPE == VMC_MACROBLOCK_ERROR_RECOVERY) && (VIDEO_MICROCODE == UCODE_COLORADO)
bool gbUseFrameScaling = FALSE;
#endif

/* Timeouts used in WaitForLiveVideoToStart */
u_int32 guVideo1Timeout = MAX_VIDEO_DATA_FLOWING_TIMEOUT;
u_int32 guVideo2Timeout = MAX_VIDEO_PICTURE_1_TIMEOUT;
u_int32 guVideo3Timeout = MAX_VIDEO_START_TIMEOUT;

unsigned char LocalVideoBuffer[LOCAL_VIDEO_BUFFER_SIZE];
u_int8 UserBuffer[LOCAL_VIDEO_USER_BUFFER_SIZE];
u_int32 UserWriteIndex = 0, UserReadIndex = 0;


LPREG glpAudioFrameDrop = (LPREG)0x30400068;
LPREG glpVideoFrameDrop = (LPREG)0x30400064;

int VBReadIndex;
int VBWriteIndex;
PFNISR pOldVideofnHandler = 0;
int FramesPerSecond = 30;       /* Default to NTSC */
int PlayState = PLAY_OFF_STATE;
int DecodeCompleteCount = 0;
int iStillSeqEndCount = 0;      /* Number of sequence end interrupts received during decode of a "still" */
int iStillSeqStartCount = 0;    /* Number of sequence start tags found when "still" is parsed */
int iStillSeqCount = 0;         /* Number of sequence end tags found when "still" is parsed */
int iStillPanOffset  = 0;       /* Still image pan offset in 1/16 pixel increments */
int iStillScanOffset = 0;       /* Still image scan offset in 1/16 pixel increments */
bool bStillVectorFound = FALSE;
u_int32 StillErrorCnt = 0;
u_int32 TotalBytesToCopy = 0;
bool AllowAudioSync = FALSE;
bool AllowStillCmd = FALSE;
bool bSeqEndFound   = FALSE;
CNXT_VID_STATUS eLastStatus = CNXT_VID_STATUS_OK;
u_int32 uVCoreResetCount = 0;

/* Error condition counters */
u_int32 guVideoBufferOverflowCount = 0;
u_int32 guVideoBufferUnderflowCount = 0;
u_int32 guDSRErrorCounts[NUM_DSR_ERROR_CODES];

#ifdef OPENTV_12
extern bool gbRunningEPG;
#endif

gen_callback_frame_t            gVideoFrameCallBack = NULL;
gen_callback_time_code_notify   gTimeCodeCallBack = NULL;
gen_callback_user_data_notify   gUserDataCallBack = NULL;
gen_callback_handle_msg_func    gHandleMsgFunc = NULL;
gen_callback_audio_error_notify gpfnAudioErrorCallBack = NULL;

#if (EMULATION_LEVEL == FINAL_HARDWARE)
//u_int32 debug_sleep_time = 5000;
u_int32 debug_sleep_time = 1000;
#else
u_int32 debug_sleep_time = 10;
#endif

u_int32 num_vid_leftovers;
volatile bool VideoProcessStarted = FALSE;
volatile bool *pVideoProcessStarted = &VideoProcessStarted;
bool PlayPesFromMem = FALSE;
PES_global_data vid_PES_global_data;

u_int32 uLastDecodedAnchor = 0;

//*****Green screen patch start***************
static volatile u_int32 bResetComplete=1;
static volatile u_int32 bChannelChanged=0;
//*****Green screen patch end***************

/* Variables for core stall recovery. */
static u_int32 guLocalVideoBufferOverflowCount = 0;
static u_int32 guContinueCommandinProgress = 0;

void ResetVCore(void);
void WaitOnCtlrFix(void);
static bool SetWritePtr(MPG_ADDRESS new_write);

static int gen_send_data(gen_farcir_q *q)
{
    /* read as many bytes as possible into audio buffer.
    * Then update the head of the far circular queue
    * and return the number of bytes consumed.
    */

    int bytes_consumed = 0;
    int bytes_to_copy;

    if (q->out > q->in){    /* wrapping occured */
        bytes_to_copy = q->in + q->size - q->out;
    }
    else{
        bytes_to_copy = q->in - q->out;
    }
    bytes_consumed = copy_data(q, bytes_to_copy, FALSE);
    return bytes_consumed;
}
/*********************************************************************************/
/* SendFrameHeader                                                               */
/*                                                                               */
/* Description: In an effort to make sure the vcore will execute new commands,   */
/*              we need to send in a frame header to ensure that the vcore       */
/*              will look up to check the command buffer for a new command       */
/*              when data has dried up.                                          */
/* Params: None                                                                  */
/* Return: TRUE on success                                                       */
/*         FALSE on failure                                                      */
/*********************************************************************************/
#define BUFFER_START_CODE_POSITION  18
#define NUM_HEADER_PADDING_BYTES   512

static bool SendFrameHeader(void)
{
  return(SendPaddingAndStartCode(0xB3, BUFFER_START_CODE_POSITION));
}


/********************************************************************/
/*  FUNCTION:    SendPaddingAndStartCode                            */
/*                                                                  */
/*  PARAMETERS:  uCode     - Start code to send                     */
/*               iPosition - Position of 0x01 byte before start     */
/*                           code in the buffer to be sent. If      */
/*                           negative, no start code is sent.       */
/*                                                                  */
/*  DESCRIPTION: Send 512 padding bytes with, optionally, a start   */
/*               code embedded at a given position.                 */
/*                                                                  */
/*  RETURNS:     TRUE on success, FALSE on failure                  */
/*                                                                  */
/*  CONTEXT:     Must be called in task context                     */
/*                                                                  */
/********************************************************************/
static bool SendPaddingAndStartCode(u_int8 uCode, int iPosition)
{
  u_int8 uData[NUM_HEADER_PADDING_BYTES];
  int    iLoop;
	gen_farcir_q VideoQueue;

  trace_new(TRACE_LEVEL_2|TRACE_MPG, "VIDEO: Sending 512 padding bytes and start code 0x%02x at position %d\n", uCode, iPosition);
  
  /* Send a buffer containing a sequence end and padding */
  for(iLoop = 0; iLoop < NUM_HEADER_PADDING_BYTES; iLoop++)
    uData[iLoop]=0;
  if((iPosition >0) && (iPosition < (NUM_HEADER_PADDING_BYTES-1)))  
  {
    uData[iPosition]   = 0x01;  
    uData[iPosition+1] = uCode;  
  }  
  
	VideoQueue.begin = (voidF)uData;
	VideoQueue.size = sizeof(uData);
	VideoQueue.in = VideoQueue.size;
	VideoQueue.out = 0;
	while(VideoQueue.in - VideoQueue.out >= 4){
			  VideoQueue.out += gen_send_data((gen_farcir_q *) &VideoQueue);
	}
   return TRUE;
}

/*SetWritePtr
 *Description: Attempts to safely reset the write pointer
 *             for the hardware buffer.
 *Params: MPG_ADDRESS new_write, value to set WritePtr to
 *Return: TRUE on success
 *        FALSE on failure
 *Added: 3/30/01 by JQR
 */
static bool SetWritePtr(MPG_ADDRESS new_write)
{
   bool ks;  /*For critical section*/
   bool RetV = TRUE;  /*Value to return*/

   ks = critical_section_begin();
   *DPS_VIDEO_WRITE_PTR_EX(gDemuxInstance) = (u_int32)new_write;
   if (*DPS_VIDEO_WRITE_PTR_EX(gDemuxInstance) != (u_int32)new_write) {
     RetV = FALSE;
     cs_trace(TRACE_MPG|TRACE_LEVEL_2,"VIDEO:Failed to set write pointer to 0x%x, read back 0x%x\n",
	      (u_int32)*DPS_VIDEO_WRITE_PTR_EX(gDemuxInstance),(u_int32)new_write);
   }
   critical_section_end(ks);

   return  RetV;
}


/*---------------------------------------------------------**
** FDelay                                                  **
** Desc.                                                   **
**     Provides a delay.                                   **
** Params                                                  **
**     NumFields       Amount of delay in number of fields **
** Returns                                                 **
**     Nothing                                             **
**---------------------------------------------------------*/
/*
void FDelay(unsigned int NumFields)
{
    *pgnFieldCount = 0;
    while (*pgnFieldCount < NumFields)
        ;
}
*/
/*-------------------------------------------------------------------------**
** WaitOnCtlrFix                                                           **
** Desc.                                                                   **
**     Function to wait until command is accepted.                         **
**     It will timeout after 5 fields.                                     **
** Params.                                                                 **
**     None                                                                **
** Returns                                                                 **
**     None                                                                **
**-------------------------------------------------------------------------*/
void WaitOnCtlrFix()
{
    *pgnFieldCount = 0;

    while (*( (LPREG)glpDecoderStatusReg) & MPG_CMD_VALID){
        if (*pgnFieldCount > 30){
            trace_new(TRACE_MPG|TRACE_LEVEL_ALWAYS, "WaitOnCtlr Timeout. Reset VCore\n");
            ResetVCore();
            trace_new(TRACE_MPG|TRACE_LEVEL_ALWAYS, "VCoreReset Done.\n");
            SetWritePtr((MPG_ADDRESS)*DPS_VIDEO_READ_PTR_EX(gDemuxInstance));
            break;
        }
        task_time_sleep(20);
    }
}

/********************************************************************/
/*  FUNCTION:    video_send_hardware_command                        */
/*                                                                  */
/*  PARAMETERS:  uCommand - command code to send to the MPEG        */
/*                          decoder hardware.                       */
/*                                                                  */
/*  DESCRIPTION: Send a command to the mpeg hardware, making sure   */
/*               that it is safe to do so first. If the hardware    */
/*               seems to have hung (holds the CmdValid bit high    */
/*               for more than 1uS), abort horribly and reset the   */
/*               IRD.                                               */
/*                                                                  */
/*  RETURNS:     Nothing                                            */
/********************************************************************/
void video_send_hardware_command(u_int32 uCommand)
{
  int  iLoop;
//*****Green screen patch start***************
  bool ks;
//*****Green screen patch end***************
  not_interrupt_safe();
  
  /* Waiting for 100 ms for a command to be accepted... */
  iLoop = MPEG_COMMAND_TIMEOUT_LOOPS;
  
  trace_new(TRACE_MPG|TRACE_LEVEL_1, "****VIDEO COMMAND: 0x%08x\n", uCommand);
  
  while((*((LPREG)glpDecoderStatusReg) & MPG_CMD_VALID) && (iLoop--)){
       task_time_sleep(20);
  }
  
  if (*((LPREG)glpDecoderStatusReg) & MPG_CMD_VALID)
  {
//*****Green screen patch start*********************************************************************************
    /* Loop timed out with MPG_CMD_VALID still set. Try to free up core by sending it a sequence end ! */
    trace_new(TRACE_MPG|TRACE_LEVEL_ALWAYS, "VIDEO: **** Core seems to have stalled. Sending sequence end\n");
    SendFrameHeader();
    
    /* Now wait a bit longer */
    iLoop = MPEG_COMMAND_TIMEOUT_LOOPS;
  
    while((*((LPREG)glpDecoderStatusReg) & MPG_CMD_VALID) && (iLoop--))
    {
         task_time_sleep(20);
    }
    
    /* Is the bit still set ? */
    if (*((LPREG)glpDecoderStatusReg) & MPG_CMD_VALID)
    {
//*****Green screen patch end*****************************************************************************************
      trace_new(TRACE_MPG|TRACE_LEVEL_ALWAYS, "VIDEO: **** Core still stalled. Try resetting\n");
      ResetVCore();
      
      /* Wait a while longer */
      iLoop = MPEG_COMMAND_TIMEOUT_LOOPS;
  
      while((*((LPREG)glpDecoderStatusReg) & MPG_CMD_VALID) && (iLoop--))
      {
           task_time_sleep(20);
      }
      
      /* If we still have the CMD_VALID bit set, we are totally hosed! Reboot the box */
      if (*((LPREG)glpDecoderStatusReg) & MPG_CMD_VALID)
      {
        trace_new(TRACE_MPG|TRACE_LEVEL_ALWAYS, "VIDEO: **** Core still stalled after reset!! ****\n");
        error_log(MOD_MPG|ERROR_FATAL);
      }
//*****Green screen patch start********************************************************
    }
    
    /* Now post the command */

    ks = critical_section_begin();
    *((LPREG)glpMpgCmd) = uCommand;
    bChannelChanged = 1;
    critical_section_end(ks);
    
  }
  else
  {
    /* The hardware is ready to receive the command */
    ks = critical_section_begin();
    *((LPREG)glpMpgCmd) = uCommand;
    bChannelChanged = 1;
    critical_section_end(ks);
  }  
//*****Green screen patch end************************************************************
  if ( uCommand == MPG_COMMAND_CONTINUE )
  {
    guContinueCommandinProgress = 0;
    //*glpIntMask |= MPG_VB_FULL;
  }  
}

/*-------------------------------------------------------------------**
** gen_video_register_audio_error_callback                           **
** Desc.                                                             **
**     Registers a callback which is called when an MPEG error       **
**     interrupt is received and the DSR register indicates that the **
**     audio decoder was the source of the error.                    ** 
** Params.                                                           **
**     pfnAudioError      Function to call when error occurs.        **
** Returns                                                           **
**     Nothing.                                                      **
**-------------------------------------------------------------------*/
void gen_video_register_audio_error_callback(gen_callback_audio_error_notify pfnAudioError)
{
    bool ks;

    ks = critical_section_begin();
    gpfnAudioErrorCallBack = pfnAudioError;
    critical_section_end(ks);
    return;
}

/*-------------------------------------------------------------------**
** gen_video_register_user_data_callback                             **
** Desc.                                                             **
**     Registers a callback which is called when user data arrives in**
**     the video stream. Also enables or disables the User Data      **
**     interupt accordingly.                                         ** 
** Params.                                                           **
**     UserDataCallback   Function to call with user data.           **
** Returns                                                           **
**     Nothing.                                                      **
**-------------------------------------------------------------------*/

void gen_video_register_user_data_callback(gen_callback_user_data_notify UserDataCallback){
    bool ks;

    ks = critical_section_begin();
    gUserDataCallBack = UserDataCallback;
    if (gUserDataCallBack != NULL){
        *glpIntMask |= MPG_USER_DETECTED;
    }
    else{
        *glpIntMask &= ~MPG_USER_DETECTED;
    }
    critical_section_end(ks);
    return;
}

/*--------------------------------------------------------------------**
** gen_video_register_handle_message_function                         **
** Desc.                                                              **
**     Registers a callback which is called when a new message is     **
**     removed from the  Task queue. The callback is given first right**
**     of refusal to process the message.                             **
** Params.                                                            **
**     handle_message_function.                                       **
** Returns                                                            ** 
**     Nothing.                                                       **
**--------------------------------------------------------------------*/
void gen_video_register_handle_message_function(gen_callback_handle_msg_func handle_message_function){
    bool ks;

    ks = critical_section_begin();
    gHandleMsgFunc = handle_message_function;
    critical_section_end(ks);
    return;
}

/********************************************************************/
/*  FUNCTION:    gen_video_set_still_scaling_mode                   */
/*                                                                  */
/*  PARAMETERS:  bUseFrameScaling - if TRUE, frame scaling is used  */
/*                                  while motion video is stopped.  */
/*                                  if FALSE, field scaling is used */
/*                                                                  */
/*  DESCRIPTION: This call sets the scaling mode to be used when    */
/*               motion video is stopped. If bUseFrameScaling is    */
/*               TRUE, the video decoder displays both fields of the*/
/*               last decoded image when video is stopped. This     */
/*               gives better vertical resolution for stills but    */
/*               can result in "jitter" when used to display a      */
/*               stopped motion stream. If FALSE is passed, field   */
/*               scaling is used where only a single field is       */
/*               displayed. This removes the possibility of jitter  */
/*               but causes stills to display with only half the    */
/*               vertical resolution.                               */
/*                                                                  */
/*               ONLY SUPPORTED ON CX2249x WITH MACROBLOCK-BASED    */
/*               ERROR RECOVERY MICROCODE!                          */
/*                                                                  */
/*  RETURNS:     TRUE on success, FALSE on failure                  */
/*                                                                  */
/*  CONTEXT:     May be called in any context                       */
/*                                                                  */
/********************************************************************/
bool gen_video_set_still_scaling_mode(bool bUseFrameScaling)
{
  #if (VIDEO_UCODE_TYPE == VMC_MACROBLOCK_ERROR_RECOVERY) && (VIDEO_MICROCODE == UCODE_COLORADO)
  gbUseFrameScaling = bUseFrameScaling;
  return(TRUE);
  #else
  trace_new(TRACE_MPG|TRACE_LEVEL_ALWAYS, "VIDEO: Scaling mode not supported except on Colorado with macroblock ucode!\n");
  return(FALSE);
  #endif
}

/********************************************************************/
/*  FUNCTION:    gen_video_set_still_scaling_mode                   */
/*                                                                  */
/*  PARAMETERS:  pbUseFrameScaling - pointer to storage for the     */
/*                                   returned scaling mode.         */
/*                                                                  */
/*  DESCRIPTION: This call gets the scaling mode which is being     */
/*               used when motion video is stopped.                 */
/*                                                                  */
/*               ONLY SUPPORTED ON CX2249x WITH MACROBLOCK-BASED    */
/*               ERROR RECOVERY MICROCODE!                          */
/*                                                                  */
/*  RETURNS:     TRUE on success, FALSE on failure                  */
/*                                                                  */
/*  CONTEXT:     May be called in any context                       */
/*                                                                  */
/********************************************************************/
bool gen_video_get_still_scaling_mode(bool *pbUseFrameScaling)
{
  #if (VIDEO_UCODE_TYPE == VMC_MACROBLOCK_ERROR_RECOVERY) && (VIDEO_MICROCODE == UCODE_COLORADO)
  if(pbUseFrameScaling)
  {
    *pbUseFrameScaling = gbUseFrameScaling;
    return(TRUE);
  }
  else
    return(FALSE);
  #else
  trace_new(TRACE_MPG|TRACE_LEVEL_ALWAYS, "VIDEO: Scaling mode not supported except on Colorado with macroblock ucode!\n");
  return(FALSE);
  #endif
}

/********************************************************************/
/*  FUNCTION:    gen_video_get_startup_timeouts                     */
/*                                                                  */
/*  PARAMETERS:  pTimeout1 - pointer to storage for returned value  */
/*                           of first video timeout.                */
/*               pTimeout2 - pointer to storage for returned value  */
/*                           of second video timeout.               */
/*               pTimeout3 - pointer to storage for returned value  */
/*                           of third video timeout.                */
/*                                                                  */
/*  DESCRIPTION: This call returns the current values of the 3      */
/*               video startup timeouts.                            */
/*                                                                  */
/*  RETURNS:     TRUE on success, FALSE on failure                  */
/*                                                                  */
/*  CONTEXT:     May be called in any context                       */
/*                                                                  */
/********************************************************************/
bool gen_video_get_startup_timeouts(u_int32 *pTimeout1, u_int32 *pTimeout2, u_int32 *pTimeout3)
{
  if(pTimeout1 && pTimeout2 && pTimeout3)
  {
    *pTimeout1 = guVideo1Timeout;
    *pTimeout2 = guVideo2Timeout;
    *pTimeout3 = guVideo3Timeout;
    return(TRUE);
  }
  else
    return(FALSE);
}

/********************************************************************/
/*  FUNCTION:    gen_video_set_startup_timeouts                     */
/*                                                                  */
/*  PARAMETERS:  uTimeout1 - Value to use for the first video       */
/*                           startup timeout in milliseconds.       */
/*               uTimeout2 - Value to use for the second video      */
/*                           startup timeout in milliseconds.       */
/*               uTimeout3 - Value to use for the third video       */
/*                           startup timeout in milliseconds.       */
/*                                                                  */
/*  DESCRIPTION: This call sets the values of the 3 timeouts used   */
/*               when starting live video.                          */
/*               uTimeout1 represents the maximum time to wait for  */
/*               data to begin flowing from the demux after         */
/*               enabling the video channel.                        */
/*               uTimeout2 represents the maximum time to wait for  */
/*               the first picture to be decoded.                   */
/*               uTimeout3 represents the maximum time to wait for  */
/*               video decode to get into sync.                     */
/*               All times describe durations from the point that   */
/*               WaitForLiveVideoToStart is called (effectively the */
/*               same as the time gen_video_play is called) so      */
/*               each timeout must be longer than the previous one. */
/*                                                                  */
/*  RETURNS:     TRUE on success, FALSE on failure                  */
/*                                                                  */
/*  CONTEXT:     May be called in any context                       */
/*                                                                  */
/********************************************************************/
bool gen_video_set_startup_timeouts(u_int32 uTimeout1, u_int32 uTimeout2, u_int32 uTimeout3)
{
  if((uTimeout1 < uTimeout2) && (uTimeout2 < uTimeout3))
  {
    guVideo1Timeout = uTimeout1;
    guVideo2Timeout = uTimeout2;
    guVideo3Timeout = uTimeout3;
    return(TRUE);
  }
  else
    return(FALSE);
}


/*******************************************************************/
/* Function: gen_video_init()                                      */
/* Parameters: void                                                */
/* Return: TRUE for no error   FALSE for failure                   */
/* Remarks:  Performs any initialization for the driver.           */
/*******************************************************************/
bool gen_video_init(bool ReInit, gen_callback_frame_t frame_callback, gen_callback_time_code_notify time_code_notify)
{
    bool ks;
    bool bAudDisabled;
    #ifdef DRIVER_INCL_CONFMGR
    sabine_config_data *pCfg;
    #endif
    int      iLoop;
    int      iWait;
    static bool FirstTime = TRUE;

    /* Create semVideoPlay for decode idle signalling */
    semVideoPlay = sem_create(0,"VPS");
    #ifdef DEBUG
    if(semVideoPlay == 0) 
       trace_new(TRACE_MPG|TRACE_LEVEL_ALWAYS,"VIDEO:Initialization of semVideoPlay FAILED!!!\n");
    #endif
    
    pgnFieldCount = &gnFieldCount;  /* Problem with VxWorks compiler */

    gen_current_time_code.hour   = 0;
    gen_current_time_code.minute = 0;
    gen_current_time_code.second = 0;
    gen_current_time_code.frame  = 0;

    trace_new(TRACE_MPG|TRACE_LEVEL_2,"MPG_BASE =%x\n", MPG_BASE);

    #ifdef DRIVER_INCL_CONFMGR
    if ( (pCfg = config_lock_data()) == NULL ){
        trace_new(TRACE_MPG|TRACE_LEVEL_ALWAYS,"VIDEO:config_lock_data() Faileded\n");
        FramesPerSecond = 30;
    }
    else{
        if (pCfg->video_standard == NTSC)
            FramesPerSecond = 30;
        else
            FramesPerSecond = 25;
        AllowAudioSync = (bool) pCfg->audio_sync;
        config_unlock_data(pCfg);
    }
    #else
    /* for NTSC, FramesPerSecond should be set to 30, otherwise 25 */
    FramesPerSecond = (VIDEO_OUTPUT_STANDARD_DEFAULT == NTSC) ? 30 : 25;
    /* use the default setting from software config file */
    AllowAudioSync = CONF_AUDIO_SYNC;
    #endif

    /* Clear the error condition counters */
    for(iLoop = 0; iLoop < NUM_DSR_ERROR_CODES; iLoop++)
      guDSRErrorCounts[iLoop] = 0;
      
    guVideoBufferOverflowCount  = 0;
    guVideoBufferUnderflowCount = 0;
    
    /* Carry out first time initialisation - task and object creation */
    if (FirstTime){
        if ( (VideoQueueID = qu_create(10, "VIDQ")) == 0){
            trace_new(TRACE_MPG|TRACE_LEVEL_ALWAYS,"VIDEO:Queue Create Failed\n");
            error_log(RC_MPG_QCREATE_FAILED + ERROR_FATAL);
            return FALSE;
        }
        *pVideoProcessStarted = FALSE;
        if ( (VideoProcessID = task_create((PFNTASK) video_process, 0, 0,
                      VID_TASK_STACK_SIZE, VID_TASK_PRIORITY, VID_TASK_NAME)) == 0){
            trace_new(TRACE_MPG|TRACE_LEVEL_ALWAYS,"VIDEO:Process Create Failed\n");
            error_log(RC_MPG_PCREATE_FAILED + ERROR_FATAL);
            return FALSE;
        }

        if (int_register_isr(INT_MPEGSUB, (PFNISR) video_IRQHandler, FALSE, FALSE, (PFNISR *)&pOldVideofnHandler) != RC_OK){
            trace_new(TRACE_MPG|TRACE_LEVEL_ALWAYS,"VIDEO:Error: int_register_isr failed\n");
            error_log(RC_MPG_REGISR_FAILED + ERROR_FATAL);
            return FALSE;
        }

        /************************************************************************************/
        /* Load the video microcode here. Note that various different flavours of microcode */
        /* exist for Colorado (CX2249x) depending upon the error concealment algorithms     */
        /* required. Only the picture-based concealment type has different versions for     */
        /* silicon rev F vs. revs C, D and D1. For all other types, a single microcode      */
        /* version works with all revisions of the chip.                                    */
        /*                                                                                  */
        /* Note, also, that only picture-based error concealment is currently available     */
        /* for Wabash and Hondo.                                                            */
        /************************************************************************************/
        ks = LoadMicrocode(1, SELECTED_VIDEO_MICROCODE, 
                           SELECTED_VIDEO_MICROCODE + SELECTED_VIDEO_MICROCODE_SIZE);
                           
        if(!ks)
        {
            trace_new(TRACE_MPG|TRACE_LEVEL_ALWAYS,"Microcode Load Failed\n");
            error_log(RC_MPG_MC_LOAD_FAILED + ERROR_FATAL);
        }
        
        /* Set the initial I, P and B buffer pointers */
        InitMPEGVideo();
        
        /* Initialise our shadow pointers to the vcore I, P and B buffers */
        gpIBuffer = (u_int8 *)HWBUF_DEC_I_ADDR;
        gpPBuffer = (u_int8 *)HWBUF_DEC_P_ADDR;
        gpBBuffer = (u_int8 *)HWBUF_DEC_B_ADDR;
        gdwLastDecodedAnchor = MPG_VID_I_COMPLETE;
        
        /* Ensure that both anchor picture buffers are marked as undisplayable */
        gsDecodedImages[0].dwStartingMode = PLAY_STILL_STATE_NO_DISP;
        gsDecodedImages[1].dwStartingMode = PLAY_STILL_STATE_NO_DISP;
    }
    
    /* Reset the video decoder before we do anything else */
    *glpIntMask &= ~VIDEO_ISR_BITS;

    /* disable audio */
    aud_before_reset();
    bAudDisabled = gbRestoreAudioAfterReset;

    /* Write the reset command to the decoder */
    ks = critical_section_begin();
    *((LPREG)glpCtrl0) &= ~MPG_VIDEO_MC_VALID;
    *((LPREG)glpMpgCmd) = MPG_COMMAND_PAUSE;
    *((LPREG)glpCtrl0) |= (MPEG_CORE_RESET_HIGH|MPEG_CORE_RESET);
    critical_section_end(ks);

    /* Wait up to a second for the reset to complete */
    iWait = 0;
    while(!(*((LPREG)glpCtrl0) & MPEG_CORE_RESET) && (iWait < 1000))
    {
      iWait+= 20;
		  task_time_sleep(20);
    }
    
	  if(!(*((LPREG)glpCtrl0) & MPEG_CORE_RESET))
    {
      /* Video core failed to complete reset within 20mS! */
      trace_new(TRACE_MPG|TRACE_LEVEL_ALWAYS,"VIDEO: ERROR! Decoder failed to reset!\n");
      error_log(ERROR_WARNING);
	  }

    /* Clear the decoder reset */
    ks = critical_section_begin();
    *((LPREG)glpCtrl0) &= ~(MPEG_CORE_RESET|MPEG_CORE_RESET_HIGH);
    *((LPREG)glpCtrl0) |= MPG_VIDEO_MC_VALID;
    critical_section_end(ks);
   
    trace_new(TRACE_MPG|TRACE_LEVEL_2,"VIDEO: Decoder reset complete\n");

    /* Initialise the encoded video buffer pointers */
    ks = critical_section_begin();
    *DPS_VIDEO_READ_PTR_EX(gDemuxInstance) = HWBUF_ENCVID_ADDR;
    *DPS_VIDEO_START_ADDR_EX(gDemuxInstance) = HWBUF_ENCVID_ADDR;
    SetWritePtr((MPG_ADDRESS) HWBUF_ENCVID_ADDR);

    *DPS_VIDEO_END_ADDR_EX(gDemuxInstance) = HWBUF_ENCVID_ADDR + HWBUF_ENCVID_SIZE - 1;
    
    /* Clear then enable the interrupts we are interested in */
    *glpIntStatus = VIDEO_ISR_BITS;
    *glpIntMask |= (VIDEO_ISR_BITS & ~MPG_VIDEO_DEC_INTERRUPT6);
    
    *glpErrorMask |= MPG_BITSTREAM_ERROR;
    critical_section_end(ks);

    #if (EMULATION_LEVEL == FINAL_HARDWARE)
    trace_new(TRACE_MPG|TRACE_LEVEL_3, "VIDEO: Waiting for video task to start up....\n");
    while (*pVideoProcessStarted == FALSE)
      task_time_sleep(200);
    trace_new(TRACE_MPG|TRACE_LEVEL_3, "VIDEO: Task started.\n");
    #endif

   /* Ensure that we have enabled the MPEG interrupt */
   int_enable(INT_MPEGSUB);
   
   /* Decode a few small, black MPEG stills to flush the decoder */
   for(iLoop = NUM_STILLS_FOR_INIT_FLUSH; iLoop > 0; iLoop--)
     gen_video_decode_blank();

   /* enable audio */
   aud_after_reset();
   gbRestoreAudioAfterReset = bAudDisabled;

   gVideoFrameCallBack = frame_callback;
   gTimeCodeCallBack   = time_code_notify;
   FirstTime = FALSE;
   return TRUE;
}


/********************************************************************/
/*  FUNCTION:    gen_video_set_frame_callback                       */
/*                                                                  */
/*  PARAMETERS:  pfnCallback - new callback function pointer        */
/*                                                                  */
/*  DESCRIPTION: Set the callback function called whenever an image */
/*               is decoded. If passed NULL, this disables callbacks*/
/*                                                                  */
/*  RETURNS:     The previous callback function pointer.            */
/*                                                                  */
/*  CONTEXT:     May be called in any context                       */
/*                                                                  */
/********************************************************************/
gen_callback_frame_t gen_video_set_frame_callback(gen_callback_frame_t pfnCallback)
{
  gen_callback_frame_t pfnOldCallback;
  
  pfnOldCallback = gVideoFrameCallBack;
  gVideoFrameCallBack = pfnCallback;
  return(pfnOldCallback);
}

/********************************************************************/
/*  FUNCTION:    gen_video_get_error_stats                          */
/*                                                                  */
/*  PARAMETERS:  pOverflowCount  - pointer to receive current       */
/*                                 overflow count.                  */
/*               pUnderflowCount - pointer to receive current       */
/*                                 underflow count.                 */
/*               pDSRErrors      - pointer to an array of           */
/*                                 NUM_DSR_ERROR_CODES entries which*/
/*                                 will receive counts of each error*/
/*                                                                  */
/*  DESCRIPTION: Return information on the number of times          */
/*               particular error conditions have been recorded     */
/*               since the last call to gen_video_init. Passing any */
/*               pointer as a NULL will result in that statistic    */
/*               not being returned.                                */
/*                                                                  */
/*  RETURNS:     Nothing                                            */
/*                                                                  */
/*  CONTEXT:     May be called in any context                       */
/*                                                                  */
/********************************************************************/
void gen_video_get_error_stats(u_int32 *pOverflowCount, 
                               u_int32 *pUnderflowCount,
                               u_int32 pDSRErrors[])
{
  bool bCS;
  int  iLoop;
  
  bCS = critical_section_begin();
  
  if(pOverflowCount)
    *pOverflowCount = guVideoBufferOverflowCount;
    
  if(pUnderflowCount)
    *pUnderflowCount = guVideoBufferUnderflowCount;
    
  if(pDSRErrors)
  {
    for(iLoop = 0; iLoop < NUM_DSR_ERROR_CODES; iLoop++)
      pDSRErrors[iLoop] = guDSRErrorCounts[iLoop];
  }
  
  critical_section_end(bCS);  
}

#ifdef DEBUG
u_int32 check_free_mem()
{
    u_int32 MemSize = 0x200000, MemInc = 0x10000;
    char *pMem = NULL;

    for (; MemSize > 0; MemSize -= MemInc){
        if ( (pMem = (char *) mem_malloc(MemSize)) != NULL)
            break;
    }

    if (pMem != NULL)
        mem_free(pMem);
    return MemSize;
}
#endif

/*-------------------------**
** VideoEnableSync         **
** Desc.                   **
**     Enables video syc.  **
** Params                  **
**     None                **
** Returns                 **
**     Previous sync state **
**-------------------------*/
bool VideoEnableSync()
{
    #ifndef DRIVER_INCL_NDSTESTS
    bool bRetcode;

    isr_trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: Enable Sync\n",0 ,0);
    
    bRetcode = (*((LPREG)glpCtrl0) & MPG_ENABLE_SYNC) ? TRUE : FALSE;

#ifndef WABASH_AVSYNC_CLKREC_DISABLED
    *( (LPREG)glpCtrl0) |= MPG_ENABLE_SYNC;
#endif
    return(bRetcode);
    #else
    return(FALSE);
    #endif
}

/*-------------------------**
** VideoDisableSync        **
** Desc.                   **
**     Disables video syc. **
** Params                  **
**     None                **
** Returns                 **
**     Previous sync state **
**-------------------------*/
bool VideoDisableSync()
{
    bool bRetcode;
    
    cs_trace(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: Disable Sync\n", 0, 0);
    bRetcode = (*((LPREG)glpCtrl0) & MPG_ENABLE_SYNC) ? TRUE : FALSE;
    *( (LPREG)glpCtrl0) &= ~MPG_ENABLE_SYNC;
    return(bRetcode);
}

/*-------------------------**
** VideoEnableDec          **
** Desc.                   **
**     Enables video decode**
** Params                  **
**     None                **
** Returns                 **
**     Nothing             **
**-------------------------*/
void VideoEnableDec()
{
    *( (LPREG)glpCtrl1) |= MPG_ENC_VIDEO_ENABLE;
}

/*-------------------------**
** VideoDisableDec         **
** Desc.                   **
**     Enables video syc.  **
** Params                  **
**     None                **
** Returns                 **
**     Nothing             **
**-------------------------*/
void VideoDisableDec()
{
    *( (LPREG)glpCtrl1) &= ~MPG_ENC_VIDEO_ENABLE;
}

/*******************************************************************/
/* Function: StopLiveVideo                                         */
/* Parameters: void                                                */
/* Return: void                                                    */
/* Remarks:  Stops live MPEG play.                                 */
/*******************************************************************/
void StopLiveVideo(bool bWaitForI)
{
  bool bRetcode;

  /* Tell the video decoder to stop */
  #if VIDEO_UCODE_TYPE == VMC_EXTRA_ERROR_RECOVERY
  
  /* If using the Canal+ version of the microcode, there are 2 options */
  /* 1. Stop immediately or                                            */
  /* 2. Stop after decoding the next I picture.                        */
  
  if(bWaitForI)
  {
    video_send_hardware_command(MPG_COMMAND_PAUSE_ON_I);
    bRetcode = WaitForVideoIdle(MPEG_I_IDLE_TIMEOUT);
  }  
  else  
  {
    video_send_hardware_command(MPG_COMMAND_PAUSE);
    bRetcode = WaitForVideoIdle(MPEG_IDLE_LOOP_TIMEOUT);
  }  
  #else /* Normal, non-Canal+ case */
  
  /* Without the Canal+ microcode, we don't have the ability to pause */
  /* on an I picture.                                                 */
  video_send_hardware_stop_command();
  bRetcode = WaitForVideoIdle(MPEG_IDLE_LOOP_TIMEOUT);
  
  #endif /* VIDEO_UCODE_TYPE == VMC_EXTRA_ERROR_RECOVERY */
  

  /* Flush the internal VBV buffer. If we don't do this, stills sent after  */
  /* stopping motion video are very occasionally ignored by the video core. */
  /* For this to happen, the core had to have read 00 00 01 from the stream */
  /* just before being stopped. Flushing the VBV buffer, gets rid of this   */
  /* data and allows a future still decode to proceed correctly.            */
  video_send_hardware_command(MPG_COMMAND_FLUSH_VBV);

  bRetcode = WaitForVideoIdle(MPEG_IDLE_LOOP_TIMEOUT);

  /* Did we get an idle interrupt? */
  if (!bRetcode)
  {
    trace_new(TRACE_MPG|TRACE_LEVEL_ALWAYS, "VIDEO: **** Video core stalled. Sending sequence header\n");
    
    SendFrameHeader();
   
    bRetcode = WaitForVideoIdle(MPEG_IDLE_LOOP_TIMEOUT);
    
    /* Did we get the idle interrupt this time? */
    if (!bRetcode)
    {
    
      /* No? Now we are really in trouble. Try kicking the vcore really hard */
      ResetVCore();
     
      /* Wait a while and look for idle interrupts */
      bRetcode = WaitForVideoIdle(MPEG_IDLE_LOOP_TIMEOUT);
      /* Did we get the idle interrupt this time? */
      if (!bRetcode)
      {
        /* Here we are toast. The video core has not reentered idle after several attempts */
        trace_new(TRACE_MPG|TRACE_LEVEL_ALWAYS, "VIDEO: **** No idle interrupt after video core reset!\n");
        error_log(MOD_MPG|ERROR_FATAL);
      }  
    }  
  }

  /* Don't clear the encoded video buffer if we have replay */
  /* optimisation enabled.                                  */
  
  
  SetWritePtr((MPG_ADDRESS)*DPS_VIDEO_READ_PTR_EX(gDemuxInstance));

  if (*DPS_VIDEO_WRITE_PTR_EX(gDemuxInstance) != *DPS_VIDEO_READ_PTR_EX(gDemuxInstance))
      isr_trace_new(TRACE_MPG|TRACE_LEVEL_ALWAYS,"VIDEO: Error MPG R/W pointers not equal!!!\n",0,0);
      
  #ifdef OPENTV
  /* Turn off the PAL bit in the DRM to ensure still scaling looks as good as possible */
  /* There is a bug in CX22490/1/6 that makes upscaled stills look bad in PAL mode     */
  /* unless this bit is clear but the bit must be set correctly while motion video is  */
  /* playing or AV sync can be affected.                                               */
  CNXT_SET_VAL(glpDrmControl, DRM_CONTROL_OUTPUT_FORMAT_MASK, 0);
  #endif  /* OPENTV */
} 


/********************************************************************/
/*  FUNCTION:    StartLiveVideo                                     */
/*                                                                  */
/*  PARAMETERS:  uState - PLAY_LIVE_STATE or PLAY_LIVE_STATE_NOSYNC */
/*                                                                  */
/*  DESCRIPTION: Start playing live video                           */
/*                                                                  */
/*  RETURNS:     As for WaitForLiveVideoToStart                     */
/*                                                                  */
/*  CONTEXT:     Must only be called from the video task.           */
/*                                                                  */
/********************************************************************/
static CNXT_VID_STATUS StartLiveVideo(u_int32 uState, bool *pbStillDecoded)
{
  u_int32 uMpgCommand = MPG_COMMAND_PLAY;
  bool    ks;
  CNXT_VID_STATUS eRetcode = CNXT_VID_STATUS_OK;
  #if (BLANK_ON_VIDEO_START == YES)
  bool    bBlanked;
  #ifdef OPENTV_12
  u_int32 uSavedBackground;
  #endif
  #endif
//*****Green screen patch start***************
  #ifdef DEBUG
  #if (BLANK_ON_VIDEO_START == YES)
//*****Green screen patch end***************
  LPREG lpFrameDrop = (LPREG)(MPG_FRAME_DROP_CNT_REG);
  #endif
  #endif

  
  #ifdef OPENTV_12    
  /* Tell the osdlib driver to release the B buffer */
  vidDecodeBuffersNeeded();
  #endif

  #ifdef OPENTV
  /* Turn on the PAL bit in the DRM to ensure sync works correctly. This needs to be  */
  /* on when decoding live video in PAL mode. We turn it off if motion video is not   */
  /* playing since there is a bug in CX22490/1/6 that makes upscaled stills look bad  */
  /* in PAL mode unless this bit is clear.                                            */
  if(gnOsdMaxHeight == 576)
    CNXT_SET_VAL(glpDrmControl, DRM_CONTROL_OUTPUT_FORMAT_MASK, 1);
  #endif 
               
  *pbStillDecoded = FALSE;
  
  #if (BLANK_ON_VIDEO_START == YES)
  /* Temporarily blank video during startup. for OpenTV 1.2, we only do this if the EPG */
  /* application is running since we set the background colour to the colour it uses.   */
  #ifdef OPENTV_12
  if (gbRunningEPG)
  {
  #endif
    trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: Blanking during video startup\n");
    
    ks = critical_section_begin();
    bBlanked = (*((LPREG)glpDrmControl) & DRM_MPEG_ENABLE) ? FALSE : TRUE;
 
    if (!bBlanked)
    {
      #ifdef OPENTV_12
      /* Set the graphics background to the EPG background colour */
      uSavedBackground = *((LPREG)glpDrmBackground);
      *((LPREG)glpDrmBackground) = EPG_BACKGROUND_COLOUR;
      #endif
      /* Blank the video plane */
      *((LPREG)glpDrmControl) &= ~DRM_MPEG_ENABLE;
    }  
    critical_section_end(ks);
  #ifdef OPENTV_12
  }  
  else
  {
    trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: EPG not running - not blanking video plane\n");
    bBlanked = TRUE;
  }  
  #endif  
  #endif
  
  /* Flush the encoded video buffer before we start */
  SetWritePtr((MPG_ADDRESS)*DPS_VIDEO_READ_PTR_EX(gDemuxInstance));
  
  /* Reset our displayed picture counter */
  ks = critical_section_begin();
  giDRMDisplayPicture = 0;
  critical_section_end(ks);
  
  /*  If sync is already enabled, don't muck with it. Else enable sync unless state != LIVE */               
  if(uState == PLAY_LIVE_STATE && !(*((LPREG)glpCtrl0) & MPG_ENABLE_SYNC)){
	VideoEnableSync();}
                 
  if(uState != PLAY_LIVE_STATE){               
    VideoDisableSync();	}	
        
  osdSetMpgScaleDelay(0);
  trace_new(TRACE_MPG|TRACE_LEVEL_2,"VIDEO:video_play live mode!!!!\n");
  if ((PlayState != PLAY_LIVE_STATE) && (PlayState != PLAY_LIVE_STATE_NO_SYNC))
  {
      video_send_hardware_command(uMpgCommand);
  }

  PlayState = uState;
    
  #if (FAST_RETURN_FROM_PLAY == NO)
  eRetcode = WaitForLiveVideoToStart((uState == PLAY_LIVE_STATE) ? TRUE : FALSE);
  #endif
  
  #if (BLANK_ON_VIDEO_START == YES)
  /* Unblank the video if necessary */
  if (!bBlanked)
  {
    #ifdef DEBUG
    trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: Unblanking video plane after video started. Rpt %d, Skip %d\n",
              CNXT_GET_VAL(lpFrameDrop, MPG_FRAME_DROP_CNT_REPEAT_MASK),
              CNXT_GET_VAL(lpFrameDrop, MPG_FRAME_DROP_CNT_DROP_MASK));
    #endif          
    
    ks = critical_section_begin();

    #if (FAST_RETURN_FROM_PLAY == NO)
    *((LPREG)glpDrmControl) |= DRM_MPEG_ENABLE;
    #ifdef OPENTV_12
    /* If noone changed the background colour, revert to the original one */
    if(*((LPREG)glpDrmBackground) == EPG_BACKGROUND_COLOUR)
      *((LPREG)glpDrmBackground) = uSavedBackground;
    #endif
    #else
    /* When using fast return, set a flag to tell our interrupt code to unblank automatically */
    gbDeferredUnblank = TRUE;
    #endif
    
    critical_section_end(ks);
  }
  else
  {
    trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: No need to unblank video plane\n");
  }
  #endif
  
  return(eRetcode);
}

/*******************************************************************/
/* Function: ReadWritePtrReset                                     */
/* Parameters: void *arg                                           */
/* Return: void                                                    */
/* Remarks:  Resets the video Read and Write pointers safely.      */
/*******************************************************************/
 void ReadWritePtrReset()
{
  SetWritePtr((MPG_ADDRESS)*DPS_VIDEO_READ_PTR_EX(gDemuxInstance));

  if (*DPS_VIDEO_READ_PTR_EX(gDemuxInstance) != *DPS_VIDEO_WRITE_PTR_EX(gDemuxInstance))
        trace_new(TRACE_MPG|TRACE_LEVEL_ALWAYS,"VIDEO: Error MPG R/W pointers not equal!!!\n",0 ,0);
}

/*----------------------------------------------------------------------**
** ResetVCore                                                           **
** Desc.                                                                **
**     Resets the video core                                            **
** Parsms                                                               **
**     None                                                             **
** Returns                                                              **
**     Nothing                                                          **
**----------------------------------------------------------------------*/
void ResetVCore()
{
    bool ks;                   
    bool bAudDisabled;
    int wait = 0;
	 
    not_interrupt_safe();
    
    uVCoreResetCount++;
    
    trace_new(TRACE_AUD|TRACE_LEVEL_2,"****VIDEO: MPEG core reset! r/w 0x%08x/0x%08x\n",
        *((LPREG)DPS_VIDEO_READ_PTR_EX(gDemuxInstance)),
        *((LPREG)DPS_VIDEO_WRITE_PTR_EX(gDemuxInstance)));

    *glpIntMask &= ~VIDEO_ISR_BITS;
    aud_before_reset();

    ks = critical_section_begin();

    bAudDisabled = gbRestoreAudioAfterReset;
    
    *((LPREG)glpCtrl0) &= ~MPG_VIDEO_MC_VALID;
    *((LPREG)glpMpgCmd) = MPG_COMMAND_PAUSE;
    *((LPREG)glpCtrl0) |= (MPEG_CORE_RESET_HIGH|MPEG_CORE_RESET);

    critical_section_end(ks);


    while(!(*((LPREG)glpCtrl0) & MPEG_CORE_RESET) && (wait < 1000))
    {
      wait+= 20;
		task_time_sleep(20);
    }
	 if(!(*((LPREG)glpCtrl0) & MPEG_CORE_RESET)){
      /* Video core failed to complete reset within 20mS! */
      fatal_exit(VIDEO_VCORE_HUNG);
	 }

    ks = critical_section_begin();
    *((LPREG)glpCtrl0) &= ~(MPEG_CORE_RESET|MPEG_CORE_RESET_HIGH);

    *((LPREG)glpCtrl0) |= MPG_VIDEO_MC_VALID;
    critical_section_end(ks);
   
    trace_new(TRACE_MPG|TRACE_LEVEL_1,"Reset of video/audio completed\n");
    
    ReadWritePtrReset();
    aud_after_reset();
    gbRestoreAudioAfterReset = bAudDisabled;

    trace_new(TRACE_MPG|TRACE_LEVEL_2,"****VIDEO: Reset complete r/w 0x%08x/0x%08x\n",
        *((LPREG)DPS_VIDEO_READ_PTR_EX(gDemuxInstance)),
        *((LPREG)DPS_VIDEO_WRITE_PTR_EX(gDemuxInstance)));

    ks = critical_section_begin();
    *glpIntMask |= VIDEO_ISR_BITS;
    critical_section_end(ks);
}

/********************************************************************/
/*  FUNCTION:    ProcessUserData                                    */
/*                                                                  */
/*  PARAMETERS:  uPictureType - the last decoded picture type as    */
/*                              read from the MPEG picture size reg */
/*                                                                  */
/*  DESCRIPTION: Send received user data back to a client who has   */
/*               registered a user data callback. Callbacks will    */
/*               always be triggered by a decode complete interrupt */
/*               and up to 2 callbacks may be made per decoded      */
/*               picture (to allow for ring buffer wrapping without */
/*               having to expose a ring buffer interface to the    */
/*               callback). The bMore callback parameter indicates  */
/*               whether or not a further callback for the same     */
/*               picture should be expected.                        */
/*                                                                  */
/*  RETURNS:     Nothing                                            */
/*                                                                  */
/*  CONTEXT:     Will be called in task context from the video task */
/*                                                                  */
/********************************************************************/
void ProcessUserData(u_int32 uPictureType)
{
    int len;
    u_int8 *pData;
    u_int32 uWriteIndexCopy;
    bool    ks;

    /***************************************************************/
    /* Ring buffer variable use:                                   */
    /*                                                             */
    /* UserWriteIndex is the next byte to be written to the buffer */
    /* UserReadIndex  is the next byte to be read from the buffer  */
    /*                                                             */
    /* The write pointer points to an empty location and the read  */
    /* pointer points to a full one (unless buffer is empty)       */
    /*                                                             */
    /* Buffer empty when read index == write index                 */
    /* Buffer full when write index == (read index - 1)            */
    /*                                                             */
    /***************************************************************/
    
    /* Take a snapshot of the current user data ring buffer write  */
    /* index.                                                      */
    ks = critical_section_begin();
    uWriteIndexCopy = UserWriteIndex;
    critical_section_end(ks);
    
    /* If the buffer is empty, do nothing */
    if(uWriteIndexCopy == UserReadIndex)
      return;

    /* Where's the new data? */
    pData = (u_int8 *) &UserBuffer[UserReadIndex];
      
    /* Figure out how much data is in the ring buffer and whether or */
    /* not the data spans a wrap (which will mean we need to make 2  */
    /* callbacks).                                                   */
    
    if(uWriteIndexCopy > UserReadIndex)
    {
      /* Data does not wrap around the end of the buffer */
      if (gUserDataCallBack)
      {
        len = (int) (uWriteIndexCopy - UserReadIndex);
        gUserDataCallBack(pData, (u_int32) len, uPictureType, FALSE);
      }  
    }
    else
    {
      /* Data wraps around the end of the buffer so make 2 callbacks */
      if (gUserDataCallBack)
      {
        len = (int) (LOCAL_VIDEO_USER_BUFFER_SIZE - UserReadIndex);
        gUserDataCallBack(pData, (u_int32) len, uPictureType, TRUE);
        gUserDataCallBack((u_int8 *)&UserBuffer[0], UserWriteIndex, uPictureType, FALSE);
      }  
    }  
    
    /* Update the read pointer appropriately */
    UserReadIndex = uWriteIndexCopy;
}

/*******************************************************************/
/* Function: video_process                                         */
/* Parameters: void *arg                                           */
/* Return: void                                                    */
/* Remarks:  This is the video task. It waits on a queue then      */
/*           process the commands and calls the command done       */
/* callback if one was provided                                    */
/*******************************************************************/
#if (defined PERIODIC_VIDEO_TRACE) && (defined DEBUG)
  #define VIDEO_RECEIVE_TIMEOUT 500
#else  
  #define VIDEO_RECEIVE_TIMEOUT KAL_WAIT_FOREVER
#endif

void video_process(void *arg)
{
    u_int32 message[4];
    int     iRetcode;
    #if (defined PERIODIC_VIDEO_TRACE) && (defined DEBUG)
    LPREG lpFrameDrop = (LPREG)(MPG_FRAME_DROP_CNT_REG);
    #endif

    while(1)
    {
      /* Set a flag to ourselves then wait for a message telling us to do something */
        *pVideoProcessStarted = TRUE;
      iRetcode = qu_receive(VideoQueueID, VIDEO_RECEIVE_TIMEOUT, message);
      if(iRetcode == RC_OK)
      {
        if (gHandleMsgFunc)
        {
            if (TRUE == gHandleMsgFunc(message)){
                continue;
            }
        }
        video_process_msg(message);
      }        
      #if (defined PERIODIC_VIDEO_TRACE) && (defined DEBUG)
      else
      {
        /* If we waited 0.5s and no-one asked us to do anything, dump some */
        /* statistics and go back to wait for another message.             */
        trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: R %08x, W %08x, Rpt %2d, Skip %2d\n",
                  *DPS_VIDEO_READ_PTR_EX(gDemuxInstance),
                  *DPS_VIDEO_WRITE_PTR_EX(gDemuxInstance),
                  CNXT_GET_VAL(lpFrameDrop, MPG_FRAME_DROP_CNT_REPEAT_MASK),
                  CNXT_GET_VAL(lpFrameDrop, MPG_FRAME_DROP_CNT_DROP_MASK));
        trace_new(TRACE_MPG|TRACE_LEVEL_2, "PCR %08x, STC S %08x %08x, VPS %02x, VPTS %08x, Bitstream PTS %1x%08x\n",
                  *glpPCRLow,
                  *glpSTCSnapLo,
                  *glpSTCLo,
                  *((LPREG)0x380006C0),
                  *glpVPTSLo,
                  (*((LPREG)(HWBUF_PCRPTS_ADDR+4))&0x1),
                  *((LPREG)HWBUF_PCRPTS_ADDR));
        trace_new(TRACE_MPG|TRACE_LEVEL_2, "Anchor PTS %1x%08x Running PTS %1x%08x\n",
                  (*((LPREG)(HWBUF_PCRPTS_ADDR+0xc))&0x1),
                   *((LPREG)HWBUF_PCRPTS_ADDR+8),
                  (*((LPREG)(HWBUF_PCRPTS_ADDR+0x14))&0x1),
                   *((LPREG)HWBUF_PCRPTS_ADDR+0x10));
        trace_new(TRACE_MPG|TRACE_LEVEL_2, "       MPG_SIZE 0x%08x, state %s\n", *(LPREG)glpMpgPicSize, GetStateDescription(PlayState));
       }
       #endif /* DEBUG and PERIODIC_VIDEO_TRACE */
    } /* endwhile */
    
    return; /* Should never get here! */
} 
       
/*******************************************************************/
/* Function: video_process                                         */
/* Parameters: void *arg                                           */
/* Return: void                                                    */
/* Remarks:  This is the video task. It waits on a queue then      */
/*           process the commands and calls the command done       */
/* callback if one was provided                                    */
/*******************************************************************/
void video_process_msg(u_int32 *message)
{                                                            
   bool ks; 
   CNXT_VID_STATUS eStatus = CNXT_VID_STATUS_OK;
   static bool bStillDecoded = FALSE;
   #ifdef DEBUG
   DumpVideoMessage(message);
   #endif
   
   switch(message[0])
   {
       /* This gets sent by gen_flush_still_data to say all data sent */
       /* Wait for video idle to indicate that the decode has         */
       /* completed then return.                                      */
       case VIDEO_COMMAND_FLUSHED:
         eStatus = WaitForStillDecodeToComplete(VIDEO_STILL_DECODE_TIMEOUT);
         PlayState = PLAY_OFF_STATE;
         send_video_command_done(message, eStatus);
         post_video_command_done(message, eStatus);
         break;                                                   
         
       /* This gets sent the first time through gen_send_still_data */
       /* for us to send the still command to the hardware          */
       case VIDEO_COMMAND_STILL_WAIT:
         trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: Sending still command and blocking still commands\n");
         video_send_hardware_command(MPG_COMMAND_STILL_WAIT);
         send_video_command_done(message, eStatus);
         post_video_command_done(message, eStatus);
         break;                                                       
         
       /* Flush the internal VBV buffer. This is needed before video starts to get rid */
       /* of any stale data that may be buffered within the decoder.                  */
       case VIDEO_COMMAND_FLUSH_VBV:
           trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: Flushing VBV\n");
           video_send_hardware_command(MPG_COMMAND_FLUSH_VBV);
           ks = WaitForVideoIdle(MPEG_IDLE_LOOP_TIMEOUT);
           if(!ks)
           {
             trace_new(TRACE_MPG|TRACE_LEVEL_ALWAYS, "VIDEO: Timeout out waiting for idle after flush VBV!!\n");
             eStatus = CNXT_VID_STATUS_DECODE_1_TIMEOUT;
           }
           SetWritePtr((MPG_ADDRESS)*DPS_VIDEO_READ_PTR_EX(gDemuxInstance));
           
           /* This is only ever sent synchronously so we don't need to call post_video_command_done */
           send_video_command_done(message, eStatus);
           break;
         
         /* This gets sent by the ISR to ask us to issue continue    */
         /* if an error occurs and the core went idle                */
       case VIDEO_COMMAND_CONTINUE:
         video_send_hardware_command(MPG_COMMAND_CONTINUE);
         break;
         
       case VIDEO_COMMAND_USER_DATA:
           ProcessUserData(message[1]);
           send_video_command_done(message, eStatus);
           break;
       case VIDEO_COMMAND_PAUSE:
           trace_new(TRACE_MPG|TRACE_LEVEL_2,"VIDEO: Pause command received. Read/write pointers 0x%08x/0x%08x\n",
                  *DPS_VIDEO_READ_PTR_EX(gDemuxInstance),
                  *DPS_VIDEO_WRITE_PTR_EX(gDemuxInstance));
           
           /* message[1] contains TRUE or FALSE to indicate whether we must stop immediately   */
           /* or wait for the next I picture. Generally this will be FALSE (stop now) but in   */
           /* Canal+ builds the gen_video_pause_at_next_I function sets the value TRUE.        */
           /* Note that this feature is only available if you use the Canal+ microcode variant */
	         if (PlayState == PLAY_LIVE_STATE)
               StopLiveVideo(message[1]);
	         else if (PlayState == PLAY_DRIP_STATE)
	           video_send_hardware_stop_command();
           
	         send_video_command_done(message, eStatus);
	         post_video_command_done(message, eStatus);
	         break;

       case VIDEO_COMMAND_RESUME:
           trace_new(TRACE_MPG|TRACE_LEVEL_2,"VIDEO: Resume command received. Read/write pointers 0x%08x/0x%08x\n",
                  *DPS_VIDEO_READ_PTR_EX(gDemuxInstance),
                  *DPS_VIDEO_WRITE_PTR_EX(gDemuxInstance));
	         if (PlayState == PLAY_LIVE_STATE)
           {
	           video_send_hardware_command(MPG_COMMAND_PLAY);
             #ifdef DRIVER_INCL_GENDMXC
	           gen_dmx_control_av_channel((u_int32) 0, (u_int32) VIDEO_CHANNEL, (gencontrol_channel_t) GEN_DEMUX_ENABLE);
             #else
	           if (cnxt_dmx_channel_control(gDemuxInstance, (u_int32) VIDEO_CHANNEL, 
					        (gencontrol_channel_t) GEN_DEMUX_ENABLE) != DMX_STATUS_OK) 
             {
               trace_new(DPS_ERROR_MSG,"DEMUX:control_channel failed\n");
             }
            #endif
	         }
	         else 
           {
             if (PlayState == PLAY_DRIP_STATE) 
             {
               video_send_hardware_command(MPG_COMMAND_CONTINUE);
               send_video_command_done(message, eStatus);
               post_video_command_done(message, eStatus);
  	         }
           }  
	         break;

       case VIDEO_COMMAND_STOP:
           #ifdef DEFERRED_UNBLANK            
           /* Prevent any pending "unblanks" from happening */
           ks = critical_section_begin();
           gbDeferredUnblank = FALSE;
           critical_section_end(ks);
           #endif /* DEFERRED_UNBLANK */
          
           trace_new(TRACE_MPG|TRACE_LEVEL_2,"VIDEO: Stop command received. Read/write pointers 0x%08x/0x%08x\n",
                  *DPS_VIDEO_READ_PTR_EX(gDemuxInstance),
                  *DPS_VIDEO_WRITE_PTR_EX(gDemuxInstance));          
           if (PlayState == PLAY_LIVE_STATE || PlayState == PLAY_LIVE_STATE_NO_SYNC)
           {
               StopLiveVideo(FALSE);
           }
           #ifdef OPENTV_12    
           /* Tell the osdlib driver that it can reuse the I and P buffers now */
           vidDecodeBuffersAvail();
           #endif
           
           ReadWritePtrReset();
           
           PlayState = PLAY_OFF_STATE;
           trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: Unblocking still commands\n");
           AllowStillCmd = TRUE;
           
           /* A nice little quirk of the video microcode is that the value used to decide     */
           /* which buffer will be the next one decoded into is sometimes swapped on stopping */
           /* video. We check here to see if this has happened and update our decode state    */
           /* variables appropriately.                                                        */

           #if MPG_SYNC_BIT_POSITION == MPG_SYNC_BIT_POSITION_COLORADO
           switch (*(LPREG)glpMpgPicSize & MPG_VID_MASK_DISPLAYED)
           #else
           switch (*(LPREG)glpMpgAddrExt & MPG_VID_MASK_DISPLAYED)
           #endif

           {
             case MPG_VID_I_DISPLAYED:       
               gpVideoDisplayBuffer = gpIBuffer; 
               gdwLastDecodedAnchor = MPG_VID_I_COMPLETE;
               trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: I frame displayed on stop. Buffer address 0x%08x\n", gpIBuffer);
               break;
             case MPG_VID_P_DISPLAYED:       
               gpVideoDisplayBuffer = gpPBuffer; 
               gdwLastDecodedAnchor = MPG_VID_P_COMPLETE;
               trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: P frame displayed on stop. Buffer address 0x%08x\n", gpPBuffer);
               break;
           }  
           
           trace_new(TRACE_MPG|TRACE_LEVEL_2, 
                     "VIDEO: Stop complete. Last decoded frame %d size (%d, %d). vcore displaying 0x%08x\n",
                     gdwLastDecodedAnchor,
                     gdwSrcWidth,
                     gdwSrcHeight,
                     gpVideoDisplayBuffer);
                     
           send_video_command_done(message, eStatus);
           post_video_command_done(message, eStatus);
           
           /* Read again to see if the pointer has mysteriously changed */
           #ifdef DEBUG
           
           #if MPG_SYNC_BIT_POSITION == MPG_SYNC_BIT_POSITION_COLORADO
           switch (*(LPREG)glpMpgPicSize & MPG_VID_MASK_DISPLAYED)
           #else
           switch (*(LPREG)glpMpgAddrExt & MPG_VID_MASK_DISPLAYED)
           #endif

           {
             case MPG_VID_I_DISPLAYED:       
               trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: Now I frame displayed on stop. Buffer address 0x%08x\n", gpIBuffer);
               break;
             case MPG_VID_P_DISPLAYED:       
               trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: Now P frame displayed on stop. Buffer address 0x%08x\n", gpPBuffer);
               break;
           }  
           #endif
           
           break;

       case VIDEO_COMMAND_RESET:
           trace_new(TRACE_MPG|TRACE_LEVEL_2,"VIDEO: Reset!!!!\n");
//*****Green screen patch start**************************************************************
            if ((PlayState == PLAY_LIVE_STATE)||(PlayState == PLAY_LIVE_STATE_NO_SYNC)) {
              StopLiveVideo(FALSE);
              ResetVCore();
              if (PlayState == PLAY_LIVE_STATE)
                  VideoEnableSync();
//*****Green screen patch end****************************************************
              video_send_hardware_command(MPG_COMMAND_PLAY);
              *glpIntMask |= VIDEO_ISR_BITS;
              #ifdef DRIVER_INCL_GENDMXC
              gen_dmx_control_av_channel((u_int32) 0, (u_int32) VIDEO_CHANNEL, (gencontrol_channel_t) GEN_DEMUX_ENABLE);
              #else
      	      if (cnxt_dmx_channel_control(gDemuxInstance, (u_int32) VIDEO_CHANNEL, 
                                          (gencontrol_channel_t) GEN_DEMUX_ENABLE) != DMX_STATUS_OK) 
              {
                trace_new(DPS_ERROR_MSG,"DEMUX:control_channel failed\n");
              }
              #endif
              trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: Restarted after reset\n");
           }
           else 
              ResetVCore();
           /* No video command done processing here !*/
//*****Green screen patch start***************
           bChannelChanged = 0;
           bResetComplete = 1;
//*****Green screen patch end***************
           break;

       case VIDEO_COMMAND_PLAY:
           trace_new(TRACE_MPG|TRACE_LEVEL_2,"VIDEO: Play command received. Read/write pointers 0x%08x/0x%08x\n",
                  *DPS_VIDEO_READ_PTR_EX(gDemuxInstance),
                  *DPS_VIDEO_WRITE_PTR_EX(gDemuxInstance));          
                  
           /* DEBUG ONLY! */
           #ifdef DEBUG
           #if MPG_SYNC_BIT_POSITION == MPG_SYNC_BIT_POSITION_COLORADO
           switch (*(LPREG)glpMpgPicSize & MPG_VID_MASK_DISPLAYED)
           #else
           switch (*(LPREG)glpMpgAddrExt & MPG_VID_MASK_DISPLAYED)
           #endif

           {
             case MPG_VID_I_DISPLAYED:       
               trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: Now I frame displayed on play. Buffer address 0x%08x\n", gpIBuffer);
               break;
             case MPG_VID_P_DISPLAYED:       
               trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: Now P frame displayed on play. Buffer address 0x%08x\n", gpPBuffer);
               break;
           }  
           #endif
           
           switch(message[1])
           {     /* starting mode */
           case PLAY_STILL_STATE:
           case PLAY_STILL_STATE_NO_DISP:
           
               trace_new(TRACE_MPG|TRACE_LEVEL_2,"VIDEO:video_play Stills Mode!!!!\n");
               
               bStillDecoded = TRUE;
               
               if (PlayState == PLAY_LIVE_STATE)
                   StopLiveVideo(FALSE);
               else
                   video_send_hardware_command(MPG_COMMAND_FLUSH_VBV);
                   
               #ifdef OPENTV_12    
               /* Tell the osdlib driver that it can reuse the I and P buffers now */
               vidDecodeBuffersAvail();
               #endif
               
               num_vid_leftovers = 0;
               VideoDisableSync();
               osdSetMpgScaleDelay(0);
               ReadWritePtrReset();
               trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: Unblocking still commands\n");
               AllowStillCmd = TRUE;
               
               PlayState = (int)message[1];
               trace_new(TRACE_MPG|TRACE_LEVEL_2,"VIDEO: video_play %s\n", 
                   (PlayState == PLAY_STILL_STATE)?"displayed still":"background still");
               /* Set the still error count to zero */
               StillErrorCnt = 0;
               
               send_video_command_done(message, eStatus);
               post_video_command_done(message, eStatus);
               break;

           case PLAY_LIVE_STATE:
           case PLAY_LIVE_STATE_NO_SYNC:
                   
               if (PlayState == PLAY_DRIP_STATE)
               {
                  cnxt_dmx_channel_control(gDemuxInstance, (u_int32) VIDEO_CHANNEL, 
                                           (gencontrol_channel_t) GEN_DEMUX_ENABLE);

               }

               /* I got fed up not having an easy symbol to set a breakpoint on when starting */
               /* motion video so...                                                          */
               eStatus = StartLiveVideo(message[1], &bStillDecoded);
               
               send_video_command_done(message, eStatus);
               post_video_command_done(message, eStatus);
               break;

           case PLAY_DRIP_STATE:
 
               if (PlayState == PLAY_DRIP_STATE)
                   break;

               trace_new(TRACE_MPG|TRACE_LEVEL_2,"VIDEO:video_play Drip Mode!!!!\n");               
               if (PlayState == PLAY_LIVE_STATE)
               {
                  cnxt_dmx_channel_control(gDemuxInstance, (u_int32) VIDEO_CHANNEL, 
                                           (gencontrol_channel_t) GEN_DEMUX_DISABLE);
                  StopLiveVideo(FALSE);
               }
               else
                   video_send_hardware_command(MPG_COMMAND_FLUSH_VBV);

               DripIn = DripOut = 0;
               DripBufEmpty = VideoBufEmpty = 0;

               VideoDisableSync();
               osdSetMpgScaleDelay(0);
               ReadWritePtrReset();
               trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: Unblocking drip commands\n");
               
               PlayState = (int)message[1];
               trace_new(TRACE_MPG|TRACE_LEVEL_2,"VIDEO: video_play PLAY_DRIP\n"); 
               
               /* Create a tick timer that will periodically check for space in the video buffer */
               VideoDripTick = tick_create(tick_timer_drip_callback, (void *)NULL, "DRTK");
               if (!VideoDripTick) 
               {
                   trace_new(TRACE_MPG|TRACE_LEVEL_ALWAYS, "Can't create Video Drip tick counter!\n");
                   error_log(ERROR_FATAL);
               }
    
               tick_set(VideoDripTick, DRIP_TIME_DELAY, FALSE); /* Periodic callbacks */
               tick_start(VideoDripTick);

               StartDrip = TRUE;

               send_video_command_done(message, eStatus);
               post_video_command_done(message, eStatus);
               break;
             
           default:
               trace_new(TRACE_MPG|TRACE_LEVEL_2,"VIDEO:video_play Invalid start mode!!!!\n");
               send_video_command_done(message, CNXT_VID_STATUS_SYS_ERROR);
               break;
           }
           break;
      
       case VIDEO_COMMAND_FEED_DRIP:
          copy_drip_data();
          break;
 
       case VIDEO_COMMAND_BUF_EMPTY:
          if ((DripIn == DripOut) && ((int)VideoBufEmpty != 0))
          /* Notify the application */
          (*VideoBufEmpty)((DripOut + (VDREQ_ARRAY_SIZE - 1)) & (VDREQ_ARRAY_SIZE - 1));
          break;

       case VIDEO_COMMAND_SET_VBUFFS:
          {
             u_int32 *pAddrs;
             
             pAddrs = (u_int32 *)message[1];
             
             trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: Setting IPB to 0x%x, 0x%x, 0x%x\n",
                       pAddrs[0], pAddrs[1], pAddrs[2]);
                       
             /* This is only ever called synchronously       */
             /* so message[2] is not callback address        */
             /* and there is no post_video_command_done call */ 
             /* message[3] is task ID, not B buffer addr     */
             video_process_set_vbuffs((u_int8 *)(pAddrs[0]), 
                                      (u_int8 *)(pAddrs[1]), 
                                      (u_int8 *)(pAddrs[2]));
             send_video_command_done(message, eStatus);
          }
          break;
          
       case VIDEO_COMMAND_BLANK:
          #ifdef DEFERRED_UNBLANK            
          /* Prevent any pending "unblanks" from happening */
          ks = critical_section_begin();
          gbDeferredUnblank = FALSE;
          critical_section_end(ks);
          #endif /* DEFERRED_UNBLANK */
          
          /* Blank the video */
          gen_video_blank_internal();
          send_video_command_done(message, eStatus);
          post_video_command_done(message, eStatus);
          break;
          
       case VIDEO_COMMAND_UNBLANK:   
          #ifdef DEFERRED_UNBLANK
          if (message[1])
          {
            int  iDecodedIndex;
            bool bUnblankNow = FALSE;
            
            /* If passed something other than 0, we defer the actual unblank */
            /* and do it later if necessary. Do checks in a critical section */
            /* since the variables we are looking at here are changed in the */
            /* interrupt service routine.                                    */
            ks = critical_section_begin();
            
            /* Which buffer will be displayed if we unblank now? */
            switch ((u_int32)gpVideoDisplayBuffer)
            {
              case HWBUF_DEC_I_ADDR: iDecodedIndex = DEC_I_BUFFER_INDEX; break;
              case HWBUF_DEC_P_ADDR: iDecodedIndex = DEC_P_BUFFER_INDEX; break;
              default:               iDecodedIndex = -1;                 break;
            }
            
            critical_section_end(ks);
            
            if(SafeToUnblankVideo( giDRMDisplayPicture, iDecodedIndex , PlayState))
            {
                ks = critical_section_begin();
                bUnblankNow = TRUE;
                gbDeferredUnblank = FALSE;
                critical_section_end(ks);
            }
            else
            {
                ks = critical_section_begin();
                gbDeferredUnblank = TRUE;
                critical_section_end(ks);
            }
            
            #ifdef DEBUG
            if (gbDeferredUnblank)
            {
              trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: Deferring unblank for buffer at 0x%08x\n", gpVideoDisplayBuffer);
            }
            #endif
            
            /* If we have to unblank immediately, do it now */
            if (bUnblankNow)
            {
              trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: Undeferred unblank\n");
              gen_video_unblank_internal();
            }  
          }
          else
          {
            /* Don't defer this unblank - just do it regardless */
            gen_video_unblank_internal();
          }  
          #else /* Not deferring unblank commands */
            /* If not deferring the unblank, we just unblank the video plane */
            gen_video_unblank_internal();
          #endif /* DEFERRED_UNBLANK */
          
          /* We send the command completion indication even if we deferred */
          /* the actual unblanking of the video.                           */
          send_video_command_done(message, eStatus);
          post_video_command_done(message, eStatus);
          break;
          
       default:
           break;
   }
   return;
}


/*******************************************************************/
/* Function: video_process_set_vbuffs                              */
/* Parameters: void *arg                                           */
/* Return: void                                                    */
/* Remarks:  This is the video task. It waits on a queue then      */
/*           process the commands and calls the command done       */
/* callback if one was provided                                    */
/*******************************************************************/
void video_process_set_vbuffs(u_int8 *IAddr, u_int8 *PAddr, u_int8 *BAddr)
{
   u_int32 dwDecodeAddr = 0xFFFFFFFF;
   u_int32 dwAddrExtn = 0;

   /* We can't use a critical section here since video_send_hardware_command */
   /* can take up to 50mS or so to complete!                                 */
    
   #ifdef DEBUG
   if ((IAddr >= (u_int8*)0x200000) || (PAddr >= (u_int8*)0x200000) || (BAddr >= (u_int8*)0x200000))
   {
     trace_new(TRACE_MPG|TRACE_LEVEL_ALWAYS, "VIDEO: Suspicious decode buffer ptrs after queue! 0x%x, 0x%x, 0x%x\n",
               IAddr,
               PAddr,
               BAddr);
   }            
   #endif

   /* Set the I frame address. */
   dwDecodeAddr = (u_int32)IAddr;
   if (dwDecodeAddr)
   {
      dwAddrExtn = (dwDecodeAddr >> 24) & 0x7;
      video_send_hardware_command(MPG_COMMAND_INITI);
      video_send_hardware_command((u_int32)MPG_PIC_ADDR_BIT | (dwDecodeAddr & 0xFFF));
      video_send_hardware_command((u_int32)MPG_PIC_ADDR_BIT | ((dwDecodeAddr >> 12) & 0xFFF));
      gpIBuffer = (u_int8 *)dwDecodeAddr;
   } /* endif */

   /* Set the P frame address. */
   dwDecodeAddr = (u_int32)PAddr;

   if (dwDecodeAddr)
   {
      if ((dwAddrExtn != 0xFFFFFFFF) &&                                            
          (dwAddrExtn != ((dwDecodeAddr >> 24) & 0x7)))
      {
         trace_new(TRACE_MPG|TRACE_LEVEL_2,"VIDEO:Inconsistent video buffer addresses!!!!\n");
      }
      video_send_hardware_command(MPG_COMMAND_INITP);
      video_send_hardware_command((u_int32)MPG_PIC_ADDR_BIT | (dwDecodeAddr & 0xFFF));
      video_send_hardware_command((u_int32)MPG_PIC_ADDR_BIT | ((dwDecodeAddr >> 12) & 0xFFF));
      gpPBuffer = (u_int8 *)dwDecodeAddr;
   }

   /* Set the B frame address. */
   dwDecodeAddr = (u_int32)BAddr;

   if (dwDecodeAddr)
   {
      if ((dwAddrExtn != 0xFFFFFFFF) &&                                            
          (dwAddrExtn != ((dwDecodeAddr >> 24) & 0x7)))
      {
         trace_new(TRACE_MPG|TRACE_LEVEL_2,"VIDEO:Inconsistent video buffer addresses!!!!\n");
      }
      video_send_hardware_command(MPG_COMMAND_INITB);
      video_send_hardware_command((u_int32)MPG_PIC_ADDR_BIT | (dwDecodeAddr & 0xFFF));
      video_send_hardware_command((u_int32)MPG_PIC_ADDR_BIT | ((dwDecodeAddr >> 12) & 0xFFF));
      gpBBuffer = (u_int8 *)dwDecodeAddr;
   }
   if (dwAddrExtn != 0xFFFFFFFF) 
   {
     CNXT_SET_VAL(glpMpgAddrExt, MPG_ADDR_EXT_ADDREXTENSION_MASK, dwAddrExtn);
   }
}



/*******************************************************************/
/* Function: video_IRQHandler                                      */
/* Parameters: void                                                */
/* Return: void                                                    */
/* Remarks:  ISR for MPEG interrupts.                              */
/*******************************************************************/
#define BSWAP(x) (((x & 0xFF) << 24) |                         \
                  ((x & 0xFF00) << 8) |                        \
                  ((x & 0xFF0000) >> 8) |                      \
                  ((x & 0xFF000000) >> 24))

int video_IRQHandler(u_int32 IntID, bool Fiq, PFNISR *pfnChain)
{
    u_int32 IRQReason = *glpIntStatus & (VIDEO_ISR_BITS|MPG_VB_EMPTY|MPG_USER_DETECTED);
    u_int32 message[4];
    u_int32 ErrorReason;
    u_int32 DSRValue;
    u_int8 *pUserData;
    u_int32 *pUserData32, temp;
    u_int32 uLastDecodedFrameType;
    u_int8 UserDataLength;
    u_int32 uErrorValue;
    bool   bRetcode;
    static bool bUserDataReceived = FALSE;
    int i;
    HW_DWORD           lTimeCode;
    int giDecodedIndex;

    /***************************************************************************/
    /* User data has been detected. Copy it to the user data buffer and notify */
    /* interested party.                                                       */
    /***************************************************************************/
    if (IRQReason & MPG_USER_DETECTED)
    {
        *glpIntStatus = MPG_USER_DETECTED;
        while (*DPS_USER_READ_PTR_EX(gDemuxInstance) != *DPS_USER_WRITE_PTR_EX(gDemuxInstance))
        {
            /* User data is read in 8 byte blocks. Up to 7 of those bytes contain user */
            /* values and one is a status byte. Values stored into the buffer are      */
            /* byte swapped so the order is:                                           */
            /*                                                                         */
            /* Buffer byte 0  - User data byte 4                                       */
            /*             1  - User data byte 3                                       */
            /*             2  - User data byte 2                                       */
            /*             3  - User data byte 1                                       */
            /*             4  - Status                                                 */
            /*             5  - User data byte 7                                       */
            /*             6  - User data byte 6                                       */
            /*             7  - User data byte 5                                       */


            /* Byte swap the data in place to make it easier to copy the bytes in */
            /* a loop.                                                            */
            pUserData = (u_int8 *)*DPS_USER_READ_PTR_EX(gDemuxInstance) + NCR_BASE;
            pUserData32 = (u_int32 *) pUserData;
            temp = *pUserData32;
            *pUserData32 = BSWAP(temp);
            ++pUserData32;
            temp = *pUserData32;
            *pUserData32 = BSWAP(temp);
            
            /* Number of user data bytes is in the bottom nibble of the 7th byte. */
            /* Although it is a nibble, no more than 7 bytes are stored per block */
            UserDataLength = *(pUserData + 7) & 0x0f;
            if (UserDataLength == 0)
            {
                isr_trace_new(TRACE_MPG|TRACE_LEVEL_1, "Error UserData length == 0\n", 0, 0);
            }

            /* We should not hit this case but... */
            if (UserDataLength > 7)
              UserDataLength = 7;

            /***************************************************************/
            /* Ring buffer variable use:                                   */
            /*                                                             */
            /* UserWriteIndex is the next byte to be written to the buffer */
            /* UserReadIndex  is the next byte to be read from the buffer  */
            /*                                                             */
            /* The write pointer points to an empty location and the read  */
            /* pointer points to a full one (unless buffer is empty)       */
            /*                                                             */
            /* Buffer empty when read index == write index                 */
            /* Buffer full when write index == (read index - 1)            */
            /*                                                             */
            /***************************************************************/

            /* Copy the user data bytes into our intermediate ring buffer if  */
            /* the buffer is not already full. If we overflow, the data gets  */
            /* thrown away until the old stuff is processed. This is a        */
            /* deliberate decision to save us having to update UserReadIndex  */
            /* inside the ISR. This would greatly complicate the handling of  */
            /* the client user data callback if we did update this index here.*/
            for (i = 0; i < UserDataLength; ++i) 
            {
               /* See if the buffer has filled up and drop out if so */
               if(((UserWriteIndex != (LOCAL_VIDEO_USER_BUFFER_SIZE-1)) && (UserWriteIndex == (UserReadIndex -1))) ||
                  ((UserWriteIndex == (LOCAL_VIDEO_USER_BUFFER_SIZE-1)) && (UserReadIndex == 0)))
               {   
                 isr_trace_new(TRACE_MPG|TRACE_LEVEL_ALWAYS,"VIDEO: User data local buffer overflow!\n", 0, 0);
                 break;
               }

                /* Copy a byte and update the write pointer */
                UserBuffer[UserWriteIndex] = *pUserData++;
                UserWriteIndex++;

                /* Handle wrap at the end of the ring buffer */
                if (UserWriteIndex >= LOCAL_VIDEO_USER_BUFFER_SIZE)
                  UserWriteIndex = 0;
            }
            
            /* Update the hardware read pointer */
            *DPS_USER_READ_PTR_EX(gDemuxInstance) += 8;
            if (*DPS_USER_READ_PTR_EX(gDemuxInstance) >= 
                (*DPS_USER_START_ADDR_EX(gDemuxInstance)+HWBUF_USRDAT_SIZE))
            {
                *DPS_USER_READ_PTR_EX(gDemuxInstance) = *DPS_USER_START_ADDR_EX(gDemuxInstance);
            }
        }
        
        /* We defer sending the user data to the client until the next decode       */
        /* complete interrupt. This is to ensure that we can tag the callback with  */
        /* the decoded picture type so allowing the client to reorder the user data */
        /* if necessary for presentation of, for example, closed captioning.        */
        bUserDataReceived = TRUE;
    }

    /****************************************************************/
    /* Error found in the MPEG stream. Handled for information only */
    /****************************************************************/
    if (IRQReason & MPG_ERROR_IN_STAT_REG)
    {
        ErrorReason = *glpErrorStatus & MPG_BITSTREAM_ERROR;
        if (ErrorReason & MPG_BITSTREAM_ERROR) 
        {
            /* NOTE WELL: Reading the decoder status register causes various bits to be */
            /* cleared. It is vital, therefore, that we only ever read this in 1 place  */
            /* and that we only read it once and keep a shadow copy!!!!                 */
            DSRValue = *(LPREG)glpDecoderStatusReg;
            
            /* Bump the relevant error counter if this is a video error */
            if(DSRValue & MPG_VIDEO_ERROR)
            {
//*****Green screen patch start*******************************
              //static u_int32 error_mask=0xF7F00010;
              static u_int32 error_mask=0xFFFFFFFF;
//*****Green screen patch end********************************
              uErrorValue = (DSRValue & MPG_DECODE_STATUS_VIDEOERROR_MASK) >> MPG_DECODE_STATUS_VIDEOERROR_SHIFT;
              
              if(uErrorValue < NUM_DSR_ERROR_CODES)
                 guDSRErrorCounts[uErrorValue]++;
                   
              #ifdef DEBUG
              isr_trace_new(TRACE_MPG|TRACE_LEVEL_ALWAYS,
                            "VIDEO: MPG_BITSTREAM_ERROR DSR = 0x%08x, error 0x%02x\n",
                            DSRValue, 
                            uErrorValue);
              isr_trace_new(TRACE_MPG|TRACE_LEVEL_ALWAYS,"Video Read/Write= 0x%x 0x%x\n", 
                        (u_int32) *DPS_VIDEO_READ_PTR_EX(gDemuxInstance), 
                        (u_int32) *DPS_VIDEO_WRITE_PTR_EX(gDemuxInstance));
              #endif
//*****Green screen patch start*************************************************************
              if (((1<<(uErrorValue-1))&error_mask)&&bResetComplete&&bChannelChanged)
              {
                 bResetComplete = 0;
                 bChannelChanged = 0;
                 message[0] = VIDEO_COMMAND_RESET;
                 message[1] = message[2] = message[3] = 0;
                 if ( qu_send(VideoQueueID, message) != RC_OK )
                 {
                    isr_trace_new(TRACE_MPG|TRACE_LEVEL_ALWAYS,"MPG_BITSTREAM_ERROR: qu_send failed \n",0,0);
                    isr_error_log(VIDEO_QSEND_FAIL + ERROR_FATAL);
                 }
              }
//*****Green screen patch end*******************************************************************
            }

            /* We call the registered audio error handler for all Audio errors */
            if((DSRValue & MPG_AUDIO_ERROR) && gpfnAudioErrorCallBack)
              gpfnAudioErrorCallBack(DSRValue);
        }
        *glpErrorStatus = ErrorReason;
        *glpIntStatus = MPG_ERROR_IN_STAT_REG;
    }

    /***************************/
    /* Video buffer overflowed */
    /***************************/
    if (IRQReason & MPG_VB_FULL)
    {
        /* DW: The hardware guys tell me that we don't need to reset in this case and */
        /* can pretty much ignore this interrupt too. Investigation required...       */
        *glpIntStatus = MPG_VB_FULL;

        /* Increment our error counter */
        guVideoBufferOverflowCount++;

        /* Check for overflows without decode completes, indicating core stall. */
        ++guLocalVideoBufferOverflowCount;
        if ( (guLocalVideoBufferOverflowCount > 1) && (!guContinueCommandinProgress) )
        {
          guContinueCommandinProgress = 1;
          isr_trace_new(TRACE_MPG|TRACE_LEVEL_ALWAYS,"VIDEO: Video buffer overflow number %d without decode complete!\n", guLocalVideoBufferOverflowCount, 0);
          
          message[0] = VIDEO_COMMAND_CONTINUE;
          message[1] = 0;
          message[2] = 0;
          message[3] = 0;
          
          if ( qu_send(VideoQueueID, message) != RC_OK )
          {
              isr_trace_new(TRACE_MPG|TRACE_LEVEL_ALWAYS,"VIDEO: qu_send failed \n",0,0);
              isr_error_log(VIDEO_QSEND_FAIL + ERROR_WARNING);
          } 
         }
        else
        {
          isr_trace_new(TRACE_MPG|TRACE_LEVEL_ALWAYS,"VIDEO: Video buffer overflow!\n", 0, 0);
        
          /* Move read pointer ahead of write pointer */
          SetReadPtr(MPEG_BUFFER_RESTART_TARGET, TRUE);
        
          /* Toggle sync (if enabled) */
          bRetcode = VideoDisableSync();
          if(bRetcode)
            VideoEnableSync();
        }
#if ((PVR==YES)||(XTV_SUPPORT==YES))
        /*
         * Call ES Buffer Event Handler
         */
        if(Video_ES_Buffer_Callback_Function)
        {
            Video_ES_Buffer_Callback_Function(CNXT_ES_BUFFER_VIDEO,
                                              CNXT_ES_BUFFER_FULL,
                                              Video_ES_Buffer_Callback_Tag);
        }
#endif /* (PVR==YES) */
    }
    
    /************************************************************/
    /* Video Buffer Empty: mask off the IRQ and do nothing else */
    /************************************************************/
    if (IRQReason & MPG_VB_EMPTY)
    {
        isr_trace_new(TRACE_MPG|TRACE_LEVEL_1,"VIDEO: Video Buffer empty\n", 0, 0);
        *glpIntMask &= ~MPG_VB_EMPTY;
        *glpIntStatus = MPG_VB_EMPTY;

        /* Increment the error counter if we are playing live video */
	      if ((PlayState == PLAY_LIVE_STATE) || (PlayState == PLAY_LIVE_STATE_NO_SYNC))
	      {
          guVideoBufferUnderflowCount++;
        }

	      if (PlayState == PLAY_DRIP_STATE)
	      {
	        message[0] = VIDEO_COMMAND_BUF_EMPTY;
	        qu_send(VideoQueueID, message);
	      }
    }

    /****************************************************************************************/
    /* Video Decode Complete: Occurs after each displayed frame and calls video_FrameNotify */ 
    /* to handle the time codes. NOTE: Do NOT use this to indicate completion of a still    */
    /* decode since we often see stills that contain multiple pictures and, as a result,    */
    /* will generate multiple decode complete interrupts.                                   */
    /****************************************************************************************/
    if (IRQReason & MPG_VIDEO_DEC_COMPLETE)
    {
        /* Reset the counter used to track core stalls. */
        guLocalVideoBufferOverflowCount = 0;

        /* Keep track of which type of frame this is and whether it was an anchor frame. */
        /* If an anchor frame record the dimensions decoded and the play state that is   */
        /* in use so that we know if the image is intended for display later.            */
        #if MPG_SYNC_BIT_POSITION == MPG_SYNC_BIT_POSITION_COLORADO  
            uLastDecodedFrameType = CNXT_GET(glpMpgPicSize, MPG_VID_MASK_COMPLETE);
        #elif MPG_SYNC_BIT_POSITION == MPG_SYNC_BIT_POSITION_WABASH
            uLastDecodedFrameType = CNXT_GET(glpMpgAddrExt, MPG_VID_MASK_COMPLETE);
        #endif

        #ifdef DEBUG
        if ( (PlayState == PLAY_STILL_STATE) ||
             (PlayState == PLAY_STILL_STATE_NO_DISP) )
        {
          isr_trace_new(TRACE_MPG|TRACE_LEVEL_2,
                        "VIDEO: VIDEO_DEC_COMPLETE type %d, count %d\n", 
                        uLastDecodedFrameType, 
                        DecodeCompleteCount);
          isr_trace_new(TRACE_MPG|TRACE_LEVEL_2,"VIDEO: Read/write pointers 0x%08x/0x%08x\n",
                        *DPS_VIDEO_READ_PTR_EX(gDemuxInstance),
                        *DPS_VIDEO_WRITE_PTR_EX(gDemuxInstance));          
        }
        else
        {
          isr_trace_new((gbWaiting ? (TRACE_MPG|TRACE_LEVEL_2) : (TRACE_MPG|TRACE_LEVEL_1)),
                        "VIDEO: VIDEO_DEC_COMPLETE type %d, size 0x%08x\n", uLastDecodedFrameType, 
                        #if MPG_SYNC_BIT_POSITION == MPG_SYNC_BIT_POSITION_WABASH
                        *((LPREG)glpMpgAddrExt));
                        #else
                        *((LPREG)glpMpgPicSize));
                        #endif
        }                
        #endif
        
        /* If playing live video, reenable the video buffer empty interrupt */
        if ((PlayState == PLAY_LIVE_STATE) ||
            (PlayState == PLAY_LIVE_STATE_NO_SYNC))
        {
           *glpIntStatus |= MPG_VB_EMPTY;
           *glpIntMask   |= MPG_VB_EMPTY;
        }             
        
        if (uLastDecodedFrameType != MPG_VID_B_COMPLETE)
        {
          gdwLastDecodedAnchor = uLastDecodedFrameType;
          
          if (uLastDecodedFrameType == MPG_VID_I_COMPLETE)
          {
            giDecodedIndex = ((u_int32)gpIBuffer == HWBUF_DEC_I_ADDR) ? DEC_I_BUFFER_INDEX : DEC_P_BUFFER_INDEX;
          }
          else  
          {
            giDecodedIndex = ((u_int32)gpPBuffer == HWBUF_DEC_I_ADDR) ? DEC_I_BUFFER_INDEX : DEC_P_BUFFER_INDEX;
          }
          
          gsDecodedImages[giDecodedIndex].dwHeight       = gdwSrcHeight;
          gsDecodedImages[giDecodedIndex].dwWidth        = gdwSrcWidth;
          gsDecodedImages[giDecodedIndex].dwStartingMode = PlayState;
        }
        
        /* If we received any user data since the last picture was decoded, */
        /* inform the video task.                                           */
        if(bUserDataReceived)
        {        
          message[0] = VIDEO_COMMAND_USER_DATA;
          message[1] = uLastDecodedFrameType;
          message[2] = 0;
          message[3] = 0;
          
          if ( qu_send(VideoQueueID, message) != RC_OK )
          {
              isr_trace_new(TRACE_MPG|TRACE_LEVEL_ALWAYS,"VIDEO: qu_send failed \n",0,0);
              isr_error_log(VIDEO_QSEND_FAIL + ERROR_WARNING);
          }
          
          bUserDataReceived = FALSE;
        }  

        if (gVideoFrameCallBack != NULL)
            gVideoFrameCallBack();
//*****Green screen patch start***************
        bChannelChanged = 0;
//*****Green screen patch end***************
        *glpIntStatus = MPG_VIDEO_DEC_COMPLETE;
        DecodeCompleteCount++;
    }

    /*************************************************************************************/
    /* Idle interrupts tells us that the core has finished what it was last asked to do. */
    /* This is used as a signal that a video_stop has completed and that an MPEG still   */
    /* decode is done.                                                                   */
    /*************************************************************************************/
    if (IRQReason & MPG_VIDEO_DEC_INTERRUPT6)
    {
        *glpIntMask &= ~MPG_VIDEO_DEC_INTERRUPT6;   /* Mask it */
        *glpIntStatus = MPG_VIDEO_DEC_INTERRUPT6;   /* clear it */
        
        isr_trace_new(TRACE_MPG|TRACE_LEVEL_4,"VIDEOISR: ** Idle interrupt occured. Disabling int r/w 0x%08x, 0x%08x**\n",
        *((LPREG)DPS_VIDEO_READ_PTR_EX(gDemuxInstance)),
        *((LPREG)DPS_VIDEO_WRITE_PTR_EX(gDemuxInstance)));
        
        isr_trace_new(TRACE_MPG|TRACE_LEVEL_4,"VIDEOISR: ** Putting idle sem from ISR **\n",0,0);
        sem_put(semVideoPlay);
    }

    /*******************************************************************************/
    /* This is a disasterous error notification from the video microcode. We never */
    /* expect to see this but, if we do, reset the video core and hope things are  */
    /* OK after this.                                                              */
    /*******************************************************************************/
    if (IRQReason & MPG_VIDEO_DEC_INTERRUPT7){
        isr_trace_new(TRACE_MPG|TRACE_LEVEL_ALWAYS,"VIDEO: VIDEO INT7!!!\n", 0, 0);
        *glpIntStatus = MPG_VIDEO_DEC_INTERRUPT7;
        message[0] = VIDEO_COMMAND_RESET;
        message[1] = message[2] = message[3] = 0;
        if ( qu_send(VideoQueueID, message) != RC_OK )
        {
            isr_trace_new(TRACE_MPG|TRACE_LEVEL_ALWAYS,"VIDEO: qu_send failed \n",0,0);
            isr_error_log(VIDEO_QSEND_FAIL + ERROR_FATAL);
        }

    }
    
    /***********************************************/
    /* Signals the decoding of a sequence end byte */
    /***********************************************/
    if (IRQReason & MPG_VIDEO_SEQ_END)
    {
        if((PlayState != PLAY_LIVE_STATE) && (PlayState != PLAY_LIVE_STATE_NO_SYNC))
        {
          isr_trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: Sequence end interrupt. MPG size reg 0x%08x\n", (u_int32)(*(LPREG)glpMpgPicSize), 0);
          isr_trace_new(TRACE_MPG|TRACE_LEVEL_2,"VIDEO: Read/write pointers 0x%08x/0x%08x\n",
                        *DPS_VIDEO_READ_PTR_EX(gDemuxInstance),
                        *DPS_VIDEO_WRITE_PTR_EX(gDemuxInstance));          
          iStillSeqEndCount++;        
        }  
        *glpIntStatus = MPG_VIDEO_SEQ_END;
        isr_trace_new(TRACE_MPG|TRACE_LEVEL_2,"Seq. End Read/Write= 0x%x 0x%x\n", 
                  (u_int32) *DPS_VIDEO_READ_PTR_EX(gDemuxInstance), 
                  (u_int32) *DPS_VIDEO_WRITE_PTR_EX(gDemuxInstance));
    }

    /****************************************************/
    /* GOP Time Code Received. Copies out the time code */
    /****************************************************/
    if (IRQReason & MPG_GOP_TC_RECEIVED)
    {
        lTimeCode = *glpTimeCode;
        isr_trace_new(TRACE_MPG|TRACE_LEVEL_1,"VIDEO: GOP_TC Received H:M=%d:%d:\n",
		      CNXT_GET_VAL(&lTimeCode, MPG_GOP_TIME_CODE_HOURS_MASK), 
		      CNXT_GET_VAL(&lTimeCode, MPG_GOP_TIME_CODE_MINUTES_MASK));
        isr_trace_new(TRACE_MPG|TRACE_LEVEL_1,"VIDEO: GOP_TC Recieved S:F=%d:%d\n",
		      CNXT_GET_VAL(&lTimeCode, MPG_GOP_TIME_CODE_SECONDS_MASK),
		      CNXT_GET_VAL(&lTimeCode, MPG_GOP_TIME_CODE_PICTURES_MASK));
        gen_current_time_code.hour   = CNXT_GET_VAL(&lTimeCode, MPG_GOP_TIME_CODE_HOURS_MASK);
        gen_current_time_code.second = CNXT_GET_VAL(&lTimeCode, MPG_GOP_TIME_CODE_SECONDS_MASK);
        gen_current_time_code.minute = CNXT_GET_VAL(&lTimeCode, MPG_GOP_TIME_CODE_MINUTES_MASK);
        gen_current_time_code.frame  = CNXT_GET_VAL(&lTimeCode, MPG_GOP_TIME_CODE_PICTURES_MASK);
        if (gTimeCodeCallBack)
            gTimeCodeCallBack(&gen_current_time_code);
        *glpIntStatus = MPG_GOP_TC_RECEIVED;
    }

#if ((PVR==YES)||(XTV_SUPPORT==YES))
    /*******************************/
    /* Video Buffer Low-water Mark */
    /*******************************/
    if (IRQReason & MPG_VB_LOWWATERMARK)
    {
        isr_trace_new(TRACE_MPG|TRACE_LEVEL_1, "VIDEO: VB_LWM Received\n",0,0);
        /*
         * Call ES Buffer Event Handler
         */
        if(Video_ES_Buffer_Callback_Function)
        {
            Video_ES_Buffer_Callback_Function(CNXT_ES_BUFFER_VIDEO,
                                              CNXT_ES_BUFFER_LWM,
                                              Video_ES_Buffer_Callback_Tag);
        }
        *glpIntStatus = MPG_VB_LOWWATERMARK;
    }
#endif /* PVR==YES */

    if (IRQReason)
        return RC_ISR_HANDLED;
    else
    {
        *pfnChain = pOldVideofnHandler;
        return RC_ISR_NOTHANDLED;
    }
}

/*******************************************************************/
/* Function: gen_video_get_buffer()                                */
/* Parameters: void                                                */
/* Return: voidF pointer to the buffer                             */
/* Remarks:  Driver supplied function to return the buffer         */
/*******************************************************************/
voidF gen_video_get_buffer(void)
{
    trace_new(TRACE_MPG|TRACE_LEVEL_2,"VIDEO: gen_video_get_buffer():0x%x\n", LocalVideoBuffer);
    return (voidF) LocalVideoBuffer;
}

/*******************************************************************/
/* Function: gen_video_buffer_size()                               */
/* Parameters: void                                                */
/* Return: void                                                    */
/* Remarks:  Driver supplied function to return the size of the    */
/*           buffer.                                               */
/*******************************************************************/
int gen_video_buffer_size(void)
{
    trace_new(TRACE_MPG|TRACE_LEVEL_2,"VIDEO: gen_video_buffer_size()=0x%x\n", LOCAL_VIDEO_BUFFER_SIZE);
    return LOCAL_VIDEO_BUFFER_SIZE;
}


/*******************************************************************/
/* Function: gen_video_blank()                                     */
/* Parameters: void                                                */
/* Return: void                                                    */
/* Remarks:  Driver supplied function to blank the video hardware. */
/*           The driver must call the video_command_done() notify  */
/*           function when this command is completed.              */
/*******************************************************************/
void gen_video_blank(gen_callback_cmd_done_t video_command_done)
{
    u_int32 message[4];

    trace_new(TRACE_MPG|TRACE_LEVEL_2,"VIDEO: gen_video_blank %s\n", video_command_done ? "with callback" : "");
    message[0] = VIDEO_COMMAND_BLANK;
    
    if (video_command_done)
        post_video_command(message,video_command_done);
    else
        send_video_command(message);
}

/*******************************************************************/
/* Function: gen_video_unblank()                                   */
/* Parameters: void                                                */
/* Return: void                                                    */
/* Remarks:  Driver supplied function to un-blank the video hdw.   */
/*******************************************************************/
void gen_video_unblank(gen_callback_cmd_done_t video_command_done)
{
    u_int32 message[4];

    trace_new(TRACE_MPG|TRACE_LEVEL_2,"VIDEO: gen_video_unblank %s\n", video_command_done ? "with callback" : "");
    message[0] = VIDEO_COMMAND_UNBLANK;
    #ifdef DEFERRED_UNBLANK
    message[1] = 1;
    #else
    message[1] = 0;
    #endif  
    
    if (video_command_done)
        post_video_command(message,video_command_done);
    else
        send_video_command(message);
}

/*******************************************************************/
/* Function: gen_video_blank_internal()                            */
/* Parameters: void                                                */
/* Return: void                                                    */
/* Remarks:  Driver supplied function to blank the video hardware. */
/*           The driver must call the video_command_done() notify  */
/*           function when this command is completed.              */
/*******************************************************************/
static void gen_video_blank_internal(void)
{
    bool ks;

    isr_trace_new(TRACE_MPG|TRACE_LEVEL_2,"VIDEO: gen_video_blank_internal\n", 0,0);
    ks = critical_section_begin();
    *((LPREG)glpDrmControl) &= ~DRM_MPEG_ENABLE;
    critical_section_end(ks);
}

/*******************************************************************/
/* Function: gen_video_unblank_internal()                          */
/* Parameters: void                                                */
/* Return: void                                                    */
/* Remarks:  Driver supplied function to un-blank the video hdw.   */
/*******************************************************************/
void gen_video_unblank_internal(void)
{
    bool ks;
    #ifdef DEBUG
    u_int8 *pBuffer;
    #endif

    isr_trace_new(TRACE_MPG|TRACE_LEVEL_2,"VIDEO: gen_video_unblank_internal\n", 0,0);
    
  	ks = critical_section_begin();
  	*((LPREG)glpDrmControl) |= DRM_MPEG_ENABLE;
  	critical_section_end(ks);
    
    #ifdef DEBUG
    gen_video_get_current_display_buffer(&pBuffer);
    isr_trace_new(TRACE_MPG|TRACE_LEVEL_2,"VIDEO: Displaying buffer at address 0x%08x\n", (u_int32)pBuffer, 0);
    #endif
}

/*******************************************************************/
/* Function: gen_video_play()                                      */
/* Parameters: video_starting_mode starting_mode                   */
/*                - This specifies the starting mode that the video*/
/*                  driver/hardware should use, Stills, live or    */
/*                  drip.                                          */
/* Return: void                                                    */
/* Remarks:  This function when called should set the hw for the   */
/*           specified mode. The command is queued up and the      */
/*           function returns immediately.                         */
/*******************************************************************/
CNXT_VID_STATUS gen_video_play(u_int32 starting_mode, 
                                 gen_callback_cmd_done_t video_command_done)
{
    u_int32 message[4];
    CNXT_VID_STATUS eStatus;
	
    trace_new(TRACE_MPG|TRACE_LEVEL_2,"VIDEO: gen_video_play %s\n", video_command_done ? "with callback" : "");
    message[0] = VIDEO_COMMAND_PLAY;
    message[1] = (u_int32) starting_mode;
    
    gStoppingVideo = FALSE;
    
    if (video_command_done)
    {
        post_video_command(message,video_command_done);
        eStatus = CNXT_VID_STATUS_ASYNC;
    }    
    else
    {
        eStatus = send_video_command(message);
    }    
 
    return(eStatus);
}

/*******************************************************************/
/* Function: gen_video_stop()                                      */
/* Parameters: void                                                */
/* Return: void                                                    */
/* Remarks:  Stops the video hardware from decoding.               */
/*******************************************************************/
bool gen_video_stop(gen_callback_cmd_done_t video_command_done)
{
    u_int32 message[4];

    trace_new(TRACE_MPG|TRACE_LEVEL_2,"VIDEO: gen_video_stop %s\n", video_command_done ? "with callback" : "");
    if (PlayState == PLAY_STILL_STATE){
        VBReadIndex = VBWriteIndex = 0;
        num_vid_leftovers = 0;
    }
    else{
        if (PlayState == PLAY_LIVE_STATE){
            DecodeCompleteCount = 0;
        }
    }

    gStoppingVideo = TRUE;
    
    *pgnFieldCount = 0;
    
    message[0] = VIDEO_COMMAND_STOP;
	
    if (video_command_done)
        post_video_command(message,video_command_done);
    else
        send_video_command(message);

    return TRUE;
}

/*******************************************************************/
/* Function: gen_video_pause()                                     */
/* Parameters: void                                                */
/* Return: void                                                    */
/* Remarks:  Stops the video hardware from decoding. The last      */
/*           frame decoded is displayed. The decoding may resume   */
/*           later.                                                */
/*                                                                 */
/* Context: May be called from interrupt context as long as a      */
/*          callback function is provided.                         */
/*                                                                 */
/*******************************************************************/
bool gen_video_pause(gen_callback_cmd_done_t video_command_done)
{
    u_int32 message[4];

    isr_trace_new(TRACE_MPG|TRACE_LEVEL_2,"VIDEO: gen_video_pause %s\n", (u_int32)(video_command_done ? "with callback" : ""), 0);
    
    #ifdef DEBUG
    if(!video_command_done)
      not_interrupt_safe();
    #endif
      
    message[0] = VIDEO_COMMAND_PAUSE;
    message[1] = FALSE;
    if (video_command_done)
        post_video_command(message,video_command_done);
    else
        send_video_command(message);

    return TRUE;
}


#if VIDEO_UCODE_TYPE == VMC_EXTRA_ERROR_RECOVERY
/*******************************************************************/
/* Function: gen_video_pause_on_next_I()                           */
/* Parameters: void                                                */
/* Return: void                                                    */
/* Remarks:  Stops the video hardware from decoding after the next */
/*           I picture in the stream.                              */
/*                                                                 */
/* Context: May be called from interrupt context as long as a      */
/*          callback function is provided.                         */
/*                                                                 */
/*******************************************************************/
bool gen_video_pause_on_next_I(gen_callback_cmd_done_t video_command_done)
{
    u_int32 message[4];

    isr_trace_new(TRACE_MPG|TRACE_LEVEL_2,"VIDEO: gen_video_pause %s\n", (u_int32)(video_command_done ? "with callback" : ""), 0);
    
    #ifdef DEBUG
    if(!video_command_done)
      not_interrupt_safe();
    #endif
      
    message[0] = VIDEO_COMMAND_PAUSE;
    message[1] = TRUE;
    if (video_command_done)
        post_video_command(message,video_command_done);
    else
        send_video_command(message);

    return TRUE;
}
#endif /* VIDEO_UCODE_TYPE == VMC_EXTRA_ERROR_RECOVERY */

/*******************************************************************/
/* Function: gen_video_reset()                                     */
/* Parameters: void                                                */
/* Return: void                                                    */
/* Remarks:                                                        */
/*     Private function to reset the video/audio.                  */
/*******************************************************************/
void gen_video_reset(void)
{
    u_int32 message[4];
    trace_new(TRACE_MPG|TRACE_LEVEL_2,"VIDEO: gen_video_reset");
    message[0] = VIDEO_COMMAND_RESET;
    send_video_command(message);

}

/*******************************************************************/
/* Function: gen_video_resume()                                    */
/* Parameters: void                                                */
/* Return: void                                                    */
/* Remarks:  Resumes decoding of video.                            */
/*******************************************************************/
bool gen_video_resume(gen_callback_cmd_done_t video_command_done)
{
    u_int32 message[4];

    trace_new(TRACE_MPG|TRACE_LEVEL_2,"VIDEO: gen_video_resume %s\n", video_command_done ? "with callback" : "");
    message[0] = VIDEO_COMMAND_RESUME;
    if (video_command_done)
        post_video_command(message,video_command_done);
    else
        send_video_command(message);
    
    return TRUE;
}

/*******************************************************************/
/* Function: gen_video_set_video_buffers                           */
/* Parameters: pIBuffer  I Buffer Address                          */
/*             pPBuffer  P Buffer Address                          */
/*             pBBuffer  B Buffer Address                          */
/* Return: void                                                    */
/* Remarks:  Only sets I and P buffers                             */
/*******************************************************************/
void gen_video_set_video_buffers(u_int8 *pIBuffer, u_int8 *pPBuffer, u_int8 *pBBuffer)
{
    u_int32 message[4];  
    static u_int32 addrs[3]; /* Just in case someone wakes this task up */
                             /* during send_video_command!              */

    not_interrupt_safe();
    trace_new(TRACE_MPG|TRACE_LEVEL_2,"VIDEO: gen_video_set_video_buffers\n");
    
    #ifdef DEBUG
    if ((pIBuffer >= (u_int8*)0x200000) || (pPBuffer >= (u_int8*)0x200000) || (pBBuffer >= (u_int8*)0x200000))
    {
      trace_new(TRACE_MPG|TRACE_LEVEL_ALWAYS, "VIDEO: Suspicious decode buffer ptrs ! 0x%x, 0x%x, 0x%x\n",
                pIBuffer,
                pPBuffer,
                pBBuffer);
    }            
    #endif
    
    addrs[0] = (u_int32)pIBuffer;
    addrs[1] = (u_int32)pPBuffer;
    addrs[2] = (u_int32)pBBuffer; 
    message[0] = VIDEO_COMMAND_SET_VBUFFS;
    message[1] = (u_int32)(&addrs[0]);
    send_video_command(message); /* This is synchronous */
    
    return;
}

/*******************************************************************/
/* Function: gen_send_still_data()                                 */
/* Parameters: gen_farcir_q *q  - Pointer to a far circular queue, */
/*                            containing the data                  */
/* Return:  Number of bytes the driver removed from the queue.     */
/* Remarks:  This function will remove as much data as possible    */
/*           from the supplied queue and place it in the encoded   */
/*           video buffer.                                         */
/*******************************************************************/
/*
 typedef struct {
         voidF           begin;
         int             size;           / NOTE:It holds at most (size-1)  bytes in queue. /
         int             in;             / Write  offset from the beginning of the buffer. /
         int             out;            / Read offset form the beginning of the buffer. /
 } farcir_q;
*/
int gen_send_still_data(gen_farcir_q *q)
{
    /* read as many bytes as possible into audio buffer.
    * Then update the head of the far circular queue
    * and return the number of bytes consumed.
    */

    int bytes_consumed = 0;
    int bytes_to_copy;
    int initial_q_out = q->out;
    u_int32 message[4];

    trace_new(TRACE_MPG|TRACE_LEVEL_2,"VIDEO: gen_send_still_data\n");
    trace_new(TRACE_MPG|TRACE_LEVEL_2,"VIDEO: Pre-send pointers r/w %x/%x\n",
        (u_int32) *DPS_VIDEO_READ_PTR_EX(gDemuxInstance),
        (u_int32) *DPS_VIDEO_WRITE_PTR_EX(gDemuxInstance));

    bSeqEndFound = fix_mpeg_still_problems(q, AllowStillCmd);

    /* Some naughty applications fail to call gen_video_play before sending still data.   */
    /* If we don't set the state here, gen_video_flush_still_data thinks nothing is being */
    /* decoded in this case and exits prematurely.                                        */
    PlayState = PLAY_STILL_STATE;
    
    if (AllowStillCmd == TRUE)
    {         
        DecodeCompleteCount = 0;
    
        /* This is the first call to send data for a new still so set up for it by flushing */
        /* the encoded video buffer and internal VBV buffer before copying the new data.    */

        /* Flush the main encoded video buffer */
        ReadWritePtrReset();

        /* Flush the VBV buffer inside the decoder */
        message[0] = VIDEO_COMMAND_FLUSH_VBV;
        message[1] = message[2] = message[3] = 0;
        send_video_command(message);
        
        /* Add 256 bytes of 0 before the still. This is a workaround for an as-yet unresolved problem */
        /* where the video read pointer sometimes seems to get confused after stopping motion video   */
        /* and starting the still.                                                                    */
        PadEncodedVideoBuffer(256);
        
        DecodeCompleteCount = 0;
    
        /* Tell the video task to issue a still decode command to the hardware on our behalf   */
        message[0] = VIDEO_COMMAND_STILL_WAIT;
        message[1] = message[2] = message[3] = 0;
        send_video_command(message);
        
        /* Remind ourselves not to do the one-off setup on the next call */
        AllowStillCmd = FALSE;
    }

    trace_new(TRACE_MPG|TRACE_LEVEL_2,"VIDEO: Copying data to encoded video buffer r/w before %x/%x\n",
        (u_int32) *DPS_VIDEO_READ_PTR_EX(gDemuxInstance),
        (u_int32) *DPS_VIDEO_WRITE_PTR_EX(gDemuxInstance));
    
    if (q->out > q->in){    /* wrapping occured */
        bytes_to_copy = q->in + q->size - q->out;
    }
    else{
        bytes_to_copy = q->in - q->out;
    }
    if (PlayPesFromMem){
        while(bytes_to_copy > 3) {
            int bytes_copied = 0;
            bytes_copied = handle_PES( q, &vid_PES_global_data, 0 );
            bytes_consumed += bytes_copied;
            bytes_to_copy -= bytes_copied;
        }
        q->out = (initial_q_out + bytes_consumed) % q->size;
    }
    else
    {
        bytes_consumed = copy_data(q, bytes_to_copy, FALSE);
    }
    
    trace_new(TRACE_MPG|TRACE_LEVEL_3, "VIDEO: Write %d bytes of encoded data to the buffer. In = 0x%08x, Out = %08x\n",
              bytes_consumed, q->in, q->out);
    
    trace_new(TRACE_MPG|TRACE_LEVEL_2,"VIDEO: Post-send pointers r/w %x/%x\n",
        (u_int32) *DPS_VIDEO_READ_PTR_EX(gDemuxInstance),
        (u_int32) *DPS_VIDEO_WRITE_PTR_EX(gDemuxInstance));
     
    
    return bytes_consumed;
}

/********************************************************************/
/*  FUNCTION:    fix_mpeg_still_problems                            */
/*                                                                  */
/*  PARAMETERS:  q - circular queue containing MPEG still data to   */
/*                   parse and, if necessary, correct.              */
/*               bReset - TRUE to reset the state before beginning  */
/*                        a new still, FALSE to continue using      */
/*                        previous state.                           */
/*                                                                  */
/*  DESCRIPTION: Parse the passed still data fixing up errors that  */
/*               we have seen which would otherwise cause our       */
/*               decoder to barf on the data. Currently, we look    */
/*               for 5 cases - an invalid frame rate, reserved      */
/*               values in the MPEG level/profile description,      */
/*               unchanged temporal reference since last still,     */
/*               sequence error code in place of sequence end,      */
/*               a bad 0x00 0x01 sequence in the user data (which   */
/*               exposes a bug in the video decoder hardware) and   */
/*               missing sequence end marker.                       */
/*                                                                  */
/*               Note that still data is patched in place. If the   */
/*               data is in ROM, therefore, the fixes will not be   */
/*               applied. People should know better than blow an    */
/*               an image with broken stills into flash. :-)        */
/*                                                                  */
/*  RETURNS:     TRUE if a sequence end marker has been found since */
/*               instance was last reset.                           */
/*               FALSE if no sequence end marker has been found or  */
/*               inserted since last reset.                         */
/*                                                                  */
/*  CONTEXT:     Must not be called from interrupt.                 */
/*                                                                  */
/********************************************************************/

/*********************************************************************/
/* NB: This function may fail if the whole of the required table is  */
/* not passed in one piece. If we get passed still data a byte at a  */
/* time, for example (which is perfectly valid), we will not catch   */
/* these cases!!!! To be fixed later (since 99% of the time, a still */
/* is passed in a small number of large pieces rather than a large   */
/* number of small pieces.                                           */
/*********************************************************************/

typedef enum _scan_state
{
  BEGIN,
  ONE_ZERO,
  TWO_ZEROES,
  START_CODE,
  FOUND_SEQ_HEADER,
  FOUND_EXTENSION,
  FOUND_SEQ_EXTENSION,
  FOUND_PICT_EXTENSION,
  FOUND_PICTURE,
  BEGIN_USER,
  ONE_ZERO_USER,
  TWO_ZEROES_USER
} scan_state;

static bool fix_mpeg_still_problems(gen_farcir_q *q, bool bReset)
{

  unsigned char    *qbyteptr;
  unsigned char    *byteafterlastptr;
  unsigned char    *wrapbyteptr;
  unsigned char     frame_rate;    
  unsigned char     mpeg_level;
  unsigned char     mpeg_profile;
  int               val;
  static scan_state eState = BEGIN;
  static bool       bSeqEndFound = FALSE;
  static int        iStateCount  = 0;
  static u_int8     uLastTempRef = 0xFF;
  static u_int16    uWidth, uHeight;
  static u_int32    uVector = 0;
    
  /* If asked to reset our state, do so */  
  if (bReset)
  {
    trace_new(TRACE_MPG|TRACE_LEVEL_3, "VIDEO: New image. Scanning still data for problems...\n");
    eState= BEGIN;
    bSeqEndFound        = FALSE;
    iStillSeqEndCount   = 0;
    iStillSeqStartCount = 0;
    iStillSeqCount      = 0;
    uWidth              = 0;
    uHeight             = 0;
    iStillPanOffset     = 0;
    iStillScanOffset    = 0;
    bStillVectorFound   = FALSE;
    uVector             = 0;
  } 
  else
  {
    trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: Scanning more still data for problems...\n");
  } 
      
  /* Check we got data passed */
  if (q->in == q->out)
  {
     trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: Still data queue is empty\n");
     return(bSeqEndFound);
  }
     
  /* Initialize pointers to first byte in queue, to last byte before wrap */
  /* and to the byte after the last valid byte in the data passed */
  qbyteptr = (unsigned char *)((int)q->begin + q->out);                               
  wrapbyteptr = (unsigned char *)((int)q->begin + q->size);
  byteafterlastptr = (unsigned char *)((int)q->begin + q->in);
  if (byteafterlastptr == wrapbyteptr)
     byteafterlastptr = q->begin;
  
  do
  {
     /* Pick up the byte to consider on this iteration */
     val = *qbyteptr;
     
     switch (eState)
     {
        /******************************************************************************/
        /* Found a single zero in the stream. Two may indicate a start code coming up */
        /******************************************************************************/
        case ONE_ZERO:
           /* Look for the second or subsequent 0 */
           if (0 == val)
              eState = TWO_ZEROES;
           else
              eState = BEGIN;
           break;
           
        case ONE_ZERO_USER:
           /* Look for the second or subsequent 0 */
           if (0 == val)
              eState = TWO_ZEROES_USER;
           else
           {
              /* Pattern 0x00 0x01 inside user data causes hardware problems */
              /* so patch it and continue.                                   */
              if(1 == val)
                *qbyteptr = 0xFF;
              eState = BEGIN_USER;
           }   
           break;
           
        /********************************************************************/
        /* Found two adjacent 0s. If 0x01 is next, we've found a start code */
        /********************************************************************/
        case TWO_ZEROES:
           /* Look for the following 1 */
           if (1 == val)
           {
              eState = START_CODE;
           }   
           else
           {
              if (0 != val)
                 eState = BEGIN;
           }      
           break;

        case TWO_ZEROES_USER:
           /* Look for the following 1 */
           if (1 == val)
           {
              eState = START_CODE;
           }   
           else
           {
              if (0 != val)
                 eState = BEGIN_USER;
           }   
           break;
           
           
        /**************************************************/
        /* We found a start code in the data. What is it? */
        /**************************************************/
        case START_CODE:
          switch (val)
          {
            /* Picture start code - look for same temporal reference */
            /* in adjacent pictures.                                 */
            case 0x00: 
              eState = FOUND_PICTURE;
              break;
              
            /* User data start code - look for problematic 0x00 0x01 pattern */
            case 0xB2: 
              eState = BEGIN_USER;
              break;
              
            /* Sequence header start code - look for invalid frame rates and extract size */  
            case 0xB3: 
              eState = FOUND_SEQ_HEADER;
              iStateCount = 0;
              iStillSeqStartCount++;
              bSeqEndFound = FALSE;
              break;
              
            /* Sequence error start code - replace it with a sequence end */
            case 0xB4: 
              *qbyteptr = 0xB7;
              iStillSeqCount++;
              eState = BEGIN;
              bSeqEndFound = TRUE;
              break;
              
             /* Extension start code - look for the type */  
            case 0xB5:
              iStateCount = 0;
              eState      = FOUND_EXTENSION;
              break;
              
            /* Sequence end code - remember that we saw this */  
            case 0xB7: 
              iStillSeqCount++;
              eState = BEGIN;
              bSeqEndFound = TRUE;
              break;
              
            /* We don't care about other start codes */  
            default:
              eState = BEGIN;
              break;
          }   
          break;

        /**************************************/   
        /* We are processing a picture header */
        /**************************************/   
        case FOUND_PICTURE:
          /* At this point, qbyteptr points to first byte after the 0x00 0x00 0x01 0x00  */
          /* pattern. This is the top 8 bits of the 10 bit temporal reference field.     */
          /* Rather than muck with all 10 bits, we just make sure that this is different */
          /* from the last code sent. If 2 stills are sent back to back with the same    */
          /* temporal reference, the vcore will ignore the second one.                   */
          if (val == uLastTempRef)
          {
            trace_new(TRACE_LEVEL_NEVER|TRACE_MPG,
                     "VIDEO: Patching duplicate adjacent temporal references in still\n");
                     
            /* Make temporal reference different */
            uLastTempRef++;         
            *qbyteptr = uLastTempRef;
          }
          else
          {
            uLastTempRef = val;
          }  
          eState = BEGIN;
          break;
            
        /***************************************/    
        /* We are processing a sequence header */
        /***************************************/    
        case FOUND_SEQ_HEADER:
        
           /* Advance our byte counter - we're looking for the 4th byte */
           iStateCount++;
           
           switch(iStateCount)
           {
             case 1:
               /* 1st byte - MSB of picture width */
               uWidth = val << 4;
               break;
               
             case 2:
               /* 2nd byte - LS nibble of width and MS nibble of height */
               uWidth |= (val >> 4);
               uHeight = (val & 0x0F) << 8;
               break;
               
             case 3:
               /* 4th byte - we reached the frame rate definition */
               uHeight |= val;
               
               trace_new(TRACE_MPG|TRACE_LEVEL_3, "VIDEO: Still frame dimensions decoded as (%d, %d)\n", uWidth, uHeight);
               
               #ifdef BLANK_BEFORE_ALL_STILLS
               {
                 u_int8 *pBuffer;
                 
                 /*****************************************************************************************/
                 /* W A R N I N G !!!!!                                                                   */
                 /*****************************************************************************************/
                 /*                                                                                       */
                 /* This is a somewhat risky trick required for Canal+ systems. We fill the video buffer  */
                 /* with black prior to decoding a still so that, if the still has an error and does not  */
                 /* completely decode, we end up seeing the decoded portion over a black background.      */
                 /* This, however, relies on some assumptions:                                            */
                 /* 1. The still data passed only contains 1 sequence.                                    */
                 /* 2. The still data passed only contains 1 picture (we only erase the first anchor      */
                 /*    buffer so if there is a failure in the second picture in the sequence you will     */
                 /*    see whatever picture was previously in that buffer below the decoded portion of    */
                 /*    the picture when decode ends rather than black.                                    */
                 /*                                                                                       */
                 /* I am led to believe that these assumptions are reasonable for Canal+ systems so....   */
                 /*                                                                                       */
                 /*****************************************************************************************/
                 if(uWidth && uHeight && bStillBlank)
                 {
                   gen_video_get_safe_still_buffer(&pBuffer);
                   trace_new(TRACE_MPG|TRACE_LEVEL_3, "VIDEO: Blanking video buffer at 0x%08x\n", pBuffer);
                   fill_video_color(uWidth, 
                                    uHeight, 
                                    (u_int32 *)pBuffer, 
                                    VIDEO_BLACK_Y, 
                                    VIDEO_BLACK_CB, 
                                    VIDEO_BLACK_CR);
                 }
               }
               #endif
               break;
               
             case 4:
              frame_rate = val;
              /* If frame rate not a valid value (as defined by MPEG) set to */
              /* value 3 which is 25fps                                      */
              if (((frame_rate & 0x0f) == 0) || ((frame_rate & 0x0f) > 8))
              {
                 frame_rate &= 0xF0; 
                 frame_rate |= 0x03;
                 *qbyteptr = frame_rate; 
                 trace_new(TRACE_MPG|TRACE_LEVEL_ALWAYS,
                      "VIDEO: Patching invalid frame rate in still\n");
              } /* endif invalid frame rate */
              
              eState = BEGIN;
               break;
           }
             
           break;
            
        /******************************************/
        /* We are processing a sequence extension */   
        /******************************************/
        case FOUND_EXTENSION:
           iStateCount++;
           
           /* We only enter this state immediately after reading the 0xB5   */
           /* extension start code so iStateCount must be 1. Now figure out */
           /* what kind of extension this is.                               */
 
           /* Is it a sequence extension? */
           if ((val & 0xF0) == 0x10)
           {
             /* This is a sequence extension so contains the profile and level   */
             /* definition. The profile is contained in the same byte as the     */
             /* extension code so we have it now. Next time round, check the     */
             /* level nibble to see if it is valid and supported. If not,        */
             /* pretend it is an MP:ML still.                                    */
         
             mpeg_profile = (val & 0x0F);
             
             if((mpeg_profile & 0x0E) != 0x04)
             {
               /* Profile found is not main profile or simple profile so patch it. */
               /* Valid values are 0101b and 0100b only                            */
               trace_new(TRACE_LEVEL_ALWAYS|TRACE_MPG,
                         "VIDEO: Patching unsupported or invalid MPEG profile %d in still\n", mpeg_profile);
               mpeg_profile = 0x04; /* Main profile */
               *qbyteptr = (val & 0xF0) | mpeg_profile;
             }  
             eState = FOUND_SEQ_EXTENSION;
           }  
           else
           {
             /* Is it a picture display extension? */
             if((val & 0xF0) == 0x70)
             {
               /* This is a picture display extension. We use this to extract the pan/scan vector    */
               /* for the still. To do this 100% properly, we should really parse the picture coding */
               /* extension too to determine how many vectors may be present here (there can be up   */
               /* to 3 depending upon the encoding type) but, since we are dealing with stills and   */
               /* since I don't want to have to parse the picture coding extension unless absolutely */
               /* necessary, I just read the first one. If this doesn't work, I will revisit the     */
               /* problem.                                                                           */
               
               /* The bottom nibble of the first byte holds the top 4 bits of the pan vector */
               
               uVector = (u_int32)(val & 0xF) << 12;
               
               eState = FOUND_PICT_EXTENSION;
               
             }
             else
             {
               /* This is not an extension we are interested in so look */
               /* for the next start code.                              */
               eState = BEGIN; 
             }
           }  
           break;
           
        /************************************************************************/   
        /* Processing the Sequence Extension (for profile and level definition) */
        /************************************************************************/   
        case FOUND_SEQ_EXTENSION:
           iStateCount++;
           
           /* We get here on the second byte of the extension. The profile */
           /* has already been checked so now check the level.             */
           mpeg_level = val & 0xF0;
           if ((mpeg_level != 0xA0) && (mpeg_level != 0x80))
           {
              /* MPEG level is not Low or Main so patch it to Main */
              trace_new(TRACE_LEVEL_ALWAYS|TRACE_MPG,
                   "VIDEO: Patching unsupported or invalid MPEG level %d in still\n", mpeg_level >> 4);
              mpeg_level = 0x80;
              *qbyteptr = (val & 0x0F) | mpeg_level;
           }
           
           eState = BEGIN;
           break;
           
        /**************************************************************************/   
        /* We are extracting the pan/scan vector from a picture display extension */
        /**************************************************************************/   
        case FOUND_PICT_EXTENSION:
          iStateCount++;
          
          switch(iStateCount)
          {
            case 2:
              uVector |= (u_int32)val << 4;
              break;
              
            case 3:
              uVector |= (u_int32)((val >> 4) & 0x0F);
              /* Pan vector is now complete - move it to the global variable */
              iStillPanOffset = (int)(int16)uVector;
              
              /* Skip past the marker bit, clear the vector accumulator and start */
              /* building the scan value.                                         */
              uVector = (u_int32)(val & 0x07) << 13;
              break;
              
            case 4:
              uVector |= (u_int32)val << 5;
              break;
              
            case 5:
              uVector |= (u_int32)((val >> 3) & 0x1F);
              iStillScanOffset  = (int)(int16)uVector;
              bStillVectorFound = TRUE;
              
              trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: Still contains pan/scan vector (%d, %d)\n", iStillPanOffset, iStillScanOffset);
              
              /* Go back to looking for the next start code since we are finished with the vector */
              eState = BEGIN;
              break;
            
            default:
              trace_new(TRACE_LEVEL_ALWAYS|TRACE_MPG, "VIDEO: !!!!! Problem in the MPEG still state machine !!!!!\n");
              break;
          }
          break;
           
        /*************************************************************/   
        /* We are looking for the first 0 inside a user data section */
        /*************************************************************/   
        case BEGIN_USER:
           if (0 == val)
              eState = ONE_ZERO_USER;
           else
              eState = BEGIN_USER;
           break;

        /**************************************************************/   
        /* We are looking for the first 0 outside a user data section */
        /**************************************************************/   
        case BEGIN:
        default:
           if (0 == val)
              eState = ONE_ZERO;
           else
              eState = BEGIN;
           break;
     }
     
     /* Increment our pointer to the next byte in the buffer */
     qbyteptr++;
     if (qbyteptr == wrapbyteptr)
        qbyteptr = q->begin;
  }
  while(qbyteptr != byteafterlastptr);

  trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: Still data scan complete. Total of %d sequences found. Sequence end %s\n",
            iStillSeqCount,
            bSeqEndFound ? "found" : "not found");
  
  return(bSeqEndFound);
   
} /* fix_mpeg_still_problems */

#ifdef BLANK_BEFORE_ALL_STILLS
/*****************************************************************************/
/* Function: fill_video_color()                                              */
/*                                                                           */
/* Parameters: uWidth  - Width of video plane to be filled                   */
/*             uHeight - Height of video plane to be filled                  */
/*             pImage  - Pointer to video image buffer to fill               */
/*             pColor  - Color to use in filling the region                  */
/*                                                                           */
/* Returns:    Nothing                                                       */
/*                                                                           */
/* Description: This function fills a video buffer with colour. The buffer   */
/*              dimensions to use as supplied rather than being assumed from */
/*              the current contents. This allows us to use the function to  */
/*              pre-fill a buffer knowing the image that will later be       */
/*              decoded into it and have this fill act as the background     */
/*              colour if the decode ends prematurely.                       */
/*                                                                           */
/*****************************************************************************/
static void fill_video_color(u_int16 uWidth, 
                             u_int16 uHeight, 
                             u_int32 *pImage, 
                             u_int8 Y, 
                             u_int8 Cb, 
                             u_int8 Cr)
{
   u_int32 dwStride;
   u_int32 dwHeight, dwWidth;
   u_int32 dwX, dwY, dwFill = 0;

   /* Get the displayed Mpg height and width and use them to fill
    * a 4:2:0 video buffer. The stride used here is the same as the
    * width since each luma sample is 8 bits. The stride of the region
    * is not used since DRM expects that data to be packed in memory
    * according to the width and height.
    */
   dwHeight = (u_int32)uHeight;
   dwWidth = (u_int32)uWidth;
   dwStride = (dwWidth+15)&0xFFFFFFF0;
   dwX = dwY = 0;

   /* Fill the Luma. */
   dwFill = Y;
   dwFill |= (dwFill << 8)|(dwFill << 16)|(dwFill << 24);
   GfxFillMem(pImage, dwStride, dwWidth, dwHeight, dwFill, dwX, dwY, 8, 0);

   /* Fill the chroma buffer. */
   /* Increment the ptr to the chroma. */
   pImage += dwStride * dwHeight / 4; 

   /* The chroma height = (luma height + 1) / 2 */
   dwHeight ++;      
   dwHeight >>= 1;

   /* The width of the chroma buffer 1/2 luma width, but 16 bpp. */
   dwWidth >>= 1;

   /* The fill color needs to be CrCb:CrCb or VVUU:VVUU */
   dwFill = (((u_int32)Cb) << 8) | ((u_int32)Cr);
   dwFill |= dwFill << 16;
   GfxFillMem(pImage, dwStride, dwWidth, dwHeight, dwFill, dwX, dwY, 16, 0);
   
   GfxIsGXACmdDone(1000);
}
#endif /* BLANK_BEFORE_ALL_STILLS */

/*******************************************************************/
/* Function: gen_video_flush_data()                                */
/* Parameters: void                                                */
/* Return: void                                                    */
/* Remarks:  This function is called so that the driver can flush  */
/*           any remaining data into the encoded hw buffer.        */
/*******************************************************************/
bool gen_flush_still_data( gen_callback_cmd_done_t video_command_done )
{
    char * read_ptr;  /* hardware encoded video buffer read pointer */
    char * write_ptr; /* hardware encoded video buffer write pointer */
    char * limit;
    int i;
    u_int32 msg[4];
    bool ks;
    u_int32 write_ptr_wrap;
    CNXT_VID_STATUS eStatus = CNXT_VID_STATUS_OK;
    
    trace_new(TRACE_MPG|TRACE_LEVEL_2,"VIDEO: gen_flush_still_data %s\n", video_command_done ? "with callback":"");
    
    /* We should never be called when the video decoder is not doing anything.  */
    /* This indicates that someone has used a bogus call sequence when decoding */
    /* a still. Unfortunately, some apps on BSkyB call the OpenTV layer's       */
    /* video_report_empty_data function twice back-to-back which results in a   */
    /* delay unless we add this check here.                                     */
    if( PlayState == PLAY_OFF_STATE)
    {
      trace_new(TRACE_MPG|TRACE_LEVEL_2,"VIDEO: No still being decoded. Returning immediately!\n");
      if (video_command_done != NULL)
      {
         video_command_done(eStatus);
      }   
      
      /* Make sure we set the flag indicating that we are ready to decode */
      /* another still. This should not be needed here since it is set on */
      /* gen_video_stop but some naughty apps don't call this function so */
      /* we need to set the flag here too.                                */
      AllowStillCmd = TRUE;
      
      return(TRUE);
    }
    
    read_ptr  = (char *) ((u_int32)*DPS_VIDEO_READ_PTR_EX(gDemuxInstance) & ~0x80000000);
    write_ptr = (char *) ((u_int32)*DPS_VIDEO_WRITE_PTR_EX(gDemuxInstance) & ~0x80000003);
    write_ptr_wrap = ( ((u_int32)*DPS_VIDEO_WRITE_PTR_EX(gDemuxInstance)) & 0x80000000) >> 31;
    limit = (char *)HWBUF_ENCVID_ADDR + HWBUF_ENCVID_SIZE;

    trace_new(TRACE_MPG|TRACE_LEVEL_2,"VIDEO: readptr/writeptr:  %x/%x\n", read_ptr, write_ptr);

#ifdef SWAP_COPY
    /* first flush the leftovers */
    if (num_vid_leftovers != 0) {
        for (i = 3; i >= (int)(4-num_vid_leftovers); i--) {
            *((char *)write_ptr + i + NCR_BASE) = leftover[1][3-i];
        }
        for (; i >= 0; --i)
            *((char *)write_ptr + i +  NCR_BASE) = 0;
        num_vid_leftovers = 0;
        write_ptr += 4;
    }
#else
    if (num_vid_leftovers != 0) {
        write_ptr += num_vid_leftovers;
        for (i = 0; i < (int)(4-num_vid_leftovers); i++) {
            *((char *) ((u_int32) write_ptr|BSWAP_OFFSET|NCR_BASE)) = 0;
            write_ptr++;
        }
        num_vid_leftovers = 0;
    }
#endif
    if (limit == write_ptr) {  /* we have hit end of buffer.  Go to beginning */
        write_ptr = (char *)HWBUF_ENCVID_ADDR;
        /* toggle wrap bit */
        write_ptr_wrap = !write_ptr_wrap;
    }

    /* Did we find a sequence end code in the data passed? */
    if (!bSeqEndFound)
    {
      /* No - we must append this to the data to prevent decode errors */
      /* Note that write_ptr is on a word boundary here and not at the */
      /* end so we know there are at least 4 bytes left in the buffer. */
      
      /* We add an extra 4 bytes of 0 here to guard against stills which   */
      /* are truncated by the app (we've seen this on 2 different networks */
      /* so far). These make it a whole lot more likely that the video     */
      /* decoder will catch the sequence end code and not make the mistake */
      /* of interpreting it as part of a damaged macroblock.               */
      
      trace_new(TRACE_MPG|TRACE_LEVEL_2,"VIDEO: Appending a sequence end code since still did not have one\n");
      iStillSeqCount++;
      *(write_ptr++ + NCR_BASE) = (char)0x00;
      *(write_ptr++ + NCR_BASE) = (char)0x00;
      *(write_ptr++ + NCR_BASE) = (char)0x00;
      *(write_ptr++ + NCR_BASE) = (char)0x00;
      
      /* Check to see whether or not we hit the end of the buffer */
      if (limit == write_ptr) 
      {
        /* If so, move to the beginning and toggle the wrap bit */
        write_ptr = (char *)HWBUF_ENCVID_ADDR;
        write_ptr_wrap = !write_ptr_wrap;
      }
      
      /* add another extra 4 bytes of 0 here to make sure macrocode reads the sequence end code */
      *(write_ptr++ + NCR_BASE) = (char)0x00;
      *(write_ptr++ + NCR_BASE) = (char)0x00;
      *(write_ptr++ + NCR_BASE) = (char)0x00;
      *(write_ptr++ + NCR_BASE) = (char)0x00;
      
      /* Check to see whether or not we hit the end of the buffer */
      if (limit == write_ptr) 
      {
        /* If so, move to the beginning and toggle the wrap bit */
        write_ptr = (char *)HWBUF_ENCVID_ADDR;
        write_ptr_wrap = !write_ptr_wrap;
      }
      
      #ifdef SWAP_COPY
      *(write_ptr++ + NCR_BASE) = (char)0xB7;
      *(write_ptr++ + NCR_BASE) = (char)0x01;
      *(write_ptr++ + NCR_BASE) = (char)0x00;
      *(write_ptr++ + NCR_BASE) = (char)0x00;
      #else
      *(write_ptr++ + NCR_BASE) = (char)0x00;
      *(write_ptr++ + NCR_BASE) = (char)0x00;
      *(write_ptr++ + NCR_BASE) = (char)0x01;
      *(write_ptr++ + NCR_BASE) = (char)0xB7;
      #endif
      
      /* Check to see whether or not we hit the end of the buffer */
      if (limit == write_ptr) 
      { 
        /* If so, move to the beginning and toggle the wrap bit */
        write_ptr = (char *)HWBUF_ENCVID_ADDR;
        write_ptr_wrap = !write_ptr_wrap;
      }
      bSeqEndFound = TRUE;
    }
    
    /* Turn on the buffer empty interrupt */
    ks = critical_section_begin();
    *glpIntStatus |= MPG_VB_EMPTY;
    *glpIntMask |= MPG_VB_EMPTY;
    critical_section_end(ks);

    /* Update the write pointer appropriately */
    ks = critical_section_begin();
    write_ptr = (char *) ((u_int32) write_ptr & ~0x03);
    SetWritePtr( (MPG_ADDRESS) ( ((u_int32) write_ptr & ~(BSWAP_OFFSET|NCR_BASE)) | (write_ptr_wrap << 31)) );
    critical_section_end(ks);

    /* Add 256 bytes of 0 after the still data to ensure that the data */
    /* is correctly flushed through the decoder.                       */
    PadEncodedVideoBuffer(NUM_FLUSH_BYTES);
    
     /* Now send a message to the video process to let it know we have flushed */
     /* In return it should wait for the video decoder to go idle and return   */
     /* to us when it has done so.                                             */
     msg[0] = VIDEO_COMMAND_FLUSHED;
     msg[1] = msg[2] = msg[3] = 0;
     eStatus = send_video_command(msg);
     
     if (video_command_done != NULL)
     {
        trace_new(TRACE_MPG|TRACE_LEVEL_2,"VIDEO:Called flush with callback\n");
        video_command_done(eStatus);
     }   

     if(eStatus == CNXT_VID_STATUS_OK)
       return(TRUE);
     else
       return(FALSE);  
}

/********************************************************************/
/*  FUNCTION:    PadEncodedVideoBuffer                              */
/*                                                                  */
/*  PARAMETERS:  iNumBytes - number of bytes of 0 to add at the     */
/*                           current write pointer position         */
/*                                                                  */
/*  DESCRIPTION: Pad the encoded video buffer with a block of 0s    */
/*               and update the write pointer.                      */
/*                                                                  */
/*  RETURNS:     Nothing                                            */
/*                                                                  */
/*  CONTEXT:     May be called in any context.                      */
/*                                                                  */
/********************************************************************/
static void PadEncodedVideoBuffer(int iNumBytes)
{
    bool ks;
    bool bWrap;
    int  iLoop;
    u_int8 *pLimit;
    u_int8 *pWritePtr;

    pWritePtr = (u_int8 *) ((u_int32)*DPS_VIDEO_WRITE_PTR_EX(gDemuxInstance) & ~0x80000003);
    bWrap     = ( ((u_int32)*DPS_VIDEO_WRITE_PTR_EX(gDemuxInstance)) & 0x80000000) >> 31;
    pLimit    = (u_int8 *)HWBUF_ENCVID_ADDR + HWBUF_ENCVID_SIZE;
    
    for (iLoop = 0; iLoop < iNumBytes; iLoop++)
    {
        *(pWritePtr + NCR_BASE) = 0;
        pWritePtr++;
        if (pWritePtr == pLimit)
        {
            pWritePtr = (u_int8 *)HWBUF_ENCVID_ADDR;
            bWrap = !bWrap;
        }
    }

    /* Update the write pointer appropriately */
    ks = critical_section_begin();
    pWritePtr = (u_int8 *) ((u_int32) pWritePtr & ~0x03);
    SetWritePtr( (MPG_ADDRESS) ( ((u_int32) pWritePtr & ~(BSWAP_OFFSET|NCR_BASE)) | (bWrap ? 0x80000000 : 0)) );
    critical_section_end(ks);
}

void gen_video_StartSendPesData()
{
    PlayPesFromMem = TRUE;
    vid_PES_global_data.where_in_PES = FIND_PREFIX;
    vid_PES_global_data.progress = LAST_BYTE_NONZERO;
    vid_PES_global_data.bytes_left_in_packet = 0;
    vid_PES_global_data.bytes_left_in_header = 0;
    vid_PES_global_data.pts_present = 0;

}

void StopSendPesData()
{
    PlayPesFromMem = FALSE;
}

void gen_video_StopSendPesData(void)
{
    PlayPesFromMem = FALSE;
}

/********************************************************************/
/*  FUNCTION:    gen_video_get_current_display_buffer               */
/*                                                                  */
/*  PARAMETERS:  ppBuffer - storage to receive pointer to video     */
/*                          buffer currently being displayed        */
/*                                                                  */
/*  DESCRIPTION: Return a pointer to the anchor frame buffer which  */
/*               is currently being displayed.                      */
/*                                                                  */
/*  RETURNS:     TRUE if returned pointer set, FALSE otherwise      */
/*                                                                  */
/*  CONTEXT:     May be called from any context                     */
/*                                                                  */
/********************************************************************/
bool gen_video_get_current_display_buffer(u_int8 **ppBuffer)
{
  u_int8 *pBuffer;
  bool    ks;
  bool    bAuto;
  u_int32 uRegVal;
  
  if (ppBuffer)
  {
    /* NOTE: This function assumes that objects displayed directly from */
    /*       the hardware decode buffers (stills or motion video) will  */
    /*       always be displayed using MPEG plane 0. This is valid (as  */
    /*       far as I am aware) for the current driver. If this changes */
    /*       this function will have to be rewritten accordingly.       */
  
    ks = critical_section_begin();
  
    /* If the DRM is being told what to display by the video hardware */
    /* then we already know the display address since we read it      */
    /* whenever vcore tells the DRM it has decoded a new picture.     */
    if (!CNXT_GET_VAL(glpDrmMpgStillCtrl[0], DRM_MPG_VIDEO_PARAM_STILL_ENABLE_MASK))
    {
      /* The problem with using the following approach is that it guarantees to      */
      /* return the wrong buffer address if gen_video_set_video_buffers has been     */
      /* called since the last time gen_video_stop was called. I strongly suspect    */
      /* that this was the real cause of the problem mentioned in the following      */
      /* comment, hence I am reverting to the previous code. We now update the       */
      /* display pointer after video decode has stopped so, hopefully, this will get */
      /* round any problem of buffer switches on stop.                               */
      #ifdef USE_OLD_BUFFER_CHECK_CODE
      /* Although the gpVideoDisplayBuffer pointer is set during processing of video */
      /* Int 5, I have seen some cases where it seems that the MPEG size register    */
      /* changes without this interrupt firing (very occasionally after a stop). To  */
      /* guard against this, we derive the display address from the current contents */
      /* of the MPEG size register rather than merely relying on the current global  */
      /* variable contents.                                                          */
      #if MPG_SYNC_BIT_POSITION == MPG_SYNC_BIT_POSITION_COLORADO  
      uRegVal = CNXT_GET(glpMpgPicSize, MPG_VID_MASK_DISPLAYED);
      #else
      uRegVal = CNXT_GET(glpMpgAddrExt, MPG_VID_MASK_DISPLAYED);
      #endif
       
      switch(uRegVal)
      {
        case MPG_VID_I_DISPLAYED:       gpVideoDisplayBuffer = gpIBuffer; break;
        case MPG_VID_P_DISPLAYED:       gpVideoDisplayBuffer = gpPBuffer; break;
        case MPG_VID_B_FRAME_DISPLAYED: 
        case MPG_VID_B_FIELD_DISPLAYED: gpVideoDisplayBuffer = gpBBuffer; break;
      }  
      #endif
      
      pBuffer = gpVideoDisplayBuffer; /* Set in OSDLIBC\OSDISRC.C during Int 5 processing */
                                      /* and also after video is stopped.                 */
      bAuto = TRUE;
    }
    else
    {
      /* The DRM is displaying from a fixed address. Read this back and */
      /* return this to the caller                                      */
      pBuffer = (u_int8 *)CNXT_GET_VAL(glpDrmMpgLumaAddr[0], DRM_ADDRESS_ADDRESS_MASK);
      bAuto = FALSE;
    }  
  
    critical_section_end(ks);

    isr_trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: Display is %s, address 0x%08x\n", 
                  (u_int32)(bAuto ? "automatic" : "manual"), 
                  (u_int32)pBuffer);
    isr_trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: MPG_SIZE used 0x%08x, read 0x%08x\n",
                  uRegVal,
                  *(LPREG)glpMpgPicSize);
    
    *ppBuffer = pBuffer;
    
    return(TRUE);
  }
  else
    return(FALSE);
}

/********************************************************************/
/*  FUNCTION:    gen_video_get_safe_still_buffer                    */
/*                                                                  */
/*  PARAMETERS:  ppBuffer - storage to receive pointer to a video   */
/*                          buffer which is safe to decode into     */
/*                          without being displayed.                */
/*                                                                  */
/*  DESCRIPTION: Return a pointer to the anchor frame buffer which  */
/*               is not currently being displayed and into which it */
/*               is therefore safe to decode a non-display still.   */
/*                                                                  */
/*  RETURNS:     TRUE if returned pointer set, FALSE otherwise      */
/*                                                                  */
/*  CONTEXT:     May be called from any non-interrupt context       */
/*                                                                  */
/********************************************************************/
bool gen_video_get_safe_still_buffer(u_int8 **ppBuffer)
{
  u_int8 *pBuffer;
  bool    bRetcode;
  
  bRetcode = gen_video_get_current_display_buffer(&pBuffer);
  
  if (bRetcode)
  {
    /* It is safe to decode into whichever of the 2 anchor frame */
    /* buffers is not currently being displayed.                 */
    *ppBuffer = (u_int8 *)((pBuffer == (u_int8 *)HWBUF_DEC_P_ADDR) ? HWBUF_DEC_I_ADDR : HWBUF_DEC_P_ADDR);
  }
  
  return(bRetcode);
  
}


/********************************************************************/
/*  FUNCTION:    gen_video_get_motion_decode_buffers                */
/*                                                                  */
/*  PARAMETERS:  ppIBuff - storage for returned I picture pointer   */
/*               ppPBuff - storage for returned P picture pointer   */
/*                                                                  */
/*  DESCRIPTION: This function returns the pointers to use when     */
/*               setting up the vcore's decode buffers. Pointers    */
/*               are chosen to ensure no transient images when      */
/*               the decode starts.                                 */
/*                                                                  */
/*  RETURNS:     TRUE on success, FALSE on error                    */
/*                                                                  */
/*  CONTEXT:     May be called from any non-interrupt context       */
/*                                                                  */
/********************************************************************/
bool gen_video_get_motion_decode_buffers(u_int8 **ppIBuff, u_int8 ** ppPBuff)
{
  bool bGotBuffer = FALSE;
  
  #ifdef DEBUG
  not_interrupt_safe();
  #endif
  
  /* The buffers are set up so that the first frame to be decoded will */
  /* go into the non-visible buffer.                                   */
  
  if (ppIBuff && ppPBuff)
  {
    if (gdwLastDecodedAnchor == MPG_VID_I_COMPLETE)
    {
      /* P will be next to be decoded */
      bGotBuffer = gen_video_get_current_display_buffer(ppIBuff);
      
      /* If we read a display buffer other than the I or P buffer, revert to the original buffers */
      /* since something has obviously gone wrong or this call has been made while motion video   */
      /* is running.                                                                              */
      if ((*ppIBuff != (u_int8 *)HWBUF_DEC_P_ADDR) && (*ppIBuff != (u_int8 *)HWBUF_DEC_I_ADDR))
      {
        trace_new(TRACE_MPG|TRACE_LEVEL_ALWAYS, "VIDEO: ****** Suspicious display address read 0x%08x ******\n", *ppIBuff);
        *ppIBuff = (u_int8 *)HWBUF_DEC_I_ADDR;
        *ppPBuff = (u_int8 *)HWBUF_DEC_P_ADDR;
      }
      else
      {
        *ppPBuff = (u_int8 *)((*ppIBuff == (u_int8 *)HWBUF_DEC_P_ADDR) ? HWBUF_DEC_I_ADDR : HWBUF_DEC_P_ADDR);
      }  
    }
    else
    {
      /* I will be next to be decoded */
      bGotBuffer = gen_video_get_current_display_buffer(ppPBuff);
      if ((*ppPBuff != (u_int8 *)HWBUF_DEC_P_ADDR) && (*ppPBuff != (u_int8 *)HWBUF_DEC_I_ADDR))
      {
        trace_new(TRACE_MPG|TRACE_LEVEL_ALWAYS, "VIDEO: ****** Suspicious display address read 0x%08x ******\n", *ppPBuff);
        *ppIBuff = (u_int8 *)HWBUF_DEC_I_ADDR;
        *ppPBuff = (u_int8 *)HWBUF_DEC_P_ADDR;
      }
      else
      {
        *ppIBuff = (u_int8 *)((*ppPBuff == (u_int8 *)HWBUF_DEC_P_ADDR) ? HWBUF_DEC_I_ADDR : HWBUF_DEC_P_ADDR);
      }  
    }
    
    trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: Last decoded anchor %d, Current vcore display ptr 0x%08x, pI 0x%08x, pP 0x%08x, MPG_SIZE 0x%08x\n",
              gdwLastDecodedAnchor,
              gpVideoDisplayBuffer,
              *ppIBuff,
              *ppPBuff,
              *(LPREG)glpMpgPicSize);
  }
  
  return(bGotBuffer);
}
             
/*******************************************************************/
/* Function: video_send_hardware_stop_command()                    */
/* Parameters: None                                                */
/* Return:     Nothing                                             */
/* Remarks:  This function sends whatever sequence of words is     */
/*           required by the target processor to stop video        */
/*           decode.                                               */
/*******************************************************************/
static void video_send_hardware_stop_command(void)
{
  /* Without the Canal+ microcode, we don't have the ability to pause */
  /* on an I picture.                                                 */
  video_send_hardware_command(MPG_COMMAND_PAUSE);
  
  /* On Colorado with macroblock-based microcode, the PAUSE command takes a   */
  /* a parameter to tell the video core which type of scaling to use in       */
  /* displaying the stopped image. This is to work around a bug in the chip   */
  /* where the scaling mode is used for both video windows even if the second */
  /* window is displaying a still. If field scaling is used in this case,     */
  /* only one field of the still is displayed and you end up with half the    */
  /* vertical resolution.                                                     */
  #if (VIDEO_UCODE_TYPE == VMC_MACROBLOCK_ERROR_RECOVERY) && (VIDEO_MICROCODE == UCODE_COLORADO)
  if(gbUseFrameScaling)
    video_send_hardware_command(MPG_COMMAND_FRAME_SCALE);
  else
    video_send_hardware_command(MPG_COMMAND_FIELD_SCALE);
  #endif
}
             
/*******************************************************************/
/* Function: send_video_command()                                  */
/* Parameters: msg to send                                         */
/* Return:   Status as returned by video task                      */
/* Remarks:  This function effectively synchronously calls the     */
/*           video process to get something done. It does not      */
/*           return until the command is done.                     */
/*           This allows us to sequence access to hardware simply  */
/*           It works by sleeping the current task. The video      */
/*           process wakes up the task when it is done.            */
/*           Task ID's are unique so no chance of interleaving     */
/*           dones by mistake.                                     */
/*******************************************************************/
CNXT_VID_STATUS send_video_command(u_int32 *msg)
{            
   
   task_id_t tid;
   bool saved_task_mask;    
   int rc;
   CNXT_VID_STATUS eStatus = CNXT_VID_STATUS_SYS_ERROR;
   
   not_interrupt_safe();
   
   trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: send_video_command entered\n");
   
   tid = task_id();
   if (tid)
   {      
      /* Set the task to wakeup on completion to ourselves */
      msg[3] = tid;       
      msg[2] = 0; /* no callback */
      
      /* Avoid races by setting the task mask. That way, if a task switch            */
      /* occurs and we sleep ourselves after we have been woken up, all will be well */
      
      saved_task_mask = task_get_mask();
      task_mask(TRUE);                                                               
      
      /* Send the command to the video_process */
      if ( qu_send(VideoQueueID, msg) != RC_OK )
      {
          trace_new(TRACE_MPG|TRACE_LEVEL_ALWAYS,"VIDEO: qu_send failed \n");
          error_log(VIDEO_QSEND_FAIL + ERROR_FATAL);
          return(eStatus);
      }          
      
      /* task_time_sleep does not log a warning if it times out */
      /* We really could do with knowing whether we woke or were awoken */
      rc = task_time_sleep_ex(VIDEO_SEND_COMMAND_TIMEOUT);

      #ifdef DEBUG
      if (rc == RC_KAL_TIMEOUT)
      {                       
         trace_new(TRACE_MPG|TRACE_LEVEL_ALWAYS,"VIDEO: send_video_command timeout!!!!! on message;\n");
         DumpVideoMessage(msg);
         
      } 
      else 
      {
        if (rc != RC_OK)
        {
          trace_new(TRACE_MPG|TRACE_LEVEL_ALWAYS,"VIDEO: send_video_command sleep error!!!!!\n");
        }                         
      }  
      #endif
      
      /* If we get a good return code from the qu_send, we look at the global status */
      /* to determine how the command was processed.                                 */
      if(rc == RC_OK)
      {
        eStatus = eLastStatus;
      }
      
      /* Restore task mask */
      task_mask(saved_task_mask);
   } /* endif */
   
   trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: send_video_command completed\n");
   
   return(eStatus);
   
} /* send_video_command */

/*******************************************************************/
/* Function: send_video_command_done()                             */
/* Parameters: msg to which is done                                */
/* Return: void                                                    */
/* Remarks:  This function wakes up the task which sent the        */
/*           message. We use msg[3] as the task ID of a sent msg   */
/*******************************************************************/
void send_video_command_done(u_int32 *msg, CNXT_VID_STATUS eStatus)
{            
  task_id_t taskWake;
  
  taskWake=(task_id_t)msg[3];                             
  if (taskWake)
  { 
     /* Save the status code for the calling task. I realise that */
     /* this is not 100% race-condition free but it will have to  */
     /* do for now until I rework the whole send command system.  */
     eLastStatus = eStatus;
     task_wakeup(taskWake);
  }                                 
  return;
   
} /* send_video_command_done */

/*******************************************************************/
/* Function: post_video_command()                                  */
/* Parameters: msg to post                                         */
/*           : callback function (must not be null)                */
/* Return: void                                                    */
/* Remarks:  This is the asynchronous version of getting something */
/*           video process to get something done. It just posts    */
/*           the command to the VideoQueue                         */
/*                                                                 */
/* Context:  May be called from interrupt context                  */
/*******************************************************************/
void post_video_command(u_int32 *msg, gen_callback_cmd_done_t video_command_done)
{            
                         
   if (video_command_done)
   {
      msg[2] = (u_int32)video_command_done;
      msg[3] = 0;
      /* Send the command to the video_process */
      if ( qu_send(VideoQueueID, msg) != RC_OK ){
          isr_trace_new(TRACE_MPG|TRACE_LEVEL_ALWAYS,"VIDEO: qu_send failed \n", 0, 0);
          isr_error_log(VIDEO_QSEND_FAIL + ERROR_FATAL);
      }          
   } else {
      isr_trace_new(TRACE_MPG|TRACE_LEVEL_ALWAYS,"VIDEO: post_video_command with null callback!!!!!!!\n", 0, 0);
   } /* endif */
   return;   
} /* post_video_command */

/*******************************************************************/
/* Function: send_video_command_done()                             */
/* Parameters: msg to which is done                                */          
/* Return: void                                                    */
/* Remarks:  This function wakes up the task which sent the        */
/*           message. We use msg[3] as the task ID of a sent msg   */
/*******************************************************************/    
void post_video_command_done(u_int32 *msg, CNXT_VID_STATUS eStatus)
{            
   not_interrupt_safe();
   if (msg[2] != 0)
   {  
      /* JQR took out a reference to call_av_command_done here..*/
      ((gen_callback_cmd_done_t)(msg[2]))(eStatus);
   }                                 
   return;
   
} /* post_video_command_done */


#ifdef DEBUG
/********************************************************************/
/*  FUNCTION:    DumpVideoMessage                                   */
/*                                                                  */
/*  PARAMETERS:  message - video task message to dump               */
/*                                                                  */
/*  DESCRIPTION: Dump the contents of a video task command message  */
/*               to the trace output.                               */
/*                                                                  */
/*  RETURNS:     Nothing                                            */
/*                                                                  */
/*  CONTEXT:     Must be called in a non-interupt context           */
/*                                                                  */
/********************************************************************/
static void DumpVideoMessage(u_int32 *message)
{         
   const char *pMsg;
   switch (message[0])
   {
       case VIDEO_COMMAND_FLUSHED:      pMsg = "VIDEO_COMMAND_FLUSHED";      break;
       case VIDEO_COMMAND_FLUSH_VBV:    pMsg = "VIDEO_COMMAND_FLUSH_VBV";    break;
       case VIDEO_COMMAND_STILL_WAIT:   pMsg = "VIDEO_COMMAND_STILL_WAIT";   break;
       case VIDEO_COMMAND_CONTINUE:     pMsg = "VIDEO_COMMAND_CONTINUE";     break;
       case VIDEO_COMMAND_USER_DATA:    pMsg = "VIDEO_COMMAND_USER_DATA";    break;
       case VIDEO_COMMAND_PAUSE:        pMsg = "VIDEO_COMMAND_PAUSE";        break;
       case VIDEO_COMMAND_RESUME:       pMsg = "VIDEO_COMMAND_RESUME";       break;
       case VIDEO_COMMAND_STOP:         pMsg = "VIDEO_COMMAND_STOP";         break;
       case VIDEO_COMMAND_RESET:        pMsg = "VIDEO_COMMAND_RESET";        break;
       case VIDEO_COMMAND_FEED_DRIP:    pMsg = "VIDEO_COMMAND_FEED_DRIP";    break;
       case VIDEO_COMMAND_BUF_EMPTY:    pMsg = "VIDEO_COMMAND_BUF_EMPTY";    break;
       case VIDEO_COMMAND_SEND_AV_DONE: pMsg = "VIDEO_COMMAND_SEND_AV_DONE"; break;
       case VIDEO_COMMAND_PLAY:        
          switch (message[1])
          {                
               case PLAY_STILL_STATE:           pMsg = "VIDEO_COMMAND_PLAY: STILL";        break;
               case PLAY_STILL_STATE_NO_DISP:   pMsg = "VIDEO_COMMAND_PLAY: STILL_ND";     break;
               case PLAY_LIVE_STATE:            pMsg = "VIDEO_COMMAND_PLAY: LIVE";         break;
               case PLAY_LIVE_STATE_NO_SYNC:    pMsg = "VIDEO_COMMAND_PLAY: LIVE_NS";      break;
               case PLAY_DRIP_STATE:            pMsg = "VIDEO_COMMAND_PLAY: DRIP";         break;
               default:                         pMsg = "VIDEO_COMMAND_PLAY: Invalid";      break;
          } /* endswitch */
          break;
       case VIDEO_COMMAND_SET_VBUFFS:   pMsg = "VIDEO_COMMAND_SET_VBUFFS";   break;                   
       case VIDEO_COMMAND_BLANK:        pMsg = "VIDEO_COMMAND_BLANK";        break;
       case VIDEO_COMMAND_UNBLANK:     
           if(message[1])
             pMsg = "VIDEO_COMMAND_UNBLANK (deferred)";
           else
             pMsg = "VIDEO_COMMAND_UNBLANK";
           break;
       default:                         pMsg = "INVALID COMMAND";            break;
   } /* endswitch */
   trace_new(TRACE_MPG|TRACE_LEVEL_2,"VIDEO: %s\n",pMsg);
   return;   
}

#ifdef PERIODIC_VIDEO_TRACE

/********************************************************************/
/*  FUNCTION:    GetStateDescription                                */
/*                                                                  */
/*  PARAMETERS:  iState - Video driver state whose description is   */
/*                        being queried                             */
/*                                                                  */
/*  DESCRIPTION: Given a particular  video driver state, return an  */
/*               ASCII string describing it.                        */
/*                                                                  */
/*  RETURNS:     Pointer to a string describing the state           */
/*                                                                  */
/*  CONTEXT:     May be called from any context                     */
/*                                                                  */
/********************************************************************/
static char *GetStateDescription(int iState)
{
  switch (iState)
  {                
    case PLAY_OFF_STATE:             return "OFF";       
    case PLAY_STILL_STATE:           return "STILL";       
    case PLAY_STILL_STATE_NO_DISP:   return "STILL_ND";    
    case PLAY_LIVE_STATE:            return "LIVE";        
    case PLAY_LIVE_STATE_NO_SYNC:    return "LIVE_NS";     
    default:                         return "**Invalid**"; 
  } /* endswitch */
}
#endif /* PERIODIC_VIDEO_TRACE */
#endif /* DEBUG */

/********************************************************************/
/*  FUNCTION:    WaitForLiveVideoToStart                            */
/*                                                                  */
/*  PARAMETERS:  bSync - TRUE if AV sync is enabled, FALSE otherwise*/
/*                                                                  */
/*  DESCRIPTION: Wait for 2 decode complete interrupts to occur     */
/*               or, in the case of fast channel change,1 decode    */
/*               complete and an int5 from ucode instructing the    */
/*               DRM to display a buffer or a period of 5 seconds   */
/*               to elapse (as would happen if no encoded video     */
/*               data as available) then return.                    */
/*                                                                  */
/*  RETURNS:     CNXT_VID_STATUS_OK               - Video started OK*/
/*               CNXT_VID_STATUS_DECODE_1_TIMEOUT - Timeout         */
/*                   waiting forfirst video picture to be decoded   */
/*               CNXT_VID_STATUS_DECODE_2_TIMEOUT - Timeout waiting */
/*                   for second video picture to be decoded         */
/*               CNXT_VID_STATUS_DEMUX_DISABLED   - Demux disabled  */
/*                   while waiting for video to start               */
/*               CNXT_VID_STATUS_DEMUX_TIMEOUT    - Timeout waiting */
/*                   for demux to start sending data                */
/*                                                                  */
/*  CONTEXT:     Must not be called from interrupt context          */
/*                                                                  */
/********************************************************************/
CNXT_VID_STATUS WaitForLiveVideoToStart(bool bSync)
{
  int iDelay;
  int iDecodes;
  int iDecodedTillNow;
  CNXT_VID_STATUS eRetcode = CNXT_VID_STATUS_OK;
  bool ks;
  bool bFinished;
  bool bEnabled;
#ifdef DEBUG
  bool bInSync = TRUE;
#endif
  u_int16 uPID;
  MPG_ADDRESS uRead, uWrite;
  MPG_ADDRESS uReadInitial, uWriteInitial;
  #ifdef DEBUG
  LPREG lpFrameDrop = (LPREG)(MPG_FRAME_DROP_CNT_REG);
  #endif
  
  uReadInitial  = (MPG_ADDRESS)*DPS_VIDEO_READ_PTR_EX(gDemuxInstance);
  uWriteInitial = (MPG_ADDRESS)*DPS_VIDEO_WRITE_PTR_EX(gDemuxInstance);
  
  #ifdef DEBUG
  not_interrupt_safe();
  gbWaiting = TRUE; /* Dump debug from decode complete handler */
  
  trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: Waiting for video to start. Read 0x%08x, Write 0x%08x\n", (u_int32)uReadInitial, (u_int32)uWriteInitial);
 
  #endif /* DEBUG */
  
  ks = critical_section_begin();
  
  /* Get the existing decode complete count as a reference */
  iDecodes = DecodeCompleteCount;
  
  /* Reset picture display counter */
  giDRMDisplayPicture = 0;
  
  critical_section_end(ks);
  
  /* Set up for our delay loop */
  iDelay = 0;
  bFinished = FALSE;

  do
  {
    ks = critical_section_begin();
    iDecodedTillNow = DecodeCompleteCount - iDecodes;
    critical_section_end(ks);
    
    if(SafeToUnblankVideo(iDecodedTillNow, -1 ,PlayState))
    {
      /* Yes - set a flag and drop out of the loop */
      bFinished = TRUE;
    }  
    else
    {
      /* How is the buffer looking? */
      uRead  = (MPG_ADDRESS)*DPS_VIDEO_READ_PTR_EX(gDemuxInstance);
      uWrite = (MPG_ADDRESS)*DPS_VIDEO_WRITE_PTR_EX(gDemuxInstance);
      
      /* Have we started receiving data yet? */
      if((iDelay >= guVideo1Timeout) && (uWrite == uWriteInitial) && !iDecodedTillNow)
      {
        bFinished = TRUE;
        eRetcode = CNXT_VID_STATUS_NO_DATA;
      }  
      else
      {
        /* Have we timed out waiting for the first decode ? */
        if ((iDelay >= guVideo2Timeout) && !iDecodedTillNow )
        {
          bFinished = TRUE;
          eRetcode = CNXT_VID_STATUS_DECODE_1_TIMEOUT;
        }
      }  
    }  
    
    /* OpenTV has been known to disable the video demux channel while waiting for   */
    /* video to start if the user initiates another channel change before the last  */
    /* one completed. We check for this and drop out of this loop if this happens.  */
    /* Without this code, we end up seeing some long channel changes in these cases */
    /* since this loop is left to time out waiting for 2 decode completes.          */
    #ifdef DRIVER_INCL_GENDMXC
    uPID = gen_dmx_get_video_pid(TRUE);
    #else
    cnxt_dmx_get_video_pid(gDemuxInstance, TRUE, &uPID);
    #endif
    bEnabled = (uPID == GEN_NULL_PID) ? FALSE : TRUE;
    if(!bEnabled || gStoppingVideo)
    {
      eRetcode = CNXT_VID_STATUS_DEMUX_DISABLED;
      bFinished = TRUE;
    }
    
    #ifdef DEBUG
    if (!bEnabled)
    {
      trace_new(TRACE_MPG|TRACE_LEVEL_3, "VIDEO: Demux channel disabled while waiting for video to start!!!\n");
    }
    #endif
    
    if (!bFinished && bEnabled)
    {
      #ifdef DEBUG
      trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: R %08x, W %08x, Size 0x%08x, Rpt %2d, Skip %2d, PCR %08x, STC S %08x %08x, VPS %02x, VPTS %08x, BPTS %08x\n",
                *DPS_VIDEO_READ_PTR_EX(gDemuxInstance),
                *DPS_VIDEO_WRITE_PTR_EX(gDemuxInstance),
                *(LPREG)glpMpgPicSize,
                CNXT_GET_VAL(lpFrameDrop, MPG_FRAME_DROP_CNT_REPEAT_MASK),
                CNXT_GET_VAL(lpFrameDrop, MPG_FRAME_DROP_CNT_DROP_MASK),
                *glpPCRLow,
                *glpSTCSnapLo,
                *glpSTCLo,
                *((LPREG)0x380006C0),
                *glpVPTSLo,
                *((LPREG)HWBUF_PCRPTS_ADDR));
      #endif
      iDelay+= VIDEO_START_LOOP_DELAY;
      task_time_sleep(VIDEO_START_LOOP_DELAY);
    }
  }  
  while (!bFinished && bEnabled && (iDelay < guVideo3Timeout));
  
  /* Did we time out waiting for the second decode complete? */
  if(iDelay >= guVideo3Timeout)
  {
    if(!bSync)
      eRetcode = CNXT_VID_STATUS_DECODE_2_TIMEOUT;
    else
      eRetcode = CNXT_VID_STATUS_SYNC_TIMEOUT;
  }      
  
  /* If we somehow try to decode an HD stream, the microcode will set the
     video width to the maximum (0x3f0).  In that case, return an error so
     the application does not unblank! */
  if (gdwSrcWidth >= 0x3f0)
  {
    eRetcode = CNXT_VID_STATUS_SYS_ERROR;
  }

  #ifdef DEBUG
  if (eRetcode != CNXT_VID_STATUS_OK)
  {
    trace_new(TRACE_MPG|TRACE_LEVEL_ALWAYS, "VIDEO: Video did not start as expected. Reason %d - %s!\n", eRetcode, strVideoStart[eRetcode]);
  }
  else
  {
    trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: Video has started\n");
  }  
  gbWaiting = FALSE;
  
  if(bSync)
  {
    trace_new(TRACE_MPG|TRACE_LEVEL_ALWAYS, "VIDEO: On exit video is %s sync. Delay %d pictures\n", (bInSync ? "in": "out of"), iDecodedTillNow);
  }  
  
  #endif  
  
  return(eRetcode);
}

/********************************************************************/
/*  FUNCTION:    WaitForVideoIdle                                   */
/*                                                                  */
/*  PARAMETERS:  uTimeoutMs - Number of milliseconds to wait before */
/*                            returning FALSE.                      */
/*                                                                  */
/*  DESCRIPTION: Wait up to uTimeoutMs for the video decoder to     */
/*               signal that it is idle.                            */
/*                                                                  */
/*  RETURNS:     TRUE - signal received, core is idle               */
/*               FALSE - timed out waiting for idle signal          */
/*                                                                  */
/*  CONTEXT:     Must not be called from interrupt context.         */
/*                                                                  */
/********************************************************************/
static bool WaitForVideoIdle(u_int32 uTimeoutMs)
{
  bool ks;
  int  iRetcode;
  
  /* Turn on the idle interrupt */
  trace_new(TRACE_MPG|TRACE_LEVEL_4,"VIDEO: ** Enabling idle interrupt r/w 0x%08x, 0x%08x**\n",
  *((LPREG)DPS_VIDEO_READ_PTR_EX(gDemuxInstance)),
  *((LPREG)DPS_VIDEO_WRITE_PTR_EX(gDemuxInstance)));
        
  ks=critical_section_begin();
  *glpIntMask |= MPG_VIDEO_DEC_INTERRUPT6;
  *pgnFieldCount = 0;
  critical_section_end(ks);
 
  /* Wait a while for the idle interrupt to fire */
  iRetcode = sem_get(semVideoPlay, uTimeoutMs);
  
  if(iRetcode == RC_OK)
  {
    /* Video has gone idle so we don't need to turn the interrupt off (the ISR */
    /* will have done this already).                                           */
    trace_new(TRACE_MPG|TRACE_LEVEL_4,"VIDEO: ** Got idle sem ISR - video is idle r/w 0x%08x, 0x%08x **\n",
    *((LPREG)DPS_VIDEO_READ_PTR_EX(gDemuxInstance)),
    *((LPREG)DPS_VIDEO_WRITE_PTR_EX(gDemuxInstance)));
    return(TRUE);
  }  
  else
  {
    trace_new(TRACE_MPG|TRACE_LEVEL_4,"VIDEO: ** Failed to get idle sem from ISR!! r/w 0x%08x, 0x%08x **\n",
    *((LPREG)DPS_VIDEO_READ_PTR_EX(gDemuxInstance)),
    *((LPREG)DPS_VIDEO_WRITE_PTR_EX(gDemuxInstance)));
    
    /* We got no idle interrupt within the timeout specified. We must disable the */
    /* interrupt ourselves and clear the semaphore (just in case an interrupt     */
    /* happens between returning from sem_get and here)                           */
    ks=critical_section_begin();
    *glpIntMask &= ~MPG_VIDEO_DEC_INTERRUPT6;
    critical_section_end(ks);
    
    /* Clear the semaphore */
    while(sem_get(semVideoPlay, 0) == RC_OK);
    
    return(FALSE);
  }  
}


/********************************************************************/
/*  FUNCTION:    WaitForStillDecodeToComplete                       */
/*                                                                  */
/*  PARAMETERS:  uTimeout - Max time to wait in mS.                 */
/*                                                                  */
/*  DESCRIPTION: Wait a period of time for a block of MPEG still    */
/*               data to be decoded.                                */
/*                                                                  */
/*  RETURNS:     CNXT_VID_STATUS_OK               - Still decoded   */
/*                   OK                                             */
/*               CNXT_VID_STATUS_DECODE_1_TIMEOUT - Timeout waiting */
/*                   for still decode to complete                   */
/*               CNXT_VID_STATUS_DECODE_2_TIMEOUT - Timeout waiting */
/*                   forvideo to go idle after decode completed.    */
/*                                                                  */
/*  CONTEXT:     Must not be called from interrupt context          */
/*                                                                  */
/********************************************************************/
static CNXT_VID_STATUS WaitForStillDecodeToComplete(u_int32 uTimeout)
{
  u_int32 uDelay = 0;
  u_int32 uWriteOnEntry;
  u_int32 uReadOnEntry;
  u_int32 uFullness;
  CNXT_VID_STATUS eStatus = CNXT_VID_STATUS_OK;
  bool    bRetcode;
    
  /* Normally, all we need to do here is post a stop command and   */
  /* wait for an idle interrupt. However, this breaks if the still */
  /* contains multiple pictures since the stop will take effect    */
  /* at the end of whichever picture was being decoded when the    */
  /* command was sent rather than at the end of the data. To get   */
  /* round this, we wait for the encoded data read pointer to      */
  /* catch up with the write pointer before we issue the stop      */
  /* command.                                                      */
  
  /* What are the current read and write pointers? */
  uReadOnEntry  = *DPS_VIDEO_READ_PTR_EX(gDemuxInstance);
  uWriteOnEntry = *DPS_VIDEO_WRITE_PTR_EX(gDemuxInstance);
  
  uFullness = GetVideoBufferFullness();

  trace_new(TRACE_MPG|TRACE_LEVEL_4,"VIDEO: Waiting for still decode to complete. Buffer contains %dKB. r/w 0x%08x/0x%08x\n",
            uFullness/1024,
            uReadOnEntry, 
            uWriteOnEntry);
  
  /* Wait until all the real data has been read or we time out */
  while((uDelay < uTimeout) && (uFullness > DECODER_BUFFER_SIZE))
  {
    task_time_sleep(20);
    uDelay += 20;
    uFullness = GetVideoBufferFullness();
  } 
  
  if(uDelay >= uTimeout)
  {
    trace_new(TRACE_MPG|TRACE_LEVEL_4, "VIDEO: Timed out waiting for still data to be consumed! r/w 0x%08x/0x%08x\n",
             *DPS_VIDEO_READ_PTR_EX(gDemuxInstance),
             *DPS_VIDEO_WRITE_PTR_EX(gDemuxInstance));
    
    eStatus = CNXT_VID_STATUS_DECODE_1_TIMEOUT;
  }
  
  trace_new(TRACE_MPG|TRACE_LEVEL_4, "VIDEO: Sending stop command after waiting for still...\n");
  
  /* Tell the hardware we are done then wait for it to complete */
  video_send_hardware_stop_command();
  
  /* Pad the encoded video buffer a bit more. This guards against the case where the buffer has */
  /* emptied before we send the stop command. If the buffer is empty, the decoder stops looking */
  /* for new commands. Clever, eh?                                                              */
  SendPaddingAndStartCode(0xB7, DECODER_BUFFER_SIZE);
  
  bRetcode = WaitForVideoIdle(uTimeout);
  if(!bRetcode)
  {
    trace_new(TRACE_MPG|TRACE_LEVEL_4,"VIDEO: Timed out waiting for still decode! r/w 0x%08x/0x%08x\n",
           *DPS_VIDEO_READ_PTR_EX(gDemuxInstance),
           *DPS_VIDEO_WRITE_PTR_EX(gDemuxInstance));
           
    /* Try freeing up the decode */
    SendFrameHeader();
    
    /* Wait again... */
    bRetcode = WaitForVideoIdle(uTimeout);
    if(!bRetcode)
    {
      trace_new(TRACE_MPG|TRACE_LEVEL_4,"VIDEO: Timed out again. Reporting an error! r/w 0x%08x/0x%08x\n",
             *DPS_VIDEO_READ_PTR_EX(gDemuxInstance),
             *DPS_VIDEO_WRITE_PTR_EX(gDemuxInstance));
      eStatus = CNXT_VID_STATUS_DECODE_2_TIMEOUT;
    }
  }
  
  if(bRetcode)
  {                         
    trace_new(TRACE_MPG|TRACE_LEVEL_4, "VIDEO: Stop completed correctly.\n");
  }
  
  /* Make sure we set the flag indicating that we are ready to decode */
  /* another still. This should not be needed here since it is set on */
  /* gen_video_stop but some naughty apps don't call this function so */
  /* we need to set the flag here too.                                */
  AllowStillCmd = TRUE;
      
  return(eStatus);
}
/********************************************************************/
/*  FUNCTION:    GetVideoBufferFullness                             */
/*                                                                  */
/*  PARAMETERS:  None                                               */
/*                                                                  */
/*  DESCRIPTION: Return the number of bytes of data currently in    */
/*               the encoded video buffer.                          */
/*                                                                  */
/*  RETURNS:     Number of bytes in the buffer                      */
/*                                                                  */
/*  CONTEXT:     May be called in any context                       */
/*                                                                  */
/********************************************************************/
static u_int32 GetVideoBufferFullness(void)
{
  u_int32 uRead, uWrite;
  u_int32 uFullness;
  
  /* Get the current video read and write pointers */
  uRead  = *DPS_VIDEO_READ_PTR_EX(gDemuxInstance);
  uWrite = *DPS_VIDEO_WRITE_PTR_EX(gDemuxInstance);

  /* Mask off everything other than the address */
  uRead &= 0x003FFFFF;
  uWrite &= 0x003FFFFF;
  
  /* If the write pointer is above the read pointer, just subtract */
  if(uWrite >= uRead)
    uFullness = uWrite - uRead;
  else
  {
    /* One has wrapped while the other hasn't */
    uFullness = (uWrite - HWBUF_ENCVID_ADDR) + 
                ((HWBUF_ENCVID_ADDR+HWBUF_ENCVID_SIZE) - uRead);
  }  

  return(uFullness);
}

/********************************************************************/
/*  FUNCTION:    SetReadPtr                                         */
/*                                                                  */
/*  PARAMETERS:  uTarget - target fullness for the encoded video    */
/*                         buffer.                                  */
/*               bOverflowed - TRUE if the buffer overflowed prior  */
/*                         to making this call, FALSE otherwise.    */
/*                                                                  */
/*  DESCRIPTION: Advance the encoded MPEG buffer read pointer to    */
/*               acheive a buffer fullness of uTarget. If there is  */
/*               not enough data available, leave the pointer where */
/*               it is.                                             */
/*                                                                  */
/*  RETURNS:     Nothing                                            */
/*                                                                  */
/*  CONTEXT:     MUST be called from within a critical section or   */
/*               from an interrupt service routine.                 */
/*                                                                  */
/********************************************************************/
static u_int32 SetReadPtr(u_int32 uTarget, bool bOverflowed)
{
  u_int32 uRead;
  u_int32 uFullness;
  
  /* How full is the buffer just now? */
  if (!bOverflowed)
    uFullness = GetVideoBufferFullness();
  else
    uFullness = HWBUF_ENCVID_SIZE;  
  
  /* If there is less data in the buffer than we would like, just return */
  if(uFullness <= uTarget)
    return(uFullness);
      
  /* Update the read value taking wrapping into account */
  uRead = (u_int32)*DPS_VIDEO_WRITE_PTR_EX(gDemuxInstance) + (HWBUF_ENCVID_SIZE-uTarget);

  if ((uRead & 0x3FFFFF) >= HWBUF_ENCVID_ADDR+HWBUF_ENCVID_SIZE)
  {
    /* We have wrapped the read pointer */
    uRead -= HWBUF_ENCVID_SIZE;
    uRead ^= DPS_BUFFER_WRAP_INDICATOR;
  }  
  
  *DPS_VIDEO_READ_PTR_EX(gDemuxInstance) = uRead;
  
  return(uFullness);
}

/********************************************************************/
/*  FUNCTION:    gen_video_decode_still_image                       */
/*                                                                  */
/*  PARAMETERS:  pImage - pointer to still picture MPEG data.       */
/*               uSize  - number of bytes of data at pImage.        */
/*                                                                  */
/*  DESCRIPTION: Decode and display the passed MPEG still picture.  */
/*                                                                  */
/*  RETURNS:     TRUE on success, FALSE otherwise                   */
/*                                                                  */
/*  CONTEXT:     Must not be called from interrupt context.         */
/*                                                                  */
/********************************************************************/
bool gen_video_decode_still_image(voidF *pImage, u_int32 uSize)
{
    bool bRetcode;
    CNXT_VID_STATUS eRetcode;
    gen_farcir_q VideoQueue;
    int          iBytesConsumed;
    
    trace_new(TRACE_MPG|TRACE_LEVEL_3, "VIDEO: Decoding still 0x%08x, size %d bytes\n", (u_int32)pImage, uSize);
    
    eRetcode = gen_video_play(PLAY_STILL_STATE, (gen_callback_cmd_done_t)NULL);

    if (eRetcode == CNXT_VID_STATUS_OK)
    {
      VideoQueue.begin = pImage;
      VideoQueue.size  = (int)uSize+1;
      VideoQueue.in    = (int)uSize;
      VideoQueue.out   = 0;

      while (VideoQueue.in != VideoQueue.out)
      {
          iBytesConsumed = gen_send_still_data((gen_farcir_q *) &VideoQueue);
      }
      bRetcode = gen_flush_still_data((gen_callback_cmd_done_t)NULL);
    }
    else
      bRetcode = FALSE;
    
    return(bRetcode);
}

/********************************************************************/
/*  FUNCTION:    gen_video_decode_blank                             */
/*                                                                  */
/*  PARAMETERS:  None                                               */
/*                                                                  */
/*  DESCRIPTION: Instruct the video driver to decode a blank        */
/*               MPEG still to effectively clear the video buffer.  */
/*               This function is intended for use during startup   */
/*               to ensure that no "flash of crud" is seen when     */
/*               video is initially unblanked.                      */
/*                                                                  */
/*  RETURNS:     Nothing                                            */
/*                                                                  */
/*  CONTEXT:     Must be called from non-interrupt context          */
/*                                                                  */
/********************************************************************/
void gen_video_decode_blank(void)
{
  #ifdef BLANK_BEFORE_ALL_STILLS
  /* This is called during startup before the graphics drivers have   */
  /* been initialised. We need to signal that the video driver should */
  /* not attempt to call the blit library to fill the video plane     */
  /* before decoding this still since this will cause KAL errors.     */
  bStillBlank = FALSE;
  #endif
  
  trace_new(TRACE_LEVEL_2|TRACE_MPG, "VIDEO: Decoding small black still\n");
  
  gen_video_decode_still_image((voidF)black16_image, sizeof(black16_image));

  trace_new(TRACE_LEVEL_2|TRACE_MPG, "VIDEO: Decode finished\n");
  
  #ifdef BLANK_BEFORE_ALL_STILLS
  /* Get rid of the flag telling the driver not to call the blitter */
  bStillBlank = TRUE;
  #endif
  
}
/********************************************************************/
/*  FUNCTION:    gen_video_invalidate_display_buffer                */
/*                                                                  */
/*  PARAMETERS:  None                                               */
/*                                                                  */
/*  DESCRIPTION: Marks the currently visible video display buffer   */
/*               as invalid. This may be used in conjunction with   */
/*               deferred blanking to ensure that an unblank call   */
/*               will only take effect after the next decode        */
/*               regardless of whether or not a good image exists   */
/*               in the display buffer already.                     */
/*                                                                  */
/*  RETURNS:     Nothing                                            */
/*                                                                  */
/*  CONTEXT:     May be called in any context                       */
/*                                                                  */
/********************************************************************/
void gen_video_invalidate_video_display_buffer(void)
{
	int ks, iDecodedIndex;
	
	ks = critical_section_begin();


    /* Which buffer will be displayed if we unblank now? */
    switch ((u_int32)gpVideoDisplayBuffer)
    {
      case HWBUF_DEC_I_ADDR: iDecodedIndex = DEC_I_BUFFER_INDEX; break;
      case HWBUF_DEC_P_ADDR: iDecodedIndex = DEC_P_BUFFER_INDEX; break;
      default:               iDecodedIndex = -1;                 break;
    }

	/* If it is displaying a still, invalidate it so the unblank will 
	   be deffered */
	if(gsDecodedImages[iDecodedIndex].dwStartingMode == PLAY_STILL_STATE)
	    gsDecodedImages[iDecodedIndex].dwStartingMode = PLAY_STILL_STATE_NO_DISP;


    critical_section_end(ks); 
}


/********************************************************************/
/*  FUNCTION:    gen_video_get_still_panscan_vector                 */
/*                                                                  */
/*  PARAMETERS:  pPan    - storage for returned pan vector          */
/*               pScan   - storage for returned scan vector         */
/*               pbFound - storage for value indicating whether or  */
/*                         not the last decoded still contained a   */
/*                         pan/scan vector. TRUE if it did, FALSE   */
/*                         otherwise.                               */
/*                                                                  */
/*  DESCRIPTION: This function returned the pan and scan values     */
/*               parsed from the last MPEG still that was decoded.  */
/*               If no vector was found, 0 will be returned for     */
/*               both components and *pbFound will be FALSE.        */
/*                                                                  */
/*  RETURNS:     TRUE on success, FALSE on error.                   */
/*                                                                  */
/*  CONTEXT:     May be called in any context.                      */
/*                                                                  */
/********************************************************************/
bool gen_video_get_still_panscan_vector(int *pPan, int *pScan, bool *pbFound)
{
  if(pPan && pScan && pbFound)
  {
    if(bStillVectorFound)
    {
      *pPan    = iStillPanOffset/16;
      *pScan   = iStillScanOffset/16;
      *pbFound = TRUE;
    }
    else
    {
      *pPan    = 0;
      *pScan   = 0;
      *pbFound = FALSE;
    }
    return(TRUE);
  }
  else
  {
    trace_new(TRACE_LEVEL_ALWAYS|TRACE_MPG, "VIDEO: NULL pointer passed to gen_video_get_still_panscan_vector!!!!\n");
    return(FALSE);
  }
}


/********************************************************************/
/*  FUNCTION:    gen_video_is_motion_video_playing                  */
/*                                                                  */
/*  PARAMETERS:  None                                               */
/*                                                                  */
/*  DESCRIPTION: Return TRUE if the driver is currently playing     */
/*               motion video, FALSE otherwise                      */
/*                                                                  */
/*  RETURNS:     TRUE/FALSE                                         */
/*                                                                  */
/*  CONTEXT:     May be called in any context                       */
/*                                                                  */
/********************************************************************/
bool gen_video_is_motion_video_playing(void)
{
  return(((PlayState == PLAY_LIVE_STATE) || (PlayState == PLAY_LIVE_STATE_NO_SYNC)) ? TRUE : FALSE);
}


/********************************************************************/
/*  FUNCTION:    gen_video_frame_displayed_signal                   */
/*                                                                  */
/*  PARAMETERS:  uBufferPtr   - Address of the buffer that the DRM  */
/*                              is being signalled to display.      */
/*               iBufferIndex - Indication of which anchor picture  */
/*                              is being signalled. -1 if a B frame.*/
/*                                                                  */
/*  DESCRIPTION: This function is called by the OSDLIBC interrupt   */
/*               handler when an INT5 is detected. This interrupt   */
/*               indicates that the DRM has been signalled to show  */
/*               a new MPEG picture. The function here handles any  */
/*               special processing that the video driver may need  */
/*               to do at this point including deferred unblanking. */
/*                                                                  */
/*  RETURNS:     Nothing                                            */
/*                                                                  */
/*  CONTEXT:     Will be called in interrupt context. Must NOT be   */
/*               called by application code - this in an internal   */
/*               driver-to-driver signalling function only.         */
/*                                                                  */
/********************************************************************/
void gen_video_frame_displayed_signal(u_int32 uBufferPtr, int iBufferIndex)
{
  bool ks;
  int  iCountCopy;
  #ifdef DEBUG
  LPREG lpFrameDrop = (LPREG)(MPG_FRAME_DROP_CNT_REG);
  bool bForceDebug = FALSE;
  #endif
  
  ks = critical_section_begin();
 	giDRMDisplayPicture++;
  iCountCopy = giDRMDisplayPicture;
  critical_section_end(ks);
  
   #ifdef DEFERRED_UNBLANK
   /* See if we have to unblank the video. */
   if (gbDeferredUnblank)
   {
     /* Is it safe to unblank now? */
     if (SafeToUnblankVideo(iCountCopy, iBufferIndex, PlayState))
     {
       /* Yes, so unblank the video plane */
       gen_video_unblank_internal();
       gbDeferredUnblank = FALSE;
#if ((PVR==YES)||(XTV_SUPPORT==YES))
       /* Tell the PVR playback driver we're unblanking now */
       if(Video_Unblank_Callback_Function)
       {
           Video_Unblank_Callback_Function(Video_Unblank_Callback_Tag);
       }
#endif
       isr_trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: Deferred video unblank completed\n",0 ,0);
       #ifdef DEBUG
       bForceDebug = TRUE;
       #endif
     }
 
     /* ...otherwise wait for the next decode and keep checking */
   }    
   #endif /* DEFERRED_UNBLANK */
   
  #ifdef DEBUG
  /* Dump periodic information on skip and repeat counts for a period of */
  /* time after video starts up.                                         */
  if((((giDRMDisplayPicture < DEBUG_INFO_MAX_PICTURES) && !(giDRMDisplayPicture % DEBUG_INFO_RATE))) || bForceDebug)
  {
    isr_trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: Repeat %d, skip %d\n", 
                  CNXT_GET_VAL(lpFrameDrop, MPG_FRAME_DROP_CNT_REPEAT_MASK),
                  CNXT_GET_VAL(lpFrameDrop, MPG_FRAME_DROP_CNT_DROP_MASK));
    #if (MPG_SYNC_BIT_POSITION == MPG_SYNC_BIT_POSITION_WABASH)
    isr_trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: Sync %s, early sync %s\n",
	       (u_int32)((CNXT_GET(MPG_ADDR_EXT_REG, MPG_ADDR_EXT_INSYNC_MASK) == MPG_ADDR_EXT_INSYNC_MASK) ? "SET" : "CLEAR"),
         (u_int32)((CNXT_GET(MPG_EARLY_SYNC_REG, MPG_EARLY_SYNC_MASK) == MPG_EARLY_SYNC_MASK) ? "SET" : "CLEAR"));
    #endif
  }                
  #endif  
   
}

/********************************************************************/
/*  FUNCTION:    SafeToUnblankVideo                                 */
/*                                                                  */
/*  PARAMETERS:  iDisplayCount - Cumulative count of displayed      */
/*                               pictures since gen_video_play was  */
/*                               called.                            */
/*               iDisplayIndex - Index of picture buffer that is    */
/*                               about to be displayed.             */
/*                                                                  */
/*               iState        - Current decoder state              */
/*                                                                  */
/*  DESCRIPTION: This function checks various conditions to         */
/*               determine whether it is safe to unblank the video  */
/*               plane after decoding a still or starting motion    */
/*               video playback.                                    */
/*                                                                  */
/*  RETURNS:     TRUE if it is safe to unblank, FALSE if not.       */
/*                                                                  */
/*  CONTEXT:     May be called in any context                       */
/*                                                                  */
/********************************************************************/
static bool SafeToUnblankVideo(int iDisplayCount, int iDisplayIndex, int iState)
{
  bool bRetcode = FALSE;
  bool bInSync  = FALSE;

  switch(iState)
  {
    /**********************************/
    /* Synchronised motion video case */
    /**********************************/
    case PLAY_LIVE_STATE:
      /* This is the complicated case. We need to check both the number of    */
      /* pictures and also various flags set by the microcode to indicate     */
      /* the sync state of the stream. We declare that it is safe to          */
      /* unblank either when we see 2 pictures decoded and the sync bit set   */
      /* or, if fast unblank is selected, after at least 1 picture is decoded */
      /* and the "nearly sync" bit is set.                                    */
      
      /* First, determine whether the relevant sync bit is set */
      #if (BRAZOS_WABASH_FAST_UNBLANK == YES)
        #if MPG_SYNC_BIT_POSITION == MPG_SYNC_BIT_POSITION_COLORADO    
           bInSync = (CNXT_GET(glpMpgPicSize, MPG_VID_MASK_IN_SYNC) == MPG_VID_IN_SYNC);
        #endif

        #if MPG_SYNC_BIT_POSITION == MPG_SYNC_BIT_POSITION_WABASH && VIDEO_UCODE_TYPE != \
           VMC_MACROBLOCK_ERROR_RECOVERY
	       bInSync = (CNXT_GET(MPG_ADDR_EXT_REG, MPG_ADDR_EXT_INSYNC_MASK) == MPG_ADDR_EXT_INSYNC_MASK);
        #endif
	  
        #if MPG_SYNC_BIT_POSITION == MPG_SYNC_BIT_POSITION_WABASH && VIDEO_UCODE_TYPE == \
           VMC_MACROBLOCK_ERROR_RECOVERY
	       bInSync = (CNXT_GET(MPG_EARLY_SYNC_REG, MPG_EARLY_SYNC_MASK) == MPG_EARLY_SYNC_MASK);
        #endif 
      #else  /* BRAZOS_WABASH_FAST_UNBLANK == YES */
         #if MPG_SYNC_BIT_POSITION == MPG_SYNC_BIT_POSITION_COLORADO    									
            bInSync = (CNXT_GET(glpMpgPicSize, MPG_VID_MASK_IN_SYNC) == MPG_VID_IN_SYNC);					   																			       
         #elif MPG_SYNC_BIT_POSITION == MPG_SYNC_BIT_POSITION_WABASH									   
            bInSync = (CNXT_GET(MPG_ADDR_EXT_REG, MPG_ADDR_EXT_INSYNC_MASK) == MPG_ADDR_EXT_INSYNC_MASK);     
         #endif																							   
      #endif /* BRAZOS_WABASH_FAST_UNBLANK == YES */

      /* If the relevant sync bit is set and we have decoded at least the minimum */
      /* number of pictures then declare that it is safe to unblank video.        */
      if(bInSync && (iDisplayCount >= VIDEO_START_DECODE_DELAY))
      {
        isr_trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: Declaring safe to unblank sync video after %d pictures\n", iDisplayCount, 0);
        bRetcode = TRUE;
      }  
        
      break;
  
    /************************************/
    /* Unsynchronised motion video case */
    /************************************/
    case PLAY_LIVE_STATE_NO_SYNC:
      /* In the case of unsynchronised playback, we merely wait for 2 pictures to have */
      /* been sent to DRM then declare that it is safe to unblank.                     */
      if(iDisplayCount > 1)
      {
        isr_trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: Declaring safe to unblank unsync video after %d pictures %d\n", iDisplayCount, 0);
        bRetcode = TRUE;
      }  
      break;
    
    /**************************************************************************/
    /* By default, we declare that unblank is safe if the buffer which will   */
    /* be displayed contains an image that was originally decoded for display */
    /**************************************************************************/
    default:  
      if ((iDisplayIndex >= 0) && (gsDecodedImages[iDisplayIndex].dwStartingMode != PLAY_STILL_STATE_NO_DISP))
      {
        isr_trace_new(TRACE_MPG|TRACE_LEVEL_2, "VIDEO: Declaring safe to unblank in state %d\n", iState, 0);
        bRetcode = TRUE;
      }  
      break;
  }
   
  return(bRetcode);  
}

/*************************************************************************
 *
 * START: VIDEO DRIP DRIVER CODE
 *
 ************************************************************************/
#define VBUF_LEVEL ((HWBUF_ENCVID_SIZE * 70)/100)

/*******************************************************************/
/* Function: copy_drip_data()                                      */
/* Parameters: none                                                */
/* Return:     none                                                */
/* Remarks:  This function copies data from the Current Drip       */
/*           buffer to the Hardware Video buffer.                  */
/*******************************************************************/
void copy_drip_data(void)
{
  char * read_ptr;  /* hardware encoded audio/video buffer read pointer */
  char * write_ptr; /* hardware encoded audio/video buffer write pointer */
  int vb_bytes_left;
  int db_bytes_left;
  int SaveDripOut;
  bool ks;

  /* While there are Drip Buffers to source from, and
   * available space in the Video buffer to sink to, keep
   * copying the data
   */

  while(1)
    {
      if (DripIn == DripOut)
	break;    /* No more Drip Buffers */

      /* How much data is in the Video buffer? */
      read_ptr = (char *)((u_int32)*DPS_VIDEO_READ_PTR_EX(gDemuxInstance) & ~0x80000000);
      write_ptr = (char *)((u_int32)*DPS_VIDEO_WRITE_PTR_EX(gDemuxInstance) & ~0x80000000);
      if (read_ptr <= write_ptr) 
	vb_bytes_left = write_ptr - read_ptr;
      else 
	vb_bytes_left = write_ptr + HWBUF_ENCVID_SIZE - read_ptr;
      
      if (vb_bytes_left >= VBUF_LEVEL)
	break;    /* Too full */
      
      /* How much data is in the current Drip Buffer? */
      /* This value will always be >0 because of other checks */
      db_bytes_left = vdreqs[DripOut].in - vdreqs[DripOut].out;

      /* Copy from the Drip Buffer to the Video buff the lesser of:
         1. VBUF_LEVEL - vb_bytes_left
         2. Number of bytes left in the Drip Buffer   */
      
      if ((VBUF_LEVEL - vb_bytes_left) < db_bytes_left)
	{
	  copy_data(&vdreqs[DripOut], (VBUF_LEVEL - vb_bytes_left) ,FALSE);           
	}
      else
	/* Copy the final bytes in this drip buffer */
	{
	  /* Is this is the last available Video Drip Buffer? */
	  if (((DripOut + 1) & (VDREQ_ARRAY_SIZE - 1)) == DripIn)
	    /* Yes, so prepare for the Hardware Video Buffer */
	    /* to run dry soon after this call to copy_data(). */
	    {
	      /* Turn on the buffer empty interrupt */
	      ks = critical_section_begin();
	      *glpIntStatus |= MPG_VB_EMPTY;
	      *glpIntMask |= MPG_VB_EMPTY;
	      critical_section_end(ks);
	    }
	  copy_data(&vdreqs[DripOut], db_bytes_left ,FALSE);           
	  /* Remove empty Drip buffer from the Request Queue */
          SaveDripOut = DripOut;
	  DripOut = ((DripOut + 1) & (VDREQ_ARRAY_SIZE - 1));
	  /* Notify the application */
	  if ((int)DripBufEmpty != 0)
	    (*DripBufEmpty)(SaveDripOut);
	}
      if (StartDrip)
	{
	  video_send_hardware_command(MPG_COMMAND_PLAY);

          /* The following PAUSE/CONTINUE pair is inserted to work
	   around a Video Microcode bug.  Without this code, you
	   will notice an ugly blocky glitch about 50% of the time
	   after the first manual PAUSE/CONTINUE after first beginning
	   to play the Video Drip data.  If the Video microcode is
	   fixed, these 2 lines can be removed. */
	  video_send_hardware_stop_command();
	  video_send_hardware_command(MPG_COMMAND_CONTINUE);
	  StartDrip = FALSE;
	}
    }
}

/*******************************************************************/
/* Function: gen_video_register_drip_callbacks()                   */
/* Parameters: DBEmpty - Pointer to "Drip Buffer Empty" callback.  */
/*             VBEmpty - Pointer to "Video Buffer Empty" callback. */
/* Return:   none                                                  */
/* Remarks:  This function registers the Video Drip Callback       */
/*           routines.                                             */
/*******************************************************************/
void gen_video_register_drip_callbacks(DRIP_CALLBACK DBEmpty,
				       DRIP_CALLBACK VBEmpty)
{
  DripBufEmpty  = DBEmpty;/* To be called when a Drip buffer runs empty. */
  VideoBufEmpty = VBEmpty;/* To be called when the Video buffer runs empty. */
}

/*******************************************************************/
/* Function: gen_video_drip_request()                              */
/* Parameters: bufptr  - Pointer to a Video Drip Buffer            */
/*             buflen  - length of data in the Video Drip Buffer   */
/* Return:   DripID or Error code.                                 */
/* Remarks:  This function will queue a Drip Request and send a    */
/*           message to the video driver to feed the Drip          */
/*           Display.  It does not block the video driver.         */
/*******************************************************************/
int  gen_video_drip_request(char *bufptr, int buflen)
{
  u_int32 message[4];
  bool ks;
  int ret;
 
  ks = critical_section_begin();

  if (buflen <= 0)
    ret = CNXT_VID_STATUS_NULL_LEN_REQ;
  else if (PlayState != PLAY_DRIP_STATE)
    ret = CNXT_VID_STATUS_DRIP_INACTIVE;
  else if (((DripIn + 1) & (VDREQ_ARRAY_SIZE - 1)) == DripOut)
    ret = CNXT_VID_STATUS_DRIP_Q_FULL;
  else
    {
      ret = DripIn;

      /* Que up the request */
      vdreqs[DripIn].begin = (void*)bufptr;
      vdreqs[DripIn].size  = buflen + 1;
      vdreqs[DripIn].in    = buflen;
      vdreqs[DripIn].out   = 0;
      DripIn = (DripIn + 1) & (VDREQ_ARRAY_SIZE - 1);
    }

  critical_section_end(ks);
  
  message[0] = VIDEO_COMMAND_FEED_DRIP;
  qu_send(VideoQueueID, message);

  return ret;
}

/*******************************************************************/
/* Function: tick_timer_drip_callback                              */
/* Parameters: tickTimer - timer ID                                */
/*             pUserData - not used                                */
/* Return:   void                                                  */
/* Remarks:  This timer callback will be called ever second as     */
/*           long as drip mode is active.                          */
/*******************************************************************/
void tick_timer_drip_callback(tick_id_t tickTimer, void *pUserData) {
    u_int32 message[4];

    message[0] = VIDEO_COMMAND_FEED_DRIP;
    qu_send(VideoQueueID, message);
}

void gen_video_stop_drip(void) {

    isr_trace_new(TRACE_MPG|TRACE_LEVEL_2,"VIDEO DRIP: gen_video_stop_drip\n",0,0);

    tick_destroy(VideoDripTick);

    /* Send down a PAUSE command followed by a Frame Header to assure that the final frame
       is flushed to the screen.  The FrameHeader will generate an error which will cause the
       video microcode to display the last decoded image */
    video_send_hardware_stop_command();
    SendFrameHeader();
    
}

/*************************************************************************
 *
 * END: VIDEO DRIP DRIVER CODE
 *
 ************************************************************************/

#if ((PVR==YES)||(XTV_SUPPORT==YES))
/*******************************************************************/
/* Function: gen_video_set_es_buffer_callback()                    */
/* Parameters: NotifyFn - Function to call for buffer events.      */
/*             pTag     - 32bit returned on the callbacks          */
/* Return:   TRUE for success, FALSE otherwise                     */
/* Remarks:  This function sets the  callback notification         */
/*           function used for buffer event callbacks.             */
/*******************************************************************/
bool gen_video_set_es_buffer_callback(CNXT_ES_BUFFER_CALLBACK NotifyFn,
                                      void                    *pTag)
{
    /*
     * Set the ES buffer notification function
     */
    Video_ES_Buffer_Callback_Function = NotifyFn;
    Video_ES_Buffer_Callback_Tag      = pTag;

    return (TRUE);
}

/*******************************************************************/
/* Function: gen_video_set_es_buffer_lwm()                         */
/* Parameters: Buffer - Es Buffer to apply LWM to                  */
/*             lwm    - low-water mark value                       */
/* Return:   TRUE for success, FALSE otherwise                     */
/* Remarks:  This function sets the  callback notification         */
/*           function used for buffer event callbacks.             */
/*******************************************************************/
bool gen_video_set_es_buffer_lwm(CNXT_ES_BUFFER Buffer,
                                 u_int32        lwm)
{
    bool rc = TRUE;

    if(Buffer == CNXT_ES_BUFFER_VIDEO)
    {
        /*
         * Ensure the lwm value is not out of bounds for the buffer
         * and that it is 64byte aligned, as required by hardware.
         */
        if((!(lwm & 0x3F)) &&
            (lwm < (*DPS_VIDEO_END_ADDR_EX(0) - *DPS_VIDEO_START_ADDR_EX(0))))
        {
            /*
             * Set the Low-water mark for an ES buffer.
             *
             * IMPORTANT NOTE:  Rio designs assume Demux 0 is the only
             *                  playback demux!
             */
            *DPS_VIDEO_LOWWATERMARK_EX(0) = lwm;
        }
        else
        {
            rc = FALSE;
        }
    }
    else
    {
        rc = FALSE;
    }

    return (rc);
}

/*******************************************************************/
/* Function: gen_video_get_es_buffer_info()                        */
/* Parameters: Buffer - Es Buffer to apply LWM to                  */
/*             size   - buffer size (returned)                     */
/*             free   - free space (returned)                      */
/*             lwm    - low-water mark value (returned)            */
/* Return:   TRUE for success, FALSE otherwise                     */
/* Remarks:  This function sets the  callback notification         */
/*           function used for buffer event callbacks.             */
/*******************************************************************/
bool gen_video_get_es_buffer_info(CNXT_ES_BUFFER Buffer,
                                  u_int32        *size,
                                  u_int32        *free,
                                  u_int32        *lwm)
{
    u_int32 buffer_size;
    bool rc = TRUE;

    if(Buffer == CNXT_ES_BUFFER_VIDEO)
    {
        /*
         * Get the ES buffer information
         *
         * IMPORTANT NOTE:  Rio designs assume Demux 0 is the only
         *                  playback demux!
         */
        buffer_size = (*DPS_VIDEO_END_ADDR_EX(0) - *DPS_VIDEO_START_ADDR_EX(0));
        *size = buffer_size;
        *free = buffer_size - *DPS_VIDEO_BUFFER_LEVEL_EX(0);
        *lwm  = *DPS_VIDEO_LOWWATERMARK_EX(0);
    }
    else
    {
        rc = FALSE;
    }

    return (rc);
}

/*******************************************************************/
/* Function: gen_video_set_unblank_callback()                      */
/* Parameters: NotifyFn - Function to call for unblanks.           */
/*             pTag     - 32bit returned on the callbacks          */
/* Return:   TRUE for success, FALSE otherwise                     */
/* Remarks:  This function sets the  callback notification         */
/*           function used for unblank callback.                   */
/*******************************************************************/
bool gen_video_set_unblank_callback(CNXT_VIDEO_UNBLANK_CALLBACK NotifyFn,
                                    void                        *pTag)
{
    /*
     * Set the unblank notification function
     */
    Video_Unblank_Callback_Function   = NotifyFn;
    Video_Unblank_Callback_Tag        = pTag;

    return (TRUE);
}
#endif /* (PVR==YES) */

bool gen_video_get_status(CNXT_VIDEO_STATUS *VideoStatus)
{
   #if MPG_SYNC_BIT_POSITION == MPG_SYNC_BIT_POSITION_COLORADO
      VideoStatus->bInSync = (CNXT_GET(glpMpgPicSize, MPG_VID_MASK_IN_SYNC) == MPG_VID_IN_SYNC);
   #else
      VideoStatus->bInSync = (CNXT_GET(MPG_ADDR_EXT_REG, MPG_ADDR_EXT_INSYNC_MASK) == MPG_ADDR_EXT_INSYNC_MASK);
   #endif

   return TRUE;
}

void ipanel_get_vpts(u_int32 *pts_value)
{
    u_int32 vPtsH,vPtsL;

    vPtsH = (u_int32)(*glpVPTSHi);
    vPtsL = (u_int32)(*glpVPTSLo);

    *pts_value = (vPtsH<<31) | (vPtsL >> 1);
}

/****************************************************************************
 * Modifications:
 * $Log: 
 *  279  mpeg      1.278       6/1/04 4:52:53 PM      Billy Jackman   CR(s) 
 *        9339 9340 : Set gbDeferredUnblank to FALSE in gen_video_stop.
 *  278  mpeg      1.277       5/25/04 11:03:37 AM    Tim White       CR(s) 
 *        9296 9297 : Add video unblank callback function.
 *        
 *  277  mpeg      1.276       5/25/04 9:23:19 AM     Billy Jackman   CR(s) 
 *        9285 9286 : Reinstate XTV changes for CRs 9000 and 9001.
 *  276  mpeg      1.275       5/25/04 9:16:27 AM     Billy Jackman   CR(s) 
 *        9285 9286 : Added definition of function gen_video_get_status to 
 *        determine if video is playing in sync or not.
 *        Momentarily remove NDS changes from CRs 9000 and 9001.
 *  275  mpeg      1.274       5/20/04 4:17:15 PM     Billy Jackman   CR(s) 
 *        9125 9126 : Reinstate XTV changes for CRs 9000 and 9001.
 *  274  mpeg      1.273       5/20/04 4:13:04 PM     Billy Jackman   CR(s) 
 *        9125 9126 : Modified code to use definitions of 
 *        BRAZOS_WABASH_FAST_UNBLANK, FAST_RETURN_FROM_PLAY and 
 *        BLANK_ON_VIDEO_START from configuration file instead of defining them
 *         locally.
 *        Momentarily remove NDS changes from CRs 9000 and 9001.
 *  273  mpeg      1.272       4/29/04 9:42:43 AM     Larry Wang      CR(s) 
 *        9000 9001 : Enable video buffer low watermark for XTV.
 *  272  mpeg      1.271       4/19/04 11:21:14 AM    Matt Korte      CR(s) 
 *        8862 8863 : Fix Warnings
 *  271  mpeg      1.270       3/19/04 2:29:01 PM     Xin Golden      CR(s) 
 *        8597 : defined variable bInSync only in DEBUG build to remove warning
 *         in RELEASE.
 *  270  mpeg      1.269       3/15/04 5:00:42 PM     Craig Dry       CR(s) 
 *        8526 : Enable/Disable the demux video channel for Video Drip mode.
 *  269  mpeg      1.268       3/12/04 9:55:38 AM     Tim White       CR(s) 
 *        8545 : Add ES buffer event notification handling required for PVR.
 *        
 *  268  mpeg      1.267       3/2/04 9:07:47 PM      Matt Korte      CR(s) 
 *        8492 : Fix spelling problem
 *  267  mpeg      1.266       3/2/04 7:26:36 PM      Matt Korte      CR(s) 
 *        8492 : Replace VIDEO_STATUS with CNXT_VID_STATUS
 *  266  mpeg      1.265       2/17/04 3:20:41 PM     Dave Wilson     CR(s) 
 *        8322 : Reworked still decode after discovering that the last few 
 *        microcode releases had caused all still decodes to time out. We now 
 *        flush with a second sequence end code after each still. This change, 
 *        coupled with suitable video microcode, allows still decodes to 
 *        complete quickly with no timeouts noted.
 *        Also added some debug code to dump skip/repeat information for 500mS 
 *        or so after motion video starts, allowing us to determine whether or 
 *        not any unclean motion video is displayed.
 *        
 *  265  mpeg      1.264       2/3/04 11:15:08 AM     Dave Wilson     CR(s) 
 *        8265 : Fairly radical rework of the way that gen_video_play reports 
 *        completion and then handles unblanking of the video plane once it is 
 *        safe to do so. In the previous version of the driver, gen_video_play 
 *        reported completion only once motion video was decoding and AV sync 
 *        had been achieved. Now, completion is signalled as soon as the 
 *        decoder is ready to accept data (as per the OpenTV video driver spec)
 *         and unblanking of the video plane is handled in a deferred way under
 *         interrupt. The old behaviour can be reinstated if required by 
 *        commenting out the definition of FAST_RETURN_FROM_PLAY at the top of 
 *        the file.
 *        
 *  264  mpeg      1.263       1/28/04 10:56:26 AM    Dave Wilson     CR(s) 
 *        8201 8200 : Removed code in StartLiveVideo and StopLiveVideo which 
 *        explicitly enabled and disabled the demux video channel. This was 
 *        causing video corruption in some cases on BSkyB due to the channel 
 *        being enabled when the client didn't want it to be.
 *        
 *  263  mpeg      1.262       1/27/04 11:36:36 AM    Mark Thissen    CR(s) 
 *        8269 8270 : Fixing Edit Formatting Error.
 *        
 *  262  mpeg      1.261       1/26/04 3:02:11 PM     Mark Thissen    CR(s) 
 *        8269 8270 : Numerous changes to support declaration of early sync 3 
 *        frames early.
 *        
 *  261  mpeg      1.260       1/5/04 4:26:29 PM      Mark Thissen    CR(s) 
 *        8170 : Removed VideoDisableSync call from StopLiveVideo and changed 
 *        the way it is handled in StartLiveVideo.
 *        
 *  260  mpeg      1.259       12/5/03 2:48:37 PM     Xin Golden      CR(s) 
 *        8105 8106 : send the stop decoding command when there're less or 
 *        equal to 128 bytes left in the still because that's when the decoder 
 *        stops fetching data to decode.
 *  259  mpeg      1.258       12/4/03 10:59:55 AM    Xin Golden      CR(s): 
 *        8039 8040 There's a seperate buffer in the decoder hardware.  We need
 *         to substract the size of the buffer from the padded 256 bytes data 
 *        to make sure the last byte of a still was decoded before the decode 
 *        stop command was issued.
 *  258  mpeg      1.257       12/2/03 2:07:27 PM     Dave Wilson     CR(s): 
 *        8070 7669 Set AllowStillCmd after gen_flush_still_data and set 
 *        PlayState at the top of gen_send_still_data. This should not be 
 *        needed but some deployed OpenTV applications don't adhere to the 
 *        correct API call order when decoding stills and these changes allow 
 *        these apps to work normally.
 *  257  mpeg      1.256       11/14/03 10:39:12 AM   Dave Wilson     CR(s): 
 *        7947 7667 Fixed a problem in gen_video_init which resulted in a 
 *        spurious idle interrupt firing and posting the sync semaphore. This 
 *        resulted in calls to WaitForVideoIdle returning immediately and, as a
 *         result, flashes of crud on transitions when decoding MPEG stills.
 *        Also tidied up some #ifdef NEVER cases and removed some redundant 
 *        code as a result of the recent still decode rework.
 *        
 *  256  mpeg      1.255       10/31/03 4:29:33 PM    Dave Wilson     CR(s): 
 *        7780 7779 Added the code that should already have been inside the 
 *        stub for
 *        gen_video_is_motion_video_playing.
 *  255  mpeg      1.254       10/29/03 11:13:02 AM   Billy Jackman   CR(s): 
 *        7737 7738 Add code to detect and recover from core stalls.
 *  254  mpeg      1.253       10/28/03 3:06:04 PM    Dave Wilson     CR(s): 
 *        7709 Added gen_video_is_motion_video_playing function
 *  253  mpeg      1.252       10/16/03 3:54:11 PM    Dave Wilson     CR(s): 
 *        7645 Added code to gen_flush_still_data that checks to see if the 
 *        decoder is
 *        already stopped. If so, it returns immediately. In the previous 
 *        version, the same code was run even if the decoder was idle and this 
 *        caused a 300mS delay. Note that gen_flush_still_data should never be 
 *        called unless the decoder has just been sent a still to decode. This 
 *        fix cures a problem seen in OpenTV apps which call 
 *        video_report_empty_data twice back-to-back for no apparent reason.
 *        
 *  252  mpeg      1.251       10/14/03 4:54:49 PM    Billy Jackman   CR(s): 
 *        7638 Support for the HD stream tolerance fix in microcode.  if a 
 *        stream provides
 *        
 *        video that is too large, the microcode sets the width to 0x3f0.  If 
 *        the width
 *        
 *        is set to 0x3f0 after video starts, report an error to the client 
 *        instead of
 *        
 *        success, so the client will not unblank.
 *  251  mpeg      1.250       9/29/03 1:45:44 PM     Dave Wilson     SCR(s) 
 *        7552 3160 :
 *        Added function gen_video_get_still_panscan_vector and updated state 
 *        machine
 *        in fix_mpeg_still_problems to parse any sequence display extension to
 *         extract
 *        the offsets.
 *        
 *  250  mpeg      1.249       9/22/03 5:17:32 PM     Dave Aerne      SCR(s) 
 *        7514 :
 *        inserted aud_before_reset() and aud_after_reset() around mpeg core 
 *        reset within gen_video_init. 
 *        
 *  249  mpeg      1.248       9/2/03 12:58:12 PM     Lucy C Allevato SCR(s) 
 *        7229 7230 :
 *        added another extra 4 bytes in gen_send_still_data if sequence end 
 *        code is not found in the original still to make sure microcode reads 
 *        the added sequence end.  Also removed header stdio.h.
 *        
 *  248  mpeg      1.247       8/15/03 9:28:52 AM     Dave Wilson     SCR(s): 
 *        7278 7279 
 *        Made video startup timeouts more conservative since some failures of 
 *        gen_video_play were seen on Dish Network and BSkyB with the previous 
 *        values.
 *        
 *  247  mpeg      1.246       8/1/03 4:04:30 PM      Dave Wilson     SCR(s) 
 *        7102 :
 *        Previous version failed to include special PAUSE command handling for
 *         Colorado
 *        macroblock error recovery case in all places where the PAUSE command 
 *        is used. 
 *        This caused problems when restarting motion video after decoding a 
 *        still. The
 *        decoder would hang and the request would fail. 
 *        
 *  246  mpeg      1.245       7/30/03 5:52:46 PM     Dave Wilson     SCR(s) 
 *        6769 :
 *        Reworked video driver and included new microcode version to change 
 *        the way
 *        that the end of an MPEG still is signalled. Previously, we made the 
 *        assumption
 *        that a still could contain several pictures but that it would always 
 *        be
 *        encapsulated in a single sequence. Unfortunately, several stills from
 *         a new
 *        network have been found to be encoded as multiple sequences. 
 *        New microcode now keeps decoding still data regardless of sequence 
 *        end tags so
 *        we can keep sending data all day until we explicitly stop the decoder
 *         and the
 *        data will be decoded.
 *        
 *  245  mpeg      1.244       7/22/03 7:35:42 PM     Angela Swartz   SCR(s) 
 *        6958 :
 *        modified the code to use the new/changed software config keys 
 *        CONF_AUDIO_SYNC & VIDEO_OUTPUT_STANDARD_DEFAULT
 *        
 *  244  mpeg      1.243       7/1/03 12:35:32 PM     Dave Wilson     SCR(s) 
 *        6363 :
 *        Added gen_video_set_frame_callback to allow an app to replace/chain 
 *        the 
 *        decode complete callback passed originally on gen_video_init.
 *        
 *  243  mpeg      1.242       6/3/03 4:29:16 PM      Craig Dry       SCR(s) 
 *        6696 6695 :
 *        Fix DripOut index used on callback.
 *        
 *  242  mpeg      1.241       6/3/03 3:12:42 PM      Dave Wilson     SCR(s) 
 *        6653 :
 *        Reworked still decode sequence to handle stills containing multiple 
 *        MPEG
 *        sequences. Also fixed a problem in gen_video_decode_still_image which
 *         could
 *        result in an (almost) infinite loop.
 *        
 *  241  mpeg      1.240       5/1/03 3:29:36 PM      Dave Wilson     SCR(s) 
 *        6141 :
 *        Changed video startup timeouts to allow OpenTV VTS test stream 
 *        (DUEL_SI.MUX)
 *        to play. It has very long PES packets so the original 200mS data flow
 *         start
 *        timeout was too short. The new timeout is 450mS. Also changed the 
 *        first 
 *        picture timeout to 1750mS since the Dish Network preview channel 
 *        sometimes
 *        sends I pictures this far apart!
 *        
 *  240  mpeg      1.239       4/25/03 12:31:04 PM    Ray Mack        SCR(s) 
 *        6103 :
 *        added extra bit for bitstream PTS report and added Anchor PTS and 
 *        Running PTS reports
 *        
 *  239  mpeg      1.238       4/22/03 4:05:12 PM     Dave Wilson     SCR(s) 
 *        6069 :
 *        Added gen_video_get/set_startup_timeouts. This allows a client app to
 *         set
 *        the timeouts used by WaitForLiveVideoToStart without having to modify
 *         and
 *        recompile the video driver.
 *        WaitForLiveVideoToStart was reworked to decouple the wait for initial
 *         data rom
 *        the wait for first picture to be decoded. This allows for a much 
 *        shorter 
 *        timeout in cases where you change to a scrambled channel but do not 
 *        have the
 *        keys set. By default, we wait 200mS for data from the demux now 
 *        whereas before
 *        the code would have waited 1250mS.
 *        
 *  238  mpeg      1.237       4/21/03 5:01:20 PM     Dave Wilson     SCR(s) 
 *        6057 :
 *        Added APIs gen_video_get/set_still_scaling_mode. This controls how 
 *        the 
 *        video decoder performs display when motion video decode is stopped. 
 *        By default
 *        field scaling is used but this lowers the displayed resolution of 
 *        stills so
 *        the new API lets the user select frame scaling instead.
 *        Note that this is only required and available on Colorado and when 
 *        using the
 *        macroblock-based error concealment microcode. Microcode version must 
 *        be at 
 *        least 2.27 to contain support for this feature.
 *        
 *  237  mpeg      1.236       4/11/03 5:33:06 PM     Dave Wilson     SCR(s) 
 *        6014 :
 *        Added an interlock (gStoppingVideo) between gen_video_stop and 
 *        WaitForLiveVideoToStart. If someone tries to stop video while waiting
 *         for
 *        a previous gen_video_play command to complete, the stop will now be 
 *        processed
 *        quickly rather than being held off while WaitForLiveVideoToStart 
 *        times out.
 *        This allows you, for example, to change through a number of scrambled
 *         channels
 *        in WatchTV without having to wait a couple of seconds each time.
 *        
 *  236  mpeg      1.235       3/20/03 3:26:18 PM     Dave Wilson     SCR(s) 
 *        5523 :
 *        Brazos now uses a different microcode build from Wabash so this has 
 *        been
 *        split out as a separate file.
 *        Also tidied up the code which includes the correct microcode build. 
 *        Rather
 *        than having 2 large, practically identical sets of #if/#elif/#else 
 *        blocks
 *        first to define the correct extern references for the microcode and 
 *        then again
 *        to actually load it, I now define macros for the microcode array and 
 *        size in
 *        the first #if block and have a single call to LoadMicrocode later 
 *        using these
 *        macros. This is a lot cleaner and means only changing one place if we
 *         add or
 *        remove microcode variants in future.
 *        
 *  235  mpeg      1.234       3/19/03 4:27:04 PM     Dave Wilson     SCR(s) 
 *        5135 :
 *        Added code to gather video error counts. This information may be 
 *        queried
 *        using the new gen_video_get_error_stats function.
 *        
 *  234  mpeg      1.233       3/6/03 9:16:10 AM      Dave Wilson     SCR(s) 
 *        5622 :
 *        Ensured that the encoded video buffer is flushed just prior to 
 *        starting
 *        video. StopLiveVideo doesn't wait for the demux to finish writing 
 *        data before
 *        returning and flushing the buffer so we need to do this to get rid of
 *         any
 *        data that was written after we stopped the video. Adding a delay in 
 *        StopLiveVideo
 *        to wait for the demux to finish is undesireable since this would 
 *        cause channel
 *        change times to increase.
 *        
 *  233  mpeg      1.232       3/5/03 1:29:38 PM      Dave Wilson     SCR(s) 
 *        5622 :
 *        WaitForLiveVideoToStart now checks for no data case by looking at the
 *        encoded video write pointer on timeout. If it is the same as the 
 *        pointer when
 *        the function was first called, we send back VIDEO_STATUS_NO_DATA. In 
 *        the 
 *        previous case, this return code was set only if the read and write 
 *        pointer
 *        were the same but we have seen cases where the pointers differ by a 
 *        slight
 *        amount resulting in the wrong return code.
 *        
 *  232  mpeg      1.231       2/27/03 4:56:16 PM     Dave Wilson     SCR(s) 
 *        5613 :
 *        Redefined MPG_COMMAND_PAUSE_ON_I from 0x60 to 0x1060. This prevents 
 *        an
 *        occasional hang after the command is issued. The extra 0x1000 
 *        supposedly tells
 *        the hardware to disable sync while the command executes since there 
 *        is 
 *        apparently a bug in the decoder that can cause it to lock up if sync 
 *        is being
 *        checked while a pause is being requested.
 *        
 *  231  mpeg      1.230       2/21/03 3:46:40 PM     Dave Wilson     SCR(s) 
 *        5580 :
 *        Added a Canal+ specific API gen_video_pause_on_next_I. This is 
 *        similar to
 *        gen_video_pause except that it guarantees to display an I picture 
 *        when the 
 *        pause takes effect.
 *        
 *  230  mpeg      1.229       2/20/03 4:53:16 PM     Senthil Veluswamy SCR(s) 
 *        5574 :
 *        Increased the Initial Video Decode timeout to prevent premature 
 *        timeouts on streams needing more time for Video decode. Suggested by 
 *        Dave W.
 *        
 *  229  mpeg      1.228       2/19/03 4:24:04 PM     Dave Wilson     SCR(s) 
 *        5552 5553 :
 *        Added code to fix_mpeg_still_problems to catch stills encoded with 
 *        illegal
 *        or unsupported MPEG profiles and patch them.
 *        
 *  228  mpeg      1.227       2/13/03 1:05:00 PM     Dave Wilson     SCR(s) 
 *        5480 :
 *        Moved the header file includes above the various feature #defines so 
 *        that
 *        it is safe to use HWCONFIG and SWCONFIG options when deciding which 
 *        other
 *        #defines to set.
 *        
 *  227  mpeg      1.226       2/11/03 1:33:44 PM     Dave Wilson     SCR(s) 
 *        5137 :
 *        Added code to make use of new ENABLE_DEFERRED_VIDEO_UNBLANK config 
 *        option.
 *        
 *  226  mpeg      1.225       12/16/02 4:18:14 PM    Tim White       SCR(s) 
 *        5169 :
 *        Remove unnecessary chip_id, chip_rev detection code.
 *        
 *        
 *  225  mpeg      1.224       12/4/02 1:08:56 PM     Lucy C Allevato SCR(s) 
 *        5065 :
 *        Enclosed in a conditional register used to determine the frame 
 *        completed or displayed for Wabash (glpMpgAddrExt) and Colorado 
 *        (glpMpgPicSize) in several places.
 *        
 *  224  mpeg      1.223       11/25/02 12:51:52 PM   Dave Wilson     SCR(s) 
 *        5000 :
 *        Added macroblock-based error recovery microcode for Wabash
 *        
 *  223  mpeg      1.222       11/15/02 4:07:50 PM    Senthil Veluswamy SCR(s) 
 *        4965 :
 *        1) Removed special case handling of the Data Dry Error and rolled it 
 *        as another case in the Generic Audio Error handling mechanism. Now 
 *        all Audio Errors will cause a callback that is handled by Audio.
 *        2) Removed // comments.
 *        
 *  222  mpeg      1.221       11/1/02 3:48:14 PM     Dave Wilson     SCR(s) 
 *        4878 :
 *        Consolidated the various video microcode options for Colorado. 
 *        Removed
 *        the 2 separate builds for Rev F vs. other production revs (since the 
 *        older microcode works fine on Rev F anyway) and the PTS masking 
 *        version (since
 *        the PTS masking fix is now in the mainstream microcode).
 *        
 *  221  mpeg      1.220       10/24/02 5:23:22 PM    Dave Wilson     SCR(s) 
 *        4824 :
 *        Reworked gen_video_init to include a video decoder reset and 3 still 
 *        decodes.
 *        This flushes the video core and ensures that some uninitialised 
 *        internal
 *        registers get initialised before an application can try to decode 
 *        video. In
 *        a few cases, chips had been seen to fail decode of the first one or 
 *        two images
 *        sent to the decoder after POR and this rework will prevent this from 
 *        happening.
 *        
 *  220  mpeg      1.219       10/16/02 12:13:48 PM   Craig Dry       SCR(s) 
 *        4800 :
 *        Added comment, explaining why the extra pause/continue pair is 
 *        inserted
 *        in the code.
 *        
 *  219  mpeg      1.218       10/16/02 11:12:48 AM   Craig Dry       SCR(s) 
 *        4800 :
 *        Workaround microcode pause/resume bug that shows up during video drip
 *         mode.
 *        
 *  218  mpeg      1.217       10/11/02 5:52:08 PM    Craig Dry       SCR(s) 
 *        4716 :
 *        Add asynchronous features to Video Drip API
 *        
 *  217  mpeg      1.216       10/11/02 4:13:16 PM    Dave Wilson     SCR(s) 
 *        4746 :
 *        Added gen_video_register_audio_error_callback to allow a client to 
 *        register
 *        a function pointer that will be called whenever an error interrupt 
 *        indicates
 *        that the source was the audio decoder. 
 *        
 *  216  mpeg      1.215       9/30/02 1:32:58 PM     Craig Dry       SCR(s) 
 *        4716 :
 *        Add Pause/Resume Capability to Video Drip Mode  
 *        
 *        
 *  215  mpeg      1.214       9/27/02 2:48:38 PM     Dave Wilson     SCR(s) 
 *        1513 :
 *        Renamed gen_flush_data to gen_flush_still_data (but put a #define in
 *        VIDEO.H to keep old code building).
 *        
 *  214  mpeg      1.213       9/25/02 9:45:54 PM     Carroll Vance   SCR(s) 
 *        3786 :
 *        Removing old DRM and AUD conditional bitfield code.
 *        
 *        
 *  213  mpeg      1.212       8/16/02 6:01:20 PM     Tim White       SCR(s) 
 *        4420 :
 *        Remove the ancient video_debug_process() from the code which ended
 *        being enabled when LEGACY_DVR is enabled.
 *        
 *        
 *  212  mpeg      1.211       8/15/02 2:00:20 PM     Dave Wilson     SCR(s) 
 *        4377 :
 *        Added VIDEO_STATUS_SYNC_TIMEOUT return code from 
 *        WaitForLiveVideoToStart and
 *        gen_video_play. This indicates that video is decoding but that AV 
 *        sync has not
 *        been achieved within 2.5s.
 *        
 *  211  mpeg      1.210       8/8/02 1:34:24 PM      Bob Van Gulick  SCR(s) 
 *        4350 :
 *        Add support for MHP Video Drips
 *        
 *        
 *  210  mpeg      1.209       7/18/02 3:03:32 PM     Dave Wilson     SCR(s) 
 *        4231 :
 *        Added new macroblock-based error recovery microcode as a build 
 *        option.
 *        
 *  209  mpeg      1.208       7/15/02 5:33:12 PM     Dave Wilson     SCR(s) 
 *        3627 :
 *        Tidied up the use of various fields in MPG_VID_SIZE_REG. The wrong 
 *        masks
 *        had been used in various places as a result of microcode changes over
 *         the
 *        last few months and these were corrected.
 *        
 *  208  mpeg      1.207       7/10/02 4:08:24 PM     Dave Wilson     SCR(s) 
 *        3954 :
 *        Added special case to load new PTS-masking microcode.
 *        
 *  207  mpeg      1.206       7/2/02 3:44:00 PM      Dave Wilson     SCR(s) 
 *        4126 :
 *        Added a global variable to record the number of times ResetVCore is 
 *        called.
 *        This is designed to be used in testcases and debug.
 *        
 *  206  mpeg      1.205       7/2/02 12:36:56 PM     Dave Wilson     SCR(s) 
 *        4125 :
 *        Previously, decode of the small black still during initialisation 
 *        would
 *        always time out. It turns out that this was due to the fact that the 
 *        MPEG
 *        interrupt was still disabled at the point where the decode was done. 
 *        The
 *        interrupt is now enabled before the still is decoded.
 *        
 *  205  mpeg      1.204       7/1/02 5:25:52 PM      Dave Wilson     SCR(s) 
 *        4068 :
 *        Various changes to improve performance and reliability of stills 
 *        decode on
 *        errors.
 *        1. Changed SendFrameHeader to send 512 bytes with the 0x01 0xB3 
 *        sequence at
 *        bytes 18 and 19 as suggested by hardware group. This should prevent 
 *        the 
 *        sequence start code from ending up in a non-flushed internal buffer 
 *        in some
 *        error cases. This scenario has been shown to prevent decode of the 
 *        next still
 *        in some circumstances.
 *        2. Added core stall recovery inside flush processing as well as 
 *        existing case
 *        in stop. OpenTV 1.2 will copy the still data before calling 
 *        video_stop so this
 *        ensure that the data is really flushed before it is copied even in 
 *        cases
 *        where the still contains errors.
 *        3. Reduced the still decode timeout from the very strange 5000ms that
 *         was
 *        there before to a somewhat more sensible 300ms. Could probably be a 
 *        whole lot
 *        lower but this is a safe number that will definitely not cause 
 *        premature
 *        timeouts.
 *        
 *  204  mpeg      1.203       6/17/02 7:47:42 PM     Miles Bintz     SCR(s) 
 *        4043 :
 *        Switched on MPG_SYNC_BIT_POSITION define for bInSync test
 *        
 *        
 *  203  mpeg      1.202       6/14/02 5:12:26 PM     Dave Wilson     SCR(s) 
 *        4032 :
 *        y
 *        OVTVIDEO calls gen_video_pause from interrupt context. Previously, 
 *        however,
 *        this function was not interrupt safe so the debug build crashed 
 *        during the
 *        TIMECODE VTS test.
 *        
 *  202  mpeg      1.201       6/13/02 6:13:44 PM     Dave Wilson     SCR(s) 
 *        4007 :
 *        WaitForLiveVideoToStart recoded to make use of a status bit in the 
 *        MPEG
 *        picture size register that tells us when the decoder has acheived AV 
 *        sync.
 *        When sync is enabled, the polling loop now continues looking until 
 *        the sync
 *        indicator is set rather than just returning after the second decode 
 *        complete
 *        interrupt. When sync is disabled, the function works as it used to, 
 *        waiting
 *        for 2 decoded pictures then returning.
 *        
 *  201  mpeg      1.200       6/13/02 4:46:36 PM     Dave Wilson     SCR(s) 
 *        3999 :
 *        Updated return code checks after calls to gen_video_play to match the
 *         new
 *        set of values passed back by the function.
 *        
 *  200  mpeg      1.199       6/12/02 6:30:06 PM     Dave Wilson     SCR(s) 
 *        3992 :
 *        Oops - I used a couple of local variables that were defined inside 
 *        #ifdef
 *        DEBUG in non-debug code. This is now fixed!
 *        
 *  199  mpeg      1.198       6/12/02 5:42:44 PM     Carroll Vance   SCR(s) 
 *        3786 :
 *        Removing DRM bitfields.
 *        
 *  198  mpeg      1.197       6/12/02 3:30:06 PM     Dave Wilson     SCR(s) 
 *        3216 :
 *        gen_video_play and the command done callback used by the video driver
 *        have both been changed to include a status value. In the case of the
 *        callback, this is passed as a parameter and in the case of 
 *        gen_video_play it
 *        replaces the previous bool type return code.
 *        The return value may be used to determine whether or not motion video
 *         started
 *        play correctly.
 *        
 *  197  mpeg      1.196       6/10/02 3:07:14 PM     Craig Dry       SCR(s) 
 *        3923 :
 *        Remove MPG bitfields unconditionally - Step 2
 *        
 *  196  mpeg      1.195       6/6/02 5:51:02 PM      Craig Dry       SCR(s) 
 *        3923 :
 *        Remove MPG bitfields conditionally - Step 1
 *        
 *  195  mpeg      1.194       6/6/02 3:19:48 PM      Dave Wilson     SCR(s) 
 *        3931 :
 *        Changes made to user data handling to allow closed captioning to be 
 *        extracted
 *        and displayed correctly. The user data notification is now sent to 
 *        the video
 *        task on the decode complete interrupt following reception of the data
 *         so that
 *        we can tag it with the picture type. The video task also sends the 
 *        data back
 *        to the client in one or two callbacks per picture so a parameter was 
 *        added to
 *        tell the client if a second call is going to be made (this saves the 
 *        interfac
 *        having to change radically to manage a ring buffer between the driver
 *         and the
 *        client).
 *        
 *  194  mpeg      1.193       5/22/02 12:00:12 PM    Dave Wilson     SCR(s) 
 *        3851 :
 *        Minor changes to remove a couple of warnings from the release build.
 *        
 *  193  mpeg      1.192       5/21/02 1:39:18 PM     Carroll Vance   SCR(s) 
 *        3786 :
 *        Removed DRM bitfields.
 *        
 *  192  mpeg      1.191       5/13/02 1:48:26 PM     Tim White       SCR(s) 
 *        3760 :
 *        DPS_ HSDP definitions are now HSDP_, remove reliance on legacy DPS 
 *        globals.
 *        
 *        
 *  191  mpeg      1.190       5/3/02 5:54:56 PM      Dave Wilson     SCR(s) 
 *        3669 :
 *        y
 *        Interrupt handler now checks for the new audio data dry interrupt 
 *        (reported
 *        in the decoder status register and interrupted as a decoder error) 
 *        and, in 
 *        the case of Canal+ builds, calls the audio driver to mute audio if 
 *        this 
 *        interrupt fires.
 *        
 *  190  mpeg      1.189       5/2/02 3:05:28 PM      Dave Wilson     SCR(s) 
 *        3684 :
 *        Disabled periodic trace which had accidentally been left enabled on a
 *         recent
 *        edit of the file.
 *        
 *  189  mpeg      1.188       4/26/02 2:57:24 PM     Tim White       SCR(s) 
 *        3562 :
 *        Add support for Colorado Rev_F.
 *        
 *        
 *  188  mpeg      1.187       4/10/02 6:56:10 PM     Dave Wilson     SCR(s) 
 *        3495 :
 *        Padded encoded video buffer with 256 bytes of 0 before each new 
 *        still. This
 *        seems to cure a problem where once in a blue moon a still would not 
 *        be decoded
 *        properly. The root cause of the problem is still under investigation 
 *        by the 
 *        hardware team.
 *        Fixed up the idle signalling, removing some questionable use of 
 *        global variables
 *        and cleaning up the semVideoPlay usage.
 *        Added some code to the NO_STILL_FLUSH_WORKAROUND case for some bad 
 *        API call
 *        sequences seen in ARD Online app (working on 2 problems at once - 
 *        this one is
 *        not completed yet but code is benign).
 *        
 *  187  mpeg      1.186       4/5/02 1:54:12 PM      Dave Wilson     SCR(s) 
 *        3510 :
 *        Reimplement fixes lost as a result of moving the file back to 
 *        pre-bitfield-
 *        fix level.
 *        
 *  186  mpeg      1.185       4/5/02 1:01:56 PM      Dave Wilson     SCR(s) 
 *        3510 :
 *        Revert to version 1.181, prior to bitfields removal.
 *        
 *  185  mpeg      1.184       4/5/02 9:54:28 AM      Dave Wilson     SCR(s) 
 *        3496 :
 *        Allow for 2 different versions of microcode, the normal stuff and a 
 *        version
 *        with enhanced error detection for use in Canal+ boxes.
 *        
 *  184  mpeg      1.183       3/28/02 5:58:22 PM     Dave Wilson     SCR(s) 
 *        3467 :
 *        Added a compile option, BLANK_BEFORE_ALL_STILLS, which causes the 
 *        driver to
 *        fill the target video buffer with black before starting to decode any
 *         still.
 *        This allows the Canal+ test where half a still is sent to display the
 *         expected
 *        output (partly decoded image and black beneath). I don't recommend 
 *        turning
 *        this option on for all builds since there are some still sequences 
 *        which may
 *        be badly affected by it (not that I have actually seen any of these 
 *        though).
 *        
 *  183  mpeg      1.182       3/28/02 2:10:46 PM     Quillian Rutherford 
 *        SCR(s) 3468 :
 *        Removed Bit Fields
 *        
 *  182  mpeg      1.181       3/20/02 11:21:16 AM    Dave Wilson     SCR(s) 
 *        3412 :
 *        For OpenTV/PAL builds we now turn the DRM PAL control bit off when we
 *         are 
 *        not decoding motion video. This works around a hardware bug that 
 *        causes
 *        upscaled 4:2:0 images to look bad on CX2249x if this bit is set.
 *        
 *  181  mpeg      1.180       3/15/02 4:46:36 PM     Dave Wilson     SCR(s) 
 *        3390 :
 *        WaitForLiveVideoToStart now times out in 750mS if it does not see a 
 *        decoded
 *        frame in this period. This means that it no longer takes the full 
 *        2.5s for
 *        gen_video_play to finish when there is no possibility of video 
 *        starting anyway.
 *        
 *  180  mpeg      1.179       3/15/02 9:12:28 AM     Dave Wilson     SCR(s) 
 *        3196 :
 *        We now pad an extra 4 0s before a sequence end code if a still passed
 *         does
 *        not already include this. We have seen cases where truncated stills 
 *        are 
 *        passed that loose a couple of bytes of the last macroblock as well as
 *         the 
 *        sequence end code and this allows these stills to be decoded 
 *        acceptably too.
 *        
 *  179  mpeg      1.178       3/12/02 4:58:16 PM     Bob Van Gulick  SCR(s) 
 *        3359 :
 *        Add multi-instance demux support
 *        
 *        
 *  178  mpeg      1.177       3/11/02 11:08:58 AM    Ian Mitchell    SCR(s): 
 *        3342 
 *        used the new definitions for creating the VDBG_TASK
 *        
 *  177  mpeg      1.176       3/4/02 4:10:08 PM      Dave Wilson     SCR(s) 
 *        3113 :
 *        Reworked fix_mpeg_still_problems so that it does not need to look 
 *        ahead in
 *        the data and can, therefore, be called 1 byte at a time and still 
 *        correct all
 *        known syntax problems. Also added a couple more traces in ResetVCore 
 *        so that
 *        we can see the read and write pointers when this gets called and 
 *        changed the
 *        pattern we send in SendFrameHeader to include 6 0s before the 
 *        sequence end
 *        marker rather than the previous 2.
 *        
 *  176  mpeg      1.175       2/22/02 10:24:44 AM    Dave Wilson     SCR(s) 
 *        3206 :
 *        Added gen_video_decode_blank API
 *        
 *  175  mpeg      1.174       2/21/02 3:21:42 PM     Dave Wilson     SCR(s) 
 *        3225 :
 *        Ficed a bug in the state machine which scans and correct MPEG still 
 *        syntax
 *        errors. The bug meant that adjacent temporal references that were the
 *         same
 *        would not always be patched and resulted in some stills decoding 
 *        incorrectly.
 *        
 *  174  mpeg      1.173       2/14/02 3:02:44 PM     Ray Mack        SCR(s) 
 *        3189 :
 *        THis should help with the dmfront splash screen problem.
 *        
 *  173  mpeg      1.172       2/6/02 4:22:04 PM      Bob Van Gulick  SCR(s) 
 *        3143 :
 *        Conditionally compile for DEMUX or GENDMXC
 *        
 *        
 *  172  mpeg      1.171       2/1/02 1:55:12 PM      Dave Wilson     SCR(s) 
 *        3085 :
 *        Further changes to decode more problem stills. Now looks for 0x00 
 *        0x01 
 *        patterns in user data (which cause a hardware problem in the vcore to
 *         show 
 *        itself), stills without sequence end tags and stills containing 
 *        multiple
 *        sequence start/end tags.
 *        
 *  171  mpeg      1.170       1/31/02 9:02:14 AM     Tim White       SCR(s) 
 *        3106 :
 *        Disable AVSync for Wabash for the time being.
 *        
 *        
 *  170  mpeg      1.169       1/10/02 4:50:24 PM     Dave Wilson     SCR(s) 
 *        3023 :
 *        Moved a line of code that was causing send_video_command_done to be 
 *        called
 *        twice after VIDEO_COMMAND_PLAY was processed. Also cleaned up a 
 *        couple of 
 *        compiler warnings.
 *        
 *  169  mpeg      1.168       12/19/01 3:13:24 PM    Bob Van Gulick  SCR(s) 
 *        2977 :
 *        Backout multi-instance demux usage
 *        
 *        
 *  168  mpeg      1.167       12/18/01 4:57:44 PM    Quillian Rutherford 
 *        SCR(s) 2933 :
 *        Merged wabash code in 
 *        
 *        
 *  167  mpeg      1.166       12/18/01 9:58:36 AM    Bob Van Gulick  SCR(s) 
 *        2977 :
 *        Changes to support new multi-instance demux driver
 *        
 *        
 *  166  mpeg      1.165       11/8/01 5:14:24 PM     Dave Wilson     SCR(s) 
 *        2875 :
 *        
 *        
 *        
 *        
 *        
 *        
 *        Oops - error in last edit meant that the new code was never called. 
 *        This version
 *        does the right thing.
 *        
 *  165  mpeg      1.164       11/8/01 1:19:40 PM     Dave Wilson     SCR(s) 
 *        2875 :
 *        It seems that the command MPG_COMMAND_FLUSH_VBV hangs the video 
 *        decoder if
 *        an MPG_COMMAND_PAUSE was not sent just before it. Added a check to 
 *        ensure that
 *        this is the case and, hey presto, the driver jumps back into life.
 *        
 *  164  mpeg      1.163       11/7/01 3:52:36 PM     Dave Wilson     SCR(s) 
 *        2872 :
 *        Added code to flush the decoded VBV buffer before playing any still. 
 *        This
 *        prevents mysterious still non-decodes in cases where the code is 
 *        stopped while
 *        partway through reception of an MPEG 00 00 01 tag.
 *        
 *  163  mpeg      1.162       11/7/01 11:50:04 AM    Dave Wilson     SCR(s) 
 *        2860 :
 *        Removed redundant error handling case where the driver used to send a
 *        CONTINUE command to vcore after receiving a bitstream error while 
 *        decoding
 *        a still. This broke with the latest microcode when decoding the nasty
 *        ViaDigital stills with the wrong number of macroblocks.
 *        
 *  162  mpeg      1.161       11/6/01 11:28:46 AM    Dave Wilson     SCR(s) 
 *        2863 :
 *        Fixed things up in gen_flush_data so that it writes padding through 
 *        the 
 *        non-cached memory aperture and always pads 256 bytes of 0s after the 
 *        still
 *        data.
 *        
 *  161  mpeg      1.160       11/1/01 5:08:50 PM     Dave Wilson     SCR(s) 
 *        2847 :
 *        Added another filter case to one of the still data fixup functions. 
 *        This
 *        now checks the temporal reference of a still and makes sure that it 
 *        is 
 *        different for adjacent stills. Without this, following stills will be
 *         ignored.
 *        
 *  160  mpeg      1.159       11/1/01 4:14:22 PM     Dave Wilson     SCR(s) 
 *        2851 :
 *        Changed byte sequence sent in SendFrameHeader to actually be a 
 *        correct
 *        sequence start pattern. Previous version omitted the two zeros 
 *        required 
 *        before the 0x01 0xB3 pattern.
 *        
 *  159  mpeg      1.158       10/23/01 6:08:06 PM    Billy Jackman   SCR(s) 
 *        2813 :
 *        Modified still image parsing to catch multiple instances of codes 
 *        which
 *        need fixing instead of stopping after the first four bytes.  Added 
 *        code
 *        to catch invalid MPEG profile information and re-write it as main.
 *        
 *  158  mpeg      1.157       10/16/01 12:45:24 PM   Dave Wilson     SCR(s) 
 *        2780 :
 *        To allow misbehaving apps to send multiple stills without intervening
 *         calls
 *        to video_report_empty_data/video_flush_data, we now ensure that a new
 *         still
 *        command is sent to the vcore whenever video_data_ready is called 
 *        after a 
 *        previous still decode has completed. This fixes the ViaSat portal app
 *         case
 *        even though the app is not adhering to the OpenTV spec.
 *        
 *  157  mpeg      1.156       10/3/01 2:21:28 PM     Dave Wilson     SCR(s) 
 *        2704 :
 *        Fixed SetReadPtr function and changed buffer fullness target to be 
 *        used in
 *        overflow cases.
 *        Added video sync toggle when an overflow occurs. These fixes appear 
 *        to cure a
 *        problem seen in the UK which sometimes caused video to get into a 
 *        permanent
 *        jerky state after channel change.
 *        
 *  156  mpeg      1.155       9/26/01 1:07:12 PM     Dave Wilson     SCR(s) 
 *        2680 :
 *        Fix equivalent to 1.152.1.1 on branch. Removed EPG running check 
 *        before
 *        unblanking video plane at end of StartLiveVideo.
 *        
 *  155  mpeg      1.154       9/20/01 4:25:58 PM     Dave Wilson     SCR(s) 
 *        2669 :
 *        Reapplied fixes as made to revision 1.152.1.0
 *        WaitForLiveVideoToStart now checks that the demux channel has not 
 *        been
 *        disabled underneath it and drops out if it has.
 *        Reduced some timeouts used to catch vcore stalls to values nearer to 
 *        those
 *        expected by the hardware team.
 *        
 *  154  mpeg      1.153       9/13/01 2:18:18 PM     Quillian Rutherford 
 *        SCR(s) 2633 2634 2635 2636 2637 :
 *        Changed to use a C structure holding microcode instead of an assembly
 *         
 *        structure. 
 *        
 *        
 *  153  mpeg      1.152       9/11/01 3:23:38 PM     Joe Kroesche    SCR(s) 
 *        2631 2632 :
 *        removed ifdef around define for MPEG_BUFFER_RESTART_TARGET that was 
 *        breaking
 *        NDSTEST build
 *        
 *  152  mpeg      1.151       9/7/01 6:11:48 AM      Dave Wilson     SCR(s) 
 *        2614 2615 :
 *        Reverted to old video channel handling until we resolve the AV sync
 *        problems associated with the new optimised CONTINUE code.
 *        
 *  151  mpeg      1.150       9/6/01 7:29:14 AM      Dave Wilson     SCR(s) 
 *        2597 2598 :
 *        Don't disabled the demux channel on StartLiveVideo unless it has not 
 *        been disabled by someone else. Without this check, channel changes 
 *        between
 *        scrambled channels are very slow indeed.
 *        
 *  150  mpeg      1.149       8/28/01 4:39:46 PM     Dave Wilson     SCR(s) 
 *        2551 2552 :
 *        New video microcode checks for PTS on starting a CONTINUE command.
 *        Video driver adjusts PTS on CONTINUE to account for delay since last 
 *        stop (in
 *        last build for another tracker).
 *        
 *  149  mpeg      1.148       8/28/01 12:44:34 PM    Dave Wilson     SCR(s) 
 *        2542 2543 :
 *        Moved deferred unblank from decode complete interrupt to INT 5 in 
 *        OSDISRC.C
 *        Changed CONTINUE operation in StartLiveVideo. Now sets a target level
 *         in the 
 *        buffer and modifies the read pointer to make the buffer this full on 
 *        restart
 *        (assuming enough data is there, of course). Also patch the saved PTS 
 *        to 
 *        compensate for the time video was stopped.
 *        
 *  148  mpeg      1.147       8/24/01 11:27:40 AM    Dave Wilson     SCR(s) 
 *        2547 2548 :
 *        Fairly substantial changes to optimise handling of play/stop/play 
 *        sequences
 *        as seen frequently in OpenTV applications.
 *        1. Demux now free-runs while video is stopped and no still decodes 
 *        have been
 *        requested. This keeps the buffer filled with the latest data from the
 *         stream.
 *        2. When asked to play a stream that was the last stream played with 
 *        no
 *        intervening calls to decode stills, the driver now issues a CONTINUE 
 *        command
 *        to the video core rather than a PLAY command. This prevents the core 
 *        from
 *        waiting for another sequence header and gets the repeat play call 
 *        finished
 *        a lot quicker.
 *        3. To support 2, added code to query whether the transponder changed 
 *        since
 *        the last video_play call. Also queries the video PID from demux.
 *        To enable all these changes VIDEO_REPLAY_OPTIMISATION must be defined
 *         in the
 *        build (this is on by default for OpenTV 1.2 but not any other 
 *        target).
 *        
 *  147  mpeg      1.146       7/19/01 3:33:16 PM     Dave Wilson     SCR(s) 
 *        2341 2340 :
 *        It turns out that the video microcode buffer swapping quirkiness can 
 *        happen
 *        any time that video is stopped from live or still decodes. 
 *        Yesterday's fix
 *        assumed that it only happened when live video was stopped so I moved 
 *        the 
 *        stop buffer check into the general stop processing path so that it is
 *         done
 *        regardless of what kind of decode had previously happened. This 
 *        should prevent
 *        decodes into the visible buffer when moving from still decode to 
 *        motion.
 *        
 *  146  mpeg      1.145       7/18/01 7:14:18 PM     Dave Wilson     SCR(s) 
 *        2253 2254 :
 *        Added code to StopLiveVideo which checks which buffer the vcore is 
 *        displaying
 *        from and updates the last decoded anchor global to ensure that we 
 *        calculate the
 *        start buffer for the next video_play correctly. It seems that the 
 *        video ucode
 *        sometimes decides to swap buffers on a stop.
 *        
 *  145  mpeg      1.144       7/18/01 5:41:16 PM     Dave Wilson     SCR(s) 
 *        2329 2330 :
 *        Reinstated blanking on channel change for OpenTV 1.2 only when EPG is
 *         running.
 *        Added some debug information to help analyse video play start times 
 *        on long
 *        channel changes.
 *        Added code (compiled out for 1.2) that allows the video buffer to be 
 *        preloaded
 *        before the play command is issued.
 *        
 *  144  mpeg      1.143       7/16/01 3:20:16 PM     Quillian Rutherford 
 *        SCR(s) 2298 2299 2300 2301 2302 :
 *        Changed Reset Sequence to match what hardware has given us.  Changed 
 *        gen_video_reset to use the queue'd command method all the other api's
 *         are using.
 *        
 *        
 *  143  mpeg      1.142       7/16/01 3:20:14 PM     Dave Wilson     SCR(s) 
 *        2283 2284 :
 *        Added some missing #ifdef DEFERRED_UNBLANK lines to allow non-OpenTV 
 *        1.2 
 *        builds to build without errors.
 *        
 *  142  mpeg      1.141       7/12/01 4:47:08 PM     Dave Wilson     SCR(s) 
 *        2256 2255 :
 *        Added DEFERRED_UNBLANK operation where gen_video_unblank only makes 
 *        the 
 *        video plane visible in cases where the driver reckons that a valid 
 *        image will
 *        be displayed. This cures various transients caused when Open apps 
 *        unblank a
 *        video plane before decoding something to it.
 *        
 *  141  mpeg      1.140       7/12/01 3:51:44 PM     Dave Wilson     SCR(s) 
 *        2249 :
 *        Added protection in StopLiveVideo. In standby for Vendor D's box, all
 *         MPEG and DRM interrupts are powered down. OpenTV calls StopLiveVideo
 *         from within standby so we can't wait for the idle interrupt in this 
 *        case.
 *        
 *  140  mpeg      1.139       7/9/01 6:17:28 PM      Dave Wilson     SCR(s) 
 *        2205 2206 2207 2208 2209 :
 *        Changed video data sent in SendFrameHeader to match values 
 *        recommended by hardware group.
 *        
 *  139  mpeg      1.138       7/9/01 11:33:38 AM     Dave Wilson     SCR(s) 
 *        2196 2197 :
 *        Added some debug code to help track down transients and channel 
 *        change
 *        problems.
 *        
 *  138  mpeg      1.137       7/3/01 6:19:36 PM      Dave Wilson     SCR(s) 
 *        2188 2189 :
 *        Added some debug traces to help determine cause of delay in starting 
 *        video
 *        
 *  137  mpeg      1.136       7/3/01 11:08:12 AM     Tim White       SCR(s) 
 *        2178 2179 2180 :
 *        Merge branched Hondo specific code back into the mainstream source 
 *        database.
 *        
 *        
 *  136  mpeg      1.135       7/2/01 10:19:04 AM     Dave Wilson     SCR(s) 
 *        2002 2003 :
 *        Fixed for some video transients on entry to and exit from Open apps
 *        
 *  135  mpeg      1.134       6/28/01 3:02:04 PM     Dave Wilson     SCR(s) 
 *        2105 2106 :
 *        New video microcode fixed the previous problem where it was not 
 *        telling us
 *        the correct buffer being displayed. New microcode fixes this and, as 
 *        a result,
 *        code to track this has been added to the video driver. As a result, P
 *         frame
 *        stills now decode correctly since they difference with the right 
 *        buffer.
 *        
 *  134  mpeg      1.133       6/15/01 12:07:06 PM    Dave Wilson     SCR(s) 
 *        2107 2108 :
 *        Backed out last change for the Fathom II problem. The function 
 *        check_and_copy_... was asynchronous and I think that the blitter was 
 *        trying to write to the same buffer as the video decoder, thereby 
 *        messing up some future still decodes on the Open menu.
 *        
 *  133  mpeg      1.132       6/10/01 12:33:12 PM    Dave Wilson     SCR(s) 
 *        1828 1829 :
 *        When a still decode for display is completed, the driver now copies 
 *        the
 *        P buffer to the I buffer. This ensures that the next still will 
 *        decode
 *        correctly if it is a P-picture. Note that this is an interim fix 
 *        until
 *        the microcode is changed to give reliable indication of which buffer 
 *        is
 *        being displayed.
 *        
 *  132  mpeg      1.131       6/8/01 8:20:54 AM      Dave Wilson     SCR(s) 
 *        1926 1927 :
 *        Changed blank/unblank to operate through the video queue and added
 *        option to defer unblanking (not currently used).
 *        Added code to blank the video during video_play startup. This gets 
 *        rid of the
 *        flash of old video and the  "unsynced" frame problems but is an 
 *        interim fix
 *        since this will likely cause flickering in some Open apps. We need to
 *         rework
 *        this one there is a reliable method of determining which buffer is 
 *        being 
 *        displayed.
 *        
 *  131  mpeg      1.130       6/5/01 10:06:08 AM     Dave Wilson     SCR(s) 
 *        2019 2020 :
 *        Added code to blank the video plane while starting video_play for 
 *        motion
 *        video. Without this, when channel changes occurred between  channels 
 *        whose
 *        MPEG video was different sizes, you sometimes got a burst of crud  as
 *         the
 *        video started.
 *        
 *  130  mpeg      1.129       6/1/01 4:31:48 PM      Dave Wilson     SCR(s) 
 *        2014 2015 :
 *        VideoDisableSync contained a trace_new call which is not interrupt 
 *        safe.
 *        Unfortunately, the function is called in cases where the video buffer
 *         
 *        overflows and this, in turn, caused a fatal_exit from 
 *        not_interrupt_safe.
 *        Changed to isr_trace_new and removed not_interrupt_safe.
 *        
 *  129  mpeg      1.128       5/31/01 6:45:02 PM     Dave Wilson     SCR(s) 
 *        1897 1898 1826 1827 1670 1389 1390 :
 *        Changes to prevent stills being decoded into the wrong buffer. This 
 *        is 
 *        complicated by the fact that there is no way to tell which buffer is 
 *        currently
 *        being displayed but we have used a scheme here which tries to decode 
 *        all stills
 *        for display into the P buffer and all non-display ones into the I 
 *        buffer. This
 *        cures many of the "blasts of crud" between stills that we had been 
 *        seeing (but
 *        not all of them).
 *        
 *  128  mpeg      1.127       5/22/01 2:47:24 PM     Dave Wilson     SCR(s) 
 *        1924 1925 :
 *        For some reason, we used to turn demux on after issuing the play 
 *        command and
 *        also wait until decode had started before turning video sync on. The 
 *        hardware
 *        guys tell me that we should turn sync on as soon as we tell the vcore
 *         to start
 *        decoding so this is now how it is done. Since this change, I have not
 *         seen the
 *        video hesitation problem but we still get flashes of the previous 
 *        image on
 *        occasions.
 *        
 *  127  mpeg      1.126       5/4/01 3:05:22 PM      Tim White       DCS#1822,
 *         DCS#1824, DCS31825 -- Critical Section Overhaul
 *        
 *  126  mpeg      1.125       4/26/01 1:03:26 PM     Dave Wilson     DCS1784: 
 *        Added code to wait for 2 decode complete interrupts or  a timeout
 *        before sending av_command_done when starting motion video.
 *        
 *  125  mpeg      1.124       4/20/01 12:17:36 PM    Dave Wilson     DCS1124: 
 *        Major changes to video memory management to get Sky Text app running
 *        
 *  124  mpeg      1.123       4/17/01 12:39:00 PM    Dave Wilson     DCS1720: 
 *        Replaced ResetVCore in StopLiveVideo with a stop command and a 
 *        wait for the idle interrupt. This minimises the use of ResetVCore 
 *        which
 *        has problems and can lock up in some situations (nasty, I know).
 *        
 *  123  mpeg      1.122       4/16/01 4:14:32 PM     Dave Wilson     DCS1707: 
 *        Replaced a trace_new inside SetWritePtr with isr_trace_new since
 *        this function is called from interrupt on some occasions.
 *        DCS1710: Replaced hardcoded timeout loop in ResetVCore with a call to
 *         
 *        task_time_sleep. Old timeout was not sufficiently long.
 *        
 *  122  mpeg      1.121       4/12/01 8:21:28 PM     Amy Pratt       DCS914 
 *        Removed Neches support.
 *        
 *  121  mpeg      1.120       4/11/01 6:55:52 PM     Quillian Rutherford Small
 *         change to remove references to call_av_command_done() to return to
 *        generic driver.
 *        
 *  120  mpeg      1.119       4/11/01 2:08:34 PM     Steve Glennon   DCS1673 
 *        (put by DW): Recoded to use the IDLE interrupt to decide when a 
 *        still decode has completed. Neither decode complete not buffer empty 
 *        interrupts are safe to use here for various reasons (buffer empty 
 *        happens
 *        too soon and decode complete can happen either once or twice for each
 *         still
 *        passed since some contain 2 fields).
 *        
 *  119  mpeg      1.118       4/9/01 4:13:34 PM      Steve Glennon   DCS??? 
 *        (tracker problems): Removed explicit references to av_command_done
 *        and call_av_command_done and used function pointer passed to 
 *        gen_flush_data
 *        instead. This is a generic driver and should have no knowledge of 
 *        OpenTV
 *        at all.
 *        
 *  118  mpeg      1.117       4/9/01 11:57:04 AM     Quillian Rutherford Added
 *         the DEBUG ifdeffing around the call_av_command_done type functions
 *        
 *  117  mpeg      1.116       4/6/01 4:33:08 PM      Quillian Rutherford 
 *        Various fixes to stop driver hangs and bad video on screen.  Not 
 *        perfect, but
 *        much nicer behaviour than before.
 *        ResetVCore protects itself somewhat now, read/write resets all funnel
 *         through
 *        an api, and still decodes wait on idle instead of decode complete.
 *        
 *  116  mpeg      1.115       4/2/01 7:58:50 PM      Steve Glennon   DCS# 
 *        1603/1604
 *        Made ResetVCore a little more safe with regard to 
 *        aud_before/after_reset
 *        Captured current state of audio enable before reset and restored 
 *        after reset
 *        
 *  115  mpeg      1.114       3/29/01 11:10:46 AM    Dave Wilson     DCS1495: 
 *        Removed infinite polling loops waiting for MPEG core to clear
 *        CmdValid status bit. Box now resets if this takes longer than about 
 *        1uS.
 *        
 *  114  mpeg      1.113       3/23/01 11:19:58 AM    Dave Wilson     DCS1470: 
 *        Got rid of a race condition in video_process handling of
 *        stills. av_command_done was called before processing of the play
 *        message was complete.
 *        
 *  113  mpeg      1.112       3/15/01 2:21:16 PM     Lucy C Allevato Changed 
 *        all the sem_get() functions for the local semaphore to use a #defined
 *        value from video.c  Also changed the message in gen_flush_data to use
 *         the
 *        proper function name for tracing.
 *        
 *  112  mpeg      1.111       3/14/01 2:50:58 PM     Lucy C Allevato 
 *        Implements changes for PVCS 1386, also renamed gen_flush_still_data 
 *        to 
 *        more generic gen_flush_data.  Make sure to use a fresh video.h for
 *        builds with this as there are changes from void to bool for some 
 *        function
 *        prototypes.
 *        
 *  111  mpeg      1.110       3/13/01 12:37:24 PM    Lucy C Allevato Changed 
 *        ResetVCore so that it waits one time for a length longer than 
 *        maximum required by the hardware to reset and then exits via 
 *        fatal_exit
 *        instead of using a while loop.
 *        This is for DCS 1383,1384,1385
 *        
 *  110  mpeg      1.109       3/9/01 9:07:42 AM      Dave Wilson     DCS13?? 
 *        (tracker down): Replaced wrong API names in trace messages to aid
 *        debug.
 *        
 *  109  mpeg      1.108       3/2/01 4:22:20 PM      Tim White       DCS#1365:
 *         Globally decrease stack memory usage.
 *        DCS#1366: Globally decrease stack memory usage.
 *        
 *  108  mpeg      1.107       3/1/01 8:46:54 PM      Steve Glennon   Fix for 
 *        DCS#898 - QVC Application Hangs with Green Screen
 *        Added code to gen_send_still_data to parse start of q buffer and 
 *        look for the sequence header and then the framerate. If invalid frame
 *         rate
 *        is found, patch to a valid value.
 *        Also took out some extraneous stuff in gen_send_still_data from 
 *        Quillian
 *        
 *  107  mpeg      1.106       3/1/01 5:58:26 PM      Steve Glennon   
 *        Effectively removed revision 1.105 - this version is the same as 
 *        1.104
 *        Need to move on, and 1.105 is badly broken.
 *        
 *  106  mpeg      1.105       2/28/01 3:09:52 PM     Lucy C Allevato Put in a 
 *        hwtimer to watch for hung still decodes.  Also a call to ResetVCore
 *        in the init function to try to deal with hung vcore on bootup.
 *        
 *  105  mpeg      1.104       2/22/01 3:25:54 PM     Lucy C Allevato Added a 
 *        counter to ResetVCore to call fatal_exit() if the vcore hangs
 *        during a reset.
 *        
 *  104  mpeg      1.103       2/16/01 2:53:40 PM     Ismail Mustafa  DCS 
 *        #1242. video_enable_sync nstubbed out for NDSTESTS.
 *        
 *  103  mpeg      1.102       2/14/01 11:42:42 AM    Lucy C Allevato Changed 
 *        the DecodeComplete interrupt to use a semaphore for signalling, also
 *        minor update to flushing.
 *        These are to help with blanking/unblanking video hardware only when a
 *         decode
 *        is complete.
 *        
 *  102  mpeg      1.101       2/7/01 9:44:26 AM      Ismail Mustafa  Removed 
 *        Rev B and Neches video microcode support to save memory. DCS #1096 &
 *        DCS #971.
 *        
 *  101  mpeg      1.100       2/2/01 4:02:34 PM      Angela          Merged 
 *        Vendor_C changes to the code(mostly #if CUSTOMER==VENDOR_C clauses) 
 *        see DCS#1049
 *        
 *  100  mpeg      1.99        2/1/01 1:19:28 PM      Tim Ross        DCS966.
 *        Added support for Hondo.
 *        
 *  99   mpeg      1.98        11/15/00 3:49:28 PM    Ismail Mustafa  Fix for 
 *        User Data. Client drivers expect data to be byte swapped.
 *        
 *  98   mpeg      1.97        11/8/00 5:12:04 PM     Lucy C Allevato Added 
 *        changes for new ucode still error handling. Now mark error and issue
 *        a continue when decoder goes idle to continue decoding the still.
 *        
 *  97   mpeg      1.96        11/6/00 2:35:02 PM     Ismail Mustafa  Added 
 *        capability for an external driver to register a function to handle
 *        video messages if needed.
 *        
 *  96   mpeg      1.95        10/18/00 3:35:04 PM    Lucy C Allevato Added a 
 *        fix to ResetVCore routine to reenable video interrupts after
 *        reset.
 *        
 *  95   mpeg      1.94        10/10/00 9:40:38 AM    Ismail Mustafa  Use 
 *        smaller sized microcode for COLORADO Rev. B. The microcode with the 
 *        AFD
 *        fix is too vig for Rev. B.
 *        
 *  94   mpeg      1.93        10/4/00 12:22:22 PM    Lucy C Allevato Added 
 *        code to gen_send_still_data to search for sequence error codes at
 *        the end of the queued still data to work around a problem with OpenTV
 *        stills that contain sequence errors instead of sequence ends at the
 *        end of the still.
 *        
 *  93   mpeg      1.92        9/11/00 4:57:56 PM     Ismail Mustafa  Increased
 *         the delay from 20 ms to 40 ms for NO_DISPLAY stills.
 *        
 *  92   mpeg      1.91        9/7/00 4:24:08 PM      Ismail Mustafa  Fixed 
 *        problem in flushing still data. When the callback was null it
 *        did not wait for completion before returning.
 *        
 *  91   mpeg      1.90        8/25/00 9:58:30 AM     Ismail Mustafa  Fix 
 *        warning in gen_video_init: unused local variable.
 *        
 *  90   mpeg      1.89        8/8/00 8:54:32 PM      Lucy C Allevato Added a 
 *        different fix for OpenTV 1.2 VTS MPG still no display that doesn't
 *        affect the demo. Now check whether we are decoding a still in no 
 *        display
 *        mode and schedule the callback at decode complete interrupt time.
 *        
 *  89   mpeg      1.88        8/8/00 5:55:18 PM      Ismail Mustafa  Reworked 
 *        fix from 1.87 which caused EN2 demo problems.
 *        
 *  88   mpeg      1.87        8/3/00 3:06:22 PM      Lucy C Allevato Changed 
 *        av_command_done callback for still decode to delay for about a field
 *        and wait for decode complete before scheduling the callback.
 *        
 *  87   mpeg      1.86        8/3/00 2:05:28 PM      Ismail Mustafa  Added 
 *        PLAY_LIVE_STATE_NO_SYNC.
 *        
 *  86   mpeg      1.85        7/13/00 6:23:16 PM     Lucy C Allevato Fixed a 
 *        bug in gen_flush_still_data where the last byte of a word was not
 *        cleared when flushing out leftover data in a word.
 *        
 *  85   mpeg      1.84        7/13/00 6:18:02 PM     Tim White       New drop 
 *        of DVR function with new microcode.
 *        
 *  84   mpeg      1.83        7/13/00 6:05:28 PM     Ismail Mustafa  Reworked 
 *        UserData code.
 *        
 *  83   mpeg      1.82        5/15/00 5:19:42 PM     Ismail Mustafa  Enabled 
 *        BITSTREAM_ERROR interrupts.
 *        
 *  82   mpeg      1.81        5/11/00 5:19:48 PM     Ismail Mustafa  Fixed 
 *        User Data processing.
 *        
 *  81   mpeg      1.80        5/1/00 4:56:04 PM      Ismail Mustafa  Encoded 
 *        Video End pointer has reappeared in Colorado and needs to
 *        be initialized.
 *        
 *  80   mpeg      1.79        4/26/00 6:34:32 PM     Ismail Mustafa  Back to 
 *        older microcode.
 *        
 *  79   mpeg      1.78        4/26/00 3:45:20 PM     Ismail Mustafa  Fixes for
 *         VxWorks.
 *        
 *  78   mpeg      1.77        4/14/00 7:02:14 PM     Ismail Mustafa  Use 
 *        assembly versions of microcode for Neches.
 *        
 *  77   mpeg      1.76        4/11/00 7:14:52 PM     Ismail Mustafa  Last rev 
 *        incorrectly reset the wrong interrupt for User Data.
 *        
 *  76   mpeg      1.75        4/10/00 6:07:18 PM     Ismail Mustafa  Reset 
 *        User Data interrupt.
 *        
 *  75   mpeg      1.74        4/7/00 12:59:30 PM     Ismail Mustafa  Modified 
 *        so that only specific microcode is included.
 *        
 *  74   mpeg      1.73        4/6/00 7:33:20 PM      Ismail Mustafa  Converted
 *         to C the microcode files.
 *        
 *  73   mpeg      1.72        4/6/00 12:00:04 PM     Ray Mack        fixes to 
 *        remove warnings
 *        
 *  72   mpeg      1.71        4/4/00 8:45:56 PM      Ismail Mustafa  Added API
 *         for User Data Support.
 *        
 *  71   mpeg      1.70        2/22/00 1:14:18 PM     Ismail Mustafa  Fixed bug
 *         in setting write wrap pointer.
 *        
 *  70   mpeg      1.69        2/18/00 5:22:44 PM     Ismail Mustafa  Removed 
 *        more bitfields.
 *        
 *  69   mpeg      1.68        2/4/00 1:08:44 PM      Ismail Mustafa  Fixed 
 *        bitfields problem.
 *        
 *  68   mpeg      1.67        1/31/00 4:13:54 PM     Ismail Mustafa  Bitstream
 *         errors are handled here instead of demux.
 *        
 *  67   mpeg      1.66        1/13/00 2:15:14 PM     Dave Wilson     Added 
 *        conditional compilation round CONFMGR calls
 *        
 *  66   mpeg      1.65        1/3/00 9:33:36 AM      Ismail Mustafa  COLORADO 
 *        changes plus changes to debug task.
 *        
 *  65   mpeg      1.64        12/7/99 4:02:10 PM     Ismail Mustafa  Added 
 *        #define INCL_DPS for new pawser.
 *        
 *  64   mpeg      1.63        11/9/99 11:06:52 AM    Ismail Mustafa  Added 
 *        code for loading RevA or RevB microcode.
 *        
 *  63   mpeg      1.62        11/8/99 4:19:34 PM     Ismail Mustafa  Changed 
 *        to use gen_dmx_control_av_channel.
 *        
 *  62   mpeg      1.61        11/3/99 4:19:20 PM     Ismail Mustafa  Fixed 
 *        race condition in flush function for BUFFER_EMPTY IRQ.
 *        
 *  61   mpeg      1.60        11/1/99 2:07:30 PM     Ismail Mustafa  Removed 
 *        warning.
 *        
 *  60   mpeg      1.59        10/27/99 5:01:24 PM    Dave Wilson     Changed 
 *        WAIT_FOREVER to KAL_WAIT_FOREVER
 *        
 *  59   mpeg      1.58        10/26/99 5:58:12 PM    Ismail Mustafa  Added 
 *        gen_video_Start/StopSendPesData();
 *        
 *  58   mpeg      1.57        10/7/99 2:24:54 PM     Ismail Mustafa  New 
 *        generic video driver. Also removed SABINE code.
 *        Note last rev. is missing. It was a change to the now removed Sabine 
 *        code.
 *        
 *  57   mpeg      1.56        10/6/99 4:24:26 PM     Anzhi Chen      Commented
 *         out DemuxReInitHW() call to make SABINE build work.
 *        
 *  56   mpeg      1.55        9/28/99 3:57:48 PM     Ismail Mustafa  Now uses 
 *        the generic demux driver.
 *        Also now video_init will wait until the video task is running before
 *        returning. This should fix the problem of video commands being lost 
 *        initially
 *        
 *  55   mpeg      1.54        9/24/99 6:35:26 PM     Ismail Mustafa  For 
 *        DMFRONT CommandDone should be a volatile bool so non debug
 *        builds would work.
 *        
 *  54   mpeg      1.53        9/2/99 5:40:58 PM      Ismail Mustafa  Removed 
 *        hack for green garbage on Neches.
 *        
 *  53   mpeg      1.52        8/25/99 5:07:42 PM     Ismail Mustafa  Added EN2
 *         initial changes.
 *        
 *  52   mpeg      1.51        8/11/99 5:17:36 PM     Dave Wilson     Changed 
 *        KAL calls to use new API.
 *        
 *  51   mpeg      1.50        7/19/99 3:57:58 PM     Ismail Mustafa  Reset 
 *        only Neches HW.
 *        
 *  50   mpeg      1.49        7/14/99 4:50:00 PM     Ismail Mustafa  When 
 *        turning off live video and there is no data comming in from the 
 *        parser,
 *        the INT6 never occurs and the VCore will not switch modes. In this 
 *        case
 *        a reset is needed.
 *        
 *  49   mpeg      1.48        6/28/99 4:29:56 PM     Ismail Mustafa  Added 
 *        SWAP_COPY version to avoid using HW SWAP.
 *        
 *  48   mpeg      1.47        6/24/99 8:06:42 PM     Ismail Mustafa  New 
 *        version with cleaner switching from stills to live.
 *        The number of flushing bytes needs to be 256.
 *        
 *  47   mpeg      1.46        6/22/99 9:03:10 PM     Ismail Mustafa  Removed 
 *        delays and no longer turns off audio sync.
 *        
 *  46   mpeg      1.45        6/22/99 6:54:00 PM     Ismail Mustafa  First 
 *        working Neches Version.
 *        
 *  45   mpeg      1.44        6/3/99 5:21:42 PM      Ismail Mustafa  Added 
 *        CheckMem function to check for free memory.
 *        
 *  44   mpeg      1.43        5/10/99 8:58:10 AM     Ismail Mustafa  Added 
 *        call to osdSetMpgScaleDelay for Neches.
 *        
 *  43   mpeg      1.42        5/4/99 3:52:10 PM      Dave Wilson     Removed a
 *         compiler warning.
 *        
 *  42   mpeg      1.41        4/30/99 5:26:00 PM     Ismail Mustafa  Set 
 *        debug_sleep_time to 20 only for emulation.
 *        
 *  41   mpeg      1.40        4/27/99 11:35:18 AM    Ismail Mustafa  Fixed up 
 *        flushing for Neches.
 *        
 *  40   mpeg      1.39        4/16/99 6:14:32 PM     Ismail Mustafa  Added 
 *        reset code.
 *        
 *  39   mpeg      1.38        4/15/99 11:25:16 AM    Ismail Mustafa  Neches 
 *        stills are now working.
 *        
 *  38   mpeg      1.37        3/26/99 10:08:12 AM    Ismail Mustafa  Added 
 *        some Neches changes.
 *        
 *  37   mpeg      1.36        3/24/99 3:44:04 PM     Ismail Mustafa  Fixed 
 *        problem in StopLiveVideo. Comment of line was missing>
 *        
 *  36   mpeg      1.35        3/10/99 10:26:22 AM    Ismail Mustafa  Fixed 
 *        problem with astra1272175 stream. OpenTV APP will only call
 *        Play Still once and send multiple stills.
 *        
 *  35   mpeg      1.34        2/15/99 1:11:20 PM     Ismail Mustafa  Do not 
 *        turn off sync when doing video_play.
 *        
 *  34   mpeg      1.33        1/29/99 9:24:44 AM     Ismail Mustafa  Cleanup 
 *        warnings.
 *        
 *  33   mpeg      1.32        1/27/99 4:19:38 PM     Ismail Mustafa  Removed 
 *        resetting of R/W pointers in WaitForCtlr. This seems to fix
 *        hangs on channel changing from no video to video.
 *        The original fix was to try to get the video decoder to accept a 
 *        command
 *        if it stays busy.
 *        
 *  32   mpeg      1.31        1/21/99 4:11:22 PM     Ismail Mustafa  Make the 
 *        WaitOn Ctlr give up after a while. This should fix the
 *        channel changing in the demo.
 *        
 *  31   mpeg      1.30        1/21/99 2:03:16 PM     Ismail Mustafa  OpenTV 
 *        changes:
 *        1. Blank the screen by disabling MPEG after first switch from Live to
 *         Stills.
 *        This is to hide the green junk that is displayed when switching.
 *        2. Added code to make switching process more robust. Now before 
 *        sending
 *        a new command we check the Command status and try to clear it up by
 *        resetting the read/Write pointers.
 *        
 *  30   mpeg      1.29        1/9/99 12:10:54 PM     Ismail Mustafa  Added 
 *        FlushMPG data but currently not using it.
 *        
 *  29   mpeg      1.28        1/7/99 6:00:42 PM      Ismail Mustafa  Added 
 *        NCR_BASE so buffers are accessed in non cached space.
 *        
 *  28   mpeg      1.27        12/22/98 4:14:04 PM    Ismail Mustafa  Changes 
 *        for new microcode to fix prblem of PLAY command with not data.
 *        Also when flushing stills write 512 bytes to ensure internal decoder
 *        buffer is flushed,
 *        
 *  27   mpeg      1.26        12/18/98 9:21:26 AM    Ismail Mustafa  Cleanup 
 *        up the switch code. Now it will just set the write pointer to the
 *        read pointer.
 *        
 *  26   mpeg      1.25        12/17/98 4:22:54 PM    Ismail Mustafa  Turn sync
 *         off when messing with the Read/Write Pointers.
 *        
 *  25   mpeg      1.24        12/14/98 5:30:36 PM    Ismail Mustafa  Disable 
 *        sync if in VBV mode.
 *        Zero first 512 bytes of encoded video buffer before resetting 
 *        pointers.
 *        
 *  24   mpeg      1.23        12/11/98 1:57:02 PM    Ismail Mustafa  Disable 
 *        PID in ISR.
 *        
 *  23   mpeg      1.22        12/11/98 10:34:14 AM   Ismail Mustafa  Changes 
 *        for use of int6 of the MPEG core. Also remove previous hacks 
 *        to disable audio pid and sync.
 *        
 *  22   mpeg      1.21        12/8/98 4:38:52 PM     Ismail Mustafa  Now calls
 *         demux_av_control_channel.
 *        
 *  21   mpeg      1.20        12/7/98 11:09:28 AM    Ismail Mustafa  Changes 
 *        from week at OpenTV.
 *        
 *  20   mpeg      1.19        11/28/98 5:37:32 PM    Ismail Mustafa  Added 
 *        demo hack to enable/disable the audio PID at same time as video PID.
 *        
 *  19   mpeg      1.18        11/25/98 5:00:54 PM    Ismail Mustafa  Changes 
 *        for Stills and live MPEG switching.
 *        
 *  18   mpeg      1.17        11/19/98 5:23:22 PM    Ismail Mustafa  Fix for 
 *        live MPEG to Still switch.
 *        
 *  17   mpeg      1.16        11/17/98 6:58:48 PM    Ismail Mustafa  Switch 
 *        back to using local buffer. This version seems more stable with 
 *        OpenTV
 *        
 *  16   mpeg      1.15        10/15/98 11:28:56 AM   Ismail Mustafa  Made a 
 *        variable for the sleep time of the debug process.
 *        
 *  15   mpeg      1.14        10/9/98 4:30:08 PM     Ismail Mustafa  Moved the
 *         call to av_command_done out of the ISR and into the video task.
 *        
 *  14   mpeg      1.13        10/2/98 2:42:14 PM     Ismail Mustafa  Cleanup  
 *        some messages.
 *        
 *  13   mpeg      1.12        9/29/98 3:25:48 PM     Ismail Mustafa  Added a 
 *        debug task to trace video and audio R/W pointers.
 *        
 *  12   mpeg      1.11        9/18/98 3:25:20 PM     Ismail Mustafa  Moved 
 *        av_command_done over here from root.c of dmxtest.
 *        
 *  11   mpeg      1.10        8/21/98 5:19:56 PM     Ismail Mustafa  Added 
 *        code to bypass intermediate buffer for MPEG stills.
 *        This is now the default operations.
 *        
 *  10   mpeg      1.9         8/20/98 6:14:34 PM     Ismail Mustafa  Was 
 *        updating the queue out offset when it was already updated in the
 *        copy_data function.
 *        
 *  9    mpeg      1.8         8/5/98 6:30:26 PM      Ismail Mustafa  
 *        MPG_VB_EMPTY added to ISR bits in service routine.
 *        
 *  8    mpeg      1.7         8/5/98 3:42:56 PM      Ismail Mustafa  Added 
 *        av_command_done to videe_play.
 *        
 *  7    mpeg      1.6         7/22/98 11:18:58 AM    Ismail Mustafa  Several 
 *        changed relating to channel change and reset.
 *        
 *  6    mpeg      1.5         6/9/98 3:36:14 PM      Ismail Mustafa  Now calls
 *         video init.
 *        
 *  5    mpeg      1.4         6/2/98 6:45:16 PM      Ismail Mustafa  New code 
 *        per latest OpenTV spec.
 *        
 *  4    mpeg      1.3         5/28/98 4:16:44 PM     Amy Pratt       added 
 *        num_vid_leftovers so that copy_data() can keep track of
 *        any non-word aligned data.
 *        
 *  3    mpeg      1.2         5/26/98 11:10:18 AM    Dave Wilson     Added 
 *        #ifndef OPENTV round av_command_done.
 *        
 *  2    mpeg      1.1         4/17/98 10:21:26 AM    Ismail Mustafa  Added 
 *        HWConfig.h.
 *        
 *  1    mpeg      1.0         4/13/98 11:49:12 AM    Ismail Mustafa  
 * $
 ****************************************************************************/

