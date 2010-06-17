#ifndef _SCANBTNS_H_
#define _SCANBTNS_H_

typedef struct _button_data {
    /* row GPIO definition */
    u_int32     row_code;

    /* column GPIO definition */
    u_int32     column_code;

    /* as defined in buttons.h BTN_CODE_POWER, _MENU, _INFO, etc... */
    u_int32     button_code;
    
} button_data;

#endif
