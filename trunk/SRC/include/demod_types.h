#ifndef __DEMOD_TYPES
#define __DEMOD_TYPES

/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                     Conexant Systems Inc. (c) 2002                       */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename: demod_types.h
 *
 * Description: This file contains the type specifications used by the
 *              demod driver.
 *
 * Author: Billy Jackman
 *
 ****************************************************************************/
/* $Id: demod_types.h,v 1.28, 2004-06-30 09:26:20Z, Steven Shen$
 ****************************************************************************/

typedef enum
{
   DEMOD_SUCCESS,
   DEMOD_ERROR,
   DEMOD_ASYNCHRONOUS,
   DEMOD_BAD_UNIT,
   DEMOD_BAD_NETWORK,
   DEMOD_BAD_FTABLE,
   DEMOD_BAD_HANDLE,
   DEMOD_BAD_PARAMETER,
   DEMOD_NOT_SUPPORTED,
   DEMOD_NOT_SATELLITE,
   DEMOD_INITIALIZED,
   DEMOD_UNINITIALIZED,
   DEMOD_CONTROLLED,
   DEMOD_NOT_CONTROLLED,
   DEMOD_MODULE_COUNT,
   DEMOD_UNIT_COUNT,
   DEMOD_OPEN_COUNT,
   DEMOD_NOT_LOCKED,
   DEMOD_BAD_COMMAND,
   DEMOD_IS_BUSY,
   DEMOD_UNIMPLEMENTED,
   DEMOD_NO_HARDWARE
} DEMOD_STATUS;

typedef enum
{
   TUNE_TO_TRANSPONDER  = 1,
   SCAN_FOR_TRANSPONDER,
   DISCONNECT,
   ABORT_SCAN
}
DEMOD_COMMANDS;

typedef enum
{
   DEMOD_NIM_SATELLITE        = 1,
   DEMOD_NIM_OPENTV_BASEBAND  = 2,
   DEMOD_NIM_DVB_BASEBAND     = 3,
   DEMOD_NIM_CABLE            = 4,
   DEMOD_NIM_HSDP             = 5,
   DEMOD_NIM_IEEE1394         = 6,
   DEMOD_NIM_TERRESTRIAL      = 7,
		DEMOD_NIM_ANALOGUE_TERRESTRIAL = 8
}
DEMOD_NIM_TYPE;

typedef enum
{
   M_HORIZONTAL = 1,
   M_VERTICAL,
   M_LEFT,
   M_RIGHT
} NIM_SATELLITE_POLARISATION;

typedef enum
{
   M_RATE_NONE = 1,
   M_RATE_1_2,
   M_RATE_2_3,
   M_RATE_3_4,
   M_RATE_5_6,
   M_RATE_7_8,
   M_RATE_4_5,
   M_RATE_3_5,
   M_RATE_6_7,
   M_RATE_5_11
#if defined(DRIVER_INCL_DEMOD_COBRA)
   ,
		M_RATE_AUTO
#endif
} NIM_FEC_RATE;

typedef enum
{
#if defined(DRIVER_INCL_DEMOD_DCF8722)
   /*
    * The Thomson Cable Tuner DCF872x supports these QAM modes,
    * including 32-QAM, 128-QAM and auto-QAM detection mode.
    */
   MOD_QAMAUTO = 0,
   MOD_QAM32   = 32,
   MOD_QAM128  = 128,
#endif /* DRIVER_INCL_DEMOD_DCF8722 */
   MOD_QAM16  = 16,
   MOD_QAM64  = 64,
   MOD_QAM256 = 256
}
NIM_CABLE_MODULATION;

typedef enum
{
   ANNEX_A    = 1,
   ANNEX_B    = 2
}
NIM_CABLE_ANNEX;

typedef enum
{
   SPECTRUM_NORMAL = 1,
   SPECTRUM_INVERTED
} NIM_CABLE_SPECTRUM;

typedef enum
{
   SAT_SPECTRUM_NORMAL = 0,
   SAT_SPECTRUM_INVERTED,
   SAT_SPECTRUM_AUTO
} NIM_SATELLITE_SPECTRUM;

