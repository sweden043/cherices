/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*           Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002         */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       setmenu.h
 *
 *
 * Description:    Header for simple module allowing user viewing and entry 
 *                 of EEPROM-stored configuration parameters.
 *
 * Author:         Dave Wilson
 *
 ****************************************************************************/
/* $Header: setmenu.h, 1, 2/28/02 9:09:56 AM, Dave Wilson$
 ****************************************************************************/
#ifndef _SETMENU_H_
#define _SETMENU_H_

/***************************************/
/* Type definitions used by the module */
/***************************************/
typedef enum _SETMENU_STATUS
{
  SETMENU_STATUS_OK,
  SETMENU_STATUS_REINIT,
  SETMENU_ERROR_INTERNAL,
  SETMENU_ERROR_NOT_INIT,
  SETMENU_ERROR_BAD_PTR
} SETMENU_STATUS;

/* Number of colors required in pColors array passed to cnxt_setmenu_init */
#define NUM_SETMENU_COLORS 5

#define SETMENU_SCREEN_BG_INDEX 0
#define SETMENU_TEXT_BG_INDEX   1
#define SETMENU_TEXT_INDEX      2
#define SETMENU_HEAD_INDEX      3
#define SETMENU_FOCUS_INDEX     4

/*************************/
/* Module API Prototypes */
/*************************/
SETMENU_STATUS cnxt_setmenu_init(IOFUNCS_COLOR *pColors);
SETMENU_STATUS cnxt_setmenu_shutdown(void);
SETMENU_STATUS cnxt_setmenu_show(void);
SETMENU_STATUS cnxt_setmenu_update(void);

#endif /* _SETMENU_H_ */

/****************************************************************************
 * Modifications:
 * $Log: 
 *  1    mpeg      1.0         2/28/02 9:09:56 AM     Dave Wilson     
 * $
 * 
 *    Rev 1.0   28 Feb 2002 09:09:56   dawilson
 * SCR(s) 2994 :
 * Header for SETMENU module
 *
 ****************************************************************************/

