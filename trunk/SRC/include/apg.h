/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*        Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002, 2003      */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        apg.h
 *
 *
 * Description:     Public header file for APG
 *
 *
 * Author:          Larry Wang
 *
 ****************************************************************************/
/* $Header: apg.h, 17, 5/8/03 6:57:34 PM, Bob Van Gulick$
 ****************************************************************************/

#ifndef _APG_H_
#define _APG_H_

/*****************/
/* Include Files */
/*****************/

/********************************/
/* Symbol and Macro definitions */
/********************************/
#define get_int32(x)  ((((u_int32)*(x)<<24)&0xff000000)|      \
                       (((u_int32)*((x)+1)<<16)&0x00ff0000)|  \
                       (((u_int32)*((x)+2)<<8)&0x0000ff00)|   \
                       (((u_int32)*((x)+3))&0x000000ff))
#define get_int16(x)  ((((u_int16)*(x)<<8)&0xff00)|((u_int16)*((x)+1)&0x00ff))
#define get_scid(x)   ((((u_int16)*(x)<<8)&0x0f00)|((u_int16)*((x)+1)&0x00ff))
#define get_int12(x)  get_scid(x)

/*****************/
/* Data Types    */
/*****************/

/* Object Types */
#define UPDATELISTOBJ  0x92  /* Update List Object Type */
#define MARKEROBJ  0x89      /* Marker Object Type */
#define BOOTOBJ  0x81        /* Boot Object Type */
#define CHANNELOBJ 0x84      /* Channel Object Type */
/* Stream_Type */
#define DTV_VIDEO_STREAM 0x02   /* IEC 13818-2 */
#define DTV_AUDIO_STREAM 0x03   /* Mpeg Audio */
#define DTV_DAVIS_STREAM 0xC0   /* Data Annotated Video Service (DAVIS) */
#define DTV_AC3_STREAM   0xC8   /* AC-3 Audio */

#define DTV_PERFORMANCE_CHECK

typedef struct
{
  u_int32  object_id;
  u_int8   object_type;
  u_int8   object_version;
  u_int8   carousel_mask;
}DTV_ULO_OBJECT_DESCR;

typedef struct
{
   u_int32                   ObjId;
   u_int8                    ObjVer;
   u_int32                   TimeFirstRef;
   u_int8                    boot_object_version;
   u_int16                   number_of_objects;
   DTV_ULO_OBJECT_DESCR      *Objects;
   u_int16                   DescriptorLen;
   u_int8                    *Descriptors;
} DTV_APG_UPDATE_LIST_OBJECT;

typedef struct
{
   u_int32 ObjId;
   u_int8  ObjVer;
   u_int32 TimeFirstRef;
   u_int16 OriginatingNetId;
   u_int8  OriginatingFreqIndex;
   u_int8  BootStreamFreqIndex;
   u_int16 DescriptorLen;
   u_int8  *Descriptors;
} DTV_APG_MARKER_OBJECT;

typedef struct
{
   u_int8       polarization;
   u_int8       channel_coding_id;
   u_int8       freq_index;
   u_int8       no_of_LNB_types;
   u_int8       LNB_type[3];
   u_int32      LNB_type_frequency[3];
} DTV_APG_SAT_FREQ_TABLE;


typedef struct
{
   u_int16       orbital_position;
   u_int8        east_west_flag;
   u_int8        number_of_frequencies;
   DTV_APG_SAT_FREQ_TABLE freq_table[32];
} DTV_APG_SATELLITE_DESCRIPTOR;


typedef struct
{
   u_int16 NetId;
   u_int8  FastLoadFreqIndex;
   u_int8  FastLoadXmitIndicator;
   u_int16 FastLoadScid;
   u_int8  CarouselXmitIndicator;
   u_int16 CarouselScid;
   u_int16 DescriptorLen;
   u_int8  *Descriptors;
} DTV_NETWORK_DESCRIPTOR;

