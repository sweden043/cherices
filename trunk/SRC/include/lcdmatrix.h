/****************************************************************************/
/*                          Conexant Systems Inc.                           */
/****************************************************************************/
/*                                                                          */
/* Filename:       LCDMATRIX.H                                              */
/*                                                                          */
/* Description:    Definitions pertaining to the Matrix Orbital Corporation */
/*                 LCD2041 display                                          */
/*                                                                          */
/* Author:         Dave Wilson                                              */
/*                                                                          */
/* Copyright Conexant Systems Inc, 2000                                     */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/
#ifndef _LCDMATRIX_H_
#define _LCDMATRIX_H_

/*****************/
/* Command codes */
/*****************/
#define LCD_COMMAND_PREFIX 0xFE

#define LCD_AUTO_WRAP_ON   'C'
#define LCD_AUTO_WRAP_OFF  'D'
#define LCD_SCROLL_ON      'Q'
#define LCD_SCROLL_OFF     'R'
#define LCD_BACKLIGHT_ON   'B'
#define LCD_BACKLIGHT_OFF  'F'
#define LCD_BLINK_ON       'S'
#define LCD_BLINK_OFF      'T'
#define LCD_SET_BRIGHTNESS 'Y'
#define LCD_CLEAR_DISPLAY  'X'
#define LCD_SET_CONTRAST   'P'
#define LCD_CURSOR_ON      'J'
#define LCD_CURSOR_OFF     'K'
#define LCD_CURSOR_LEFT    'L'
#define LCD_CURSOR_RIGHT   'M'
#define LCD_GOTO_XY        'G'
#define LCD_GOTO_HOME      'H'

#endif /* _LCDMATRIX_H_ */
