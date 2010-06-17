/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*          Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002          */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        gendemux.h
 *
 *
 * Description:     Generic Demux Driver Header File
 *
 *
 * Author:          Tim White
 *
 ****************************************************************************/
/* $Header: gendemux.h, 60, 3/17/03 11:31:54 AM, Matt Korte$
 ****************************************************************************/

/***************************************Public Section*****************************************************/
#if CUSTOMER == VENDOR_B
#define PARSER_FILTERING FILTER_888  
#endif




/******
 ******   Once the new PES per slot buffer interface is implemented in the
 ******   parser micrcode, remove this #define and any code which references it.
 ******/

#define USE_OLD_PES





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

#define DESC_CHANNELS           6

#define GEN_MAX_CLIENTS         5
#define GEN_NULL_PID            0x1FFF
#define GEN_INVALID_PID         0xFFFF
#define GEN_DEFAULT_CLIENT_ID   0
#define GEN_INVALID_PID         0xFFFF

#define GENDMX_DEFAULT_PES_INTR_FREQ       1024

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
    PSI_CHANNEL_TYPE
} genchannel_t;

typedef enum
{
    GEN_DEMUX_DISABLE = 0,
    GEN_DEMUX_ENABLE,
    GEN_DEMUX_RESET,
    GEN_DEMUX_DISABLE_RESET
} gencontrol_channel_t;

typedef struct _gen_notify_t
{
    // Set by the generic demux driver
    u_int8      *pData;                     /* pointer to the header data or block data*/
    u_int32     condition;
    u_int32     chid;

    // set by the client demux driver in header callback
    u_int8      *write_ptr;
    u_int32     skip;
    u_int32     length;
    u_int32     tag;

} gen_notify_t;
typedef struct _gen_notify_t *pgen_notify_t;

#ifdef MHP
typedef genfilter_mode (*gen_section_callback_fct_t)( pgen_notify_t pdata ,u_int32 fid);
#endif
typedef genfilter_mode (*gen_callback_fct_t)( pgen_notify_t pdata );


typedef void (*gen_pcr_callback_fct_t)(u_int32 PCR_High, u_int32 PCR_Low);
typedef void (*dvr_notify_fct_t)(u_int32 Event);

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

bool gen_dmx_init(bool ReInit);
void gen_dmx_shutdown(void);
u_int32 gen_dmx_get_number_of_channels_available(bool desc_capable, u_int32 number_of_filters, u_int32 size_of_filter);
u_int32 gen_dmx_allocate_channel(u_int32 ClientID, genchannel_t channel_type, u_int32 number_of_filters, bool scrambling_cap);
u_int32 gen_dmx_free_channel(u_int32 ClientID, u_int32 chid);
u_int32 gen_dmx_set_channel_buffer(u_int32 ClientID, u_int32 chid, void *buffer, u_int32 size);
u_int32 gen_dmx_get_channel_buffer_pointer(u_int32 ClientID, u_int32 chid, u_int8 **write_ptr, u_int32 *buffer_size);
u_int32 gen_dmx_get_channel_write_pointer(u_int32 ClientID, u_int32 chid, u_int8 **write_ptr);
u_int32 gen_dmx_allocate_section_filter(u_int32 ClientID, u_int32 chid, u_int32 filter_size);
u_int32 gen_dmx_get_filter_chid(u_int32 fid);
u_int32 gen_dmx_get_channel_filter(u_int32 chid);
u_int32 gen_dmx_get_allocated_filters(u_int32 chid);
bool    gen_dmx_channel_enabled(u_int32 chid);
u_int32 gen_dmx_free_section_filter(u_int32 ClientID, u_int32 chid, u_int32 fid);
bool    gen_dmx_query_channel_scrambling(u_int32 ClientID, u_int32 chid);
#ifdef MHP
u_int32 gen_dmx_register_section_channel_notifications(u_int32 ClientID, u_int32 chid, gen_callback_fct_t header_notify, gen_section_callback_fct_t section_notify, u_int32 MinHdrSize, u_int32 TimeOut);
#else
u_int32 gen_dmx_register_section_channel_notifications(u_int32 ClientID, u_int32 chid, gen_callback_fct_t header_notify, gen_callback_fct_t section_notify, u_int32 MinHdrSize, u_int32 TimeOut);
#endif
//u_int32 gen_dmx_set_section_channel_attributes(u_int32 ClientID, u_int32 chid, gen_callback_fct_t header_notify, gen_callback_fct_t section_notify, u_int32 TimeOut, u_int32 MinHdrSize);
//u_int32 gen_dmx_set_pes_channel_attributes(u_int32 ClientID, u_int32 chid, gen_callback_fct_t data_notify, gen_callback_fct_t error_notify, u_int32 frequency);
u_int32 gen_dmx_register_pcr_notify(u_int32 ClientID, gen_pcr_callback_fct_t pcr_notify);

