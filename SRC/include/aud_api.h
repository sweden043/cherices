/****************************************************************************/ 
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*          Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002          */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename: AUD_API.H
 *
 *
 * Description: Definitions and prototypes for the AUD_API driver.
 *
 *
 * Author: Craig Dry
 *
 ****************************************************************************/
/* $Header: aud_api.h, 8, 4/6/04 1:47:04 PM, Joe Kroesche$
****************************************************************************/ 
#ifndef _AUD_API_H_
#define _AUD_API_H_

/*===========================================================================
 * define Section
 *===========================================================================*/
#ifdef _AUD_API_C
#define PUBLIC
#else
#define PUBLIC extern
#endif

/* CNXT_AUDIO_INFO Start */
/*
 * This section is for the 'soft' register for Audio Info
 * and associated definitions. The 'soft' register is a
 * location in the Audio 2 decode buffer that the audio
 * microcode updates with a 32 bit value every audio frame
 * and when they go to reset. The upper bit currently contains
 * a valid bit, and the low 20 bits contain the mpeg header.
 */
#define AUD_INFO_REG              0xf200        /* Location of 'soft' reg */
#define AUD_INFO_VALID_MASK       0x80000000    /* Mask for audio valid */
#define AUD_INFO_FMT_MASK         0x30000000    /* Mask for ac3/mpeg format identifier */
#define AUD_INFO_FMT_SHFT         0x0000001c    /* shft for ac3/mpeg format identifier */
#define AUD_INFO_FMT_MPEG         0x00          /* mpeg format identifier */ 
#define AUD_INFO_FMT_AC3          0x03          /* ac3 format identifier */

#define MPG_HEADER_EMPH_MASK      0x00000003    /* Header: Emphasis */
#define MPG_HEADER_CPYRHT_MASK    0x00000008    /* Header: Copyright, 1=YES */
#define MPG_HEADER_MODEX_MASK     0x00000030    /* Header: Mode Extension */
#define MPG_HEADER_MODE_MASK      0x000000c0    /* Header: Mode */
#define MPG_HEADER_PAD_MASK       0x00000200    /* Header: Padding, 1=ON */
#define MPG_HEADER_FREQ_MASK      0x00000c00    /* Header: Sampling Frequency */
#define MPG_HEADER_BITRATE_MASK   0x0000f000    /* Header: Bit rate */
#define MPG_HEADER_PROT_MASK      0x00010000    /* Header: Protection: 1=No CRC present */
#define MPG_HEADER_LAYER_MASK     0x00060000    /* Header: Layer */
#define MPG_HEADER_ID_MASK        0x00080000    /* Header: ID */

#define MPG_HEADER_EMPH_SHFT      0x00000000    /* Header: Emphasis */
#define MPG_HEADER_CPYRHT_SHFT    0x00000003    /* Header: Copyright, 1=YES */
#define MPG_HEADER_MODEX_SHFT     0x00000004    /* Header: Mode Extension */
#define MPG_HEADER_MODE_SHFT      0x00000006    /* Header: Mode */
#define MPG_HEADER_PAD_SHFT       0x00000009    /* Header: Padding, 1=ON */
#define MPG_HEADER_FREQ_SHFT      0x0000000a    /* Header: Sampling Frequency */
#define MPG_HEADER_BITRATE_SHFT   0x0000000c    /* Header: Bit rate */
#define MPG_HEADER_PROT_SHFT      0x00000010    /* Header: Protection: 1=No CRC present */
#define MPG_HEADER_LAYER_SHFT     0x00000011    /* Header: Layer */
#define MPG_HEADER_ID_SHFT        0x00000013    /* Header: ID */

#define AC3_HEADER_FSCOD_MASK     0x03000000    /* Header: Sampling Frequency */
#define AC3_HEADER_FRMSZCOD_MASK  0x003f0000    /* Header: Frame Size Code */
#define AC3_HEADER_BSID_MASK      0x0000f000    /* Header: Bit Stream ID */
#define AC3_HEADER_BSMOD_MASK     0x00000700    /* Header: Bit Stream Mode */
#define AC3_HEADER_ACMOD_MASK     0x00000070    /* Header: Audio Coding Mode */
#define AC3_HEADER_CPYRHT_MASK    0x00010000    /* Header: Copyright, 1=YES */
#define AC3_HEADER_LFEON_MASK     0x00000001    /* Header: LFE channel exists */

