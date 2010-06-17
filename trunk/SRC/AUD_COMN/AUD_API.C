/****************************************************************************/ 
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*          Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002          */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename: AUD_API.C
 *
 *
 * Description: Repository for new audio API. 
 *              Play Audio from Memory.
 *
 *
 * Author: Craig Dry
 *
 ****************************************************************************/
/* $Header: AUD_API.C, 43, 4/6/04 1:51:57 PM, Joe Kroesche$
****************************************************************************/ 
#define _AUD_API_C_
#define AC3_ALIGNED

/*===========================================================================
 *
 * include files
 *
 *===========================================================================
 */
#include <string.h>
#include "stbcfg.h"
#if defined OPENTV
#include "opentvx.h"
#else
#include "basetype.h"
#include "cnxtv.h"
#endif
#include "audio.h"
#include "kal.h"
#include "retcodes.h"
#include "globals.h"
#include "aud_comn.h"
#include "aud_api.h"

/*===========================================================================
 * public data           
 *===========================================================================*/
/*===========================================================================
 * external function prototypes  
 *===========================================================================*/
#ifdef AC3_ALIGNED
extern int copy_data_ac3_aligned( farcir_q *, int, bool );
extern int handle_PES_ac3_aligned( farcir_q *, PES_global_data *, bool );
#endif /* AC3_ALIGNED */

extern void gen_audio_play( bool SyncWithPCR );
extern void gen_audio_stop( void );
extern void hw_aud_stop( bool bClearBuffers );
extern void hw_aud_start( void );
extern void hw_set_aud_mute(bool enableMute);
#if (CANAL_AUDIO_HANDLING==YES)
extern bool audio_ramp_vol_down( u_int32 min_time_down );
extern void audio_ramp_vol_up( void );
#endif
/*===========================================================================
 * external data           
 *===========================================================================*/
extern u_int32 gDemuxInstance;
#if (CANAL_AUDIO_HANDLING==YES)
extern sem_id_t aud_ramp_up_sem;
#endif
/*===========================================================================
 * local defines
 *===========================================================================*/
#define NAUDIO            1    /* Total instances of Audio Hardware available */
#define AUDREQ_ARRAY_SIZE 64   /* Must be a power of 2!!! */
#define HWENC_LOWATER     2048 /* Low water mark must be multiple of 64 bytes */
#define MIN_PAUSE_TIME    250  /* Min time in ms between pause and resume     */
#define MAX_PES_LOOKAHEAD 8192 /* Maximum PES lookahead                       */
/*===========================================================================
 * local typedefs
 *===========================================================================*/
/* Audio Instance State */
typedef enum {
  CNXT_AUDIO_STATE_CLOSED,  /* successful cnxt_audio_close                   */
  CNXT_AUDIO_STATE_OPENED,  /* successful cnxt_audio_open                    */
  CNXT_AUDIO_STATE_START1,  /* successful cnxt_audio_start                   */
  CNXT_AUDIO_STATE_START2,  /* format determined & ucode has been downloaded */
  CNXT_AUDIO_STATE_PLAYING  /* play_threshold is met or exceeded             */
} CNXT_AUDIO_STATE;

typedef enum {
  CNXT_AUDIO_FEED_DATA,
  CNXT_AUDIO_LOW_WATER_INT,
  CNXT_AUDIO_START_PLAY,
  CNXT_AUDIO_BUF_EMPTY,
  CNXT_AUDIO_PTS_RECEIVED_INT
} CNXT_AUDIO_PROCESS_CMDS;

typedef struct {
  void                      *check;
  CNXT_AUDIO_STATE          state;
  CNXT_AUDIO_CALLBACK_PFN   pfn_callback;
  void                      *pinfo;
  u_int32                   play_threshold;     
  u_int32                   request_threshold;
  u_int32                   bytes_avail;   /* Total audio data in all clips */
  CNXT_AUDIO_FORMAT         format;
  CNXT_AUDIO_ENCAPSULATION  encap;
  CNXT_AUDIO_SOURCE         source;
  u_int32                   in;
  u_int32                   out;
  farcir_q                  audreqs[AUDREQ_ARRAY_SIZE];
  CNXT_CLIP_EV_DATA         evdata[AUDREQ_ARRAY_SIZE];
} AudioNode;


/*===========================================================================
 * local (static) function prototypes           
 *===========================================================================*/
static void audio_process(void *arg);
static void audio_process_msg(u_int32 *message);
static bool extract_audio_format(void                     *pbuf,
                                 u_int32                  length,
                                 CNXT_AUDIO_FORMAT        *fmt,
                                 CNXT_AUDIO_ENCAPSULATION *encap,
                                 CNXT_AUDIO_CHAN_MODE     *chanmode,
                                 CNXT_AUDIO_MPEG_LAYER    *layer,
                                 CHAN_FLAG                *avail_chans,
                                 unsigned int             *bitrate,
                                 unsigned int             *sampfreq,
                                 unsigned int             *halffreq,
                                 unsigned int             *totalchans,
                                 u_int32                  *pes_offset);
static void report_event(AudioNode *h, u_int32 clip_idx, CNXT_AUDIO_EVENT event);
static void wait_resume_complete(void);
static void ResumeTickCallback(tick_id_t tickTimer, void *pUserData);
static void resume_now(void);

/*===========================================================================
 * local (static) data                 
 *===========================================================================*/
static bool initialized = FALSE;
static task_id_t         AudioProcessID;
static queue_id_t        AudioQueueID;
static AudioNode         anode[NAUDIO];
static HAUDIO            handles[NAUDIO];
static CNXT_AUDIO_FORMAT ucode_fmt;
static CNXT_AUDIO_FORMAT ucode_fmt_at_open;
static bool              ABEmptyIntEnabled;   
static bool              ABLowWaterIntEnabled;    
static bool              APTSReceivedIntEnabled;    
static PES_global_data   aud_PES_global_data;
static bool              resume_active;
static bool              feed_active;
static u_int32           volume_ramp_target_tick;
static tick_id_t         ResumeAudioTick;

 
static int  mpeg_sampfreq[] = { 44, 48, 32, 0 };
static int  mpeg_kbps[4][16] = { 
  /* rsvd */       {   0 },
  /* layer III */  {   0,  32,  40,  48,  56,  64,  80,  96,
                     112, 128, 160, 192, 224, 256, 320,   0},
  /* layer II */   {   0,  32,  48,  56,  64,  80,  96, 112,
                     128, 160, 192, 224, 256, 320, 384,   0},
  /* layer I */    {   0,  32,  64,  96, 128, 160, 192, 224,
                     256, 288, 320, 352, 384, 416, 448,   0} };

static int  ac3_sampfreq[] = { 48, 44, 32, 0 };
/* ac3_kbps[] entries are for 48Khz only. Other rates are not supported */
static int  ac3_kbps[] = {  32,  32,  40,  40,  48,  48,  56,  56,  64,  64,
                            80,  80,  96,  96, 112, 112, 128, 128, 160, 160,
                           192, 192, 224, 224, 256, 256, 320, 320, 384, 384,
                           448, 448, 512, 512, 576, 576, 640, 640};

typedef enum {
  THRESHOLD_CMD,
  REGISTER_CMD,
  OPEN_CMD,
  CLOSE_CMD,
  START_CMD,
  STOP_CMD,
  PAUSE_CMD,
  RESUME_CMD,
  RESUME_CMD_FINISHED,
  DATA_CMD,
  CALLBACK,
  RETURN,
  DATA_COPY,
  ERRORCODE,
  PTS_RECEIVED_INT
} TRACE_ENTRY;
#define NTRACE_ENTRIES 300
/* #define REALTIME_TRACE_LOGGING */
#ifdef REALTIME_TRACE_LOGGING
static int     nxt_trace_entry;
static u_int32 trace_entries[NTRACE_ENTRIES * 5];
static bool    trace_active    = TRUE;
static bool    trace_immediate = FALSE;

void output_trace_log_entry(u_int32 entry_idx)
{
  u_int32  time, entry, p1, p2, p3;
  u_int32  idx; 
  char     *sptr;
  char     estr[100];

  if (!trace_active)
    return;

  idx = (entry_idx << 2) + entry_idx;
  time  = trace_entries[idx + 0] - trace_entries[0];
  entry = trace_entries[idx + 1];
  p1    = trace_entries[idx + 2];
  p2    = trace_entries[idx + 3];
  p3    = trace_entries[idx + 4];

  switch(entry)
    {
    case  THRESHOLD_CMD:
      sptr = "%5d: THRESHOLD_CMD play_thresh = %d, req_thres = %d, %d\n";
      break;
    case  REGISTER_CMD:
      sptr = "%5d: REGISTER_CMD\n";
      break;
    case  OPEN_CMD:
      sptr = "%5d: OPEN_CMD\n";
      break;
    case  CLOSE_CMD:
      sptr = "%5d: CLOSE_CMD\n";
      break;
    case  START_CMD:
      sptr = "%5d: START_CMD\n";
      break;
    case  STOP_CMD:
      sptr = "%5d: STOP_CMD\n";
      break;
    case  PAUSE_CMD:
      sptr = "%5d: PAUSE_CMD\n";
      break;
    case  RESUME_CMD:
      sptr = "%5d: RESUME_CMD\n";
      break;
    case  RESUME_CMD_FINISHED:
      sptr = "%5d: RESUME_CMD_FINISHED\n";
      break;
    case  DATA_CMD:
      sptr = "%5d: DATA_CMD     index = %2d, bufptr = 0x%X, length = %5d\n";
      break;
    case  CALLBACK:
      strcpy(estr, "%5d: <-CALLBACK 0x%X ");
      if (p1 & EV_CLIP_PLAY)
        strcat(estr, "PLAY ");
      if (p1 & EV_CLIP_STOPPED)
        strcat(estr, "STOPPED ");
      if (p1 & EV_CLIP_FEED)
        strcat(estr, "FEED ");
      if (p1 & EV_CLIP_COPIED)
        strcat(estr, "COPIED ");
      if (p1 & EV_CLIP_ERROR)
        strcat(estr, "ERROR ");
      strcat(estr, "index = %d\n");
      sptr = estr;
      break;
    case RETURN:
      sptr = "%5d: ->RETURN FROM CALLBACK\n";
      break;
    case  DATA_COPY:
      sptr = "%5d:   DATA_COPY  ncopy = %2d, NtoCopy = %5d,   Copied = %5d\n";
      break;
    case ERRORCODE:
      sptr = "%5d:   ERROR occured at location %d!!!!\n";
      break;
    case PTS_RECEIVED_INT:
      sptr = "%5d:   AUDIO_PTS_RECEIVED_INTerrupt\n";
      break;
    default:
      break;
    }
  
  trace_new(TRACE_MPG|TRACE_LEVEL_ALWAYS, sptr, time, p1, p2, p3);
  //  printf(sptr, time, p1, p2, p3);
}

void trace_dump(void)
{
  int i;

  if (!trace_active)
    return;
  for (i = 0; i < nxt_trace_entry ; i++) {
    output_trace_log_entry(i);
    task_time_sleep(30);
  }
}

void trace_ini(bool active, bool immediate)
{
  trace_active    = active;
  trace_immediate = immediate;
  nxt_trace_entry = 0;
}

void trace_log(TRACE_ENTRY entry,  u_int32 p1, u_int32 p2, u_int32 p3)
{
  int idx;
  
  if (!trace_active)
    return;
  if (nxt_trace_entry == NTRACE_ENTRIES) 
    return;
  
  idx = (nxt_trace_entry << 2) + nxt_trace_entry;
  trace_entries[idx + 0] = get_system_time();
  trace_entries[idx + 1] = entry;
  trace_entries[idx + 2] = p1;
  trace_entries[idx + 3] = p2;
  trace_entries[idx + 4] = p3;
  
  if (trace_immediate)
    output_trace_log_entry(0);
  else 
    nxt_trace_entry++;
}
#else
#define output_trace_log_entry if (0) ((int (*)(u_int32)) 0)
#define trace_dump             if (0) ((int (*)(void)) 0)
#define trace_ini              if (0) ((int (*)(bool, bool)) 0)
#define trace_log              if (0) ((int (*)(TRACE_ENTRY, u_int32, u_int32, u_int32)) 0)
#endif /* ifdef REALTIME_TRACE_LOGGING */

/*********************************************************************/
/*   FUNCTION:     cnxt_audio_empty_int                              */
/*                                                                   */
/*   PARAMETERS:   idx    - Index to Audio handle                    */
/*                                                                   */
/*   DESCRIPTION:  Called from audio interrupt routine whenever      */
/*                 there is an audio buffer empty interrupt.         */
/*                 It sends a message to the audio task where        */
/*                 the work is actually done.                        */
/*                                                                   */
/*   RETURNS:      None                                              */
/*                                                                   */
/*********************************************************************/
void cnxt_audio_empty_int(u_int32 idx){
  u_int32 message[4];
  if (ABEmptyIntEnabled) {
    message[0] = (u_int32)handles[idx];
    message[1] = CNXT_AUDIO_BUF_EMPTY;
    qu_send(AudioQueueID, message);
  }
}

