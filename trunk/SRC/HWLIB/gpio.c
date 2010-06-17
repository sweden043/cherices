/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*           Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002         */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       gpio.c
 *
 *
 * Description:    Generic GPIO access functions
 *
 *
 * Author:         Dave Wilson
 *
 ****************************************************************************/
/* $Header: gpio.c, 8, 7/10/03 2:18:40 PM, Larry Wang$
 ****************************************************************************/

/*****************/
/* Include Files */
/*****************/
#include "stbcfg.h"
#include "retcodes.h"
#include "hwlib.h"
#include "hwfuncs.h"
#include "iic.h"
#include "gpio.h"
#if RTOS != NOOS
#include "trace.h"
#endif

/******************************************/
/* Local Definitions and Global Variables */
/******************************************/
u_int32 split_gpio_int(u_int32 iInt, u_int32 *uBank, u_int32 *uBitNum);

#if RTOS == NOOS
extern HW_DWORD CNXT_PIO_EXP_REG_SHADOW, CNXT_PIO_EXP_REG_2_SHADOW;
#else
/* Shadow variables for write-only ISA extenders */
#if (CNXT_PIO_EXP_TYPE == GPIO_EXTEND_IO)
#if (CNXT_PIO_EXP_REG != NOT_PRESENT)
HW_DWORD CNXT_PIO_EXP_REG_SHADOW;
#endif
#if (CNXT_PIO_EXP_REG_2 != NOT_PRESENT)
HW_DWORD CNXT_PIO_EXP_REG_2_SHADOW;
#endif
#endif
#endif

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
CNXT_GPIO_STATUS cnxt_gpio_set_output_level(u_int32 uGPIO, bool bOn)
{
  u_int32 uDevice;
  u_int16 uBit;
  bool    ks;
  CNXT_GPIO_STATUS eReturn = CNXT_GPIO_STATUS_OK;
  
  /* GPIO must be an output */
  if((uGPIO & GPIO_PIN_INPUT_MASK) == GPIO_PIN_IS_INPUT)
  {
    eReturn = CNXT_GPIO_INVALID_TYPE;
  }
  else
  {  
    /* Depending upon the device type, do the right thing */
    uDevice = uGPIO & GPIO_MASK_DEVICE_ID;
    uBit    = uGPIO & GPIO_MASK_PIN_NUMBER;
    
    switch (uDevice)
    {
      /* A normal, internal GPIO pin */
      case GPIO_DEVICE_ID_INTERNAL:
        ks = critical_section_begin();
        set_gpio_output_level(uBit, 
                              ((uGPIO & GPIO_POLARITY_MASK) == GPIO_POSITIVE_POLARITY) ? bOn : !bOn);
        critical_section_end(ks);                      
        break;
      
      #if (CNXT_PIO_EXP_TYPE == GPIO_EXTEND_IIC)
      /* First I2C GPIO extender */  
      #if (I2C_BUS_NIM_EXT != I2C_BUS_NONE)
      case GPIO_DEVICE_ID_I2C_1:
        if(uBit < 8)
        {
          if((uGPIO & GPIO_POLARITY_MASK) == GPIO_POSITIVE_POLARITY)
            write_gpio_extender ( (u_int8)( 1 << uBit ), bOn ? (u_int8)( 1 << uBit ) : (u_int8)0 );
          else  
            write_gpio_extender ( (u_int8)( 1 << uBit ), !bOn ? (u_int8)( 1 << uBit ) : (u_int8)0 );
        }  
        else
          eReturn = CNXT_GPIO_INVALID_NUMBER;  
        break;
      #endif  
        
      /* Second I2C GPIO extender */  
      #if (I2C_BUS_GPIO2_EXT != I2C_BUS_NONE)
      case GPIO_DEVICE_ID_I2C_2:
        if(uBit < 8)
        {                                 
          if((uGPIO & GPIO_POLARITY_MASK) == GPIO_POSITIVE_POLARITY)
            write_second_gpio_extender ( (u_int8)( 1 << uBit ), bOn ? (u_int8)( 1 << uBit ) : (u_int8)0);
          else  
            write_second_gpio_extender ( (u_int8)( 1 << uBit ), !bOn ? (u_int8)( 1 << uBit ) : (u_int8)0);
        }  
        else
          eReturn = CNXT_GPIO_INVALID_NUMBER;  
        break;
      #endif  
      #endif
        
      #if (CNXT_PIO_EXP_TYPE == GPIO_EXTEND_IO)
      /* First ISA-connected GPIO extender */
      #if (CNXT_PIO_EXP_REG != NOT_PRESENT)
      case GPIO_DEVICE_ID_ISA_1:
        ks = critical_section_begin();
        if( (((uGPIO & GPIO_POLARITY_MASK) == GPIO_POSITIVE_POLARITY) && bOn) ||
            (((uGPIO & GPIO_POLARITY_MASK) == GPIO_NEGATIVE_POLARITY) && !bOn))
          CNXT_PIO_EXP_REG_SHADOW |= (u_int16)( 1<<(uGPIO & GPIO_MASK_PIN_NUMBER));
        else  
          CNXT_PIO_EXP_REG_SHADOW &= (u_int16)~( 1<<(uGPIO & GPIO_MASK_PIN_NUMBER));
        *((LPREG)CNXT_PIO_EXP_REG) = CNXT_PIO_EXP_REG_SHADOW;
        critical_section_end(ks);                      
        break;
      #endif  
        
      /* Second ISA-connected GPIO extender */
      #if (CNXT_PIO_EXP_REG_2 != NOT_PRESENT)
      case GPIO_DEVICE_ID_ISA_2:
        ks = critical_section_begin();
        if( (((uGPIO & GPIO_POLARITY_MASK) == GPIO_POSITIVE_POLARITY) && bOn) ||
            (((uGPIO & GPIO_POLARITY_MASK) == GPIO_NEGATIVE_POLARITY) && !bOn))
          CNXT_PIO_EXP_REG_2_SHADOW |= (u_int16)( 1<<(uGPIO & GPIO_MASK_PIN_NUMBER));
        else  
          CNXT_PIO_EXP_REG_2_SHADOW &= (u_int16)~( 1<<(uGPIO & GPIO_MASK_PIN_NUMBER));
        *((LPREG)CNXT_PIO_EXP_REG_2) = CNXT_PIO_EXP_REG_2_SHADOW;
        critical_section_end(ks);                      
        break;
      #endif
      #endif
        
      /* An invalid device code was passed in the GPIO description */  
      default:
        eReturn = CNXT_GPIO_INVALID_DEVICE;
        break;
    }
  } 
  
  return(eReturn);  
}


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
CNXT_GPIO_STATUS cnxt_gpio_get_level(u_int32 uGPIO, bool *pbOn)
{
  u_int32 uDevice;
  u_int16 uBit;
  int     iLevel;
  bool    ks;
  CNXT_GPIO_STATUS eReturn = CNXT_GPIO_STATUS_OK;
  #if (CNXT_PIO_EXP_TYPE == GPIO_EXTEND_IIC)
  u_int8  cRead;
  #endif
  #if (CNXT_PIO_EXP_TYPE == GPIO_EXTEND_IO)
  u_int32 uValue;
  #endif
  
  if(!pbOn)
    return(CNXT_GPIO_BAD_PTR);
    
  /* Depending upon the device type, do the right thing */
  uDevice = uGPIO & GPIO_MASK_DEVICE_ID;
  uBit    = uGPIO & GPIO_MASK_PIN_NUMBER;
  
  switch (uDevice)
  {
    /* A normal, internal GPIO pin */
    case GPIO_DEVICE_ID_INTERNAL:
      ks = critical_section_begin();
      iLevel = get_gpio_input(uBit);
      *pbOn =  (((uGPIO & GPIO_POLARITY_MASK) == GPIO_POSITIVE_POLARITY) ? (iLevel != 0) : (iLevel == 0));
      critical_section_end(ks);                      
      break;
    
    #if (CNXT_PIO_EXP_TYPE == GPIO_EXTEND_IIC)
    /* First I2C GPIO extender */  
    #if (I2C_BUS_NIM_EXT != I2C_BUS_NONE)
    case GPIO_DEVICE_ID_I2C_1:
      if(uBit < 8)
      {
        cRead = read_gpio_extender();
        cRead &= (u_int8)( 1 << uBit );
        *pbOn =  (((uGPIO & GPIO_POLARITY_MASK) == GPIO_POSITIVE_POLARITY) ? (cRead != 0) : (cRead == 0));
      }  
      else
        eReturn = CNXT_GPIO_INVALID_NUMBER;  
      break;
    #endif  
      
    /* Second I2C GPIO extender */  
    #if (I2C_BUS_GPIO2_EXT != I2C_BUS_NONE)
    case GPIO_DEVICE_ID_I2C_2:
      if(uBit < 8)
      {                                 
        cRead = read_second_gpio_extender();
        cRead &= (u_int8)( 1 << uBit );
        *pbOn =  (((uGPIO & GPIO_POLARITY_MASK) == GPIO_POSITIVE_POLARITY) ? (cRead != 0) : (cRead == 0));
      }  
      else
        eReturn = CNXT_GPIO_INVALID_NUMBER;  
      break;
    #endif  
    #endif
      
    #if (CNXT_PIO_EXP_TYPE == GPIO_EXTEND_IO)
    /* First ISA-connected GPIO extender - note this is NOT read-modify-write so we */
    /* must use a shadow copy to ensure that we update the extender correctly!      */
    #if (CNXT_PIO_EXP_REG != NOT_PRESENT)
    case GPIO_DEVICE_ID_ISA_1:
      ks = critical_section_begin();
      uValue = CNXT_PIO_EXP_REG_SHADOW & (1<<(uGPIO & GPIO_MASK_PIN_NUMBER));
      *pbOn =  (((uGPIO & GPIO_POLARITY_MASK) == GPIO_POSITIVE_POLARITY) ? (uValue != 0) : (uValue == 0));
      critical_section_end(ks);                      
      break;
    #endif  
      
    /* Second ISA-connected GPIO extender */
    #if (CNXT_PIO_EXP_REG_2 != NOT_PRESENT)
    case GPIO_DEVICE_ID_ISA_2:
      ks = critical_section_begin();
      uValue = CNXT_PIO_EXP_REG_2_SHADOW & (1<<(uGPIO & GPIO_MASK_PIN_NUMBER));
      *pbOn =  (((uGPIO & GPIO_POLARITY_MASK) == GPIO_POSITIVE_POLARITY) ? (uValue != 0) : (uValue == 0));
      critical_section_end(ks);                      
      break;
    #endif
    #endif
      
    /* An invalid device code was passed in the GPIO description */  
    default:
      eReturn = CNXT_GPIO_INVALID_DEVICE;
      break;
  }

  return(eReturn);  
}

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
CNXT_GPIO_STATUS cnxt_gpio_set_input(u_int32 uGPIO)
{
  u_int32 uDevice;
  u_int16 uBit;
  bool    ks;
  CNXT_GPIO_STATUS eReturn = CNXT_GPIO_STATUS_OK;
  
  /* Depending upon the device type, do the right thing */
  uDevice = uGPIO & GPIO_MASK_DEVICE_ID;
  uBit    = uGPIO & GPIO_MASK_PIN_NUMBER;
  
  switch (uDevice)
  {
    /* A normal, internal GPIO pin */
    case GPIO_DEVICE_ID_INTERNAL:
      ks = critical_section_begin();
      set_gpio_output_off(uBit);
      critical_section_end(ks);                      
      break;
    
    #if (CNXT_PIO_EXP_TYPE == GPIO_EXTEND_IIC)
    /* First I2C GPIO extender - cannot act as an input */  
    #if (I2C_BUS_NIM_EXT != I2C_BUS_NONE)
    case GPIO_DEVICE_ID_I2C_1:
      eReturn = CNXT_GPIO_INVALID_STATE;  
      break;
    #endif  
      
    /* Second I2C GPIO extender - cannot act as an input */  
    #if (I2C_BUS_GPIO2_EXT != I2C_BUS_NONE)
    case GPIO_DEVICE_ID_I2C_2:
      eReturn = CNXT_GPIO_INVALID_STATE;  
      break;
    #endif  
    #endif
      
    #if (CNXT_PIO_EXP_TYPE == GPIO_EXTEND_IO)
    /* First ISA-connected GPIO extender - cannot act as an input */
    #if (CNXT_PIO_EXP_REG != NOT_PRESENT)
    case GPIO_DEVICE_ID_ISA_1:
      eReturn = CNXT_GPIO_INVALID_STATE;  
      break;
    #endif  
      
    /* Second ISA-connected GPIO extender - cannot act as an input */
    #if (CNXT_PIO_EXP_REG_2 != NOT_PRESENT)
    case GPIO_DEVICE_ID_ISA_2:
      eReturn = CNXT_GPIO_INVALID_STATE;  
      break;
    #endif
    #endif
      
    /* An invalid device code was passed in the GPIO description */  
    default:
      eReturn = CNXT_GPIO_INVALID_DEVICE;
      break;
  }
  
  return(eReturn);
}

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
CNXT_GPIO_STATUS cnxt_gpio_set_int_edge(u_int32 uGPIO, u_int32 uEdge)
{
  int     iPolarity = -1;
  u_int32 uDevice;
  u_int16 uBit;
  bool    ks;
  CNXT_GPIO_STATUS eReturn = CNXT_GPIO_STATUS_OK;
  
  /* Depending upon the device type, do the right thing */
  uDevice = uGPIO & GPIO_MASK_DEVICE_ID;
  uBit    = uGPIO & GPIO_MASK_PIN_NUMBER;
  
  switch (uDevice)
  {
    /* A normal, internal GPIO pin */
    case GPIO_DEVICE_ID_INTERNAL:
      ks = critical_section_begin();
      
      switch(uEdge)
      {
        case POS_EDGE:
          iPolarity = ((uGPIO & GPIO_POLARITY_MASK) == GPIO_POSITIVE_POLARITY) ? POS_EDGE : NEG_EDGE;
          break;
           
        case NEG_EDGE:
          iPolarity = ((uGPIO & GPIO_POLARITY_MASK) == GPIO_POSITIVE_POLARITY) ? NEG_EDGE : POS_EDGE;
          break;
          
        case BOTH_EDGES:
          iPolarity = BOTH_EDGES;
          break;
          
        default:
          eReturn = CNXT_GPIO_INVALID_EDGE;
          break;
      }  
      
      if(eReturn == CNXT_GPIO_STATUS_OK)
        set_gpio_int_edge(uBit, iPolarity );
        
      critical_section_end(ks);                      
      break;
    
    #if (CNXT_PIO_EXP_TYPE == GPIO_EXTEND_IIC)
    /* First I2C GPIO extender - can't generate interrupts */  
    #if (I2C_BUS_NIM_EXT != I2C_BUS_NONE)
    case GPIO_DEVICE_ID_I2C_1:
      eReturn = CNXT_GPIO_INVALID_STATE;  
      break;
    #endif  
      
    /* Second I2C GPIO extender - can't generate interrupts */  
    #if (I2C_BUS_GPIO2_EXT != I2C_BUS_NONE)
    case GPIO_DEVICE_ID_I2C_2:
      eReturn = CNXT_GPIO_INVALID_STATE;  
      break;
    #endif  
    #endif
      
    #if (CNXT_PIO_EXP_TYPE == GPIO_EXTEND_IO)
    /* First ISA-connected GPIO extender -  - can't generate interrupts */
    #if (CNXT_PIO_EXP_REG != NOT_PRESENT)
    case GPIO_DEVICE_ID_ISA_1:
      eReturn = CNXT_GPIO_INVALID_STATE;  
      break;
    #endif  
      
    /* Second ISA-connected GPIO extender - can't generate interrupts */
    #if (CNXT_PIO_EXP_REG_2 != NOT_PRESENT)
    case GPIO_DEVICE_ID_ISA_2:
      eReturn = CNXT_GPIO_INVALID_STATE;  
      break;
    #endif
    #endif
      
    /* An invalid device code was passed in the GPIO description */  
    default:
      eReturn = CNXT_GPIO_INVALID_DEVICE;
      break;
  }
  
  return(eReturn);
}

