/****************************************************************************/
/*                          Conexant Systems Inc.                           */
/****************************************************************************/
/*                                                                          */
/* Filename:       SCANBTNS_DARE.C                                               */
/*                                                                          */
/* Description:    Generic front panel button driver for scan matrix        */
/*                 implementation used on Klondike IRD                      */
/*                                                                          */
/* Author:         Dave Wilson                                              */
/*                                                                          */
/* Copyright Conexant Systems Inc, 2000                                     */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/
/* $Header: scanbtns.c, 23, 7/22/03 12:11:16 PM, Bobby Bradford$
 ****************************************************************************/

/*********************************************************************************/
/* The basic operation of this driver is as follows:                             */
/*                                                                               */
/* All row GPIOs are set to drive 0s.                                            */
/* All columns are set as inputs with negative edge interrupts.                  */
/* When any interrupt fires, all column interrupts are disabled and a debounce   */
/*   timer is started.                                                           */
/* Once the timer times out, all buttons are scanned to find their states.       */
/* If any states have changed, the appropriate message is sent to the registered */
/*   callback.                                                                   */
/* Interrupts are reenabled and the driver waits for the next button press.      */
/*********************************************************************************/

/*****************/
/* Include Files */
/*****************/
#include "stbcfg.h"
#include <stdio.h>
#include "retcodes.h"
#include "hwlib.h"
#include "basetype.h"
#include "gpio.h"
#include "buttons.h"
#include "scanbtns.h"
#if RTOS != NOOS
#include "kal.h"
#include "globals.h"
#endif

/******************************************/
/* Local Definitions and Global Variables */
/******************************************/
#define CONTINUOUS_PRESS 

#define  LED_SLICE_TIMEOUT 1
#define  DEBOUNCE_TIMEOUT  LED_SLICE_TIMEOUT /* milliseconds */
#ifndef FRONT_PANEL_KEYPAD_NUM_ROWS
#define FRONT_PANEL_KEYPAD_NUM_ROWS (5)
#endif

#ifndef FRONT_PANEL_KEYPAD_NUM_COLS
#define FRONT_PANEL_KEYPAD_NUM_COLS (2)
#endif

#ifndef PIO_FRONT_PANEL_KEYPAD_ROW_0
#define PIO_FRONT_PANEL_KEYPAD_ROW_0 (27+GPIO_DEVICE_ID_INTERNAL+GPIO_POSITIVE_POLARITY+GPIO_PIN_IS_OUTPUT)
#endif
#define  BTN_NUM_ROWS      FRONT_PANEL_KEYPAD_NUM_ROWS
#define  BTN_NUM_COLS      FRONT_PANEL_KEYPAD_NUM_COLS
#define  BTN_GPIO_LED      PIO_FRONT_PANEL_KEYPAD_ROW_0

#define LED_NUMBER_COUNTER 5

#define F_DATA_PIO_NUM  PIO_FRONT_PANEL_KEYPAD_DATA
#define F_CLK_PIO_NUM  PIO_FRONT_PANEL_KEYPAD_CLK
/*add by linjianpeng,3-1-2005*/
#define MaxCycTimer 150
#define MaxCycTimerTimes 21
#define OneSecond 440
static unsigned char led_number_table_display_time_point[] =
    {
	     0x40, //			case 0:	Ls164_data = 0x42;
           0x79, //			case 1:	Ls164_data = 0x7b;
	    0x24, //			case 2:	Ls164_data = 0xc8;
	    0x30, //			case 3:	Ls164_data = 0x68;
	    0x19, //			case 4:	Ls164_data = 0x71;
           0x12, //	      	case 5:	Ls164_data = 0xb4;
	    0x02, //			case 6:	Ls164_data = 0x84;
	    0x78, //			case 7:	Ls164_data = 0x82;
	    0x00, //			case 8:	Ls164_data = 0x80;
	    0x10, //			case 9:	Ls164_data = 0xa0;
	    
    };

static unsigned char led_number_table_display_time[] =
    {
	       0xC0, //			case 0:	Ls164_data = 0x42;
        0xF9, //			case 1:	Ls164_data = 0x7b;
	    0xA4, //			case 2:	Ls164_data = 0xc8;
	    0xB0, //			case 3:	Ls164_data = 0x68;
	    0x99, //			case 4:	Ls164_data = 0x71;
        0x92, //	      	case 5:	Ls164_data = 0xb4;
	    0x82, //			case 6:	Ls164_data = 0x84;
	    0xF8, //			case 7:	Ls164_data = 0x82;
	    0x80, //			case 8:	Ls164_data = 0x80;
	    0x90, //			case 9:	Ls164_data = 0xa0;
    };
/*end add*/

/*****************************/
/*        Externs            */
/*****************************/
#if EMULATION_LEVEL == FINAL_HARDWARE
extern button_data button_matrix[BTN_NUM_ROWS][BTN_NUM_COLS];
#endif
/*delete by linjianpeng*/
//static unsigned char fp_led_data[LED_NUMBER_COUNTER]={0xc0, 0xc0, 0xc0, 0xc0,0xdf};
/*end delete*/
/*add by linjianpeng for the led display powered on*/
u_int32 curr_channel;
//extern u_int32 uFTJudge;
//extern u_int32 chmgr_get_curr_channel();
//extern void get_current_time(u_int8* hour, u_int8* min);
static bool bCycLed=0;//record the led state
static bool bCycTimer=0;//record the time when next led state displays
static bool bCycTimerTimes=0;//record the timers the led state changes 

static unsigned char fp_led_data[LED_NUMBER_COUNTER]={0xd7, 0xfe, 0xf7, 0xee,0xfa};//add by linjianpeng
//static unsigned char fp_led_data[LED_NUMBER_COUNTER]={0xc0, 0xc0, 0xc0, 0xc0,0xc0};//add by linjianpeng
//end add
int power_state=1;
int standby_state=0;
int lock_state=0; 
//extern bool strength_quality_state;//add by wlk ,12-13-2004



#if RTOS != NOOS
/*****************************/
/* Local Function Prototypes */
/*****************************/
static int  button_int_handler(u_int32 dwIntID, bool bFIQ, void *pfnChain);
static void key_detect_timer_handler(timer_id_t timer, void *userData);
static void led_glint_timer_handler(timer_id_t timer, void *userData);
static bool set_matrix_for_detect(void);
static u_int32 query_row_state(int iRow);
static bool scan_button_is_pressed(u_int32 key_code);

/***************/
/* Global Data */
/***************/
PFNISR  pfnChain[BTN_NUM_COLS];
static tick_id_t  timerKeyDetect;
static tick_id_t  timerLedGlint;
static bool bInitialised = FALSE;
static PFNBUTTONCALLBACK pfnBtnCallback = (PFNBUTTONCALLBACK)NULL;
static volatile u_int32 button_states;
static volatile u_int32 button_matrix_row_index = 0;
//static volatile u_int32 button_matrix_col_index = 0;
static volatile bool bKeyPressed = FALSE;
//static u_int32 lastState = 0;
static volatile u_int32 lastState=0;
int count_interrupt =0;
int button_mask = 0;
int button_mask1 = 0;
int button_mask_up=0;
int continue_press=0;
static bool button_power_press=FALSE;
bool button_power_release=FALSE;
/*int power_state=1;
int standby_state=0;
int lock_state=0; */
static u_int32 last_row_index=0;
u_int32   channel_number_display;
/*add by linjianepng,3-1-2005*/
u_int32 uKeyTimer=MaxKeyTimer;
u_int32 uMicrosecond =0;
u_int32 uShowPoint=0;
void mwavm_set_led(u_int32 value)
{
	fp_led_data[4]=value;
}
/*end add*/

/********************************************************************/
/*  FUNCTION:    button_init                                        */
/*                                                                  */
/*  PARAMETERS:  pfnCallback - notification function called when a  */
/*                             key changes state.                   */
/*                                                                  */
/*  DESCRIPTION: Initialise the scan matrix button driver.          */
/*                                                                  */
/*  RETURNS:     TRUE on success, FALSE on failure                  */
/********************************************************************/
/*      WARNING: Renamed if GPIOBTNS is also in the build!          */
/********************************************************************/


