/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*           Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002         */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       sony_cxa2161r.h
 *
 *
 * Description:    Sony CXA2161R SCART Controller header
 *
 *
 * Author:         Dave Wilson
 *
 ****************************************************************************/
/* $Header: sony_cxa2161r.h, 1, 4/4/03 3:15:58 PM, Dave Wilson$
 ****************************************************************************/

/*********************************************************************************/
/* Note on register content definitions in this file:                            */
/*                                                                               */
/* Other headers, notably those defining Conexant chip register maps, define the */
/* contents of a particular register field as the unshifted value and assume     */
/* that code using them access the register via a macro that will shift the      */
/* data appropriately. For example, 2 labels for a field at bit 31 would be      */
/* defined having values 0 and 1 and a macro such as CNXT_SET_VAL will be used   */
/* to automatically shift the data to the correct position within the register.  */
/* In this header, however, field values are defined already shifted so that     */
/* it is easier to define the initial values of the 7 device registers. Do not   */
/* use CNXT_SET_VAL or CNXT_GET_VAL to set or read CXA2161R registers since      */
/* these will write or return values that are incorrect given the definitions    */
/* here!                                                                         */
/*********************************************************************************/

#ifndef _SONY_CXA2161R_H_
#define _SONY_CXA2161R_H_

#define NUM_CXA2161R_REGISTERS 7

/*****************************************/
/* wRITE Register addresses and contents */
/*****************************************/

/**********/
/* Data 1 */
/**********/
#define CXA_AUDIO_VOLUME_REG       0x00
  #define CXA_TV_AUD_MUTE_MASK     0x01
  #define CXA_TV_AUD_MUTE_SHIFT       0
    #define CXA_TV_AUD_MUTE           (1 << CXA_TV_AUD_MUTE_SHIFT)
    #define CXA_TV_AUD_UNMUTE         (0 << CXA_TV_AUD_MUTE_SHIFT)
  #define CXA_AUD_VOLUME_MASK      0x3E
  #define CXA_AUD_VOLUME_SHIFT        1
    /* Volume values start at 0 for +6dB with volume */
    /* decreasing by 2dB for each increment of the   */
    /* register aud volume field.                    */
  #define CXA_AUD_INPUT_ATTN_MASK  0xC0
  #define CXA_AUD_INPUT_ATTN_SHIFT    6
    /* Attenuation values values start at 0 for -6dB */
    /* increasing by 3dB for each increment of the   */
    /* register field.                               */
    
/**********/
/* Data 2 */
/**********/
#define CXA_TV_AUD_SELECT_REG      0x01
  #define CXA_PHONO_BYPASS_MASK    0x01
  #define CXA_PHONO_BYPASS_SHIFT      0
    #define CXA_PHONO_AFTER_VOL       (0 << CXA_PHONO_BYPASS_SHIFT)
    #define CXA_PHONO_BEFORE_VOL      (1 << CXA_PHONO_BYPASS_SHIFT)
  #define CXA_CHANNEL_SEL_TV_MASK  0x06
  #define CXA_CHANNEL_SEL_TV_SHIFT    1
    #define CXA_CHAN_SEL_TV_R1_L1     (0 << CXA_CHANNEL_SEL_TV_SHIFT)
    #define CXA_CHAN_SEL_TV_R2_L2     (1 << CXA_CHANNEL_SEL_TV_SHIFT)
    #define CXA_CHAN_SEL_TV_R3_L3     (2 << CXA_CHANNEL_SEL_TV_SHIFT)
    #define CXA_CHAN_SEL_TV_R4_L4     (3 << CXA_CHANNEL_SEL_TV_SHIFT)
  #define CXA_MONO_SWITCH_TV_MASK  0x38
  #define CXA_MONO_SWITCH_TV_SHIFT    3
    #define CXA_MONO_SWITCH_TV_NORMAL (0 << CXA_MONO_SWITCH_TV_SHIFT)
    #define CXA_MONO_SWITCH_TV_MONO   (1 << CXA_MONO_SWITCH_TV_SHIFT)
    #define CXA_MONO_SWITCH_TV_SWAP   (2 << CXA_MONO_SWITCH_TV_SHIFT)
    #define CXA_MONO_SWITCH_TV_R_ONLY (3 << CXA_MONO_SWITCH_TV_SHIFT)
    #define CXA_MONO_SWITCH_TV_L_ONLY (4 << CXA_MONO_SWITCH_TV_SHIFT)
  #define CXA_TV_BYPASS_MASK       0x40
  #define CXA_TV_BYPASS_SHIFT         6
    #define CXA_TV_AFTER_VOL          (0 << CXA_TV_BYPASS_SHIFT)
    #define CXA_TV_BEFORE_VOL         (1 << CXA_TV_BYPASS_SHIFT)
  #define CXA_MONO_MIX_MASK        0x80
  #define CXA_MONO_MIX_SHIFT          7
    #define CXA_MONO_TV_MIX           (0 << CXA_MONO_MIX_SHIFT)
    #define CXA_MONO_R_L_MIX          (1 << CXA_MONO_MIX_SHIFT)