#if (IMAGE_TYPE != BOOT && IMAGE_TYPE != BOOTEXT)
/********************************************************************/
/*  FUNCTION:    cnxt_gpio_int_register_isr                         */
/*                                                                  */
/*  PARAMETERS:  uGPIO    - General GPIO identifier for the pin     */
/*                          which is to be set as an input.         */
/*               uEdge    - POS_EDGE, NEG_EDGE or BOTH_EDGES        */
/*                                                                  */
/*  DESCRIPTION: This function is a "thin" wrapper function that    */
/*               will call the appropriate interrupt register       */
/*               function(s) for the GPIO                           */
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
CNXT_GPIO_STATUS cnxt_gpio_int_register_isr(
    u_int32 uGPIO,
    PFNISR  pfnHandler,
    bool    bFIQ,
    bool    bInvert,
    PFNISR *pfnChain)
{
  u_int32 uDevice;
  u_int16 uBit;
  CNXT_GPIO_STATUS eReturn = CNXT_GPIO_STATUS_OK;
  int   retCode;
  
  /* Depending upon the device type, do the right thing */
  uDevice = uGPIO & GPIO_MASK_DEVICE_ID;
  uBit    = uGPIO & GPIO_MASK_PIN_NUMBER;
  
  switch (uDevice)
  {
    /* A normal, internal GPIO pin */
    case GPIO_DEVICE_ID_INTERNAL:
      /* Register the ISR */
      retCode = int_register_isr( CALC_INT_ID(PIC_GPIO, uBit),
        pfnHandler,
        bFIQ,
        bInvert,
        pfnChain);

      /* Check the KAL/ISR return code and set the GPIO return code */
      if (RC_OK != retCode) {
         if (RC_KAL_BADPTR == retCode) {
            eReturn = CNXT_GPIO_KAL_BADPTR;
         }
         else if (RC_KAL_INVALID == retCode) {
            eReturn = CNXT_GPIO_KAL_INVALID;
         }
         else if (RC_KAL_INUSE == retCode) {
            eReturn = CNXT_GPIO_KAL_INUSE;
         }
         else {
            eReturn = CNXT_GPIO_KAL_UNKNOWN;
         }
      }

      break;
    
    #if (CNXT_PIO_EXP_TYPE == GPIO_EXTEND_IIC)
    /* First I2C GPIO extender - can't generate interrupts */  
    #if (I2C_BUS_NIM_EXT != I2C_BUS_NONE)
    case GPIO_DEVICE_ID_I2C_1:
      eReturn = CNXT_GPIO_INVALID_STATE;  
      break;
    #endif  
      
    /* Second I2C GPIO extender - can't generate interrupts */  
    #if (I2C_BUS_GPIO2_EXT != I2C_BUS_NONE)
    case GPIO_DEVICE_ID_I2C_2:
      eReturn = CNXT_GPIO_INVALID_STATE;  
      break;
    #endif  
    #endif
      
    #if (CNXT_PIO_EXP_TYPE == GPIO_EXTEND_IO)
    /* First ISA-connected GPIO extender -  - can't generate interrupts */
    #if (CNXT_PIO_EXP_REG != NOT_PRESENT)
    case GPIO_DEVICE_ID_ISA_1:
      eReturn = CNXT_GPIO_INVALID_STATE;  
      break;
    #endif  
      
    /* Second ISA-connected GPIO extender - can't generate interrupts */
    #if (CNXT_PIO_EXP_REG_2 != NOT_PRESENT)
    case GPIO_DEVICE_ID_ISA_2:
      eReturn = CNXT_GPIO_INVALID_STATE;  
      break;
    #endif
    #endif
      
    /* An invalid device code was passed in the GPIO description */  
    default:
      eReturn = CNXT_GPIO_INVALID_DEVICE;
      break;
  }
  
  return(eReturn);
}

