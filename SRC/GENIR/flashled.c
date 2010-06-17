/****************************************************************************/
/*                          Conexant Systems Inc.                           */
/****************************************************************************/
/*                                                                          */
/* Filename:       flashled.c                                               */
/*                                                                          */
/* Description:    Manage the IRD's message received LED                    */
/*                                                                          */
/* Author:         Dave Wilson                                              */
/*                                                                          */
/* Copyright Conexant Systems Inc, 2000-2004                                */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/
/* $Id: flashled.c,v 1.9, 2004-03-04 06:17:52Z, Matt Korte$
 *****************************************************************************/

/*****************/
/* Include Files */
/*****************/
#include "warnfix.h"
#include "hwconfig.h"
#include "kal.h"
#include "retcodes.h"
#include "trace.h"
#include "leds.h"

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

/******************************************/
/* Local Definitions and Global Variables */
/******************************************/
#define IR_LED_FLASH_TIME 125

bool      bIRLedInitialised = FALSE;
tick_id_t hIRLedTick;

/********************************/
/* Internal Function Prototypes */
/********************************/
bool InitialiseIRLED(void);
void IRLedTickCallback(tick_id_t hTick, void *pUser);
void ir_set_led_state(u_int8 state);

/**********************/
/* Exported Functions */
/**********************/

/********************************************************************/
/*  FUNCTION:    FlashIRLEDEnable                                   */
/*                                                                  */
/*  PARAMETERS:  TRUE      Enable flashing the IR LED               */
/*               FALSE     Disable flashing the IR LED              */
/*                                                                  */
/*  DESCRIPTION: Enable/Disable the flashing of the IR LED          */
/*                                                                  */
/*  RETURNS:     Nothing                                            */
/********************************************************************/
static bool bFlashIRLEDEnabled = TRUE;
void FlashIRLEDEnable(bool bState)
{
   bFlashIRLEDEnabled = bState;
} /* end routine FlashIRLEDEnable() */

/********************************************************************/
/*  FUNCTION:    FlashIRLED                                         */
/*                                                                  */
/*  PARAMETERS:  None                                               */
/*                                                                  */
/*  DESCRIPTION: Flash the IR message received LED once             */
/*                                                                  */
/*  RETURNS:     Nothing                                            */
/********************************************************************/
void FlashIRLED(void)
{
  int iRetcode;

  if(!bIRLedInitialised)
    InitialiseIRLED();

  if (bIRLedInitialised && bFlashIRLEDEnabled)
  {
    /* Stop the tick timer and restart it to ensure we generate a */
    /* new pulse even if the LED is already lit.                  */

    iRetcode = tick_stop(hIRLedTick);
    if (iRetcode != RC_OK)
    {
      trace_new(TRACE_IRD|TRACE_LEVEL_ALWAYS, "IR: Can't stop LED timer. RC = 0x%x\n", iRetcode);
      return;
    }

    ir_set_led_state(TRUE);

    iRetcode = tick_set(hIRLedTick, IR_LED_FLASH_TIME, TRUE);
    if (iRetcode != RC_OK)
    {
      error_log(ERROR_WARNING|MOD_IRD);
      trace_new(TRACE_IRD|TRACE_LEVEL_ALWAYS, "IR: Can't set LED tick timer!\n");
    }
    else
    {
      iRetcode = tick_start(hIRLedTick);
      if (iRetcode != RC_OK)
      {
        trace_new(TRACE_IRD|TRACE_LEVEL_ALWAYS, "IR: Can't start LED timer. RC = 0x%x\n", iRetcode);
      }
    }
  }
}

/**********************/
/* Internal Functions */
/**********************/

/********************************************************************/
/*  FUNCTION:    InitialiseIRLED                                    */
/*                                                                  */
/*  PARAMETERS:  None                                               */
/*                                                                  */
/*  DESCRIPTION: Initialise the IR LED handler, creating the timer  */
/*               needed to generate the pulses.                     */
/*                                                                  */
/*  RETURNS:     TRUE on success, FALSE otherwise                   */
/********************************************************************/
bool InitialiseIRLED(void)
{
  if(bIRLedInitialised)
    return(TRUE);

  /* Create the tick timer we will use to time the flashes */
  hIRLedTick = tick_create(IRLedTickCallback, NULL, "IRTK");
  if (hIRLedTick)
  {
    trace_new(TRACE_IRD|TRACE_LEVEL_3, "IR: Initialised message received LED handler\n");
    bIRLedInitialised = TRUE;
    return(TRUE);
  }
  else
  {
    /* Tick timer creation failed */
    error_log(ERROR_WARNING|MOD_IRD);
    trace_new(TRACE_IRD|TRACE_LEVEL_ALWAYS, "IR: Can't create LED tick timer!\n");
    return(FALSE);
  }
}

