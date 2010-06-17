/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                   Conexant Systems Inc. (c) 1998-2003                    */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       conf_ndstst.h
 *
 *
 * Description:    default configuration data for ndstests.
 *
 *
 * Author:         Angela Swartz 
 *
 ****************************************************************************/
/* $Header: conf_ndstst.h, 1, 8/4/03 5:47:20 PM, Angela Swartz$
 ****************************************************************************/
#include "confmgr.h"
#include "tuner.h"
#include "demod.h"
#include "demod_types.h"

#define CONFMGR_DEFAULT_TRACE (TRACE_LEVEL_2 | TRACE_CTL | TRACE_DMD)
/*************************************/
/* NDSTESTS - USA Default Parameters */
/*************************************/
#define CONF_VIDEO_OUTPUT_STANDARD NTSC
#define CONF_FREQUENCY 0x01110000
#define CONF_SYMBOL_RATE 0x00200000
#define CONF_FEC DVB_FEC_23
#define CONF_POLARISATION RIGHT        
#define CONF_LNB_A 0x01000000
#define CONF_LNB_B 0x01125000
#define CONF_RIGHT_IS_LOW TRUE
#define CONF_CM_FREQUENCY 710000000
#define CONF_CM_SYMBOL_RATE 4444000
#define CONF_CM_ANNEX ANNEX_A
#define CONF_CM_UART_FLAGS 0
#define CONF_ATV_CHANNEL 7
#define CONF_ATV_COUNTRY COUNTRY_USA 

/****************************************************************************
 * Modifications:
 * $Log: 
 *  1    mpeg      1.0         8/4/03 5:47:20 PM      Angela Swartz   
 * $
 * 
 *    Rev 1.0   04 Aug 2003 16:47:20   swartzwg
 * SCR(s) 7137 7138 :
 * default config data for NDSTESTS
 * 
 ****************************************************************************/

