/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*          Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002          */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*                                                                          
 * Filename:           GLOBALS.H                                            
 *                                                                          
 * Description:        Definitions of register pointers and other           
 *                     generally useful global variables.                   
 *                                                                          
 * Author:             Mustafa Ismail                                       
 *                                                                          
 * Copyright Rockwell Semiconductor Systems, 1997                           
 * All Rights Reserved.                                                     
 *                                                                          
 * To add a new global variable                                             
 *        1. Add the definition with the initial value in the DEFINE_GLOBALS
 *           section                                                        
 *        2. In the appropriate  section (ie #ifdef INCL_DRM) of the        
 *           else DEFINE_GLOBALS add the extern declaration.                
 *                                                                          
 ****************************************************************************/

#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#ifdef DEFINE_GLOBALS

#ifdef BITFIELDS
/* From EVID */
LPMOV_VS_ACT_LINE_RANGE glpMovVsActLineRange           = (LPMOV_VS_ACT_LINE_RANGE)MOV_VS_ACT_LINE_RANGE_REG;
LPMOV_VS_VBI_LINE_RANGE glpMovVsVBILineRange           = (LPMOV_VS_VBI_LINE_RANGE)MOV_VS_VBI_LINE_RANGE_REG;
LPMOV_VS_MAX_PIXEL glpMovVsMaxPixel                    = (LPMOV_VS_MAX_PIXEL)MOV_VS_MAX_PIXEL_REG;
LPMOV_VS_CONTROL glpMovVsControl                       = (LPMOV_VS_CONTROL)MOV_VS_CONTROL_REG;
LPMOV_VS_STATUS glpMovVsStatus                         = (LPMOV_VS_STATUS)MOV_VS_STATUS_REG;
LPMOV_VS_DROPPED_LINE_COUNT glpMovVsDroppedLineCount   = (LPMOV_VS_DROPPED_LINE_COUNT)MOV_VS_DROPPED_LINE_COUNT_REG;
LPMOV_SS_ACT_EVEN glpMovSsActEven                      = (LPMOV_SS_ACT_EVEN)MOV_SS_ACT_EVEN_REG;
LPMOV_SS_ACT_ODD glpMovSsActOdd                        = (LPMOV_SS_ACT_ODD)MOV_SS_ACT_ODD_REG;
LPMOV_SS_ACT_THIRD glpMovSsActThird                    = (LPMOV_SS_ACT_THIRD)MOV_SS_ACT_THIRD_REG;
LPMOV_SS_VBI_EVEN glpMovSsVBIEven                      = (LPMOV_SS_VBI_EVEN)MOV_SS_VBI_EVEN_REG;
LPMOV_SS_VBI_ODD glpMovSsVBIOdd                        = (LPMOV_SS_VBI_ODD)MOV_SS_VBI_ODD_REG;
LPMOV_SS_STRIDE glpMovSsStride                         = (LPMOV_SS_STRIDE)MOV_SS_STRIDE_REG;
LPMOV_SS_CONTROL glpMovSsControl                       = (LPMOV_SS_CONTROL)MOV_SS_CONTROL_REG;
LPMOV_SS_ENABLE glpMovSsEnable                         = (LPMOV_SS_ENABLE)MOV_SS_ENABLE_REG;
LPMOV_SS_TIMER glpMovSsTimer                           = (LPMOV_SS_TIMER)MOV_SS_TIMER_REG;
LPMOV_SS_STATUS glpMovSsStatus                         = (LPMOV_SS_STATUS)MOV_SS_STATUS_REG;
LPMOV_SS_TIMER_STATUS glpMovSsTimerStatus              = (LPMOV_SS_TIMER_STATUS)MOV_SS_TIMER_STATUS_REG;
#endif /* BITFIELDS */

#if IIC_TYPE == IIC_TYPE_COLORADO
/* From I2C */
#ifdef BITFIELDS
LPI2C_CTRL_WRITE glpI2CCtrlW = (LPI2C_CTRL_WRITE)I2C_CTRL_WRITE_REG;
LPI2C_CTRL_READ glpI2CCtrlR = (LPI2C_CTRL_READ)I2C_CTRL_READ_REG;
LPI2C_MASTER_CTLW glpI2CMasterCtrlW = (LPI2C_MASTER_CTLW)I2C_MASTER_CTLW_REG;
LPI2C_MASTER_CTLR glpI2CMasterCtrlR = (LPI2C_MASTER_CTLR)I2C_MASTER_CTLR_REG;
LPI2C_SLAVE_CTLW glpI2CSlaveCtrlW = (LPI2C_SLAVE_CTLW)I2C_SLAVE_CTLW_REG;
LPI2C_SLAVE_CTLR glpI2CSlaveCtrlR = (LPI2C_SLAVE_CTLR)I2C_SLAVE_CTLR_REG;
LPI2C_MASTER_DATA glpI2CMasterData = (LPI2C_MASTER_DATA)I2C_MASTER_DATA_REG;
LPI2C_SLAVE_DATA glpI2CSlaveData = (LPI2C_SLAVE_DATA)I2C_SLAVE_DATA_REG;
#else
LPREG glpI2CCtrlW       = (LPREG)I2C_CTRL_WRITE_REG;
LPREG glpI2CCtrlR       = (LPREG)I2C_CTRL_READ_REG;
LPREG glpI2CMasterCtrlW = (LPREG)I2C_MASTER_CTLW_REG;
LPREG glpI2CMasterCtrlR = (LPREG)I2C_MASTER_CTLR_REG;
LPREG glpI2CSlaveCtrlW  = (LPREG)I2C_SLAVE_CTLW_REG;
LPREG glpI2CSlaveCtrlR  = (LPREG)I2C_SLAVE_CTLR_REG;
LPREG glpI2CMasterData  = (LPREG)I2C_MASTER_DATA_REG;
LPREG glpI2CSlaveData   = (LPREG)I2C_SLAVE_DATA_REG;
#endif /* BITFIELDS */
#endif /* IIC_TYPE == IIC_TYPE_COLORADO */

#ifdef BITFIELD_MPG
LPMPG_DECODE_STATUS glpDecoderStatusReg = (LPMPG_DECODE_STATUS) MPG_DECODE_STATUS_REG;
#else
LPREG               glpDecoderStatusReg = (LPREG)               MPG_DECODE_STATUS_REG;
#endif

LPREG glpPCRHigh              = (LPREG) MPG_PSCRS_HI_REG;
LPREG glpPCRLow               = (LPREG) MPG_PSCRS_LO_REG;
LPREG glpSTCSnapHi            = (LPREG) MPG_STCS_HI_REG;
LPREG glpSTCSnapLo            = (LPREG) MPG_STCS_LO_REG;
LPREG glpAPTSHi               = (LPREG) MPG_PTS_HI_REG;
LPREG glpAPTSLo               = (LPREG) MPG_PTS_LO_REG;
LPREG glpVPTSHi               = (LPREG) MPG_VPTS_H_REG;
LPREG glpVPTSLo               = (LPREG) MPG_VPTS_L_REG;
LPREG glpSTCHi                = (LPREG) MPG_STC_HI_REG;
LPREG glpSTCLo                = (LPREG) MPG_STC_LO_REG;
LPREG glpCtrl0Reg             = (LPREG) MPG_CONTROL0_REG;
LPREG glpCtrl1Reg             = (LPREG) MPG_CONTROL1_REG;

#ifdef BITFIELD_MPG
LPMPG_CONTROL0 glpCtrl0       = (LPMPG_CONTROL0) MPG_CONTROL0_REG;
LPMPG_CONTROL1 glpCtrl1       = (LPMPG_CONTROL1) MPG_CONTROL1_REG;
LPMPG_COMMAND  glpMpgCmd      = (LPMPG_COMMAND)  MPG_COMMAND_REG;
#else
LPREG          glpCtrl0       = (LPREG)          MPG_CONTROL0_REG;
LPREG          glpCtrl1       = (LPREG)          MPG_CONTROL1_REG;
LPREG          glpMpgCmd      = (LPREG)          MPG_COMMAND_REG;
#endif

LPREG glpMpgCmdReg            = (LPREG) MPG_COMMAND_REG;
LPREG glpIntStatus            = (LPREG) MPG_ISR_REG;
LPREG glpIntMask              = (LPREG) MPG_IMR_REG;
LPREG glpErrorStatus          = (LPREG) MPG_ERROR_STATUS_REG;
LPREG glpErrorMask            = (LPREG) MPG_ERROR_MASK_REG;
LPREG glpMCWrite              = (LPREG) MPG_MCODE_W_REG;

#ifdef BITFIELD_MPG
LPMPG_VID_SIZE glpMpgPicSize  = (LPMPG_VID_SIZE) MPG_VIDEO_SIZE_REG;
LPMPG_GOP_TIME_CODE glpTimeCode= (LPMPG_GOP_TIME_CODE) MPG_GOP_TIME_CODE_REG;
LPMPG_ADDR_EXT glpMpgAddrExt  = (LPMPG_ADDR_EXT)MPG_ADDR_EXT_REG;
#else
LPREG          glpMpgPicSize  = (LPREG)MPG_VIDEO_SIZE_REG;
LPREG          glpTimeCode    = (LPREG)MPG_GOP_TIME_CODE_REG;
LPREG          glpMpgAddrExt  = (LPREG)MPG_ADDR_EXT_REG;
#endif

/* From OSD */
LPREG glpDrmControl = (LPREG)DRM_CONTROL_REG;
LPREG glpDrmInterrupt = (LPREG)DRM_INTERRUPT_ENABLE_REG;
LPREG glpDrmStatus = (LPREG)DRM_STATUS_REG;
LPREG glpDrmOsdPoiner = (LPREG)DRM_OSD_POINTER_REG;
LPREG glpDrmColorKey = (LPREG)DRM_COLOR_KEY_REG;
LPREG glpDrmBackground = (LPREG)DRM_GRAPHICS_BORDER_COLOR_REG;
LPREG glpDrmDWIControl1 = (LPREG)DRM_DWI_CONTROL_1_REG;
LPREG glpDrmDWIControl2 = (LPREG)DRM_DWI_CONTROL_2_REG;
LPREG glpDrmDWIControl3 = (LPREG)DRM_DWI_CONTROL_3_REG;
LPREG glpDrmScreenStart = (LPREG)DRM_SCREEN_START_REG;
LPREG glpDrmScreenEnd = (LPREG)DRM_SCREEN_END_REG;
LPREG glpDrmControl2 = (LPREG)DRM_CONTROL_2_REG;

LPREG glpDrmMpegOffsetWidth[2] = {
   (LPREG)DRM_MPEG_OFFSET_WIDTH_REG,
   (LPREG)DRM_MPEG_OFFSET_WIDTH_2_REG};
LPREG glpDrmMpegVoffset[2] = {
   (LPREG)DRM_MPEG_VERTICAL_OFFSET_REG,
   (LPREG)DRM_MPEG_VERTICAL_OFFSET_2_REG};
LPREG glpDrmMpegPos[2] = {
   (LPREG)DRM_MPEG_POSITION_REG,
   (LPREG)DRM_MPEG_POSITION_2_REG};
LPREG glpDrmMpegSize[2] = {
   (LPREG)DRM_MPEG_SIZE_REG,
   (LPREG)DRM_MPEG_SIZE_2_REG};
LPREG glpDrmMpegXInc[2] = {
   (LPREG)DRM_MPEG_X_INC_REG,
   (LPREG)DRM_MPEG_X_INC_2_REG};
LPREG glpDrmMpegYInc[2] = {
   (LPREG)DRM_MPEG_Y_INC_REG,
   (LPREG)DRM_MPEG_Y_INC_2_REG};
LPREG glpDrmSharp[2] = {
   (LPREG)DRM_SHARP_REG,
   (LPREG)DRM_SHARP_2_REG};
LPREG glpDrmVbiBuf0[2] = {
   (LPREG)DRM_VBI_BUF_0_REG,
   (LPREG)DRM_VBI_BUF_0_2_REG};
LPREG glpDrmVbiBuf1[2] = {
   (LPREG)DRM_VBI_BUF_1_REG,
   (LPREG)DRM_VBI_BUF_1_2_REG};
LPREG glpDrmVidBuf0[2] = {
   (LPREG)DRM_VID_BUF_0_REG,
   (LPREG)DRM_VID_BUF_0_2_REG};
LPREG glpDrmVidBuf1[2] = {
   (LPREG)DRM_VID_BUF_1_REG,
   (LPREG)DRM_VID_BUF_1_2_REG};
LPREG glpDrmVidBuf2[2] = {
   (LPREG)DRM_VID_BUF_2_REG,
   (LPREG)DRM_VID_BUF_2_2_REG};
LPREG glpDrmVidPos[2] = {
   (LPREG)DRM_VID_POSITION_REG,
   (LPREG)DRM_VID_POSITION_2_REG};
LPREG glpDrmVidSize[2] = {
   (LPREG)DRM_VID_SIZE_REG,
   (LPREG)DRM_VID_SIZE_2_REG};
LPREG glpDrmVidStride[2] = {
   (LPREG)DRM_VID_STRIDE_REG,
   (LPREG)DRM_VID_STRIDE_2_REG};
LPREG glpDrmDWIControl[2] = {
   (LPREG)DRM_DWI_CONTROL_2_REG,
   (LPREG)DRM_DWI_CONTROL_3_REG};

LPREG glpDrmTTStride = (LPREG)DRM_TELETEXT_STRIDE_REG;
LPREG glpDrmTTField1 = (LPREG)DRM_TELETEXT_FIELD1_REG;
LPREG glpDrmTTField2 = (LPREG)DRM_TELETEXT_FIELD2_REG;
LPREG glpDrmTTField1Addr = (LPREG)DRM_TELETEXT_FIELD1_ADDR_REG;
LPREG glpDrmTTField2Addr = (LPREG)DRM_TELETEXT_FIELD2_ADDR_REG;
LPREG glpDrmCursorControl = (LPREG)DRM_CURSOR_CONTROL_REG;
LPREG glpDrmCursorStoreAddr = (LPREG)DRM_CURSOR_STORE_ADDR_REG;
LPREG glpDrmCursorPal0 = (LPREG)DRM_CURSOR_PALETTE_0_REG;
LPREG glpDrmCursorPal1 = (LPREG)DRM_CURSOR_PALETTE_1_REG;
LPREG glpDrmCursorPal2 = (LPREG)DRM_CURSOR_PALETTE_2_REG;

LPREG glpDrmMpgStillCtrl[2] = {
   (LPREG)DRM_MPEG_STILL_CONTROL_REG,
   (LPREG)DRM_MPEG_STILL_CONTROL_2_REG};
LPREG glpDrmMpgLumaAddr[2] = {
   (LPREG)DRM_MPEG_LUMA_ADDR_REG,
   (LPREG)DRM_MPEG_LUMA_ADDR_2_REG};
LPREG glpDrmMpgChromaAddr[2] = {
   (LPREG)DRM_MPEG_CHROMA_ADDR_REG,
   (LPREG)DRM_MPEG_CHROMA_ADDR_2_REG};
LPREG glpDrmMpgSrcSize[2] = {
   (LPREG)DRM_MPEG_HEIGHTWIDTH_REG,
   (LPREG)DRM_MPEG_HEIGHTWIDTH_2_REG};
LPREG glpDrmWipeLumaAddr[2] = {
   (LPREG)DRM_MPEG_WIPE_LUMA_REG,
   (LPREG)DRM_MPEG_WIPE_LUMA_2_REG};
LPREG glpDrmWipeChromaAddr[2] = {
   (LPREG)DRM_MPEG_WIPE_CHROMA_REG,
   (LPREG)DRM_MPEG_WIPE_CHROMA_2_REG};
LPREG glpDrmMpgTileWipe[2] = {
   (LPREG)DRM_MPEG_TILE_PARMS_REG,
   (LPREG)DRM_MPEG_TILE_PARMS_2_REG};

LPREG glpDrmVideoAlpha = (LPREG)DRM_VIDEO_ALPHA_REG;

LPREG glpDrmSar = (LPREG)DRM_SAR_REG;

/* From flash managing software */
bool  FlashAccessible = TRUE;    // tells if it is able to read data from flash

#else  /* don't DEFINE_GLOBALS */

#ifdef INCL_MOV
#ifdef BITFIELDS
/* From EVID */
extern LPMOV_VS_ACT_LINE_RANGE      glpMovVsActLineRange;
extern LPMOV_VS_VBI_LINE_RANGE      glpMovVsVBILineRange;
extern LPMOV_VS_MAX_PIXEL           glpMovVsMaxPixel;
extern LPMOV_VS_CONTROL             glpMovVsControl;
extern LPMOV_VS_STATUS              glpMovVsStatus;
extern LPMOV_VS_DROPPED_LINE_COUNT  glpMovVsDroppedLineCount;
extern LPMOV_SS_ACT_EVEN            glpMovSsActEven;
extern LPMOV_SS_ACT_ODD             glpMovSsActOdd;
extern LPMOV_SS_ACT_THIRD           glpMovSsActThird;
extern LPMOV_SS_VBI_EVEN            glpMovSsVBIEven;
extern LPMOV_SS_VBI_ODD             glpMovSsVBIOdd;
extern LPMOV_SS_STRIDE              glpMovSsStride;
extern LPMOV_SS_CONTROL             glpMovSsControl;
extern LPMOV_SS_ENABLE              glpMovSsEnable;
extern LPMOV_SS_TIMER               glpMovSsTimer;
extern LPMOV_SS_STATUS              glpMovSsStatus;
extern LPMOV_SS_TIMER_STATUS        glpMovSsTimerStatus;
#endif /* BITFIELDS */
#endif /* INCL_MOV */

#ifdef INCL_I2C
/* From I2C */
#ifdef BITFIELDS
extern LPI2C_CTRL_WRITE    glpI2CCtrlW;
extern LPI2C_CTRL_READ     glpI2CCtrlR;
extern LPI2C_MASTER_CTLW   glpI2CMasterCtrlW;
extern LPI2C_MASTER_CTLR   glpI2CMasterCtrlR;
extern LPI2C_SLAVE_CTLW    glpI2CSlaveCtrlW;
extern LPI2C_SLAVE_CTLR    glpI2CSlaveCtrlR;
extern LPI2C_MASTER_DATA   glpI2CMasterData;
extern LPI2C_SLAVE_DATA    glpI2CSlaveData;
#else
extern LPREG               glpI2CCtrlW;
extern LPREG               glpI2CCtrlR;
extern LPREG               glpI2CMasterCtrlW;
extern LPREG               glpI2CMasterCtrlR;
extern LPREG               glpI2CSlaveCtrlW;
extern LPREG               glpI2CSlaveCtrlR;
extern LPREG               glpI2CMasterData;
extern LPREG               glpI2CSlaveData;
#endif /* BITFIELDS */
#endif /* INCL_I2C */

#ifdef INCL_MPG
extern LPREG glpPCRHigh;
extern LPREG glpPCRLow;
extern LPREG glpSTCSnapHi;
extern LPREG glpSTCSnapLo;
extern LPREG glpAPTSHi;
extern LPREG glpAPTSLo;
extern LPREG glpVPTSHi;
extern LPREG glpVPTSLo;
extern LPREG glpSTCHi;
extern LPREG glpSTCLo;
extern LPREG glpCtrl0Reg;
extern LPREG glpCtrl1Reg;

#ifdef BITFIELD_MPG
extern LPMPG_CONTROL0 glpCtrl0;
extern LPMPG_CONTROL1 glpCtrl1;
extern LPMPG_COMMAND  glpMpgCmd;
#else
extern LPREG          glpCtrl0;
extern LPREG          glpCtrl1;
extern LPREG          glpMpgCmd;
#endif

extern LPREG glpMpgCmdReg;
extern LPREG glpIntStatus;
extern LPREG glpIntMask;
extern LPREG glpErrorStatus;
extern LPREG glpErrorMask;
extern LPREG glpMCWrite;

#ifdef BITFIELD_MPG
extern LPMPG_VID_SIZE      glpMpgPicSize;
extern LPMPG_GOP_TIME_CODE glpTimeCode;
extern LPMPG_DECODE_STATUS glpDecoderStatusReg;
extern LPMPG_ADDR_EXT      glpMpgAddrExt;
#else
extern LPREG          glpMpgPicSize;
extern LPREG          glpTimeCode;
extern LPREG          glpDecoderStatusReg;
extern LPREG          glpMpgAddrExt;
#endif

#endif //INCL_MPG

/* From OSD */
#ifdef INCL_DRM

extern LPREG glpDrmControl;
extern LPREG glpDrmControl2;
extern LPREG glpDrmInterrupt;
extern LPREG glpDrmStatus;
extern LPREG glpDrmOsdPoiner;
extern LPREG glpDrmColorKey;
extern LPREG glpDrmBackground;
extern LPREG glpDrmDWIControl1;
extern LPREG glpDrmDWIControl2;
extern LPREG glpDrmDWIControl3;
extern LPREG glpDrmScreenStart;
extern LPREG glpDrmScreenEnd;
extern LPREG glpDrmMpegOffsetWidth[2];
extern LPREG glpDrmMpegVoffset[2];
extern LPREG glpDrmMpegPos[2];
extern LPREG glpDrmMpegSize[2];
extern LPREG glpDrmMpegXInc[2];
extern LPREG glpDrmMpegYInc[2];
extern LPREG glpDrmSharp[2];
extern LPREG glpDrmVbiBuf0[2];
extern LPREG glpDrmVbiBuf1[2];
extern LPREG glpDrmVidBuf0[2];
extern LPREG glpDrmVidBuf1[2];
extern LPREG glpDrmVidBuf2[2];
extern LPREG glpDrmVidPos[2];
extern LPREG glpDrmVidSize[2];
extern LPREG glpDrmVidStride[2];
extern LPREG glpDrmTTStride;
extern LPREG glpDrmTTField1;
extern LPREG glpDrmTTField2;
extern LPREG glpDrmTTField1Addr;
extern LPREG glpDrmTTField2Addr;
extern LPREG glpDrmCursorControl;
extern LPREG glpDrmCursorStoreAddr;
extern LPREG glpDrmCursorPal0;
extern LPREG glpDrmCursorPal1;
extern LPREG glpDrmCursorPal2;
extern LPREG glpDrmMpgStillCtrl[2];
extern LPREG glpDrmMpgLumaAddr[2];
extern LPREG glpDrmMpgChromaAddr[2];
extern LPREG glpDrmMpgSrcSize[2];
extern LPREG glpDrmWipeLumaAddr[2];
extern LPREG glpDrmWipeChromaAddr[2];
extern LPREG glpDrmMpgTileWipe[2];
extern LPREG glpDrmVideoAlpha;
extern LPREG glpDrmSar;
extern LPREG glpDrmDWIControl[2];

#endif //INCL_DRM

/* From flash managing software */
extern bool FlashAccessible;

#endif // of else DEFINE_GLOBALS

#endif // _GLOBALS_H_

