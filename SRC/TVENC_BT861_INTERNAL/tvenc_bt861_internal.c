/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                Copyright Conexant Systems Inc. 1998-2003                 */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        tvenc_bt861_internal.c
 *
 *
 * Description:     Functions relating to setup and operation of internal video encoders
 *
 *
 * Author:          Xin Golden (based on the encoder.c in pvcs\osdlibc)
 *
 ****************************************************************************/
/* $Id: tvenc_bt861_internal.c,v 1.10, 2004-03-24 18:12:23Z, Billy Jackman$
 ****************************************************************************/

#define USE_MATH   0

/*****************/
/* Include Files */
/*****************/
#include "math.h"
#include "kal.h"
#include "globals.h"
#include "iic.h"
#include "osdlibc.h"
#include "vidlibc.h"
#include "vidprvc.h"
#include "handle.h"
#include "tvenc.h"
#include "tvenc_priv.h"
#include "tvenc_module_api.h"
#if DIGITAL_DISPLAY == PW113_DISPLAY
#include "pw113_display.h"
#endif

extern int gnOsdMaxHeight;
extern int gnOsdMaxWidth;
extern POSDLIBREGION gpOsdLibList[];
extern u_int32 gnHBlank;
extern u_int32 gnVBlank;


/* registers needed to set up NTSC_M standard for internal encode type INTERNAL_BT861_INTERNAL */ 
/* the first element of each array is register address */
static const TVENC_REG_T TVENC_861LIKE_NTSC_M_Setup[] =
{
   { ENC_CONTROL0_REG, ENC_CTL0_OUTMODE_MASK, INTERNAL_ENCODER_OUTMODE },
   { ENC_LINE_TIME_REG, ENC_LINE_TIME_HACTIVE_MASK, 0x2c8 << ENC_LINE_TIME_HACTIVE_SHIFT },
   { ENC_LINE_TIME_REG, ENC_LINE_TIME_HCLK_MASK, 0x6b4 << ENC_LINE_TIME_HCLK_SHIFT },
   { ENC_HORIZONTAL_TIMING_REG, ENC_HORZ_TIMING_HBURST_END_MASK, 0x54 << ENC_HORZ_TIMING_HBURST_END_SHIFT },
   { ENC_HORIZONTAL_TIMING_REG, ENC_HORZ_TIMING_HBURST_BEG_MASK, 0x90 << ENC_HORZ_TIMING_HBURST_BEG_SHIFT },
   { ENC_HORIZONTAL_TIMING_REG, ENC_HORZ_TIMING_AHSYNCWIDTH_MASK, 0x7e << ENC_HORZ_TIMING_AHSYNCWIDTH_SHIFT },
   { ENC_HBLANK_REG, ENC_HBLANK_VACTIVE_MASK, 0x0F1 << ENC_HBLANK_VACTIVE_SHIFT },
   { ENC_HBLANK_REG, ENC_HBLANK_HBLANK_MASK, 0x106 << ENC_HBLANK_HBLANK_SHIFT },
   { ENC_HSYNC_REG, ENC_HSYNC_VBLANK_MASK, 0x13 << ENC_HSYNC_VBLANK_SHIFT },
   { ENC_CONTROL0_REG, ENC_CTL0_PHASE_ALTERATION_MASK, ENC_CTL0_PHASE_ALTERATION_DISABLE },
   { ENC_CONTROL0_REG, ENC_CTL0_SETUP_MASK, ENC_CTL0_SETUP_ENABLE },
   { ENC_CONTROL0_REG, ENC_CTL0_LINE625_MASK, ENC_CTL0_LINE625_525_FORMAT },
   { ENC_CONTROL0_REG, ENC_CTL0_VSYNC_DURATION_MASK, ENC_CTL0_VSYNC_3_LINES },
   { ENC_AMPLITUDE_REG, ENC_AMP_BURST_AMP_MASK, 0x75 << ENC_AMP_BURST_AMP_SHIFT },
   { ENC_AMPLITUDE_REG, ENC_AMP_SYNC_AMP_MASK, 0xE5 << ENC_AMP_SYNC_AMP_SHIFT },
   { ENC_YCRCB_MULTIPLICATION_REG, ENC_YCRCB_MULT_MCR_MASK, 0xc1 << ENC_YCRCB_MULT_MCR_SHIFT },
   { ENC_YCRCB_MULTIPLICATION_REG, ENC_YCRCB_MULT_MCB_MASK, 0x89 << ENC_YCRCB_MULT_MCB_SHIFT },
   { ENC_YCRCB_MULTIPLICATION_REG, ENC_YCRCB_MULT_MY_MASK, 0x9a << ENC_YCRCB_MULT_MY_SHIFT },
   { ENC_SECAM_DR_REG, ENC_SECAM_DR_MASK, 0x21f07c1f << ENC_SECAM_DR_SHIFT },
   { ENC_YUV_MULTIPLICATION_REG, ENC_YUV_MULTIPLICATION_MASK, 0x00806288 <<ENC_YUV_MULTIPLICATION_SHIFT},
#if ENCODER_HAS_DISABLE_DACS == 1
  { ENC_CONTROL0_REG, ENC_CTL0_DAC_DISABLE_MASK, ENC_CTL0_DAC_DISABLE_ENABLE },
#endif
   { ENC_CONTROL0_REG, ENC_CTL0_EACTIVE_MASK, ENC_CTL0_EACTIVE_ENABLE_NORMAL }
};
static const u_int16 uTVENC_861LIKE_NTSC_M_Ops = sizeof(TVENC_861LIKE_NTSC_M_Setup)/sizeof(TVENC_REG_T);

/* registers needed to set up NTSC_J standard for internal encode type INTERNAL_BT861_INTERNAL */ 
/* the first element of each array is register address */
static const TVENC_REG_T TVENC_861LIKE_NTSC_J_Setup[] =
{
   { ENC_CONTROL0_REG, ENC_CTL0_OUTMODE_MASK, INTERNAL_ENCODER_OUTMODE },
   { ENC_LINE_TIME_REG, ENC_LINE_TIME_HACTIVE_MASK, 0x2c9 << ENC_LINE_TIME_HACTIVE_SHIFT },
   { ENC_LINE_TIME_REG, ENC_LINE_TIME_HCLK_MASK, 0x6b4 << ENC_LINE_TIME_HCLK_SHIFT },
   { ENC_HORIZONTAL_TIMING_REG, ENC_HORZ_TIMING_HBURST_END_MASK, 0x52 << ENC_HORZ_TIMING_HBURST_END_SHIFT },
   { ENC_HORIZONTAL_TIMING_REG, ENC_HORZ_TIMING_HBURST_BEG_MASK, 0x8e << ENC_HORZ_TIMING_HBURST_BEG_SHIFT },
   { ENC_HORIZONTAL_TIMING_REG, ENC_HORZ_TIMING_AHSYNCWIDTH_MASK, 0x7e << ENC_HORZ_TIMING_AHSYNCWIDTH_SHIFT },
   { ENC_HBLANK_REG, ENC_HBLANK_VACTIVE_MASK, 0x0F1 << ENC_HBLANK_VACTIVE_SHIFT },
   { ENC_HBLANK_REG, ENC_HBLANK_HBLANK_MASK, 0x108 << ENC_HBLANK_HBLANK_SHIFT },
   { ENC_HSYNC_REG, ENC_HSYNC_VBLANK_MASK, 0x13 << ENC_HSYNC_VBLANK_SHIFT },
   { ENC_CONTROL0_REG, ENC_CTL0_PHASE_ALTERATION_MASK, ENC_CTL0_PHASE_ALTERATION_DISABLE },
   { ENC_CONTROL0_REG, ENC_CTL0_SETUP_MASK, ENC_CTL0_SETUP_ENABLE },
   { ENC_CONTROL0_REG, ENC_CTL0_LINE625_MASK, ENC_CTL0_LINE625_525_FORMAT },
   { ENC_CONTROL0_REG, ENC_CTL0_VSYNC_DURATION_MASK, ENC_CTL0_VSYNC_3_LINES },
   { ENC_AMPLITUDE_REG, ENC_AMP_BURST_AMP_MASK, 0x7b << ENC_AMP_BURST_AMP_SHIFT },
   { ENC_AMPLITUDE_REG, ENC_AMP_SYNC_AMP_MASK, 0xE4 << ENC_AMP_SYNC_AMP_SHIFT },
   { ENC_YCRCB_MULTIPLICATION_REG, ENC_YCRCB_MULT_MCR_MASK, 0xda << ENC_YCRCB_MULT_MCR_SHIFT },
   { ENC_YCRCB_MULTIPLICATION_REG, ENC_YCRCB_MULT_MCB_MASK, 0x9b << ENC_YCRCB_MULT_MCB_SHIFT },
   { ENC_YCRCB_MULTIPLICATION_REG, ENC_YCRCB_MULT_MY_MASK, 0xa5 << ENC_YCRCB_MULT_MY_SHIFT },
   { ENC_SECAM_DR_REG, ENC_SECAM_DR_MASK, 0x21f07c1f << ENC_SECAM_DR_SHIFT },
   { ENC_YUV_MULTIPLICATION_REG, ENC_YUV_MULTIPLICATION_MASK, 0x00806288 <<ENC_YUV_MULTIPLICATION_SHIFT},
#if ENCODER_HAS_DISABLE_DACS == 1
  { ENC_CONTROL0_REG, ENC_CTL0_DAC_DISABLE_MASK, ENC_CTL0_DAC_DISABLE_ENABLE },
#endif
   { ENC_CONTROL0_REG, ENC_CTL0_EACTIVE_MASK, ENC_CTL0_EACTIVE_ENABLE_NORMAL }
};
static const u_int16 uTVENC_861LIKE_NTSC_J_Ops = sizeof(TVENC_861LIKE_NTSC_J_Setup)/sizeof(TVENC_REG_T);