/*********************************************************************/
/*   FUNCTION:     cnxt_audio_lowwater_int                           */
/*                                                                   */
/*   PARAMETERS:   idx    - Index to Audio handle                    */
/*                                                                   */
/*   DESCRIPTION:  Called from audio interrupt routine whenever      */
/*                 there is a low water interrupt.                   */
/*                 It sends a message to the audio task where        */
/*                 the work is actually done.                        */
/*                                                                   */
/*   RETURNS:      None                                              */
/*                                                                   */
/*********************************************************************/
void cnxt_audio_lowwater_int(u_int32 idx){
  u_int32 message[4];
  if (ABLowWaterIntEnabled) {
    message[0] = (u_int32)handles[idx];
    message[1] = CNXT_AUDIO_LOW_WATER_INT;
    qu_send(AudioQueueID, message);
  }
}

/*********************************************************************/
/*   FUNCTION:     cnxt_audio_pts_received_int                       */
/*                                                                   */
/*   PARAMETERS:   idx    - Index to Audio handle                    */
/*                                                                   */
/*   DESCRIPTION:  Called from audio interrupt routine whenever      */
/*                 there is an audio pts received interrupt.         */
/*                 It sends a message to the audio task where        */
/*                 the work is actually done.                        */
/*                                                                   */
/*   RETURNS:      None                                              */
/*                                                                   */
/*********************************************************************/
void cnxt_audio_pts_received_int(u_int32 idx){
  u_int32 message[4];
  if (APTSReceivedIntEnabled) {
    message[0] = (u_int32)handles[idx];
    message[1] = CNXT_AUDIO_PTS_RECEIVED_INT;
    qu_send(AudioQueueID, message);
  }
}

/*********************************************************************/
/*   FUNCTION:     cnxt_audio_init                                   */
/*                                                                   */
/*   PARAMETERS:   useAC3ucode - initial state of audio microcode    */
/*                                                                   */
/*   DESCRIPTION:  Creates the Audio task and associated message     */
/*                 queue.  Initializes variables                     */
/*                                                                   */
/*   RETURNS:      TRUE  - Initialization succeeded                  */
/*                 FALSE - Initialization failed                     */
/*                                                                   */
/*********************************************************************/
bool cnxt_audio_init(bool useAC3ucode)
{
  bool ret;
  int i;

  if (initialized) {
    ret = TRUE;        /* Can only be initialized once */
  }
  else if ( (AudioQueueID = qu_create(20, "AIDQ")) == 0){
    trace_new(TRACE_MPG|TRACE_LEVEL_ALWAYS,"AUDIO:Queue Create Failed\n");
    error_log(RC_MPG_QCREATE_FAILED + ERROR_FATAL);
    ret = FALSE;
  }
  else if ( (AudioProcessID = task_create((PFNTASK) audio_process, 0, 0,
                                          AID_TASK_STACK_SIZE, AID_TASK_PRIORITY, AID_TASK_NAME)) == 0){
    trace_new(TRACE_MPG|TRACE_LEVEL_ALWAYS,"AUDIO:Process Create Failed\n");
    error_log(RC_MPG_PCREATE_FAILED + ERROR_FATAL);
    ret = FALSE;
  }
  else if ( (ResumeAudioTick = tick_create(ResumeTickCallback,  (void *)NULL, "AUDR")) == 0) {
      trace_new(TRACE_MPG|TRACE_LEVEL_ALWAYS, "Can't create Resume Audio tick counter!\n");
      error_log(ERROR_FATAL);
      ret = FALSE;
    }
  else {
    for (i = 0; i < NAUDIO; i++)
      {
        handles[i]                 = (void *)&anode[i];
        anode[i].check             = (void *)&anode[i];
        anode[i].state             = CNXT_AUDIO_STATE_CLOSED;
        anode[i].pfn_callback      = NULL;
        anode[i].pinfo             = NULL;
        anode[i].play_threshold    = 0;
        anode[i].request_threshold = HWBUF_ENCAUD_SIZE;
      }
    if (useAC3ucode)
      ucode_fmt = CNXT_AUDIO_FMT_AC3;
    else
      ucode_fmt = CNXT_AUDIO_FMT_MPEG;

    ABEmptyIntEnabled    = FALSE; 
    ABLowWaterIntEnabled = FALSE;
    APTSReceivedIntEnabled = FALSE;
    initialized          = TRUE;
    resume_active        = FALSE;
    feed_active          = FALSE;
    trace_ini(TRUE, FALSE);
    ret                  = TRUE;
  }

  return ret;
}

/*********************************************************************/
/*   FUNCTION:     audio_process                                     */
/*                                                                   */
/*   PARAMETERS:   void *arg                                         */
/*                                                                   */
/*   DESCRIPTION:  This is the Audio task.  It waits on messages     */
/*                 and then processes them                           */
/*                                                                   */
/*   RETURNS:      never                                             */
/*                                                                   */
/*********************************************************************/
void audio_process(void *arg)
{
  u_int32 message[4];
  int     iRetcode;

  while(1)
    {
      iRetcode = qu_receive(AudioQueueID, KAL_WAIT_FOREVER, message);
      if(iRetcode == RC_OK)
        {
          audio_process_msg(message);
        }
      else 
        trace_log(ERRORCODE, 1, 0, 0);
    }
  return; /* Should never get here! */
}

/*********************************************************************/
/*   FUNCTION:     audio_process_msg                                 */
/*                                                                   */
/*   PARAMETERS:   message - event that has occurred for a           */
/*                           specified handle                        */
/*                                                                   */
/*   DESCRIPTION:  called by audio_process with 1 of 4 different     */
/*                 types of message:                                 */
/*                 CNXT_AUDIO_FEED_DATA - app feeds data to audio    */
/*                                        task.                      */
/*                 CNXT_AUDIO_LOW_WATER_INT - Each time data in the  */
/*                            audio encoded buffer gets below a      */
/*                            certain point, the interrupt handler   */
/*                            notifies us by sending this message.   */
/*                 CNXT_AUDIO_BUF_EMPTY - When the audio buffer runs */
/*                            dry, the interrupt handler lets us know*/
/*                 CNXT_AUDIO_PTS_RECEIVED_INT - When an audio pts   */
/*                            has been received by demux             */
/*                                                                   */
/*   RETURNS:      none                                              */
/*                                                                   */
/*********************************************************************/
int ncopy = 0;
void audio_process_msg(u_int32 *message)
{                                                            
  bool           ks;
  int            bytes_to_copy;
  int            bytes_copied;
  farcir_q       *q;
  CNXT_CLIP_EV_DATA *pevdata;
  AudioNode      *h       = (AudioNode *)message[0];
  u_int8         stream_id;
  bool           useAC3ucode;

  switch(message[1])
    {
    case CNXT_AUDIO_FEED_DATA:
      
      feed_active = FALSE;
      if ((h->state == CNXT_AUDIO_STATE_START2) && 
          (h->bytes_avail >= h->play_threshold)) {
        /*
         *  Since we last issued 'cnxt_audio_start', this is the first time we've
         *  had enough data to actually start playing.
         */
   
        *DPS_AUDIO_LOWWATERMARK_EX(gDemuxInstance) = HWENC_LOWATER;

        ks = critical_section_begin();
        *glpIntMask   &= ~MPG_AB_EMPTY;         /* Disable Buff Empty Int */
        *glpIntStatus =  MPG_AB_EMPTY;
        ABEmptyIntEnabled    = FALSE; 
        *glpIntMask   |=  MPG_AB_LOWWATERMARK;  /* Enable  Lowwater Int   */ 
        *glpIntStatus =  MPG_AB_LOWWATERMARK;   
        ABLowWaterIntEnabled = TRUE;
        critical_section_end(ks);

        /* Initialization of variables used for packet header parsing */
        aud_PES_global_data.where_in_PES         = FIND_PREFIX;
        aud_PES_global_data.progress             = LAST_BYTE_NONZERO;
        aud_PES_global_data.bytes_left_in_packet = 0;
        aud_PES_global_data.bytes_left_in_header = 0;
        aud_PES_global_data.pts_present          = 0;
   
        h->state = CNXT_AUDIO_STATE_PLAYING;
        report_event(h, h->out, EV_CLIP_PLAY);
      }

      /* fall through */
      
    case CNXT_AUDIO_LOW_WATER_INT:
      
      if ((h->state == CNXT_AUDIO_STATE_PLAYING) && (h->bytes_avail > 0)) { 
        /* play_threshold has been achieved at lease once since last start */
        while( (h->in != h->out) ) {
          q       = &h->audreqs[h->out];
          pevdata = &h->evdata[h->out];
          bytes_to_copy = (q->size + q->in - q->out) % q->size;
#ifdef AC3_ALIGNED
          if (h->encap == CNXT_AUDIO_ENCAP_ES) 
          {
             if (h->format == CNXT_AUDIO_FMT_AC3)
                bytes_copied = copy_data_ac3_aligned((farcir_q*)q, bytes_to_copy, 1 );
             else
                bytes_copied = copy_data((farcir_q*)q, bytes_to_copy, 1 );
          }
          else
          {
             if (h->format == CNXT_AUDIO_FMT_AC3)
                bytes_copied = handle_PES_ac3_aligned((farcir_q*)q, &aud_PES_global_data, 1 );
             else
                bytes_copied = handle_PES((farcir_q*)q, &aud_PES_global_data, 1 );
          }
#else
          if (h->encap == CNXT_AUDIO_ENCAP_ES) 
            bytes_copied = copy_data((farcir_q*)q, bytes_to_copy, 1 );
          else
            bytes_copied = handle_PES((farcir_q*)q, &aud_PES_global_data, 1 );
#endif /* AC3_ALIGNED */
          h->bytes_avail -= bytes_copied;
          
          /*
            trace_new(TRACE_MPG|TRACE_LEVEL_ALWAYS,
            "ncopy = %d, bytes_to_copy =%d, bytes_copied = %d\n",
            ++ncopy, bytes_to_copy, bytes_copied);
          */
          trace_log(DATA_COPY, ++ncopy, bytes_to_copy, bytes_copied);
            
          if (bytes_to_copy == bytes_copied) {
            /* All of the Clip has been copied to the Decode Buffer */
            report_event(h, h->out, EV_CLIP_COPIED);
            h->out  = (h->out + 1) & (AUDREQ_ARRAY_SIZE - 1);
            if (ABEmptyIntEnabled == FALSE) {
              ks = critical_section_begin();
              *glpIntMask  |=  MPG_AB_EMPTY;        /* Enable Buf Empty  Int */
              ABEmptyIntEnabled    = TRUE; 
              critical_section_end(ks);
            }
          }
          else if ((h->encap == CNXT_AUDIO_ENCAP_PES) &&
                   (aud_PES_global_data.where_in_PES == FIND_PREFIX)) {
            /* A PES Packet is complete, but there is more in the clip. */
            /* Repeat  call to handle_PES with the same clip.           */
            ;
          }
          else {
            /* Didn't manage to copy the current PES or the current clip, */
            /* This means the audio decode buffer is full.                */
            /* Wait for a low water interrupt to occur.                   */
            break;
          }
        }
      }
      else
        ; /* Play threshold has not been achieved even once since last start */
      
      break;
      
    case CNXT_AUDIO_START_PLAY:

      report_event(h, h->out, 0);
      
      break;
      
    case CNXT_AUDIO_BUF_EMPTY:
      if (h->in == h->out) {
        ks = critical_section_begin();
        *glpIntMask  &= ~(MPG_AB_LOWWATERMARK | MPG_AB_EMPTY);
        *glpIntStatus =  (MPG_AB_LOWWATERMARK | MPG_AB_EMPTY);
        ABEmptyIntEnabled    = FALSE;
        ABLowWaterIntEnabled = FALSE;
        critical_section_end(ks);
        h->state = CNXT_AUDIO_STATE_START2;
        report_event(h, h->out, EV_CLIP_STOPPED);
      }
      break;


    case CNXT_AUDIO_PTS_RECEIVED_INT:
       trace_log(PTS_RECEIVED_INT, 0, 0, 0);
       ks = critical_section_begin();
       *glpIntMask &= ~MPG_NEW_PTS_RECEIVED;   /* disable interrupt */
       *glpIntStatus = MPG_NEW_PTS_RECEIVED;
       APTSReceivedIntEnabled = FALSE;
       critical_section_end(ks);
      /* pawser has written audio pes pid stream id to memory, go read it */
      stream_id = (char) *DPS_AUDIO_PID_STREAM_ID_EX(gDemuxInstance);
      trace_new(TRACE_MPG|TRACE_LEVEL_ALWAYS, "audio pes stream_id is 0x%x\n", stream_id);

      if (stream_id == 0xBD) {                                /* AC3 audio */
         h->format = CNXT_AUDIO_FMT_AC3;
      }
      else if ((stream_id >= 0xC0) && (stream_id <= 0xDF)) {  /* mpeg audio */
         h->format = CNXT_AUDIO_FMT_MPEG;
      }
      else {  /* this is error state as cannot determine audio format */
         h->state = CNXT_AUDIO_STATE_OPENED;
      }

      if ((h->format != ucode_fmt) && (h->state != CNXT_AUDIO_STATE_OPENED)) {  
         /* need to download correct audio ucode */
         /* if ucode valid then first step is to assert halt decoding */
         if ( *( (LPREG)glpCtrl0) & MPG_AUDIO_MC_VALID) { 
            hw_aud_stop(FALSE);
         }
         useAC3ucode = FALSE;              /* Assume MPEG */
         if (h->format == CNXT_AUDIO_FMT_AC3) {
             useAC3ucode = TRUE;             /* It's AC3 instead of MPEG */
         }
         if (cnxt_download_ucode(useAC3ucode) == FALSE) { /* Download ucode */
            h->state = CNXT_AUDIO_STATE_OPENED;  /* Error downloading the microcode */
         }
         else {
           ucode_fmt = h->format;            /* Make record of new ucode format */
           hw_aud_start();                   /* deassert halt decoding */
           h->state = CNXT_AUDIO_STATE_START2;
         }
      }
      /* else correct ucode already downloaded, just update state */
      else if ( (h->format == ucode_fmt) && 
                (h->state != CNXT_AUDIO_STATE_OPENED)) {  
         h->state = CNXT_AUDIO_STATE_START2;
      }
      break;


    default:
      break;
    }
}