#define AC3_HEADER_FSCOD_SHFT     0x00000018    /* Header: Sampling Frequency */
#define AC3_HEADER_FRMSZCOD_SHFT  0x00000010    /* Header: Frame Size Code */
#define AC3_HEADER_BSID_SHFT      0x0000000c    /* Header: Bit Stream ID */
#define AC3_HEADER_BSMOD_SHFT     0x00000008    /* Header: Bit Stream Mode */
#define AC3_HEADER_ACMOD_SHFT     0x00000004    /* Header: Audio Coding Mode */
#define AC3_HEADER_CPYRHT_SHFT    0x00000010    /* Header: Copyright, 1=YES */
#define AC3_HEADER_LFEON_SHFT     0x00000000    /* Header: LFE channel exists */

/*===========================================================================
 * typedef Section
 *===========================================================================*/
typedef bool BOOL;
typedef void* HAUDIO;
typedef void* HCLIP;

/* function return codes */
typedef enum
  {
    CNXT_AUDIO_OK,
    CNXT_AUDIO_BAD_PARAMETER,    /* bad function parameter */
    CNXT_AUDIO_BUSY,             /* audio decoder is busy playing data */
    CNXT_AUDIO_CLIP_FULL,        /* audio clip queue is full, no more clips */
    CNXT_AUDIO_BAD_DATA,         /* bad data encountered in audio clip */
    CNXT_AUDIO_BAD_FORMAT,       /* unrecognized or invalid audio format specified */
    CNXT_AUDIO_NOT_PLAYING,      /* audio decoder is not playing data */
    CNXT_AUDIO_NOT_VALID,        /* data playing is not valid */
    CNXT_AUDIO_FMT_UNKNOWN,      /* unknown audio format */
    CNXT_AUDIO_ERROR             /* other error */
  } CNXT_AUDIO_STATUS;

/* encoded audio clip types */
typedef enum
  {
    CNXT_AUDIO_FMT_AUTO,        /* auto detect the type, used for clips */
    CNXT_AUDIO_FMT_MPEG,        /* mpeg encoded */
    CNXT_AUDIO_FMT_AC3          /* ac3 encoded */
  } CNXT_AUDIO_FORMAT;

/* audio encapsulation method */
typedef enum
  {
    CNXT_AUDIO_ENCAP_AUTO,
    CNXT_AUDIO_ENCAP_ES,
    CNXT_AUDIO_ENCAP_PES        /* PES encap, valid only for clips */
  } CNXT_AUDIO_ENCAPSULATION;

/* encoded data source */
typedef enum
  {
    CNXT_AUDIO_SOURCE_LIVE,   /* encoded data comes from live stream */
    CNXT_AUDIO_SOURCE_CLIP    /* encoded data provided from memory */
  } CNXT_AUDIO_SOURCE;

/* structure to hold encoded audio format information */
typedef struct
{
  CNXT_AUDIO_FORMAT format;
  CNXT_AUDIO_ENCAPSULATION encapsulation;
  CNXT_AUDIO_SOURCE source;
  bool pcrsync;          /* true - sync audio with PCR */
} cnxt_audio_format_t, CNXT_AUDIO_CONFIG;

/* selectable channels for mono playing */
typedef enum
  {
    CNXT_AUDIO_CHAN_DEFAULT,
    CNXT_AUDIO_CHAN_FRONT_LEFT_RIGHT,
    CNXT_AUDIO_CHAN_LT_RT,
    CNXT_AUDIO_CHAN_LO_RO,
    CNXT_AUDIO_CHAN_FRONT_LEFT,
    CNXT_AUDIO_CHAN_FRONT_RIGHT,
    CNXT_AUDIO_CHAN_FRONT_CENTER,
    CNXT_AUDIO_CHAN_REAR_LEFT,
    CNXT_AUDIO_CHAN_REAR_RIGHT,
    CNXT_AUDIO_CHAN_MIXED_MONO
  } CNXT_AUDIO_CHAN;

