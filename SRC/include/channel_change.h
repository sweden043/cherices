/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                Copyright Conexant Systems Inc. 1998-2003                 */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        channel_change.h
 *
 *
 * Description:     Public header file for channel_change driver
 *
 *
 * Author:          Dave Wilson 
 *
 ****************************************************************************/
/* $Header:   
 ****************************************************************************/

#ifndef _CCHANGE_H_
#define _CCHANGE_H_

/***************************************************************************************/
/* CHANNEL_CHANGE is a driver intended to offer a simple-to-use channel change service */
/* for free-to-air DVB services. It offers the following features:                     */
/*                                                                                     */
/* - Support for multiple instances based upon supplied demod, demux, video and audio  */
/*   handles. It is assumed that the caller will handle routing of decoded audio and   */
/*   video to the correct display device.                                              */
/* - Fully automated channel change based upon delivery descriptor and service ID.     */
/* - The ability to hook parsing functions so that the client can monitor PAT and PMT  */
/*   sections when channel change is reading them.                                     */
/* - Caching of the PMT information for the current stream to allow changes to         */
/*   services in the current transponder to be optimised.                              */
/* - Timestamping of critical events during each channel change to allow for gathering */
/*   of statistics by the calling application.                                         */
/* - Callbacks to allow the client to implement conditional access key handling if     */
/*   required. All PAT, PMT and CAT data may be passed to the client allowing them to  */
/*   set independent channels to gather ECMs and EMMs and derive descrambler keys.     */
/*                                                                                     */
/* The driver makes the following assumptions and will only work correctly if these    */
/* are adhered to:                                                                     */
/*                                                                                     */
/* 1. Although a demod handle is passed when the channel change instance is opened,    */
/*    it is assumed that the client application will not call the demod driver to      */
/*    tune and that all tuning will be carried out via the channel change module       */
/*    functions.                                                                       */
/* 2. The module monitors PAT, PMT and CAT tables. It is assumed that the client       */
/*    will not attempt to set demux channels/filters to monitor the same PIDs.         */
/* 3. The client is responsible for building the basic transport stream/service list   */
/*    from the NIT/BAT and SDTs of the stream. The channel change module does not      */
/*    monitor these tables to it is safe for the client to do this.                    */
/* 4. The client is responsible for handling any required data streams in the          */
/*    target service. PMT stream information can be queried to allow the client to,    */
/*    for example, find DVB subtitle or teletext PIDs.                                 */
/*                                                                                     */
/***************************************************************************************/
   
/********************************/
/* Symbol and Macro definitions */
/********************************/

/*****************/
/* Data Types    */
/*****************/

/* Return values for the driver APIs */
typedef enum
{
   CNXT_CCHANGE_OK = 0,
   CNXT_CCHANGE_ALREADY_INIT,
   CNXT_CCHANGE_NOT_INIT,
   CNXT_CCHANGE_BAD_UNIT,
   CNXT_CCHANGE_CLOSED_HANDLE,
   CNXT_CCHANGE_BAD_HANDLE,
   CNXT_CCHANGE_BAD_PARAMETER,
   CNXT_CCHANGE_RESOURCE_ERROR,
   CNXT_CCHANGE_INTERNAL_ERROR,
   CNXT_CCHANGE_NOT_AVAILABLE,
   CNXT_CCHANGE_BUSY,
   CNXT_CCHANGE_BAD_INDEX,
   CNXT_CCHANGE_DEMOD_TIMEOUT,
   CNXT_CCHANGE_PAT_TIMEOUT,
   CNXT_CCHANGE_PMT_TIMEOUT,
   CNXT_CCHANGE_CAT_TIMEOUT,
   CNXT_CCHANGE_VIDEO_TIMEOUT,
   CNXT_CCHANGE_AUDIO_TIMEOUT,
   CNXT_CCHANGE_SERVICE_NOT_FOUND,
   CNXT_CCHANGE_CANCELLED,
   CNXT_CCHANGE_STATUS_CODE_COUNT
} CNXT_CCHANGE_STATUS;