/********************************************************************/
/*  FUNCTION:    cnxt_gpio_int_query_id                             */
/*                                                                  */
/*  PARAMETERS:  uGPIO    - General GPIO identifier for the pin     */
/*                          which is to be set as an input.         */
/*               pIntId   - Pointer to location to store the        */
/*                          calculated interrupt id                 */
/*                                                                  */
/*  DESCRIPTION: This function will calculate the interrupt id for  */
/*               a given General GPIO idendifier                    */
/*  RETURNS:     CNXT_GPIO_STATUS_OK,                               */
/*               CNXT_GPIO_INVALID_DEVICE,                          */
/*                                                                  */
/*  CONTEXT:     Interrupt safe if the GPIO passed is NOT connected */
/*               to an I2C GPIO extender on the target board.       */
/*                                                                  */
/********************************************************************/
CNXT_GPIO_STATUS cnxt_gpio_int_query_id(u_int32 uGPIO, u_int32 uIntId)
{
  u_int32 uDevice;
  u_int16 uBit;
  u_int32 uTmpIntId;
  CNXT_GPIO_STATUS eReturn = CNXT_GPIO_STATUS_OK;

  /* Depending upon the device type, do the right thing */
  uDevice = uGPIO & GPIO_MASK_DEVICE_ID;
  uBit    = uGPIO & GPIO_MASK_PIN_NUMBER;

  switch (uDevice)
  {
    /* A normal, internal GPIO pin */
    case GPIO_DEVICE_ID_INTERNAL:
      /* Calculate the interrupt ID */
      uTmpIntId = CALC_INT_ID(PIC_GPIO, uBit);
      if (uTmpIntId != uIntId) {
        eReturn = CNXT_GPIO_INVALID_NUMBER;
      }
      break;
    
    #if (CNXT_PIO_EXP_TYPE == GPIO_EXTEND_IIC)
    /* First I2C GPIO extender - can't generate interrupts */  
    #if (I2C_BUS_NIM_EXT != I2C_BUS_NONE)
    case GPIO_DEVICE_ID_I2C_1:
      eReturn = CNXT_GPIO_INVALID_STATE;  
      break;
    #endif  
      
    /* Second I2C GPIO extender - can't generate interrupts */  
    #if (I2C_BUS_GPIO2_EXT != I2C_BUS_NONE)
    case GPIO_DEVICE_ID_I2C_2:
      eReturn = CNXT_GPIO_INVALID_STATE;  
      break;
    #endif  
    #endif
      
    #if (CNXT_PIO_EXP_TYPE == GPIO_EXTEND_IO)
    /* First ISA-connected GPIO extender -  - can't generate interrupts */
    #if (CNXT_PIO_EXP_REG != NOT_PRESENT)
    case GPIO_DEVICE_ID_ISA_1:
      eReturn = CNXT_GPIO_INVALID_STATE;  
      break;
    #endif  
      
    /* Second ISA-connected GPIO extender - can't generate interrupts */
    #if (CNXT_PIO_EXP_REG_2 != NOT_PRESENT)
    case GPIO_DEVICE_ID_ISA_2:
      eReturn = CNXT_GPIO_INVALID_STATE;  
      break;
    #endif
    #endif
      
    /* An invalid device code was passed in the GPIO description */  
    default:
      eReturn = CNXT_GPIO_INVALID_DEVICE;
      break;
  }
  
  return(eReturn);
}

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
CNXT_GPIO_STATUS cnxt_gpio_int_disable(u_int32 uGPIO)
{
  u_int32 uDevice;
  u_int16 uBit;
  bool    ks;
  CNXT_GPIO_STATUS eReturn = CNXT_GPIO_STATUS_OK;
  int   retCode;
  
  /* Depending upon the device type, do the right thing */
  uDevice = uGPIO & GPIO_MASK_DEVICE_ID;
  uBit    = uGPIO & GPIO_MASK_PIN_NUMBER;
  
  switch (uDevice)
  {
    /* A normal, internal GPIO pin */
    case GPIO_DEVICE_ID_INTERNAL:
      ks = critical_section_begin();
      
      /* Register the ISR */
      retCode = int_disable( CALC_INT_ID(PIC_GPIO, uBit));

      /* Check the KAL/ISR return code and set the GPIO return code */
      if (RC_OK != retCode) {
         if (RC_KAL_INVALID == retCode) {
            eReturn = CNXT_GPIO_KAL_INVALID;
         }
         else if (RC_KAL_NOTHOOKED == retCode) {
            eReturn = CNXT_GPIO_KAL_NOTHOOKED;
         }
         else {
            eReturn = CNXT_GPIO_KAL_UNKNOWN;
         }
      }

      critical_section_end(ks);                      
      break;
    
    #if (CNXT_PIO_EXP_TYPE == GPIO_EXTEND_IIC)
    /* First I2C GPIO extender - can't generate interrupts */  
    #if (I2C_BUS_NIM_EXT != I2C_BUS_NONE)
    case GPIO_DEVICE_ID_I2C_1:
      eReturn = CNXT_GPIO_INVALID_STATE;  
      break;
    #endif  
      
    /* Second I2C GPIO extender - can't generate interrupts */  
    #if (I2C_BUS_GPIO2_EXT != I2C_BUS_NONE)
    case GPIO_DEVICE_ID_I2C_2:
      eReturn = CNXT_GPIO_INVALID_STATE;  
      break;
    #endif  
    #endif
      
    #if (CNXT_PIO_EXP_TYPE == GPIO_EXTEND_IO)
    /* First ISA-connected GPIO extender -  - can't generate interrupts */
    #if (CNXT_PIO_EXP_REG != NOT_PRESENT)
    case GPIO_DEVICE_ID_ISA_1:
      eReturn = CNXT_GPIO_INVALID_STATE;  
      break;
    #endif  
      
    /* Second ISA-connected GPIO extender - can't generate interrupts */
    #if (CNXT_PIO_EXP_REG_2 != NOT_PRESENT)
    case GPIO_DEVICE_ID_ISA_2:
      eReturn = CNXT_GPIO_INVALID_STATE;  
      break;
    #endif
    #endif
      
    /* An invalid device code was passed in the GPIO description */  
    default:
      eReturn = CNXT_GPIO_INVALID_DEVICE;
      break;
  }
  
  return(eReturn);
}

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
CNXT_GPIO_STATUS cnxt_gpio_int_enable(u_int32 uGPIO)
{
  u_int32 uDevice;
  u_int16 uBit;
  bool    ks;
  CNXT_GPIO_STATUS eReturn = CNXT_GPIO_STATUS_OK;
  int   retCode;
  
  /* Depending upon the device type, do the right thing */
  uDevice = uGPIO & GPIO_MASK_DEVICE_ID;
  uBit    = uGPIO & GPIO_MASK_PIN_NUMBER;
  
  switch (uDevice)
  {
    /* A normal, internal GPIO pin */
    case GPIO_DEVICE_ID_INTERNAL:
      ks = critical_section_begin();
      
      /* Register the ISR */
      retCode = int_enable( CALC_INT_ID(PIC_GPIO, uBit));

      /* Check the KAL/ISR return code and set the GPIO return code */
      if (RC_OK != retCode) {
         if (RC_KAL_INVALID == retCode) {
            eReturn = CNXT_GPIO_KAL_INVALID;
         }
         else if (RC_KAL_NOTHOOKED == retCode) {
            eReturn = CNXT_GPIO_KAL_NOTHOOKED;
         }
         else {
            eReturn = CNXT_GPIO_KAL_UNKNOWN;
         }
      }

      critical_section_end(ks);                      
      break;
    
    #if (CNXT_PIO_EXP_TYPE == GPIO_EXTEND_IIC)
    /* First I2C GPIO extender - can't generate interrupts */  
    #if (I2C_BUS_NIM_EXT != I2C_BUS_NONE)
    case GPIO_DEVICE_ID_I2C_1:
      eReturn = CNXT_GPIO_INVALID_STATE;  
      break;
    #endif  
      
    /* Second I2C GPIO extender - can't generate interrupts */  
    #if (I2C_BUS_GPIO2_EXT != I2C_BUS_NONE)
    case GPIO_DEVICE_ID_I2C_2:
      eReturn = CNXT_GPIO_INVALID_STATE;  
      break;
    #endif  
    #endif
      
    #if (CNXT_PIO_EXP_TYPE == GPIO_EXTEND_IO)
    /* First ISA-connected GPIO extender -  - can't generate interrupts */
    #if (CNXT_PIO_EXP_REG != NOT_PRESENT)
    case GPIO_DEVICE_ID_ISA_1:
      eReturn = CNXT_GPIO_INVALID_STATE;  
      break;
    #endif  
      
    /* Second ISA-connected GPIO extender - can't generate interrupts */
    #if (CNXT_PIO_EXP_REG_2 != NOT_PRESENT)
    case GPIO_DEVICE_ID_ISA_2:
      eReturn = CNXT_GPIO_INVALID_STATE;  
      break;
    #endif
    #endif
      
    /* An invalid device code was passed in the GPIO description */  
    default:
      eReturn = CNXT_GPIO_INVALID_DEVICE;
      break;
  }
  
  return(eReturn);
}

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
CNXT_GPIO_STATUS cnxt_gpio_clear_pic_interrupt(u_int32 uGPIO)
{
  u_int32 uDevice;
  u_int16 uBit;
  bool    ks;
  CNXT_GPIO_STATUS eReturn = CNXT_GPIO_STATUS_OK;
  
  /* Depending upon the device type, do the right thing */
  uDevice = uGPIO & GPIO_MASK_DEVICE_ID;
  uBit    = uGPIO & GPIO_MASK_PIN_NUMBER;
  
  switch (uDevice)
  {
    /* A normal, internal GPIO pin */
    case GPIO_DEVICE_ID_INTERNAL:
      ks = critical_section_begin();
      
      /* Register the ISR */
      clear_pic_interrupt(PIC_GPIO, uBit);

      critical_section_end(ks);                      
      break;
    
    #if (CNXT_PIO_EXP_TYPE == GPIO_EXTEND_IIC)
    /* First I2C GPIO extender - can't generate interrupts */  
    #if (I2C_BUS_NIM_EXT != I2C_BUS_NONE)
    case GPIO_DEVICE_ID_I2C_1:
      eReturn = CNXT_GPIO_INVALID_STATE;  
      break;
    #endif  
      
    /* Second I2C GPIO extender - can't generate interrupts */  
    #if (I2C_BUS_GPIO2_EXT != I2C_BUS_NONE)
    case GPIO_DEVICE_ID_I2C_2:
      eReturn = CNXT_GPIO_INVALID_STATE;  
      break;
    #endif  
    #endif
      
    #if (CNXT_PIO_EXP_TYPE == GPIO_EXTEND_IO)
    /* First ISA-connected GPIO extender -  - can't generate interrupts */
    #if (CNXT_PIO_EXP_REG != NOT_PRESENT)
    case GPIO_DEVICE_ID_ISA_1:
      eReturn = CNXT_GPIO_INVALID_STATE;  
      break;
    #endif  
      
    /* Second ISA-connected GPIO extender - can't generate interrupts */
    #if (CNXT_PIO_EXP_REG_2 != NOT_PRESENT)
    case GPIO_DEVICE_ID_ISA_2:
      eReturn = CNXT_GPIO_INVALID_STATE;  
      break;
    #endif
    #endif
      
    /* An invalid device code was passed in the GPIO description */  
    default:
      eReturn = CNXT_GPIO_INVALID_DEVICE;
      break;
  }
  
  return(eReturn);
}

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
CNXT_GPIO_STATUS cnxt_gpio_int_query_enabled(u_int32 uGPIO, bool *bEnabled)
{
  u_int32 uDevice;
  u_int16 uBit;
  bool    ks;
  CNXT_GPIO_STATUS eReturn = CNXT_GPIO_STATUS_OK;
  
  /* Depending upon the device type, do the right thing */
  uDevice = uGPIO & GPIO_MASK_DEVICE_ID;
  uBit    = uGPIO & GPIO_MASK_PIN_NUMBER;
  *bEnabled = FALSE;
  
  switch (uDevice)
  {
    /* A normal, internal GPIO pin */
    case GPIO_DEVICE_ID_INTERNAL:
      ks = critical_section_begin();
      
      /* Query the ISR */
      *bEnabled = is_int_enabled( PIC_GPIO, uBit );

      critical_section_end(ks);                      
      break;
    
    #if (CNXT_PIO_EXP_TYPE == GPIO_EXTEND_IIC)
    /* First I2C GPIO extender - can't generate interrupts */  
    #if (I2C_BUS_NIM_EXT != I2C_BUS_NONE)
    case GPIO_DEVICE_ID_I2C_1:
      eReturn = CNXT_GPIO_INVALID_STATE;  
      break;
    #endif  
      
    /* Second I2C GPIO extender - can't generate interrupts */  
    #if (I2C_BUS_GPIO2_EXT != I2C_BUS_NONE)
    case GPIO_DEVICE_ID_I2C_2:
      eReturn = CNXT_GPIO_INVALID_STATE;  
      break;
    #endif  
    #endif
      
    #if (CNXT_PIO_EXP_TYPE == GPIO_EXTEND_IO)
    /* First ISA-connected GPIO extender -  - can't generate interrupts */
    #if (CNXT_PIO_EXP_REG != NOT_PRESENT)
    case GPIO_DEVICE_ID_ISA_1:
      eReturn = CNXT_GPIO_INVALID_STATE;  
      break;
    #endif  
      
    /* Second ISA-connected GPIO extender - can't generate interrupts */
    #if (CNXT_PIO_EXP_REG_2 != NOT_PRESENT)
    case GPIO_DEVICE_ID_ISA_2:
      eReturn = CNXT_GPIO_INVALID_STATE;  
      break;
    #endif
    #endif
      
    /* An invalid device code was passed in the GPIO description */  
    default:
      eReturn = CNXT_GPIO_INVALID_DEVICE;
      break;
  }
  
  return(eReturn);
}

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
CNXT_GPIO_STATUS cnxt_gpio_int_query_pending(u_int32 uGPIO, bool *bPending)
{
  u_int32 uDevice;
  u_int16 uBit;
  bool    ks;
  CNXT_GPIO_STATUS eReturn = CNXT_GPIO_STATUS_OK;
  
  /* Depending upon the device type, do the right thing */
  uDevice = uGPIO & GPIO_MASK_DEVICE_ID;
  uBit    = uGPIO & GPIO_MASK_PIN_NUMBER;
  *bPending = FALSE;
  
  switch (uDevice)
  {
    /* A normal, internal GPIO pin */
    case GPIO_DEVICE_ID_INTERNAL:
      ks = critical_section_begin();
      
      /* Query the ISR */
      *bPending = is_raw_int_active( PIC_GPIO, uBit );

      critical_section_end(ks);                      
      break;
    
    #if (CNXT_PIO_EXP_TYPE == GPIO_EXTEND_IIC)
    /* First I2C GPIO extender - can't generate interrupts */  
    #if (I2C_BUS_NIM_EXT != I2C_BUS_NONE)
    case GPIO_DEVICE_ID_I2C_1:
      eReturn = CNXT_GPIO_INVALID_STATE;  
      break;
    #endif  
      
    /* Second I2C GPIO extender - can't generate interrupts */  
    #if (I2C_BUS_GPIO2_EXT != I2C_BUS_NONE)
    case GPIO_DEVICE_ID_I2C_2:
      eReturn = CNXT_GPIO_INVALID_STATE;  
      break;
    #endif  
    #endif
      
    #if (CNXT_PIO_EXP_TYPE == GPIO_EXTEND_IO)
    /* First ISA-connected GPIO extender -  - can't generate interrupts */
    #if (CNXT_PIO_EXP_REG != NOT_PRESENT)
    case GPIO_DEVICE_ID_ISA_1:
      eReturn = CNXT_GPIO_INVALID_STATE;  
      break;
    #endif  
      
    /* Second ISA-connected GPIO extender - can't generate interrupts */
    #if (CNXT_PIO_EXP_REG_2 != NOT_PRESENT)
    case GPIO_DEVICE_ID_ISA_2:
      eReturn = CNXT_GPIO_INVALID_STATE;  
      break;
    #endif
    #endif
      
    /* An invalid device code was passed in the GPIO description */  
    default:
      eReturn = CNXT_GPIO_INVALID_DEVICE;
      break;
  }
  
  return(eReturn);
}
#endif /* (IMAGE_TYPE != BOOT && IMAGE_TYPE != BOOTEXT) */