/*********************************************************************/
/*   FUNCTION:     report_event                                      */
/*                                                                   */
/*   PARAMETERS:   h        - handle of audio hardware instance      */
/*                 clip_idx - If 'event' parm is EV_CLIP_PLAY        */
/*                            or EV_CLIP_COPIED, clip_idx is used    */
/*                            to report the clip associated with the */
/*                            event.  If neither of these events has */
/*                            occurred, then this parm is ignored.   */
/*                 event    - audio event to report                  */
/*                                                                   */
/*   DESCRIPTION:  Reports 'event' parm to application via callback. */
/*                 Also looks to see if the uncopied queued data     */
/*                 has dropped below the request_threshold.  If so,  */
/*                 then this event is reported simultaneously with   */
/*                 the 'event' parm.                                 */
/*                                                                   */
/*   RETURNS:      None                                              */
/*                                                                   */
/*********************************************************************/
static void report_event(AudioNode *h, u_int32 clip_idx, CNXT_AUDIO_EVENT event)
{
  farcir_q       *q;
  CNXT_CLIP_EV_DATA *pevdata;
  
  if (event & (EV_CLIP_PLAY | EV_CLIP_COPIED)) {
    q       = &h->audreqs[clip_idx];
    pevdata = &h->evdata [clip_idx];      
    pevdata->hclip   = q;
    pevdata->errdata = 0;
  }
  else
    pevdata = NULL;
  
  if ((h->state >= CNXT_AUDIO_STATE_START1) && 
      (h->bytes_avail <= h->request_threshold))
    event |= EV_CLIP_FEED;
  
  if (event && (*h->pfn_callback != NULL)) {
    trace_log(CALLBACK, event, clip_idx, 0);
    (*h->pfn_callback)(h, event, pevdata, h->pinfo);
    trace_log(RETURN, 0, 0, 0);
  }
}

/*********************************************************************/
/*   FUNCTION:     cnxt_audio_open                                   */
/*                                                                   */
/*   PARAMETERS:   phaudio - points to application provided memory   */
/*                           location where handle is to be placed.  */
/*                                                                   */
/*   DESCRIPTION:  Opens 1 instance of hardware audio and returns    */
/*                 a handle for subsequent calls                     */
/*                                                                   */
/*   RETURNS:      CNXT_AUDIO_OK                                     */
/*                 CNXT_AUDIO_BAD_PARAMETER                          */
/*                 CNXT_AUDIO_ERROR                                  */
/*                                                                   */
/*********************************************************************/
CNXT_AUDIO_STATUS cnxt_audio_open( HAUDIO *phaudio )
{
  int i;
  CNXT_AUDIO_STATUS ret = CNXT_AUDIO_ERROR;

  if (phaudio == NULL)
    return CNXT_AUDIO_BAD_PARAMETER;

  /*
   * See if there are any audio instances available
   */
  for (i = 0; i < NAUDIO; i++) {
    if (anode[i].state == CNXT_AUDIO_STATE_CLOSED) {
      trace_log(OPEN_CMD, 0, 0, 0);
      anode[i].state = CNXT_AUDIO_STATE_OPENED;
      anode[i].in    = 0;
      anode[i].out   = 0;
      *phaudio = (HAUDIO)(&anode[i]);
      ucode_fmt_at_open = ucode_fmt;

      ret = CNXT_AUDIO_OK;         /* Found one */
      break;
    }
  }

  return ret;
}

/*********************************************************************/
/*   FUNCTION:     cnxt_audio_close                                  */
/*                                                                   */
/*   PARAMETERS:   haudio - handle of audio hardware instance        */
/*                          provided earlier by cnxt_audio_open.     */
/*                                                                   */
/*   DESCRIPTION:  Closes 1 instance of hardware audio.              */
/*                                                                   */
/*   RETURNS:      CNXT_AUDIO_OK                                     */
/*                 CNXT_AUDIO_BAD_PARAMETER                          */
/*                 CNXT_AUDIO_ERROR                                  */
/*                                                                   */
/*********************************************************************/
CNXT_AUDIO_STATUS cnxt_audio_close( HAUDIO haudio )
{
  bool         useAC3ucode;
  CNXT_AUDIO_STATUS ret;
  AudioNode *h = haudio;

  if (h->check != haudio) {
    ret = CNXT_AUDIO_BAD_PARAMETER;   /* Invalid Handle */
  }
  else {
    trace_log(CLOSE_CMD, 0, 0, 0);
    if (h->state >= CNXT_AUDIO_STATE_START1)
      cnxt_audio_stop(haudio);  /* Handle must be stopped before being closed */

    if (h->state == CNXT_AUDIO_STATE_OPENED) {
      if (ucode_fmt != ucode_fmt_at_open) {
        /* need to download correct audio ucode */
        /* if ucode valid then first step is to assert halt decoding */
        if ( *( (LPREG)glpCtrl0) & MPG_AUDIO_MC_VALID)
           hw_aud_stop(FALSE);
        useAC3ucode = FALSE;              /* Assume MPEG */
        if (ucode_fmt_at_open == CNXT_AUDIO_FMT_AC3)
          useAC3ucode = TRUE;             /* It's AC3 instead of MPEG */
        if (cnxt_download_ucode(useAC3ucode) == TRUE) { /* Restore ucode */
          ucode_fmt = ucode_fmt_at_open;
          hw_aud_start();                   /* deassert halt decoding */
        }
      }
      h->state = CNXT_AUDIO_STATE_CLOSED;
      ret = CNXT_AUDIO_OK;      /* Close was successful */
    }
    else
      ret = CNXT_AUDIO_ERROR;   /* Not open, so can't be closed */
  }

  return ret;
}

/*********************************************************************/
/*   FUNCTION:     cnxt_audio_get_instances                          */
/*                                                                   */
/*   PARAMETERS:   haudio - handle of audio hardware instance        */
/*                          provided earlier by cnxt_audio_open.     */
/*                                                                   */
/*   DESCRIPTION:  Closes 1 instance of hardware audio.              */
/*                                                                   */
/*   RETURNS:      CNXT_AUDIO_OK                                     */
/*                 CNXT_AUDIO_BAD_PARAMETER                          */
/*                 CNXT_AUDIO_ERROR                                  */
/*                                                                   */
/*********************************************************************/
CNXT_AUDIO_STATUS cnxt_audio_get_instances( int *pinstances )
{
  *pinstances = NAUDIO;
  return CNXT_AUDIO_OK;
}

/*********************************************************************/
/*   FUNCTION:     cnxt_audio_register_callback                      */
/*                                                                   */
/*   PARAMETERS:   haudio - handle of audio hardware instance        */
/*                          provided earlier by cnxt_audio_open.     */
/*                 pfn_callback - The callback function to use for   */
/*                          audio event notifications. The value can */
/*                          be NULL to specify that no notification  */
/*                          callback should be made.                 */
/*                 pinfo - a pointer to user defined information     */
/*                         that will be passed back as a parameter   */
/*                         to the callback function. For example,    */
/*                         This could be application defined         */
/*                         instance data.                            */
/*                                                                   */
/*   DESCRIPTION:  This functions is used to register a              */
/*                 callback function for audio events.               */
/*                 The callback functions is used to notify the      */
/*                 client about events related to playing audio.     */
/*                                                                   */
/*   RETURNS:      CNXT_AUDIO_OK                                     */
/*                 CNXT_AUDIO_ERROR                                  */
/*                                                                   */
/*********************************************************************/
CNXT_AUDIO_STATUS cnxt_audio_register_callback(HAUDIO                haudio, 
                                          CNXT_AUDIO_CALLBACK_PFN pfn_callback,
                                          void                  *pinfo)
{
  CNXT_AUDIO_STATUS ret;
  AudioNode *h = haudio;

  if (h->check != haudio) {
    ret = CNXT_AUDIO_BAD_PARAMETER;   /* Invalid Handle */
  }
  else if ((pfn_callback == NULL) || (h->pfn_callback == NULL)) {
    trace_log(REGISTER_CMD, 0, 0, 0);
    h->pfn_callback = pfn_callback;
    h->pinfo        = pinfo;
    ret = CNXT_AUDIO_OK;              /* Callback successfully registered */
  }
  else
    ret = CNXT_AUDIO_ERROR;  /* A callback was already registered */

  return ret;
}

/*********************************************************************/
/*   FUNCTION:     cnxt_audio_set_thresholds                         */
/*                                                                   */
/*   PARAMETERS:   haudio - handle of audio hardware instance        */
/*                          provided earlier by cnxt_audio_open.     */
/*                 play_threshold - The number of bytes that must be */
/*                          queued up by the application before an   */
/*                          audio clip will begin playing.           */
/*                 request_threshld - The number of buffered bytes   */
/*                          remaining before the audio driver will   */
/*                          request that the application provide     */
/*                          more data.                               */
/*                                                                   */
/*   DESCRIPTION:  Set the threshold limits to control playing of    */
/*                 encoded audio clips.                              */
/*                                                                   */
/*   RETURNS:      CNXT_AUDIO_OK                                     */
/*                 CNXT_AUDIO_BAD_PARAMETER                          */
/*                                                                   */
/*********************************************************************/
CNXT_AUDIO_STATUS cnxt_audio_set_thresholds(HAUDIO  haudio,
                                       u_int32 play_threshold,
                                       u_int32 request_threshold)
{
  CNXT_AUDIO_STATUS ret;
  AudioNode *h = haudio;

  if (h->check != haudio) {
    ret = CNXT_AUDIO_BAD_PARAMETER;   /* Invalid Handle */
  }
  else {
    trace_log(THRESHOLD_CMD, play_threshold, request_threshold, 0);
    h->play_threshold    = play_threshold;
    h->request_threshold = request_threshold;
    ret = CNXT_AUDIO_OK;              /* Thresholds set successfully */
  }

  return ret;
}

