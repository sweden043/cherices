/****************************************************************************/
/*                            Conexant Systems                              */
/****************************************************************************/
/*                                                                          */
/* Filename:       rtc.h                                                    */
/*                                                                          */
/* Description:    Real Time Clock Driver                                   */
/*                                                                          */
/* Author:         Tim White                                                */
/*                                                                          */
/* Copyright Conexant Systems Inc, 2001.  All Rights Reserved.              */
/*                                                                          */
/****************************************************************************/

#ifndef _RTC_H_
#define _RTC_H_

/*
 * Time structure
 */


typedef struct _time_t
{
    u_int32 sec;
    u_int32 msec;
} rtc_time_t, *prtc_time_t;

/*
 * Function declarations
 */

bool rtc_init (void);
bool rtc_set  (prtc_time_t time);
void rtc_get  (prtc_time_t time);

/*
 * Interrupt handler
 */

int rtc_isr_handler(u_int32, bool, PFNISR*);

#endif // _RTC_H_ 

