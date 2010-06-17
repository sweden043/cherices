/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                 Conexant Systems Inc. (c) 1998-2002                      */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:      rc5ktabl.h
 *
 * Description:   Header file for the new RC5 IR decoder driver (irrc5new)
 *
 * Author:        Steve Glennon
 *
 ****************************************************************************/
/* $Header: rc5ktabl.h, 2, 10/24/02 2:10:48 PM, Steve Glennon$
 ****************************************************************************/

#ifndef _RC5KTABL_H
#define _RC5KTABL_H

#include "kal.h"
#define TABLE_SIZE 0x80

/* This table is a best composite of supported codes for the GE/Jasco */
/* universal remote RC94927 in TV settings 183, 184 and 213           */
/* It is best optimized for setting 213.                              */

u_int32 rc5_xlate_table[] = {
/* 0x00 */ CNXT_0, CNXT_1, CNXT_2, CNXT_3, CNXT_4, CNXT_5, CNXT_6, CNXT_7,
/* 0x08 */ CNXT_8, CNXT_9, CNXT_ENTER, 0, CNXT_POWER, CNXT_MUTE, 0, CNXT_ENTER,
/* 0x10 */ CNXT_VOLUP, CNXT_VOLDN, 0, 0, CNXT_GREEN, 0, 0, 0,
/* 0x18 */ 0, 0, CNXT_RED, 0, CNXT_UP, CNXT_DOWN, 0, 0,
/* 0x20 */ CNXT_CHANUP, CNXT_CHANDN, CNXT_LAST, 0, 0, 0, 0, 0,
/* 0x28 */ 0, CNXT_PAUSE, 0, CNXT_RIGHT, CNXT_LEFT, 0, CNXT_MENU, 0,
/* 0x30 */ 0, 0, CNXT_REVERSE, 0, CNXT_FORWARD, CNXT_PLAY, CNXT_GUIDE, CNXT_RECORD,
/* 0x38 */ CNXT_YELLOW, CNXT_REFRESH, 0, 0, 0, 0, CNXT_DISPLAY, 0,
/* 0x40 */ 0, 0, 0, 0, 0, 0, 0, 0,
/* 0x48 */ 0, 0, 0, 0, 0, 0, 0, 0,
/* 0x50 */ 0, 0, 0, 0, 0, 0, 0, 0,
/* 0x58 */ 0, 0, 0, 0, 0, 0, 0, 0,
/* 0x60 */ 0, 0, 0, 0, 0, 0, 0, 0,
/* 0x68 */ 0, 0, 0, 0, 0, 0, 0, 0,
/* 0x70 */ 0, 0, 0, 0, 0, 0, 0, 0,
/* 0x78 */ 0, 0, 0, 0, 0, 0, 0, 0
};

/* The following two defines control which device addresses  */
/* are considered valid. With the following values it should */
/* treat addresses 0-7 as valid to decode.                   */
/* Remember there are 5 address bits. These defines are in   */
/* the format of the lowest 5 bits.                          */
#define DEVICE_ADDRESS_MASK  0x18
#define DEVICE_ADDRESS_MATCH 0x00

#endif /* _RC5KTABLE_H */
 /****************************************************************************
 * Modifications:
 * $Log: 
 *  2    mpeg      1.1         10/24/02 2:10:48 PM    Steve Glennon   SCR(s): 
 *        4834 
 *        Modified code table to match codes from GE/Jasco RC94927 on TV code 
 *        163.
 *        Also modified the PIP/SWAP/INPUT keys to generate RED/GREEN/BLUE 
 *        CXNT_ codes as these are required for demoing BSkyB boxes.
 *        
 *        
 *  1    mpeg      1.0         10/15/02 6:23:42 PM    Steve Glennon   
 * $
 * 
 *    Rev 1.1   24 Oct 2002 13:10:48   glennon
 * SCR(s): 4834 
 * Modified code table to match codes from GE/Jasco RC94927 on TV code 163.
 * Also modified the PIP/SWAP/INPUT keys to generate RED/GREEN/BLUE CXNT_ codes as these are required for demoing BSkyB boxes.
 * 
 * 
 *    Rev 1.0   15 Oct 2002 17:23:42   glennon
 * SCR(s): 4796 
 * Key translation table and settings controlling which remote device addresses
 * are considered valid for decode.
 * 
 * 
 *    Rev 1.0   15 Oct 2002 16:17:32   glennon
 * SCR(s): 4796 
 * Key translation table and settings for which device addresses to support
 * for new style RC5 IR decoder (irrc5new module)
 * 
 ****************************************************************************/ 



































