/********************************************************************/
/*  FUNCTION:    ir_set_led_state                                   */
/*                                                                  */
/*  PARAMETERS:  state - TRUE for on, FALSE for off                 */
/*                                                                  */
/*  DESCRIPTION: Special function called by the IR driver to turn   */
/*               on and off the IR message LED. We can't go via the */
/*               SQ&C interface easily since it requires that all   */
/*               calls be made from the control task context and    */
/*               makes things rather more awkward than need be.     */
/*                                                                  */
/*  RETURNS:     Nothing                                            */
/*                                                                  */
/*  NOTES:       This function must be callable from interrupt      */
/*               context so it can't call existing driver functions */
/*               such as set_led_state which are not interrupt safe */
/*                                                                  */
/********************************************************************/
void ir_set_led_state(u_int8 state)
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
        cnxt_led_set((6+GPIO_DEVICE_ID_ISA_1+GPIO_PIN_IS_OUTPUT+GPIO_NEGATIVE_POLARITY),
                     (FP_LED_STATE)(state ? CNXT_LED_LIT : CNXT_LED_DARK));
    }
    else
#endif
/*
 * !!! HACK ALERT !!!
 * !!! HACK ALERT !!!
 */

   cnxt_led_set(PIO_LED_IR_MSG, (FP_LED_STATE)(state ? CNXT_LED_LIT : CNXT_LED_DARK));
}

/********************************************************************/
/*  FUNCTION:    IRLedTickCallback                                  */
/*                                                                  */
/*  PARAMETERS:  hTick - handle of the tick timer which timed out   */
/*               pUser - pointer passed to tick_create for this     */
/*                       timer.                                     */
/*                                                                  */
/*  DESCRIPTION: Turn the IR received LED off when a particular     */
/*               time has elapsed.                                  */
/*                                                                  */
/*  RETURNS:     Nothing                                            */
/********************************************************************/
void IRLedTickCallback(tick_id_t hTick, void *pUser)
{
  ir_set_led_state(FALSE);
}

/****************************************************************************
 * Modifications:
 * $Log: 
 *  10   mpeg      1.9         3/4/04 12:17:52 AM     Matt Korte      CR(s) 
 *        8510 : Updated ID, Log, CR lines
 *  9    mpeg      1.8         3/4/04 12:09:22 AM     Matt Korte      CR(s) 
 *        8510 : Add routine FlashIRLEDEnable()
 *  8    mpeg      1.7         4/1/03 2:03:04 PM      Tim White       SCR(s) 
 *        5925 :
 *        Added runtime support for both Bronco1 & Bronco3 boards with one 
 *        hwconfig file.  ALL CHANGES MADE WITH THIS DEFECT ARE HACKS.  Please 
 *        remove when support for Bronco1 is no longer required.
 *        
 *        
 *  7    mpeg      1.6         12/16/02 2:21:38 PM    Dave Wilson     SCR(s) 
 *        5080 :
 *        Reworked to use new generic GPIO description model and HWLIB LED 
 *        control
 *        functions.
 *        
 *  6    mpeg      1.5         11/20/02 10:45:20 AM   Ian Mitchell    SCR(s): 
 *        4993 
 *        Pulses the POWER led pin on IR reception if bradx box.
 *        
 *  5    mpeg      1.4         7/12/02 8:12:12 AM     Steven Jones    SCR(s): 
 *        4176 
 *        Build & run for/with a Brady box, hit any IR remote key and see the 
 *        LED flash.
 *        
 *  4    mpeg      1.3         5/15/02 5:40:04 PM     Larry Wang      SCR(s) 
 *        3764 :
 *        Reverse LED on/off logic only for Athens front pannel.
 *        
 *  3    mpeg      1.2         4/22/02 3:35:20 PM     Larry Wang      SCR(s) 
 *        3585 :
 *        For Athens board, IR LED is on GPIO extender and the bit needs to be 
 *        high to turn the LED on.
 *        
 *  2    mpeg      1.1         3/30/01 4:46:22 PM     Anzhi Chen      Changed 
 *        LED on time to 250ms.
 *        
 *  1    mpeg      1.0         2/22/01 12:07:08 PM    Anzhi Chen      
 * $
 ****************************************************************************/

