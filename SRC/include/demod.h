/****************************************************************************/
/*                 Rockwell Semiconductor Systems - SABINE                  */
/****************************************************************************/
/*                                                                          */
/* Filename:           DEMOD.H                                              */
/*                                                                          */
/* Description:        Sabine IRD OpenTV Demodulator Driver Include File    */
/*                                                                          */
/* Author:             Steve Glennon                                        */
/*                                                                          */
/* Copyright Rockwell Semiconductor Systems, 1998                           */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/
/*
$Header: demod.h, 23, 2/13/03 1:24:14 PM, Billy Jackman$
*/
#ifndef ___DEMOD_INCLUDE___

#define ___DEMOD_INCLUDE___

/* Initial tuning variable required for some versions of middleware */
extern int initial_tune;

/*Viterbi encoding */

/* Internal representation */
enum CodeRates { 
                 RATE_1_2=0x10,
                 RATE_2_3=0x20, 
                 RATE_3_4=0x40, 
                 RATE_5_6=0x100, 
                 RATE_7_8=0x200, 
                 RATE_4_5=0x400, 
                 RATE_3_5=0x800,
                 RATE_6_7=0x1000, 
                 RATE_5_11=0x2000, 
                 RATE_K_N=0x4000,
                 RATE_UNKNOWN=0x8000 
};

/* FEC as defined in DVB delivery descriptor */
#define DVB_FEC_12                  1
#define DVB_FEC_23                  2
#define DVB_FEC_34                  3
#define DVB_FEC_56                  4
#define DVB_FEC_78                  5
#define DVB_FEC_11                  6
#define DVB_FEC_67                  7

#define DVB_FEC_MASK  0x0F

/* LNB Polarity selection configuration.  Controls 12/18 volt selection
   These are duplicates.  An LNB does either left/right or horizontal/vertical
   but never both sets at the same time. */

#define RIGHT_IS_LOW   1
#define RIGHT_IS_HIGH  2
#define VERT_IS_LOW    1
#define VERT_IS_HIGH   2

/* Polarization definitions */

/* Standard DVB position and shift. These are NOT used with the   */
/* definitions below which are bit-masks used by the demod driver */
#define DVB_POL_MASK       (3 << 9)
#define DVB_POL_SHIFT       9
#define DVB_POL_HORIZONTAL (0 << 9)
#define DVB_POL_VERTICAL   (1 << 9)
#define DVB_POL_LEFT       (2 << 9)
#define DVB_POL_RIGHT      (3 << 9)

/* Internal definitions which can be ORed together */
#define HORIZONTAL      1
#define VERTICAL        2
#define LEFT            4
#define RIGHT           8

/* Notify codes for TUNE operation */
#define DEMOD_ADDRESS_CONNECTED    1
#define DEMOD_ADDRESS_DISCONNECTED 2
#define DEMOD_SIGNAL_LOST          3
#define TUNE_NORMAL_FAILED         4

/* Notify codes for SCAN operation */
#define SCAN_COMPLETE              1
#define SCAN_FAILED                2
#define SCAN_UNLOCKED              3
#define SCAN_NO_SIGNAL             4

/* Modes of operation */
#define TUNE                       1
#define SCAN                       2

/***********************/
/* Function Prototypes */
/***********************/
#ifndef __ocod__

/* See API document for detailed descriptions of the external interface for the
   drivers demod4900, demod_hw, and demod_24106 */

void          gen_demod_init(void);
unsigned char demod_signal(unsigned char *quality);
unsigned char demod_lock_status(void);
int ScanRequest(int start_frequency,            /* frequency in kHz */
                int hop_freq,                   /* frequency in kHz */
                unsigned long symbol_rates[],   /* Zero terminated array of symbol 
                                                   rates in kHz */
                unsigned int viterbi_list,      /* bit mapped list of Viterbi rates */
                unsigned int polarization_list, /* bit mappped list of polarizations */
                int afc_on);                    /* do we use afc?  not actually used */
void SetSatelliteTuning( int connect,           /* TRUE to connect, 
                                                   FALSE for disconnect */
                         unsigned int frequency,/* frequency of transponder in kHz */  
                         unsigned int symbol_rate,/* symbol rate in kS/s */
                         unsigned short polarisation,/* see list above */
                         enum CodeRates Viterbi_encoding);
void SetLNB(int LowFrequency,  /* frequency in kHz */
            int HiFrequency,   /* frequency in kHz */
            int SwitchFrequency,/*frequency in kHz */
            int PolaritySense); /* TRUE for High is Right/Horizontal */

void    ProgramConnectNotify(int status,      /* Status return code. see above */
                             int destination, /* SCAN or TUNE logic */
                             int frequency,   /* actual frequency of lock in kHz */
                             int symbol_rate, /* symbol rate locked in kHz */
                             enum CodeRates viterbi_encoding);

enum CodeRates MapCodeRateFromDVB(unsigned long uDVBValue);

unsigned long MapPolarizationFromDVB(unsigned long uDVBValue);
unsigned char LNBPowerIsEnabled(void);
void          EnableLNBPower(unsigned char bEnable);

