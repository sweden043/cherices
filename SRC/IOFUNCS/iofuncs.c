/****************************************************************************/
/*                   Conexant Proprietary and Confidential                  */
/*                       Software File/Module Header                        */
/*              Conexant Systems Inc. (c) 1998, 1999, 2000, 2001            */
/*                               Austin, TX                                 */
/*                            All Rights Eeserved                           */
/****************************************************************************/
/*
 * Filename:       IOFUNCS.C
 *
 *
 * Description:    This module contains functions relating to rendering of 
 *                 text on the OSD plane and handling input from remote and 
 *                 front panel.
 *
 * Author:         Dave Wilson
 *
 ****************************************************************************/
/* $Header: iofuncs.c, 27, 11/13/03 3:20:22 PM, Dave Wilson$
 ****************************************************************************/
 
/*****************************************************************************/
/* This module is dependent upon services provided by the following modules: */ 
/*                                                                           */
/* OSDLIBC                                                                   */
/* GFXLIB                                                                    */
/* GXAGFX                                                                    */
/* GENIR + a suitable driver for the IR remote in use                        */
/* SCANBTNS or another compatible front panel button driver                  */
/*                                                                           */
/*****************************************************************************/

#include "stbcfg.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "basetype.h"
#include "kal.h"
#include "retcodes.h"                 
#include "osdlibc.h"
#include "curlib.h"
#include "gfxtypes.h"
#include "gfxlib.h"
#include "gfxutls.h"
#include "gxagfx.h"
#include "iofuncs.h" 
#include "genir.h"
#include "iof_int.h"

/************************/
/* Internal labels, etc */
/************************/
#define STRING_MAX_LENGTH 1023
#define IOFUNCS_KEY_QUEUE_LEN 16
#define PRINTF_BUFFER_SIZE  256
#define CHAR_TIMEOUT_MARKER 0xFF

typedef struct _SCREEN
{
   OSDHANDLE hOSD;
   PGFX_BITMAP pBitmap;
   struct _SCREEN *pNext;
} SCREEN;

/********************/
/* Global Variables */
/********************/

/* To fix the height of OSD region to 576, Steven Shen, 2004-12-9 10:37 */
static int           gMyOsdMaxHeight = 576;

static SCREEN *pScreenList=NULL;
static PGFX_FONT     pDefaultFont;
static GFX_BITMAP    sScreenBitmap;
static PGFX_BITMAP   pCurrentBitmap;
static int           iScreenWidth, iScreenHeight;
static int           iTextCursorX, iTextCursorY;
static int           giCellWidth;
static int           giCellHeight;
static bool          bMouseShown    = FALSE;
static bool          bCursorShown   = FALSE;
static bool          bInitialised   = FALSE;
static bool          bAutoScroll    = FALSE;
static bool          bCharEcho      = TRUE;
static bool          bMouseRelative = FALSE;
static OSDHANDLE     hScreen;
static OSDHANDLE     hCurrent;
static HOSDCUR       hCursor;
static HOSDCUR       hDefaultCursor;
static sem_id_t      semSerialise;
static sem_id_t      semStringSignal;
static sem_id_t      semCursor;
static u_int32       uIofuncsOsdMode;
static OSDRECT       rectWindow;
static OSDRECT       rectOsd;
static OSDRECT       rectWindowPixel;
static IOFUNCS_COLOR colForeground, colBackground;
static PFNINPUTDEVICECALLBACK pfnInputCallback = NULL;
static task_id_t     taskCommand;
static task_id_t     taskCallback;
static char          szStringBuffer[STRING_MAX_LENGTH+1];
static queue_id_t    qCallback;
static u_int8        cAATextIndices[4] = {0, 1, 2, 3};

queue_id_t           qCommand;
int                  iMouseCursorX, iMouseCursorY;
/***********************/
/* Default cursor data */
/***********************/

unsigned char small_arrow_data[] =
{
  0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0xd7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x56, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x54, 0xd5, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x58, 0x55, 0x25, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x50, 0x55, 0x55, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x60, 0x55, 0x55, 0x35, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x40, 0x55, 0x55, 0x55, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x80, 0x55, 0x55, 0x55, 0x95, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x55, 0x55, 0x55, 0x55, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x57, 0x55, 0x55, 0x55, 0xd5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x54, 0x55, 0x55, 0x55, 0x55, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x5c, 0x55, 0x55, 0x55, 0x55, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x50, 0x55, 0x55, 0x55, 0x95, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x70, 0x55, 0x55, 0x55, 0x25, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x60, 0x55, 0x55, 0x55, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x40, 0x55, 0x55, 0x55, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x80, 0x55, 0x55, 0x55, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x55, 0x55, 0x55, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x56, 0x55, 0x55, 0x35, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x54, 0x55, 0x56, 0xd5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x58, 0x95, 0x50, 0x55, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x50, 0x25, 0x40, 0x55, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x70, 0x09, 0x00, 0x55, 0x35, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x40, 0x02, 0x00, 0x54, 0xd5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0x95, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x25, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
};

/***********************/
/* Cursor palette data */
/***********************/

const u_int32 small_arrow_palette[3] =
{
  0x008080ea,
  0x00808058,
  0x008080a1
};

#define SMALL_ARROW_CURSOR_WIDTH  64
#define SMALL_ARROW_CURSOR_HEIGHT 30
#define SMALL_ARROW_INVERT_ENABLED FALSE
#define SMALL_ARROW_INVERT_INDEX   0

/********************************/
/* Internal Function Prototypes */
/********************************/
static IOFUNCS_STATUS internal_vprintf_font_at_pixel(PGFX_FONT pFont, 
                                                     int       x, 
                                                     int       y, 
                                                     char     *strFormat, 
                                                     int      *pNumChars,
                                                     bool      bClip,
                                                     va_list   arg);

static IOFUNCS_STATUS internal_vprintf_font_at_pixel_auto_wrap(PGFX_FONT pFont, 
                                                     int       x, 
                                                     int       y, 
                                                     int		x2,
                                                     int		y2,
                                                     u_int8 calculate,
                                                     u_int8 *end,
                                                     char     *strFormat, 
                                                     int      *pNumChars,
                                                     va_list   arg);

static u_int8 internal_osd_mode_to_gfx_type(u_int32  uMode);
static IOFUNCS_STATUS internal_draw_string(PGFX_FONT pFont, 
                                           int       x, 
                                           int       y, 
                                           char     *szString, 
                                           int       iCount);
static int internal_count_chars_to_end_of_line(GFX_FONT *pFont, 
                                               char     *pString, 
                                               int       iStringLen, 
                                               int       iOutputWidth);
static IOFUNCS_STATUS internal_scroll_window(IOFUNCS_SCROLL eDirection, 
                                             int iNumLines);
static void internal_command_task(void *pIgnored);
static void internal_callback_task(void *pIgnored);
static IOFUNCS_STATUS internal_printf(char *strFormat, ...);
static void internal_tick_callback(tick_id_t hTick, void *pUserData);
static IOFUNCS_STATUS internal_cursor_show(IOFUNCS_CURSOR eCursor, bool bShow);
static bool internal_draw_cursor(int x, int y, bool bState);
static IOFUNCS_STATUS internal_reinit(PGFX_FONT pDefFont, u_int32 uOsdMode);

static bool get_screen(OSDHANDLE hOSD, SCREEN **ppScreen);
static void register_OSD_handle(OSDHANDLE hOSD, PGFX_BITMAP pBitmap);
static PGFX_BITMAP get_bitmap(OSDHANDLE hOSD);

static IOFUNCS_STATUS internal_post_callback_message(IOFUNCS_MESSAGE msg, u_int32 p1, u_int32 p2);

#ifdef DEBUG
static void internal_debug_dump_msg(IOFUNCS_MESSAGE msg, u_int32 p1, u_int32 p2, u_int32 p3);
char *internal_debug_map_message_to_string(IOFUNCS_MESSAGE msg);
#endif

/************************************/
/************************************/
/** Initialisation and OSD control **/
/************************************/
/************************************/

/********************************************************************/
/*  FUNCTION:    cnxt_iofuncs_init                                  */
/*                                                                  */
/*  PARAMETERS:  pDefFont - pointer to the default font to use. If  */
/*                          NULL, 8x8 is assumed.                   */
/*               uOsdMode - Mode to use for IOFUNCS OSD region.     */
/*                                                                  */
/*  DESCRIPTION: This function initialises the IOFUNCS library and  */
/*               creates the OSD region it will use. It also hooks  */
/*               the front panel and IR input. Screen grid sizes    */
/*               are fixed based on the choice of default font.     */
/*                                                                  */
/*  RETURNS:     IOFUNCS_STATUS_OK     if no errors occurred        */
/*               IOFUNCS_STATUS_REINIT if the module was already    */
/*                                     initialised                  */
/*               IOFUNCS_ERROR_BAD_MODE if the OSD mode passed is   */
/*                                      not supported.              */
/*                                                                  */
/*  CONTEXT:     Must be called from a non-interrupt context.       */
/*                                                                  */
/********************************************************************/
IOFUNCS_STATUS cnxt_iofuncs_init(PGFX_FONT pDefFont, u_int32 uOsdMode)
{
  u_int32 uOptions;
  u_int8  cGfxType;
  IOFUNCS_STATUS eRetcode = IOFUNCS_STATUS_OK;
  int     iRetcode;
  
  trace_new(TR_FUNC, "IOFUNCS: cnxt_iofuncs_init\n");
  
  #ifdef DEBUG
  not_interrupt_safe();
  #endif

  if (bInitialised)
  {
    /* If we are already initialised, reinitialise with the new parameters */
    iRetcode = sem_get(semSerialise, KAL_WAIT_FOREVER);
    
    if (iRetcode == RC_OK)
    {
      eRetcode = internal_reinit(pDefFont, uOsdMode);
            
      sem_put(semSerialise);
    }
    else
    {
      trace_new(TR_ERR, "IOFUNCS: Can't get access semaphore on reinit\n");
      eRetcode = IOFUNCS_ERROR_INTERNAL;
    }  
      
    return(eRetcode);
  }
  else
  {
    /* Check parameters */
    
    cGfxType = internal_osd_mode_to_gfx_type(uOsdMode);
    if (cGfxType == GFX_MONO)
    {
      trace_new(TR_ERR, "IOFUNCS: Unsupported OSD mode 0x%x requested.\n", uOsdMode);
      return(IOFUNCS_ERROR_BAD_MODE);
    }
    else
      uIofuncsOsdMode = uOsdMode;
    
    /* Set default font */
    if(pDefFont)
      pDefaultFont = pDefFont;
    else
      pDefaultFont = &font_8x8;
        
        
    /* Set screen cell dimensions */
    GfxGetFontCellSize(pDefaultFont, &giCellWidth, &giCellHeight);
      
    iScreenWidth  = gnOsdMaxWidth/giCellWidth;
    iScreenHeight = gMyOsdMaxHeight/giCellHeight;
       
    /* Create the serialisation semaphore */
    semSerialise = sem_create(1, "IOSS");
    if (semSerialise == (sem_id_t)NULL)
    {
      trace_new(TR_ERR, "IOFUNCS: Can't create serialisation semaphore!\n");
      return(IOFUNCS_ERROR_INTERNAL);
    }  
    
    /* Create the string capture signal semaphore */
    semStringSignal = sem_create(0, "IOSC");
    if (semSerialise == (sem_id_t)NULL)
    {
      trace_new(TR_ERR, "IOFUNCS: Can't create serialisation semaphore!\n");
      sem_delete(semSerialise);
      return(IOFUNCS_ERROR_INTERNAL);
    }  
    
    /* Create the cursor control signal semaphore */
    semCursor = sem_create(0, "IOCS");
    if (semCursor == (sem_id_t)NULL)
    {
      trace_new(TR_ERR, "IOFUNCS: Can't create cursor signal semaphore!\n");
      sem_delete(semSerialise);
      sem_delete(semStringSignal);
      return(IOFUNCS_ERROR_INTERNAL);
    }  
    
    /* Create the command queue */
    qCommand = qu_create(IOFUNCS_KEY_QUEUE_LEN, "IOKQ");
    if (qCommand == (queue_id_t)NULL)
    {
      trace_new(TR_ERR, "IOFUNCS: Can't create serialisation semaphore!\n");
      sem_delete(semSerialise);
      sem_delete(semStringSignal);
      sem_delete(semCursor);
      return(IOFUNCS_ERROR_INTERNAL);
    }
 
    /* Create our full screen OSD region */
    rectOsd.top    = 0;
    rectOsd.left   = 0;
    rectOsd.bottom = gMyOsdMaxHeight;
    rectOsd.right  = gnOsdMaxWidth;

    /* Set the palette load option if not a 32bpp mode */
    if(uOsdMode != OSD_MODE_32AYUV)
      uOptions = OSD_RO_LOADPALETTE;
    else
      uOptions = 0;
        
    uOptions |= (OSD_RO_ALPHABOTHVIDEO | OSD_RO_ALPHAENABLE);  
    
    hScreen = CreateOSDRgn(&rectOsd, 
                           uOsdMode, 
                           uOptions, 
                           NULL, 
                           0);
    
    if (!hScreen)
    {
      trace_new(TR_ERR, "IOFUNCS: Unable to create requested OSD region!\n");
      qu_destroy(qCommand);
      sem_delete(semSerialise);
      sem_delete(semStringSignal);
      sem_delete(semCursor);
      return(IOFUNCS_ERROR_INTERNAL);
    }

    /* Set up a GFX bitmap which represents the screen OSD region */
    cnxt_iofuncs_associate_bitmap(&sScreenBitmap,hScreen);
    pCurrentBitmap=&sScreenBitmap;
    hCurrent=hScreen; /* Use the default handle */

    /* Create the mouse cursor */
    hDefaultCursor = curCreateCursorNative(small_arrow_data,
                                          (LPREG)small_arrow_palette,
                                          SMALL_ARROW_CURSOR_HEIGHT,
                                          0,0,
                                          SMALL_ARROW_INVERT_ENABLED,
                                          SMALL_ARROW_INVERT_INDEX);
    hCursor = hDefaultCursor;
                                    
    /* Set initial cursor positions */
    iTextCursorX = 0;
    iTextCursorY = 0;
    iMouseCursorX = gnOsdMaxWidth/2;
    iMouseCursorY = gMyOsdMaxHeight/2;
    
    /* Set the initial display window */
    rectWindow.bottom = gMyOsdMaxHeight / giCellWidth;
    rectWindow.right  = gnOsdMaxWidth  / giCellWidth;
    rectWindow.top    = 0;
    rectWindow.left   = 0;
    
    rectWindowPixel.bottom = gMyOsdMaxHeight;
    rectWindowPixel.right  = gnOsdMaxWidth;
    rectWindowPixel.top    = 0;
    rectWindowPixel.left   = 0;
    
    /* Initialise the IR remote and front panel keyboard */
    
    bInitialised = input_devices_init();
    if (!bInitialised)
    {
      trace_new(TR_ERR, "IOFUNCS: Unable to initialise input handler!\n");
      qu_destroy(qCommand);
      sem_delete(semSerialise);
      sem_delete(semStringSignal);
      sem_delete(semCursor);
      bInitialised = FALSE;
      return(IOFUNCS_ERROR_INTERNAL);
    }  
    
    /* Create the callback queue */
    qCallback = qu_create(IOFUNCS_KEY_QUEUE_LEN, "IOCQ");
    if (qCallback == (queue_id_t)NULL)
    {
      trace_new(TR_ERR, "IOFUNCS: Can't create callback queue!\n");
      qu_destroy(qCommand);
      sem_delete(semSerialise);
      sem_delete(semStringSignal);
      sem_delete(semCursor);
      return(IOFUNCS_ERROR_INTERNAL);
    }
    
    /* Start the command task to handle keystrokes, cursor and mouse */
    taskCommand = task_create(internal_command_task,
                              NULL,
                              NULL,
                              IFCT_TASK_STACK_SIZE,
                              IFCT_TASK_PRIORITY,
                              IFCT_TASK_NAME);
    if (taskCommand == (task_id_t)0)
    {
      trace_new(TR_ERR, "IOFUNCS: Unable to create command task!\n");
      qu_destroy(qCommand);
      sem_delete(semSerialise);
      sem_delete(semStringSignal);
      sem_delete(semCursor);
      bInitialised = FALSE;
      return(IOFUNCS_ERROR_INTERNAL);
    }  
    
    /* Start the callback task to handle keystrokes, cursor and mouse events to client*/
    taskCallback = task_create(internal_callback_task,
                               NULL,
                               NULL,
                               IFCB_TASK_STACK_SIZE,
                               IFCB_TASK_PRIORITY,
                               IFCB_TASK_NAME);
    if (taskCallback == (task_id_t)0)
    {
      trace_new(TR_ERR, "IOFUNCS: Unable to create callback task!\n");
      qu_destroy(qCallback);
      qu_destroy(qCommand);
      sem_delete(semSerialise);
      sem_delete(semStringSignal);
      sem_delete(semCursor);
      bInitialised = FALSE;
      return(IOFUNCS_ERROR_INTERNAL);
    }  
  }  
  
  return(eRetcode);
}

/********************************************************************/
/*  FUNCTION:    cnxt_input_init                                  */
/*                                                                  */
/*  DESCRIPTION: This function initialises the IOFUNCS library */
/*                It also hooks  the front panel and IR input.*/
 /*                                                                  */
/*  RETURNS:     IOFUNCS_STATUS_OK     if no errors occurred        */
/*               IOFUNCS_STATUS_REINIT if the module was already    */
/*                                     initialised                  */
/*               IOFUNCS_ERROR_BAD_MODE if the OSD mode passed is   */
/*                                      not supported.              */
/*                                                                  */
/*  CONTEXT:     Must be called from a non-interrupt context.       */
/*                                                                  */
/********************************************************************/
IOFUNCS_STATUS  cnxt_input_init(void)
{
	u_int32 uOptions;
	u_int8  cGfxType;
	IOFUNCS_STATUS eRetcode = IOFUNCS_STATUS_OK;
	int     iRetcode;

	trace_new(TR_FUNC, "IOFUNCS: cnxt_input_init \n");

#ifdef DEBUG
	not_interrupt_safe();
#endif		

	/* Create the serialisation semaphore */
	semSerialise = sem_create(1, "IOSS");
	if (semSerialise == (sem_id_t)NULL)
	{
		trace_new(TR_ERR, "IOFUNCS: Can't create serialisation semaphore!\n");
		return(IOFUNCS_ERROR_INTERNAL);
	}  

	/* Create the string capture signal semaphore */
	semStringSignal = sem_create(0, "IOSC");
	if (semStringSignal == (sem_id_t)NULL)
	{
		trace_new(TR_ERR, "IOFUNCS: Can't create serialisation semaphore!\n");
		sem_delete(semSerialise);
		return(IOFUNCS_ERROR_INTERNAL);
	}  

	/* Create the cursor control signal semaphore */
	semCursor = sem_create(0, "IOCS");
	if (semCursor == (sem_id_t)NULL)
	{
		trace_new(TR_ERR, "IOFUNCS: Can't create cursor signal semaphore!\n");
		sem_delete(semSerialise);
		sem_delete(semStringSignal);
		return(IOFUNCS_ERROR_INTERNAL);
	}  

	/* Create the command queue */
	qCommand = qu_create(IOFUNCS_KEY_QUEUE_LEN, "IOKQ");
	if (qCommand == (queue_id_t)NULL)
	{
		trace_new(TR_ERR, "IOFUNCS: Can't create serialisation semaphore!\n");
		sem_delete(semSerialise);
		sem_delete(semStringSignal);
		sem_delete(semCursor);
		return(IOFUNCS_ERROR_INTERNAL);
	}	
	
	
	/* Initialise the IR remote and front panel keyboard */
	bInitialised = input_devices_init();
	if (!bInitialised)
	{
		trace_new(TR_ERR, "IOFUNCS: Unable to initialise input handler!\n");
		qu_destroy(qCommand);
		sem_delete(semSerialise);
		sem_delete(semStringSignal);
		sem_delete(semCursor);
		bInitialised = FALSE;
		return(IOFUNCS_ERROR_INTERNAL);
	}  

	/* Create the callback queue */
	qCallback = qu_create(IOFUNCS_KEY_QUEUE_LEN, "IOCQ");
	if (qCallback == (queue_id_t)NULL)
	{
		trace_new(TR_ERR, "IOFUNCS: Can't create callback queue!\n");
		qu_destroy(qCommand);
		sem_delete(semSerialise);
		sem_delete(semStringSignal);
		sem_delete(semCursor);
		return(IOFUNCS_ERROR_INTERNAL);
	}

	/* Start the command task to handle keystrokes, cursor and mouse */
	taskCommand = task_create(internal_command_task,
								NULL,
								NULL,
								IFCT_TASK_STACK_SIZE,
								IFCT_TASK_PRIORITY,
								IFCT_TASK_NAME);
	if (taskCommand == (task_id_t)0)
	{
		trace_new(TR_ERR, "IOFUNCS: Unable to create command task!\n");
		qu_destroy(qCommand);
		sem_delete(semSerialise);
		sem_delete(semStringSignal);
		sem_delete(semCursor);
		bInitialised = FALSE;
		return(IOFUNCS_ERROR_INTERNAL);
	}  

	/* Start the callback task to handle keystrokes, cursor and mouse events to client*/
	taskCallback = task_create(internal_callback_task,
								NULL,
								NULL,
								IFCB_TASK_STACK_SIZE,
								IFCB_TASK_PRIORITY,
								IFCB_TASK_NAME);
	if (taskCallback == (task_id_t)0)
	{
		trace_new(TR_ERR, "IOFUNCS: Unable to create callback task!\n");
		qu_destroy(qCallback);
		qu_destroy(qCommand);
		sem_delete(semSerialise);
		sem_delete(semStringSignal);
		sem_delete(semCursor);
		bInitialised = FALSE;
		return(IOFUNCS_ERROR_INTERNAL);
	} 

	return(eRetcode);
}

/********************************************************************/
/*  FUNCTION:    cnxt_input_exit                              */
/*                                                                  */
/*  PARAMETERS:  None                                               */
/*                                                                  */
/*  DESCRIPTION: End use of IOFUNCS services and destroy all the    */
/*               resources used by the module. After calling this   */
/*               function, cnxt_iofuncs_init must be called again   */
/*               to restart the module.                             */
/*                                                                  */
/*  RETURNS:     IOFUNCS_STATUS_OK      if no errors occurred       */
/*               IOFUNCS_ERROR_NOT_INIT if the module was not       */
/*                                      previously initialised      */
/*               IOFUNCS_ERROR_INTERNAL if an internal error        */
/*                                      occurred.                   */
/*                                                                  */
/*  CONTEXT:     Must be called from a non-interrupt context.       */
/*                                                                  */
/********************************************************************/
IOFUNCS_STATUS cnxt_input_exit(void)
{
	int iRetcode;
	IOFUNCS_STATUS eRetcode = IOFUNCS_STATUS_OK;
	sem_id_t semShutdown;

	trace_new(TR_FUNC, "IOFUNCS: cnxt_iofuncs_shutdown\n");

#ifdef DEBUG
	not_interrupt_safe();
#endif

	iRetcode = sem_get(semSerialise, KAL_WAIT_FOREVER);
	if (iRetcode == RC_OK)
	{
		/* Shut things down here. Be careful how we handle the access */
		/* semaphore which we currently hold.                         */
		/* First tell our other tasks to shut down gracefully */
		/* Create a temporary signalling semaphore */
		semShutdown = sem_create(0, NULL);

		/* Tell the command task to die */
		internal_post_command_message(IOFUNCS_MSG_SHUTDOWN, semShutdown, 0, 0);

		/* Wait for this to complete if the semaphore is OK */
		if(semShutdown)
			sem_get(semShutdown, 5000);

		/* Now tell the callback task to die */
		internal_post_callback_message(IOFUNCS_MSG_SHUTDOWN, semShutdown, 0);

		/* Wait for this to complete if the semaphore is OK */
		if(semShutdown)
			sem_get(semShutdown, 5000);

		/* Destroy our temporary semaphore */
		if(semShutdown)
			sem_delete(semShutdown);

		/* Now destroy other resources created during initialisation */
		qu_destroy(qCallback);
		qu_destroy(qCommand);
		sem_delete(semStringSignal);
		sem_delete(semCursor);	
		
		bInitialised    = FALSE;
		sem_delete(semSerialise);
	}
	else
	{
		trace_new(TR_ERR, "IOFUNCS: Failed to get module access semaphore! rc = 0x%08x\n", iRetcode);
		eRetcode = IOFUNCS_ERROR_INTERNAL;
	}
	
	return(eRetcode);
}

/********************************************************************/
/*  FUNCTION:    get_screen                                         */
/*                                                                  */
/*  PARAMETERS:  hOSD       OSD handle whose SCREEN structure is to */
/*                          be found.                               */
/*               ppScreen   Room for the pointer to the screen to be*/
/*                          passed back to the calling function.    */
/*                                                                  */
/*  DESCRIPTION: Navigates the linked list of SCREEN structures,    */
/*               looking for the SCREEN associated with hOSD.       */
/*               A SCREEN is what links an OSD back to its bitmap.  */
/*                                                                  */
/*  RETURNS:     TRUE       The screen was found.                   */
/*               FALSE      The screen was not found and probably   */
/*                          needs to be created.                    */
/*                                                                  */
/*  CONTEXT:     Any.                                               */
/*                                                                  */
/********************************************************************/
static bool get_screen(OSDHANDLE hOSD, SCREEN **ppScreen)
{
   SCREEN *pCurrent=pScreenList, *pLast=NULL;
   bool bFinished=FALSE, bRetVal=FALSE;

   while (bFinished==FALSE)
   {
      if (pCurrent!=(SCREEN *)NULL)
      {
         if (pCurrent->hOSD==hOSD)
         {
            bRetVal=bFinished=TRUE; /* We've found it, it's already registered */
            *ppScreen=pCurrent;
         }
         else
         {
            pLast=pCurrent;
            pCurrent=pCurrent->pNext;
         }
      }
      else
      {
         bFinished=TRUE; /* We ran off the end of the list */
                         /* - pass back the place to put next member of list */
         if (pLast!=(SCREEN *)NULL)
         {
            /* pLast is our thing */
            *ppScreen=pLast;
            trace_new(TR_INFO,"IOFUNCS: In get_screen, %i is yet to be registered.\n",(int)hOSD);
         }
         else
         {
            /* We get here the first time we register a handle */
            *ppScreen=(SCREEN *)NULL;
            trace_new(TR_INFO,"IOFUNCS: In get_screen, %i is first handle to be registered.\n",(int)hOSD);
         }
      }
   }
   return bRetVal;
}

