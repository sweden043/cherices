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
 * Description:    default configuration data for OpenTV 1.2.
 *
 *
 * Author:         Angela Swartz 
 *
 ****************************************************************************/
/* $Header: conf_otv12.h, 1, 8/4/03 5:46:32 PM, Angela Swartz$
 ****************************************************************************/
#include "confmgr.h"
#include "tuner.h"
#include "demod.h"
#include "demod_types.h"


#define CONFMGR_DEFAULT_TRACE (TRACE_LEVEL_2 | TRACE_OCD | TRACE_CTL)

#define CONF_VIDEO_OUTPUT_STANDARD PAL
#define CONF_FREQUENCY 0x01212900
#define CONF_SYMBOL_RATE 0x00275000
#define CONF_FEC DVB_FEC_23
#define CONF_POLARISATION VERTICAL        
#define CONF_LNB_A 0x00975000
#define CONF_LNB_B 0x01060000
#define CONF_RIGHT_IS_LOW FALSE
#define CONF_CM_FREQUENCY 129512500
#define CONF_CM_SYMBOL_RATE 4999989
#define CONF_CM_ANNEX ANNEX_A
#define CONF_CM_UART_FLAGS 0
#define CONF_ATV_CHANNEL 2
#define CONF_ATV_COUNTRY COUNTRY_UK 

/****************************************************************************
 * Modifications:
 * $Log: 
 *  1    mpeg      1.0         8/4/03 5:46:32 PM      Angela Swartz   
 * $
 * 
 *    Rev 1.0   04 Aug 2003 16:46:32   swartzwg
 * SCR(s) 7137 7138 :
 * default config data for otv12ctl
 * 
 ****************************************************************************/

