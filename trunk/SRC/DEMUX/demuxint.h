/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*          Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002          */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        demuxint.h
 *
 *
 * Description:     Demux Driver Internals Header File
 *
 *
 * Author:          Bob Van Gulick
 *
 ****************************************************************************/
/* $Id: demuxint.h,v 1.34, 2004-06-16 16:13:27Z, Tim White$
 ****************************************************************************/

#define DEMUX_ISR_BITS      (DPS_EVENT_INTERRUPT|DPS_BITSTREAM_IN_FULL)

#if PARSER_TYPE==DTV_PARSER
#define DEMUX_BLOCK_EVENTS (DPS_PSI_BLOCK_STORED | \
                            DPS_CWP_BLOCK_STORED | \
                            DPS_CAP_BLOCK_STORED | \
                            DPS_PSI_BUFFER_FULL  | \
                            DPS_CWP_BUFFER_FULL  | \
                            DPS_CAP_BUFFER_FULL)
#else
#define DEMUX_BLOCK_EVENTS (DPS_PSI_BLOCK_STORED | \
                            DPS_PSI_BUFFER_FULL)
#endif

#if (LEGACY_DVR==YES)
    #define DEMUX_XPRT_EVENTS   (DPS_ALT_TRANSPORT_BLOCK_STORED|DPS_ALT_TRANSPORT_BUFFER_FULL)
    #define DEMUX_DVR_EVENT_BITS  (DPS_DVR_VIDEO_INTR|DPS_DVR_AUDIO_INTR|DPS_DVR_AUDIO1_INTR|    \
                         DPS_DVR_AUDIO2_INTR|DPS_DVR_VIDEO_OVERFLOW|DPS_DVR_AUDIO_OVERFLOW|  \
                         DPS_DVR_AUDIO1_OVERFLOW|DPS_DVR_AUDIO2_OVERFLOW|DPS_DVR_EVENT_INTR| \
                         DPS_DVR_EVENT_FULL)
    #define DEMUX_EVENT_BITS    (DPS_SYNC_ERROR|DEMUX_BLOCK_EVENTS|DEMUX_XPRT_EVENTS|DEMUX_DVR_EVENT_BITS)
#else
    #define DEMUX_XPRT_EVENTS   (DPS_TRANSPORT_BLOCK_STORED|DPS_TRANSPORT_BUFFER_FULL)
    #define DEMUX_SPLICE_EVENTS (DPS_VIDEO_SPLICED|DPS_AUDIO_SPLICED)
    #define DEMUX_EVENT_BITS    (DPS_SYNC_ERROR|DEMUX_BLOCK_EVENTS|DEMUX_XPRT_EVENTS|DPS_VIDEO_SPLICED|DPS_AUDIO_SPLICED)
#endif

/*#define DEMUX_EVENT_BITS    (DEMUX_BLOCK_EVENTS|DEMUX_XPRT_EVENTS|DPS_VIDEO_SPLICED|DPS_AUDIO_SPLICED)*/

#define CHANNEL_FREE            0
#define CHANNEL_IN_USE          1

#define FRAME_SIZE              192
#if PARSER_TYPE==DTV_PARSER
#define TS_COPY_BLOCK_SIZE      130
#else
#define TS_COPY_BLOCK_SIZE      188
#endif

#if PARSER_TYPE==DTV_PARSER
#define CWP_COPY_BLOCK_SIZE     128 
#define CWP_KEY_SIZE            120
#define CWP_BLOCK_SIZE_SHIFT    7 
#define CWP_BUFFER_SIZE         12800

#define CAP_COPY_BLOCK_SIZE     128 
#define CAP_KEY_SIZE            127 
#define CAP_BLOCK_SIZE_SHIFT    7 
#define CAP_BUFFER_SIZE         12800
#endif

#define TOTAL_FILTERS           32

#define PES_CHANNEL             0x02000
#define RAW_PES_CHANNEL         0x04000

#define BUFFER_BOUNDRY_32M      0x02000000

/* 1 demux unit on Colorado, 4 on Hondo & Wabash (Only 0, 2 & 3 are used today) */
#define MAX_DEMUX_UNITS DPS_NUM_DEMUXES
#define DEBUGPID 0x1FFF