typedef struct
{
   u_int32                ObjId;
   u_int8                 ObjVer;
   u_int32                TimeFirstRef;
   u_int32                RootCsoId;
   u_int8                 SecondBootCycleTime;
   u_int16                SecondBootScid;
   u_int8                 SecondBootFreqIndex;
   u_int16                CaScid;
   u_int16                PipScid;
   u_int16                InteractiveScid;
   u_int8                 InteractiveFreqIndex;
   u_int16                InteractiveNetId;
   u_int32                SysTime;
   u_int8                 UtcOffset;
   u_int16                TotalGuideDuration;
   u_int8                 NumOfCarousels;
   u_int16                CarouselDuration[7];
   u_int16                NumOfNetworks;
   DTV_NETWORK_DESCRIPTOR Networks[4];
   u_int16                DescriptorLen;
   u_int8                 *Descriptors;
   u_int16                cycle_time;   /* boot cycle time descriptor */
} DTV_APG_BOOT_OBJECT;


typedef struct
{
    u_int8       StreamType;   /* Video,  MPEG Audio, AC3 Audio, DAVIS */
    u_int16      SCID;
} DTV_APG_STREAM_DEF;

#define DTV_INVALID_INDEX 255 /* for Current*Index in Channel Def (below) */

typedef struct
{
   u_int8  ServiceType;
   u_int8  SPI;
   u_int8  NumOfStream;
   u_int8  FreqIndex;
   u_int8  NumVideoStreams;
   u_int8  NumMpegAudioStreams;
   u_int8  NumAC3AudioStreams;
   u_int8  NumDAVISStreams;
   u_int8  CurrentVideoIndex;
   u_int8  CurrentMpegAudioIndex;
   u_int8  CurrentAC3AudioIndex;
   u_int8  CurrentDAVISIndex;
   /* Note: if there is a root_SCID, it will ALWAYS be the first StreamDef */
   DTV_APG_STREAM_DEF  *StreamDef; /* There can be MANY streams.. */
} DTV_APG_CHANNEL_DEF;        



#define ADDITIONAL_NETWORK 1
#define TRANSMIT_NETWORK  2

typedef struct
{
   u_int32                ObjId;
   u_int8                 ObjVer;
   u_int32                TimeFirstRef;
   u_int16                SourceId;
   u_int16                MajorNumber;
   u_int8                 ShortNameLen;
   u_int8                 ShortName[256];
   u_int8                 Indicators;
   DTV_APG_CHANNEL_DEF    ChannelDef;
   u_int16                MinorNumber;
   u_int16                DescriptorLen;
   u_int8                 *Descriptors;
   u_int8                 valid_descriptors;     /* ADDITIONAL_NETWORK, TRANSMIT_NETWORK */
   u_int16                additional_network_id; /* network_id as found in additional_network_descriptor */
   u_int16                transmit_network_id;   /* network_id as found in transmit_network_descriptor */
} DTV_APG_CHANNEL_OBJECT;



typedef genfilter_mode (*APG_OBJ_PFNNOTIFY) ( u_int8 *pApgObj, u_int32 uObjLength );
typedef bool (*APG_OBJ_HDR_PFNNOTIFY) ( u_int8 *pApgObjHdr );

typedef enum
{
   DTV_APG_OK,
   DTV_APG_NO_RESOURCES,
   DTV_APG_ERROR
} DTV_APG_STATUS;

typedef enum _DEMOD_CONTROL_MSG_ID
{
  CTRL_MSG_DMD_CONNECTED,
  CTRL_MSG_DMD_FAIL
} DEMOD_CONTROL_MSG_ID;

typedef struct _DEMOD_CONTROL_MSG
{
  DEMOD_CONTROL_MSG_ID eMsgID;
} DEMOD_CONTROL_MSG;

/**********************/
/* Utility prototypes */
/**********************/
bool cnxt_apg_parse_bo ( DTV_APG_BOOT_OBJECT *pBootObjInfo, u_int8 *pBootObj );
void cnxt_apg_free_bo ( DTV_APG_BOOT_OBJECT *pBootObjInfo );
bool cnxt_apg_parse_co ( DTV_APG_CHANNEL_OBJECT *pChanObjInfo, u_int8 *pChanObj );
void cnxt_apg_free_co ( DTV_APG_CHANNEL_OBJECT *pChanObjInfo );
bool cnxt_apg_parse_mo ( DTV_APG_MARKER_OBJECT *pMarkerObjInfo, u_int8 *pMarkerObj );
void cnxt_apg_free_mo ( DTV_APG_MARKER_OBJECT *pMarkerObjInfo );
bool cnxt_apg_parse_ulo ( DTV_APG_UPDATE_LIST_OBJECT *pUpdateListObjInfo, u_int8 *pUpdateListObj );
void cnxt_apg_free_ulo ( DTV_APG_UPDATE_LIST_OBJECT *pUpdateListObjInfo );

