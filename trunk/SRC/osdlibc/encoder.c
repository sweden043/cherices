/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*           Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002         */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       encoder.c
 *
 *
 * Description:    Functions relating to setup and operation of various supported video encoders
 *
 *
 * Author:         Rob Tilton, Dave Wilson & Billy Jackman
 *
 ****************************************************************************/
/* $Header: encoder.c, 22, 9/29/03 4:35:28 PM, Miles Bintz$
 ****************************************************************************/

/************************************************************************************/
/*                                                                                  */
/* NOTE: This module is a work in progress. New functions have been added which     */
/* adhere to our new multi-instance API model but old functions (called exclusively */
/* from within the OSDLIBC driver) have not been updated to conform to this model   */
/* yet. This will be done in due course so beware of changes in this module over    */
/* the next few months.                                                             */
/*                                                                                  */
/* Note also that implementation of the new functions is incomplete and that handle */
/* and instance management (even though there is only 1 instance currently) is not  */
/* yet in place.                                                                    */
/************************************************************************************/

#define USE_MATH 0

/*****************/
/* Include Files */
/*****************/
#include <string.h>
#include "stbcfg.h"
#include "basetype.h"
#include "kal.h"
#include "osdlibc.h"
#include "vidlibc.h"
#include "iic.h"
#include "globals.h"
#include "retcodes.h"
#include "osdprv.h"
#include "handle.h"
#include "encoder.h"
#include "encoder_priv.h"
#include "math.h"

/***********************/
/* External References */
/***********************/
extern int gnOsdMaxHeight;
extern int gnOsdMaxWidth;

/******************************************/
/* Local Definitions and Global Variables */
/******************************************/

static int     gnHSync;
static u_int32 gnHBlankInitial;
static u_int32 gnVBlankInitial;
static int     gnHSyncInitial;         
static u_int16 gVideoStandard = 0; /* Not PAL, nor NTSC, nor SECAM. */
u_int32        gdwEncoderType; /* Beware - referenced still by some external drivers! */

#if CUSTOMER == VENDOR_D
OSD_DISPLAY_OUTPUT gVideoOutput = OSD_OUTPUT_RGB | OSD_OUTPUT_COMPOSITE;
#else
OSD_DISPLAY_OUTPUT gVideoOutput = OSD_OUTPUT_COMPOSITE;
#endif
IICBUS giicEncoderBus;
BYTE   gEncoderAddr = (BYTE)0;

#define BT861_DAC_ENABLE_REG 0x18

#define ENCODER_PHASE_SHIFT  2
#define INITIAL_Y_OFFSET     0
#define INITIAL_X_OFFSET     0

/* Decide which encoder setup code to include. If the chip in use does not have */
/* an internal decoder, look at the EXTERNAL_DECODER definition to decide which */
/* devices to support.                                                          */
#if (INTERNAL_ENCODER == NOT_PRESENT)
  #if (EXTERNAL_ENCODER == DETECT_BT861_BT865) || (EXTERNAL_ENCODER == BT861)
    #define INCLUDE_BT861
  #endif
  #if (EXTERNAL_ENCODER == DETECT_BT861_BT865) || (EXTERNAL_ENCODER == BT865)
    #define INCLUDE_BT865
  #endif

  /* If using an external decoder, we will need this typedef to store I2C */
  /* register address and value.                                          */
  typedef struct _I2C_REGISTER_OPERATION
  {
    BYTE bRegIndex;
    BYTE bValue;
    BYTE bMask;
  } I2C_REGISTER_OPERATION;

#endif

/********************************/
/* Internal Function Prototypes */
/********************************/
static void    private_encoder_internal_data_init(void);
static u_int32 private_encoder_get_unit_num(CNXT_ENCODER_CAPS *pCaps);
static CNXT_ENCODER_STATUS private_get_encoder_controls( CNXT_ENCODER_CONTROLS *pControls );
static CNXT_ENCODER_STATUS private_set_encoder_controls( CNXT_ENCODER_CONTROLS *pControls );

/***********************************************************************/
/* Global data used for the new API functions published by this module */
/***********************************************************************/
static CNXT_ENCODER_DRIVER_INST DriverInst = { 0 };
static CNXT_ENCODER_DRIVER_INST *pDriverInst = &DriverInst;
static CNXT_ENCODER_INST InstArray[CNXT_ENCODER_MAX_HANDLES]; 
static bool              bInstUsed[CNXT_ENCODER_MAX_HANDLES];                                           
static CNXT_ENCODER_UNIT_INST UnitInst[CNXT_ENCODER_NUM_UNITS]; 
static CNXT_ENCODER_CAPS CapsArray[CNXT_ENCODER_NUM_UNITS] = 
{ 
  {
  sizeof(CNXT_ENCODER_CAPS) , 
  0,  
  ENCODER_CONTROL_BRIGHTNESS | ENCODER_CONTROL_CONTRAST | ENCODER_CONTROL_HUE
  }
}; 
static u_int8 uSavedHue = 0; 

#define BT861_HUE_REGISTER_INDEX        0x3B
#define BT861_CONTRAST_REGISTER_INDEX   0x22
#define BT861_BRIGHTNESS_REGISTER_INDEX 0x37
#define BT861_MULT_UU_REGISTER_INDEX    0x5C
#define BT861_MULT_VU_REGISTER_INDEX    0x5D
#define BT861_MULT_UV_REGISTER_INDEX    0x5E
#define BT861_MULT_VV_REGISTER_INDEX    0x5F 
                                                                   
#define DEFAULT_CONTRAST   0xA2                                                                   
#define DEFAULT_BRIGHTNESS 0x80                                                                   
#define DEFAULT_HUE        0x80                                                                   
#define DEFAULT_SATURATION 0x80                                                                   
#define DEFAULT_MULT_UU    0x7F
#define DEFAULT_MULT_VU    0x00
#define DEFAULT_MULT_UV    0x00
#define DEFAULT_MULT_VV    0x7F

/* Encoder setup parameters. We use arrays here containing register address and value */
/* to ensure that we can dictate the order the registers are written as well as the   */
/* values which are written to them.                                                  */
#ifdef INCLUDE_BT861
static const I2C_REGISTER_OPERATION sBt861NTSCSetup[] =
{
  {0x17, 0x08, 0xFF},
  {0x16, 0x00, 0x20}, /* 625LINE         */
  {0x08, 0x7F, 0xFF}, /* AHSYNC_WIDTH    */
  {0x1F, 0x7A, 0xDD}, /* BURST_AMP       */
  {0x1D, 0x01, 0x01}, /* CROSSFILT       */
  {0x16, 0x00, 0x04}, /* FM              */
  {0x06, 0xD0, 0xFF}, /* HACTIVE low     */
  {0x07, 0x02, 0x03}, /* HACTIVE high    */
  {0x10, (BT861_NTSC_HSYNC&0xFF), 0xFF}, /* HSYNC low       */
  {0x11, ((BT861_NTSC_HSYNC>>8)&0xFF), 0x03}, /* HSYNC high      */
  {0x0B, (BT861_NTSC_HBLANK&0xFF), 0xFF}, /* HBLANK low      */
  {0x0C, ((BT861_NTSC_HBLANK>>8)&0xFF), 0x03}, /* HBLANK high     */
  {0x09, 0x8C, 0xFF}, /* HBURST_BEG      */
  {0x0A, 0x54, 0xFF}, /* HBURST_END      */
  {0x04, 0xB4, 0xFF}, /* HCLOCK low      */
  {0x05, 0x06, 0x0F}, /* HCLOCK high     */
  {0x21, 0x8D, 0xFF}, /* M_CB            */
  {0x20, 0xC7, 0xFF}, /* M_CR            */
  {0x22, 0x9A, 0xFF}, /* M_Y             */
  {0x26, 0x1F, 0xFF}, /* MSC_DR(0)       */
  {0x27, 0x7C, 0xFF}, /* MSC_DR(1)       */
  {0x28, 0xF0, 0xFF}, /* MSC_DR(2)       */
  {0x29, 0x21, 0xFF}, /* MSC_DR(3)       */
  {0x16, 0x00, 0x02}, /* NI              */
  {0x16, 0x00, 0x08}, /* PAL             */
  {0x38, 0x00, 0xFF}, /* PHASE_OFF       */
  {0x16, 0x00, 0x80}, /* SC_RESET        */
  {0x16, 0x10, 0x10}, /* SETUP           */
  {0x1E, 0xE4, 0xFF}, /* SYNC_AMP        */
  {0x0E, 0xF1, 0xFF}, /* VACTIVE low     */
  {0x0F, 0x00, 0x01}, /* VACTIVE high    */
  {0x0D, 0x13, 0xFF}, /* VBLANK          */
  {0x16, 0x00, 0x40}, /* VSYNC_DUR       */

  {0x1D, 0x23, 0xFF},
  {0x19, 0x10, 0xEF},
};
static const u_int16 uNum861NTSCOps = sizeof(sBt861NTSCSetup)/sizeof(I2C_REGISTER_OPERATION);

static const I2C_REGISTER_OPERATION sBt861PALSetup[] =
{
  /* Brady platfrom has no scart switch. */
 #if SCART_TYPE == SCART_TYPE_BRADY
  {0x17,0x28,0xFF}, /* Not in timing list */
  {0x18,0x39,0xFF}, /* Disable DAC's B and C */
 #else
 {0x17,0x08,0xFF}, /* Not in timing list */
 #endif



  {0x16, 0x20, 0x20}, /* 625LINE         */
  {0x08, 0x7F, 0xFF}, /* AHSYNC_WIDTH    */
  {0x1F, 0x5D, 0xDD}, /* BURST_AMP       */
  {0x1D, 0x01, 0x01}, /* CROSSFILT       */
  {0x16, 0x00, 0x04}, /* FM              */
  {0x06, 0xD0, 0xFF}, /* HACTIVE low     */
  {0x07, 0x02, 0x03}, /* HACTIVE high    */
  {0x10, (BT861_PAL_HSYNC&0xFF), 0xFF}, /* HSYNC low       */
  {0x11, ((BT861_PAL_HSYNC>>8)&0xFF), 0x03}, /* HSYNC high      */
  {0x0B, (BT861_PAL_HBLANK&0xFF), 0xFF}, /* HBLANK low      */
  {0x0C, ((BT861_PAL_HBLANK>>8)&0xFF), 0x03}, /* HBLANK high     */
  {0x09, 0x95, 0xFF}, /* HBURST_BEG      */
  {0x0A, 0x51, 0xFF}, /* HBURST_END      */
  {0x04, 0xC0, 0xFF}, /* HCLOCK low      */
  {0x05, 0x06, 0x0F}, /* HCLOCK high     */
  {0x21, 0x9B, 0xFF}, /* M_CB            */
  {0x20, 0xDA, 0xFF}, /* M_CR            */
  {0x22, 0xA2, 0xFF}, /* M_Y             */
  {0x26, 0xCB, 0xFF}, /* MSC_DR(0)       */
  {0x27, 0x8A, 0xFF}, /* MSC_DR(1)       */
  {0x28, 0x09, 0xFF}, /* MSC_DR(2)       */
  {0x29, 0x2A, 0xFF}, /* MSC_DR(3)       */
  {0x16, 0x00, 0x02}, /* NI              */
  {0x16, 0x08, 0x08}, /* PAL             */
  {0x38, 0x00, 0xFF}, /* PHASE_OFF       */
  {0x16, 0x00, 0x80}, /* SC_RESET        */
  {0x16, 0x00, 0x10}, /* SETUP           */
  {0x1E, 0xF0, 0xFF}, /* SYNC_AMP        */
  {0x0E, 0x20, 0xFF}, /* VACTIVE low     */
  {0x0F, 0x01, 0x01}, /* VACTIVE high    */
  {0x0D, 0x17, 0xFF}, /* VBLANK          */
  {0x16, 0x40, 0x40}, /* VSYNC_DUR       */

  {0x1D, 0x23, 0xFF},
  {0x19, 0x10, 0xEF}, // Not in timing list
};

static const u_int16 uNum861PALOps = sizeof(sBt861PALSetup)/sizeof(I2C_REGISTER_OPERATION);

