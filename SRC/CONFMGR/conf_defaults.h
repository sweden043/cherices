/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                   Conexant Systems Inc. (c) 1998-2003                    */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       conf_defaults.h
 *
 *
 * Description:    default configuration data for applications other than NDSTESTS and 1.2.
 *
 *
 * Author:         Angela Swartz 
 *
 ****************************************************************************/
/* $Header: conf_defaults.h, 3, 11/13/03 11:38:11 AM, $
 ****************************************************************************/
#include "confmgr.h"
#include "tuner.h"
#include "demod.h"
#include "demod_types.h"
#include "stbcfg.h"

/* Note that all frequencies are now truncated at an integral number of 
MHz for WatchTV */

#define CONFMGR_DEFAULT_TRACE (TRACE_LEVEL_4 | TRACE_ALL)

// 将开始的默认制式改为PAL - 2006-02-23
//#if defined(LOCAL_TS_INFO)&&(LOCAL_TS_INFO==TS_SHANDONG_TAIAN)
#define CONF_VIDEO_OUTPUT_STANDARD PAL
//#else
//#define CONF_VIDEO_OUTPUT_STANDARD AUTO_DETECT
//#endif

#define CONF_FREQUENCY 0x01237000
#define CONF_SYMBOL_RATE 0x00200000
#define CONF_FEC DVB_FEC_56
#define CONF_POLARISATION RIGHT        
#define CONF_LNB_A 0x01125000
#define CONF_LNB_B 0x01125000
#define CONF_RIGHT_IS_LOW TRUE
#define CONF_CM_FREQUENCY 127250000
#define CONF_CM_SYMBOL_RATE 5759768
#define CONF_CM_ANNEX ANNEX_B
#if (SERIAL2 == UART_NONE)
#define CONF_CM_UART_FLAGS 0
#else
  #define CONF_CM_UART_FLAGS SERIAL2_CM_MPEG_OWNER
#endif
#define CONF_ATV_CHANNEL 7
#define CONF_ATV_COUNTRY COUNTRY_USA 

/****************************************************************************
 * Modifications:
 * $Log: 
 *  3    mpeg      1.2         11/13/03 11:38:11 AM   Yong Lu         CR(s): 
 *        7920 7921 Milano use the default defined in config
 *  2    mpeg      1.1         10/30/03 10:51:03 AM   Tim Ross        CR(s): 
 *        7743 7744 Updated cable channel parameters to match ADC headend.
 *  1    mpeg      1.0         8/4/03 5:47:04 PM      Angela Swartz   
 * $
 * 
 *    Rev 1.0   04 Aug 2003 16:47:04   swartzwg
 * SCR(s) 7137 7138 :
 * default config data for non OpenTV, non NDSTESTS application
 * 
 ****************************************************************************/

