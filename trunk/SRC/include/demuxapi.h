/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                    Conexant Systems Inc. (c) 1998-2003                   */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        demuxapi.h
 *
 *
 * Description:     Demux Driver Public API Header File
 *
 *
 * Author:          Bob Van Gulick
 *
 ****************************************************************************/
/* $Id: demuxapi.h,v 1.46, 2004-06-15 22:28:03Z, Tim White$
 ****************************************************************************/

#if CUSTOMER == VENDOR_B
#define PARSER_FILTERING FILTER_888  
#endif

/* Interface version of the pawser microcode required to interoperate with 
   this driver */
#define DPS_UCODE_VERSION      0x00002000
#define DPS_UCODE_VERSION_MASK 0x0000F000
#define DPS_UCODE_WORD_IDX     3

#define DPS_UCODE_SCRAM_STATUS 0x80000000
#define DPS_UCODE_CAP_WORD_IDX 4

// Debug messaging functions

#define DPS_ERROR_MSG                           (TRACE_DPS|TRACE_LEVEL_ALWAYS)
#define DPS_FUNC_TRACE                          (TRACE_DPS|TRACE_LEVEL_2)
#define DPS_MSG                                 (TRACE_DPS|TRACE_LEVEL_3)
#define DPS_ISR_MSG                             (TRACE_DPS|TRACE_LEVEL_1)

#define GENDMX_CONNECT          0
#define GENDMX_DISCONNECT       1
#define GENDMX_UNKNOWN          2

#define GENDMX_ERROR_RETURN     0xFFFFFFFF

#define AUDIO2_CHANNEL          4
#define AUDIO1_CHANNEL          3
#define AUDIO_CHANNEL           2
#define VIDEO_CHANNEL           1
#define TOTAL_CHANNELS          32
#define TOTAL_FILTERS           32

#define GEN_NULL_PID            0x1FFF
#define GEN_INVALID_PID         0xFFFF
#define GEN_DEFAULT_DEMUX_ID    0
#define GEN_INVALID_PID         0xFFFF

#define GENDMX_DEFAULT_PES_INTR_FREQ       1024
#define GEN_DEFAULT_PES_BUFFER_SIZE ((1024/188) * 188)

typedef enum
{
    GENDEMUX_CONTINUOUS,
    GENDEMUX_ONE_SHOT,
    GENDEMUX_TOGGLE
} genfilter_mode;

typedef enum
{
    VIDEO_PES_TYPE = 1,
    AUDIO_PES_TYPE,
    PES_CHANNEL_TYPE,
    ES_CHANNEL_TYPE,
#if XTV_SUPPORT==YES
    PSI_CHANNEL_TYPE,
    XTV_RECORD_CHANNEL_TYPE,
    RECORD_PSI_CHANNEL_TYPE,
    XTV_RECORD_PSI_CHANNEL_TYPE,
    XTV_RECORD_ECM_CHANNEL_TYPE
#else
    PSI_CHANNEL_TYPE
#endif
} genchannel_t;

typedef enum
{
    GEN_DEMUX_DISABLE = 0,
    GEN_DEMUX_ENABLE,
    GEN_DEMUX_RESET,
    GEN_DEMUX_DISABLE_RESET,
    GEN_DEMUX_DISABLE_RECORD,
    GEN_DEMUX_ENABLE_RECORD
} gencontrol_channel_t;

/* Demux API return codes */
typedef enum {
    DMX_STATUS_OK,
    DMX_ERROR,
    DMX_BAD_DMXID,
    DMX_BAD_CHID,
    DMX_BAD_PID,
    DMX_BAD_FID,
    DMX_BAD_CTL,
    DMX_BAD_FILTER,
    DMX_CH_UNAVAILABLE,
    DMX_CH_CONTROL_ERROR
} DMX_STATUS;

/* demux hardware instance capability flags */
#define DMX_CAP_PLAYBACK      0x0001
#define DMX_CAP_RECORD        0x0002
#define DMX_CAP_DESCRAMBLE    0x0004
#define DMX_CAP_LITTLE_ENDIAN 0x0008

/* channel capabilities */
#define DMX_CH_CAP_DESCRAMBLING  0x0001
#define DMX_CH_CAP_HWFILTER      0x0002
#define DMX_CH_CAP_SWFILTER      0x0004

/* input device selection. Used with the cnxt_dmx_set_input_device() function */
#define DMX_INPUT_DEV_NIM  0x0001
#define DMX_INPUT_DEV_DMA  0x0002

#define DEMUX_FREE -1

/* Brazos.h has DPS_NUM_DESCRAMBLERS defined to be 25.  On earlier chips we can run
   with a variable number of key slots based on multi or single PSI configuration. */
#ifndef DPS_NUM_DESCRAMBLERS
  #if (PARSER_PSI_BUFFER == MULTI_PSI) 
    #define DPS_NUM_DESCRAMBLERS     8
  #else
    #define DPS_NUM_DESCRAMBLERS     6
  #endif
#endif

typedef struct _gen_notify_t
{
    // Set by the generic demux driver
    u_int8      *pData;                     /* pointer to the header data or block data*/
    u_int32     condition;
    u_int32     chid;

    // set by the client demux driver in header callback
    u_int8  *write_ptr;
    u_int32 skip;
    u_int32 length;
    u_int32 tag;
    u_int32 fid_mask;
} gen_notify_t;
typedef struct _gen_notify_t *pgen_notify_t;

