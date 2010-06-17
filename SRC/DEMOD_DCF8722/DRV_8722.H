/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                   Conexant Systems Inc. (c) 2003 - 2004                  */
/*                            Shanghai, CHINA                               */
/*                          All Rights Reserved                             */
/****************************************************************************/
/*
 * Filename:      DRV_8722.H
 *
 * Description:   Header File for SGS-THOMSON DCF8722 FrontEnd Driver.
 *                The header file is ONLY for SGS-THOMSON.
 *
 * Author:        Steven Shen
 *
 ****************************************************************************/
/* $Header: DRV_8722.H, 2, 5/26/04 2:47:07 AM, Steven Shen$
 * $Id: DRV_8722.H,v 1.1, 2004-05-26 07:47:07Z, Steven Shen$
 ****************************************************************************/

#ifndef __DRV_8722_H__
#define __DRV_8722_H__

#include "basetype.h"
#include "demod_types.h"

#ifdef __cplusplus
extern "C"
{
#endif
#if(MTG_m88dc2000==YES)
#define m88dc2000
#endif
#if(TUNER_LG==YES)
#define TDCH_G101F
#endif
/****************************************************************************/
/* DEFINES: */

/******************************************************/
/*                *********************               */
/*                * NIM Types - Cable *               */
/*                *********************               */
/******************************************************/
/* Possible values for key CABLE_TUNER_TYPE           */
#define CTT_THOMSON_TUNER     (1)
#define CTT_SVA_TUNER         (2)
#define CTT_XIMEI_TUNER       (3)
/* End of possible values for key CABLE_TUNER_TYPE    */
/******************************************************/
/* Define the cable front-end type used in the system */
#define CABLE_TUNER_TYPE      CTT_THOMSON_TUNER

/* Define the i2c bus used for accessing the cable front-end */
#ifndef I2C_BUS_CABLE_FE
#define I2C_BUS_CABLE_FE      (I2C_BUS_0)
#endif

/* Define the GPIO pin used for the interrupt of the cable front-end */
#ifndef CABLE_INT_DISABLED
#define INT_CABLE_NIM         (INT_GPIO44)
#define CABLE_INT_GPIO        (0x2C)   /* (44)                    */
#define CABLE_INT_GPIO_BANK   (0x01)   /* (CABLE_INT_GPIO >> 5)   */
#define CABLE_INT_GPIO_BIT    (0x0C)   /* (CABLE_INT_GPIO & 0x1F) */
#endif

/* Define the GPIO pin used for the reset signal of the cable front-end */
#ifndef CABLE_RESET_DISABLED
#define CABLE_RST_GPIO        (0x2D)   /* (45)                    */
#define CABLE_RST_GPIO_BANK   (0x01)   /* (CABLE_RST_GPIO >> 5)   */
#define CABLE_RST_GPIO_BIT    (0x0D)   /* (CABLE_RST_GPIO & 0x1F) */
#endif

/* Define the GPIO pin used for the lock indicator of the cable front-end */
#ifndef CABLE_LKDT_DISABLED
#define CABLE_LKDT_GPIO       (0x18)   /* (24)                       */
#define CABLE_LKDT_GPIO_BANK  (0x00)   /* (CABLE_LKDT_GPIO >> 5)     */
#define CABLE_LKDT_GPIO_BIT   (0x18)   /* (CABLE_LKDT_GPIO & 0x1F)   */
#define CABLE_LKDT_TURN_ON    DRIVE_GPIO_LOW_BANK     /* LOW is ON   */
#define CABLE_LKDT_TURN_OFF   DRIVE_GPIO_HIGH_BANK    /* HIGH is OFF */
#endif

#define DCF872x_TUNER_DELAY   (100)
#define DCF872x_ACQP1_DELAY   (30)
#define DCF872x_ACQP2_DELAY   (150)

/*--------------------------------------------------------------------------*/
/*           DEFINITIONS OF RETURN VALUES OF DRIVER FUNCTIONS
 */
typedef enum
{
   DRV_OK = 0,
   DRV_INVALID_CHIPID = 1,
   DRV_BAD_SIGNAL = 2,
   DRV_ERROR = -1
}
DRV_RETURN;


/*--------------------------------------------------------------------------*/
/*     DEFINITIONS OF THE STATUS OF DEMODULATOR IN CABLE FRONT-END CAN
 */
typedef enum
{
   DEM_NO_LOCKED = 0,      /* demodulator hasn't started to lock a signal  */
   DEM_LOCKED,             /* demodulator has been locked on the signal    */
   DEM_LOCK_LOSS           /* demodulator has lost the lock                */
}
DEM_SYNC;

typedef struct
{
   DEM_SYNC    CurrSYNC;   /* the current sync status    */
   DEM_SYNC    PrevSYNC;   /* the previous sync status   */
   u_int32     uNumLOSS;   /* the statistic of lock loss */
}
DEM_STATUS;


/*--------------------------------------------------------------------------*/
/*         DEFINITIONS OF THE CONFIGURATION OF CABLE NIM MODULE
 */

#define DCF872x_STEP_31250       (1)
#define DCF872x_STEP_50000       (2)
#define DCF872x_STEP_62500       (3)

#define DCF872x_CP_50            (0)
#define DCF872x_CP_220           (1)

typedef struct
{
   u_int32           UnitId;     /* the unit id */
   DEMOD_NIM_TYPE    UnitType;   /* the unit type, always DEMOD_NIM_CABLE */
   
   /* DCF8722's Tuner Hardware Configurations & Hardware Information */
   u_int32     TunerIF;          /* tuner output IF in KHz: 36000       */
   u_int32     TunerBW;          /* tuner bandwidth in  Hz: 8000000     */
   u_int8      TunerI2cRepeater; /* Is tuner controlled by i2c repeater */
   u_int8      TunerAddr;        /* I2C address for tuner i2c control   */
   u_int8      TunerB[6];        /* data format for tuner i2c control   */
   u_int32     TunerREF;         /* tuner reference frequency (step) */
   u_int32     TunerIFN;         /* tuner output IF in step */
   u_int8      TunerCP;          /* tuner charge pump current option */
   u_int8      TunerFlag;        /* tuner frequency change flag */
   u_int32     TunerFREQ;        /* the signal frequency to tune */
   
   /* STV0297 Hardware Configurations & Hardware Information */
   u_int32     DemodClkExt;      /* external clock input, 28800000 Hz   */
   u_int8      DemodAddr;        /* I2C address for accessing registers */
   u_int8      DemodChipId;      /* chip version information            */
   
   u_int8      DemodFlag;        /* the change flag of demodualator     */
   u_int8      DemodQAM;         /* QAM size                            */
   u_int32     DemodSR;          /* symbol rate                         */
   u_int8      DemodSI;          /* spectrum inversion option           */
   u_int8      DemodITUJ83;      /* ITU-J83A mode or ITU-J83C mode      */
   
   u_int8      DemodINITDEM;     /* initial demodulator mode            */
   u_int32     DemodF1;          /* A/D sampled IF = Tuner IF - CLKEXT  */
   u_int32     DemodINITDEMfreq; /* the initial demodulator frequency   */
   
   u_int8      DemodWBAGCLock;   /* the WBAGC lock status               */
   u_int16     Demodagc2sd;      /* 10-bit value agc2sd                 */
   #ifndef STV0297_CNS
   u_int8      SignalStrength;   /* signal strength 0-255               */
   #else
   u_int32    SignalStrength;
   #endif
   
   u_int8      DemodBERTsrc;     /* integrated BERT source bit/byte     */
   u_int8      DemodBERTsp;      /* integrated BERT mode auto/manual    */
   u_int8      DemodBERTmode;    /* integrated BERT mode byte           */
   u_int8      DemodBERTst;      /* integrated BERT status byte         */
   u_int8      DemodBERTwait;    /* integrated BERT is not ready if "1" */
   u_int16     DemodBERTerr;     /* integrated BERT error counter       */
   u_int32     DemodBERTnb;      /* integrated BERT the number of bytes */
   float       DemodBER;         /* Bit/Byte Error Rate                 */
   
   u_int16     DemodBlkCnt;      /* the block counter                   */
   u_int16     DemodCorrCnt;     /* the corrected block counter         */
   u_int16     DemodUncorrCnt;   /* the uncorrected block counter       */
   
   int32       DemodCNEmean;     /* the C/N estimator mean              */
   int32       DemodCNEoffset;
   int32       DemodCNE;         /* the C/N estimator                   */
   
   DEM_STATUS  LockStatus;       /* the STV0297 lock status             */
   
   /* the signal to tune */
   u_int32              SignalFREQ;    /* signal frequency (Hz)         */
   u_int32              SignalSR;      /* signal symbol rate (baud/s)   */
   NIM_CABLE_MODULATION SignalQAM;     /* signal QAM size               */
   u_int32              SignalAutoSP;  /* signal auto spectrum          */
   NIM_CABLE_SPECTRUM   SignalSI;      /* signal spectrum inversion     */
   NIM_CABLE_ANNEX      SignalAnnex;   /* 1 - ANNEX_A / 2 - ANNEX_B     */
   
   int32       iSweepRate;       /* sweep rate */
   int32       iCarrierOffset;   /* carrier offset */
}
CNIM_CFG;

extern CNIM_CFG      gCNimCfg;

/****************************************************************************/
/* FUNCTIONS: */

DRV_RETURN dcf872x_get_CNE (u_int32 uUnit);
DRV_RETURN dcf872x_set_bert (u_int32 uUnit);
DRV_RETURN dcf872x_get_bert (u_int32 uUnit);
DRV_RETURN dcf872x_get_strength (u_int32 uUnit);
DRV_RETURN dcf872x_init_statics (u_int32 uUnit);
DRV_RETURN dcf872x_get_statics (u_int32 uUnit);
DRV_RETURN dcf872x_print_statics (u_int32 uUnit);
DRV_RETURN dcf872x_get_lockstatus (u_int32 uUnit, bool *pLocked);
DRV_RETURN dcf872x_set_symbol_rate (u_int32 uUnit);
DRV_RETURN dcf872x_reinit (u_int32 uUnit);
DRV_RETURN dcf872x_connect (u_int32 uUnit, TUNING_SPEC *pTuning, bool bForced);
DRV_RETURN dcf872x_get_tuning (u_int32 uUnit, TUNING_SPEC *pTuning);
DRV_RETURN dcf872x_init (u_int32 uUnit);

/****************************************************************************/
/* THE END */

#ifdef __cplusplus
}
#endif

#endif   /* __DRV_8722_H__ */

/****************************************************************************
 * Modifications:
 * $Log: 
 *  2    mpeg      1.1         5/26/04 2:47:07 AM     Steven Shen     CR(s) 
 *        9022 9023 : The DEMOD_DCF8722 driver version 1.20. Add the support of
 *         getting the signal strength.
 *  1    mpeg      1.0         3/15/04 10:30:30 AM    Matt Korte      CR(s) 
 *        8566 : Initial version of Thomson Cable Tuner/Demod
 * $
 *
 ****************************************************************************/