/********************************************************************/
/*  FUNCTION:    register_OSD_handle                                */
/*                                                                  */
/*  PARAMETERS:  hOSD       Will be linked to pBitmap.              */
/*               pBitmap    While cnxt_iofuncs_associate_bitmap     */
/*                          fixes properties between the two to be  */
/*                          the same, this creates an entry in the  */
/*                          SCREEN linked list. It uses get_screen  */
/*                          to check for an existing entry and,     */
/*                          on failing to find one, to find the end */
/*                          of the list.                            */
/*                                                                  */
/*  DESCRIPTION: This function is used to simplify the use of OSD   */
/*               regions with a separate bitmap structure. The OSD  */
/*               region will be used as an argument to osdlibc      */
/*               functions while the bitmap will be passed to gfx   */
/*               functions. They share a pixel area.                */
/*                                                                  */
/*  RETURNS:     None.                                              */
/*                                                                  */
/*  CONTEXT:     Any.                                               */
/*                                                                  */
/********************************************************************/
static void register_OSD_handle(OSDHANDLE hOSD, PGFX_BITMAP pBitmap)
{
   SCREEN *pScreen;
   if (get_screen(hOSD,&pScreen)==FALSE) /* Hasn't already been registered */
   {
      /* pScreen now points to last one in list or is NULL if the structure is empty */
      if (pScreen == (SCREEN *)NULL)
      {
         /* We get here the first time we register a handle */
         pScreen=pScreenList=mem_malloc(sizeof(SCREEN));
         trace_new(TR_INFO,"IOFUNCS: register_OSD_handle making first link in list for %i.\n",(int)hOSD);
      }
      else
      {
         pScreen->pNext=mem_malloc(sizeof(SCREEN));
         pScreen=pScreen->pNext; /* Now pointing at 'our' structure */
         trace_new(TR_INFO,"IOFUNCS: register_OSD_handle adding on to list for %i.\n",(int)hOSD);
      }
      pScreen->pNext=NULL;
      pScreen->pBitmap=pBitmap;
      pScreen->hOSD=hOSD;
   }
   /* else { If the handle is actually already registered, do nothing } */
}

/********************************************************************/
/*  FUNCTION:    get_bitmap                                         */
/*                                                                  */
/*  PARAMETERS:  hOSD  This OSD region's entry in the SCREEN linked */
/*                     list will be found and its corresponding     */
/*                     bitmap structure will be returned.           */
/*                                                                  */
/*  DESCRIPTION: This function is used to simplify the use of OSD   */
/*               regions with a separate bitmap structure. The      */
/*               bitmap structure is required as the argument to gfx*/
/*               functions and this is the function which finds it, */
/*               given the relevant OSD region handle               */
/*                                                                  */
/*  RETURNS:     The pointer to the bitmap structure required.      */
/*                                                                  */
/*  CONTEXT:     Any.                                               */
/*                                                                  */
/********************************************************************/
static PGFX_BITMAP get_bitmap(OSDHANDLE hOSD)
{
   PGFX_BITMAP pRetVal=(PGFX_BITMAP)NULL;
   SCREEN *pScreen;
   if (get_screen(hOSD, &pScreen))
   {
      /* Don't dereference pScreen if it's NULL! */
      /* It shouldn't be if we got here (TRUE return from get_screen), but check again */
      pRetVal=(pScreen==(SCREEN *)NULL)?(PGFX_BITMAP)NULL:pScreen->pBitmap;
   }
   return pRetVal; /* NULL if the OSD handle has not been registered */
}

/********************************************************************/
/*  FUNCTION:    cnxt_iofuncs_associate_bitmap                      */
/*                                                                  */
/*  PARAMETERS:  pBitmap    ...will assume relevant properties of...*/
/*               oHandle    The OSD region handle. They will also be*/
/*                          tied together in the SCREEN structure   */
/*                          linked list.                            */
/*                                                                  */
/*  DESCRIPTION: This function is used to simplify the use of OSD   */
/*               regions with a separate bitmap structure. The OSD  */
/*               region will be used as an argument to osdlibc      */
/*               functions while the bitmap will be passed to gfx   */
/*               functions. They share a pixel area.                */
/*                                                                  */
/*  RETURNS:     IOFUNCS_STATUS_OK     if no errors occurred        */
/*                                                                  */
/*  CONTEXT:     Any.                                               */
/*                                                                  */
/********************************************************************/
IOFUNCS_STATUS cnxt_iofuncs_associate_bitmap(GFX_BITMAP *pBitmap, OSDHANDLE oHandle)
{
    trace_new(TR_FUNC, "IOFUNCS: cnxt_iofuncs_associate_bitmap\n");
    register_OSD_handle(oHandle, pBitmap);
    trace_new(TR_INFO,"IOFUNCS: Registering handle %i with bitmap %i.\n",(int)oHandle,(int)pBitmap);
    pBitmap->Height = (u_int16)GetOSDRgnOptions(oHandle,OSD_RO_BUFFERHEIGHT);
    pBitmap->Width = (u_int16)GetOSDRgnOptions(oHandle,OSD_RO_BUFFERWIDTH);
    pBitmap->Type = internal_osd_mode_to_gfx_type(GetOSDRgnOptions(oHandle,OSD_RO_MODE));
    pBitmap->Version = 0x0001;
    pBitmap->VerSize = sizeof(GFX_BITMAP);
    pBitmap->Bpp     = GetOSDRgnBpp(oHandle);
    pBitmap->Stride  = (u_int16)GetOSDRgnOptions(oHandle, OSD_RO_FRAMESTRIDE);
    pBitmap->dwRef   = (u_int32)oHandle;
    pBitmap->pPalette = (void *)GetOSDRgnOptions(oHandle, OSD_RO_PALETTEADDRESS);
    pBitmap->pBits    = (void *)GetOSDRgnOptions(oHandle, OSD_RO_FRAMEBUFFER);
    return IOFUNCS_STATUS_OK;
}

/********************************************************************/
/*  FUNCTION:    cnxt_iofuncs_shutdown                              */
/*                                                                  */
/*  PARAMETERS:  None                                               */
/*                                                                  */
/*  DESCRIPTION: End use of IOFUNCS services and destroy all the    */
/*               resources used by the module. After calling this   */
/*               function, cnxt_iofuncs_init must be called again   */
/*               to restart the module.                             */
/*                                                                  */
/*  RETURNS:     IOFUNCS_STATUS_OK      if no errors occurred       */
/*               IOFUNCS_ERROR_NOT_INIT if the module was not       */
/*                                      previously initialised      */
/*               IOFUNCS_ERROR_INTERNAL if an internal error        */
/*                                      occurred.                   */
/*                                                                  */
/*  CONTEXT:     Must be called from a non-interrupt context.       */
/*                                                                  */
/********************************************************************/
IOFUNCS_STATUS cnxt_iofuncs_shutdown(void)
{
  int iRetcode;
  IOFUNCS_STATUS eRetcode = IOFUNCS_STATUS_OK;
  sem_id_t semShutdown;
  
  trace_new(TR_FUNC, "IOFUNCS: cnxt_iofuncs_shutdown\n");
  
  #ifdef DEBUG
  not_interrupt_safe();
  #endif

  if (!bInitialised)
  {
    trace_new(TR_ERR, "IOFUNCS: Module not initialised before use\n");
    eRetcode = IOFUNCS_ERROR_NOT_INIT;
  }
  else
  {
    iRetcode = sem_get(semSerialise, KAL_WAIT_FOREVER);
    
    if (iRetcode == RC_OK)
    {
      /* Shut things down here. Be careful how we handle the access */
      /* semaphore which we currently hold.                         */
      
      /* First tell our other tasks to shut down gracefully */
      
      /* Create a temporary signalling semaphore */
      semShutdown = sem_create(0, NULL);
      
      /* Tell the command task to die */
      internal_post_command_message(IOFUNCS_MSG_SHUTDOWN, semShutdown, 0, 0);
      
      /* Wait for this to complete if the semaphore is OK */
      if(semShutdown)
        sem_get(semShutdown, 5000);
      
      /* Now tell the callback task to die */
      internal_post_callback_message(IOFUNCS_MSG_SHUTDOWN, semShutdown, 0);
      
      /* Wait for this to complete if the semaphore is OK */
      if(semShutdown)
        sem_get(semShutdown, 5000);
      
      /* Destroy our temporary semaphore */
      if(semShutdown)
        sem_delete(semShutdown);
        
      /* Now destroy other resources created during initialisation */
      qu_destroy(qCallback);
      qu_destroy(qCommand);
      sem_delete(semStringSignal);
      sem_delete(semCursor);

      /* Destroy the mouse cursor */
      curDeleteCursor(hDefaultCursor);
      
      /* Destroy our OSD region */
      DestroyOSDRegion(hScreen);
      
      /* The tricky bit - we have a potential (though minor) race condition */
      /* in destroying the access semaphore and setting bInitialised. If we */
      /* do it this way, calls to most of the APIs will fail correctly if   */
      /* they overlap this call. A call to cnxt_iofuncs_init right here may */
      /* result in a KAL error trying to recreate the serialisation sem but */
      /* the chances of this are pretty slight so...                        */
       
      pDefaultFont    = NULL;
      uIofuncsOsdMode = 0; 
      bInitialised    = FALSE;
     
      sem_delete(semSerialise);
    }
    else
    {
      trace_new(TR_ERR, "IOFUNCS: Failed to get module access semaphore! rc = 0x%08x\n", iRetcode);
      eRetcode = IOFUNCS_ERROR_INTERNAL;
    }
  }
  return(eRetcode);
}

/********************************************************************/
/*  FUNCTION:    cnxt_iofuncs_get_osd_handle                        */
/*                                                                  */
/*  PARAMETERS:  pHandle - pointer to storage for returned OSD      */
/*                         handle.                                  */
/*                                                                  */
/*  DESCRIPTION: Query the handle of the OSD region that IOFUNCS    */
/*               created. This allows a client to draw its own      */
/*               UI elements into areas of the OSD outside the      */
/*               current IOFUNCS display window (areas which are    */
/*               unaffected by scrolling, for example).             */
/*                                                                  */
/*  RETURNS:     IOFUNCS_STATUS_OK      if no errors occurred       */
/*               IOFUNCS_ERROR_BAD_PTR  NULL pointer passed         */
/*               IOFUNCS_ERROR_INTERNAL if an internal function     */
/*                                      call failed.                */
/*               IOFUNCS_ERROR_NOT_INIT if the module has not been  */
/*                                      initialised.                */
/*               IOFUNCS_ERROR_NO_PALETTE if called when the OSD is */
/*                                      set to a 32bpp mode.        */
/*                                                                  */
/*  CONTEXT:     Must be called from a non-interrupt context.       */
/*                                                                  */
/********************************************************************/
IOFUNCS_STATUS cnxt_iofuncs_get_osd_handle(OSDHANDLE *pHandle)
{
  int iRetcode;
  IOFUNCS_STATUS eRetcode = IOFUNCS_STATUS_OK;
  
  trace_new(TR_FUNC, "IOFUNCS: cnxt_iofuncs_get_osd_handle\n");
  
  #ifdef DEBUG
  not_interrupt_safe();
  #endif

  if (!bInitialised)
  {
    trace_new(TR_ERR, "IOFUNCS: Module not initialised before use\n");
    eRetcode = IOFUNCS_ERROR_NOT_INIT;
  }
  else
  {
    iRetcode = sem_get(semSerialise, KAL_WAIT_FOREVER);
    
    if (iRetcode == RC_OK)
    {
      if (pHandle)
      {
        *pHandle = hScreen;
      }
      else
      {
        trace_new(TR_ERR, "IOFUNCS: NULL pointer passed!\n");
        eRetcode = IOFUNCS_ERROR_BAD_PTR;
      }  
          
      sem_put(semSerialise);
    }
    else
    {
      trace_new(TR_ERR, "IOFUNCS: Failed to get module access semaphore! rc = 0x%08x\n", iRetcode);
      eRetcode = IOFUNCS_ERROR_INTERNAL;
    }
  }
  return(eRetcode);
}

/********************************************************************/
/*  FUNCTION:    cnxt_iofuncs_change_OSD                            */
/*                                                                  */
/*  PARAMETERS:  hNew - Handle to OSD region which iofuncs is now   */
/*               to operate on.                                     */
/*                                                                  */
/*  DESCRIPTION: Make Iofuncs modify a new OSD region.              */
/*                                                                  */
/*  RETURNS:     IOFUNCS_STATUS_OK      if no errors occurred       */
/*               IOFUNCS_ERROR_INTERNAL if an error, normally       */
/*                                      because hNew has not        */
/*                                      been registered using       */
/*                                      register_OSD_handle. This is*/
/*                                      called for you via          */
/*                                     cnxt_iofuncs_associate_bitmap*/
/*                                                                  */
/*  CONTEXT:     Any.                                               */
/*                                                                  */
/********************************************************************/
IOFUNCS_STATUS cnxt_iofuncs_change_OSD(OSDHANDLE hNew)
{
   PGFX_BITMAP pTemp=get_bitmap(hNew);
   IOFUNCS_STATUS iosRetVal=IOFUNCS_ERROR_INTERNAL;
   trace_new(TR_FUNC, "IOFUNCS: cnxt_iofuncs_change_OSD\n");
   trace_new(TR_INFO,"IOFUNCS: Changing to OSD %i, got bitmap %i.\n",(int)hNew,(int)pTemp);
   if (pTemp!=NULL)
   {
      pCurrentBitmap=pTemp;
      iosRetVal=IOFUNCS_STATUS_OK;
      hCurrent=hNew;
   }
   else
   {
      trace_new(TR_ERR,"IOFUNCS: Failed to switch to OSD %i - maybe has not been registered?\n",(int)hNew);
   }
   return iosRetVal;
}

/********************************************************************/
/*  FUNCTION:    cnxt_iofuncs_restore_OSD                           */
/*                                                                  */
/*  PARAMETERS:  None.                                              */
/*                                                                  */
/*  DESCRIPTION: Restore Iofuncs to modifying its own default OSD.  */
/*                                                                  */
/*  RETURNS:     IOFUNCS_STATUS_OK      if no errors occurred       */
/*               IOFUNCS_ERROR_INTERNAL if an error, should never   */
/*                                      happen.                     */
/*                                                                  */
/*  CONTEXT:     Any.                                               */
/*                                                                  */
/********************************************************************/
IOFUNCS_STATUS cnxt_iofuncs_restore_OSD(void)
{
   /* This is just an OSD change back to the default OSD */
   return (cnxt_iofuncs_change_OSD(hScreen));
}

/********************************************************************/
/*  FUNCTION:    cnxt_iofuncs_get_gfx_bitmap                        */
/*                                                                  */
/*  PARAMETERS:  ppBitmap - pointer to storage for returned OSD     */
/*                          handle.                                 */
/*                                                                  */
/*  DESCRIPTION: Query the handle of the OSD region that IOFUNCS    */
/*               created. This allows a client to draw its own      */
/*               UI elements into areas of the OSD outside the      */
/*               current IOFUNCS display window (areas which are    */
/*               unaffected by scrolling, for example).             */
/*                                                                  */
/*  RETURNS:     IOFUNCS_STATUS_OK      if no errors occurred       */
/*               IOFUNCS_ERROR_BAD_PTR  NULL pointer passed         */
/*               IOFUNCS_ERROR_INTERNAL if an internal function     */
/*                                      call failed.                */
/*               IOFUNCS_ERROR_NOT_INIT if the module has not been  */
/*                                      initialised.                */
/*               IOFUNCS_ERROR_NO_PALETTE if called when the OSD is */
/*                                      set to a 32bpp mode.        */
/*                                                                  */
/*  CONTEXT:     Must be called from a non-interrupt context.       */
/*                                                                  */
/********************************************************************/
IOFUNCS_STATUS cnxt_iofuncs_get_gfx_bitmap(GFX_BITMAP **ppBitmap)
{
  int iRetcode;
  IOFUNCS_STATUS eRetcode = IOFUNCS_STATUS_OK;
  
  trace_new(TR_FUNC, "IOFUNCS: cnxt_iofuncs_get_gfx_bitmap\n");
  
  #ifdef DEBUG
  not_interrupt_safe();
  #endif

  if (!bInitialised)
  {
    trace_new(TR_ERR, "IOFUNCS: Module not initialised before use\n");
    eRetcode = IOFUNCS_ERROR_NOT_INIT;
  }
  else
  {
    iRetcode = sem_get(semSerialise, KAL_WAIT_FOREVER);
    
    if (iRetcode == RC_OK)
    {
      if (ppBitmap)
      {
        *ppBitmap = &sScreenBitmap;
      }
      else
      {
        trace_new(TR_ERR, "IOFUNCS: NULL pointer passed!\n");
        eRetcode = IOFUNCS_ERROR_BAD_PTR;
      }  
          
      sem_put(semSerialise);
    }
    else
    {
      trace_new(TR_ERR, "IOFUNCS: Failed to get module access semaphore! rc = 0x%08x\n", iRetcode);
      eRetcode = IOFUNCS_ERROR_INTERNAL;
    }
  }
  return(eRetcode);
}

/********************************************************************/
/*  FUNCTION:    cnxt_iofuncs_get_font_and_mode                     */
/*                                                                  */
/*  PARAMETERS:  ppDefaultFont - ptr to storage for default font    */
/*               pMode         - ptr to storage for OSD mode        */
/*                                                                  */
/*  DESCRIPTION: Query the current default font and OSD mode in use */
/*               by IOFUNCS. This allows a client to restore the    */
/*               state of IOFUNCS if it needs to reset the OSD or   */
/*               font for a period of time.                         */
/*                                                                  */
/*  RETURNS:     IOFUNCS_STATUS_OK      if no errors occurred       */
/*               IOFUNCS_ERROR_BAD_PTR  NULL pointer passed         */
/*               IOFUNCS_ERROR_INTERNAL if an internal function     */
/*                                      call failed.                */
/*               IOFUNCS_ERROR_NOT_INIT if the module has not been  */
/*                                      initialised.                */
/*                                                                  */
/*  CONTEXT:     Must be called from a non-interrupt context.       */
/*                                                                  */
/********************************************************************/
IOFUNCS_STATUS cnxt_iofuncs_get_font_and_mode(PGFX_FONT *ppDefaultFont, u_int32 *pMode) 
{
  int iRetcode;
  IOFUNCS_STATUS eRetcode = IOFUNCS_STATUS_OK;
  
  trace_new(TR_FUNC, "IOFUNCS: cnxt_iofuncs_get_font_and_mode\n");
  
  #ifdef DEBUG
  not_interrupt_safe();
  #endif

  if (!bInitialised)
  {
    trace_new(TR_ERR, "IOFUNCS: Module not initialised before use\n");
    eRetcode = IOFUNCS_ERROR_NOT_INIT;
  }
  else
  {
    iRetcode = sem_get(semSerialise, KAL_WAIT_FOREVER);
    
    if (iRetcode == RC_OK)
    {
      /* If pointers passed are both good, write the information to them */
      if (ppDefaultFont && pMode)
      {
        *ppDefaultFont = pDefaultFont;
        *pMode         = uIofuncsOsdMode;
      }
      else
      {
        trace_new(TR_ERR, "IOFUNCS: NULL pointer passed!\n");
        eRetcode = IOFUNCS_ERROR_BAD_PTR;
      }  
          
      sem_put(semSerialise);
    }
    else
    {
      trace_new(TR_ERR, "IOFUNCS: Failed to get module access semaphore! rc = 0x%08x\n", iRetcode);
      eRetcode = IOFUNCS_ERROR_INTERNAL;
    }
  }
  return(eRetcode);
}

/********************************************************************/
/*  FUNCTION:    cnxt_iofuncs_screen_display                        */
/*                                                                  */
/*  PARAMETERS:  bShow - TRUE to show the OSD region, FALSE to hide */
/*                                                                  */
/*  DESCRIPTION: Show or hide the IOFUNCS OSD region.               */
/*                                                                  */
/*  RETURNS:     IOFUNCS_STATUS_OK      if no errors occurred       */
/*               IOFUNCS_ERROR_INTERNAL if an internal function     */
/*                                      call failed                 */
/*               IOFUNCS_ERROR_NOT_INIT if the module has not been  */
/*                                      initialised.                */
/*                                                                  */
/*  CONTEXT:     Must be called from a non-interrupt context.       */
/*                                                                  */
/********************************************************************/
IOFUNCS_STATUS cnxt_iofuncs_screen_display(bool bShow)
{
  int  iRetcode;
  bool bRetcode;
  IOFUNCS_STATUS eRetcode = IOFUNCS_STATUS_OK;
  
  trace_new(TR_FUNC, "IOFUNCS: cnxt_iofuncs_screen_display\n");
  
  #ifdef DEBUG
  not_interrupt_safe();
  #endif

  if (!bInitialised)
  {
    trace_new(TR_ERR, "IOFUNCS: Module not initialised before use\n");
    return(IOFUNCS_ERROR_NOT_INIT);
  }
  else
  {
    iRetcode = sem_get(semSerialise, KAL_WAIT_FOREVER);
    
    if (iRetcode == RC_OK)
    {
      /* Enable the OSD region */
      bRetcode = SetOSDRgnOptions(hScreen, OSD_RO_ENABLE, bShow);
      
      if(!bRetcode)
        eRetcode = IOFUNCS_ERROR_INTERNAL;
    
      sem_put(semSerialise);
    }
    else
    {
      trace_new(TR_ERR, "IOFUNCS: Failed to get module access semaphore! rc = 0x%08x\n", iRetcode);
      eRetcode = IOFUNCS_ERROR_INTERNAL;
    }
  }
  return(eRetcode);
}

/********************************************************************/
/*  FUNCTION:    cnxt_iofuncs_set_palette                           */
/*                                                                  */
/*  PARAMETERS:  pPal - pointer to RGBA palette data to use for     */
/*                      IOFUNCS OSD region when displayed.          */
/*                                                                  */
/*  DESCRIPTION: Set the RGBA palette used by the IOFUNCS OSD       */
/*               region. The region will always have alpha enabled. */
/*                                                                  */
/*  RETURNS:     IOFUNCS_STATUS_OK      if no errors occurred       */
/*               IOFUNCS_ERROR_BAD_PTR  NULL pointer passed         */
/*               IOFUNCS_ERROR_INTERNAL if an internal function     */
/*                                      call failed.                */
/*               IOFUNCS_ERROR_NOT_INIT if the module has not been  */
/*                                      initialised.                */
/*               IOFUNCS_ERROR_NO_PALETTE if called when the OSD is */
/*                                      set to a 32bpp mode.        */
/*                                                                  */
/*  CONTEXT:     Must be called from a non-interrupt context.       */
/*                                                                  */
/********************************************************************/
IOFUNCS_STATUS cnxt_iofuncs_set_palette(PDRMPAL pPal)
{
  int  iRetcode;
  IOFUNCS_STATUS eRetcode = IOFUNCS_STATUS_OK;
  
  trace_new(TR_FUNC, "IOFUNCS: cnxt_iofuncs_set_palette\n");
  
  #ifdef DEBUG
  not_interrupt_safe();
  #endif

  if (!bInitialised)
  {
    trace_new(TR_ERR, "IOFUNCS: Module not initialised before use\n");
    return(IOFUNCS_ERROR_NOT_INIT);
  }
  else
  {
    iRetcode = sem_get(semSerialise, KAL_WAIT_FOREVER);
    
    if (iRetcode == RC_OK)
    {
      /* Were we passed valid pointer for the palette? */
      if (!pPal)
      {
        trace_new(TR_ERR, "IOFUNCS: NULL palette pointer passed\n");
        eRetcode = IOFUNCS_ERROR_BAD_PTR;
      }  
      else
      {
        /* Does this OSD mode require a palette? */
        if(uIofuncsOsdMode == OSD_MODE_32AYUV)
        {
          trace_new(TR_ERR, "IOFUNCS: Palette passed for direct color, 32bpp OSD mode\n");
          eRetcode = IOFUNCS_ERROR_NO_PALETTE;
        }
        else
        {
          /* Everything is OK - go ahead and set the palette */
          iRetcode = SetOSDRgnPalette(hScreen, pPal, TRUE);
          if (!iRetcode)
          {
            trace_new(TR_ERR, "IOFUNCS: Failed to set any palette entries for OSD region!\n");
            eRetcode = IOFUNCS_ERROR_INTERNAL;
          }
        }
      }      
          
      /* Set the region palette */
      sem_put(semSerialise);
    }
    else
    {
      trace_new(TR_ERR, "IOFUNCS: Failed to get module access semaphore! rc = 0x%08x\n", iRetcode);
      eRetcode = IOFUNCS_ERROR_INTERNAL;
    }
  }
  return(eRetcode);
}

/********************************************************************/
/*  FUNCTION:    cnxt_iofuncs_set_aa_indices                        */
/*                                                                  */
/*  PARAMETERS:  c33Fore     - 33% foreground, 67% background index */
/*               c66Fore     - 66% foreground, 34% background index */
/*                                                                  */
/*  DESCRIPTION: When drawing anti-aliased text with an 8bpp        */
/*               OSD mode in use, IOFUNCS needs to use 4 palette    */
/*               locations to store foreground, background and 2    */
/*               intermediate colours. This function allows the     */
/*               client to inform IOFUNCS of palette locations that */
/*               it has set aside for the intermediate tones.       */
/*                                                                  */
/*  RETURNS:     IOFUNCS_STATUS_OK      if no errors occurred       */
/*               IOFUNCS_ERROR_BAD_PTR  NULL pointer passed         */
/*               IOFUNCS_ERROR_INTERNAL if an internal function     */
/*                                      call failed.                */
/*               IOFUNCS_ERROR_NOT_INIT if the module has not been  */
/*                                      initialised.                */
/*               IOFUNCS_ERROR_NO_PALETTE if called when the OSD is */
/*                                      set to a 32bpp mode.        */
/*                                                                  */
/*  CONTEXT:     Must be called from a non-interrupt context.       */
/*                                                                  */
/********************************************************************/
IOFUNCS_STATUS cnxt_iofuncs_set_aa_indices(u_int8 c33Fore, u_int8 c66Fore)
{                                     
  int  iRetcode;
  IOFUNCS_STATUS eRetcode = IOFUNCS_STATUS_OK;
  
  trace_new(TR_FUNC, "IOFUNCS: cnxt_iofuncs_set_aa_indices\n");
  
  #ifdef DEBUG
  not_interrupt_safe();
  #endif

  if (!bInitialised)
  {
    trace_new(TR_ERR, "IOFUNCS: Module not initialised before use\n");
    return(IOFUNCS_ERROR_NOT_INIT);
  }
  else
  {
    iRetcode = sem_get(semSerialise, KAL_WAIT_FOREVER);
    
    if (iRetcode == RC_OK)
    {
      /* Save the palette indices for later use */  
      cAATextIndices[1] = c33Fore;
      cAATextIndices[2] = c66Fore;
      
      /* Set the region palette */
      sem_put(semSerialise);
    }
    else
    {
      trace_new(TR_ERR, "IOFUNCS: Failed to get module access semaphore! rc = 0x%08x\n", iRetcode);
      eRetcode = IOFUNCS_ERROR_INTERNAL;
    }
  }
  return(eRetcode);
}