/*********************************************************************/
/*   FUNCTION:     cnxt_audio_start                                  */
/*                                                                   */
/*   PARAMETERS:   haudio - handle of audio hardware instance        */
/*                          provided earlier by cnxt_audio_open.     */
/*                 pfmt - Pointer to a structure that contains       */
/*                        information about the format of the data.  */
/*                        See the definition of CNXT_AUDIO_CONFIG    */
/*                        for the format choices.                    */
/*                                                                   */
/*   DESCRIPTION:  Starts audio playing according to the specified   */
/*                 format. The function can be used to play a live   */
/*                 stream or audio clips from memory.                */
/*                                                                   */
/*   RETURNS:      CNXT_AUDIO_OK                                     */
/*                 CNXT_AUDIO_BAD_PARAMETER                          */
/*                 CNXT_AUDIO_BAD_FORMAT                             */
/*                 CNXT_AUDIO_BUSY                                   */
/*                                                                   */
/*********************************************************************/
CNXT_AUDIO_STATUS cnxt_audio_start( HAUDIO haudio, CNXT_AUDIO_CONFIG *pfmt)
{
  CNXT_AUDIO_STATUS ret;
  u_int32 message[4];
  AudioNode *h = haudio;
  bool           ks;
  bool           useAC3ucode;
  u_int8         tmpcnt;
  
  if (h->check != haudio) {
    ret =  CNXT_AUDIO_BAD_PARAMETER;  /* Invalid Handle */
  }
  else if (h->state < CNXT_AUDIO_STATE_OPENED) {
    ret = CNXT_AUDIO_BAD_PARAMETER;   /* Handle is not open */
  }
  else if (h->state >= CNXT_AUDIO_STATE_START1) {
    ret = CNXT_AUDIO_BUSY;            /* Audio is already started */
  }
  else if ((pfmt->format < CNXT_AUDIO_FMT_AUTO) || 
           (pfmt->format > CNXT_AUDIO_FMT_AC3)) {
    ret = CNXT_AUDIO_BAD_FORMAT;      /* Unknown format */
  }
  else if ((pfmt->format != CNXT_AUDIO_FMT_AUTO) &&
           ((pfmt->encapsulation < CNXT_AUDIO_ENCAP_ES) || 
            (pfmt->encapsulation > CNXT_AUDIO_ENCAP_PES))) {
    ret = CNXT_AUDIO_BAD_FORMAT;      /* If not auto, ES or PES must be specified */
  }
  else if ((pfmt->source < CNXT_AUDIO_SOURCE_LIVE) || 
           (pfmt->source > CNXT_AUDIO_SOURCE_CLIP)) {
    ret = CNXT_AUDIO_BAD_FORMAT;      /* Unknown source */
  }
  else {
    h->format = pfmt->format;
    h->encap  = pfmt->encapsulation;
    h->source = pfmt->source;
    h->bytes_avail = 0;
    h->state  = CNXT_AUDIO_STATE_START1;
    ret = CNXT_AUDIO_OK;

    ncopy = 0;
    trace_log(START_CMD, 0, 0, 0);

    if (pfmt->source == CNXT_AUDIO_SOURCE_LIVE) {
      if (pfmt->format == CNXT_AUDIO_FMT_AUTO) {
         /* enable audio pid to produce audio pts interrupt and make choice about a/v sync */
         /* audio pts interrupt is used to indicate info regarding audio format can be read */
         gen_audio_play(pfmt->pcrsync);
         ks = critical_section_begin();
         *glpIntStatus = MPG_NEW_PTS_RECEIVED;
         *glpIntMask |= MPG_NEW_PTS_RECEIVED;   /* enable interrupt */
         APTSReceivedIntEnabled = TRUE;
         critical_section_end(ks);
         tmpcnt = 0;
         /* now that interrupt has been enabled the following state variable */
         /* should eventually be changed by audio_process_msg of             */
         /* CNXT_AUDIO_PTS_RECEIVED interrupt, else timeout and error        */
         while (h->state == CNXT_AUDIO_STATE_START1) {
           task_time_sleep(10);
           tmpcnt++;
           if (tmpcnt > 30) {
              trace_new(TRACE_MPG|TRACE_LEVEL_ALWAYS, "cnxt_audio_start live stream auto_fmt_detect timeout\n");
              ret = CNXT_AUDIO_FMT_UNKNOWN;
              ks = critical_section_begin();
              *glpIntMask &= ~MPG_NEW_PTS_RECEIVED;   /* disable interrupt */
              *glpIntStatus = MPG_NEW_PTS_RECEIVED;
              APTSReceivedIntEnabled = FALSE;
              critical_section_end(ks);
              h->state = CNXT_AUDIO_STATE_OPENED;
           }
         }
      }
      /* source == LIVE and format != AUTO, verify correct audio ucode */
      else if (h->format != ucode_fmt) {  
         /* need to download correct audio ucode */
         /* if ucode is currently valid then first step is to assert halt decoding */
         if ( *( (LPREG)glpCtrl0) & MPG_AUDIO_MC_VALID) { 
            hw_aud_stop(FALSE);
         }
         useAC3ucode = FALSE;              /* Assume MPEG */
         if (h->format == CNXT_AUDIO_FMT_AC3) {
             useAC3ucode = TRUE;             /* It's AC3 instead of MPEG */
         }
         if (cnxt_download_ucode(useAC3ucode) == FALSE) { /* Download ucode */
            hw_aud_start();                 /* deassert halt decoding but don't start */
            h->state = CNXT_AUDIO_STATE_OPENED;  /* Error downloading the microcode */
         }
         else {
           ucode_fmt = h->format;            /* Make record of new ucode format */
           gen_audio_play(pfmt->pcrsync);   /* deassert halt decoding and start audio */
           h->state = CNXT_AUDIO_STATE_START2;
         }
      }
      /* else correct ucode already downloaded, start audio and update state */
      else {  /* (h->format == ucode_fmt) */
         gen_audio_play(pfmt->pcrsync);
         h->state = CNXT_AUDIO_STATE_START2;
      }
    }
    else {  /* source == CNXT_AUDIO_SOURCE_CLIP */
       message[0] = (u_int32)haudio;         /* Signal the audio process           */
       message[1] = CNXT_AUDIO_START_PLAY;
       qu_send(AudioQueueID, message);     
    }
  }
  return ret;
}

/*********************************************************************/
/*   FUNCTION:     cnxt_audio_stop                                   */
/*                                                                   */
/*   PARAMETERS:   haudio - handle of audio hardware instance        */
/*                          provided earlier by cnxt_audio_open.     */
/*                                                                   */
/*   DESCRIPTION:  Stops playing audio. All internal states and      */
/*                 buffers are cleared.                              */
/*                                                                   */
/*   RETURNS:      CNXT_AUDIO_OK                                     */
/*                 CNXT_AUDIO_ERROR                                  */
/*                                                                   */
/*********************************************************************/
CNXT_AUDIO_STATUS cnxt_audio_stop( HAUDIO haudio )
{
  bool         ks;
  CNXT_AUDIO_STATUS ret;
  u_int32      message[4];
  AudioNode   *h = haudio;
  
  if (h->check != haudio) {
    ret = CNXT_AUDIO_BAD_PARAMETER;   /* Invalid Handle */
  }
  else if (h->state < CNXT_AUDIO_STATE_START1) {
    ret = CNXT_AUDIO_ERROR;           /* Audio not yet started */
  }
  else if (h->source == CNXT_AUDIO_SOURCE_LIVE) {
    ks = critical_section_begin();
    *glpIntMask &= ~MPG_NEW_PTS_RECEIVED;   /* disable interrupt */
    *glpIntStatus = MPG_NEW_PTS_RECEIVED;
    APTSReceivedIntEnabled = FALSE;
    critical_section_end(ks);
    hw_set_aud_mute(TRUE);
    gen_audio_stop();
    /* Wait for valid and clean audio before unmuting */
    task_time_sleep(100);
    hw_set_aud_mute(FALSE);

    trace_log(STOP_CMD, 0, 0, 0);

    /* Empty out the message queue */
    while (qu_receive(AudioQueueID, FALSE, message) == RC_OK)
      ;

    h->state = CNXT_AUDIO_STATE_OPENED;
    trace_dump();
    ret = CNXT_AUDIO_OK;              /* Audio successfully stopped */
  }
  else {
    ks = critical_section_begin();
    h->in  = 0;
    h->out = 0;
    *glpIntMask  &= ~(MPG_AB_LOWWATERMARK | MPG_AB_EMPTY);
    *glpIntStatus =  (MPG_AB_LOWWATERMARK | MPG_AB_EMPTY);
    ABEmptyIntEnabled    = FALSE;
    ABLowWaterIntEnabled = FALSE;
    critical_section_end(ks);

    trace_log(STOP_CMD, 0, 0, 0);

    /* Empty out the message queue */
    while (qu_receive(AudioQueueID, FALSE, message) == RC_OK)
      ;

    cnxt_audio_resume(h);
    wait_resume_complete();

    hw_set_aud_mute(TRUE);
    gen_audio_stop();
    /* Wait for valid and clean audio before unmuting */
    if (h->state >= CNXT_AUDIO_STATE_START2)
      task_time_sleep(100);
    hw_set_aud_mute(FALSE);
    feed_active = FALSE;

    h->state = CNXT_AUDIO_STATE_OPENED;
    trace_dump();
    ret = CNXT_AUDIO_OK;              /* Audio successfully stopped */
  }

  return ret;
}

/*********************************************************************/
/*   FUNCTION:     cnxt_audio_pause                                  */
/*                                                                   */
/*   PARAMETERS:   haudio - handle of audio hardware instance        */
/*                          provided earlier by cnxt_audio_open.     */
/*                                                                   */
/*   DESCRIPTION:  Pause playing of clip                             */
/*                                                                   */
/*   RETURNS:      CNXT_AUDIO_OK                                     */
/*                 CNXT_AUDIO_ERROR                                  */
/*                                                                   */
/*********************************************************************/
CNXT_AUDIO_STATUS cnxt_audio_pause( HAUDIO haudio )
{
  u_int32   pause_time;
  AudioNode *h = haudio;

  if (h->source == CNXT_AUDIO_SOURCE_LIVE) 
    return CNXT_AUDIO_ERROR;    /* Live audio cannot be paused */
  else if (h->state <  CNXT_AUDIO_STATE_START1)
    return CNXT_AUDIO_ERROR;    /* Audio not yet started */
  else {
    trace_log(PAUSE_CMD, 0, 0, 0);
    resume_active = FALSE;
    pause_time = (h->state==CNXT_AUDIO_STATE_START1) ? 0 : MIN_PAUSE_TIME;
    volume_ramp_target_tick = get_system_time() + pause_time;
#if (CANAL_AUDIO_HANDLING==YES)
    audio_ramp_vol_down(0);
#endif
    hw_aud_stop(FALSE);
    return CNXT_AUDIO_OK;
  }
}

/*********************************************************************/
/*   FUNCTION:     cnxt_audio_resume                                 */
/*                                                                   */
/*   PARAMETERS:   haudio - handle of audio hardware instance        */
/*                          provided earlier by cnxt_audio_open.     */
/*                                                                   */
/*   DESCRIPTION:  Resume playing of clip                            */
/*                                                                   */
/*   RETURNS:      CNXT_AUDIO_OK                                     */
/*                 CNXT_AUDIO_ERROR                                  */
/*                                                                   */
/*********************************************************************/
CNXT_AUDIO_STATUS cnxt_audio_resume( HAUDIO haudio )
{
  int        delay;
  AudioNode *h = haudio;

  if (h->source == CNXT_AUDIO_SOURCE_LIVE) 
    return CNXT_AUDIO_ERROR;    /* Live audio cannot be resumed */
  else if (h->state <  CNXT_AUDIO_STATE_START1)
    return CNXT_AUDIO_ERROR;    /* Audio not yet started */
  else {
    trace_log(RESUME_CMD, 0, 0, 0);
    if (!resume_active) {
      delay = volume_ramp_target_tick - get_system_time();
      if (delay > 0) {
        resume_active = TRUE;
        tick_set(ResumeAudioTick, delay, TRUE); /* One Shot */
        tick_start(ResumeAudioTick);
      }
      else 
        resume_now();
    }
    return CNXT_AUDIO_OK;
  }
}

static void wait_resume_complete(void) {
  while (resume_active) {
    task_time_sleep(100);
  }
}

static void ResumeTickCallback(tick_id_t tickTimer, void *pUserData) {
  if (resume_active)
    resume_now();
}

static void resume_now() {
    hw_aud_start();
#if (CANAL_AUDIO_HANDLING==YES)
    audio_ramp_vol_up();
#endif
    trace_log(RESUME_CMD_FINISHED, 0, 0, 0);
    resume_active = FALSE;
}
  