u_int32 gen_dmx_buffers_inquire(u_int32 ClientID, u_int32 chid, u_int8 **beg, u_int8 **end);
void gen_dmx_section_unlock(u_int32 ClientID, u_int32 chid, u_int8 *buffer, u_int32 length);
u_int32 gen_dmx_set_section_filter(u_int32 ClientID, u_int32 chid, u_int32 fid, u_int8 *filter, u_int8 *mask, u_int8 *notmask);
u_int32 gen_dmx_set_ext_filter(u_int32 ClientID, u_int32 chid, u_int32 fid, u_int8 *filter, u_int8 *mask, u_int32 size, u_int32 start_offset);
u_int32 gen_dmx_set_version_filter(u_int32 ClientID, u_int32 chid, u_int32 fid, u_int8 version,
                                   bool version_equal, bool enable);

u_int32 gen_dmx_control_filter(u_int32 ClientID, u_int32 chid, u_int32 fid, gencontrol_channel_t enable_disable);
u_int32 gen_dmx_set_channel_pid(u_int32 ClientID, u_int32 chid, u_int16 pid);
u_int16 gen_dmx_get_channel_pid(u_int32 ClientID, u_int32 chid);

int gen_dmx_control_channel(u_int32 ClientID, u_int32 chid, gencontrol_channel_t channel_command);
int gen_dmx_control_av_channel(u_int32 ClientID, u_int32 chid, gencontrol_channel_t channel_command);

int gen_dmx_read_pes_buffer(u_int32 ClientID, u_int32 chid, u_int32 size, u_int8 *buffer);
bool gen_dmx_set_pcr_pid(u_int16 pid);
int gen_dmx_get_current_pcr(genpcr_t *current_pcr);
void gen_demux_connect_notify(int event_type);
u_int32 gen_dmx_get_audio_buff_size(void);
u_int32 gen_dmx_get_video_buff_size(void);
u_int16 gen_dmx_get_video_pid( bool bHardware );
u_int16 gen_dmx_get_audio_pid( void );
void genDemuxReInitHW(void);
//void demux_set_xprt_buffer(void *buffer, size_t size);
u_int32 gen_demux_get_callback_handle(void);

/* ACTV APIs */
void gen_dmx_set_standby_video(u_int16 VideoPID);
void gen_dmx_set_standby_audio(u_int16 AudioPID);
void gen_dmx_enable_video_switch(void);
void gen_dmx_enable_audio_switch(void);
u_int32 gen_dmx_get_video_switch_status(void);
u_int32 gen_dmx_get_audio_switch_status(void);


/* Descrambler API */
u_int32 gen_descrambler_init(void);
u_int32 gen_descrambler_open(u_int32 ClientID, genchannel_t type);
u_int32 gen_descrambler_close(u_int32 ClientID, u_int32 descrambler);
u_int32 gen_descrambler_set_pid(u_int32 ClientID, u_int32 descrambler, u_int16 pid);
u_int32 gen_descrambler_set_keys(u_int32 ClientID, u_int32 descrambler, u_int32 odd_key_length, const char * odd_key,
                                 u_int32 even_key_length, const char * even_key);

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
void gen_dmx_dvr_set_highwater_mark(u_int32 HighWater_Mark);
void gen_dmx_dvr_set_mode(u_int32 Mode);
void gen_dmx_dvr_register_notification_handlers (dvr_notify_fct_t EventHandler,
                                                 dvr_notify_fct_t DataHandler,
                                                 dvr_notify_fct_t OverflowHandler);
int gen_dmx_read_dvr_buffer(u_int32 ClientID, gendmx_dvr_channel_t channel,
                                          u_int32 *size, u_int8 *buffer, u_int8 **addr);
#endif

/***************************************Private Section*****************************************************/
#ifdef _GENDMXPRIV

#define DEMUX_ISR_BITS      (DPS_EVENT_INTERRUPT|DPS_BITSTREAM_IN_FULL)

#define DEMUX_XPRT_EVENTS   (DPS_TRANSPORT_BLOCK_STORED|DPS_TRANSPORT_BUFFER_FULL)
#define DEMUX_BLOCK_EVENTS (DPS_PSI_BLOCK_STORED|DPS_PES_BLOCK_STORED|DPS_PSI_BUFFER_FULL|DPS_PES_BUFFER_FULL)

