#ifndef INCLUDE_EXTERNAL_AV_MUX_H
#define INCLUDE_EXTERNAL_AV_MUX_H
/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                        Conexant Systems Inc. (c)                         */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        EXTERNAL_AV_MUX.H
 *
 *
 * Description:     Audio and video multiplexers API user header file
 *
 *
 * Author:          Ian Mitchell
 *
 ****************************************************************************/
/* $Id: external_av_mux.h,v 1.1, 2003-12-23 16:26:58Z, Ian Mitchell$
 ****************************************************************************/

typedef enum
{
   EXTERNAL_AV_MUX_SUCCESS = 0,
   EXTERNAL_AV_MUX_NOT_INIT,
   EXTERNAL_AV_MUX_FAIL
}
EXTERNAL_AV_MUX_STATUS;

typedef enum
{
   EXTERNAL_AV_MUX_AUDIO_INPUT_BRAZOS  = BRAZOS_DAC_OUTPUT,
   EXTERNAL_AV_MUX_AUDIO_INPUT_MAKO    = MAKO_DAC_OUTPUT
}
EXTERNAL_AV_MUX_AUDIO_INPUT;

typedef enum
{
   EXTERNAL_AV_MUX_VIDEO_INPUT_BRAZOS  = BRAZOS_VIDEO_OUTPUT,
   EXTERNAL_AV_MUX_VIDEO_INPUT_MAKO    = MAKO_VIDEO_OUTPUT
}
EXTERNAL_AV_MUX_VIDEO_INPUT;

EXTERNAL_AV_MUX_STATUS  cnxt_external_av_mux_init(void);
EXTERNAL_AV_MUX_STATUS  cnxt_external_av_mux_audio_select_input(EXTERNAL_AV_MUX_AUDIO_INPUT   aiInput);
EXTERNAL_AV_MUX_STATUS  cnxt_external_av_mux_video_select_input(EXTERNAL_AV_MUX_VIDEO_INPUT   viInput);

/****************************************************************************
 * Modifications:
 * $Log: 
 *  2    mpeg      1.1         12/23/03 10:26:58 AM   Ian Mitchell    CR(s) 
 *        7739 : Modifications due to testing on Crosby.
 *        
 *  1    mpeg      1.0         11/20/03 5:42:16 AM    Ian Mitchell    CR(s): 
 *        7739 Public header file for the external_av_mux driver.
 * $
 * 
 ****************************************************************************/
#endif /* #ifndef INCLUDE_EXTERNAL_AV_MUX_H */
