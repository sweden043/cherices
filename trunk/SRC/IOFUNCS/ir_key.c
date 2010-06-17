/****************************************************************************/
/*                   Conexant Proprietary and Confidential                  */
/*                       Software File/Module Header                        */
/*              Conexant Systems Inc. (c) 1998, 1999, 2000, 2001            */
/*                               Austin, TX                                 */
/*                            All Rights Eeserved                           */
/****************************************************************************/
/*
 * Filename:       ir_key.c
 *
 *
 * Description:    Low Level IR and Front Panel Handlers for IOFUNCS module
 *
 *
 * Author:         Dave Wilson
 *
 ****************************************************************************/
/* $Header:ir_key.c, 12, 2004-6-25 15:11:37, Xiao Guang Yan$
 ****************************************************************************/

/*****************/
/* Include files */
/*****************/
#include "stbcfg.h"
#include "retcodes.h"
#include "basetype.h"
#include "kal.h"
#include "osdlibc.h"
#include "gfxtypes.h"
#include "gfxlib.h"
#include "gfxutls.h"
#include "gxagfx.h"
#include "confmgr.h"
#include "buttons.h"
#include "iofuncs.h"
#include "iof_int.h"
#include "genir.h"
#include "Hwlibprv.h"  //add by mxd according to ljp

/*************************************************************/
/* IR Support. Assume IRSEJ and remove if another present    */
/* This is required because DRIVER_INCL_IRSEJ is not defined */
/* for a non-internal build since we do not build the driver */
/* but just include the library.                             */
/*************************************************************/
#undef DRIVER_INCL_IRSEJ
#define DRIVER_INCL_IRSEJ
#ifdef DRIVER_INCL_IRRC5NEW
#include "rc5decod.h"
#undef DRIVER_INCL_IRSEJ
#endif
#ifdef DRIVER_INCL_IRBSBNEW
#include "irbsbnew.h"
#undef DRIVER_INCL_IRSEJ
#endif
#ifdef DRIVER_INCL_IRDTV
#include "irdtv.h"
#undef DRIVER_INCL_IRSEJ
#endif
#ifdef DRIVER_INCL_IR38K
//#include "ir38kdecode.h"
#include "rc38kdecode.h"
#undef DRIVER_INCL_IRSEJ
#endif
#ifdef DRIVER_INCL_IRSEJ
#include "irsej.h"
#endif


/******************************************/
/* Local definitions and global variables */
/******************************************/
u_int32 uKeyPressSec[5]={0,0,0,0,0}; //add by mxd
u_int32 ukeyRegister[5]={BTN_CODE_INFO,BTN_CODE_INFO,BTN_CODE_INFO,
                                                  BTN_CODE_INFO,BTN_CODE_INFO};

#define MOUSE_BUTTON_1 0x01
#define MOUSE_BUTTON_2 0x02

#ifdef DRIVER_INCL_IRSEJ
/* The keycode used to toggle between trackball and button modes on the new remote */
#define MOUSE_MODE_SWITCH_CODE CNXT_PC

bool bMimicTrackball = FALSE;
extern bool bNewRemote;
extern bool bNewKeyboard;
#endif 

button_ftable     front_panel_func_table;

/***********************/
/* External references */
/***********************/
extern u_int32 rtc_get_current_time(void);//add by mxd
extern u_int32 uKeyTimer;
//extern u_int32 uFTJudge;
extern void IR_Init(IRRX_PORTID portid);
extern void IR_Decode(void * pinst_decode, PIRRX_DATAINFO pdatainfo, PIRRX_KEYINFO pkeyinfo);

/********************************/
/* Internal function prototypes */
/********************************/
static void KB_Driver(void * pinst_notify, PIRRX_KEYINFO pkeyinfo);
static void XY_Driver(void * pinst_notify, PIRRX_KEYINFO pkeyinfo);
static void IR_Notify(void * pinst_notify, PIRRX_KEYINFO pkeyinfo);

#ifdef DRIVER_INCL_SCANBTNS
static void front_panel_button_callback(u_int16 uKey, bool bPressed);
#endif

