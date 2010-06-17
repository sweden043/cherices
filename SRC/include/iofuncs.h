/****************************************************************************/
/*                   Conexant Proprietary and Confidential                  */
/*                       Software File/Module Header                        */
/*              Conexant Systems Inc. (c) 1998, 1999, 2000, 2001            */
/*                               Austin, TX                                 */
/*                            All Rights Eeserved                           */
/****************************************************************************/
/*
 * Filename:       IOFUNCS.H
 *
 *
 * Description:    Public header file for IOFUNCS module. This module
 *                 contains high level functions relating to rendering of 
 *                 text on the OSD plane and handling input from remote 
 *                 and front panel.
 *
 * Author:         Dave Wilson
 *
 ****************************************************************************/
/* $Header: iofuncs.h, 12, 7/31/03 3:52:36 PM, Steven Jones$
 ****************************************************************************/
#ifndef _IOFUNCS_H_
#define _IOFUNCS_H_

#include "osdlib.h"
#include "curlib.h"
#include "gfxtypes.h"

/**********************/
/**********************/
/**                  **/
/** Type Definitions **/
/**                  **/
/**********************/
/**********************/

typedef enum _IOFUNCS_STATUS
{
  IOFUNCS_STATUS_OK,
  IOFUNCS_STATUS_TIMEOUT,
  IOFUNCS_STATUS_REINIT,
  IOFUNCS_ERROR_INVALID,
  IOFUNCS_ERROR_BAD_PARAM,
  IOFUNCS_ERROR_BAD_PTR,
  IOFUNCS_ERROR_OUT_OF_BOUNDS,
  IOFUNCS_ERROR_INTERNAL,
  IOFUNCS_ERROR_NOT_INIT,
  IOFUNCS_ERROR_NO_PALETTE,
  IOFUNCS_ERROR_BAD_MODE
} IOFUNCS_STATUS;
  
typedef enum _IOFUNCS_SCROLL
{
  SCROLL_UP,
  SCROLL_DOWN,
  SCROLL_LEFT,
  SCROLL_RIGHT
} IOFUNCS_SCROLL;  

typedef enum _IOFUNCS_CURSOR
{
  CURSOR_TEXT,
  CURSOR_POINTER
} IOFUNCS_CURSOR;

typedef union _IOFUNCS_COLOR
{
  DRMPAL sColor;
  char   cIndex;
} IOFUNCS_COLOR, *PIOFUNCS_COLOR;

typedef enum _IOFUNCS_MESSAGE
{
  IOFUNCS_MSG_KEY_PRESSED,   
  IOFUNCS_MSG_KEY_RELEASED, 
  IOFUNCS_MSG_KEY_HOLD,
  IOFUNCS_MSG_MOUSE_BTN1_DOWN,
  IOFUNCS_MSG_MOUSE_BTN1_UP,  
  IOFUNCS_MSG_MOUSE_BTN2_DOWN,
  IOFUNCS_MSG_MOUSE_BTN2_UP,
  IOFUNCS_MSG_MOUSE_MOVE_ABSOLUTE,
  IOFUNCS_MSG_MOUSE_MOVE_RELATIVE,
  IOFUNCS_MSG_SHUTDOWN,
  /* For internal use only. The following messages are not passed to applications */
  IOFUNCS_MSG_KEY_TIMEOUT,
  IOFUNCS_MSG_GET_STRING,
  IOFUNCS_MSG_GET_KEY,
  IOFUNCS_MSG_SHOW_CURSOR,
  IOFUNCS_MSG_UPDATE_CURSOR,
  IOFUNCS_NUM_MESSAGES /* This must be last! */
} IOFUNCS_MESSAGE;

typedef void (*PFNINPUTDEVICECALLBACK)(IOFUNCS_MESSAGE, u_int32, u_int32);

/* Backwards compatibility definition */
#define IOFUNCS_MSG_MOUSE_MOVE IOFUNCS_MSG_MOUSE_MOVE_ABSOLUTE

/*******************************/
/*******************************/
/**                           **/
/** Internal font definitions **/
/**                           **/
/*******************************/
/*******************************/

extern GFX_FONT font_8x8;
extern GFX_FONT font_8x14;
extern GFX_FONT font_8x16;