static const I2C_REGISTER_OPERATION sBt861SECAMSetup[] =
{
  /* Parameters for 27MHz system clock, no synchronization bottleneck pulses */
  {0x17, 0x08, 0xFF},

  {0x16, 0x20, 0x20}, /* 625LINE         */
  {0x08, 0x7F, 0xFF}, /* AHSYNC_WIDTH    */
  {0x1D, 0x00, 0x01}, /* CROSSFILT       */
  {0x33, 0xA3, 0xFF}, /* DB_MAX low      */
  {0x34, 0x05, 0x07}, /* DB_MAX high     */
  {0x35, 0x9F, 0xFF}, /* DB_MIN low      */
  {0x36, 0x04, 0x07}, /* DB_MIN high     */
  {0x2F, 0xA3, 0xFF}, /* DR_MAX low      */
  {0x30, 0x05, 0x07}, /* DR_MAX high     */
  {0x31, 0x9F, 0xFF}, /* DR_MIN low      */
  {0x32, 0x04, 0x07}, /* DR_MIN high     */
  {0x1B, 0x00, 0x40}, /* FIELD_ID        */
  {0x16, 0x04, 0x04}, /* FM              */
  {0x06, 0xC0, 0xFF}, /* HACTIVE low     */
  {0x07, 0x02, 0x03}, /* HACTIVE high    */
  {0x0B, 0x28, 0xFF}, /* HBLANK low      */
  {0x0C, 0x01, 0x03}, /* HBLANK high     */
  {0x09, 0x97, 0xFF}, /* HBURST_BEG      */
  {0x04, 0xC0, 0xFF}, /* HCLOCK low      */
  {0x05, 0x06, 0x0F}, /* HCLOCK high     */
  {0x21, 0x94, 0xFF}, /* M_CB            */
  {0x20, 0xB5, 0xFF}, /* M_CR            */
  {0x22, 0xA4, 0xFF}, /* M_Y             */
  {0x2A, 0x13, 0xFF}, /* MSC_DB(0)       */
  {0x2B, 0xDA, 0xFF}, /* MSC_DB(1)       */
  {0x2C, 0x4B, 0xFF}, /* MSC_DB(2)       */
  {0x2D, 0x28, 0xFF}, /* MSC_DB(3)       */
  {0x26, 0x72, 0xFF}, /* MSC_DR(0)       */
  {0x27, 0x1C, 0xFF}, /* MSC_DR(1)       */
  {0x28, 0xC7, 0xFF}, /* MSC_DR(2)       */
  {0x29, 0x29, 0xFF}, /* MSC_DR(3)       */
  {0x16, 0x00, 0x02}, /* NI              */
  {0x16, 0x00, 0x08}, /* PAL             */
  {0x1A, 0x00, 0x02}, /* PROG_SC         */
  {0x2E, 0x86, 0xFF}, /* SC_AMP          */
  {0x1A, 0x00, 0x01}, /* SC_PATTERN      */
  {0x16, 0x00, 0x10}, /* SETUP           */
  {0x1E, 0xF0, 0xFF}, /* SYNC_AMP        */
  {0x0E, 0x1F, 0xFF}, /* VACTIVE low     */
  {0x0F, 0x01, 0x01}, /* VACTIVE high    */
  {0x0D, 0x17, 0xFF}, /* VBLANK          */
  {0x16, 0x40, 0x40}, /* VSYNC_DUR       */

  {0x1D, 0x23, 0xFF},
  {0x19, 0x10, 0xEF},
};

static const u_int16 uNum861SECAMOps = sizeof(sBt861SECAMSetup)/sizeof(I2C_REGISTER_OPERATION);

#endif

#if (INTERNAL_ENCODER == INTERNAL_BT861_LIKE)

typedef struct _ONBOARD_REGISTER_OPERATION
{
  u_int32 RegAddress;
  u_int32 Mask;
  u_int32 Value;
} ONBOARD_REGISTER_OPERATION;

static const ONBOARD_REGISTER_OPERATION sOnboard861NTSCSetup[] =
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
  { ENC_CONTROL0_REG, ENC_CTL0_EACTIVE_MASK, ENC_CTL0_EACTIVE_ENABLE_NORMAL },
#if ENCODER_HAS_DISABLE_DACS == 1
  { ENC_CONTROL0_REG, ENC_CTL0_DAC_DISABLE_MASK, ENC_CTL0_DAC_DISABLE_ENABLE },
#endif
  { ENC_CONTROL0_REG, ENC_CTL0_ALL_DAC_MASK, 0 }
};
static const u_int16 uNumOnboard861NTSCOps = sizeof(sOnboard861NTSCSetup)/sizeof(ONBOARD_REGISTER_OPERATION);

static const ONBOARD_REGISTER_OPERATION sOnboard861PALSetup[] =
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
  { ENC_CONTROL0_REG, ENC_CTL0_EACTIVE_MASK, ENC_CTL0_EACTIVE_ENABLE_NORMAL },
#if ENCODER_HAS_DISABLE_DACS == 1
  { ENC_CONTROL0_REG, ENC_CTL0_DAC_DISABLE_MASK, ENC_CTL0_DAC_DISABLE_ENABLE },
#endif
  { ENC_CONTROL0_REG, ENC_CTL0_ALL_DAC_MASK, 0 }
};
static const u_int16 uNumOnboard861PALOps = sizeof(sOnboard861PALSetup)/sizeof(ONBOARD_REGISTER_OPERATION);

static const ONBOARD_REGISTER_OPERATION sOnboard861SECAMSetup[] =
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
  { ENC_CONTROL0_REG, ENC_CTL0_EACTIVE_MASK, ENC_CTL0_EACTIVE_ENABLE_NORMAL },
#if ENCODER_HAS_DISABLE_DACS == 1
  { ENC_CONTROL0_REG, ENC_CTL0_DAC_DISABLE_MASK, ENC_CTL0_DAC_DISABLE_ENABLE },
#endif
  { ENC_CONTROL0_REG, ENC_CTL0_ALL_DAC_MASK, 0 }
};
static const u_int16 uNumOnboard861SECAMOps = sizeof(sOnboard861SECAMSetup)/sizeof(ONBOARD_REGISTER_OPERATION);

#endif

#ifdef INCLUDE_BT861
/********************************************************************/
/*  FUNCTION:    SetupBt861Encoder                                  */
/*                                                                  */
/*  PARAMETERS:  video_standard - NTSC, PAL or SECAM                */
/*               pHBlank        - Storage for returned HBLANK count */
/*               pVBlank        - Storage for returned VBLANK count */
/*                                                                  */
/*  DESCRIPTION: Set up the external Bt861 encoder to drive NTSC    */
/*               or PAL-format video.                               */
/*                                                                  */
/*  RETURNS:     Nothing                                            */
/*                                                                  */
/*  CONTEXT:     Must be called in non-interrupt context.           */
/*                                                                  */
/********************************************************************/
static void SetupBt861Encoder(u_int16 video_standard, u_int32 *pHBlank, u_int32 *pVBlank)
{
  const I2C_REGISTER_OPERATION *pOpList;
  u_int16                 uOpCount;
  u_int16                 uLoop;
  BYTE                    byVal;

  /* Set up for initialisation depending upon the standard required */
  switch(video_standard)
  {
    case NTSC:
      *pHBlank = BT861_NTSC_HBLANK+ENCODER_PHASE_SHIFT;
      *pVBlank = BT861_NTSC_VBLANK;
      gnHSync = gnHSyncInitial = BT861_NTSC_HSYNC;
      uOpCount = uNum861NTSCOps;
      pOpList  = sBt861NTSCSetup;
      break;

    case PAL:
      *pHBlank = BT861_PAL_HBLANK+ENCODER_PHASE_SHIFT; /* Was -2 but experimentation seems to suggest this sets the */
                                     /* screen origin correctly.                                  */
      *pVBlank = BT861_PAL_VBLANK-1; /* Move PAL display up 1 line so that bottom line is visible (for BSkyB) */
      gnHSync = gnHSyncInitial = BT861_PAL_HSYNC;
      uOpCount = uNum861PALOps;
      pOpList  = sBt861PALSetup;
      break;

    case SECAM:
      *pHBlank = BT861_PAL_HBLANK-2;
      *pVBlank = BT861_PAL_VBLANK;
      gnHSync = gnHSyncInitial = BT861_PAL_HSYNC;
      uOpCount = uNum861SECAMOps;
      pOpList  = sBt861SECAMSetup;
      break;

    default:
      trace_new(OSD_ERROR_MSG, "OSD: Invalid video standard %d passed to SetupBt861Encoder!", video_standard);
      error_log(ERROR_WARNING);
      return;
  }

  /* Write the appropriate values to the Bt861 internal registers */
  for(uLoop = 0; uLoop < uOpCount; uLoop++)
  {
    if(pOpList[uLoop].bMask == (BYTE)0xFF)
    {
       /* Mask is 0xFF so we just write the register */
      iicWriteIndexedReg(gEncoderAddr,
                         pOpList[uLoop].bRegIndex,
                         pOpList[uLoop].bValue,
                         giicEncoderBus);
    }
    else
    {
      /* Mask is not 0xFF so we need to do a read/modify/write operation */
      iicReadIndexedReg(gEncoderAddr,
                        pOpList[uLoop].bRegIndex,
                        &byVal,
                        giicEncoderBus);
      byVal = (byVal & ~pOpList[uLoop].bMask) | pOpList[uLoop].bValue;
      iicWriteIndexedReg(gEncoderAddr,
                         pOpList[uLoop].bRegIndex,
                         byVal,
                         giicEncoderBus);
    }
  }
}
#endif /* INCLUDE_BT861 */

#ifdef INCLUDE_BT865
/********************************************************************/
/*  FUNCTION:    SetupBt865Encoder                                  */
/*                                                                  */
/*  PARAMETERS:  video_standard - NTSC, PAL or SECAM                */
/*               pHBlank        - Storage for returned HBLANK count */
/*               pVBlank        - Storage for returned VBLANK count */
/*                                                                  */
/*  DESCRIPTION: Set up the external Bt865 encoder to drive NTSC    */
/*               or PAL-format video.                               */
/*                                                                  */
/*  RETURNS:     Nothing                                            */
/*                                                                  */
/*  CONTEXT:     Must be called in non-interrupt context.           */
/*                                                                  */
/********************************************************************/
static void SetupBt865Encoder(u_int16 video_standard, u_int32 *pHBlank, u_int32 *pVBlank)
{
  BYTE byVal;

  if (video_standard == NTSC)
  {
     *pHBlank = BT865_NTSC_HBLANK;
     *pVBlank = BT865_NTSC_VBLANK;
  }
  else /* PAL/SECAM */
  {
     *pHBlank = BT865_PAL_HBLANK;
     *pVBlank = BT865_PAL_VBLANK;
  }

  /* Reset the encoder */
  iicWriteIndexedReg(I2C_ADDR_BT865, 0xA6, 0x80, giicEncoderBus);

  #if CUSTOMER == VENDOR_D
    /* Set the encoder format */
    if (video_standard == PAL)
       byVal = 0x64; /* Set the encoder to PAL, setup is off */
    else
       byVal = 0;    /* Set the encoder for NTSC */
    iicWriteIndexedReg(I2C_ADDR_BT865, 0xCC, byVal, giicEncoderBus);

    /* Enable normal video                                  */
    /* 42 for RGB/SCART                                     */
    iicWriteIndexedReg(I2C_ADDR_BT865, 0xCE, 0x42, giicEncoderBus);
    gVideoOutput = OSD_OUTPUT_RGB | OSD_OUTPUT_COMPOSITE;

    /* Invert sense of Field pin */
    /* Enable HSync Adjustment */
    iicWriteIndexedReg(I2C_ADDR_BT865, 0xCA, 0x40, giicEncoderBus);

  #else /* Not VENDOR_D */

    /* Set the encoder format */
    if (video_standard == PAL)
       byVal = 0x24; /* Set the encoder to PAL */
    else
       byVal = 0;    /* Set the encoder for NTSC */
    iicWriteIndexedReg(I2C_ADDR_BT865, 0xCC, byVal, giicEncoderBus);

    /* Enable normal video                                  */
    iicWriteIndexedReg(I2C_ADDR_BT865, 0xCE, 0x02, giicEncoderBus);
    gVideoOutput = OSD_OUTPUT_COMPOSITE | OSD_OUTPUT_SVIDEO;

    /* Invert sense of Field pin */
    iicWriteIndexedReg(I2C_ADDR_BT865, 0xCA, 0x40, giicEncoderBus);

  #endif /* Vendor D */

}
#endif /* INCLUDE_BT865 */