/********************************************************************/
/*  FUNCTION:    cnxt_iofuncs_get_aa_indices                        */
/*                                                                  */
/*  PARAMETERS:  p33Fore     - 33% foreground, 67% background index */
/*               p66Fore     - 66% foreground, 34% background index */
/*                                                                  */
/*  DESCRIPTION: Query the 2 palette indices which will be used     */
/*               for intermediate tones when drawing anti-aliased   */
/*               text into an 8bpp surface.                         */
/*                                                                  */
/*  RETURNS:     IOFUNCS_STATUS_OK      if no errors occurred       */
/*               IOFUNCS_ERROR_BAD_PTR  NULL pointer passed         */
/*               IOFUNCS_ERROR_INTERNAL if an internal function     */
/*                                      call failed.                */
/*               IOFUNCS_ERROR_NOT_INIT if the module has not been  */
/*                                      initialised.                */
/*               IOFUNCS_ERROR_NO_PALETTE if called when the OSD is */
/*                                      set to a 32bpp mode.        */
/*                                                                  */
/*  CONTEXT:     Must be called from a non-interrupt context.       */
/*                                                                  */
/********************************************************************/
IOFUNCS_STATUS cnxt_iofuncs_get_aa_indices(u_int8 *p33Fore, u_int8 *p66Fore)
{                                     
  int  iRetcode;
  IOFUNCS_STATUS eRetcode = IOFUNCS_STATUS_OK;
  
  trace_new(TR_FUNC, "IOFUNCS: cnxt_iofuncs_get_aa_indices\n");
  
  #ifdef DEBUG
  not_interrupt_safe();
  #endif

  if( !p33Fore || !p66Fore)
  {
    trace_new(TR_ERR, "IOFUNCS: Bad pointer passed\n");
    return(IOFUNCS_ERROR_BAD_PTR);
  }
  
  if (!bInitialised)
  {
    trace_new(TR_ERR, "IOFUNCS: Module not initialised before use\n");
    return(IOFUNCS_ERROR_NOT_INIT);
  }
  else
  {
    iRetcode = sem_get(semSerialise, KAL_WAIT_FOREVER);
    
    if (iRetcode == RC_OK)
    {
      /* Return the indices to the caller */
      *p33Fore     = cAATextIndices[1];
      *p66Fore     = cAATextIndices[2];
      
      /* Set the region palette */
      sem_put(semSerialise);
    }
    else
    {
      trace_new(TR_ERR, "IOFUNCS: Failed to get module access semaphore! rc = 0x%08x\n", iRetcode);
      eRetcode = IOFUNCS_ERROR_INTERNAL;
    }
  }
  return(eRetcode);
}

/********************************************************************/
/*  FUNCTION:    cnxt_iofuncs_get_screen_grid                       */
/*                                                                  */
/*  PARAMETERS:  pWidth  - pointer for returned width information   */
/*               pHeight - pointer for returned height information  */
/*                                                                  */
/*  DESCRIPTION: Get the dimensions of the screen in terms of       */
/*               character cells. All x,y coordinates passed to     */
/*               IOFUNCS are based upon the cell size of the        */
/*               chosen default font.                               */
/*                                                                  */
/*  RETURNS:     IOFUNCS_STATUS_OK       if no errors occurred      */
/*               IOFUNCS_ERROR_BAD_PTR  NULL pointer passed         */
/*               IOFUNCS_ERROR_NOT_INIT if the module has not been  */
/*                                       initialised.               */
/*                                                                  */
/*  CONTEXT:     Must be called from a non-interrupt context.       */
/*                                                                  */
/********************************************************************/
IOFUNCS_STATUS cnxt_iofuncs_get_screen_grid( int *pWidth, int *pHeight)
{
  int iRetcode;
  IOFUNCS_STATUS eRetcode = IOFUNCS_STATUS_OK;
  
  trace_new(TR_FUNC, "IOFUNCS: cnxt_iofuncs_get_screen_grid\n");
  
  #ifdef DEBUG
  not_interrupt_safe();
  #endif

  if (!bInitialised)
  {
    trace_new(TR_ERR, "IOFUNCS: Module not initialised before use\n");
    eRetcode = IOFUNCS_ERROR_NOT_INIT;
  }
  else
  {
    iRetcode = sem_get(semSerialise, KAL_WAIT_FOREVER);
    
    if (iRetcode == RC_OK)
    {
      if (pWidth && pHeight)
      {
        *pWidth = iScreenWidth;
        *pHeight = iScreenHeight;
      }
      else
      {
        trace_new(TR_ERR, "IOFUNCS: NULL pointer passed!\n");
        eRetcode = IOFUNCS_ERROR_BAD_PTR;
      }  
          
      sem_put(semSerialise);
    }
    else
    {
      trace_new(TR_ERR, "IOFUNCS: Failed to get module access semaphore! rc = 0x%08x\n", iRetcode);
      eRetcode = IOFUNCS_ERROR_INTERNAL;
    }
  }
  return(eRetcode);
}

/*********************/
/*********************/
/** Text and cursor **/
/*********************/
/*********************/


/********************************************************************/
/*  FUNCTION:    cnxt_iofuncs_printf                                */
/*                                                                  */
/*  PARAMETERS:  strFormat - printf-compatible formatting string    */
/*               Other parameters as necessary depending upon the   */
/*               supplied formating string.                         */
/*                                                                  */
/*  DESCRIPTION: This function displays a formatted string with     */
/*               inserted values at the current text cursor         */
/*               position. The string is wrapped at line ends and   */
/*               the current display window scrolled as required.   */
/*               The function also updates the current text cursor  */
/*               position.                                          */
/*                                                                  */
/*  RETURNS:     IOFUNCS_STATUS_OK      - No errors occured.        */
/*               IOFUNCS_ERROR_NOT_INIT - Module not initialised.   */
/*               IOFUNCS_ERROR_BAD_PTR  - NULL strFormat passed.    */
/*               IOFUNCS_ERROR_INTERNAL - Internal call failed.     */
/*                                                                  */
/*  CONTEXT:     May be called from any non-interrupt context.      */
/*                                                                  */
/********************************************************************/
IOFUNCS_STATUS cnxt_iofuncs_printf(char *strFormat, ...)
{
  IOFUNCS_STATUS eRetcode = IOFUNCS_STATUS_OK;
  int  iKalRetcode;
  int  iNumChars;
  bool bCursorSave = FALSE;
  va_list args;
  
  trace_new(TR_FUNC, "IOFUNCS: cnxt_iofuncs_printf\n");
  
  #ifdef DEBUG
  not_interrupt_safe();
  #endif

  if (!bInitialised)
  {
    trace_new(TR_ERR, "IOFUNCS: Module not initialised before use\n");
    eRetcode = IOFUNCS_ERROR_NOT_INIT;
  }
  else
  {
    iKalRetcode = sem_get(semSerialise, KAL_WAIT_FOREVER);
    
    if (iKalRetcode == RC_OK)
    {
      if (strFormat)
      {
        /* Remove the text cursor while we are printing */
        bCursorSave = bCursorShown;
        if(bCursorSave)
          internal_cursor_show(CURSOR_TEXT, FALSE);
         
        va_start(args, strFormat);
        eRetcode = internal_vprintf_font_at_pixel(pDefaultFont,
                                (rectWindow.left + iTextCursorX) * giCellWidth,
                                (rectWindow.top  + iTextCursorY) * giCellHeight,
                                 strFormat,
                                 &iNumChars,
                                 FALSE,
                                 args);
        va_end(args);
        
        /* Replace the cursor */
        if(bCursorSave)
          internal_cursor_show(CURSOR_TEXT, TRUE);
        
      }
      else
      {
        trace_new(TR_ERR, "IOFUNCS: NULL pointer passed\n");
        eRetcode = IOFUNCS_ERROR_BAD_PTR;
      }  
      
      sem_put(semSerialise);
    }
    else
    {
      trace_new(TR_ERR, "IOFUNCS: Failed to get module access semaphore! rc = 0x%08x\n", iKalRetcode);
      eRetcode = IOFUNCS_ERROR_INTERNAL;
    }
  }
  return(eRetcode);
}

/********************************************************************/
/*  FUNCTION:    cnxt_iofuncs_printf_at                             */
/*                                                                  */
/*  PARAMETERS:  x         - screen x cell coordinate for string    */
/*               y         - screen y cell coordinate for string    */
/*               strFormat - printf-compatible formatting string    */
/*               Other parameters as necessary depending upon the   */
/*               supplied formating string.                         */
/*                                                                  */
/*  DESCRIPTION: This function displays a formatted string with     */
/*               inserted values at the cell position supplied.     */
/*               The X and Y values represent screen cell           */
/*               coordinates and may be outside the currently set   */
/*               display window. The string is displayed and        */
/*               clipped to the screen. The current display window  */
/*               is ignored for this call and no text cursor        */
/*               position changes are made.                         */
/*                                                                  */
/*  RETURNS:     IOFUNCS_STATUS_OK      - No errors occured.        */
/*               IOFUNCS_ERROR_NOT_INIT - Module not initialised.   */
/*               IOFUNCS_ERROR_BAD_PTR  - NULL strFormat passed.    */
/*               IOFUNCS_ERROR_INTERNAL - Internal call failed.     */
/*                                                                  */
/*  CONTEXT:     May be called from any non-interrupt context.      */
/*                                                                  */
/********************************************************************/
IOFUNCS_STATUS cnxt_iofuncs_printf_at(int x, int y, char *strFormat, ...)
{
  IOFUNCS_STATUS eRetcode = IOFUNCS_STATUS_OK;
  int iKalRetcode;
  int iNumChars;
  bool bCursorSave = FALSE;
  va_list args;
  
  trace_new(TR_FUNC, "IOFUNCS: cnxt_iofuncs_printf_at\n");
  
  #ifdef DEBUG
  not_interrupt_safe();
  #endif

  if (!bInitialised)
  {
    trace_new(TR_ERR, "IOFUNCS: Module not initialised before use\n");
    eRetcode = IOFUNCS_ERROR_NOT_INIT;
  }
  else
  {
    iKalRetcode = sem_get(semSerialise, KAL_WAIT_FOREVER);
    
    if (iKalRetcode == RC_OK)
    {
      if (strFormat)
      {
        /* Remove the text cursor while we are printing */
        bCursorSave = bCursorShown;
        if(bCursorSave)
          internal_cursor_show(CURSOR_TEXT, FALSE);
         
        va_start(args, strFormat);
        eRetcode = internal_vprintf_font_at_pixel(pDefaultFont,
                                                  x * giCellWidth,
                                                  y * giCellHeight,
                                                  strFormat,
                                                  &iNumChars,
                                                  TRUE,
                                                  args);
        va_end(args);
        
        /* Replace the cursor */
        if(bCursorSave)
          internal_cursor_show(CURSOR_TEXT, TRUE);
      }
      else
      {
        trace_new(TR_ERR, "IOFUNCS: NULL pointer passed\n");
        eRetcode = IOFUNCS_ERROR_BAD_PTR;
      }  
      
      sem_put(semSerialise);
    }
    else
    {
      trace_new(TR_ERR, "IOFUNCS: Failed to get module access semaphore! rc = 0x%08x\n", iKalRetcode);
      eRetcode = IOFUNCS_ERROR_INTERNAL;
    }
  }
  return(eRetcode);
}

/********************************************************************/
/*  FUNCTION:    cnxt_iofuncs_printf_at_pixel                       */
/*                                                                  */
/*  PARAMETERS:  x         - screen x pixel coordinate for string   */
/*               y         - screen y pixel coordinate for string   */
/*               strFormat - printf-compatible formatting string    */
/*               Other parameters as necessary depending upon the   */
/*               supplied formating string.                         */
/*                                                                  */
/*  DESCRIPTION: This function displays a formatted string with     */
/*               inserted values at the pixel position supplied.    */
/*               The X and Y values represent screen pixel          */
/*               coordinates and may be outside the currently set   */
/*               display window. The string is displayed and        */
/*               clipped to the screen. The current display window  */
/*               is ignored for this call and no text cursor        */
/*               position changes are made.                         */
/*                                                                  */
/*  RETURNS:     IOFUNCS_STATUS_OK      - No errors occured.        */
/*               IOFUNCS_ERROR_NOT_INIT - Module not initialised.   */
/*               IOFUNCS_ERROR_BAD_PTR  - NULL strFormat passed.    */
/*               IOFUNCS_ERROR_INTERNAL - Internal call failed.     */
/*                                                                  */
/*  CONTEXT:     May be called from any non-interrupt context.      */
/*                                                                  */
/********************************************************************/
IOFUNCS_STATUS cnxt_iofuncs_printf_at_pixel(int x, int y, char *strFormat, ...)
{
  IOFUNCS_STATUS eRetcode = IOFUNCS_STATUS_OK;
  int iKalRetcode;
  int iNumChars;
  bool bCursorSave = FALSE;
  va_list args;
  
  trace_new(TR_FUNC, "IOFUNCS: cnxt_iofuncs_printf_at_pixel\n");
  
  #ifdef DEBUG
  not_interrupt_safe();
  #endif

  if (!bInitialised)
  {
    trace_new(TR_ERR, "IOFUNCS: Module not initialised before use\n");
    eRetcode = IOFUNCS_ERROR_NOT_INIT;
  }
  else
  {
    iKalRetcode = sem_get(semSerialise, KAL_WAIT_FOREVER);
    
    if (iKalRetcode == RC_OK)
    {
      if (strFormat)
      {
        /* Remove the text cursor while we are printing */
        bCursorSave = bCursorShown;
        if(bCursorSave)
          internal_cursor_show(CURSOR_TEXT, FALSE);
         
        va_start(args, strFormat);
        eRetcode = internal_vprintf_font_at_pixel(pDefaultFont,
                                                  x,
                                                  y,
                                                  strFormat,
                                                  &iNumChars,
                                                  TRUE,
                                                  args);
        va_end(args);
        
        /* Replace the cursor */
        if(bCursorSave)
          internal_cursor_show(CURSOR_TEXT, TRUE);
      }
      else
      {
        trace_new(TR_ERR, "IOFUNCS: NULL pointer passed\n");
        eRetcode = IOFUNCS_ERROR_BAD_PTR;
      }  
      
      sem_put(semSerialise);
    }
    else
    {
      trace_new(TR_ERR, "IOFUNCS: Failed to get module access semaphore! rc = 0x%08x\n", iKalRetcode);
      eRetcode = IOFUNCS_ERROR_INTERNAL;
    }
  }
  return(eRetcode);
}

/********************************************************************/
/*  FUNCTION:    cnxt_iofuncs_printf_font_at                        */
/*                                                                  */
/*  PARAMETERS:  pFont     - Font to use in rendering string        */
/*               x         - screen x cell coordinate for string    */
/*               y         - screen y cell coordinate for string    */
/*               strFormat - printf-compatible formatting string    */
/*               Other parameters as necessary depending upon the   */
/*               supplied formating string.                         */
/*                                                                  */
/*  DESCRIPTION: This function displays a formatted string with     */
/*               inserted values at the cell position supplied.     */
/*               The X and Y values represent screen cell           */
/*               coordinates and may be outside the currently set   */
/*               display window. The string is displayed and        */
/*               clipped to the screen. The current display window  */
/*               is ignored for this call and no text cursor        */
/*               position changes are made.                         */
/*                                                                  */
/*  RETURNS:     IOFUNCS_STATUS_OK      - No errors occured.        */
/*               IOFUNCS_ERROR_NOT_INIT - Module not initialised.   */
/*               IOFUNCS_ERROR_BAD_PTR  - NULL strFormat passed.    */
/*               IOFUNCS_ERROR_INTERNAL - Internal call failed.     */
/*                                                                  */
/*  CONTEXT:     May be called from any non-interrupt context.      */
/*                                                                  */
/********************************************************************/
IOFUNCS_STATUS cnxt_iofuncs_printf_font_at(PGFX_FONT pFont, int x, int y, char *strFormat, ...)
{
  IOFUNCS_STATUS eRetcode = IOFUNCS_STATUS_OK;
  int iKalRetcode;
  int iNumChars;
  bool bCursorSave = FALSE;
  va_list args;
  
  trace_new(TR_FUNC, "IOFUNCS: cnxt_iofuncs_printf_font_at\n");
  
  #ifdef DEBUG
  not_interrupt_safe();
  #endif

  if (!bInitialised)
  {
    trace_new(TR_ERR, "IOFUNCS: Module not initialised before use\n");
    eRetcode = IOFUNCS_ERROR_NOT_INIT;
  }
  else
  {
    iKalRetcode = sem_get(semSerialise, KAL_WAIT_FOREVER);
    
    if (iKalRetcode == RC_OK)
    {
      if (strFormat)
      {
        /* Remove the text cursor while we are printing */
        bCursorSave = bCursorShown;
        if(bCursorSave)
          internal_cursor_show(CURSOR_TEXT, FALSE);
         
        va_start(args, strFormat);
        eRetcode = internal_vprintf_font_at_pixel(pFont,
                                                  x * giCellWidth,
                                                  y * giCellHeight,
                                                  strFormat,
                                                  &iNumChars,
                                                  TRUE,
                                                  args);
        va_end(args);
        
        /* Replace the cursor */
        if(bCursorSave)
          internal_cursor_show(CURSOR_TEXT, TRUE);
      }
      else
      {
        trace_new(TR_ERR, "IOFUNCS: NULL pointer passed\n");
        eRetcode = IOFUNCS_ERROR_BAD_PTR;
      }  
      
      sem_put(semSerialise);
    }
    else
    {
      trace_new(TR_ERR, "IOFUNCS: Failed to get module access semaphore! rc = 0x%08x\n", iKalRetcode);
      eRetcode = IOFUNCS_ERROR_INTERNAL;
    }
  }
  return(eRetcode);
}


/********************************************************************/
/*  FUNCTION:    cnxt_iofuncs_printf_font_at_pixel                  */
/*                                                                  */
/*  PARAMETERS:  pFont     - Font to use in rendering string        */
/*               x         - screen x pixel coordinate for string   */
/*               y         - screen y pixel coordinate for string   */
/*               strFormat - printf-compatible formatting string    */
/*               Other parameters as necessary depending upon the   */
/*               supplied formating string.                         */
/*                                                                  */
/*  DESCRIPTION: This function displays a formatted string with     */
/*               inserted values at the pixel position supplied.    */
/*               The X and Y values represent screen pixel          */
/*               coordinates and may be outside the currently set   */
/*               display window. The string is displayed and        */
/*               clipped to the screen. The current display window  */
/*               is ignored for this call and no text cursor        */
/*               position changes are made.                         */
/*                                                                  */
/*  RETURNS:     IOFUNCS_STATUS_OK      - No errors occured.        */
/*               IOFUNCS_ERROR_NOT_INIT - Module not initialised.   */
/*               IOFUNCS_ERROR_BAD_PTR  - NULL strFormat passed.    */
/*               IOFUNCS_ERROR_INTERNAL - Internal call failed.     */
/*                                                                  */
/*  CONTEXT:     May be called from any non-interrupt context.      */
/*                                                                  */
/********************************************************************/
IOFUNCS_STATUS cnxt_iofuncs_printf_font_at_pixel(PGFX_FONT pFont, int x, int y, char *strFormat, ...)
{
  IOFUNCS_STATUS eRetcode = IOFUNCS_STATUS_OK;
  int iKalRetcode;
  int iNumChars;
  bool bCursorSave = FALSE;
  va_list args;
  
  trace_new(TR_FUNC, "IOFUNCS: cnxt_iofuncs_printf_font_at_pixel\n");
  
  #ifdef DEBUG
  not_interrupt_safe();
  #endif

  if (!bInitialised)
  {
    trace_new(TR_ERR, "IOFUNCS: Module not initialised before use\n");
    eRetcode = IOFUNCS_ERROR_NOT_INIT;
  }
  else
  {
    iKalRetcode = sem_get(semSerialise, KAL_WAIT_FOREVER);
    
    if (iKalRetcode == RC_OK)
    {
      if (strFormat)
      {
        /* Remove the text cursor while we are printing */
        bCursorSave = bCursorShown;
        if(bCursorSave)
          internal_cursor_show(CURSOR_TEXT, FALSE);
         
        va_start(args, strFormat);
        eRetcode = internal_vprintf_font_at_pixel(pFont,
                                                  x,
                                                  y,
                                                  strFormat,
                                                  &iNumChars,
                                                  TRUE,
                                                  args);
        va_end(args);
        
        /* Replace the cursor */
        if(bCursorSave)
          internal_cursor_show(CURSOR_TEXT, TRUE);
      }
      else
      {
        trace_new(TR_ERR, "IOFUNCS: NULL pointer passed\n");
        eRetcode = IOFUNCS_ERROR_BAD_PTR;
      }  
      
      sem_put(semSerialise);
    }
    else
    {
      trace_new(TR_ERR, "IOFUNCS: Failed to get module access semaphore! rc = 0x%08x\n", iKalRetcode);
      eRetcode = IOFUNCS_ERROR_INTERNAL;
    }
  }
  return(eRetcode);
}




/********************************************************************/
/*  FUNCTION:    cnxt_iofuncs_printf_font_at_pixel                  */
/*                                                                  */
/*  PARAMETERS:  pFont     - Font to use in rendering string        */
/*               x         - screen x pixel coordinate for string   */
/*               y         - screen y pixel coordinate for string   */
/*               strFormat - printf-compatible formatting string    */
/*               Other parameters as necessary depending upon the   */
/*               supplied formating string.                         */
/*                                                                  */
/*  DESCRIPTION: This function displays a formatted string with     */
/*               inserted values at the pixel position supplied.    */
/*               The X and Y values represent screen pixel          */
/*               coordinates and may be outside the currently set   */
/*               display window. The string is displayed and        */
/*               clipped to the screen. The current display window  */
/*               is ignored for this call and no text cursor        */
/*               position changes are made.                         */
/*                                                                  */
/*  RETURNS:     IOFUNCS_STATUS_OK      - No errors occured.        */
/*               IOFUNCS_ERROR_NOT_INIT - Module not initialised.   */
/*               IOFUNCS_ERROR_BAD_PTR  - NULL strFormat passed.    */
/*               IOFUNCS_ERROR_INTERNAL - Internal call failed.     */
/*                                                                  */
/*  CONTEXT:     May be called from any non-interrupt context.      */
/*                                                                  */
/********************************************************************/
IOFUNCS_STATUS cnxt_iofuncs_printf_font_at_pixel_auto_wrap(PGFX_FONT pFont, int x, int y, int x2, int y2,u_int8 calculate,u_int8 *end, int *pNumChars,char *strFormat, ...)
{
  IOFUNCS_STATUS eRetcode = IOFUNCS_STATUS_OK;
  int iKalRetcode;
  bool bCursorSave = FALSE;
  va_list args;
  
  trace_new(TR_FUNC, "IOFUNCS: cnxt_iofuncs_printf_font_at_pixel_auto_wrap\n");
  
  #ifdef DEBUG
  not_interrupt_safe();
  #endif

  if (!bInitialised)
  {
    trace_new(TR_ERR, "IOFUNCS: Module not initialised before use\n");
    eRetcode = IOFUNCS_ERROR_NOT_INIT;
  }
  else
  {
    iKalRetcode = sem_get(semSerialise, KAL_WAIT_FOREVER);
    
    if (iKalRetcode == RC_OK)
    {
      if (strFormat)
      {
        /* Remove the text cursor while we are printing */
        bCursorSave = bCursorShown;
        if(bCursorSave)
          internal_cursor_show(CURSOR_TEXT, FALSE);
         
        va_start(args, strFormat);
        eRetcode = internal_vprintf_font_at_pixel_auto_wrap(pFont,
                                                  x,
                                                  y,
                                                  x2,
                                                  y2,
                                                  calculate,
                                                  end,
                                                  strFormat,
                                                  pNumChars,
                                                  args);
        va_end(args);
        
        /* Replace the cursor */
        if(bCursorSave)
          internal_cursor_show(CURSOR_TEXT, TRUE);
      }
      else
      {
        trace_new(TR_ERR, "IOFUNCS: NULL pointer passed\n");
        eRetcode = IOFUNCS_ERROR_BAD_PTR;
      }  
      
      sem_put(semSerialise);
    }
    else
    {
      trace_new(TR_ERR, "IOFUNCS: Failed to get module access semaphore! rc = 0x%08x\n", iKalRetcode);
      eRetcode = IOFUNCS_ERROR_INTERNAL;
    }
  }
  return(eRetcode);
}


