/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                 Conexant Systems Inc. (c) 1998-2002                      */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:      rc38ktabl.h
 *
 * Description:   Header file for the NEC IR decoder driver (rc38kdecode)
 *
 * Author:        Sunbey Tu
 *
 ****************************************************************************/
/* $Header: rc38ktabl.h, 2, 07/12/04 2:10:48 PM, Sunbey Tu$
 ****************************************************************************/

#ifndef _RC38KTABL_H
#define _RC38KTABL_H

#include "kal.h"
#define TABLE_SIZE 0x80

/* This table is a best composite of supported codes for the SC6122   */
//#if IR_D == 1

#if (defined(REMOTE_CONTROL_BLACK) && (REMOTE_CONTROL_BLACK == YES))
u_int32 rc38k_xlate_table[] =
{
/* 0x00 */ CNXT_RED,0,CNXT_DOWNARROW,CNXT_UPARROW, 0,0,CNXT_PAGEDOWN,CNXT_ENTER,
/* 0x08 */ CNXT_YELLOW,CNXT_1,CNXT_BLUE, 0,CNXT_GREEN,CNXT_4,CNXT_LARROW,0, 
/* 0x10 */ CNXT_FN,CNXT_7,CNXT_0,CNXT_PAGEUP, CNXT_MENU,CNXT_8,0,CNXT_9,
/* 0x18 */ CNXT_EPG,CNXT_5,CNXT_RARROW,CNXT_6,CNXT_RATOTV, CNXT_2,0,CNXT_3, 
/* 0x20 */ 0, 0, 0, 0, 0, 0, 0, 0,
/* 0x28 */ 0, 0, 0, 0, 0, 0, 0, 0,
/* 0x30 */ 0, 0, 0, 0, 0, 0, 0, 0,
/* 0x38 */ 0, 0, 0, 0, 0, 0, 0, 0,
/* 0x40 */ 0,CNXT_MUTE,0,0,0,CNXT_POWER, 0,0,
/* 0x48 */ CNXT_CHANNEL,CNXT_BUY,CNXT_VOD,0,0,0,CNXT_MAIL,CNXT_DATEBOAST,
/* 0x50 */ CNXT_CINAME, CNXT_SORT, CNXT_VOLDN, CNXT_VOLUP, 0, 0, CNXT_LAST,CNXT_TRACK,
/* 0x58 */ 0, 0, 0, 0, CNXT_EXIT, 0, 0, 0,
/* 0x60 */ 0, 0, 0, 0, 0, 0, 0, 0,
/* 0x68 */ 0, 0, 0, 0, 0, 0, 0, 0,
/* 0x70 */ 0, 0, 0, 0, 0, 0, 0, 0,
/* 0x78 */ 0, 0, 0, 0, 0, 0, 0, 0
};
#endif

