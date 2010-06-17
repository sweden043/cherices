/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                   Conexant Systems Inc. (c) 2003 - 2004                  */
/*                            Shanghai, CHINA                               */
/*                          All Rights Reserved                             */
/****************************************************************************/
/*
 * Filename:      DEBUGINF.H
 *
 * Description:   Header File for the debug information output functions.
 *
 * Author:        Steven Shen
 *
 ****************************************************************************/
/* $Header: DEBUGINF.H, 2, 3/18/04 11:58:27 PM, Steven Shen$
 * $Id: DEBUGINF.H,v 1.1, 2004-03-19 05:58:27Z, Steven Shen$
 ****************************************************************************/

#ifndef __DEBUG_INF_H__
#define __DEBUG_INF_H__

#include <stdarg.h>
#include "basetype.h"

#ifdef __cplusplus
extern "C"
{
#endif


/****************************************************************************/
/* DEFINES: */

/*--------------------------------------------------------------------------*/
/*            DEFINITIONS OF DEBUG INFORMATION OUTPUT OPTION
 */

#define DEBUG_OUT_NONE        (0)
#define DEBUG_OUT_TRACEPORT   (1)
#define DEBUG_OUT_CMDSHELL    (2)


#ifdef DEBUG
 /* output the debug information */
 
 #ifdef DRIVER_INCL_CMDSHELL
  /* through the command shell */
  
  #define DEBUG_OUT_OPTION    DEBUG_OUT_CMDSHELL
  
  #ifdef  DEBUG_GLOBALS
  #define DEBUG_EXT
  #else
  #define DEBUG_EXT           extern
  #endif
  
  DEBUG_EXT void debug_out (u_int32 flags, char *string, ...);
  
  #define TL_ALWAYS           (0x70800000)
  #define TL_ERROR            (0x70800000)
  #define TL_INFO             (0x40800000)
  #define TL_FUNC             (0x20800000)
  #define TL_VERBOSE          (0x10800000)
  
  extern int cmdvprintf (const char* format, va_list arg);
 
 #else /* only for DRIVER_INCL_TRACE */
  /* through the trace port */
  
  #define DEBUG_OUT_OPTION    DEBUG_OUT_TRACEPORT
  #define debug_out           trace_new
  
  #define TL_ALWAYS           (TRACE_LEVEL_ALWAYS | TRACE_DMD)
  #define TL_ERROR            (TRACE_LEVEL_ALWAYS | TRACE_DMD)
  #define TL_INFO             (TRACE_LEVEL_4 | TRACE_DMD)
  #define TL_FUNC             (TRACE_LEVEL_2 | TRACE_DMD)
  #define TL_VERBOSE          (TRACE_LEVEL_1 | TRACE_DMD)
  
 #endif
 
#else
 /* no debug information output */
 
 #define DEBUG_OUT_OPTION     DEBUG_OUT_NONE
 /* in non-debug builds, trace calls are compiled to nothing */
 #define debug_out            if (0) ((int (*)(u_int32,char *, ...)) 0)
 
 /* these definitions are only needed for passing the release compile. */
 #define TL_ALWAYS            (0)
 #define TL_ERROR             (0)
 #define TL_INFO              (0)
 #define TL_FUNC              (0)
 #define TL_VERBOSE           (0)
 
#endif /* end of (ifdef DEBUG) */


/****************************************************************************/
/* FUNCTIONS: */


/****************************************************************************/
/* THE END */

#ifdef __cplusplus
}
#endif

#endif   /* __DRV_8722_H__ */

/****************************************************************************
 * Modifications:
 * $Log: 
 *  2    mpeg      1.1         3/18/04 11:58:27 PM    Steven Shen     CR(s) 
 *        8596 : To fix the build error during the RELEASE building, just add 
 *        some definitions into the DEBUGINF.H file. These definitions are only
 *         used for the Thomson cable front-end driver.
 *  1    mpeg      1.0         3/15/04 10:30:25 AM    Matt Korte      CR(s) 
 *        8566 : Initial version of Thomson Cable Tuner/Demod
 * $
 *
 ****************************************************************************/
