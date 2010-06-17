/****************************************************************************/
/*                 Rockwell Semiconductor Systems - SABINE                  */
/****************************************************************************/
/*                                                                          */
/* Filename:           TUNER.H                                              */
/*                                                                          */
/* Description:        Public header file for the Sabine analog TV tuner    */
/*                     driver.                                              */
/*                                                                          */
/* Author:             Dave Wilson                                          */
/*                                                                          */
/* Copyright Rockwell Semiconductor Systems, 1998                           */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************
$Header: tuner.h, 6, 2/5/03 12:42:40 PM, Miles Bintz$
$Log: 
 6    mpeg      1.5         2/5/03 12:42:40 PM     Miles Bintz     SCR(s) 5227 
       :
       added new lines at end of file
       
       
 5    mpeg      1.4         3/17/99 5:01:28 PM     Dave Wilson     Updated for 
       channel scan function.
       
 4    mpeg      1.3         1/21/99 12:37:14 PM    Dave Wilson     Added 
       definitions for new audio switch matrix functions.
       
 3    mpeg      1.2         1/10/99 9:54:36 AM     Dave Wilson     Analog TV 
       setup and ploarization signal fields added.
       
 2    mpeg      1.1         12/22/98 4:45:04 PM    Dave Wilson     First 
       working version.
       
 1    mpeg      1.0         11/9/98 3:54:42 PM     Dave Wilson     
$
 * 
 *    Rev 1.5   05 Feb 2003 12:42:40   bintzmf
 * SCR(s) 5227 :
 * added new lines at end of file
 * 
 * 
 *    Rev 1.4   17 Mar 1999 17:01:28   dawilson
 * Updated for channel scan function.
 *
 *    Rev 1.3   21 Jan 1999 12:37:14   dawilson
 * Added definitions for new audio switch matrix functions.
 *
 *    Rev 1.2   10 Jan 1999 09:54:36   dawilson
 * Analog TV setup and ploarization signal fields added.
 *
 *    Rev 1.1   22 Dec 1998 16:45:04   dawilson
 * First working version.
 *
 *    Rev 1.0   09 Nov 1998 15:54:42   dawilson
 * Initial revision.
 *
 */

#ifndef _TUNER_H_
#define _TUNER_H_

/*******************************************/
/* Label definitions used by tuning tables */
/*******************************************/
#ifndef __ocod__
typedef u_int16 TUNER_MEDIUM;
#endif

#define TUNER_CABLE     0
#define TUNER_BROADCAST 1
#define MEDIUM_COUNT    2

/***********************************************************/
/* Country codes used to determine which tuning map to use */
/***********************************************************/
#ifndef __ocod__
typedef u_int16 TUNER_COUNTRY;
#endif

#define COUNTRY_AFGHANISTAN      0
#define COUNTRY_ALBANIA          1
#define COUNTRY_ARMENIA          2
#define COUNTRY_AUSTRALIA        3
#define COUNTRY_AZERBAIJAN       4
#define COUNTRY_BAHRAIN          5
#define COUNTRY_BANGLADESH       6
#define COUNTRY_BELARUS          7
#define COUNTRY_BELGIUM          8
#define COUNTRY_BRUNEI           9
#define COUNTRY_BULGARIA        10
#define COUNTRY_CHINA           11
#define COUNTRY_CZECHREPUBLIC   12
#define COUNTRY_DENMARK         13
#define COUNTRY_FRANCE          14
#define COUNTRY_GEORGIA         15
#define COUNTRY_GERMANY         16
#define COUNTRY_HONGKONG        17
#define COUNTRY_HUNGARY         18
#define COUNTRY_INDIA           19
#define COUNTRY_ISRAEL          20
#define COUNTRY_ITALY           21
#define COUNTRY_JAPAN           22
#define COUNTRY_JORDAN          23
#define COUNTRY_KAZAKHSTAN      24
#define COUNTRY_KUWAIT          25
#define COUNTRY_KYRGYZSTAN      26
#define COUNTRY_MALAYSIA        27
#define COUNTRY_MOLDOVA         28
#define COUNTRY_MONGOLIA        29
#define COUNTRY_NETHERLANDS     30
#define COUNTRY_NEWZEALAND      31
#define COUNTRY_NORTHKOREA      32
#define COUNTRY_NORWAY          33
#define COUNTRY_OMAN            34
#define COUNTRY_PAKISTAN        35
#define COUNTRY_POLAND          36
#define COUNTRY_QUATAR          37
#define COUNTRY_ROMANIA         38
#define COUNTRY_RUSSIA          39
#define COUNTRY_SINGAPORE       40
#define COUNTRY_SOUTHAFRICA     41
#define COUNTRY_SRILANKA        42
#define COUNTRY_SWEDEN          43
#define COUNTRY_TAJIKISTAN      44
#define COUNTRY_THAILAND        45
#define COUNTRY_TURKMENISTAN    46
#define COUNTRY_UEA             47
#define COUNTRY_UK              48
#define COUNTRY_UKRAINE         49
#define COUNTRY_USA             50
#define COUNTRY_UZBEKISTAN      51
#define COUNTRY_YEMEN           52