typedef genfilter_mode (*gen_callback_fct_t)( pgen_notify_t pdata );
#ifdef MHP
typedef genfilter_mode (*gen_section_callback_fct_t)( pgen_notify_t pdata, u_int32 fid);
#endif
typedef void (*gen_pcr_callback_fct_t)(u_int32 PCR_High, u_int32 PCR_Low);
typedef void (*gen_event6_callback_fct_t)(void);
typedef void (*dvr_notify_fct_t)(u_int32 dmxid, u_int32 Event);

typedef struct {
  u_int32 pcr_base_1msb;
  u_int32 pcr_base_32lsb;
  u_int32 pcr_extension_9b;
} genpcr_t;

#ifdef MHP
#define GENDMX_MAX_HW_FILTER_SIZE   12
#else
#if PARSER_FILTERING == FILTER_888
#define GENDMX_MAX_HW_FILTER_SIZE   8
#else
#define GENDMX_MAX_HW_FILTER_SIZE   12
#endif
#endif

#define GENDMX_EXT_FILTER_SIZE      8
#define GENDMX_BAD_CHANNEL          0xFFFFFFFF
#define GENDMX_BAD_FILTER           0xFFFFFFFF

#define GENDMX_SECTION_AVAILABLE    0
#define GENDMX_ERROR                1
#define GENDMX_CRC_CHECKED          2
#define GENDMX_PES_OVERFLOW         4
#define GENDMX_PES_SCRBL_ERROR      8
#define GENDMX_CHANNEL_TIMEOUT      16

/*
 * Legacy DVR
 */
#if (LEGACY_DVR==YES)
typedef enum
{
    GEN_DMX_DVR_CHANNEL_EVENT = 0,
    GEN_DMX_DVR_CHANNEL_VIDEO,
    GEN_DMX_DVR_CHANNEL_AUDIO,
    GEN_DMX_DVR_CHANNEL_AUDIO1,
    GEN_DMX_DVR_CHANNEL_AUDIO2
} gendmx_dvr_channel_t;

#endif

#define BSWAP(x) (((x & 0xFF) << 24) |                         \
                  ((x & 0xFF00) << 8) |                        \
                  ((x & 0xFF0000) >> 8) |                      \
                  ((x & 0xFF000000) >> 24))

//******************************//
// Demux External API Functions //
//******************************//
DMX_STATUS cnxt_dmx_open(bool ReInit, u_int32 capabilities, u_int32* dmxid);
DMX_STATUS cnxt_dmx_query_status(u_int32 dmxid);
DMX_STATUS cnxt_dmx_close(u_int32 dmxid);
DMX_STATUS cnxt_dmx_reset(u_int32 dmxid);
DMX_STATUS cnxt_dmx_channel_open(u_int32 dmxid, u_int32 capabilities, 
               genchannel_t channel_type, u_int32* chid); 
DMX_STATUS cnxt_dmx_channel_close(u_int32 dmxid, u_int32 chid);
DMX_STATUS cnxt_dmx_channels_available(u_int32 dmxid, u_int32 capabilities, u_int32* cnt,
 	           u_int32 size_of_filter);
DMX_STATUS cnxt_dmx_channel_set_pid(u_int32 dmxid, u_int32 chid, u_int16 pid);
DMX_STATUS cnxt_dmx_channel_get_pid(u_int32 dmxid, u_int32 chid, u_int16* pid);
DMX_STATUS cnxt_dmx_channel_control(u_int32 dmxid, u_int32 chid, gencontrol_channel_t channel_command);
DMX_STATUS cnxt_dmx_filter_open(u_int32 dmxid, u_int32 chid, u_int32 filter_size,u_int32* fid);
DMX_STATUS cnxt_dmx_filter_close(u_int32 dmxid, u_int32 chid, u_int32 fid);
DMX_STATUS cnxt_dmx_set_filter(u_int32 dmxid, u_int32 chid, u_int32 fid,
               u_int8 *filter, u_int8 *mask, u_int8 *notmask);
DMX_STATUS cnxt_dmx_filters_available(u_int32 dmxid, u_int32 *num_hw_filters_avail);
DMX_STATUS cnxt_dmx_set_version_filter(u_int32 dmxid, u_int32 chid, u_int32 fid, 
               u_int8 version, bool version_equal, bool enable);
DMX_STATUS cnxt_dmx_set_ext_filter(u_int32 dmxid, u_int32 chid, u_int32 fid, u_int8 *filter,
               u_int8 *mask, u_int32 size, u_int32 start_offset);
DMX_STATUS cnxt_dmx_filter_control(u_int32 dmxid, u_int32 chid,
               u_int32 fid, gencontrol_channel_t enable_disable);
#ifdef MHP
DMX_STATUS cnxt_dmx_set_section_channel_attributes(u_int32 dmxid, u_int32 chid, 
               gen_callback_fct_t header_notify, gen_section_callback_fct_t section_notify, 
               u_int32 TimeOut, u_int32 MinHdrSize);