#if (LEGACY_DVR==YES)
#define DEMUX_DVR_EVENT_BITS  (DPS_DVR_VIDEO_INTR|DPS_DVR_AUDIO_INTR|DPS_DVR_AUDIO1_INTR|    \
                         DPS_DVR_AUDIO2_INTR|DPS_DVR_VIDEO_OVERFLOW|DPS_DVR_AUDIO_OVERFLOW|  \
                         DPS_DVR_AUDIO1_OVERFLOW|DPS_DVR_AUDIO2_OVERFLOW|DPS_DVR_EVENT_INTR| \
                         DPS_DVR_EVENT_FULL)
#define DEMUX_EVENT_BITS    (DPS_SYNC_ERROR|DEMUX_BLOCK_EVENTS|DEMUX_XPRT_EVENTS|DEMUX_DVR_EVENT_BITS)
#else
#define DEMUX_SPLICE_EVENTS (DPS_VIDEO_SPLICED|DPS_AUDIO_SPLICED)
#define DEMUX_EVENT_BITS    (DPS_SYNC_ERROR|DEMUX_BLOCK_EVENTS|DEMUX_XPRT_EVENTS|DPS_VIDEO_SPLICED|DPS_AUDIO_SPLICED)
#endif

//#define DEMUX_EVENT_BITS    (DEMUX_BLOCK_EVENTS|DEMUX_XPRT_EVENTS|DPS_VIDEO_SPLICED|DPS_AUDIO_SPLICED)

#define CHANNEL_FREE            0
#define CHANNEL_IN_USE          1

#define FRAME_SIZE              192
#if PARSER_TYPE==DTV_PARSER
#define TS_COPY_BLOCK_SIZE      130
#else
#define TS_COPY_BLOCK_SIZE      188
#endif

#define TOTAL_FILTERS           32

#define PES_CHANNEL             0x02000
#define RAW_PES_CHANNEL         0x04000

typedef struct _ChannelInformationStruc
{
        u_int16 PID;
        u_int32 Slot;                       // HW PID Slot
        genchannel_t stype;
        gen_notify_t NotifyData;
        u_int32 client_id;                  // set on channel allocation
        u_int32 FilterEnable;
        u_int32 FiltersAllocated;
        bool DisabledByFilter;
        bool InUseFlag;
        bool DemuxEnable;
        u_int32 NumOFilters;
        u_int32 Overflows;
        u_int32 OverflowsHandled;
        u_int8 *pBuffer;
        u_int8 *pBufferEnd;
        u_int8 *pWritePtr;
        u_int8 *pReadPtr;
        u_int8 *pAckPtr;
        gen_callback_fct_t HdrErrNotify;
#ifdef MHP
        gen_section_callback_fct_t DataNotify;
#else
        gen_callback_fct_t DataNotify;
#endif
        u_int32 HdrSize;
        u_int8 *HdrArea;
        u_int8 *HdrAlloc;
        tick_id_t ChannelTimer;
        u_int32 TimerNotifyCount;
        u_int32 NotifyCount;
        bool PESChannel;
        u_int32 PESDataCount;
        u_int32 CurrentFilterId;
        genfilter_mode LoopMode;            // One Shot, loop, toggle
        u_int32 TimeOutMS;
#ifdef USE_OLD_PES
        u_int32 CCounter;                   // Continuity Counter
#endif
// debug stuff
        int Notify_Unlock;
        int NotifyCalled;
} ChannelInformationStruc;

typedef struct _FilterStruc
{
        u_int32 OwnerChid;
        u_int32 FilterSize;
        u_int32 ExtFilterSize;
        bool FilterEnabled;
        bool VersionEqual;
        bool NewFilter;
        u_int32 Match[GENDMX_MAX_HW_FILTER_SIZE/4];
        u_int32 Mask[GENDMX_MAX_HW_FILTER_SIZE/4];
        u_int32 NotMask[GENDMX_MAX_HW_FILTER_SIZE/4];
        bool    NotMaskZero;
        bool ExtFilterEnabled;
        u_int32 ExtMatch[GENDMX_EXT_FILTER_SIZE/4];
        u_int32 ExtMask[GENDMX_EXT_FILTER_SIZE/4];
        u_int32 ExtFilterOffset;
        // debug stuff
        u_int32 OldMatch[3];
        u_int32 OldMask[3];
} FilterStruc;

typedef struct {
        bool Open;
        bool DescDisabled;
        u_int32 chid;
        u_int16 pid;
} desc_info;

typedef struct {
        u_int32 OddKeyEnable;
        u_int32 EvenKeyEnable;
        LPDPS_KEY_ENTRY lpKey;
        bool FirstKeySet;
} desc_key_info;

#endif

