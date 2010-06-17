/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*           Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002         */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       otvkal.h
 *
 *
 * Description:    Public header file for OpenTV Kernel Adaptation layer driver
 *
 *
 * Author:         Dave Wilson
 *
 ****************************************************************************/
/* $Header: otvkal.h, 30, 9/17/03 4:03:30 PM, Dave Wilson$
 ****************************************************************************/
#ifndef _OTVKAL_H_
#define _OTVKAL_H_

/**************************************/
/**************************************/
/** OpenTV-specific type definitions **/
/**************************************/
/**************************************/

#include <opentvx.h>

/*********************************************************************/
/* Memory block descriptor used in o-code memory protection routines */
/*********************************************************************/
typedef struct
{
   void         *pBlock;  
   u_int32       uLength;
} memory_block;

typedef void (*pfn_opentv_task)(void *);

/****************************************************/
/* Size of the fonts linked to the OpenTV 1.2 image */
/****************************************************/
#ifdef OPENTV_12
#define SIZE_BLTNFONT 126616
#endif

#if (RTOS==NUP)
/*WS: we no longer need to set aside some memory for process stacks and 
OS objects, the memory is already reserved during kal initialization*/       
#define STACK_MEM_REQUIRED      0
#else
#define STACK_MEM_REQUIRED      (DEFAULT_STACK_SIZE * (gMaxProcesses+2))
#endif

/* PSOS statically saves memory for it's OS objects.  Therefore, we need not
   save any memory for PSOS */
#define RESOURCE_MEM_REQUIRED      0

/****************************************/
/****************************************/
/** OpenTV defined function prototypes **/
/****************************************/
/****************************************/

/* Functions which map closely to our KAL entry points. We define these as  */
/* macros so that our own drivers using the OpenTV KAL won't be slowed down */
/* any. OpenTV actually calls the named entry points, though, so thin code  */
/* veneers will also be required to allow correct operation of the system.  */

#ifndef OTVKAL_COMPILE

#define process_create( entry_point, arg, stack, stack_size, prio) \
              (opentv_pid_t)task_create((pfn_opentv_task)(entry_point), \
                                        (arg),                          \
                                        (stack),                        \
                                        (stack_size),                   \
                                        (prio),                         \
                                        NULL)                          
        
#define process_self (opentv_pid_t)task_id
#define process_exit task_terminate
#define process_sleep() task_time_sleep(KAL_WAIT_FOREVER)
#define process_time_sleep(ms) task_time_sleep((ms))
#define process_wakeup(pid) task_wakeup((task_id_t)(pid))
#define process_mask(on_off) task_mask((bool)(on_off))
#define process_priority (delta) (char)task_priority(task_id(), (delta))
#define critical_start (kernel_state)critical_section_begin
#define critical_stop(previous_state) critical_section_end(previous_state)
#define semaphore_create(intial_value) (semaphore_id_t)sem_create((intial_value), NULL)
#define semaphore_delete(sem_id) sem_delete((sem_id_t)(sem_id))
#define semaphore_wait(sem_id) sem_get((sem_id_t)(sem_id), KAL_WAIT_FOREVER)
#define semaphore_nowait(sem_id) sem_get((sem_id_t)(sem_id), 0)
#define semaphore_signal(sem_id) sem_put((sem_id_t)(sem_id))

#define queue_create(x) qu_create((x), NULL)
#define queue_destroy(x) qu_destroy((x))
#define queue_send(x, y) qu_send((x), (y))
#define queue_receive(x, y, z) qu_receive((x), ((y) ? KAL_WAIT_FOREVER : 0),  (z))
#define timer_allocate(x) hwtimer_create(pfnTimerCompatCallback, (void *)(x), NULL)
#define timer_free(x) hwtimer_destroy(x)

#if (ALTERNATE_ABORT_REPORT_HANDLING != YES) 
#define abort_report(x) fatal_exit(x)
#endif

#endif

/* OpenTV-specific functions which are implemented in OTVKAL code */