/* registers needed to set up PAL_BDGHI standard for internal encode type INTERNAL_BT861_INTERNAL */ 
/* the first element of each array is register address */
static const TVENC_REG_T TVENC_861LIKE_PAL_BDGHI_Setup[] =
{
   { ENC_CONTROL0_REG, ENC_CTL0_OUTMODE_MASK, INTERNAL_ENCODER_OUTMODE },
   { ENC_LINE_TIME_REG, ENC_LINE_TIME_HACTIVE_MASK, 0x2cf << ENC_LINE_TIME_HACTIVE_SHIFT },
   { ENC_LINE_TIME_REG, ENC_LINE_TIME_HCLK_MASK, 0x6c0 << ENC_LINE_TIME_HCLK_SHIFT },
   { ENC_HORIZONTAL_TIMING_REG, ENC_HORZ_TIMING_HBURST_END_MASK, 0x54 << ENC_HORZ_TIMING_HBURST_END_SHIFT },
   { ENC_HORIZONTAL_TIMING_REG, ENC_HORZ_TIMING_HBURST_BEG_MASK, 0x98 << ENC_HORZ_TIMING_HBURST_BEG_SHIFT },
   { ENC_HORIZONTAL_TIMING_REG, ENC_HORZ_TIMING_AHSYNCWIDTH_MASK, 0x7e << ENC_HORZ_TIMING_AHSYNCWIDTH_SHIFT },
   { ENC_HBLANK_REG, ENC_HBLANK_VACTIVE_MASK, 0x120 << ENC_HBLANK_VACTIVE_SHIFT },
   { ENC_HBLANK_REG, ENC_HBLANK_HBLANK_MASK, 0x106 << ENC_HBLANK_HBLANK_SHIFT },
   { ENC_HSYNC_REG, ENC_HSYNC_VBLANK_MASK, 0x17 << ENC_HSYNC_VBLANK_SHIFT },
   { ENC_CONTROL0_REG, ENC_CTL0_PHASE_ALTERATION_MASK, ENC_CTL0_PHASE_ALTERATION_ENABLE },
   { ENC_CONTROL0_REG, ENC_CTL0_SETUP_MASK, ENC_CTL0_SETUP_DISABLE },
   { ENC_CONTROL0_REG, ENC_CTL0_LINE625_MASK, ENC_CTL0_LINE625_625_FORMAT },
   { ENC_CONTROL0_REG, ENC_CTL0_VSYNC_DURATION_MASK, ENC_CTL0_VSYNC_2_5_LINES },
   { ENC_AMPLITUDE_REG, ENC_AMP_BURST_AMP_MASK, 0x5e << ENC_AMP_BURST_AMP_SHIFT },
   { ENC_AMPLITUDE_REG, ENC_AMP_SYNC_AMP_MASK, 0xf0 << ENC_AMP_SYNC_AMP_SHIFT },
   { ENC_YCRCB_MULTIPLICATION_REG, ENC_YCRCB_MULT_MCR_MASK, 0xcf << ENC_YCRCB_MULT_MCR_SHIFT },
   { ENC_YCRCB_MULTIPLICATION_REG, ENC_YCRCB_MULT_MCB_MASK, 0x93 << ENC_YCRCB_MULT_MCB_SHIFT },
   { ENC_YCRCB_MULTIPLICATION_REG, ENC_YCRCB_MULT_MY_MASK, 0xa3 << ENC_YCRCB_MULT_MY_SHIFT },
   { ENC_SECAM_DR_REG, ENC_SECAM_DR_MASK, 0x2a098acb << ENC_SECAM_DR_SHIFT },
   { ENC_YUV_MULTIPLICATION_REG, ENC_YUV_MULTIPLICATION_MASK, 0x00806288 <<ENC_YUV_MULTIPLICATION_SHIFT},
#if ENCODER_HAS_DISABLE_DACS == 1
  { ENC_CONTROL0_REG, ENC_CTL0_DAC_DISABLE_MASK, ENC_CTL0_DAC_DISABLE_ENABLE },
#endif
   { ENC_CONTROL0_REG, ENC_CTL0_EACTIVE_MASK, ENC_CTL0_EACTIVE_ENABLE_NORMAL }
};
static const u_int16 uTVENC_861LIKE_PAL_BDGHI_Ops = sizeof(TVENC_861LIKE_PAL_BDGHI_Setup)/sizeof(TVENC_REG_T);


/* registers needed to set up SECAM standard for internal encode type INTERNAL_BT861_INTERNAL */ 
/* the first element of each array is register address */
#if (0)
static const TVENC_REG_T TVENC_861LIKE_SECAM_Setup[] =
{
   { ENC_CONTROL0_REG, ENC_CTL0_OUTMODE_MASK, INTERNAL_ENCODER_OUTMODE },
   { ENC_LINE_TIME_REG, ENC_LINE_TIME_HACTIVE_MASK, 0x2cf << ENC_LINE_TIME_HACTIVE_SHIFT },
   { ENC_LINE_TIME_REG, ENC_LINE_TIME_HCLK_MASK, 0x6c0 << ENC_LINE_TIME_HCLK_SHIFT },
   { ENC_HORIZONTAL_TIMING_REG, ENC_HORZ_TIMING_HBURST_END_MASK, 0x54 << ENC_HORZ_TIMING_HBURST_END_SHIFT },
   { ENC_HORIZONTAL_TIMING_REG, ENC_HORZ_TIMING_HBURST_BEG_MASK, 0x98 << ENC_HORZ_TIMING_HBURST_BEG_SHIFT },
   { ENC_HORIZONTAL_TIMING_REG, ENC_HORZ_TIMING_AHSYNCWIDTH_MASK, 0x7e << ENC_HORZ_TIMING_AHSYNCWIDTH_SHIFT },
   { ENC_HBLANK_REG, ENC_HBLANK_VACTIVE_MASK, 0x120 << ENC_HBLANK_VACTIVE_SHIFT },
   { ENC_HBLANK_REG, ENC_HBLANK_HBLANK_MASK, 0x106 << ENC_HBLANK_HBLANK_SHIFT },
   { ENC_HSYNC_REG, ENC_HSYNC_VBLANK_MASK, 0x17 << ENC_HSYNC_VBLANK_SHIFT },
   { ENC_CONTROL0_REG, ENC_CTL0_PHASE_ALTERATION_MASK, ENC_CTL0_PHASE_ALTERATION_ENABLE },
   { ENC_CONTROL0_REG, ENC_CTL0_SETUP_MASK, ENC_CTL0_SETUP_DISABLE },
   { ENC_CONTROL0_REG, ENC_CTL0_LINE625_MASK, ENC_CTL0_LINE625_625_FORMAT },
   { ENC_CONTROL0_REG, ENC_CTL0_VSYNC_DURATION_MASK, ENC_CTL0_VSYNC_2_5_LINES },
   { ENC_AMPLITUDE_REG, ENC_AMP_BURST_AMP_MASK, 0x5e << ENC_AMP_BURST_AMP_SHIFT },
   { ENC_AMPLITUDE_REG, ENC_AMP_SYNC_AMP_MASK, 0xf0 << ENC_AMP_SYNC_AMP_SHIFT },
   { ENC_YCRCB_MULTIPLICATION_REG, ENC_YCRCB_MULT_MCR_MASK, 0xcf << ENC_YCRCB_MULT_MCR_SHIFT },
   { ENC_YCRCB_MULTIPLICATION_REG, ENC_YCRCB_MULT_MCB_MASK, 0x93 << ENC_YCRCB_MULT_MCB_SHIFT },
   { ENC_YCRCB_MULTIPLICATION_REG, ENC_YCRCB_MULT_MY_MASK, 0xa3 << ENC_YCRCB_MULT_MY_SHIFT },
   { ENC_SECAM_DR_REG, ENC_SECAM_DR_MASK, 0x2a098acb << ENC_SECAM_DR_SHIFT },
   { ENC_CONTROL0_REG, ENC_CTL0_EACTIVE_MASK, ENC_CTL0_EACTIVE_ENABLE_NORMAL }
};
#else
/* to configure the TV Encoder registers to SECAM video standard. */
static const TVENC_REG_T      TVENC_861LIKE_SECAM_Setup[] =
{
   /* DAC Output Format Control */
   { ENC_CONTROL0_REG,             ENC_CTL0_OUTMODE_MASK,            INTERNAL_ENCODER_OUTMODE },
   /* HACTIVE: 2D0   /   HCLK: 6C0 */
   { ENC_LINE_TIME_REG,            ENC_LINE_TIME_HACTIVE_MASK,       0x2D0 << ENC_LINE_TIME_HACTIVE_SHIFT },
   { ENC_LINE_TIME_REG,            ENC_LINE_TIME_HCLK_MASK,          0x6C0 << ENC_LINE_TIME_HCLK_SHIFT },
   /* AHSYNCWIDTH: 7F   /   HBURSTBEG: 97   /   HBURSTEND: 54 */
   { ENC_HORIZONTAL_TIMING_REG,    ENC_HORZ_TIMING_HBURST_END_MASK,  0x54 << ENC_HORZ_TIMING_HBURST_END_SHIFT },
   { ENC_HORIZONTAL_TIMING_REG,    ENC_HORZ_TIMING_HBURST_BEG_MASK,  0x97 << ENC_HORZ_TIMING_HBURST_BEG_SHIFT },
   { ENC_HORIZONTAL_TIMING_REG,    ENC_HORZ_TIMING_AHSYNCWIDTH_MASK, 0x80 << ENC_HORZ_TIMING_AHSYNCWIDTH_SHIFT },
   /* VACTIVE: 120   /   HBLANK: 114 */
   { ENC_HBLANK_REG,               ENC_HBLANK_VACTIVE_MASK,          0x120 << ENC_HBLANK_VACTIVE_SHIFT },
   { ENC_HBLANK_REG,               ENC_HBLANK_HBLANK_MASK,           0x114 << ENC_HBLANK_HBLANK_SHIFT },
   /* VBLANK: 17 */
   { ENC_HSYNC_REG,                ENC_HSYNC_VBLANK_MASK,            0x17 << ENC_HSYNC_VBLANK_SHIFT },
   /* SECAM Cross Color Filter: Enabled */
   { ENC_CONTROL0_REG,             ENC_CTL0_CROSS_FIL_MASK,          ENC_CTL0_CROSS_FIL_APPLY },
   /* FIELD_ID: 0 */
   { ENC_CONTROL0_REG,             ENC_CTL0_FIELD_ID_MASK,           ENC_CTL0_FIELD_ID_DISABLE },
   /* SC_PATTERN: 0 */
   { ENC_CONTROL0_REG,             ENC_CTL0_SC_PATTERN_MASK,         ENC_CTL0_SC_PATTERN_0_0_180_0_0_180 },
   /* PROG_SC: 0 */
   { ENC_CONTROL0_REG,             ENC_CTL0_SC_PROC_MASK,            ENC_CTL0_SC_PROC_LINES_23_310_AND_336_623 },
   /* Chroma Encoding: FM */
   { ENC_CONTROL0_REG,             ENC_CTL0_FM_MODULATION_MASK,      ENC_CTL0_FM_MODULATION_FM },
   /* Phase Alternation: disabled */
   { ENC_CONTROL0_REG,             ENC_CTL0_PHASE_ALTERATION_MASK,   ENC_CTL0_PHASE_ALTERATION_DISABLE },
   /* 7.5 IRE Setup: disabled */
   { ENC_CONTROL0_REG,             ENC_CTL0_SETUP_MASK,              ENC_CTL0_SETUP_DISABLE },
   /* Lines per Frame: 625 lines */
   { ENC_CONTROL0_REG,             ENC_CTL0_LINE625_MASK,            ENC_CTL0_LINE625_625_FORMAT },
   /* VSYNC_DUR: 2.5 lines */
   { ENC_CONTROL0_REG,             ENC_CTL0_VSYNC_DURATION_MASK,     ENC_CTL0_VSYNC_2_5_LINES },
   /* SC_AMP: 86   /   SYNC_AMP: F0   /   BURST_AMP: ignore */
   { ENC_AMPLITUDE_REG,            ENC_AMP_SC_AMP_MASK,              0x86 << ENC_AMP_SC_AMP_SHIFT },
   { ENC_AMPLITUDE_REG,            ENC_AMP_SYNC_AMP_MASK,            0xF0 << ENC_AMP_SYNC_AMP_SHIFT },
   /* YCrCb Multiplication Factor: Y:A4 / CB:94 / CR: B5 */
   { ENC_YCRCB_MULTIPLICATION_REG, ENC_YCRCB_MULT_MCR_MASK,          0xB5 << ENC_YCRCB_MULT_MCR_SHIFT },
   { ENC_YCRCB_MULTIPLICATION_REG, ENC_YCRCB_MULT_MCB_MASK,          0x94 << ENC_YCRCB_MULT_MCB_SHIFT },
   { ENC_YCRCB_MULTIPLICATION_REG, ENC_YCRCB_MULT_MY_MASK,           0xA4 << ENC_YCRCB_MULT_MY_SHIFT },
   /* Dr for SECAM: 0x29C71C72   /   Db for SECAM: 0x284BDA13 */
   { ENC_SECAM_DR_REG,             ENC_SECAM_DR_MASK,                0x29C83A70 << ENC_SECAM_DR_SHIFT },
   { ENC_SECAM_DB_REG,             ENC_SECAM_DB_MASK,                0x284CEDED << ENC_SECAM_DB_SHIFT },
   /* DR_MAX: 5A3   /   DR_MIN: 49F */
   { ENC_DR_RANGE_REG,             ENC_DR_RANGE_MAX_MASK,            0x5A3 << ENC_DR_RANGE_MAX_SHIFT },
   { ENC_DR_RANGE_REG,             ENC_DR_RANGE_MIN_MASK,            0x49F << ENC_DR_RANGE_MIN_SHIFT },
   /* DB_MAX: 5A3   /   DB_MIN: 49F */
   { ENC_DB_RANGE_REG,             ENC_DB_RANGE_MAX_MASK,            0x5A3 << ENC_DB_RANGE_MAX_SHIFT },
   { ENC_DB_RANGE_REG,             ENC_DB_RANGE_MIN_MASK,            0x49F << ENC_DB_RANGE_MIN_SHIFT },
   { ENC_CONTROL0_REG,             ENC_CTL0_EACTIVE_MASK,            ENC_CTL0_EACTIVE_ENABLE_NORMAL }
};
#endif
static const u_int16 uTVENC_861LIKE_SECAM_Ops = sizeof(TVENC_861LIKE_SECAM_Setup)/sizeof(TVENC_REG_T);


