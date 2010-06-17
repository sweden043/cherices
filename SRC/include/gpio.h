/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*           Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002         */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       GPIO.H
 *
 *
 * Description:    Low Level GPIO access function prototypes
 *
 *
 * Author:         Dave Wilson
 *
 ****************************************************************************/
/* $Header:   
 ****************************************************************************/

#ifndef _GPIO_H_
#define _GPIO_H_

/*************************/
/* Function return codes */
/*************************/
typedef enum _CNXT_GPIO_STATUS
{
  CNXT_GPIO_STATUS_OK,
  CNXT_GPIO_INVALID_DEVICE,
  CNXT_GPIO_INVALID_NUMBER,
  CNXT_GPIO_INVALID_TYPE,
  CNXT_GPIO_INVALID_STATE,
  CNXT_GPIO_INVALID_EDGE,
  CNXT_GPIO_BAD_PTR,
  CNXT_GPIO_KAL_BADPTR,
  CNXT_GPIO_KAL_INVALID,
  CNXT_GPIO_KAL_INUSE,
  CNXT_GPIO_KAL_UNKNOWN,
  CNXT_GPIO_KAL_NOTHOOKED
} CNXT_GPIO_STATUS;

/*******************************/
/* Exported function prototype */
/*******************************/

/********************************************************************/
/*  FUNCTION:    cnxt_gpio_set_output_level                         */
/*                                                                  */
/*  PARAMETERS:  uGPIO    - General GPIO identifier for the pin     */
/*                          whose logic level is to be set.         */
/*               bOn      - TRUE to set logic level 1, FALSE for 0  */
/*                                                                  */
/*  DESCRIPTION: This function sets the state of an individual GPIO */
/*               output to logical 1 or 0. The output level is      */
/*               determined based upon whether the GPIO is marked   */
/*               GPIO_NEGATIVE_POLARITY or GPIO_POSITIVE_POLARITY.  */
/*                                                                  */
/*  RETURNS:     CNXT_GPIO_STATUS_OK,                               */
/*               CNXT_GPIO_INVALID_DEVICE,                          */
/*               CNXT_GPIO_INVALID_NUMBER,                          */
/*               CNXT_GPIO_INVALID_TYPE                             */
/*                                                                  */
/*  CONTEXT:     Interrupt safe if the GPIO passed is NOT connected */
/*               to an I2C GPIO extender on the target board.       */
/*                                                                  */
/********************************************************************/
CNXT_GPIO_STATUS cnxt_gpio_set_output_level(u_int32 uGPIO, bool bOn);

/********************************************************************/
/*  FUNCTION:    cnxt_gpio_get_level                                */
/*                                                                  */
/*  PARAMETERS:  uGPIO    - General GPIO identifier for the pin     */
/*                          whose logic level is to be set.         */
/*               pbOn     - Pointer to storage for returned logic   */
/*                          level.                                  */
/*                                                                  */
/*  DESCRIPTION: This function returns the current value of a GPIO  */
/*               pin. If it is an output, the last value written    */
/*               will be returned, else the value will indicate the */
/*               level on an input pin. The logic level is written  */
/*               to the address pointed to by pbOn.                 */
/*                                                                  */
/*               Note that the returned logic level takes into      */
/*               account the state of the GPIO polarity bit in the  */
/*               generic GPIO descriptor passed to the function.    */
/*                                                                  */
/*  RETURNS:     CNXT_GPIO_STATUS_OK,                               */
/*               CNXT_GPIO_INVALID_DEVICE,                          */
/*               CNXT_GPIO_INVALID_NUMBER,                          */
/*               CNXT_GPIO_INVALID_TYPE,                            */
/*               CNXT_GPIO_BAD_PTR                                  */
/*                                                                  */
/*  CONTEXT:     Interrupt safe if the GPIO passed is NOT connected */
/*               to an I2C GPIO extender on the target board.       */
/*                                                                  */
/********************************************************************/
CNXT_GPIO_STATUS cnxt_gpio_get_level(u_int32 uGPIO, bool *pbOn);

/********************************************************************/
/*  FUNCTION:    cnxt_gpio_set_input                                */
/*                                                                  */
/*  PARAMETERS:  uGPIO    - General GPIO identifier for the pin     */
/*                          which is to be set as an input.         */
/*                                                                  */
/*  DESCRIPTION: This function sets the supplied GPIO pin as an     */
/*               input assuming this is possible.                   */
/*                                                                  */
/*  RETURNS:     CNXT_GPIO_STATUS_OK,                               */
/*               CNXT_GPIO_INVALID_DEVICE,                          */
/*               CNXT_GPIO_INVALID_NUMBER,                          */
/*               CNXT_GPIO_INVALID_TYPE,                            */
/*               CNXT_GPIO_INVALID_STATE                            */
/*                                                                  */
/*  CONTEXT:     Interrupt safe if the GPIO passed is NOT connected */
/*               to an I2C GPIO extender on the target board.       */
/*                                                                  */
/********************************************************************/
CNXT_GPIO_STATUS cnxt_gpio_set_input(u_int32 uGPIO);