#else
DMX_STATUS cnxt_dmx_set_section_channel_attributes(u_int32 dmxid, u_int32 chid, 
	       gen_callback_fct_t header_notify, gen_callback_fct_t section_notify, 
               u_int32 TimeOut, u_int32 MinHdrSize);
#endif
DMX_STATUS cnxt_dmx_set_section_channel_tag(u_int32 dmxid, u_int32 chid, u_int32 tag);
DMX_STATUS cnxt_dmx_set_pes_channel_attributes(u_int32 dmxid, u_int32 chid, 
	       gen_callback_fct_t data_notify, gen_callback_fct_t error_notify);
#if PARSER_TYPE==DTV_PARSER
DMX_STATUS cnxt_dmx_set_CWP_attributes(u_int32 dmxid, gen_callback_fct_t CWP_notify,
                                       u_int32 TimeOut);
DMX_STATUS cnxt_dmx_set_CAP_attributes(u_int32 dmxid, gen_callback_fct_t CAP_notify,
                                       u_int32 TimeOut);
DMX_STATUS cnxt_dmx_CAP_set_scid(u_int32 dmxid, u_int16 scid);
DMX_STATUS cnxt_dmx_CAP_set_filter(u_int32 dmxid, u_int32 filter);
DMX_STATUS cnxt_dmx_CAP_control(u_int32 dmxid, gencontrol_channel_t channel_command);
#endif
DMX_STATUS cnxt_dmx_connect_notify(u_int32 dmxid, int event_type);
DMX_STATUS cnxt_dmx_get_notify_data(u_int32 dmxid, u_int32 chid, gen_notify_t *notify_data); 
DMX_STATUS cnxt_dmx_register_callbacks(u_int32 dmxid, gen_callback_fct_t HeaderCallBackEntry, 
               gen_callback_fct_t SectionCallBackEntry);
DMX_STATUS cnxt_dmx_get_channel_write_pointer(u_int32 dmxid, u_int32 chid, u_int8 **write_ptr); 
DMX_STATUS cnxt_dmx_get_channel_buffer_pointer(u_int32 dmxid, u_int32 chid, 
               u_int8 **write_ptr, u_int32 *buff_size);
DMX_STATUS cnxt_dmx_set_pcr_pid(u_int32 dmxid, u_int16 pid);
DMX_STATUS cnxt_dmx_get_current_pcr(u_int32 dmxid, genpcr_t *current_pcr);
DMX_STATUS cnxt_dmx_register_pcr_notify(gen_pcr_callback_fct_t pcr_notify);
DMX_STATUS cnxt_dmx_query_pcr_notify(gen_pcr_callback_fct_t *pcr_notify);
DMX_STATUS cnxt_dmx_register_event6_notify(gen_event6_callback_fct_t event_notify);
DMX_STATUS cnxt_dmx_query_event6_notify(gen_event6_callback_fct_t *event_notify);
DMX_STATUS cnxt_dmx_crc_enable(u_int32 dmxid, gencontrol_channel_t crc_command);
DMX_STATUS cnxt_dmx_set_channel_buffer(u_int32 dmxid, u_int32 chid,
				       void *buffer, u_int32 size);
DMX_STATUS cnxt_dmx_read_pes_buffer(u_int32 dmxid, u_int32 chid, 
               u_int32 size,u_int8 *buffer, u_int32 *actual_size);
DMX_STATUS cnxt_dmx_empty_pes_buffer(u_int32 dmxid, u_int32 chid, u_int8 *write);
DMX_STATUS cnxt_dmx_get_video_pid(u_int32 dmxid, bool bHardware, u_int16 *vidpid);
DMX_STATUS cnxt_dmx_get_pes_frame_rate(u_int32 dmxid, u_int16 *frame_rate);
DMX_STATUS cnxt_dmx_set_pts_offset(u_int32 dmxid, u_int32 pts_video, u_int32 pts_audio);
DMX_STATUS cnxt_dmx_set_timebase_offset(u_int32 dmxid, u_int64 pts_video, u_int64 pts_audio);
DMX_STATUS cnxt_dmx_get_timebase_offset(u_int32 dmxid, u_int64 *pts_video, u_int64 *pts_audio);
DMX_STATUS cnxt_dmx_descrambler_init(u_int32 dmxid);
DMX_STATUS cnxt_dmx_descrambler_open(u_int32 dmxid, genchannel_t type, 
                        u_int32 *descrambler_channel);
DMX_STATUS cnxt_dmx_query_channel_scrambling(u_int32 dmxid, u_int32 chid, bool *status); 
DMX_STATUS cnxt_dmx_descrambler_close(u_int32 dmxid, u_int32 descrambler);
DMX_STATUS cnxt_dmx_descrambler_set_pid(u_int32 dmxid, u_int32 descrambler, u_int16 pid);
DMX_STATUS cnxt_dmx_descrambler_set_odd_keys(u_int32 dmxid, u_int32 descrambler, 
 		        u_int32 odd_key_length, const char * odd_key); 
DMX_STATUS cnxt_dmx_descrambler_set_even_keys(u_int32 dmxid, u_int32 descrambler, 
                        u_int32 even_key_length, const char * even_key);
DMX_STATUS cnxt_dmx_descrambler_control(u_int32 dmxid, u_int32 descrambler, 
                        gencontrol_channel_t enable_disable);