/* See bottom of the file encoder.c for program to generate the following tables */
#if USE_MATH != 1
static unsigned char uU_V_table[0xff] = {
127,
127,127,127,127,127,129,129,129,130,130,
130,131,131,132,132,133,134,134,135,136,
137,137,138,139,140,141,142,143,144,145,
146,147,149,150,151,152,154,155,157,158,
159,161,162,164,165,167,169,170,172,174,
175,177,179,181,183,184,186,188,190,192,
194,196,198,200,202,204,206,208,210,212,
214,216,219,221,223,225,227,229,232,234,
236,238,240,243,245,247,249,252,254,0,
1,3,6,8,10,12,15,17,19,21,
23,26,28,30,32,34,36,39,41,43,
45,47,49,51,53,55,57,59,61,63,
65,67,69,71,72,74,76,78,80,81,
83,85,86,88,90,91,93,94,96,97,
98,100,101,103,104,105,106,108,109,110,
111,112,113,114,115,116,117,118,118,119,
120,121,121,122,123,123,124,124,125,125,
125,126,126,126,127,127,127,127,127,127,
127,127,127,127,127,126,126,126,125,125,
125,124,124,123,123,122,121,121,120,119,
119,118,117,116,115,114,113,112,111,110,
109,108,106,105,104,103,101,100,98,97,
96,94,93,91,90,88,86,85,83,81,
80,78,76,74,72,71,69,67,65,63,
61,59,57,55,53,51,49,47,45,43,
41,39,36,34}; /* uU_V[0xff] */

static unsigned char uVU_table[0xff] = {
0,
2,4,7,9,11,13,16,18,20,22,
24,27,29,31,33,35,37,40,42,44,
46,48,50,52,54,56,58,60,62,64,
66,68,70,72,73,75,77,79,81,82,
84,86,87,89,91,92,94,95,97,98,
99,101,102,104,105,106,107,109,110,111,
112,113,114,115,116,117,118,119,119,120,
121,122,122,123,124,124,125,125,126,126,
126,127,127,127,127,127,127,127,127,127,
127,127,127,127,127,127,127,127,126,126,
126,125,125,124,124,123,122,122,121,120,
119,119,118,117,116,115,114,113,112,111,
110,109,107,106,105,104,102,101,99,98,
97,95,94,92,91,89,87,86,84,82,
81,79,77,75,73,72,70,68,66,64,
62,60,58,56,54,52,50,48,46,44,
42,40,37,35,33,31,29,27,24,22,
20,18,16,13,11,9,7,4,2,0,
255,253,250,248,246,244,241,239,237,235,
233,230,228,226,224,222,220,217,215,213,
211,209,207,205,203,201,199,197,195,193,
191,189,187,185,184,182,180,178,176,175,
173,171,170,168,166,165,163,162,160,159,
158,156,155,153,152,151,150,148,147,146,
145,144,143,142,141,140,139,138,138,137,
136,135,135,134}; /* uVU[0xff] */

static unsigned char uUV_table[0xff] = {
0,
254,252,249,247,245,243,240,238,236,234,
232,229,227,225,223,221,219,216,214,212,
210,208,206,204,202,200,198,196,194,192,
190,188,186,184,183,181,179,177,175,174,
172,170,169,167,165,164,162,161,159,158,
157,155,154,152,151,150,149,147,146,145,
144,143,142,141,140,139,138,137,137,136,
135,134,134,133,132,132,131,131,130,130,
130,129,129,129,127,127,127,127,127,127,
127,127,127,127,127,129,129,129,130,130,
130,131,131,132,132,133,134,134,135,136,
137,137,138,139,140,141,142,143,144,145,
146,147,149,150,151,152,154,155,157,158,
159,161,162,164,165,167,169,170,172,174,
175,177,179,181,183,184,186,188,190,192,
194,196,198,200,202,204,206,208,210,212,
214,216,219,221,223,225,227,229,232,234,
236,238,240,243,245,247,249,252,254,0,
1,3,6,8,10,12,15,17,19,21,
23,26,28,30,32,34,36,39,41,43,
45,47,49,51,53,55,57,59,61,63,
65,67,69,71,72,74,76,78,80,81,
83,85,86,88,90,91,93,94,96,97,
98,100,101,103,104,105,106,108,109,110,
111,112,113,114,115,116,117,118,118,119,
120,121,121,122}; /* uUV[0xff] */

#endif 


/********************************************************************/
/*  FUNCTION:    bt861_internal_set_picture_control                 */
/*                                                                  */
/*  PARAMETERS:  pUnitInst: Pointer to the unit instance            */ 
/*               Control - a member of the CNXT_TVENC_CONTROL enum  */
/*                         specifying whicch picture controls to set*/
/*               Value - Specifies the new value to set for the     */
/*                       control specified. Range from -127 to 128  */
/*                                                                  */
/*  DESCRIPTION: set the specified control value                    */
/*                                                                  */
/*  RETURNS:     CNXT_TVENC_OK                                      */
/*               CNXT_TVENC_NOT_AVAILABLE                           */
/*                                                                  */
/*  CONTEXT:    Must be called in task context.                     */
/*                                                                  */
/********************************************************************/
static CNXT_TVENC_STATUS bt861_internal_set_picture_control(CNXT_TVENC_UNIT_INST *pUnitInst, CNXT_TVENC_CONTROL Control, int32 Value )
{            
   u_int32 uValue_fil, uValue_pkfil;

   switch (Control)
   {
      /* Set brightness (if requested) */
      case CNXT_TVENC_BRIGHTNESS:
         pUnitInst->picctrl_brightness = Value;
         /* Brightness is odd - it ranges from 0x10 (min) to 0x0F (max) */
         /* with 0 as nominal.                                          */
         CNXT_SET_VAL(ENC_HUE_BRIGHT_CTL_REG, ENC_HUEBRICTL_YOFF_MASK, (u_int8)Value);
         break;
         
      /* Set contrast (if requested).  Rescale range from [-128,127] to [0,255] */
      case CNXT_TVENC_CONTRAST:
         pUnitInst->picctrl_contrast = Value;
         Value += 128;
         CNXT_SET_VAL(ENC_YCRCB_MULTIPLICATION_REG, ENC_YCRCB_MULT_MY_MASK, (u_int8)Value);
         break;
    
      case CNXT_TVENC_SATURATION:
         pUnitInst->picctrl_saturation = Value;
         Value += 128;
         CNXT_SET_VAL(ENC_YCRCB_MULTIPLICATION_REG, ENC_YCRCB_MULT_MCR_MASK, (u_int8)Value);
         CNXT_SET_VAL(ENC_YCRCB_MULTIPLICATION_REG, ENC_YCRCB_MULT_MCB_MASK, (u_int8)Value);
        break;

      case CNXT_TVENC_HUE:
         /* Method 1: Adjust the subcarrier phase, effective for composite and svideo signals     */
         /* Method 2: uses MULT_UU,MULT_VU, MULT_UV, MULT_VV in register Chroma control to        */
         /*           matrix multiply the color vectors, effective for RGB signal in non-PAL mode */ 
         Value += 128;
         if( (pUnitInst->uOutputConnection & VIDEO_OUTPUT_CVBS) \
           ||(pUnitInst->uOutputConnection & VIDEO_OUTPUT_YC) )
         {
            /* set the registers used by other method to their default values  */
            CNXT_SET_VAL(ENC_CHROMA_CTL_REG, 0xffffffff, 0x7f00007f);
            CNXT_SET_VAL(ENC_HUE_BRIGHT_CTL_REG, ENC_HUEBRICTL_HUEADJ_MASK, (Value&0xFF));
            pUnitInst->picctrl_hue = Value - 128;      /* store the set value */
         }
         else    /* RGB signals */
         {
            if( (pUnitInst->VideoStandard == CNXT_TVENC_NTSC_M) \
              ||(pUnitInst->VideoStandard == CNXT_TVENC_NTSC_JAPAN) \
              ||(pUnitInst->VideoStandard == CNXT_TVENC_SECAM_L) \
              ||(pUnitInst->VideoStandard == CNXT_TVENC_SECAM_D) ) 
            {
        #if USE_MATH==1
               double dCos, dSin;
               const double dDegToRad = 0.02; /* 0.017453 */
        #endif
               u_int8 uVU, uUV, uU_V;
      
               /* set the registers used by other method to their default values  */
               CNXT_SET_VAL(ENC_HUE_BRIGHT_CTL_REG, ENC_HUEBRICTL_HUEADJ_MASK, 0);

        #if USE_MATH==1
               dCos = 128*cos((double)Value*dDegToRad);
               dSin = 128*sin((double)Value*dDegToRad);
        
               uU_V = ~((int8)(dCos+0.5)) + 1 ;  if (uU_V == 128) uU_V = 127;
               uVU = ~(-(int8)(dSin+0.5)) + 1; if (uVU == 128) uVU = 127;
               uUV = ~((int8)(dSin+0.5)) + 1; if (uUV == 128) uUV = 127;
        #else
               Value &= 0xff;
               uU_V = uU_V_table[Value];
               uVU = uVU_table[Value];
               uUV = uUV_table[Value];
        #endif

               CNXT_SET_VAL(ENC_CHROMA_CTL_REG, ENC_CHROMA_CTL_MULT_UU_MASK, uU_V);
               CNXT_SET_VAL(ENC_CHROMA_CTL_REG, ENC_CHROMA_CTL_MULT_VV_MASK, uU_V);
               CNXT_SET_VAL(ENC_CHROMA_CTL_REG, ENC_CHROMA_CTL_MULT_VU_MASK, uVU);
               CNXT_SET_VAL(ENC_CHROMA_CTL_REG, ENC_CHROMA_CTL_MULT_UV_MASK, uUV);
               pUnitInst->picctrl_hue = Value - 128;    /* store the set value */
            }
            else 
            {
               trace_new(TRACE_TVENC, "TVENC: Cannot adjust hue in RGB signal for PAL mode!");
               return CNXT_TVENC_NOT_AVAILABLE;
            }
         }
         break;

      case CNXT_TVENC_SHARPNESS:
         pUnitInst->picctrl_sharpness = Value;
         /* if the input value is positive, set fil_sel to 0 to enable peaking filters */
         if( Value >= 0 )
         {
            uValue_fil = 0;
         }
         /* otherwise, set fil_sel to 1 to enable reduction filters */
         else
         {
            Value += 128;
            uValue_fil = 1;
         }
         /* select one of the four filters depending on the input value */
         uValue_pkfil = ( Value == 128 ) ? 3 : (Value >> 5);
         /* set the pkfil_sel field */
         /* write back the fil_sel bit and the pkfil_sel field */
         CNXT_SET_VAL(ENC_CONTROL0_REG, ENC_CTL0_FIL_SEL_MASK, uValue_fil);
         CNXT_SET_VAL(ENC_CONTROL0_REG, ENC_CTL0_PKFIL_SEL_MASK, uValue_pkfil);        
         break;

      default:
        trace_new(TRACE_TVENC, "TVENC: Specified picture control is not available!");
        return CNXT_TVENC_NOT_AVAILABLE;
   }
  
   return CNXT_TVENC_OK;
}

