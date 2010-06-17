/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                   Conexant Systems Inc. (c) 1998 - 2003                  */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       GAMEPAD.H
 *
 *
 * Description:    OpenTV 1.2 BSkyB Game Controller Driver Header
 *
 *
 * Author:         Dave Wilson
 *
 ****************************************************************************/
/* $Header: gamepad.h, 1, 8/14/03 7:15:24 AM, Dave Wilson$
 ****************************************************************************/
#ifndef _GAMEPAD_H_
#define _GAMEPAD_H_

/***********************/
/* Function Prototypes */
/***********************/
bool gamepad_init(void);
void gamepad_filter_function(xyman_device_enum device,
                             u_int16 button_mask,
                             int move_flag);

#endif


/****************************************************************************
 * Modifications:
 * $Log: 
 *  1    mpeg      1.0         8/14/03 7:15:24 AM     Dave Wilson     
 * $
 * 
 *    Rev 1.0   14 Aug 2003 06:15:24   dawilson
 * SCR(s): 7256 7257 
 * Header file for BSkyB/OpenTV 1.2 gamepad driver
 *
 ****************************************************************************/