bool cnxt_dtv_tune_to_freq ( u_int16 network_id, u_int8 freq_index );
u_int32 cnxt_dtv_get_current_freq_index ( void );
u_int16 cnxt_dtv_get_current_network_id ( void );

DTV_APG_STATUS cnxt_dtv_acquire_apg_obj ( u_int16 uScid, 
                                          u_int8 *pFilterMatch,
                                          u_int8 *pFilterMask,
                                          u_int32 uTimeout, 
                                          APG_OBJ_HDR_PFNNOTIFY pObjHdrNotifyFn, 
                                          APG_OBJ_PFNNOTIFY pObjNotifyFn );

typedef struct 
{
    u_int32 min, us;
} dtv_time_t;
bool cnxt_dtv_timer_init();
void cnxt_dtv_get_current_time( dtv_time_t *pTime );
u_int32 cnxt_dtv_calc_duration( dtv_time_t StartTime, dtv_time_t EndTime );

#endif   /* _APG_H_ */

/****************************************************************************
 * Modifications:
 * $Log: 
 *  17   mpeg      1.16        5/8/03 6:57:34 PM      Bob Van Gulick  SCR(s) 
 *        6278 6279 :
 *        Fix Ooops on previous drop
 *        
 *        
 *  16   mpeg      1.15        5/8/03 6:49:48 PM      Bob Van Gulick  SCR(s) 
 *        6278 6279 :
 *        Add support for demod notify messages when switching transponders.
 *        
 *        
 *  15   mpeg      1.14        5/2/03 11:58:18 AM     Dave Moore      SCR(s) 
 *        5972 :
 *        Changes to support integration into WatchTV
 *        
 *        
 *  14   mpeg      1.13        4/22/03 3:08:58 PM     Larry Wang      SCR(s) 
 *        6068 :
 *        Add prototype of cnxt_consolidate_co_list().
 *        
 *  13   mpeg      1.12        4/9/03 6:34:42 PM      Dave Moore      SCR(s) 
 *        5972 :
 *        added sat descriptor structs, other descriptor entries in 
 *        DTV_APG_XX_OBJ.
 *        
 *        
 *  12   mpeg      1.11        4/8/03 3:30:02 PM      Larry Wang      SCR(s) 
 *        5984 :
 *        modify proto type of cnxt_dtv_tune_to_freq(); add proto type of 
 *        cnxt_dtv_get_current_network_id().
 *        
 *  11   mpeg      1.10        3/27/03 7:52:40 PM     Larry Wang      SCR(s) 
 *        5900 :
 *        Fix get_int32() micro to support the case that x is not 4 
 *        byte-aligned.
 *        
 *  10   mpeg      1.9         3/26/03 3:51:46 PM     Dave Moore      SCR(s) 
 *        5805 :
 *        Channel Object changes
 *        
 *        
 *  9    mpeg      1.8         3/25/03 9:53:56 AM     Angela Swartz   SCR(s) 
 *        5826 :
 *        added #define DTV_PERORMANCE_CHECK
 *        
 *  8    mpeg      1.7         3/24/03 12:30:32 PM    Larry Wang      SCR(s) 
 *        5859 :
 *        change the type of header notification function.
 *        
 *  7    mpeg      1.6         3/24/03 12:14:16 PM    Larry Wang      SCR(s) 
 *        5859 :
 *        Add APG header notification callback.
 *        
 *  6    mpeg      1.5         3/20/03 4:06:52 PM     Dave Moore      SCR(s) 
 *        5805 :
 *        added SPI field to channel_definition struct
 *        
 *        
 *  5    mpeg      1.4         3/19/03 3:47:58 PM     Angela Swartz   SCR(s) 
 *        5826 :
 *        added APIs to support time measurement for performance check
 *        
 *  4    mpeg      1.3         3/19/03 11:59:46 AM    Matt Korte      SCR(s) 
 *        5819 :
 *        Code clean up
 *        
 *  3    mpeg      1.2         3/18/03 4:06:46 PM     Dave Moore      SCR(s) 
 *        5805 :
 *        added update list obj data types and parsing func prototypes
 *        
 *        
 *  2    mpeg      1.1         3/18/03 10:41:00 AM    Matt Korte      SCR(s) 
 *        5777 :
 *        Change to DTV and DIRECTV
 *        
 *  1    mpeg      1.0         3/17/03 11:14:54 AM    Matt Korte      
 * $
 * 
 *    Rev 1.16   08 May 2003 17:57:34   vangulr
 * SCR(s) 6278 6279 :
 * Fix Ooops on previous drop
 * 
 * 
 *    Rev 1.14   02 May 2003 10:58:18   mooreda
 * SCR(s) 5972 :
 * Changes to support integration into WatchTV
 * 
 * 
 *    Rev 1.13   22 Apr 2003 14:08:58   wangl2
 * SCR(s) 6068 :
 * Add prototype of cnxt_consolidate_co_list().
 * 
 *    Rev 1.12   09 Apr 2003 17:34:42   mooreda
 * SCR(s) 5972 :
 * added sat descriptor structs, other descriptor entries in DTV_APG_XX_OBJ.
 * 
 * 
 *    Rev 1.11   08 Apr 2003 14:30:02   wangl2
 * SCR(s) 5984 :
 * modify proto type of cnxt_dtv_tune_to_freq(); add proto type of cnxt_dtv_get_current_network_id().
 * 
 *    Rev 1.10   27 Mar 2003 19:52:40   wangl2
 * SCR(s) 5900 :
 * Fix get_int32() micro to support the case that x is not 4 byte-aligned.
 * 
 *    Rev 1.9   26 Mar 2003 15:51:46   mooreda
 * SCR(s) 5805 :
 * Channel Object changes
 * 
 * 
 *    Rev 1.8   25 Mar 2003 09:53:56   swartzwg
 * SCR(s) 5826 :
 * added #define DTV_PERORMANCE_CHECK
 * 
 *    Rev 1.7   24 Mar 2003 12:30:32   wangl2
 * SCR(s) 5859 :
 * change the type of header notification function.
 * 
 *    Rev 1.6   24 Mar 2003 12:14:16   wangl2
 * SCR(s) 5859 :
 * Add APG header notification callback.
 * 
 *    Rev 1.5   20 Mar 2003 16:06:52   mooreda
 * SCR(s) 5805 :
 * added SPI field to channel_definition struct
 * 
 * 
 *    Rev 1.4   19 Mar 2003 15:47:58   swartzwg
 * SCR(s) 5826 :
 * added APIs to support time measurement for performance check
 * 
 *    Rev 1.3   19 Mar 2003 11:59:46   kortemw
 * SCR(s) 5819 :
 * Code clean up
 * 
 *    Rev 1.2   18 Mar 2003 16:06:46   mooreda
 * SCR(s) 5805 :
 * added update list obj data types and parsing func prototypes
 * 
 * 
 *    Rev 1.1   18 Mar 2003 10:41:00   kortemw
 * SCR(s) 5777 :
 * Change to DTV and DIRECTV
 * 
 *    Rev 1.0   17 Mar 2003 11:14:54   kortemw
 * SCR(s) 5777 :
 * DIRECTV Support
 * 
 *    Rev 1.3   14 Mar 2003 16:24:02   wangl2
 * SCR(s) 5699 :
 * Add a return code for APG_OBJ_PFNNOTIFY function so that the application can determine that if the acquisition should continue.
 * 
 *    Rev 1.2   14 Mar 2003 11:19:34   wangl2
 * SCR(s) 5769 :
 * Add length argument in object callback.
 * 
 *    Rev 1.1   11 Mar 2003 17:54:10   wangl2
 * SCR(s) 5699 :
 * Add an API to acquire APG object.
 * 
 *    Rev 1.0   06 Mar 2003 15:33:28   wangl2
 * SCR(s) 5699 :
 * Initial version of APG header file.
 * 
 * 
 ****************************************************************************/