#if DMXDMA==YES
#define DMX_DMA_INPUT_Q_SIZE    256
typedef struct _dmx_dma_input_q_t
{
    u_int8                    *buffer;
    u_int32                   size;
    u_int32                   tag;
    struct _dmx_dma_input_q_t *next;
} dmx_dma_input_q_t;
#endif /* DMXDMA==YES */

typedef struct _ChannelInformationStruc {
    u_int16 PID;
    u_int32 Slot;                       /* HW PID Slot */
    genchannel_t stype;
    gen_notify_t NotifyData;
    u_int32 client_id;                  /* set on channel allocation */
    u_int32 FilterEnable;
    u_int32 FiltersAllocated;
    bool DisabledByFilter;
    bool InUseFlag;
    bool DemuxEnable;
    bool CallbackFlushedChannel;
    bool TagSet;
    u_int32 Tag;
    u_int32 Overflows;
    u_int32 OverflowsHandled;
    u_int8 *pBuffer;
    u_int8 *pBufferEnd;
    u_int8 *pWritePtr;
    u_int8 *pReadPtr;
    u_int8 *pAckPtr;
    u_int32 CrcStatus;
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
    bool ChannelTimerActive;
    u_int32 NotifyCount;
    bool PESChannel;
    u_int32 PESDataCount;
    u_int32 CurrentFilterId;
    genfilter_mode LoopMode;            /* One Shot, loop, toggle */
    u_int32 TimeOutMS;
    u_int32 TimerNotifyCount;
    u_int32 CCounter;                   /* Continuity Counter */
    bool bRecording;

/* debug stuff */
    int Notify_Unlock;
    int NotifyCalled;

} ChannelInformationStruc;