/********************/
/********************/
/**                **/
/** API prototypes **/
/**                **/
/********************/
/********************/

/**********************************/
/* Initialisation and OSD control */
/**********************************/

/* IOFUNCS creates and manages a full screen OSD region on behalf         */
/* of the client. These functions relate to region-level control.         */
/* Coordinates used by the client when printing text are (generally)      */
/* based upon cell coordinates where a cell is the size of the monospaced */
/* font passed to the init call.                                          */

IOFUNCS_STATUS cnxt_iofuncs_init(PGFX_FONT pDefaultFont, u_int32 uOsdMode);
IOFUNCS_STATUS cnxt_iofuncs_get_font_and_mode(PGFX_FONT *ppDefaultFont, u_int32 *pMode);
IOFUNCS_STATUS cnxt_iofuncs_shutdown(void);
IOFUNCS_STATUS cnxt_iofuncs_get_screen_grid( int *pWidth, int *pHeight);
IOFUNCS_STATUS cnxt_iofuncs_screen_display(bool bShow);
IOFUNCS_STATUS cnxt_iofuncs_set_palette(PDRMPAL pPal);
IOFUNCS_STATUS cnxt_iofuncs_get_osd_handle(OSDHANDLE *pHandle);
IOFUNCS_STATUS cnxt_iofuncs_change_OSD(OSDHANDLE hNew);
IOFUNCS_STATUS cnxt_iofuncs_restore_OSD(void);
IOFUNCS_STATUS cnxt_iofuncs_get_gfx_bitmap(GFX_BITMAP **ppBitmap);
IOFUNCS_STATUS cnxt_iofuncs_associate_bitmap(GFX_BITMAP *pBitmap, OSDHANDLE oHandle);

IOFUNCS_STATUS  cnxt_input_init(void);
IOFUNCS_STATUS cnxt_input_exit(void);

/* To maintain backwards compatibility... */
#define cnxt_iofuncs_osd_display() cnxt_iofuncs_screen_display(TRUE)
#define cnxt_iofuncs_osd_hide() cnxt_iofuncs_screen_display(FALSE)

/****************/ 
/* Text display */
/****************/ 

/* IOFUNCS provides several printf-style functions for drawing text in    */
/* various fonts to the screen. The basic cnxt_iofuncs_printf function    */
/* draws at the current cursor position, using the default font  and      */
/* scrolls or wraps if necessary. Later functions give more control over  */
/* font and position. Two different print_at type functions are provided  */
/* to allow printing at cell coordinates and pixel coordinates. Most of   */
/* IOFUNCS internal functions work on a cell coordinate system but pixel  */
/* coordinates are supported here to allow fine tuning of text position.  */
/* Note that scrolling, clipping and windowing only apply to the basic    */
/* cnxt_iofuncs_printf function. For all other cases, the text is drawn   */
/* on the screen at the position specified and clipped to the screen      */
/* boundaries only. No line wrapping is performed in these cases.         */

IOFUNCS_STATUS cnxt_iofuncs_printf(char *strFormat, ...);
IOFUNCS_STATUS cnxt_iofuncs_printf_at(int iX, int iY, char *strFormat, ...);
IOFUNCS_STATUS cnxt_iofuncs_printf_at_pixel(int iX, int iY, char *strFormat, ...);
IOFUNCS_STATUS cnxt_iofuncs_printf_font_at(PGFX_FONT pFont, int iX, int iY, char *strFormat, ...);
IOFUNCS_STATUS cnxt_iofuncs_printf_font_at_pixel(PGFX_FONT pFont, int iX, int iY, char *strFormat, ...);
IOFUNCS_STATUS cnxt_iofuncs_printf_font_at_pixel_auto_wrap(PGFX_FONT pFont, int x, int y, int x2, int y2,u_int8 calculate,u_int8 *end, int *pNumChars,char *strFormat, ...);
IOFUNCS_STATUS cnxt_iofuncs_get_bounding_rect(PGFX_FONT pFont, POSDRECT pRect, char *strFormat, ...);

/*******************/
/* Cursor handling */
/*******************/

/* IOFUNCS uses the CX2249x hardware cursor to indicate the current text */
/* entry or print position. These functions allow manipulation of the    */
/* cursor.                                                               */