/* audio clip playing events */
typedef u_int32 CNXT_AUDIO_EVENT;

#define EV_CLIP_PLAY    0x00000001
#define EV_CLIP_STOPPED 0x00000002
#define EV_CLIP_FEED    0x00000004
#define EV_CLIP_COPIED  0x00000008
#define EV_CLIP_ERROR   0x00000010

/* clip info data for callback function */
typedef struct
{
  HCLIP hclip;            /* clip handle associated with the event */
  unsigned int errdata;   /* if error, index of byte with error */
} clip_ev_data_t, CNXT_CLIP_EV_DATA;

typedef enum
  {
    CHAN_MODE_STEREO,
    CHAN_MODE_JOINT_STEREO,
    CHAN_MODE_DUAL_MONO, /* dual mono */
    CHAN_MODE_MONO,
    CHAN_MODE_MULTI      /* multi-channel; surround, etc */
    /*
     * Note: customer also asked for multi-channel mono,
     * multi-channel stereo, and pro-logic.  It is not obvious how
     * this information can be detected.  If customer suggests a method
     * for detecting these other types, then they can be implemented.
     CHAN_MODE_MULTI_MONO,
     CHAN_MODE_MULTI_STEREO,
     CHAN_MODE_PROLOGIC
    */
  } CNXT_AUDIO_CHAN_MODE;

typedef enum
  {
    CHAN_MODEX_SUB_4_31,
    CHAN_MODEX_SUB_8_31,
    CHAN_MODEX_SUB_12_31,
    CHAN_MODEX_SUB_16_31
  } CNXT_AUDIO_CHAN_MODEX;

typedef enum
  {
    CNXT_AUDIO_MPEG_LAYER_NA,     /* N/A, for example if AC3 */
    CNXT_AUDIO_MPEG_LAYER_III,
    CNXT_AUDIO_MPEG_LAYER_II,
    CNXT_AUDIO_MPEG_LAYER_I
  } CNXT_AUDIO_MPEG_LAYER;

typedef enum
  {
    CNXT_AUDIO_EMPH_NA,
    CNXT_AUDIO_EMPH_50_15,
    CNXT_AUDIO_EMPH_RSVD,
    CNXT_AUDIO_EMPH_CCITT
  } CNXT_AUDIO_EMPH;

typedef u_int32 CHAN_FLAG;

#define CHAN_FLAG_FRONT_LEFT     0x00000001
#define CHAN_FLAG_FRONT_RIGHT    0x00000002
#define CHAN_FLAG_FRONT_CENTER   0x00000004
#define CHAN_FLAG_REAR_LEFT      0x00000008
#define CHAN_FLAG_REAR_RIGHT     0x00000010
#define CHAN_FLAG_REAR_CENTER    0x00000020
#define CHAN_FLAG_LFE            0x00000040

/* bit stream mode definitions for AC3 */
#define BSMOD_CM    0    /* complete main */
#define BSMOD_ME    1    /* music and effects */
#define BSMOD_VI    2    /* visually impaired */
#define BSMOD_HI    3    /* hearing impaired */
#define BSMOD_D     4    /* dialogue */
#define BSMOD_C     5    /* commentary */
#define BSMOD_E     6    /* emergency */
#define BSMOD_VO    7    /* voice over/karaoke */

/* audio coding mode definitions for AC3 */
#define ACMOD_DUAL_MONO   0   /* Ch1, Ch2 */
#define ACMOD_1_0         1   /* C */
#define ACMOD_2_0         2   /* L,R */
#define ACMOD_3_0         3   /* L,C,R */
#define ACMOD_2_1         4   /* L,R,S */
#define ACMOD_3_1         5   /* L,C,R,S */
#define ACMOD_2_2         6   /* L,R,SL,SR */
#define ACMOD_3_2         7   /* L,C,R,SL,SR */