void cnxt_dmx_descrambler_notify_pid_change(u_int32 dmxid, u_int32 chid, u_int16 pid);
void cnxt_dmx_descrambler_connect_notify(u_int32 dmxid, bool connected);

#if (CANAL_AUDIO_HANDLING == YES)
/* Global variables used to monitor arrival of audio/video packets
 * under Media Highway.  This should really use an API instead of
 * global variables.
 */
extern u_int32 gdwVideoPktsReceived;
extern u_int32 gdwAudioPktsReceived;
#endif

#if (PVR == YES) || (XTV_SUPPORT == YES)
/*
 * Demux event structure
 */
typedef struct _DMX_REC_EVENT_
{
   u_int32  uHdr;
   u_int32  uPkt;
   u_int32  uPrevPkt;
   u_int32  uGTC;
} DMX_REC_EVENT, *PDMX_REC_EVENT;

#define  DMX_EVENT_SIZE    (sizeof(DMX_REC_EVENT))

/*
 * Demux Event Header
 */
#define  DMX_REC_EVENT_PROG_FRAME_SHIFT               30  
#define  DMX_REC_EVENT_PROG_FRAME_MASK                (1UL << DMX_REC_EVENT_PROG_FRAME_SHIFT)
#define  DMX_REC_EVENT_TOP_FIELD_FIRST_SHIFT          29       
#define  DMX_REC_EVENT_TOP_FIELD_FIRST_MASK           (1UL << DMX_REC_EVENT_TOP_FIELD_FIRST_SHIFT)
#define  DMX_REC_EVENT_SPLIT_SHIFT                    28
#define  DMX_REC_EVENT_SPLIT_MASK                     (1UL << DMX_REC_EVENT_SPLIT_SHIFT)
#define  DMX_REC_EVENT_PICT_TYPE_SHIFT                26
#define  DMX_REC_EVENT_PICT_TYPE_MASK                 (0x3UL << DMX_REC_EVENT_PICT_TYPE_SHIFT)
#define     DMX_REC_EVENT_PICT_TYPE_I_FRAME           0x1
#define     DMX_REC_EVENT_PICT_TYPE_P_FRAME           0x2
#define     DMX_REC_EVENT_PICT_TYPE_B_FRAME           0x3
#define  DMX_REC_EVENT_PICT_STRUCT_SHIFT              24
#define  DMX_REC_EVENT_PICT_STRUCT_MASK               (0x3UL << DMX_REC_EVENT_PICT_STRUCT_SHIFT)
#define     DMX_REC_EVENT_PICT_STRUCT_TOP_FIELD       0x1
#define     DMX_REC_EVENT_PICT_STRUCT_BOT_FIELD       0x2
#define     DMX_REC_EVENT_PICT_STRUCT_FRAME           0x3
#define  DMX_REC_EVENT_REPEAT_FIRST_FIELD_SHIFT       23          
#define  DMX_REC_EVENT_REPEAT_FIRST_FIELD_MASK        (1UL << DMX_REC_EVENT_REPEAT_FIRST_FIELD_SHIFT)
#define  DMX_REC_EVENT_SEQ_EXT_SHIFT                  22
#define  DMX_REC_EVENT_SEQ_EXT_MASK                   (1UL << DMX_REC_EVENT_SEQ_EXT_SHIFT)
#define  DMX_REC_EVENT_PICT_SHIFT                     20
#define  DMX_REC_EVENT_PICT_MASK                      (1UL << DMX_REC_EVENT_PICT_SHIFT)
#define  DMX_REC_EVENT_SEQ_SHIFT                      19
#define  DMX_REC_EVENT_SEQ_MASK                       (1UL << DMX_REC_EVENT_SEQ_SHIFT)
#define  DMX_REC_EVENT_PCE_SHIFT                      18
#define  DMX_REC_EVENT_PCE_MASK                       (1UL << DMX_REC_EVENT_PCE_SHIFT)
#define  DMX_REC_EVENT_PROG_SEQ_SHIFT                 17
#define  DMX_REC_EVENT_PROG_SEQ_MASK                  (1UL << DMX_REC_EVENT_PROG_SEQ_SHIFT)
#define     DMX_REC_EVENT_1_PROG_FRAME                0x0
#define     DMX_REC_EVENT_2_PROG_FRAMES               0x1
#define     DMX_REC_EVENT_3_PROG_FRAMES               0x3
#define  DMX_REC_EVENT_FRAME_RATE_SHIFT               13
#define  DMX_REC_EVENT_FRAME_RATE_MASK                (0xfUL << DMX_REC_EVENT_FRAME_RATE_SHIFT)
#define     DMX_REC_EVENT_FRAME_RATE_FORBIDDEN        0x0
#define     DMX_REC_EVENT_FRAME_RATE_23_976           0x1
#define     DMX_REC_EVENT_FRAME_RATE_24               0x2
#define     DMX_REC_EVENT_FRAME_RATE_25               0x3
#define     DMX_REC_EVENT_FRAME_RATE_29_97            0x4
#define     DMX_REC_EVENT_FRAME_RATE_30               0x5
#define     DMX_REC_EVENT_FRAME_RATE_50               0x6
#define     DMX_REC_EVENT_FRAME_RATE_59_94            0x7
#define     DMX_REC_EVENT_FRAME_RATE_60               0x8
#define  DMX_REC_EVENT_PID_SHIFT                      0
#define  DMX_REC_EVENT_PID_MASK                       (0x1fffUL << DMX_REC_EVENT_PID_SHIFT)

