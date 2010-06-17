/****************************************************************************/
/*                          Conexant Systems Inc                            */
/****************************************************************************/
/*                                                                          */
/* Filename:           HWLIBINT.H                                           */
/*                                                                          */
/* Description:        Private header file for the RTOS-independent h/w     */
/*                     function library                                     */
/*                                                                          */
/* Author:             Dave Wilson                                          */
/*                                                                          */
/* Copyright Conexant Systems Inc, 1999                                     */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/

#ifndef _HWLIBINT_H_
#define _HWLIBINT_H_

/*****************/
/* Include files */
/*****************/
#include "kal.h"
#include "hwlibprv.h"

/*********************/
/*********************/
/** Resource limits **/
/*********************/
/*********************/

#define NUM_GPIO_INTS    (NUM_GPI_BANKS * 32)
#define NUM_TIMER_INTS   NUM_TIM_BANKS

/**********************/
/**********************/
/** Type definitions **/
/**********************/
/**********************/

/**********************************************************************/
/* Union allowing access to psos names using long integers instead of */
/* character arrays. This structure is used with other RTOSs too!     */
/**********************************************************************/

/******************************/
/* KAL interrupt vector table */
/******************************/
typedef struct _interrupt_vector
{
  PFNISR pfnVector;
  u_int32  dwCount;
  u_int32  dwFlags;
} interrupt_vector;

#define INT_FLAG_HOOKED  0x01
#define INT_FLAG_CHAINED 0x02
#define INT_FLAG_ERROR   0x04
#define INT_FLAG_ACTIVE  0x08
#define INT_PSOS_HANDLER 0x80

/********************/
/* Timer structures */
/********************/
typedef struct _timer_descriptor
{
   unsigned long dwFlags;
   unsigned long dwPeriod;
   unsigned long dwIntMask;
   void         *pUserData;
   PFNTIMERC     pfnCallback;
   PFNISR        pfnChain;
   char          name[MAX_OBJ_NAME_LENGTH];
} timer_descriptor;

#define NUM_TIMERS        8

#define TIMER_MAGIC_NUM 0x0BBC0000
#define TIMERID_FROM_INDEX(x) (TIMER_MAGIC_NUM | (x))
#define INDEX_FROM_TIMERID(x) ((x) & 0xFFFF)

#define IS_VALID_TIMERID(x) (((((x) & TIMER_MAGIC_NUM) == TIMER_MAGIC_NUM) && \
                              (INDEX_FROM_TIMERID(x) < NUM_TIMERS)) ? 1 : 0)

typedef struct _systimer_client
{
  PFNTIMERC   pfnHandler;
  void       *pArg;
} systimer_client;

/* System timer rates for different emulation phases (in kHz) */

#if EMULATION_LEVEL == PHASE3
#define SYSCLKFREQ 54000
#define TIMER_ALGORITHM_MAX 79500
#endif

#if EMULATION_LEVEL == FINAL_HARDWARE
#define SYSCLKFREQ 54000
#define TIMER_ALGORITHM_MAX 79500
#endif

/**************************/
/* Tick counter structure */
/**************************/
typedef struct _tick_descriptor
{
   unsigned long dwFlags;
   int           iPeriodMs;
   int           iMsToNextTick;
   void         *pUserData;
   PFNTICKC      pfnCallback;
   char          name[MAX_OBJ_NAME_LENGTH];
} tick_descriptor;

/*
#define NUM_TICKS      16
#define TICK_MAGIC_NUM 0xABCD0000
#define TICKID_FROM_INDEX(x) (TICK_MAGIC_NUM | (x))
#define INDEX_FROM_TICKID(x) ((x) & 0xFFFF)

#define IS_VALID_TICKID(x) (((((x) & TICK_MAGIC_NUM) == TICK_MAGIC_NUM) && \
                              (INDEX_FROM_TICKID(x) < NUM_TICKS)) ? 1 : 0)
*/

/***********************/
/***********************/
/** Tick Timer Labels **/
/***********************/
/***********************/

/* Which hardware timer to use */
#define TICK_TIMER      2

/* Number of timer ticks that equal 1ms */
#define TICKS_PER_MS    SYSCLKFREQ

/* The longest period that a tick timer can time - 30s */
#define MAX_TICK_PERIOD 30000

/* The shortest period that will be set - 0.5ms */
#define MIN_TICK_PERIOD (SYSCLKFREQ/2)

#endif /* _HWLIBINT_H_ */
