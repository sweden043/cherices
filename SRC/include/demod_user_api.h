#ifndef __DEMOD_USER_API
#define __DEMOD_USER_API

/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                     Conexant Systems Inc. (c) 2002                       */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename: demod_user_api.h
 *
 * Description: This file contains the specification of the user-level
 *              interface to the demod driver.
 *
 * Author: Billy Jackman
 *
 ****************************************************************************/
/* $Header: demod_user_api.h, 5, 5/15/03 12:39:48 PM, Dave Wilson$
 ****************************************************************************/

#include "demod_types.h"

typedef void (USER_STATUS_FUNCTION)
   (
      u_int32 unit,
      DEMOD_CALLBACK_TYPE type,
      void *callback_data
   );

DEMOD_STATUS   cnxt_demod_init(u_int32 *number_units);
DEMOD_STATUS   cnxt_demod_get_number_of_units(u_int32 *number_units);
DEMOD_STATUS   cnxt_demod_get_unit_type(u_int32        unit,
                                        DEMOD_NIM_TYPE *unit_type);
DEMOD_STATUS   cnxt_demod_get_unit_num(u_int32 handle, u_int32 *unitnum);
DEMOD_STATUS   cnxt_demod_open(u_int32     unit,
                               bool        controlled,
                               u_int32     *handle);
DEMOD_STATUS   cnxt_demod_close(u_int32 handle);
DEMOD_STATUS   cnxt_demod_ioctl(u_int32           handle,
                                DEMOD_IOCTL_TYPE  type,
                                void              *data);
DEMOD_STATUS   cnxt_demod_connect(u_int32 handle, TUNING_SPEC *tuning);
DEMOD_STATUS   cnxt_demod_disconnect(u_int32 handle);
DEMOD_STATUS   cnxt_demod_scan(u_int32 handle, SCAN_SPEC *scanning);
DEMOD_STATUS   cnxt_demod_get_tuning(u_int32 handle, TUNING_SPEC *tuning);
DEMOD_STATUS   cnxt_demod_set_callback(u_int32              handle,
                                       USER_STATUS_FUNCTION *callback);
DEMOD_STATUS   cnxt_demod_clear_callback(u_int32 handle);
DEMOD_STATUS   cnxt_demod_get_signal_stats(u_int32        handle,
                                           SIGNAL_STATS   *signal_stats);
DEMOD_STATUS   cnxt_demod_get_lock_status(u_int32 handle, bool *locked);
void           cnxt_demod_set_debounce_time(u_int32 time);

/****************************************************************************
 * Modifications:
 *
 * $Log: 
 *  5    mpeg      1.4         5/15/03 12:39:48 PM    Dave Wilson     SCR(s) 
 *        6105 6362 :
 *        Added cnxt_demod_get_unit_num API.
 *        
 *        
 *  4    mpeg      1.3         7/8/02 1:44:56 PM      Billy Jackman   SCR(s) 
 *        4148 :
 *        Change usage of 'cnxt_bool' in demod interface to 'bool'.
 *        
 *  3    mpeg      1.2         6/25/02 11:46:50 AM    Steven Jones    SCR(s): 
 *        4055 
 *        Add debounce time modification functionality.
 *        
 *  2    mpeg      1.1         4/3/02 9:46:18 AM      Billy Jackman   SCR(s) 
 *        3445 :
 *        Modified the prototype for cnxt_demod_get_unit_type so that the 
 *        return
 *        parameter is the interface type, instead of a single member 
 *        structure.
 *        
 *  1    mpeg      1.0         3/25/02 4:36:40 PM     Billy Jackman   
 * $
 * 
 *    Rev 1.4   15 May 2003 11:39:48   dawilson
 * SCR(s) 6105 6362 :
 * Added cnxt_demod_get_unit_num API.
 * 
 * 
 *    Rev 1.3   08 Jul 2002 12:44:56   jackmaw
 * SCR(s) 4148 :
 * Change usage of 'cnxt_bool' in demod interface to 'bool'.
 * 
 *    Rev 1.2   Jun 25 2002 11:46:50   joness
 * SCR(s): 4055 
 * Add debounce time modification functionality.
 * 
 *    Rev 1.1   24 May 2002 10:29:34   joness
 * SCR(s): 3680 
 * Add interface to change debounce time.
 * 
 *    Rev 1.1   03 Apr 2002 09:46:18   jackmaw
 * SCR(s) 3445 :
 * Modified the prototype for cnxt_demod_get_unit_type so that the return
 * parameter is the interface type, instead of a single member structure.
 * 
 *    Rev 1.0   25 Mar 2002 16:36:40   jackmaw
 * SCR(s) 3445 :
 * The user level API header for multi-instance demod.
 *
 ****************************************************************************/

#endif /* __DEMOD_USER_API */