/********************************************************************/
/*  FUNCTION:    irdevice_init                                      */
/*                                                                  */
/*  PARAMETERS:  None                                               */
/*                                                                  */
/*  DESCRIPTION: Initialise the IR remote and front panel button    */
/*               drivers.                                           */
/*                                                                  */
/*  RETURNS:     TRUE on success, else FALSE.                       */
/*                                                                  */
/*  CONTEXT:     Must be called from a non-interrupt context.       */
/*                                                                  */
/********************************************************************/
bool input_devices_init(void)
{
   bool retcode = TRUE;
   IRRX_STATUS status;
   static bool bDevicesInit = FALSE;
   void *pInstInfo;                   

   if(!bDevicesInit)
   {
     /* initialize the infra red remote control hardware */
     status = cnxt_irrx_init(IRRX_TYPE_FRONT, IR_Init);
     if (status != IRRX_ERROR)
     {
        trace_new(TR_INFO, "IOFUNCS: IR hardware initialized\n");
        /* initialize the IR decoding instance structure */
        status = cnxt_irrx_swinit(PORT_FRONT, IR_SW_Init, &pInstInfo);
        if (status != IRRX_ERROR)
        {
           status = cnxt_irrx_register(PORT_FRONT, IR_Decode, pInstInfo, IR_Notify, NULL);
           if (status != IRRX_ERROR)
           {
              trace_new(TR_INFO, "IR decode and notify functions registered\n");
              /* Read the keyboard and remote types we are using */

              // To be completed
              #ifdef DRIVER_INCL_SCANBTNS
               /* Initialise the front panel button driver */
               retcode = button_init(front_panel_button_callback, &front_panel_func_table);
              #endif

           }
           else 
           {
              retcode = FALSE;
           }
        } else {
           retcode = FALSE;
        } /* endif */

     }
     else
     {
        retcode = FALSE;
     }
   }

   bDevicesInit = retcode;

   return retcode;
}


/********************************************************************/
/*  FUNCTION:    IR_Notify                                          */
/*                                                                  */
/*  PARAMETERS:  pinst_notify - instance data poiner as passed to   */
/*                              cnxt_irrx_register.                 */
/*               pkeyinfo     - information on event being notified */
/*                                                                  */
/*  DESCRIPTION: This function is called by the low level IR driver */
/*               whenever any key event is decoded for either the   */
/*               keyboard or cursor devices.                        */
/*                                                                  */
/*  RETURNS:     Nothing                                            */
/*                                                                  */
/*  CONTEXT:     May be called from interrupt or non-interrupt      */
/*               contexts.                                          */
/*                                                                  */
/********************************************************************/
static void IR_Notify(void * pinst_notify, PIRRX_KEYINFO pkeyinfo)
{
   if ((pkeyinfo -> device_type) & IRRX_TRACKBALL)
      XY_Driver(pinst_notify, pkeyinfo);
   else if ( (pkeyinfo -> device_type & IRRX_REMOTEBUTTON) || (pkeyinfo -> device_type & IRRX_KEYBOARD) )
      KB_Driver(pinst_notify, pkeyinfo);
}


