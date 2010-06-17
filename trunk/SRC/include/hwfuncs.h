/****************************************************************************/
/*                 Rockwell Semiconductor Systems - SABINE                  */
/****************************************************************************/
/*                                                                          */
/* Filename:           HWFUNCS.H                                            */
/*                                                                          */
/* Description:        Low level functions prototypes used by the KAL.      */
/*                                                                          */
/* Author:             Dave Wilson                                          */
/*                                                                          */
/* Copyright Rockwell Semiconductor Systems, 1998                           */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/

#ifndef _HWFUNCS_H_
#define _HWFUNCS_H_

#if EMULATION_LEVEL == 1 || EMULATION_LEVEL == 2
#define PID_TIMER1_CLEAR_REG 0x0A80000C
#define PID_TIMER2_CLEAR_REG 0x0A80002C
#define PID_TIMER1_LOAD_REG  0x0A800000
#define PID_TIMER2_LOAD_REG  0x0A800020
#define PID_TIMER1_CTRL_REG  0x0A800008
#define PID_TIMER2_CTRL_REG  0x0A800028
#define PID_INT_STATUS_REG   0x0A000000

typedef struct _PID_TIMER_CTRL
{
  unsigned int Reserved1:2,
               Prescale:2,
               Reserved2:2,
               Mode:1,
               Enable:1,
               Reserved3:24;
} PID_TIMER_CTRL, *LPPID_TIMER_CTRL;

#define MODE_PERIODIC 1
#define MODE_RUN_ON   0

#define PRESCALE_1    0
#define PRESCALE_16   1
#define PRESCALE_256  2

#endif

void    clear_pic_interrupt(u_int32 uPic, u_int32 uInt);
bool    is_raw_int_active(u_int32 uPic, u_int32 uInt);
bool    is_int_enabled(u_int32 uPic, u_int32 uInt);
void    enable_hw_int(u_int32 uPic, u_int32 uInt);
void    disable_hw_int(u_int32 uPic, u_int32 uInt);
void    disable_all_hw_ints(void);

void    clear_timer_int(u_int32 uTimer);
u_int32 set_hw_timer(u_int32 uTimer, u_int32 uPerioduS);
u_int32 set_hw_timer_native(u_int32 uTimer, u_int32 uLimit);
void    start_hw_timer(u_int32 uTimer);
void    start_hw_timer_nowrap(u_int32 uTimer);
void    stop_hw_timer(u_int32 uTimer);
void    restart_hw_timer(u_int32 uTimer);
u_int32 read_hw_timer(u_int32 uTimer);
u_int32 read_hw_timer_timeout(u_int32 uTimer);
bool    is_timer_running(u_int32 uTimer);
bool    is_timer_pending(u_int32 uTimer);
void    make_timer_watchdog(u_int32 uTimer, bool bEnable);
u_int32 get_active_gpio_int(void);
u_int32 get_active_timer_int(void);
u_int32 find_highest_bit(u_int32 uValue);
u_int32 find_lowest_bit(u_int32 uValue);

/* GPIO Pin Access Functions */
void set_gpio_output_level(unsigned int uGpioNum, bool bHigh);
void set_gpio_output_off(unsigned int uGpioNum);
void set_gpio_output_low(unsigned int uGpioNum);
void set_gpio_output_high(unsigned int uGpioNum);
int get_gpio_input(unsigned int  GPIO_bit);

#endif
