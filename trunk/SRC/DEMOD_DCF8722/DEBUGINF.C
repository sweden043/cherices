/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                   Conexant Systems Inc. (c) 2003 - 2004                  */
/*                            Shanghai, CHINA                               */
/*                          All Rights Reserved                             */
/****************************************************************************/
/*
 * Filename:      DEBUGINF.C
 *
 * Description:   The file contains functions to control the debug inf output
 *
 * Author:        Steven Shen
 *
 ****************************************************************************/
/* $Header: DEBUGINF.C, 1, 3/15/04 10:30:34 AM, Matt Korte$
 * $Id: DEBUGINF.C,v 1.0, 2004-03-15 16:30:34Z, Matt Korte$
 ****************************************************************************/

/***************************/
/*       Header Files      */
/***************************/
#include <stdio.h>
#include <string.h>

#define DEBUG_GLOBALS
#include "DEBUGINF.h"


#ifdef DRIVER_INCL_CMDSHELL

void debug_out (u_int32 flags, char *string, ...)
{
   va_list     arg;
   
   va_start(arg, string); 
   cmdvprintf(string, arg);
   va_end(arg);
   return;
}

#endif /* DRIVER_INCL_CMDSHELL */

/****************************************************************************
 * Modifications:
 * $Log: 
 *  1    mpeg      1.0         3/15/04 10:30:34 AM    Matt Korte      CR(s) 
 *        8566 : Initial version of Thomson Cable Tuner/Demod
 * $
 *
 ****************************************************************************/
