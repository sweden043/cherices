/****************************************************************************/
/*                           Conexant Systems                               */
/****************************************************************************/
/*                                                                          */
/* Filename:           KALPRV.H                                             */
/*                                                                          */
/* Description:        Private header file for the Conexant Hardware        */
/*                     Library                                              */
/*                                                                          */
/* Author:             Senthil Veluswamy                                    */
/*                                                                          */
/* Copyright Conexant Systems, 2000                                         */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/

#ifndef _HWLIBPRV_H_
#define _HWLIBPRV_H_

/*********************/
/*********************/
/** Resource limits **/
/*********************/
/*********************/
#define NUM_PICS 4
#define NUM_INTS_PER_PIC 32

/*********************/
/* Timer state flags */
/*********************/
#define TIMER_FLAG_RUNNING   0x00000001
#define TIMER_FLAG_ONESHOT   0x00000002
#define TIMER_FLAG_ALLOCATED 0x00000004
#define TIMER_FLAG_SHARED    0x00000008
#define TIMER_FLAG_SET       0x00000010
#define TIMER_FLAG_STARTING  0x00000020
#define TIMER_FLAG_STOPPING  0x00000040
#define TIMER_FLAG_SYSTEM    0x80000000
#define TIMER_FLAG_WATCHDOG  0x40000000

/****************************/
/* Useful Tick timer Macros */
/****************************/
#define NUM_TICKS      24
#define TICK_MAGIC_NUM 0xABCD0000
#define TICKID_FROM_INDEX(x) (TICK_MAGIC_NUM | (x))
#define INDEX_FROM_TICKID(x) ((x) & 0xFFFF)

#define IS_VALID_TICKID(x) (((((x) & TICK_MAGIC_NUM) == TICK_MAGIC_NUM) && \
                              (INDEX_FROM_TICKID(x) < NUM_TICKS)) ? 1 : 0)

#endif // _HWLIBPRV_H_