#if (INTERNAL_ENCODER == INTERNAL_BT861_LIKE)
/********************************************************************/
/*  FUNCTION:    SetupInternalEncoder                               */
/*                                                                  */
/*  PARAMETERS:  video_standard - NTSC, PAL or SECAM                */
/*               pHBlank        - Storage for returned HBLANK count */
/*               pVBlank        - Storage for returned VBLANK count */
/*                                                                  */
/*  DESCRIPTION: Set up the internal video encoder to drive NTSC    */
/*               or PAL-format video.                               */
/*                                                                  */
/*  RETURNS:     Nothing                                            */
/*                                                                  */
/*  CONTEXT:     Must be called in non-interrupt context.           */
/*                                                                  */
/********************************************************************/
static void SetupInternalEncoder(u_int16 video_standard, u_int32 *pHBlank, u_int32 *pVBlank)
{
  const ONBOARD_REGISTER_OPERATION *pOpList;
  u_int16 uOpCount;
  u_int16 uLoop;

  gdwEncoderType = BT861;

  /* Set up for initialisation depending upon the standard required */
  switch(video_standard)
  {
    case NTSC:
#if 0
      *pHBlank = BT861_NTSC_HBLANK+ENCODER_PHASE_SHIFT;
      *pVBlank = BT861_NTSC_VBLANK;
#endif
     *pHBlank = BT861_NTSC_HBLANK;
     *pVBlank = BT861_NTSC_VBLANK;
      gnHSync = gnHSyncInitial = BT861_NTSC_HSYNC;
      uOpCount = uNumOnboard861NTSCOps;
      pOpList  = sOnboard861NTSCSetup;
      break;

    case PAL:
#if 0
      *pHBlank = BT861_PAL_HBLANK+ENCODER_PHASE_SHIFT; /* Was -2 but experimentation seems to suggest this sets the */
                                     /* screen origin correctly.                                  */
      *pVBlank = BT861_PAL_VBLANK-1; /* Move PAL display up 1 line so that bottom line is visible (for BSkyB) */
#endif
     *pHBlank = BT861_PAL_HBLANK;
     *pVBlank = BT861_PAL_VBLANK;
      gnHSync = gnHSyncInitial = BT861_PAL_HSYNC;
      uOpCount = uNumOnboard861PALOps;
      pOpList  = sOnboard861PALSetup;
      break;

    case SECAM:
      *pHBlank = BT861_PAL_HBLANK-2;
      *pVBlank = BT861_PAL_VBLANK;
      gnHSync = gnHSyncInitial = BT861_PAL_HSYNC;
      uOpCount = uNumOnboard861SECAMOps;
      pOpList  = sOnboard861SECAMSetup;
      break;

    default:
      trace_new(OSD_ERROR_MSG, "OSD: Invalid video standard %d passed to SetupInternalEncoder!", video_standard);
      error_log(ERROR_WARNING);
      return;
  }

  /* Write the appropriate values to the encoder registers */
  for(uLoop = 0; uLoop < uOpCount; uLoop++)
  {
    *(LPREG)pOpList[uLoop].RegAddress = (*(LPREG)pOpList[uLoop].RegAddress & ~pOpList[uLoop].Mask) | pOpList[uLoop].Value;
  }
}
#endif /* INTERNAL_ENCODER == INTERNAL_BT861_LIKE */

/*****************************************************************************/
/* Function:    SetOutputType()                                              */
/*                                                                           */
/* Parameters:  u_int16  video_standard - NTSC/PAL/SECAM                     */
/*                                                                           */
/* Returns:     Nothing                                                      */
/*                                                                           */
/* Description: Initializes the video encoder and DRM to the correct output  */
/*              video format.                                                */
/*                                                                           */
/* Context:     Must be called in non-interrupt context                      */
/*                                                                           */
/*****************************************************************************/
void SetOutputType(u_int16 video_standard)
{
  /* This function handles 3 different encoder types - Bt861, Bt865 and */
  /* the internal encoder found in Wabash and later chips. Fan out as   */
  /* necessary and call a single function for each case to make things  */
  /* a lot more readable.                                               */

  /* Determine which encoder setup function to call based upon */
  /* config settings and/or I2C bus address queries.           */

  #if (INTERNAL_ENCODER ==  INTERNAL_BT861_LIKE)

    /********************/
    /* Internal Decoder */
    /********************/
    SetupInternalEncoder(video_standard, &gnHBlank, &gnVBlank);

  #else
    BYTE byVal;

    /********************/
    /* External Decoder */
    /********************/

    /*************************************************************/
    /* Check that we have an encoder at the required I2C address */
    /*************************************************************/
    
    /* Make sure the encoder is actually there */
    #ifdef INCLUDE_BT865
    if (!iicAddressTest(I2C_ADDR_BT865, I2C_BUS_BT865, FALSE))
    {
      #ifdef INCLUDE_BT861
      /* We didn't find the 865 so look for an 861 instead */
      if (!iicAddressTest(I2C_ADDR_BT861, I2C_BUS_BT861, FALSE))
      {
        trace_new(OSD_ERROR_MSG, "OSD: Can't find video encoder at I2C address 0x%02x!\n", I2C_ADDR_BT865);
        error_log(ERROR_WARNING);
      }
      else
      {
        gEncoderAddr   = I2C_ADDR_BT861;
        giicEncoderBus = I2C_BUS_BT861;
      }  
      #else  
      trace_new(OSD_ERROR_MSG, "OSD: Can't find video encoder at I2C address 0x%02x!\n", I2C_ADDR_BT865);
      error_log(ERROR_WARNING);
      #endif    
    }
    else
    {
      giicEncoderBus = I2C_BUS_BT865;
      gEncoderAddr   = I2C_ADDR_BT865;
    }  
    #else /* Not including BT865, only BT861 */
    if (!iicAddressTest(I2C_ADDR_BT861, I2C_BUS_BT861, FALSE))
    {
      trace_new(OSD_ERROR_MSG, "OSD: Can't find video encoder at I2C address 0x%02x!\n", I2C_ADDR_BT865);
      error_log(ERROR_WARNING);
    }
    else
    {
      giicEncoderBus = I2C_BUS_BT861;
      gEncoderAddr   = I2C_ADDR_BT861;
    }  
    #endif
    
    #if (defined INCLUDE_BT861) && (defined INCLUDE_BT865)

      /* We must support both possible external encoders so detect based */
      /* on chip ID returned from the device.                            */

      /* Read the status register and decide if it is an 865 or 861 */
      iicReadIndexedReg(gEncoderAddr, 0x00, &byVal, giicEncoderBus);

      if (((byVal & 0xE0) == (4 << 5)) || ((byVal & 0xE0) == (5 << 5)))
        gdwEncoderType = BT865;
      else
        gdwEncoderType = BT861;
    #else

      /* We are only supporting one or other of the 2 external decoders */

      #if (defined INCLUDE_BT861)
        gdwEncoderType = BT861;
      #else
        #if (defined INCLUDE_BT865)
          gdwEncoderType = BT865
        #else
          /* No decoder type is being initialised! */
          trace_new(OSD_ERROR_MSG, "OSD: No video encoder type configured!\n");
          error_log(ERROR_WARNING);
        #endif /* Bt865 only */
      #endif /* Bt861 only */
    #endif /* Supporting auto-detection of 861 or 865 */

    /****************************************************************/
    /* We have determined which external encoder we are supposed to */
    /* support so now initialise it.                                */
    /****************************************************************/
    if(gdwEncoderType == BT865)
      SetupBt865Encoder(video_standard, &gnHBlank, &gnVBlank);
    else
      SetupBt861Encoder(video_standard, &gnHBlank, &gnVBlank);

  #endif /* Not an internal decoder */

  /******************************************************/
  /* Enable the outputs that are configured for the IRD */
  /******************************************************/
  SetDisplayOutput(VIDEO_SIGNAL_OUTPUT_TYPES);

  /***************************************************************/
  /* Now set up the DRM. This is independent of the encoder type */
  /***************************************************************/
  gnOsdMaxHeight = (video_standard == NTSC) ? 480 : 576;
  gnOsdMaxWidth = 720;

  #ifndef OPENTV
  if (gnOsdMaxHeight != 480)
     CNXT_SET_VAL(glpDrmControl, DRM_CONTROL_OUTPUT_FORMAT_MASK, 1);
  else
     CNXT_SET_VAL(glpDrmControl, DRM_CONTROL_OUTPUT_FORMAT_MASK, 0);
  #else
  /* CX22490/1/6 has a bug that causes upscaled MPEG stills to be badly filtered */
  /* (they flicker) if this bit is set to 1. We need to set it to 1 when playing */
  /* motion video but the video driver turns it on and off as necessary. We also */
  /* need to set it correctly when doing aspect ratio conversion so this is why  */
  /* this code is OpenTV specific - it doesn't support aspect ratio conversion   */
  CNXT_SET_VAL(glpDrmControl, DRM_CONTROL_OUTPUT_FORMAT_MASK, 0);
  #endif

  CNXT_SET_VAL(glpDrmScreenStart, DRM_SCREEN_START_FIRST_ACTIVE_PIXEL_MASK, gnHBlank);
  CNXT_SET_VAL(glpDrmScreenStart, DRM_SCREEN_START_FIELD2_ACTIVE_LINE_MASK, gnVBlank);
  CNXT_SET_VAL(glpDrmScreenStart, DRM_SCREEN_START_FIELD1_ACTIVE_LINE_MASK, gnVBlank);
  CNXT_SET_VAL(glpDrmScreenEnd, DRM_SCREEN_END_LAST_ACTIVE_PIXEL_MASK,
    gnHBlank + (2*OSD_MAX_WIDTH) - 1);
  CNXT_SET_VAL(glpDrmScreenEnd, DRM_SCREEN_END_FIELD2_LAST_LINE_MASK,
    gnVBlank + (gnOsdMaxHeight/2) - 1);
  CNXT_SET_VAL(glpDrmScreenEnd, DRM_SCREEN_END_FIELD1_LAST_LINE_MASK,
    gnVBlank + (gnOsdMaxHeight/2) - 1);

  gVideoStandard = video_standard;
  gnHBlankInitial = gnHBlank;
  gnVBlankInitial = gnVBlank;

  SetDisplayPosition(INITIAL_X_OFFSET, INITIAL_Y_OFFSET);

  vidSetHWBuffSize();
}

/********************************************************************/
/*  FUNCTION:    SetDisplayPosition                                 */
/*                                                                  */
/*  PARAMETERS:  iXoffset - Horizontal offset from default position */
/*               iYoffset - Vertical offset from default position   */
/*                                                                  */
/*  DESCRIPTION: Adjust the display origin by a small amount in the */
/*               vertical and horizontal directions.                */
/*                                                                  */
/*  RETURNS:     TRUE if offset is applied. FALSE if values are out */
/*               of bounds.                                         */
/*                                                                  */
/*  CONTEXT:     Must be called from a non-interrupt context        */
/*                                                                  */
/********************************************************************/
bool SetDisplayPosition(int iXoffset, int iYoffset)
{
   int iXClkOffset;
   bool ks;
   POSDLIBREGION pRgn;
   int iHBlankEnd, iVBlankEnd;

   /* Each pixel is represented by 2 clocks horizontally and we */
   /* must in 2 pixel increments to avoid colour shifts.        */

   /* With an external 861 encoder, we can shift by a single pixel by */
   /* manipulating the HSync Offset and HBlank Length together.  This */
   /* is accomplished by first moving the display by multiples of 2   */
   /* pixels using the OSD registers, then moving the display one     */
   /* pixel left with the encoder registers if necessary.  So for an  */
   /* external 861, we round up to a 2-bit shift and then shift one   */
   /* pixel left if the original iXoffset is odd.                     */
#if (INTERNAL_ENCODER == INTERNAL_BT861_LIKE)
   iXClkOffset = (iXoffset&~1) * 2;
#else
   if (gdwEncoderType == BT861)
     iXClkOffset = ((iXoffset+1)&~1) * 2;
   else
     iXClkOffset = (iXoffset&~1) * 2;
#endif

   /* Make sure we are passed valid values */
   if((abs(iXoffset) > MAX_SCREEN_POSITION_X_OFFSET) ||
      (abs(iYoffset) > MAX_SCREEN_POSITION_Y_OFFSET))
   {
     trace_new(OSD_MSG, "OSD: Attempt to set invalid screen offset (%d, %d). Max is (%d, %d)\n",
               iXoffset, iYoffset, gnHBlankInitial, gnVBlankInitial);
     return(FALSE);
   }

   /* Work out the new offset */
   gnHBlank = gnHBlankInitial+(u_int32)iXClkOffset;
   gnVBlank = gnVBlankInitial+(u_int32)iYoffset;
   iHBlankEnd = gnHBlank + (2*OSD_MAX_WIDTH)  - 1;
   iVBlankEnd = gnVBlank + (gnOsdMaxHeight/2) - 1;

   /* Restrict the overal size to be within the real size of video. */
   if ( (gVideoStandard == PAL) || (gVideoStandard == SECAM) )
   {
     if ( iHBlankEnd > 1720 )
     {
       iHBlankEnd = 1720;
     }
     if ( iVBlankEnd > 310 )
     {
       iVBlankEnd = 310;
     }
   }
   else if ( gVideoStandard == NTSC )
   {
     if ( iHBlankEnd > 1712 )
     {
       iHBlankEnd = 1712;
     }
     if ( iVBlankEnd > 262 )
     {
       iVBlankEnd = 262;
     }
   }

   /* Apply the new offset to the screen start and end registers*/
   ks = critical_section_begin();

   CNXT_SET_VAL(glpDrmScreenStart, DRM_SCREEN_START_FIRST_ACTIVE_PIXEL_MASK, gnHBlank);
   CNXT_SET_VAL(glpDrmScreenStart, DRM_SCREEN_START_FIELD2_ACTIVE_LINE_MASK, gnVBlank);
   CNXT_SET_VAL(glpDrmScreenStart, DRM_SCREEN_START_FIELD1_ACTIVE_LINE_MASK, gnVBlank);
   CNXT_SET_VAL(glpDrmScreenEnd, DRM_SCREEN_END_LAST_ACTIVE_PIXEL_MASK, iHBlankEnd);
   CNXT_SET_VAL(glpDrmScreenEnd, DRM_SCREEN_END_FIELD2_LAST_LINE_MASK, iVBlankEnd);
   CNXT_SET_VAL(glpDrmScreenEnd, DRM_SCREEN_END_FIELD1_LAST_LINE_MASK, iVBlankEnd);

   critical_section_end(ks);

   /* If the encoder is an external 861, write the HSync Delay and HBlank Length registers. */
#if (INTERNAL_ENCODER != INTERNAL_BT861_LIKE)
   if (gdwEncoderType == BT861)
   {
     if (iXoffset & 1)
     {
       gnHSync = gnHSyncInitial-2;
       iicWriteIndexedReg( gEncoderAddr, 0x0b, gnHBlankInitial&0xff, giicEncoderBus );
       iicWriteIndexedReg( gEncoderAddr, 0x0c, (gnHBlankInitial>>8)&3, giicEncoderBus );
     }
     else
     {
       gnHSync = gnHSyncInitial;
       iicWriteIndexedReg( gEncoderAddr, 0x0b, (gnHBlankInitial-ENCODER_PHASE_SHIFT)&0xff, giicEncoderBus );
       iicWriteIndexedReg( gEncoderAddr, 0x0c, ((gnHBlankInitial-ENCODER_PHASE_SHIFT)>>8)&3, giicEncoderBus );
     }
     iicWriteIndexedReg( gEncoderAddr, 0x10, gnHSync&0xff, giicEncoderBus );
     iicWriteIndexedReg( gEncoderAddr, 0x11, (gnHSync>>8)&3, giicEncoderBus );
   }
#endif

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

   return(TRUE);
}