#if (CANAL_AUDIO_HANDLING == YES)
/* Global variables used to monitor arrival of audio/video packets */
/* under Media Highway.                                            */
extern u_int32 gdwVideoPktsReceived;
extern u_int32 gdwAudioPktsReceived;
#endif

/****************************************************************************
 * Modifications:
 * $Log: 
 *  60   mpeg      1.59        3/17/03 11:31:54 AM    Matt Korte      SCR(s) 
 *        5777 :
 *        Change to DTV and DIRECTV
 *        
 *  59   mpeg      1.58        3/3/03 10:55:32 AM     Larry Wang      SCR(s) 
 *        5631 :
 *        Define TS_COPY_BLOCK_SIZE to be 130 if PARSER_TYPE is DSS_PARSER.
 *        
 *  58   mpeg      1.57        12/11/02 3:06:40 PM    Bob Van Gulick  SCR(s) 
 *        5121 :
 *        Rename scrambler query function to gen_dmx_query_channel_scrambling
 *        
 *        
 *  57   mpeg      1.56        11/12/02 5:26:12 PM    Bob Van Gulick  SCR(s) 
 *        4945 :
 *        Add prototype for descrambler query function
 *        
 *        
 *  56   mpeg      1.55        10/16/02 3:27:00 PM    Bob Van Gulick  SCR(s) 
 *        4799 :
 *        Remove CANAL_PLUS_FILTERING #ifdefs and use #if PARSER_FILTERING == 
 *        FILTER_xxx
 *        instead.  PARSER_FILTERING is defined in the sw config
 *        
 *        
 *  55   mpeg      1.54        10/2/02 4:42:20 PM     Bob Van Gulick  SCR(s) 
 *        4636 :
 *        Add NewFilter to Filter struct.  Remove old CRC code.
 *        
 *        
 *  54   mpeg      1.53        9/5/02 6:52:26 PM      Bob Van Gulick  SCR(s) 
 *        4530 :
 *        Use header notify instead of section notify for CRC errors
 *        
 *        
 *  53   mpeg      1.52        8/30/02 3:30:04 PM     Bob Van Gulick  SCR(s) 
 *        4485 :
 *        Fix error on previous put
 *        
 *        
 *  52   mpeg      1.51        8/30/02 2:40:44 PM     Bob Van Gulick  SCR(s) 
 *        4485 :
 *        Add defines to support CRC checking in demux driver
 *        
 *        
 *  51   mpeg      1.50        6/12/02 5:37:50 AM     Ian Mitchell    SCR(s): 
 *        3956 
 *        Changed two API's if the application is MHP
 *        
 *  50   mpeg      1.49        5/3/02 1:56:06 PM      Dave Wilson     SCR(s) 
 *        3688 :
 *        Exported a couple of globals used in audio/video data detection for 
 *        Media
 *        Highway.
 *        
 *  49   mpeg      1.48        4/26/02 3:21:00 PM     Tim White       SCR(s) 
 *        3562 :
 *        Use LEGACY_DVR instead of obsolete DVR hwconfig option.
 *        
 *        
 *  48   mpeg      1.47        2/19/02 5:00:52 PM     Bob Van Gulick  SCR(s) 
 *        3210 :
 *        Eliminate GEN_DESC_CHANNELS define and use DESC_CHANNELS instead
 *        
 *        
 *  47   mpeg      1.46        9/28/01 4:02:56 PM     Tim White       SCR(s) 
 *        2690 :
 *        The timer handler now calls the notify functions from the PSI_Task
 *        rather than at interrupt time.
 *        
 *        
 *  46   mpeg      1.45        9/20/01 3:07:26 PM     Dave Wilson     SCR(s) 
 *        2669 :
 *        Added bHardware parameter to gen_dmx_get_video_pid.
 *        
 *  45   mpeg      1.44        9/7/01 4:40:10 PM      Tim White       SCR(s) 
 *        2592 :
 *        Added tag parameter back to pNotifyData.
 *        
 *        
 *  44   mpeg      1.43        9/6/01 2:51:36 PM      Tim White       SCR(s) 
 *        2592 :
 *        Modified gendemux PSI/PES handling for the new Per-Slot-PSI/PES
 *        buffering microcode.  This included some minor gendemux interface
 *        changes as well as a complete overhaul of the PSI/PES handling
 *        path internally.  Added a new PES Task.
 *        
 *        
 *  43   mpeg      1.42        8/21/01 2:51:52 PM     Joe Kroesche    SCR(s) 
 *        2521 2522 :
 *        added prototypes for new query functions (audio/video pids)
 *        
 *  42   mpeg      1.41        8/2/01 5:25:12 PM      Tim White       SCR(s) 
 *        2345 2346 2347 :
 *        Prep work for new verifier.
 *        
 *        
 *  41   mpeg      1.40        7/18/01 1:03:06 PM     Joe Kroesche    SCR(s) 
 *        2213 2331 :
 *        took out the last changes for the channel virtualization; added 
 *        fields for
 *        to the channel structure to support PID throttling; this is the same 
 *        as
 *        1.37.1.0
 *        
 *  40   mpeg      1.39        7/5/01 4:39:48 PM      Joe Kroesche    SCR(s) 
 *        2173 2174 :
 *        changed number of descramblers back to 6 from 8 in order to avoid the
 *        problem of running out of hardware descramblers.  This is a 
 *        workaround until
 *        the real problem of 8 vs 6 descramblers is understood
 *        
 *  39   mpeg      1.38        6/22/01 5:23:10 PM     Joe Kroesche    SCR(s) 
 *        1758 2140 :
 *        changed the number of descrambler channels that gendemux thinks it
 *        has from 6 to 8.  These are "virtual" descramblers as opposed to real
 *        physical descrambler slots (which is still 6).  Added a field to the
 *        channel info struct to indicate whether a channel is a descrambler or
 *         not.
 *        Also added prototypes for a couple of functions that I added to 
 *        gendemux
 *        to support access to the hardware pid table.
 *        
 *  38   mpeg      1.37        4/26/01 1:12:38 PM     Steve Glennon   SCR 1782:
 *        New prototype for gen_dmx_process_packets. Also renamed the old 
 *        prototype
 *        to be old_gen_dmx_process_packets as still used by ECM task.
 *        
 *        
 *  37   mpeg      1.36        4/19/01 5:38:28 PM     Steve Glennon   SCR 1740:
 *        Added prototype for new PSI buffer full task. Also removed a couple
 *        of duplicate prototypes for PSI and ECM tasks
 *        
 *        
 *  36   mpeg      1.35        4/12/01 4:43:26 PM     Amy Pratt       DCS914 
 *        Removed support for Neches.
 *        
 *  35   mpeg      1.34        4/9/01 10:05:00 AM     Tim White       DCS#1563,
 *         DCS#1652, DCS#1653 -- Added MaxUsed to keep track of amount of 
 *        BufferSpace
 *        used by the registered client of the channel.
 *        
 *  34   mpeg      1.33        3/22/01 4:39:28 PM     Tim White       DCS#1421:
 *         New otv demux.  Dynamic memory allocation, task priority leveling, 
 *        stack size changes.
 *        DCS#1468:
 *        DCS#1469:
 *        
 *  33   mpeg      1.32        2/27/01 9:57:50 AM     Ismail Mustafa  DCS #1322
 *         & 1323. Reduced ECM buffer size for NDSDEMUX cases only.
 *        
 *  32   mpeg      1.31        2/22/01 1:46:54 PM     Ismail Mustafa  DCS 
 *        #1290. ADded GENDMX_UNKOWN definition.
 *        
 *  31   mpeg      1.30        2/13/01 10:15:56 AM    Ismail Mustafa  DCS 1195.
 *         Added bool member HeaderNotifyAlreadyCalled to the
 *        ChannelInfoStruc. This is to prevent HeaderNotify from being called 
 *        more
 *        than once which seems to cause OpenTV problems in  loading apps.
 *        
 *  30   mpeg      1.29        2/6/01 2:22:52 PM      Ismail Mustafa  Added new
 *         structure member DisableByFilter and added type check_CRC_t.
 *        Changes are related to DCS #1135 but do not fix it.
 *        
 *  29   mpeg      1.28        9/29/00 6:11:42 PM     Ismail Mustafa  Added 
 *        minimum_header_size and header_notify_called.
 *        These 2 variables are used by the OpenTV Demux Driver or any other
 *        future client driver. The Generic Demux Driver totally ignores them.
 *        
 *  28   mpeg      1.27        9/15/00 4:19:40 PM     Ismail Mustafa  Added 
 *        tag2 to the Notify structure.
 *        
 *  27   mpeg      1.26        9/1/00 2:23:06 PM      Ismail Mustafa  Moved 
 *        ifdef CANAL PLUS.
 *        
 *  26   mpeg      1.25        9/1/00 11:45:40 AM     Ismail Mustafa  Added 
 *        #define CANAL_PLUS_FILTERING for VENDOR_B.
 *        
 *  25   mpeg      1.24        8/4/00 10:40:08 AM     Ismail Mustafa  Removed 
 *        BITSTREAM_OUT_FULL IRQ because of HW problem using parallel NIM and
 *        High Speed Data port.
 *        
 *  24   mpeg      1.23        7/26/00 12:39:00 PM    Lucy C Allevato Updated 
 *        prototype for DVR read function (gen_dmx_read_dvr_buffer).
 *        
 *  23   mpeg      1.22        7/14/00 5:30:00 PM     Ismail Mustafa  Added 
 *        fields to implement Canal Plus filtering.
 *        
 *  22   mpeg      1.21        7/11/00 2:35:10 PM     Tim White       Added 
 *        initial DVR support into gendmxc driver.
 *        
 *  21   mpeg      1.20        6/21/00 10:52:40 AM    Ismail Mustafa  Fixed 
 *        gen_pcr_callback_fct_t.
 *        
 *  20   mpeg      1.19        6/19/00 2:31:20 PM     Ismail Mustafa  Cleanup 
 *        VxWorks warnings.
 *        
 *  19   mpeg      1.18        5/15/00 5:16:58 PM     Ismail Mustafa  Added 
 *        DescDisabled to desc info.
 *        
 *  18   mpeg      1.17        5/11/00 10:30:34 AM    Ismail Mustafa  Changes 
 *        for new microcode.
 *        
 *  17   mpeg      1.16        4/27/00 3:52:50 PM     Ismail Mustafa  Removed 
 *        bitstream out & in full interrupt.
 *        
 *  16   mpeg      1.15        4/4/00 8:54:22 PM      Ismail Mustafa  Added 
 *        defines for error conditions and redefined -1 to 0xFFFF or 0xFFFFFFFF
 *        as appropriate.
 *        
 *  15   mpeg      1.14        3/24/00 2:26:16 PM     Ismail Mustafa  Added 
 *        parameter pid to FindChannelPID.
 *        
 *  14   mpeg      1.13        3/7/00 3:46:10 PM      Ismail Mustafa  Added 
 *        prototypes for new functions.
 *        
 *  13   mpeg      1.12        2/17/00 3:47:30 PM     Ismail Mustafa  Added 
 *        CCounter to ChannelInfoTable.
 *        
 *  12   mpeg      1.11        1/26/00 10:09:58 AM    Ismail Mustafa  Added 
 *        OldMatch OldMask for HW debug.
 *        
 *  11   mpeg      1.10        1/3/00 9:26:48 AM      Ismail Mustafa  Changed 
 *        prototypes for registration parameters.
 *        
 *  10   mpeg      1.9         12/7/99 3:56:26 PM     Ismail Mustafa  Added 
 *        defines for NULL PID etc.
 *        
 *  9    mpeg      1.8         11/8/99 4:20:00 PM     Ismail Mustafa  Added 
 *        gen_dmx_control_av_channel().
 *        
 *  8    mpeg      1.7         10/26/99 5:56:50 PM    Ismail Mustafa  Added 
 *        prototype for gen_dmx_Xprt_Task.
 *        
 *  7    mpeg      1.6         10/13/99 6:33:16 PM    Ismail Mustafa  Fixed 
 *        prototypes.
 *        
 *  6    mpeg      1.5         10/12/99 11:06:38 AM   Ismail Mustafa  Added 
 *        timeout parameter to call back registration function.
 *        
 *  5    mpeg      1.4         10/7/99 2:27:56 PM     Ismail Mustafa  Cleanup.
 *        
 *  4    mpeg      1.3         9/30/99 12:07:56 PM    Ismail Mustafa  Fixed 
 *        prototype defines for callbacks.
 *        
 *  3    mpeg      1.2         9/28/99 4:00:08 PM     Ismail Mustafa  Fixed 
 *        prototypes.
 *        
 *  2    mpeg      1.1         9/21/99 11:07:02 AM    Ismail Mustafa  Changes 
 *        for Nagra Demux.
 *        
 *  1    mpeg      1.0         9/2/99 5:40:08 PM      Ismail Mustafa  
 * $
 * 
 *    Rev 1.59   17 Mar 2003 11:31:54   kortemw
 * SCR(s) 5777 :
 * Change to DTV and DIRECTV
 * 
 *    Rev 1.58   03 Mar 2003 10:55:32   wangl2
 * SCR(s) 5631 :
 * Define TS_COPY_BLOCK_SIZE to be 130 if PARSER_TYPE is DSS_PARSER.
 * 
 *    Rev 1.57   11 Dec 2002 15:06:40   vangulr
 * SCR(s) 5121 :
 * Rename scrambler query function to gen_dmx_query_channel_scrambling
 * 
 * 
 *    Rev 1.56   12 Nov 2002 17:26:12   vangulr
 * SCR(s) 4945 :
 * Add prototype for descrambler query function
 * 
 * 
 *    Rev 1.55   16 Oct 2002 14:27:00   vangulr
 * SCR(s) 4799 :
 * Remove CANAL_PLUS_FILTERING #ifdefs and use #if PARSER_FILTERING == FILTER_xxx
 * instead.  PARSER_FILTERING is defined in the sw config
 * 
 * 
 *    Rev 1.54   02 Oct 2002 15:42:20   vangulr
 * SCR(s) 4636 :
 * Add NewFilter to Filter struct.  Remove old CRC code.
 * 
 * 
 *    Rev 1.50   Jun 12 2002 05:37:50   mitchei
 * SCR(s): 3956 
 * Changed two API's if the application is MHP
 *
 *    Rev 1.49   03 May 2002 12:56:06   dawilson
 * SCR(s) 3688 :
 * Exported a couple of globals used in audio/video data detection for Media
 * Highway.
 *
 *    Rev 1.48   26 Apr 2002 14:21:00   whiteth
 * SCR(s) 3562 :
 * Use LEGACY_DVR instead of obsolete DVR hwconfig option.
 *
 *
 *    Rev 1.47   19 Feb 2002 17:00:52   vangulr
 * SCR(s) 3210 :
 * Eliminate GEN_DESC_CHANNELS define and use DESC_CHANNELS instead
 *
 *
 *    Rev 1.46   28 Sep 2001 15:02:56   whiteth
 * SCR(s) 2690 :
 * The timer handler now calls the notify functions from the PSI_Task
 * rather than at interrupt time.
 *
 *
 *    Rev 1.45   20 Sep 2001 14:07:26   dawilson
 * SCR(s) 2669 :
 * Added bHardware parameter to gen_dmx_get_video_pid.
 *
 *    Rev 1.44   07 Sep 2001 15:40:10   whiteth
 * SCR(s) 2592 :
 * Added tag parameter back to pNotifyData.
 *
 *
 *    Rev 1.43   06 Sep 2001 13:51:36   whiteth
 * SCR(s) 2592 :
 * Modified gendemux PSI/PES handling for the new Per-Slot-PSI/PES
 * buffering microcode.  This included some minor gendemux interface
 * changes as well as a complete overhaul of the PSI/PES handling
 * path internally.  Added a new PES Task.
 *
 *
 *    Rev 1.42   21 Aug 2001 13:51:52   kroescjl
 * SCR(s) 2521 2522 :
 * added prototypes for new query functions (audio/video pids)
 *
 *    Rev 1.41   02 Aug 2001 16:25:12   whiteth
 * SCR(s) 2345 2346 2347 :
 * Prep work for new verifier.
 *
 *    Rev 1.40   18 Jul 2001 12:03:06   kroescjl
 * SCR(s) 2213 2331 :
 * took out the last changes for the channel virtualization; added fields for
 * to the channel structure to support PID throttling; this is the same as
 * 1.37.1.0
 *
 *    Rev 1.37.1.0   16 Jul 2001 22:43:20   kroescjl
 * added fields to support PID throttling as a test of concept
 *
 *    Rev 1.37   26 Apr 2001 12:12:38   glennon
 * SCR 1782:
 * New prototype for gen_dmx_process_packets. Also renamed the old prototype
 * to be old_gen_dmx_process_packets as still used by ECM task.
 *
 *    Rev 1.36   19 Apr 2001 16:38:28   glennon
 * SCR 1740:
 * Added prototype for new PSI buffer full task. Also removed a couple
 * of duplicate prototypes for PSI and ECM tasks
 *
 *    Rev 1.35   12 Apr 2001 15:43:26   prattac
 * DCS914 Removed support for Neches.
 *
 *    Rev 1.34   09 Apr 2001 09:05:00   whiteth
 * DCS#1563, DCS#1652, DCS#1653 -- Added MaxUsed to keep track of amount of BufferSpace
 * used by the registered client of the channel.
 *
 *    Rev 1.33   22 Mar 2001 16:39:28   whiteth
 * DCS#1421: New otv demux.  Dynamic memory allocation, task priority leveling, stack size changes.
 * DCS#1468:
 * DCS#1469:
 *
 *    Rev 1.32   27 Feb 2001 09:57:50   mustafa
 * DCS #1322 & 1323. Reduced ECM buffer size for NDSDEMUX cases only.
 *
 *    Rev 1.31   22 Feb 2001 13:46:54   mustafa
 * DCS #1290. ADded GENDMX_UNKOWN definition.
 *
 *    Rev 1.30   13 Feb 2001 10:15:56   mustafa
 * DCS 1195. Added bool member HeaderNotifyAlreadyCalled to the
 * ChannelInfoStruc. This is to prevent HeaderNotify from being called more
 * than once which seems to cause OpenTV problems in  loading apps.
 *
 *    Rev 1.29   06 Feb 2001 14:22:52   mustafa
 * Added new structure member DisableByFilter and added type check_CRC_t.
 * Changes are related to DCS #1135 but do not fix it.
 *
 *    Rev 1.28   29 Sep 2000 17:11:42   mustafa
 * Added minimum_header_size and header_notify_called.
 * These 2 variables are used by the OpenTV Demux Driver or any other
 * future client driver. The Generic Demux Driver totally ignores them.
 *
 *    Rev 1.27   15 Sep 2000 15:19:40   mustafa
 * Added tag2 to the Notify structure.
 *
 *    Rev 1.26   01 Sep 2000 13:23:06   mustafa
 * Moved ifdef CANAL PLUS.
 *
 *    Rev 1.25   01 Sep 2000 10:45:40   mustafa
 * Added #define CANAL_PLUS_FILTERING for VENDOR_B.
 *
 *    Rev 1.24   04 Aug 2000 09:40:08   mustafa
 * Removed BITSTREAM_OUT_FULL IRQ because of HW problem using parallel NIM and
 * High Speed Data port.
 *
 *    Rev 1.23   26 Jul 2000 11:39:00   whitey
 * Updated prototype for DVR read function (gen_dmx_read_dvr_buffer).
 *
 *    Rev 1.22   14 Jul 2000 16:30:00   mustafa
 * Added fields to implement Canal Plus filtering.
 *
 *    Rev 1.21   11 Jul 2000 13:35:10   whiteth
 * Added initial DVR support into gendmxc driver.
 *
 *    Rev 1.20   21 Jun 2000 09:52:40   mustafa
 * Fixed gen_pcr_callback_fct_t.
 *
 *    Rev 1.19   19 Jun 2000 13:31:20   mustafa
 * Cleanup VxWorks warnings.
 *
 *    Rev 1.18   15 May 2000 16:16:58   mustafa
 * Added DescDisabled to desc info.
 *
 *    Rev 1.18   15 May 2000 16:16:24   mustafa
 * Added DescDisabled to Descramle info.
 *
 *    Rev 1.17   11 May 2000 09:30:34   mustafa
 * Changes for new microcode.
 *
 *    Rev 1.16   27 Apr 2000 14:52:50   mustafa
 * Removed bitstream out & in full interrupt.
 *
 *    Rev 1.15   04 Apr 2000 19:54:22   mustafa
 * Added defines for error conditions and redefined -1 to 0xFFFF or 0xFFFFFFFF
 * as appropriate.
 *
 *    Rev 1.14   24 Mar 2000 14:26:16   mustafa
 * Added parameter pid to FindChannelPID.
 *
 *    Rev 1.13   07 Mar 2000 15:46:10   mustafa
 * Added prototypes for new functions.
 *
 *    Rev 1.12   17 Feb 2000 15:47:30   mustafa
 * Added CCounter to ChannelInfoTable.
 *
 *    Rev 1.11   26 Jan 2000 10:09:58   mustafa
 * Added OldMatch OldMask for HW debug.
 *
 *    Rev 1.10   03 Jan 2000 09:26:48   mustafa
 * Changed prototypes for registration parameters.
 *
 *    Rev 1.9   07 Dec 1999 15:56:26   mustafa
 * Added defines for NULL PID etc.
 *
 *    Rev 1.8   08 Nov 1999 16:20:00   mustafa
 * Added gen_dmx_control_av_channel().
 *
 *    Rev 1.7   26 Oct 1999 16:56:50   mustafa
 * Added prototype for gen_dmx_Xprt_Task.
 *
 *    Rev 1.6   13 Oct 1999 17:33:16   mustafa
 * Fixed prototypes.
 *
 *    Rev 1.5   12 Oct 1999 10:06:38   mustafa
 * Added timeout parameter to call back registration function.
 *
 *    Rev 1.4   07 Oct 1999 13:27:56   mustafa
 * Cleanup.
 *
 *    Rev 1.3   30 Sep 1999 11:07:56   mustafa
 * Fixed prototype defines for callbacks.
 *
 *    Rev 1.2   28 Sep 1999 15:00:08   mustafa
 * Fixed prototypes.
 *
 *    Rev 1.1   21 Sep 1999 10:07:02   mustafa
 * Changes for Nagra Demux.
 *
 *    Rev 1.0   02 Sep 1999 16:40:08   mustafa
 * Initial revision.
 *
 ****************************************************************************/