/*
 * Demux Event GOP Time Code
 */
#define  DMX_REC_EVENT_GTC_DROP_FRAME_SHIFT           31
#define  DMX_REC_EVENT_GTC_DROP_FRAME_MASK            (1UL << DMX_REC_EVENT_GTC_DROP_FRAME_SHIFT)
#define  DMX_REC_EVENT_GTC_HOUR_SHIFT                 26
#define  DMX_REC_EVENT_GTC_HOUR_MASK                  (0x1fUL <<  DMX_REC_EVENT_GTC_HOUR_SHIFT)
#define  DMX_REC_EVENT_GTC_MIN_SHIFT                  20
#define  DMX_REC_EVENT_GTC_MIN_MASK                   (0x3fUL << DMX_REC_EVENT_GTC_MIN_SHIFT)
#define  DMX_REC_EVENT_GTC_MARKER_BIT_SHIFT           19
#define  DMX_REC_EVENT_GTC_MARKER_BIT_MASK            (1UL << DMX_REC_EVENT_GTC_MARKER_BIT_SHIFT)
#define  DMX_REC_EVENT_GTC_SEC_SHIFT                  13
#define  DMX_REC_EVENT_GTC_SEC_MASK                   (0x3fUL << DMX_REC_EVENT_GTC_SEC_SHIFT)
#define  DMX_REC_EVENT_GTC_PICTURES_SHIFT             7
#define  DMX_REC_EVENT_GTC_PICTURES_MASK              (0x3fUL << DMX_REC_EVENT_GTC_PICTURES_SHIFT)

/* PVR Record notification events */
typedef enum _dmx_rec_event_t
{
    /*
     * A block of size notify_block_size bytes are available in the
     * transport buffer for reading by the record driver.
     */
    DMX_REC_TS_BUFFER_INTR,
    /*
     * The transport buffer has completely filled and incoming transport
     * packets are currently being thrown away by the demux.
     */
    DMX_REC_TS_BUFFER_FULL,
    /*
     * One or more events are available for reading from the event buffer. 
     */
    DMX_REC_EVT_BUFFER_INTR,
    /*
     * The event buffer has completely filled and events corresponding
     * to the incoming transport stream are currently being thrown away
     * by the demux.
     */
    DMX_REC_EVT_BUFFER_FULL
} dmx_rec_event_t;

/********************************************************************/
/*  FUNCTION:     dmx_rec_notify_fct_t                              */
/*                                                                  */
/*  DESCRIPTION:                                                    */
/*    This is the form of the callback function to provide the      */
/*    client with PVR related transport buffer and event buffer     */
/*    notifications. This function is to be implemented by the      */
/*    demux driver PVR client. The client can implement one         */
/*    callback for both transport and event buffer notifications,   */
/*    or a separate one for each type of notification. The client   */
/*    callback function(s) are registered using the demux driver    */
/*    functions cnxt_dmx_set_transport_buffer and                   */
/*    cnxt_dmx_set_event_buffer.                                    */
/*                                                                  */
/*  PARAMETERS:                                                     */
/*      dmxid   -   Demux instance                                  */
/*      tag     -   32 bit user\-defined tag from the               */
/*                  cnxt_dmx_set_transport_buffer or                */
/*                  cnxt_dmx_set_event_buffer functions             */
/*      event   -   One of several possible notifications, see      */
/*                  table below                                     */
/*                                                                  */
/*  CONTEXT:                                                        */
/*    Will be called in interrupt context.                          */
/*                                                                  */
/*  RETURN VALUES:                                                  */
/*      DMX_STATUS_OK - Callback completed successfully.            */
/*      DMX_BAD_DMXID - The demux ID passed to the callback did not */
/*                      match correlate to the tag provided.        */
/*      DMX_ERROR     - An internal error occurred, an invalid tag  */
/*                      was passed to the callback, or an           */
/*                      unrecognized event was passed.              */
/*                                                                  */
/*  NOTES:                                                          */
/*    The client should minimize the work done in this call, must   */
/*    not block, and should return as quickly as possible.          */
/*                                                                  */
/*    The event parameter can have one of the following values:     */
/*    <TABLE>                                                       */
/*    Event Value               \Description                        */
/*    -----------               -----------                         */
/*    DMX_REC_TS_BUFFER_INTR    A block of size notify_block_size   */
/*                               bytes are available in the         */
/*                               transport buffer for reading by    */
/*                               the record driver.                 */
/*    DMX_REC_TS_BUFFER_FULL    The transport buffer has completely */
/*                               filled and incoming transport      */
/*                               packets are currently being        */
/*                               thrown away by the demux.          */
/*    DMX_REC_EVT_BUFFER_INTR   One or more events are available    */
/*                               for reading from the event buffer. */
/*    DMX_REC_EVT_BUFFER_FULL   The event buffer has completely     */
/*                               filled and events corresponding to */
/*                               the incoming transport stream are  */
/*                               currently being thrown away by     */
/*                               the demux.                         */
/*    </TABLE>                                                      */
/********************************************************************/
typedef DMX_STATUS (*dmx_rec_notify_fct_t)(u_int32 dmxid, u_int32 tag, dmx_rec_event_t event);