/********************************************************************/
/*  FUNCTION:    cnxt_iofuncs_get_bounding_rect                     */
/*                                                                  */
/*  PARAMETERS:  pFont     - Font to use in rendering string        */
/*               pRect     - Storage for returned bounding rectangle*/
/*               strFormat - printf-compatible formatting string    */
/*               Other parameters as necessary depending upon the   */
/*               supplied formating string.                         */
/*                                                                  */
/*  DESCRIPTION: This function returns a rectangle representing the */
/*               bounds of the given string rendered in the given   */
/*               font. This rectangle encloses all pixels in the    */
/*               test that would be rendered if the same format     */
/*               string and inserts are passed to one of the printf */
/*               functions with the same font.                      */
/*                                                                  */
/*               Note: This function has no dependence on any       */
/*               IOFUNCS resource so does not claim the API access  */
/*               semaphore.                                         */
/*                                                                  */
/*  RETURNS:     IOFUNCS_STATUS_OK      - No errors occured.        */
/*               IOFUNCS_ERROR_BAD_PTR  - NULL pointer passed.      */
/*               IOFUNCS_ERROR_OUT_OF_BOUNDS - the rendered string  */
/*                                        caused an internal buffer */
/*                                        overflow. Stack corruption*/
/*                                        is possible.              */
/*               IOFUNCS_ERROR_INTERNAL - Internal call failed.     */
/*                                                                  */
/*  CONTEXT:     May be called from any non-interrupt context.      */
/*                                                                  */
/********************************************************************/
IOFUNCS_STATUS cnxt_iofuncs_get_bounding_rect(PGFX_FONT pFont, POSDRECT pRect, char *strFormat, ...)
{
  IOFUNCS_STATUS eRetcode = IOFUNCS_STATUS_OK;
  va_list vaArgs;
  int     iStringLen;
  int     iChars;
  int     iWidth;
  int     iHeight;
  int     iRunningWidth;
  int     iNumLines;
  int     iMaxLineLen;
  int     iCharHeight;
  char    cCurrentChar;
  char    szBuffer[PRINTF_BUFFER_SIZE];
  
  /* Check parameters */
  if(!pFont || !pRect || !strFormat)
    return(IOFUNCS_ERROR_BAD_PTR);
    
  /* Render the provided string with inserts into our local buffer */
  va_start(vaArgs, strFormat);
  iStringLen = vsprintf(szBuffer, strFormat, vaArgs);
  va_end(vaArgs);
  
  /* If vsprintf wrote more characters than we have buffer space for, the stack */
  /* will likely have been corrupted so  warn the user                          */
  if(iStringLen >= PRINTF_BUFFER_SIZE)
  {
    error_log(ERROR_WARNING);
    trace_new(TRACE_LEVEL_ALWAYS|TRACE_CTL, "IOFUNCS: ****** Possible stack overflow in cnxt_iofuncs_get_bounding_rect!!! ******\n");
    eRetcode = IOFUNCS_ERROR_OUT_OF_BOUNDS;
  }

  /* Set up for the loop */
  iRunningWidth = 0;
  iMaxLineLen   = 0;
  
  if(iStringLen)
    iNumLines = 1;
  else
    iNumLines = 0;
    
  /* Figure out the length of the rendered string in pixels */
  for(iChars = 0; iChars < iStringLen; iChars++)
  {
    cCurrentChar = (u_int8)szBuffer[iChars];
    
    /* If this character causes a line feed, increment the line counter */
    if(cCurrentChar == 0x0A)
      iNumLines ++;

    /* If this character causes the cursor to move to the left, do end of line checks */
    if((cCurrentChar == 0x0D) || (cCurrentChar == 0x0A))  
    {
      if(iRunningWidth > iMaxLineLen)
        iMaxLineLen = iRunningWidth;
      iRunningWidth = 0;
    }
    else
    {  
      /* If the font contains this character, use its real width, else use */
      /* the font cell width.                                              */
      GfxGetFontCharacterSize(pFont, cCurrentChar, &iWidth, &iHeight);
      
      iRunningWidth += iWidth;
    }
  }  
  
  /* See if the last line was the longest */
  if(iRunningWidth > iMaxLineLen)
    iMaxLineLen = iRunningWidth;
  
  GfxGetFontCellSize(pFont, &iWidth, &iCharHeight);
  
  /* Calculate the final bounding rectangle */
  pRect->left   = 0;
  pRect->top    = 0;
  pRect->right  = (short)iMaxLineLen;
  pRect->bottom = (short)iNumLines * (short)iCharHeight;
  
  return(eRetcode);
}

/**********************/
/**********************/
/**                  **/
/** Cursor Functions **/
/**                  **/
/**********************/
/**********************/


/********************************************************************/
/*  FUNCTION:    cnxt_iofuncs_set_cursor_pos                        */
/*                                                                  */
/*  PARAMETERS:  eCursor - Cursor to move, text or mouse.           */
/*               x       - New X position for cursor                */
/*               y       - New Y position for cursor                */
/*                                                                  */
/*  DESCRIPTION: Move the mouse pointer or text cursor to a new     */
/*               position. Coordinates passed are pixel coordinates */
/*               in the case of the mouse pointer or cell coords    */
/*               relative to the top left of the current display    */
/*               window for the text cursor. Values are clipped to  */
/*               the screen or window if they are outside the valid */
/*               range.                                             */
/*                                                                  */
/*  RETURNS:     IOFUNCS_STATUS_OK      - No errors occured.        */
/*               IOFUNCS_ERROR_NOT_INIT - Module not initialised.   */
/*               IOFUNCS_ERROR_INTERNAL - Internal call failed.     */
/*                                                                  */
/*  CONTEXT:     This function must be called in a non-interrupt    */
/*               context.                                           */
/*                                                                  */
/********************************************************************/
IOFUNCS_STATUS cnxt_iofuncs_set_cursor_pos(IOFUNCS_CURSOR eCursor, int x, int y)
{
  int iRetcode;
  IOFUNCS_STATUS eRetcode = IOFUNCS_STATUS_OK;
  bool ks;
  
  trace_new(TR_FUNC, "IOFUNCS: cnxt_iofuncs_set_cursor_pos\n");
  
  #ifdef DEBUG
  not_interrupt_safe();
  #endif
  
  if (!bInitialised)
  {
    trace_new(TR_ERR, "IOFUNCS: Module not initialised before use\n");
    eRetcode = IOFUNCS_ERROR_NOT_INIT;
  }
  else
  {
    iRetcode = sem_get(semSerialise, KAL_WAIT_FOREVER);
    
    if (iRetcode == RC_OK)
    {
      /* We must remove and redraw the text cursor when we move it */
      if (eCursor == CURSOR_TEXT)
      {
        if (bCursorShown)
        {
          eRetcode = internal_cursor_show(eCursor, FALSE);
          ks = TRUE;
        }
        else
          ks = FALSE;  
        
        iTextCursorX = max(0, min(x, (rectWindow.right-(rectWindow.left+1))));
        iTextCursorY = max(0, min(y, (rectWindow.bottom-(rectWindow.top+1))));
  
        /* Show the cursor again */
        if(ks)
          eRetcode = internal_cursor_show(eCursor, TRUE);
      }
      else
      {
        /* We can move the mouse cursor without hiding it but updates to the */
        /* coordinates must be done inside a critical section since they are */
        /* also changed under interrupt.                                     */
        ks = critical_section_begin();
        
        iMouseCursorX = max(0, min(x, gnOsdMaxWidth));
        iMouseCursorY = max(0, min(y, gMyOsdMaxHeight));
        
        critical_section_end(ks);
        
        /* Update the mouse position on screen if it is visible */
        if(bMouseShown)
          eRetcode = internal_cursor_show(eCursor, TRUE);
      }                 
      
      sem_put(semSerialise);
    }
    else
    {
      trace_new(TR_ERR, "IOFUNCS: Failed to get module access semaphore! rc = 0x%08x\n", iRetcode);
      eRetcode = IOFUNCS_ERROR_INTERNAL;
    }
  }
  return(eRetcode);
}

/********************************************************************/
/*  FUNCTION:    cnxt_iofuncs_get_cursor_pos                        */
/*                                                                  */
/*  PARAMETERS:  eCursor - Cursor to move, text or mouse.           */
/*               px      - Storage for returned X coordinate        */
/*               px      - Storage for returned Y coordinate        */
/*                                                                  */
/*  DESCRIPTION: Return the current mouse pointer or text cursor    */
/*               position. Values returned are pixel coordinates    */
/*               in the case of the mouse pointer or cell coords    */
/*               relative to the top left of the current display    */
/*               window for the text cursor.                        */
/*                                                                  */
/*  RETURNS:     IOFUNCS_STATUS_OK      - No errors occured.        */
/*               IOFUNCS_ERROR_NOT_INIT - Module not initialised.   */
/*               IOFUNCS_ERROR_INTERNAL - Internal call failed.     */
/*               IOFUNCS_ERROR_BAD_PTR  - NULL pointer passed       */
/*                                                                  */
/*  CONTEXT:     This function must be called in a non-interrupt    */
/*               context.                                           */
/*                                                                  */
/********************************************************************/
IOFUNCS_STATUS cnxt_iofuncs_get_cursor_pos(IOFUNCS_CURSOR eCursor, int *px, int *py)
{
  int iRetcode;
  IOFUNCS_STATUS eRetcode = IOFUNCS_STATUS_OK;
  bool ks;
  
  trace_new(TR_FUNC, "IOFUNCS: cnxt_iofuncs_get_cursor_pos\n");
  
  #ifdef DEBUG
  not_interrupt_safe();
  #endif
  
  if (!bInitialised)
  {
    trace_new(TR_ERR, "IOFUNCS: Module not initialised before use\n");
    eRetcode = IOFUNCS_ERROR_NOT_INIT;
  }
  else
  {
    iRetcode = sem_get(semSerialise, KAL_WAIT_FOREVER);
    
    if (iRetcode == RC_OK)
    {
      if (px && py)
      {
        /* We must remove and redraw the text cursor when we move it */
        if (eCursor == CURSOR_TEXT)
        {
          *px = iTextCursorX;
          *py = iTextCursorY;
        }
        else
        {
          ks = critical_section_begin();
          *px = iMouseCursorX;
          *py = iMouseCursorY;
          critical_section_end(ks);
        }                 
      }
      else
      {
        trace_new(TR_ERR, "IOFUNCS: NULL pointer passed\n");
      }
        
      sem_put(semSerialise);
    }
    else
    {
      trace_new(TR_ERR, "IOFUNCS: Failed to get module access semaphore! rc = 0x%08x\n", iRetcode);
      eRetcode = IOFUNCS_ERROR_INTERNAL;
    }
  }
  return(eRetcode);
}

/********************************************************************/
/*  FUNCTION:    cnxt_iofuncs_get_cursor_pos                        */
/*                                                                  */
/*  PARAMETERS:  eCursor - Cursor to show or hide                   */
/*               bShow   - TRUE to show the cursor, FALSE to hide   */
/*                                                                  */
/*  DESCRIPTION: Make the mouse pointer or text cursor visible or   */
/*               hidden depending upon the state of the bShow       */
/*               parameter.                                         */
/*                                                                  */
/*  RETURNS:     IOFUNCS_STATUS_OK      - No errors occured.        */
/*               IOFUNCS_ERROR_NOT_INIT - Module not initialised.   */
/*               IOFUNCS_ERROR_INTERNAL - Internal call failed.     */
/*                                                                  */
/*  CONTEXT:     This function must be called in a non-interrupt    */
/*               context.                                           */
/*                                                                  */
/********************************************************************/
IOFUNCS_STATUS cnxt_iofuncs_cursor_display(IOFUNCS_CURSOR eCursor, bool bShow)
{
  int iRetcode;
  IOFUNCS_STATUS eRetcode = IOFUNCS_STATUS_OK;
  
  trace_new(TR_FUNC, "IOFUNCS: cnxt_iofuncs_cursor_show\n");
  
  #ifdef DEBUG
  not_interrupt_safe();
  #endif
  
  if (!bInitialised)
  {
    trace_new(TR_ERR, "IOFUNCS: Module not initialised before use\n");
    eRetcode = IOFUNCS_ERROR_NOT_INIT;
  }
  else
  {
    /* Get the access serialisation semaphore */
    iRetcode = sem_get(semSerialise, KAL_WAIT_FOREVER);
    
    if (iRetcode == RC_OK)
    {
      /* Actually show or hide the relevant cursor */
      eRetcode = internal_cursor_show(eCursor, bShow);
      
      sem_put(semSerialise);
    }
    else
    {
      trace_new(TR_ERR, "IOFUNCS: Failed to get module access semaphore! rc = 0x%08x\n", iRetcode);
      eRetcode = IOFUNCS_ERROR_INTERNAL;
    }
  }
  return(eRetcode);
}

/********************************************************************/
/*  FUNCTION:    cnxt_iofuncs_set_mouse_cursor                      */
/*                                                                  */
/*  PARAMETERS:  hNewCursor - Handle of new cursor to use or NULL   */
/*                            to set default (arrow) cursor.        */
/*                                                                  */
/*  DESCRIPTION: Set the cursor to be shown as the mouse pointer.   */
/*               If NULL is passed, the default cursor is set.      */
/*                                                                  */
/*  RETURNS:     IOFUNCS_STATUS_OK      - No errors occured.        */
/*               IOFUNCS_ERROR_NOT_INIT - Module not initialised.   */
/*               IOFUNCS_ERROR_INTERNAL - Internal call failed.     */
/*                                                                  */
/*  CONTEXT:     This function must be called in a non-interrupt    */
/*               context.                                           */
/*                                                                  */
/********************************************************************/
IOFUNCS_STATUS cnxt_iofuncs_set_mouse_cursor(HOSDCUR hNewCursor)
{
  int iRetcode;
  IOFUNCS_STATUS eRetcode = IOFUNCS_STATUS_OK;
  
  trace_new(TR_FUNC, "IOFUNCS: cnxt_iofuncs_set_mouse_cursor\n");
  
  #ifdef DEBUG
  not_interrupt_safe();
  #endif
  
  if (!bInitialised)
  {
    trace_new(TR_ERR, "IOFUNCS: Module not initialised before use\n");
    eRetcode = IOFUNCS_ERROR_NOT_INIT;
  }
  else
  {
    /* Get the access serialisation semaphore */
    iRetcode = sem_get(semSerialise, KAL_WAIT_FOREVER);
    
    if (iRetcode == RC_OK)
    {
      if(hNewCursor)
        hCursor = hNewCursor;
      else
        hCursor = hDefaultCursor;
          
      /* If the mouse is visible, make the bitmap update */    
      if(bMouseShown)
        eRetcode = internal_cursor_show(CURSOR_POINTER, TRUE);
            
      sem_put(semSerialise);
    }
    else
    {
      trace_new(TR_ERR, "IOFUNCS: Failed to get module access semaphore! rc = 0x%08x\n", iRetcode);
      eRetcode = IOFUNCS_ERROR_INTERNAL;
    }
  }
  return(eRetcode);
}

/********************************************************************/
/*  FUNCTION:    cnxt_iofuncs_set_mouse_relative                    */
/*                                                                  */
/*  PARAMETERS:  bRelative - send relative movement messages for    */
/*                           mouse if TRUE else send absolute.      */
/*                                                                  */
/*  DESCRIPTION: Set whether mouse move messages send relative      */
/*               coordinates or absolute position.                  */
/*                                                                  */
/*  RETURNS:     IOFUNCS_STATUS_OK      - No errors occured.        */
/*               IOFUNCS_ERROR_NOT_INIT - Module not initialised.   */
/*                                                                  */
/*  CONTEXT:     This functiom must be called from a non-interrupt  */
/*               context.                                           */
/*                                                                  */
/********************************************************************/
IOFUNCS_STATUS cnxt_iofuncs_set_mouse_relative(bool bRelative)
{
  int iRetcode;
  IOFUNCS_STATUS eRetcode = IOFUNCS_STATUS_OK;
  
  trace_new(TR_FUNC, "IOFUNCS: cnxt_iofuncs_set_mouse_relative\n");
  
  #ifdef DEBUG
  not_interrupt_safe();
  #endif
  
  if (!bInitialised)
  {
    trace_new(TR_ERR, "IOFUNCS: Module not initialised before use\n");
    eRetcode = IOFUNCS_ERROR_NOT_INIT;
  }
  else
  {
    iRetcode = sem_get(semSerialise, KAL_WAIT_FOREVER);
    
    if (iRetcode == RC_OK)
    {
      bMouseRelative = bRelative;
      
      trace_new(TR_INFO, "IOFUNCS: Mouse mode set to %s\n", bRelative ? "relative" : "absolute");
      
      sem_put(semSerialise);
    }
    else
    {
      trace_new(TR_ERR, "IOFUNCS: Failed to get module access semaphore! rc = 0x%08x\n", iRetcode);
      eRetcode = IOFUNCS_ERROR_INTERNAL;
    }
  }
  return(eRetcode);
}

/********************************************************************/
/*  FUNCTION:    cnxt_iofuncs_is_mouse_relative                     */
/*                                                                  */
/*  PARAMETERS:  pbRelative - pointer to storage for returned value.*/
/*                                                                  */
/*  DESCRIPTION: Set the contents of pbRelative to TRUE if the mouse*/
/*               is currently sending relative movement information */
/*               or FALSE if absolute position.                     */
/*                                                                  */
/*  RETURNS:     IOFUNCS_STATUS_OK      - No errors occured.        */
/*               IOFUNCS_ERROR_NOT_INIT - Module not initialised.   */
/*               IOFUNCS_ERROR_INTERNAL - Internal call failed.     */
/*               IOFUNCS_ERROR_BAD_PTR  - NULL pointer passed       */
/*                                                                  */
/*  CONTEXT:     This function must be called in a non-interrupt    */
/*               context.                                           */
/*                                                                  */
/********************************************************************/
IOFUNCS_STATUS cnxt_iofuncs_is_mouse_relative(bool *pbRelative)
{
  int iRetcode;
  IOFUNCS_STATUS eRetcode = IOFUNCS_STATUS_OK;
  
  trace_new(TR_FUNC, "IOFUNCS: cnxt_iofuncs_get_cursor_pos\n");
  
  #ifdef DEBUG
  not_interrupt_safe();
  #endif
  
  if (!bInitialised)
  {
    trace_new(TR_ERR, "IOFUNCS: Module not initialised before use\n");
    eRetcode = IOFUNCS_ERROR_NOT_INIT;
  }
  else
  {
    iRetcode = sem_get(semSerialise, KAL_WAIT_FOREVER);
    
    if (iRetcode == RC_OK)
    {
      if (pbRelative)
        *pbRelative = bMouseRelative;
      else
      {
        trace_new(TR_ERR, "IOFUNCS: NULL pointer passed\n");
      }
        
      sem_put(semSerialise);
    }
    else
    {
      trace_new(TR_ERR, "IOFUNCS: Failed to get module access semaphore! rc = 0x%08x\n", iRetcode);
      eRetcode = IOFUNCS_ERROR_INTERNAL;
    }
  }
  return(eRetcode);
}

/**************************************************/
/**************************************************/
/** Front panel and remote button press handling **/
/**************************************************/
/**************************************************/

IOFUNCS_STATUS cnxt_iofuncs_getch(int iTimeoutMs, char *pChar)
{
  int iRetcode;
  IOFUNCS_STATUS eRetcode = IOFUNCS_STATUS_OK;
  char cBuffer[2];
  
  trace_new(TR_FUNC, "IOFUNCS: cnxt_iofuncs_getch\n");
  
  #ifdef DEBUG
  not_interrupt_safe();
  #endif
  
  if (!bInitialised)
  {
    trace_new(TR_ERR, "IOFUNCS: Module not initialised before use\n");
    eRetcode = IOFUNCS_ERROR_NOT_INIT;
  }
  else
  {
    iRetcode = sem_get(semSerialise, KAL_WAIT_FOREVER);
    
    if (iRetcode == RC_OK)
    {
      if (pChar)
      {
        /* Here we pass the request to the command task and wait for it */
        /* to signal completion.                                        */
        eRetcode = internal_post_command_message(IOFUNCS_MSG_GET_KEY,  
                                                 (u_int32)cBuffer,
                                                 (u_int32)2,
                                                 (u_int32)iTimeoutMs);
        if (eRetcode == IOFUNCS_STATUS_OK)
        {
          /* Wait for the command task to tell us it is done */
          iRetcode = sem_get(semStringSignal, KAL_WAIT_FOREVER);
          if (iRetcode != RC_OK)
          {
            trace_new(TR_ERR, "IOFUNCS: Signalling problem waiting for string entry!\n");
            eRetcode = IOFUNCS_ERROR_INTERNAL;
          }  
          else
          {
            *pChar = cBuffer[0];
            if(cBuffer[0] == (char)CHAR_TIMEOUT_MARKER)
              eRetcode = IOFUNCS_STATUS_TIMEOUT;
            else
              eRetcode = IOFUNCS_STATUS_OK;  
          }
        }
        else
        {
          /* Problem posting message to the command task */
          trace_new(TR_ERR, "IOFUNCS: Unable to process character request\n");
        }                                         
      }
      else
      {
        trace_new(TR_ERR, "IOFUNCS: NULL pointer passed\n");
        eRetcode = IOFUNCS_ERROR_BAD_PTR;
      }  
      
      sem_put(semSerialise);
    }
    else
    {
      trace_new(TR_ERR, "IOFUNCS: Failed to get module access semaphore! rc = 0x%08x\n", iRetcode);
      eRetcode = IOFUNCS_ERROR_INTERNAL;
    }
  }
  return(eRetcode);
}

/********************************************************************/
/*  FUNCTION:    cnxt_iofuncs_getstr                                */
/*                                                                  */
/*  PARAMETERS:  lpBuffer  - Pointer to buffer for string storage   */
/*               iBuffSize - Size of buffer (incl trailing 0)       */
/*                                                                  */
/*  DESCRIPTION: Capture a string from the keyboard or remote.      */
/*               Function returns when the user presses Enter or    */
/*               Select or the buffer fills.                        */
/*                                                                  */
/*  RETURNS:     IOFUNCS_STATUS_OK       - No errors occured        */
/*               IOFUNCS_ERROR_NOT_INIT  - Module not initialised   */
/*               IOFUNCS_ERROR_INTERNAL  - Internal call failed     */
/*               IOFUNCS_ERROR_BAD_PTR   - NULL pointer passed      */
/*               IOFUNCS_ERROR_BAD_PARAM - Buffer too small         */
/*                                                                  */
/*  CONTEXT:     This function must be called from a non-interrupt  */
/*               context.                                           */
/*                                                                  */
/********************************************************************/
IOFUNCS_STATUS cnxt_iofuncs_getstr(char *lpBuffer, int iBuffSize)
{
  int iRetcode;
  IOFUNCS_STATUS eRetcode = IOFUNCS_STATUS_OK;
  
  trace_new(TR_FUNC, "IOFUNCS: cnxt_iofuncs_getstr\n");
  
  #ifdef DEBUG
  not_interrupt_safe();
  #endif
  
  if (!bInitialised)
  {
    trace_new(TR_ERR, "IOFUNCS: Module not initialised before use\n");
    eRetcode = IOFUNCS_ERROR_NOT_INIT;
  }
  else
  {
    iRetcode = sem_get(semSerialise, KAL_WAIT_FOREVER);
    
    if (iRetcode == RC_OK)
    {
      if (lpBuffer)
      {
        /* Here we pass the request to the command task and wait for it */
        /* to signal completion.                                        */
        if (iBuffSize > 1)
        {
          eRetcode = internal_post_command_message(IOFUNCS_MSG_GET_STRING,  
                                                   (u_int32)lpBuffer,
                                                   (u_int32)iBuffSize,
                                                   (u_int32)bCharEcho);
          if (eRetcode == IOFUNCS_STATUS_OK)
          {
            /* Wait for the command task to tell us it is done */
            iRetcode = sem_get(semStringSignal, KAL_WAIT_FOREVER);
            if (iRetcode != RC_OK)
            {
              trace_new(TR_ERR, "IOFUNCS: Signalling problem waiting for string entry!\n");
              eRetcode = IOFUNCS_ERROR_INTERNAL;
            }  
            
            /* We got a string. Strip off the trailing 0x0D if it is present */
            if(lpBuffer[strlen(lpBuffer)-1] == 0x0D)
              lpBuffer[strlen(lpBuffer)-1] = 0x00;
          }
          else
          {
            /* Problem posting message to the command task */
            trace_new(TR_ERR, "IOFUNCS: Unable to process string request\n");
          }                                         
        }
        else
        {
          trace_new(TR_ERR, "IOFUNCS: Buffer too small to recieve string\n");
          eRetcode = IOFUNCS_ERROR_BAD_PARAM;
        }  
      }
      else
      {
        trace_new(TR_ERR, "IOFUNCS: NULL pointer passed\n");
        eRetcode = IOFUNCS_ERROR_BAD_PTR;
      }  
      
      sem_put(semSerialise);
    }
    else
    {
      trace_new(TR_ERR, "IOFUNCS: Failed to get module access semaphore! rc = 0x%08x\n", iRetcode);
      eRetcode = IOFUNCS_ERROR_INTERNAL;
    }
  }
  return(eRetcode);
}


/********************************************************************/
/*  FUNCTION:    cnxt_iofuncs_press_any_key                         */
/*                                                                  */
/*  PARAMETERS:  None                                               */
/*                                                                  */
/*  DESCRIPTION: Waits for the user to press any key before         */
/*               returning.                                         */
/*                                                                  */
/*  RETURNS:     IOFUNCS_STATUS_OK       - No errors occured        */
/*               IOFUNCS_ERROR_NOT_INIT  - Module not initialised   */
/*               IOFUNCS_ERROR_INTERNAL  - Internal call failed     */
/*                                                                  */
/*  CONTEXT:     Must be called in task context but NOT from the    */
/*               IOFUNCS callback function.                         */
/*                                                                  */
/********************************************************************/
IOFUNCS_STATUS cnxt_iofuncs_press_any_key(void)
{
  char cDummy;
  PFNINPUTDEVICECALLBACK pfnCallback;
  IOFUNCS_STATUS eRetcode;

  /* Unhook any current input callback */
  cnxt_iofuncs_query_input_device_callback(&pfnCallback);
  
  /* Purge the current key queue */
  while(cnxt_iofuncs_getch(0, &cDummy) == IOFUNCS_STATUS_OK);
  
  cnxt_iofuncs_printf("\nPress any key to continue\n");
  
  eRetcode = cnxt_iofuncs_getch(KAL_WAIT_FOREVER, &cDummy);
  
  /* Reinstate any callback registered on entry */
  cnxt_iofuncs_register_input_device_callback(pfnCallback);
  
  return(eRetcode);
}

/********************************************************************/
/*  FUNCTION:    cnxt_iofuncs_char_echo                             */
/*                                                                  */
/*  PARAMETERS:  bOn - Enable echo if TRUE, disable if FALSE        */
/*                                                                  */
/*  DESCRIPTION: Enable or disable the echoing of characters to the */
/*               screen while processing cnxt_iofuncs_getstr calls. */
/*                                                                  */
/*  RETURNS:     IOFUNCS_STATUS_OK      - No errors occured.        */
/*               IOFUNCS_ERROR_NOT_INIT - Module not initialised.   */
/*                                                                  */
/*  CONTEXT:     This functiom must be called from a non-interrupt  */
/*               context.                                           */
/*                                                                  */
/********************************************************************/
IOFUNCS_STATUS cnxt_iofuncs_char_echo(bool bOn)
{
  int iRetcode;
  IOFUNCS_STATUS eRetcode = IOFUNCS_STATUS_OK;
  
  trace_new(TR_FUNC, "IOFUNCS: cnxt_iofuncs_char_echo\n");
  
  #ifdef DEBUG
  not_interrupt_safe();
  #endif
  
  if (!bInitialised)
  {
    trace_new(TR_ERR, "IOFUNCS: Module not initialised before use\n");
    eRetcode = IOFUNCS_ERROR_NOT_INIT;
  }
  else
  {
    iRetcode = sem_get(semSerialise, KAL_WAIT_FOREVER);
    
    if (iRetcode == RC_OK)
    {
      bCharEcho = bOn;
      
      trace_new(TR_INFO, "IOFUNCS: Character echo for string input %s\n", bOn ? "enabled" : "disabled");
      
      sem_put(semSerialise);
    }
    else
    {
      trace_new(TR_ERR, "IOFUNCS: Failed to get module access semaphore! rc = 0x%08x\n", iRetcode);
      eRetcode = IOFUNCS_ERROR_INTERNAL;
    }
  }
  return(eRetcode);
}