/********************************************************************/
/*  FUNCTION:    KB_Driver                                          */
/*                                                                  */
/*  PARAMETERS:  pinst_notify - instance data poiner as passed to   */
/*                              cnxt_irrx_register.                 */
/*               pkeyinfo     - information on event being notified */
/*                                                                  */
/*  DESCRIPTION: This function is called to process all key down &  */
/*               key up events from the IR device. Events are       */
/*               posted to the keyboard message queue.              */
/*                                                                  */
/*  RETURNS:     Nothing.                                           */
/*                                                                  */
/*  CONTEXT:     This function may be called from interrupt or non- */
/*               interrupt contexts.                                */
/*                                                                  */
/********************************************************************/
static void KB_Driver(void * pinst_notify, PIRRX_KEYINFO pkeyinfo)
{
IOFUNCS_MESSAGE message_type;
#ifdef DRIVER_INCL_IRSEJ
   if (bNewRemote && (pkeyinfo->key_state == IRRX_KEYDOWN) && (pkeyinfo->key_code == MOUSE_MODE_SWITCH_CODE))
   {
     bMimicTrackball = !bMimicTrackball;
     isr_trace_new(TR_INFO, "Trackball remapping is %s\n", (u_int32)(bMimicTrackball ? "OFF" : "ON"), 0);
   }
#endif
   /* This is a slight hack. The IR driver has a bug where it fairly frequently */
   /* send spurious messages for keycode 0. We ignore all these.                */
   if (pkeyinfo->key_code != 0)
   {
    /* map vol down to <-, vol up to ->, chan up to up, chan down to down on remote */
    if ( pkeyinfo->key_code == 0xc2 )
    {
       //pkeyinfo->key_code = 0x87;
    }
    if ( pkeyinfo->key_code == 0xc1 )
    {
       //pkeyinfo->key_code = 0x8e;
    }
    if ( pkeyinfo->key_code == 0xc5 )
    {
       pkeyinfo->key_code = 0x8b;
    }
    if ( pkeyinfo->key_code == 0xc4 )
    {
       pkeyinfo->key_code = 0x8a;
    }
    if( pkeyinfo->key_state == IRRX_KEYDOWN )
    {
        message_type = IOFUNCS_MSG_KEY_PRESSED;
        isr_trace_new(TR_FUNC, "IOFUNCS: Key message, code 0x%x %s\n", 
                        (u_int32)pkeyinfo->key_code, (u_int32)"DOWN" );
    }
    else if( pkeyinfo->key_state == IRRX_KEYHOLD )
    {
       message_type = IOFUNCS_MSG_KEY_HOLD;
       isr_trace_new(TR_FUNC, "IOFUNCS: Key message, code 0x%x %s\n", 
                        (u_int32)pkeyinfo->key_code, (u_int32)"HOLD" );
    }
    else
    {
       message_type = IOFUNCS_MSG_KEY_RELEASED;
       isr_trace_new(TR_FUNC, "IOFUNCS: Key message, code 0x%x %s\n", 
                        (u_int32)pkeyinfo->key_code, (u_int32)"UP" );
    }
    internal_post_command_message( message_type, pkeyinfo->key_code, 0, 0);
   }

//   isr_trace_new(TR_FUNC, "IOFUNCS: Key message, code 0x%x %s\n", 
//      (u_int32)pkeyinfo->key_code, (u_int32)((pkeyinfo->key_state == IRRX_KEYDOWN) ? "DOWN" : "UP"));
}