typedef struct
{
   u_int32                    frequency;
   u_int32                    symbol_rate;
   NIM_SATELLITE_POLARISATION polarisation;
   NIM_FEC_RATE               fec;
   int16                      orbital_position;
   NIM_SATELLITE_SPECTRUM     spectrum;
} NIM_SATELLITE_TUNE;

typedef struct
{
   NIM_CABLE_ANNEX        annex;
   u_int32                frequency;
   u_int32                symbol_rate;
   NIM_CABLE_MODULATION   modulation;
   u_int32                auto_spectrum;
   NIM_CABLE_SPECTRUM     spectrum;
   u_int8                 ucSDPMin; /* RFAGC Min Threshold */
   u_int8                 ucSDPMax; /* RFAGC Max Threshold */
} NIM_CABLE_TUNE;

typedef struct
{
   /* MFB some these things will need enumerating */
   int            rs_time_span;
   int            state_locked_unlocked;
   int            lock_detected;
   int            agc;
   int            snr;
   int            eq_coeff;
   int            main_top_coeff;
   int            correctable_bytes;
   int            uncorrectable_blocks;
   NIM_CABLE_TUNE tuning;
} NIM_CABLE_STATUS;

/* Terrestrial tune enums... */

typedef enum
{
   BANDWIDTH_8_MHZ = 1,
   BANDWIDTH_7_MHZ,
   BANDWIDTH_6_MHZ
} NIM_TERRESTRIAL_INPUT_BANDWIDTH;

typedef enum
{
   FRAME1 = 1,
   FRAME2,
   FRAME3,
   FRAME4,
   FRAME_AUTO
} NIM_TERRESTRIAL_TPS_FRAME;

typedef enum
{
   CONSTELLATION_QPSK = 1,
   CONSTELLATION_16QAM,
   CONSTELLATION_64QAM,
   CONSTELLATION_AUTO
} NIM_TERRESTRIAL_TPS_CONSTELLATION;

typedef enum
{
   HIERARCHY_NONE = 1,
   HIERARCHY_ALPHA1,
   HIERARCHY_ALPHA2,
   HIERARCHY_ALPHA4,
   HIERARCHY_AUTO
} NIM_TERRESTRIAL_TPS_HIERARCHY;

typedef enum
{
   HPCODERATE_1_2 = 1,
   HPCODERATE_2_3,
   HPCODERATE_3_4,
   HPCODERATE_5_6,
   HPCODERATE_7_8,
   HPCODERATE_AUTO
} NIM_TERRESTRIAL_TPS_CODERATE;

typedef enum
{
   GUARDINTERVAL_1_32 = 1,
   GUARDINTERVAL_1_16,
   GUARDINTERVAL_1_8,
   GUARDINTERVAL_1_4,
   GUARDINTERVAL_AUTO
} NIM_TERRESTRIAL_TPS_GUARDINTERVAL;

typedef enum
{
   MODE_2K = 1,
   MODE_8K,
   MODE_AUTO
} NIM_TERRESTRIAL_TPS_MODE;

typedef enum
{
   NIM_ANAOLOGUE_TERRESTIAL_PAL,
   NIM_ANAOLOGUE_TERRESTIAL_NTSC
} NIM_ANAOLOGUE_TERRESTIAL_STANDARD;

typedef enum
{
   NIM_ANALOGUE_TERRESTIAL_16_9,
   NIM_ANALOGUE_TERRESTIAL_4_3,
		NIM_ANALOGUE_TERRESTIAL_OFF
} NIM_ANAOLGUE_TERRESTIAL_FORMAT;

typedef struct
{
	NIM_ANAOLGUE_TERRESTIAL_FORMAT format;
	NIM_ANAOLOGUE_TERRESTIAL_STANDARD standard;
} NIM_ANAOLOGUE_TERRESTIAL_SCREEN_SETTINGS; 
typedef struct
{
   u_int32  Frequency;
   NIM_TERRESTRIAL_INPUT_BANDWIDTH     InputBW;
   NIM_TERRESTRIAL_TPS_FRAME           Frame;
   NIM_TERRESTRIAL_TPS_CONSTELLATION   Constellation;
   NIM_TERRESTRIAL_TPS_HIERARCHY       Hierarchy;
   NIM_TERRESTRIAL_TPS_CODERATE        HpCodeRate;
   NIM_TERRESTRIAL_TPS_CODERATE        LpCodeRate;
   NIM_TERRESTRIAL_TPS_GUARDINTERVAL   GuardInterval;
   NIM_TERRESTRIAL_TPS_MODE            Mode;
} NIM_TERRESTRIAL_TUNE;