/********************************************************************/
/*  FUNCTION:    GetDisplayPosition                                 */
/*                                                                  */
/*  PARAMETERS:  puXStart - Storage for returned X start position   */
/*               puYStart - Storage for returned Y start position   */
/*                                                                  */
/*  DESCRIPTION: Query the current X and Y start positions values   */
/*               for the display.                                   */
/*                                                                  */
/*  RETURNS:     TRUE if values are returned as expected. FALSE if  */
/*               a NULL pointer is passed.                          */
/*                                                                  */
/*  CONTEXT:     Must be called from a non-interrupt context        */
/*                                                                  */
/********************************************************************/
bool GetDisplayPosition(int *piXStart, int *piYStart)
{
  if(!piXStart || !piYStart)
  {
    trace_new(OSD_MSG, "OSD: NULL pointer passed to GetDisplayPosition\n");
    error_log(ERROR_WARNING);
    return(FALSE);
  }

  *piXStart = ((int)gnHBlank - (int)gnHBlankInitial)/2;
  *piYStart =  (int)gnVBlank - (int)gnVBlankInitial;

   /* If the encoder is an external 861, check the HSync Delay for a -1 offset. */
#if (INTERNAL_ENCODER != INTERNAL_BT861_LIKE)
   if (gdwEncoderType == BT861)
   {
     if ( gnHSync != gnHSyncInitial )
       *piXStart -= 1;
   }
#endif

  return(TRUE);
}

#if CUSTOMER == VENDOR_D
/*****************************************************************************/
/* Function: SetDisplayOutput() - VENDOR_D Version                           */
/*                                                                           */
/* Parameters: OSD_DISPLAY_OUTPUT odoType - Video output connection type.    */
/*                                                                           */
/* Returns: bool - TRUE on success.                                          */
/*                                                                           */
/* Description: Sets the video output connection.                            */
/*****************************************************************************/
extern void enable_rgb_outputs(bool bEnable);
extern bool set_output_mode_signalling(bool bUseRGB);

bool SetDisplayOutput(OSD_DISPLAY_OUTPUT odoType)
{
   if (odoType & OSD_OUTPUT_RGB)
   {
     /* RGB output. Turn on RGB SCART signal pins */
     set_output_mode_signalling(TRUE);
   }
   else
   {
     /* Composite or SVideo output. Turn off RGB SCART pins off */
     set_output_mode_signalling(FALSE);
   }

   gVideoOutput = odoType;
   return TRUE;
}

#else
#if INTERNAL_ENCODER == NOT_PRESENT
/*****************************************************************************/
/* Function: SetDisplayOutput() - Normal external Bt865/861 version          */
/*                                                                           */
/* Parameters: OSD_DISPLAY_OUTPUT odoType - Video output connection type.    */
/*                                                                           */
/* Returns: bool - TRUE on success.                                          */
/*                                                                           */
/* Description: Sets the video output connection.                            */
/*****************************************************************************/
bool SetDisplayOutput(OSD_DISPLAY_OUTPUT odoType)
{
   bool bRet = FALSE;
   BYTE cEnableDacs = 0;
   BYTE cReg24Val = 0x80;
   BYTE cReg25Val = 0x80;

   /********************************************************/
   /* Bt865 case - only 1 output type can be set at a time */
   /********************************************************/
   if(gdwEncoderType == BT865)
   {
     /* Composite and/or SVideo */
     if(odoType & (OSD_OUTPUT_COMPOSITE | OSD_OUTPUT_SVIDEO))
     {
       /* Composite or SVideo output. */
       iicWriteIndexedReg(I2C_ADDR_BT865, 0xCE, 0x02, giicEncoderBus);

       /* We can't support a request for CVBS and RGB simultaneously */
       odoType &= ~OSD_OUTPUT_RGB;
       bRet = TRUE;
     }
     else
     {
       /* RGB case */
       if(odoType & OSD_OUTPUT_RGB)
       {
         iicWriteIndexedReg(I2C_ADDR_BT865, 0xCE, 0x42, giicEncoderBus);
       }
     }

     /* If we were asked to set an output type, remember which */
     if(odoType)
      gVideoOutput = odoType;
   }
   else
   {
     /*******************************************************************************/
     /* Bt861 case - this encoder supports simultaneous RGB, SVIDEO and CVBS output */
     /*******************************************************************************/
     if(odoType & OSD_OUTPUT_COMPOSITE)
     {
       #ifdef OPENTV_12
       /* For OpenTV 1.2, we set SVideo in tandem with CVBS */
       cEnableDacs |= (VIDEO_ENCODER_CVBS_DAC_MASK | VIDEO_ENCODER_YC_DAC_MASK);
       #else
       /* Minor hack - enable both composite and RGB if asked for Composite only */
       /* Pending a fix to ensure SCART signalling is implemented in the OpenTV  */
       /* driver.                                                                */
       cEnableDacs |= (VIDEO_ENCODER_CVBS_DAC_MASK | VIDEO_ENCODER_RGB_DAC_MASK);
       #endif
     }

     if(odoType & OSD_OUTPUT_SVIDEO)
     {
       cEnableDacs |= VIDEO_ENCODER_YC_DAC_MASK;
     }

     if (odoType & OSD_OUTPUT_RGB)
     {
       /* Minor hack - enable both composite and RGB if asked for RGB only        */
       /* This prevents us from loosing video on SCART TVs that don't support RGB */
       /* when running some OpenTV VTS tests.                                     */
       cEnableDacs |= (VIDEO_ENCODER_CVBS_DAC_MASK | VIDEO_ENCODER_RGB_DAC_MASK);

       /* Set the multiplier register values according to the spec */
       if(gnOsdMaxHeight == 480)
       {
         cReg24Val = 0x90;
         cReg25Val = 0x66;
       }
     }

     /* If we were asked to set any mode, set up the encoder accordingly */
     if(odoType)
     {
       /* Set RGB multipliers correctly */
       iicWriteIndexedReg(gEncoderAddr, 0x23, 0x80,      giicEncoderBus);
       iicWriteIndexedReg(gEncoderAddr, 0x24, cReg24Val, giicEncoderBus);
       iicWriteIndexedReg(gEncoderAddr, 0x25, cReg25Val, giicEncoderBus);

       /* Turn on the relevant subset of DACs */
       iicWriteIndexedReg(gEncoderAddr, BT861_DAC_ENABLE_REG, cEnableDacs, giicEncoderBus);

       /* If we enabled RGB, set the chrominance filter into wide mode, else set to normal */
       iicReadIndexedReg(gEncoderAddr, 0x17, &cEnableDacs, giicEncoderBus);
       if(odoType & OSD_OUTPUT_RGB)
         cEnableDacs |= 0x80;
       else
         cEnableDacs &= ~0x80;
       iicWriteIndexedReg(gEncoderAddr, 0x17, cEnableDacs, giicEncoderBus);

       bRet = TRUE;

       gVideoOutput = odoType;
     }
   }

   return bRet;
}
#else
/*****************************************************************************/
/* Function: SetDisplayOutput() - Internal encoder version                   */
/*                                                                           */
/* Parameters: OSD_DISPLAY_OUTPUT odoType - Video output connection type.    */
/*                                                                           */
/* Returns: bool - TRUE on success.                                          */
/*                                                                           */
/* Description: Sets the video output connection.                            */
/*****************************************************************************/
bool SetDisplayOutput(OSD_DISPLAY_OUTPUT odoType)
{
   bool bRet = FALSE;
   BYTE cEnableDacs = 0;

   /*****************************************************************/
   /* The encoder supports simultaneous RGB, SVIDEO and CVBS output */
   /*****************************************************************/
   if(odoType & OSD_OUTPUT_COMPOSITE)
   {
     #ifdef OPENTV_12
     /* For OpenTV 1.2, we set SVideo in tandem with CVBS */
     cEnableDacs |= (VIDEO_ENCODER_CVBS_DAC_MASK | VIDEO_ENCODER_YC_DAC_MASK);
     #else
     /* Minor hack - enable both composite and RGB if asked for Composite only */
     /* Pending a fix to ensure SCART signalling is implemented in the OpenTV  */
     /* EN2 driver for Klondike/Bronco.                                        */
     cEnableDacs |= (VIDEO_ENCODER_CVBS_DAC_MASK | VIDEO_ENCODER_RGB_DAC_MASK);
     #endif
   }

   if(odoType & OSD_OUTPUT_SVIDEO)
   {
     cEnableDacs |= VIDEO_ENCODER_YC_DAC_MASK;
   }

   if (odoType & OSD_OUTPUT_RGB)
   {
     /* Minor hack - enable both composite and RGB if asked for RGB only        */
     /* This prevents us from losing video on SCART TVs that don't support RGB  */
     /* when running some OpenTV VTS tests.                                     */
     cEnableDacs |= (VIDEO_ENCODER_CVBS_DAC_MASK | VIDEO_ENCODER_RGB_DAC_MASK);
   }

   /* If we were asked to set any mode, set up the encoder accordingly */
   if(odoType)
   {
     /* Set the chrominance filter to wide mode if RGB is in use, else normal mode */
     CNXT_SET_VAL(ENC_CONTROL0_REG, ENC_CTL0_CHROMA_BW_MASK, ((odoType & OSD_OUTPUT_RGB)?1:0));
		//CNXT_SET_VAL(ENC_CONTROL0_REG, ENC_CTL0_CHROMA_BW_MASK, ENC_CTL0_CHROMA_BW_NORMAL);

		//CNXT_SET_VAL(ENC_CONTROL0_REG, ENC_CTL0_CROSS_FIL_MASK, ENC_CTL0_CROSS_FIL_APPLY);

		//CNXT_SET_VAL(ENC_CONTROL0_REG, ENC_CTL0_FIL_SEL_MASK, ENC_CTL0_FIL_SEL_ENABLE_REDUCT_FILTERS);

   		//CNXT_SET_VAL(ENC_CONTROL0_REG, ENC_CTL0_PKFIL_SEL_MASK, ENC_CTL0_PKFIL_SEL_FILTER4);

     /* Turn on the relevant subset of DACs */
     CNXT_SET_VAL(ENC_CONTROL0_REG, ENC_CTL0_ALL_DAC_MASK, cEnableDacs);

     bRet = TRUE;

     gVideoOutput = odoType;
   }

   return bRet;
}
#endif /* INTERNAL_ENCODER */
#endif /* Board- and customer-specific implementations */

/*****************************************************************************/
/* Function: GetDisplayOutput()                                              */
/*                                                                           */
/* Parameters: void                                                          */
/*                                                                           */
/* Returns: OSD_DISPLAY_OUTPUT - The output connection types.                */
/*                                                                           */
/* Description: Returns the video output connection types.                   */
/*****************************************************************************/
OSD_DISPLAY_OUTPUT GetDisplayOutput(void)
{
   OSD_DISPLAY_OUTPUT odoRet;

   #if CUSTOMER == VENDOR_D
   if(gVideoOutput != OSD_OUTPUT_SVIDEO)
     odoRet = OSD_OUTPUT_COMPOSITE | OSD_OUTPUT_RGB;
   else
     odoRet = OSD_OUTPUT_SVIDEO;
   #else

//   if (gVideoOutput != OSD_OUTPUT_RGB)
//      odoRet = OSD_OUTPUT_COMPOSITE | OSD_OUTPUT_SVIDEO;
//   else
//      odoRet = OSD_OUTPUT_RGB;
   odoRet = gVideoOutput;
   #endif

   return odoRet;
}

/**************************/
/**************************/
/**                      **/
/**   Public Functions   **/
/**                      **/
/**************************/
/**************************/