/********************************************************************/
/*  FUNCTION:    cnxt_gpio_set_int_edge                             */
/*                                                                  */
/*  PARAMETERS:  uGPIO    - General GPIO identifier for the pin     */
/*                          which is to be set as an input.         */
/*               uEdge    - POS_EDGE, NEG_EDGE or BOTH_EDGES        */
/*                                                                  */
/*  DESCRIPTION: This function sets the supplied GPIO pin to        */
/*               interrupt on the transition specified. Note that   */
/*               the edge is described in terms of logic level and  */
/*               this function takes account of the positive or     */
/*               negative logic specified in the uGPIO descriptor   */
/*               passed.                                            */
/*                                                                  */
/*  RETURNS:     CNXT_GPIO_STATUS_OK,                               */
/*               CNXT_GPIO_INVALID_DEVICE,                          */
/*               CNXT_GPIO_INVALID_NUMBER,                          */
/*               CNXT_GPIO_INVALID_TYPE,                            */
/*               CNXT_GPIO_INVALID_STATE                            */
/*                                                                  */
/*  CONTEXT:     Interrupt safe if the GPIO passed is NOT connected */
/*               to an I2C GPIO extender on the target board.       */
/*                                                                  */
/********************************************************************/
CNXT_GPIO_STATUS cnxt_gpio_set_int_edge(u_int32 uGPIO, u_int32 uEdge);

/********************************************************************/
/*  FUNCTION:    cnxt_gpio_int_register_isr                         */
/*                                                                  */
/*  PARAMETERS:  uGPIO    - General GPIO identifier for the pin     */
/*                          which is to be set as an input.         */
/*               pfnHandler, bFIQ, bInvert, pfnChan - values passed */
/*                  through to the KAL/HWLIB int function           */
/*                                                                  */
/*  DESCRIPTION: This function is a "thin" wrapper function that    */
/*               will call the appropriate interrupt register       */
/*               function(s) for the GPIO                           */
/*  RETURNS:     CNXT_GPIO_STATUS_OK,                               */
/*               CNXT_GPIO_KAL_BADPTR,                              */
/*               CNXT_GPIO_KAL_INUSE,                               */
/*               CNXT_GPIO_KAL_INVALID,                             */
/*                                                                  */
/*  CONTEXT:     Interrupt safe if the GPIO passed is NOT connected */
/*               to an I2C GPIO extender on the target board.       */
/*                                                                  */
/********************************************************************/
CNXT_GPIO_STATUS cnxt_gpio_int_register_isr(
    u_int32 uGPIO,
    PFNISR  pfnHandler,
    bool    bFIQ,
    bool    bInvert,
    PFNISR *pfnChain);

/********************************************************************/
/*  FUNCTION:    cnxt_gpio_int_query_id                             */
/*                                                                  */
/*  PARAMETERS:  uGPIO    - General GPIO identifier for the pin     */
/*                          which is to be set as an input.         */
/*               uIntId   - Interrupt Id to query/compare           */
/*                                                                  */
/*  DESCRIPTION: This function will query the General GPIO          */
/*               Interrupt ID, and compare to passed uIntID         */
/*               If match, returns OK                               */
/*  RETURNS:     CNXT_GPIO_STATUS_OK,                               */
/*               CNXT_GPIO_INVALID_NUMBER,                          */
/*                                                                  */
/*  CONTEXT:     Interrupt safe if the GPIO passed is NOT connected */
/*               to an I2C GPIO extender on the target board.       */
/*                                                                  */
/********************************************************************/
CNXT_GPIO_STATUS cnxt_gpio_int_query_id(u_int32 uGPIO, u_int32 uIntId);

/********************************************************************/
/*  FUNCTION:    cnxt_gpio_int_disable                              */
/*                                                                  */
/*  PARAMETERS:  uGPIO    - General GPIO identifier for the pin     */
/*                          which is to be set as an input.         */
/*                                                                  */
/*  DESCRIPTION: This function is a "thin" wrapper function that    */
/*               will call the appropriate interrupt disable        */
/*               function(s) for the GPIO                           */
/*  RETURNS:     CNXT_GPIO_STATUS_OK,                               */
/*               CNXT_GPIO_KAL_BADPTR,                              */
/*               CNXT_GPIO_KAL_INUSE,                               */
/*               CNXT_GPIO_KAL_INVALID,                             */
/*                                                                  */
/*  CONTEXT:     Interrupt safe if the GPIO passed is NOT connected */
/*               to an I2C GPIO extender on the target board.       */
/*                                                                  */
/********************************************************************/
CNXT_GPIO_STATUS cnxt_gpio_int_disable(u_int32 uGPIO);

/********************************************************************/
/*  FUNCTION:    cnxt_gpio_int_enable                               */
/*                                                                  */
/*  PARAMETERS:  uGPIO    - General GPIO identifier for the pin     */
/*                          which is to be set as an input.         */
/*                                                                  */
/*  DESCRIPTION: This function is a "thin" wrapper function that    */
/*               will call the appropriate interrupt enable         */
/*               function(s) for the GPIO                           */
/*  RETURNS:     CNXT_GPIO_STATUS_OK,                               */
/*               CNXT_GPIO_KAL_BADPTR,                              */
/*               CNXT_GPIO_KAL_INUSE,                               */
/*               CNXT_GPIO_KAL_INVALID,                             */
/*                                                                  */
/*  CONTEXT:     Interrupt safe if the GPIO passed is NOT connected */
/*               to an I2C GPIO extender on the target board.       */
/*                                                                  */
/********************************************************************/
CNXT_GPIO_STATUS cnxt_gpio_int_enable(u_int32 uGPIO);