/************************/
/************************/
/** INTERNAL FUNCTIONS **/
/************************/
/************************/

/***************************************************************/
/* Set a given internal GPIO pin output high or low.           */
/*                                                             */
/* Note that this function is for internal GPIOs ONLY and does */
/* not handle any GPIO on an external expander. This is by     */
/* design while we transition to the use of general purpose    */
/* GPIO pin descriptions in higher layers of code.             */
/***************************************************************/
void set_gpio_output_level(unsigned int uGpioNum, bool bHigh)
{
   unsigned long Bank, BitPos;

   /* Convert from a linear, internal GPIO number to a bank */
   /* and bit position.                                     */
   split_gpio_int(uGpioNum, &Bank, &BitPos);

   if(bHigh)
   {
     DRIVE_GPIO_HIGH_BANK(Bank, BitPos);
   }
   else
   {
     DRIVE_GPIO_LOW_BANK(Bank, BitPos);
   }
}

/*************************************************************************************/
/* GPIO_bit is a number from 0 to 95 (with extender, it can go higher)
                   */
/*************************************************************************************/
#if (CNXT_PIO_EXP_TYPE == GPIO_EXTEND_IO)
#if (CNXT_PIO_EXP_REG != NOT_PRESENT)
extern HW_DWORD CNXT_PIO_EXP_REG_SHADOW;
#endif
#if (CNXT_PIO_EXP_REG_2 != NOT_PRESENT)
extern HW_DWORD CNXT_PIO_EXP_REG_2_SHADOW;
#endif
#endif
void set_gpio_output_low(unsigned int  GPIO_bit)
{
   unsigned long Bank, BitPos;

   if ( GPIO_bit < NUM_OF_GPIO )
   {
      /* It is on internal GPIO */
      /* find which bit in which of 3 registers */
      split_gpio_int(GPIO_bit, &Bank, &BitPos);
      DRIVE_GPIO_LOW_BANK(Bank, BitPos);
   }
   else
   {
      /* it is GPIO extender */
      GPIO_bit -= NUM_OF_GPIO;
#if (CNXT_PIO_EXP_TYPE == GPIO_EXTEND_IO)
      CNXT_PIO_EXP_REG_SHADOW &= ~(u_int16)( 1 << GPIO_bit );
      *((volatile u_int16 *)CNXT_PIO_EXP_REG) = CNXT_PIO_EXP_REG_SHADOW;
#else
      if ( GPIO_bit < 8 )
      {
         write_gpio_extender ( (u_int8)( 1 << GPIO_bit ), 0 );
      }
      else
      {
         GPIO_bit -= 8;
         write_second_gpio_extender ( (u_int8)( 1 << GPIO_bit ), 0 );
      }
#endif
   }
}