bool         otvkal_initialise(void);
void        *direct_segment_create( size_t size);
size_t       direct_segment_size( void );
int          private_memory_attach (void *chunk);
void        *private_memory_access( opentv_pid_t process_id);
far_size_t   far_size_inquire( void );
int          far_segments_create( void );
int          far_segments_add( far_size_t minimum_size );
void         far_segments_delete( void );
int          far_segments_number( void );
voidF        far_segment_address( int segment );
size_t       far_segment_size( int segment );
bool         is_memprot( voidF address );
void         memprot( voidF address );
void         should_stay_in_same_segment( voidF fp1, size_t size );
unsigned int kernel_tick_period( void );
void         kernel_tick_attach( irq_fct_t tick);

#endif /* _OTVKAL_H_ */

/****************************************************************************
 * Modifications:
 * $Log: 
 *  30   mpeg      1.29        9/17/03 4:03:30 PM     Dave Wilson     SCR(s) 
 *        7271 :
 *        Increased the size of the built in fonts used by OpenTV. In the 
 *        1.24F1
 *        release, this has moved from 113K to 126K (or thereabouts).
 *        
 *  29   mpeg      1.28        4/2/03 9:43:28 AM      Dave Wilson     SCR(s) 
 *        5937 5938 :
 *        Macro redefining abort_report to fatal_exit is now compiled out if
 *        ALTERNATE_ABORT_REPORT_HANDLING is set to YES.
 *        
 *  28   mpeg      1.27        1/16/03 11:39:42 AM    Dave Wilson     SCR(s) 
 *        5146 :
 *        Removed some redundant definitions and changed others. OpenTV direct 
 *        segment
 *        size is now specified via the software config file.
 *        
 *  27   mpeg      1.26        1/14/03 3:13:44 PM     Dave Wilson     SCR(s) 
 *        5210 :
 *        Removed some definitions relating to far heap handling. Number and 
 *        size of
 *        far heap segments is now controlled by settings in the software 
 *        config file.
 *        
 *  26   mpeg      1.25        11/1/02 10:22:20 AM    Dave Wilson     SCR(s) 
 *        4871 :
 *        Removed definition of OSD_SCRATCHPAD_SIZE. This is now in OTVOSD.H.
 *        
 *  25   mpeg      1.24        5/6/02 6:17:34 PM      Angela Swartz   SCR(s) 
 *        3200 :
 *        define STACK_MEM_REQUIRED and RESOURCE_MEM_REQUIRED as 0 for Nup 
 *        since the memory is already reserved during kal initialization
 *        
 *  24   mpeg      1.23        10/10/01 11:40:22 AM   Senthil Veluswamy SCR(s) 
 *        2731 :
 *        Removed Stack & Resource Memory Reserve for NUPKALEX. Memory is 
 *        already reserved in the KAL.
 *        
 *  23   mpeg      1.22        3/16/01 2:37:40 PM     Joe Kroesche    #1446 - 
 *        reduced amount of memory set aside for stacks
 *        
 *  22   mpeg      1.21        2/8/01 11:06:56 AM     Miles Bintz     Tracker 
 *        #1045.  Allocating more memory for OS objects
 *        
 *  21   mpeg      1.20        2/6/01 2:46:00 PM      Miles Bintz     added 
 *        define for RESOURCE_MEM_REQUIRED
 *        
 *  20   mpeg      1.19        11/30/00 2:39:24 AM    Dave Wilson     Added XSI
 *         cache size definition
 *        
 *  19   mpeg      1.18        11/6/00 3:31:54 PM     Dave Wilson     Increased
 *         OSD scratchpad size for 1.2 builds
 *        
 *  18   mpeg      1.17        10/19/00 3:17:28 PM    Anzhi Chen      
 *        task_priority() is defined as "unsigned char" now.  Modified for 
 *        this.
 *        
 *  17   mpeg      1.16        10/7/00 3:20:56 PM     Dave Wilson     Freed up 
 *        a bunch of memory for other use (250K) by being a lot less
 *        conservative in STACK_MEM_REQUIRED.
 *        
 *  16   mpeg      1.15        10/4/00 8:02:30 PM     Dave Wilson     Increased
 *         far segment size to 1.5MB as required by BSkyB
 *        
 *  15   mpeg      1.14        10/4/00 7:40:50 PM     Dave Wilson     Added 
 *        256K to the far segment size to allow mod_deferred VTS test to 
 *        complete
 *        
 *  14   mpeg      1.13        9/28/00 10:44:16 AM    Dave Wilson     Reduced 
 *        the far segment size for 1.2 pending info on what it should be
 *        from BSkyB
 *        
 *  13   mpeg      1.12        8/9/00 10:49:04 AM     Dave Wilson     Changed 
 *        far segment size to 1.5MB for BSkyB case
 *        
 *  12   mpeg      1.11        8/7/00 7:13:18 PM      Dave Wilson     Updated 
 *        size of build in fonts for 1.23BU
 *        
 *  11   mpeg      1.10        1/18/00 10:08:54 AM    Dave Wilson     Added 
 *        OpenTV 1.2 specific functions
 *        
 *  10   mpeg      1.9         10/28/99 12:55:04 PM   Dave Wilson     Changed 
 *        all timer_ APIs to hwtimer_
 *        
 *  9    mpeg      1.8         10/27/99 5:05:18 PM    Dave Wilson     Changed 
 *        WAIT_FOREVER to KAL_WAIT_FOREVER
 *        
 *  8    mpeg      1.7         6/11/99 5:39:38 PM     Dave Wilson     Fixed the
 *         fimer_free macro.
 *        
 *  7    mpeg      1.6         5/20/99 5:58:04 PM     Chris Chapman   Changed 
 *        FAR_SEGMENT_MIN_SIZE from 1024k to 768k
 *        
 *  6    mpeg      1.5         5/17/99 3:40:40 PM     Ismail Mustafa  Changed 
 *        DIRECT_SEGMENT_GRANULARITY to 8K instead of 128K.
 *        
 *  5    mpeg      1.4         5/12/99 10:33:12 AM    Dave Wilson     Added new
 *         timer_allocate definition for portability after
 *        changing to timer_create API.
 *        
 *  4    mpeg      1.3         5/5/99 3:23:56 PM      Dave Wilson     Changed 
 *        timer API to accept a user data pointer.
 *        
 *  3    mpeg      1.2         5/4/99 4:07:58 PM      Dave Wilson     Updated 
 *        some macros mapping from OpenTV to KAL APIs.
 *        
 *  2    mpeg      1.1         4/29/99 9:55:56 AM     Dave Wilson     New 
 *        OpenTV KAL header file. KAL is now 2 components, one
 *        of which is OpenTV-independent.
 *        
 *  1    mpeg      1.0         4/27/99 5:12:58 PM     Dave Wilson     
 * $
 * 
 *    Rev 1.29   17 Sep 2003 15:03:30   dawilson
 * SCR(s) 7271 :
 * Increased the size of the built in fonts used by OpenTV. In the 1.24F1
 * release, this has moved from 113K to 126K (or thereabouts).
 * 
 *    Rev 1.28   02 Apr 2003 09:43:28   dawilson
 * SCR(s) 5937 5938 :
 * Macro redefining abort_report to fatal_exit is now compiled out if
 * ALTERNATE_ABORT_REPORT_HANDLING is set to YES.
 * 
 *    Rev 1.27   16 Jan 2003 11:39:42   dawilson
 * SCR(s) 5146 :
 * Removed some redundant definitions and changed others. OpenTV direct segment
 * size is now specified via the software config file.
 * 
 *    Rev 1.26   14 Jan 2003 15:13:44   dawilson
 * SCR(s) 5210 :
 * Removed some definitions relating to far heap handling. Number and size of
 * far heap segments is now controlled by settings in the software config file.
 * 
 *    Rev 1.25   01 Nov 2002 10:22:20   dawilson
 * SCR(s) 4871 :
 * Removed definition of OSD_SCRATCHPAD_SIZE. This is now in OTVOSD.H.
 * 
 *    Rev 1.24   06 May 2002 17:17:34   swartzwg
 * SCR(s) 3200 :
 * define STACK_MEM_REQUIRED and RESOURCE_MEM_REQUIRED as 0 for Nup since the memory is already reserved during kal initialization
 * 
 *    Rev 1.23   10 Oct 2001 10:40:22   velusws
 * SCR(s) 2731 :
 * Removed Stack & Resource Memory Reserve for NUPKALEX. Memory is already reserved in the KAL.
 * 
 *    Rev 1.22   16 Mar 2001 14:37:40   kroescjl
 * #1446 - reduced amount of memory set aside for stacks
 * 
 *    Rev 1.21   08 Feb 2001 11:06:56   bintzmf
 * Tracker #1045.  Allocating more memory for OS objects
 * 
 *    Rev 1.20   06 Feb 2001 14:46:00   bintzmf
 * added define for RESOURCE_MEM_REQUIRED
 * 
 *    Rev 1.19   30 Nov 2000 02:39:24   dawilson
 * Added XSI cache size definition
 * 
 *    Rev 1.18   06 Nov 2000 15:31:54   dawilson
 * Increased OSD scratchpad size for 1.2 builds
 * 
 *    Rev 1.17   19 Oct 2000 14:17:28   achen
 * task_priority() is defined as "unsigned char" now.  Modified for this.
 * 
 *    Rev 1.16   07 Oct 2000 14:20:56   dawilson
 * Freed up a bunch of memory for other use (250K) by being a lot less
 * conservative in STACK_MEM_REQUIRED.
 * 
 *    Rev 1.15   04 Oct 2000 19:02:30   dawilson
 * Increased far segment size to 1.5MB as required by BSkyB
 * 
 *    Rev 1.14   04 Oct 2000 18:40:50   dawilson
 * Added 256K to the far segment size to allow mod_deferred VTS test to complete
 * 
 *    Rev 1.13   28 Sep 2000 09:44:16   dawilson
 * Reduced the far segment size for 1.2 pending info on what it should be
 * from BSkyB
 * 
 *    Rev 1.12   09 Aug 2000 09:49:04   dawilson
 * Changed far segment size to 1.5MB for BSkyB case
 * 
 *    Rev 1.11   07 Aug 2000 18:13:18   dawilson
 * Updated size of build in fonts for 1.23BU
 * 
 *    Rev 1.10   18 Jan 2000 10:08:54   dawilson
 * Added OpenTV 1.2 specific functions
 * 
 *    Rev 1.9   28 Oct 1999 11:55:04   dawilson
 * Changed all timer_ APIs to hwtimer_
 * 
 *    Rev 1.8   27 Oct 1999 16:05:18   dawilson
 * Changed WAIT_FOREVER to KAL_WAIT_FOREVER
 * 
 *    Rev 1.7   11 Jun 1999 16:39:38   dawilson
 * Fixed the fimer_free macro.
 * 
 *    Rev 1.6   20 May 1999 16:58:04   chapmacj
 * Changed FAR_SEGMENT_MIN_SIZE from 1024k to 768k
 * 
 *    Rev 1.5   17 May 1999 14:40:40   mustafa
 * Changed DIRECT_SEGMENT_GRANULARITY to 8K instead of 128K.
 * 
 *    Rev 1.4   12 May 1999 09:33:12   dawilson
 * Added new timer_allocate definition for portability after
 * changing to timer_create API.
 * 
 *    Rev 1.3   05 May 1999 14:23:56   dawilson
 * Changed timer API to accept a user data pointer.
 * 
 *    Rev 1.2   04 May 1999 15:07:58   dawilson
 * Updated some macros mapping from OpenTV to KAL APIs.
 * 
 *    Rev 1.1   29 Apr 1999 08:55:56   dawilson
 * New OpenTV KAL header file. KAL is now 2 components, one
 * of which is OpenTV-independent.
 ****************************************************************************/