/******************************************************/
/* This function acceptes xy device packets and       */
/* and process them and then pass to OpenTV.          */
/******************************************************/
static void XY_Driver(void * pinst_notify, PIRRX_KEYINFO pkeyinfo)
{
  bool ks;
  u_int32 uBtnState;
  static u_int32 last_buttonmask = 0;
  u_int32 uCopyX, uCopyY;
  static  int iLastX = -1, iLastY = -1;

  /* Update the cursor position inside a critical section since application code can also */
  /* modify the position using an API call.
                              */
  ks = critical_section_begin();

  iMouseCursorX += pkeyinfo->deltax;
  iMouseCursorY -= pkeyinfo->deltay; /* Y delta is inverted */

  /* Clip to the screen */
  iMouseCursorX = max(0, min(gnOsdMaxWidth, iMouseCursorX));
  iMouseCursorY = max(0, min(gnOsdMaxHeight,iMouseCursorY));

  /* Take a copy of the coordinates in case we need to send a mouse message */
  uCopyX = (u_int32)iMouseCursorX;
  uCopyY = (u_int32)iMouseCursorY;

  critical_section_end(ks);

  #ifdef DEBUG
  if (pkeyinfo->deltax)
    isr_trace_new(TR_FUNC, "IOFUNCS: DeltaX = %d, iMouseCursorX = %d\n", pkeyinfo->deltax, iMouseCursorX);
  if(pkeyinfo->deltay)
    isr_trace_new(TR_FUNC, "IOFUNCS: DeltaY = %d, iMouseCursorY = %d\n", -pkeyinfo->deltay, iMouseCursorY);
  #endif

  /* If the button state changed, send appropriate messages to the keyboard queue */
  if(pkeyinfo->buttonmask != last_buttonmask)
  {
    isr_trace_new(TR_FUNC, "IOFUNCS: Button Mask = 0x%2x\n", pkeyinfo->buttonmask, 0);

    /* Handle state changes for button 1 */
    uBtnState = pkeyinfo->buttonmask & MOUSE_BUTTON_1;

    if (uBtnState != (last_buttonmask & MOUSE_BUTTON_1))
    {
      internal_post_command_message((IOFUNCS_MESSAGE)(uBtnState ? IOFUNCS_MSG_MOUSE_BTN1_DOWN : IOFUNCS_MSG_MOUSE_BTN1_UP),
                                     uCopyX, uCopyY, 0);
    }

    /* Handle state changes for button 2 */
    uBtnState = pkeyinfo->buttonmask & MOUSE_BUTTON_2;

    if (uBtnState != (last_buttonmask & MOUSE_BUTTON_2))
    {
      internal_post_command_message((IOFUNCS_MESSAGE)(uBtnState ? IOFUNCS_MSG_MOUSE_BTN2_DOWN : IOFUNCS_MSG_MOUSE_BTN2_UP),
                                  uCopyX, uCopyY, 0);
    }

    last_buttonmask = pkeyinfo->buttonmask;
  }

  /* Send a mouse move message if the mouse moved */
  if(pkeyinfo->deltax || pkeyinfo->deltay)
  {
    internal_post_command_message(IOFUNCS_MSG_MOUSE_MOVE_ABSOLUTE, uCopyX, uCopyY, ((u_int32)pkeyinfo->deltax & 0xFFFF) | ((u_int32)(-pkeyinfo->deltay) << 16));
    iLastX = iMouseCursorX;
    iLastY = iMouseCursorY;
  }
}

#if  defined(DRIVER_INCL_SCANBTNS) || defined(DRIVER_INCL_FPIM)
/********************************************************************/
/*  FUNCTION:    front_panel_button_callback                        */
/*                                                                  */
/*  PARAMETERS:  uKey - Code of key which is changing state         */
/*               bPressed - TRUE if key is pressed, FALSE released. */
/*                                                                  */
/*  DESCRIPTION: This function is called by the low level buttons   */
/*               driver whenever the state of any front panel       */
/*               button changes. This function remaps the button    */
/*               code and passes the appropriate message to the     */
/*               IOFUNCS handler.                                   */
/*                                                                  */
/*  RETURNS:     Nothing                                            */
/*                                                                  */
/*  CONTEXT:     This function may be called from any context.      */
/*                                                                  */
/********************************************************************/
//extern int is_desktop_wnd();