#ifdef INSTANTIATE_CCHANGE_STRINGS
/***********************************************/
/* ASCII representations of the status strings */
/***********************************************/
const char *strCChangeStatusStrings[] =
{
   "CNXT_CCHANGE_OK",
   "CNXT_CCHANGE_ALREADY_INIT",
   "CNXT_CCHANGE_NOT_INIT",
   "CNXT_CCHANGE_BAD_UNIT",
   "CNXT_CCHANGE_CLOSED_HANDLE",
   "CNXT_CCHANGE_BAD_HANDLE",
   "CNXT_CCHANGE_BAD_PARAMETER",
   "CNXT_CCHANGE_RESOURCE_ERROR",
   "CNXT_CCHANGE_INTERNAL_ERROR",
   "CNXT_CCHANGE_NOT_AVAILABLE",
   "CNXT_CCHANGE_BUSY",
   "CNXT_CCHANGE_BAD_INDEX",
   "CNXT_CCHANGE_DEMOD_TIMEOUT",
   "CNXT_CCHANGE_PAT_TIMEOUT",
   "CNXT_CCHANGE_PMT_TIMEOUT",
   "CNXT_CCHANGE_CAT_TIMEOUT",
   "CNXT_CCHANGE_VIDEO_TIMEOUT",
   "CNXT_CCHANGE_AUDIO_TIMEOUT",
   "CNXT_CCHANGE_SERVICE_NOT_FOUND",
   "CNXT_CCHANGE_CANCELLED"
};
#else
extern const char *strCChangeStatusStrings[];
#endif /* INSTANTIATE_CCHANGE_STRINGS */

/* Events passed to the callback */
typedef enum
{
   CNXT_CCHANGE_EVENT_TERM = 0,
   CNXT_CCHANGE_EVENT_RESET,
   CNXT_CCHANGE_EVENT_LOCKED,
   CNXT_CCHANGE_EVENT_SIGNAL_LOST,
   CNXT_CCHANGE_EVENT_PAT_SECTION,
   CNXT_CCHANGE_EVENT_PMT_SECTION,
   CNXT_CCHANGE_EVENT_CAT_SECTION,
   CNXT_CCHANGE_EVENT_DEMOD_TIMEOUT,
   CNXT_CCHANGE_EVENT_PAT_TIMEOUT,
   CNXT_CCHANGE_EVENT_PMT_TIMEOUT,
   CNXT_CCHANGE_EVENT_CAT_TIMEOUT,
   CNXT_CCHANGE_EVENT_VIDEO_TIMEOUT,  /* pData return code from video driver (cast to void *) */
   CNXT_CCHANGE_EVENT_AUDIO_TIMEOUT,
   CNXT_CCHANGE_EVENT_SERVICE_NOT_FOUND,
   CNXT_CCHANGE_EVENT_COMPLETE,
   CNXT_CCHANGE_EVENT_CANCELLED,
   CNXT_CCHANGE_EVENT_ERROR,          /* pData offers extended error information (u_int32 case to void *) */
   CNXT_CCHANGE_EVENT_INVALID,
   CNXT_CCHANGE_NUM_EVENTS /* Event counter for validity checking only */
} CNXT_CCHANGE_EVENT;