//button_data button_matrix[FRONT_PANEL_KEYPAD_NUM_ROWS][FRONT_PANEL_KEYPAD_NUM_COLS] = { 
///*    Row 0 */
//     {                                                                                     
//         {PIO_FRONT_PANEL_KEYPAD_ROW_0,  PIO_FRONT_PANEL_KEYPAD_COL_0,   BTN_CODE_DOWN }, 
//         {PIO_FRONT_PANEL_KEYPAD_ROW_0,  PIO_FRONT_PANEL_KEYPAD_COL_1,   BTN_CODE_POWER  }  
//     },                                                                                     
//                                                                                           
///*    Row 1 */
//     {                                                                                     
//         {PIO_FRONT_PANEL_KEYPAD_ROW_1,  PIO_FRONT_PANEL_KEYPAD_COL_0,   BTN_CODE_MENU  }, 
//         {PIO_FRONT_PANEL_KEYPAD_ROW_1,  PIO_FRONT_PANEL_KEYPAD_COL_1,   BTN_CODE_LEFT    }  
//      },                                                                                    
//                                                                                           
///*    Row 2 */
//     {                                                                                     
//         {PIO_FRONT_PANEL_KEYPAD_ROW_2,  PIO_FRONT_PANEL_KEYPAD_COL_0,   BTN_CODE_SELECT  }, 
//         {PIO_FRONT_PANEL_KEYPAD_ROW_2,  PIO_FRONT_PANEL_KEYPAD_COL_1,   BTN_CODE_RIGHT }  
//     },                                                                                     
//                                                                                           
///*    Row 3 */
//     {                                                                                     
//         {PIO_FRONT_PANEL_KEYPAD_ROW_3,  PIO_FRONT_PANEL_KEYPAD_COL_0,   BTN_CODE_UP}, 
//         {PIO_FRONT_PANEL_KEYPAD_ROW_3,  PIO_FRONT_PANEL_KEYPAD_COL_1,   BTN_CODE_NONE}
//     }                                                                                     
//}; 
/*增加前面板定义，根据盒子的大小*/
#if defined(FONTPANEL_LARGE) && (FONTPANEL_LARGE == YES)
button_data button_matrix[FRONT_PANEL_KEYPAD_NUM_ROWS][FRONT_PANEL_KEYPAD_NUM_COLS] = { 
/*    Row 0 */
     {                                                                                     
         {PIO_FRONT_PANEL_KEYPAD_ROW_0,  PIO_FRONT_PANEL_KEYPAD_COL_0,BTN_CODE_UP }, 
         {PIO_FRONT_PANEL_KEYPAD_ROW_0,  PIO_FRONT_PANEL_KEYPAD_COL_1,BTN_CODE_DOWN }  
     },                                                                                     
                                                                                           
/*    Row 1 */
     {                                                                                     
         {PIO_FRONT_PANEL_KEYPAD_ROW_1,  PIO_FRONT_PANEL_KEYPAD_COL_0,  BTN_CODE_MENU}, 
         {PIO_FRONT_PANEL_KEYPAD_ROW_1,  PIO_FRONT_PANEL_KEYPAD_COL_1,  BTN_CODE_SELECT}  
      },                                                                                    
                                                                                           
/*    Row 2 */
     {                                                                                     
         {PIO_FRONT_PANEL_KEYPAD_ROW_2,  PIO_FRONT_PANEL_KEYPAD_COL_0,BTN_CODE_NONE }, 
         {PIO_FRONT_PANEL_KEYPAD_ROW_2,  PIO_FRONT_PANEL_KEYPAD_COL_1, BTN_CODE_NONE}  
     },                                                                                     
                                                                                           
/*    Row 3 */
     {                                                                                     
         {PIO_FRONT_PANEL_KEYPAD_ROW_3,  PIO_FRONT_PANEL_KEYPAD_COL_0, BTN_CODE_POWER}, 
         {PIO_FRONT_PANEL_KEYPAD_ROW_3,  PIO_FRONT_PANEL_KEYPAD_COL_1, BTN_CODE_NONE}
     },                                                                                     
                                                                                           
/*    Row 4 */
     {                                                                                     
         {PIO_FRONT_PANEL_KEYPAD_ROW_4,  PIO_FRONT_PANEL_KEYPAD_COL_0, BTN_CODE_RIGHT}, 
         {PIO_FRONT_PANEL_KEYPAD_ROW_4,  PIO_FRONT_PANEL_KEYPAD_COL_1, BTN_CODE_LEFT}
     }                                                                                       
};    
#endif

/*增加前面板定义，根据盒子的大小*/
#if defined(FONTPANEL_SMALL) && (FONTPANEL_SMALL == YES)
button_data button_matrix[FRONT_PANEL_KEYPAD_NUM_ROWS][FRONT_PANEL_KEYPAD_NUM_COLS] = { 
/*    Row 0 */
     {                                                                                     
         {PIO_FRONT_PANEL_KEYPAD_ROW_0,  PIO_FRONT_PANEL_KEYPAD_COL_0,BTN_CODE_MENU }, 
         {PIO_FRONT_PANEL_KEYPAD_ROW_0,  PIO_FRONT_PANEL_KEYPAD_COL_1,BTN_CODE_SELECT }  
     },                                                                                     
                                                                                           
/*    Row 1 */
     {                                                                                     
         {PIO_FRONT_PANEL_KEYPAD_ROW_1,  PIO_FRONT_PANEL_KEYPAD_COL_0,  BTN_CODE_DOWN   }, 
         {PIO_FRONT_PANEL_KEYPAD_ROW_1,  PIO_FRONT_PANEL_KEYPAD_COL_1,  BTN_CODE_UP   }  
      },                                                                                    
                                                                                           
/*    Row 2 */
     {                                                                                     
         {PIO_FRONT_PANEL_KEYPAD_ROW_2,  PIO_FRONT_PANEL_KEYPAD_COL_0,BTN_CODE_NONE }, 
         {PIO_FRONT_PANEL_KEYPAD_ROW_2,  PIO_FRONT_PANEL_KEYPAD_COL_1, BTN_CODE_NONE}  
     },                                                                                     
                                                                                           
/*    Row 3 */
     {                                                                                     
         {PIO_FRONT_PANEL_KEYPAD_ROW_3,  PIO_FRONT_PANEL_KEYPAD_COL_0, BTN_CODE_POWER}, 
         {PIO_FRONT_PANEL_KEYPAD_ROW_3,  PIO_FRONT_PANEL_KEYPAD_COL_1, BTN_CODE_NONE}
     },                                                                                     
                                                                                           
/*    Row 4 */
     {                                                                                     
         {PIO_FRONT_PANEL_KEYPAD_ROW_4,  PIO_FRONT_PANEL_KEYPAD_COL_0, BTN_CODE_LEFT}, 
         {PIO_FRONT_PANEL_KEYPAD_ROW_4,  PIO_FRONT_PANEL_KEYPAD_COL_1, BTN_CODE_RIGHT}
     }                                                                                       
};                                                                                    
#endif
/*
 * NOTE:  For now, all of the scan matrix front panels use the same
 * matrix for row/column/button
 */

/*              4          5                
 *             col 0     col 1    
 *  1  row 0    SW2       SW7    
 *  2  row 1    SW3       SW5        
 *  3  row 2    SW4       SW6
 *  4  row 3    SW1        x     
 */


bool button_init(PFNBUTTONCALLBACK pfnCallback, lpbutton_ftable lpFuncs)
{
#if EMULATION_LEVEL == FINAL_HARDWARE
   int  iRetcode;
   int  i,j;
   int  isr_registered[BTN_NUM_COLS];
   button_data *t;
   u_int16 uBit = 0;

    static u_int32 mux_reg_table[]=
    {
        PLL_PIN_GPIO_MUX0_REG,
        PLL_PIN_GPIO_MUX1_REG,
        PLL_PIN_GPIO_MUX2_REG,
        PLL_PIN_GPIO_MUX3_REG
    };
   
   #if defined(PIO_FRONT_PANEL_KEYPAD_DATA) && defined(PIO_FRONT_PANEL_KEYPAD_CLK)
   static u_int32 data_clk[] =
   {
      PIO_FRONT_PANEL_KEYPAD_DATA,
      PIO_FRONT_PANEL_KEYPAD_CLK
   };
   #endif
   /* clear out this array */

   for (i=0; i < BTN_NUM_COLS; i++) isr_registered[i] = 0;

   button_matrix_row_index = 0;
    
   /* Store the callback pointer and our function pointer */
   if (pfnCallback) {
       pfnBtnCallback = pfnCallback;
   } else {
       trace_new(TRACE_KEY | TRACE_LEVEL_ALWAYS, 
                 "BUTTON: Line %d: Error: Pointer to callback function is NULL!\n",
                 __LINE__);
   }

   if(button_get_pointers(lpFuncs))
   {
    
      #if defined(PIO_FRONT_PANEL_KEYPAD_DATA) && defined(PIO_FRONT_PANEL_KEYPAD_CLK)
      for (i=0; i< sizeof(data_clk)/4; i++)
      {
         uBit    = data_clk[i] & GPIO_MASK_PIN_NUMBER;
         CNXT_SET(mux_reg_table[uBit/32], 1<<uBit%32, 0);
      }
      #endif
     cnxt_gpio_set_output_level(F_CLK_PIO_NUM, 0);
     
     /* Grab the interrupts we need for the columns */
     for (i = 0; i < BTN_NUM_ROWS; i++) {
         for (j = 0; j < BTN_NUM_COLS; j++) {
             /* we have to be careful not to register a column int
              * over and over for each row.  However, we need to 
              * go through every row to make sure we find every column.
              */
             t = &button_matrix[i][j];
 
             if (t->row_code != 0)
             {
                uBit    = t->row_code & GPIO_MASK_PIN_NUMBER;
                CNXT_SET(mux_reg_table[uBit/32], 1<<uBit%32, 0);
                
             }
             
             if ( (t->column_code != 0) && (isr_registered[j] == 0) ) {
                 isr_registered[j] =  t->column_code;
                 cnxt_gpio_int_register_isr( t->column_code,
                                   button_int_handler,
                                   FALSE,
                                   FALSE,
                                   &pfnChain[j]);
                 uBit    = t->column_code & GPIO_MASK_PIN_NUMBER;
                 CNXT_SET(mux_reg_table[uBit/32], 1<<uBit%32, 0);
                 
                 //cnxt_gpio_int_enable( t->column_code );
                 cnxt_gpio_set_input(t->column_code);
             }
                            
         }
     }
                 
     /* set all buttons to up position */
     button_states = 0x0;
     
     /* Grab a timer and set its period to the debounce timeout */


     timerLedGlint = tick_create(led_glint_timer_handler, 0, "LEDG");
     if(timerLedGlint == 0)
     {
        error_log(ERROR_FATAL | RC_KEY_TIMERCREATE);
        return(FALSE); /* Should never be reached */
     }
  
     iRetcode = tick_set(timerLedGlint, DEBOUNCE_TIMEOUT, FALSE);     
     if(iRetcode != RC_OK)
     {
        error_log(ERROR_FATAL | RC_KEY_TIMERSET);
        return(FALSE); /* Should never be reached */
     }     
     
    timerKeyDetect = tick_create(key_detect_timer_handler, 0, "BTNT");
     if(timerKeyDetect == 0)
     {
        error_log(ERROR_FATAL | RC_KEY_TIMERCREATE);
        return(FALSE); /* Should never be reached */
     } 
  
     iRetcode = tick_set(timerKeyDetect, LED_SLICE_TIMEOUT, FALSE);
     if(iRetcode != RC_OK)
     {
        error_log(ERROR_FATAL | RC_KEY_TIMERSET);
        return(FALSE); /* Should never be reached */
     }
   
     bInitialised = set_matrix_for_detect();
     
     tick_start(timerLedGlint);
     return(bInitialised);
   } else {
     return(FALSE);
   }
#else
   return (TRUE);
#endif
}

