/****************************************************************************/
/*                          Conexant Systems Inc.                           */
/****************************************************************************/
/*                                                                          */
/* Filename:       AVPRO5002.H                                              */
/*                                                                          */
/* Description:    Header file defining register contents for TDK AVpro5002 */
/*                 SCART controller                                         */
/*                                                                          */
/* Author:         Dave Wilson                                              */
/*                                                                          */
/* Copyright Conexant Systems Inc, 2000                                     */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/
#ifndef _AVPRO5002_H_
#define _AVPRO5002_H_

/***************/
/* I2C Address */
/***************/

/* See relevant vendor header file for I2C_ADDR_AVPRO5002B definition */

/******************************/
/* Intenal Register Addresses */
/******************************/
#define NUM_5002_WRITE_REGS     5

/* NB: Write registers are not individually addressable. All 5 must be */
/*     written in a single I2C transaction                             */

#define AVPRO_AUD_CTL_REG    0x00
#define AVPRO_AV_CTL_REG     0x01
#define AVPRO_VID_CTL_REG    0x02
#define AVPRO_AUD_B_CTL_REG  0x03
#define AVPRO_AUD_C_CTL_REG  0x04

/*****************/
/* Read Register */
/*****************/
#define READ_INPUT_MASK      0x03
#define READ_BELOW_2V        0x00
#define READ_4_TO_7V         0x01
#define READ_9_TO_12V        0x02

#define READ_TV_SYNC_MASK    0x04
#define READ_TV_NO_SYNC      0x00
#define READ_TV_SYNC         0x04

#define READ_AUX_SYNC_MASK   0x08
#define READ_AUX_NO_SYNC     0x00
#define READ_AUX_SYNC        0x08

#define READ_ID_MASK         0xF0
#define READ_ID_5002B        0x20

/****************************/
/* Audio Control Register A */
/****************************/
#define AUD_A_VOLUME_MASK    0x3F

#define AUD_A_TV_MUTE_MASK   0x40
#define AUD_A_TV_UNMUTED     0x00
#define AUD_A_TV_MUTED       0x40

#define AUD_A_AUX_MUTE_MASK  0x80
#define AUD_A_AUX_UNMUTED    0x00
#define AUD_A_AUX_MUTED      0x80

/********************************/
/* Audio/Video Control Register */
/********************************/
#define AV_TV_SRC_MASK        0x07
#define AV_TV_SRC_ENC_YC      0x00
#define AV_TV_SRC_AUX_YC      0x01
#define AV_TV_SRC_SVHS_ENC1   0x02
#define AV_TV_SRC_SVHS_ENC2   0x03
#define AV_TV_SRC_SVHS_ENC3   0x04
#define AV_TV_SRC_SVHS_AUX1   0x05
#define AV_TV_SRC_MUTE1       0x06
#define AV_TV_SRC_MUTE2       0x07

#define AV_AUX_SRC_MASK       0x38
#define AV_AUX_SRC_COMP_ENC1  0x00
#define AV_AUX_SRC_COMP_ENC2  0x08
#define AV_AUX_SRC_SVHS_ENC1  0x18
#define AV_AUX_SRC_SVHS_ENC2  0x20
#define AV_AUX_SRC_SVHS_ENC3  0x28
#define AV_AUX_SRC_MUTE1      0x30
#define AV_AUX_SRC_MUTE2      0x38

#define AV_RGB_SYNC_MASK      0x80
#define AV_RGB_SYNC_RGB       0x00
#define AV_RGB_SYNC_YC        0x80

/**************************/
/* Video Control Register */
/**************************/
#define VID_FUNC_MASK         0x03
#define VID_FUNC_NORMAL       0x00
#define VID_FUNC_16_9         0x01
#define VID_FUNC_PTV_OUTPUT1  0x02
#define VID_FUNC_PTV_OUTPUT2  0x03

#define VID_FUNC_DIR_MASK     0x0C
#define VID_FUNC_BOTH_OUT     0x00
#define VID_FUNC_TV_IN        0x04
#define VID_FUNC_AUX_IN       0x08

#define VID_RGB_ATT_MASK      0x30
#define VID_RGB_ATT_NORMAL    0x00
#define VID_RGB_ATT_10        0x10
#define VID_RGB_ATT_20        0x20
#define VID_RGB_ATT_30        0x30

#define VID_BLANK_SEL_MASK    0xC0
#define VID_BLANK_ABLANK      0x00
#define VID_BLANK_EBLANK      0x40
#define VID_BLANK_0V          0x80
#define VID_BLANK_2V          0xC0

/****************************/
/* Audio Control Register B */
/****************************/
#define AUD_B_DATA_MASK       0x03
#define AUD_B_DATA_0          0x00
#define AUD_B_DATA_1          0x01
#define AUD_B_DATA_2          0x02
#define AUD_B_DATA_3          0x03
#define AUD_B_TV_STEREO       0x00
#define AUD_B_TV_MONO         0x10
#define AUD_B_TV_LEFT         0x20
#define AUD_B_TV_RIGHT        0x30

#define AUD_B_AUX_STEREO      0x00
#define AUD_B_AUX_MONO        0x40
#define AUD_B_AUX_LEFT        0x80
#define AUD_B_AUX_RIGHT       0xC0

/****************************/
/* Audio Control Register C */
/****************************/
#define AUD_C_TV_VOL_MASK     0x03
#define AUD_C_TV_VOL1_CONTROL 0x00
#define AUD_C_TV_VOL1_BYPASS  0x01
#define AUD_C_TV_VOL2_CONTROL 0x00
#define AUD_C_TV_VOL2_BYPASS  0x02

#define AUD_C_AUD_1_ENABLED   0x00
#define AUD_C_AUD_1_DISABLED  0x08

#define AUD_C_AUD_2_0DB_ATT   0x00
#define AUD_C_AUD_2_20DB_ATT  0x10
#define AUD_C_AUD_2_DISABLED  0x20

#define AUD_C_TV_0DB_ATT      0x00
#define AUD_C_TV_20DB_ATT     0x40
#define AUD_C_TV_DISABLED     0x80


#endif /* _AVPRO5002_H_ */