typedef struct
{
   int   no_tuning_required;
} NIM_OPENTV_BASEBAND_TUNE;

typedef struct
{
   int   no_tuning_required;
} NIM_DVB_BASEBAND_TUNE;

typedef struct
{
   int   no_tuning_required;
} NIM_HSDP_TUNE;

typedef struct
{
   int   who_knows_what_goes_here;
} NIM_IEEE1394_TUNE;

typedef union
{
   NIM_SATELLITE_TUNE         nim_satellite_tune;
   NIM_OPENTV_BASEBAND_TUNE   nim_opentv_baseband_tune;
   NIM_DVB_BASEBAND_TUNE      nim_dvb_baseband_tune;
   NIM_CABLE_TUNE             nim_cable_tune;
   NIM_TERRESTRIAL_TUNE       nim_terrestrial_tune;
   NIM_HSDP_TUNE              nim_hsdp_tune;
   NIM_IEEE1394_TUNE          nim_ieee1394_tune;
} DEMOD_NIM_TUNE;

typedef struct
{
   DEMOD_NIM_TYPE type;
   DEMOD_NIM_TUNE tune;
} TUNING_SPEC;

typedef enum
{
   DEMOD_CONNECT_STATUS,
   DEMOD_DISCONNECT_STATUS,
   DEMOD_SCAN_STATUS
} DEMOD_CALLBACK_TYPE;

typedef enum
{
   DEMOD_CONNECTED,
   DEMOD_DISCONNECTED,
   DEMOD_TIMEOUT,
   DEMOD_DRIVER_LOST_SIGNAL,
   DEMOD_DRIVER_REACQUIRED_SIGNAL,
   DEMOD_SCAN_NO_SIGNAL,
   DEMOD_SCAN_COMPLETE,
   DEMOD_FAILED,
   DEMOD_ABORTED
} DEMOD_ASYNC_RESULT;

typedef struct
{
   DEMOD_ASYNC_RESULT   type;
   TUNING_SPEC          tune;
} DEMOD_CALLBACK_PARM;

typedef struct
{
   DEMOD_CALLBACK_TYPE  type;
   DEMOD_CALLBACK_PARM  parm;
} DEMOD_CALLBACK_DATA;

typedef struct
{
   DEMOD_NIM_TYPE nim_type;
} DEMOD_UNIT_TYPE;

typedef enum
{
   SET_LNB                       = 1
#if defined(DRIVER_INCL_DEMOD_COBRA)
		,
		SEND_DISEQC_MESSAGE,
		RECEIVE_DISEQC_MESSAGE,
		SET_AUTO_FECS
#endif
#if defined(DRIVER_INCL_DEMOD_ANALOGUE_TERRESTRIAL)
	 ,
		ENCODE_COMPOSITE,
		ENCODE_S_VIDEO,
		ENCODE_COMPONENT,
		QUEERY_SCREEN_SETTINGS,
		SET_VOLUME
#endif
   ,
   SET_LNB_OUTPUT_ENABLE,
   GET_LNB_OUTPUT_ENABLE,
   SET_LNB_POLARIZATION,
   GET_LNB_POLARIZATION,
   SET_LNB_TONE,
   GET_LNB_TONE
}
DEMOD_IOCTL_TYPE;

typedef enum
{
   LNB_SINGLE_FREQUENCY    = 0,
   LNB_DUAL_FREQUENCY,
   LNB_ORBITAL_POSITION,
   LNB_MANUAL,
   LNB_FREQUENCY_STACK
}
LNB_TYPE;

typedef enum
{
   V_12VOLTS                     = 1,
   V_18VOLTS
}
POLARIZATION_VOLTAGE;

typedef struct
{
   LNB_TYPE             type;
   u_int32              lnb_a;
   u_int32              lnb_b;
   u_int32              lnb_c;
   u_int32              lnb_switch;
   int16                orbital_position_a;
   int16                orbital_position_b;
   int16                orbital_position_c;
   bool                 orbital_22khz_a;
   bool                 orbital_22khz_b;
   bool                 orbital_22khz_c;
   POLARIZATION_VOLTAGE horizontal_voltage;
   POLARIZATION_VOLTAGE left_voltage;
   POLARIZATION_VOLTAGE right_voltage;
   POLARIZATION_VOLTAGE vertical_voltage;
} LNB_SETTINGS;