#ifdef INSTANTIATE_CCHANGE_STRINGS
/********************************************/
/* ASCII representations of the event names */
/********************************************/
const char *strCChangeEventStrings[] =
{
   "CNXT_CCHANGE_EVENT_TERM",
   "CNXT_CCHANGE_EVENT_RESET",
   "CNXT_CCHANGE_EVENT_LOCKED",
   "CNXT_CCHANGE_EVENT_SIGNAL_LOST",
   "CNXT_CCHANGE_EVENT_PAT_SECTION",
   "CNXT_CCHANGE_EVENT_PMT_SECTION",
   "CNXT_CCHANGE_EVENT_CAT_SECTION",
   "CNXT_CCHANGE_EVENT_DEMOD_TIMEOUT",
   "CNXT_CCHANGE_EVENT_PAT_TIMEOUT",
   "CNXT_CCHANGE_EVENT_PMT_TIMEOUT",
   "CNXT_CCHANGE_EVENT_CAT_TIMEOUT",
   "CNXT_CCHANGE_EVENT_VIDEO_TIMEOUT",
   "CNXT_CCHANGE_EVENT_AUDIO_TIMEOUT",
   "CNXT_CCHANGE_EVENT_SERVICE_NOT_FOUND",
   "CNXT_CCHANGE_EVENT_COMPLETE",
   "CNXT_CCHANGE_EVENT_CANCELLED",
   "CNXT_CCHANGE_EVENT_ERROR",
   "CNXT_CCHANGE_EVENT_INVALID" 
};
#else
extern const char *strCChangeEventStrings[];
#endif /* INSTANTIATE_CCHANGE_STRINGS */

/* Masks used to enable and disable various optional events */
#define CCHANGE_ENABLE_PAT_SECTION  (1 << CNXT_CCHANGE_EVENT_PAT_SECTION)
#define CCHANGE_ENABLE_PMT_SECTION  (1 << CNXT_CCHANGE_EVENT_PMT_SECTION)
#define CCHANGE_ENABLE_CAT_SECTION  (1 << CNXT_CCHANGE_EVENT_CAT_SECTION)

/* Extended error codes used with CNXT_CCHANGE_EVENT_ERROR */
#define CNXT_CCHANGE_ERROR_GENERAL            0x00
#define CNXT_CCHANGE_ERROR_PAT_REQUEST_FAILED 0x01
#define CNXT_CCHANGE_ERROR_PMT_REQUEST_FAILED 0x02
#define CNXT_CCHANGE_ERROR_CAT_REQUEST_FAILED 0x03
#define CNXT_CCHANGE_ERROR_AV_START_FAILURE   0x04
#define CNXT_CCHANGE_ERROR_DEMOD_FAILURE      0x05

#ifdef INSTANTIATE_CCHANGE_STRINGS
/********************************************/
/* ASCII representations of the error codes */
/********************************************/
const char *strCChangeErrorStrings[] =
{
  "CNXT_CCHANGE_ERROR_GENERAL",
  "CNXT_CCHANGE_ERROR_PAT_REQUEST_FAILED",
  "CNXT_CCHANGE_ERROR_PMT_REQUEST_FAILED",
  "CNXT_CCHANGE_ERROR_CAT_REQUEST_FAILED",
  "CNXT_CCHANGE_ERROR_AV_START_FAILURE", 
  "CNXT_CCHANGE_ERROR_DEMOD_FAILURE"     
};
#else
extern const char *strCChangeErrorStrings[];
#endif /* INSTANTIATE_CCHANGE_STRINGS */

/* Driver configuration structure - dummy here since this driver doesn't current use it */
typedef void* CNXT_CCHANGE_CONFIG;

#ifndef CNXT_DEMOD_HANDLE
#define CNXT_DEMOD_HANDLE u_int32
#endif

#ifndef CNXT_DEMUX_HANDLE
#define CNXT_DEMUX_HANDLE u_int32
#endif

#ifndef CNXT_AUDIO_HANDLE
#define CNXT_AUDIO_HANDLE u_int32
#endif

#ifndef CNXT_VIDEO_HANDLE
#define CNXT_VIDEO_HANDLE u_int32
#endif