/********************************************************************/
/*  FUNCTION:    cnxt_encoder_init                                  */
/*                                                                  */
/*  PARAMETERS:  pCfg - Pointer to driver configuration structure.  */
/*                                                                  */
/*  DESCRIPTION: This function initialises the driver and makes it  */
/*               ready for use.                                     */
/*                                                                  */
/*  RETURNS:     CNXT_ENCODER_OK                                    */
/*               CNXT_ENCODER_ALREADY_INIT                          */
/*                                                                  */
/*  CONTEXT:     May be called in any context.                      */
/*                                                                  */
/********************************************************************/
CNXT_ENCODER_STATUS cnxt_encoder_init ( CNXT_ENCODER_CONFIG *pCfg )
{
  static bool ReEntry = FALSE;
  bool bKs;
  #if (INTERNAL_ENCODER == NOT_PRESENT)
  BYTE byVal;
  #endif
  
  /* This function should take the video standard in the pCfg function */
  /* and do the stuff that is currently done in the SetupXxxxEncoder   */
  /* functions above.                                                  */
  
  /* Currently, the assumption is made that SetOutputType has been     */
  /* called prior to this function so the encoder address and type     */
  /* has been determined.                                              */

   
  /* Need to ensure that this routine only called once, so     */
  /* start crit section, test the semaphore for existence.     */
  /* If not exist, then create.                                */
  /* get out of the critical section, and see if the semaphore */
  /* exists. If it doesn't, then return an error.              */
  /* Now get the semaphore. If this fails then                 */
  /* someone else grapped the semaphore, or there is a bigger  */
  /* system resource problem                                   */
   
  while (1)
  {
     bKs = critical_section_begin();
     if ( ReEntry == FALSE )
     {
        ReEntry = TRUE;
        critical_section_end(bKs);
        break;
     }
     critical_section_end(bKs);
     task_time_sleep(5);
  }

  if (pDriverInst->DriverSem == 0)
  {
     pDriverInst->DriverSem = sem_create(1, NULL);
  }

  if (pDriverInst->DriverSem == 0)
  {
     ReEntry = FALSE;
     return CNXT_ENCODER_RESOURCE_ERROR;
  }

  if (sem_get(pDriverInst->DriverSem, KAL_WAIT_FOREVER) != RC_OK)
  {
     ReEntry = FALSE;
     return CNXT_ENCODER_INTERNAL_ERROR;
  }

  if (pDriverInst->bInit == TRUE)
  {
     sem_put(pDriverInst->DriverSem);
     ReEntry = FALSE;
     return CNXT_ENCODER_ALREADY_INIT;
  }

  /* Initialise our internal data structures */
  private_encoder_internal_data_init();
  
  /* connect internal data to the driver instance structure */
  pDriverInst->pInstList = InstArray;
  pDriverInst->pUnitInst  = UnitInst;
   
  /* If we are using a Bt865, we can't control brightness, */
  /* contrast, hue or saturation so we need to update the  */
  /* capabilities structure to reflect this.               */
  #if (INTERNAL_ENCODER == NOT_PRESENT)
  #if (EXTERNAL_ENCODER == DETECT_BT861_BT865) || \
      (EXTERNAL_ENCODER == BT865)              || \
      (EXTERNAL_ENCODER == BT861)
  /* Read the status register and decide if it is an 865 or 861 */
  iicReadIndexedReg(gEncoderAddr, 0x00, &byVal, giicEncoderBus);

  if (((byVal & 0xE0) == (4 << 5)) || ((byVal & 0xE0) == (5 << 5)))
  {
    /* We are talking to a Bt865 so fix up the caps structure */
    pDriverInst->pUnitInst[0].pCaps->uFlags = 0;
  }
  #endif /* EXTERNAL_DECODER may be an 865 */
  #endif /* INTERNAL_DECODER == NOT_PRESENT */
  
  /* Flag that we are initialised, release the semaphore and go home */
  pDriverInst->bInit = TRUE; 
  sem_put(pDriverInst->DriverSem); 
  ReEntry = FALSE;
  
  return(CNXT_ENCODER_OK);
}

/********************************************************************/
/*  FUNCTION:    cnxt_encoder_term                                  */
/*                                                                  */
/*  PARAMETERS:  None                                               */
/*                                                                  */
/*  DESCRIPTION: This function shuts down the driver, closing any   */
/*               open handles in the process.                       */
/*                                                                  */
/*  RETURNS:     CNXT_ENCODER_OK                                    */
/*               CNXT_ENCODER_NOT_INIT                              */
/*                                                                  */
/*  CONTEXT:     Must be called in task context.                    */
/*                                                                  */
/********************************************************************/
CNXT_ENCODER_STATUS cnxt_encoder_term ( void )
{
   IS_DRIVER_INITED(ENCODER, pDriverInst->bInit);
   
   if (sem_get(pDriverInst->DriverSem, KAL_WAIT_FOREVER) != RC_OK)
      return CNXT_ENCODER_INTERNAL_ERROR;

   /* Mark the driver as not initialized */
   pDriverInst->bInit = FALSE;

   /* Release the semaphore */
   sem_put(pDriverInst->DriverSem);

   /* Destroy semaphore */
   sem_delete(pDriverInst->DriverSem);
   pDriverInst->DriverSem = 0;

   return CNXT_ENCODER_OK;
}

/********************************************************************/
/*  FUNCTION:    cnxt_encoder_get_units                             */
/*                                                                  */
/*  PARAMETERS:  puCount - storage for returned unit count          */
/*                                                                  */
/*  DESCRIPTION: This function returns the number of encoders in    */
/*               the system.                                        */
/*                                                                  */
/*  RETURNS:     CNXT_ENCODER_OK                                    */
/*               CNXT_ENCODER_NOT_INIT                              */
/*               CNXT_ENCODER_BAD_PTR                               */
/*                                                                  */
/*  CONTEXT:     Must be called in task context.                    */
/*                                                                  */
/********************************************************************/
CNXT_ENCODER_STATUS cnxt_encoder_get_units ( u_int32 *puCount )
{
  IS_DRIVER_INITED(ENCODER,pDriverInst->bInit);
  IS_NOT_NULL_POINTER(ENCODER, puCount);
  
  *puCount = CNXT_ENCODER_NUM_UNITS;
  return(CNXT_ENCODER_OK);
}


/********************************************************************/
/*  FUNCTION:    cnxt_encoder_get_unit_caps                         */
/*                                                                  */
/*  PARAMETERS:  uUnitNumber - the unit whose capabilities are to   */
/*                             be returned. Unit number is 0-based. */
/*               pCaps       - pointer to storage for the returned  */
/*                             capabilities. Caller must complete   */
/*                             the uLength field prior to making    */
/*                             the call.                            */
/*                                                                  */
/*  DESCRIPTION: This function returns information on the           */
/*               capabilities of one of the video encoders in the   */
/*               system.                                            */
/*                                                                  */
/*  RETURNS:     CNXT_ENCODER_OK                                    */
/*               CNXT_ENCODER_NOT_INIT                              */
/*               CNXT_ENCODER_BAD_UNIT                              */
/*               CNXT_ENCODER_BAD_PTR                               */
/*                                                                  */
/*  CONTEXT:     Must be called in task context.                    */
/*                                                                  */
/********************************************************************/
CNXT_ENCODER_STATUS cnxt_encoder_get_unit_caps ( u_int32            uUnitNumber, 
                                                 CNXT_ENCODER_CAPS *pCaps )
{
   u_int32 uLength = pCaps->uLength;

   IS_DRIVER_INITED(ENCODER, pDriverInst->bInit);
   IS_NOT_NULL_POINTER(ENCODER, pCaps);

   /* Check that the module requested exists */
   if (uUnitNumber >= CNXT_ENCODER_NUM_UNITS)
   {
      return CNXT_ENCODER_BAD_UNIT;
   }

   /* Check that the passed length was not zero */
   if (uLength == 0)
   {
      return CNXT_ENCODER_BAD_PARAMETER;   
   }

   /* Copy the caps structure to the clients buffer with length protection */
   uLength = min(uLength, sizeof(CNXT_ENCODER_CAPS));
   memcpy(pCaps, pDriverInst->pUnitInst[uUnitNumber].pCaps, uLength);
   pCaps->uLength = uLength;

   return CNXT_ENCODER_OK;
}                                                 

/********************************************************************/
/*  FUNCTION:    cnxt_encoder_open                                  */
/*                                                                  */
/*  PARAMETERS:  pHandle - pointer to storage for returned handle   */
/*               pCaps   - capabilities required of encoder to be   */
/*                         opened.                                  */
/*               pNotifyFn - callback (ignored)                     */
/*               pUserData - user data value (ignored)              */
/*                                                                  */
/*  DESCRIPTION: This function creates a new driver instance        */
/*               meeting the requirements as provided in the pCaps  */
/*               structure. If successful, a handle is returned in  */
/*               *pHandle.                                          */
/*                                                                  */
/*  RETURNS:     CNXT_ENCODER_OK                                    */
/*               CNXT_ENCODER_NOT_INIT                              */
/*               CNXT_ENCODER_BAD_UNIT                              */
/*               CNXT_ENCODER_BAD_PTR                               */
/*                                                                  */
/*  CONTEXT:     Must be called in task context.                    */
/*                                                                  */
/********************************************************************/
CNXT_ENCODER_STATUS cnxt_encoder_open ( CNXT_ENCODER_HANDLE    *pHandle,
                                        CNXT_ENCODER_CAPS      *pCaps,
                                        CNXT_ENCODER_PFNNOTIFY pNotifyFn,
                                        void                   *pUserData )
{
   CNXT_ENCODER_INST *pInst;
   CNXT_ENCODER_UNIT_INST *pUnitInst;
   u_int32 uUnit;

   trace_new(TRACE_ENCODER, "ENCODER: Opening...\n");
   
   IS_DRIVER_INITED(ENCODER,pDriverInst->bInit);
   IS_NOT_NULL_POINTER(ENCODER, pHandle);
   IS_NOT_NULL_POINTER(ENCODER, pCaps);

   if (sem_get(pDriverInst->DriverSem, KAL_WAIT_FOREVER) != RC_OK)
   {
      return CNXT_ENCODER_INTERNAL_ERROR;
   }

   /* Figure out unit number based on info in pCaps */
   uUnit = private_encoder_get_unit_num(pCaps);

   /* Check bExclusive */
   pUnitInst = &(pDriverInst->pUnitInst[uUnit]);
   if ((pUnitInst->pFirstInst != NULL) && 
       (pUnitInst->bExclusive || pCaps->bExclusive))
   {
      sem_put(pDriverInst->DriverSem); 
      return(CNXT_ENCODER_RESOURCE_ERROR);
   }

   /* Create an instance */
   if ( !CREATE_HANDLE(&(pDriverInst->pInstList), &pInst) )
   {
      *pHandle = NULL;
      sem_put(pDriverInst->DriverSem);
      return(CNXT_ENCODER_RESOURCE_ERROR);
   }

   /* add the instance into the list */
   pInst->uUnitNumber = uUnit;
   if (ADD_HANDLE (&pUnitInst->pFirstInst, pInst) == FALSE)
   {
      DESTROY_HANDLE(&(pDriverInst->pInstList), pInst);
      sem_put(pDriverInst->DriverSem); 
      return(CNXT_ENCODER_INTERNAL_ERROR);
   }

   pInst->Preface.pSelf = (CNXT_HANDLE_PREFACE*)pInst;
   pInst->pNotifyFn = pNotifyFn;    /* Use this fcn to notify appl of events */
   pInst->pUserData = pUserData;    /* Store data the inst needs */

   /* set driver bExclusive field */
   pUnitInst->bExclusive = pCaps->bExclusive;

   *pHandle = (CNXT_ENCODER_HANDLE)pInst;

   trace_new(TRACE_ENCODER, "ENCODER: New handle returned 0x%08x\n", *pHandle);

   sem_put(pDriverInst->DriverSem); 
   return(CNXT_ENCODER_OK);
}