typedef enum _dmx_encode_buffer_select_t
{
    DMX_VIDEO_ENC_BUFFER_SEL,
    DMX_MPEG_AUDIO_ENC_BUFFER_SEL,
    DMX_AC3_AUDIO_ENC_BUFFER_SEL
} dmx_encode_buffer_select_t;

DMX_STATUS cnxt_dmx_set_transport_buffer(
   u_int32              dmxid, 
   u_int8               *buffer,
   u_int32              size, 
   u_int32              tag,
   u_int32              notify_block_size,
   dmx_rec_notify_fct_t notify_function);
DMX_STATUS cnxt_dmx_get_transport_buffer_ptrs(
   u_int32  dmxid,
   u_int8   **start, 
   u_int8   **end,
   u_int8   **read, 
   u_int8   **write);
DMX_STATUS cnxt_dmx_set_event_buffer(
   u_int32              dmxid, 
   u_int8               *buffer,
   u_int32              size, 
   u_int32              tag,
   dmx_rec_notify_fct_t notify_function);
DMX_STATUS cnxt_dmx_get_event_buffer_ptrs(
   u_int32  dmxid,
   u_int8   **start, 
   u_int8   **end,
   u_int8   **read, 
   u_int8   **write);
DMX_STATUS cnxt_dmx_transport_bytes_recorded(u_int32 dmxid, u_int32 bytes);
DMX_STATUS cnxt_dmx_events_recorded(u_int32 dmxid, u_int32 bytes);
#endif /* PVR == YES */

#if DMXDMA == YES
typedef enum
{
    DMX_BIG_ENDIAN,
    DMX_LITTLE_ENDIAN
} DMX_ENDIANESS;

/********************************************************************/
/*  FUNCTION:     dmx_dma_input_notify_fct_t                        */
/*                                                                  */
/*  DESCRIPTION:                                                    */
/*    This function is to be implemented by the client.  This is    */
/*    the DMA input extended functionality callback prototype.  It  */
/*   is used to indicate a memory buffer has been DMA'ed to a demux */
/*   and that the buffer can be re-used by the client.              */
/*                                                                  */
/*  PARAMETERS:                                                     */
/*      dmxid       -   Demux instance                              */
/*      tag         -   32 bit user-defined tag from the            */
/*                      cnxt_dmx_dma_input_add_buffer function      */
/*      buffer      -   Memory buffer that can be re-used           */
/*      size        -   Size (in bytes) of the memory buffer        */
/*                                                                  */
/*  CONTEXT:                                                        */
/*    Will be called in interrupt context.                          */
/*                                                                  */
/*  RETURN VALUES:                                                  */
/*      0                 - Success                                 */
/*      non\-0            - Error                                   */
/*                                                                  */
/*  NOTES:                                                          */
/*    The client should minimize the work done in this call, must   */
/*    not block, and should return as quickly as possible.          */
/********************************************************************/
typedef u_int32 (*dmx_dma_input_notify_fct_t)(u_int32 dmxid,
                                              u_int32 tag,
                                              u_int8 *buffer,
                                              u_int32 size);
DMX_STATUS cnxt_dmx_set_input_device(u_int32 dmxid, u_int16 input_device);
DMX_STATUS cnxt_dmx_set_dma_input_endianess(u_int32 dmxid, DMX_ENDIANESS endianess);
DMX_STATUS cnxt_dmx_set_dma_input_channel(u_int32 dmxid,
                                          u_int32 channel);
DMX_STATUS cnxt_dmx_set_dma_input_notify_fct(u_int32 dmxid,
                                             dmx_dma_input_notify_fct_t notify_fct);
DMX_STATUS cnxt_dmx_dma_input_add_buffer(u_int32 dmxid,
                                         u_int32 tag,
                                         u_int8 *buffer,
                                         u_int32 size);
DMX_STATUS cnxt_dmx_dma_input_start(u_int32 dmxid);
DMX_STATUS cnxt_dmx_dma_input_stop(u_int32 dmxid, bool immediate);
DMX_STATUS cnxt_dmx_dma_input_reset(u_int32 dmxid, bool immediate);
#endif /* DMXDMA == YES */

#if PARSER_PASSAGE_ENABLE==YES
DMX_STATUS cnxt_dmx_set_passage_mode ( u_int32 dmxid, u_int32 mode );
DMX_STATUS cnxt_dmx_set_shadow_pid ( u_int32 dmxid, u_int16 vid_pid, u_int16 aud_pid );
#endif

/****************************************************************************
	方法一: 为了防止如下情况下死锁:

	proj-demux给dvbcore送数据的时候, 先把sem lock起来; 这时底层
	demux要送数据过来(而此时也lock了自己的sem), 只好等ipanel
	进程处理完, 处于挂起状态; 而此时如果dvbcore要add filter, 会
	立即调用跟底层demux有关的函数--而此类函数往往也要
	lock sem, 如果跟notify使用的是同一个sem,那这里就死锁了!

	所以特增加一个标志位，如果Notify 正在送数据给iPanel 时，
	此时Add / Remove filter 时先释放此信号量，机率虽小，但总有
	发生的时候了!!!@@@@@  ^~^ 
****************************************************************************/
#define IPANEL_SEM_LOCKED_TEST1   // 使用自解锁方式