/* Device capability structure */
typedef struct
{
   u_int32             uLength;
   bool                bExclusive;
   bool                bUseStillVideo;
   /* Fields below must be completed by the caller before calling cnxt_cchange_open */
   u_int32             uEventEnables;  /* OR together CCHANGE_ENABLE_xxx for optional events */
   TUNING_SPEC         sInitialTransponder;
   CNXT_DEMOD_HANDLE   hDemod;   /* Currently this is a u_int32 type for the DEMOD driver */
   CNXT_DEMUX_HANDLE   hDemux;   /* Currently this is the u_int32 instanced handle from DEMUX */
   CNXT_VIDEO_HANDLE   hVideo;   /* Dummy - for use with future multi-instance video decoder driver */
   CNXT_AUDIO_HANDLE   hAudio;   /* Dummy - for use with future multi-instance audio decoder driver */
   u_int32             iDemuxChannelAudio;
   u_int32             iDemuxChannelVideo;
   /* Note: put other elements here */
} CNXT_CCHANGE_CAPS;

/* device handle */
typedef struct cnxt_cchange_inst *CNXT_CCHANGE_HANDLE;

/* notification function */
typedef CNXT_CCHANGE_STATUS (*CNXT_CCHANGE_PFNNOTIFY) ( CNXT_CCHANGE_HANDLE   Handle, 
                                                        void                 *pUserData,
                                                        CNXT_CCHANGE_EVENT    Event,
                                                        void                 *pData,
                                                        void                 *Tag );

/* Indices of the various timestamps gathered during each channel change */
typedef enum
{
  CNXT_CCHANGE_TIMESTAMP_BEGIN = 0,
  CNXT_CCHANGE_TIMESTAMP_DEMOD_TUNE,
  CNXT_CCHANGE_TIMESTAMP_DEMOD_LOCK,
  CNXT_CCHANGE_TIMESTAMP_DEMOD_UNLOCK,
  CNXT_CCHANGE_TIMESTAMP_PAT_RECEIVED,
  CNXT_CCHANGE_TIMESTAMP_CAT_RECEIVED,
  CNXT_CCHANGE_TIMESTAMP_PMT_REQUESTED,
  CNXT_CCHANGE_TIMESTAMP_PMT_RECEIVED,
  CNXT_CCHANGE_TIMESTAMP_AUDIO_STARTED,
  CNXT_CCHANGE_TIMESTAMP_AUDIO_PLAYING,
  CNXT_CCHANGE_TIMESTAMP_VIDEO_STARTED,
  CNXT_CCHANGE_TIMESTAMP_PES_HDR_RECEIVED,
  CNXT_CCHANGE_TIMESTAMP_PCR_RECEIVED, 
  CNXT_CCHANGE_TIMESTAMP_PTS_RECEIVED, 
  CNXT_CCHANGE_TIMESTAMP_PTS_MATURED,
  CNXT_CCHANGE_TIMESTAMP_FIRST_PICTURE_DECODE,
  CNXT_CCHANGE_TIMESTAMP_SECOND_PICTURE_DECODE,
  CNXT_CCHANGE_TIMESTAMP_VIDEO_PLAYING,
  CNXT_CCHANGE_TIMESTAMP_COMPLETE,
  CNXT_CCHANGE_NUM_TIMESTAMPS
} CNXT_CCHANGE_TIMESTAMP_INDEX;  

/* Structure used to return timing information on last channel change */
typedef struct
{
  TUNING_SPEC sTransponder;
  u_int16     uServiceID;
  u_int16     uVideoFrameCount;
  u_int32 uTimestamp[CNXT_CCHANGE_NUM_TIMESTAMPS];
} CNXT_CCHANGE_TIMESTAMPS;

/* Default timeout values in milliseconds - these are very conservative! */
#define CCHANGE_DEFAULT_TIMEOUT_PAT        300
#define CCHANGE_DEFAULT_TIMEOUT_PMT        500
#define CCHANGE_DEFAULT_TIMEOUT_CAT        500
#define CCHANGE_DEFAULT_TIMEOUT_VIDEO1     200
#define CCHANGE_DEFAULT_TIMEOUT_VIDEO2    1800
#define CCHANGE_DEFAULT_TIMEOUT_VIDEO3    3000
#define CCHANGE_DEFAULT_TIMEOUT_AUDIO     2000

