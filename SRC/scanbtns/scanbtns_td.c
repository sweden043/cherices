/****************************************************************************/
/*                          Conexant Systems Inc.                           */
/****************************************************************************/
/*                                                                          */
/* Filename:       SCANBTNS.C                                               */
/*                                                                          */
/* Description:    Generic front panel button driver for scan matrix        */
/*                 implementation used on Tongda IRD                        */
/*                                                                          */
/* Author:         Shen Wei Wang                                            */
/*                                                                          */
/* Copyright Conexant Systems Inc, 2000                                     */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/

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

#define DEBOUNCE_TIMEOUT  3 /* milliseconds */

#define LED_NUMBER_COUNTER 3
#define KEY_DETECT_CODE     PIO_FRONT_PANEL_KEYPAD_COL_0

#define LED_CTRL_CODE_B1    PIO_FRONT_PANEL_KEYPAD_ROW_0
#define LED_CTRL_CODE_B2    PIO_FRONT_PANEL_KEYPAD_ROW_1
#define LED_CTRL_CODE_B3    PIO_FRONT_PANEL_KEYPAD_ROW_2

#define  BTN_NUM_ROWS     FRONT_PANEL_KEYPAD_NUM_ROWS
#define  BTN_NUM_COLS     FRONT_PANEL_KEYPAD_NUM_COLS

#define F_DATA_PIO_NUM  PIO_FRONT_PANEL_KEYPAD_DATA
#define F_CLK_PIO_NUM   PIO_FRONT_PANEL_KEYPAD_CLK

/*****************************/
/*        Externs            */
/*****************************/

#if RTOS != NOOS
/*****************************/
/* Local Function Prototypes */
/*****************************/
static int  button_int_handler(u_int32 dwIntID, bool bFIQ, void *pfnChain);
//static void key_detect_timer_handler(timer_id_t timer, void *userData);
static void led_glint_timer_handler(timer_id_t timer, void *userData);
static bool set_matrix_for_detect(void);
static u_int32 query_button_state(void);
static bool scan_button_is_pressed(u_int32 key_code);

/***************/
/* Global Data */
/***************/
PFNISR  pfnChain[BTN_NUM_COLS];
#if (0)
static tick_id_t  timerKeyDetect;
#endif

static tick_id_t  timerLedGlint;

static bool bInitialised = FALSE;
static PFNBUTTONCALLBACK pfnBtnCallback = (PFNBUTTONCALLBACK)NULL;

static unsigned char fp_led_data[LED_NUMBER_COUNTER]={0x90, 0xFF, 0x00}; 
static volatile u_int32 led_num_glinting = 0;
static volatile u_int32 lastState=0;

static volatile bool bKeyPressed = FALSE;
static volatile bool bLedGlintStart = FALSE;
static volatile int iLedGlintCount = 0;