#ifdef IPANEL_SEM_LOCKED_TEST1 // 测试Notify 与Add/Remove channel/filter 共同信号量互锁
extern int flag_pop_sec_to_callback; 
#endif

/****************************************************************************
	方法二:  使用对DataNotify 不实行保护方式
	不对DataNotify 函数进行保护，分批保护数据
	需要定义部分局部变量，从而不影响到全局
	变量gDemuxInfo[]  管理的数据资源。
****************************************************************************/
//#define IPANEL_SEM_LOCKED_TEST2   // 测试Notify 与Add/Remove channel/filter 共同信号量互锁

/****************************************************************************
* Modifications:
* $Log: 
*  47   mpeg      1.46        6/15/04 5:28:03 PM     Tim White       CR(s) 9474
*         9475 : Changed pvr to rec for consistency.
*        
*  46   mpeg      1.45        6/15/04 11:35:04 AM    Joe Kroesche    CR(s) 9474
*         9475 : added function comments for callback functions, to aid 
*        automated
*        documentation tool
*  45   mpeg      1.44        6/14/04 4:10:46 PM     Tim Ross        CR(s) 9456
*         9457 : Initial debug of demux-record event style translation.
*  44   mpeg      1.43        5/21/04 5:09:18 PM     Tim Ross        CR(s) 9282
*         9283 : Added definition of new demux sequence extension bit and moved
*         progressive
*        frame event bit.
*  43   mpeg      1.42        5/17/04 3:17:44 PM     Larry Wang      CR(s) 9215
*         9216 : Restore Rev 1.40
*  42   mpeg      1.41        5/17/04 3:14:00 PM     Larry Wang      CR(s) 9215
*         9216 : This revision = Rev1.38 + ( Rev1.40 - Rev1.39).
*  41   mpeg      1.40        5/17/04 9:06:43 AM     Larry Wang      CR(s) 9212
*         9213 : Remove XTV related definition.
*  40   mpeg      1.39        5/14/04 4:11:09 PM     Tim Ross        CR(s) 9205
*         9206 : Added demux record microcode event structure definitions.
*  39   mpeg      1.38        4/21/04 11:10:10 AM    Larry Wang      CR(s) 8868
*         8869 : Add proto-type for cnxt_dmx_xtv_check_payload_mask().
*  38   mpeg      1.37        4/20/04 11:02:33 AM    Larry Wang      CR(s) 8868
*         8869 : add more APIs for RASPLDI.
*  37   mpeg      1.36        4/15/04 12:05:26 PM    Larry Wang      CR(s) 8868
*         8869 : Add prototypes of new APIs in demuxxtv.c.
*  36   mpeg      1.35        3/23/04 3:11:14 PM     Larry Wang      CR(s) 8638
*         8639 : (1) add more channel types for XTV; (2) add demux extension 
*        API for XTV.
*  35   mpeg      1.34        3/19/04 2:37:17 PM     Tim Ross        CR(s) 8545
*         : Added DMX_ENDIANESS typedef & cnxt_dmx_set_dma_input_endianess() 
*        prototype.
*  34   mpeg      1.33        3/16/04 3:04:29 PM     Tim White       CR(s) 8545
*         : Added cnxt_dmx_get_timebase_offset() function and moved 
*        cnxt_dmx_set_input_device()
*        function from demuxapi.c to demuxdma.c.
*        
*  33   mpeg      1.32        3/10/04 2:54:59 PM     Larry Wang      CR(s) 8551
*         : Add Sony Passage demux API proto-types.
*  32   mpeg      1.31        3/10/04 10:41:16 AM    Bob Van Gulick  CR(s) 8546
*         : Add support to return PES frame rate
*  31   mpeg      1.30        3/2/04 10:54:54 AM     Tim Ross        CR(s) 8451
*         : Added PVR record extensions.
*  30   mpeg      1.29        2/24/04 2:43:07 PM     Bob Van Gulick  CR(s) 8427
*         : Add timebase_offset functionality to demux
*        
*  29   mpeg      1.28        1/12/04 6:40:23 PM     Yong Lu         CR(s) 8078
*         : added little endian input support
*  28   mpeg      1.27        11/25/03 3:50:59 PM    Tim White       CR(s): 
*        8027 Drop the Demux DMA Input Extension function.
*        
*  27   mpeg      1.26        11/19/03 10:13:40 AM   Tim White       CR(s): 
*        7987 Added PVR & DMA demux extension support for PVR Phase 1.
*        
*  26   mpeg      1.25        10/17/03 9:47:10 AM    Tim White       CR(s): 
*        7674 Change Header to Id.
*        
*  25   mpeg      1.24        10/15/03 5:11:29 PM    Tim White       CR(s): 
*        7659 Add Demux PVR Extensions file.
*        
*  24   mpeg      1.23        9/22/03 4:51:38 PM     Bob Van Gulick  SCR(s) 
*        7519 :
*        Add support for DirecTV CAPs
*        
*        
*  23   mpeg      1.22        8/27/03 11:03:00 AM    Bob Van Gulick  SCR(s) 
*        7387 :
*        Add support for CWP processing in DirecTV
*        
*        
*  22   mpeg      1.21        6/9/03 6:03:34 PM      Bob Van Gulick  SCR(s) 
*        6755 :
*        Add support for 8 slot descram in demux.  Also change use of 
*        DESC_CHANNELS to DPS_NUM_DESCRAMBLERS
*        
*        
*  21   mpeg      1.20        5/2/03 11:14:36 AM     Bob Van Gulick  SCR(s) 
*        6151 :
*        Add pts_offset and dma_mux memory locations
*        
*        
*  20   mpeg      1.19        4/24/03 4:53:40 PM     Tim White       SCR(s) 
*        6097 :
*        Allow 6 descrambled simultaneous PES channels.  Remove #ifndef 
*        USE_OLD_PES code.
*        
*        
*  19   mpeg      1.18        4/15/03 3:51:40 PM     Dave Wilson     SCR(s) 
*        6025 :
*        Added prototypes for cnxt_dmx_register/query_event6_notify.
*        
*  18   mpeg      1.17        4/15/03 11:45:10 AM    Dave Wilson     SCR(s) 
*        6021 :
*        Added prototype for cnxt_dmx_query_pcr_notify. This allows a client to
*        retrieve a pointer to a function currently hooked to receive PCR 
*        notifications
*        and enables chaining of these notifications.
*        
*  17   mpeg      1.16        4/11/03 3:58:48 PM     Brendan Donahe  SCR(s) 
*        6010 :
*        Added prototype for cnxt_dmx_query_status().
*        
*        
*  16   mpeg      1.15        4/11/03 1:01:38 PM     Brendan Donahe  SCR(s) 
*        5997 :
*        Added defines for supporting demux microcode capability checking by 
*        query_channel_scrambling in demuxdesc.c.
*        
*        
*  15   mpeg      1.14        4/10/03 5:03:30 PM     Dave Wilson     SCR(s) 
*        5990 :
*        Added API cnxt_dmx_set_section_channel_tag
*        
*  14   mpeg      1.13        4/2/03 11:15:50 AM     Brendan Donahe  SCR(s) 
*        5886 :
*        Added versioning defines to allow for easy checking of microcode 
*        version 
*        within demuxhw.c.
*        
*        
*  13   mpeg      1.12        2/4/03 12:56:14 PM     Dave Wilson     SCR(s) 
*        5399 :
*        Added prototype for API cnxt_dmx_empty_pes_buffer().
*        
*  12   mpeg      1.11        11/20/02 3:58:28 PM    Bob Van Gulick  SCR(s) 
*        4998 :
*        Add register_pcr_notify_callback function to demux api.
*        
*        
*  11   mpeg      1.10        11/12/02 5:26:18 PM    Bob Van Gulick  SCR(s) 
*        4945 :
*        Add prototype for descrambler query function
*        
*        
*  10   mpeg      1.9         10/16/02 3:27:04 PM    Bob Van Gulick  SCR(s) 
*        4799 :
*        Remove CANAL_PLUS_FILTERING #ifdefs and use #if PARSER_FILTERING == 
*        FILTER_xxx
*        instead.  PARSER_FILTERING is defined in the sw config
*        
*        
*  9    mpeg      1.8         9/19/02 3:38:28 PM     Joe Kroesche    SCR(s) 
*        4610 :
*        modification to support crc notification method, undoes previous 
*        changes
*        for crc notification
*        
*  8    mpeg      1.7         9/18/02 4:27:12 PM     Joe Kroesche    SCR(s) 
*        4619 :
*        added conditionally compiled globals needed for canal+ special cases
*        
*  7    mpeg      1.6         9/5/02 6:30:54 PM      Bob Van Gulick  SCR(s) 
*        4530 :
*        Change CRC check to use Header Notify instead of Section Notify
*        
*        
*  6    mpeg      1.5         8/30/02 2:40:38 PM     Bob Van Gulick  SCR(s) 
*        4485 :
*        Add defines to support CRC checking in demux driver
*        
*        
*  5    mpeg      1.4         8/16/02 6:02:16 PM     Tim White       SCR(s) 
*        4420 :
*        Added dmxid parameter to DVR event handler callback function 
*        prototype.
*        
*        
*  4    mpeg      1.3         6/27/02 5:55:46 PM     Tim White       SCR(s) 
*        4108 :
*        Convert MHP glue layer to use new DEMUX driver.
*        
*        
*  3    mpeg      1.2         4/26/02 3:20:54 PM     Tim White       SCR(s) 
*        3562 :
*        Use LEGACY_DVR instead of obsolete DVR hwconfig option.
*        
*        
*  2    mpeg      1.1         12/19/01 9:47:12 AM    Bob Van Gulick  SCR(s) 
*        2977 :
*        Change cnxt_dmx_reset and cnxt_dmx_close to have u_int32 dmxid's
*        
*        
*  1    mpeg      1.0         12/18/01 10:12:06 AM   Bob Van Gulick  
* $
****************************************************************************/