/* Indices of the various timeouts held in the CNXT_CCHANGE_TIMEOUTS structure */
typedef enum
{
  CNXT_CCHANGE_TIMEOUT_PAT = 0,
  CNXT_CCHANGE_TIMEOUT_PMT,
  CNXT_CCHANGE_TIMEOUT_CAT,
  CNXT_CCHANGE_TIMEOUT_VIDEO1,
  CNXT_CCHANGE_TIMEOUT_VIDEO2,
  CNXT_CCHANGE_TIMEOUT_VIDEO3,
  CNXT_CCHANGE_TIMEOUT_AUDIO,
  CNXT_CCHANGE_NUM_TIMEOUTS  
} CNXT_CCHANGE_TIMEOUT_INDEX;  

/* Structure containing timeout values for various parts of the channel change process */
typedef struct
{
  u_int32 uTimeout[CNXT_CCHANGE_NUM_TIMEOUTS];
} CNXT_CCHANGE_TIMEOUTS;  

/* Structures related to  program and stream lists */
typedef struct 
{
  u_int16 uServiceID;
  u_int16 uPMTPID;
} CNXT_CCHANGE_PATENTRY;

typedef struct 
{
  u_int8  cStreamType;
  u_int16 uStreamPID;
  u_int16 uStreamCAPID;
} CNXT_CCHANGE_PMTENTRY;

typedef struct 
{
  DVBPATINFO sPATInfo;
  u_int32    uNumPrograms;
} CNXT_CCHANGE_PATINFO;

typedef struct 
{
  DVBPMTINFO sPMTInfo;
  u_int32    uNumStreams;
} CNXT_CCHANGE_PMTINFO;

/*******************************/
/* DVB Elementary Stream Types */
/*******************************/
#define MPEG_ES_TYPE_RESERVED     0x00
#define MPEG_ES_TYPE_MPEG1_VIDEO  0x01
#define MPEG_ES_TYPE_MPEG2_VIDEO  0x02
#define MPEG_ES_TYPE_MPEG1_AUDIO  0x03
#define MPEG_ES_TYPE_MPEG2_AUDIO  0x04
#define MPEG_ES_TYPE_PRIVATE_SEC  0x05
#define MPEG_ES_TYPE_PRIVATE_PES  0x06
#define MPEG_ES_TYPE_MHEG         0x07
#define MPEG_ES_TYPE_DSM_CC       0x08
#define MPEG_ES_TYPE_H222         0x09
#define MPEG_ES_TYPE_13818_6_A    0x0A
#define MPEG_ES_TYPE_13818_6_B    0x0B
#define MPEG_ES_TYPE_13818_6_C    0x0C
#define MPEG_ES_TYPE_13818_6_D    0x0D
#define MPEG_ES_TYPE_AUX          0x0E

#define NUM_STREAM_TYPE_STRINGS 15

#define IS_VIDEO_STREAM(c) ((((c) == MPEG_ES_TYPE_MPEG1_VIDEO) || ((c) == MPEG_ES_TYPE_MPEG2_VIDEO)) ? TRUE : FALSE)
#define IS_AUDIO_STREAM(c) ((((c) == MPEG_ES_TYPE_MPEG1_AUDIO) || ((c) == MPEG_ES_TYPE_MPEG2_AUDIO)) ? TRUE : FALSE)

#ifdef INSTANTIATE_CCHANGE_STRINGS
const char *strMPEGStreamTypes[] =
{
  "Reserved   ",
  "MPEG1 Video",
  "MPEG2 Video",
  "MPEG1 Audio",
  "MPEG2 Audio",
  "Private Sec",
  "Private PES",
  "MHEG       ",
  "DSM CC     ",
  "H.222.1    ",
  "13818-6 A  ",
  "13818-6 B  ",
  "13818-6 C  ",
  "13818-6 D  ",
  "Auxiliary  "
};
#else
extern const char *strMPEGStreamTypes[];
#endif

