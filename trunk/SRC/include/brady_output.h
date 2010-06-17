/*****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                   */
/*                       SOFTWARE FILE/MODULE HEADER                         */
/*                      Conexant Systems Inc. (c) 2003                       */
/*                             Bristol, UK                                   */
/*                         All Rights Reserved                               */
/*****************************************************************************/
/*
 * Filename:    brady_output.h
 *
 *
 * Description: Header file for Brady-specific outputs. At first, just the
 *              distinctive front-panel LEDs.
 *
 * Author:      Ian Mitchell, Steve Jones
 *
 *****************************************************************************/
/* $Header: brady_output.h, 2, 1/24/03 8:09:58 AM, Steven Jones$
 *****************************************************************************/

/* Check that the file has not already been included *************************/
#ifndef BRADY_OUTPUT_H
#define BRADY_OUTPUT_H

typedef enum
{
   BRADY_LED_GREEN,
   BRADY_LED_RED,
   BRADY_LED_AMBER,
   BRADY_LED_OFF
} BRADY_LED_COMMAND; /* ("blc" Prefix) */

typedef enum
{
   BRADY_OUTPUT_SUCCESS          = 0,
   BRADY_OUTPUT_COMMAND_NOT_EXIST
}
BRADY_OUTPUT_STATUS; /* ("bos" Prefix) */

BRADY_OUTPUT_STATUS  cnxt_brady_led(BRADY_LED_COMMAND blcCmd);

#endif /* BRADY_OUTPUT_H */

/****************************************************************************
* Modifications:
* $Log: 
*  2    mpeg      1.1         1/24/03 8:09:58 AM     Steven Jones    SCR(s): 
*        5310 
*        Make sure success exit code equals zero.
*        
*  1    mpeg      1.0         1/13/03 9:42:58 AM     Steven Jones    
* $
 * 
 *    Rev 1.1   24 Jan 2003 08:09:58   joness
 * SCR(s): 5310 
 * Make sure success exit code equals zero.
 * 
 *    Rev 1.0   13 Jan 2003 09:42:58   joness
 * SCR(s): 5234 
 * New API header file for Brady-specific output modules, e.g. front-panel LEDs.
*
*****************************************************************************/