/********************************************************************/
/*  FUNCTION:    cnxt_iofuncs_is_char_echo_on                       */
/*                                                                  */
/*  PARAMETERS:  pbOn - Pointer to storage for returned state.      */
/*                                                                  */
/*  DESCRIPTION: Return the state of the character echo setting.    */
/*                                                                  */
/*  RETURNS:     IOFUNCS_STATUS_OK      - No errors occured.        */
/*               IOFUNCS_ERROR_NOT_INIT - Module not initialised.   */
/*               IOFUNCS_ERROR_BAD_PTR  - NULL pointer passed       */
/*                                                                  */
/*  CONTEXT:     This functiom must be called from a non-interrupt  */
/*               context.                                           */
/*                                                                  */
/********************************************************************/
IOFUNCS_STATUS cnxt_iofuncs_is_char_echo_on(bool *pbOn)
{
  int iRetcode;
  IOFUNCS_STATUS eRetcode = IOFUNCS_STATUS_OK;
  
  trace_new(TR_FUNC, "IOFUNCS: cnxt_iofuncs_get_window\n");
  
  #ifdef DEBUG
  not_interrupt_safe();
  #endif
  
  if (!bInitialised)
  {
    trace_new(TR_ERR, "IOFUNCS: Module not initialised before use\n");
    eRetcode = IOFUNCS_ERROR_NOT_INIT;
  }
  else
  {
    iRetcode = sem_get(semSerialise, KAL_WAIT_FOREVER);
    
    if (iRetcode == RC_OK)
    {
      if (pbOn)
      {
        *pbOn = bCharEcho;
      }
      else
      {
        trace_new(TR_ERR, "IOFUNCS: NULL pointer passed\n");
        eRetcode = IOFUNCS_ERROR_BAD_PTR;
      }  
      
      sem_put(semSerialise);
    }
    else
    {
      trace_new(TR_ERR, "IOFUNCS: Failed to get module access semaphore! rc = 0x%08x\n", iRetcode);
      eRetcode = IOFUNCS_ERROR_INTERNAL;
    }
  }
  return(eRetcode);
}

/********************/
/********************/
/**                **/
/** Window control **/
/**                **/
/********************/
/********************/

/********************************************************************/
/*  FUNCTION:    cnxt_iofuncs_cls                                   */
/*                                                                  */
/*  PARAMETERS:  bFullScreen - if TRUE, whole graphics screen is    */
/*                             cleared, if FALSE only the current   */
/*                             display window is affected.          */
/*                                                                  */
/*  DESCRIPTION: Clear the graphics screen or display window with   */
/*               the current background colour.                     */
/*                                                                  */
/*  RETURNS:     IOFUNCS_STATUS_OK      - No errors occured.        */
/*               IOFUNCS_ERROR_NOT_INIT - Module not initialised.   */
/*               IOFUNCS_ERROR_INTERNAL - Internal call failed.     */
/*                                                                  */
/*  CONTEXT:     Must be called from a non-interrupt context.       */
/*                                                                  */
/********************************************************************/
IOFUNCS_STATUS cnxt_iofuncs_cls(bool bFullScreen)
{
  int       iRetcode;
  u_int32   uRetcode;
  GFX_RECT  rectClear;
  GFX_OP    sOp;
  
  
  IOFUNCS_STATUS eRetcode = IOFUNCS_STATUS_OK;
  
  trace_new(TR_FUNC, "IOFUNCS: cnxt_iofuncs_cls\n");
  
  #ifdef DEBUG
  not_interrupt_safe();
  #endif

  if (!bInitialised)
  {
    trace_new(TR_ERR, "IOFUNCS: Module not initialised before use\n");
    eRetcode = IOFUNCS_ERROR_NOT_INIT;
  }
  else
  {
    iRetcode = sem_get(semSerialise, KAL_WAIT_FOREVER);
    
    if (iRetcode == RC_OK)
    {
      /* Clear the current display window or the full screen */
      if (bFullScreen)
      {
        trace_new(TR_INFO, "IOFUNCS: Clearing full screen\n");
        
        rectClear.Left   = 0;
        rectClear.Top    = 0;
        rectClear.Right  = gnOsdMaxWidth;
        rectClear.Bottom = gMyOsdMaxHeight;
      }
      else
      {
        trace_new(TR_INFO, "IOFUNCS: Clearing current display window\n");
        
        rectClear.Left   = rectWindowPixel.left;
        rectClear.Top    = rectWindowPixel.top;
        rectClear.Right  = rectWindowPixel.right;
        rectClear.Bottom = rectWindowPixel.bottom;
      }

      /* Clear the screen with the current background colour */
      sOp.BltControl = GFX_OP_COPY;
      sOp.ROP        = GFX_ROP_COPY;
  
      uRetcode = GfxSolidBlt(pCurrentBitmap,
                             &rectClear,
                             &sOp);
      
      if (uRetcode)
      {
        trace_new(TR_ERR, "IOFUNCS: Error 0x%x from GfxSolidBlt\n", uRetcode);
        eRetcode = IOFUNCS_ERROR_INTERNAL;
      }
      
      /* Move the text cursor back to the origin */
      iTextCursorX = 0;
      iTextCursorY = 0;
      
      sem_put(semSerialise);
    }
    else
    {
      trace_new(TR_ERR, "IOFUNCS: Failed to get module access semaphore! rc = 0x%08x\n", iRetcode);
      eRetcode = IOFUNCS_ERROR_INTERNAL;
    }
  }
  return(eRetcode);
}
/********************************************************************/
/*  FUNCTION:    cnxt_iofuncs_set_window                            */
/*                                                                  */
/*  PARAMETERS:  pRect - pointer to new display window rectangle    */
/*                                                                  */
/*  DESCRIPTION: Set the display window rectangle. This constrains  */
/*               the area of the screen affected by scrolling and   */
/*               calls to cnxt_iofuncs_printf.                      */
/*                                                                  */
/*  RETURNS:     IOFUNCS_STATUS_OK if no error occurred             */
/*               IOFUNCS_ERROR_BAD_PTR if a NULL pointer is passed  */
/*               IOFUNCS_ERROR_NOT_INIT if module not initialised   */
/*               IOFUNCS_ERROR_BAD_PARAM Invalid rectangle passed   */
/*                                                                  */
/*               pRect is updated to show rectangle set.            */
/*                                                                  */
/*  CONTEXT:     Must be called from a non-interrupt context.       */
/*                                                                  */
/********************************************************************/
IOFUNCS_STATUS cnxt_iofuncs_set_window(POSDRECT pRect)
{
  int iRetcode;
  IOFUNCS_STATUS eRetcode = IOFUNCS_STATUS_OK;
  
  trace_new(TR_FUNC, "IOFUNCS: cnxt_iofuncs_set_window\n");
  
  #ifdef DEBUG
  not_interrupt_safe();
  #endif
  
  if (!bInitialised)
  {
    trace_new(TR_ERR, "IOFUNCS: Module not initialised before use\n");
    eRetcode = IOFUNCS_ERROR_NOT_INIT;
  }
  else
  {
    iRetcode = sem_get(semSerialise, KAL_WAIT_FOREVER);
    
    if (iRetcode == RC_OK)
    {
      if (pRect)
      {
        /* Check validity of passed rectangle */
        if ((pRect->left   >= iScreenWidth ) ||
            (pRect->right  <= 0            ) ||
            (pRect->top    >= iScreenHeight) ||
            (pRect->bottom <= 0            ) ||
            (pRect->left   >= pRect->right ) ||
            (pRect->top    >= pRect->bottom))
        {
          eRetcode = IOFUNCS_ERROR_BAD_PARAM;
          trace_new(TR_ERR, "IOFUNCS: Bad rectangle passed (%d, %d, %d, %d)\n", 
                    pRect->left,   
                    pRect->top,   
                    pRect->right,   
                    pRect->bottom);
        }
        else
        {
          /* Rectangle is at least partially on the screen so clip it to */
          /* the screen and update our local copy.                       */
          pRect->left   = max(0, pRect->left);
          pRect->top    = max(0, pRect->top);
          pRect->right  = min(iScreenWidth,  pRect->right);
          pRect->bottom = min(iScreenHeight, pRect->bottom);
          
          rectWindow.left   = pRect->left;
          rectWindow.top    = pRect->top;
          rectWindow.right  = pRect->right;
          rectWindow.bottom = pRect->bottom;
          
          rectWindowPixel.left   = giCellWidth  * pRect->left;
          rectWindowPixel.top    = giCellHeight * pRect->top;
          rectWindowPixel.right  = giCellWidth  * pRect->right;
          rectWindowPixel.bottom = giCellHeight * pRect->bottom;
          
          /* Move the cursor to the top left of the new window */
          iTextCursorX = 0;
          iTextCursorY = 0;
        }
      }
      else
      {
        trace_new(TR_ERR, "IOFUNCS: NULL pointer passed\n");
        eRetcode = IOFUNCS_ERROR_BAD_PTR;
      }  
      
      sem_put(semSerialise);
    }
    else
    {
      trace_new(TR_ERR, "IOFUNCS: Failed to get module access semaphore! rc = 0x%08x\n", iRetcode);
      eRetcode = IOFUNCS_ERROR_INTERNAL;
    }
  }
  return(eRetcode);
}


/********************************************************************/
/*  FUNCTION:    cnxt_iofuncs_get_window                            */
/*                                                                  */
/*  PARAMETERS:  pRect - storage for returned window rectangle      */
/*                                                                  */
/*  DESCRIPTION: Return the currently-set display window rectangle  */
/*               in character cell coordinates.                     */
/*                                                                  */
/*  RETURNS:     IOFUNCS_STATUS_OK if no error occurred             */
/*               IOFUNCS_ERROR_BAD_PTR if a NULL pointer is passed  */
/*               IOFUNCS_ERROR_NOT_INIT if module not initialised   */
/*                                                                  */
/*  CONTEXT:     Must be called from a non-interrupt context.       */
/*                                                                  */
/********************************************************************/
IOFUNCS_STATUS cnxt_iofuncs_get_window(POSDRECT pRect)
{
  int iRetcode;
  IOFUNCS_STATUS eRetcode = IOFUNCS_STATUS_OK;
  
  trace_new(TR_FUNC, "IOFUNCS: cnxt_iofuncs_get_window\n");
  
  #ifdef DEBUG
  not_interrupt_safe();
  #endif
  
  if (!bInitialised)
  {
    trace_new(TR_ERR, "IOFUNCS: Module not initialised before use\n");
    eRetcode = IOFUNCS_ERROR_NOT_INIT;
  }
  else
  {
    iRetcode = sem_get(semSerialise, KAL_WAIT_FOREVER);
    
    if (iRetcode == RC_OK)
    {
      if (pRect)
      {
        pRect->left   = rectWindow.left;
        pRect->top    = rectWindow.top;
        pRect->right  = rectWindow.right;
        pRect->bottom = rectWindow.bottom;
      }
      else
      {
        trace_new(TR_ERR, "IOFUNCS: NULL pointer passed\n");
        eRetcode = IOFUNCS_ERROR_BAD_PTR;
      }  
      
      sem_put(semSerialise);
    }
    else
    {
      trace_new(TR_ERR, "IOFUNCS: Failed to get module access semaphore! rc = 0x%08x\n", iRetcode);
      eRetcode = IOFUNCS_ERROR_INTERNAL;
    }
  }
  return(eRetcode);
}


/********************************************************************/
/*  FUNCTION:    cnxt_iofuncs_scroll_window                         */
/*                                                                  */
/*  PARAMETERS:  eDirection - Direction to scroll the window        */
/*               iNumPix    - Number of pixels to scroll the window */
/*                            by.                                   */
/*                                                                  */
/*  DESCRIPTION: Move the contens of the current display window by  */
/*               a particular number of pixels in a given direction.*/
/*               Clear the newly exposed edge areas with the current*/
/*               background colour.                                 */
/*                                                                  */
/*  RETURNS:     IOFUNCS_STATUS_OK      - No errors occured         */
/*               IOFUNCS_ERROR_INTERNAL - Error from internal func  */
/*               IOFUNCS_ERROR_NOT_INIT - Module not initialised    */
/*                                                                  */
/*  CONTEXT:     Must be called from a non-interrupt context.       */
/*                                                                  */
/********************************************************************/
IOFUNCS_STATUS cnxt_iofuncs_scroll_window(IOFUNCS_SCROLL eDirection, int iNumPix)
{
  int iRetcode;
  IOFUNCS_STATUS eRetcode = IOFUNCS_STATUS_OK;
  
  trace_new(TR_FUNC, "IOFUNCS: cnxt_iofuncs_auto_scroll\n");
  
  #ifdef DEBUG
  not_interrupt_safe();
  #endif
  
  if (!bInitialised)
  {
    trace_new(TR_ERR, "IOFUNCS: Module not initialised before use\n");
    eRetcode = IOFUNCS_ERROR_NOT_INIT;
  }
  else
  {
    iRetcode = sem_get(semSerialise, KAL_WAIT_FOREVER);
    
    if (iRetcode == RC_OK)
    {
      eRetcode = internal_scroll_window(eDirection, iNumPix);
      
      sem_put(semSerialise);
    }
    else
    {
      trace_new(TR_ERR, "IOFUNCS: Failed to get module access semaphore! rc = 0x%08x\n", iRetcode);
      eRetcode = IOFUNCS_ERROR_INTERNAL;
    }
  }
  return(eRetcode);
}

/********************************************************************/
/*  FUNCTION:    cnxt_iofuncs_auto_scroll                           */
/*                                                                  */
/*  PARAMETERS:  bEnable - TRUE to enable automatic scrolling of    */
/*                         the display window, FALSE to disable.    */
/*                                                                  */
/*  DESCRIPTION: Turn on or off automatic scrolling of the display  */
/*               window. If enabled, automatic scrolling will scroll*/
/*               the contents of the current display window upwards */
/*               to accomodate more text drawn at the bottom. If    */
/*               disabled, display will wrap back to the top of the */
/*               window if the cursor moves off the bottom.         */
/*                                                                  */
/*  RETURNS:     IOFUNCS_STATUS_OK      if no errors occurred       */
/*               IOFUNCS_ERROR_NOT_INIT if the module has not been  */
/*                                      initialised.                */
/*                                                                  */
/*  CONTEXT:     Must be called from a non-interrupt context.       */
/*                                                                  */
/********************************************************************/
IOFUNCS_STATUS cnxt_iofuncs_auto_scroll(bool bEnable)
{
  int iRetcode;
  IOFUNCS_STATUS eRetcode = IOFUNCS_STATUS_OK;
  
  trace_new(TR_FUNC, "IOFUNCS: cnxt_iofuncs_auto_scroll\n");
  
  #ifdef DEBUG
  not_interrupt_safe();
  #endif
  
  if (!bInitialised)
  {
    trace_new(TR_ERR, "IOFUNCS: Module not initialised before use\n");
    eRetcode = IOFUNCS_ERROR_NOT_INIT;
  }
  else
  {
    iRetcode = sem_get(semSerialise, KAL_WAIT_FOREVER);
    
    if (iRetcode == RC_OK)
    {
      bAutoScroll = bEnable;
      
      trace_new(TR_INFO, "IOFUNCS: Auto scrolling has been %sabled\n", bAutoScroll ? "en" : "dis");    
      
      sem_put(semSerialise);
    }
    else
    {
      trace_new(TR_ERR, "IOFUNCS: Failed to get module access semaphore! rc = 0x%08x\n", iRetcode);
      eRetcode = IOFUNCS_ERROR_INTERNAL;
    }
  }
  return(eRetcode);
}

/********************/
/* Colour Selection */
/********************/


/********************************************************************/
/*  FUNCTION:    cnxt_iofuncs_set_foreground_color                  */
/*                                                                  */
/*  PARAMETERS:  pColor - new foreground colour to be used          */
/*                                                                  */
/*  DESCRIPTION: Set the graphics foreground colour used for all    */
/*               future text drawing.                               */
/*                                                                  */
/*  RETURNS:     IOFUNCS_STATUS_OK      - No errors reported.       */
/*               IOFUNCS_ERROR_NOT_INIT - Module not initialised.   */
/*               IOFUNCS_ERROR_BAD_PTR  - NULL pointer passed       */
/*                                                                  */
/*  CONTEXT:     Must be called from a non-interrupt context.       */
/*                                                                  */
/********************************************************************/
IOFUNCS_STATUS cnxt_iofuncs_set_foreground_color(PIOFUNCS_COLOR pColor)
{
  int iRetcode;
  IOFUNCS_STATUS eRetcode = IOFUNCS_STATUS_OK;
  u_int32 uColour;
  trace_new(TR_FUNC, "IOFUNCS: cnxt_iofuncs_set_foreground_color\n");
  
  #ifdef DEBUG
  not_interrupt_safe();
  #endif
  
  if (!bInitialised)
  {
    trace_new(TR_ERR, "IOFUNCS: Module not initialised before use\n");
    eRetcode = IOFUNCS_ERROR_NOT_INIT;
  }
  else
  {
    iRetcode = sem_get(semSerialise, KAL_WAIT_FOREVER);
    
    if (iRetcode == RC_OK)
    {

      if (pColor)
      {
        colForeground = *pColor;
      #if  (OSD_MODE == OSD_16ARGB1555)

        uColour = GfxTranslateColor((PGFX_COLOR)pColor, 
                                     (u_int8)GetOSDRgnBpp(hCurrent),
                                     internal_osd_mode_to_gfx_type((u_int8)GetOSDRgnOptions(hCurrent,OSD_RO_MODE)));
      #else
        uColour = GfxTranslateColor((PGFX_COLOR)pColor, 
                                     (u_int8)GetOSDRgnBpp(hCurrent),
                                     (u_int8)GetOSDRgnOptions(hCurrent,OSD_RO_MODE));
      #endif
        GfxSetFGColor(uColour);
        
        /* Fix up the AA font indices so that 8bpp works correctly */
        cAATextIndices[3] = colForeground.cIndex;        
      } 
      else
      {
        trace_new(TR_ERR, "IOFUNCS: NULL pointer passed!\n");
        eRetcode = IOFUNCS_ERROR_BAD_PTR;
      }
      
      sem_put(semSerialise);
    }
    else
    {
      trace_new(TR_ERR, "IOFUNCS: Failed to get module access semaphore! rc = 0x%08x\n", iRetcode);
      eRetcode = IOFUNCS_ERROR_INTERNAL;
    }
  }
  return(eRetcode);
}

/********************************************************************/
/*  FUNCTION:    cnxt_iofuncs_set_background_color                  */
/*                                                                  */
/*  PARAMETERS:  pColor - new background colour to be used          */
/*                                                                  */
/*  DESCRIPTION: Set the graphics background colour used for all    */
/*               future text drawing and screen clearing.           */
/*                                                                  */
/*  RETURNS:     IOFUNCS_STATUS_OK      - No errors reported.       */
/*               IOFUNCS_ERROR_NOT_INIT - Module not initialised.   */
/*               IOFUNCS_ERROR_BAD_PTR  - NULL pointer passed       */
/*                                                                  */
/*  CONTEXT:     Must be called from a non-interrupt context.       */
/*                                                                  */
/********************************************************************/
IOFUNCS_STATUS cnxt_iofuncs_set_background_color(PIOFUNCS_COLOR pColor)
{
  int     iRetcode;
  u_int32 uColour;
  IOFUNCS_STATUS eRetcode = IOFUNCS_STATUS_OK;
  
  trace_new(TR_FUNC, "IOFUNCS: cnxt_iofuncs_set_background_color\n");
  
  #ifdef DEBUG
  not_interrupt_safe();
  #endif
  
  if (!bInitialised)
  {
    trace_new(TR_ERR, "IOFUNCS: Module not initialised before use\n");
    eRetcode = IOFUNCS_ERROR_NOT_INIT;
  }
  else
  {
    iRetcode = sem_get(semSerialise, KAL_WAIT_FOREVER);
    
    if (iRetcode == RC_OK)
    {
      if (pColor)
      {
        colBackground = *pColor;
    #if  (OSD_MODE == OSD_16ARGB1555)
        uColour = GfxTranslateColor((PGFX_COLOR)pColor, 
                                     (u_int8)GetOSDRgnBpp(hCurrent),                                   
                                     internal_osd_mode_to_gfx_type((u_int8)GetOSDRgnOptions(hCurrent,OSD_RO_MODE)));
    #else
        uColour = GfxTranslateColor((PGFX_COLOR)pColor, 
                                     (u_int8)GetOSDRgnBpp(hCurrent),
                                     (u_int8)GetOSDRgnOptions(hCurrent,OSD_RO_MODE));
    #endif
        GfxSetBGColor(uColour);
        
        /* Fix up the AA font indices so that 8bpp works correctly */
        cAATextIndices[0] = colBackground.cIndex;
      }  
      else
      {
        trace_new(TR_ERR, "IOFUNCS: NULL pointer passed!\n");
        eRetcode = IOFUNCS_ERROR_BAD_PTR;
      }
      
      sem_put(semSerialise);
    }
    else
    {
      trace_new(TR_ERR, "IOFUNCS: Failed to get module access semaphore! rc = 0x%08x\n", iRetcode);
      eRetcode = IOFUNCS_ERROR_INTERNAL;
    }
  }
  return(eRetcode);
}

/********************************************************************/
/*  FUNCTION:    cnxt_iofuncs_get_foreground_color                  */
/*                                                                  */
/*  PARAMETERS:  pColor - storage for returned colour.              */
/*                                                                  */
/*  DESCRIPTION: Get the current graphics foreground colour.        */
/*                                                                  */
/*  RETURNS:     IOFUNCS_STATUS_OK      - No errors reported.       */
/*               IOFUNCS_ERROR_NOT_INIT - Module not initialised.   */
/*               IOFUNCS_ERROR_BAD_PTR  - NULL pointer passed       */
/*                                                                  */
/*  CONTEXT:     Must be called from a non-interrupt context.       */
/*                                                                  */
/********************************************************************/
IOFUNCS_STATUS cnxt_iofuncs_get_foreground_color(PIOFUNCS_COLOR pColor)
{
  int iRetcode;
  IOFUNCS_STATUS eRetcode = IOFUNCS_STATUS_OK;
  
  trace_new(TR_FUNC, "IOFUNCS: cnxt_iofuncs_get_foreground_color\n");
  
  #ifdef DEBUG
  not_interrupt_safe();
  #endif
  
  if (!bInitialised)
  {
    trace_new(TR_ERR, "IOFUNCS: Module not initialised before use\n");
    eRetcode = IOFUNCS_ERROR_NOT_INIT;
  }
  else
  {
    iRetcode = sem_get(semSerialise, KAL_WAIT_FOREVER);
    
    if (iRetcode == RC_OK)
    {
      if(pColor)
       *pColor = colForeground;
      else
      {
        trace_new(TR_ERR, "IOFUNCS: NULL pointer passed!\n");
        eRetcode = IOFUNCS_ERROR_BAD_PTR;
      }
      
      sem_put(semSerialise);
    }
    else
    {
      trace_new(TR_ERR, "IOFUNCS: Failed to get module access semaphore! rc = 0x%08x\n", iRetcode);
      eRetcode = IOFUNCS_ERROR_INTERNAL;
    }
  }
  return(eRetcode);
}

/********************************************************************/
/*  FUNCTION:    cnxt_iofuncs_get_background_color                  */
/*                                                                  */
/*  PARAMETERS:  pColor - storage for returned colour.              */
/*                                                                  */
/*  DESCRIPTION: Get the current graphics background colour.        */
/*                                                                  */
/*  RETURNS:     IOFUNCS_STATUS_OK      - No errors reported.       */
/*               IOFUNCS_ERROR_NOT_INIT - Module not initialised.   */
/*               IOFUNCS_ERROR_BAD_PTR  - NULL pointer passed       */
/*                                                                  */
/*  CONTEXT:     Must be called from a non-interrupt context.       */
/*                                                                  */
/********************************************************************/
IOFUNCS_STATUS cnxt_iofuncs_get_background_color(PIOFUNCS_COLOR pColor)
{
  int iRetcode;
  IOFUNCS_STATUS eRetcode = IOFUNCS_STATUS_OK;
  
  trace_new(TR_FUNC, "IOFUNCS: cnxt_iofuncs_get_background_color\n");
  
  #ifdef DEBUG
  not_interrupt_safe();
  #endif
  
  if (!bInitialised)
  {
    trace_new(TR_ERR, "IOFUNCS: Module not initialised before use\n");
    eRetcode = IOFUNCS_ERROR_NOT_INIT;
  }
  else
  {
    iRetcode = sem_get(semSerialise, KAL_WAIT_FOREVER);
    
    if (iRetcode == RC_OK)
    {
      if(pColor)
       *pColor = colBackground;
      else
      {
        trace_new(TR_ERR, "IOFUNCS: NULL pointer passed!\n");
        eRetcode = IOFUNCS_ERROR_BAD_PTR;
      }
      
      sem_put(semSerialise);
    }
    else
    {
      trace_new(TR_ERR, "IOFUNCS: Failed to get module access semaphore! rc = 0x%08x\n", iRetcode);
      eRetcode = IOFUNCS_ERROR_INTERNAL;
    }
  }
  return(eRetcode);
}