/*********************************************************************/
/*   FUNCTION:     cnxt_audio_write_data_async                       */
/*                                                                   */
/*   PARAMETERS:   haudio - handle of audio hardware instance        */
/*                          provided earlier by cnxt_audio_open.     */
/*                 pbuf - Pointer to the start of the buffer         */
/*                        containing the encoded audio data. Note    */
/*                        that the client should preserve this buffer*/
/*                        until notified by the callback function    */
/*                        that the data has been completely copied.  */
/*                 length - The size of the data in the buffer,      */
/*                          in bytes.                                */
/*                 phclip - A handle to the audio clip used by the   */
/*                          driver for notification purposes. This   */
/*                          value is filled in by the function and   */
/*                          returned to the caller. This handle      */
/*                          value will be used in the callback       */
/*                          function to associate events with this   */
/*                          clip.                                    */
/*                                                                   */
/*   DESCRIPTION:  This function is used to pass encoded audio data  */
/*                 to the audio driver when playing audio clips from */
/*                 memory.                                           */
/*                                                                   */
/*   RETURNS:      CNXT_AUDIO_OK                                     */
/*                 CNXT_AUDIO_BAD_PARAMETER                          */
/*                 CNXT_AUDIO_BUSY                                   */
/*                 CNXT_AUDIO_CLIP_FULL                              */
/*                 CNXT_AUDIO_BAD_DATA                               */
/*                                                                   */
/*********************************************************************/
CNXT_AUDIO_STATUS cnxt_audio_write_data_async(HAUDIO      haudio,
                                         void      *pbuf,
                                         u_int32   length,
                                         HCLIP      *phclip )
{
  bool useAC3ucode;
  CNXT_AUDIO_STATUS ret;
  CNXT_AUDIO_FORMAT format;
  CNXT_AUDIO_ENCAPSULATION encap;
  u_int32    message[4];
  farcir_q   *q;
  bool fmt_detected;
  AudioNode  *h = haudio;
  u_int32    pes_offset = 0;
  
  if (h->check != haudio) {
    ret = CNXT_AUDIO_BAD_PARAMETER;   /* Invalid Handle */
  }
  else if (length == 0) {
    ret = CNXT_AUDIO_BAD_PARAMETER;   /* Zero length buffer */
  }
  else if (h->state < CNXT_AUDIO_STATE_START1) {
    ret = CNXT_AUDIO_BUSY;            /* Audio was never started */
  }
  else if (((h->in + 1) & (AUDREQ_ARRAY_SIZE - 1)) == h->out) {
    ret = CNXT_AUDIO_CLIP_FULL;     /* The Audio Queue is full */
  }
  else {
    /*
     *  Determine audio format and make sure proper ucode is downloaded 
     */
    if (h->state == CNXT_AUDIO_STATE_START1) {
      /* This is the first data buffer after 'cnxt_audio_start' was issued */
      fmt_detected = extract_audio_format(pbuf, length, &format, &encap, NULL, NULL, 
                                          NULL, NULL, NULL, NULL, NULL, &pes_offset);
      if (h->format == CNXT_AUDIO_FMT_AUTO) {
         if (fmt_detected) {                  /* If auto format detection desired   */
            h->format = format;               /* and valid format was detected, then*/
         }                                    /* save the detected format type      */
         else {
            return CNXT_AUDIO_BAD_DATA;            /* Could not auto-detect the audio format */
         }
      }

      if (h->encap == CNXT_AUDIO_ENCAP_AUTO) {/* If auto encap detection desired    */
         if (fmt_detected) {                  /* and valid encap was detected, then */
            h->encap  = encap;                /* save the detected encap type       */
         }
         else {
            return CNXT_AUDIO_BAD_DATA;       /* Could not auto-detect the audio format */
         }
      }

      if (h->format == ucode_fmt) {
        ;                                 /* Correct ucode is already loaded    */
      }
      else {
        /* need to download correct audio ucode */
        /* if current ucode valid then first step is to assert halt decoding */
        if ( *( (LPREG)glpCtrl0) & MPG_AUDIO_MC_VALID) { 
           hw_aud_stop(FALSE);
        }
        useAC3ucode = FALSE;              /* Assume MPEG */
        if (h->format == CNXT_AUDIO_FMT_AC3)
          useAC3ucode = TRUE;             /* It's AC3 instead of MPEG */
        if (cnxt_download_ucode(useAC3ucode) == FALSE) { /* Download ucode     */
          return CNXT_AUDIO_ERROR;        /* Error downloading the microcode.   */
        }
        else {
          ucode_fmt = h->format;            /* Make record of new ucode format    */
          hw_aud_start();                   /* deassert halt decoding */
        }
      }
      h->state = CNXT_AUDIO_STATE_START2; /* proper ucode is downloaded */
    }
    
    /* Adjust for start of pes header, if any */
    if (h->encap == CNXT_AUDIO_ENCAP_PES) {
       pbuf   = (void *)((u_int32)pbuf + pes_offset);
       length -= pes_offset;
    }

    /* Here h->state is CNXT_AUDIO_STATE_START2 or CNXT_AUDIO_STATE_PLAYING */
    /* Queue the new audio request        */
    q = &h->audreqs[h->in];
    *phclip  = (HCLIP)q;
    q->begin = pbuf;
    q->size  = length + 1;
    q->in    = length;
    q->out   = 0;
    h->bytes_avail += length;
    trace_log(DATA_CMD, h->in, (u_int32)pbuf, length);
    h->in = (h->in + 1) & (AUDREQ_ARRAY_SIZE - 1);
    
    ret = CNXT_AUDIO_OK;
    if (!feed_active) {
      feed_active = TRUE;
      message[0] = (u_int32)haudio;         /* Signal the audio process           */
      message[1] = CNXT_AUDIO_FEED_DATA;
      if (qu_send(AudioQueueID, message) != RC_OK) {
        trace_log(ERRORCODE, 2, 0, 0);
        ret = CNXT_AUDIO_ERROR;
      }
    }
  }

  return ret;
}

/*********************************************************************/
/*   FUNCTION:     cnxt_audio_get_info                               */
/*                                                                   */
/*   PARAMETERS:   haudio - handle of audio hardware instance        */
/*                          provided by cnxt_audio_open.             */
/*                 pinfo - pointer to caller supplied storage for    */
/*                         the audio stream information              */
/*                 pbuf - pointer to the start of the buffer         */
/*                        containing the encoded audio data, or      */
/*                        NULL to get info about a live stream       */
/*                 length - the size, in bytes, of the data in the   */
/*                          buffer or 0 if pbuf is not used          */
/*                                                                   */
/*   DESCRIPTION:  Function is used to get information from the audio*/
/*                 decoder about the currently playing stream or clip*/
/*                 or encoded data in a buffer. The information is   */
/*                 returned in a caller supplied structure. See the  */
/*                 definition for CNXT_AUDIO_INFO for the details of */
/*                 the structure and the data it contains.           */
/*                                                                   */
/*   RETURNS:      CNXT_AUDIO_OK           valid data returned       */
/*                 CNXT_AUDIO_NOT_VALID    data invalid, structure   */
/*                                         cleared                   */
/*                 CNXT_AUDIO_ERROR        invalid handle OR one of  */
/*                                         pbuf or length is NULL    */
/*                 CNXT_AUDIO_FMT_UNKNOWN  no header found in buffer */
/*                                                                   */
/*   CONTEXT:      can be called from any context                    */
/*********************************************************************/
CNXT_AUDIO_STATUS cnxt_audio_get_info( HAUDIO haudio, 
              CNXT_AUDIO_INFO    *pinfo,
              void          *pbuf,
              u_int32       length)
{
  LPREG lpAudioStreamInfo = (LPREG) (AUD_INFO_REG | NCR_BASE);
  CNXT_AUDIO_FORMAT format;
  CNXT_AUDIO_ENCAPSULATION encap;
  CNXT_AUDIO_CHAN_MODE chanmode;
  CNXT_AUDIO_MPEG_LAYER layer;
  CHAN_FLAG avail_chans;
  unsigned int bitrate, sampfreq, halffreq, totalchans;
  AudioNode *h = haudio;
  bool kstate;
  u_int32 pes_offset;
  static u_int32 AudioStreamInfo[2];
  static u_int32 uiLastTime = 0;
  static u_int32 uiMinTimeLimit = AUDIO_INFO_MIN_TIME; /* may be dynamic later */
  static bool bInitialized = FALSE;
   
  /* clear the structure passed to us*/
  memset(pinfo,0,sizeof(CNXT_AUDIO_INFO));
  
  if (h->check != haudio)
    {
      return CNXT_AUDIO_ERROR;  /* Invalid Handle */
    }
   
  if ((pbuf != NULL) | (length != 0))
    {
      if ((pbuf == NULL) | (length == 0)) return CNXT_AUDIO_ERROR;
      if (extract_audio_format(pbuf, length, &format, &encap, &chanmode, &layer, &avail_chans,
                               &bitrate, &sampfreq, &halffreq, &totalchans, &pes_offset) == FALSE)
      {
        return CNXT_AUDIO_FMT_UNKNOWN;  /* Unknown audio format */
      }
      /* if return valid data then populate the structure with info */
      pinfo->playing = FALSE;
      pinfo->encap = encap;
      pinfo->encfmt = format;
      if (format == CNXT_AUDIO_FMT_MPEG)
      {
        pinfo->fmt.mpeg.layer = layer;
        pinfo->fmt.mpeg.bitrate = mpeg_kbps[pinfo->fmt.mpeg.layer][bitrate];
        pinfo->fmt.mpeg.sampfreq = mpeg_sampfreq[sampfreq];
        pinfo->fmt.mpeg.halffreq = !halffreq;
        pinfo->fmt.mpeg.chanmode = chanmode;
        pinfo->fmt.mpeg.avail_chans = avail_chans;
        pinfo->fmt.mpeg.totalchans = totalchans;
      }
      else /* (encap == CNXT_AUDIO_FMT_AC3) */
      {
        pinfo->fmt.ac3.bitrate = ac3_kbps[bitrate];
        pinfo->fmt.ac3.sampfreq = ac3_sampfreq[sampfreq];
        pinfo->fmt.ac3.chanmode = chanmode;
        pinfo->fmt.ac3.avail_chans = avail_chans;
        pinfo->fmt.ac3.totalchans = totalchans;
      }
      return CNXT_AUDIO_OK;
    }

  /* 
     if the microcode has not had enough time to update the value 
     since the last time, then don't update the info structure.
  */
  if ((!bInitialized) || (get_system_time() - uiLastTime) >= uiMinTimeLimit)
    {    
      /* Enough time has elapsed.
         Read the current information, update the timer, 
         and clear the current information.
      */
      kstate = critical_section_begin();
      AudioStreamInfo[0] = lpAudioStreamInfo[0];
      AudioStreamInfo[1] = lpAudioStreamInfo[1];
      /* clear valid flag within memory */
      *lpAudioStreamInfo = 0;
      uiLastTime = get_system_time();
      critical_section_end(kstate);
      bInitialized = TRUE;
    }

  /* is stream/info valid ??? */
  if ( !(AudioStreamInfo[0] & AUD_INFO_VALID_MASK))
    {
      /* audio header info is not valid */
      return CNXT_AUDIO_NOT_VALID;
    }
  else 
    {  /* info is valid */
      pinfo->playing = TRUE;
      pinfo->encap = CNXT_AUDIO_ENCAP_PES;  /* default for transport stream */

      if (((AudioStreamInfo[0] & AUD_INFO_FMT_MASK) >> AUD_INFO_FMT_SHFT) == AUD_INFO_FMT_AC3)
      {   /* AC3 and valid */
        pinfo->encfmt = CNXT_AUDIO_FMT_AC3;
 
        pinfo->fmt.ac3.sampfreq = ac3_sampfreq[((AudioStreamInfo[0] &
                                   AC3_HEADER_FSCOD_MASK) >> 
                                   AC3_HEADER_FSCOD_SHFT)];
  
        pinfo->fmt.ac3.bitrate = ac3_kbps[((AudioStreamInfo[0] &
                                  AC3_HEADER_FRMSZCOD_MASK) >> 
                                  AC3_HEADER_FRMSZCOD_SHFT)];
  
        pinfo->fmt.ac3.bsmod = ((AudioStreamInfo[0] & AC3_HEADER_BSMOD_MASK)
                                  >> AC3_HEADER_BSMOD_SHFT);
  
        pinfo->fmt.ac3.acmod = (AudioStreamInfo[0] & AC3_HEADER_ACMOD_MASK)
                                 >> AC3_HEADER_ACMOD_SHFT;

        pinfo->fmt.ac3.bsid = (AudioStreamInfo[0] & AC3_HEADER_BSID_MASK)
                                >> AC3_HEADER_BSID_SHFT;

        pinfo->fmt.ac3.copyright = (AudioStreamInfo[1] & AC3_HEADER_CPYRHT_MASK)
                                     >> AC3_HEADER_CPYRHT_SHFT;

        switch((AudioStreamInfo[0] & AC3_HEADER_ACMOD_MASK) >> AC3_HEADER_ACMOD_SHFT)
        {
          case ACMOD_DUAL_MONO:
            pinfo->fmt.ac3.totalchans = 2;
            pinfo->fmt.ac3.chanmode = CHAN_MODE_DUAL_MONO;
            pinfo->fmt.ac3.avail_chans = 
              CHAN_FLAG_FRONT_LEFT |
              CHAN_FLAG_FRONT_RIGHT;
            break;
          case ACMOD_1_0:
            pinfo->fmt.ac3.totalchans = 1;
            pinfo->fmt.ac3.chanmode = CHAN_MODE_MONO;
            pinfo->fmt.ac3.avail_chans = 
              CHAN_FLAG_FRONT_CENTER;
            break;
          case ACMOD_2_0:
            pinfo->fmt.ac3.totalchans = 2;
            pinfo->fmt.ac3.chanmode = CHAN_MODE_STEREO;
            pinfo->fmt.ac3.avail_chans = 
              CHAN_FLAG_FRONT_LEFT |
              CHAN_FLAG_FRONT_RIGHT;
            break;
          case ACMOD_3_0:
            pinfo->fmt.ac3.totalchans = 3;
            pinfo->fmt.ac3.chanmode = CHAN_MODE_MULTI;
            pinfo->fmt.ac3.avail_chans = 
              CHAN_FLAG_FRONT_LEFT |
              CHAN_FLAG_FRONT_CENTER |
              CHAN_FLAG_FRONT_RIGHT;
            break;
          case ACMOD_2_1:
            pinfo->fmt.ac3.totalchans = 3;
            pinfo->fmt.ac3.chanmode = CHAN_MODE_MULTI;
            pinfo->fmt.ac3.avail_chans = 
              CHAN_FLAG_FRONT_LEFT |
              CHAN_FLAG_FRONT_RIGHT |
              CHAN_FLAG_REAR_LEFT;
            break;
          case ACMOD_3_1:
            pinfo->fmt.ac3.totalchans = 4;
            pinfo->fmt.ac3.chanmode = CHAN_MODE_MULTI;
            pinfo->fmt.ac3.avail_chans = 
              CHAN_FLAG_FRONT_LEFT |
              CHAN_FLAG_FRONT_CENTER |
              CHAN_FLAG_FRONT_RIGHT |
              CHAN_FLAG_REAR_LEFT;
            break;
          case ACMOD_2_2:
            pinfo->fmt.ac3.totalchans = 4;
            pinfo->fmt.ac3.chanmode = CHAN_MODE_MULTI;
            pinfo->fmt.ac3.avail_chans = 
              CHAN_FLAG_FRONT_LEFT |
              CHAN_FLAG_FRONT_RIGHT |
              CHAN_FLAG_REAR_LEFT |
              CHAN_FLAG_REAR_RIGHT;
            break;
          case ACMOD_3_2:
            pinfo->fmt.ac3.totalchans = 5;
            pinfo->fmt.ac3.chanmode = CHAN_MODE_MULTI;
            pinfo->fmt.ac3.avail_chans = 
              CHAN_FLAG_FRONT_LEFT |
              CHAN_FLAG_FRONT_CENTER |
              CHAN_FLAG_FRONT_RIGHT |
              CHAN_FLAG_REAR_LEFT |
              CHAN_FLAG_REAR_RIGHT;
            break;
          default:
            break;
        }

       /* account for lfe channel within totalchans and avail_chans */
       /* dja commented out lfe code as this channel is not full bandwidth and */ 
       /* is ignored during downmix */
       /*
       if((AudioStreamInfo[1] & AC3_HEADER_LFEON_MASK) >> AC3_HEADER_LFEON_SHFT)
         { 
           pinfo->fmt.ac3.totalchans++;
           pinfo->fmt.ac3.avail_chans += CHAN_FLAG_LFE;
         }
       */
      }  /* end ac3 */

      else if (((AudioStreamInfo[0] & AUD_INFO_FMT_MASK) >> AUD_INFO_FMT_SHFT) == AUD_INFO_FMT_MPEG)
      { /* MPEG and valid */
        pinfo->encfmt = CNXT_AUDIO_FMT_MPEG;

        pinfo->fmt.mpeg.layer = (CNXT_AUDIO_MPEG_LAYER) ((AudioStreamInfo[0] & MPG_HEADER_LAYER_MASK)
                                  >> MPG_HEADER_LAYER_SHFT);
   
        pinfo->fmt.mpeg.sampfreq = mpeg_sampfreq[((AudioStreamInfo[0] &
                                    MPG_HEADER_FREQ_MASK) >> 
                                    MPG_HEADER_FREQ_SHFT)];

        pinfo->fmt.mpeg.halffreq = (!((AudioStreamInfo[0] & MPG_HEADER_ID_MASK)
                                        >> MPG_HEADER_ID_SHFT));

        pinfo->fmt.mpeg.bitrate = mpeg_kbps[pinfo->fmt.mpeg.layer][((AudioStreamInfo[0] &
                                   MPG_HEADER_BITRATE_MASK) >> 
                                   MPG_HEADER_BITRATE_SHFT)];

        pinfo->fmt.mpeg.chanmode = (CNXT_AUDIO_CHAN_MODE) ((AudioStreamInfo[0] & MPG_HEADER_MODE_MASK)
                                     >> MPG_HEADER_MODE_SHFT);

        pinfo->fmt.mpeg.totalchans = (((AudioStreamInfo[0] & MPG_HEADER_MODE_MASK)
                                         >> MPG_HEADER_MODE_SHFT) == 3) ? 1 : 2;

        pinfo->fmt.mpeg.avail_chans = CHAN_FLAG_FRONT_LEFT | 
                                      ((pinfo->fmt.mpeg.totalchans == 2) ? 
                                        CHAN_FLAG_FRONT_RIGHT : 0);

        pinfo->fmt.mpeg.emphasis = (CNXT_AUDIO_EMPH) ((AudioStreamInfo[0] & 
                                     MPG_HEADER_EMPH_MASK)
                                     >> MPG_HEADER_EMPH_SHFT);

        pinfo->fmt.mpeg.copyright = (AudioStreamInfo[0] & MPG_HEADER_CPYRHT_MASK)
                                      >> MPG_HEADER_CPYRHT_SHFT;
   
        pinfo->fmt.mpeg.modex = (CNXT_AUDIO_CHAN_MODEX) ((AudioStreamInfo[0] & 
                                  MPG_HEADER_MODEX_MASK)
                                  >> MPG_HEADER_MODEX_SHFT);

        pinfo->fmt.mpeg.padding = (AudioStreamInfo[0] & MPG_HEADER_PAD_MASK)
                                    >> MPG_HEADER_PAD_SHFT;

        pinfo->fmt.mpeg.prot = (AudioStreamInfo[0] & MPG_HEADER_PROT_MASK)
                                 >> MPG_HEADER_PROT_SHFT;
      }  /* end MPEG */
  
    }  /* end valid */

  /* return ok */
  return CNXT_AUDIO_OK;
}

