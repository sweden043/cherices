/****************************************************************************/
/*                           Conexant Systems                               */
/****************************************************************************/
/*                                                                          */
/* Filename:           KALPRV.H                                             */
/*                                                                          */
/* Description:        Private header file for the Conexant Kernel          */
/*                     Adaptation Layer                                     */
/*                                                                          */
/* Author:             Tim Ross                                             */
/*                                                                          */
/* Copyright Conexant Systems, 2000                                         */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************
 * $Header: kalprv.h, 6, 3/4/03 4:03:24 PM, $
 ****************************************************************************/

#ifndef _KALPRV_H_
#define _KALPRV_H_

/*******************/
/*******************/
/** Useful macros **/
/*******************/
/*******************/

#define PROC_MAGIC_NUMBER 0x39240000
#define IS_VALID_PID(x) ((((x & 0xFFFF0000) == PROC_MAGIC_NUMBER) && \
                          ((x & 0x0000FFFF) < MAX_PROCESSES)) \
                         ? TRUE : FALSE)
#define INDEX_FROM_PID(x) (x & 0x0000FFFF)
#define PID_FROM_INDEX(x) (x | PROC_MAGIC_NUMBER)

#define POOL_MAGIC_NUMBER 0x07120000
#define IS_VALID_MPID(x) ((((x & 0xFFFF0000) == POOL_MAGIC_NUMBER) && \
                           ((x & 0x0000FFFF) < MAX_MEMORY_POOLS)) \
                          ? TRUE : FALSE)
#define INDEX_FROM_MPID(x) (x & 0x0000FFFF)
#define MPID_FROM_INDEX(x) (x | POOL_MAGIC_NUMBER)

#define SEM_MAGIC_NUMBER 0x11070000
#define IS_VALID_SID(x) ((((x & 0xFFFF0000) == SEM_MAGIC_NUMBER) && \
                          ((x & 0x0000FFFF) < MAX_SEMAPHORES)) \
                         ? TRUE : FALSE)
#define INDEX_FROM_SID(x) (x & 0x0000FFFF)
#define SID_FROM_INDEX(x) (x | SEM_MAGIC_NUMBER)

#define MUT_MAGIC_NUMBER 0x06040000
#define IS_VALID_MID(x) ((((x & 0xFFFF0000) == MUT_MAGIC_NUMBER) && \
                          ((x & 0x0000FFFF) < MAX_MUTEXES)) \
                         ? TRUE : FALSE)
#define INDEX_FROM_MID(x) (x & 0x0000FFFF)
#define MID_FROM_INDEX(x) (x | MUT_MAGIC_NUMBER)

#define QU_MAGIC_NUMBER 0x02040000
#define IS_VALID_QID(x) ((((x & 0xFFFF0000) == QU_MAGIC_NUMBER) && \
                          ((x & 0x0000FFFF) < MAX_QUEUES)) \
                         ? TRUE : FALSE)
#define INDEX_FROM_QID(x) (x & 0x0000FFFF)
#define QID_FROM_INDEX(x) (x | QU_MAGIC_NUMBER)

#endif /* _KALPRV_H_ */

/****************************************************************************
 * $Log: 
 *  6    mpeg      1.5         3/4/03 4:03:24 PM      Senthil Veluswamy SCR(s) 
 *        5340 :
 *        Changed bit mask in IS_VALID macro to get the correct resource index
 *        
 *  5    mpeg      1.4         3/4/03 2:57:48 PM      Senthil Veluswamy SCR(s) 
 *        5340 :
 *        Modified the IS_VALID_?ID macros to also check if the id used exceeds
 *         the number of available resource allocations. 
 *        Added Header & Log.
 *        
 *  4    mpeg      1.3         5/1/00 7:19:54 PM      Tim Ross        Added 
 *        magic numbers for memory pools, queues, mutexes, semaphores,
 *        
 *  3    mpeg      1.2         4/17/00 11:35:18 AM    Tim Ross        Rework 
 *        for KAL extensions.
 *        
 *  2    mpeg      1.1         4/6/00 5:44:44 PM      Tim Ross        Changed 
 *        MAX_NAME_LENGTH to MAX_OBJ_NAME_LENGTH.
 *        
 *  1    mpeg      1.0         4/6/00 12:01:08 PM     Tim Ross        
 * $
 * 
 *    Rev 1.5   04 Mar 2003 16:03:24   velusws
 * SCR(s) 5340 :
 * Changed bit mask in IS_VALID macro to get the correct resource index
 * 
 *    Rev 1.4   04 Mar 2003 14:57:48   velusws
 * SCR(s) 5340 :
 * Modified the IS_VALID_?ID macros to also check if the id used exceeds the number of available resource allocations. 
 * Added Header & Log.
 ****************************************************************************/

