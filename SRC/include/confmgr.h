/****************************************************************************/
/*                 Rockwell Semiconductor Systems - SABINE                  */
/****************************************************************************/
/*                                                                          */
/* Filename:           CONFMGR.H                                            */
/*                                                                          */
/* Description:        Functions managing the storage and retrieval of      */
/*                     flash configuration data on behalf of Sabine drivers.*/
/*                                                                          */
/* Author:             Dave Wilson                                          */
/*                                                                          */
/* Copyright Conexant Systems, 1998-2004                                    */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/* Note:                                                                    */
/*                                                                          */
/* This is a dual mode header file used by both native and O-code apps. DO  */
/* NOT use // comments in any area of the file which is not protected by    */
/* #ifndef __ocod__.                                                        */
/*                                                                          */
/* When included by the o-code compiler, only the config structure labels   */
/* are required.                                                            */
/*                                                                          */
/****************************************************************************/
/* $Id: confmgr.h,v 1.50, 2004-04-01 15:00:17Z, Billy Jackman$
 ****************************************************************************/ 

#ifndef _CONFIGMGR_H_
#define _CONFIGMGR_H_

#ifndef __ocod__
#include "oconfig.h"
#endif

#include "basetype.h"

/**********************************************/
/* Values used in the configuration structure */
/**********************************************/

#define MODULATION_QPSK_SHFT    (1 << 4)

#define MODULATION_MASK         (31 << 4)

#define W_E_MASK                (1 << 11)

#define NIM_SATELLITE_DEMOD     1
#define NIM_OPENTV_BASEBAND     2
#define NIM_DVB_BASEBAND        3
#define NIM_CABLE               4
#define NIM_HSDP                5
#define NIM_IEEE1394            6
#define NIM_TERRESTRIAL         7

#define ASPECT43   1
#define ASPECT169  2

#define VIDEO_RGB       1
#define VIDEO_COMPOSITE 2

#define AUDIO_SOURCE_INTERNAL 0
#define AUDIO_SOURCE_EXTERNAL 1

#define WATCHDOG_ENABLED 0x80000000

#define SVL_USE_TRANSPORT  1
#define SVL_USE_NIT        2
#if CUSTOMER == VENDOR_D
#define SVL_USE_TSD		     3	 
#endif

#ifndef __ocod__
/* Defined otherwise in OPENTV.H for app use */
#define PANSCAN    1
#define LETTERBOX  2
#endif

/***************************/
/* Device flag definitions */
/***************************/
#define REMOTE_FLAG_MASK         0x00000001
  #define NEW_REMOTE_PRESENT       0x00000001
  #define OLD_REMOTE_PRESENT       0x00000000

#define KEYBOARD_FLAG_MASK       0x00000002  
  #define NEW_KEYB_PRESENT         0x00000002
  #define OLD_KEYB_PRESENT         0x00000000

#define TELETEXT_FLAG_MASK       0x00000004
  #define TELETEXT_ENABLE          0x00000004
  #define TELETEXT_DISABLE         0x00000000

#define CLOSED_CAPTION_FLAG_MASK 0x00000008
  #define CLOSED_CAPTION_ENABLE    0x00000008
  #define CLOSED_CAPTION_DISABLE   0x00000000

#define RFMOD_FLAG_MASK 0x00000030   /* RF Modulator */
  #define RFMOD_OFF         0x00000000
  #define RFMOD_PASSTHRU    0x00000010
  #define RFMOD_CH3         0x00000020
  #define RFMOD_CH4         0x00000030

/************************/
/* LNB flag definitions */
/************************/
#define LNB_TYPE_MASK            0x0000000F
  #define LNB_TYPE_SINGLE          0x00000000
  #define LNB_TYPE_DUAL            0x00000001
  #define LNB_TYPE_ORBITAL         0x00000002
  #define LNB_TYPE_MANUAL          0x00000003
#define LNB_LONG_EW_A_MASK       0x00000010
  #define LNB_LONG_EAST_A          0x00000000
  #define LNB_LONG_WEST_A          0x00000010
#define LNB_LONG_EW_B_MASK       0x00000020
  #define LNB_LONG_EAST_B          0x00000000
  #define LNB_LONG_WEST_B          0x00000020
#define LNB_LONG_EW_C_MASK       0x00000040
  #define LNB_LONG_EAST_C          0x00000000
  #define LNB_LONG_WEST_C          0x00000040
#define LNB_22KHZ_A_MASK         0x00000080
  #define LNB_22KHZ_OFF_A          0x00000000
  #define LNB_22KHZ_ON_A           0x00000080