typedef struct
{
  CNXT_AUDIO_MPEG_LAYER layer;    /* mpeg audio layer, 1, 2, or 3 */
  unsigned int sampfreq;          /* sampling frequency in kHz; 44--> 44.1 kHz */
  BOOL halffreq;                  /* true: sampling is half indicated */
  unsigned int bitrate;           /* bit rate in kbps; */
  unsigned int totalchans;        /* total number of channels */
  CNXT_AUDIO_CHAN_MODE chanmode;  /* channel mode */
  CHAN_FLAG avail_chans;          /* bit map of available channels */
  CNXT_AUDIO_EMPH emphasis;       /* emphasis */
  BOOL copyright;                 /* copyright flag, 1 == YES */
  CNXT_AUDIO_CHAN_MODEX modex;    /* channel mode extension */
  BOOL padding;                   /* true: padding on */
  BOOL prot;                      /* protection, 1 == no CRC */
} CNXT_AUDIO_FMT_INFO_MPEG;

typedef struct
{
  unsigned int sampfreq;          /* sampling frequency in kHz; 44--> 44.1 kHz */
  unsigned int bitrate;           /* bit rate in kbps; */
  unsigned int totalchans;        /* total number of channels */
  CNXT_AUDIO_CHAN_MODE chanmode;  /* channel mode */
  CHAN_FLAG avail_chans;          /* bit map of available channels */
  BOOL copyright;                 /* copyright flag, 1 == YES */
  int bsid;                       /* bit stream identification */
  int bsmod;                      /* bit stream mode */
  int acmod;                      /* audio coding (channel) mode */
} CNXT_AUDIO_FMT_INFO_AC3;

typedef struct
{
  BOOL playing;                   /* true if decoder is playing */
  CNXT_AUDIO_FORMAT encfmt;       /* encoded format */
  CNXT_AUDIO_ENCAPSULATION encap; /* encapsulation, PES or ES */
  union
  {
    CNXT_AUDIO_FMT_INFO_MPEG mpeg;
    CNXT_AUDIO_FMT_INFO_AC3 ac3;
  } fmt;                      /* format specific info data */
} CNXT_AUDIO_INFO;

typedef CNXT_AUDIO_STATUS (*cnxt_audio_callback_t)
     ( HAUDIO haudio, CNXT_AUDIO_EVENT event, void *pevdata, void *pinfo );

typedef CNXT_AUDIO_STATUS (*CNXT_AUDIO_CALLBACK_PFN)
     ( HAUDIO haudio, CNXT_AUDIO_EVENT event, void *pevdata, void *pinfo );



/*===========================================================================
 * PUBLIC prototype Section 
 *===========================================================================*/
PUBLIC bool         cnxt_audio_init( bool useAC3ucode );
PUBLIC void         cnxt_audio_empty_int(u_int32 idx);
PUBLIC void         cnxt_audio_lowwater_int(u_int32 idx);
PUBLIC void         cnxt_audio_pts_received_int(u_int32 idx);
PUBLIC CNXT_AUDIO_STATUS cnxt_audio_open( HAUDIO *phaudio );
PUBLIC CNXT_AUDIO_STATUS cnxt_audio_close( HAUDIO haudio );
PUBLIC CNXT_AUDIO_STATUS cnxt_audio_get_instances( int *pinstances );
PUBLIC CNXT_AUDIO_STATUS cnxt_audio_register_callback(HAUDIO haudio, 
                                        CNXT_AUDIO_CALLBACK_PFN pfn_callback,
                                        void *pinfo);
PUBLIC CNXT_AUDIO_STATUS cnxt_audio_set_thresholds(HAUDIO haudio,
                                                 u_int32    play_threshold,
                                                 u_int32    request_threshold);
PUBLIC CNXT_AUDIO_STATUS cnxt_audio_start( HAUDIO haudio, 
                                           CNXT_AUDIO_CONFIG *pfmt);
PUBLIC CNXT_AUDIO_STATUS cnxt_audio_stop( HAUDIO haudio );
PUBLIC CNXT_AUDIO_STATUS cnxt_audio_pause( HAUDIO haudio );
PUBLIC CNXT_AUDIO_STATUS cnxt_audio_resume( HAUDIO haudio );
PUBLIC CNXT_AUDIO_STATUS cnxt_audio_write_data_async(  HAUDIO   haudio,
                                                       void     *pbuf,
                                                       u_int32  length,
                                                       HCLIP    *phclip );