/********************************************************************/
/*  FUNCTION:    cnxt_gpio_clear_pic_interrupt                      */
/*                                                                  */
/*  PARAMETERS:  uGPIO    - General GPIO identifier for the pin     */
/*                          which is to be set as an input.         */
/*                                                                  */
/*  DESCRIPTION: This function is a "thin" wrapper function that    */
/*               will call the appropriate call the clear pic       */
/*               intrrupt function for the designated GPIO          */
/*  RETURNS:     CNXT_GPIO_STATUS_OK,                               */
/*               CNXT_GPIO_KAL_BADPTR,                              */
/*               CNXT_GPIO_KAL_INUSE,                               */
/*               CNXT_GPIO_KAL_INVALID,                             */
/*                                                                  */
/*  CONTEXT:     Interrupt safe if the GPIO passed is NOT connected */
/*               to an I2C GPIO extender on the target board.       */
/*                                                                  */
/********************************************************************/
CNXT_GPIO_STATUS cnxt_gpio_clear_pic_interrupt(u_int32 uGPIO);

/********************************************************************/
/*  FUNCTION:    cnxt_gpio_int_query_enabled                        */
/*                                                                  */
/*  PARAMETERS:  uGPIO    - General GPIO identifier for the pin     */
/*                          which is to be set as an input.         */
/*               bEnabled - boolean to indicate enabled (TRUE) or   */
/*                          not (FALSE)                             */
/*                                                                  */
/*  DESCRIPTION: This function is a "thin" wrapper function that    */
/*               will call the appropriate interrupt disable        */
/*               function(s) for the GPIO                           */
/*  RETURNS:     CNXT_GPIO_STATUS_OK,                               */
/*               CNXT_GPIO_KAL_BADPTR,                              */
/*               CNXT_GPIO_KAL_INUSE,                               */
/*               CNXT_GPIO_KAL_INVALID,                             */
/*                                                                  */
/*  CONTEXT:     Interrupt safe if the GPIO passed is NOT connected */
/*               to an I2C GPIO extender on the target board.       */
/*                                                                  */
/********************************************************************/
CNXT_GPIO_STATUS cnxt_gpio_int_query_enabled(u_int32 uGPIO, bool *bEnabled);

/********************************************************************/
/*  FUNCTION:    cnxt_gpio_int_query_pending                        */
/*                                                                  */
/*  PARAMETERS:  uGPIO    - General GPIO identifier for the pin     */
/*                          which is to be set as an input.         */
/*               bPending - boolean to indicate pending (TRUE) or   */
/*                          not (FALSE)                             */
/*                                                                  */
/*  DESCRIPTION: This function is a "thin" wrapper function that    */
/*               will call the appropriate interrupt disable        */
/*               function(s) for the GPIO                           */
/*  RETURNS:     CNXT_GPIO_STATUS_OK,                               */
/*               CNXT_GPIO_KAL_BADPTR,                              */
/*               CNXT_GPIO_KAL_INUSE,                               */
/*               CNXT_GPIO_KAL_INVALID,                             */
/*                                                                  */
/*  CONTEXT:     Interrupt safe if the GPIO passed is NOT connected */
/*               to an I2C GPIO extender on the target board.       */
/*                                                                  */
/********************************************************************/
CNXT_GPIO_STATUS cnxt_gpio_int_query_pending(u_int32 uGPIO, bool *bPending);

#endif /* _GPIO_H_ */

/****************************************************************************
 * Modifications:
 * $Log: 
 *  3    mpeg      1.2         3/18/03 4:14:42 PM     Bobby Bradford  SCR(s) 
 *        5108 :
 *        Added two new functions to query enabled/pending status of
 *        interrupts based on the generic interrupt definition.
 *        
 *  2    mpeg      1.1         1/23/03 1:03:12 PM     Bobby Bradford  SCR(s) 
 *        5275 :
 *        Added new return code and function prototypes for general GPIO
 *        interrupt processing
 *        
 *  1    mpeg      1.0         1/17/03 4:35:54 PM     Dave Wilson     
 * $
 * 
 *    Rev 1.2   18 Mar 2003 16:14:42   bradforw
 * SCR(s) 5108 :
 * Added two new functions to query enabled/pending status of
 * interrupts based on the generic interrupt definition.
 * 
 *    Rev 1.1   23 Jan 2003 13:03:12   bradforw
 * SCR(s) 5275 :
 * Added new return code and function prototypes for general GPIO
 * interrupt processing
 * 
 *    Rev 1.0   17 Jan 2003 16:35:54   dawilson
 * SCR(s) 5264 :
 * Generic virtual GPIO access function header file
 *
 ****************************************************************************/

