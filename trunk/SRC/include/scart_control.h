/*****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                   */
/*                       SOFTWARE FILE/MODULE HEADER                         */
/*                      Conexant Systems Inc. (c) 2003                       */
/*                             Bristol, UK                                   */
/*                         All Rights Reserved                               */
/*****************************************************************************/
/*
 * Filename:    scart_control.h
 *
 *
 * Description: Header file for controlling the voltages of the pins in the
 *              SCART socket.
 *
 * Author:      Ian Mitchell, Steve Jones
 *
 *****************************************************************************/
/* $Header: scart_control.h, 2, 1/24/03 8:13:48 AM, Steven Jones$
 *****************************************************************************/

/* Check that the file has not already been included *************************/
#ifndef SCART_CONTROL_H
#define SCART_CONTROL_H

typedef enum
{
   SCART_4_3,
   SCART_16_9,
   SCART_OFF
} SCART_AR;          /* ("sar" Prefix) */

typedef enum
{
   SCART_FORMAT_RGB,
   SCART_FORMAT_COMPOSITE
} SCART_FORMAT;      /* ("sf" Prefix) */

typedef enum
{
   SCART_SUCCESS           = 0,
   SCART_COMMAND_NOT_EXIST
}
SCART_RETURN_CODE;   /* ("src" Prefix) */

SCART_RETURN_CODE cnxt_scart_AR(SCART_AR sarAspect);
SCART_RETURN_CODE cnxt_scart_format(SCART_FORMAT sfFormat);

#endif /* SCART_CONTROL_H */

/****************************************************************************
* Modifications:
* $Log: 
*  2    mpeg      1.1         1/24/03 8:13:48 AM     Steven Jones    SCR(s): 
*        5309 
*        Add SCART_OFF value to input enum, ensure success exit code equals 
*        zero.
*        
*  1    mpeg      1.0         1/13/03 6:42:38 AM     Steven Jones    
* $
 * 
 *    Rev 1.1   24 Jan 2003 08:13:48   joness
 * SCR(s): 5309 
 * Add SCART_OFF value to input enum, ensure success exit code equals zero.
 * 
 *    Rev 1.0   13 Jan 2003 06:42:38   joness
 * SCR(s): 5231 
 * API for new SCART-pin driver.
*
*****************************************************************************/