/********************************************************************/
/*  FUNCTION:    cnxt_iofuncs_register_input_device_callback        */
/*                                                                  */
/*  PARAMETERS:  pfnCallback - pointer to function which will be    */
/*                             called with all keyboard and mouse   */
/*                             event information. If NULL, any      */
/*                             previously registered hook is        */
/*                             removed.                             */
/*                                                                  */
/*  DESCRIPTION: Register or remove a callback function which will  */
/*               receive all keyboard and mouse messages.           */
/*                                                                  */
/*  RETURNS:     IOFUNCS_STATUS_OK - no errors detected             */
/*               IOFUNCS_ERROR_INTERNAL - internal call failed      */
/*               IOFUNCS_ERROR_NOT_INIT - module not initialised    */
/*                                                                  */
/*  CONTEXT:     Must be called from a non-interrupt context.       */
/*                                                                  */
/********************************************************************/
IOFUNCS_STATUS cnxt_iofuncs_register_input_device_callback(PFNINPUTDEVICECALLBACK pfnCallback)
{
  int iRetcode;
  IOFUNCS_STATUS eRetcode = IOFUNCS_STATUS_OK;
  
  trace_new(TR_FUNC, "IOFUNCS: cnxt_iofuncs_register_input_device_callback\n");
  
  #ifdef DEBUG
  not_interrupt_safe();
  #endif
  
  if (!bInitialised)
  {
    trace_new(TR_ERR, "IOFUNCS: Module not initialised before use\n");
    eRetcode = IOFUNCS_ERROR_NOT_INIT;
  }
  else
  {
    iRetcode = sem_get(semSerialise, KAL_WAIT_FOREVER);
    
    if (iRetcode == RC_OK)
    {
      pfnInputCallback = pfnCallback;
      
      sem_put(semSerialise);
    }
    else
    {
      trace_new(TR_ERR, "IOFUNCS: Failed to get module access semaphore! rc = 0x%08x\n", iRetcode);
      eRetcode = IOFUNCS_ERROR_INTERNAL;
    }
  }
  return(eRetcode);
}

/********************************************************************/
/*  FUNCTION:    cnxt_iofuncs_query_input_device_callback           */
/*                                                                  */
/*  PARAMETERS:  ppfnCallback - storage for returned input device   */
/*                              callback function pointer.          */
/*                                                                  */
/*  DESCRIPTION: Query the current input device callback function   */
/*               pointer. NULL is returned if no callback is        */
/*               currently installed.                               */
/*                                                                  */
/*  RETURNS:     IOFUNCS_STATUS_OK - no errors detected             */
/*               IOFUNCS_ERROR_INTERNAL - internal call failed      */
/*               IOFUNCS_ERROR_NOT_INIT - module not initialised    */
/*               IOFUNCS_ERROR_BAD_PTR  - NULL pointer passed       */
/*                                                                  */
/*  CONTEXT:     Must be called from a non-interrupt context.       */
/*                                                                  */
/********************************************************************/
IOFUNCS_STATUS cnxt_iofuncs_query_input_device_callback(PFNINPUTDEVICECALLBACK *ppfnCallback) 
{
  int iRetcode;
  IOFUNCS_STATUS eRetcode = IOFUNCS_STATUS_OK;
  
  trace_new(TR_FUNC, "IOFUNCS: cnxt_iofuncs_query_input_device_callback\n");
  
  #ifdef DEBUG
  not_interrupt_safe();
  #endif
  
  if (!bInitialised)
  {
    trace_new(TR_ERR, "IOFUNCS: Module not initialised before use\n");
    eRetcode = IOFUNCS_ERROR_NOT_INIT;
  }
  else
  {
    iRetcode = sem_get(semSerialise, KAL_WAIT_FOREVER);
    
    if (iRetcode == RC_OK)
    {
      if(ppfnCallback)
       *ppfnCallback = pfnInputCallback;
      else
      {
        trace_new(TR_ERR, "IOFUNCS: NULL pointer passed!\n");
        eRetcode = IOFUNCS_ERROR_BAD_PTR;
      } 
      
      sem_put(semSerialise);
    }
    else
    {
      trace_new(TR_ERR, "IOFUNCS: Failed to get module access semaphore! rc = 0x%08x\n", iRetcode);
      eRetcode = IOFUNCS_ERROR_INTERNAL;
    }
  }
  return(eRetcode);
}

/**************************/
/**************************/
/**                      **/
/**  Internal Functions  **/
/**                      **/
/**************************/
/**************************/


/********************************************************************/
/*  FUNCTION:    internal_reinit                                    */
/*                                                                  */
/*  PARAMETERS:  pDefFont - pointer to the default font to use. If  */
/*                          NULL, 8x8 is assumed.                   */
/*               uOsdMode - Mode to use for IOFUNCS OSD region.     */
/*                                                                  */
/*  DESCRIPTION: This function reinitialises the IOFUNCS library &  */
/*               creates the OSD region it will use. It also hooks  */
/*               the front panel and IR input. Screen grid sizes    */
/*               are fixed based on the choice of default font.     */
/*                                                                  */
/*  RETURNS:     IOFUNCS_STATUS_OK     if no errors occurred        */
/*               IOFUNCS_ERROR_BAD_MODE if the OSD mode passed is   */
/*                                      not supported.              */
/*                                                                  */
/*  CONTEXT:     Must be called from a non-interrupt context.       */
/*                                                                  */
/********************************************************************/
static IOFUNCS_STATUS internal_reinit(PGFX_FONT pDefFont, u_int32 uOsdMode)
{
  u_int32   uOptions;
  u_int8    cGfxType;
  bool      bRetcode;
  u_int32   uRetcode;
  GFX_RECT  rectClear;
  GFX_OP    sOp;
  IOFUNCS_STATUS eRetcode = IOFUNCS_STATUS_OK;
  
  #ifdef DEBUG
  not_interrupt_safe();
  #endif

  /* Disable the current OSD region */
  bRetcode = SetOSDRgnOptions(hScreen, OSD_RO_ENABLE, FALSE);
      
  if (!bRetcode)
  {
    eRetcode = IOFUNCS_ERROR_INTERNAL;
    return(eRetcode);
  }
  
  /* Check parameters */
  cGfxType = internal_osd_mode_to_gfx_type(uOsdMode);
  if (cGfxType == GFX_MONO)
  {
    trace_new(TR_ERR, "IOFUNCS: Unsupported OSD mode 0x%x requested.\n", uOsdMode);
    return(IOFUNCS_ERROR_BAD_MODE);
  }
  
  /* Set the new default font */
  if(pDefFont)
    pDefaultFont = pDefFont;
  else
    pDefaultFont = &font_8x8;

  GfxGetFontCellSize(pDefaultFont, &giCellWidth, &giCellHeight);
  
  iScreenWidth  = gnOsdMaxWidth/giCellWidth;
  iScreenHeight = gMyOsdMaxHeight/giCellHeight;
  
  /* If the new OSD mode is different from the one we are currently using */
  /* destroy the existing surface and create a new one.                   */
  if (uOsdMode != uIofuncsOsdMode)
  {
    uIofuncsOsdMode = uOsdMode;
  
    /* Get rid of the existing region */
    DestroyOSDRegion(hScreen);
   
    /* Create our full screen OSD region */
    rectOsd.top    = 0;
    rectOsd.left   = 0;
    rectOsd.bottom = gMyOsdMaxHeight;
    rectOsd.right  = gnOsdMaxWidth;
  
    /* Set the palette load option if not a 32bpp mode */
    if(uOsdMode != OSD_MODE_32AYUV)
      uOptions = OSD_RO_LOADPALETTE;
    else
      uOptions = 0;
      
    uOptions |= (OSD_RO_ALPHABOTHVIDEO | OSD_RO_ALPHAENABLE);  
  
    hScreen = CreateOSDRgn(&rectOsd, 
                           uOsdMode, 
                           uOptions, 
                           NULL, 
                           0);
  
    if (!hScreen)
    {
      trace_new(TR_ERR, "IOFUNCS: Unable to create requested OSD region!\n");
      bInitialised = FALSE;
      return(IOFUNCS_ERROR_INTERNAL);
    }
     
    /* Set up a GFX bitmap which represents the screen OSD region */
    cnxt_iofuncs_associate_bitmap(&sScreenBitmap,hScreen);
    pCurrentBitmap=&sScreenBitmap;
    hCurrent=hScreen; /* Use the default handle */
  }
  
  /* Clear the screen */
  rectClear.Left   = 0;
  rectClear.Top    = 0;
  rectClear.Right  = gnOsdMaxWidth;
  rectClear.Bottom = gMyOsdMaxHeight;

  /* Clear the screen with the current background colour */
  sOp.BltControl = GFX_OP_COPY;
  sOp.ROP        = GFX_ROP_COPY;

  uRetcode = GfxSolidBlt(pCurrentBitmap,
                         &rectClear,
                         &sOp);
     
  /* Set initial cursor positions */
  iTextCursorX = 0;
  iTextCursorY = 0;
  iMouseCursorX = gnOsdMaxWidth/2;
  iMouseCursorY = gMyOsdMaxHeight/2;
  
  /* Set the initial display window */
  rectWindow.bottom = gMyOsdMaxHeight / giCellHeight;
  rectWindow.right  = gnOsdMaxWidth  / giCellWidth;
  rectWindow.top    = 0;
  rectWindow.left   = 0;
  
  rectWindowPixel.bottom = gMyOsdMaxHeight;
  rectWindowPixel.right  = gnOsdMaxWidth;
  rectWindowPixel.top    = 0;
  rectWindowPixel.left   = 0;
  
  /* Clear the input device callback */
  pfnInputCallback = NULL;
  
  return(eRetcode);
}

/********************************************************************/
/*  FUNCTION:    internal_osd_mode_to_gfx_type                      */
/*                                                                  */
/*  PARAMETERS:  uMode - OSD mode whose GFX equivalent is sought.   */
/*                                                                  */
/*  DESCRIPTION: From a given OSD mode identifier, determine the    */
/*               matching GFX type.                                 */
/*                                                                  */
/*  RETURNS:     GFX pixel type corresponding to the supplied OSD   */
/*               mode if a match exists. GFX_MONO if the mode is    */
/*               not supported by the graphics engine.              */
/*                                                                  */
/*  CONTEXT:     Must be called from a non-interrupt context.       */
/*                                                                  */
/********************************************************************/
static u_int8 internal_osd_mode_to_gfx_type(u_int32 uMode)
{
u_int8 uType;

   switch (uMode)
    {
      case  OSD_MODE_8ARGB:      uType = GFX_ARGB8;       break;
      case  OSD_MODE_8AYUV:      uType = GFX_AYCC8;       break;
      case  OSD_MODE_16ARGB:     uType = GFX_ARGB16_4444; break;
      case  OSD_MODE_16AYUV:     uType = GFX_AYCC16_4444; break;
      case  OSD_MODE_16RGB:      uType = GFX_ARGB16_0565; break;
      case  OSD_MODE_16YUV655:   uType = GFX_AYCC16_0655; break;
      case  OSD_MODE_16ARGB1555: uType = GFX_ARGB16_1555; break;
      case  OSD_MODE_32ARGB:     uType = GFX_ARGB32;      break;
      case  OSD_MODE_32AYUV:     uType = GFX_AYCC32;      break;
         
      default:
         trace_new(TR_ERR,"IOFUNCS: Unsupported OSD mode 0x%x passed\n", uMode);
         uType = GFX_MONO;
         break;
   }
   
   return (uType);
}


/********************************************************************/
/*  FUNCTION:    internal_printf                                    */
/*                                                                  */
/*  PARAMETERS:   strFormat - printf formatting string              */
/*                Other parameters as required by strFormat content */
/*                                                                  */
/*  DESCRIPTION:  A version of printf used to echo characters       */
/*                during string input.                              */
/*                                                                  */
/*  RETURNS:      IOFUNCS_STATUS_OK     - No errors occured         */
/*                IOFUNCS_ERROR_BAD_PTR - A NULL pointer was passed */
/*                                                                  */
/*  CONTEXT:      Must ONLY be called from the command task.        */
/*                                                                  */
/********************************************************************/
static IOFUNCS_STATUS internal_printf(char *strFormat, ...)
{
  IOFUNCS_STATUS eRetcode = IOFUNCS_STATUS_OK;
  int iNumChars;
  va_list args;
  
  va_start(args, strFormat);
  eRetcode = internal_vprintf_font_at_pixel(pDefaultFont,
                          (rectWindow.left + iTextCursorX) * giCellWidth,
                          (rectWindow.top  + iTextCursorY) * giCellHeight,
                           strFormat,
                           &iNumChars,
                           FALSE,
                           args);
  va_end(args);
  
  return(eRetcode);
}

/********************************************************************/
/*  FUNCTION:    internal_vprintf_font_at_pixel                     */
/*                                                                  */
/*  PARAMETERS:  pFont     - pointer to font to be used             */
/*               x         - left pixel coordinate for string       */
/*               y         - top pixel coordinate for string        */
/*               strFormat - printf-style format string             */
/*               pNumChars - pointer to storage for number of chars */
/*                           rendered or vsprintf error code if -ve */
/*               bClip     - If TRUE, string is clipped to the      */
/*                           current display window and no wrapping */
/*                           or scrolling takes place. If FALSE,    */
/*                           text is wrapped or scrolled if string  */
/*                           extends past the end of the line or    */
/*                           current display window.                */
/*               arg       - variadic argument list                 */
/*                                                                  */
/*  DESCRIPTION: Render a formatted text string at a given position */
/*               on the OSD plane using current foreground and      */
/*               background colours and using the font specified.   */
/*               This function may scroll the display window if the */
/*               supplied text extends past the end of the current  */
/*               line. The text cursor is updated if bClip is set   */
/*               to FALSE.                                          */
/*                                                                  */
/*  RETURNS:     The number of characters drawn in storage pointed  */
/*               to by pNumChars and status code as follows:        */
/*               IOFUNCS_STATUS_OK     - String rendered w/o errors */
/*               IOFUNCS_ERROR_BAD_PTR - A NULL pointer was passed  */
/*                                                                  */
/*  CONTEXT:     Must be called from a non-interrupt context.       */
/*               This function must be called by a client who holds */
/*               the API serialisation semaphore and has previously */
/*               checked that the module has been initialised.      */
/*                                                                  */
/********************************************************************/
static IOFUNCS_STATUS internal_vprintf_font_at_pixel(PGFX_FONT pFont, 
                                                     int       x, 
                                                     int       y, 
                                                     char     *strFormat, 
                                                     int      *pNumChars,
                                                     bool      bClip,
                                                     va_list   arg)
{                                              
  int  iRetcode;
  bool bFinished;
  int  iCharsToEnd, iCharsLeft;
  int  iCharsToPrint, iCharsSubstr;
  char *pcStart;
  IOFUNCS_STATUS eRetcode = IOFUNCS_STATUS_OK;
  OSDRECT rectClip;
  int  iCurrentX, iCurrentY;
  int  iCharWidth, iCharHeight;
  bool bScroll;

  if (strFormat && pNumChars)
  {  
  
    /* Format the string into our buffer */
    iRetcode = vsprintf(szStringBuffer, strFormat, arg);
    
    /* There's no point in rendering a string that has nothing in it */
    if (iRetcode > 0)
    {
      /* What's our clipping rectangle? */
      if (bClip)
      {
        /* If supporting an API with explicit positioning, we set the clip */
        /* rectangle to the whole screen minus the vertical slice to the   */
        /* left of the start point. This means that text will wrap below   */
        /* the start character in the string and not back to the left edge */
        /* of the screen.                                                  */
        rectClip = rectOsd;
        rectClip.left = x;
      }  
      else
        rectClip = rectWindowPixel;
      
      /* Set up for the output loop */
      iCurrentX  = x;
      iCurrentY  = y;
      bFinished  = FALSE;
      pcStart    = szStringBuffer;
      iCharsLeft = iRetcode;

      /* Get the basic character cell size for this font */
      GfxGetFontCellSize(pFont, &iCharWidth, &iCharHeight);
      
      while (!bFinished)
      {
        /* Find the first substring within the formatted string. Look    */
        /* for characters that cause the cursor to move - CR, LF. Note   */
        /* that strcspn also checks for terminating NULLs so iCharSubstr */
        /* will always be a valid number since pcStart always points to  */
        /* a NULL terminated string.                                     */
        iCharsSubstr = strcspn(pcStart, "\x08\x0A\x0D\x00");
        
        /* Only draw if we are inside the clipping window */
        if ((iCurrentX <= (int)rectClip.right  - iCharWidth)  && (iCurrentX >= rectClip.left) &&
            (iCurrentY <= (int)rectClip.bottom - iCharHeight) && (iCurrentY >= rectClip.top))
        {
          /* If the substring starts with a non-control character, print something */
          if (iCharsSubstr)
          {
            iCharsToEnd = internal_count_chars_to_end_of_line(pFont, pcStart, iCharsSubstr, rectClip.right - iCurrentX);
            iCharsToPrint = min(iCharsSubstr, iCharsToEnd);
            eRetcode = internal_draw_string(pFont, iCurrentX, iCurrentY, pcStart, iCharsToPrint);
          
            /* Handle end of line conditions - wrapping or clipping */
            if (bClip)
            {
              /* In this case, we clip substrings at the right edge of the screen and just */
              /* throw away the rest of the text. No cursor adjustments are made.          */
              pcStart += iCharsSubstr;
            }
            else
            {
              /* Here we are dealing with wrapping or scrolling text so handle this */
              pcStart += iCharsToPrint;
            }
            
            /* Adjust the printing position across the line */
            iCurrentX += (iCharWidth * iCharsToPrint);
          }
          
          /* Look at control characters in the substring and adjust the */
          /* cursor and/or print position accordingly.                  */
          
          bScroll = FALSE;
          
          switch (*pcStart)
          {
            /* We reached the end of the string so finish things off */
            case (char)0:
              /* If we ended up at the very end of a line, scroll or wrap */
              /* if bClip is FALSE.                                       */
              if(!bClip && (iCurrentX >= rectClip.right))
              {
                iCurrentY += iCharHeight;
                iCurrentX = rectClip.left;
              }
              bFinished = TRUE;
              break;
              
            /* Backspace */  
            case (char)0x08:
              /* Move one character left along the current line if */
              /* not at the start of the line already.             */
              if(iCurrentX >= ((int)rectClip.left+iCharWidth))
                iCurrentX -= iCharWidth;
              else
              {
                /* If we are at the start of the line and not at the     */
                /* top of the page, move to the end of the previous line */
                if (iCurrentY >= ((int)rectClip.top+iCharHeight))  
                {
                  iCurrentY -= iCharHeight;
                  iCurrentX = (int)rectClip.right - iCharWidth;
                }
              }
              pcStart++;
              break;
              
            /* Carriage return */  
            case (char)0x0D:
              iCurrentX = rectClip.left;
              pcStart++;
              break;
              
            /* Line feed (treated as line feed + carriage return) */  
            case (char)0x0A:
              iCurrentY += iCharHeight;
              iCurrentX = rectClip.left;
              pcStart++;
              break;
            
            /* If anything else is at pcStart, this implies we split a string and */
            /* need to wrap to the next line, scrolling the window if necessary.  */
            default:
              iCurrentX = rectClip.left;
              iCurrentY += iCharHeight;
              break;
          }  /* End switch */
          
          /* Check if we hit the bottom of the window. If so, wrap, scroll or finish */
          /* as appropriate.                                                         */
          if (iCurrentY >= rectClip.bottom)
          {
            if (bClip)
            {
              /* If we are clipping and fall off the end of the screen */
              /* just drop out of the output loop.                     */
              bFinished = TRUE;
            }
            else
            {
              /* We are scrolling or wrapping */
              if (bAutoScroll)
              {
                /* Scroll the window up enough lines for another line of text  */
                /* to be drawn. We always scroll an integral number of default */
                /* characters.                                                 */
                internal_scroll_window(SCROLL_UP, iCharHeight);
                iCurrentY -= iCharHeight;
              }
              else
              {
                /* No scrolling, just wrap to the top of the window */
                iCurrentY = rectClip.top;
              }
            }  
          }
        } 
        else
        {
          /* Printing has dropped off the bottom of the screen or current */
          /* window so we exit the function.                              */
          bFinished = TRUE;
          
        } /* If inside window */ 
      } /* while not finished */
      
      /* Adjust the text cursor to the new position if necessary */
      if (!bClip)
      {
        iTextCursorX = (iCurrentX-rectClip.left)/giCellWidth;
        iTextCursorY = (iCurrentY-rectClip.top)/giCellHeight;
      }
    } /* If any characters to output */

    /* Set return codes as appropriate */
    *pNumChars = iRetcode;
  }
  else
  {
    eRetcode = IOFUNCS_ERROR_BAD_PTR;  
  }  
  
  return(eRetcode);
}


/********************************************************************/
/*  FUNCTION:    internal_vprintf_font_at_pixel                     */
/*                                                                  */
/*  PARAMETERS:  pFont     - pointer to font to be used             */
/*               x         - left pixel coordinate for string       */
/*               y         - top pixel coordinate for string        */
/*               strFormat - printf-style format string             */
/*               pNumChars - pointer to storage for number of chars */
/*                           rendered or vsprintf error code if -ve */
/*               bClip     - If TRUE, string is clipped to the      */
/*                           current display window and no wrapping */
/*                           or scrolling takes place. If FALSE,    */
/*                           text is wrapped or scrolled if string  */
/*                           extends past the end of the line or    */
/*                           current display window.                */
/*               arg       - variadic argument list                 */
/*                                                                  */
/*  DESCRIPTION: Render a formatted text string at a given position */
/*               on the OSD plane using current foreground and      */
/*               background colours and using the font specified.   */
/*               This function may scroll the display window if the */
/*               supplied text extends past the end of the current  */
/*               line. The text cursor is updated if bClip is set   */
/*               to FALSE.                                          */
/*                                                                  */
/*  RETURNS:     The number of characters drawn in storage pointed  */
/*               to by pNumChars and status code as follows:        */
/*               IOFUNCS_STATUS_OK     - String rendered w/o errors */
/*               IOFUNCS_ERROR_BAD_PTR - A NULL pointer was passed  */
/*                                                                  */
/*  CONTEXT:     Must be called from a non-interrupt context.       */
/*               This function must be called by a client who holds */
/*               the API serialisation semaphore and has previously */
/*               checked that the module has been initialised.      */
/*                                                                  */
/********************************************************************/
static IOFUNCS_STATUS internal_vprintf_font_at_pixel_auto_wrap(PGFX_FONT pFont, 
                                                     int       x, 
                                                     int       y, 
                                                     int		x2,
                                                     int		y2,
                                                     u_int8 calculate,
                                                     u_int8 *end,
                                                     char     *strFormat, 
                                                     int      *pNumChars,
                                                     va_list   arg)
{                                              
  int  iRetcode;
  bool bFinished;
  int  iCharsToEnd, iCharsLeft;
  int  iCharsToPrint, iCharsSubstr;
  char *pcStart;
  IOFUNCS_STATUS eRetcode = IOFUNCS_STATUS_OK;
  OSDRECT rectClip;
  int  iCurrentX, iCurrentY;
  int  iCharWidth, iCharHeight;
  bool bScroll;

	if(x2 <= x ||y2 <= y )
	{
		return -1;
	}
	
  if (strFormat && pNumChars)
  {  
  
    /* Format the string into our buffer */
    iRetcode = vsprintf(szStringBuffer, strFormat, arg);
    
    /* There's no point in rendering a string that has nothing in it */
    if (iRetcode > 0)
    {
      /* What's our clipping rectangle? */

        /* If supporting an API with explicit positioning, we set the clip */
        /* rectangle to the whole screen minus the vertical slice to the   */
        /* left of the start point. This means that text will wrap below   */
        /* the start character in the string and not back to the left edge */
        /* of the screen.                                                  */
        rectClip = rectOsd;

		if(x < rectClip.left|| y < rectClip.top||x2 > rectClip.right||y2 > rectClip.bottom)
		{			
			return -1;
		}
		
      /* Set up for the output loop */
      iCurrentX  = x;
      iCurrentY  = y;
      bFinished  = FALSE;
      pcStart    = szStringBuffer;
      iCharsLeft = iRetcode;

      /* Get the basic character cell size for this font */
      GfxGetFontCellSize(pFont, &iCharWidth, &iCharHeight);
      
      while (!bFinished)
      {
        /* Find the first substring within the formatted string. Look    */
        /* for characters that cause the cursor to move - CR, LF. Note   */
        /* that strcspn also checks for terminating NULLs so iCharSubstr */
        /* will always be a valid number since pcStart always points to  */
        /* a NULL terminated string.                                     */
        iCharsSubstr = strcspn(pcStart, "\x08\x0A\x0D\x00");
        
        /* Only draw if we are inside the clipping window */
        if ((iCurrentX <= (int)x2 - iCharWidth)  && (iCurrentX >= x) &&
            (iCurrentY <= (int)y2 - iCharHeight) && (iCurrentY >= y))
        {

          /* If the substring starts with a non-control character, print something */
          while (iCharsSubstr > 0)
          {
            iCharsToEnd = internal_count_chars_to_end_of_line(pFont, pcStart, iCharsSubstr, x2 - iCurrentX);
            iCharsToPrint = min(iCharsSubstr, iCharsToEnd);
			if(calculate == 0)
				{
           		 eRetcode = internal_draw_string(pFont, iCurrentX, iCurrentY, pcStart, iCharsToPrint);
				}
			*pNumChars += iCharsToPrint;
            /* Handle end of line conditions - wrapping or clipping */
            //if (bClip)
            //{
              /* In this case, we clip substrings at the right edge of the screen and just */
              /* throw away the rest of the text. No cursor adjustments are made.          */
              //pcStart += iCharsSubstr;
           // }
         //   else
         //   {
              /* Here we are dealing with wrapping or scrolling text so handle this */
              pcStart += iCharsToPrint;
            //}
            
            /* Adjust the printing position across the line */
            iCurrentX += (iCharWidth * iCharsToPrint);
			if(iCharsToPrint < iCharsSubstr)
				{
					//must print next row
              	iCurrentY += iCharHeight;
             	 	iCurrentX = x;
					if(iCurrentY > y2 - iCharHeight)
					{
						return  eRetcode;
					}
				}
				iCharsSubstr -= iCharsToPrint;
          }
          /* Look at control characters in the substring and adjust the */
          /* cursor and/or print position accordingly.                  */
          
          bScroll = FALSE;
          
          switch (*pcStart)
          {
            /* We reached the end of the string so finish things off */
            case (char)0:
              /* If we ended up at the very end of a line, scroll or wrap */
              bFinished = TRUE;
				if(end)
				{
					*end = 1;
				}
              break;
              
            /* Backspace */  
            case (char)0x08:
              /* Move one character left along the current line if */
              /* not at the start of the line already.             */
              if(iCurrentX >= ((int)x+iCharWidth))
                iCurrentX -= iCharWidth;
              else
              {
                /* If we are at the start of the line and not at the     */
                /* top of the page, move to the end of the previous line */
                if (iCurrentY >= ((int)y+iCharHeight))  
                {
                  iCurrentY -= iCharHeight;
                  iCurrentX = (int)x2 - iCharWidth;
                }
              }
				*pNumChars += 1;
              pcStart++;
              break;
              
            /* Carriage return */  
            case (char)0x0D:
              iCurrentX = x;
				*pNumChars += 1;
              pcStart++;					
              break;
              
            /* Line feed (treated as line feed + carriage return) */  
            case (char)0x0A:
              iCurrentY += iCharHeight;
              iCurrentX = x;
				*pNumChars += 1;
              pcStart++;
              break;
            
            /* If anything else is at pcStart, this implies we split a string and */
            /* need to wrap to the next line, scrolling the window if necessary.  */
            default:
              iCurrentX = x;
              iCurrentY += iCharHeight;
              break;
          }  /* End switch */
          
          /* Check if we hit the bottom of the window. If so, wrap, scroll or finish */
          /* as appropriate.                                                         */
          if (iCurrentY >= y2)
          {
              /* If we are clipping and fall off the end of the screen */
              /* just drop out of the output loop.                     */
              bFinished = TRUE;

          }
        } 
        else
        {
          /* Printing has dropped off the bottom of the screen or current */
          /* window so we exit the function.                              */
          bFinished = TRUE;
          
        } /* If inside window */ 
      } /* while not finished */
      
    } /* If any characters to output */

    /* Set return codes as appropriate */
    *pNumChars = iRetcode;
  }
  else
  {
    eRetcode = IOFUNCS_ERROR_BAD_PTR;  
  }  
  
  return(eRetcode);
}