IOFUNCS_STATUS cnxt_iofuncs_set_cursor_pos(IOFUNCS_CURSOR eCursor, int iX, int iY);
IOFUNCS_STATUS cnxt_iofuncs_get_cursor_pos(IOFUNCS_CURSOR eCursor, int *piX, int *piY);
IOFUNCS_STATUS cnxt_iofuncs_cursor_display(IOFUNCS_CURSOR eCursor, bool bShow);
IOFUNCS_STATUS cnxt_iofuncs_set_mouse_cursor(HOSDCUR hNewCursor);
IOFUNCS_STATUS cnxt_iofuncs_set_mouse_relative(bool bRelative);
IOFUNCS_STATUS cnxt_iofuncs_is_mouse_relative(bool *pbRelative);

/******************/
/* Window control */
/******************/

/* Although a full screen OSD is used, the caller can define a window  */
/* which will be affected by all future IOFUNCS drawing commands. Text */
/* will be displayed within this window and scrolling or wrapping will */
/* occur at the window edges. This allows a client to define a region  */
/* of the screen to, for example, contain scrolling text while         */
/* preserving the contents of the rest of the OSD.                     */
/* With auto-scrolling enabled, IOFUNCS will move the contents of the  */
/* current window up one line whenever something is printed to the     */
/* bottom visible line. If disabled, the text will wrap back to the    */
/* top of the current window and overwrite whatever is there.          */

IOFUNCS_STATUS cnxt_iofuncs_cls(bool bFullScreen);
IOFUNCS_STATUS cnxt_iofuncs_set_window(POSDRECT pRect);
IOFUNCS_STATUS cnxt_iofuncs_get_window(POSDRECT pRect);
IOFUNCS_STATUS cnxt_iofuncs_scroll_window(IOFUNCS_SCROLL eDirection, int iNumPixels);
IOFUNCS_STATUS cnxt_iofuncs_auto_scroll(bool bEnable);

/********************/
/* Colour Selection */
/********************/

/* All text drawing operations make use of foreground and background  */
/* colours. These APIs let the client set and query the colours to    */
/* be used in future drawing operations.                              */

IOFUNCS_STATUS cnxt_iofuncs_set_foreground_color(PIOFUNCS_COLOR pColor);
IOFUNCS_STATUS cnxt_iofuncs_set_background_color(PIOFUNCS_COLOR pColor);
IOFUNCS_STATUS cnxt_iofuncs_get_foreground_color(PIOFUNCS_COLOR pColor);
IOFUNCS_STATUS cnxt_iofuncs_get_background_color(PIOFUNCS_COLOR pColor);
IOFUNCS_STATUS cnxt_iofuncs_set_aa_indices(u_int8 c33Fore, u_int8 c66Fore);
IOFUNCS_STATUS cnxt_iofuncs_get_aa_indices(u_int8 *p33Fore, u_int8 *p66Fore);

/************************************************/
/* Front panel and remote button press handling */
/************************************************/

/* This group of functions gives the client a conio-like interface to     */
/* access keystrokes from the front panel and IR remote. By default,      */
/* keyboard/front panel ASCII characters are only echoed to the screen    */
/* at the current cursor position while getstr is being processed but     */
/* the client can choose to turn this off if desired. The client may      */
/* also choose to hook all keyboard and mouse events. When a callback     */
/* is registered, all events are passed directly to the callback function */
/* unless cnxt_iofuncs_getstr or cnxt_iofuncs_getch is currently being    */
/* processed in which case keys entered are passed only to the caller of  */
/* those functions.                                                       */

IOFUNCS_STATUS cnxt_iofuncs_getch(int iTimeoutMs, char *pChar);
IOFUNCS_STATUS cnxt_iofuncs_getstr(char *lpBuffer, int iBuffSize);
IOFUNCS_STATUS cnxt_iofuncs_press_any_key(void);
IOFUNCS_STATUS cnxt_iofuncs_char_echo(bool bOn);
IOFUNCS_STATUS cnxt_iofuncs_is_char_echo_on(bool *pbOn);
IOFUNCS_STATUS cnxt_iofuncs_register_input_device_callback(PFNINPUTDEVICECALLBACK pfnCallback);
IOFUNCS_STATUS cnxt_iofuncs_query_input_device_callback(PFNINPUTDEVICECALLBACK *ppfnCallback);

