#ifndef __DEMOD_MODULE_API
#define __DEMOD_MODULE_API

/****************************************************************************/ 
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                     Conexant Systems Inc. (c) 2002                       */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename: demod_module_api.h
 *
 * Description: This file contains the specification of the module-level
 *              interface for the demod driver.
 *
 * Author: Billy Jackman
 *
 ****************************************************************************/
/* $Header: demod_module_api.h, 4, 7/8/02 1:44:38 PM, Billy Jackman$
 ****************************************************************************/

#include "demod_types.h"

typedef void (MODULE_STATUS_FUNCTION)( u_int32 module, u_int32 unit, DEMOD_CALLBACK_TYPE type, void *callback_data );

typedef DEMOD_STATUS (UNIT_TYPE_FUNC)( u_int32 unit, DEMOD_NIM_TYPE *unit_type );
typedef DEMOD_STATUS (IOCTL_FUNC)(u_int32 unit, DEMOD_IOCTL_TYPE type, void *data);
typedef DEMOD_STATUS (CONNECT_FUNC)( u_int32 unit, TUNING_SPEC *tuning, u_int32 *connection_limit );
typedef DEMOD_STATUS (DISCONNECT_FUNC)( u_int32 unit );
typedef DEMOD_STATUS (SCAN_FUNC)( u_int32 unit, SCAN_SPEC *scanning );
typedef DEMOD_STATUS (GET_TUNING_FUNC)( u_int32 unit, TUNING_SPEC *tuning );
typedef DEMOD_STATUS (SET_CALLBACK_FUNC)( u_int32 unit, MODULE_STATUS_FUNCTION *callback );
typedef DEMOD_STATUS (CLEAR_CALLBACK_FUNC)( u_int32 unit );
typedef DEMOD_STATUS (GET_SIGNAL_FUNC)( u_int32 unit, SIGNAL_STATS *signal_stats );
typedef DEMOD_STATUS (GET_LOCK_FUNC)( u_int32 unit, bool *locked );
typedef bool         (SCAN_NEXT_FUNC)( u_int32 unit, SCAN_SPEC *scan );
typedef bool         (RE_ACQUIRE_FUNC)(TUNING_SPEC *original, TUNING_SPEC *actual);

typedef struct {
    UNIT_TYPE_FUNC        *unit_type;
    IOCTL_FUNC            *ioctl;
    CONNECT_FUNC          *connect;
    DISCONNECT_FUNC       *disconnect;
    SCAN_FUNC             *scan;
    GET_TUNING_FUNC       *get_tuning;
    SET_CALLBACK_FUNC     *set_callback;
    CLEAR_CALLBACK_FUNC   *clear_callback;
    GET_SIGNAL_FUNC       *get_signal;
    GET_LOCK_FUNC         *get_lock;
    SCAN_NEXT_FUNC        *scan_next;
    RE_ACQUIRE_FUNC       *re_acquire;
} DEMOD_FTABLE;

typedef DEMOD_STATUS (INIT_FUNC)( u_int32 module, u_int32 *number_units, DEMOD_FTABLE *function_table );

/****************************************************************************
 * Modifications:
 *
 * $Log: 
 *  4    mpeg      1.3         7/8/02 1:44:38 PM      Billy Jackman   SCR(s) 
 *        4148 :
 *        Change usage of 'cnxt_bool' in demod interface to 'bool'.
 *        
 *  3    mpeg      1.2         6/18/02 12:00:00 PM    Steven Jones    SCR(s): 
 *        3960 
 *        Update Demod API.
 *        
 *  2    mpeg      1.1         4/3/02 9:46:06 AM      Billy Jackman   SCR(s) 
 *        3445 :
 *        Modified the prototype for cnxt_demod_get_unit_type so that the 
 *        return
 *        parameter is the interface type, instead of a single member 
 *        structure.
 *        
 *  1    mpeg      1.0         3/25/02 4:35:46 PM     Billy Jackman   
 * $
 * 
 *    Rev 1.3   08 Jul 2002 12:44:38   jackmaw
 * SCR(s) 4148 :
 * Change usage of 'cnxt_bool' in demod interface to 'bool'.
 * 
 *    Rev 1.2   18 Jun 2002 11:00:00   joness
 * SCR(s): 3960 
 * Update Demod API.
 * 
 *    Rev 1.2   20 May 2002 08:48:44   joness
 * SCR(s): 3680 
 * new version, archive only.
 * 
 *    Rev 1.1   03 Apr 2002 09:46:06   jackmaw
 * SCR(s) 3445 :
 * Modified the prototype for cnxt_demod_get_unit_type so that the return
 * parameter is the interface type, instead of a single member structure.
 * 
 *    Rev 1.0   25 Mar 2002 16:35:46   jackmaw
 * SCR(s) 3445 :
 * The module level API header for multi-instance demod.
 *
 ****************************************************************************/

#endif /* __DEMOD_MODULE_API */
