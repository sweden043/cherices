/****************************************************************************/
/*                 Rockwell Semiconductor Systems - SABINE                  */
/****************************************************************************/
/*                                                                          */
/* Filename:           retcodes.h                                           */
/*                                                                          */
/* Description:        Public header file defining common return codes used */
/*                     by Sabine software components                        */
/*                                                                          */
/* Author:             Dave Wilson                                          */
/*                                                                          */
/* Copyright Rockwell Semiconductor Systems, 1997                           */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/

#ifndef _RETCODES_H_
#define _RETCODES_H_

/****************************************************************************/
/*                                                                          */
/* Note: To aid debugging, return code labels are all of the form:          */
/*                                                                          */
/*                          RC_<mod>_<description>                          */
/*                                                                          */
/* Where <mod> is a 3 character string indicating the source of the error   */
/* and <description> is an indication of the error condition.               */
/*                                                                          */
/****************************************************************************/

/* Module return code start values.  These labels are also used as           */
/* identifiers for the modules when reporting errors so please keep them in  */
/* the range 0x0000 - 0xFF0000 with the bottom 4 digits 0. Return codes for  */
/* a given module are defined as MOD_XXX + <return code>                     */


#define MOD_GEN  0x000000
#define MOD_AUD  0x010000
#define MOD_DPS  0x020000
#define MOD_DRM  0x030000
#define MOD_ETH  0x040000
#define MOD_FPU  0x050000
#define MOD_GCP  0x060000
#define MOD_I2C  0x070000
#define MOD_IRD  0x080000
#define MOD_MOV  0x090000
#define MOD_MPG  0x0A0000
#define MOD_PAR  0x0B0000
#define MOD_RST  0x0C0000
#define MOD_RTC  0x0D0000
#define MOD_SDM  0x0E0000
#define MOD_SER  0x0F0000
#define MOD_SMC  0x100000
#define MOD_TUN  0x110000
#define MOD_KAL  0x120000
#define MOD_KEY  0x130000
#define MOD_FSH  0x140000
#define MOD_OTV  0x150000
#define MOD_MODEM  0x160000
#define MOD_MDM  MOD_MODEM
#define MOD_NVS  0x170000
#define MOD_E2P  0x180000           /* Not Used */
#define MOD_GXA  0x190000
#define MOD_JPG  0x1A0000
#define MOD_PWR  0x1B0000
#define MOD_ATA  0x1C0000



/* Strings used when logging errors from each module */

#define STR_GEN  "General"
#define STR_AUD "Audio"
#define STR_DPS "DVB Parser"
#define STR_DRM "Onscreen Display"
#define STR_ETH "Ethernet"
#define STR_FPU  "Floating Point"
#define STR_GCP "Graphics Coprocessor"
#define STR_I2C "I2C"
#define STR_IRD "Infra Red"
#define STR_MOV "Movie-In"
#define STR_MPG "MPEG"
#define STR_PAR "Parallel"
#define STR_RST "Reset and power"
#define STR_RTC "Real Time Clock"
#define STR_SDM "Demodulator"
#define STR_SER "Serial"
#define STR_SMC "Smartcard"
#define STR_TUN "Analog Tuner"
#define STR_KAL "Kernel Adaptation Layer"
#define STR_KEY "Front Panel"
#define STR_FSH "Flash"
#define STR_OTV "OpenTV Control Driver"
#define STR_MODEM "Modem"
#define STR_MDM STR_MODEM
#define STR_NVS "NonVolatile Store Driver"
#define STR_E2P "E2P Driver"              /* Not Used */
#define STR_GXA "Graphics Accelerator"
#define STR_JPG "JPEG Decoder"
#define STR_PWR "Power Driver"
#define STR_ATA "IDE/ATA Driver"

#define MOD_ISR  0xF00000
#define STR_ISR "Interrupt Service"
#define STR_UNK "Unknown"

#define RC_OK 0

/******************************/
/* KAL specific return values */
/******************************/
#define RC_KAL_EMPTY            (MOD_KAL + 0x0001)
#define RC_KAL_DESTROYED        (MOD_KAL + 0x0002)
#define RC_KAL_INVALID          (MOD_KAL + 0x0003)
#define RC_KAL_NOTHOOKED        (MOD_KAL + 0x0004)
#define RC_KAL_BADPTR           (MOD_KAL + 0x0005)
#define RC_KAL_PRIVATE          (MOD_KAL + 0x0006)
#define RC_KAL_SYSERR           (MOD_KAL + 0x0007)
#define RC_KAL_QFULL            (MOD_KAL + 0x0008)
#define RC_KAL_NORESOURCES      (MOD_KAL + 0x0009)
#define RC_KAL_ASLEEP           (MOD_KAL + 0x000A)
#define RC_KAL_INUSE            (MOD_KAL + 0x000B)
#define RC_KAL_NOTSET           (MOD_KAL + 0x000C)
#define RC_KAL_PERIOD           (MOD_KAL + 0x000D)
#define RC_KAL_INITIALISED      (MOD_KAL + 0x000E)
#define RC_KAL_TASKFAIL         (MOD_KAL + 0x000F)
#define RC_KAL_DIRECTSEG        (MOD_KAL + 0x0010)
#define RC_KAL_FARSEGALLOC      (MOD_KAL + 0x0011)
#define RC_KAL_FARSEGFREE       (MOD_KAL + 0x0012)
#define RC_KAL_TIMEOUT          (MOD_KAL + 0x0013)
#define RC_KAL_INTSAFETY        (MOD_KAL + 0x0014)
#define RC_KAL_SUSPENDED        (MOD_KAL + 0x0015)