/*************************************************************************************/
/* GPIO_bit is a number from 0 to 95 (with extender, it can go higher)
                   */
/*************************************************************************************/
void set_gpio_output_high(unsigned int  GPIO_bit)
{
   unsigned long Bank, BitPos;

   if ( GPIO_bit < NUM_OF_GPIO )
   {
      /* find which bit in which of 3 registers */
      split_gpio_int(GPIO_bit, &Bank, &BitPos);
      DRIVE_GPIO_HIGH_BANK(Bank, BitPos);
   }
   else
   {
      /* it is on GPIO extender */
      GPIO_bit -= NUM_OF_GPIO;
#if (CNXT_PIO_EXP_TYPE == GPIO_EXTEND_IO)
      CNXT_PIO_EXP_REG_SHADOW |= (u_int16)( 1 << GPIO_bit );
      *((volatile u_int16 *)CNXT_PIO_EXP_REG) = CNXT_PIO_EXP_REG_SHADOW;
#else
      if ( GPIO_bit < 8 )
      {
         write_gpio_extender ( (u_int8)( 1 << GPIO_bit ), (u_int8)( 1 << GPIO_bit ) );
      }
      else
      {
         GPIO_bit -= 8;
         write_second_gpio_extender ( (u_int8)( 1 << GPIO_bit ), (u_int8)( 1 << GPIO_bit ) );
      }
#endif
   }
}