/********************************************************************/
/*  FUNCTION:    bt861_internal_get_picture_control                 */
/*                                                                  */
/*  PARAMETERS:  pUnitInst: Pointer to the unit instance            */ 
/*               Control - a member of the CNXT_TVENC_CONTROL enum  */
/*                         specifying whicch picture controls to get*/
/*               pValue - Pointer to hold the returned value set    */
/*                        for the control specified.                */
/*                        Range from -127 to 128                    */
/*                                                                  */
/*  DESCRIPTION: Get the specified control value                    */
/*                                                                  */
/*  RETURNS:     CNXT_TVENC_OK                                      */
/*               CNXT_TVENC_NOT_AVAILABLE                           */
/*                                                                  */
/*  CONTEXT:    May be called from any context.                     */
/*                                                                  */
/********************************************************************/
static CNXT_TVENC_STATUS bt861_internal_get_picture_control(CNXT_TVENC_UNIT_INST *pUnitInst, CNXT_TVENC_CONTROL Control, int32 *pValue )
{
   switch (Control)
   {
      /* Set brightness (if requested) */
      case CNXT_TVENC_BRIGHTNESS:
         *pValue = pUnitInst->picctrl_brightness;
         break;
      
      /* Get contrast from the device (if supported) */
      case CNXT_TVENC_CONTRAST:
         *pValue = pUnitInst->picctrl_contrast;
         break;

      case CNXT_TVENC_SATURATION:
         *pValue = pUnitInst->picctrl_saturation;
         break;     

      /* Get hue from the device (if supported) */
      case CNXT_TVENC_HUE:
         *pValue = pUnitInst->picctrl_hue;
         break;

      case CNXT_TVENC_SHARPNESS:
         *pValue = pUnitInst->picctrl_sharpness;
         break;

      default:
         trace_new(TRACE_TVENC, "TVENC: Specified picture control is not available!");
         return CNXT_TVENC_NOT_AVAILABLE;
   }
   
   return CNXT_TVENC_OK;
}

/********************************************************************/
/*  FUNCTION:    bt861_internal_set_connection                      */
/*                                                                  */
/*  PARAMETERS:  pUnitInst: Pointer to the unit instance            */ 
/*               uConnection - Video output connection type.        */
/*                  It should be a logic OR of the following:       */
/*                      VIDEO_OUTPUT_CVBS                           */
/*                      VIDEO_OUTPUT_RGB                            */
/*                      VIDEO_OUTPUT_YC                             */
/*                                                                  */                                                                
/*  DESCRIPTION: This function sets the video output connection.    */
/*                                                                  */
/*  RETURNS:     CNXT_TVENC_OK                                      */
/*               CNXT_TVENC_NOT_AVAILABLE                           */
/*                                                                  */
/*  CONTEXT:     Must be called in task context.                    */
/*                                                                  */
/********************************************************************/
static CNXT_TVENC_STATUS bt861_internal_set_connection(CNXT_TVENC_UNIT_INST *pUnitInst, u_int8 uConnection)
{
   BYTE cEnableDacs = 0;

   if( uConnection == 0 )
   {
      /* Turn off all the DACs */
      CNXT_SET_VAL(ENC_CONTROL0_REG, ENC_CTL0_ALL_DAC_MASK, cEnableDacs);
   }

   /*****************************************************************/
   /* The encoder supports simultaneous RGB, SVIDEO and CVBS output */
   /*****************************************************************/
   if( uConnection & VIDEO_OUTPUT_CVBS )
   {
      #ifdef OPENTV_12
      /* For OpenTV 1.2, we set SVideo in tandem with CVBS */
      cEnableDacs |= (VIDEO_ENCODER_CVBS_DAC_MASK | VIDEO_ENCODER_YC_DAC_MASK);
      #else
      /* Minor hack - enable both composite and RGB if asked for Composite only */
      /* Pending a fix to ensure SCART signalling is implemented in the OpenTV  */
      /* EN2 driver for Klondike/Bronco.                                        */
      cEnableDacs |= VIDEO_ENCODER_CVBS_DAC_MASK ;
      #endif
   }

   if( uConnection & VIDEO_OUTPUT_YC )
   {
      cEnableDacs |= VIDEO_ENCODER_YC_DAC_MASK;
   }

   if( uConnection & VIDEO_OUTPUT_RGB )
   {
      /* Minor hack - enable both composite and RGB if asked for RGB only        */
      /* This prevents us from losing video on SCART TVs that don't support RGB  */
      /* when running some OpenTV VTS tests.                                     */
      cEnableDacs |= (VIDEO_ENCODER_CVBS_DAC_MASK | VIDEO_ENCODER_RGB_DAC_MASK);
   }

   /* If we were asked to set any mode, set up the encoder accordingly */
   if( uConnection )
   {
      /* Set the chrominance filter to wide mode if RGB is in use, else normal mode */
      CNXT_SET_VAL(ENC_CONTROL0_REG, ENC_CTL0_CHROMA_BW_MASK, ((uConnection & VIDEO_OUTPUT_RGB)?1:0));
     
      /* Turn on the relevant subset of DACs */
      CNXT_SET_VAL(ENC_CONTROL0_REG, ENC_CTL0_ALL_DAC_MASK, cEnableDacs);
   }

   pUnitInst->uOutputConnection = uConnection;

   return(CNXT_TVENC_OK);

}

/********************************************************************/
/*  FUNCTION:    bt861_internal_get_connection                      */
/*                                                                  */
/*  PARAMETERS:  pUnitInst: Pointer to the unit instance            */ 
/*               puConnection - Pointer to hold the video output    */
/*                              connection type.                    */
/*                  It should be a logic OR of the following:       */
/*                      VIDEO_OUTPUT_CVBS                           */
/*                      VIDEO_OUTPUT_RGB                            */
/*                      VIDEO_OUTPUT_YC                             */
/*                                                                  */                                                                
/*  DESCRIPTION: This function sets the video output connection.    */
/*                                                                  */
/*  RETURNS:     CNXT_TVENC_OK                                      */
/*               CNXT_TVENC_NOT_AVAILABLE                           */
/*                                                                  */
/*  CONTEXT:     May be called from any context.                    */
/*                                                                  */
/********************************************************************/
static CNXT_TVENC_STATUS bt861_internal_get_connection(CNXT_TVENC_UNIT_INST *pUnitInst, u_int8 *puConnection)
{
   *puConnection = pUnitInst->uOutputConnection;
   return CNXT_TVENC_OK;
}

/********************************************************************/
/*  FUNCTION:    bt861_internal_set_position                        */
/*                                                                  */
/*  PARAMETERS:  pUnitInst: Pointer to the unit instance            */ 
/*               Xoffset - Horizontal offset from default position  */
/*               Yoffset - Vertical offset from default position    */  
/*                                                                  */
/*  DESCRIPTION: Modify the screen start position by by a small     */
/*               amount in the horizontal and vertical directions   */
/*                                                                  */
/*  RETURNS:     CNXT_TVENC_OK                                      */
/*                                                                  */
/*  CONTEXT:    Must be called in task context.                     */
/*                                                                  */
/********************************************************************/
static CNXT_TVENC_STATUS bt861_internal_set_position_offset(CNXT_TVENC_UNIT_INST *pUnitInst, int8 XOffset, int8 YOffset)
{
   int XClkOffset;
   bool bks;
   POSDLIBREGION pRgn;
   int HBlankEnd, VBlankEnd;

   /* Set the video standard */
   /* Each pixel is represented by 2 clocks horizontally and we */
   /* must in 2 pixel increments to avoid colour shifts.        */

   XClkOffset = (XOffset&~1) * 2;

   /* Work out the new offset */
   gnHBlank = pUnitInst->uHBlank = pUnitInst->uHBlankInitial + (u_int32)XClkOffset;
   gnVBlank = pUnitInst->uVBlank = pUnitInst->uVBlankInitial + (u_int32)YOffset;
   HBlankEnd = pUnitInst->uHBlank + (2*OSD_MAX_WIDTH)  - 1;
   VBlankEnd = pUnitInst->uVBlank + (gnOsdMaxHeight/2) - 1;

   /* Restrict the overal size to be within the real size of video. */
   switch( pUnitInst->VideoStandard )
   {
      case CNXT_TVENC_NTSC_M:
      case CNXT_TVENC_NTSC_JAPAN:
         if ( HBlankEnd > 1712 )
         {
            HBlankEnd = 1712;
         }
         if ( VBlankEnd > 262 )
         {
            VBlankEnd = 262;
         }
         break;

      case CNXT_TVENC_PAL_B_ITALY:
      case CNXT_TVENC_PAL_B_WEUR:
      case CNXT_TVENC_PAL_B_AUS:
      case CNXT_TVENC_PAL_B_NZ:
      case CNXT_TVENC_PAL_I:
      case CNXT_TVENC_SECAM_L:
      case CNXT_TVENC_SECAM_D:
         if ( HBlankEnd > 1720 )
         {
            HBlankEnd = 1720;
         }
         if ( VBlankEnd > 310 )
         {
            VBlankEnd = 310;
         }
         break;
      default:
         trace_new(TRACE_TVENC, "private_set_display_position: Invalid video standard %d !", pUnitInst->VideoStandard);
         error_log(ERROR_WARNING);
         return CNXT_TVENC_INTERNAL_ERROR;
   }     
   
   /* Apply the new offset to the screen start and end registers*/
   bks = critical_section_begin();

   CNXT_SET_VAL(glpDrmScreenStart, DRM_SCREEN_START_FIRST_ACTIVE_PIXEL_MASK, pUnitInst->uHBlank);
   CNXT_SET_VAL(glpDrmScreenStart, DRM_SCREEN_START_FIELD2_ACTIVE_LINE_MASK, pUnitInst->uVBlank);
   CNXT_SET_VAL(glpDrmScreenStart, DRM_SCREEN_START_FIELD1_ACTIVE_LINE_MASK, pUnitInst->uVBlank);
   CNXT_SET_VAL(glpDrmScreenEnd, DRM_SCREEN_END_LAST_ACTIVE_PIXEL_MASK, HBlankEnd);
   CNXT_SET_VAL(glpDrmScreenEnd, DRM_SCREEN_END_FIELD2_LAST_LINE_MASK, VBlankEnd);
   CNXT_SET_VAL(glpDrmScreenEnd, DRM_SCREEN_END_FIELD1_LAST_LINE_MASK, VBlankEnd);

   critical_section_end(bks);

   /* Update the positions of all active OSD regions */
   pRgn = gpOsdLibList[0];

   while(pRgn != NULL)
   {
     UpdateOSDHeader((OSDHANDLE)pRgn);
     pRgn = pRgn->pNext;
   }

   pRgn = gpOsdLibList[1];

   while(pRgn != NULL)
   {
      UpdateOSDHeader((OSDHANDLE)pRgn);
      pRgn = pRgn->pNext;
   }

   /* Update video region positions */
   vidUpdateScaling(0);
   vidUpdateScaling(1);

   /* store the set X and Y offsets */
   pUnitInst->XOffset = XOffset;
   pUnitInst->YOffset = YOffset;

   return CNXT_TVENC_OK;
}