#endif /* _IOFUNCS_H_ */

/****************************************************************************
 * Modifications:
 * $Log: 
 *  12   mpeg      1.11        7/31/03 3:52:36 PM     Steven Jones    SCR(s): 
 *        5046 
 *        Add functionality for redirecting to different osd regions.
 *        
 *  11   mpeg      1.10        6/18/03 3:20:56 PM     Dave Wilson     SCR(s): 
 *        6782 
 *        Changed cnxt_iofuncs_set_aa_indices to remove indices for foreground 
 *        and
 *        background colours. These are already set when the foreground and 
 *        background
 *        colours are set using cnxt_iofuncs_set_foreground/background_color so
 *         are not
 *        needed here too.
 *        
 *  10   mpeg      1.9         10/2/02 12:28:00 PM    Dave Wilson     SCR(s) 
 *        4621 :
 *        Added code and defintions to support greyscale font rendering.
 *        
 *  9    mpeg      1.8         8/22/02 4:08:28 PM     Dave Wilson     SCR(s) 
 *        4319 :
 *        Added prototype for cnxt_iofuncs_get_bounding_rect function
 *        
 *  8    mpeg      1.7         8/2/02 12:16:50 PM     Dave Wilson     SCR(s) 
 *        4314 :
 *        Added prototype for new API cnxt_iofuncs_get_gfx_handle
 *        
 *  7    mpeg      1.6         6/19/02 10:40:42 AM    Dave Wilson     SCR(s) 
 *        3993 :
 *        Added 2 new function prototypes for cnxt_iofuncs_set_mouse_relative 
 *        and
 *        cnxt_iofuncs_is_mouse_relative. Replaces IOFUNCS_MSG_MOUSE_MOVE with 
 *        2 new
 *        messages, IOFUNCS_MSG_MOUSE_MOVE_ABSOLUTE/RELATIVE.
 *        
 *  6    mpeg      1.5         3/6/02 11:24:58 AM     Dave Wilson     SCR(s) 
 *        3315 :
 *        Added cnxt_iofuncs_press_any_key function which implements an 
 *        easy-to-use
 *        "Press any key to continue" API.
 *        
 *  5    mpeg      1.4         2/28/02 9:07:58 AM     Dave Wilson     SCR(s) 
 *        3254 :
 *        Added cnxt_iofuncs_screen_display and cnxt_iofuncs_get_font_and_mode 
 *        API
 *        prototypes. Also made old cnxt_iofuncs_osd_show and 
 *        cnxt_iofuncs_osd_hide
 *        into macros using the new cnxt_iofuncs_screen_display function.
 *        
 *        
 *        
 *  4    mpeg      1.3         1/7/02 2:38:36 PM      Dave Wilson     SCR(s) 
 *        3002 :
 *        Added cnxt_iofuncs_set_mouse_cursor to allow clients to set their own
 *        cursor bitmaps.
 *        
 *  3    mpeg      1.2         1/3/02 3:56:06 PM      Dave Wilson     SCR(s) 
 *        2995 :
 *        Added cnxt_iofuncs_shutdown API.
 *        
 *        
 *  2    mpeg      1.1         1/3/02 1:38:58 PM      Dave Wilson     SCR(s) 
 *        2996 :
 *        Added new API cnxt_iofuncs_query_input_device_callback()
 *        
 *  1    mpeg      1.0         11/27/01 11:57:26 AM   Dave Wilson     
 * $
 * 
 *    Rev 1.11   31 Jul 2003 14:52:36   joness
 * SCR(s): 5046 
 * Add functionality for redirecting to different osd regions.
 * 
 *    Rev 1.10   18 Jun 2003 14:20:56   dawilson
 * SCR(s): 6782 
 * Changed cnxt_iofuncs_set_aa_indices to remove indices for foreground and
 * background colours. These are already set when the foreground and background
 * colours are set using cnxt_iofuncs_set_foreground/background_color so are not
 * needed here too.
 * 
 *    Rev 1.9   02 Oct 2002 11:28:00   dawilson
 * SCR(s) 4621 :
 * Added code and defintions to support greyscale font rendering.
 * 
 *    Rev 1.8   22 Aug 2002 15:08:28   dawilson
 * SCR(s) 4319 :
 * Added prototype for cnxt_iofuncs_get_bounding_rect function
 * 
 *    Rev 1.7   02 Aug 2002 11:16:50   dawilson
 * SCR(s) 4314 :
 * Added prototype for new API cnxt_iofuncs_get_gfx_handle
 * 
 *    Rev 1.6   19 Jun 2002 09:40:42   dawilson
 * SCR(s) 3993 :
 * Added 2 new function prototypes for cnxt_iofuncs_set_mouse_relative and
 * cnxt_iofuncs_is_mouse_relative. Replaces IOFUNCS_MSG_MOUSE_MOVE with 2 new
 * messages, IOFUNCS_MSG_MOUSE_MOVE_ABSOLUTE/RELATIVE.
 * 
 *    Rev 1.5   06 Mar 2002 11:24:58   dawilson
 * SCR(s) 3315 :
 * Added cnxt_iofuncs_press_any_key function which implements an easy-to-use
 * "Press any key to continue" API.
 * 
 *    Rev 1.4   28 Feb 2002 09:07:58   dawilson
 * SCR(s) 3254 :
 * Added cnxt_iofuncs_screen_display and cnxt_iofuncs_get_font_and_mode API
 * prototypes. Also made old cnxt_iofuncs_osd_show and cnxt_iofuncs_osd_hide
 * into macros using the new cnxt_iofuncs_screen_display function.
 * 
 * 
 * 
 *    Rev 1.4   27 Feb 2002 16:00:54   dawilson
 * SCR(s) 3254 :
 * Added cnxt_iofuncs_screen_display and cnxt_iofuncs_get_font_and_mode API
 * prototypes. Also made old cnxt_iofuncs_osd_show and cnxt_iofuncs_osd_hide
 * into macros using the new cnxt_iofuncs_screen_display function.
 * 
 * 
 * 
 *    Rev 1.4   27 Feb 2002 15:59:28   dawilson
 * SCR(s) 3254 :
 * Added cnxt_iofuncs_screen_display and cnxt_iofuncs_get_font_and_mode API
 * prototypes. Also made old cnxt_iofuncs_osd_show and cnxt_iofuncs_osd_hide
 * into macros using the new cnxt_iofuncs_screen_display function.
 * 
 * 
 * 
 *    Rev 1.4   27 Feb 2002 15:52:04   dawilson
 * SCR(s) 3254 :
 * Added cnxt_iofuncs_screen_display and cnxt_iofuncs_get_font_and_mode API
 * prototypes. Also made old cnxt_iofuncs_osd_show and cnxt_iofuncs_osd_hide
 * into macros using the new cnxt_iofuncs_screen_display function.
 * 
 * 
 * 
 *    Rev 1.4   27 Feb 2002 15:50:44   dawilson
 * SCR(s) 3254 :
 * Added cnxt_iofuncs_screen_display and cnxt_iofuncs_get_font_and_mode API
 * prototypes. Also made old cnxt_iofuncs_osd_show and cnxt_iofuncs_osd_hide
 * into macros using the new cnxt_iofuncs_screen_display function.
 * 
 * 
 * 
 *    Rev 1.4   27 Feb 2002 15:48:54   dawilson
 * SCR(s) 3254 :
 * Added cnxt_iofuncs_screen_display and cnxt_iofuncs_get_font_and_mode API
 * prototypes. Also made old cnxt_iofuncs_osd_show and cnxt_iofuncs_osd_hide
 * into macros using the new cnxt_iofuncs_screen_display function.
 * 
 *    Rev 1.3   07 Jan 2002 14:38:36   dawilson
 * SCR(s) 3002 :
 * Added cnxt_iofuncs_set_mouse_cursor to allow clients to set their own
 * cursor bitmaps.
 * 
 *    Rev 1.2   03 Jan 2002 15:56:06   dawilson
 * SCR(s) 2995 :
 * Added cnxt_iofuncs_shutdown API.
 * 
 * 
 *    Rev 1.1   03 Jan 2002 13:38:58   dawilson
 * SCR(s) 2996 :
 * Added new API cnxt_iofuncs_query_input_device_callback()
 * 
 *    Rev 1.0   27 Nov 2001 11:57:26   dawilson
 * SCR(s) 2927 :
 * 
 *
 ****************************************************************************/