/********************************************************************/
/*  FUNCTION:    button_get_pointers                                */
/*                                                                  */
/*  PARAMETERS:  lpFuncs - pointer to a button_ftable structure     */
/*                         into which we should write our pfns      */
/*                                                                  */
/*  DESCRIPTION: Retrieve the drivers function entry points.        */
/*                                                                  */
/*  RETURNS:     Nothing                                            */
/********************************************************************/
/*      WARNING: Renamed if GPIOBTNS is also in the build!          */
/********************************************************************/
bool button_get_pointers(lpbutton_ftable lpFuncs)
{
#if EMULATION_LEVEL == FINAL_HARDWARE
  if (lpFuncs)
  {
    lpFuncs->button_is_pressed = scan_button_is_pressed;
    return(TRUE);
  } else {
    return(FALSE);  
  }
#else
  return(TRUE);  
#endif
}


/********************************************************************/
/*  FUNCTION:    scan_button_is_pressed                                  */
/*                                                                  */
/*  PARAMETERS:  key_code - code of button whose state is to be     */
/*                          queried.                                */
/*                                                                  */
/*  DESCRIPTION: Return the state of a particular button.           */
/*                                                                  */
/*               If the driver has already been initialised, the    */
/*               state returned is read from our state shadow       */
/*               array.                                             */
/*               If the driver has not already been initialised,    */
/*               the hardware is queried to determine the state.    */
/*                                                                  */
/*  RETURNS:     TRUE if button is pressed. FALSE is not pressed    */
/*               or an unsupported button code is passed.           */
/*                                                                  */
/********************************************************************/
static bool scan_button_is_pressed(u_int32 key_code)
{
#if EMULATION_LEVEL == FINAL_HARDWARE
  int  iRow, iCol;
  bool bFound = FALSE;
  u_int32 dwState;
  bool bPressed;
  
  /* Figure out which physical button to query by looking through */
  /* our array of button codes to find the one requested.         */
  for (iRow = 0; iRow < BTN_NUM_ROWS; iRow++) {
    for (iCol = 0; iCol < BTN_NUM_COLS; iCol++) {
      if (button_matrix[iRow][iCol].button_code == key_code) {
        bFound = TRUE;
        break;
      }  
    }
    if(bFound) {
      break;
    }
  }
  
  /* If we found the key, go ahead and return its state */
  if (bFound) {
    if (bInitialised) {
      /* The driver is already initialised so our shadow copy of the button states */
      /* will be up to date. In this case, just tell the caller the state that we  */
      /* last read.                                                                */
                
      bPressed = (button_states & key_code ? 1 : 0);
    } else {
      /* The driver has not been initialised so call the hardware directly. If we */
      /* do this after the driver is initialised, it would mess up the state of   */
      /* the hardware and prevent future button press/release events from being   */
      /* caught so DON'T DO IT.                                                   */
      dwState = query_row_state(iRow);
      if(dwState & key_code) {
        bPressed = TRUE;
      } else {
        bPressed = FALSE;  
      }
    }
    
    trace_new(TRACE_KEY | TRACE_LEVEL_2,
              "Query keycode %d - row %d, col %d, state %d\n",
              key_code,
              iRow,
              iCol,
              bPressed);
    
    return(bPressed);
  } else {
    /* If we couldn't find the keycode, generate an error and return FALSE */
    trace_new(TRACE_KEY | TRACE_LEVEL_ALWAYS, 
              "Keycode %d not found\n", key_code);
    error_log(MOD_KEY | ERROR_WARNING);
    
    return(FALSE);
  }  
#else
  return (FALSE);
#endif
}

/********************************************************************/
/*  FUNCTION:    button_int_handler                                 */
/*                                                                  */
/*  PARAMETERS:  dwIntID - ID of interrupt we are being called to   */
/*                         service.                                 */
/*               bFIQ    - TRUE if FIQ, FALSE if IRQ                */
/*               pfnCh   - Pointer to storage for returned chain    */
/*                         vector.                                  */
/*                                                                  */
/*  DESCRIPTION: Handle any interrupt generated by the column GPIOs */
/*                                                                  */
/*  RETURNS:     RC_ISR_HANDLED - interrupt was processed           */
/*               RC_ISR_NOT_HANDLED - error. Int is not ours.       */
/*                                                                  */
/********************************************************************/
static int button_int_handler(u_int32 dwIntID, bool bFIQ, void *pfnCh)
{
#if (EMULATION_LEVEL == FINAL_HARDWARE)
  int i,j;
  //int iRetcode;
  int iColNum = -1;
  u_int32 temp;

  
  /* Find which column generated the interrupt */
  for (i = 0; i < BTN_NUM_ROWS; i++) {
     for (j = 0; j < BTN_NUM_COLS; j++) {
         temp = button_matrix[i][j].column_code;
         if (CNXT_GPIO_STATUS_OK == cnxt_gpio_int_query_id(temp, dwIntID)) {
           iColNum = j;
           break;  
         }
     }
     if (iColNum != -1) {
         break;
     }
  }    
  
  if (iColNum != -1) {
    /* Disable all the column interrupts*/
      for (i = 0; i < BTN_NUM_ROWS; i++) {
          for (j = 0; j < BTN_NUM_COLS; j++) {
              temp = button_matrix[i][j].column_code;
              cnxt_gpio_int_disable( temp );
           }
      }
       bKeyPressed = TRUE;
      /* We found a valid column so start the debounce timer */
//      iRetcode = tick_stop ( timerLedGlint );
//      if(iRetcode != RC_OK) {
//          isr_error_log(ERROR_WARNING | RC_KEY_TIMERSTART);
//      }
      
     // iRetcode = tick_start(timerKeyDetect);
      
    //  if(iRetcode != RC_OK)
         // isr_error_log(ERROR_WARNING | RC_KEY_TIMERSTART);
  
      *(PFNISR *)pfnCh = pfnChain[iColNum];
      return (RC_ISR_HANDLED);
  } else {
      /* Error case - our vector was called for an interrupt we didn't */
      /* register!                                                     */
      isr_error_log(ERROR_WARNING | RC_KEY_ISR);
      *(PFNISR *)pfnCh = pfnChain[0];
      return(RC_ISR_NOTHANDLED);
  }// Laiyunliang recover the else {}

  
#else
  return (RC_ISR_NOTHANDLED);
#endif
}

/*************************************/
/* Timer handler for detecting a key */
/*************************************/
static void key_detect_timer_handler(timer_id_t timer, void *userData)
{
#if EMULATION_LEVEL == FINAL_HARDWARE

   #ifdef CONTINUOUS_PRESS
        static int count =0;
   #endif
    
  static u_int32 lastState = 0;
  u_int32 dwState;
  
  
  /* Make sure this is our timer */
  if(timer == timerKeyDetect) {
     button_states = 0;    
    /* Read the state of each button and send a message to the caller */
    /* if state has changed.                                          */
//    for (i = 0; i < BTN_NUM_ROWS; i++) {
      /* dwState contains the bitmask of which buttons 
       * are pressed in the form of BTN_CODE.  If all
       * buttons have been released then dwState will
       * be zero for every row. 
       */
      dwState = query_row_state(0);
         
      if (dwState != lastState) {
          /* Button state has changed. Send a message */
          pfnBtnCallback(dwState ^ lastState, ( ( (dwState ^ lastState) & dwState) > 0 ) );
          
          #ifdef CONTINUOUS_PRESS    
          count = 0;
          #endif          
      }
      lastState = dwState;
      button_states |= lastState;
      
      /* if button has been released, stop debounce timer */
      if ( dwState == 0 )
      {             
         tick_stop ( timerKeyDetect );
         tick_start ( timerLedGlint );
         
         /* Reset the scan matrix state to wait for the next press or release */
         set_matrix_for_detect();
      }            

    #ifdef CONTINUOUS_PRESS    
        if ( (count*DEBOUNCE_TIMEOUT)>500 )
        {
            count = 0;
            pfnBtnCallback(dwState, 1);
        }
        count++;
    #endif    
  }
#endif
}

