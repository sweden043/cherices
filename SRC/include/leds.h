/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*           Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002         */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       LEDS.H
 *
 *
 * Description:    Low Level LED Access Function Prototypes
 *
 *
 * Author:         Dave Wilson
 *
 ****************************************************************************/
/* $Header: leds.h, 2, 1/17/03 5:08:12 PM, Dave Wilson$
 ****************************************************************************/

#ifndef _LEDS_H_
#define _LEDS_H_

/********************/
/* Basic LED states */
/********************/
typedef enum _FP_LED_STATE
{
  CNXT_LED_DARK = 0,
  CNXT_LED_LIT
} FP_LED_STATE;

/*************************/
/* Function return codes */
/*************************/
typedef enum _FP_LED_STATUS
{
  FP_LED_STATUS_OK,
  FP_LED_ERROR_INVALID_STATE,
  FP_LED_ERROR_UNDEFINED_LED,
  FP_LED_ERROR_BAD_GPIO,
  FP_LED_BAD_PTR
} FP_LED_STATUS;

/*******************************/
/* Exported function prototype */
/*******************************/
FP_LED_STATUS cnxt_led_initialize(bool bRTOSUp);
FP_LED_STATUS cnxt_led_set(u_int32 uLedGpio, FP_LED_STATE eState);
FP_LED_STATUS cnxt_led_get(u_int32 uLedGpio, FP_LED_STATE *peState);

#endif /* _LEDS_H_ */
/****************************************************************************
 * Modifications:
 * $Log: 
 *  2    mpeg      1.1         1/17/03 5:08:12 PM     Dave Wilson     SCR(s) 
 *        5264 :
 *        Added new value to FP_LED_STATUS - FP_LED_ERROR_BAD_GPIO
 *        
 *  1    mpeg      1.0         12/16/02 2:03:06 PM    Dave Wilson     
 * $
 * 
 *    Rev 1.1   17 Jan 2003 17:08:12   dawilson
 * SCR(s) 5264 :
 * Added new value to FP_LED_STATUS - FP_LED_ERROR_BAD_GPIO
 * 
 *    Rev 1.0   16 Dec 2002 14:03:06   dawilson
 * SCR(s) 5177 :
 * Public header for generic LED control functions.
 *
 ****************************************************************************/