/********************************************************************/
/*  FUNCTION:    internal_count_chars_to_end_of_line                */
/*                                                                  */
/*  PARAMETERS:  pFont        - Font to be used to print string     */
/*               pString      - Character string which is to be     */
/*                              used in counting characters that    */
/*                              can be fitted into given width.     */
/*               iStringLen   - Number of characters of pString to  */
/*                              consider.                           */
/*               iOutputWidth - Width in pixels of the rectangle    */
/*                              into which the string is to be      */
/*                              output.                             */
/*                                                                  */
/*  DESCRIPTION: Return the number of characters from string pString*/
/*               that may be printed in pixel width iOutputWidth    */
/*               without clipping.                                  */
/*                                                                  */
/*  RETURNS:     The number of characters that may be printed.      */
/*                                                                  */
/*  CONTEXT:     May be called in any context                       */
/*                                                                  */
/********************************************************************/
static int internal_count_chars_to_end_of_line(GFX_FONT *pFont, char *pString, int iStringLen, int iOutputWidth)
{
  int iChars;
  int iPixelsLeft = iOutputWidth;
  int iCharWidth;
  int iCharHeight;
  u_int8 cCurrentChar;
    
  for(iChars = 0; iChars < iStringLen; iChars++)
  {
    cCurrentChar = (u_int8)pString[iChars];
    
    GfxGetFontCharacterSize(pFont, cCurrentChar, &iCharWidth, &iCharHeight);
        
    /* If we ran out of space, exit the loop */
    if(iPixelsLeft < iCharWidth)
      break;
      
    /* Adjust the available space and continue */  
    iPixelsLeft -= iCharWidth;
  }
    
  return(iChars);
}

/********************************************************************/
/*  FUNCTION:    internal_draw_string                               */
/*                                                                  */
/*  PARAMETERS:  pFont    - font to be used in drawing string       */
/*               x        - X pixel coordinate for left of string   */
/*               y        - Y pixel coordinate for top of string    */
/*               pcString - Character string to be rendered.        */
/*               iCount   - Number of characters to render.         */
/*                                                                  */
/*  DESCRIPTION: This function renders iCount characters from the   */
/*               the supplied string to the OSD region at the       */
/*               coordinates supplied. It assumes that all clipping */
/*               and scrolling has been handled at a higher level.  */
/*                                                                  */
/*  RETURNS:     IOFUNCS_STATUS_OK                                  */
/*               IOFUNCS_ERROR_NOT_INIT                             */
/*               IOFUNCS_ERROR_BAD_PTR                              */
/*                                                                  */
/*  CONTEXT:     Must be called from a non-interrupt context. The   */
/*               caller should already be holding the module access */
/*               serialisation semaphore.                           */
/*                                                                  */
/********************************************************************/
static IOFUNCS_STATUS internal_draw_string(PGFX_FONT pFont, int x, int y, 
                                           char *pcString, int iCount)
{
  IOFUNCS_STATUS eRetcode = IOFUNCS_STATUS_OK;
  u_int32        uBlitRet;
  GFX_XY         sDstPt;
  GFX_OP         sOp;
  char           cSaved;

  /* GfxTextBlt assumes that the string is zero terminated but this  */
  /* may not be true. To set this up, we add a dummy zero at the     */
  /* point in the string where we want the blitting to stop. After   */
  /* the blit, we replace whatever was there before. This is safe    */
  /* since we always deal with zero terminated strings at the higher */
  /* level.                                                          */
  
  if (strlen(pcString) > iCount)
  {
    cSaved = pcString[iCount];
    pcString[iCount] = (char)0;
  }  
  else
    cSaved = (char)0;
      
  /* Set up blit parameters */
  sDstPt.X = (u_int16)x;
  sDstPt.Y = (u_int16)y;

  #if(OSD_MODE == OSD_16ARGB1555)
  {
        sOp.BltControl =GFX_OP_TRANS;  // GFX_OP_COPY;
       sOp.ROP        =GFX_ROP_OR;  //GFX_ROP_COPY;
  }
  #else//8bit
  {
       sOp.BltControl =GFX_OP_TRANS;  // GFX_OP_COPY;
       sOp.ROP        =GFX_ROP_OR;  //GFX_ROP_COPY;
  } 
  #endif
  sOp.AlphaUse   = 0;
  
  uBlitRet = GfxTextBltEx( pcString,
                           pFont,
                           pCurrentBitmap,
                           &sDstPt, 
                           &sOp,
                           cAATextIndices);
                         
  if (uBlitRet)
  {
    trace_new(TR_ERR, "IOFUNCS: Error 0x%x from GfxTextBltEx\n", uBlitRet);
    eRetcode = IOFUNCS_ERROR_INTERNAL;
  }
                         
  /* Remove our temporary 0 from the string if necessary */                       
  if(cSaved)
    pcString[iCount] = cSaved;
                           
  return(eRetcode);
}


/********************************************************************/
/*  FUNCTION:    internal_scroll_window                             */
/*                                                                  */
/*  PARAMETERS:  eDirection - Direction to scroll the window        */
/*               iNumPix    - Number of pixels to scroll the window */
/*                                                                  */
/*  DESCRIPTION: Move the contents of the current display window by */
/*               a particular number of pixels in a given direction.*/
/*               Clear the newly exposed edge areas with the current*/
/*               background colour.                                 */
/*                                                                  */
/*  RETURNS:     IOFUNCS_STATUS_OK      - No errors occured         */
/*               IOFUNCS_ERROR_INTERNAL - Error from internal func  */
/*               IOFUNCS_ERROR_NOT_INIT - Module not initialised    */
/*                                                                  */
/*  CONTEXT:     Must be called from a non-interrupt context. The   */
/*               caller must be holding the module serialisation    */
/*               semaphore.                                         */
/*                                                                  */
/********************************************************************/
static IOFUNCS_STATUS internal_scroll_window(IOFUNCS_SCROLL eDirection, 
                                             int iNumPix)
{
  IOFUNCS_STATUS eRetcode = IOFUNCS_STATUS_OK;
  u_int32   uRetcode;
  GFX_RECT  rectSrc;
  GFX_RECT  rectClear;
  GFX_XY    sDstPt;
  GFX_OP    sOp;
  
  /* Copy the current window rectangle to the source and clear rectangles */
  rectSrc.Left     = rectWindowPixel.left;
  rectSrc.Right    = rectWindowPixel.right;
  rectSrc.Top      = rectWindowPixel.top;
  rectSrc.Bottom   = rectWindowPixel.bottom;
  
  rectClear.Left   = rectWindowPixel.left;
  rectClear.Right  = rectWindowPixel.right;
  rectClear.Top    = rectWindowPixel.top;
  rectClear.Bottom = rectWindowPixel.bottom;
  
  sDstPt.X         = rectWindowPixel.left;
  sDstPt.Y         = rectWindowPixel.top;
  
  sOp.BltControl   = GFX_OP_COPY;
  sOp.ROP          = GFX_ROP_COPY;
  sOp.AlphaUse     = 0;
  
  /* Now correct for the direction we are moving */
  
  switch (eDirection)
  {
    case SCROLL_UP:
      rectSrc.Top += iNumPix;
      rectClear.Top = rectClear.Bottom - iNumPix;
      break;
      
    case SCROLL_DOWN:
      sDstPt.Y += iNumPix;
      rectSrc.Bottom -= iNumPix;
      rectClear.Bottom = rectClear.Top + iNumPix;
      sOp.BltControl |= GFX_OP_REVERSE;
      break;
      
    case SCROLL_LEFT:
      rectSrc.Left += iNumPix;
      rectClear.Left = rectClear.Right - iNumPix;
      break;
      
    case SCROLL_RIGHT:
      sDstPt.X += iNumPix;
      rectSrc.Right -= iNumPix;
      rectClear.Right = rectClear.Left + iNumPix;
      break;
  }
  
  /* Move the current contents as appropriate */
  uRetcode = GfxCopyBlt(pCurrentBitmap,
                        &rectSrc,
                        pCurrentBitmap,
                        &sDstPt,
                        &sOp);
  
  if (uRetcode)
  {
    trace_new(TR_ERR, "IOFUNCS: Error 0x%x from GfxCopyBlt\n", uRetcode);
    eRetcode = IOFUNCS_ERROR_INTERNAL;
  }
  
  /* Now clear the exposed edge strip */
  sOp.BltControl = GFX_OP_COPY;
  sOp.ROP        = GFX_ROP_COPY;
  
  uRetcode = GfxSolidBlt(pCurrentBitmap,
                         &rectClear,
                         &sOp);
  if (uRetcode)
  {
    trace_new(TR_ERR, "IOFUNCS: Error 0x%x from GfxSolidBlt\n", uRetcode);
    eRetcode = IOFUNCS_ERROR_INTERNAL;
  }
  
  return(eRetcode);
}                                              


/********************************************************************/
/*  FUNCTION:    internal_post_message                              */
/*                                                                  */
/*  PARAMETERS:  msg - Message identifier                           */
/*               p1  - Message-dependent parameter 1                */
/*               p2  - Message dependent parameter 2                */
/*                                                                  */
/*  DESCRIPTION: Post a keyboard or mouse message to the input      */
/*               queue of the command task.                         */
/*                                                                  */
/*  RETURNS:     Nothing                                            */
/*                                                                  */
/*  CONTEXT:     May be called in any context.                      */
/*                                                                  */
/********************************************************************/
IOFUNCS_STATUS internal_post_command_message(IOFUNCS_MESSAGE msg, u_int32 p1, u_int32 p2, u_int32 p3)
{
  u_int32 uMsg[4];
  int     iRetcode;
  IOFUNCS_STATUS eRetcode = IOFUNCS_STATUS_OK;

  #ifdef DEBUG
  if ((u_int32)msg >= (u_int32)IOFUNCS_NUM_MESSAGES)
  {
    trace_new(TR_ERR, "IOFUNCS: Error - attempt to post illegal command message %d\n", msg);
    error_log(ERROR_WARNING);
  }
  #endif
  
  uMsg[0] = (u_int32)msg;
  uMsg[1] = p1;
  uMsg[2] = p2;
  uMsg[3] = p3;
  
  iRetcode = qu_send(qCommand, uMsg);
  
  if (iRetcode != RC_OK)
  {
    isr_trace_new(TR_ERR, "IOFUNCS: Can't post to command queue. Command ignored! RC = 0x%x\n", iRetcode, 0);
    eRetcode = IOFUNCS_ERROR_INTERNAL;
  }
  
  return(eRetcode);
}  

/********************************************************************/
/*  FUNCTION:    internal_post_callback_message                     */
/*                                                                  */
/*  PARAMETERS:  msg - Message identifier                           */
/*               p1  - Message-dependent parameter 1                */
/*               p2  - Message dependent parameter 2                */
/*                                                                  */
/*  DESCRIPTION: Post a keyboard or mouse message to the client     */
/*               application via another task.                      */
/*                                                                  */
/*  RETURNS:     Nothing                                            */
/*                                                                  */
/*  CONTEXT:     May be called in any context.                      */
/*                                                                  */
/********************************************************************/
static IOFUNCS_STATUS internal_post_callback_message(IOFUNCS_MESSAGE msg, u_int32 p1, u_int32 p2)
{
  u_int32 uMsg[4];
  int     iRetcode;
  IOFUNCS_STATUS eRetcode = IOFUNCS_STATUS_OK;

  #ifdef DEBUG
  if ((u_int32)msg >= (u_int32)IOFUNCS_NUM_MESSAGES)
  {
    trace_new(TR_ERR, "IOFUNCS: Error - attempt to post illegal command message %d\n", msg);
    error_log(ERROR_WARNING);
  }
  #endif
  
  uMsg[0] = (u_int32)msg;
  uMsg[1] = p1;
  uMsg[2] = p2;
  
  iRetcode = qu_send(qCallback, uMsg);
  
  if (iRetcode != RC_OK)
  {
    isr_trace_new(TR_ERR, "IOFUNCS: Can't post to callback queue. Event ignored! RC = 0x%x\n", iRetcode, 0);
    eRetcode = IOFUNCS_ERROR_INTERNAL;
  }
  
  return(eRetcode);
}  


#ifdef DEBUG

/********************************************************************/
/*  FUNCTION:    internal_debug_dump_msg                            */
/*                                                                  */
/*  PARAMETERS:  msg - Message identifier                           */
/*               p1  - First parameter                              */
/*               p2  - Second parameter                             */
/*               p3  - Third parameter                              */
/*                                                                  */
/*  DESCRIPTION: Dump a trace message indicating the type of message*/
/*               and parameters associated with it. For debug use   */
/*               only.                                              */
/*                                                                  */
/*  RETURNS:     Nothing                                            */
/*                                                                  */
/*  CONTEXT:     Must be called from a non-interrupt context.       */
/*                                                                  */
/********************************************************************/
static void internal_debug_dump_msg(IOFUNCS_MESSAGE msg, u_int32 p1, u_int32 p2, u_int32 p3)
{
  not_interrupt_safe();
  
  trace_new(TR_INFO, 
            "IOFUNCS: Message %s (%d), %d, %d, %d\n", 
            internal_debug_map_message_to_string(msg), 
            msg, 
            p1, 
            p2, 
            p3);
}

/********************************************************************/
/*  FUNCTION:    internal_debug_map_message_to_string               */
/*                                                                  */
/*  PARAMETERS:  msg - Message identifier                           */
/*                                                                  */
/*  DESCRIPTION: Given a message ID, return a pointer to a string   */
/*               containing the human-readable name for the message.*/
/*                                                                  */
/*  RETURNS:     Pointer to a string.                               */
/*                                                                  */
/*  CONTEXT:     Must be called from a non-interrupt context.       */
/*                                                                  */
/********************************************************************/
char *internal_debug_map_message_to_string(IOFUNCS_MESSAGE msg)
{
  char *pString;
  
  switch (msg)
  {
    case IOFUNCS_MSG_KEY_PRESSED:         pString = "IOFUNCS_MSG_KEY_PRESSED";         break;   
    case IOFUNCS_MSG_KEY_RELEASED:        pString = "IOFUNCS_MSG_KEY_RELEASED";        break;
    case IOFUNCS_MSG_KEY_HOLD:            pString = "IOFUNCS_MSG_KEY_HOLD";            break;    
    case IOFUNCS_MSG_MOUSE_BTN1_DOWN:     pString = "IOFUNCS_MSG_MOUSE_BTN1_DOWN";     break;
    case IOFUNCS_MSG_MOUSE_BTN1_UP:       pString = "IOFUNCS_MSG_MOUSE_BTN1_UP";       break; 
    case IOFUNCS_MSG_MOUSE_BTN2_DOWN:     pString = "IOFUNCS_MSG_MOUSE_BTN2_DOWN";     break;
    case IOFUNCS_MSG_MOUSE_BTN2_UP:       pString = "IOFUNCS_MSG_MOUSE_BTN2_UP";       break; 
    case IOFUNCS_MSG_MOUSE_MOVE_ABSOLUTE: pString = "IOFUNCS_MSG_MOUSE_MOVE_ABSOLUTE"; break;    
    case IOFUNCS_MSG_MOUSE_MOVE_RELATIVE: pString = "IOFUNCS_MSG_MOUSE_MOVE_RELATIVE"; break;    
    case IOFUNCS_MSG_SHUTDOWN:            pString = "IOFUNCS_MSG_SHUTDOWN";            break;    
    case IOFUNCS_MSG_KEY_TIMEOUT:         pString = "IOFUNCS_MSG_KEY_TIMEOUT";         break;   
    case IOFUNCS_MSG_GET_STRING:          pString = "IOFUNCS_MSG_GET_STRING";          break;    
    case IOFUNCS_MSG_GET_KEY:             pString = "IOFUNCS_MSG_GET_KEY";             break;      
    case IOFUNCS_MSG_SHOW_CURSOR:         pString = "IOFUNCS_MSG_SHOW_CURSOR";         break;      
    case IOFUNCS_MSG_UPDATE_CURSOR:       pString = "IOFUNCS_MSG_UPDATE_CURSOR";       break;      
    default:                              pString = "**ILLEGAL**";                     break;
  }
  return(pString);
}
#endif /* DEBUG */
           

/********************************************************************/
/*  FUNCTION:    internal_command_task                              */
/*                                                                  */
/*  PARAMETERS:  None                                               */
/*                                                                  */
/*  DESCRIPTION: This task is responsible for reading and processing*/
/*               messages from the internal command queue. This     */
/*               includes routing all mouse and keyboard messages,  */
/*               handling string and character input, flashing the  */
/*               text cursor and moving the mouse pointer.          */
/*                                                                  */
/*  RETURNS:     Nothing                                            */
/*                                                                  */
/*  CONTEXT:     This function forms the body of a separate task.   */
/*                                                                  */
/********************************************************************/
static void internal_command_task(void *pIgnored)
{
  u_int32 uMsg[4];
  bool    bExit = FALSE;
  int     iRetcode;
  char *szString = NULL;
  int  iStringMax = 0;
  int  iStringMarker = 0;
  bool bCaptureString = FALSE;
  bool bCaptureEcho   = TRUE;
  bool bCursorDrawn   = FALSE;
  bool bEndCapture    = TRUE;
  tick_id_t tickTimeout;  
  tick_id_t tickCursor;
  OSDCURSORPOS sCurPos;
  
  /* Create the timeout tick timer */
  tickTimeout = tick_create(internal_tick_callback, (void *)IOFUNCS_TIMEOUT_TICK, "IOTK");
  tickCursor  = tick_create(internal_tick_callback, (void *)IOFUNCS_CURSOR_TICK,  "ICTK");
  
  if ((tickCursor == (tick_id_t)0) || (tickTimeout == (tick_id_t)0))
  {
    trace_new(TR_ERR, "IOFUNCS: Can't create required timers!\n");
    error_log(ERROR_FATAL);
    return; /* Should not get here - error_log(ERROR_FATAL) doesn't return*/
  }  
  
  /* Set the period of the cursor timer */
  iRetcode = tick_set(tickCursor, CURSOR_FLASH_PERIOD, FALSE);
  if (iRetcode != RC_OK)
  {
    trace_new(TR_ERR, "IOFUNCS: Can't set cursor flash timer!\n");
    /* We can live with a solid cursor if this fails */
  }  
    
  /* Loop waiting for a command or message to process */
  while (!bExit)
  {
    iRetcode = qu_receive(qCommand, KAL_WAIT_FOREVER, uMsg);
    if (iRetcode != RC_OK)
    {
      trace_new(TR_ERR, "IOFUNCS: Queue receive error 0x%x!\n", iRetcode);
      task_time_sleep(1000);
    }
    else
    {
      #ifdef DEBUG
      if((IOFUNCS_MESSAGE)uMsg[0] != IOFUNCS_MSG_UPDATE_CURSOR)
        internal_debug_dump_msg((IOFUNCS_MESSAGE)uMsg[0], uMsg[1], uMsg[2], uMsg[3]);
      #endif
      
      switch ((IOFUNCS_MESSAGE)uMsg[0])
      {
        /******************************************************************/
        /* We are being told to shut ourselves down so go ahead and do it */
        /******************************************************************/
        case IOFUNCS_MSG_SHUTDOWN:
          trace_new(TR_INFO, "IOFUNCS: Command task ending\n");
          
          /* Destroy resources created by this task */
          tick_destroy(tickCursor);
          tick_destroy(tickTimeout);
          
          /* Signal the person who called is that we are dead (almost) */
          if(uMsg[1]);
            sem_put(uMsg[1]);
          
          /* Kill ourselves off */
          task_terminate();
          
          break;
        
        /****************************************************************/
        /* Key press - pass the key on or add it to string input buffer */
        /* Key hold - the same as key press                             */
        /****************************************************************/
        case IOFUNCS_MSG_KEY_HOLD:
        case IOFUNCS_MSG_KEY_PRESSED:
          if (bCaptureString)
          {
            if (((char)uMsg[1] == 0x0D) || (uMsg[1] == CNXT_ENTER) || (uMsg[1] == CNXT_MENU))
            {
              /* If enter was pressed, terminate the string */
              szString[iStringMarker++] = 0x0D;
              if(iStringMarker != iStringMax)
                szString[iStringMarker] = '\0';
              bEndCapture = TRUE;
            }  
            else
            {
              /* Only capture the keystroke if it is a printing character */
              if ((uMsg[1] >= 0x20) && (uMsg[1] <= 0x7F))
              {
                /* Copy the new keystroke to the buffer */
                szString[iStringMarker] = (char)uMsg[1];
                iStringMarker++;
              
                /* If echoing the input, draw the character to the screen */
                if (bCaptureEcho)
                {
                  /* Ensure the cursor state is currently inactive to save */
                  /* potential drawing artifacts.                          */
                  if (bCursorShown && bCursorDrawn)
                    bCursorDrawn = internal_draw_cursor(iTextCursorX, iTextCursorY, bCursorDrawn);
                
                  internal_printf("%c", (char)uMsg[1]);
                }  
              
                /* If we reached the end of the buffer, stop capturing */
                if(iStringMarker == iStringMax)
                  bEndCapture = TRUE;
              }
              else
              {
                /* Handle any special cases */
                
                if (uMsg[1] == 0x08)   /* Backspace */
                {
                  if (iStringMarker)
                  {
                    /* Ensure the cursor state is currently inactive to save */
                    /* potential drawing artifacts.                          */
                    if (bCursorShown && bCursorDrawn)
                      bCursorDrawn = internal_draw_cursor(iTextCursorX, iTextCursorY, bCursorDrawn);
                    
                    iStringMarker--;
                    internal_printf("%c %c", 8, 8);
                  }  
                }
              }  
            }  
          }    
          else /* Send event to callback handler if we are not capturing a string */
          {
            /* Call any hook function for this key event */
            if(pfnInputCallback)
            {
              internal_post_callback_message((IOFUNCS_MESSAGE)uMsg[0], uMsg[1], uMsg[2]);
            }
          }    
          break;
          
        /*****************************************************/
        /* Key released - pass to the input handler callback */
        /*****************************************************/
        case IOFUNCS_MSG_KEY_RELEASED:
          /* If capture ended on the last character, signal the caller */
          if (bCaptureString && bEndCapture)
          {
            bCaptureString = FALSE;
            sem_put(semStringSignal);
            iRetcode = tick_stop(tickTimeout);
          }  
          else
          {
            /* Call any hook function for this key event */
            if(pfnInputCallback && !bCaptureString)
              internal_post_callback_message((IOFUNCS_MESSAGE)uMsg[0], uMsg[1], uMsg[2]);
          }    
          break;
          
      
        /*************************************************************************/
        /* Mouse move message - here we update the pointer position if necessary */      
        /*************************************************************************/
        case IOFUNCS_MSG_MOUSE_MOVE_ABSOLUTE:
          sCurPos.x = uMsg[1];
          sCurPos.y = uMsg[2];
          curSetPos(&sCurPos);
          
          /* Are we supposed to send back an absolute or relative mouse message? */
          if(pfnInputCallback)
          {
            if(bMouseRelative)
              internal_post_callback_message(IOFUNCS_MSG_MOUSE_MOVE_RELATIVE, (uMsg[3] & 0xFFFF), (uMsg[3] >> 16));
            else
              internal_post_callback_message(IOFUNCS_MSG_MOUSE_MOVE_ABSOLUTE, uMsg[1], uMsg[2]);
          }  
          break;
        
        /*************************/
        /* Mouse button messages */
        /*************************/
        case IOFUNCS_MSG_MOUSE_BTN1_DOWN:
        case IOFUNCS_MSG_MOUSE_BTN1_UP:
        case IOFUNCS_MSG_MOUSE_BTN2_DOWN:
        case IOFUNCS_MSG_MOUSE_BTN2_UP:
          if(pfnInputCallback)
            internal_post_callback_message((IOFUNCS_MESSAGE)uMsg[0], uMsg[1], uMsg[2]);
          break;
              
        /****************************************/      
        /* The key entry tick timer has expired */
        /****************************************/      
        case IOFUNCS_MSG_KEY_TIMEOUT:
          if (bCaptureString)
          {
            trace_new(TR_INFO, "IOFUNCS: Get character timed out\n");
            szString[0] = (char)CHAR_TIMEOUT_MARKER;
            bCaptureString = FALSE;
            bEndCapture    = TRUE;
            sem_put(semStringSignal);
          }
          break;
          
        /*******************************************/  
        /* Start gathering characters for a string */
        /*******************************************/  
        case IOFUNCS_MSG_GET_STRING:
          if (bCaptureString)
          {
            trace_new(TR_ERR, "IOFUNCS: Request to capture a string before last capture completed!\n");
          }
          else
          {
            szString = (char *)uMsg[1];
            bCaptureString = TRUE;
            bEndCapture    = FALSE;
            bCaptureEcho   = (bool)uMsg[3];
            iStringMax     = (u_int32)uMsg[2]-1; /* max # characters we can capture, excluding the NULL */
            szString[iStringMax-1] = (char)0; /* Add a terminating NULL at the end of the buffer */
            iStringMarker  = 0;
            trace_new(TR_INFO, 
                      "IOFUNCS: Capturing string to 0x%08x, length %d, echo %s\n",
                      szString,
                      iStringMax,
                      bCaptureEcho ? "on" : "off");
          }  
          break;

        /**************************/
        /* Get a single keystroke */
        /**************************/
        case IOFUNCS_MSG_GET_KEY:
          if (bCaptureString)
          {
            trace_new(TR_ERR, "IOFUNCS: Request to capture a character before last capture completed!\n");
          }
          else
          {
            /* Set up for a string capture of 1 character */
            
            szString = (char *)uMsg[1];
            bCaptureString = TRUE;
            bEndCapture    = FALSE;
            bCaptureEcho   = FALSE;
            iStringMax     = 1;
            szString[1]    = (char)0;
            iStringMarker  = 0;
            
            /* If a timeout is specified, start the tick timer */
            if (uMsg[3] != KAL_WAIT_FOREVER)
            {
              trace_new(TR_INFO, 
                      "IOFUNCS: Waiting for character. Timeout %d milliseconds\n", uMsg[3]);
                
              /* Make sure we don't ignore really short timeouts */        
              if(uMsg[3] < 10)
                uMsg[3] = 10;
                        
              iRetcode = tick_set(tickTimeout, uMsg[3], TRUE);
              if(iRetcode == RC_OK)
                iRetcode = tick_start(tickTimeout);
              
              if (iRetcode != RC_OK)
              {
                trace_new(TR_ERR, "IOFUNCS: Unable to start getch timeout timer! RC = 0x%x\n", iRetcode);
              }  
            }      
            else
            {
              trace_new(TR_INFO, 
                      "IOFUNCS: Waiting for character. No timeout\n");
            }
          }  
          break;
        
        /**************************************************/
        /* Enable or disable mouse pointer or text cursor */
        /**************************************************/
        case IOFUNCS_MSG_SHOW_CURSOR:
          if (uMsg[1] == (u_int32)CURSOR_TEXT)
          {
            /* Text cursor */
            if (uMsg[2] == TRUE)
            {
              /* Make the text cursor visible */
              if (!bCursorShown)
              {
                bCursorShown = TRUE;
                bCursorDrawn =  internal_draw_cursor(iTextCursorX, iTextCursorY, bCursorDrawn);
                
                /* Start the timer that flashes the cursor */
                tick_start(tickCursor);
              }  
            }
            else
            {
              /* Stop the update timer */
              tick_stop(tickCursor);
              
              /* Hide the text cursor */
              if (bCursorShown && bCursorDrawn)
              {
                /* If the cursor is displayed, remove it */
                bCursorDrawn = internal_draw_cursor(iTextCursorX, iTextCursorY, bCursorDrawn);
              }
              bCursorShown = FALSE;
            }
          }
          else
          {
            /* Mouse cursor */
            if (uMsg[2] == TRUE)
            {
              /* Make the mouse cursor visible. Don't check for already visible here */
              /* since this command is also used to change the mouse pointer bitmap  */
              sCurPos.x = iMouseCursorX;
              sCurPos.y = iMouseCursorY;
              curSetPos(&sCurPos);
              bMouseShown = curShowCursor(hCursor, TRUE);
            }
            else
            {
              /* Hide the mouse cursor */
              curShowCursor(hCursor, FALSE);
              bMouseShown = FALSE;
            }
          }
          
          /* Signal that we are finished by putting the supplied semaphore */
          sem_put((sem_id_t)uMsg[3]);  
          
          break;
          
        /******************************************/  
        /* Flash the text cursor if it is enabled */
        /******************************************/  
        case IOFUNCS_MSG_UPDATE_CURSOR:
          if (bCursorShown)
          {
            bCursorDrawn = internal_draw_cursor(iTextCursorX, iTextCursorY, bCursorDrawn);
          }
          break;
            
        /*****************************/  
        /* Illegal message received! */
        /*****************************/  
        default:                          
          trace_new(TR_ERR, "IOFUNCS: Undefined message %d recieved!\n", uMsg[0]);
          error_log(ERROR_WARNING);
          break;
      }
    }
  }  
}