/*************************************/
/* Timer handler for led glint       */
/* & front key scan       */
/*************************************/
static void led_glint_timer_handler(timer_id_t timer, void *userData)
{
#if EMULATION_LEVEL == FINAL_HARDWARE
   #ifdef CONTINUOUS_PRESS
	static int count =0;
   #endif
    int i = 0;
    int j=0;
    //extern unsigned char led_testing;
    u_int8 hour,minute;
	button_data *t = &button_matrix[button_matrix_row_index][0];  
	u_int32 dwState;
	bool buttonInputLevel=0x01;

     #if 0 //delete Vicegod modify
     curr_channel=chmgr_get_curr_channel();

     if(standby_state==0)
     {
        if ((curr_channel==0)&&(uFTJudge!=1)&&(bCycTimerTimes==MaxCycTimerTimes)&&!led_testing)
        {
            fp_led_data[0]=0xc0;
            fp_led_data[1]=0xc0;
            fp_led_data[2]=0xc0;
            fp_led_data[3]=0xc0;
        }
        else if((curr_channel==0)&&(uFTJudge!=1)&&(bCycLed==0)&&!led_testing)
        {
            fp_led_data[0]=0xd7;
            fp_led_data[1]=0xfe;
            fp_led_data[2]=0xf7;
            fp_led_data[3]=0xfa;
        }
        else if((curr_channel==0)&&(uFTJudge!=1)&&(bCycLed==1)&&!led_testing)
        {
            fp_led_data[0]=0xee;
            fp_led_data[1]=0xf7;
            fp_led_data[2]=0xfe;
            fp_led_data[3]=0xf5;
        }
    }
    else if(standby_state==1)
    {
        if(uMicrosecond<OneSecond)
            uMicrosecond++;
        if(uMicrosecond==OneSecond)
        {
            uMicrosecond=0;
            uShowPoint=1-uShowPoint;
        }
        
        get_current_time(&hour,&minute);
        if(uShowPoint==0)
        {    
            fp_led_data[0]=led_number_table_display_time[hour/10];
            fp_led_data[1]=led_number_table_display_time[hour%10];
            fp_led_data[2]=led_number_table_display_time[minute/10];
            fp_led_data[3]=led_number_table_display_time[minute%10];
            fp_led_data[4]=0xef;
        }
        if(uShowPoint==1)
        {
            fp_led_data[0]=led_number_table_display_time[hour/10];
            fp_led_data[1]=led_number_table_display_time_point[hour%10];
            fp_led_data[2]=led_number_table_display_time[minute/10];
            fp_led_data[3]=led_number_table_display_time[minute%10];
            fp_led_data[4]=0xef;
        }
    }
    #endif
    
	/*set gpio output level*/
	button_mask1  = button_mask;
	for (i = 0; i < BTN_NUM_ROWS; i++) 
	{
		t = &button_matrix[i][0];
		cnxt_gpio_set_output_level(t->row_code,1);//wlk
	}
    
    /* Make sure this is our timer */
	if(timer == timerLedGlint) 
	{
		for (i=0; i<8; i++)
		{    
			cnxt_gpio_set_output_level(F_CLK_PIO_NUM, 0);
			cnxt_gpio_set_output_level(F_CLK_PIO_NUM, 0);
			cnxt_gpio_set_output_level(F_CLK_PIO_NUM, 0);
			if ( fp_led_data[button_matrix_row_index] & (0x80>>i) )
			{
				cnxt_gpio_set_output_level(F_DATA_PIO_NUM, TRUE);
			}
			else
			{
				cnxt_gpio_set_output_level(F_DATA_PIO_NUM, FALSE);
			}
			cnxt_gpio_set_output_level(F_CLK_PIO_NUM, 1);   
		}
		
		cnxt_gpio_set_output_level(F_CLK_PIO_NUM, 0);
		
		t = &button_matrix[last_row_index][0];
		cnxt_gpio_set_output_level(t->row_code,1);
		t=&button_matrix[button_matrix_row_index][0];
		cnxt_gpio_set_output_level(t->row_code, 0);

		for (j = 0; j < BTN_NUM_COLS; j++) 
		{
            /* again, we're going to do some unnecessary work in this loop,
             * but to understand why, first understand how the button_matrix
             * array is layed out.  Then, consider a 4x4 button matrix that has 
             * only one button defined in the (bottom of the) fourth column. 
             * Obviously, if we didn't loop for every column, we might never drive 
             * the row low/off.
             */
			t = &button_matrix[button_matrix_row_index][j];
			/* now read the value */
			cnxt_gpio_get_level(t->column_code, &buttonInputLevel);
			if ( buttonInputLevel == 0 )//wlk
			{
				//first col
				if(j==0)
				{
					button_mask= t->button_code;
					if(button_matrix_row_index==3)
					{
						button_power_press=TRUE;
					}
				}
				//second col
				else if(j==1)
				{
					button_mask_up=t->button_code;
				}
			} 
			else
			{
				//first col
				if(j==0)
				{
					button_mask=0;
					if(button_matrix_row_index==3)
					{
						button_power_release=TRUE;
					}
				}
				//second col
				else if(j==1)
				{
					button_mask_up=0;
				}
			}  
             
		} 
		if(button_mask_up!=0)
		{
			button_mask=button_mask_up;
		}
		
		t = &button_matrix[button_matrix_row_index][0];
		if(bKeyPressed == TRUE)
		{
			dwState=button_mask1;
			if (((dwState != lastState)&&(button_mask1!=0))||
				((count_interrupt>300)&&(button_mask1!=0)))
			{
          		/* Button state has changed. Send a message */
				if(dwState!=0x01)
				{
					pfnBtnCallback(dwState, 1);
					button_power_press=FALSE;
				}
				
				if(count_interrupt>300)
				{ 
					continue_press=0;
					count_interrupt=0;
				}
				
				continue_press=1;
				lastState = dwState;
				#ifdef CONTINUOUS_PRESS    
				count = 0;
				#endif          
			}

			if(button_power_press&&button_power_release&&(button_mask1!=0))
			{
				pfnBtnCallback(dwState, 1);
				button_power_press=FALSE;
				button_power_release=FALSE;				
			}
      
			/* if button has been released, stop debounce timer */
			if ( (button_mask1 == 0)&&(lastState!=button_mask1) )
			{  
				/* Reset the scan matrix state to wait for the next press or release */
				set_matrix_for_detect();
				bKeyPressed = FALSE;
				return;
			}            
			#ifdef CONTINUOUS_PRESS    
			if ( (count*DEBOUNCE_TIMEOUT)>500 )
			{
				count = 0;
				pfnBtnCallback(dwState, 1);
			}
			count++;
			#endif 
		}
		last_row_index=button_matrix_row_index;//add by wlk,12-10-2004
		button_matrix_row_index++;
		if(button_matrix_row_index==5)
		{
			button_matrix_row_index=0;
		}
		if(continue_press==1)       
		{
			count_interrupt++;
		}
	}
	/*add by linjianpeng,2-28-2005*/
	if(uKeyTimer<MaxKeyTimer)
	{
		uKeyTimer++;
	}
	bCycTimer++;
	if(bCycTimer==MaxCycTimer)
	{
		if(bCycTimerTimes<MaxCycTimerTimes)
		{
			bCycTimerTimes++;
		}
		bCycTimer=0;
		bCycLed=1-bCycLed;
	}
	/*end add*/	
#endif
}




/********************************************************************/
/*  FUNCTION:    set_matrix_for_detect                              */
/*                                                                  */
/*  PARAMETERS:  None                                               */
/*                                                                  */
/*  DESCRIPTION: Set up the GPIO and interrupt states to wait for   */
/*               the next button press or release.                  */
/*                                                                  */
/*  RETURNS:     TRUE on success, FALSE on error                    */
/*                                                                  */
/********************************************************************/
static bool set_matrix_for_detect(void)
{
#if EMULATION_LEVEL == FINAL_HARDWARE
   /* By definition, each row should be on one gpio
    * and each column should be on one gpio.  However, in the 
    * button_matrix array, each button could be on a completely
    * different row and column.  Therefore, we're going to
    * support that here.  This will most likely lead to 
    * the same GPIOs being set multiple times, but it 
    * provides the most flexibility 
    */

   button_data *t;
   int i,j;
   int ks;
   
   ks = critical_section_begin();
   for (j = 0; j < BTN_NUM_COLS; j++) {
       for (i = 0; i < BTN_NUM_ROWS; i++) {
           t = &button_matrix[i][j];
           /* for each button, we need to set its row to drive a 0 */
           cnxt_gpio_set_output_level(t->row_code, 1);
           /* On the columns, we want to detect an edge interrupt */
           cnxt_gpio_set_int_edge(t->column_code, BOTH_EDGES);
           cnxt_gpio_int_enable(t->column_code);
           cnxt_gpio_clear_pic_interrupt(t->column_code);
       }
   }
   critical_section_end(ks);
#endif
   return TRUE;

}

#endif /* RTOS != NOOS */

