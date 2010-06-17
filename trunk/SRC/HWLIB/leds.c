/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*           Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002         */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       LEDS.C
 *
 * Description:    Low Level LED Access Functions
 *
 *
 * Author:         Dave Wilson
 *
 ****************************************************************************/
/* $Header: leds.c, 10, 3/19/04 2:27:18 PM, Craig Dry$
 ****************************************************************************/


/*****************/
/* Include Files */
/*****************/
#include "stbcfg.h"
#include "basetype.h"
#include "leds.h"
#include "iic.h"
#include "trace.h"
#include "hwfuncs.h"
#include "hwlib.h"
#include "gpio.h"

/*
 * !!! HACK ALERT !!!
 * WARNING!!!!! Hack requested for supporting Bronco1 & Bronco3 at runtime.  When
 * Bronco1 boards disappear, so should this hack.
 * !!! HACK ALERT !!!
 */
#if ((FRONT_PANEL_KEYPAD_TYPE == FRONT_PANEL_KEYPAD_BRONCO) && \
     (I2C_CONFIG_EEPROM_ADDR != NOT_PRESENT))
extern int ConfigurationValid;
extern CONFIG_TABLE  config_table;
#endif
/*
 * !!! HACK ALERT !!!
 * !!! HACK ALERT !!!
 */

/******************************/
/* Global Types and Variables */
/******************************/
typedef struct _CNXT_FP_LED
{
  u_int32      uGPIO;
  FP_LED_STATE eState;
}CNXT_FP_LED;

/******************************************/
/* State information for the defined LEDs */
/******************************************/

/**********************************************************************/
/* The values PIO_LED_x are defined in the relevant hardware config   */
/* file. Each is an alias to a "real" LED definition, for example     */
/* PIO_LED_POWER. This allows us to build a LED table here without    */
/* having to know the real names of each LED in the IRD. Applications */
/* should use the LED's descriptive name rather than the simple       */
/* PIO_LED_x labels.                                                  */
/**********************************************************************/
CNXT_FP_LED sLEDInfo[NUMBER_OF_LEDS] 
#if (defined(PIO_LED_0) || defined(PIO_LED_1) || defined(PIO_LED_2) || \
     defined(PIO_LED_3) || defined(PIO_LED_4) || defined(PIO_LED_5) || \
     defined(PIO_LED_6) || defined(PIO_LED_7) || defined(PIO_LED_8) || \
     defined(PIO_LED_9))
= {
  #ifdef PIO_LED_0
  { PIO_LED_0, CNXT_LED_DARK},
  #endif
  #ifdef PIO_LED_1
  { PIO_LED_1, CNXT_LED_DARK},
  #endif
  #ifdef PIO_LED_2
  { PIO_LED_2, CNXT_LED_DARK},
  #endif
  #ifdef PIO_LED_3
  { PIO_LED_3, CNXT_LED_DARK},
  #endif
  #ifdef PIO_LED_4
  { PIO_LED_4, CNXT_LED_DARK},
  #endif
  #ifdef PIO_LED_5
  { PIO_LED_5, CNXT_LED_DARK},
  #endif
  #ifdef PIO_LED_6
  { PIO_LED_6, CNXT_LED_DARK},
  #endif
  #ifdef PIO_LED_7
  { PIO_LED_7, CNXT_LED_DARK},
  #endif
  #ifdef PIO_LED_8
  { PIO_LED_8, CNXT_LED_DARK},
  #endif
  #ifdef PIO_LED_9
  { PIO_LED_9, CNXT_LED_DARK},
  #endif
}
#endif
;

/********************************/
/* Internal Function Prototypes */
/********************************/
static int cnxt_led_get_state_index(u_int32 uLED);

/**********************/
/* Exported Functions */
/**********************/