#define LNB_22KHZ_B_MASK         0x00000100
  #define LNB_22KHZ_OFF_B          0x00000000
  #define LNB_22KHZ_ON_B           0x00000100
#define LNB_22KHZ_C_MASK         0x00000200
  #define LNB_22KHZ_OFF_C          0x00000000
  #define LNB_22KHZ_ON_C           0x00000200

/* CM/MPEG uart sharing, ownership details */
#define SHARED_UART_OWNER_CM     1  /* Shared debug port owner? */
#define SEPARATE_CM_MPEG_UARTS   2  /* Share or use separate Uarts */

#ifndef __ocod__

/***************************************/
/* Sabine Configuration Data Structure */
/***************************************/

/************************************************************************/
/*                                                                      */
/* IMPORTANT:                                                           */
/*                                                                      */
/* Please add new fields to the end of this structure only! This allows */
/* compatibility with downlevel versions in flash during development.   */
/*                                                                      */
/* Most fields in this structure should be user-settable via the menu   */
/* application. If you need additions or changes to the menu app,       */
/* see Dave Wilson.                                                     */
/*                                                                      */
/************************************************************************/
typedef struct
{
  u_int32 length;
  u_int32 checksum; /* Sum of fields below this one */
  u_int32 magic;

  /*************************************/
  /* Audio and video setup information */
  /*************************************/
  u_int16  video_standard;
  u_int16  video_format;
  u_int16  video_aspect;
  u_int16  video_connector;
  u_int16  audio_sync;
  u_int16  audio_format;
  u_int16  audio_source;
  u_int16  deemphasis;
  u_int16  clock_recovery;

  /*************************/
  /* Interface information */
  /*************************/
  u_int16  nim_type;            /* satellite, terrestrial, cable, baseband, etc. */

  /* Satellite-specific information */
  u_int32  frequency_bcd_ghz;   /* BCD 3.5 in GHz */
  u_int32  symbol_rate_bcd_mss; /* BCD 4.4 in MS/s */
  u_int16  fec_inner;
  u_int16  polarisation;
  /* LNB information */
  u_int32  lnb_a_bcd_ghz;       /* BCD 3.5 in GHz */
  u_int32  lnb_b_bcd_ghz;       /* BCD 3.5 in GHz */
  u_int32  lnb_c_bcd_ghz;       /* BCD 3.5 in GHz */
  u_int32  lnb_flags;
  u_int16  orbit_pos_a;
  u_int16  orbit_pos_b;
  u_int16  orbit_pos_c;
  u_int8   right_is_low;        /* Low control voltage selects right polarization */

  /* Terrestrial-specific information */
  u_int32  frequency_bcd_mhz;   /* BCD 3.5 in MHz */
  u_int8   bandwidth;

  /* Cable-specific information */
  u_int32             frequency_hz;
  u_int32             symbol_rate;
  u_int16             modulation_type;
  u_int16             spectral_inversion;
  u_int32             annex;
  u_int32             uart_flags;   /* CM/MPEG uart sharing, ownership details */
  u_int32             cmMode; 
  u_int32             Mode;
  u_int8              uNumMacAddr;

  /*********************************************/
  /* Analog TV tuning and picture control info */
  /*********************************************/
  u_int8   channel;
  u_int8   tuning_scheme;
  u_int8   country;
  u_int8   analog_source;
  u_int32  channel_map[4];

  /****************************/
  /* Encoder picture settings */
  /****************************/
  u_int8   brightness;
  u_int8   contrast;
  u_int8   color;

  /**********************************************/
  /* Astra service list flags (OpenTV EN2 only) */
  /**********************************************/
  u_int8  svl_chosen;
  u_int16 svl_service;

  /***************/
  /* Debug level */
  /***************/
  u_int32  debug_level;
  u_int32  enable_opentv_download;

  /****************/
  /* Device flags */
  /****************/
  u_int32  device_flags;  /* New/old remote, new/old keyboard in use, watchdog on/off */

  /* Additional satellite-specific information */
  int16    orbital_position;    /* tenths of degree, negative is West longitude */

  u_int8   padding1;            /* add 3 bytes of padding to make it 32-bit aligned */
  u_int8   padding2;
  u_int8   padding3;
  u_int8   MacAddr[48];         /* 8 MPEG MAC addresses */

  /****************/
  /* Power Calibration for US and DS in Open Cable */
  /****************/
  u_int16  PowerCalPad;
  u_int16  UpstreamPowerGain;
  u_int16  UpstreamPowerOffset;
  u_int32  DownstreamPowerCalFreq0;
  u_int8   DownstreamPowerCalSDPMin0;
  u_int8   DownstreamPowerCalSDPMax0;
  u_int8   DownstreamPowerCalPad00;
  u_int8   DownstreamPowerCalPad01;
  u_int32  DownstreamPowerCalFreq1;
  u_int8   DownstreamPowerCalSDPMin1;
  u_int8   DownstreamPowerCalSDPMax1;
  u_int8   DownstreamPowerCalPad10;
  u_int8   DownstreamPowerCalPad11;
  u_int32  DownstreamPowerCalFreq2;
  u_int8   DownstreamPowerCalSDPMin2;
  u_int8   DownstreamPowerCalSDPMax2;
  u_int8   DownstreamPowerCalPad20;
  u_int8   DownstreamPowerCalPad21;
  /****************/
  /* Power Calibration for DS in inband video */
  /****************/
  u_int32  InbandVideoDownstreamPowerCalFreq0;
  u_int8   InbandVideoDownstreamPowerCalSDPMin0;
  u_int8   InbandVideoDownstreamPowerCalSDPMax0;
  u_int8   InbandVideoDownstreamPowerCalPad00;
  u_int8   InbandVideoDownstreamPowerCalPad01;
  u_int32  InbandVideoDownstreamPowerCalFreq1;
  u_int8   InbandVideoDownstreamPowerCalSDPMin1;
  u_int8   InbandVideoDownstreamPowerCalSDPMax1;
  u_int8   InbandVideoDownstreamPowerCalPad10;
  u_int8   InbandVideoDownstreamPowerCalPad11;
  u_int32  InbandVideoDownstreamPowerCalFreq2;
  u_int8   InbandVideoDownstreamPowerCalSDPMin2;
  u_int8   InbandVideoDownstreamPowerCalSDPMax2;
  u_int8   InbandVideoDownstreamPowerCalPad20;
  u_int8   InbandVideoDownstreamPowerCalPad21;
} sabine_config_data;