/**********/
/* Data 3 */
/**********/
#define CXA_VCR_AUD_SELECT_REG     0x02
  #define CXA_AUD_OVERLAY_MASK     0x01
  #define CXA_AUD_OVERLAY_SHIFT       0
    #define CXA_AUD_OVERLAY_OFF       (0 << CXA_AUD_OVERLAY_SHIFT)
    #define CXA_AUD_OVERLAY_ON        (1 << CXA_AUD_OVERLAY_SHIFT)
  #define CXA_CHANNEL_SEL_VCR_MASK 0x06
  #define CXA_CHANNEL_SEL_VCR_SHIFT   1
    #define CXA_CHAN_SEL_VCR_R1_L1    (0 << CXA_CHANNEL_SEL_VCR_SHIFT)
    #define CXA_CHAN_SEL_VCR_R2_L2    (1 << CXA_CHANNEL_SEL_VCR_SHIFT)
    #define CXA_CHAN_SEL_VCR_R3_L3    (2 << CXA_CHANNEL_SEL_VCR_SHIFT)
    #define CXA_CHAN_SEL_VCR_R4_L4    (3 << CXA_CHANNEL_SEL_VCR_SHIFT)
  #define CXA_MONO_SWITCH_VCR_MASK 0x38
  #define CXA_MONO_SWITCH_VCR_SHIFT   3
    #define CXA_MONO_SWITCH_VCR_NORMAL (0 << CXA_MONO_SWITCH_VCR_SHIFT)
    #define CXA_MONO_SWITCH_VCR_MUTE   (7 << CXA_MONO_SWITCH_VCR_SHIFT)
    #define CXA_MONO_SWITCH_VCR_MONO   (1 << CXA_MONO_SWITCH_VCR_SHIFT)
    #define CXA_MONO_SWITCH_VCR_SWAP   (2 << CXA_MONO_SWITCH_VCR_SHIFT)
    #define CXA_MONO_SWITCH_VCR_R_ONLY (3 << CXA_MONO_SWITCH_VCR_SHIFT)
    #define CXA_MONO_SWITCH_VCR_L_ONLY (4 << CXA_MONO_SWITCH_VCR_SHIFT)
  #define CXA_OUTPUT_LIMIT_MASK    0x40
  #define CXA_OUTPUT_LIMIT_SHIFT      6
    #define CXA_OUTPUT_LIMIT_ON       (1 << CXA_OUTPUT_LIMIT_SHIFT)
    #define CXA_OUTPUT_LIMIT_OFF      (0 << CXA_OUTPUT_LIMIT_SHIFT)
  #define CXA_TV_AUD_MUTE_2_MASK   0x01
  #define CXA_TV_AUD_MUTE_2_SHIFT     7
    #define CXA_TV_AUD_MUTE_2         (1 << CXA_TV_AUD_MUTE_2_SHIFT)
    #define CXA_TV_AUD_UNMUTE_2       (0 << CXA_TV_AUD_MUTE_2_SHIFT)
    