/********************************************************************/
/*  FUNCTION:    cnxt_led_initialize                                */
/*                                                                  */
/*  PARAMETERS:  bRTOSUp  - General GPIO identifier for the LED     */
/*                          whose state is being queried.           */
/*                                                                  */
/*  DESCRIPTION: This function initialises all the LEDs to their    */
/*               default state defined in sLEDInfo. If bRTOSUp is   */
/*               false, only GPIO- and ISA-attached LEDs are        */
/*               touched (since the I2C driver will not be usable   */
/*               at this point).                                    */
/*                                                                  */
/*  RETURNS:     FP_LED_STATUS_OK,                                  */
/*               FP_LED_ERROR_BAD_GPIO,                             */
/*               FP_LED_ERROR_UNDEFINED_LED                         */
/*                                                                  */
/*  CONTEXT:     Interrupt safe                                     */
/*                                                                  */
/********************************************************************/
FP_LED_STATUS cnxt_led_initialize(bool bRTOSUp)
{
  int           iLoop;
  FP_LED_STATUS eReturn = FP_LED_STATUS_OK;
  
  for(iLoop = 0; iLoop < NUMBER_OF_LEDS; iLoop++)
  {
    /* If the RTOS is up or the RTOS is not up but the LED is not on I2C */
    /* go ahead and set the LED state.                                   */
    if(bRTOSUp || (!bRTOSUp && !(sLEDInfo[iLoop].uGPIO & GPIO_DEVICE_ID_I2C_MASK)))
    {
/*
 * !!! HACK ALERT !!!
 * WARNING!!!!! Hack requested for supporting Bronco1 & Bronco3 at runtime.  When
 * Bronco1 boards disappear, so should this hack.
 * !!! HACK ALERT !!!
 */
#if ((FRONT_PANEL_KEYPAD_TYPE == FRONT_PANEL_KEYPAD_BRONCO) && \
     (I2C_CONFIG_EEPROM_ADDR != NOT_PRESENT))
    if(!ConfigurationValid ||
         ((config_table.board_type == 0x00) && (config_table.board_rev == 0x01)))
    {
       switch(sLEDInfo[iLoop].uGPIO)
       {
           case PIO_LED_IR_MSG:
              sLEDInfo[iLoop].uGPIO = (6+GPIO_DEVICE_ID_ISA_1+GPIO_PIN_IS_OUTPUT+GPIO_NEGATIVE_POLARITY);
              break;

           case PIO_LED_ONLINE_MSG:
              sLEDInfo[iLoop].uGPIO = (24+GPIO_DEVICE_ID_INTERNAL+GPIO_NEGATIVE_POLARITY+GPIO_PIN_IS_OUTPUT+GPIO_DEVICE_FLAG_PWM);
              break;
      }
    }
#endif
/*
 * !!! HACK ALERT !!!
 * !!! HACK ALERT !!!
 */

      eReturn = cnxt_led_set(sLEDInfo[iLoop].uGPIO, CNXT_LED_DARK);
      
      /* If we couldn't set the LED state, bomb out early */
      if(eReturn != FP_LED_STATUS_OK)
        break;
    }
  }
  return(eReturn);  
}

/********************************************************************/
/*  FUNCTION:    cnxt_led_get                                       */
/*                                                                  */
/*  PARAMETERS:  uLedGpio - General GPIO identifier for the LED     */
/*                          whose state is being queried.           */
/*               peState  - Pointer for returned state.             */
/*                                                                  */
/*  DESCRIPTION: This function returns the current state of a LED.  */
/*                                                                  */
/*  RETURNS:     FP_LED_STATUS_OK,                                  */
/*               FP_LED_ERROR_UNDEFINED_LED                         */
/*               FP_LED_BAD_PTR                                     */
/*                                                                  */
/*  CONTEXT:     Interrupt safe                                     */
/*                                                                  */
/********************************************************************/
FP_LED_STATUS cnxt_led_get(u_int32 uLedGpio, FP_LED_STATE *peState) 
{
  FP_LED_STATUS eReturn;
  int           iIndex;
  
  /* Which LED in our table are we looking at? */
  iIndex = cnxt_led_get_state_index(uLedGpio);
  if(iIndex == -1)
  {
    /* We were passed a LED that we don't know about! */
    eReturn = FP_LED_ERROR_UNDEFINED_LED;
  }
  else
  {
    if(peState)
    {
      /* Return the current state from the table */
      eReturn  = FP_LED_STATUS_OK;
      *peState = sLEDInfo[iIndex].eState;
    }
    else
    {
      /* Oops - someone gave us a NULL pointer */
      eReturn = FP_LED_BAD_PTR;
    }
  }
  
  return(eReturn);  
}