static void front_panel_button_callback(u_int16 uKey, bool bPressed)
{
  u_int32 uKeyCode;
if(bPressed)
  {
 /*add by mxd according to ljp*/
    ukeyRegister[4]=ukeyRegister[3];
   ukeyRegister[3]=ukeyRegister[2];
   ukeyRegister[2]=ukeyRegister[1];
  ukeyRegister[1]=ukeyRegister[0];
  ukeyRegister[0]=uKey;
/*end add*/

/*add by mxd*/
 //uKeyPressSec[4]=uKeyPressSec[3];
  //uKeyPressSec[3]=uKeyPressSec[2];
  //uKeyPressSec[2]=uKeyPressSec[1];
 //uKeyPressSec[1]=uKeyPressSec[0];
 // uKeyPressSec[0]=rtc_get_current_time();;
   }
  /*end add*/
  

  switch (uKey)
  {
    case BTN_CODE_POWER:   uKeyCode = CNXT_POWER     ; break;
    case BTN_CODE_MENU:
	{
		//if(is_desktop_wnd() == 1)
		//{
		//	uKeyCode = CNXT_MENU      ; 
		//}
		//else
			uKeyCode = CNXT_EXIT      ; 
		break;
	}
    case BTN_CODE_INFO:    uKeyCode = CNXT_INFO      ; break;
    case BTN_CODE_LEFT:    uKeyCode = CNXT_LARROW    ; break;
    case BTN_CODE_RIGHT:   uKeyCode = CNXT_RARROW    ; break;
    case BTN_CODE_UP:      uKeyCode = CNXT_UPARROW   ; 
                                                            // uKeyTimer=0;
 
                                                              break;
    case BTN_CODE_DOWN:    uKeyCode = CNXT_DOWNARROW ; break;
    case BTN_CODE_SELECT: 

    /*add by mxd*/
     if (//bPressed
     	//&&(uKeyPressSec[0]-uKeyPressSec[4]<4)
      //    &&uKeyTimer<MaxKeyTimer
          (ukeyRegister[4]==BTN_CODE_UP)&&
     						 	(ukeyRegister[3]==BTN_CODE_DOWN)&&(ukeyRegister[2]==BTN_CODE_LEFT)&&
     						 	(ukeyRegister[1]==BTN_CODE_RIGHT)&&(ukeyRegister[0]==BTN_CODE_SELECT))
     	{
                             //uFTJudge=1;					 
    						 uKeyCode =CNXT_Factory_Test;
 
     	}			

    else

   /*end add*/
    uKeyCode = CNXT_ENTER     ; break;
    case BTN_CODE_BACKUP:  uKeyCode = CNXT_BACKUP    ; break;
    default:
      isr_trace_new(TR_ERR, "IOFUNCS: Undefined keycode from button driver.\n", uKey, 0);
      return;
  }

  /* Post the message to the keystroke handler */
  internal_post_command_message((IOFUNCS_MESSAGE)(bPressed ? IOFUNCS_MSG_KEY_PRESSED : IOFUNCS_MSG_KEY_RELEASED), uKeyCode, 2, 0);
  isr_trace_new(TR_INFO, "IOFUNCS: Front panel button code 0x%x pressed\n", uKeyCode, 0);
}
#endif /* DRIVER_INCL_SCANBTNS  */