/********************************************************************/
/*  FUNCTION:    query_row_state                                    */
/*                                                                  */
/*  PARAMETERS:  iRow - the row number whose button states are to   */
/*                      be queried                                  */
/*                                                                  */
/*  DESCRIPTION: Read the states of all buttons in a single scan    */
/*               matrix row and return a value which is the logical */
/*               OR of all the rows buttone_codes                   */
/*                                                                  */
/*  RETURNS:     Bit mask containing a 1 if a given button is       */
/*               pressed or 0 if unpressed. Bit masks are the       */
/*               same as the keycodes defined in BUTTONS.H.         */
/*                                                                  */
/*  SIDE EFFECT: On exit, the GPIOs are set up to read the row. It  */
/*               is expected that the calling code will reset the   */
/*               row GPIO states after finishing all the queries    */
/*               it needs to make.                                  */
/*                                                                  */
/********************************************************************/
u_int32 query_row_state(int iRow)
{
#if EMULATION_LEVEL == FINAL_HARDWARE
   int i,j;
   button_data *t;
   int button_mask = 0;
   bool bInputLevel;

   /* Set all rows to high-z */
   for (i = 0; i < BTN_NUM_ROWS; i++) {
       for (j = 0; j < BTN_NUM_COLS; j++) {
           t = &button_matrix[i][j];
           //cnxt_gpio_set_output_level(t->row_code, 0);
           cnxt_gpio_set_output_level(t->row_code, 1);//wlk
       }
   }
   
   for (i = 0; i < BTN_NUM_ROWS; i++) {

        t = &button_matrix[i][0];
       // cnxt_gpio_set_output_level(t->row_code, 1);
       cnxt_gpio_set_output_level(t->row_code, 0);//wlk
          
        for (j = 0; j < BTN_NUM_COLS; j++) {
            /* again, we're going to do some unnecessary work in this loop,
             * but to understand why, first understand how the button_matrix
             * array is layed out.  Then, consider a 4x4 button matrix that has 
             * only one button defined in the (bottom of the) fourth column. 
             * Obviously, if we didn't loop for every column, we might never drive 
             * the row low/off.
             */
            t = &button_matrix[i][j];
            //cnxt_gpio_set_output_level(t->row_code, 1);
            /* allow some settling time */
            cnxt_gpio_get_level(t->column_code, &bInputLevel);
            cnxt_gpio_get_level(t->column_code, &bInputLevel);
            cnxt_gpio_get_level(t->column_code, &bInputLevel);
            cnxt_gpio_get_level(t->column_code, &bInputLevel);
            cnxt_gpio_get_level(t->column_code, &bInputLevel);

            /* now read the value */
            cnxt_gpio_get_level(t->column_code, &bInputLevel);
            
            if ( bInputLevel == 0 )//wlk
                button_mask |= t->button_code;     
        }
        
        t = &button_matrix[i][0];
        //cnxt_gpio_set_output_level(t->row_code, 0);//wlk
        cnxt_gpio_set_output_level(t->row_code,1);//wlk
    }
     
   return button_mask;
#else
   return 0;
#endif
}


/********************************************************************/
/*  FUNCTION:    cnxt_btn_led_number                                */
/*                                                                  */
/*  PARAMETERS:  VOID                                               */
/*                                                                  */
/*  DESCRIPTION: Enable capability for system reset when one        */
/*               button is held for a certain amount of time        */
/*                                                                  */
/*  RETURNS:     TRUE - capability is supported                     */
/*               FALSE - capability is not supported                */
/*                                                                  */
/********************************************************************/
			
bool cnxt_btn_led_number(u_int32 word, unsigned char pos, bool bHex,u_int8 TA) 
{
    char str_array[LED_NUMBER_COUNTER];
    int i;
  //  char strength_bar;
//    char quality_bar;

  /*  static unsigned char led_number_table[] =
    {
	    0x42, //			case 0:	Ls164_data = 0x42;
        0x7b, //			case 1:	Ls164_data = 0x7b;
	    0xc8, //			case 2:	Ls164_data = 0xc8;
	    0x68, //			case 3:	Ls164_data = 0x68;
	    0x31, //			case 4:	Ls164_data = 0x71;
        0x24, //	      	case 5:	Ls164_data = 0xb4;
	    0x04, //			case 6:	Ls164_data = 0x84;
	    0x7a, //			case 7:	Ls164_data = 0x82;
	    0x40, //			case 8:	Ls164_data = 0x80;
	    0x20, //			case 9:	Ls164_data = 0xa0;
	    0x90, //			case 10:Ls164_data = 0x90;
	    0x85, //			case 11:Ls164_data = 0x85;
	    0xc6, //			case 12:Ls164_data = 0xc6;
	    0x89, //			case 13:Ls164_data = 0x89;
	    0xc4, //			case 14:Ls164_data = 0xc4;
	    0xd4  //			case 15:Ls164_data = 0xd4;
    };*/
/*
  static unsigned char led_number_table[] =
    {
	    0xC0, //			case 0:	Ls164_data = 0x42;
        0xED, //			case 1:	Ls164_data = 0x7b;
	    0xB0, //			case 2:	Ls164_data = 0xc8;
	    0xA4, //			case 3:	Ls164_data = 0x68;
	    0x8D, //			case 4:	Ls164_data = 0x71;
        0x86, //	      	case 5:	Ls164_data = 0xb4;
	    0x82, //			case 6:	Ls164_data = 0x84;
	    0xEC, //			case 7:	Ls164_data = 0x82;
	    0x80, //			case 8:	Ls164_data = 0x80;
	    0x84, //			case 9:	Ls164_data = 0xa0;
	    0x90, //			case 10:Ls164_data = 0x90;
	    0x85, //			case 11:Ls164_data = 0x85;
	    0xc6, //			case 12:Ls164_data = 0xc6;
	    0x89, //			case 13:Ls164_data = 0x89;
	    0xc4, //			case 14:Ls164_data = 0xc4;
	    0xd4,  //			case 15:Ls164_data = 0xd4;
	    0xdf,//                    case 16:Ls164_data = 0xd4;
	    0xbf,//                    case 17: Ls164_data = 0xd4;
	    0x5f,//                     case 18:Ls164_data = 0xd4;
	    0xbf,//                    case 19:6<x<9 
	    0xb7,//                   case 20:3<x<6
	    0xf7,//                     case 21:0<x<3 add by wlk ,12-13-2004
	    
    };
*/    
//new add by wangyu 3/1/2005 10:39AM
  /*static unsigned char led_number_table[] =
    {
	    0xC0, //			case 0:	Ls164_data = 0x42;
        0xF9, //			case 1:	Ls164_data = 0x7b;
	    0xA4, //			case 2:	Ls164_data = 0xc8;
	    0xB0, //			case 3:	Ls164_data = 0x68;
	    0x99, //			case 4:	Ls164_data = 0x71;
        0x92, //	      	case 5:	Ls164_data = 0xb4;
	    0x82, //			case 6:	Ls164_data = 0x84;
	    0xF8, //			case 7:	Ls164_data = 0x82;
	    0x80, //			case 8:	Ls164_data = 0x80;
	    0x90, //			case 9:	Ls164_data = 0xa0;
	    0x90, //			case 10:Ls164_data = 0x90;
	    0x85, //			case 11:Ls164_data = 0x85;
	    0xc6, //			case 12:Ls164_data = 0xc6;
	    0x89, //			case 13:Ls164_data = 0x89;
	    0xc4, //			case 14:Ls164_data = 0xc4;
	    0xd4,  //			case 15:Ls164_data = 0xd4;
	    0xdf,//0xef,//                    case 16:Ls164_data = 0xd4;
	    0xef,//0xbf,//                    case 17: Ls164_data = 0xd4;
	    0x5f,//                     case 18:Ls164_data = 0xd4;
	    0xbf,//                    case 19:6<x<9 
	    0xb7,//                   case 20:3<x<6
	    0xf7,//                     case 21:0<x<3 add by wlk ,12-13-2004
	    
    };*/    
  static unsigned char led_number_table[] =
  	{ 
 
                0xC0,  //"0"
                0xF9,  //"1"
                0xA4,  //"2"
                0xB0,  //"3"
                0x99,  //"4"
                0x92,  //"5"
                0x82,  //"6"
                0xF8,  //"7"
                0x80,  //"8"
                0x90,  //"9"
                0x88,  //"A"
                0x83,  //"B"
                0xC6,  //"C"
                0xA1,  //"D"
                0x86,  //"E"
                0x8E,  //"F"
                0x89,  //"H"
                0xC7,  //"L"
                0xC8,  //"n"
                0xC1,  //"u"
                0x8C,  //"P"
                0xA3,  //"o"
                0xBF,  //"-"
                0xFF,  //熄灭
                0x00,  //全亮
                0xf8,  //"T"
                0xA3,//"0"
                0xC1, //"V"
                0xf9,//"I"
 
                         };

    if (pos&0xF0)
        return FALSE;
        
    if( word > 0xFFFF )
        return FALSE;
    
    if ( (word>9999) & (bHex==FALSE) )
        return FALSE;
        
    if (bHex==TRUE)
        sprintf(str_array, "%04X", (int)word);
    else
        sprintf(str_array, "%04d", (int)word);
   
       str_array[4]=word/1000;
	str_array[3]=word/100 - str_array[4]*10;
       str_array[2]=word/10-str_array[3]*10-str_array[4]*100;
	str_array[1]=word%10;

       //TA 0:TV and DATA SERVICE
       //TA 1:AUDIO SERVICE
       //TA 2:light up all the led "8.8.8.8."
       //TA 1:DATA SERVICE
	if(TA == 0x01)
         str_array[4]=10;//display "A"
       if(TA == 0x00)
          str_array[4]=27;//display "V"
       if(TA == 0x02)
         {
           str_array[1]=24;
           str_array[2]=24;
           str_array[3]=24;
           str_array[4]=24;
           
         }
       if(TA == 0x03)
       {
          str_array[4]=13;//display "D"
          str_array[3]=10;//display "A"
          str_array[2]=25;//display "T"
          str_array[1]=10;//display "A"
       }
       if(TA == 0x04)
       {
          str_array[4]=23;//display " "
          str_array[3]=27;//display "V"
          str_array[2]=26;//display "o"
          str_array[1]=13;//display "d"
       }
	   
       if(TA == 0x05)
       {
          str_array[4]=23;//display " "
          str_array[3]=28;//display "I "
          str_array[2]=20;//display "P"
          str_array[1]=23;//display " "
       }

       if(TA == 0x06)
       {
          str_array[4]=11;//display " "
       }
	   
	if(TA == 0x0B) //ftm
	{
		fp_led_data[0]=0x0;//8.
            	fp_led_data[1]=0x0;//8.
            	fp_led_data[2]=0x0;//8.
            	fp_led_data[3]=0x0;//8.	  
            	return ;
	}	 
	

        /*if90*/
	
	if(power_state==0x01)
		 {
		     str_array[0]=16;	
		 }
		if(standby_state==0x01)
		 {
		     str_array[0]=17;	
		 }
		if(power_state==0x01&&lock_state==0x01)
		  {
		     str_array[0]=18;	
		  }
		/*add by wlk ,12-13-2004*/
	/*if(strength_quality_state)
		{strength_bar=str_array[4]/3;
	         switch (strength_bar) 
                   {
                   case 0:
                   	str_array[3]=21;
                   	break;
                   case 1:
                	str_array[3]=21;
                	break;
                   case 2:
                	str_array[3]=20;
                	break;
                   case 3:
                	str_array[3]=19;
                	break;
	         	}
                quality_bar=str_array[2]/3;
                switch (quality_bar)
                   {
                   case 0:
                   	str_array[1]=21;
                   	break;
                   case 1:
                   	str_array[1]=21;
                   	break;
                   case 2:
                   	str_array[1]=20;
                   	break;
                   case 3:
                   	str_array[1]=19;
                   	break;
                	}
             

	       }*/
if(standby_state==0)
   {	
    for(i=0; i<LED_NUMBER_COUNTER; i++)
    {
        if ((str_array[i]>='A')&&(str_array[i]<='F'))
            str_array[i] = str_array[i] - 'A' + 10;
        else
        //    str_array[i] = str_array[i] - '0';
        
        str_array[i] = led_number_table[str_array[i]];    
        /*delete by wlk, let standby led bright, 12-13-2004*/
        /*if (pos&(1<<i))
            str_array[i] = str_array[i] & ~(1<<6);*/
            
        fp_led_data[LED_NUMBER_COUNTER-i-1] = str_array[i];
    }    
}
    return FALSE;
}