#if (defined(REMOTE_CONTROL_WHITE) && (REMOTE_CONTROL_WHITE == YES))
u_int32 rc38k_xlate_table[] =
{
/* 0x00 */ CNXT_ENTER,CNXT_1,CNXT_2,CNXT_3,CNXT_4,CNXT_5,CNXT_6,CNXT_7,
/* 0x08 */ CNXT_8,CNXT_9,CNXT_MUTE,CNXT_RSV0,CNXT_POWER,CNXT_PAGEUP,CNXT_SAT,CNXT_LARROW,
/* 0x10 */ //CNXT_LAST, CNXT_PAGEDOWN, CNXT_UPARROW, CNXT_BLUE, CNXT_GUIDE, CNXT_RARROW, CNXT_VIEW, 0,
                 CNXT_LAST, CNXT_PAGEDOWN, CNXT_UPARROW, CNXT_MAIL, CNXT_RED, CNXT_RARROW, CNXT_GREEN, CNXT_BLUE,
/* 0x18 */ //CNXT_0, CNXT_INFO, CNXT_GREEN, CNXT_YELLOW, CNXT_EXIT, CNXT_DOWNARROW, CNXT_MENU, CNXT_TV,
                 CNXT_0, CNXT_INFO, 0, CNXT_TVLIST, CNXT_EXIT, CNXT_DOWNARROW, CNXT_MENU, CNXT_YELLOW,
/* 0x20 */ 0, 0, 0, 0, 0, 0, 0, 0,
/* 0x28 */ 0, 0, 0, 0, 0, 0, 0, 0,
/* 0x30 */ 0, 0, 0, 0, 0, 0, 0, 0,
/* 0x38 */ 0, 0, 0, 0, 0, 0, 0, 0,
/* 0x40 */ CNXT_VOLUP, 0, 0, 0, CNXT_VOLDN, 0, 0, 0,
/* 0x48 */ CNXT_RSV2, 0, 0, 0, CNXT_GAME, 0, 0, 0,
/* 0x50 */ CNXT_BUY, 0, 0, 0, CNXT_VIEW, 0, 0, 0,
/* 0x58 */ CNXT_RSV3, 0, 0, 0, CNXT_PAUSE, 0, 0, 0,
/* 0x60 */ 0, 0, 0, 0, 0, 0, 0, 0,
/* 0x68 */ 0, 0, 0, 0, 0, 0, 0, 0,
/* 0x70 */ 0, 0, 0, 0, 0, 0, 0, 0,
/* 0x78 */ 0, 0, 0, 0, 0, 0, 0, 0
};
#endif
//#else
//u_int32 rc38k_xlate_table[] = {
///* 0x00 */ 0, CNXT_4,     CNXT_TV,     CNXT_MUTE, 0,              CNXT_ENTER, CNXT_8,      0,
///* 0x08 */ 0, CNXT_POWER, CNXT_VOLDN,  CNXT_SAT,  CNXT_EXIT,      CNXT_1,     CNXT_CHANUP, CNXT_RSV0,
///* 0x10 */ 0, CNXT_5,     CNXT_9,      0,         0,              CNXT_6,     CNXT_0,      CNXT_7, 
///* 0x18 */ 0, CNXT_2,     CNXT_CHANDN, CNXT_3,    CNXT_MENU,      0,          CNXT_VOLUP,  CNXT_RED,
///* 0x20 */ 0, 0, 0, 0, 0, 0, 0, 0,
///* 0x28 */ 0, 0, 0, 0, 0, 0, 0, 0,
///* 0x30 */ 0, 0, 0, 0, 0, 0, 0, 0,
///* 0x38 */ 0, 0, 0, 0, 0, 0, 0, 0,
///* 0x40 */ CNXT_YELLOW,   CNXT_BLUE,   0, 0,      CNXT_PAUSE,     0,          0,           0,
///* 0x48 */ 0, 0, 0, 0, CNXT_INFO, CNXT_VIEW, 0, 0,
///* 0x50 */ 0, CNXT_BUY, 0, CNXT_GREEN, 0, 0, 0, 0,
///* 0x58 */ 0, 0, 0, 0, CNXT_GUIDE, 0, 0, 0,
///* 0x60 */ 0, 0, 0, 0, 0, 0, 0, 0,
///* 0x68 */ 0, 0, 0, 0, 0, 0, 0, 0,
///* 0x70 */ 0, 0, 0, 0, 0, 0, 0, 0,
///* 0x78 */ 0, 0, 0, 0, 0, 0, 0, 0
//};

//#endif

/* The following two defines control which device addresses  */
/* are considered valid. With the following values it should */
/* treat addresses 0-7 as valid to decode.                   */
/* Remember there are 5 address bits. These defines are in   */
/* the format of the lowest 5 bits.                          */
#define DEVICE_ADDRESS_MASK  0x18
#define DEVICE_DATA_MASK  0xff
#define DEVICE_ADDRESS_MATCH 0x00

#endif /* _RC5KTABLE_H */
 /****************************************************************************
 * Modifications:
 *    Rev 1.0   12 Jul 2004 10:15:32   sunbey
 * SCR(s):  
 * Key translation table and settings for which device addresses to support
 * for new NEC IR decoder (IR38K module)
 * 
 ****************************************************************************/ 



































