/****************************************************************************
 * Modifications:
 * $Log:
 *  12   mpeg      1.11        2004-6-25 15:11:37     Xiao Guang Yan  CR(s)
 *       9583 9584 : Added include file for IR38K driver. 
 *  11   mpeg      1.10        2003-12-5 6:13:53      Tim Ross        CR(s)
 *       8098 8099 8100 : Added support for DIrect TV IR remote control driver.
 *  10   mpeg      1.9         2003-2-14 2:03:58      Matt Korte      SCR(s)
 *       5479 :
 *       Removed old header reference
 *  9    mpeg      1.8         2002-10-29 1:54:24     Steve Glennon   SCR(s):
 *       4847 
 *       #undef'ed DRIVER_INCL_IRSEJ prior to #defining it to avoid build
 *       problem
 *       
 *  8    mpeg      1.7         2002-10-26 4:57:08     Steve Glennon   SCR(s):
 *       4842 
 *       Added support for RC5 and BSkyB remotes in addition to SEJIN. The
 *       non-Sejin remotes will not currently generate trackball messages as
 *       they don't have one. THis means you cannot use the scale/position
 *       video feature in WATCHTV with these other remotes.
 *       
 *       Had to remove the nasty peeking into the global decode instance data
 *       to see the previous mouse button status, as we no longer have
 *       visibility into this. Replaced it with static local data for the
 *       last_buttonmask.
 *       
 *  7    mpeg      1.6         2002-9-4 8:58:12       Matt Korte      SCR(s)
 *       4498 :
 *       Remove warnings
 *  6    mpeg      1.5         2002-7-12 21:18:16     Steven Jones    SCR(s):
 *       4176 
 *       Support Brady.
 *  5    mpeg      1.4         2002-6-19 23:39:14     Dave Wilson     SCR(s)
 *       3993 :
 *       Added 2 new APIs - cnxt_iofuncs_set_mouse_relative which results in
 *       relative
 *       mouse movement being reported rather than absolute cursor position and
 *       cnxt_iofuncs_is_mouse_relative to return the current mouse mode. Also
 *       changed
 *       the original IOFUNCS_MSG_MOUSE_MOVE, splitting it in 2 to report the
 *       mouse
 *       movement in the 2 different ways. Now the messages are MOVE_RELATIVE
 *       and
 *       MOVE_ABSOLUTE.
 *  4    mpeg      1.3         2002-4-23 4:53:40      Larry Wang      SCR(s)
 *       3593 :
 *       Map Vol up/down on Sejin remote to left/right, Channel up/down to
 *       up/down  
 *        before foward the key codes.
 *  3    mpeg      1.2         2002-1-9 6:23:14       Dave Wilson     SCR(s)
 *       3005 :
 *       Ignore bogus keystrokes for key code 0 from IR driver.
 *       Only report a key as having been pressed or a string as having been
 *       entered
 *       after receiving the key up event for the last key. This prevents the
 *       calling
 *       app from getting a key up event for the last character typed during
 *       calls to
 *       cnxt_iofuncs_getstr or cnxt_iofuncs_getch after these calls have
 *       returned.
 *       Changed the way that character timeouts are reported (use -1 as a
 *       marker now
 *       rather than 0) and also ensured that "Enter" keycode is returned
 *       correctlyy
 *       when it is the key pressed during cnxt_iofuncs_getch.
 *  2    mpeg      1.1         2002-1-4 5:58:00       Dave Wilson     SCR(s)
 *       2995 :
 *       Added new cnxt_iofuncs_shutdown API and code to allow the internal
 *       tasks to
 *       be terminated.
 *       Changed cnxt_iofuncs_init to allow the API to be called again with a
 *       different
 *       OSD mode and/or default font.
 *  1    mpeg      1.0         2001-11-28 1:59:44     Dave Wilson     
 * $
 * 
 *    Rev 1.9   13 Feb 2003 12:03:58   kortemw
 * SCR(s) 5479 :
 * Removed old header reference
 * 
 *    Rev 1.8   28 Oct 2002 11:54:24   glennon
 * SCR(s): 4847 
 * #undef'ed DRIVER_INCL_IRSEJ prior to #defining it to avoid build problem
 * 
 * 
 *    Rev 1.7   25 Oct 2002 14:57:08   glennon
 * SCR(s): 4842 
 * Added support for RC5 and BSkyB remotes in addition to SEJIN. The non-Sejin remotes will not currently generate trackball messages as they don't have one. THis means you cannot use the scale/position video feature in WATCHTV with these other remotes.
 * 
 * Had to remove the nasty peeking into the global decode instance data to see the previous mouse button status, as we no longer have visibility into this. Replaced it with static local data for the last_buttonmask.
 * 
 * 
 *    Rev 1.6   03 Sep 2002 18:58:12   kortemw
 * SCR(s) 4498 :
 * Remove warnings
 * 
 *    Rev 1.5   12 Jul 2002 07:18:16   joness
 * SCR(s): 4176 
 * Support Brady.
 *
 *    Rev 1.4   19 Jun 2002 09:39:14   dawilson
 * SCR(s) 3993 :
 * Added 2 new APIs - cnxt_iofuncs_set_mouse_relative which results in relative
 * mouse movement being reported rather than absolute cursor position and
 * cnxt_iofuncs_is_mouse_relative to return the current mouse mode. Also changed
 * the original IOFUNCS_MSG_MOUSE_MOVE, splitting it in 2 to report the mouse
 * movement in the 2 different ways. Now the messages are MOVE_RELATIVE and
 * MOVE_ABSOLUTE.
 *
 *    Rev 1.3   22 Apr 2002 14:53:40   wangl2
 * SCR(s) 3593 :
 * Map Vol up/down on Sejin remote to left/right, Channel up/down to up/down
 *  before foward the key codes.
 *
 *    Rev 1.2   08 Jan 2002 16:23:14   dawilson
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
 *    Rev 1.1   03 Jan 2002 15:58:00   dawilson
 * SCR(s) 2995 :
 * Added new cnxt_iofuncs_shutdown API and code to allow the internal tasks to
 * be terminated.
 * Changed cnxt_iofuncs_init to allow the API to be called again with a different
 * OSD mode and/or default font.
 *
 *    Rev 1.0   27 Nov 2001 11:59:44   dawilson
 * SCR(s) 2927 :
 *
 *
 ****************************************************************************/