/********************************************************************/
/*  FUNCTION:    cnxt_led_set                                       */
/*                                                                  */
/*  PARAMETERS:  uLedGpio - General GPIO identifier for the LED     */
/*                          which is to be turned on or off.        */
/*               eState   - ON to light the LED, OFF to turn it off */
/*                                                                  */
/*  DESCRIPTION: This function turns a front panel LED on or off.   */
/*               The GPIO ID passed to the function must conform to */
/*               the general GPIO definition as used in the hardware*/
/*               configuration file.                                */
/*                                                                  */
/*  RETURNS:     FP_LED_STATUS_OK,                                  */
/*               FP_LED_ERROR_BAD_GPIO,                             */
/*               FP_LED_ERROR_UNDEFINED_LED                         */
/*                                                                  */
/*  CONTEXT:     Interrupt safe if the LED passed is NOT connected  */
/*               to an I2C GPIO extender on the target board.       */
/*                                                                  */
/********************************************************************/
FP_LED_STATUS cnxt_led_set(u_int32 uLedGpio, FP_LED_STATE eState)
{
  FP_LED_STATUS    eReturn = FP_LED_STATUS_OK;
  CNXT_GPIO_STATUS eGPIOStatus;
  int              iIndex;
  
  /* Which LED in our table are we mucking with? */
  iIndex = cnxt_led_get_state_index(uLedGpio);
  if(iIndex == -1)
  {
    /* We were passed a LED that we don't know about! */
    eReturn = FP_LED_ERROR_UNDEFINED_LED;
  }
  else
  {  
    switch(eState)
    {
      case CNXT_LED_LIT:
#if PLL_PIN_GPIO_MUX0_REG_DEFAULT == NOT_DETERMINED
        /* If the LED is on the PWM pin, ensure that the GPIO mux is set */
        /* correctly for basic GPIO operation.                           */
        if(uLedGpio & GPIO_DEVICE_FLAG_PWM)
        {
          *((LPREG)PLL_PIN_GPIO_MUX0_REG) &= ~PLL_PIN_GPIO_MUX0_PWM;
        }
#endif /* PLL_PIN_GPIO_MUX0_REG_DEFAULT == NOT_DETERMINED */
        eGPIOStatus = cnxt_gpio_set_output_level(uLedGpio, TRUE);
        if(eGPIOStatus != CNXT_GPIO_STATUS_OK)
          eReturn = FP_LED_ERROR_BAD_GPIO;
        break;
      
      case CNXT_LED_DARK:
#if PLL_PIN_GPIO_MUX0_REG_DEFAULT == NOT_DETERMINED
        /* If the LED is on the PWM pin, ensure that the GPIO mux is set */
        /* correctly for basic GPIO operation.                           */
        if(uLedGpio & GPIO_DEVICE_FLAG_PWM)
        {
          *((LPREG)PLL_PIN_GPIO_MUX0_REG) &= ~PLL_PIN_GPIO_MUX0_PWM;
        }
#endif /* PLL_PIN_GPIO_MUX0_REG_DEFAULT == NOT_DETERMINED */
        eGPIOStatus = cnxt_gpio_set_output_level(uLedGpio, FALSE);
        if(eGPIOStatus != CNXT_GPIO_STATUS_OK)
          eReturn = FP_LED_ERROR_BAD_GPIO;
        break;
      
      default:
        eReturn = FP_LED_ERROR_INVALID_STATE;
        break;
    }    
    
    /* Keep track of the current state of the LED */
    if(eReturn == FP_LED_STATUS_OK)
      sLEDInfo[iIndex].eState = eState;      
  }  
      
  return(eReturn);
}

/********************************************************************/
/*  FUNCTION:    cnxt_led_get_state_index                           */
/*                                                                  */
/*  PARAMETERS:  uLED - GPIO identifier for LED whose index is      */
/*                      being queried.                              */
/*                                                                  */
/*  DESCRIPTION: Return the index of the LED whose GPIO has been    */
/*               passed to the function.                            */
/*                                                                  */
/*  RETURNS:    Index in the range [0, NUMBER_OF_LEDS) or -1 if the */
/*              LED is not found.                                   */
/*                                                                  */
/*  CONTEXT:    May be called in any context.                       */
/********************************************************************/
static int cnxt_led_get_state_index(u_int32 uLED)
{
  int iLoop;
  
  for(iLoop = 0; iLoop < NUMBER_OF_LEDS; iLoop++)
  {
    if(uLED == sLEDInfo[iLoop].uGPIO)
      return(iLoop);
  }
  return(-1);
}
    