/******************/
/* API prototypes */
/******************/

/* Initialise the channel change driver. */
CNXT_CCHANGE_STATUS cnxt_cchange_init ( CNXT_CCHANGE_CONFIG *pCfg );

/* Shut down the driver and free all associated resources */
CNXT_CCHANGE_STATUS cnxt_cchange_term ( void );

/* Channel change is basically a helper module that doesn't own any particular */
/* hardware unit. This call will return 1 in all cases.                        */
CNXT_CCHANGE_STATUS cnxt_cchange_get_units ( u_int32 *puCount );

/* Return capabilities of the unit. A subset of the caps structure will be */
/* completed on this call. Handles for demux, demod, decoders and display  */
/* will not be passed back since only the client can provide these.        */
CNXT_CCHANGE_STATUS cnxt_cchange_get_unit_caps ( u_int32            uUnitNumber, 
                                                 CNXT_CCHANGE_CAPS *pCaps );

/* Create an instance of the channel change object. The caps structure passed */
/* contains handles allowing channel change to determine which demod, demux,  */
/* video decoder, audio decoder and display device to use. These are supplied */
/* by the caller.                                                             */
CNXT_CCHANGE_STATUS cnxt_cchange_open ( CNXT_CCHANGE_HANDLE     *pHandle,
                                        CNXT_CCHANGE_CAPS       *pCaps,
                                        CNXT_CCHANGE_PFNNOTIFY   pNotifyFn,
                                        void                    *pUserData );

/* Destroy an instance of the channel change object and free any associated */
/* resources.                                                               */
CNXT_CCHANGE_STATUS cnxt_cchange_close ( CNXT_CCHANGE_HANDLE Handle );

/* Initiate a channel change operation */
CNXT_CCHANGE_STATUS cnxt_cchange_change_to( CNXT_CCHANGE_HANDLE  Handle,
                                            TUNING_SPEC         *pTransponder,
                                            u_int16              uServiceID,
                                            bool                 bAsync,
                                            void                *Tag );

/* Cancel a pending channel change operation */
CNXT_CCHANGE_STATUS cnxt_cchange_cancel( CNXT_CCHANGE_HANDLE Handle );

/* Query the timestamps gathered during the last channel change */
CNXT_CCHANGE_STATUS cnxt_cchange_query_timestamps( CNXT_CCHANGE_HANDLE        Handle,
                                                   CNXT_CCHANGE_TIMESTAMPS  *pTimestamps);
                                                   
/* Query the timeout values currently set for various parts of the */
/* channel change process                                          */
CNXT_CCHANGE_STATUS cnxt_cchange_query_timeouts( CNXT_CCHANGE_HANDLE     Handle,
                                                 CNXT_CCHANGE_TIMEOUTS  *pTimeouts);

/* Set the timeouts to be used for various parts of the channel change process */
CNXT_CCHANGE_STATUS cnxt_cchange_set_timeouts( CNXT_CCHANGE_HANDLE     Handle,
                                               CNXT_CCHANGE_TIMEOUTS  *pTimeouts);

/* Enable or disable optional events */
CNXT_CCHANGE_STATUS cnxt_cchange_set_events( CNXT_CCHANGE_HANDLE Handle,
                                             u_int32             uEventMask);

/* Query which optional events are currently enabled */
CNXT_CCHANGE_STATUS cnxt_cchange_query_events( CNXT_CCHANGE_HANDLE Handle,
                                               u_int32            *pEventMask);

/* Get information on the current PAT read for this instance */
CNXT_CCHANGE_STATUS cnxt_cchange_query_pat( CNXT_CCHANGE_HANDLE   Handle,
                                            CNXT_CCHANGE_PATINFO *pPatInfo);

/* Get information on one of the services read from the current PAT */
CNXT_CCHANGE_STATUS cnxt_cchange_query_pat_service_by_index( CNXT_CCHANGE_HANDLE    Handle,
                                                             int                    iIndex,
                                                             CNXT_CCHANGE_PATENTRY *pPatEntry);
                                                             