/********************************************************************/
/*  FUNCTION:    cnxt_encoder_close                                 */
/*                                                                  */
/*  PARAMETERS:  Handle - a handle returned on a previous call to   */
/*                        cnxt_encoder_open.                        */
/*                                                                  */
/*  DESCRIPTION: This function closes a handle freeing up any       */
/*               resources associated with it.                      */
/*                                                                  */
/*  RETURNS:     CNXT_ENCODER_OK                                    */
/*               CNXT_ENCODER_NOT_INIT                              */
/*               CNXT_ENCODER_BAD_HANDLE                            */
/*                                                                  */
/*  CONTEXT:     Must be called in task context.                    */
/*                                                                  */
/********************************************************************/
CNXT_ENCODER_STATUS cnxt_encoder_close ( CNXT_ENCODER_HANDLE Handle )
{
   CNXT_ENCODER_INST *pInst = (CNXT_ENCODER_INST*)Handle;
   CNXT_ENCODER_UNIT_INST *pUnitInst;

   trace_new(TRACE_ENCODER, "ENCODER: Closing handle 0x%08x\n", Handle);
   
   IS_DRIVER_INITED(ENCODER,pDriverInst->bInit);
   IS_VALID_HANDLE(ENCODER, pInst, &(pDriverInst->pUnitInst[pInst->uUnitNumber].pFirstInst));
   
   pUnitInst = &(pDriverInst->pUnitInst[pInst->uUnitNumber]);
   if (sem_get(pDriverInst->DriverSem, KAL_WAIT_FOREVER) != RC_OK)
   {
      return CNXT_ENCODER_INTERNAL_ERROR;
   }

   /* release the resource of the instance */
   if (REMOVE_HANDLE(&pUnitInst->pFirstInst, pInst) == FALSE)
   {
      sem_put(pDriverInst->DriverSem); /* must be last line of routine! */
      return CNXT_ENCODER_INTERNAL_ERROR;
   }

   DESTROY_HANDLE(&(pDriverInst->pInstList), pInst);

   /* set the unit to be shared if no instance opened for the unit */
   if ( pUnitInst->pFirstInst == NULL )
   {
      pUnitInst->bExclusive = FALSE;
   }

   sem_put(pDriverInst->DriverSem); /* must be last line of routine! */
   return CNXT_ENCODER_OK;
}

/********************************************************************/
/*  FUNCTION:    cnxt_encoder_set_picture_controls                  */
/*                                                                  */
/*  PARAMETERS:  Handle - a handle returned on a previous call to   */
/*                        cnxt_encoder_open.                        */
/*               pControls - pointer to a structure containing      */
/*                        brightness, contrast, saturation and hue  */
/*                        values to set.                            */
/*                                                                  */
/*  DESCRIPTION: This function allows the caller to change the      */
/*               current settings of brightness, contrast, hue and  */
/*               saturation assuming the connected video encoder    */
/*               supports these.                                    */
/*                                                                  */
/*  RETURNS:     CNXT_ENCODER_OK                                    */
/*               CNXT_ENCODER_NOT_INIT                              */
/*               CNXT_ENCODER_BAD_HANDLE                            */
/*                                                                  */
/*  CONTEXT:     Must be called in task context.                    */
/*                                                                  */
/********************************************************************/
CNXT_ENCODER_STATUS cnxt_encoder_set_picture_controls( CNXT_ENCODER_HANDLE Handle, CNXT_ENCODER_CONTROLS *pControls )
{
   CNXT_ENCODER_INST *pInst = (CNXT_ENCODER_INST*)Handle;
   CNXT_ENCODER_STATUS eRetcode;

   trace_new(TRACE_ENCODER, "ENCODER: Setting picture controls for handle 0x%08x\n", Handle);
   
   IS_DRIVER_INITED(ENCODER,pDriverInst->bInit);
   IS_NOT_NULL_POINTER(ENCODER, pControls);
   IS_VALID_HANDLE(ENCODER, pInst, &(pDriverInst->pUnitInst[pInst->uUnitNumber].pFirstInst));

   if (sem_get(pDriverInst->DriverSem, KAL_WAIT_FOREVER) != RC_OK)
   {
      return CNXT_ENCODER_INTERNAL_ERROR;
   }

   /* Set the controls if possible */
   eRetcode = private_set_encoder_controls(pControls);
   
   sem_put(pDriverInst->DriverSem); 
   return(eRetcode);
}

/********************************************************************/
/*  FUNCTION:    cnxt_encoder_get_picture_controls                  */
/*                                                                  */
/*  PARAMETERS:  Handle - a handle returned on a previous call to   */
/*                        cnxt_encoder_open.                        */
/*               pControls - pointer to a structure to receive the  */
/*                        current brightness, contrast, saturation  */
/*                        and hue settings.                         */
/*                                                                  */
/*  DESCRIPTION: This function allows the caller to query the       */
/*               current settings of brightness, contrast, hue and  */
/*               saturation assuming the connected video encoder    */
/*               supports these. The uFlags field in the pControls  */
/*               structure is set to show which parameters may be   */
/*               set by a call to cnxt_encoder_set_picture_controls */
/*                                                                  */
/*  RETURNS:     CNXT_ENCODER_OK                                    */
/*               CNXT_ENCODER_NOT_INIT                              */
/*               CNXT_ENCODER_BAD_HANDLE                            */
/*                                                                  */
/*  CONTEXT:     Must be called in task context.                    */
/*                                                                  */
/********************************************************************/
CNXT_ENCODER_STATUS cnxt_encoder_get_picture_controls( CNXT_ENCODER_HANDLE Handle, CNXT_ENCODER_CONTROLS *pControls )
{
   CNXT_ENCODER_INST *pInst = (CNXT_ENCODER_INST*)Handle;
   CNXT_ENCODER_STATUS eRetcode;

   trace_new(TRACE_ENCODER, "ENCODER: Getting picture controls for handle 0x%08x\n", Handle);
   
   IS_DRIVER_INITED(ENCODER,pDriverInst->bInit);
   IS_NOT_NULL_POINTER(ENCODER, pControls);
   IS_VALID_HANDLE(ENCODER, pInst, &(pDriverInst->pUnitInst[pInst->uUnitNumber].pFirstInst));

   if (sem_get(pDriverInst->DriverSem, KAL_WAIT_FOREVER) != RC_OK)
   {
      return CNXT_ENCODER_INTERNAL_ERROR;
   }

   /* Get the controls if possible */
   eRetcode = private_get_encoder_controls(pControls);

   sem_put(pDriverInst->DriverSem); /* must be last line of routine! */
   return(eRetcode);
}

/********************************************************************/
/*  FUNCTION:    private_encoder_internal_data_init                 */
/*                                                                  */
/*  PARAMETERS:  None                                               */
/*                                                                  */
/*  DESCRIPTION: Initialise the internal data used by the driver    */
/*                                                                  */
/*  RETURNS:     Nothing                                            */
/*                                                                  */
/*  CONTEXT:     May be called in any context.                      */
/*                                                                  */
/********************************************************************/
static void private_encoder_internal_data_init(void)
{
   u_int32 i;

   /* Loop through the array of available Inst to set them to */
   /* unused, and make the list pointer invalid.              */
   
   for (i = 0; i < CNXT_ENCODER_MAX_HANDLES; ++i)
   {
      bInstUsed[i] = FALSE;
      InstArray[i].Preface.pSelf = NULL;
      InstArray[i].Preface.pNext = &InstArray[i+1].Preface;   
   }

   /* Terminate the linked list of instance structures. */
   InstArray[CNXT_ENCODER_MAX_HANDLES-1].Preface.pNext = NULL;

   /* Initialize UnitInst */
   for (i = 0; i < CNXT_ENCODER_NUM_UNITS; ++i)
   {
      UnitInst[i].pFirstInst = NULL;  /* list of handles is empty */
      UnitInst[i].bExclusive = FALSE;
      UnitInst[i].pCaps      = &CapsArray[i];
   }
}


/********************************************************************/
/*  FUNCTION:    private_encoder_get_unit_num                       */
/*                                                                  */
/*  PARAMETERS:  pCaps - pointer to capabilities structure for the  */
/*                       unit which is to be opened.                */
/*                                                                  */
/*  DESCRIPTION: Given a capabilities structure, this function      */
/*               (which is trivial for the single unit case here)   */
/*               determines which unit offers the required          */
/*               capabilities and returns the unit index.           */
/*                                                                  */
/*  RETURNS:     Index of unit offering the supplied capabilities.  */
/*                                                                  */
/*  CONTEXT:     May be called in any context.                      */
/*                                                                  */
/********************************************************************/
static u_int32 private_encoder_get_unit_num(CNXT_ENCODER_CAPS *pCaps)
{
   return(0);
}

#if INTERNAL_ENCODER == NOT_PRESENT
/********************************************************************/
/*  FUNCTION:    private_get_encoder_controls (I2C encoder version) */
/*                                                                  */
/*  PARAMETERS:  pControls - pointer to structure to receive the    */
/*                           current control settings.              */
/*                                                                  */
/*  DESCRIPTION: Read the current contrast, brightness, hue and     */
/*               saturation values from the video decoder and       */
/*               return this in the structure provided.             */
/*                                                                  */
/*  RETURNS:     CNXT_ENCODER_OK                                    */
/*                                                                  */
/*  CONTEXT:    Must be called in task context.                     */
/*                                                                  */
/********************************************************************/
static CNXT_ENCODER_STATUS private_get_encoder_controls( CNXT_ENCODER_CONTROLS *pControls )
{
  u_int8 uValue;
  int    iValue;  
  
  pControls->uFlags = pDriverInst->pUnitInst[0].pCaps->uFlags;
  
  /* Get brightness from the device (if supported) */
  if(pControls->uFlags & ENCODER_CONTROL_BRIGHTNESS)
  {
    iicReadIndexedReg(gEncoderAddr,
                      BT861_BRIGHTNESS_REGISTER_INDEX,
                      &uValue,
                      giicEncoderBus);
    
    /* Rescale brightness from 0xFF - 0x7F range to 0x0 - 0xFF */
    
    /* NOTE: Bt861 spec is wrong - it suggests that is a 5 bit signed number but */
    /* it appears to be 8 bit after all.                                         */
    
    iValue = (int)((int8)uValue) + 128; /* Range now 0 to 255 */
    pControls->ucBrightness = (u_int8)iValue;    /* Convert back to unsigned char size */
  }  
  else
    pControls->ucBrightness = DEFAULT_BRIGHTNESS;
  
  /* Get contrast from the device (if supported) */
  if(pControls->uFlags & ENCODER_CONTROL_CONTRAST)
    iicReadIndexedReg(gEncoderAddr,
                      BT861_CONTRAST_REGISTER_INDEX,
                      &(pControls->ucContrast),
                      giicEncoderBus);
  else
    pControls->ucContrast = DEFAULT_CONTRAST;

  /* Get hue from the device (if supported) */
  if(pControls->uFlags & ENCODER_CONTROL_HUE)
  {
    #if 0 
    /* this is the old way, the value returned is not always the same as the one passed to the _set function */
    
    /* The value 0 is mid-range for Hue in the Bt861 and the effect of small changes */
    /* is very apparent. As a result, we rescale so that range [-16,15] read from    */
    /* the device is translated to range [0-255] reported to the caller.             */
    iicReadIndexedReg(gEncoderAddr,
                      BT861_HUE_REGISTER_INDEX,
                      &uValue,
                      giicEncoderBus);
    pControls->ucHue = (u_int8)((int8)uValue + 16) << 3;
    #else
    /* this is the temperate new way, return the value that was passed to the _set function */
    pControls->ucHue = uSavedHue;
    #endif
  }  
  else
    pControls->ucHue = DEFAULT_HUE;
  
  /* None of the supported encoders offer single-register saturation */
  /* controls so let's not offer this for the time being!            */
  pControls->ucSaturation = DEFAULT_SATURATION;
  
  return(CNXT_ENCODER_OK);  
}
#else
/********************************************************************/
/*  FUNCTION:    private_get_encoder_controls (internal encoder)    */
/*                                                                  */
/*  PARAMETERS:  pControls - pointer to structure to receive the    */
/*                           current control settings.              */
/*                                                                  */
/*  DESCRIPTION: Read the current contrast, brightness, hue and     */
/*               saturation values from the video decoder and       */
/*               return this in the structure provided.             */
/*                                                                  */
/*  RETURNS:     CNXT_ENCODER_OK                                    */
/*                                                                  */
/*  CONTEXT:    Must be called in task context.                     */
/*                                                                  */
/********************************************************************/
static CNXT_ENCODER_STATUS private_get_encoder_controls( CNXT_ENCODER_CONTROLS *pControls )
{
  u_int32 uRegVal;
  /*u_int8  uValue;*/
  int     iValue;  
  
  pControls->uFlags = pDriverInst->pUnitInst[0].pCaps->uFlags;
  
  /* Get brightness from the device (if supported) */
  if(pControls->uFlags & ENCODER_CONTROL_BRIGHTNESS)
  {
    uRegVal = CNXT_GET_VAL(ENC_HUE_BRIGHT_CTL_REG, ENC_HUEBRICTL_YOFF_MASK);
    
    /* Rescale brightness from 0xFF - 0x7F range to 0x0 - 0xFF */
    
    /* NOTE: Bt861 spec is wrong - it suggests that is a 5 bit signed number but */
    /* it appears to be 8 bit after all.                                         */
    
    iValue = (int)((int8)uRegVal) + 128; /* Range now 0 to 255 */
    pControls->ucBrightness = (u_int8)iValue;    /* Convert back to unsigned char size */
  }  
  else
    pControls->ucBrightness = DEFAULT_BRIGHTNESS;
  
  /* Get contrast from the device (if supported) */
  if(pControls->uFlags & ENCODER_CONTROL_CONTRAST)
    pControls->ucContrast = (u_int8)CNXT_GET_VAL(ENC_YCRCB_MULTIPLICATION_REG, ENC_YUV_MULT_MCOMPY_MASK);
  else
    pControls->ucContrast = DEFAULT_CONTRAST;

  /* Get hue from the device (if supported) */
  if(pControls->uFlags & ENCODER_CONTROL_HUE)
  {
    #if 0 
    /* this is the old way, the value returned is not always the same as the one passed to the _set function */
    
    /* The value 0 is mid-range for Hue in the Bt861 and the effect of small changes */
    /* is very apparent. As a result, we rescale so that range [-16,15] read from    */
    /* the device is translated to range [0-255] reported to the caller.             */
    uValue = (u_int8)CNXT_GET_VAL(ENC_HUE_BRIGHT_CTL_REG, ENC_HUEBRICTL_HUEADJ_MASK);
    pControls->ucHue = (u_int8)((int8)uValue + 16) << 3;
    #else
    /* this is the temperate new way, return the value that was passed to the _set function */
    pControls->ucHue = uSavedHue;
    #endif
  }  
  else
    pControls->ucHue = DEFAULT_HUE;
  
  /* None of the supported encoders offer single-register saturation */
  /* controls so let's not offer this for the time being!            */
  pControls->ucSaturation = DEFAULT_SATURATION;
  
  return(CNXT_ENCODER_OK);  
}
#endif

