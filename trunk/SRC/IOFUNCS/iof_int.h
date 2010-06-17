/****************************************************************************/
/*                   Conexant Proprietary and Confidential                  */
/*                       Software File/Module Header                        */
/*              Conexant Systems Inc. (c) 1998, 1999, 2000, 2001            */
/*                               Austin, TX                                 */
/*                            All Rights Eeserved                           */
/****************************************************************************/
/*
 * Filename:       IOF_INT.H
 *
 *
 * Description:    Internal header for IOFUNCS module sources.
 *
 *
 * Author:         Dave Wilson
 *
 ****************************************************************************/
/* $Header: iof_int.h, 2, 9/3/02 7:53:52 PM, Matt Korte$
 ****************************************************************************/

#ifndef _IOF_INT_H_
#define _IOF_INT_H_

/******************************************/
/* Local definitions and global variables */
/******************************************/
#define TR_INFO (TRACE_CTL|TRACE_LEVEL_3)
#define TR_ERR  (TRACE_CTL|TRACE_LEVEL_ALWAYS)
#define TR_FUNC (TRACE_CTL|TRACE_LEVEL_2)

#define IOFUNCS_TIMEOUT_TICK 1
#define IOFUNCS_CURSOR_TICK  2

#define CURSOR_FLASH_PERIOD 250

extern int gnOsdMaxHeight;
extern int gnOsdMaxWidth;

extern queue_id_t qKeys;
extern int iMouseCursorX, iMouseCursorY;

/********************************/       
/* Internal function prototypes */
/********************************/
bool input_devices_init(void);

IOFUNCS_STATUS internal_post_command_message(IOFUNCS_MESSAGE, u_int32, u_int32, u_int32);

#endif /* _IOF_INT_H_ */

/****************************************************************************
 * Modifications:
 * $Log: 
 *  2    mpeg      1.1         9/3/02 7:53:52 PM      Matt Korte      SCR(s) 
 *        4498 :
 *        Remove warnings
 *        
 *  1    mpeg      1.0         11/27/01 12:00:22 PM   Dave Wilson     
 * $
 * 
 *    Rev 1.1   03 Sep 2002 18:53:52   kortemw
 * SCR(s) 4498 :
 * Remove warnings
 * 
 *    Rev 1.0   27 Nov 2001 12:00:22   dawilson
 * SCR(s) 2927 :
 * 
 *
 ****************************************************************************/