/********************************************************************/
/*  FUNCTION:    cnxt_btn_pio_reset_enable                          */
/*                                                                  */
/*  PARAMETERS:  VOID                                               */
/*                                                                  */
/*  DESCRIPTION: Enable capability for system reset when one        */
/*               button is held for a certain amount of time        */
/*                                                                  */
/*  RETURNS:     TRUE - capability is supported                     */
/*               FALSE - capability is not supported                */
/*                                                                  */
/********************************************************************/
bool cnxt_btn_pio_reset_enable() {
#if (CHIP_SUPPORTS_PIO_TIMED_RESET == YES)
    CNXT_SET(RST_PUSHBTN_RESET_CTL_REG,
             RST_PUSHBTN_RESET_CTL_ENABLE_MASK,
             RST_PUSHBTN_RESET_CTL_ENABLE);
    return TRUE;
#else
    return FALSE;
#endif
}

/********************************************************************/
/*  FUNCTION:    cnxt_btn_pio_reset_disable                         */
/*                                                                  */
/*  PARAMETERS:  VOID                                               */
/*                                                                  */
/*  DESCRIPTION: disables hold-button reset capability              */
/*                                                                  */
/*  RETURNS:     VOID                                               */
/*                                                                  */
/********************************************************************/
void cnxt_btn_pio_reset_disable() {
#if (CHIP_SUPPORTS_PIO_TIMED_RESET == YES)
    CNXT_SET(RST_PUSHBTN_RESET_CTL_REG,
             RST_PUSHBTN_RESET_CTL_ENABLE_MASK,
             RST_PUSHBTN_RESET_CTL_DISABLE);
#endif
}

/********************************************************************/
/*  FUNCTION:    cnxt_btn_pio_reset_set_timeout                     */
/*                                                                  */
/*  PARAMETERS:  ms - number of milliseconds button should be held  */
/*                    to force reset.                               */
/*                                                                  */
/*  DESCRIPTION: change the timeout for the one button reset        */
/*               capability                                         */
/*                                                                  */
/*  RETURNS:     TRUE - capability is supported                     */
/*               FALSE - capability is not supported                */
/*                                                                  */
/********************************************************************/
bool cnxt_btn_pio_reset_set_timeout(int ms) {
#if (CHIP_SUPPORTS_PIO_TIMED_RESET == YES)
    int time;
    /* Debounce reg counts in units of 4.7 uS */
    time = ms * 10000 / 47;
    CNXT_SET_VAL(RST_PUSHBTN_RESET_CTL_REG,
		 RST_PUSHBTN_RESET_CTL_DEBOUNCE_MASK,
		 time);
    return TRUE;
#else
    return FALSE;
#endif
}

/********************************************************************/
/*  FUNCTION:    cnxt_btn_pio_reset_set_polarity                    */
/*                                                                  */
/*  PARAMETERS:  pol - 0 will make a negative polarity signal reset */
/*                     the system.  ie. When the signal is at       */
/*                     ground level.  Non-0 will make a positive    */
/*                     polarity reset the system. ie. When the      */
/*                     signal is held a Vcc.                        */
/*                                                                  */
/*  DESCRIPTION: Change the polarity of the signal which generates  */
/*               a reset                                            */
/*                                                                  */
/*  RETURNS:     TRUE - capability is supported                     */
/*               FALSE - capability is not supported                */
/*                                                                  */
/********************************************************************/
bool cnxt_btn_pio_reset_set_polarity(int pol) {
#if (CHIP_SUPPORTS_PIO_TIMED_RESET == YES)
    CNXT_SET(
             RST_PUSHBTN_RESET_CTL_REG,
             RST_PUSHBTN_RESET_CTL_POLARITY_MASK,
             (pol ? 
                    RST_PUSHBTN_RESET_CTL_ACTIVE_HIGH :
                    RST_PUSHBTN_RESET_CTL_ACTIVE_LOW
             )
            );

    return TRUE;
#else
    return FALSE;
#endif
}

/*****************************************************************************
 * panel LED display	
 *****************************************************************************/

//    a  
//  =====  
//f||   ||b
// || g ||
//  ===== 
//e||   ||c
// ||   ||
//  =====
//    d
#define PANEL_SEG_A     0x01
#define PANEL_SEG_B     0x02
#define PANEL_SEG_C     0x04
#define PANEL_SEG_D     0x08
#define PANEL_SEG_E     0x10
#define PANEL_SEG_F     0x20
#define PANEL_SEG_G     0x40
#define PANEL_SEG_DOT   0x80

#define PANEL_CHAR_NULL 0x00

#define PANEL_CHAR_0    (PANEL_SEG_A | PANEL_SEG_B | PANEL_SEG_C | PANEL_SEG_D | PANEL_SEG_E | PANEL_SEG_F)
#define PANEL_CHAR_1    (PANEL_SEG_B | PANEL_SEG_C)
#define PANEL_CHAR_2    (PANEL_SEG_A | PANEL_SEG_B | PANEL_SEG_G | PANEL_SEG_E | PANEL_SEG_D)
#define PANEL_CHAR_3    (PANEL_SEG_A | PANEL_SEG_B | PANEL_SEG_G | PANEL_SEG_C | PANEL_SEG_D)
#define PANEL_CHAR_4    (PANEL_SEG_F | PANEL_SEG_G | PANEL_SEG_B | PANEL_SEG_C)
#define PANEL_CHAR_5    (PANEL_SEG_A | PANEL_SEG_F | PANEL_SEG_G | PANEL_SEG_C | PANEL_SEG_D)
#define PANEL_CHAR_6    (PANEL_SEG_A | PANEL_SEG_F | PANEL_SEG_E | PANEL_SEG_G | PANEL_SEG_C | PANEL_SEG_D)
#define PANEL_CHAR_7    (PANEL_SEG_A | PANEL_SEG_B | PANEL_SEG_C)
#define PANEL_CHAR_8    (PANEL_SEG_A | PANEL_SEG_B | PANEL_SEG_C | PANEL_SEG_D | PANEL_SEG_E | PANEL_SEG_F | PANEL_SEG_G)
#define PANEL_CHAR_9    (PANEL_SEG_A | PANEL_SEG_F | PANEL_SEG_G | PANEL_SEG_B | PANEL_SEG_C | PANEL_SEG_D)

#define PANEL_CHAR_A    (PANEL_SEG_A | PANEL_SEG_B | PANEL_SEG_C | PANEL_SEG_E | PANEL_SEG_F | PANEL_SEG_G)
#define PANEL_CHAR_a    (PANEL_SEG_A | PANEL_SEG_B | PANEL_SEG_C | PANEL_SEG_D | PANEL_SEG_E | PANEL_SEG_G)

#define PANEL_CHAR_B    (PANEL_SEG_C | PANEL_SEG_D | PANEL_SEG_E | PANEL_SEG_F | PANEL_SEG_G)
#define PANEL_CHAR_b    (PANEL_SEG_C | PANEL_SEG_D | PANEL_SEG_E | PANEL_SEG_F | PANEL_SEG_G)

#define PANEL_CHAR_C    (PANEL_SEG_A | PANEL_SEG_D | PANEL_SEG_E | PANEL_SEG_F)
#define PANEL_CHAR_c    (PANEL_SEG_D | PANEL_SEG_E | PANEL_SEG_G)

#define PANEL_CHAR_D    (PANEL_SEG_B | PANEL_SEG_C | PANEL_SEG_D | PANEL_SEG_E | PANEL_SEG_G)
#define PANEL_CHAR_d    (PANEL_SEG_B | PANEL_SEG_C | PANEL_SEG_D | PANEL_SEG_E | PANEL_SEG_G)

#define PANEL_CHAR_E    (PANEL_SEG_A | PANEL_SEG_D | PANEL_SEG_E | PANEL_SEG_F | PANEL_SEG_G)
#define PANEL_CHAR_e    (PANEL_SEG_A | PANEL_SEG_D | PANEL_SEG_E | PANEL_SEG_F | PANEL_SEG_G)

#define PANEL_CHAR_F    (PANEL_SEG_A | PANEL_SEG_E | PANEL_SEG_F | PANEL_SEG_G)
#define PANEL_CHAR_f    (PANEL_SEG_A | PANEL_SEG_E | PANEL_SEG_F | PANEL_SEG_G)