typedef struct
{
  u_int32 length;
  u_int32 checksum;
  u_int32 magic;
  u_int32 ethernet_addr;
  u_int32 gateway_addr;
  u_int32 subnet_mask;
  u_int32 debug_baud;
  bool    enable_lan;
} bsp_config_data;

/********************/
/* Access Functions */
/********************/

/***************************************************************************/
/* Retrieve a pointer to the current configuration data. This pointer will */
/* be to a copy of the data in RAM that the caller can then modify as      */
/* required. While the config data is locked, no other process may access  */
/* it.                                                                     */
/***************************************************************************/
sabine_config_data *config_lock_data(void);
bsp_config_data *config_lock_bsp_data(void);

/***************************************************************************/
/* Unlock the config data. This frees up the data for another process to   */
/* lock.                                                                   */
/***************************************************************************/
void config_unlock_data(sabine_config_data *pConfig);
void config_unlock_bsp_data(bsp_config_data *pConfig);

/***************************************************************************/
/* Write the latest config data to flash if it is different from the       */
/* version currently stored there.                                         */
/***************************************************************************/
bool config_write_data(void);

#endif /* ifndef __ocod__ */

#endif

/****************************************************************************
 * Modifications:
 * $Log: 
 *  51   mpeg      1.50        4/1/04 9:00:17 AM      Billy Jackman   CR(s) 
 *        8722 8723 : Added definition of LNB_TYPE_MANUAL for manual LNB 
 *        signalling control.
 *  50   mpeg      1.49        11/5/03 2:08:29 PM     Song Qiao       CR(s): 
 *        7752 7824 
 *        Modified sabine_config_data to include inband video downstream power 
 *        calibration settings.
 *        
 *  49   mpeg      1.48        9/26/03 1:58:08 PM     Billy Jackman   SCR(s) 
 *        7555 :
 *        Handle config manager entries as native types.
 *        
 *  48   mpeg      1.47        9/25/03 6:27:54 PM     Billy Jackman   SCR(s) 
 *        7553 :
 *        Replace two include directives with the contents of the files to 
 *        avoid a
 *        build break for one night only.
 *        
 *  47   mpeg      1.46        7/23/03 7:31:36 PM     Sunil Cheruvu   SCR(s): 
 *        6681 
 *        Added the US and DS Power calibration infrastructure for OC_OOBFE
 *        
 *  46   mpeg      1.45        7/10/03 8:56:34 PM     Song Qiao       SCR(s): 
 *        6912 
 *        Added 48 bytes of MacAddr into sabine_config_data structure.
 *        
 *        
 *  45   mpeg      1.44        7/7/03 4:36:16 PM      Dave Wilson     SCR(s) 
 *        6817 :
 *        Added more #ifndef __ocod__ lines to allow the latest version of the 
 *        header
 *        to compile when building OpenTV SDK applications.
 *        
 *  44   mpeg      1.43        6/30/03 2:28:24 PM     Billy Jackman   SCR(s) 
 *        5816 :
 *        Added #defines for LNB flags field and added orbital_position field 
 *        in config
 *        structure for use in DirecTV tuning.
 *        
 *  43   mpeg      1.42        6/26/03 3:54:40 PM     Billy Jackman   SCR(s) 
 *        6849 :
 *        Remove un-needed include files.
 *        
 *  42   mpeg      1.41        6/26/03 2:24:02 PM     Ian Mitchell    SCR(s): 
 *        6849 
 *        Removed data for FTA as it is not used any more.
 *        
 *  41   mpeg      1.40        6/25/03 6:17:44 PM     Billy Jackman   SCR(s) 
 *        6844 :
 *        Modified sabine_config_data structure to get rid of conditional 
 *        inclusion of
 *        some sections.  Added the fields necessary to allow coexistence of 
 *        satellite,
 *        terrestrial, and cable interfaces.
 *        
 *  40   mpeg      1.39        5/9/03 5:54:38 PM      Sahil Bansal    SCR(s): 
 *        6286 6287 
 *        Added support for cm mode selection to watchtv
 *        
 *  39   mpeg      1.38        4/26/03 5:30:32 PM     Sahil Bansal    SCR(s): 
 *        6110 
 *        Added 2 parms to config structure for OC_OOBFE driver
 *        
 *  38   mpeg      1.37        1/28/03 5:17:02 PM     Angela Swartz   SCR(s) 
 *        5307 :
 *        added RFMOD support
 *        
 *  37   mpeg      1.36        11/29/02 6:55:32 AM    Ian Mitchell    SCR(s): 
 *        4876 
 *        Added data to the sabine structure if the build is FTA.
 *        
 *  36   mpeg      1.35        8/21/02 12:07:42 PM    Miles Bintz     SCR(s) 
 *        4436 :
 *        Updates for ANNEX_[A | B] enumereation.  Move MOD_QAM #define from 
 *        demod.h and used enumeration from demod_types instead.
 *        
 *        
 *  35   mpeg      1.34        7/25/02 1:50:52 PM     Dave Wilson     SCR(s) 
 *        2775 :
 *        Minor changes for new transport stream service list manager. This 
 *        replaces
 *        the broken Astra service list manager shipped by OpenTV.
 *        
 *  34   mpeg      1.33        6/25/02 11:46:48 AM    Steven Jones    SCR(s): 
 *        4055 
 *        Add demod terrestrial nim #define.
 *        
 *  33   mpeg      1.32        6/13/02 4:10:16 PM     Miles Bintz     SCR(s) 
 *        4001 :
 *        Changed ifdef to if has_cable_demod
 *        
 *        
 *  32   mpeg      1.31        6/13/02 12:50:56 PM    Miles Bintz     SCR(s) 
 *        4001 :
 *        Added parameters for cable tuning
 *        
 *  31   mpeg      1.30        6/6/02 5:46:16 PM      Dave Wilson     SCR(s) 
 *        3952 :
 *        Moved the remote type device flag definitions from HWCONFIG.H
 *        Added new flags for closed captioning and teletext.
 *        
 *  30   mpeg      1.29        5/31/02 12:45:26 PM    Senthil Veluswamy SCR(s) 
 *        3869 :
 *        Added separate Debug Uart option/selection for Wabash
 *        
 *  29   mpeg      1.28        4/23/02 8:00:32 PM     Senthil Veluswamy SCR(s) 
 *        3521 :
 *        renamed uart3 field as debug port field
 *        
 *        
 *  28   mpeg      1.27        4/19/02 7:36:50 PM     Senthil Veluswamy SCR(s) 
 *        3521 :
 *        Wrapped uart owner & removed mac addr data field
 *        
 *  27   mpeg      1.26        12/20/01 3:38:00 PM    Senthil Veluswamy SCR(s) 
 *        2982 :
 *        Added 2 new fields (MPEG_MAC, UART3 owner) to the Sabine Config data 
 *        structure 
 *        
 *  26   mpeg      1.25        2/16/01 5:14:46 PM     Dave Wilson     DCS1217: 
 *        Changes for Vendor D rev 7 board.
 *        
 *  25   mpeg      1.24        7/10/00 3:33:28 PM     Dave Wilson     Moved a 
 *        couple of #defines out of the area included by ocode apps since they
 *        conflicted with some similarly named labels in the OpenTV 1.2 version
 *         of 
 *        opentv.h
 *        
 *  24   mpeg      1.23        4/13/00 10:10:52 AM    Dave Wilson     Added new
 *         HSDP and 1394 connection types for Klondike
 *        
 *  23   mpeg      1.22        3/1/00 6:40:08 PM      Dave Wilson     Moved 
 *        some FEC and polarisation label definitions from CONFMGR.H to DEMOD.H
 *        and renamed some to minimise confusion
 *        
 *  22   mpeg      1.21        12/14/99 12:25:42 PM   Dave Wilson     Added new
 *         config fields
 *        
 *  21   mpeg      1.20        10/29/99 5:28:06 PM    Dave Wilson     Added new
 *         structure for bsp config data.
 *        
 *  20   mpeg      1.19        10/14/99 7:10:42 PM    Dave Wilson     Added 
 *        device_flags fields
 *        
 *  19   mpeg      1.18        8/23/99 2:32:46 PM     Ismail Mustafa  Renamed 
 *        MODULATION_QPSK to MODULATION_QPSK_SHFT.
 *        
 *  18   mpeg      1.17        3/17/99 5:01:12 PM     Dave Wilson     Added 
 *        tuner channel map fields to config data structure.
 *        
 *  17   mpeg      1.16        2/16/99 8:22:52 AM     Dave Wilson     Added new
 *         menu setting to disable OpenTV download
 *        
 *  16   mpeg      1.15        1/21/99 12:35:16 PM    Dave Wilson     Added 
 *        field to allow menu to enable and disable clock recovery.
 *        
 *  15   mpeg      1.14        1/10/99 9:54:22 AM     Dave Wilson     Analog TV
 *         setup and ploarization signal fields added.
 *        
 *  14   mpeg      1.13        12/17/98 5:08:58 PM    Dave Wilson     Added 
 *        debug_level field to config structure.
 *        
 *  13   mpeg      1.12        12/17/98 2:55:44 PM    Dave Wilson     Removed 
 *        transport stream ID and service ID fields.
 *        
 *  12   mpeg      1.11        12/7/98 11:10:32 AM    Dave Wilson     Added 
 *        fields for av_sync, tsid and service_id.
 *        
 *  11   mpeg      1.10        10/13/98 12:16:00 PM   Dave Wilson     Added new
 *         video connector field and renamed some fields for consistency.
 *        
 *  10   mpeg      1.9         10/12/98 5:48:20 PM    Dave Wilson     Put back 
 *        v1.7 since I accidentally overwrote it with an older version.
 *        
 *  9    mpeg      1.8         10/12/98 3:51:32 PM    Dave Wilson     Renamed 
 *        the LNB frequency fields.
 *        
 *  8    mpeg      1.7         10/5/98 4:53:02 PM     Steve Glennon   Changed 
 *        names of some of fields to reflect better the contents - frequencies
 *        are in BCD 3.5 in GHz, symbol rate in BCD 4.4 in MSym/s
 *        
 *  7    mpeg      1.6         9/25/98 5:50:06 PM     Dave Wilson     Added new
 *         fields for audio source and deemphsis selection.
 *        
 *  6    mpeg      1.5         9/14/98 12:39:26 PM    Dave Wilson     Added 
 *        another conditional compile clause round #include <sysvars.h> to 
 *        prevent o-code builds falling over during resource compilation.
 *        
 *  5    mpeg      1.4         9/11/98 3:32:04 PM     Steve Glennon   Added a 
 *        W_E mask value
 *        Added notation that symbol rate stored i n KSymbols/Sec
 *        
 *  4    mpeg      1.3         9/11/98 11:28:44 AM    Dave Wilson     Added 
 *        pSOS BSP init vars functions.
 *        Updated sabine_config_data structure with checksum and new fields.
 *        
 *  3    mpeg      1.2         9/4/98 6:00:00 PM      Steve Glennon   Added 
 *        #defines for masks for modulation, FEC etc.
 *        Added fields to config structure for LNB frequencies
 *        Added field to config structure for transport source
 *        Added #define for cable modem transport source
 *        
 *  2    mpeg      1.1         7/27/98 3:42:42 PM     Dave Wilson     Compiles 
 *        and works with o-code to native wrappers.
 *        
 *  1    mpeg      1.0         7/16/98 3:13:48 PM     Dave Wilson     
 * $
 * 
 ****************************************************************************/ 