/* See bottom of this file for program to generate the following tables */
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

#if INTERNAL_ENCODER == NOT_PRESENT
/********************************************************************/
/*  FUNCTION:    private_set_encoder_controls (I2C encoder version) */
/*                                                                  */
/*  PARAMETERS:  pControls - pointer to structure containing        */
/*                           the control values to set.             */
/*                                                                  */
/*  DESCRIPTION: Set the current contrast, brightness, hue and      */
/*               saturation values.                                 */
/*                                                                  */
/*  RETURNS:     CNXT_ENCODER_OK                                    */
/*               CNXT_ENCODER_NOT_INIT                              */
/*                                                                  */
/*  CONTEXT:    Must be called in task context.                     */
/*                                                                  */
/********************************************************************/
static CNXT_ENCODER_STATUS private_set_encoder_controls( CNXT_ENCODER_CONTROLS *pControls )
{
  int iValue;
  
  /* Set brightness (if requested) */
  if(pControls->uFlags & ENCODER_CONTROL_BRIGHTNESS)
  {
    /* Brightness is odd - it ranges from 0x10 (min) to 0x0F (max) */
    /* with 0 as nominal. Rescale our 0-255 value as appropriate.  */
    iValue = (int)pControls->ucBrightness - 128;
    iicWriteIndexedReg(gEncoderAddr,
                       BT861_BRIGHTNESS_REGISTER_INDEX,
                       (u_int8)iValue,
                       giicEncoderBus);   
  }
  
  /* Set contrast (if requested) */
  if(pControls->uFlags & ENCODER_CONTROL_CONTRAST)
    iicWriteIndexedReg(gEncoderAddr,
                       BT861_CONTRAST_REGISTER_INDEX,
                       pControls->ucContrast,
                       giicEncoderBus);   
    

  /* Set hue (if requested) */
  if(pControls->uFlags & ENCODER_CONTROL_HUE)
  {
    /* Method 1: Adjust the subcarrier phase, effective for composite and svideo signals */
    /* Method 2: uses MULT_UU,MULT_VU, MULT_UV, MULT_VV in register Chroma control to    */
    /*           matrix multiply the color vectors, effective for RGB signal in non-PAL mode */ 
    if((gVideoOutput & OSD_OUTPUT_COMPOSITE) || (gVideoOutput & OSD_OUTPUT_SVIDEO))  
    {
      /* set the registers used by other method to their default values  */
      iicWriteIndexedReg(gEncoderAddr,
                         BT861_MULT_UU_REGISTER_INDEX,
                         DEFAULT_MULT_UU,
                         giicEncoderBus);
      iicWriteIndexedReg(gEncoderAddr,
                         BT861_MULT_VU_REGISTER_INDEX,
                         DEFAULT_MULT_VU,
                         giicEncoderBus);
      iicWriteIndexedReg(gEncoderAddr,
                         BT861_MULT_UV_REGISTER_INDEX,
                         DEFAULT_MULT_UV,
                         giicEncoderBus);
      iicWriteIndexedReg(gEncoderAddr,
                         BT861_MULT_VV_REGISTER_INDEX,
                         DEFAULT_MULT_VV,
                         giicEncoderBus);
      /* The value 0 is mid-range for Hue in the Bt861 and the effect of small changes */
      /* is very apparent. As a result, we rescale so that range [0,255] passed to     */
      /* this function is translated to range [-16,15] set in the device.              */
      iValue = (u_int8)((int8)(pControls->ucHue >> 3) - 16);
      iicWriteIndexedReg(gEncoderAddr,
                         BT861_HUE_REGISTER_INDEX,
                         (u_int8)iValue,
                         giicEncoderBus);
      /* save the hue setting for the get function to return */
      uSavedHue = pControls->ucHue;
    } 
    else /* RGB signals */
    { 
      if(gVideoStandard != PAL) 
      {
        /* The use of sin & cos are breaking in PSOS builds.  Aside from that, there are a
         * plethora of reasons for not using these functions the biggest of which are:
         * 1.  the binary code for sin and cos are bigger than the lookup table itself 
         *     (assuming 8 bits in and 8 bits out)
         * 2.  its slow slow slow slow slow.  And did I mention slow?
         */

#if USE_MATH==1
        double dCos, dSin;
        const double dDegToRad = 0.02; /* 0.017453 */
#endif
        u_int8 uVU, uUV, uU_V;
        
        /* set the registers used by other method to their default values  */
        iicWriteIndexedReg(gEncoderAddr,
                           BT861_HUE_REGISTER_INDEX,
                           0,
                           giicEncoderBus);
      
#if USE_MATH==1
        dCos = -128*cos((double)(pControls->ucHue)*dDegToRad);
        dSin = -128*sin((double)(pControls->ucHue)*dDegToRad);
        
        uU_V = ~((int8)(dCos+0.5)) + 1 ;  if (uU_V == 128) uU_V = 127;
        uVU = ~(-(int8)(dSin+0.5)) + 1; if (uVU == 128) uVU = 127;
        uUV = ~((int8)(dSin+0.5)) + 1; if (uUV == 128) uUV = 127;
#else
        pControls->ucHue &= 0xff;
        uU_V = uU_V_table[pControls->ucHue];
        uVU = uVU_table[pControls->ucHue];
        uUV = uUV_table[pControls->ucHue];
#endif

        iicWriteIndexedReg(gEncoderAddr,
                           BT861_MULT_UU_REGISTER_INDEX,
                           uU_V,
                           giicEncoderBus);
        iicWriteIndexedReg(gEncoderAddr,
                           BT861_MULT_VU_REGISTER_INDEX,
                           uVU,
                           giicEncoderBus);
        iicWriteIndexedReg(gEncoderAddr,
                           BT861_MULT_UV_REGISTER_INDEX,
                           uUV,
                           giicEncoderBus);
        iicWriteIndexedReg(gEncoderAddr,
                           BT861_MULT_VV_REGISTER_INDEX,
                           uU_V,
                           giicEncoderBus);
        /* save the hue setting for the get function to return */
        uSavedHue = pControls->ucHue;
      }
      else   /* RGB signal & PAL mode*/
      {
        trace_new(OSD_ERROR_MSG, "OSD: Cannot adjust hue in RGB signal for PAL mode!");
        error_log(ERROR_WARNING);
        return(CNXT_ENCODER_UNSUPPORTED);
      }
    }  
                         
  }
  return(CNXT_ENCODER_OK);  
}
#else
/********************************************************************/
/*  FUNCTION:    private_set_encoder_controls (internal encoder)    */
/*                                                                  */
/*  PARAMETERS:  pControls - pointer to structure containing        */
/*                           the control values to set.             */
/*                                                                  */
/*  DESCRIPTION: Set the current contrast, brightness, hue and      */
/*               saturation values.                                 */
/*                                                                  */
/*  RETURNS:     CNXT_ENCODER_OK                                    */
/*               CNXT_ENCODER_NOT_INIT                              */
/*                                                                  */
/*  CONTEXT:    Must be called in task context.                     */
/*                                                                  */
/********************************************************************/
static CNXT_ENCODER_STATUS private_set_encoder_controls( CNXT_ENCODER_CONTROLS *pControls )
{
  int iValue;
  
  /* Set brightness (if requested) */
  if(pControls->uFlags & ENCODER_CONTROL_BRIGHTNESS)
  {
    /* Brightness is odd - it ranges from 0x10 (min) to 0x0F (max) */
    /* with 0 as nominal. Rescale our 0-255 value as appropriate.  */
    iValue = (int)pControls->ucBrightness - 128;
    CNXT_SET_VAL(ENC_HUE_BRIGHT_CTL_REG, ENC_HUEBRICTL_YOFF_MASK, (u_int8)iValue);
  }
  
  /* Set contrast (if requested) */
  if(pControls->uFlags & ENCODER_CONTROL_CONTRAST)
  {
    CNXT_SET_VAL(ENC_YCRCB_MULTIPLICATION_REG, ENC_YUV_MULT_MCOMPY_MASK, pControls->ucContrast);
  }  

  /* Set hue (if requested) */
  if(pControls->uFlags & ENCODER_CONTROL_HUE)
  {
    /* Method 1: Adjust the subcarrier phase, effective for composite and svideo signals */
    /* Method 2: uses MULT_UU,MULT_VU, MULT_UV, MULT_VV in register Chroma control to    */
    /*           matrix multiply the color vectors, effective for RGB signal in non-PAL mode */ 
    if((gVideoOutput & OSD_OUTPUT_COMPOSITE) || (gVideoOutput & OSD_OUTPUT_SVIDEO))  
    {
      /* set the registers used by other method to their default values  */
      CNXT_SET_VAL(ENC_CHROMA_CTL_REG, 0xffffffff, 0x7f00007f);
      /* The value 0 is mid-range for Hue in the Bt861 and the effect of small changes */
      /* is very apparent. As a result, we rescale so that range [0,255] passed to     */
      /* this function is translated to range [-16,15] set in the device.              */
      iValue = (u_int8)((int8)(pControls->ucHue >> 3) - 16);
      CNXT_SET_VAL(ENC_HUE_BRIGHT_CTL_REG, ENC_HUEBRICTL_HUEADJ_MASK, (iValue & 0xFF));
      /* save the hue setting for the get function to return */
      uSavedHue = pControls->ucHue;
    } 
    else /* RGB signals */
    { 
      if(gVideoStandard != PAL) 
      {
#if USE_MATH==1
        double dCos, dSin;
        const double dDegToRad = 0.02; /* 0.017453 */
#endif
        u_int8 uVU, uUV, uU_V;
        
        /* set the registers used by other method to their default values  */
        CNXT_SET_VAL(ENC_HUE_BRIGHT_CTL_REG, ENC_HUEBRICTL_HUEADJ_MASK, 0);
      
#if USE_MATH==1
        dCos = -128*cos((double)(pControls->ucHue)*dDegToRad);
        dSin = -128*sin((double)(pControls->ucHue)*dDegToRad);
        
        uU_V = ~((int8)(dCos+0.5)) + 1 ;  if (uU_V == 128) uU_V = 127;
        uVU = ~(-(int8)(dSin+0.5)) + 1; if (uVU == 128) uVU = 127;
        uUV = ~((int8)(dSin+0.5)) + 1; if (uUV == 128) uUV = 127;
#else
        pControls->ucHue &= 0xff;
        uU_V = uU_V_table[pControls->ucHue];
        uVU = uVU_table[pControls->ucHue];
        uUV = uUV_table[pControls->ucHue];
#endif

        CNXT_SET_VAL(ENC_CHROMA_CTL_REG, ENC_CHROMA_CTL_MULT_UU_MASK, uU_V);
        CNXT_SET_VAL(ENC_CHROMA_CTL_REG, ENC_CHROMA_CTL_MULT_VV_MASK, uU_V);
        CNXT_SET_VAL(ENC_CHROMA_CTL_REG, ENC_CHROMA_CTL_MULT_VU_MASK, uVU);
        CNXT_SET_VAL(ENC_CHROMA_CTL_REG, ENC_CHROMA_CTL_MULT_UV_MASK, uUV);
        /* save the hue setting for the get function to return */
        uSavedHue = pControls->ucHue;
      }
      else   /* RGB signal & PAL mode*/
      {
        trace_new(OSD_ERROR_MSG, "OSD: Cannot adjust hue in RGB signal for PAL mode!");
        error_log(ERROR_WARNING);
        return(CNXT_ENCODER_UNSUPPORTED);
      }
    }  
  }
  return(CNXT_ENCODER_OK);  
}
#endif

#if 0
/* Compile this source as a console application (win32 or whatever)
 * and it will generate the tables above
 */
#include <math.h>
#include <stdio.h>