typedef struct _FilterStruc {
    u_int32 OwnerChid;
    u_int32 FilterSize;
    u_int32 ExtFilterSize;
    bool FilterEnabled;
    bool NewFilter;
    bool VersionEqual;
    u_int32 Match[GENDMX_MAX_HW_FILTER_SIZE/4];
    u_int32 Mask[GENDMX_MAX_HW_FILTER_SIZE/4];
    u_int32 NotMask[GENDMX_MAX_HW_FILTER_SIZE/4];
    bool    NotMaskZero;
    bool ExtFilterEnabled;
    u_int32 ExtMatch[GENDMX_EXT_FILTER_SIZE/4];
    u_int32 ExtMask[GENDMX_EXT_FILTER_SIZE/4];
    u_int32 ExtFilterOffset;
    /* debug stuff */
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

typedef struct {
    /* CWP parameters */
    gen_notify_t CWPNotifyData;
    u_int32 CWP_Overflows;
    gen_callback_fct_t CWPNotify;
    u_int8 *pCWPBuffer;
    u_int8 *pCWPBufferEnd;
    u_int8 *pCWPWritePtr;
    u_int8 *pCWPReadPtr;
    tick_id_t CWPTimer;
    bool CWPTimerActive;
    bool CWPBufferFull;
    u_int32 CWPTimeOutMS;
    u_int32 CWPTimerNotifyCount;
    u_int32 CWPDataCount;
    int CWPMaxBlocksToCopy;
} CWPInformationStruc;

typedef struct {
    /* CAP parameters */
    gen_notify_t CAPNotifyData;
    u_int32 CAP_Overflows;
    gen_callback_fct_t CAPNotify;
    u_int8 *pCAPBuffer;
    u_int8 *pCAPBufferEnd;
    u_int8 *pCAPWritePtr;
    u_int8 *pCAPReadPtr;
    tick_id_t CAPTimer;
    bool CAPTimerActive;
    bool CAPBufferFull;
    u_int32 CAPTimeOutMS;
    u_int32 CAPTimerNotifyCount;
    u_int32 CAPDataCount;
    int CAPMaxBlocksToCopy;
    int CAP_scid;
    int CAP_filter;
    bool CAPEnable;
} CAPInformationStruc;

/***************************************************************
 * This structure will contain all variables associated with 
 * a particular instance of a demux.  There will be one of these
 * instantiated on Colorado and 3 on Hondo & Wabash, one for each demux 
 * unit.
 **************************************************************/
typedef struct DemuxInfo {
    u_int32 DemuxID;
    u_int32 CapabilitiesRequested;
    u_int32 NonDescChannelsAvail;
    u_int32 DescChannelsAvail;
    u_int32 AvailableFilters;
    u_int32 SyncErrorCount;
    u_int32 gInSyncCount;
    u_int16 PCR_PID_Value;
    int GenDmxTSMaxBlocksToCopy;
    PFNISR DemuxIRQHandler;
    u_int32 dmx_interrupt_bit;

    /* Booleans */
    bool TransportBufferFull;
    bool gParserInSync;
    bool HWFiltering;
    bool DemodConnected;
    bool DemuxInitialized;
    bool PSILocalBufferFull;
    bool NotifyCRC;  /* determined if CRC notification is enabled */

    /* Channel, Filter and Descrambler tables */
    ChannelInformationStruc ChannelInfoTable[TOTAL_CHANNELS];
    FilterStruc FilterTable[TOTAL_FILTERS];
    desc_info Descramblers[DPS_NUM_DESCRAMBLERS];
    desc_key_info DescramblerKeyInfo[DPS_NUM_DESCRAMBLERS];

    PFNISR pOldfnHandler;

#if PARSER_TYPE==DTV_PARSER
    CWPInformationStruc CWPInfo;
    CAPInformationStruc CAPInfo;
#endif

#if (PVR==YES) || (XTV_SUPPORT==YES)
    /*
     * Record fields
     */
    u_int8 *rec_buffer_start_addr;
    u_int8 *rec_buffer_end_addr;
    u_int8 *rec_buffer_read_ptr;
    u_int8 *rec_buffer_write_ptr;
    u_int32 rec_buffer_notify_block_size;
    u_int32 rec_buffer_tag;
    dmx_rec_notify_fct_t rec_buffer_notify;
    u_int8 *rec_event_buffer_start_addr;
    u_int8 *rec_event_buffer_end_addr;
    u_int8 *rec_event_buffer_read_ptr;
    u_int8 *rec_event_buffer_write_ptr;
    u_int32 rec_event_buffer_tag;
    dmx_rec_notify_fct_t rec_event_buffer_notify;
#endif /* PVR==YES */

#if DMXDMA==YES
    /*
     * Demux DMA Input Extension fields
     */
    bool dma_input_selected;
    bool dma_input_init;
    bool dma_input_active;
    bool dma_input_busy;
    u_int32 dma_current_size;
    u_int8 *dma_current_buffer_ptr;
    u_int32 dma_bytes_left;
    u_int32 dma_input_channel;
    u_int32 num_dma_input_errors;
    dmx_dma_input_notify_fct_t dma_input_notify;
    dmx_dma_input_q_t dma_input_q[DMX_DMA_INPUT_Q_SIZE];
    dmx_dma_input_q_t *dma_input_q_head;
    dmx_dma_input_q_t *dma_input_q_tail;
    dmx_dma_input_q_t *dma_input_q_free;
    sem_id_t dma_input_sem;
    u_int32 dma_ts_in_buffer;
    u_int32 dma_req_sel;
#endif /* DMXDMA==YES */
    u_int64 video_pts_offset;
    u_int64 audio_pts_offset;
    u_int64 video_timebase_offset;
    u_int64 audio_timebase_offset;
} DemuxInfo;

#if DMXDMA==YES
typedef struct DemuxDriverInfo {
    u_int32 dma_input_intr_mask;
    bool dma_input_init;
    PFNISR dma_input_intr_chain;
} DemuxDriverInfo;
#endif

/* Key offset defines to simplify the descrambler code */
#define KEY_EVEN_HIGH 0x0
#define KEY_EVEN_LOW  0x1
#define KEY_ODD_HIGH  0x2
#define KEY_ODD_LOW   0x3

/******************************/
/* Hardware Access Functions  */
/******************************/
bool genDemuxInitHardware(u_int32 dmxid, bool ReInit);
void genDemuxReInitHW(u_int32 dmxid);
void gen_dmx_hw_free_pid(u_int32 dmxid, u_int32 chid);
void gen_dmx_shutdown(u_int32 dmxid);
void gen_dmx_hw_set_pid(u_int32 dmxid, u_int32 chid, u_int16 pid);
void gen_dmx_set_standby_video(u_int32 dmxid, u_int16 VideoPID);
void gen_dmx_set_standby_audio(u_int32 dmxid, u_int16 AudioPID);
void gen_dmx_enable_video_switch(u_int32 dmxid);
void gen_dmx_enable_audio_switch(u_int32 dmxid);
u_int32 gen_dmx_get_video_switch_status(u_int32 dmxid);
u_int32 gen_dmx_get_audio_switch_status(u_int32 dmxid);
#if (LEGACY_DVR==YES)
void gen_dmx_dvr_set_highwater_mark(u_int32 dmxid, u_int32 HighWater_Mark);
void gen_dmx_dvr_set_mode(u_int32 dmxid, u_int32 Mode);
int gen_dmx_read_dvr_buffer(u_int32 dmxid, gendmx_dvr_channel_t channel,
                            u_int32 *size, u_int8 *buffer, u_int8 **addr);
#endif

/******************************/
/* TS/SI Processing Functions */
/******************************/
bool gen_dmx_section_notify(u_int32 dmxid, u_int8 *wptr, u_int32 chid, u_int32 filter_mask);
void gen_dmx_section_unlock(u_int32 dmxid, u_int32 chid, u_int8 *buffer, 
                            u_int32 length);
/******************************/
/*Miscellaneous Functions     */
/******************************/
bool genDemuxInitSW();
bool genDemuxInitSW(u_int32 dmxid);
int DemuxIRQHandler0(u_int32 IntID, bool Fiq, PFNISR *pfnChain);
int DemuxIRQHandler2(u_int32 IntID, bool Fiq, PFNISR *pfnChain);
int DemuxIRQHandler3(u_int32 IntID, bool Fiq, PFNISR *pfnChain);
int DemuxIRQHandler(u_int32 IntID, bool Fiq, PFNISR *pfnChain, u_int32 dmxid);
void gen_dmx_PSI_Task(void *arg);
void gen_dmx_Xprt_Task(void *arg);
void gen_dmx_CWP_Task(void *arg);
void dvr_check_for_dvr_events(u_int32 dmxid, u_int32 EventReason);
void gen_demux_timer_call_back(timer_id_t timer, void *pUserData);
u_int32 gen_dmx_register_callbacks(u_int32 dmxid, gen_callback_fct_t HeaderCallBackEntry, 
                                   gen_callback_fct_t SectionCallBackEntry);
u_int32 gen_demux_get_callback_handle(u_int32 dmxid);
u_int32 GetBufferFullness(u_int32 rdptr, u_int32 wrptr, u_int32 start, u_int32 end, u_int32 full);
/* is this used anywhere?
void AdvancePointer(LPDPS_ADDRESS lpPtr, DPS_ADDRESS BufferStart, DPS_ADDRESS BufferEnd, 
                    u_int32 Amount); */
bool SoftwareFilter(u_int32 dmxid, ChannelInformationStruc *pCh, char *pByteData, u_int32 length);
u_int32 gen_dmx_get_filter_chid(u_int32 dmxid, u_int32 fid);
u_int32 gen_dmx_get_channel_filter(u_int32 dmxid, u_int32 chid);
u_int32 gen_dmx_get_allocated_filters(u_int32 dmxid, u_int32 chid);
u_int32 gen_dmx_buffers_inquire(u_int32 dmxid, u_int32 chid, u_int8 **beg, u_int8 **end);
u_int16 gen_dmx_get_video_pid(u_int32 dmxid, bool bHardware);
u_int16 gen_dmx_get_audio_pid(u_int32 dmxid);
bool gen_dmx_channel_enabled(u_int32 dmxid, u_int32 chid);
int gen_dmx_control_av_channel(u_int32 dmxid, u_int32 chid, gencontrol_channel_t channel_command);
void DumpAll(u_int32 dmxid);
#if (LEGACY_DVR==YES)
void gen_dmx_dvr_register_notification_handlers (dvr_notify_fct_t EventHandler,
                                                 dvr_notify_fct_t DataHandler, dvr_notify_fct_t OverflowHandler);
void dvr_default_handler (u_int32 dmxid, u_int32 Event);
void dvr_init(u_int32 dmxid);
#endif
u_int32 gen_dmx_get_audio_buff_size();
u_int32 gen_dmx_get_video_buff_size(); 
bool WCopyBytes(u_int8 *dest, u_int8 *source, u_int32 length, u_int8 *pBuffer, u_int8 *pBufferEnd);
bool AdvancePtr(u_int8 **p_ptr, u_int32 length, u_int8 *pBuffer, u_int8 *pBufferEnd);

/* Conditional access routines for DirecTV */
#if PARSER_TYPE==DTV_PARSER
void gen_demux_CWP_timer_call_back(timer_id_t timer, void *pUserData);
void gen_dmx_CWP_Task(void *arg);
void gen_demux_CAP_timer_call_back(timer_id_t timer, void *pUserData);
void gen_dmx_CAP_Task(void *arg);
#endif

/****************************************************************************
 * Modifications:
 * $Log: 
 *  35   mpeg      1.34        6/16/04 11:13:27 AM    Tim White       CR(s) 
 *        9483 9484 : Changed pvr to rec for consistency.
 *        
 *  34   mpeg      1.33        5/17/04 9:07:43 AM     Larry Wang      CR(s) 
 *        9212 9213 : Remove XTV related definition.
 *  33   mpeg      1.32        4/15/04 11:59:18 AM    Larry Wang      CR(s) 
 *        8868 8869 : Move some XTV related definition into cnxt_xtv.h.
 *  32   mpeg      1.31        3/23/04 3:14:52 PM     Larry Wang      CR(s) 
 *        8638 8639 : Add some definition for XTV.
 *  31   mpeg      1.30        3/18/04 10:38:42 AM    Tim White       CR(s) 
 *        8591 : Allow use of >64KB buffers to the 
 *        cnxt_dmx_dma_input_add_buffer() function.  The
 *        DMA function internally handles breaking up the DMA transaction into 
 *        sub 64KB sections
 *        and only delivers the interrupt at the end.  Halt is supported only 
 *        on request
 *        boundaries.
 *        
 *  30   mpeg      1.29        3/16/04 3:57:06 PM     Tim White       CR(s) 
 *        8545 : Add dma_input_selected bool for DMXDMA operation.
 *        
 *  29   mpeg      1.28        3/10/04 1:26:51 PM     Tim Ross        CR(s) 
 *        8545 : Corrected compilation errors for PVR record demux code. 
 *  28   mpeg      1.27        3/10/04 10:57:16 AM    Tim White       CR(s) 
 *        8545 : Removed PVR playback LWM notification from demux.
 *        
 *  27   mpeg      1.26        3/2/04 10:59:13 AM     Tim Ross        CR(s) 
 *        8451 : Added bRecording flag to channel structure.
 *  26   mpeg      1.25        2/24/04 2:45:01 PM     Bob Van Gulick  CR(s) 
 *        8427 : Add cnxt_dmx_set_timebase_offset function to offset PTS by +/-
 *         12 hours
 *        
 *  25   mpeg      1.24        12/7/03 6:22:31 PM     Tim White       CR(s) 
 *        8113 : Allow any word aligned buffer to work with the Demux DMA Input
 *         Extension.
 *        
 *  24   mpeg      1.23        11/25/03 3:52:38 PM    Tim White       CR(s): 
 *        8027 Drop Demux DMA Input Extension function.
 *        
 *  23   mpeg      1.22        11/19/03 10:10:01 AM   Tim White       CR(s): 
 *        7987 Added Demux DMA and Demux PVR extension support phase 1.
 *        
 *  22   mpeg      1.21        9/22/03 4:52:50 PM     Bob Van Gulick  SCR(s) 
 *        7519 :
 *        Add support for DirecTV CAPs
 *        
 *        
 *  21   mpeg      1.20        9/16/03 4:04:22 PM     Tim White       SCR(s) 
 *        7474 :
 *        Add capabilities to DemuxInfo structure array.
 *        
 *        
 *  20   mpeg      1.19        8/27/03 11:01:28 AM    Bob Van Gulick  SCR(s) 
 *        7387 :
 *        Add support for CWP processing in DirecTV 
 *        
 *        
 *  19   mpeg      1.18        6/9/03 5:58:54 PM      Bob Van Gulick  SCR(s) 
 *        6755 :
 *        Add support for 8 slot descram in demux.  Also change use of 
 *        DESC_CHANNELS to DPS_NUM_DESCRAMBLERS.
 *        
 *        
 *  18   mpeg      1.17        4/24/03 5:50:52 PM     Tim White       SCR(s) 
 *        6097 :
 *        Allow 6 descrambled simultaneous PES channels.  Remove #ifndef 
 *        USE_OLD_PES code.
 *        
 *        
 *  17   mpeg      1.16        4/10/03 5:02:38 PM     Dave Wilson     SCR(s) 
 *        5990 :
 *        
 *        
 *        
 *        Added cnxt_dmx_set_section_channel_tag API.d
 *        
 *        
 *  16   mpeg      1.15        3/17/03 11:32:20 AM    Matt Korte      SCR(s) 
 *        5777 :
 *        Change to DTV and DIRECTV
 *        
 *  15   mpeg      1.14        3/5/03 5:15:22 PM      Dave Wilson     SCR(s) 
 *        5667 :
 *        Added field CallbackFlushedChannel to the ChannelInformationStruc
 *        
 *  14   mpeg      1.13        3/3/03 11:00:28 AM     Larry Wang      SCR(s) 
 *        5631 :
 *        Define TS_COPY_BLOCK_SIZE to be 130 if PARSER_TYPE is DSS_PARSER.
 *        
 *  13   mpeg      1.12        11/20/02 5:06:30 PM    Bob Van Gulick  SCR(s) 
 *        4998 :
 *        Remove gendemux version of register_pcr_notify function.  It is now 
 *        part of the API.
 *        
 *        
 *  12   mpeg      1.11        10/2/02 2:51:20 PM     Bob Van Gulick  SCR(s) 
 *        4636 :
 *        Only set info change bit while setting a filter if the filter had 
 *        already been allocated and set.  
 *        
 *        
 *  11   mpeg      1.10        9/19/02 3:42:50 PM     Joe Kroesche    SCR(s) 
 *        4610 :
 *        added crc notification feature, removed changes for previous crc 
 *        notification
 *        method. NOTE!!! requires matching pawser ucode update of #4626
 *        
 *  10   mpeg      1.9         9/5/02 6:30:04 PM      Bob Van Gulick  SCR(s) 
 *        4530 :
 *        Change CRC check to use Header Notify instead of Section Notify
 *        
 *        
 *  9    mpeg      1.8         8/30/02 2:44:28 PM     Bob Van Gulick  SCR(s) 
 *        4485 :
 *        Add support for CRC checking in demux
 *        
 *        
 *  8    mpeg      1.7         8/16/02 6:05:06 PM     Tim White       SCR(s) 
 *        4420 :
 *        Add support for new DVR microcode which supports DVR, XPRT, and 
 *        MULTI_PSI together.
 *        
 *        
 *  7    mpeg      1.6         8/5/02 11:55:04 AM     Tim White       SCR(s) 
 *        4330 :
 *        Fixed timeout and single shot (ONE_SHOT) capabilities.
 *        
 *        
 *  6    mpeg      1.5         6/27/02 5:57:12 PM     Tim White       SCR(s) 
 *        4108 :
 *        Convert MHP glue layer to use new DEMUX driver.
 *        
 *        
 *  5    mpeg      1.4         4/26/02 3:16:10 PM     Tim White       SCR(s) 
 *        3562 :
 *        Add support for Colorado Rev_F.
 *        
 *        
 *  4    mpeg      1.3         2/21/02 4:19:40 PM     Tim White       SCR(s) 
 *        3229 :
 *        Use DPS_NUM_DEMUXES since it takes into account the missing demux #1 
 *        for Honfo/Wabash chips.
 *        
 *        
 *  3    mpeg      1.2         2/21/02 4:12:16 PM     Tim White       SCR(s) 
 *        3229 :
 *        Use NUM_PARSERS #definition instead of GPIOM_CONFIG to find out the 
 *        number of demux'es in the chip.
 *        
 *        
 *  2    mpeg      1.1         2/7/02 11:54:26 AM     Bob Van Gulick  SCR(s) 
 *        3143 :
 *        Allow Wabash to the list of supported multi-instance demux chips
 *        
 *        
 *  1    mpeg      1.0         12/18/01 10:15:56 AM   Bob Van Gulick  
 * $
 ****************************************************************************/

