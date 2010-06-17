/****************************************************************************/
/*                          Conexant Systems Inc.                           */
/****************************************************************************/
/*                                                                          */
/* Filename:       SCANBTNS.C                                               */
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
#define  DEBOUNCE_TIMEOUT  6 /* milliseconds */
#define  BTN_NUM_ROWS      FRONT_PANEL_KEYPAD_NUM_ROWS
#define  BTN_NUM_COLS      FRONT_PANEL_KEYPAD_NUM_COLS

/*****************************/
/*        Externs            */
/*****************************/
#if EMULATION_LEVEL == FINAL_HARDWARE
extern button_data button_matrix[BTN_NUM_ROWS][BTN_NUM_COLS];
#endif

#if RTOS != NOOS
/*****************************/
/* Local Function Prototypes */
/*****************************/
static int  button_int_handler(u_int32 dwIntID, bool bFIQ, void *pfnChain);
static void button_timer_handler(timer_id_t timer, void *userData);
static bool set_matrix_for_detect(void);
       u_int32 query_row_state(int iRow);
static bool scan_button_is_pressed(u_int32 key_code);

/***************/
/* Global Data */
/***************/
PFNISR  pfnChain[BTN_NUM_COLS];
static tick_id_t  timButtons;
static bool bTimerRunning;
static bool bInitialised = FALSE;
static PFNBUTTONCALLBACK pfnBtnCallback = (PFNBUTTONCALLBACK)NULL;
static u_int32 button_states;
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
bool button_init(PFNBUTTONCALLBACK pfnCallback, lpbutton_ftable lpFuncs)
{
#if EMULATION_LEVEL == FINAL_HARDWARE
   int  iRetcode;
   int  i,j;
   int  isr_registered[BTN_NUM_COLS];
   button_data *t;
   
   /* clear out this array */
   for (i=0; i < BTN_NUM_COLS; i++) isr_registered[i] = 0;

   
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
     /* Grab the interrupts we need for the columns */
     for (i = 0; i < BTN_NUM_ROWS; i++) {
         for (j = 0; j < BTN_NUM_COLS; j++) {
             /* we have to be careful not to register a column int
              * over and over for each row.  However, we need to 
              * go through every row to make sure we find every column.
              */
             t = &button_matrix[i][j];
             if ( (t->column_code != 0) && (isr_registered[j] == 0) ) {
                 isr_registered[j] =  t->column_code;
                 cnxt_gpio_int_register_isr( t->column_code,
                                   button_int_handler,
                                   FALSE,
                                   FALSE,
                                   &pfnChain[j]);
             }
         }
     }
                 
     /* set all buttons to up position */
     button_states = 0x0;
     
     /* Grab a timer and set its period to the debounce timeout */
     timButtons = tick_create(button_timer_handler, 0, "BTNT");
     if(timButtons == 0)
     {
        error_log(ERROR_FATAL | RC_KEY_TIMERCREATE);
        return(FALSE); /* Should never be reached */
     }
  
     iRetcode = tick_set(timButtons, DEBOUNCE_TIMEOUT, TRUE);
     if(iRetcode != RC_OK)
     {
        error_log(ERROR_FATAL | RC_KEY_TIMERSET);
        return(FALSE); /* Should never be reached */
     }
   
     bInitialised = set_matrix_for_detect();
     
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
/*  FUNCTION:    button_is_pressed                                  */
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
#if EMULATION_LEVEL == FINAL_HARDWARE
  int i,j;
  int iRetcode;
  int iColNum = -1;
  u_int32 temp;

  #ifdef DEBUG
  if (bTimerRunning) {
    isr_trace_new(TRACE_KEY | TRACE_LEVEL_ALWAYS,
                  "Button interrupt while debounce timer is running!\n", 0, 0);
    isr_error_log(ERROR_WARNING | RC_KEY_ISR);
  }  
  #endif
    
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
      
      /* We found a valid column so start the debounce timer */
      iRetcode = tick_start(timButtons);
      if(iRetcode != RC_OK) {
          isr_error_log(ERROR_WARNING | RC_KEY_TIMERSTART);
      } else {
          bTimerRunning = TRUE;
      }
  
      *(PFNISR *)pfnCh = pfnChain[iColNum];
      return(RC_ISR_HANDLED);
  } else {
      /* Error case - our vector was called for an interrupt we didn't */
      /* register!                                                     */
      isr_error_log(ERROR_WARNING | RC_KEY_ISR);
      *(PFNISR *)pfnCh = pfnChain[0];
      return(RC_ISR_NOTHANDLED);
  }
#else
  return(RC_ISR_NOTHANDLED);
#endif
}

/*************************************/
/* Handler for button debounce timer */
/*************************************/
static void button_timer_handler(timer_id_t timer, void *userData)
{
#if EMULATION_LEVEL == FINAL_HARDWARE
  int i;
  static u_int32 lastState[BTN_NUM_ROWS];
  static int inited = 0;
  u_int32 dwState;
  
  if (!inited) {
     for (i = 0; i < BTN_NUM_ROWS; i++) lastState[i] = 0;
     inited = 1;
  }
  
  /* Make sure this is our timer */
  if(timer == timButtons) {
    bTimerRunning= FALSE;
    button_states = 0;    
    /* Read the state of each button and send a message to the caller */
    /* if state has changed.                                          */
    for (i = 0; i < BTN_NUM_ROWS; i++) {
      /* dwState contains the bitmask of which buttons 
       * are pressed in the form of BTN_CODE.  If all
       * buttons have been released then dwState will
       * be zero for every row. 
       */
      dwState = query_row_state(i);
      if (dwState != lastState[i]) {
          /* if button has been released, stop debounce timer */
          if ( ( ( (dwState ^ lastState[i]) & dwState) == 0 ) )
          {
             tick_stop ( timButtons );
          }
          /* Button state has changed. Send a message */
          pfnBtnCallback(dwState ^ lastState[i], ( ( (dwState ^ lastState[i]) & dwState) > 0 ) );
      }
      lastState[i] = dwState;
      button_states |= lastState[i];
    }
    /* Reset the scan matrix state to wait for the next press or release */
    set_matrix_for_detect();
  }
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
           cnxt_gpio_set_output_level(t->row_code, FALSE);
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
           cnxt_gpio_set_output_level(t->row_code, TRUE);
           cnxt_gpio_set_input(t->row_code);
       }
   }
   
   for (j = 0; j < BTN_NUM_COLS; j++) {
       /* again, we're going to do some unnecessary work in this loop,
        * but to understand why, first understand how the button_matrix
        * array is layed out.  Then, consider a 4x4 button matrix that has 
        * only one button defined in the (bottom of the) fourth column. 
        * Obviously, if we didn't loop for every column, we might never drive 
        * the row low/off.
        */

       t = &button_matrix[iRow][j];
       cnxt_gpio_set_output_level(t->row_code, FALSE);

       /* allow some settling time */
       cnxt_gpio_get_level(t->column_code, &bInputLevel);
       cnxt_gpio_get_level(t->column_code, &bInputLevel);
       cnxt_gpio_get_level(t->column_code, &bInputLevel);
       cnxt_gpio_get_level(t->column_code, &bInputLevel);
       cnxt_gpio_get_level(t->column_code, &bInputLevel);

       /* now read the value */
//       button_mask |= ( get_gpio_input(t->col_internal_gpio_num) ? (t->button_code) : 0);
       cnxt_gpio_get_level(t->column_code, &bInputLevel);
       if ( !bInputLevel ) {
               button_mask |= t->button_code;
       }
   } 
   return button_mask;
#else
   return 0;
#endif
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