#if defined(DRIVER_INCL_DEMOD_COBRA)

typedef enum
{     
   BURST_TYPE_MODULATED=1,                  /* tone (at end-of-message) is modulated */
   BURST_TYPE_UNMODULATED                   /* tone ... is not modulated */
}
BURST_TYPE;

typedef struct
{
   u_int8          *pMessage;                /* Pointer to message to send */
   u_int8          ui8MessageLength;         /* length in BYTEs of message to send */
   bool            bLastMessage;             /* indicates if current message is the last message */
   BURST_TYPE      btBurstType;              /* burst-type at last-message: (modulated/un-mod'd) */
   /* for receiving the diseqc message */
   int             rxmode;                   /* Receive message mode: (refer to RXMODE) */
   int             bufferlen;
   int             receivedlen;
   int             parityerr;
} DISEQC_MESSAGE;

typedef struct
{
	/* No need for 1/2 because this always checked in auto FEC mode */
  bool  coderate_2div3;                 /*   " 2/3 */
  bool  coderate_3div4;                 /*   " 3/4 */
  bool  coderate_4div5;                 /*   " 4/5 */
  bool  coderate_5div6;                 /*   " 5/6 */
  bool  coderate_6div7;                 /*   " 6/7 */
  bool  coderate_7div8;                 /*   " 7/8 */
} AUTO_FECS;

#endif /* #if defined(DRIVER_INCL_DEMOD_COBRA) */

typedef struct
{
   u_int8   AgcLevel;
   int8     AdcOffset;
   u_int32  AdcSaturationRate;
   int32    FrequencyOffset;
   u_int32  PreViterbiBer;
   u_int32  PostViterbiBer;
   u_int8   MpegErrorCount;
   u_int8   ChannelChangeCount;
} TERRESTRIAL_STATS;

typedef struct
{
   unsigned char  signal_quality;
   unsigned char  signal_strength;
   int32          signal_ber_int;
   int32          signal_ber_div;
} SATELLITE_STATS;

typedef struct

{
    int  signal_strength;
    int  signal_quality;
//    int  rfagc;
//    int  ifagc;
    int  rs_uncorrected;
    int  rs_corrected;
    int  rs_total;
} CABLE_STATS;

typedef union
{
   TERRESTRIAL_STATS t_signal;
   SATELLITE_STATS   s_signal;
   CABLE_STATS       c_signal;
} DEMOD_NIM_STATS;

typedef struct
{
   DEMOD_NIM_TYPE    type;
   DEMOD_NIM_STATS   stats;
} SIGNAL_STATS;

#define DEMOD_FEC_12                   0x00000001
#define DEMOD_FEC_23                   0x00000002
#define DEMOD_FEC_34                   0x00000004
#define DEMOD_FEC_45                   0x00000008
#define DEMOD_FEC_56                   0x00000010
#define DEMOD_FEC_67                   0x00000020
#define DEMOD_FEC_78                   0x00000040

#define DEMOD_SPECTRAL_INVERSION_ON    0x00000001
#define DEMOD_SPECTRAL_INVERSION_OFF   0x00000002

typedef struct
{
   u_int32                    start_frequency;
   u_int32                    end_frequency;
   u_int32                    hop_value;
   
   u_int32                    num_fecs;
   NIM_FEC_RATE               *fecs;
   
   u_int32                    num_symrates;
   u_int32                    *symbol_rates;
   
   u_int32                    num_pols;
   NIM_SATELLITE_POLARISATION *polarizations;
   
} SATELLITE_SCAN_SPEC;

typedef struct
{
   NIM_CABLE_ANNEX           annex;

   u_int32                   start_frequency;
   u_int32                   end_frequency;
   u_int32                   hop_value;
   
   u_int32                   num_symrates;
   u_int32                   *symrates;
   
   u_int32                   num_qams;
   NIM_CABLE_MODULATION      *qams;
   u_int8                    ucSDPMin; /* RFAGC Min Threshold */
   u_int8                    ucSDPMax; /* RFAGC Max Threshold */
} CABLE_SCAN_SPEC;