/********************************************************************/
/*  FUNCTION:    internal_callback_task                             */
/*                                                                  */
/*  PARAMETERS:  Ignored                                            */
/*                                                                  */
/*  DESCRIPTION: This small task ensures that all callback to the   */
/*               IOFUNCS client are made on a different context     */
/*               from the main command task. This allows clients    */
/*               to call API functions from the callback without    */
/*               locking things solid.                              */
/*                                                                  */
/*  RETURNS:     Doesn't                                            */
/*                                                                  */
/*  CONTEXT:     This is a main task loop.                          */
/*                                                                  */
/********************************************************************/
static void internal_callback_task(void *pIgnored)
{
  int iRetcode;
  u_int32 uMsg[4];
  bool ks;
  PFNINPUTDEVICECALLBACK pfnCallback;
  
  trace_new(TR_INFO, "IOFUNCS: Internal callback task starting up\n");
  
  while (1)
  {
    iRetcode = qu_receive(qCallback, KAL_WAIT_FOREVER, uMsg);
    if (iRetcode != RC_OK)
    {
      trace_new(TR_ERR, "IOFUNCS: Callback queue receive error 0x%x!\n", iRetcode);
      task_time_sleep(1000);
    }
    #ifdef DEBUG
    else
    {
      trace_new(TR_FUNC, "IOFUNCS: Callback task got message %s\n",internal_debug_map_message_to_string((IOFUNCS_MESSAGE)uMsg[0]));
    }
    #endif
    
    /* Take a copy of the callback function pointer and use this  */
    /* to close a race condition involving the app clearing the   */
    /* handler between our check and the call actually being made */
    ks = critical_section_begin();
    pfnCallback = pfnInputCallback;
    critical_section_end(ks);
   
    /* Call back into the app if a handler is hooked */
    if (pfnCallback)
    {
      trace_new(TR_FUNC, "IOFUNCS: -> Callback\n");
      pfnCallback((IOFUNCS_MESSAGE)uMsg[0], uMsg[1], uMsg[2]);
      trace_new(TR_FUNC, "IOFUNCS: <- Callback\n");
    }  
    
    if(uMsg[0] == IOFUNCS_MSG_SHUTDOWN)
      break;  
  }    
  
  /* We were signalled to end so do it */
  trace_new(TR_INFO, "IOFUNCS: Callback handler task shutting down\n");
  
  /* Signal the semaphore whose handle we were passed */
  if(uMsg[1]);
    sem_put(uMsg[1]);
  
  task_terminate();
}

/********************************************************************/
/*  FUNCTION:    internal_tick_callback                             */
/*                                                                  */
/*  PARAMETERS:  hTick     - Handle of tick timer that has fired    */
/*               pUserData - User data passed on tick_create        */
/*                                                                  */
/*  DESCRIPTION: Handle callbacks from tick timers used by the      */
/*               module.                                            */
/*                                                                  */
/*  RETURNS:     Nothing                                            */
/*                                                                  */
/*  CONTEXT:     Will likely be called in interrupt context.        */
/*                                                                  */
/********************************************************************/
static void internal_tick_callback(tick_id_t hTick, void *pUserData)
{
  IOFUNCS_STATUS eRetcode = IOFUNCS_STATUS_OK;
  
  switch ((u_int32)pUserData)
  {
    /***********************************************/
    /* Timeout waiting for a character to be typed */
    /***********************************************/
    case IOFUNCS_TIMEOUT_TICK:
      eRetcode = internal_post_command_message(IOFUNCS_MSG_KEY_TIMEOUT, 0, 0, 0);
      break;
    
    /*********************/  
    /* Cursor flash tick */
    /*********************/  
    case IOFUNCS_CURSOR_TICK:
      eRetcode = internal_post_command_message(IOFUNCS_MSG_UPDATE_CURSOR, 0, 0, 0);
      break;
      
    /*********************************/
    /* Unrecognised tick identifier! */
    /*********************************/
    default:
      isr_trace_new(TR_ERR, "IOFUNCS: Bad tick identifier %d!\n", (u_int32)pUserData, 0);
      isr_error_log(ERROR_WARNING);
      return;
  }
  
  if (eRetcode != IOFUNCS_STATUS_OK)
  {
    isr_trace_new(TR_ERR, "IOFUNCS: Failed to post to command queue!\n", 0, 0);
    isr_error_log(ERROR_WARNING);
  }
}


/********************************************************************/
/*  FUNCTION:    internal_cursor_show                               */
/*                                                                  */
/*  PARAMETERS:  eCursor - CURSOR_TEXT or CURSOR_POINTER            */
/*               bShow   - TRUE to show the cursor, FALSE to hide it*/
/*                                                                  */
/*  DESCRIPTION: Show or hide the text cursor or mouse pointer by   */
/*               sending a message to the command task and waiting  */
/*               for it to carry out the opertion.                  */
/*                                                                  */
/*  RETURNS:     IOFUNCS_STATUS_OK if no error occured              */
/*               IOFUNCS_ERROR_INTERNAL if an internal call failed. */
/*                                                                  */
/*  CONTEXT:     Should be called from process context by the       */
/*               holder of the serialisation semaphore.             */
/*                                                                  */
/********************************************************************/
static IOFUNCS_STATUS internal_cursor_show(IOFUNCS_CURSOR eCursor, bool bShow)
{
  int iRetcode;
  IOFUNCS_STATUS eRetcode;
  
  /* Send the command to the command task */
  eRetcode = internal_post_command_message(IOFUNCS_MSG_SHOW_CURSOR, (u_int32)eCursor, (u_int32)bShow, semCursor);
 
  if (eRetcode == IOFUNCS_STATUS_OK)
  {
    /* Wait for its response */
    iRetcode = sem_get(semCursor, KAL_WAIT_FOREVER);
    if(iRetcode != RC_OK)
      eRetcode = IOFUNCS_ERROR_INTERNAL;
    else
      eRetcode = IOFUNCS_STATUS_OK;
  }
  return(eRetcode);    
}

/********************************************************************/
/*  FUNCTION:    internal_draw_cursor                               */
/*                                                                  */
/*  PARAMETERS:  x      - X coordinate of cursor cell               */
/*               y      - Y coordinate of cursor cell               */
/*               bState - Current draw state                        */
/*                                                                  */
/*  DESCRIPTION: Draw the text cursor at cell position (x,y)        */
/*               relative to the current display window. Return a   */
/*               toggled state variable allowing us to correctly    */
/*               erase.                                             */
/*                                                                  */
/*  RETURNS:     The inverse of the state passed.                   */
/*                                                                  */
/*  CONTEXT:     This function must only be called by the command   */
/*               task;                                              */
/*                                                                  */
/********************************************************************/
static bool internal_draw_cursor(int x, int y, bool bState)
{
  GFX_RECT rectCursor;
  GFX_OP  sOp;
  u_int32 uRetcode;

  /* Cursor rectangle */
  rectCursor.Left   = (rectWindow.left + x) * giCellWidth;
  rectCursor.Top    = (rectWindow.top  + y) * giCellHeight;
  rectCursor.Right  = rectCursor.Left + giCellWidth;
  rectCursor.Bottom = rectCursor.Top  + giCellHeight;
      
  /* Invert this area of the screen */
  /* Now clear the exposed edge strip */
  sOp.BltControl = GFX_OP_ROP;
  sOp.ROP        = GFX_ROP_XOR;
  
  uRetcode = GfxSolidBlt(pCurrentBitmap,
                         &rectCursor,
                         &sOp);
  
  return(!bState);
}      

/****************************************************************************
 * Modifications:
 * $Log: 
 *  27   mpeg      1.26        11/13/03 3:20:22 PM    Dave Wilson     CR(s): 
 *        7939 7940 Updated to use new GXAGFX APIs GfxGetFontCharacterSize and 
 *        GfxGetFontCellSize rather than several copies of similar code within 
 *        the app itself.
 *        
 *  26   mpeg      1.25        9/2/03 7:25:04 PM      Joe Kroesche    SCR(s) 
 *        7415 :
 *        reordered header files to eliminate warnings when building for PSOS
 *        
 *  25   mpeg      1.24        7/31/03 3:54:32 PM     Steven Jones    SCR(s): 
 *        5046 
 *        Add functionality for redirecting to different osd regions.
 *        
 *  24   mpeg      1.23        6/18/03 3:22:42 PM     Dave Wilson     SCR(s): 
 *        6782 
 *        Removed 2 redundant parameters (foreground and background indices) 
 *        from
 *        cnxt_iofuncs_set_aa_indices. These are already specified when the 
 *        colours
 *        are set using cnxt_iofuncs_set_foreground/background_color.
 *        
 *  23   mpeg      1.22        4/8/03 10:53:26 AM     Dave Wilson     SCR(s) 
 *        5974 :
 *        Text cursor positioning was incorrect in cases where the default font
 *         was
 *        of type GFX_FONTTYPE_AAFIXED. This has been corrected.
 *        
 *  22   mpeg      1.21        2/13/03 12:03:22 PM    Matt Korte      SCR(s) 
 *        5479 :
 *        Removed old header reference
 *        
 *  21   mpeg      1.20        10/10/02 10:30:16 AM   Dave Wilson     SCR(s) 
 *        4772 :
 *        Added cnxt_iofuncs_set_aa_indices function to allow a client to tell 
 *        IOFUNCS
 *        which palette indices to use when drawing antialiased fonts.
 *        
 *  20   mpeg      1.19        9/25/02 10:16:52 PM    Carroll Vance   SCR(s) 
 *        3786 :
 *        
 *        Removing old DRM and AUD conditional bitfield code.
 *        
 *  19   mpeg      1.18        9/3/02 7:56:04 PM      Matt Korte      SCR(s) 
 *        4498 :
 *        Remove warnings
 *        
 *  18   mpeg      1.17        8/22/02 4:09:38 PM     Dave Wilson     SCR(s) 
 *        4319 :
 *        Added cnxt_iofuncs_get_bounding_rect function to allow an application
 *         to 
 *        query the rectangle that a string will render into in a given font.
 *        
 *  17   mpeg      1.16        8/2/02 1:07:02 PM      Dave Wilson     SCR(s) 
 *        4321 :
 *        Fixed a piece of code which looked at the GFX_FONT version number. It
 *         was
 *        looking for "==2" when it should have been ">=2" to allow for newer 
 *        versions
 *        of the structure.
 *        
 *  16   mpeg      1.15        8/2/02 12:15:18 PM     Dave Wilson     SCR(s) 
 *        4314 :
 *        Added API cnxt_iofuncs_get_gfx_bitmap to allow client to query the 
 *        GFX
 *        bitmap describing the current IOFUNCS OSD region.
 *        
 *  15   mpeg      1.14        7/31/02 5:45:14 PM     Dave Wilson     SCR(s) 
 *        3044 :
 *        Support added for variable width font handling.
 *        
 *  14   mpeg      1.13        6/19/02 10:39:10 AM    Dave Wilson     SCR(s) 
 *        3993 :
 *        Added 2 new APIs - cnxt_iofuncs_set_mouse_relative which results in 
 *        relative
 *        mouse movement being reported rather than absolute cursor position 
 *        and
 *        cnxt_iofuncs_is_mouse_relative to return the current mouse mode. Also
 *         changed
 *        the original IOFUNCS_MSG_MOUSE_MOVE, splitting it in 2 to report the 
 *        mouse
 *        movement in the 2 different ways. Now the messages are MOVE_RELATIVE 
 *        and
 *        MOVE_ABSOLUTE.
 *        
 *  13   mpeg      1.12        5/21/02 1:46:04 PM     Carroll Vance   SCR(s) 
 *        3786 :
 *        Removed DRM bitfields.
 *        
 *  12   mpeg      1.11        3/12/02 4:26:26 PM     Dave Wilson     SCR(s) 
 *        3347 :
 *        Fixed code so that keyboard event callbacks are not made while the 
 *        module
 *        is processing a getstr or getch call. This prevents a race condition 
 *        where
 *        a phantom key release message was getting through to the callback 
 *        just after
 *        a string was completed resulting in some annoying behaviour on the 
 *        uCOS version
 *        of DMFRONT.
 *        
 *  11   mpeg      1.10        3/8/02 4:18:50 PM      Dave Wilson     SCR(s) 
 *        3290 :
 *        y
 *        Used #defines from hwconfig.h (or ucosconf.h) to control stack size, 
 *        priority
 *        and name for the 2 tasks created by the module.
 *        
 *  10   mpeg      1.9         3/6/02 11:39:38 AM     Dave Wilson     SCR(s) 
 *        3315 :
 *        
 *        
 *        
 *        
 *        
 *        
 *        Oops - forgot to set the correct function return code from 
 *        cnxt_iofuncs_press_any_key in the last edit.
 *        
 *  9    mpeg      1.8         3/6/02 11:27:16 AM     Dave Wilson     SCR(s) 
 *        3315 :
 *        Added cnxt_iofuncs_press_any_key, an API providing an easy way to 
 *        access
 *        "Press any key to continue" functionality. The function makes sure 
 *        that the
 *        input queue is flushed before waiting so you don't have to worry 
 *        about type-
 *        ahead.
 *        
 *  8    mpeg      1.7         2/27/02 3:32:42 PM     Dave Wilson     SCR(s) 
 *        3254 :
 *        Added new cnxt_iofuncs_screen_display function to replace existing 2 
 *        APIs
 *        to show and hide the OSD. Left the old functions in the header as 
 *        macros to
 *        save problems with apps that use the old API names. Also added 
 *        function to
 *        allow default font and OSD mode to be queried 
 *        cnxt_iofuncs_get_font_and_mode.
 *        
 *  7    mpeg      1.6         1/8/02 4:22:58 PM      Dave Wilson     SCR(s) 
 *        3005 :
 *        Ignore bogus keystrokes for key code 0 from IR driver.
 *        Only report a key as having been pressed or a string as having been 
 *        entered
 *        after receiving the key up event for the last key. This prevents the 
 *        calling
 *        app from getting a key up event for the last character typed during 
 *        calls to
 *        cnxt_iofuncs_getstr or cnxt_iofuncs_getch after these calls have 
 *        returned.
 *        Changed the way that character timeouts are reported (use -1 as a 
 *        marker now
 *        rather than 0) and also ensured that "Enter" keycode is returned 
 *        correctlyy
 *        when it is the key pressed during cnxt_iofuncs_getch.
 *        
 *  6    mpeg      1.5         1/7/02 2:36:30 PM      Dave Wilson     SCR(s) 
 *        3002 :
 *        Completed mouse cursor handling code and added new API 
 *        cnxt_iofuncs_set_mouse_cursor
 *        to allow clients to set their own cursor images.
 *        
 *  5    mpeg      1.4         1/3/02 3:57:48 PM      Dave Wilson     SCR(s) 
 *        2995 :
 *        Added new cnxt_iofuncs_shutdown API and code to allow the internal 
 *        tasks to
 *        be terminated.
 *        Changed cnxt_iofuncs_init to allow the API to be called again with a 
 *        different
 *        OSD mode and/or default font.
 *        
 *  4    mpeg      1.3         1/3/02 1:44:20 PM      Dave Wilson     SCR(s) 
 *        2997 :
 *        Fixed a problem where stray copies of the text cursor could be left 
 *        lying
 *        around if backspace was used to delete characters during 
 *        cnxt_iofuncs_getstr.
 *        
 *  3    mpeg      1.2         1/3/02 1:37:54 PM      Dave Wilson     SCR(s) 
 *        2996 :
 *        Added new API cnxt_iofuncs_query_input_device_callback
 *        
 *  2    mpeg      1.1         12/7/01 12:00:40 PM    Dave Wilson     SCR(s) 
 *        2958 :
 *        Text cursor now moves to the origin of the display window when the 
 *        screen
 *        is cleared.
 *        
 *  1    mpeg      1.0         11/27/01 11:59:16 AM   Dave Wilson     
 * $
 * 
 *    Rev 1.25   02 Sep 2003 18:25:04   kroescjl
 * SCR(s) 7415 :
 * reordered header files to eliminate warnings when building for PSOS
 * 
 *    Rev 1.24   31 Jul 2003 14:54:32   joness
 * SCR(s): 5046 
 * Add functionality for redirecting to different osd regions.
 * 
 *    Rev 1.23   18 Jun 2003 14:22:42   dawilson
 * SCR(s): 6782 
 * Removed 2 redundant parameters (foreground and background indices) from
 * cnxt_iofuncs_set_aa_indices. These are already specified when the colours
 * are set using cnxt_iofuncs_set_foreground/background_color.
 * 
 *    Rev 1.22   08 Apr 2003 09:53:26   dawilson
 * SCR(s) 5974 :
 * Text cursor positioning was incorrect in cases where the default font was
 * of type GFX_FONTTYPE_AAFIXED. This has been corrected.
 * 
 *    Rev 1.21   13 Feb 2003 12:03:22   kortemw
 * SCR(s) 5479 :
 * Removed old header reference
 * 
 *    Rev 1.20   10 Oct 2002 09:30:16   dawilson
 * SCR(s) 4772 :
 * Added cnxt_iofuncs_set_aa_indices function to allow a client to tell IOFUNCS
 * which palette indices to use when drawing antialiased fonts.
 * 
 *    Rev 1.19   25 Sep 2002 21:16:52   vancec
 * SCR(s) 3786 :
 * 
 * Removing old DRM and AUD conditional bitfield code.
 * 
 *    Rev 1.18   03 Sep 2002 18:56:04   kortemw
 * SCR(s) 4498 :
 * Remove warnings
 * 
 *    Rev 1.17   22 Aug 2002 15:09:38   dawilson
 * SCR(s) 4319 :
 * Added cnxt_iofuncs_get_bounding_rect function to allow an application to 
 * query the rectangle that a string will render into in a given font.
 * 
 *    Rev 1.16   02 Aug 2002 12:07:02   dawilson
 * SCR(s) 4321 :
 * Fixed a piece of code which looked at the GFX_FONT version number. It was
 * looking for "==2" when it should have been ">=2" to allow for newer versions
 * of the structure.
 * 
 *    Rev 1.15   02 Aug 2002 11:15:18   dawilson
 * SCR(s) 4314 :
 * Added API cnxt_iofuncs_get_gfx_bitmap to allow client to query the GFX
 * bitmap describing the current IOFUNCS OSD region.
 * 
 *    Rev 1.14   31 Jul 2002 16:45:14   dawilson
 * SCR(s) 3044 :
 * Support added for variable width font handling.
 * 
 *    Rev 1.13   19 Jun 2002 09:39:10   dawilson
 * SCR(s) 3993 :
 * Added 2 new APIs - cnxt_iofuncs_set_mouse_relative which results in relative
 * mouse movement being reported rather than absolute cursor position and
 * cnxt_iofuncs_is_mouse_relative to return the current mouse mode. Also changed
 * the original IOFUNCS_MSG_MOUSE_MOVE, splitting it in 2 to report the mouse
 * movement in the 2 different ways. Now the messages are MOVE_RELATIVE and
 * MOVE_ABSOLUTE.
 * 
 *    Rev 1.12   21 May 2002 12:46:04   vancec
 * SCR(s) 3786 :
 * Removed DRM bitfields.
 * 
 *    Rev 1.11   12 Mar 2002 16:26:26   dawilson
 * SCR(s) 3347 :
 * Fixed code so that keyboard event callbacks are not made while the module
 * is processing a getstr or getch call. This prevents a race condition where
 * a phantom key release message was getting through to the callback just after
 * a string was completed resulting in some annoying behaviour on the uCOS version
 * of DMFRONT.
 * 
 *    Rev 1.10   08 Mar 2002 16:18:50   dawilson
 * SCR(s) 3290 :
 * y
 * Used #defines from hwconfig.h (or ucosconf.h) to control stack size, priority
 * and name for the 2 tasks created by the module.
 * 
 *    Rev 1.9   06 Mar 2002 11:39:38   dawilson
 * SCR(s) 3315 :
 * 
 * 
 * 
 * 
 * 
 * 
 * Oops - forgot to set the correct function return code from 
 * cnxt_iofuncs_press_any_key in the last edit.
 * 
 *    Rev 1.8   06 Mar 2002 11:27:16   dawilson
 * SCR(s) 3315 :
 * Added cnxt_iofuncs_press_any_key, an API providing an easy way to access
 * "Press any key to continue" functionality. The function makes sure that the
 * input queue is flushed before waiting so you don't have to worry about type-
 * ahead.
 * 
 *    Rev 1.7   27 Feb 2002 15:32:42   dawilson
 * SCR(s) 3254 :
 * Added new cnxt_iofuncs_screen_display function to replace existing 2 APIs
 * to show and hide the OSD. Left the old functions in the header as macros to
 * save problems with apps that use the old API names. Also added function to
 * allow default font and OSD mode to be queried cnxt_iofuncs_get_font_and_mode.
 * 
 *    Rev 1.6   08 Jan 2002 16:22:58   dawilson
 * SCR(s) 3005 :
 * Ignore bogus keystrokes for key code 0 from IR driver.
 * Only report a key as having been pressed or a string as having been entered
 * after receiving the key up event for the last key. This prevents the calling
 * app from getting a key up event for the last character typed during calls to
 * cnxt_iofuncs_getstr or cnxt_iofuncs_getch after these calls have returned.
 * Changed the way that character timeouts are reported (use -1 as a marker now
 * rather than 0) and also ensured that "Enter" keycode is returned correctlyy
 * when it is the key pressed during cnxt_iofuncs_getch.
 * 
 *    Rev 1.5   07 Jan 2002 14:36:30   dawilson
 * SCR(s) 3002 :
 * Completed mouse cursor handling code and added new API cnxt_iofuncs_set_mouse_cursor
 * to allow clients to set their own cursor images.
 * 
 *    Rev 1.4   03 Jan 2002 15:57:48   dawilson
 * SCR(s) 2995 :
 * Added new cnxt_iofuncs_shutdown API and code to allow the internal tasks to
 * be terminated.
 * Changed cnxt_iofuncs_init to allow the API to be called again with a different
 * OSD mode and/or default font.
 * 
 *    Rev 1.3   03 Jan 2002 13:44:20   dawilson
 * SCR(s) 2997 :
 * Fixed a problem where stray copies of the text cursor could be left lying
 * around if backspace was used to delete characters during cnxt_iofuncs_getstr.
 * 
 *    Rev 1.2   03 Jan 2002 13:37:54   dawilson
 * SCR(s) 2996 :
 * Added new API cnxt_iofuncs_query_input_device_callback
 * 
 *    Rev 1.1   07 Dec 2001 12:00:40   dawilson
 * SCR(s) 2958 :
 * Text cursor now moves to the origin of the display window when the screen
 * is cleared.
 * 
 *    Rev 1.0   27 Nov 2001 11:59:16   dawilson
 * SCR(s) 2927 :
 * 
 *
 ****************************************************************************/

