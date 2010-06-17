/****************************************************************************/
/*                 Rockwell Semiconductor Systems - SABINE                  */
/****************************************************************************/
/*                                                                          */
/* Filename:           OCONFIG.H                                            */
/*                                                                          */
/* Description:        Data types and labels used in ocode applications     */
/*                                                                          */
/* Author:             Dave Wilson                                          */
/*                                                                          */
/* Copyright Rockwell Semiconductor Systems, 1998                           */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/
/*
$Header:   K:/sabine/pvcs/include/oconfig.h_v
$Log:   K:/sabine/pvcs/include/oconfig.h_v
 */

#ifndef _OCONFIG_H_
#define _OCONFIG_H_

/******************************************************************/
/* Structure holding all menu-alterable configuration information */
/* This is a combination of the sabine and pSOS configuration     */
/* structures used by the native code.                            */
/*                                                                */
/* Definitions of labels used in this structure are to be found   */
/* in the CONFMGR.H header file.                                  */
/******************************************************************/
typedef struct _sabine_ocode_config
{
  unsigned int  frequency_bcd_ghz;       /* BCD 3.5 in GHz */
  unsigned int  symbol_rate_bcd_mss;     /* BCD 4.4 in MS/s */
  unsigned int  polarisation;
  unsigned int  fec;
  unsigned int  video_output;
  unsigned int  video_aspect;
  unsigned int  video_format;
  unsigned int  video_connector;
  unsigned int  audio_sync;
  unsigned int  audio_format;
  unsigned int  audio_source;
  unsigned int  deemphasis;
  unsigned int  clock_recovery;
  unsigned int  nim_type;
  unsigned int  lnb_a_bcd_ghz;    /* BCD 3.5 in GHz, LNB frequency with no tone applied (lower)  */
  unsigned int  lnb_b_bcd_ghz;    /* BCD 3.5 in GHz, LNB frequency with tone applied    (higher) */
  unsigned int  right_is_low;
  /*********************************************/
  /* Analog TV tuning and picture control info */
  /*********************************************/
  unsigned int  channel;
  unsigned int  tuning_scheme;
  unsigned int  country;
  unsigned int  analog_source;
  /****************************/
  /* Encoder picture settings */
  /****************************/
  unsigned int  brightness;
  unsigned int  contrast;
  unsigned int  color;
  /************************/
  /* Service list options */
  /************************/
  unsigned int  svl_service;
  unsigned int  svl_chosen;
  /*****************/
  /* Debug Options */
  /*****************/
  unsigned int  debug_level;
  unsigned int  enable_opentv_download;
  /****************/
  /* Device flags */
  /****************/
  unsigned int  device_flags;
  /********************/
  /* pSOS BSP options */
  /********************/
  unsigned int  ethernet_enabled;
  unsigned int  ethernet_addr;
  unsigned int  subnet_mask;
  unsigned int  gateway_addr;
  unsigned int  debug_baud;
} sabine_ocode_config;

#endif  /* _OCONFIG_H_ */