/****************************************************************************
 * Modifications:
 * $Log: 
 *  10   mpeg      1.9         3/19/04 2:27:18 PM     Craig Dry       CR(s) 
 *        8599 : Fix compiler warnings for Download.
 *  9    mpeg      1.8         7/9/03 3:27:34 PM      Tim White       SCR(s) 
 *        6901 :
 *        Phase 3 codeldrext drop.
 *        
 *        
 *  8    mpeg      1.7         6/24/03 2:06:52 PM     Miles Bintz     SCR(s) 
 *        6822 :
 *        added initialization value to remove warning in release build
 *        
 *  7    mpeg      1.6         5/2/03 5:42:16 PM      Craig Dry       SCR(s) 
 *        5521 :
 *        Conditionally remove GPIO Pin Mux register accesses
 *        
 *  6    mpeg      1.5         4/2/03 3:57:38 PM      Tim White       SCR(s) 
 *        5943 :
 *        Use the bronco.cfg definitions instead of hard-coding the switch.
 *        
 *        
 *  5    mpeg      1.4         4/1/03 2:05:46 PM      Tim White       SCR(s) 
 *        5925 :
 *        Added runtime support for both Bronco1 & Bronco3 boards with one 
 *        hwconfig file.  ALL CHANGES MADE WITH THIS DEFECT ARE HACKS.  Please 
 *        remove when support for Bronco1 is no longer required.
 *        
 *        
 *  4    mpeg      1.3         1/21/03 8:20:06 PM     Dave Wilson     SCR(s) 
 *        5099 :
 *        Fixed an uninitialised return code from cnxt_led_set.
 *        
 *  3    mpeg      1.2         1/17/03 4:36:46 PM     Dave Wilson     SCR(s) 
 *        5264 :
 *        Moved GPIO access function to the new GPIO source file.
 *        
 *  2    mpeg      1.1         1/17/03 2:05:48 PM     Bobby Bradford  SCR(s) 
 *        5264 :
 *        Added SHADOW register(s) for extended GPIO to support BRONCO
 *        IRD (which does not support read-back of extended GPIO register)
 *        
 *  1    mpeg      1.0         12/16/02 2:00:52 PM    Dave Wilson     
 * $
 * 
 *    Rev 1.8   09 Jul 2003 14:27:34   whiteth
 * SCR(s) 6901 :
 * Phase 3 codeldrext drop.
 * 
 * 
 *    Rev 1.7   24 Jun 2003 13:06:52   bintzmf
 * SCR(s) 6822 :
 * added initialization value to remove warning in release build
 * 
 *    Rev 1.6   02 May 2003 16:42:16   dryd
 * SCR(s) 5521 :
 * Conditionally remove GPIO Pin Mux register accesses
 * 
 *    Rev 1.5   02 Apr 2003 15:57:38   whiteth
 * SCR(s) 5943 :
 * Use the bronco.cfg definitions instead of hard-coding the switch.
 * 
 * 
 *    Rev 1.4   01 Apr 2003 14:05:46   whiteth
 * SCR(s) 5925 :
 * Added runtime support for both Bronco1 & Bronco3 boards with one hwconfig file.  ALL CHANGES MADE WITH THIS DEFECT ARE HACKS.  Please remove when support for Bronco1 is no longer required.
 * 
 * 
 *    Rev 1.3   21 Jan 2003 20:20:06   dawilson
 * SCR(s) 5099 :
 * Fixed an uninitialised return code from cnxt_led_set.
 * 
 *    Rev 1.2   17 Jan 2003 16:36:46   dawilson
 * SCR(s) 5264 :
 * Moved GPIO access function to the new GPIO source file.
 * 
 *    Rev 1.1   17 Jan 2003 14:05:48   bradforw
 * SCR(s) 5264 :
 * Added SHADOW register(s) for extended GPIO to support BRONCO
 * IRD (which does not support read-back of extended GPIO register)
 * 
 *    Rev 1.0   16 Dec 2002 14:00:52   dawilson
 * SCR(s) 5177 :
 * Functions to access front panel LEDs using virtualised GPIO descriptors.
 *
 ****************************************************************************/