/********************************************************************/
/*  FUNCTION:    bt861_internal_get_position                        */
/*                                                                  */
/*  PARAMETERS:  pUnitInst: Pointer to the unit instance            */ 
/*               pXStart - pointer to the horizontal screen start   */
/*                         position                                 */
/*               pYStart - pointer to the vertical screen start     */
/*                         position                                 */  
/*                                                                  */
/*  DESCRIPTION: Query the current X and Y start positions values   */
/*               for the display.                                   */
/*                                                                  */
/*  RETURNS:     CNXT_TVENC_OK                                      */
/*                                                                  */
/*  CONTEXT:    May be called from any context.                     */
/*                                                                  */
/********************************************************************/
static CNXT_TVENC_STATUS bt861_internal_get_position_offset(CNXT_TVENC_UNIT_INST *pUnitInst, int8 *pXOffset, int8 *pYOffset)
{
   *pXOffset = pUnitInst->XOffset;
   *pYOffset = pUnitInst->YOffset;

   return(CNXT_TVENC_OK);
}

/********************************************************************/
/*  FUNCTION:    bt861_internal_set_standard                        */
/*                                                                  */
/*  PARAMETERS:  uUnitNumber: The unit(controller) number n         */ 
/*               Standard - output video format.                    */
/*                           One of the following:                  */
/*                              CNXT_TVENC_NTSC_M,                  */
/*                              CNXT_TVENC_NTSC_JAPAN,              */   
/*                              CNXT_TVENC_PAL_B_ITALY,             */
/*                              CNXT_TVENC_PAL_B_WEUR,              */ 
/*                              CNXT_TVENC_PAL_B_AUS,               */ 
/*                              CNXT_TVENC_PAL_B_NZ,                */ 
/*                              CNXT_TVENC_PAL_I,                   */ 
/*                              CNXT_TVENC_SECAM_L,                 */
/*                              CNXT_TVENC_SECAM_D                  */ 
/*                                                                  */
/*  DESCRIPTION: Set up the internal encoder for wabash and brazos  */
/*               to drive NTSC or PAL-format video.                 */
/*                                                                  */
/*  RETURNS:     CNXT_TVENC_OK                                      */
/*               CNXT_TVENC_BAD_PARAMETER                           */
/*                                                                  */
/*  CONTEXT:     Must be called in non-interrupt context.           */
/*                                                                  */
/********************************************************************/
static CNXT_TVENC_STATUS bt861_internal_set_standard(CNXT_TVENC_UNIT_INST *pUnitInst, CNXT_TVENC_VIDEO_STANDARD Standard)
{
   const TVENC_REG_T *pOpList;
   u_int16 uOpCount;
   u_int16 uLoop;

   /* Set up for initialisation depending upon the standard required */
   switch(Standard)
   {
      case CNXT_TVENC_NTSC_M:
         gnHBlank = pUnitInst->uHBlank = BT861_NTSC_HBLANK;
         gnVBlank = pUnitInst->uVBlank = BT861_NTSC_VBLANK;
         pUnitInst->iHSync = pUnitInst->iHSyncInitial = BT861_NTSC_HSYNC;
         uOpCount = uTVENC_861LIKE_NTSC_M_Ops;
         pOpList  = TVENC_861LIKE_NTSC_M_Setup;
			#if DIGITAL_DISPLAY == PW113_DISPLAY
			cnxt_pw113_display_set_screen_standard(PW113_SCREEN_STANDARD_NTSC);
			#endif
         break;

      case CNXT_TVENC_NTSC_JAPAN:
         gnHBlank = pUnitInst->uHBlank = BT861_NTSC_HBLANK;
         gnVBlank = pUnitInst->uVBlank = BT861_NTSC_VBLANK;
         pUnitInst->iHSync = pUnitInst->iHSyncInitial = BT861_NTSC_HSYNC;
         uOpCount = uTVENC_861LIKE_NTSC_J_Ops;
         pOpList  = TVENC_861LIKE_NTSC_J_Setup;
			#if DIGITAL_DISPLAY == PW113_DISPLAY
			cnxt_pw113_display_set_screen_standard(PW113_SCREEN_STANDARD_NTSC);
			#endif
         break;

      case CNXT_TVENC_PAL_B_ITALY:
      case CNXT_TVENC_PAL_B_WEUR:
      case CNXT_TVENC_PAL_B_AUS:
      case CNXT_TVENC_PAL_B_NZ:
      case CNXT_TVENC_PAL_I:
         gnHBlank = pUnitInst->uHBlank = BT861_PAL_HBLANK;                                   
         gnVBlank = pUnitInst->uVBlank = BT861_PAL_VBLANK; 
         pUnitInst->iHSync = pUnitInst->iHSyncInitial = BT861_PAL_HSYNC;
         uOpCount = uTVENC_861LIKE_PAL_BDGHI_Ops;
         pOpList  = TVENC_861LIKE_PAL_BDGHI_Setup;
			#if DIGITAL_DISPLAY == PW113_DISPLAY
			cnxt_pw113_display_set_screen_standard(PW113_SCREEN_STANDARD_PAL);
			#endif
         break;

      case CNXT_TVENC_SECAM_L:
      case CNXT_TVENC_SECAM_D:
         gnHBlank = pUnitInst->uHBlank = BT861_PAL_HBLANK - 2;
         gnVBlank = pUnitInst->uVBlank = BT861_PAL_VBLANK;
         pUnitInst->iHSync = pUnitInst->iHSyncInitial = BT861_PAL_HSYNC;
         uOpCount = uTVENC_861LIKE_SECAM_Ops;
         pOpList  = TVENC_861LIKE_SECAM_Setup;
			#if DIGITAL_DISPLAY == PW113_DISPLAY
			cnxt_pw113_display_set_screen_standard(PW113_SCREEN_STANDARD_SECAM);
			#endif
         break;

      default:
         trace_new(TRACE_TVENC, "set_internal_video_standard: Invalid video standard %d passed!", Standard);
         error_log(ERROR_WARNING);
         return CNXT_TVENC_BAD_PARAMETER;
   }

   /* Write the appropriate values to the encoder registers */
   for(uLoop = 0; uLoop < uOpCount; uLoop++)
   {
      *(LPREG)pOpList[uLoop].uReg = (*(LPREG)pOpList[uLoop].uReg & ~pOpList[uLoop].uMask) | pOpList[uLoop].uValue;
   }

   return(CNXT_TVENC_OK);
}

/********************************************************************/
/*  FUNCTION:    bt861_internal_get_standard                        */
/*                                                                  */
/*  PARAMETERS:  pUnitInst: Pointer to the unit instance            */ 
/*               pStandard - pointer to a structure to receive      */
/*                           output video format.                   */
/*               Return values should be one of the following:      */
/*                              CNXT_TVENC_NTSC_M,                  */
/*                              CNXT_TVENC_NTSC_JAPAN,              */   
/*                              CNXT_TVENC_PAL_B_ITALY,             */
/*                              CNXT_TVENC_PAL_B_WEUR,              */ 
/*                              CNXT_TVENC_PAL_B_AUS,               */ 
/*                              CNXT_TVENC_PAL_B_NZ,                */ 
/*                              CNXT_TVENC_PAL_I,                   */ 
/*                              CNXT_TVENC_SECAM_L,                 */
/*                              CNXT_TVENC_SECAM_D                  */ 
/*                                                                  */
/*  DESCRIPTION: This function returns the output video standard    */
/*               from the BT861 like video encoder.                 */
/*                                                                  */
/*  RETURNS:     CNXT_TVENC_OK                                      */
/*                                                                  */
/*  CONTEXT:    May be called from any context.                     */
/*                                                                  */
/********************************************************************/
static CNXT_TVENC_STATUS bt861_internal_get_standard(CNXT_TVENC_UNIT_INST *pUnitInst, CNXT_TVENC_VIDEO_STANDARD *pStandard)
{
   *pStandard = pUnitInst->VideoStandard;
   return CNXT_TVENC_OK;
}

/********************************************************************/
/*  FUNCTION:    bt861_internal_video_timing_reset                  */
/*                                                                  */
/*  PARAMETERS:  pUnitInst: Pointer to the unit instance            */ 
/*                                                                  */                                                                
/*  DESCRIPTION: This function resets the video timing.             */
/*                                                                  */
/*  RETURNS:     CNXT_TVENC_OK                                      */
/*                                                                  */
/*  CONTEXT:     Must be called in task context.                    */
/*                                                                  */
/********************************************************************/
static CNXT_TVENC_STATUS bt861_internal_video_timing_reset( CNXT_TVENC_UNIT_INST *pUnitInst )
{
   u_int8 Value = 0x01;

   /* set the video timing reset bit to 1 */
   CNXT_SET_VAL(ENC_RESET_REG, ENC_RESET_TIMING_RESET_MASK, (u_int8)Value);

   return CNXT_TVENC_OK;

}