/*********************/
/* ISR return values */
/*********************/

#define RC_ISR_HANDLED    (MOD_ISR + 0x01)
#define RC_ISR_NOTHANDLED (MOD_ISR + 0x02)

/******************************/
/* DEMUX DRIVER return values */
/******************************/
#define RC_DPS_SCREATE_FAILED           (MOD_DPS + 0x01)
#define RC_DPS_PCREATE_FAILED           (MOD_DPS + 0x02)
#define RC_DPS_REGISR_FAILED            (MOD_DPS + 0x03)
#define RC_DPS_QSEND_FAILED             (MOD_DPS + 0x04)
#define RC_DPS_REINIT_FAILED            (MOD_DPS + 0x05)

/************************************/
/* MPEG(VIDEO) DRIVER return values */
/************************************/
#define RC_MPG_QCREATE_FAILED           (MOD_MPG + 0x01)
#define RC_MPG_PCREATE_FAILED           (MOD_MPG + 0x02)
#define RC_MPG_REGISR_FAILED            (MOD_MPG + 0x03)
#define RC_MPG_QSEND_FAILED             (MOD_MPG + 0x04)
#define RC_MPG_REINIT_FAILED            (MOD_MPG + 0x05)
#define RC_MPG_MC_LOAD_FAILED           (MOD_MPG + 0x06)

/************************************/
/* Demodulator driver return values */
/************************************/
#define RC_SDM_QCREATE                  (MOD_SDM + 0x01)
#define RC_SDM_PCREATE                  (MOD_SDM + 0x02)
#define RC_SDM_QFULL                    (MOD_SDM + 0x03)
#define RC_SDM_BADMSG                   (MOD_SDM + 0x04)
#define RC_SDM_BADFREQ                  (MOD_SDM + 0x05)
#define RC_SDM_BADVAL                   (MOD_SDM + 0x06)
#define RC_SDM_SYSERR                   (MOD_SDM + 0x07)

/*********************************************/
/* Front panel keyboard driver return values */
/*********************************************/
#define RC_KEY_ISR                      (MOD_KEY + 0x01)
#define RC_KEY_TIMERCREATE              (MOD_KEY + 0x02)
#define RC_KEY_TIMERSTART               (MOD_KEY + 0x03)
#define RC_KEY_TIMERSET                 (MOD_KEY + 0x04)

/******************************/
/* Infra Red return values */
/******************************/
#define RC_IRD_SCREATE_FAILED           (MOD_IRD + 0x01)
#define RC_IRD_PCREATE_FAILED           (MOD_IRD + 0x02)
#define RC_IRD_REGISR_FAILED            (MOD_IRD + 0x03)
#define RC_IRD_QSEND_FAILED             (MOD_IRD + 0x04)
#define RC_IRD_REINIT_FAILED            (MOD_IRD + 0x05)
#define RC_IRD_BUFFER_OVERFLOWED        (MOD_IRD + 0x06)
#define RC_IRD_FIFO_OVERFLOWED          (MOD_IRD + 0x07)
#define RC_IRD_HDR_FOUND_GAP_ZERO       (MOD_IRD + 0x08)
#define RC_IRD_INVALID_STATE            (MOD_IRD + 0x09)

/******************************/
/* Flash return values        */
/******************************/
#define RC_FLASH_QCREATE_FAILED           (MOD_FSH + 0x01)
#define RC_FLASH_SCREATE_FAILED           (MOD_FSH + 0x01)
#define RC_FLASH_PCREATE_FAILED           (MOD_FSH + 0x02)
#define RC_FLASH_REGISR_FAILED            (MOD_FSH + 0x03)
#define RC_FLASH_QSEND_FAILED             (MOD_FSH + 0x04)
#define RC_FLASH_REINIT_FAILED            (MOD_FSH + 0x05)
#define RC_FLASH_QFULL                    (MOD_FSH + 0x06)
#define RC_FLASH_BADMSG                   (MOD_FSH + 0X07)

/******************************/
/* I2C return values          */
/******************************/
#define RC_I2C_NO_ADDRESS_ACK             (MOD_I2C + 0x01)
#define RC_I2C_BUS_TIMEOUT                (MOD_I2C + 0x02)
#define RC_I2C_NO_ACK                     (MOD_I2C + 0x03)

/***************************************/
/* OpenTV Control Driver return values */
/***************************************/
#define RC_OTV_BADHANDLE                  (MOD_OTV + 0x01)
#define RC_OTV_BADCONFIG                  (MOD_OTV + 0x02)

/********************************/
/* General Driver Return Values */
/********************************/
#define RC_GEN_IIC                        (MOD_GEN + 0x01)
#define RC_GEN_BAD_GPIO_POLARITY          (MOD_GEN + 0x02)

/********************************/
/* IDE/ATA Driver Return Values */
/********************************/
#define RC_ATA_IO_ERROR                   (MOD_ATA + 0x01)

#endif