/*********************************************************************/
/*  FUNCTION:     cnxt_audio_select_channels                         */
/*                                                                   */
/*  PARAMETERS:   haudio - handle of audio hardware instance         */
/*                          provided by cnxt_audio_open.             */
/*                chan - channel selection for audio output          */
/*                prev_chan - pointer to storage for previous channel*/
/*                                                                   */
/*  DESCRIPTION:  Select the channel for audio playing. This function*/
/*                is used to select a 'normal' playing mode, select  */
/*                one channel to play as mono, or to select special  */
/*                AC3 channel configurations.                        */
/*                                                                   */
/*  RETURNS:      CNXT_AUDIO_OK             specified channel is     */
/*                                          selected                 */
/*                CNXT_AUDIO_BAD_PARAMETER  specified channel is     */
/*                                          invalid                  */
/*                                                                   */
/*  CONTEXT:      can be called from any context                     */
/*********************************************************************/
CNXT_AUDIO_STATUS cnxt_audio_select_channels( HAUDIO         haudio,
                CNXT_AUDIO_CHAN   chan,
                CNXT_AUDIO_CHAN   *prevchan )
{
  AudioNode *h = haudio;
  static CNXT_AUDIO_CHAN prev = CNXT_AUDIO_CHAN_DEFAULT;
  /* set default values */
  volatile u_int8 MPG_CONTROL1_ENABLEDOWNMIX = FALSE;
  volatile u_int8 MPG_CONTROL1_LTRTMODE = FALSE;
  volatile u_int8 MPG_CONTROL1_LROUTPUTCNTRL = MPG_LRCONTROL_NORMAL;
  volatile u_int8 MPG_CONTROL1_SPDIFCNTRL = MPG_SPDIFCONTROL_LR;

  if (h->check != haudio)
    {
      *prevchan = prev;
      prev = chan;
      return CNXT_AUDIO_BAD_PARAMETER;   /* Invalid Handle */
    }
  
  /* now change defaults when appropriate */
  switch (chan)
  {
    /* all downmix selections are ignored for mpeg decode */
    /* thus default can assert lt/rt downmix for ac3 */
    case CNXT_AUDIO_CHAN_DEFAULT:
    case CNXT_AUDIO_CHAN_LT_RT:
      MPG_CONTROL1_ENABLEDOWNMIX = TRUE;
      MPG_CONTROL1_LTRTMODE = TRUE;
      break;
    /* this case is 2-ch w/o downmixing */
    /* same as above for mpeg */
    case CNXT_AUDIO_CHAN_FRONT_LEFT_RIGHT:
      break;
    case CNXT_AUDIO_CHAN_LO_RO:
      MPG_CONTROL1_ENABLEDOWNMIX = TRUE;
      break;
    case CNXT_AUDIO_CHAN_FRONT_LEFT:
      MPG_CONTROL1_LROUTPUTCNTRL = MPG_LRCONTROL_LREPEAT;
      break;
    case CNXT_AUDIO_CHAN_FRONT_RIGHT:
      MPG_CONTROL1_LROUTPUTCNTRL = MPG_LRCONTROL_RREPEAT;
      break;
    case CNXT_AUDIO_CHAN_FRONT_CENTER:
      MPG_CONTROL1_LROUTPUTCNTRL = MPG_LRCONTROL_LREPEAT;
      MPG_CONTROL1_SPDIFCNTRL = MPG_SPDIFCONTROL_CLFE;
      break;
    case CNXT_AUDIO_CHAN_REAR_LEFT:
      MPG_CONTROL1_LROUTPUTCNTRL = MPG_LRCONTROL_LREPEAT;
      MPG_CONTROL1_SPDIFCNTRL = MPG_SPDIFCONTROL_SLSR;
      break;
    case CNXT_AUDIO_CHAN_REAR_RIGHT:
      MPG_CONTROL1_LROUTPUTCNTRL = MPG_LRCONTROL_RREPEAT;
      MPG_CONTROL1_SPDIFCNTRL = MPG_SPDIFCONTROL_SLSR;
      break;
    case CNXT_AUDIO_CHAN_MIXED_MONO:
      MPG_CONTROL1_LROUTPUTCNTRL = MPG_LRCONTROL_MIXMONO;
      break;
    default:
      *prevchan = prev;
      prev = chan;
      return CNXT_AUDIO_BAD_PARAMETER;   /* Invalid channel selection */
      break;
  }
     
  CNXT_SET_VAL(glpCtrl1, MPG_CONTROL1_ENABLEDOWNMIX_MASK, MPG_CONTROL1_ENABLEDOWNMIX);
  CNXT_SET_VAL(glpCtrl1, MPG_CONTROL1_LTRTMODE_MASK, MPG_CONTROL1_LTRTMODE);
  CNXT_SET_VAL(glpCtrl1, MPG_CONTROL1_LROUTPUTCNTRL_MASK, MPG_CONTROL1_LROUTPUTCNTRL);
  CNXT_SET_VAL(glpCtrl1, MPG_CONTROL1_SPDIFCNTRL_MASK, MPG_CONTROL1_SPDIFCNTRL);
   
  *prevchan = prev;
  prev = chan;
  return CNXT_AUDIO_OK;
}

