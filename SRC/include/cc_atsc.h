#ifndef __CC_ATSC
#define __CC_ATSC

/****************************************************************************/
/*                   Conexant Proprietary and Confidential                  */
/*                       Software File/Module Header                        */
/*                     Conexant Systems Inc. (c) 2002                       */
/*                               Austin, TX                                 */
/*                            All Rights Eeserved                           */
/****************************************************************************/
/*
 * Filename:       CC_ATSC.H
 *
 * Description:    This file specifies the interface for a driver for closed
 *                 captioning in accordance with ATSC Document A/53, ATSC
 *                 DIGITAL TELEVISION STANDARD AND AMENDMENT NO.1, 16 Sep 95,
 *                 Amendment No. 1 to A/53 16 Mar 00.
 *
 * Author:         Billy Jackman
 *
 ****************************************************************************/
/* $Header: cc_atsc.h, 1, 6/7/02 12:21:54 PM, Billy Jackman$
 ****************************************************************************/

/* Typedef used for status returns from interface functions. */
typedef enum
{
    CC_SUCCESS,
    CC_ALREADY_INITIALIZED,
    CC_NOT_INITIALIZED,
    CC_ERROR
} CC_STATUS;

/* Typedef for callbacks to be registered via cnxt_cc_atsc_register_callback. */
typedef void (*cnxt_cc_atsc_callback_t)( u_int32 type, u_int32 one, u_int32 two );

/* Modes for callback/insertion operation.  Or values together if needed.
   The insertion mode sends captioning data out through the video output.
   The extraction mode sends data to a registered callback function.
   If both modes are true, the data is sent both ways.
   It is the default mode of operation to do insertion, so if extraction is
       not needed there is no need to register a callback.
   It is an error to specify insertion alone as the mode in a callback
       registration.
*/
#define CC_INSERTION (1)
#define CC_EXTRACTION (2)

/* The API functions. */
CC_STATUS cnxt_cc_atsc_init( void );
CC_STATUS cnxt_cc_atsc_start( void );
CC_STATUS cnxt_cc_atsc_stop( void );
CC_STATUS cnxt_cc_atsc_register_callback( cnxt_cc_atsc_callback_t funcptr, u_int32 mode );
CC_STATUS cnxt_cc_atsc_unregister_callback( void );

/****************************************************************************
 * Modifications:
 *
 * $Log: 
 *  1    mpeg      1.0         6/7/02 12:21:54 PM     Billy Jackman   
 * $
 * 
 *    Rev 1.0   07 Jun 2002 11:21:54   jackmaw
 * SCR(s) 3932 :
 * The ATSC closed captioning header file.
 ****************************************************************************/

#endif /* __CC_ATSC */