PUBLIC CNXT_AUDIO_STATUS cnxt_audio_get_info( HAUDIO            haudio, 
                                              CNXT_AUDIO_INFO   *pinfo,
                                              void              *pbuf,
                                              u_int32           length);
PUBLIC CNXT_AUDIO_STATUS cnxt_audio_select_channels(   HAUDIO haudio,
                                                 CNXT_AUDIO_CHAN   chan,
                                                 CNXT_AUDIO_CHAN   *prevchan );

/*===========================================================================
 * PUBLIC variable  Section 
 *===========================================================================*/

#endif /* _AUD_API_H_ */

/****************************************************************************
 * Modifications:
 * $Log: 
 *  8    mpeg      1.7         4/6/04 1:47:04 PM      Joe Kroesche    CR(s) 
 *        8578 8793 : updated the naming style of some data types for 
 *        consistency, replaced the
 *        following: cnxt_audio_format_t becomes CNXT_AUDIO_CONFIG;
 *        clip_ev_data_t becomes CNXT_CLIP_EV_DATA; and
 *        cnxt_audio_callback_t becomes CNXT_AUDIO_CALLBACK_PFN
 *        All the original type names were retained so existing code should not
 *         break
 *  7    mpeg      1.6         2/13/04 11:32:18 AM    Matt Korte      CR(s) 
 *        8406 : Changed AUDIO_ to CNXT_AUDIO_
 *        
 *  6    mpeg      1.5         2/27/03 6:21:34 PM     Dave Aerne      SCR(s) 
 *        4672 :
 *        added code to support AUTO_DETECT of encoded audio format for live 
 *        streams.
 *        Format is dynamically determined once stream feed has begun and 
 *        correct
 *        microcode is downloaded.
 *        
 *  5    mpeg      1.4         12/5/02 6:56:32 PM     Dave Aerne      SCR(s) 
 *        4991 :
 *        reordered AUDIO_CHAN enum to have _LEFT before _RIGHT
 *        
 *  4    mpeg      1.3         12/2/02 5:28:20 PM     Craig Dry       SCR(s) 
 *        4991 :
 *        correction to enum definition for AUDIO_MPEG_LAYER; previously was 
 *        AUDIO_LAYER
 *        
 *  3    mpeg      1.2         11/26/02 8:37:14 PM    Dave Aerne      SCR(s) 
 *        4991 :
 *        correction to AUDIO_CHAN enum to remove unsupported 
 *        AUDIO_CHAN_REAR_CENTER entry
 *        
 *        
 *  2    mpeg      1.1         11/25/02 7:41:32 PM    Dave Aerne      SCR(s) 
 *        4991 :
 *        added and expanded extensions for MPEG and AC3 header info
 *        
 *        
 *  1    mpeg      1.0         11/20/02 10:20:32 AM   Craig Dry       
 * $
 * 
 *    Rev 1.5   27 Feb 2003 18:21:34   aernedj
 * SCR(s) 4672 :
 * added code to support AUTO_DETECT of encoded audio format for live streams.
 * Format is dynamically determined once stream feed has begun and correct
 * microcode is downloaded.
 * 
 *    Rev 1.4   05 Dec 2002 18:56:32   aernedj
 * SCR(s) 4991 :
 * reordered AUDIO_CHAN enum to have _LEFT before _RIGHT
 * 
 *    Rev 1.3   02 Dec 2002 17:28:20   dryd
 * SCR(s) 4991 :
 * correction to enum definition for AUDIO_MPEG_LAYER; previously was AUDIO_LAYER
 * 
 *    Rev 1.2   26 Nov 2002 20:37:14   aernedj
 * SCR(s) 4991 :
 * correction to AUDIO_CHAN enum to remove unsupported AUDIO_CHAN_REAR_CENTER entry
 * 
 * 
 *    Rev 1.1   25 Nov 2002 19:41:32   aernedj
 * SCR(s) 4991 :
 * added and expanded extensions for MPEG and AC3 header info
 * 
 * 
 *    Rev 1.0   20 Nov 2002 10:20:32   dryd
 * SCR(s) 4991 :
 * Canal+ DLI4.2 Audio Extenstions and Audio Driver Enhancements
 *
 ****************************************************************************/ 

