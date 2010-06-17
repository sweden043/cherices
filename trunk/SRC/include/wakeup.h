/****************************************************************************/
/*                          Conexant Systems Inc.                           */
/****************************************************************************/
/*                                                                          */
/* Filename:       WAKEUP.H                                                 */
/*                                                                          */
/* Description:    Generic Power Control Driver Header File                 */
/*                                                                          */
/* Author:         Dave Wilson                                              */
/*                                                                          */
/* Copyright Conexant Systems Inc, 2000                                     */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/

#ifndef _WAKEUP_H_
#define _WAKEUP_H_

/********************/
/* Type Definitions */
/********************/

/* Hardware sources that will cause coma mode exit */
#define WAKEUP_SOURCE_RTC     RST_WAKEUP_RTC   
#define WAKEUP_SOURCE_GPIO    RST_WAKEUP_GPIO  
#define WAKEUP_SOURCE_IR      RST_WAKEUP_IR    
#define WAKEUP_SOURCE_UART1   RST_WAKEUP_UART1
#define WAKEUP_SOURCE_UART2   RST_WAKEUP_UART2
#define WAKEUP_SOURCE_PULSE   RST_WAKEUP_PULSE 

#define WAKEUP_SOURCE_ALL (WAKEUP_SOURCE_RTC   |  \
                           WAKEUP_SOURCE_GPIO  |  \
                           WAKEUP_SOURCE_IR    |  \
                           WAKEUP_SOURCE_UART1 |  \
                           WAKEUP_SOURCE_UART2 |  \
                           WAKEUP_SOURCE_PULSE)

#define WAKEUP_SOURCE_UNKNOWN 0xFF

/**********************/
/* Exported Functions */
/**********************/
bool    wakeup_init(void);
u_int32 wakeup_set_resume_interval(u_int32 uSeconds);
u_int32 wakeup_set_mask(u_int32 uWakeupSources);
u_int32 wakeup_enter_coma_mode(bool bReset);
bool    wakeup_enter_standby_mode(void);
bool    wakeup_exit_standby_mode(void);
u_int32 wakeup_reason(void);

#endif