/**********/
/* Data 4 */
/**********/
#define CXA_MISC_SIGNAL_REG         0x03
  #define CXA_FAST_BLANK_MASK       0x03
  #define CXA_FAST_BLANK_SHIFT         0
    #define CXA_FAST_BLANK_0V          (0 << CXA_FAST_BLANK_SHIFT)
    #define CXA_FAST_BLANK_IN_1        (1 << CXA_FAST_BLANK_SHIFT)
    #define CXA_FAST_BLANK_IN_2        (2 << CXA_FAST_BLANK_SHIFT)
    #define CXA_FAST_BLANK_3V          (3 << CXA_FAST_BLANK_SHIFT)
  #define CXA_PIN8_CTL_MASK         0x0C
  #define CXA_PIN8_CTL_SHIFT           2
    #define CXA_PIN8_VCR_IN_TV_CTRL    (0 << CXA_PIN8_CTL_SHIFT)
    #define CXA_PIN8_VCR_CTRL_TV_HOLD  (1 << CXA_PIN8_CTL_SHIFT)
    #define CXA_PIN8_VCR_IN_TV_FOLLOW  (2 << CXA_PIN8_CTL_SHIFT)
    #define CXA_PIN8_BOTH_CTRL         (3 << CXA_PIN8_CTL_SHIFT)
  #define CXA_FNC_LEVEL_MASK        0x30
  #define CXA_FNC_LEVEL_SHIFT          4
    #define CXA_FNC_LEVEL_OFF          (0 << CXA_FNC_LEVEL_SHIFT)
    #define CXA_FNC_LEVEL_16_9         (1 << CXA_FNC_LEVEL_SHIFT)
    #define CXA_FNC_LEVEL_4_3          (3 << CXA_FNC_LEVEL_SHIFT)
  #define CXA_LOGIC_LVL_MASK        0x04
  #define CXA_LOGIC_LVL_SHIFT          6
    #define CXA_LOGIC_LVL_OFF          (0 << CXA_LOGIC_LVL_SHIFT)
    #define CXA_LOGIC_LVL_ON           (1 << CXA_LOGIC_LVL_SHIFT)
  #define CXA_TV_INPUT_MASK         0x04
  #define CXA_TV_INPUT_SHIFT           7
    #define CXA_TV_INPUT_MUTE          (1 << CXA_TV_INPUT_SHIFT)
    #define CXA_TV_INPUT_UNMUTE        (0 << CXA_TV_INPUT_SHIFT)
  
