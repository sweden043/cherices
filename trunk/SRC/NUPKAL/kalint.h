/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*              Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002      */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       kalint.h
 *
 *
 * Description:    Private header file for the OpenTV Kernel Adaptation
 *                 layer for Nucleus+
 *
 *
 * Author:         Dave Wilson
 *
 ****************************************************************************/
/* $Header: kalint.h, 5, 5/22/03 5:35:00 PM, Dave Wilson$
 ****************************************************************************/

#ifndef _KALINT_H_
#define _KALINT_H_

/*********************/
/*********************/
/** Resource limits **/
/*********************/
/*********************/
#define NUP_MAX_NAME_LENGTH            8

#define RSS_EV_WAIT                    0x01

/* Timeout used in internal semaphore requests when WAIT_FOREVER may not */
/* be the most helpful option.                                           */
#define GENERAL_TIMEOUT                10000

/**********************/
/**********************/
/** Type definitions **/
/**********************/
/**********************/
#define NU_DETAILED_DEBUG
#undef KAL_DO_PRIORITY_INHERITANCE

#define KAL_TIME_SLICE                 10
#define KAL_TICKS2SEC                  500 /* the tick rate is 2000 us for 
                                                hwlib call */

#define KAL_POOL_MIN_ALLOC_SIZE        128
#define KAL_OS_POOL_ALLOC_OVERHEAD     16
#define KAL_OS_PART_MEM_BLK_OVERHEAD   8

/**************************/
/* KAL Process Descriptor */
/**************************/

/* NB: No more than 99 tasks may be defined at once due to naming */
/* convention applied to sleep semaphores.                        */

typedef struct _proc_descriptor
{
   unsigned long  psos_pid;
   unsigned long  proc_flags;
   void           *private_mem;
   PFNTASKEX      entry_point;
   unsigned char  priority;          //  MFB  IKM  Added 8/25/2000
#ifdef KAL_DO_PRIORITY_INHERITANCE
   unsigned char  priority_original;
   unsigned char  priority_inherited;
   unsigned char  inversion_protection_interest;
#endif /* KAL_DO_PRIORITY_INHERITANCE */
   u_int32        dwArgs[4];         // Keep track of arguments passed to task
   u_int32        *pstack_check;
   u_int32        *pstack_ptr;
   u_int32        *pstack_mem_ptr;
   u_int32        stack_size;
   sem_id_t       sleep_sem;
   NU_EVENT_GROUP *pEvent;
   events_t       free_events;
   char           name[NUP_MAX_NAME_LENGTH];
} proc_descriptor;

/* Process state flags */
#define PROC_FLAG_SLEEPING             0x00000001
#define PROC_FLAG_WAKEUP_MASK          0x00000002
#define PROC_FLAG_TIMER_ACTIVE         0x00000004
#define PROC_FLAG_SUSPENDED            0x00000008
#define PROC_FLAG_STRUCT_IN_USE        0x80000000
#define PROC_FLAG_STACK_CHECK          0x40000000
#define PROC_FLAG_DELETE_TASK          0x00008000

#define PROC_MAGIC_NUMBER              0x39240000

/* Distance from bottom of stack to place check pointer. Note that this */
/* number must be at least the number of bytes on the stack when a new  */
/* process starts or data aborts could be caused!                       */
#define STACK_CHECK_GUARD              128
#define STACK_CHECK_MARKER             0xBAD0DEED
#ifdef CHECK_STACK_USAGE
#define STACK_CHECK_DELAY              10000
#define STACK_CHECK_DELAY_TICKS        5000
#define CHECK_STACK_USAGE_CHAR         0x99
#else
#define STACK_CHECK_DELAY              200
#define STACK_CHECK_DELAY_TICKS        100
#endif

/******************************/
/* KAL Memory Pool Descriptor */
/******************************/
typedef struct _mem_pool_desc
{
   u_int32  flags;
   u_int32  rtos_id;
   void     *pool_start;
   u_int32  pool_size;
   char     name[NUP_MAX_NAME_LENGTH];
} mem_pool_desc;

#define POOL_FLAG_STRUCT_IN_USE        0x80000000

#endif /* _KALINT_H_ */

/****************************************************************************
 * Modifications:
 * $Log: 
 *  5    mpeg      1.4         5/22/03 5:35:00 PM     Dave Wilson     SCR(s) 
 *        6564 6565 6566 :
 *        Removed definitions of CACHE_LINE_SIZE and NONCACHEABLE_RAM_MASK. 
 *        These are
 *        now in the chip header files.
 *        
 *  4    mpeg      1.3         2/15/02 4:26:42 PM     Senthil Veluswamy SCR(s) 
 *        3013 :
 *        Private header file for the new Stardard KAL (not using the pSOS 
 *        Transkit). This a drop from nupkalex\kalintex.h (the part supporting 
 *        the standard KAL interfaces)
 *        
 *  3    mpeg      1.2         2/28/01 4:52:26 PM     Tim White       DCS#1337:
 *         Added check stack size tool.
 *        
 *  2    mpeg      1.1         11/2/00 1:48:12 PM     Angela          changed 
 *        priority type from char to unsigned char
 *        
 *  1    mpeg      1.0         8/25/00 4:30:22 PM     Ismail Mustafa  
 * $
 * 
 *    Rev 1.4   22 May 2003 16:35:00   dawilson
 * SCR(s) 6564 6565 6566 :
 * Removed definitions of CACHE_LINE_SIZE and NONCACHEABLE_RAM_MASK. These are
 * now in the chip header files.
 * 
 *    Rev 1.3   15 Feb 2002 16:26:42   velusws
 * SCR(s) 3013 :
 * Private header file for the new Stardard KAL (not using the pSOS Transkit). This a drop from nupkalex\kalintex.h (the part supporting the standard KAL interfaces)
 *
 ****************************************************************************/

