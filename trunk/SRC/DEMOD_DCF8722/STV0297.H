/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                   Conexant Systems Inc. (c) 2003 - 2004                  */
/*                            Shanghai, CHINA                               */
/*                          All Rights Reserved                             */
/****************************************************************************/
/*
 * Filename:      STV0297.H
 *
 * Description:   Header File for STV0297 QAM Demodulator IC Driver.
 *
 * Author:        Steven Shen
 *
 ****************************************************************************/
/* $Header: STV0297.H, 3, 5/26/04 2:59:34 AM, Steven Shen$
 * $Id: STV0297.H,v 1.2, 2004-05-26 07:59:34Z, Steven Shen$
 ****************************************************************************/

#ifndef __STV0297_H__
#define __STV0297_H__

/* Includes the head file for the data types */
#include "basetype.h"

#ifdef __cplusplus
extern "C"
{
#endif


/****************************************************************************/
/* DEFINES:
 *
 * the prtfix DEM_ is for all demodulator low-level driver
 * the prefix ST0_ & st0_ are for STV0297 QAM demodulator.
 * the prefix ST0_RID_ is for the index of STV0297 internal registers.
 */

/*--------------------------------------------------------------------------*/
/*           DEFINITIONS OF RETURN VALUES OF DEMODULATOR FUNCTIONS
 */
typedef enum
{
   DEM_OK = 0,
   DEM_ERROR
}
DEM_RETURN;

#if(MTG_m88dc2000==YES)
#define m88dc2000
#endif
#if(TUNER_LG==YES)
#define TDCH_G101F
#endif
/*--------------------------------------------------------------------------*/
/*             DEFINITIONS OF QAM DEMODULATOR INTERNAL REGISTERS
 */
/* the number of STV0297 internal registers */
#define DEM_REG_NUM              (90)
/* the string length of the register name */
#define DEM_REG_STRLEN           (16)

/***************************/
/*   REGISTER STRUCTURE    */
/***************************/
typedef struct
{
   u_int8   addr;                   /* the register address             */
   u_int8   value;                  /* the current value                */
   u_int8   wflag;                  /* the flag of write: 0 - read only */
   u_int8   start;                  /* the initialization value         */
   u_int8   store;                  /* the stored value for reloading   */
   u_int8   resv1;                  /* reserved (as a index)            */
   u_int8   resv2;                  /* reserved (as a reset value       */
   u_int8   resv3;                  /* reserved.                        */
   char     name[DEM_REG_STRLEN];   /* the register name                */
}
DEM_REGISTER;


/*--------------------------------------------------------------------------*/
/*      STV0297 I2C ADDRESS FOR READING AND WRITING INTERNAL REGISTERS
 *
 * The STV0297 is controlled via an I2C bus and is a pure slave. Its 7-bit
 * chip address is "0011100",  so its 8-bit write address is 0x38, and its
 * 8-bit read address is 0x39.
 */
#define ST0_I2CADDR              (0x38)

#define ST0_I2CBT_LEN            (12)  /* the max number of data for burst */
#define ST0_I2CBF_LEN            (20)  /* the buffer length for i2c burst */


/*--------------------------------------------------------------------------*/
/*    DEFINITIONS OF THE INDEX OF STV0297 INTERNAL REGISTERS IN THE ARRAY
 */

/* EQUALIZER - CONSTELLATION QUALITY ESTIMATOR */
#define ST0_RID_EQU_0            (0x00)
#define ST0_RID_EQU_1            (0x01)
#define ST0_RID_EQU_3            (0x02)
#define ST0_RID_EQU_4            (0x03)
#define ST0_RID_EQU_7            (0x04)
#define ST0_RID_EQU_8            (0x05)
/* QUADRATURE DEMODULATOR */
#define ST0_RID_INITDEM_0        (0x06)
#define ST0_RID_INITDEM_1        (0x07)
#define ST0_RID_INITDEM_2        (0x08)
#define ST0_RID_INITDEM_3        (0x09)
#define ST0_RID_INITDEM_4        (0x0A)
#define ST0_RID_INITDEM_5        (0x0B)
/* ANALOG AGCS - A/D OVERFLOW MONITOR */
#define ST0_RID_DELAGC_0         (0x0C)
#define ST0_RID_DELAGC_1         (0x0D)
#define ST0_RID_DELAGC_2         (0x0E)
#define ST0_RID_DELAGC_3         (0x0F)
#define ST0_RID_DELAGC_4         (0x10)
#define ST0_RID_DELAGC_5         (0x11)
#define ST0_RID_DELAGC_6         (0x12)
#define ST0_RID_DELAGC_7         (0x13)
#define ST0_RID_DELAGC_8         (0x14)
#define ST0_RID_WBAGC_0          (0x15)
#define ST0_RID_WBAGC_1          (0x16)
#define ST0_RID_WBAGC_2          (0x17)
#define ST0_RID_WBAGC_3          (0x18)
#define ST0_RID_WBAGC_4          (0x19)
#define ST0_RID_WBAGC_5          (0x1A)
#define ST0_RID_WBAGC_6          (0x1B)
#define ST0_RID_WBAGC_9          (0x1C)
#define ST0_RID_WBAGC_10         (0x1D)
#define ST0_RID_WBAGC_11         (0x1E)
/* SYMBOL TIMING RECOVERY LOOP */
#define ST0_RID_STLOOP_2         (0x1F)
#define ST0_RID_STLOOP_3         (0x20)
#define ST0_RID_STLOOP_5         (0x21)
#define ST0_RID_STLOOP_6         (0x22)
#define ST0_RID_STLOOP_7         (0x23)
#define ST0_RID_STLOOP_8         (0x24)
#define ST0_RID_STLOOP_9         (0x25)
#define ST0_RID_STLOOP_10        (0x26)
#define ST0_RID_STLOOP_11        (0x27)
/* CARRIER RECOVERY LOOP */
#define ST0_RID_CRL_0            (0x28)
#define ST0_RID_CRL_1            (0x29)
#define ST0_RID_CRL_2            (0x2A)
#define ST0_RID_CRL_3            (0x2B)
#define ST0_RID_CRL_4            (0x2C)
#define ST0_RID_CRL_5            (0x2D)
#define ST0_RID_CRL_6            (0x2E)
#define ST0_RID_CRL_7            (0x2F)
#define ST0_RID_CRL_8            (0x30)
#define ST0_RID_CRL_9            (0x31)
#define ST0_RID_CRL_10           (0x32)
#define ST0_RID_CRL_11           (0x33)
/* POST-FILTER DIGITAL AGC */
#define ST0_RID_PMFAGC_0         (0x34)
#define ST0_RID_PMFAGC_1         (0x35)
#define ST0_RID_PMFAGC_2         (0x36)
#define ST0_RID_PMFAGC_3         (0x37)
#define ST0_RID_PMFAGC_4         (0x38)
/* CONFIGURATION AND CONTROL */
#define ST0_RID_CTRL_0           (0x39)
#define ST0_RID_CTRL_1           (0x3A)
#define ST0_RID_CTRL_2           (0x3B)
#define ST0_RID_CTRL_3           (0x3C)
#define ST0_RID_CTRL_4           (0x3D)
#define ST0_RID_CTRL_5           (0x3E)
#define ST0_RID_CTRL_6           (0x3F)
#define ST0_RID_CTRL_7           (0x40)
#define ST0_RID_CTRL_8           (0x41)
#define ST0_RID_CTRL_9           (0x42)
/* DEINTERLEAVER SYNC DETECTOR */
#define ST0_RID_DEINT_SYNC_0     (0x43)
#define ST0_RID_DEINT_SYNC_1     (0x44)
/* INTEGRATED BER TESTER */
#define ST0_RID_BERT_0           (0x45)
#define ST0_RID_BERT_1           (0x46)
#define ST0_RID_BERT_2           (0x47)
/* DEINTERLEAVER */
#define ST0_RID_DEINT_0          (0x48)
#define ST0_RID_DEINT_1          (0x49)
/* OUTPUT FORMATTER */
#define ST0_RID_OUTFORMAT_0      (0x4A)
#define ST0_RID_OUTFORMAT_1      (0x4B)
#define ST0_RID_OUTFORMAT_2      (0x4C)
/* REED-SOLOMON DESCRAMBLER SYNC DETECTOR */
#define ST0_RID_RS_DESC_0        (0x4D)
#define ST0_RID_RS_DESC_1        (0x4E)
#define ST0_RID_RS_DESC_2        (0x4F)
#define ST0_RID_RS_DESC_3        (0x50)
#define ST0_RID_RS_DESC_4        (0x51)
#define ST0_RID_RS_DESC_5        (0x52)
#define ST0_RID_RS_DESC_14       (0x53)
#define ST0_RID_RS_DESC_15       (0x54)
/* RESERVED */
#define ST0_RID_EQU_2            (0x55)
#define ST0_RID_EQU_5            (0x56)
#define ST0_RID_EQU_6            (0x57)
#define ST0_RID_DEINT_2          (0x58)
#define ST0_RID_DEINT_3          (0x59)


/*--------------------------------------------------------------------------*/
/*                     DEFINITIONS OF THE QAM SIZE
 */
#if (0/*defined m88dc2000*/)
#define ST0_QAM16                (0x01)
#define ST0_QAM32                (0x02)
#define ST0_QAM64                (0x05)
#define ST0_QAM128               (0x03)
#define ST0_QAM256               (0x04)
#define ST0_QAM_AUTO             (0x00)
#define ST0_QAM_UNSUPPORTED      (0xFF)
#else
#define ST0_QAM16                (0x00)
#define ST0_QAM32                (0x01)
#define ST0_QAM64                (0x04)
#define ST0_QAM128               (0x02)
#define ST0_QAM256               (0x03)
#define ST0_QAM_AUTO             (0xFE)
#define ST0_QAM_UNSUPPORTED      (0xFF)
#endif


/*--------------------------------------------------------------------------*/
/*                DEFINITIONS OF THE STV0297 CHIP VERSION
 */
#if (defined m88dc2000)
#define DC2_VERSION              (0x00)
#else
#define ST0_VERSION              (0x02)
#endif

#define ST0_ITU_J83A             (0)
#define ST0_ITU_J83C             (1)

#define ST0_SPECTRUM_NORMAL      (0)
#define ST0_SPECTRUM_INVERSION   (1)

#define ST0_BERT_BIT_SRC         (0)
#define ST0_BERT_BYTE_SRC        (1)

#define ST0_BERT_AUTO_STOP       (0)
#define ST0_BERT_MANUAL_STOP     (1)

#define ST0_BERT_NBYTE           (0x06)   /* 2 ^ (12 + 2 * ST0_BERT_NBYTE) */

#define ST0_INITDEM_DISABLE      (0)
#define ST0_INITDEM_ENABLE       (1)

#define ST0_WBAGC_UNLOCKED       (0)
#define ST0_WBAGC_LOCKED         (1)


/****************************************************************************/
/* FUNCTIONS: */

/* init/catch/store/reload all internal registers of STV0297 */
#if (defined m88dc2000)
DEM_RETURN dc2_init_regs (u_int8 i2cAddr);
#endif
DEM_RETURN st0_init_regs (u_int8 i2cAddr);
DEM_RETURN st0_catch_regs (u_int8 i2cAddr);
DEM_RETURN st0_store_regs (u_int8 i2cAddr);
DEM_RETURN st0_reload_regs (u_int8 i2cAddr);

/* software reset STV0297 */
#if (defined m88dc2000)
DEM_RETURN dc2_global_soft_reset (u_int8 i2cAddr);
DEM_RETURN dc2_soft_reset (u_int8 i2cAddr);
#endif
DEM_RETURN st0_swreset (u_int8 i2cAddr);
DEM_RETURN st0_rs_swreset (u_int8 i2cAddr);
DEM_RETURN st0_di_swreset (u_int8 i2cAddr);

/* get the chip version information */
#if (defined m88dc2000)
DEM_RETURN dc2_get_ver (u_int8 i2cAddr, u_int8 *pData);
#else
DEM_RETURN st0_get_ver (u_int8 i2cAddr, u_int8 *pData);
#endif

/* spectrum inversion */
#if (defined m88dc2000)
DEM_RETURN dc2_set_spec_inv (u_int8 i2cAddr, u_int8 ui8SI);
#else
DEM_RETURN st0_set_spec_inv (u_int8 i2cAddr, u_int8 ui8SI);
#endif
DEM_RETURN st0_get_spec_inv (u_int8 i2cAddr, u_int8 *pData);

/* ITU-J83 mode: ITU-J83A or ITU-J83C */
DEM_RETURN st0_set_itu_j83_mode (u_int8 i2cAddr, u_int8 ui8Mode);
DEM_RETURN st0_get_itu_j83_mode (u_int8 i2cAddr, u_int8 *pData);

/* enable the i2c repeater */
#if (defined m88dc2000)
DEM_RETURN dc2_start_i2c_repeater (u_int8 i2cAddr);
#else
DEM_RETURN st0_start_i2c_repeater (u_int8 i2cAddr);
#endif

/* operation functions of equalizer / constellation quality estimator */
DEM_RETURN st0_equ_swreset (u_int8 i2cAddr);
DEM_RETURN st0_equ_init (u_int8 i2cAddr);
#if (defined m88dc2000)
DEM_RETURN dc2_set_qam_size (u_int8 i2cAddr, u_int8 ui8QAM);
DEM_RETURN dc2_get_qam_size (u_int8 i2cAddr, u_int8 *pData);
DEM_RETURN dc2_get_noise_accumulator (u_int8 i2cAddr, u_int16 *pData);
#else
DEM_RETURN st0_set_qam_size (u_int8 i2cAddr, u_int8 ui8QAM);
DEM_RETURN st0_get_qam_size (u_int8 i2cAddr, u_int8 *pData);
DEM_RETURN st0_get_noise_accumulator (u_int8 i2cAddr, u_int16 *pData);
#endif
/* no ither functions are needed. */
/* use the default configuration values when initialization. */

/* operation functions of initial quadrature demodulator */
DEM_RETURN st0_disable_initdem (u_int8 i2cAddr);
/*
DEM_RETURN st0_enable_initdem (u_int8 i2cAddr, u_int8 ui8Mode);
DEM_RETURN st0_set_initdem_freq (u_int8 i2cAddr, u_int32 uFreq);
DEM_RETURN st0_set_initdem_step (u_int8 i2cAddr, u_int16 ui16Step);
DEM_RETURN st0_set_initdem_latency (u_int8 i2cAddr, u_int8 ui8Latency);
*/

/* operation functions of analog AGCs & A/D overflow monitor */
DEM_RETURN st0_wbagc_init (u_int8 i2cAddr);
#if (defined m88dc2000)
DEM_RETURN dc2_set_a_reg (u_int8 ui8Addr, u_int8 ui8RegAddr, u_int8 ui8RegValue);
DEM_RETURN dc2_get_a_reg (u_int8 ui8Addr, u_int8 ui8RegAddr, u_int8 *pRegValue);
DEM_RETURN dc2_get_agc_lock (u_int8 i2cAddr, u_int8 *pData);
#endif
DEM_RETURN st0_wbagc_get_acq (u_int8 i2cAddr, u_int8 *pData);
#if (defined m88dc2000)
DEM_RETURN dc2_get_agc2sd (u_int8 i2cAddr, u_int16 *pData);
#else
DEM_RETURN st0_wbagc_get_agc2sd (u_int8 i2cAddr, u_int16 *pData);
#endif

/* operation functions of symbol timing recovery loop */
DEM_RETURN st0_stl_init (u_int8 i2cAddr);
DEM_RETURN st0_stl_freeze (u_int8 i2cAddr);
DEM_RETURN st0_stl_unfreeze (u_int8 i2cAddr);
#if (defined m88dc2000)
//DEM_RETURN dc2_set_symbol_rate (u_int8 i2cAddr, u_int32 uSR, u_int32 uSR_inv, u_int8 LowSym, u_int8 DDSStep);
DEM_RETURN dc2_set_symbol_rate (u_int8 i2cAddr, u_int32 uSR, u_int32 uSR_inv, u_int8 LowSym, u_int8 reg6FH, u_int8 reg12H);
#else
DEM_RETURN st0_set_symbol_rate (u_int8 i2cAddr, u_int32 uSR);
#endif
DEM_RETURN st0_get_symbol_rate (u_int8 i2cAddr, u_int32 *pData);

/* operation functions of carrier recovery loop. */
DEM_RETURN st0_crl_init (u_int8 i2cAddr);
DEM_RETURN st0_set_sweep_rate (u_int8 i2cAddr);
DEM_RETURN st0_set_iphase (u_int8 i2cAddr, u_int32 uIPHASE);
/*
DEM_RETURN st0_set_aphase (u_int8 i2cAddr, u_int32 uAPHASE);
DEM_RETURN st0_get_accumulators (u_int8 i2cAddr, u_int32 *pAPHASE, u_int32 *pIPHASE);
*/

/* operation functions of post-filter digital AGC. */
DEM_RETURN st0_pmfagc_init (u_int8 i2cAddr);

DEM_RETURN st0_fec_init (u_int8 i2cAddr);

DEM_RETURN st0_acquisition_1 (u_int8 i2cAddr);
DEM_RETURN st0_acquisition_2 (u_int8 i2cAddr);
DEM_RETURN st0_acquisition_3 (u_int8 i2cAddr);

/* operation functions of deinterleaver sync detector. */
/* no functions are needed. */
/* use the default configuration values when initialization. */

/* operation functions of integrated BER tester. */
#if (defined m88dc2000)
DEM_RETURN dc2_start_bert (u_int8 i2cAddr, u_int8 ui8Mode);
#else
DEM_RETURN st0_start_bert (u_int8 i2cAddr, u_int8 ui8Mode);
#endif
DEM_RETURN st0_stop_bert (u_int8 i2cAddr);
#if (defined m88dc2000)
DEM_RETURN dc2_get_bert_status (u_int8 i2cAddr, u_int8 *pData);
DEM_RETURN dc2_get_bert_error (u_int8 i2cAddr, u_int16 *pData);
DEM_RETURN dc2_get_fec_error (u_int8 i2cAddr);
DEM_RETURN dc2_get_mse (u_int8 i2cAddr);
DEM_RETURN dc2_get_locks (u_int8 i2cAddr);
#else
DEM_RETURN st0_get_bert_status (u_int8 i2cAddr, u_int8 *pData);
DEM_RETURN st0_get_bert_error (u_int8 i2cAddr, u_int16 *pData);
#endif

/* operation functions of deinterleaver. */
/* no functions are needed. */
/* use the default configuration values when initialization. */

/* operation functions of output formatter. */
/* no functions are needed. */
/* use the default configuration values when initialization. */

/* operation functions of R-S FEC decoder, descrambler */
#if (defined m88dc2000)
DEM_RETURN dc2_get_lock_status (u_int8 i2cAddr, bool *pData);
#else
DEM_RETURN st0_get_lock_status (u_int8 i2cAddr, bool *pData);
#endif
#if (defined m88dc2000)
DEM_RETURN dc2_start_ct (u_int8 i2cAddr);
#else
DEM_RETURN st0_start_ct (u_int8 i2cAddr);
#endif
DEM_RETURN st0_get_blk_ct (u_int8 i2cAddr, u_int16 *pData);
DEM_RETURN st0_get_corr_ct (u_int8 i2cAddr, u_int16 *pData);
DEM_RETURN st0_get_uncorr_ct (u_int8 i2cAddr, u_int16 *pData);

/* initialize the C/N estimation look-up table */
DEM_RETURN st0_init_CN_estimation (u_int8 i2cAddr);


/****************************************************************************/
/* THE END */

#ifdef __cplusplus
}
#endif

#endif   /* __STV0297_H__ */

/****************************************************************************
 * Modifications:
 * $Log: 
 *  3    mpeg      1.2         5/26/04 2:59:34 AM     Steven Shen     CR(s) 
 *        9022 9023 : The DEMOD_DCF8722 driver version 1.20. Add some new 
 *        functions.
 *  2    mpeg      1.1         5/20/04 3:18:50 AM     Steven Shen     CR(s) 
 *        9254 9255 : Add the definition of the Auto-QAM detection mode.
 *  1    mpeg      1.0         3/15/04 10:30:32 AM    Matt Korte      CR(s) 
 *        8566 : Initial version of Thomson Cable Tuner/Demod
 * $
 *
 ****************************************************************************/