/*************************************************************************************/
/* GPIO_bit is a number from 0 to 95                                                 */
/*************************************************************************************/
int get_gpio_input(unsigned int  GPIO_bit)
{
  unsigned long Bank, BitPos;

  /* find which bit in which of 3 registers */
  split_gpio_int(GPIO_bit, &Bank, &BitPos);
  return GET_GPIO_PIN_BANK(Bank, BitPos);
}

/********************************************************************/
/*  FUNCTION:    set_gpio_output_off                                */
/*                                                                  */
/*  PARAMETERS:  uGpioNum - Linear GPIO number to make an input     */
/*                                                                  */
/*  DESCRIPTION: Set a given GPIO as an input. This function        */
/*               maps a linear GPIO number to the relevant bank     */
/*               and pin then writes the correct register.          */
/*                                                                  */
/*  RETURNS:     Nothing                                            */
/********************************************************************/
void set_gpio_output_off(unsigned int GPIO_bit)
{
  unsigned long Bank, BitPos;

  split_gpio_int(GPIO_bit, &Bank, &BitPos);
  MAKE_GPIO_INPUT_BANK(Bank, BitPos);
}

/*************************************************************************************/
/* GPIO_bit is a number from 0 to 95,  polarity is POS_EDGE, NEG_EDGE, or BOTH_EDGES */
/*************************************************************************************/
void set_gpio_int_edge(unsigned int  GPIO_bit, int  polarity)
{
unsigned long uBank, uBitPos, register_base;
LPREG      lpGpiPolarity;

    // find which bit in which of 3 registers
    split_gpio_int(GPIO_bit, &uBank, &uBitPos);

    switch (polarity)
    {
       case POS_EDGE:
          register_base = GPI_POS_EDGE_REG;
          break;
       case NEG_EDGE:
          register_base = GPI_NEG_EDGE_REG;
          break;
       case BOTH_EDGES:
          set_gpio_int_edge(GPIO_bit, POS_EDGE);
          set_gpio_int_edge(GPIO_bit, NEG_EDGE);
          return;
       default:
#if RTOS != NOOS
          error_log(ERROR_WARNING | RC_GEN_BAD_GPIO_POLARITY);
#endif
          return;
    }
    /* Calculate correct register address */
    lpGpiPolarity = (LPREG)(register_base + (uBank * GPI_BANK_SIZE));

    *lpGpiPolarity |= (1 << uBitPos);
}

