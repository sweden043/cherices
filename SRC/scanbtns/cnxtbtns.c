#include "hwconfig.h"
#include "hwlib.h"
#include "kal.h"
#include "globals.h"
#include "retcodes.h"
#include "buttons.h"
#include "scanbtns.h"

/*
 * NOTE: The SCANBTNS driver can be used to decode a keypad with each button attached
 * to a unique GPIO (a GPIO button keypad). To do this, only define 1 row with each
 * button defined as a separate column as shown in the example below. An unused GPIO
 * is assigned to the row while the GPIO attached to each button is assigned to the 
 * corresponding column. The SCANBTNS driver will harmlessly drive the unused row GPIO 
 * before sensing a press of any of the column buttons.
 */
/*          col 0     col 1     col 2     col 3     col 4     col 5     col 6
 *  row 0   power     menu      left      up        down      right     select       
 */

/* button_data button_matrix[FRONT_PANEL_KEYPAD_NUM_ROWS][FRONT_PANEL_KEYPAD_NUM_COLS] = { */
    /* Row 0 */
/*     {                                                                                     */
/*         {PIO_FRONT_PANEL_KEYPAD_ROW_0,  PIO_FRONT_PANEL_KEYPAD_COL_0,   BTN_CODE_POWER }, */
/*         {PIO_FRONT_PANEL_KEYPAD_ROW_0,  PIO_FRONT_PANEL_KEYPAD_COL_1,   BTN_CODE_MENU  }, */
/*         {PIO_FRONT_PANEL_KEYPAD_ROW_0,  PIO_FRONT_PANEL_KEYPAD_COL_2,   BTN_CODE_LEFT  }, */
/*         {PIO_FRONT_PANEL_KEYPAD_ROW_0,  PIO_FRONT_PANEL_KEYPAD_COL_3,   BTN_CODE_UP    }, */
/*         {PIO_FRONT_PANEL_KEYPAD_ROW_0,  PIO_FRONT_PANEL_KEYPAD_COL_4,   BTN_CODE_DOWN  }, */
/*         {PIO_FRONT_PANEL_KEYPAD_ROW_0,  PIO_FRONT_PANEL_KEYPAD_COL_5,   BTN_CODE_RIGHT }, */
/*         {PIO_FRONT_PANEL_KEYPAD_ROW_0,  PIO_FRONT_PANEL_KEYPAD_COL_6,   BTN_CODE_SELECT}  */
/*     }                                                                                     */
/* };                                                                                        */

/*
 * NOTE:  For now, all of the scan matrix front panels use the same
 * matrix for row/column/button
 */

/*              4          5         6           
 *             col 0     col 1     col 2
 *  1  row 0   backup    guide     info
 *  2  row 1   left      on        down
 *  3  row 2   select    right     up
 */

button_data button_matrix[FRONT_PANEL_KEYPAD_NUM_ROWS][FRONT_PANEL_KEYPAD_NUM_COLS] = { 
    /* Row 0 */
    {                                                                                      
        {PIO_FRONT_PANEL_KEYPAD_ROW_0,  PIO_FRONT_PANEL_KEYPAD_COL_0,   BTN_CODE_BACKUP }, 
        {PIO_FRONT_PANEL_KEYPAD_ROW_0,  PIO_FRONT_PANEL_KEYPAD_COL_1,   BTN_CODE_GUIDE },  
        {PIO_FRONT_PANEL_KEYPAD_ROW_0,  PIO_FRONT_PANEL_KEYPAD_COL_2,   BTN_CODE_INFO }    
    },                                                                                     

    /* Row 1 */
    {                                                                                     
        {PIO_FRONT_PANEL_KEYPAD_ROW_1,  PIO_FRONT_PANEL_KEYPAD_COL_0,   BTN_CODE_LEFT },  
        {PIO_FRONT_PANEL_KEYPAD_ROW_1,  PIO_FRONT_PANEL_KEYPAD_COL_1,   BTN_CODE_POWER }, 
        {PIO_FRONT_PANEL_KEYPAD_ROW_1,  PIO_FRONT_PANEL_KEYPAD_COL_2,   BTN_CODE_DOWN }   
    },                                                                                    

    /* Row 2 */
    {                                                                                      
        {PIO_FRONT_PANEL_KEYPAD_ROW_2,  PIO_FRONT_PANEL_KEYPAD_COL_0,   BTN_CODE_SELECT }, 
        {PIO_FRONT_PANEL_KEYPAD_ROW_2,  PIO_FRONT_PANEL_KEYPAD_COL_1,   BTN_CODE_RIGHT },  
        {PIO_FRONT_PANEL_KEYPAD_ROW_2,  PIO_FRONT_PANEL_KEYPAD_COL_2,   BTN_CODE_UP }      
    }                                                                                      
};                                                                                         