/********************************************************************/
/*  FUNCTION:    bt861_internal_ttx_set_lines                       */
/*                                                                  */
/*  PARAMETERS:  pUnitInst: Pointer to the unit instance            */ 
/*                                                                  */                                                                
/*  DESCRIPTION: This function sets active and inactive lines       */
/*               in teletext.                                       */
/*                                                                  */
/*  RETURNS:     CNXT_TVENC_OK                                      */
/*                                                                  */
/*  CONTEXT:     Must be called in task context.                    */
/*                                                                  */
/********************************************************************/
static CNXT_TVENC_STATUS bt861_internal_ttx_set_lines( CNXT_TVENC_UNIT_INST *pUnitInst, \
                    u_int32 uField1ActiveLines, u_int32 uField2ActiveLines )
{

   u_int32 uField1StartLine, uField1EndLine, uField2StartLine, uField2EndLine;
   u_int32 uMaxStartLine, uMinEndLine, uField1ComLines, uField2ComLines;
   u_int32 uDisableLines;
   u_int8 uDisableBits;

   uField1StartLine = RMO(uField1ActiveLines) + 1;
   uField1EndLine = LMO(uField1ActiveLines) + 1;

   uField2StartLine = RMO(uField2ActiveLines) + 1;
   uField2EndLine = LMO(uField2ActiveLines) + 1;

   /* the start lines can't be less than 7 and the end lines can't be more than 22 */
   if ( (uField1StartLine < 7) || (uField2StartLine < 7) \
      ||(uField1EndLine > 23) || (uField2EndLine > 23) )
   {
      return CNXT_TVENC_BAD_PARAMETER;
   }

   /* check if the start line is more than the end line */
   if ( (uField1StartLine > uField1EndLine) || (uField2StartLine > uField2EndLine) )
   {
      return CNXT_TVENC_BAD_PARAMETER;
   }

   /* If a line in field 1 is disabled the same corresponding line in field 2 has to be disabled as well */
   /* which means between the max start line of field 1 and field 2 and the min end line of field 1 and */
   /* field 2, the bit map should be the same */
   uMaxStartLine = max(uField1StartLine, uField2StartLine);
   uMinEndLine = min(uField1EndLine, uField2EndLine);

   uField1ComLines = getbits(uField1ActiveLines, uMinEndLine-1, uMinEndLine-uMaxStartLine+1);
   uField2ComLines = getbits(uField2ActiveLines, uMinEndLine-1, uMinEndLine-uMaxStartLine+1);
   if( uField1ComLines ^ uField2ComLines )
   {
      return CNXT_TVENC_BAD_PARAMETER;
   }

   /* store the start lines and end lines for field 1 and field 2 */
   pUnitInst->uTTXBF1 = uField1StartLine;
   pUnitInst->uTTXEF1 = uField1EndLine;
   pUnitInst->uTTXBF2 = uField2StartLine;
   pUnitInst->uTTXEF2 = uField2EndLine;

   /* Since I2C writes every field crater the system, hard code the enables.     */
   /* set the start and end lines for field 1 and field 2 in the DRM registers */
   CNXT_SET_VAL(glpDrmTTField1, DRM_FIELD1_TELETEXT_FIRST_LINE_MASK, pUnitInst->uTTXBF1+1);
   CNXT_SET_VAL(glpDrmTTField1, DRM_FIELD1_TELETEXT_LAST_LINE_MASK, pUnitInst->uTTXEF1+1);
   CNXT_SET_VAL(glpDrmTTField2, DRM_FIELD1_TELETEXT_FIRST_LINE_MASK, pUnitInst->uTTXBF2);
   CNXT_SET_VAL(glpDrmTTField2, DRM_FIELD1_TELETEXT_LAST_LINE_MASK, pUnitInst->uTTXEF2);

   /* set the start and end lines for field 1 and field 2 in the TV encoder registers */
   CNXT_SET_VAL(ENC_TTX_F1_CTL_REG, ENC_TTXF1_CTL_TTXBF1_MASK, pUnitInst->uTTXBF1);
   CNXT_SET_VAL(ENC_TTX_F1_CTL_REG, ENC_TTXF1_CTL_TTXEF1_MASK, pUnitInst->uTTXEF1);
   CNXT_SET_VAL(ENC_TTX_F2_CTL_REG, ENC_TTXF2_CTL_TTXBF2_MASK, pUnitInst->uTTXBF2);
   CNXT_SET_VAL(ENC_TTX_F2_CTL_REG, ENC_TTXF2_CTL_TTXEF2_MASK, pUnitInst->uTTXEF2);
   
   /* get the lines that teletext is disabled, bit 0 represents 8/321 in f1/f2 */
   uDisableLines = getbits(~uField1ComLines, uMinEndLine-1, uMinEndLine-uMaxStartLine+1);
   uDisableBits = (uDisableLines >> 8) & 0xFFFF; 

   /* set the teletext line disable register */
   CNXT_SET_VAL(ENC_TTX_LINE_CTL_REG, ENC_TTXLINE_CTL_TTX_DIS_MASK, uDisableBits);

   return CNXT_TVENC_OK;

}

/********************************************************************/
/*  FUNCTION:    bt861_internal_ttx_enable                          */
/*                                                                  */
/*  PARAMETERS:  pUnitInst: Pointer to the unit instance            */ 
/*                                                                  */                                                                
/*  DESCRIPTION: This function enables teletext.                    */
/*                                                                  */
/*  RETURNS:     CNXT_TVENC_OK                                      */
/*                                                                  */
/*  CONTEXT:     Must be called in task context.                    */
/*                                                                  */
/********************************************************************/
static CNXT_TVENC_STATUS bt861_internal_ttx_enable( CNXT_TVENC_UNIT_INST *pUnitInst )
{
   u_int32 uFieldActiveLines;
   CNXT_TVENC_STATUS eRetcode;

   /* enable ttx */
   CNXT_SET_VAL(ENC_TTX_LINE_CTL_REG, ENC_TTXLINE_CTL_TXE_MASK, 1);
   /* set TXRM to 1 */
   CNXT_SET_VAL(ENC_TTX_LINE_CTL_REG, ENC_TTXLINE_CTL_TXRM_MASK, 1);

   /* if teletext lines are not set by cnxt_tvenc_ttx_set_lines yet */
   /* set the teletext active lines from 7 to 23 for both fields */
   if( (pUnitInst->uTTXBF1 == 0) && (pUnitInst->uTTXEF1 == 0)   \
    && (pUnitInst->uTTXBF2 == 0) && (pUnitInst->uTTXEF2 == 0) )
   {
      uFieldActiveLines = CNXT_TVENC_VBI_LINE_7 |CNXT_TVENC_VBI_LINE_8 | CNXT_TVENC_VBI_LINE_9 \
        | CNXT_TVENC_VBI_LINE_10 | CNXT_TVENC_VBI_LINE_11 | CNXT_TVENC_VBI_LINE_12 \
        | CNXT_TVENC_VBI_LINE_13 | CNXT_TVENC_VBI_LINE_14 | CNXT_TVENC_VBI_LINE_15 \
        | CNXT_TVENC_VBI_LINE_16 | CNXT_TVENC_VBI_LINE_17 | CNXT_TVENC_VBI_LINE_18 \
        | CNXT_TVENC_VBI_LINE_19 | CNXT_TVENC_VBI_LINE_20 | CNXT_TVENC_VBI_LINE_21 \
        | CNXT_TVENC_VBI_LINE_22;
      
      eRetcode = bt861_internal_ttx_set_lines( pUnitInst, uFieldActiveLines, uFieldActiveLines );
   }

   return CNXT_TVENC_OK;

}

/********************************************************************/
/*  FUNCTION:    bt861_internal_ttx_disable                         */
/*                                                                  */
/*  PARAMETERS:  pUnitInst: Pointer to the unit instance            */ 
/*                                                                  */                                                                
/*  DESCRIPTION: This function disables teletext.                   */
/*                                                                  */
/*  RETURNS:     CNXT_TVENC_OK                                      */
/*                                                                  */
/*  CONTEXT:     Must be called in task context.                    */
/*                                                                  */
/********************************************************************/
static CNXT_TVENC_STATUS bt861_internal_ttx_disable( CNXT_TVENC_UNIT_INST *pUnitInst )
{
   /* disable TTX */
   CNXT_SET_VAL(ENC_TTX_LINE_CTL_REG, ENC_TTXLINE_CTL_TXE_MASK, 0);
   /* set TXRM to 1 */
   CNXT_SET_VAL(ENC_TTX_LINE_CTL_REG, ENC_TTXLINE_CTL_TXRM_MASK, 1);

   return CNXT_TVENC_OK;

}

/********************************************************************/
/*  FUNCTION:    bt861_internal_cc_enable                           */
/*                                                                  */
/*  PARAMETERS:  pUnitInst: Pointer to the unit instance            */ 
/*                                                                  */                                                                
/*  DESCRIPTION: This function enables closed captioning.           */
/*                                                                  */
/*  RETURNS:     CNXT_TVENC_OK                                      */
/*                                                                  */
/*  CONTEXT:     Must be called in task context.                    */
/*                                                                  */
/********************************************************************/
static CNXT_TVENC_STATUS bt861_internal_cc_enable( CNXT_TVENC_UNIT_INST *pUnitInst )
{
   u_int32 uValue;

   uValue = (DEFAULT_BT861_CC_TYPE << ENC_XDS_CC_CTL_ECCGATE_SHIFT ) \
          | (DEFAULT_BT861_CC_LINE << ENC_XDS_CC_CTL_CCSEL_SHIFT);
   CNXT_SET( ENC_XDS_CC_CTL_REG, ~(ENC_XDS_CC_CTL_EWSSF1_MASK|ENC_XDS_CC_CTL_EWSSF2_MASK), uValue );

   return CNXT_TVENC_OK;

}

/********************************************************************/
/*  FUNCTION:    bt861_internal_cc_send_data                        */
/*                                                                  */
/*  PARAMETERS:  pUnitInst: Pointer to the unit instance            */ 
/*               Type - the type of captioning data being sent,     */
/*                   either 0 or 1.                                 */
/*               uByteOne - byte one of the captioning data.        */
/*               uByteTwo - byte two of the captioning data.        */
/*                                                                  */                                                                
/*  DESCRIPTION: This function sends the captioning data to the     */
/*               encoder registers specified by type.               */
/*                                                                  */
/*  RETURNS:     CNXT_TVENC_OK                                      */
/*                                                                  */
/*  CONTEXT:     Must be called in task context.                    */
/*                                                                  */
/********************************************************************/
static CNXT_TVENC_STATUS bt861_internal_cc_send_data( CNXT_TVENC_UNIT_INST *pUnitInst, CNXT_TVENC_CC_TYPE Type, \
                                             u_int8 uByteOne, u_int8 uByteTwo )
{
   u_int32 uValue;

   /* if sent is CC data */
   if ( Type == CNXT_TVENC_CC_SEL )
   {
      uValue = (uByteTwo << ENC_CC_DATA_CCB2_SHIFT) | uByteOne;  
      CNXT_SET_VAL( ENC_CC_DATA_REG, (ENC_CC_DATA_CCB1_MASK|ENC_CC_DATA_CCB2_MASK), uValue );
   }
   /* if sent is XDS data */
   else
   {
      uValue = (uByteTwo << ENC_XDS_DATA_XDSB2_SHIFT) | uByteOne;
      CNXT_SET_VAL( ENC_XDS_DATA_REG, (ENC_XDS_DATA_XDSB1_MASK|ENC_XDS_DATA_XDSB2_MASK), uValue );
   }

   return CNXT_TVENC_OK;
}


/********************************************************************/
/*  FUNCTION:    bt861_internal_cc_disable                          */
/*                                                                  */
/*  PARAMETERS:  pUnitInst: Pointer to the unit instance            */ 
/*                                                                  */                                                                
/*  DESCRIPTION: This function disables closed captioning.          */
/*                                                                  */
/*  RETURNS:     CNXT_TVENC_OK                                      */
/*                                                                  */
/*  CONTEXT:     Must be called in task context.                    */
/*                                                                  */
/********************************************************************/
static CNXT_TVENC_STATUS bt861_internal_cc_disable( CNXT_TVENC_UNIT_INST *pUnitInst )
{
   u_int32 uValue;

   uValue = (0 << ENC_XDS_CC_CTL_ECCGATE_SHIFT ) \
          | (DEFAULT_BT861_CC_LINE << ENC_XDS_CC_CTL_CCSEL_SHIFT);

   CNXT_SET( ENC_XDS_CC_CTL_REG, ~(ENC_XDS_CC_CTL_EWSSF1_MASK|ENC_XDS_CC_CTL_EWSSF2_MASK), uValue );

   return CNXT_TVENC_OK;
}