/*******************************************************************/
/* Function:   extract_audio_format                                */
/* Parameters: u_int32 *message                                    */
/* Return:     FALSE = unknown format                              */
/*             TRUE  =  known format                               */
/* Remarks:    Processes audio queue messages.                     */
/*******************************************************************/
static bool extract_audio_format(void                *pbuf,
             u_int32             length,
             CNXT_AUDIO_FORMAT        *fmt,
             CNXT_AUDIO_ENCAPSULATION *encap,
             CNXT_AUDIO_CHAN_MODE     *chanmode,
             CNXT_AUDIO_MPEG_LAYER    *layer,
             CHAN_FLAG           *avail_chans,
             unsigned int        *bitrate,
             unsigned int        *sampfreq,
             unsigned int        *halffreq,
             unsigned int        *totalchans,
             u_int32             *pes_offset)
{
   u_int32   i;
   u_int8    *c = (u_int8 *)pbuf;
   bool      det_pes = FALSE;
   bool      det_es  = FALSE;
   
   *pes_offset = 0;
   if (length < 8) return FALSE;
   for (i = 0; i < (length - 7); i++, c++) {
      
      if (det_es && (i > MAX_PES_LOOKAHEAD))
         break;
      
      if ((!det_pes) && (c[0] == 0)) 
      {
         u_int8  bits = c[6] & 0xF0;
         u_int32 len  = c[4];
         len  = (len << 8) + c[5];

         if ((c[1] == 0) && (c[2] == 1) && 
             ((bits == 0x80) || (bits == 0x90)) &&
             (len < MAX_PES_LOOKAHEAD))
         {                                                   /* PES packet      */
            if (c[3] == 0xBD)                                /* PES AC3 packet  */
            {
               *pes_offset = i;
               det_pes = TRUE;
               if (det_es) {
                  if (*fmt != CNXT_AUDIO_FMT_AC3)
                     return FALSE;
                  else
                     break;
               }
               else
                  *fmt   = CNXT_AUDIO_FMT_AC3;
            }
            else if ((c[3] >= 0xC0) && (c[3] <= 0xDF))       /* PES mpeg audio packet */
            {
               *pes_offset = i;
               det_pes = TRUE;
               if (det_es) {
                  if (*fmt != CNXT_AUDIO_FMT_MPEG)
                     return FALSE;
                  else
                     break;
               }
               else
                  *fmt   = CNXT_AUDIO_FMT_MPEG;
            }
         }
         continue;
      }
      
      /* 
         validate MPEG audio elementary using following parameters:
         mpeg syncword: 0xfff
         mpeg layer != 0x0 (reserved)
         mpeg bitrate != 0xf (reserved)
         mpeg sampling_frequency != 0x3 (reserved)
         mpeg emphasis != 0x2 (reserved)
      */
      if ((!det_es) && (c[0] == 0xFF) && ((c[1] & 0xF0) == 0xF0)) 
      {
         #define JSTEREO              1
         #define _44KHZ               0
         #define RESERVED_SAMPLE_RATE 0x0C

         u_int8 layer_     = c[1] & 0x06;
         u_int8 bitrate_   = c[2] & 0xF0;
         u_int8 samp_rate_ = c[2] & 0x0C;
         u_int8 pad_       = c[2] & 0x02;
         u_int8 mode_      = c[3] & 0xC0;
         u_int8 mode_ext_  = c[3] & 0x30;
         u_int8 emphasis_  = c[3] & 0x03;

         if ((layer_ != 0) && (bitrate_ != 0xF0) &&
             (samp_rate_ != RESERVED_SAMPLE_RATE) &&     /* MPEG ES         */
             ( !((samp_rate_ != _44KHZ ) && (pad_ != 0     ))) &&
             ( !((mode_      != JSTEREO) && (mode_ext_ != 0))) &&
             (emphasis_ == 0))
         {
            if (det_pes && (*fmt != CNXT_AUDIO_FMT_MPEG))
               return FALSE;
            *fmt   = CNXT_AUDIO_FMT_MPEG;
            *halffreq = (c[1] & 0x8) >> 3;           /* ID, is 1 for MPEG-1 */
            *layer = (CNXT_AUDIO_MPEG_LAYER) ((c[1] & 0x06) >> 1);
            *bitrate = (c[2] & 0xF0) >> 4;
            *sampfreq = (c[2] & 0x0C) >> 2;
            *chanmode   = (CNXT_AUDIO_CHAN_MODE) ((c[3] & 0xC0) >> 6);  /*mode */
            *totalchans = (*chanmode == 3) ? 1 : 2;  /*1 if mono */
            *avail_chans = CHAN_FLAG_FRONT_LEFT | 
               ((*totalchans == 2) ? CHAN_FLAG_FRONT_RIGHT : 0);
            det_es = TRUE;
            if (det_pes)
               break;
         }
         continue;
      }

      /* 
         validate AC3 elementary using following parameters:
         ac3 syncword: 0x0b77
         ac3 fscod == 48kHz
         ac3 frmsizecod <= 37
         ac3 bsid <= 8
      */
      if ((!det_es) && (c[0] == 0x0B) && (c[1] == 0x77) && 
          ((c[4] & 0xC0) == 0x00) && ((c[4] & 0x3F) <= 37) && 
          ((c[5] & 0xF8) <= 0x40) )   /* AC3 ES  */
      {
         if (det_pes && (*fmt != CNXT_AUDIO_FMT_AC3))
            return FALSE;
         *fmt   = CNXT_AUDIO_FMT_AC3;
         *sampfreq = (c[4] & 0xC0) >> 6;
         *bitrate = (c[4] & 0x3F) >> 0;
         *layer = (CNXT_AUDIO_MPEG_LAYER) NULL;
         *halffreq = (unsigned int) FALSE;
         switch ((c[6] & 0xE0) >> 5)  /* acmod */
         {
         case ACMOD_DUAL_MONO:
            *chanmode = CHAN_MODE_DUAL_MONO;
            *totalchans = 2;
            *avail_chans = 
               CHAN_FLAG_FRONT_LEFT |
               CHAN_FLAG_FRONT_RIGHT;
            break;
         case ACMOD_1_0:
            *chanmode = CHAN_MODE_MONO;
            *totalchans = 1;
            *avail_chans = 
               CHAN_FLAG_FRONT_CENTER;
            break;
         case ACMOD_2_0:
            *chanmode = CHAN_MODE_STEREO;
            *totalchans = 2;
            *avail_chans = 
               CHAN_FLAG_FRONT_LEFT |
               CHAN_FLAG_FRONT_RIGHT;
            break;
         case ACMOD_3_0:
            *chanmode = CHAN_MODE_MULTI;
            *totalchans = 3;
            *avail_chans = 
               CHAN_FLAG_FRONT_LEFT |
               CHAN_FLAG_FRONT_CENTER |
               CHAN_FLAG_FRONT_RIGHT;
            break;
         case ACMOD_2_1:
            *chanmode = CHAN_MODE_MULTI;
            *totalchans = 3;
            *avail_chans = 
               CHAN_FLAG_FRONT_LEFT |
               CHAN_FLAG_FRONT_RIGHT |
               CHAN_FLAG_REAR_LEFT;
            break;
         case ACMOD_3_1:
            *chanmode = CHAN_MODE_MULTI;
            *totalchans = 4;
            *avail_chans = 
               CHAN_FLAG_FRONT_LEFT |
               CHAN_FLAG_FRONT_CENTER |
               CHAN_FLAG_FRONT_RIGHT |
               CHAN_FLAG_REAR_LEFT;
            break;
         case ACMOD_2_2:
            *chanmode = CHAN_MODE_MULTI;
            *totalchans = 4;
            *avail_chans = 
               CHAN_FLAG_FRONT_LEFT |
               CHAN_FLAG_FRONT_RIGHT |
               CHAN_FLAG_REAR_LEFT |
               CHAN_FLAG_REAR_RIGHT;
            break;
         case ACMOD_3_2:
            *chanmode = CHAN_MODE_MULTI;
            *totalchans = 5;
            *avail_chans = 
               CHAN_FLAG_FRONT_LEFT |
               CHAN_FLAG_FRONT_CENTER |
               CHAN_FLAG_FRONT_RIGHT |
               CHAN_FLAG_REAR_LEFT |
               CHAN_FLAG_REAR_RIGHT;
            break;
         default:
            break;
         }
         det_es = TRUE;
         if (det_pes)
            break;
         continue;
      }
   }

   if (det_pes) {
      *encap = CNXT_AUDIO_ENCAP_PES;
   }
   else {
      *encap = CNXT_AUDIO_ENCAP_ES;
   }

   return (det_es || det_pes);
}