/**********/
/* Data 5 */
/**********/
#define CXA_VIDEO_SWITCH_REG        0x04
  #define CXA_TV_VID_SWITCH_MASK    0x07
  #define CXA_TV_VID_SWITCH_SHIFT      0
    #define CXA_TV_VID_ENC_RGB_CVBS_1  (0 << CXA_TV_VID_SWITCH_SHIFT)
    #define CXA_TV_VID_ENC_YC_1        (1 << CXA_TV_VID_SWITCH_SHIFT)
    #define CXA_TV_VID_VCR_YC_RGB      (2 << CXA_TV_VID_SWITCH_SHIFT)
    #define CXA_TV_VID_TV              (3 << CXA_TV_VID_SWITCH_SHIFT)
    #define CXA_TV_VID_ENC_YC_2        (4 << CXA_TV_VID_SWITCH_SHIFT)
    #define CXA_TV_VID_ENC_RGB_CVBS_2  (5 << CXA_TV_VID_SWITCH_SHIFT)
    #define CXA_TV_VID_AUX_YC_CVBS     (6 << CXA_TV_VID_SWITCH_SHIFT)
    #define CXA_TV_VID_MUTE            (7 << CXA_TV_VID_SWITCH_SHIFT)
  #define CXA_RGB_GAIN_MASK         0x18
  #define CXA_RGB_GAIN_SHIFT           3
    #define CXA_RGB_GAIN_0DB           (0 << CXA_RGB_GAIN_SHIFT)
    #define CXA_RGB_GAIN_1DB           (1 << CXA_RGB_GAIN_SHIFT
    #define CXA_RGB_GAIN_2DB           (2 << CXA_RGB_GAIN_SHIFT
    #define CXA_RGB_GAIN_3DB           (3 << CXA_RGB_GAIN_SHIFT
  #define CXA_VCR_VID_SWITCH_MASK   0xE0
  #define CXA_VCR_VID_SWITCH_SHIFT     5
    #define CXA_VCR_VID_ENC_YC_1       (0 << CXA_VCR_VID_SWITCH_SHIFT)
    #define CXA_VCR_VID_ENC_YC_CVBS    (1 << CXA_VCR_VID_SWITCH_SHIFT)
    #define CXA_VCR_VID_VCR_YC         (2 << CXA_VCR_VID_SWITCH_SHIFT)
    #define CXA_VCR_VID_TV_CVBS        (3 << CXA_VCR_VID_SWITCH_SHIFT)
    #define CXA_VCR_VID_ENC_YC_2       (4 << CXA_VCR_VID_SWITCH_SHIFT)
    #define CXA_VCR_VID_AUX_CVBS       (5 << CXA_VCR_VID_SWITCH_SHIFT)
    #define CXA_VCR_VID_AUX_YC_CVBS    (6 << CXA_VCR_VID_SWITCH_SHIFT)
    #define CXA_VCR_VID_MUTE           (7 << CXA_VCR_VID_SWITCH_SHIFT)
  
/**********/
/* Data 6 */
/**********/
#define CXA_CLAMP_SYNC_REG          0x05
  #define CXA_MIXER_CTL_MASK        0x03
  #define CXA_MIXER_CTL_SHIFT          0
    #define CXA_MIXER_VO7_V04          (0 << CXA_MIXER_CTL_SHIFT)
    #define CXA_MIXER_MIX_Y_C          (1 << CXA_MIXER_CTL_SHIFT)
    #define CXA_MIXER_VO7_VO8          (2 << CXA_MIXER_CTL_SHIFT)
    #define CXA_MIXER_VO7_VO4          (3 << CXA_MIXER_CTL_SHIFT)
  #define CXA_VIN3_CLAMP_MASK       0x04
  #define CXA_VIN3_CLAMP_SHIFT         2
    #define CXA_VIN3_GREEN             (0 << CXA_VIN3_CLAMP_SHIFT)
    #define CXA_VIN3_CVBS              (1 << CXA_VIN3_CLAMP_SHIFT)
  #define CXA_VIN7_CLAMP_MASK       0x08
  #define CXA_VIN7_CLAMP_SHIFT         3
    #define CXA_VIN7_CHROMINANCE       (0 << CXA_VIN7_CLAMP_SHIFT)
    #define CXA_VIN7_RED               (1 << CXA_VIN7_CLAMP_SHIFT)
  #define CXA_VIN5_CLAMP_MASK       0x10
  #define CXA_VIN5_CLAMP_SHIFT         4
    #define CXA_VIN5_RED               (0 << CXA_VIN5_CLAMP_SHIFT)
    #define CXA_VIN5_CHROMINANCE       (1 << CXA_VIN5_CLAMP_SHIFT)
  #define CXA_SYNC_SEL_MASK         0x60
  #define CXA_SYNC_SEL_SHIFT           5
    #define CXA_SYNC_VIN8              (0 << CXA_SYNC_SEL_SHIFT)
    #define CXA_SYNC_VIN9              (1 << CXA_SYNC_SEL_SHIFT)
    #define CXA_SYNC_VIN10             (2 << CXA_SYNC_SEL_SHIFT)
    #define CXA_SYNC_VIN12             (3 << CXA_SYNC_SEL_SHIFT)
  #define CXA_VCR_IN_MUTE_MASK      0x80
  #define CXA_VCR_IN_MUTE_SHIFT        7
    #define CXA_VCR_IN_MUTE            (1 << CXA_VCR_IN_MUTE_SHIFT)
    #define CXA_VCR_IN_UNMUTE          (0 << CXA_VCR_IN_MUTE_SHIFT)