/********************************************************************/
/*  FUNCTION:    bt861_internal_wss_enable                          */
/*                                                                  */
/*  PARAMETERS:  pUnitInst: Pointer to the unit instance            */ 
/*                                                                  */                                                                
/*  DESCRIPTION: This function enables wide screen signaling.       */
/*                                                                  */
/*  RETURNS:     CNXT_TVENC_OK                                      */
/*                                                                  */
/*  CONTEXT:     Must be called in task context.                    */
/*                                                                  */
/********************************************************************/
static CNXT_TVENC_STATUS bt861_internal_wss_enable( CNXT_TVENC_UNIT_INST *pUnitInst )
{    
   /* set the EWSSF1 bit to 1 */
   CNXT_SET_VAL(ENC_XDS_CC_CTL_REG, ENC_XDS_CC_CTL_EWSSF1_MASK, 1);
   /* write 0 to the WSSDAT register */
   CNXT_SET_VAL(ENC_WS_DATA_REG, ENC_WS_DATA_WSDAT_MASK, 0);

   return CNXT_TVENC_OK;

}


/********************************************************************/
/*  FUNCTION:    bt861_internal_wss_set_config                      */
/*                                                                  */
/*  PARAMETERS:  pUnitInst: Pointer to the unit instance            */ 
/*               pWSS_Settings - pointer to structure containing    */
/*               wide screen signaling configuration settings       */
/*                                                                  */                                                                
/*  DESCRIPTION: This function passes the configuration information */
/*               of the wide screen signaling to TV encoder.        */
/*                                                                  */
/*  RETURNS:     CNXT_TVENC_OK                                      */
/*                                                                  */
/*  CONTEXT:     Must be called in task context.                    */
/*                                                                  */
/********************************************************************/
static CNXT_TVENC_STATUS bt861_internal_wss_set_config( CNXT_TVENC_UNIT_INST *pUnitInst, \
                                                        CNXT_TVENC_WSS_SETTINGS *pWSS_Settings  )
{   
   u_int32 uValue = 0;

   /* set the aspect ratio */
   switch( pWSS_Settings->AspectRatio )
   {
      case FFORMAT_43_NA:
         uValue = 0x00000008;
			#if DIGITAL_DISPLAY == PW113_DISPLAY
			cnxt_pw113_display_set_screen_size(PW113_SCREEN_SIZE_4_3);
			#endif
         break;
      case LBOX_149_CTR:
         uValue = 0x00000001;
         break;
      case LBOX_149_TOP:
         uValue = 0x00000002;
         break;
      case LBOX_169_CTR:
         uValue = 0x0000000B;
         break;
      case LBOX_169_TOP:
         uValue = 0x00000004;
         break;
      case LBOX_G_169_CTR:
         uValue = 0x0000000D;
         break;
      case FFORMAT_149_CTR:
         uValue = 0x0000000E;
         break;
      case FFORMAT_169_NA:
         uValue = 0x00000007;
			#if DIGITAL_DISPLAY == PW113_DISPLAY
			cnxt_pw113_display_set_screen_size(PW113_SCREEN_SIZE_16_9);
			#endif
         break;
      default:
         trace_new(TRACE_TVENC, "bt861_wss_set_config: invalid aspect ratio in settings. \n");
         return CNXT_TVENC_BAD_PARAMETER;
   }

   /* set film bit */
   if( pWSS_Settings->bFilmMode )
   { 
      uValue = uValue | 0x00000010;    /* fild mode is set if true; otherwise, camera mode is set */
   }
   
   /* set color coding bit */
   if( pWSS_Settings->bColorCoding )
   {
      /* motion adaptive colur plus if true; otherwise, standard coding is set */
      uValue = uValue | 0x00000020;     
   }

   /* set helper bit */
   if( pWSS_Settings->bHelper )
   {
      uValue = uValue | 0x00000040;   /* modulated helper if true; otherwise, no helper */
   }

   /* set subtitles within teletext bit */
   if( pWSS_Settings->bSubtitlesTTX )
   {
      /* subtitles within teletext if true; otherwise, no subtitles within teletext */
      uValue = uValue | 0x00000100;   
   }

   /* set subtitling mode */
   switch( pWSS_Settings->SubtitlingMode )
   {
      case SBTITLE_NO_OPEN:
         uValue = uValue | 0x00000000;
         break;
      case SBTITLE_IN_ACTIVE:
         uValue = uValue | 0x00000200;
         break;
      case SBTITLE_OUT_ACTIVE:
         uValue = uValue | 0x00000400;
         break;
      case SBTITLE_RESERVED:
         uValue = uValue | 0x00000600;
         break;
      default:
         trace_new(TRACE_TVENC, "bt861_wss_set_config: invalid subtitling mode in settings. \n");
         return CNXT_TVENC_BAD_PARAMETER;
   }

   /* set surround sound bit */
   if( pWSS_Settings->bSurroundSound )
   {
      uValue = uValue | 0x00000800;   /* surround sound mode if true; otherwise, no surround sound info */
   }

   /* set copyright info */
   if( pWSS_Settings->bCopyright )
   {
      uValue = 0x00001000;   /* copy right asserted if true; otherwise, no copyright or status unknown */
   }

   if( pWSS_Settings->bCopyRestrict )
   {
      uValue = uValue | 0x00002000;  /* copying restricted if true; otherwise, no copy restriction */
   }

   /* write WSS data */
    CNXT_SET_VAL(ENC_WS_DATA_REG, ENC_WS_DATA_WSDAT_MASK, uValue);

   /* keep all the settings */   
   pUnitInst->WSS_Settings.AspectRatio = pWSS_Settings->AspectRatio;
   pUnitInst->WSS_Settings.bFilmMode = pWSS_Settings->bFilmMode;
   pUnitInst->WSS_Settings.bColorCoding = pWSS_Settings->bColorCoding;
   pUnitInst->WSS_Settings.bHelper = pWSS_Settings->bHelper;
   pUnitInst->WSS_Settings.bSubtitlesTTX = pWSS_Settings->bSubtitlesTTX;
   pUnitInst->WSS_Settings.SubtitlingMode = pWSS_Settings->SubtitlingMode;
   pUnitInst->WSS_Settings.bSurroundSound = pWSS_Settings->bSurroundSound;
   pUnitInst->WSS_Settings.bCopyright = pWSS_Settings->bCopyright;
   pUnitInst->WSS_Settings.bCopyRestrict = pWSS_Settings->bCopyRestrict;

   return CNXT_TVENC_OK;
}

/********************************************************************/
/*  FUNCTION:    bt861_internal_wss_get_config                      */
/*                                                                  */
/*  PARAMETERS:  pUnitInst: Pointer to the unit instance            */ 
/*               pWSS_Settings - pointer to structure receiving     */
/*               wide screen signaling configuration settings       */
/*                                                                  */                                                                
/*  DESCRIPTION: This function queries the configuration information*/
/*               of the wide screen signaling to TV encoder.        */
/*                                                                  */
/*  RETURNS:     CNXT_TVENC_OK                                      */
/*                                                                  */
/*  CONTEXT:     May be called from any context.                    */
/*                                                                  */
/********************************************************************/
static CNXT_TVENC_STATUS bt861_internal_wss_get_config( CNXT_TVENC_UNIT_INST *pUnitInst, \
                                                        CNXT_TVENC_WSS_SETTINGS *pWSS_Settings  )
{   
   pWSS_Settings->AspectRatio = pUnitInst->WSS_Settings.AspectRatio;
   pWSS_Settings->bFilmMode = pUnitInst->WSS_Settings.bFilmMode;
   pWSS_Settings->bColorCoding = pUnitInst->WSS_Settings.bColorCoding;
   pWSS_Settings->bHelper = pUnitInst->WSS_Settings.bHelper;
   pWSS_Settings->bSubtitlesTTX = pUnitInst->WSS_Settings.bSubtitlesTTX;
   pWSS_Settings->SubtitlingMode = pUnitInst->WSS_Settings.SubtitlingMode;
   pWSS_Settings->bSurroundSound = pUnitInst->WSS_Settings.bSurroundSound;
   pWSS_Settings->bCopyright = pUnitInst->WSS_Settings.bCopyright;
   pWSS_Settings->bCopyRestrict = pUnitInst->WSS_Settings.bCopyRestrict;
 
   return CNXT_TVENC_OK;
}

/********************************************************************/
/*  FUNCTION:    bt861_internal_wss_disable                         */
/*                                                                  */
/*  PARAMETERS:  pUnitInst: Pointer to the unit instance            */ 
/*                                                                  */                                                                
/*  DESCRIPTION: This function disables wide screen signaling.      */
/*                                                                  */
/*  RETURNS:     CNXT_TVENC_OK                                      */
/*                                                                  */
/*  CONTEXT:     Must be called in task context.                    */
/*                                                                  */
/********************************************************************/
static CNXT_TVENC_STATUS bt861_internal_wss_disable( CNXT_TVENC_UNIT_INST *pUnitInst )
{    
   /* set the EWSSF1 bit to 0 */
   CNXT_SET_VAL(ENC_XDS_CC_CTL_REG, ENC_XDS_CC_CTL_EWSSF1_MASK, 0);

   return CNXT_TVENC_OK;
}

/********************************************************************/
/*  FUNCTION:    bt861_internal_cgms_enable                         */
/*                                                                  */
/*  PARAMETERS:  pUnitInst: Pointer to the unit instance            */ 
/*                                                                  */                                                                
/*  DESCRIPTION: This function enables copy generation management   */
/*               system.                                            */
/*                                                                  */
/*  RETURNS:     CNXT_TVENC_OK                                      */
/*                                                                  */
/*  CONTEXT:     Must be called in task context.                    */
/*                                                                  */
/********************************************************************/
static CNXT_TVENC_STATUS bt861_internal_cgms_enable( CNXT_TVENC_UNIT_INST *pUnitInst )
{    
   /* set the EWSSF1 and EWSSF2 bits to 1 */
   CNXT_SET_VAL(ENC_XDS_CC_CTL_REG, ENC_XDS_CC_CTL_EWSSF1_MASK, 1);
   CNXT_SET_VAL(ENC_XDS_CC_CTL_REG, ENC_XDS_CC_CTL_EWSSF2_MASK, 1);
   /* write 0 to the WSSDAT register */
   CNXT_SET_VAL(ENC_WS_DATA_REG, ENC_WS_DATA_WSDAT_MASK, 0);   

   return CNXT_TVENC_OK;

}

