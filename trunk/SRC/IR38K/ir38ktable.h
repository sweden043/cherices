/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                 Conexant Systems Inc. (c) 1998-2002                      */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:      ir38ktable.h
 *
 * Description:   Header file for the NEC 38KHz IR decoder driver
 *
 * Author:        Steven Shen
 *
 ****************************************************************************/
/* $Header: ir38ktable.h, 1, 6/24/04 10:08:09 PM, Xiao Guang Yan$
 ****************************************************************************/

#ifndef __IR38KTABLE_H_
#define __IR38KTABLE_H_

#include "kal.h"

#define TABLE_SIZE            (0x80)

/* This table is a best composite of supported codes for the universal */
/* remote controller.                                                  */
u_int32 rc38k_xlate_table[] =
{
/* 0x00 */ 0, 0, 0, 0, 0, CNXT_FORWARD, CNXT_GUIDE, CNXT_REVERSE,
/* 0x08 */ CNXT_1, CNXT_4, CNXT_7, CNXT_MUTE, CNXT_2, CNXT_5, CNXT_8, CNXT_0,
/* 0x10 */ CNXT_3, CNXT_6, CNXT_9, 0, CNXT_POWER, CNXT_EXIT, CNXT_UPARROW, CNXT_MENU, 
/* 0x18 */ CNXT_INFO, CNXT_RARROW, CNXT_ENTER, CNXT_LARROW, 0, CNXT_BUY, CNXT_DOWNARROW, 0,
/* 0x20 */ 0, 0, 0, 0, 0, 0, 0, 0,
/* 0x28 */ 0, 0, 0, 0, 0, 0, 0, 0,
/* 0x30 */ 0, 0, 0, 0, 0, 0, 0, 0,
/* 0x38 */ 0, 0, 0, 0, 0, 0, 0, 0,
/* 0x40 */ 0, 0, 0, 0, 0, 0, 0, 0,
/* 0x48 */ 0, 0, 0, 0, 0, 0, 0, 0,
/* 0x50 */ 0, 0, 0, 0, 0, 0, 0, 0,
/* 0x58 */ 0, 0, 0, 0, 0, 0, 0, 0,
/* 0x60 */ 0, 0, 0, 0, 0, 0, 0, 0,
/* 0x68 */ 0, 0, 0, 0, 0, 0, 0, 0,
/* 0x70 */ 0, 0, 0, 0, 0, 0, 0, 0,
/* 0x78 */ 0, 0, 0, 0, 0, 0, 0, 0
};


#if (0)
u_int32 rc38k_xlate_table[] = {
/* 0x00 */ CNXT_CHANUP, CNXT_CHANDN, CNXT_VOLUP,CNXT_VOLDN, CNXT_PAUSE, CNXT_MENU, 0, 0,
/* 0x08 */ CNXT_REVERSE, CNXT_FORWARD, CNXT_MUTE, 0, CNXT_POWER, 0, 0, 0,
/* 0x10 */ CNXT_0, CNXT_1, CNXT_2, CNXT_3,CNXT_4, CNXT_5, CNXT_6, CNXT_7, 
/* 0x18 */ CNXT_8, CNXT_9, CNXT_RED, 0, 0, 0, CNXT_DISPLAY, CNXT_ENTER,
/* 0x20 */ 0, 0, 0, 0, 0, 0, 0, 0,
/* 0x28 */ 0, 0, 0, 0, 0, 0, 0, 0,
/* 0x30 */ 0, 0, 0, 0, 0, 0, 0, 0,
/* 0x38 */ 0, 0, 0, 0, 0, 0, 0, 0,
/* 0x40 */ 0, 0, 0, 0, 0, 0, 0, 0,
/* 0x48 */ 0, 0, 0, 0, 0, 0, 0, 0,
/* 0x50 */ 0, 0, 0, 0, 0, 0, 0, 0,
/* 0x58 */ 0, 0, 0, 0, 0, 0, 0, 0,
/* 0x60 */ 0, 0, 0, 0, 0, 0, 0, 0,
/* 0x68 */ 0, 0, 0, 0, 0, 0, 0, 0,
/* 0x70 */ 0, 0, 0, 0, 0, 0, 0, 0,
/* 0x78 */ 0, 0, 0, 0, 0, 0, 0, 0
};
#endif



/* The following two defines control which device addresses  */
/* are considered valid. With the following values it should */
/* treat addresses 0-7 as valid to decode.                   */
/* Remember there are 5 address bits. These defines are in   */
/* the format of the lowest 5 bits.                          */
#define DEVICE_ADDRESS_MASK  0x18
#define DEVICE_DATA_MASK  0xff
#define DEVICE_ADDRESS_MATCH 0x00

#endif /* __IR38KTABLE_H_ */

/****************************************************************************
 * Modifications:
 * $Log: 
 *  1    mpeg      1.0         6/24/04 10:08:09 PM    Xiao Guang Yan  CR(s) 
 *        9583 9584 : Ir38k key table.
 * $
 ****************************************************************************/ 