button_data button_matrix[BTN_NUM_ROWS][BTN_NUM_COLS] = 
{ 
/*    SW 1 */
     {                                                                                     
         {0,  KEY_DETECT_CODE,   BTN_CODE_MENU }
     },                                                                                     
                                                                                           
/*    SW 2 */
     {                                                                                     
         {0,  KEY_DETECT_CODE,   BTN_CODE_SELECT }
     }, 
          
/*    SW 3 */
     {                                                                                     
         {0,  KEY_DETECT_CODE,   BTN_CODE_LEFT }
     },
           
/*    SW 4 */
     {                                                                                     
         {0,  KEY_DETECT_CODE,   BTN_CODE_RIGHT }
     },
           
/*    SW 5 */
     {                                                                                     
         {0,  KEY_DETECT_CODE,   BTN_CODE_DOWN }
     },
           
/*    SW 6 */
     {                                                                                     
         {0,  KEY_DETECT_CODE,   BTN_CODE_UP }
     }                                                                                              
};   


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
    int  i;
    u_int16 uBit = 0;
      
    u_int32 mux_reg_table[]=
        {
            PLL_PIN_GPIO_MUX0_REG,
            PLL_PIN_GPIO_MUX1_REG,
            PLL_PIN_GPIO_MUX2_REG,
            PLL_PIN_GPIO_MUX3_REG
        };    
    u_int32 GPIO_Out_Array[]=
        {
            LED_CTRL_CODE_B1,
            LED_CTRL_CODE_B2,
            LED_CTRL_CODE_B3,
            F_DATA_PIO_NUM,
            F_CLK_PIO_NUM,
            KEY_DETECT_CODE    
        };

    bInitialised = FALSE;         
    
    if (pfnCallback) 
        pfnBtnCallback = pfnCallback;
    else 
        trace_new(TRACE_KEY | TRACE_LEVEL_ALWAYS, 
             "BUTTON: Line %d: Error: Pointer to callback function is NULL!\n", __LINE__);
             
    if(button_get_pointers(lpFuncs))
    {  
        for(i=0; i<sizeof(GPIO_Out_Array)/sizeof(u_int32); i++)
        {
            uBit    = GPIO_Out_Array[i] & GPIO_MASK_PIN_NUMBER;
            CNXT_SET(mux_reg_table[uBit/32], 1<<(uBit%32), 0);            
        }

        cnxt_gpio_set_output_level(F_CLK_PIO_NUM, 0); 
                      
        cnxt_gpio_int_register_isr( KEY_DETECT_CODE,
                                   button_int_handler,
                                   FALSE,
                                   FALSE,
                                   &pfnChain[0]); 
                                   

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
              
//        timerKeyDetect = tick_create(key_detect_timer_handler, 0, "KBDT");
//        if(timerKeyDetect == 0)
//        {
//            error_log(ERROR_FATAL | RC_KEY_TIMERCREATE);
//            return(FALSE); /* Should never be reached */
//        }
//        
//        iRetcode = tick_set(timerKeyDetect, DEBOUNCE_TIMEOUT, FALSE);     
//        if(iRetcode != RC_OK)
//        {
//            error_log(ERROR_FATAL | RC_KEY_TIMERSET);
//            return(FALSE); /* Should never be reached */
//        }                            
        
        lastState = 0;        

        bKeyPressed = FALSE;
        bLedGlintStart = FALSE;         
        iLedGlintCount = 0;
        
        cnxt_gpio_set_input(KEY_DETECT_CODE);
        cnxt_gpio_set_int_edge(KEY_DETECT_CODE, BOTH_EDGES);
        cnxt_gpio_clear_pic_interrupt(KEY_DETECT_CODE);
        cnxt_gpio_int_enable(KEY_DETECT_CODE);
        cnxt_gpio_clear_pic_interrupt(KEY_DETECT_CODE);  
        
        tick_start(timerLedGlint); 
               
        bInitialised = TRUE;      
    }

    return (bInitialised);
    
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
    } 
    else 
    {
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
    
    if (bInitialised == FALSE)
         return FALSE;
    
    for (iRow = 0; iRow < BTN_NUM_ROWS; iRow++) 
    {
        if (button_matrix[iRow][0].button_code == key_code) 
        {
            bFound = TRUE;
            break;
        }  
    }
  
    /* If we found the key, go ahead and return its state */
    if (bFound) 
    {
        if (bInitialised) 
            bPressed = (lastState & key_code ? 1 : 0);
        else 
        {
            /* The driver has not been initialised so call the hardware directly. If we */
            /* do this after the driver is initialised, it would mess up the state of   */
            /* the hardware and prevent future button press/release events from being   */
            /* caught so DON'T DO IT.                                                   */
            dwState = query_button_state();
            bPressed = (dwState & key_code)? TRUE:FALSE;

        }
    
        trace_new(TRACE_KEY | TRACE_LEVEL_2,
              "Query keycode %d - row %d, col %d, state %d\n",
              key_code,
              iRow,
              iCol,
              bPressed);
    
        return(bPressed);
    } 
    else 
    {
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
//    int i;
//    int iRetcode;
    int iColNum = 0;

    /* Find which column generated the interrupt */
    if (CNXT_GPIO_STATUS_OK != cnxt_gpio_int_query_id(KEY_DETECT_CODE, dwIntID)) 
    {
        /* Error case - our vector was called for an interrupt we didn't */
        /* register!                                                     */
        isr_error_log(ERROR_WARNING | RC_KEY_ISR);
        *(PFNISR *)pfnCh = pfnChain[0];
        return(RC_ISR_NOTHANDLED);
    }  
    
//    tick_stop ( timerLedGlint );    
//    
//    /* Disable all the column interrupts, pull Low all the RAW output*/
//    cnxt_gpio_int_disable( KEY_DETECT_CODE );
//    
//    cnxt_gpio_set_output_level(LED_CTRL_CODE_B1, 0);
//    cnxt_gpio_set_output_level(LED_CTRL_CODE_B2, 0);
//    cnxt_gpio_set_output_level(LED_CTRL_CODE_B3, 0);
//      
//    /*Pull Low all the output pin*/
//    for (i=0; i<8; i++)
//    {    
//        cnxt_gpio_set_output_level(F_CLK_PIO_NUM, 0);
//        cnxt_gpio_set_output_level(F_CLK_PIO_NUM, 0);
//        
//        cnxt_gpio_set_output_level(F_DATA_PIO_NUM, 1);
//               
//        cnxt_gpio_set_output_level(F_CLK_PIO_NUM, 1);
//    }              
//          
//    /* We found a valid column so start the debounce timer */
//    iRetcode = tick_start(timerKeyDetect);
//    if(iRetcode != RC_OK) 
//        isr_error_log(ERROR_WARNING | RC_KEY_TIMERSTART);


    /* Disable all the column interrupts, pull Low all the RAW output*/
    cnxt_gpio_int_disable( KEY_DETECT_CODE );
    
    bKeyPressed = TRUE;
    

    *(PFNISR *)pfnCh = pfnChain[iColNum];
    return(RC_ISR_HANDLED);     

#else
  return(RC_ISR_NOTHANDLED);
#endif
}

/********************************************************************/
/*  FUNCTION:    query_button_state                                 */
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
u_int32 query_button_state()
{
#if EMULATION_LEVEL == FINAL_HARDWARE
    int i;

    int button_mask = 0;
    bool bInputLevel;
    
    static u_int32 key_code[] = 
    {
        /*Bit0_DP*/     0,
        /*Bit1_D */     BTN_CODE_RIGHT,      /* SW 4 */
        /*Bit2_E */     BTN_CODE_DOWN,       /* SW 5 */
        /*Bit3_A */     BTN_CODE_MENU,       /* SW 1 */                                                                                  
        /*Bit4_C */     BTN_CODE_LEFT,       /* SW 3 */
        /*Bit5_G */     0,
        /*Bit6_F */     BTN_CODE_UP,         /* SW 6 */                                                                                              
        /*Bit7_B */     BTN_CODE_SELECT      /* SW 2 */                    
    };

    /*Pull High the output pin one by one, and check the input power level*/
    for (i=0; i<8; i++)
    {    
        cnxt_gpio_set_output_level(F_CLK_PIO_NUM, 0);
        cnxt_gpio_set_output_level(F_CLK_PIO_NUM, 0);
        
        cnxt_gpio_set_output_level(F_DATA_PIO_NUM, i!=0);
        cnxt_gpio_set_output_level(F_CLK_PIO_NUM, 1);
        
        cnxt_gpio_get_level(KEY_DETECT_CODE, &bInputLevel);
        cnxt_gpio_get_level(KEY_DETECT_CODE, &bInputLevel);
//        cnxt_gpio_get_level(KEY_DETECT_CODE, &bInputLevel);
//        cnxt_gpio_get_level(KEY_DETECT_CODE, &bInputLevel);               
//        cnxt_gpio_get_level(KEY_DETECT_CODE, &bInputLevel);
        
        if (bInputLevel==0) 
        {      
            button_mask |= key_code[i];               
        }
        
    }              
 
    return button_mask;
#else
    return 0;
#endif
}

#if 0
/*************************************/
/* Timer handler for detecting a key */
/*************************************/
static void key_detect_timer_handler(timer_id_t timer, void *userData)
{
#if EMULATION_LEVEL == FINAL_HARDWARE

    #ifdef CONTINUOUS_PRESS
        static int count =0;
    #endif
    u_int32 dwState;
  
  
    /* Make sure this is our timer */
    if(timer == timerKeyDetect) 
    {
 
        /* Read the state of each button and send a message to the caller */
        /* if state has changed.                                          */
        dwState = query_button_state();
               
        if (dwState != lastState) 
        {
            /* Button state has changed. Send a message */
            pfnBtnCallback(dwState ^ lastState, ( ( (dwState ^ lastState) & dwState) > 0 ) );
            
            lastState = dwState;
            
            #ifdef CONTINUOUS_PRESS    
            count = 0;
            #endif
        }
         
         /* if button has been released, stop debounce timer */
        if ( dwState == 0 )
        {
            /* Reset the scan matrix state to wait for the next press or release */
            set_matrix_for_detect();
            tick_start( timerLedGlint );
            tick_stop ( timerKeyDetect );
            
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
#endif
}
#endif
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
 
    cnxt_gpio_set_int_edge(KEY_DETECT_CODE, BOTH_EDGES);
    cnxt_gpio_int_enable(KEY_DETECT_CODE);
    cnxt_gpio_clear_pic_interrupt(KEY_DETECT_CODE);
    
#endif
    return TRUE;
}


/*************************************/
/* Handler for LED glint timer       */
/*************************************/

#define LED_GLINT_BLANK_INTERVAL    (100)

static void led_glint_timer_handler(timer_id_t timer, void *userData)
{
#if EMULATION_LEVEL == FINAL_HARDWARE
    #ifdef CONTINUOUS_PRESS
        static int count =0;
    #endif

    int i;
    u_int32 dwState;
    static int glint = 0;
    
    static u_int32 fp_led_control_io[]=
    {
        LED_CTRL_CODE_B1,
        LED_CTRL_CODE_B2,
        LED_CTRL_CODE_B3,
        
        LED_CTRL_CODE_B3
    };

    /* Make sure this is our timer */
    if(timer == timerLedGlint) 
    {
        if (led_num_glinting==3)
        {
            if ((bLedGlintStart==TRUE) || (iLedGlintCount < 1) )
            {
                glint ++;
                
                if (glint*DEBOUNCE_TIMEOUT<LED_GLINT_BLANK_INTERVAL) 
                {
                    cnxt_gpio_set_output_level(LED_CTRL_CODE_B1, 0);
                    cnxt_gpio_set_output_level(LED_CTRL_CODE_B2, 0);
                    cnxt_gpio_set_output_level(LED_CTRL_CODE_B3, 0);  
                    
                    return;
                }
                
                if (glint*DEBOUNCE_TIMEOUT>2*LED_GLINT_BLANK_INTERVAL)
                {
                    glint = 0;
                    iLedGlintCount++;
                }
                
            }

            cnxt_gpio_set_output_level( fp_led_control_io[0], 0 );
            cnxt_gpio_set_output_level( fp_led_control_io[1], 0 );
            cnxt_gpio_set_output_level( fp_led_control_io[2], 0 );
            
            for (i=0; i<8; i++)
            {    
                cnxt_gpio_set_output_level(F_CLK_PIO_NUM, 0);
                cnxt_gpio_set_output_level(F_CLK_PIO_NUM, 0);
                cnxt_gpio_set_output_level(F_CLK_PIO_NUM, 0);
        
                cnxt_gpio_set_output_level(F_DATA_PIO_NUM, 1);
                cnxt_gpio_set_output_level(F_CLK_PIO_NUM, 1);
            }
        
            cnxt_gpio_set_output_level(F_CLK_PIO_NUM, 0);
            led_num_glinting = 0;

            
            if (bKeyPressed==FALSE)
                return;
                
            /* Read the state of each button and send a message to the caller */
            /* if state has changed.                                          */
            dwState = query_button_state();
               
            if (dwState != lastState) 
            {
                /* Button state has changed. Send a message */
                pfnBtnCallback(dwState ^ lastState, ( ( (dwState ^ lastState) & dwState) > 0 ) );
            
                lastState = dwState;
                #ifdef CONTINUOUS_PRESS    
                count = 0;
                #endif                                
            }
         
            /* if button has been released, stop debounce timer */
            if ( dwState == 0 )
            {
                /* Reset the scan matrix state to wait for the next press or release */
                set_matrix_for_detect();
                bKeyPressed = FALSE;
                
                return;
            }

            #ifdef CONTINUOUS_PRESS    
            if ( (count*DEBOUNCE_TIMEOUT)>125 )
            {
                count = 0;
                pfnBtnCallback(dwState, 1);
            }
            count++;
            #endif

            return;
        }

        cnxt_gpio_set_output_level( fp_led_control_io[(led_num_glinting+3)&3], 0 );        
        
        for (i=0; i<8; i++)
        {    
            cnxt_gpio_set_output_level(F_CLK_PIO_NUM, 0);
            cnxt_gpio_set_output_level(F_CLK_PIO_NUM, 0);
            cnxt_gpio_set_output_level(F_CLK_PIO_NUM, 0);
        
            if ( fp_led_data[led_num_glinting] & (0x80>>i) )
                cnxt_gpio_set_output_level(F_DATA_PIO_NUM, TRUE);
            else
                cnxt_gpio_set_output_level(F_DATA_PIO_NUM, FALSE);
               
            cnxt_gpio_set_output_level(F_CLK_PIO_NUM, 1);
        }
        
        cnxt_gpio_set_output_level(F_CLK_PIO_NUM, 0);

        cnxt_gpio_set_output_level( fp_led_control_io[led_num_glinting], 1 ); 
        
        led_num_glinting++;                       
    }
    
    
    
#endif
}

/********************************************************************/
/*  FUNCTION:    cnxt_btn_led_number                                */
/*                                                                  */
/*  PARAMETERS:  bool                                               */
/*                                                                  */
/*  DESCRIPTION: Enable capability for system reset when one        */
/*               button is held for a certain amount of time        */
/*                                                                  */
/*  RETURNS:     TRUE - capability is supported                     */
/*               FALSE - Parameters error or                        */
/*                       capability is not supported                */
/*                                                                  */
/********************************************************************/

static unsigned char led_number_table[] =
{
    0xDE, //			case 0:	Ls164_data = 0x42;
    0x90, //			case 1:	Ls164_data = 0x7b;
    0xAE, //			case 2:	Ls164_data = 0xc8;
    0xBA, //			case 3:	Ls164_data = 0x68;
    0xF0, //			case 4:	Ls164_data = 0x71;
    0x7A, //	      	case 5:	Ls164_data = 0xb4;
    0x7E, //			case 6:	Ls164_data = 0x84;
    0x98, //			case 7:	Ls164_data = 0x82;
    0xFE, //			case 8:	Ls164_data = 0x80;
    0xFA, //			case 9:	Ls164_data = 0xa0;
    0x90, //			case A:Ls164_data = 0x90;
    0x85, //			case B:Ls164_data = 0x85;
    0xc6, //			case C:Ls164_data = 0xc6;
    0x89, //			case D:Ls164_data = 0x89;
    0x6E, //			case E:Ls164_data = 0xc4;
    0x6c,  //			case F:Ls164_data = 0xd4;
    
    0xDE, //			case G:	Ls164_data = 0x42;
    0x90, //			case H:	Ls164_data = 0x7b;
    0xAE, //			case I:	Ls164_data = 0xc8;
    0xBA, //			case J:	Ls164_data = 0x68;
    0xF0, //			case K:	Ls164_data = 0x71;
    0x7A, //	      	case L:	Ls164_data = 0xb4;
    0x7E, //			case M:	Ls164_data = 0x84;
    0x98, //			case N:	Ls164_data = 0x82;
    0xDE, //			case O:	Ls164_data = 0x80;
    0xEC, //			case P:	Ls164_data = 0xa0;
    0x90, //			case Q:Ls164_data = 0x90;
    0x85, //			case R:Ls164_data = 0x85;
    0xc6, //			case S:Ls164_data = 0xc6;
    0x89, //			case T:Ls164_data = 0x89;
    0xc4, //			case U:Ls164_data = 0xc4;
    0xd4,  //			case V:Ls164_data = 0xd4;    
    0xFA, //			case W:	Ls164_data = 0xa0;
    0x90, //			case X:Ls164_data = 0x90;
    0x85, //			case Y:Ls164_data = 0x85;
    0xc6 //			    case Z:Ls164_data = 0xc6;

};

bool cnxt_btn_led_number(u_int32 word, unsigned char pos, bool bHex) 
{
    char str_array[LED_NUMBER_COUNTER];
    int i;
   
    if (pos&0xF8)
        return FALSE;
        
    if( word > 0xFFF )
        return FALSE;
    
    if ( (word>999) & (bHex==FALSE) )
        return FALSE;
        
    if (bHex==TRUE)
        sprintf(str_array, "%03X", (int)word);
    else
        sprintf(str_array, "%03d", (int)word);

    for(i=0; i<LED_NUMBER_COUNTER; i++)
    {
        if ((str_array[i]>='A')&&(str_array[i]<='F'))
            str_array[i] = str_array[i] - 'A' + 10;
        else
            str_array[i] = str_array[i] - '0';
        
        str_array[i] = led_number_table[str_array[i]];    
        if (pos&(1<<i))
            str_array[i] = str_array[i] | 0x1;
            
        fp_led_data[i] = str_array[i];
    }    
           
    return TRUE;
}


bool cnxt_led_display_num(u_int32 word, bool bHex, u_int8 pos)
{
    
    char str_array[LED_NUMBER_COUNTER];
    int i;
   
    if (pos&0xF8)
        return FALSE;
        
    if( word > 0xFFF )
        return FALSE;
    
    if ( (word>999) & (bHex==FALSE) )
        return FALSE;
        
    if (bHex==TRUE)
        sprintf(str_array, "%03X", (int)word);
    else
        sprintf(str_array, "%03d", (int)word);

    for(i=0; i<LED_NUMBER_COUNTER; i++)
    {
        if ((str_array[i]>='A')&&(str_array[i]<='F'))
            str_array[i] = str_array[i] - 'A' + 10;
        else
            str_array[i] = str_array[i] - '0';
        
        str_array[i] = led_number_table[str_array[i]];    
        if (pos&(1<<i))
            str_array[i] = str_array[i] | 0x1;
            
        fp_led_data[i] = str_array[i];
    }    
           
    return TRUE;    
    
}

bool cnxt_led_display_chars(char *pstr, u_int8 len, u_int8 pos)
{
    
    char str_array[LED_NUMBER_COUNTER];
    int i;
   
    if (pos&0xF8)
        return FALSE;
         
    if (!pstr)
        return FALSE;
    
    if (len>3)
        return FALSE;
    
    switch (len)
    {
       case 0:
          str_array[0]=str_array[1]=str_array[2]=0;
          break;
          
       case 1:
         str_array[0]=str_array[1]=0;
         str_array[2]=*pstr;
         break;
         
       case 2:
         str_array[0]=0;
         str_array[1]=*pstr;
         str_array[2]=*(pstr+1);
         break;
                
       default:
       case 3:
         str_array[0]=*pstr;
         str_array[1]=*(pstr+1);
         str_array[2]=*(pstr+2);
         break;       
    } 
       
    for(i=0; i<LED_NUMBER_COUNTER; i++)
    {
        if (str_array[i]=='-')
        {
            str_array[i] = 0x20;
            if (pos&(1<<i))
                 str_array[i] = str_array[i] | 0x1;
        
            fp_led_data[i] = str_array[i];
            continue;
        }
        
        if ((str_array[i]>='A')&&(str_array[i]<='Z'))
            str_array[i] = str_array[i] - 'A' + 10;
        else
            str_array[i] = str_array[i] - '0';
        
        str_array[i] = led_number_table[str_array[i]];    
        if (pos&(1<<i))
            str_array[i] = str_array[i] | 0x1;
            
        fp_led_data[i] = str_array[i];
    }    
           
    return TRUE;    
    
}


bool cnxt_led_display_pplus_num(u_int32 word, bool bHex, u_int8 pos)
{
    char str_array[LED_NUMBER_COUNTER];
    int i;
   
    if (pos&0xF8)
        return FALSE;
        
    if( word > 0xFF )
        return FALSE;
    
    if ( (word>99) & (bHex==FALSE) )
        return FALSE;
        
    if (bHex==TRUE)
        sprintf(str_array, "P%02X", (int)word);
    else
        sprintf(str_array, "P%02d", (int)word);

    for(i=0; i<LED_NUMBER_COUNTER; i++)
    {
        if ((str_array[i]>='A')&&(str_array[i]<='Z'))
            str_array[i] = str_array[i] - 'A' + 10;
        else
            str_array[i] = str_array[i] - '0';
        
        str_array[i] = led_number_table[str_array[i]];    
        if (pos&(1<<i))
            str_array[i] = str_array[i] | 0x1;
            
        fp_led_data[i] = str_array[i];
    }    
           
    return TRUE;        
}

bool cnxt_led_glint_start()
{
    iLedGlintCount = 0;
    bLedGlintStart = TRUE;
    
    return TRUE;
}

bool cnxt_led_glint_stop()
{
    
    bLedGlintStart = FALSE;
    
    return TRUE;
}

#endif /* RTOS != NOOS */



#if FRONT_PANEL_KEYPAD_TYPE == FRONT_PANEL_KEYPAD_TONGDA

void InitTDFPLed(void)
{
   

/*
PIO_FRONT_PANEL_KEYPAD_ROW_0 = 27+GPIO_DEVICE_ID_INTERNAL+GPIO_POSITIVE_POLARITY+GPIO_PIN_IS_OUTPUT
PIO_FRONT_PANEL_KEYPAD_ROW_1 = 75+GPIO_DEVICE_ID_INTERNAL+GPIO_POSITIVE_POLARITY+GPIO_PIN_IS_OUTPUT
PIO_FRONT_PANEL_KEYPAD_ROW_2 = 10+GPIO_DEVICE_ID_INTERNAL+GPIO_POSITIVE_POLARITY+GPIO_PIN_IS_OUTPUT
*/

{
   int i;
   u_int8 data = 0x00;
   u_int16 uBit = 0;
   u_int32 mux_reg_table[]=
   {
      PLL_PIN_GPIO_MUX0_REG,
      PLL_PIN_GPIO_MUX1_REG,
      PLL_PIN_GPIO_MUX2_REG,
      PLL_PIN_GPIO_MUX3_REG
    };    


#ifdef PIO_FRONT_PANEL_KEYPAD_ROW_0
      uBit    = PIO_FRONT_PANEL_KEYPAD_ROW_0 & GPIO_MASK_PIN_NUMBER;
      CNXT_SET(mux_reg_table[uBit/32], 1<<(uBit%32), 0);  
      cnxt_gpio_set_output_level(PIO_FRONT_PANEL_KEYPAD_ROW_0, 0);
#endif

#ifdef PIO_FRONT_PANEL_KEYPAD_ROW_1
      uBit    = PIO_FRONT_PANEL_KEYPAD_ROW_1 & GPIO_MASK_PIN_NUMBER;
      CNXT_SET(mux_reg_table[uBit/32], 1<<(uBit%32), 0);
      cnxt_gpio_set_output_level(PIO_FRONT_PANEL_KEYPAD_ROW_1, 0);  
#endif

#ifdef PIO_FRONT_PANEL_KEYPAD_ROW_2
      uBit    = PIO_FRONT_PANEL_KEYPAD_ROW_2 & GPIO_MASK_PIN_NUMBER;
      CNXT_SET(mux_reg_table[uBit/32], 1<<(uBit%32), 0);  
      cnxt_gpio_set_output_level(PIO_FRONT_PANEL_KEYPAD_ROW_2, 0);
#endif

#if defined(PIO_FRONT_PANEL_KEYPAD_DATA) && defined(PIO_FRONT_PANEL_KEYPAD_CLK)
      uBit    = PIO_FRONT_PANEL_KEYPAD_DATA & GPIO_MASK_PIN_NUMBER;
      CNXT_SET(mux_reg_table[uBit/32], 1<<(uBit%32), 0);  
      cnxt_gpio_set_output_level(PIO_FRONT_PANEL_KEYPAD_DATA, 0);

      uBit    = PIO_FRONT_PANEL_KEYPAD_CLK & GPIO_MASK_PIN_NUMBER;
      CNXT_SET(mux_reg_table[uBit/32], 1<<(uBit%32), 0);  
      cnxt_gpio_set_output_level(PIO_FRONT_PANEL_KEYPAD_CLK, 0);
      
      for (i=0; i<8; i++)
      {    
         cnxt_gpio_set_output_level(PIO_FRONT_PANEL_KEYPAD_CLK, 0);
         cnxt_gpio_set_output_level(PIO_FRONT_PANEL_KEYPAD_CLK, 0);
        
         if ( data & (0x80>>i) )
             cnxt_gpio_set_output_level(PIO_FRONT_PANEL_KEYPAD_DATA, TRUE);
         else
             cnxt_gpio_set_output_level(PIO_FRONT_PANEL_KEYPAD_DATA, FALSE);
               
         cnxt_gpio_set_output_level(PIO_FRONT_PANEL_KEYPAD_CLK, 1);
     }

#endif


#ifdef PIO_FRONT_PANEL_KEYPAD_ROW_0
      cnxt_gpio_set_output_level(PIO_FRONT_PANEL_KEYPAD_ROW_0, 1);
#endif

#ifdef PIO_FRONT_PANEL_KEYPAD_ROW_1
      cnxt_gpio_set_output_level(PIO_FRONT_PANEL_KEYPAD_ROW_1, 1);  
#endif

#ifdef PIO_FRONT_PANEL_KEYPAD_ROW_2
      cnxt_gpio_set_output_level(PIO_FRONT_PANEL_KEYPAD_ROW_2, 1);
#endif
      cnxt_gpio_set_output_level(PIO_LED_NIM_LOCK, 1);

} 
   
}

#endif




