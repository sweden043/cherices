#include "hwconfig.h"
#include "hwlib.h"
#include "kal.h"
#include "sabine.h"
#include "globals.h"
#include "retcodes.h"
#include "buttons.h"
#include "scanbtns.h"

#if FRONT_PANEL_KEYPAD_TYPE == FRONT_PANEL_KEYPAD_VEND_D_PROD_1

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
   
#endif  /* #if FRONT_PANEL_KEYPAD_TYPE == FRONT_PANEL_KEYPAD_VEND_D_PROD_1 */