/**********/
/* Data 7 */
/**********/
#define CXA_VIDEO_ENABLE_REG        0x06
  #define CXA_VIDEO_ENABLE_MASK     0x3F
  #define CXA_VIDEO_ENABLE_SHIFT       0
    #define CXA_VIDEO_ENABLE_V0UT1  (0x01 << CXA_VIDEO_ENABLE_SHIFT)
    #define CXA_VIDEO_ENABLE_V0UT2  (0x02 << CXA_VIDEO_ENABLE_SHIFT)
    #define CXA_VIDEO_ENABLE_V0UT3  (0x04 << CXA_VIDEO_ENABLE_SHIFT)
    #define CXA_VIDEO_ENABLE_V0UT4  (0x08 << CXA_VIDEO_ENABLE_SHIFT)
    #define CXA_VIDEO_ENABLE_V0UT5  (0x10 << CXA_VIDEO_ENABLE_SHIFT)
    #define CXA_VIDEO_ENABLE_V0UT6  (0x20 << CXA_VIDEO_ENABLE_SHIFT)
  #define CXA_VOUT5_0V_MASK         (0x40 << CXA_VIDEO_ENABLE_SHIFT)
  #define CXA_VOUT5_0V_SHIFT           6
    #define CXA_VOUT5_ACTIVE           (0 << CXA_VOUT5_0V_SHIFT)
    #define CXA_VOUT5_0V               (1 << CXA_VOUT5_0V_SHIFT)
  #define CXA_ZCD_MASK              0x80
  #define CXA_ZCD_SHIFT                7
    #define CXA_ZCD_ENABLE             (1 << CXA_ZCD_SHIFT)
    #define CXA_ZCD_DISABLE            (0 << CXA_ZCD_SHIFT)
  
/**************************/
/* Read Register Contents */  
/**************************/
#define CXA_READ_FNC_VCR_MASK       0x03
#define CXA_READ_FNC_VCR_SHIFT         0
  #define CXA_FNC_VCR_0FF              (0 << CXA_READ_FNC_VCR_SHIFT)
  #define CXA_FNC_VCR_16_9             (1 << CXA_READ_FNC_VCR_SHIFT)
  #define CXA_FNC_VCR_4_3              (3 << CXA_READ_FNC_VCR_SHIFT)
#define CXA_READ_SYNC_DETECT_MASK   0x04
#define CXA_READ_SYNC_DETECT_SHIFT     2
  #define CXA_SYNC_DETECTED            (1 << CXA_READ_SYNC_DETECT_SHIFT)
  #define CXA_SYNC_NOT_DETECTED        (0 << CXA_READ_SYNC_DETECT_SHIFT)
#define CXA_READ_POWER_DETECT_MASK  0x10
#define CXA_READ_POWER_DETECT_SHIFT    4
  #define CXA_POWER_DETECT_SET         (1 << CXA_READ_POWER_DETECT_SHIFT)
  #define CXA_POWER_DETECT_CLEAR       (0 << CXA_READ_POWER_DETECT_SHIFT)
#define CXA_READ_ZCD_STATUS_MASK    0x20
#define CXA_READ_ZCD_STATUS_SHIFT      5
  #define CXA_ZCD_DETECTED             (0 << CXA_READ_ZCD_STATUS_SHIFT)
  #define CXA_ZCD_NOT_DETECTED         (1 << CXA_READ_ZCD_STATUS_SHIFT)
    
#endif

/****************************************************************************
 * Modifications:
 * $Log: 
 *  1    mpeg      1.0         4/4/03 3:15:58 PM      Dave Wilson     
 * $
 * 
 *    Rev 1.0   04 Apr 2003 15:15:58   dawilson
 * SCR(s) 5968 :
 * Header file describing the registers in the Sony CXA2161R SCART controller
 *
 ****************************************************************************/