#define COUNTRY_COUNT           53

/************************************/
/* Labels for supported tuning maps */
/************************************/
#ifndef __ocod__
typedef u_int16 TUNER_STANDARD;
#endif

#define TUNE_NTSC_N_USA       0
#define TUNE_PAL_I_UK         1
#define TUNE_SECAM_L_FRANCE   2
#define TUNE_PAL_B_ITALY      3
#define TUNE_PAL_B_WEUR       4
#define TUNE_NTSC_J_JAPAN     5
#define TUNE_PAL_B_AUS        6
#define TUNE_PAL_B_NZ         7
#define TUNE_SECAM_D_EEUR     8

#define TUNE_NUM_MAPS         9

/***********************************/
/* Audio Switch Matrix Definitions */
/***********************************/
#define ADDR_TEA6420 0x98

#define TEA6420_EXTAUD_OUT 1
#define TEA6420_SCART_OUT  2
#define TEA6420_SABINE_OUT 4
#define TEA6420_ALL_OUT  (TEA6420_EXTAUD_OUT | TEA6420_SCART_OUT | TEA6420_SABINE_OUT)

#define TEA6420_MODEM_IN  2
#define TEA6420_TUNER_IN  3
#define TEA6420_EXTAUD_IN 4
#define TEA6420_SABINE_IN 5
#define TEA6420_MUTE_IN   6

#define TEA6420_GAIN_6DB  0x00
#define TEA6420_GAIN_4DB  0x08
#define TEA6420_GAIN_2DB  0x10
#define TEA6420_GAIN_0DB  0x18

#define TEA6420_OUT_SHIFT 5

/********************/
/* Tuning Direction */
/********************/

#define UPWARDS   TRUE
#define DOWNWARDS FALSE

/*********************/
/* Tuner Status Info */
/*********************/

#define TUNER_LOCKED 0x40
#define TUNER_AFC_STATUS 0x07

/***********************/
/* Function Prototypes */
/***********************/
#ifndef __ocod__
/* Initialise the tuner driver, read the tuning scheme and check hardware */
bool TunerInit(void);

/* Set tuning standard according to country */
bool TunerSetCountry(TUNER_COUNTRY Country);

/* Get the code for the country currently set */
TUNER_COUNTRY TunerGetCountry(void);

/* Check to ensure that the tuner hardware is compatible with the country */
/* currently selected.                                                    */
bool TunerTestCountry(TUNER_COUNTRY Country);

/* Get the tuning standard currently selected */
TUNER_STANDARD TunerGetStandard(void);

/* Set cable or antenna tuning schemes */
bool TunerSetMedium(TUNER_MEDIUM MediumType);

/* Set cable or antenna tuning schemes */
TUNER_MEDIUM TunerGetMedium(void);

/* Get maximum and minimum channel numbers for the given connection type */
bool TunerGetChannelLimits(TUNER_STANDARD Standard,
                           TUNER_MEDIUM   Medium,
                           int           *pMinChannel,
                           int           *pMaxChannel);

/* Tune to a particular channel */
bool TunerSetChannel(int iChannel);

/* Return the number of the channel currently tuned */
bool TunerGetChannel(int *pChannel);

/* Scan all possible channels and look for valid signals, building a map for */
/* use with TunerNextChannel. Tuning band is as selected by previous calls   */
/* to TunerSetCountry (or TunerSetStandard) and TunerSetMedium. Notification */
/* callbacks are made throughout the scanning process.                       */

typedef void (*PFNTUNERCALLBACK)(u_int32 uStatus, u_int32 uChannel);

#define TUNE_STATUS_FOUND    0x01
#define TUNE_STATUS_NOTFOUND 0x02
#define TUNE_STATUS_STARTING 0x03
#define TUNE_STATUS_FINISHED 0x04

bool TunerBuildChannelMap(PFNTUNERCALLBACK cbTuner);

/* Tune to the next valid channel upwards or downwards from the current one */
bool TunerNextChannel(bool bUp);

/* Add a specific channel number to the current channel map */
bool TunerAddChannel(int iChannel);

/* Remove a specific channel number from the current channel map */
bool TunerRemoveChannel(int iChannel);

/* Set the audio routing switch matrix appropriately */
bool SetAudioRouting(u_int32 uInput, u_int32 uOutputs, u_int32 uGain);

#endif /* __ocod__ */

#endif /* _TUNER_H_ */