/*************************************************************************/
/* For Neches, given a particular GPIO interrupt number, return the bank */
/* number and bit number corresponding to the interrupt ID.              */
/* Needs to be critical_section safe!                                    */
/*************************************************************************/
u_int32 split_gpio_int(u_int32 iInt, u_int32 *uBank, u_int32 *uBitNum)
{
   *uBitNum = iInt & 31;
   *uBank   = iInt >> 5;

   return(TRUE);
}

/****************************************************************************
 * Modifications:
 * $Log: 
 *  8    mpeg      1.7         7/10/03 2:18:40 PM     Larry Wang      SCR(s) 
 *        6924 :
 *        Restore to Rev 1.5
 *        
 *  7    mpeg      1.6         7/10/03 10:42:16 AM    Larry Wang      SCR(s) 
 *        6924 :
 *        make CODELDR build also includes the case where RTOS==VXWORKS && 
 *        APPNAME==CODELDR.
 *        
 *        
 *  6    mpeg      1.5         7/9/03 3:27:28 PM      Tim White       SCR(s) 
 *        6901 :
 *        Phase 3 codeldrext drop.
 *        
 *        
 *  5    mpeg      1.4         6/24/03 2:06:54 PM     Miles Bintz     SCR(s) 
 *        6822 :
 *        added initialization value to remove warning in release build
 *        
 *  4    mpeg      1.3         3/18/03 4:15:34 PM     Bobby Bradford  SCR(s) 
 *        5108 :
 *        Added two new functions to query the pending/enabled status
 *        of interrupts based on the generic defintions of GPIO pins
 *        
 *  3    mpeg      1.2         1/29/03 9:58:30 AM     Bobby Bradford  SCR(s) 
 *        5103 :
 *        When enabling the front-panel buttons on Bronco/Watchtv, there were
 *        some trace errors (due to calling trace_new() function inside a
 *        critical section).  This was ultimately from the GPIO changes that
 *        had been recently made.  Got rid of the critical section in this file
 *        that was not really needed.
 *        
 *  2    mpeg      1.1         1/23/03 1:04:24 PM     Bobby Bradford  SCR(s) 
 *        5275 :
 *        Added new functions for interrupt processing using the General
 *        GPIO definitions
 *        
 *  1    mpeg      1.0         1/17/03 4:37:06 PM     Dave Wilson     
 * $
 * 
 *    Rev 1.7   10 Jul 2003 13:18:40   wangl2
 * SCR(s) 6924 :
 * Restore to Rev 1.5
 * 
 *    Rev 1.5   09 Jul 2003 14:27:28   whiteth
 * SCR(s) 6901 :
 * Phase 3 codeldrext drop.
 * 
 * 
 *    Rev 1.4   24 Jun 2003 13:06:54   bintzmf
 * SCR(s) 6822 :
 * added initialization value to remove warning in release build
 * 
 *    Rev 1.3   18 Mar 2003 16:15:34   bradforw
 * SCR(s) 5108 :
 * Added two new functions to query the pending/enabled status
 * of interrupts based on the generic defintions of GPIO pins
 * 
 *    Rev 1.2   29 Jan 2003 09:58:30   bradforw
 * SCR(s) 5103 :
 * When enabling the front-panel buttons on Bronco/Watchtv, there were
 * some trace errors (due to calling trace_new() function inside a
 * critical section).  This was ultimately from the GPIO changes that
 * had been recently made.  Got rid of the critical section in this file
 * that was not really needed.
 * 
 *    Rev 1.1   23 Jan 2003 13:04:24   bradforw
 * SCR(s) 5275 :
 * Added new functions for interrupt processing using the General
 * GPIO definitions
 * 
 *    Rev 1.0   17 Jan 2003 16:37:06   dawilson
 * SCR(s) 5264 :
 * Generic (virtual) GPIO access functions.
 *
 ****************************************************************************/
 