#endif /* __ocod__ */
#endif

/*
$Log: 
 23   mpeg      1.22        2/13/03 1:24:14 PM     Billy Jackman   SCR(s) 5077 
       :
       Add FEC rate 6/7 as legal.  This is used in DSS tuning mode.
       
 22   mpeg      1.21        2/11/03 2:18:34 PM     Billy Jackman   SCR(s) 5143 
       :
       Remove defines for MOD_QAM16, MOD_QAM64, and MOD_QAM256.  They were 
       unused
       and conflicted with an enum in demod_types.h.
       
 21   mpeg      1.20        7/30/02 4:24:36 PM     Miles Bintz     SCR(s) 4302 
       :
       Change define of QAM16 from 0 to 1, QAM64 from 1 to 64, and QAM256 from 
       2 to 256
       
 20   mpeg      1.19        6/13/02 12:53:52 PM    Miles Bintz     SCR(s) 4001 
       :
       added QAM modulation type defines
       
       
 19   mpeg      1.18        4/12/02 2:24:46 PM     Ray Mack        SCR(s) 3545 
       :
       broke this file into 2 files so we don't have collisions between old 
       demod stuff and multi-instance demod stuff.
       
 18   mpeg      1.17        2/25/02 11:00:36 AM    Ray Mack        SCR(s) 3244 
       :
       added comments.
       
 17   mpeg      1.16        5/30/01 2:15:18 PM     Ray Mack        SCR(s) 1917 
       1980 :
       Added new message to be sent back to the middleware.  Added 
       TUNE_NORMAL_FAILED message.
       
 16   mpeg      1.15        1/18/01 9:40:52 AM     Ray Mack        DCS 930 
       Candidate code for fixing this bug.  Added new Notify code for 
       generic driver to notify when lock is lost
       
 15   mpeg      1.14        10/12/00 5:18:10 PM    Dave Wilson     Added 
       support for LNB power control
       
 14   mpeg      1.13        7/24/00 5:22:30 PM     Dave Wilson     DCS464: 
       Added demod_lock_status API.
       
 13   mpeg      1.12        5/12/00 5:01:16 PM     Dave Wilson     Added a new 
       parameter to SetSatelliteTuning to explicitly indicate
       connection or disconnection. OpenTV sometimes sends f == 0 when asking
       to connect when baseband connections are in use.
       
 12   mpeg      1.11        3/1/00 6:39:40 PM      Dave Wilson     Moved some 
       FEC and polarisation label definitions from CONFMGR.H to DEMOD.H
       and renamed some to minimise confusion
       
 11   mpeg      1.10        2/29/00 7:36:36 AM     Ray Mack        changes for 
       opentv 1.2
       
 10   mpeg      1.9         2/28/00 2:55:28 PM     Ray Mack        changes to 
       reflect new interface to gendemod driver
       
 9    mpeg      1.8         8/18/99 10:33:06 AM    Ray Mack        Changes to 
       handle new NIM
       
 8    mpeg      1.7         8/18/99 10:06:28 AM    Dave Wilson     Moved I2C 
       GPIO extender defines to GPIOEXT.H
       
 7    mpeg      1.6         3/11/99 12:58:36 PM    Steve Glennon   Added 
       #include of hwconfig.h - was not always included before use of this
       header file, so some #if statements were throwing warnings which were 
       real.
       
 6    mpeg      1.5         3/4/99 3:35:50 PM      Dave Wilson     Added 
       polarity switch for NIMFAIL when using emulation.
       
 5    mpeg      1.4         11/20/98 12:21:52 PM   Steve Glennon   Changed 
       POL_R_HIV and POL_L_HIV to correctly indicate that Right polarisation is
        with the high voltage off
       
 4    mpeg      1.3         11/13/98 5:13:26 PM    Steve Glennon   Added 
       #define of NIMFAIL_POLARITY
       
 3    mpeg      1.2         10/6/98 6:35:06 PM     Steve Glennon   added 
       prototypes
       
 2    mpeg      1.1         9/21/98 2:01:58 PM     Steve Glennon   Added all 
       the remaining code to implement the demod driver for the HiMedia NIM
       
 1    mpeg      1.0         9/11/98 6:24:32 PM     Steve Glennon   
$
 * 
 *    Rev 1.22   13 Feb 2003 13:24:14   jackmaw
 * SCR(s) 5077 :
 * Add FEC rate 6/7 as legal.  This is used in DSS tuning mode.
 * 
 *    Rev 1.21   11 Feb 2003 14:18:34   jackmaw
 * SCR(s) 5143 :
 * Remove defines for MOD_QAM16, MOD_QAM64, and MOD_QAM256.  They were unused
 * and conflicted with an enum in demod_types.h.
 * 
 *    Rev 1.20   30 Jul 2002 15:24:36   bintzmf
 * SCR(s) 4302 :
 * Change define of QAM16 from 0 to 1, QAM64 from 1 to 64, and QAM256 from 2 to 256
 * 
 *    Rev 1.19   13 Jun 2002 11:53:52   bintzmf
 * SCR(s) 4001 :
 * added QAM modulation type defines
 * 
*/

