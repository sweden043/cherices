/****************************************************************************/
/*                            Conexant Systems                              */
/****************************************************************************/
/*                                                                          */
/* Filename:       systime.h                                                */
/*                                                                          */
/* Description:    System Time Header file                                  */
/*                                                                          */
/* Author:         Senthil Veluswamy                                        */
/*                                                                          */
/* Copyright Conexant Systems, 1999                                         */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/

// Sytem Time Driver maintains the current time and time zone information of the
// system. From time to time OTV provides time stamps which give Time and Time
// zone  information. The System Time Driver stores and updates this using the
// Real Time Clock to provide information to all other componets of OTV system.

#ifndef _SYSTIME_H_
#define _SYSTIME_H_

#include "rtc.h"

// function declarations
bool systime_init(void);
void otv_system_time_get(time_oF);
void otv_system_time_zone_get(time_zoneF);
date_t otv_system_get_date(void);
int otv_system_time_set(time_oF);
int otv_system_time_zone_set(time_zoneF);
void get_current_time(time_oF);

#endif // _SYSTIME_H_ 