/********************************************************************/
/*  FUNCTION:    bt861_internal_cgms_set_config                     */
/*                                                                  */
/*  PARAMETERS:  pUnitInst: Pointer to the unit instance            */ 
/*               pCGMS_Settings - pointer to structure containing   */
/*               CGMS configuration settings                        */
/*                                                                  */                                                                
/*  DESCRIPTION: This function passes the CGMS configuration        */
/*               information to TV encoder.                         */
/*                                                                  */
/*  RETURNS:     CNXT_TVENC_OK                                      */
/*               CNXT_TVENC_BAD_PARAMETER                           */
/*                                                                  */
/*  CONTEXT:     Must be called in task context.                    */
/*                                                                  */
/********************************************************************/
static CNXT_TVENC_STATUS bt861_internal_cgms_set_config( CNXT_TVENC_UNIT_INST *pUnitInst, \
                                               CNXT_TVENC_CGMS_SETTINGS *pCGMS_Settings  )
{    
   u_int32 uValue = 0;
   u_int32 uValueCRC;
   u_int16 uValueData;

   /* set the aspect ratio */
   switch( pCGMS_Settings->AspectRatio )
   {
      case CGMS_FFORMAT_43:
         uValue = 0;
         break;
      case CGMS_FFORMAT_169:
         uValue = 0x00000001;
         break;
      case CGMS_LBOX_43:
         uValue = 0x00000002;
         break;
      default:
         trace_new(TRACE_TVENC, "bt861_internal_cgms_set_config: invalid aspect ratio in CGMS settings. \n");
         return CNXT_TVENC_BAD_PARAMETER;
   }

   /* set the copy control information */
   /* if copy control info is not to be transferred, bit number 3,4,5,6 are all 1s; */
   /* otherwise, all 0s */
   if( !pCGMS_Settings->bTransfCopyright )       
   {
      uValue = uValue | 0x0000003C;   
   }
      
   /* set CGMS-A */
   switch( pCGMS_Settings->CGMS_A )
   {
      case CGMS_COPY_NO_RSTRICT:
         uValue = uValue;
         break;
      case CGMS_NO_CONDITION:
         uValue = uValue | 0x00000080;
         break;
      case CGMS_ONE_GEN_COPY:
         uValue = uValue | 0x00000040;
         break;
      case CGMS_NO_COPY:
         uValue = uValue | 0x0000000C0;
         break;
      default:
         trace_new(TRACE_TVENC, "bt861_internal_cgms_set_config: invalid CGMS-A in CGMS settings. \n");
         return CNXT_TVENC_BAD_PARAMETER;
   }
   
   /* set APS trigger bit */
   switch( pCGMS_Settings->APSTrigger )
   {
      case CGMS_PSP_OFF:
         uValue = uValue;
         break;
      case CGMS_PSP_ON_SBURST_OFF:
         uValue = uValue | 0x00000200;
         break;
      case CGMS_PSP_ON_2Line_SBURST:
         uValue = uValue | 0x00000100;
         break;
      case CGMS_PSP_ON_4Line_SBURST:
         uValue = uValue | 0x00000300;
         break;
      default:
         trace_new(TRACE_TVENC, "bt861_internal_cgms_set_config: invalid APS trigger bit in CGMS settings. \n");
         return CNXT_TVENC_BAD_PARAMETER;
   }

   /* set analog source bit */
   if( pCGMS_Settings->bAlogSrc )
   {
      uValue = uValue | 0x00000400;   /* analog pre-recored package medium */
   }

   /* now calculate the CRC value */
   uValueData = reflect(uValue, 16);  /* make bit 0 to the MSB */
   uValueCRC = cal_crc_8(CRC_INIT,uValueData);
   uValue = uValue | (uValueCRC << 12);

   /* write WSS data */
   CNXT_SET_VAL(ENC_WS_DATA_REG, ENC_WS_DATA_WSDAT_MASK, uValue);

   /* keep all the settings */   
   pUnitInst->CGMS_Settings.AspectRatio = pCGMS_Settings->AspectRatio;
   pUnitInst->CGMS_Settings.bTransfCopyright = pCGMS_Settings->bTransfCopyright;
   pUnitInst->CGMS_Settings.CGMS_A = pCGMS_Settings->CGMS_A;
   pUnitInst->CGMS_Settings.APSTrigger = pCGMS_Settings->APSTrigger;
   pUnitInst->CGMS_Settings.bAlogSrc = pCGMS_Settings->bAlogSrc;

   return CNXT_TVENC_OK;
}

/********************************************************************/
/*  FUNCTION:    bt861_internal_cgms_get_config                     */
/*                                                                  */
/*  PARAMETERS:  pUnitInst: Pointer to the unit instance            */ 
/*               pCGMS_Settings - pointer to structure receiving    */
/*               CGMS configuration settings                        */
/*                                                                  */                                                                
/*  DESCRIPTION: This function queries the CGMS configuration       */
/*               information.                                       */
/*                                                                  */
/*  RETURNS:     CNXT_TVENC_OK                                      */
/*                                                                  */
/*  CONTEXT:     May be called from any context.                    */
/*                                                                  */
/********************************************************************/
static CNXT_TVENC_STATUS bt861_internal_cgms_get_config( CNXT_TVENC_UNIT_INST *pUnitInst, \
                                               CNXT_TVENC_CGMS_SETTINGS *pCGMS_Settings  )
{    
   pCGMS_Settings->AspectRatio = pUnitInst->CGMS_Settings.AspectRatio;
   pCGMS_Settings->bTransfCopyright = pUnitInst->CGMS_Settings.bTransfCopyright;
   pCGMS_Settings->CGMS_A = pUnitInst->CGMS_Settings.CGMS_A;
   pCGMS_Settings->APSTrigger = pUnitInst->CGMS_Settings.APSTrigger;
   pCGMS_Settings->bAlogSrc = pUnitInst->CGMS_Settings.bAlogSrc;

   return CNXT_TVENC_OK;
}

/********************************************************************/
/*  FUNCTION:    bt861_internal_cgms_disable                        */
/*                                                                  */
/*  PARAMETERS:  pUnitInst: Pointer to the unit instance            */ 
/*                                                                  */                                                                
/*  DESCRIPTION: This function disables CGMS.                       */
/*                                                                  */
/*  RETURNS:     CNXT_TVENC_OK                                      */
/*                                                                  */
/*  CONTEXT:     Must be called in task context.                    */
/*                                                                  */
/********************************************************************/
static CNXT_TVENC_STATUS bt861_internal_cgms_disable( CNXT_TVENC_UNIT_INST *pUnitInst )
{    
   /* set the EWSSF1 and EWSSF2 bits to 0 */
   CNXT_SET_VAL(ENC_XDS_CC_CTL_REG, ENC_XDS_CC_CTL_EWSSF1_MASK, 0);
   CNXT_SET_VAL(ENC_XDS_CC_CTL_REG, ENC_XDS_CC_CTL_EWSSF2_MASK, 0);

   return CNXT_TVENC_OK;
}

/********************************************************************/
/*  FUNCTION:    bt861_internal_tvenc_init                          */
/*                                                                  */
/*  PARAMETERS:  pUnitInst: Pointer to the unit instance            */ 
/*               function_table - pointer to the DEMOD_FTABLE       */
/*               structure to be filled out with function pointers  */
/*               for this module.                                   */
/*                                                                  */
/*  DESCRIPTION: This function initialises the driver and makes it  */
/*               ready for use.                                     */
/*                                                                  */
/*  RETURNS:     CNXT_TVENC_OK                                      */
/*               CNXT_TVENC_RESOURCE_ERROR                          */
/*                                                                  */
/*  CONTEXT:     May be called in any context.                      */
/*                                                                  */
/********************************************************************/
CNXT_TVENC_STATUS bt861_internal_tvenc_init(CNXT_TVENC_UNIT_INST *pUnitInst, TVENC_FTABLE *function_table)
{
   pUnitInst->module = CNXT_TVENC_BT861_INTERNAL;

   /* read the supported output connection types from the config file */
   pUnitInst->pCaps->uConnections = VIDEO_SIGNAL_OUTPUT_TYPES;

   function_table->set_picctl = bt861_internal_set_picture_control;
   function_table->get_picctl = bt861_internal_get_picture_control;    
   function_table->set_connection = bt861_internal_set_connection;
   function_table->get_connection = bt861_internal_get_connection;
   function_table->set_posoffset = bt861_internal_set_position_offset;
   function_table->get_posoffset = bt861_internal_get_position_offset;
   function_table->set_standard = bt861_internal_set_standard;
   function_table->get_standard = bt861_internal_get_standard; 
   function_table->vtiming_reset = bt861_internal_video_timing_reset;
   function_table->ttx_enable = bt861_internal_ttx_enable;
   function_table->ttx_set_lines = bt861_internal_ttx_set_lines;
   function_table->ttx_disable = bt861_internal_ttx_disable;
   function_table->cc_enable = bt861_internal_cc_enable;
   function_table->cc_disable = bt861_internal_cc_disable;
   function_table->cc_send_data = bt861_internal_cc_send_data;  
   function_table->wss_enable = bt861_internal_wss_enable;
   function_table->wss_setconfig = bt861_internal_wss_set_config;
   function_table->wss_getconfig = bt861_internal_wss_get_config;
   function_table->wss_disable = bt861_internal_wss_disable;
   function_table->cgms_enable = bt861_internal_cgms_enable;
   function_table->cgms_setconfig = bt861_internal_cgms_set_config;
   function_table->cgms_getconfig = bt861_internal_cgms_get_config;
   function_table->cgms_disable = bt861_internal_cgms_disable;

	#if DIGITAL_DISPLAY == PW113_DISPLAY
	cnxt_pw113_display_init();
	#endif

   return CNXT_TVENC_OK;
}

/****************************************************************************
 * Modifications:
 * $Log: 
 *  11   mpeg      1.10        3/24/04 12:12:23 PM    Billy Jackman   CR(s) 
 *        8535 8650 : Modified settings of DRM teletext start and end lines to 
 *        conform to hardware implementation.
 *  10   mpeg      1.9         3/18/04 11:56:24 AM    Xin Golden      CR(s) 
 *        8592 : modified picture control setting range from [-127,128] to 
 *        [-128,127].
 *  9    mpeg      1.8         3/3/04 2:36:28 PM      Billy Jackman   CR(s) 
 *        8499 : Modified to set correct VBI lines for DRM/encoder for 
 *        teletext.
 *  8    mpeg      1.7         1/6/04 6:58:30 PM      Xin Golden      CR(s) 
 *        8158 : added feature to disable all DACs in function 
 *        cnxt_tvenc_set_output_connection.
 *  7    mpeg      1.6         11/25/03 5:44:42 PM    Xin Golden      CR(s): 
 *        8028 8029 removed references to sin and cos by replacing math with 
 *        lookup table as it in tvenc_bt861.c.
 *  6    mpeg      1.5         11/25/03 8:21:39 AM    Ian Mitchell    CR(s): 
 *        7739 7982 Interface to the PW113 display driver.
 *  5    mpeg      1.4         10/24/03 11:44:05 AM   Xin Golden      CR(s): 
 *        7463 add CGMS support to tvenc driver.
 *  4    mpeg      1.3         9/17/03 4:01:10 PM     Lucy C Allevato SCR(s) 
 *        7482 :
 *        update the TV encoder driver to use the new handle lib.
 *        
 *  3    mpeg      1.2         8/28/03 4:03:46 PM     Lucy C Allevato SCR(s) 
 *        5519 :
 *        set the disable DAC bit to 1 for wabash so the normal DAC operation 
 *        will be performed
 *        
 *  2    mpeg      1.1         8/18/03 7:16:18 PM     Lucy C Allevato SCR(s) 
 *        5519 :
 *        modified the teletext and wss APIs
 *        
 *  1    mpeg      1.0         7/30/03 4:04:20 PM     Lucy C Allevato 
 * $
 * 
 ****************************************************************************/