#define PANEL_CHAR_G    (PANEL_SEG_A | PANEL_SEG_F | PANEL_SEG_G | PANEL_SEG_B | PANEL_SEG_C | PANEL_SEG_D)
#define PANEL_CHAR_g    (PANEL_SEG_A | PANEL_SEG_F | PANEL_SEG_G | PANEL_SEG_B | PANEL_SEG_C | PANEL_SEG_D)

#define PANEL_CHAR_H    (PANEL_SEG_B | PANEL_SEG_C | PANEL_SEG_E | PANEL_SEG_F | PANEL_SEG_G)
#define PANEL_CHAR_h    (PANEL_SEG_C | PANEL_SEG_E | PANEL_SEG_F | PANEL_SEG_G)

#define PANEL_CHAR_I    (PANEL_SEG_E | PANEL_SEG_F)
#define PANEL_CHAR_i    (PANEL_SEG_E | PANEL_SEG_F)

#define PANEL_CHAR_J    (PANEL_SEG_A | PANEL_SEG_B | PANEL_SEG_C | PANEL_SEG_D)
#define PANEL_CHAR_j    (PANEL_SEG_B | PANEL_SEG_C | PANEL_SEG_D)

#define PANEL_CHAR_K    (PANEL_CHAR_NULL)
#define PANEL_CHAR_k    (PANEL_CHAR_NULL)

#define PANEL_CHAR_L    (PANEL_SEG_D | PANEL_SEG_E | PANEL_SEG_F)
#define PANEL_CHAR_l    (PANEL_SEG_D | PANEL_SEG_E | PANEL_SEG_F)

#define PANEL_CHAR_M    (PANEL_CHAR_NULL)
#define PANEL_CHAR_m    (PANEL_CHAR_NULL)

#define PANEL_CHAR_N    (PANEL_SEG_A | PANEL_SEG_B | PANEL_SEG_C | PANEL_SEG_E | PANEL_SEG_F)
#define PANEL_CHAR_n    (PANEL_SEG_C | PANEL_SEG_E | PANEL_SEG_G)

#define PANEL_CHAR_O    (PANEL_SEG_A | PANEL_SEG_B | PANEL_SEG_C | PANEL_SEG_D | PANEL_SEG_E | PANEL_SEG_F)
#define PANEL_CHAR_o    (PANEL_SEG_C | PANEL_SEG_D | PANEL_SEG_E | PANEL_SEG_G)

#define PANEL_CHAR_P    (PANEL_SEG_A | PANEL_SEG_B | PANEL_SEG_E | PANEL_SEG_F | PANEL_SEG_G)
#define PANEL_CHAR_p    (PANEL_SEG_A | PANEL_SEG_B | PANEL_SEG_E | PANEL_SEG_F | PANEL_SEG_G)

#define PANEL_CHAR_Q    (PANEL_SEG_A | PANEL_SEG_B | PANEL_SEG_C | PANEL_SEG_F | PANEL_SEG_G)
#define PANEL_CHAR_q    (PANEL_SEG_A | PANEL_SEG_B | PANEL_SEG_C | PANEL_SEG_F | PANEL_SEG_G)

#define PANEL_CHAR_R    (PANEL_SEG_E | PANEL_SEG_G)
#define PANEL_CHAR_r    (PANEL_SEG_E | PANEL_SEG_G)

#define PANEL_CHAR_S    (PANEL_SEG_A | PANEL_SEG_C | PANEL_SEG_D | PANEL_SEG_F | PANEL_SEG_G)
#define PANEL_CHAR_s    (PANEL_SEG_A | PANEL_SEG_C | PANEL_SEG_D | PANEL_SEG_F | PANEL_SEG_G)

#define PANEL_CHAR_T    (PANEL_SEG_A | PANEL_SEG_E | PANEL_SEG_F)
#define PANEL_CHAR_t    (PANEL_SEG_A | PANEL_SEG_E | PANEL_SEG_F)

#define PANEL_CHAR_U    (PANEL_SEG_B | PANEL_SEG_C | PANEL_SEG_D | PANEL_SEG_E | PANEL_SEG_F)
#define PANEL_CHAR_u    (PANEL_SEG_C | PANEL_SEG_D | PANEL_SEG_E)

#define PANEL_CHAR_V    (PANEL_SEG_B | PANEL_SEG_C | PANEL_SEG_D | PANEL_SEG_E | PANEL_SEG_F)
#define PANEL_CHAR_v    (PANEL_SEG_C | PANEL_SEG_D | PANEL_SEG_E)

#define PANEL_CHAR_W    (PANEL_CHAR_NULL)
#define PANEL_CHAR_w    (PANEL_CHAR_NULL)

#define PANEL_CHAR_X    (PANEL_CHAR_NULL)
#define PANEL_CHAR_x    (PANEL_CHAR_NULL)

#define PANEL_CHAR_Y    (PANEL_SEG_B | PANEL_SEG_C | PANEL_SEG_D |PANEL_SEG_F | PANEL_SEG_G)
#define PANEL_CHAR_y    (PANEL_SEG_B | PANEL_SEG_C | PANEL_SEG_D |PANEL_SEG_F | PANEL_SEG_G)

#define PANEL_CHAR_Z    (PANEL_CHAR_NULL)
#define PANEL_CHAR_z    (PANEL_CHAR_NULL)

#define PANEL_CHAR_DASH (PANEL_SEG_G)

static unsigned char char_code[] = 
{
    /* ' ' */
    PANEL_CHAR_NULL,
    
    /* '-' */
    PANEL_CHAR_DASH,
    
    /* 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 */
    PANEL_CHAR_0,PANEL_CHAR_1,PANEL_CHAR_2,PANEL_CHAR_3,PANEL_CHAR_4,PANEL_CHAR_5,PANEL_CHAR_6,PANEL_CHAR_7,PANEL_CHAR_8,PANEL_CHAR_9,
    
    /* A, B, C, D, E, F, G, H, I, J */
    PANEL_CHAR_A,PANEL_CHAR_B,PANEL_CHAR_C,PANEL_CHAR_D,PANEL_CHAR_E,PANEL_CHAR_F,PANEL_CHAR_G,PANEL_CHAR_H,PANEL_CHAR_I,PANEL_CHAR_J,
    
    /* K, L, M, N, O, P, Q, R, S, T */
    PANEL_CHAR_K,PANEL_CHAR_L,PANEL_CHAR_M,PANEL_CHAR_N,PANEL_CHAR_O,PANEL_CHAR_P,PANEL_CHAR_Q,PANEL_CHAR_R,PANEL_CHAR_S,PANEL_CHAR_T,
    
    /* U, V, W, X, Y, Z */
    PANEL_CHAR_U,PANEL_CHAR_V,PANEL_CHAR_W,PANEL_CHAR_X,PANEL_CHAR_Y,PANEL_CHAR_Z,
    
    /* a, b, c, d, e, f, g, h, i, j */
    PANEL_CHAR_a,PANEL_CHAR_b,PANEL_CHAR_c,PANEL_CHAR_d,PANEL_CHAR_e,PANEL_CHAR_f,PANEL_CHAR_g,PANEL_CHAR_h,PANEL_CHAR_i,PANEL_CHAR_j,
    
    /* k, l, m, n, o, p, q, r, s, t */
    PANEL_CHAR_k,PANEL_CHAR_l,PANEL_CHAR_m,PANEL_CHAR_n,PANEL_CHAR_o,PANEL_CHAR_p,PANEL_CHAR_q,PANEL_CHAR_r,PANEL_CHAR_s,PANEL_CHAR_t,
    
    /* u, v, w, x, y, z */
    PANEL_CHAR_u,PANEL_CHAR_v,PANEL_CHAR_w,PANEL_CHAR_x,PANEL_CHAR_y,PANEL_CHAR_z
};

static unsigned char panel_encode_char(unsigned char ch)
{
    unsigned char i = 0;

    if(ch >= 0x20 && ch <= 0x7f)
    {
        if(ch == 0x2d)
        {
            i = 1;
        }
        else
        {            
            if(ch >= 0x30 && ch <= 0x39)
                i = 2 + ch - 0x30;

            if(ch >= 0x41 && ch <= 0x5a)
                i = 2 + 10 + ch - 0x41;

            if(ch >= 0x61 && ch <= 0x7a)
                i = 2 + 10 + 26 + ch - 0x61;
        }
    }

    return char_code[i];
}

int panel_pio_display_string(unsigned char string[4])
{
	int i,j=0;
	unsigned char tmp_data =0x00;
	unsigned char disp_data=0x00;

	/* 使用0-3基数,4为其它所用 */
	for(i=0; i<LED_NUMBER_COUNTER-1; i++)
	{
		tmp_data = *(unsigned char*)(string + j);
		disp_data = panel_encode_char(tmp_data);
		fp_led_data[i] = (~disp_data)&0xff;

		j++;
	}

	return 0;
}

int panel_pio_led_set(u_int32 num, bool bOn)
{
   if (num > 7)
       return -1;
   
   if (!bOn)
      fp_led_data[4] |= (1<<num);  
   else
      fp_led_data[4] &= ~(1<<num);  
      
   return 0;
}