CNXT_CCHANGE_STATUS cnxt_cchange_query_pat_service_by_id( CNXT_CCHANGE_HANDLE    Handle,
                                                          u_int16                uServiceID,
                                                          CNXT_CCHANGE_PATENTRY *pPatEntry);

/* Get information on the current service's PMT */
CNXT_CCHANGE_STATUS cnxt_cchange_query_pmt( CNXT_CCHANGE_HANDLE  Handle,
                                            CNXT_CCHANGE_PMTINFO *pPmtInfo);
                                                     
/* Get information on the streams associated with the current service */
CNXT_CCHANGE_STATUS cnxt_cchange_query_pmt_stream(CNXT_CCHANGE_HANDLE   Handle,
                                                 int                    iStreamIndex,
                                                 CNXT_CCHANGE_PMTENTRY *pPmtEntry);

/* Enumerate video streams in the current service */
CNXT_CCHANGE_STATUS cnxt_cchange_query_num_video_streams(CNXT_CCHANGE_HANDLE Handle,
                                                         int                 *pNumStreams);
                                                         
/* Enumerate audio streams in the current service */
CNXT_CCHANGE_STATUS cnxt_cchange_query_num_audio_streams(CNXT_CCHANGE_HANDLE Handle,
                                                         int                 *pNumStreams);
                                                               
/* Choose one each of the available video and audio streams */
CNXT_CCHANGE_STATUS cnxt_cchange_set_audio_video_streams(CNXT_CCHANGE_HANDLE Handle,
                                                         int                 iVideoIndex,
                                                         int                 iAudioIndex,
                                                         bool                bAsync,
                                                         void               *Tag);

/* Get the indices of the audio and video streams currently selected */
CNXT_CCHANGE_STATUS cnxt_cchange_query_audio_video_streams(CNXT_CCHANGE_HANDLE Handle,
                                                           int                 *pVideoIndex,
                                                           int                 *pAudioIndex);

/* Enumerate streams of a given type in the current service */
CNXT_CCHANGE_STATUS cnxt_cchange_query_num_streams_by_type(CNXT_CCHANGE_HANDLE Handle,
                                                           u_int8              cStreamType,
                                                           int                 *pNumStreams);

/* Get information on a particular stream in the current service */                                                         
CNXT_CCHANGE_STATUS cnxt_cchange_query_stream_by_type(CNXT_CCHANGE_HANDLE    Handle,
                                                      u_int8                 cStreamType,
                                                      int                    iIndex,
                                                      CNXT_CCHANGE_PMTENTRY *pPmtEntry);
                                                                                                                 
#endif   /* _CCHANGE_H_ */

/****************************************************************************
 * Modifications:
 * $Log: 
 *  3    mpeg      1.2         7/31/03 12:38:46 PM    Dave Wilson     SCR(s) 
 *        7090 :
 *        Removed #ifdef DEBUG/#endif directives around instantiation of state 
 *        and 
 *        return code strings. These may be of use in release builds too.
 *        
 *  2    mpeg      1.1         7/1/03 12:06:04 PM     Dave Wilson     SCR(s) 
 *        6363 :
 *        Latest, working version.
 *        
 *  1    mpeg      1.0         6/4/03 2:22:46 PM      Dave Wilson     
 * $
 * 
 *    Rev 1.2   31 Jul 2003 11:38:46   dawilson
 * SCR(s) 7090 :
 * Removed #ifdef DEBUG/#endif directives around instantiation of state and 
 * return code strings. These may be of use in release builds too.
 * 
 *    Rev 1.1   01 Jul 2003 11:06:04   dawilson
 * SCR(s) 6363 :
 * Latest, working version.
 * 
 *    Rev 1.0   04 Jun 2003 13:22:46   dawilson
 * SCR(s) 6363 :
 * Public header for channel change module API.
 *
 ****************************************************************************/