typedef struct
{
    u_int32                  end_frequency;
} TERR_SCAN_SPEC;
   
typedef union
{
   SATELLITE_SCAN_SPEC  satellite;
   CABLE_SCAN_SPEC      cable;
   TERR_SCAN_SPEC       terr;
} DEMOD_NIM_SCAN;

typedef struct
{
   DEMOD_NIM_TYPE type;
   TUNING_SPEC    current;
   DEMOD_NIM_SCAN scan;
} SCAN_SPEC;

/****************************************************************************
 * Modifications:
 *
 * $Log: 
 *  29   mpeg      1.28        6/30/04 4:26:20 AM     Steven Shen     CR(s) 
 *        9624 9625 : Add RECEIVE_DISEQC_MESSAGE and modify the DISEQC_MESSAGE 
 *        data structure to support the DiSEqC 2.x.
 *  28   mpeg      1.27        6/7/04 9:08:36 AM      Dave Wilson     CR(s) 
 *        9360 : Added definition of new LNB type to support frequency-stacking
 *         systems such as Echostar's DishPro.
 *  27   mpeg      1.26        5/20/04 2:04:13 AM     Steven Shen     CR(s) 
 *        9254 9255 : Add the enum values for the new QAM modes supported by 
 *        Thomson cable tuner DCF8722.
 *  26   mpeg      1.25        3/22/04 1:41:38 PM     Billy Jackman   CR(s) 
 *        8585 : Added spectrum field to satellite tuning spec.
 *        Changed BER for satellite to be an integer/divisor pair.
 *        Added ioctl functions for manual control of LNB signalling.
 *        Added LNB_MANUAL LNB type for use with manual LNB control.
 *  25   mpeg      1.24        12/23/03 10:41:50 AM   Ian Mitchell    CR(s) 
 *        7739 : Add new IOCTL types for use with the analogue terrestrial 
 *        demod and fixed a spelling mistake.
 *        
 *  24   mpeg      1.23        12/2/03 5:23:25 AM     Ian Mitchell    CR(s): 
 *        7739 Add values for the new analogue terrestrial demod.
 *        
 *  23   mpeg      1.22        11/7/03 5:01:43 AM     Ian Mitchell    CR(s): 
 *        7865 7866 7867 Add extra value in NIM_FEC_RATE enum of M_RATE_AUTO, 
 *        this is only used when connecting to a sattelite transponder on 
 *        Brazos. If it is used the driver uses the hardwares ability to find 
 *        the FEC.
 *        Added new IOCTL functionality for Brasos so the FEC's automatically 
 *        searched by the hardware can be changed.
 *        Added the associated data type AUTO_FECS for this.
 *        
 *  22   mpeg      1.21        8/12/03 7:47:54 PM     Sunil Cheruvu   SCR(s): 
 *        7211 7212 
 *        Added the support for OEM to pass the SDP Min and Max values into the
 *         CABLE TUNE and CABLE SCAN structs.  And write the SDP values into 
 *        registers.
 *        
 *  21   mpeg      1.20        8/5/03 6:28:44 AM      Ian Mitchell    SCR(s): 
 *        7153 7155 
 *        Fix spelling for diseqc.
 *        
 *  20   mpeg      1.19        7/11/03 12:45:16 PM    Ian Mitchell    SCR(s): 
 *        6896 
 *        Added signal_ber to the satellite statistics structure.
 *        Added new IOCTL enum entry and associated enums and structures.
 *        
 *  19   mpeg      1.18        6/30/03 6:12:24 PM     Billy Jackman   SCR(s) 
 *        5816 :
 *        Added orbital_position to satellite tuning specification.
 *        Added LNB_TYPE enum to specify what kind of LNB is used.
 *        Added type field and orbital position specifiers to LNB 
 *        specification.
 *        
 *  18   mpeg      1.17        2/11/03 2:37:58 PM     Billy Jackman   SCR(s) 
 *        5143 :
 *        Changes to avoid conflicts with OpenTV header demod.h:
 *        Change DEMOD_BUSY to DEMOD_IS_BUSY.  Change SCAN to 
 *        SCAN_FOR_TRANSPONDER.
 *        Remove unused definition LNB_DATA.
 *        
 *  17   mpeg      1.16        11/27/02 2:04:44 PM    Billy Jackman   SCR(s) 
 *        4977 :
 *        Added explicit return code DEMOD_NO_HARDWARE to indicate that the
 *        hardware for a particular demod is not installed.  The demod driver 
 *        can use
 *        this return code to allow it to autoconfigure based on what NIM is in
 *         the box.
 *        
 *  16   mpeg      1.15        8/30/02 11:55:00 AM    Miles Bintz     SCR(s) 
 *        4497 :
 *        Added structures specific to cable for reporting signal strength, and
 *         scanning.  Updates to terrestrial scans to match satellite and cable
 *         style.
 *        
 *  15   mpeg      1.14        8/20/02 11:18:04 AM    Miles Bintz     SCR(s) 
 *        4434 :
 *        Made all enumerations start with 1 instead of 0, added ANNEX type 
 *        enumeration for cable
 *        
 *        
 *  14   mpeg      1.13        8/8/02 4:24:02 PM      Billy Jackman   SCR(s) 
 *        4337 :
 *        Added some return status values to be used for parameter checking.
 *        
 *  13   mpeg      1.12        7/8/02 1:44:48 PM      Billy Jackman   SCR(s) 
 *        4148 :
 *        Change usage of 'cnxt_bool' in demod interface to 'bool'.
 *        
 *  12   mpeg      1.11        6/26/02 5:36:48 AM     Steven Jones    SCR(s): 
 *        4084 
 *        Cosmetic changes only.
 *        
 *  11   mpeg      1.10        6/18/02 12:00:02 PM    Steven Jones    SCR(s): 
 *        3960 
 *        Update Demod API.
 *        
 *  10   mpeg      1.9         6/13/02 1:14:42 PM     Miles Bintz     SCR(s) 
 *        4001 :
 *        Added reacquired callback time and changed "cable modem" to just 
 *        "cable"
 *        
 *        
 *  9    mpeg      1.8         5/15/02 4:22:52 PM     Billy Jackman   SCR(s) 
 *        3795 :
 *        Regularized the usage of unions so that the enclosing structure 
 *        includes an
 *        DEMOD_NIM_TYPE tag to discriminate what element of the union is 
 *        active.
 *        
 *  8    mpeg      1.7         4/12/02 2:26:00 PM     Ray Mack        SCR(s) 
 *        3545 :
 *        broke this file into 2 files so we don't have collisions between old 
 *        demod stuff and multi-instance demod stuff.
 *        
 *  7    mpeg      1.6         4/5/02 9:46:48 AM      Ray Mack        SCR(s) 
 *        3507 :
 *        removed collision between San Diego RATE names and ours
 *        
 *  6    mpeg      1.5         4/5/02 9:20:04 AM      Ray Mack        SCR(s) 
 *        3507 :
 *        Changes to harmonize differences and compatibilities with San Diego 
 *        code and to add new stuff peculiar to satellite.
 *        
 *  5    mpeg      1.4         4/5/02 2:45:50 AM      Ian Mitchell    SCR(s): 
 *        3466 
 *        Add DEMOD_ERROR to the DEMOD_STATUS enum. Change the name of the 
 *        TERRESTRIAL_STATS element to terrestrial_signal_stats.
 *        
 *  4    mpeg      1.3         4/4/02 9:44:44 AM      Ian Mitchell    SCR(s): 
 *        3466 
 *        Spelling mistake in the NIM_TERRESTRIAL_TPS_CONSTELLATION enum
 *        
 *  3    mpeg      1.2         4/4/02 4:49:58 AM      Ian Mitchell    SCR(s): 
 *        3466 
 *        Add first cut of terrestrial tuning, scanning and stats structures, 
 *        also their asocciated enums.
 *        
 *  2    mpeg      1.1         4/3/02 9:48:56 AM      Billy Jackman   SCR(s) 
 *        3445 :
 *        Changed the enum DEMOD_CALLBACK_TYPE to include types for connect, 
 *        disconnect,
 *        and scan.
 *        
 *  1    mpeg      1.0         3/25/02 4:36:14 PM     Billy Jackman   
 * $
 *
 ****************************************************************************/

#endif /* __DEMOD_TYPES */