int main() {
        double dCos, dSin;
        unsigned char uVU, uUV, uU_V;
        const double dDegToRad = 0.017453;
        int i;
        
        printf("static unsigned char uU_V_table[0xff] = {\n");
        for (i=0; i<0xff; i++) {
            dCos = 128*cos(i * dDegToRad);
            dSin = 128*sin(i * dDegToRad);
            uU_V = ~((char)(dCos+0.5)) + 1 ;  if (uU_V == 128) uU_V = 127;
            printf("%d%s" ,uU_V, (i==0xfe ? "" : ","));
            if (i % 10 == 0) printf("\n");
        }
        printf("}; /* uU_V[0xff] */\n");

        printf("static unsigned char uVU_table[0xff] = {\n");
        for (i=0; i<0xff; i++) {
            dCos = 128*cos(i * dDegToRad);
            dSin = 128*sin(i * dDegToRad);
            uVU = ~(-(char)(dSin+0.5)) + 1; if (uVU == 128) uVU = 127;
            printf("%d%s" ,uVU,(i==0xfe ? "" : ","));
            if (i % 10 == 0) printf("\n");
        }
        printf("}; /* uVU[0xff] */\n");
        
        printf("static unsigned char uUV_table[0xff] = {\n");
        for (i=0; i<0xff; i++) {
            dCos = 128*cos(i * dDegToRad);
            dSin = 128*sin(i * dDegToRad);
            uUV = ~((char)(dSin+0.5)) + 1; if (uUV == 128) uUV = 127;
            printf("%d%s" ,uUV,(i==0xfe ? "" : ",") );
            if (i % 10 == 0) printf("\n");
        }
        printf("}; /* uUV[0xff] */\n");
}
#endif

/****************************************************************************
 * Modifications:
 * $Log: 
 *  22   mpeg      1.21        9/29/03 4:35:28 PM     Miles Bintz     SCR(s) 
 *        7576 :
 *        removed cos and sin functions and replaced math with a lookup table
 *        
 *  21   mpeg      1.20        9/23/03 4:40:08 PM     Lucy C Allevato SCR(s) 
 *        7115 :
 *        include handle driver unconditionally.
 *        
 *  20   mpeg      1.19        9/17/03 3:52:12 PM     Lucy C Allevato SCR(s) 
 *        7482 :
 *        update the current encoder driver to use the new handle lib.
 *        
 *  19   mpeg      1.18        8/28/03 3:48:12 PM     Lucy C Allevato SCR(s) 
 *        7397 7398 :
 *        enable the normal DAC operation for wabash because wabash has 4 DACs 
 *        instead of 6
 *        
 *  18   mpeg      1.17        7/15/03 10:43:48 AM    Angela Swartz   SCR(s) 
 *        6902 :
 *        more with hue adjustment
 *        
 *  17   mpeg      1.16        7/7/03 4:18:50 PM      Angela Swartz   SCR(s) 
 *        5464 :
 *        added code to control hue in RGB mode for internal encoder
 *        
 *  16   mpeg      1.15        6/19/03 3:09:38 PM     Dave Wilson     SCR(s): 
 *        6783 
 *        Removed previous fix. Register involved cannot be referred to in the 
 *        general codebase due to feature licensing issues.
 *        
 *  15   mpeg      1.14        6/18/03 9:59:28 AM     Dave Wilson     SCR(s): 
 *        6778 
 *        Added register missing from initialisation for internal encoders.
 *        
 *  14   mpeg      1.13        6/17/03 9:41:00 AM     Ian Mitchell    SCR(s): 
 *        6687 6686 
 *        Brady scart type is now SCART_TYPE_BRADY, it was SCART_TYPE_NONE 
 *        which enabled
 *        some encoder setup code. I have changed the #ifdef to use 
 *        SCART_TYPE_BRADY.
 *        
 *  13   mpeg      1.12        5/23/03 4:18:40 PM     Billy Jackman   SCR(s) 
 *        6582 6583 :
 *        Removed code that enabled DACs conditionally by modifying initial 
 *        register
 *        values in the configuration tables for NTSC, PAL, and SECAM.  
 *        Instead, use a
 *        call to SetDisplayOutput with the parameter VIDEO_SIGNAL_OUTPUT_TYPES
 *         after
 *        the register initialization is done.  This will use the appropriate 
 *        definition
 *        of what DACs to enable from the hardware configuration, and as a 
 *        bonus bug fix
 *        will also include other register settings, such as chrominance 
 *        bandwidth, that
 *        SetDisplayOutput already does conditionally depending upon the output
 *         settings.
 *        Since no Bronco configuration file enables more than 4 DACs, the bug 
 *        in tracker
 *        #6526 is still worked around and now the appropriate DACs are enabled
 *         for each
 *        hardware configuration instead of hardcoding two DACs to disable.
 *        
 *  12   mpeg      1.11        5/22/03 2:37:28 PM     Dave Wilson     SCR(s) 
 *        6526 6527 :
 *        Changed DAC enable for internal encoder to explicitly enable all DACs
 *         that
 *        we want rather than just tweaking the changes from the supposed 
 *        power-one
 *        default state.
 *        
 *  11   mpeg      1.10        5/20/03 6:13:04 PM     Dave Wilson     SCR(s) 
 *        6499 6500 :
 *        If ENCODER_HAS_DAC_BUG is set to YES, we disable DACs A and B. If 
 *        not, we
 *        don't do these writes since this has the effect of blanking video on 
 *        Wabash!
 *        
 *  10   mpeg      1.9         5/19/03 4:34:56 PM     Dave Wilson     SCR(s) 
 *        6440 6441 :
 *        
 *        
 *        
 *        
 *        
 *        
 *        
 *        
 *        
 *        
 *        
 *        
 *        
 *        
 *        
 *        
 *        Ensured that video DACs A and B on Brazos are disabled. Driving all 6
 *         DACs at
 *        once on Rev B causes poor video quality.
 *        
 *  9    mpeg      1.8         5/9/03 7:12:52 PM      Steve Glennon   SCR(s): 
 *        6224 6225 6190 6179 
 *        Fixed to be consistent with dual plane support in osdlibc.c
 *        
 *        
 *  8    mpeg      1.7         4/23/03 4:24:14 PM     Dave Wilson     SCR(s) 
 *        6084 :
 *        Removed critical section from cnxt_encoder_term. This was used to 
 *        protect
 *        a sem_delete call but you can't call RTOS functions from within a 
 *        critical
 *        section!
 *        
 *  7    mpeg      1.6         2/10/03 2:06:50 PM     Angela Swartz   SCR(s) 
 *        5451 :
 *        code changes associated with the Handle Driver API changes
 *        
 *  6    mpeg      1.5         2/5/03 4:54:06 PM      Dave Wilson     SCR(s) 
 *        5413 :
 *        Corrected a couple of typos introduced in the last edit that broke 
 *        builds
 *        for Klondike.
 *        
 *  5    mpeg      1.4         2/5/03 2:39:02 PM      Dave Wilson     SCR(s) 
 *        5413 :
 *        Added versions of private_set/get_encoder_controls for target IRDs 
 *        with
 *        internal video decoders (Brazos and Wabash boxes).
 *        
 *  4    mpeg      1.3         2/5/03 11:47:26 AM     Dave Wilson     SCR(s) 
 *        5405 :
 *        Added a version of SetDisplayOutput supporting the internal video 
 *        encoder
 *        and moved DAC mask definitions to the hardware config file.
 *        
 *  3    mpeg      1.2         1/29/03 4:41:36 PM     Matt Korte      SCR(s) 
 *        5352 :
 *        Fixed warnings
 *        
 *  2    mpeg      1.1         1/28/03 11:36:42 AM    Dave Wilson     SCR(s) 
 *        5335 :
 *        Removed the static modifier from the declaration of gdwEncoderType. 
 *        It 
 *        appears that some misbehaving drivers are looking at this variable!.
 *        
 *  1    mpeg      1.0         1/27/03 3:25:26 PM     Dave Wilson     
 * $
 * 
 *    Rev 1.21   29 Sep 2003 15:35:28   bintzmf
 * SCR(s) 7576 :
 * removed cos and sin functions and replaced math with a lookup table
 * 
 *    Rev 1.20   23 Sep 2003 15:40:08   goldenx
 * SCR(s) 7115 :
 * include handle driver unconditionally.
 * 
 *    Rev 1.19   17 Sep 2003 14:52:12   goldenx
 * SCR(s) 7482 :
 * update the current encoder driver to use the new handle lib.
 * 
 *    Rev 1.18   28 Aug 2003 14:48:12   goldenx
 * SCR(s) 7397 7398 :
 * enable the normal DAC operation for wabash because wabash has 4 DACs instead of 6
 * 
 *    Rev 1.17   15 Jul 2003 09:43:48   swartzwg
 * SCR(s) 6902 :
 * more with hue adjustment
 * 
 *    Rev 1.16   07 Jul 2003 15:18:50   swartzwg
 * SCR(s) 5464 :
 * added code to control hue in RGB mode for internal encoder
 * 
 *    Rev 1.15   19 Jun 2003 14:09:38   dawilson
 * SCR(s): 6783 
 * Removed previous fix. Register involved cannot be referred to in the 
 * general codebase due to feature licensing issues.
 * 
 *    Rev 1.14   18 Jun 2003 08:59:28   dawilson
 * SCR(s): 6778 
 * Added register missing from initialisation for internal encoders.
 * 
 *    Rev 1.13   17 Jun 2003 08:41:00   mitchei
 * SCR(s): 6687 6686 
 * Brady scart type is now SCART_TYPE_BRADY, it was SCART_TYPE_NONE which enabled
 * some encoder setup code. I have changed the #ifdef to use SCART_TYPE_BRADY.
 * 
 *    Rev 1.12   23 May 2003 15:18:40   jackmaw
 * SCR(s) 6582 6583 :
 * Removed code that enabled DACs conditionally by modifying initial register
 * values in the configuration tables for NTSC, PAL, and SECAM.  Instead, use a
 * call to SetDisplayOutput with the parameter VIDEO_SIGNAL_OUTPUT_TYPES after
 * the register initialization is done.  This will use the appropriate definition
 * of what DACs to enable from the hardware configuration, and as a bonus bug fix
 * will also include other register settings, such as chrominance bandwidth, that
 * SetDisplayOutput already does conditionally depending upon the output settings.
 * Since no Bronco configuration file enables more than 4 DACs, the bug in tracker
 * #6526 is still worked around and now the appropriate DACs are enabled for each
 * hardware configuration instead of hardcoding two DACs to disable.
 * 
 *    Rev 1.11   22 May 2003 13:37:28   dawilson
 * SCR(s) 6526 6527 :
 * Changed DAC enable for internal encoder to explicitly enable all DACs that
 * we want rather than just tweaking the changes from the supposed power-one
 * default state.
 * 
 *    Rev 1.10   20 May 2003 17:13:04   dawilson
 * SCR(s) 6499 6500 :
 * If ENCODER_HAS_DAC_BUG is set to YES, we disable DACs A and B. If not, we
 * don't do these writes since this has the effect of blanking video on Wabash!
 * 
 *    Rev 1.9   19 May 2003 15:34:56   dawilson
 * SCR(s) 6440 6441 :
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * Ensured that video DACs A and B on Brazos are disabled. Driving all 6 DACs at
 * once on Rev B causes poor video quality.
 * 
 *    Rev 1.8   09 May 2003 18:12:52   glennon
 * SCR(s): 6224 6225 6190 6179 
 * Fixed to be consistent with dual plane support in osdlibc.c
 * 
 * 
 *    Rev 1.7   23 Apr 2003 15:24:14   dawilson
 * SCR(s) 6084 :
 * Removed critical section from cnxt_encoder_term. This was used to protect
 * a sem_delete call but you can't call RTOS functions from within a critical
 * section!
 * 
 *    Rev 1.6   10 Feb 2003 14:06:50   swartzwg
 * SCR(s) 5451 :
 * code changes associated with the Handle Driver API changes
 * 
 *    Rev 1.5   05 Feb 2003 16:54:06   dawilson
 * SCR(s) 5413 :
 * Corrected a couple of typos introduced in the last edit that broke builds
 * for Klondike.
 * 
 *    Rev 1.4   05 Feb 2003 14:39:02   dawilson
 * SCR(s) 5413 :
 * Added versions of private_set/get_encoder_controls for target IRDs with
 * internal video decoders (Brazos and Wabash boxes).
 * 
 *    Rev 1.3   05 Feb 2003 11:47:26   dawilson
 * SCR(s) 5405 :
 * Added a version of SetDisplayOutput supporting the internal video encoder
 * and moved DAC mask definitions to the hardware config file.
 * 
 *    Rev 1.2   29 Jan 2003 16:41:36   kortemw
 * SCR(s) 5352 :
 * Fixed warnings
 * 
 *    Rev 1.1   28 Jan 2003 11:36:42   dawilson
 * SCR(s) 5335 :
 * Removed the static modifier from the declaration of gdwEncoderType. It 
 * appears that some misbehaving drivers are looking at this variable!.
 * 
 *    Rev 1.0   27 Jan 2003 15:25:26   dawilson
 * SCR(s) 5320 :
 * Video-encoder related functions used by OSDLIBC. Also contains the 
 * implementation of the new video encoder API.
 *
 ****************************************************************************/