/****************************************************************************
 * Modifications:
 * $Log: 
 *  43   mpeg      1.42        4/6/04 1:51:57 PM      Joe Kroesche    CR(s) 
 *        8578 8793 : updated the naming style of some data types for 
 *        consistency, replaced the
 *        following: cnxt_audio_format_t becomes CNXT_AUDIO_CONFIG;
 *        clip_ev_data_t becomes CNXT_CLIP_EV_DATA; and
 *        cnxt_audio_callback_t becomes CNXT_AUDIO_CALLBACK_PFN
 *  42   mpeg      1.41        2/13/04 11:25:29 AM    Matt Korte      CR(s) 
 *        8406 : Changed AUDIO_ to CNXT_AUDIO_
 *        
 *  41   mpeg      1.40        9/24/03 5:45:36 PM     Dave Aerne      SCR(s) 
 *        7541 :
 *        added support for download of proper audio ucode when stream source 
 *        is live
 *        and format is not auto_detect within cnxt_audio_start function.
 *        
 *  40   mpeg      1.39        5/23/03 1:44:58 PM     Craig Dry       SCR(s) 
 *        6579 :
 *        Force AC3 header to 32 bit boundary in audio decode buffer.
 *        
 *  39   mpeg      1.38        4/24/03 3:13:40 PM     Craig Dry       SCR(s) 
 *        6095 :
 *        Qualify PES header match with PES length < 8192
 *        
 *  38   mpeg      1.37        4/24/03 12:32:26 PM    Craig Dry       SCR(s) 
 *        6095 :
 *        1. Replaced (data == TRUE) with equivalent (h->state >= 
 *        AUDIO_STATE_START2)
 *        2. Computed volume_ramp_target_tick so that there is no delay between
 *         a
 *        pause and resume, unless data was queued to play.
 *        3. Modified extract_audio_format to handle case where PES header 
 *        comes
 *        after a valid es stream header(mpeg or ac3).  Previously, the PES
 *        header, if it existed, was assumed to come before the es header.
 *        
 *  37   mpeg      1.36        2/27/03 6:17:22 PM     Dave Aerne      SCR(s) 
 *        4672 :
 *        added code to support AUTO_DETECT for live stream in which correct 
 *        audio
 *        microcode is determined dynamically once stream feed has begun and is
 *         downloaded
 *        
 *        
 *  36   mpeg      1.35        2/13/03 1:21:02 PM     Matt Korte      SCR(s) 
 *        5479 :
 *        Removed old header reference
 *        
 *  35   mpeg      1.34        2/13/03 11:12:38 AM    Matt Korte      SCR(s) 
 *        5479 :
 *        Removed old header reference
 *        
 *  34   mpeg      1.33        1/27/03 9:18:58 PM     Dave Aerne      SCR(s) 
 *        5330 :
 *        cast assignment to halffreq to prevent vxworks compile warning
 *        
 *  33   mpeg      1.32        1/25/03 1:20:04 PM     Dave Aerne      SCR(s) 
 *        5321 :
 *        support added for extract_audio_format to return totalchans and 
 *        avail_chans
 *        parameters when passed an audio encoded clip.
 *        
 *  32   mpeg      1.31        1/23/03 4:52:24 PM     Dave Aerne      SCR(s) 
 *        5302 :
 *        extract_audio_format function was expanded to report additional 
 *        parameters
 *        when passed an audio clip. For MPEG clip, the additional data 
 *        includes bitrate,
 *        sampleing frequency, layer and ID. For AC3 clip the additional 
 *        parameters 
 *        include bitrate and sampling frequency.
 *        
 *        
 *  31   mpeg      1.30        1/15/03 1:42:14 PM     Craig Dry       SCR(s) 
 *        5248 :
 *        Reorder include files. Watchtv built fine, but OTV builds need the 
 *        new order.
 *        
 *  30   mpeg      1.29        1/14/03 2:21:26 PM     Craig Dry       SCR(s) 
 *        5242 :
 *        move PES info from aud_api.c to audio.h
 *        
 *  29   mpeg      1.28        1/13/03 3:47:02 PM     Craig Dry       SCR(s) 
 *        5214 :
 *        added realtime trace_log. changed delays during cnxt_audio_stop.
 *        changed cnxt_audio_resume logic.
 *        
 *  28   mpeg      1.27        1/6/03 6:37:30 PM      Craig Dry       SCR(s) 
 *        5214 :
 *        add extern declaration
 *        
 *  27   mpeg      1.26        1/6/03 6:07:18 PM      Craig Dry       SCR(s) 
 *        5214 :
 *        ramp task semaphore is conditional on CANAL_AUDIO_HANDLING
 *        
 *  26   mpeg      1.25        1/6/03 5:11:38 PM      Craig Dry       SCR(s) 
 *        5214 :
 *        State change on buffer empty interrupt.  This allows pause
 *        command to be issue after buffer runs dry.
 *        
 *  25   mpeg      1.24        1/6/03 12:11:38 PM     Craig Dry       SCR(s) 
 *        5205 :
 *        Mute audio for 200ms whenever a stop is issued
 *        
 *  24   mpeg      1.23        1/6/03 11:43:50 AM     Craig Dry       SCR(s) 
 *        5205 :
 *        Use CANAL_AUDIO_HANDLING ramp task to resume audio
 *        
 *  23   mpeg      1.22        1/5/03 5:48:00 PM      Craig Dry       SCR(s) 
 *        5205 :
 *        Keep audio decode buffer as full as possible for PES clips as well as
 *        for ES clips.
 *        
 *  22   mpeg      1.21        12/20/02 8:45:12 PM    Craig Dry       SCR(s) 
 *        5205 :
 *        Keep audio decode buffer as full as possible
 *        
 *  21   mpeg      1.20        12/19/02 5:40:04 PM    Dave Aerne      SCR(s) 
 *        5200 :
 *        corrections to get_info when called back to back. If too early to 
 *        update
 *        stream info then return last valid information. Also correction for 
 *        intStatus
 *        clears.
 *        
 *  20   mpeg      1.19        12/19/02 12:05:12 PM   Craig Dry       SCR(s) 
 *        5195 :
 *        1. Automatically resume audio if a memory clip is 
 *        paused and then stopped before completion.
 *        2. Previously when the app issued start, the callback
 *        was issued from the same task.  Now the callback is
 *        issued from the audio task. 
 *        3. Return error message if pause or resume feature is
 *        called from live stream.
 *        
 *        
 *  19   mpeg      1.18        12/18/02 9:18:56 PM    Dave Aerne      SCR(s) 
 *        4991 :
 *        corrected chanmode return value for mpeg format within 
 *        extract_audio_format,
 *        removed tabs within same function.
 *        
 *  18   mpeg      1.17        12/17/02 2:12:16 PM    Craig Dry       SCR(s) 
 *        4991 :
 *        Do not disable low water interrupt until after the audio encode 
 *        buffer
 *        runs empty.
 *        
 *  17   mpeg      1.16        12/6/02 12:58:48 PM    Dave Aerne      SCR(s) 
 *        4991 :
 *        formatting changes only, replacing tabs with spaces
 *        
 *  16   mpeg      1.15        12/5/02 6:59:34 PM     Dave Aerne      SCR(s) 
 *        4991 :
 *        set AUDIO_CHAN_FRONT_LEFT_RIGHT to mean no downmixing for AC3, 
 *        modify length check upon entry to cnxt_audio_get_info, and
 *        comment cleanup and enhancement
 *        
 *  15   mpeg      1.14        12/5/02 10:45:26 AM    Dave Aerne      SCR(s) 
 *        4991 :
 *        correction to audio_get_info for returning info about a buffer, also 
 *        expanded
 *        validation of ac3 and mpeg es within extract_audio_format by checking
 *         for
 *        additional bitstream parameters beyond the syncword.
 *        
 *        
 *  14   mpeg      1.13        12/4/02 1:04:46 PM     Craig Dry       SCR(s) 
 *        4991 :
 *        For live streams, pass thru the start/stop functions to the
 *        old api play/stop functions.
 *        
 *  13   mpeg      1.12        12/3/02 7:08:50 PM     Craig Dry       SCR(s) 
 *        4991 :
 *        Restore initial ucode(AC3 or MPEG) after changing it to play
 *        an audio clip.
 *        
 *  12   mpeg      1.11        12/3/02 4:31:20 PM     Craig Dry       SCR(s) 
 *        4991 :
 *        Turn mute off on cnxt_audio_resume.
 *        
 *  11   mpeg      1.10        12/3/02 4:25:12 PM     Craig Dry       SCR(s) 
 *        4991 :
 *        Add logic to cnxt_audio_pause and cnxt_audio_resume.
 *        
 *  10   mpeg      1.9         12/3/02 3:53:16 PM     Dave Aerne      SCR(s) 
 *        4991 :
 *        added support within cnxt_audio_get_info to return stream parameters 
 *        when 
 *        passed a pointer to a buffer. 
 *        
 *  9    mpeg      1.8         12/2/02 8:03:16 PM     Craig Dry       SCR(s) 
 *        4991 :
 *        (Dave Aerne says:) expanding the cnxt_audio_get_info function to 
 *        handle buffers and modified cnxt_audio_select_channels to use defines
 *         rather than numeric constants
 *        
 *  8    mpeg      1.7         12/2/02 5:26:42 PM     Craig Dry       SCR(s) 
 *        4991 :
 *        correction to enum definition for AUDIO_MPEG_LAYER; previously was 
 *        AUDIO_LAYER
 *        
 *  7    mpeg      1.6         11/26/02 8:25:40 PM    Dave Aerne      SCR(s) 
 *        4991 :
 *        correction for Control_reg_1 settings for 
 *        cnxt_audio_select_channels{},
 *        selections FRONT_CENTER, REAR_LEFT, REAR_RIGHT.
 *        
 *  6    mpeg      1.5         11/25/02 7:43:12 PM    Dave Aerne      SCR(s) 
 *        4991 :
 *        added and expanded extensions for MPEG and AC3 header info
 *        
 *        
 *  5    mpeg      1.4         11/25/02 12:34:56 PM   Craig Dry       SCR(s) 
 *        4991 :
 *        put PES structures/data here temporarily until consolidation
 *        into a single *.h file can be accomplished which does not break
 *        openTV
 *        
 *  4    mpeg      1.3         11/21/02 10:21:44 AM   Craig Dry       SCR(s) 
 *        4991 :
 *        Remove remnant //comments that are not needed or helpful
 *        
 *  3    mpeg      1.2         11/21/02 10:05:22 AM   Craig Dry       SCR(s) 
 *        4991 :
 *        Fixed extern declaration of gDemuxInstance for GCC compiler
 *        
 *  2    mpeg      1.1         11/20/02 3:05:20 PM    Craig Dry       SCR(s) 
 *        4991 :
 *        Move aud_PES_global_data to aud_comn.c to share with OpenTV
 *        
 *  1    mpeg      1.0         11/20/02 10:17:08 AM   Craig Dry       
 * $
 * 
 *    Rev 1.40   24 Sep 2003 16:45:36   aernedj
 * SCR(s) 7541 :
 * added support for download of proper audio ucode when stream source is live
 * and format is not auto_detect within cnxt_audio_start function.
 * 
 *    Rev 1.39   23 May 2003 12:44:58   dryd
 * SCR(s) 6579 :
 * Force AC3 header to 32 bit boundary in audio decode buffer.
 * 
 *    Rev 1.38   24 Apr 2003 14:13:40   dryd
 * SCR(s) 6095 :
 * Qualify PES header match with PES length < 8192
 * 
 *    Rev 1.37   24 Apr 2003 11:32:26   dryd
 * SCR(s) 6095 :
 * 1. Replaced (data == TRUE) with equivalent (h->state >= AUDIO_STATE_START2)
 * 2. Computed volume_ramp_target_tick so that there is no delay between a
 *    pause and resume, unless data was queued to play.
 * 3. Modified extract_audio_format to handle case where PES header comes
 *    after a valid es stream header(mpeg or ac3).  Previously, the PES
 *    header, if it existed, was assumed to come before the es header.
 * 
 *    Rev 1.36   27 Feb 2003 18:17:22   aernedj
 * SCR(s) 4672 :
 * added code to support AUTO_DETECT for live stream in which correct audio
 * microcode is determined dynamically once stream feed has begun and is downloaded
 * 
 * 
 *    Rev 1.35   13 Feb 2003 13:21:02   kortemw
 * SCR(s) 5479 :
 * Removed old header reference
 * 
 *    Rev 1.34   13 Feb 2003 11:12:38   kortemw
 * SCR(s) 5479 :
 * Removed old header reference
 * 
 *    Rev 1.33   27 Jan 2003 21:18:58   aernedj
 * SCR(s) 5330 :
 * cast assignment to halffreq to prevent vxworks compile warning
 * 
 *    Rev 1.32   25 Jan 2003 13:20:04   aernedj
 * SCR(s) 5321 :
 * support added for extract_audio_format to return totalchans and avail_chans
 * parameters when passed an audio encoded clip.
 * 
 *    Rev 1.31   23 Jan 2003 16:52:24   aernedj
 * SCR(s) 5302 :
 * extract_audio_format function was expanded to report additional parameters
 * when passed an audio clip. For MPEG clip, the additional data includes bitrate,
 * sampleing frequency, layer and ID. For AC3 clip the additional parameters 
 * include bitrate and sampling frequency.
 * 
 * 
 *    Rev 1.30   15 Jan 2003 13:42:14   dryd
 * SCR(s) 5248 :
 * Reorder include files. Watchtv built fine, but OTV builds need the new order.
 * 
 *    Rev 1.29   14 Jan 2003 14:21:26   dryd
 * SCR(s) 5242 :
 * move PES info from aud_api.c to audio.h
 * 
 *    Rev 1.28   13 Jan 2003 15:47:02   dryd
 * SCR(s) 5214 :
 * added realtime trace_log. changed delays during cnxt_audio_stop.
 * changed cnxt_audio_resume logic.
 * 
 *    Rev 1.27   06 Jan 2003 18:37:30   dryd
 * SCR(s) 5214 :
 * add extern declaration
 * 
 *    Rev 1.26   06 Jan 2003 18:07:18   dryd
 * SCR(s) 5214 :
 * ramp task semaphore is conditional on CANAL_AUDIO_HANDLING
 * 
 *    Rev 1.25   06 Jan 2003 17:11:38   dryd
 * SCR(s) 5214 :
 * State change on buffer empty interrupt.  This allows pause
 * command to be issue after buffer runs dry.
 * 
 *    Rev 1.24   06 Jan 2003 12:11:38   dryd
 * SCR(s) 5205 :
 * Mute audio for 200ms whenever a stop is issued
 * 
 *    Rev 1.23   06 Jan 2003 11:43:50   dryd
 * SCR(s) 5205 :
 * Use CANAL_AUDIO_HANDLING ramp task to resume audio
 * 
 *    Rev 1.22   05 Jan 2003 17:48:00   dryd
 * SCR(s) 5205 :
 * Keep audio decode buffer as full as possible for PES clips as well as
 * for ES clips.
 * 
 *    Rev 1.21   20 Dec 2002 20:45:12   dryd
 * SCR(s) 5205 :
 * Keep audio decode buffer as full as possible
 * 
 *    Rev 1.20   19 Dec 2002 17:40:04   aernedj
 * SCR(s) 5200 :
 * corrections to get_info when called back to back. If too early to update
 * stream info then return last valid information. Also correction for intStatus
 * clears.
 * 
 *    Rev 1.19   19 Dec 2002 12:05:12   dryd
 * SCR(s) 5195 :
 * 1. Automatically resume audio if a memory clip is 
 *  paused and then stopped before completion.
 * 2. Previously when the app issued start, the callback
 *    was issued from the same task.  Now the callback is
 *    issued from the audio task. 
 * 3. Return error message if pause or resume feature is
 *    called from live stream.
 * 
 * 
 *    Rev 1.18   18 Dec 2002 21:18:56   aernedj
 * SCR(s) 4991 :
 * corrected chanmode return value for mpeg format within extract_audio_format,
 * removed tabs within same function.
 * 
 *    Rev 1.17   17 Dec 2002 14:12:16   dryd
 * SCR(s) 4991 :
 * Do not disable low water interrupt until after the audio encode buffer
 * runs empty.
 * 
 *    Rev 1.16   06 Dec 2002 12:58:48   aernedj
 * SCR(s) 4991 :
 * formatting changes only, replacing tabs with spaces
 * 
 *    Rev 1.15   05 Dec 2002 18:59:34   aernedj
 * SCR(s) 4991 :
 * set AUDIO_CHAN_FRONT_LEFT_RIGHT to mean no downmixing for AC3, 
 * modify length check upon entry to cnxt_audio_get_info, and
 * comment cleanup and enhancement
 * 
 *    Rev 1.14   05 Dec 2002 10:45:26   aernedj
 * SCR(s) 4991 :
 * correction to audio_get_info for returning info about a buffer, also expanded
 * validation of ac3 and mpeg es within extract_audio_format by checking for
 * additional bitstream parameters beyond the syncword.
 * 
 * 
 *    Rev 1.13   04 Dec 2002 13:04:46   dryd
 * SCR(s) 4991 :
 * For live streams, pass thru the start/stop functions to the
 * old api play/stop functions.
 * 
 *    Rev 1.12   03 Dec 2002 19:08:50   dryd
 * SCR(s) 4991 :
 * Restore initial ucode(AC3 or MPEG) after changing it to play
 * an audio clip.
 * 
 *    Rev 1.11   03 Dec 2002 16:31:20   dryd
 * SCR(s) 4991 :
 * Turn mute off on cnxt_audio_resume.
 * 
 *    Rev 1.10   03 Dec 2002 16:25:12   dryd
 * SCR(s) 4991 :
 * Add logic to cnxt_audio_pause and cnxt_audio_resume.
 * 
 *    Rev 1.9   03 Dec 2002 15:53:16   aernedj
 * SCR(s) 4991 :
 * added support within cnxt_audio_get_info to return stream parameters when 
 * passed a pointer to a buffer. 
 * 
 *    Rev 1.8   02 Dec 2002 20:03:16   dryd
 * SCR(s) 4991 :
 * (Dave Aerne says:) expanding the cnxt_audio_get_info function to handle buffers and modified cnxt_audio_select_channels to use defines rather than numeric constants
 * 
 *    Rev 1.7   02 Dec 2002 17:26:42   dryd
 * SCR(s) 4991 :
 * correction to enum definition for AUDIO_MPEG_LAYER; previously was AUDIO_LAYER
 * 
 *    Rev 1.6   26 Nov 2002 20:25:40   aernedj
 * SCR(s) 4991 :
 * correction for Control_reg_1 settings for cnxt_audio_select_channels{},
 * selections FRONT_CENTER, REAR_LEFT, REAR_RIGHT.
 * 
 *    Rev 1.5   25 Nov 2002 19:43:12   aernedj
 * SCR(s) 4991 :
 * added and expanded extensions for MPEG and AC3 header info
 * 
 * 
 *    Rev 1.4   25 Nov 2002 12:34:56   dryd
 * SCR(s) 4991 :
 * put PES structures/data here temporarily until consolidation
 * into a single *.h file can be accomplished which does not break
 * openTV
 * 
 *    Rev 1.3   21 Nov 2002 10:21:44   dryd
 * SCR(s) 4991 :
 * Remove remnant //comments that are not needed or helpful
 * 
 *    Rev 1.2   21 Nov 2002 10:05:22   dryd
 * SCR(s) 4991 :
 * Fixed extern declaration of gDemuxInstance for GCC compiler
 * 
 *    Rev 1.1   20 Nov 2002 15:05:20   dryd
 * SCR(s) 4991 :
 * Move aud_PES_global_data to aud_comn.c to share with OpenTV
 * 
 *    Rev 1.0   20 Nov 2002 10:17:08   dryd
 * SCR(s) 4991 :
 * Canal+ DLI4.2 Audio Extenstions and Audio Driver Enhancements
 *
 ****************************************************************************/ 