/****************************************************************************
 * Modifications:
 * $Log: 
 *  23   mpeg      1.22        7/22/03 12:11:16 PM    Bobby Bradford  SCR(s) 
 *        7014 :
 *        Add HEADER and LOG keywords to the file ...
 *        Moved the 4s reset functions outside the RTOS conditional so that
 *        they can be accessed from CODELDR
 *        
 *  22   mpeg      1.21        7/10/03 2:16:52 PM     Larry Wang      SCR(s) 
 *        6924 :
 *        Restore to rev 1.19
 *        
 *  21   mpeg      1.20        7/10/03 10:28:06 AM    Larry Wang      SCR(s) 
 *        6924 :
 *        make CODELDR build also includes the case where RTOS==VXWORKS && 
 *        APPNAME==CODELDR.
 *        
 *  20   mpeg      1.19        7/9/03 3:28:52 PM      Tim White       SCR(s) 
 *        6901 :
 *        Phase 3 codeldrext drop.
 *        
 *        
 *  19   mpeg      1.18        2/13/03 12:29:04 PM    Matt Korte      SCR(s) 
 *        5479 :
 *        Removed old header reference
 *        
 *  18   mpeg      1.17        2/10/03 11:54:30 AM    Brendan Donahe  SCR(s) 
 *        5456 :
 *        Replaced CNXT_SET with CNXT_SET_VAL so that value is shifted based on
 *         mask
 *        for reset on button hold timer value.
 *        
 *        
 *  17   mpeg      1.16        1/24/03 12:34:54 PM    Bobby Bradford  SCR(s) 
 *        5312 :
 *        Fixed the query_row_state function to set the row to the OFF
 *        condition (like it had done before) ... for ROWS that are on
 *        extended GPIO, this will do nothing, but for ROWS on internal
 *        GPIO, this will tri-state the pin (and allow the function
 *        to report multiple buttons pressed correctly)
 *        
 *  16   mpeg      1.15        1/23/03 1:06:24 PM     Bobby Bradford  SCR(s) 
 *        5275 :
 *        Modified the scan matrix processing to use the General GPIO
 *        definitions (from GPIO.H/C)
 *        
 *  15   mpeg      1.14        1/17/03 5:22:56 PM     Miles Bintz     SCR(s) 
 *        5089 :
 *        added functions to support PIO timed reset button
 *        
 *        
 *  14   mpeg      1.13        12/19/02 3:08:42 PM    Tim White       SCR(s) 
 *        5068 :
 *        There's no scan buttons on emulation.  Allow this module to build and
 *         "work" when
 *        keypad is set to NONE.
 *        
 *        
 *  13   mpeg      1.12        9/6/02 4:42:08 PM      Bobby Bradford  SCR(s) 
 *        4552 :
 *        At line 354, changed an comparison "<=" to "==", because it was
 *        an unsigned comparison to 0, and would never be less than 0.  The
 *        compiler generated a warning for this.
 *        
 *  12   mpeg      1.11        7/17/02 6:51:24 PM     Larry Wang      SCR(s) 
 *        4221 :
 *        Stop debounce timer when we detect a button release.
 *        
 *  11   mpeg      1.10        5/17/02 12:13:54 PM    Joe Kroesche    SCR(s) 
 *        3811 :
 *        replaced hwtimer usage with tick timer usage
 *        
 *  10   mpeg      1.9         4/30/02 3:29:30 PM     Billy Jackman   SCR(s) 
 *        3661 :
 *        Removed unused local variables to prevent compiler warnings.
 *        
 *  9    mpeg      1.8         4/1/02 8:58:20 PM      Miles Bintz     SCR(s) 
 *        3469 :
 *        New scan button driver architecture
 *        
 *  8    mpeg      1.7         11/26/01 4:56:54 PM    Bobby Bradford  SCR(s) 
 *        2921 :
 *        Added support (it's a HACK) for ATHENS and ZAPATA keypad types.  The 
 *        proper
 *        solution will take a bit more design work, but this will get the 
 *        chip-evaluation
 *        going.
 *        
 *  7    mpeg      1.6         7/3/01 11:05:20 AM     Tim White       SCR(s) 
 *        2178 2179 2180 :
 *        Merge branched Hondo specific code back into the mainstream source 
 *        database.
 *        
 *        
 *  6    mpeg      1.5         4/12/01 4:48:34 PM     Amy Pratt       DCS914 
 *        Removed support for Neches.
 *        
 *  5    mpeg      1.4         9/28/00 5:59:22 PM     Dave Wilson     Swapped 
 *        INFO and BACKUP to agree with vendor A's new board
 *        
 *  4    mpeg      1.3         6/14/00 4:06:42 PM     Tim White       Added 
 *        500ns delay in button scan function for all boards.
 *        Added VENDOR_C specific button information.
 *        
 *  3    mpeg      1.2         4/26/00 1:19:04 AM     Dave Wilson     Added 
 *        Colorado RevA workaround for accesses to GPIO_DRIVE_OFF_REG
 *        
 *  2    mpeg      1.1         4/17/00 6:28:54 PM     Dave Wilson     Driver 
 *        now working but still requires some code to guard against phantom
 *        keypresses when 3 or moer keys are pressed simultaneously.
 *        
 *  1    mpeg      1.0         4/7/00 2:42:48 PM      Dave Wilson     
 * $
 * 
 *    Rev 1.22   22 Jul 2003 11:11:16   bradforw
 * SCR(s) 7014 :
 * Add HEADER and LOG keywords to the file ...
 * Moved the 4s reset functions outside the RTOS conditional so that
 * they can be accessed from CODELDR
 * 
 *    Rev 1.21  10 Jul 2003 13:16:52   wangl2
 * SCR(s) 6924 :
 * Restore to rev 1.19
 *
 *    Rev 1.20  10 Jul 2003 09:28:06   wangl2     
 * SCR(s) 6924 :
 * make CODELDR build also includes the case where RTOS==VXWORKS && APPNAME==CODELDR.
 *
 *    Rev 1.19  09 Jul 2003 14:28:52   whiteth     
 * SCR(s) 6901 :
 * Phase 3 codeldrext drop.
 * 
 *
 *    Rev 1.18  13 Feb 2003 12:29:04   kortemw     
 * SCR(s) 5479 :
 * Removed old header reference
 *
 *    Rev 1.17  10 Feb 2003 11:54:30   donaheb     
 * SCR(s) 5456 :
 * Replaced CNXT_SET with CNXT_SET_VAL so that value is shifted based on mask
 * for reset on button hold timer value.
 * 
 *
 *    Rev 1.16  24 Jan 2003 12:34:54   bradforw     
 * SCR(s) 5312 :
 * Fixed the query_row_state function to set the row to the OFF
 * condition (like it had done before) ... for ROWS that are on
 * extended GPIO, this will do nothing, but for ROWS on internal
 * GPIO, this will tri-state the pin (and allow the function
 * to report multiple buttons pressed correctly)
 *
 *    Rev 1.15  23 Jan 2003 13:06:24   bradforw     
 * SCR(s) 5275 :
 * Modified the scan matrix processing to use the General GPIO
 * definitions (from GPIO.H/C)
 *
 *    Rev 1.14  17 Jan 2003 17:22:56   bintzmf     
 * SCR(s) 5089 :
 * added functions to support PIO timed reset button
 * 
 *
 *    Rev 1.13  19 Dec 2002 15:08:42   whiteth     
 * SCR(s) 5068 :
 * There's no scan buttons on emulation.  Allow this module to build and "work" when
 * keypad is set to NONE.
 * 
 *
 *    Rev 1.12  06 Sep 2002 15:42:08   bradforw     
 * SCR(s) 4552 :
 * At line 354, changed an comparison "<=" to "==", because it was
 * an unsigned comparison to 0, and would never be less than 0.  The
 * compiler generated a warning for this.
 *
 *    Rev 1.11  17 Jul 2002 17:51:24   wangl2     
 * SCR(s) 4221 :
 * Stop debounce timer when we detect a button release.
 *
 *    Rev 1.10  17 May 2002 11:13:54   kroescjl     
 * SCR(s) 3811 :
 * replaced hwtimer usage with tick timer usage
 *
 *    Rev 1.9   30 Apr 2002 14:29:30   jackmaw     
 * SCR(s) 3661 :
 * Removed unused local variables to prevent compiler warnings.
 *
 *    Rev 1.8   01 Apr 2002 20:58:20   bintzmf     
 * SCR(s) 3469 :
 * New scan button driver architecture
 *
 *    Rev 1.7   26 Nov 2001 16:56:54   bradforw     
 * SCR(s) 2921 :
 * Added support (it's a HACK) for ATHENS and ZAPATA keypad types.  The proper
 * solution will take a bit more design work, but this will get the chip-evaluation
 * going.
 *
 *    Rev 1.6   03 Jul 2001 10:05:20   whiteth     
 * SCR(s) 2178 2179 2180 :
 * Merge branched Hondo specific code back into the mainstream source database.
 * 
 *
 *    Rev 1.5   12 Apr 2001 15:48:34   prattac     
 * Branches:  1.5.1
 * DCS914 Removed support for Neches.
 *
 *        Rev 1.5.1.0  May 2001 16:50:18  id: 
 *     Added support for Abilene front panel button mapping.
 *
 *    Rev 1.4   28 Sep 2000 16:59:22   dawilson     
 * Swapped INFO and BACKUP to agree with vendor A's new board
 *
 *    Rev 1.3   14 Jun 2000 15:06:42   whiteth     
 * Added 500ns delay in button scan function for all boards.
 * Added VENDOR_C specific button information.
 *
 *    Rev 1.2   26 Apr 2000 00:19:04   dawilson     
 * Added Colorado RevA workaround for accesses to GPIO_DRIVE_OFF_REG
 *
 *    Rev 1.1   17 Apr 2000 17:28:54   dawilson     
 * Driver now working but still requires some code to guard against phantom
 * keypresses when 3 or moer keys are pressed simultaneously.
 *
 *    Rev 1.0   07 Apr 2000 13:42:48   dawilson     
 * Initial revision.
 * 
 ****************************************************************************/
