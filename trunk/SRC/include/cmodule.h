/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                 Conexant Systems Inc. (c) 1998, 1999, 2000               */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       cmodule.h
 *
 *
 * Description:    CMODULE compliant driver for Conexant STB Modem 
 *                 
 *
 * Author:         Angela Swartz
 *
 ****************************************************************************/
/* $Header: cmodule.h, 3, 2/13/03 11:26:04 AM, Matt Korte$
 ****************************************************************************/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "retcodes.h"
#include "opentvx.h"
#include "kal.h"
#include "iic.h"
#include "hwlib.h"

/* Used by cstack_call() to run callbacks from ISR */
typedef struct {
    /* "To use cstack_call(), a C structure must be allocated    */
    /* which contains as its first field a pointer to a function */
    /* of type cstack_fct_t."                                    */
    cstack_fct_t dummy;
    int type;
    int value;
    cmodule_t *me;
} cmod_msg_t; 

void cnxt_modem_line_status( void );
cmodule_t* cnxt_modem_cmod_init( void );

/****************************************************************************
 * Modifications:
 * $Log: 
 *  3    mpeg      1.2         2/13/03 11:26:04 AM    Matt Korte      SCR(s) 
 *        5479 :
 *        Removed old header reference
 *        
 *  2    mpeg      1.1         2/14/01 3:09:14 PM     Angela          removed 
 *        included file init.h
 *        
 *  1    mpeg      1.0         2/1/01 2:49:16 PM      Angela          
 * $
 * 
 *    Rev 1.2   13 Feb 2003 11:26:04   kortemw
 * SCR(s) 5479 :
 * Removed old header reference
 * 
 *    Rev 1.1   14 Feb 2001 15:09:14   angela
 * removed included file init.h
 * 
 *    Rev 1.0   01 Feb 2001 14:49:16   angela
 * Initial revision.
 ****************************************************************************/

