/****************************************************************************/
/*                          Conexant Systems Inc.                           */
/****************************************************************************/
/*                                                                          */
/* Filename:       BUTTONS.H                                                */
/*                                                                          */
/* Description:    Generic front panel button driver interface header       */
/*                                                                          */
/* Author:         Dave Wilson                                              */
/*                                                                          */
/* Copyright Conexant Systems Inc, 2000                                     */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/
#ifndef _BUTTONS_H_
#define _BUTTONS_H_

/**************************************/
/* Keycodes returned by button driver */
/**************************************/
#define BTN_CODE_POWER   0x0001
#define BTN_CODE_MENU    0x0002
#define BTN_CODE_INFO    0x0004
#define BTN_CODE_LEFT    0x0008
#define BTN_CODE_RIGHT   0x0010
#define BTN_CODE_UP      0x0020
#define BTN_CODE_DOWN    0x0040
#define BTN_CODE_SELECT  0x0080
#define BTN_CODE_BACKUP  0x0100


/*add by txl,2005-4-4*/
#define MaxKeyTimer 5000
/* On Klondike, the TV Guide button replaces the Thor Menu button */
#define BTN_CODE_GUIDE   BTN_CODE_MENU

#define BTN_CODE_NONE    0x0000

/************/
/* Typedefs */
/************/
typedef void (*PFNBUTTONCALLBACK)(u_int16, bool);

typedef struct _button_ftable 
{
  int  driver_number;
//  bool (*button_is_pressed)(u_int16);
  bool (*button_is_pressed)(u_int32);
} button_ftable, *lpbutton_ftable;

/************************/
/* Driver API functions */
/************************/

bool button_init(PFNBUTTONCALLBACK pfnCallback, lpbutton_ftable lpFuncTable);
bool button_get_pointers(lpbutton_ftable lpFuncTable);

bool cnxt_btn_pio_reset_enable();
void cnxt_btn_pio_reset_disable();
bool cnxt_btn_pio_reset_set_timeout(int ms);
bool cnxt_btn_pio_reset_set_polarity(int pol);

#endif /* _BUTTONS_H_ */
