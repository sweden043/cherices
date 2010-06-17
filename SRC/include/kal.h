/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*           Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002         */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*                                                                          */
/* Filename:       kal.h                                                    */
/*                                                                          */
/*                                                                          */
/* Description:        Public header file for the Conexant Kernel           */
/*                     Adaptation Layer.                                    */
/*                                                                          */
/* Author:         Dave Wilson                                              */
/*                                                                          */
/****************************************************************************/
/* $Header: kal.h, 89, 11/26/03 11:00:22 AM, Dave Wilson$
 ****************************************************************************/
#include "hwconfig.h"
#include "kalprv.h"

#ifndef _KAL_H_
#define _KAL_H_

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/**********************************/
/* Low level function definitions */
/**********************************/
#include "hwlib.h"

/********************/
/* Type definitions */
/********************/
#include <basetype.h>

#include <stddef.h>

typedef int queue_id_t;
typedef int task_id_t;
typedef int sem_id_t;
typedef unsigned long pool_id_t;

typedef int pipe_id_t;


#if CUSTOMER == VENDOR_C
   typedef int event_id_t;
   typedef unsigned long events_t;
#else
   typedef unsigned short events_t;
#endif

typedef enum _task_state_t
{
   KAL_TASK_RUNNING,
   KAL_TASK_READY,
   KAL_TASK_SLEEPING,
   KAL_TASK_BLOCKED,
   KAL_TASK_SUSPENDED
} task_state_t;

typedef enum _task_wait_order_t
{
   KAL_TASK_ORDER_FIFO,
   KAL_TASK_ORDER_PRIORITY
} task_wait_order_t;
typedef enum _qu_msg_type_t
{
   KAL_FIXED_LENGTH,
   KAL_VARIABLE_LENGTH
} qu_msg_type_t;
typedef enum _qu_priority_t
{
   KAL_QMSG_PRI_NORM,
   KAL_QMSG_PRI_HI
} qu_priority_t;

typedef void (*PFNTASK)(void *);
typedef void (*PFNTASKEX)(int, void **);
typedef void (*PFNTICK)(void);

/*********************/
/*********************/
/** Resource limits **/
/*********************/
/*********************/
/* Edit these values when we know how many are required ! */
#ifdef MEM_ZISE_4M

#if MEM_ZISE_4M==YES
#define MAX_PROCESSES 30 /* KC_NTASK */
#else
#define MAX_PROCESSES 60 /* KC_NTASK */
#endif //#if MEM_ZISE_4M==YES

#else
#define MAX_PROCESSES 60 /* KC_NTASK */
#endif //#ifdef MEM_ZISE_4M

/* Maximum number of concurrent OpenTV semaphores */
#ifdef OPENTV
#define MAX_SEMAPHORES 180
#else /* OPENTV */
#ifdef MHP
#define MAX_SEMAPHORES 200
#else /* CTICA */
#ifdef CTICA
#define MAX_SEMAPHORES 150
#else
#ifdef TFCAS
#define MAX_SEMAPHORES 160
#else
#ifdef DVNCA
#define MAX_SEMAPHORES 150 /* KC_NSEMA4 */
#else
#define MAX_SEMAPHORES 150 /* KC_NSEMA4 */
#endif /* DVNCA*/
#endif /* TFCAS */
#endif /* CTICA */
#endif /* MHP */
#endif /* OPENTV */

/* Maximum number of pipes */
#define MAX_PIPES 30

/* Maximum number of queues */
#define MAX_QUEUES 34 /* KC_NQUEUE */

/* Maximum number of events per process */
#define MAX_EVENTS 16

/* Maximum memory pools.  */
#define MAX_MEMORY_POOLS 10

/*****************/
/* Useful Macros */
/*****************/
#ifdef MEM_ZISE_4M

#if MEM_ZISE_4M==YES
#define DEFAULT_STACK_SIZE 3072
#else
#define DEFAULT_STACK_SIZE 5120
#endif //#if MEM_ZISE_4M==YES

#else
#define DEFAULT_STACK_SIZE 5120

#endif //#ifdef MEM_ZISE_4M

#if (RTOS == UCOS) || (RTOS == UCOS2)
/* #define DEFAULT_PRIORITY     Do_not_create_a_task_with_DEFAULT_PRIORITY_in_UCOS */
#define DEFAULT_PRIORITY	33
#else
#define DEFAULT_PRIORITY     64
#endif

#define KAL_WAIT_FOREVER ((u_int32)-1)

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

/**********************************************/
/* Trace & error logging function definitions */
/**********************************************/
#include "trace.h"


/******************************/
/******************************/
/** Initialisation Functions **/
/******************************/
/******************************/

int  kal_initialise(void);
int  kal_early_initialise(void);
int  kal_timer_initialise(void);
void kal_set_timer_tick_func(PFNTICK pfnTick);
unsigned int kal_get_tick_period( void );
void disable_all_hw_ints(void);

/* Application entry point */
void application_entry(void);

/*****************************************/
/*****************************************/
/** Task Creation/Destruction functions **/
/*****************************************/
/*****************************************/

task_id_t    task_create( PFNTASK entry_point,
                       void *arg,
                       void *stack,
                       size_t stack_size,
                       int prio,
                       const char *name);
task_id_t    task_id(void);
void         task_terminate(void );
int          task_destroy(task_id_t pid);
void         task_time_sleep(unsigned int ms);
int          task_time_sleep_ex(unsigned int ms);
void         task_wakeup(task_id_t pid);
void         task_mask(bool on_off);
bool         task_get_mask( void );
unsigned char task_priority(task_id_t pid, signed short delta);
void        *task_set_pointer(task_id_t pid, void *ptr);
void        *task_get_pointer(task_id_t pid);

#if (RTOS == UCOS) || (RTOS == UCOS2)
#define KAL_MAX_TASK_PRIORITY 63
#else
#define KAL_MAX_TASK_PRIORITY 239
#endif

#define KAL_MIN_TASK_PRIORITY 0

#define TASK_PRIO_NON_PREEMPTED 0x4000

/*************************/
/*************************/
/** Semaphore Functions **/
/*************************/
/*************************/

sem_id_t     sem_create(unsigned int initial_value, const char *name);
sem_id_t sem_create_ex(unsigned int initial_value, 
                              const char *name, 
                              task_wait_order_t wait_order);
int          sem_delete(sem_id_t sem_id);
int          sem_get (sem_id_t sem_id, u_int32 timeout_ms);
int          sem_put( sem_id_t sem_id);


//////////////////////////////////////////////////
	// Pipe Fuctions, add by ÀµÔÆÁ¼ 2006.03

pipe_id_t pipe_create(const char *name);
int pipe_destroy(pipe_id_t pipe_id);
int pipe_read(pipe_id_t pipe_id, void *message, u_int32 size, u_int32 *actual_size, u_int32 timeout_ms);
int pipe_write(pipe_id_t pipe_id, void *message, u_int32 size, u_int32 timeout_ms);

///////////////////////////////////////////////////

/********************************/
/********************************/
/** Fall over and die horribly **/
/********************************/
/********************************/
void         fatal_exit( u_int32 cosmic_constant );

/**************/
/**************/
/* Queue APIs */
/**************/
/**************/

queue_id_t   qu_create(unsigned int max_elements, const char *name);
queue_id_t qu_create_ex(unsigned int max_elements, const char *name, 
                        int max_length, qu_msg_type_t msg_type, 
                        task_wait_order_t wait_order);
int          qu_destroy(queue_id_t qu_id);
int          qu_send(queue_id_t qu_id, void *message);
int qu_send_ex(queue_id_t qu_id, void *message, int message_length,
               u_int32 timeout_ms, qu_priority_t priority);
int          qu_receive(queue_id_t qu_id, u_int32 timeout_ms, void *message);
int qu_receive_ex(queue_id_t qu_id, u_int32 timeout_ms, void *message, 
                  int message_length);
int 		 qu_reset(queue_id_t qu_id);
int qu_reset_ex(queue_id_t qu_id);
/************************/
/************************/
/** Timer Service APIs **/
/************************/
/************************/

extern void pfnTimerCompatCallback(timer_id_t, void *);

/**********************************/
/**********************************/
/* Dynamic Memory Allocation APIs */
/**********************************/
/**********************************/

void        *mem_malloc(u_int32 size);
void         mem_free(void *ptr);
void        *mem_nc_malloc(u_int32 size);
void         mem_nc_free(void *ptr);
u_int32      mem_inquire_largest_block(void);
void        *mem_get_os_segment(u_int32 uSize);
#if CUSTOMER == VENDOR_C
void        *mem_get_os_far_segment(u_int32 uSize);
#endif 
void         mem_release_os_segment(void *ptrSegment);

#define BLOCK_SEARCH_GRANULARITY (8 * 1024)
#define BLOCK_SEARCH_MASK        0xFFFFF000

/*******************/
/*******************/
/* Event functions */
/*******************/
/*******************/
events_t event_create(void);
int event_destroy(events_t pEvents);
int event_receive(events_t eEvents, events_t *eReceived, u_int32 timeout_ms, bool bWaitAll);
int event_send(task_id_t tTargetTask, events_t eEvents);

/**********************************/
/**********************************/
/* LAN related defines            */
/**********************************/
/**********************************/

#define NET_CMD_ID1        0xff
#define NET_CMD_ID2        0xfa
#define NET_CMD_DMXREAD    0x10
#define NET_CMD_DMXWRITE   0x11
#define NET_CMD_IRD_RESET  0x12
#define NET_CMD_SET_TRACE  0x13
#define NET_CMD_GET_TRACE  0x14
#define NET_CMD_MEMREAD    0x15
#define NET_CMD_MEMWRITE   0x16
#define NET_CMD_IICREAD    0x17
#define NET_CMD_IICWRITE   0x18
#define NET_CMD_PROFILEDUMP 0x19
#define NET_CMD_TTXPLAY    0x1A
#define NET_CMD_TTXSTOP    0x1B

#define CMD_SERVER_PORT    8600
#define MIN_BUFF_LEN       256
#define MAX_BUFF_LEN       3072
#define TCP_CHECK_DELAY2   200
#define MIN_PAYLOAD_SIZE   244
typedef struct _NetCmdStruc
{
        u_int8 CmdID1;
        u_int8 CmdID2;
        u_int8 CmdCode;
        u_int8 dummy1;
        u_int32 CmdAddress;
        u_int32 BufferSize;                     /* 256 bytes minimum */
        u_int8 Buffer[MIN_PAYLOAD_SIZE];        /* to make total minimum size 256 bytes */
} NetCmdStruc;
#define CMD_STRUC_SIZE      (sizeof(NetCmdStruc) - MIN_PAYLOAD_SIZE)

typedef struct _tagMEMCOMMANDS
{
   u_int32 *lpMem;
   u_int32 dwMem;
} MEMCOMMANDS, *PMEMCOMMANDS;

#endif

/****************************************************************************
 * Modifications:
 * $Log: 
 *  89   mpeg      1.88        11/26/03 11:00:22 AM   Dave Wilson     CR(s): 
 *        8033 Previous version had different function prototypes for sem_put 
 *        when built for Nucleus vs. any other RTOS. This broke any code which 
 *        used the sem_put return code when built with, for example, pSOS. 
 *        Changed sem_put so that it always returns an int.
 *  88   mpeg      1.87        11/14/03 7:43:28 AM    Ganesh Banghar  CR(s): 
 *        7926 added deinitions for UCOS.
 *  87   mpeg      1.86        8/1/03 1:52:20 PM      Billy Jackman   SCR(s) 
 *        7048 :
 *        Increase the number of semaphores from 160 to 180 for OpenTV.
 *        
 *  86   mpeg      1.85        7/14/03 12:39:18 PM    Senthil Veluswamy SCR(s) 
 *        5354 :
 *        Changed initial_value to unsigned for sem_create and num_queues for 
 *        qu_create to disallow negative initial values (as do the RTOSes).
 *        
 *  85   mpeg      1.84        1/16/03 11:39:36 AM    Dave Wilson     SCR(s) 
 *        5146 :
 *        Removed some redundant definitions and changed others. OpenTV direct 
 *        segment
 *        size is now specified via the software config file.
 *        
 *  84   mpeg      1.83        12/18/02 3:41:08 PM    Senthil Veluswamy SCR(s) 
 *        4785 :
 *        Changed the task_priority delta to size short to enable a better 
 *        range for task priority change.
 *        
 *  83   mpeg      1.82        10/24/02 11:31:34 AM   Ian Mitchell    SCR(s): 
 *        4828 
 *        Added new define TASK_PRIO_NON_PREEMPTED for use when creating a task
 *         and
 *        changed the function prototype for sem put if the RTOS is NUP
 *        
 *  82   mpeg      1.81        6/11/02 7:10:26 AM     Ian Mitchell    SCR(s): 
 *        3956 
 *        If the build is for the MHP application the maximum number of 
 *        semephores is up from 100 to 200
 *        
 *  81   mpeg      1.80        5/31/02 4:03:22 PM     Dave Wilson     SCR(s) 
 *        3913 :
 *        Removed double slash comments from the file. These were causing our 
 *        dependency list generator to stop parsing the file early and miss 
 *        trace.h from
 *        the generated lists.
 *        
 *  80   mpeg      1.79        5/8/02 12:32:50 PM     Angela Swartz   SCR(s) 
 *        3736 :
 *        increased MAX_SEMAPHORES to 160(was 150) for OTV build
 *        
 *  79   mpeg      1.78        3/7/02 11:44:18 AM     Ian Mitchell    SCR(s): 
 *        3322 
 *        undo the change
 *        
 *  78   mpeg      1.77        3/7/02 7:47:18 AM      Ian Mitchell    SCR(s): 
 *        3322 
 *        defined DEFAULT_PRIORTY as 
 *        Do_not_create_a_task_with_DEFAULT_PRIORITY_in_UCOS which flags an 
 *        error if used.
 *        
 *  77   mpeg      1.76        2/15/02 5:40:58 PM     Senthil Veluswamy SCR(s) 
 *        3013 :
 *        This is the public Header file containing the Standard KAL 
 *        interfaces. Moved the KAL Extension interfaces to include\kalex.h
 *        
 *  76   mpeg      1.75        2/1/02 9:50:52 AM      Ian Mitchell    SCR(s): 
 *        3101 
 *        Changed the definition of DEFAULT_PRIORITY and KAL_MAX_TASK_PRIORITY 
 *        if the RTOS is UCOS
 *        
 *        
 *  75   mpeg      1.74        10/10/01 11:33:06 AM   Senthil Veluswamy SCR(s) 
 *        2731 :
 *        Increased MAX_SEMAPHORES to 150 for OTV builds
 *        
 *  74   mpeg      1.73        9/19/01 3:32:38 PM     Senthil Veluswamy SCR(s) 
 *        2574 :
 *        All our Kal Timeouts are in MilliSeconds. Had some Function arguments
 *         named timeout_us. corrected this to timeout_ms
 *        
 *  73   mpeg      1.72        4/11/01 1:52:48 PM     Steve Glennon   DCS1675 
 *        (put by DW): Added new task_sleep_ex function required by video
 *        driver.
 *        
 *  72   mpeg      1.71        2/5/01 1:20:56 PM      Angela          took out 
 *        prototype of task_priority for Vendor_C to get 
 *        vendor_c build successfully
 *        
 *  71   mpeg      1.70        2/2/01 4:20:46 PM      Angela          Merged 
 *        Vendor_c changes into the code( mostly #if CUSTOMER==VENDOR_C blocks)
 *        see DCS#1049
 *        
 *  70   mpeg      1.69        10/19/00 3:16:58 PM    Anzhi Chen      Changed 
 *        type of task_priority from "char" to "unsigned char".
 *        
 *  69   mpeg      1.68        9/11/00 7:07:04 PM     Senthil Veluswamy renamed
 *         pool_is_valid to mem_pool_is_valid
 *        
 *  68   mpeg      1.67        9/11/00 6:50:30 PM     Ismail Mustafa  Again 
 *        increased the number of processes. This was for the VERIFIER tasks.
 *        
 *  67   mpeg      1.66        8/25/00 3:46:46 PM     Dave Wilson     Increased
 *         MAX_PROCESSES
 *        
 *  66   mpeg      1.65        8/24/00 6:01:24 PM     Senthil Veluswamy changed
 *         mem_create_pool() to take 3 arguments (pool_id_ptr, size, name)
 *        - return an error code
 *        
 *  65   mpeg      1.64        8/23/00 11:08:12 PM    Senthil Veluswamy added 
 *        new functions to determine validity of various objects (tasks, 
 *        - sems, muts, qus, & heaps)
 *        
 *  64   mpeg      1.63        7/17/00 6:32:42 PM     Tim Ross        Changed 
 *        CN8600_application_entry to application_entry.
 *        
 *  63   mpeg      1.62        6/23/00 5:57:14 PM     Senthil Veluswamy moved 
 *        MAX_OBJ_NAME_LENGTH from here to hwlib.h
 *        
 *  62   mpeg      1.61        6/20/00 3:15:36 PM     Dave Wilson     Added 
 *        #define for largest block search granularity
 *        
 *  61   mpeg      1.60        6/1/00 6:00:32 PM      Senthil Veluswamy Changed
 *         KAL_MAX_TASK_PRIORITY to 239 from 127
 *        
 *  60   mpeg      1.59        5/1/00 7:51:26 PM      Tim Ross        Removed 
 *        gpio reasd/write prototypes.
 *        
 *  59   mpeg      1.58        5/1/00 7:17:40 PM      Tim Ross        Added 
 *        task_scheduling_xxx() functions.
 *        Added task states to enum.
 *        
 *  58   mpeg      1.57        4/17/00 8:23:58 PM     Tim Ross        Replaced 
 *        macros with function calls for task_create(), sem_create(),
 *        mut_create(), qu_create(), qu_send(), qu_receive().
 *        
 *  57   mpeg      1.56        4/17/00 7:30:20 PM     Tim Ross         
 *        
 *  56   mpeg      1.55        11/18/99 8:06:24 PM    Dave Wilson     Reworked 
 *        system timer handling architecture to keep in inside HWLIB.
 *        
 *  55   mpeg      1.54        10/27/99 5:18:36 PM    Dave Wilson     Changed 
 *        WAIT_FOREVER to KAL_WAIT_FOREVER
 *        Added prototype for CN8600_application_entry
 *        
 *  54   mpeg      1.53        10/14/99 3:56:20 PM    Rob Tilton      Added TTX
 *         net commands.
 *        
 *  53   mpeg      1.52        10/6/99 4:58:38 PM     Dave Wilson     Changed 
 *        sem_delete to return a value.
 *        
 *  52   mpeg      1.51        10/6/99 4:43:24 PM     Dave Wilson     Added 
 *        task_destroy prototype.
 *        
 *  51   mpeg      1.50        9/23/99 3:29:08 PM     Dave Wilson     Moved 
 *        timer function prototypes to HWLIB.H
 *        
 *  50   mpeg      1.49        9/16/99 6:06:22 PM     Dave Wilson     Added 
 *        event functions
 *        
 *  49   mpeg      1.48        9/14/99 3:08:10 PM     Dave Wilson     Changes 
 *        due to splitting KAL into PSOSKAL and HWLIB components.
 *        
 *  48   mpeg      1.47        9/13/99 2:28:42 PM     Dave Wilson     Split out
 *         trace functions to new component.
 *        
 *  47   mpeg      1.46        8/16/99 6:06:04 PM     Rob Tilton      Added 
 *        another NET command for getting profile data.
 *        
 *  46   mpeg      1.45        8/11/99 5:38:32 PM     Dave Wilson     Changes 
 *        to completely decouple the KAL from OpenTV.
 *        
 *  45   mpeg      1.44        8/2/99 4:36:08 PM      Senthil Veluswamy 
 *        Replaced definition for TRACE_FLS with TRACE_NV. 
 *        TRACE_FLS will now be derived from TRACE_NV.
 *        
 *  44   mpeg      1.43        8/2/99 11:59:48 AM     Tim Ross        Added 
 *        mem_nc_malloc() and mem_nc_free() prototypes.
 *        
 *  43   mpeg      1.42        6/28/99 2:21:58 PM     Rob Tilton      Added 
 *        MEMCOMMANDS struct for net commands.
 *        
 *  42   mpeg      1.41        6/23/99 2:42:18 PM     Rob Tilton      Added IIC
 *         net commands.
 *        
 *  41   mpeg      1.40        6/21/99 6:37:48 PM     Rob Tilton      Added 
 *        more net commands.
 *        
 *  40   mpeg      1.39        6/15/99 5:03:10 PM     Ismail Mustafa  Added 
 *        set/get trace level command.
 *        
 *  39   mpeg      1.38        6/14/99 11:41:24 AM    Ismail Mustafa  Added 
 *        network command defines.
 *        
 *  38   mpeg      1.37        6/8/99 3:01:54 PM      Tim Ross        Updated 
 *        for non-OS dependent BSP.
 *        
 *  37   mpeg      1.36        5/12/99 10:34:20 AM    Dave Wilson     Changes 
 *        to timer API to keep same create/destroy syntax as
 *        rest of objects.
 *        
 *  36   mpeg      1.35        5/5/99 3:23:32 PM      Dave Wilson     Changed 
 *        timer API to accept a user data pointer.
 *        
 *  35   mpeg      1.34        5/4/99 4:11:06 PM      Dave Wilson     Updated 
 *        for new, split KAL
 *        
 *  34   mpeg      1.33        3/11/99 12:57:48 PM    Steve Glennon   Added 
 *        #include of hwconfig.h - was not always included before use of this
 *        header file, so some #if statements were throwing warnings which were
 *         real.
 *        
 *  33   mpeg      1.32        3/5/99 10:24:56 AM     Dave Wilson     A few 
 *        more changes for Neches interrupt handling.
 *        
 *  32   mpeg      1.31        3/4/99 6:38:46 PM      Dave Wilson     Further 
 *        changes for Neches version.
 *        
 *  31   mpeg      1.30        3/4/99 3:36:12 PM      Dave Wilson     Changes 
 *        for Neches.
 *        
 *  30   mpeg      1.29        1/29/99 5:48:34 PM     Dave Moore      Added 
 *        TRACE_MODEM
 *        
 *  29   mpeg      1.28        1/28/99 4:08:28 PM     Amy Pratt       Added 
 *        definition for TRACE_CR, for use by system clock recovery driver.
 *        
 *  28   mpeg      1.27        1/7/99 5:35:12 PM      Rob Tilton      Trace 
 *        messages are not compiled for non-debug builds.
 *        
 *  27   mpeg      1.26        10/6/98 3:50:10 PM     Dave Wilson     Increased
 *         the default stack size.
 *        
 *  26   mpeg      1.25        9/21/98 3:30:52 PM     Tim Ross        Corrected
 *         the INT_ definitions for the Sabine serial ports.
 *        
 *  25   mpeg      1.24        9/4/98 10:55:46 AM     Dave Wilson     Added 
 *        definitions of INT_INVALID for PID7T builds.
 *        
 *  24   mpeg      1.23        8/14/98 3:16:00 PM     Dave Wilson     Fixed up 
 *        isr_error_log so that it generates the correct filename and 
 *        line number.
 *        
 *  23   mpeg      1.22        7/7/98 10:40:30 AM     Dave Wilson     
 *  22   mpeg      1.21        6/29/98 3:19:18 PM     Dave Wilson     Added 
 *        memory alloc and free functions.
 *        Added new trace functions with level and source flags.
 *        
 *  21   mpeg      1.20        6/26/98 7:50:08 PM     Ismail Mustafa  Fix for 
 *        interrupt defines for netlist 06/25.
 *        
 *  20   mpeg      1.19        5/29/98 10:56:38 AM    Dave Wilson     Added 
 *        DEFAULT_PRIORITY from kalint.h
 *        
 *  19   mpeg      1.18        5/26/98 8:34:10 AM     Dave Wilson     Added OSD
 *         scratchpad size label.
 *        
 *  18   mpeg      1.17        5/6/98 12:43:52 PM     Dave Wilson     Added 
 *        prototype for semaphore_nowait.
 *        
 *  17   mpeg      1.16        5/6/98 5:07:34 AM      Tim Ross        Updated 
 *        PIC/interrupt bit/ID definitions.
 *        
 *  16   mpeg      1.15        4/16/98 4:17:12 PM     Dave Wilson     Shifted 
 *        the watchdog timer to timer 1 (just to be neat).
 *        
 *  15   mpeg      1.14        4/15/98 2:44:00 PM     Dave Wilson     Changed 
 *        ULONG type to u_int32 to save others having to include
 *        pSOS headers.
 *        
 *  14   mpeg      1.13        4/15/98 12:20:50 PM    Dave Wilson     Changes 
 *        for Phase3.0 and new opentvx.h
 *        
 *  13   mpeg      1.12        4/14/98 3:48:52 PM     Ismail Mustafa  Changes 
 *        to allow the use of the real opentvx.h. Basically removed some
 *        typedefs.
 *        
 *  12   mpeg      1.11        3/27/98 4:34:48 PM     Dave Wilson     Library 
 *        test - removed a line of whitespace.
 *        
 *  11   mpeg      1.10        3/25/98 2:39:26 PM     Dave Wilson     Removed 
 *        use of PID7T label in favour of EMULATION_LEVEL.
 *        
 *  10   mpeg      1.9         3/25/98 10:24:42 AM    Dave Wilson     Added 
 *        default stack size value.
 *        
 *  9    mpeg      1.8         3/24/98 2:23:32 PM     Dave Wilson     Added 
 *        prototype for kal_int_initialise function.
 *        
 *  8    mpeg      1.7         3/23/98 11:12:02 AM    Dave Wilson     Fixed up 
 *        to remove nested includes and removed commenting on debug case
 *        for error_log
 *        
 *  7    mpeg      1.6         3/19/98 1:01:18 PM     Ismail Mustafa  Added 
 *        define for globals.
 *        
 *  6    mpeg      1.5         3/11/98 2:41:36 PM     Dave Wilson     Fixed an 
 *        error in the debug macro for error_log
 *        
 *  5    mpeg      1.4         3/4/98 10:54:56 AM     Dave Wilson     Added 
 *        conditional definitions for TRUE and FALSE.
 *        
 *  4    mpeg      1.3         2/13/98 3:38:40 PM     Dave Wilson     Latest 
 *        version (usable).
 *        
 *  3    mpeg      1.2         2/4/98 2:12:24 PM      Dave Wilson     Updated 
 *        to a point where semaphores, queues and processes are apparently OK.
 *        
 *  2    mpeg      1.1         2/2/98 11:24:16 AM     Dave Wilson     Changed 
 *        basic types to conform to OpenTV types (u_int32, etc).
 *        
 *  1    mpeg      1.0         12/30/97 11:28:28 AM   Dave Wilson     
 * $
 * 
 *    Rev 1.86   01 Aug 2003 12:52:20   jackmaw
 * SCR(s) 7048 :
 * Increase the number of semaphores from 160 to 180 for OpenTV.
 * 
 *    Rev 1.85   14 Jul 2003 11:39:18   velusws
 * SCR(s) 5354 :
 * Changed initial_value to unsigned for sem_create and num_queues for qu_create to disallow negative initial values (as do the RTOSes).
 * 
 *    Rev 1.84   16 Jan 2003 11:39:36   dawilson
 * SCR(s) 5146 :
 * Removed some redundant definitions and changed others. OpenTV direct segment
 * size is now specified via the software config file.
 * 
 *    Rev 1.83   18 Dec 2002 15:41:08   velusws
 * SCR(s) 4785 :
 * Changed the task_priority delta to size short to enable a better range for task priority change.
 * 
 *    Rev 1.82   24 Oct 2002 10:31:34   mitchei
 * SCR(s): 4828 
 * Added new define TASK_PRIO_NON_PREEMPTED for use when creating a task and
 * changed the function prototype for sem put if the RTOS is NUP
 * 
 *    Rev 1.81   Jun 11 2002 07:10:26   mitchei
 * SCR(s): 3956 
 * If the build is for the MHP application the maximum number of semephores is up from 100 to 200
 * 
 *    Rev 1.80   31 May 2002 15:03:22   dawilson
 * SCR(s) 3913 :
 * Removed double slash comments from the file. These were causing our 
 * dependency list generator to stop parsing the file early and miss trace.h from
 * the generated lists.
 * 
 *    Rev 1.79   08 May 2002 11:32:50   swartzwg
 * SCR(s) 3736 :
 * increased MAX_SEMAPHORES to 160(was 150) for OTV build
 * 
 *    Rev 1.78   07 Mar 2002 11:44:18   mitchei
 * SCR(s): 3322 
 * undo the change
 * 
 *    Rev 1.77   07 Mar 2002 07:47:18   mitchei
 * SCR(s): 3322 
 * defined DEFAULT_PRIORTY as Do_not_create_a_task_with_DEFAULT_PRIORITY_in_UCOS which flags an error if used.
 * 
 *    Rev 1.76   15 Feb 2002 17:40:58   velusws
 * SCR(s) 3013 :
 * This is the public Header file containing the Standard KAL interfaces. Moved the KAL Extension interfaces to include\kalex.h
 * 
 *    Rev 1.75   01 Feb 2002 09:50:52   mitchei
 * SCR(s): 3101 
 * Changed the definition of DEFAULT_PRIORITY and KAL_MAX_TASK_PRIORITY if the RTOS is UCOS
 * 
 * 
 *    Rev 1.74   10 Oct 2001 10:33:06   velusws
 * SCR(s) 2731 :
 * Increased MAX_SEMAPHORES to 150 for OTV builds
 * 
 *    Rev 1.73   19 Sep 2001 14:32:38   velusws
 * SCR(s) 2574 :
 * All our Kal Timeouts are in MilliSeconds. Had some Function arguments named timeout_us. corrected this to timeout_ms
 * 
 *    Rev 1.72   11 Apr 2001 12:52:48   glennon
 * DCS1675 (put by DW): Added new task_sleep_ex function required by video
 * driver.
 * 
 *    Rev 1.71   05 Feb 2001 13:20:56   angela
 * took out prototype of task_priority for Vendor_C to get 
 * vendor_c build successfully
 * 
 *    Rev 1.70   02 Feb 2001 16:20:46   angela
 * Merged Vendor_c changes into the code( mostly #if CUSTOMER==VENDOR_C blocks)
 * see DCS#1049
 * 
 *    Rev 1.69   19 Oct 2000 14:16:58   achen
 * Changed type of task_priority from "char" to "unsigned char".
 * 
 *    Rev 1.68   11 Sep 2000 18:07:04   velusws
 * renamed pool_is_valid to mem_pool_is_valid
 * 
 *    Rev 1.67   11 Sep 2000 17:50:30   mustafa
 * Again increased the number of processes. This was for the VERIFIER tasks.
 *
 *    Rev 1.66   25 Aug 2000 14:46:46   dawilson
 * Increased MAX_PROCESSES
 *
 *    Rev 1.65   24 Aug 2000 17:01:24   velusws
 * changed mem_create_pool() to take 3 arguments (pool_id_ptr, size, name)
 * - return an error code
 *
 *    Rev 1.64   23 Aug 2000 22:08:12   velusws
 * added new functions to determine validity of various objects (tasks,
 * - sems, muts, qus, & heaps)
 *
 *    Rev 1.63   17 Jul 2000 17:32:42   rossst
 * Changed CN8600_application_entry to application_entry.
 *
 *    Rev 1.62   23 Jun 2000 16:57:14   velusws
 * moved MAX_OBJ_NAME_LENGTH from here to hwlib.h
 *
 *    Rev 1.61   20 Jun 2000 14:15:36   dawilson
 * Added #define for largest block search granularity
 *
 *    Rev 1.60   01 Jun 2000 17:00:32   velusws
 * Changed KAL_MAX_TASK_PRIORITY to 239 from 127
 *
 *    Rev 1.59   01 May 2000 18:51:26   rossst
 * Removed gpio reasd/write prototypes.
 *
 *    Rev 1.58   01 May 2000 18:17:40   rossst
 * Added task_scheduling_xxx() functions.
 * Added task states to enum.
 *
 *    Rev 1.57   17 Apr 2000 19:23:58   rossst
 * Replaced macros with function calls for task_create(), sem_create(),
 * mut_create(), qu_create(), qu_send(), qu_receive().
 *
 *    Rev 1.56   17 Apr 2000 18:30:20   rossst
 *
 *
 *    Rev 1.55   18 Nov 1999 20:06:24   dawilson
 * Reworked system timer handling architecture to keep in inside HWLIB.
 *
 *    Rev 1.54   27 Oct 1999 16:18:36   dawilson
 * Changed WAIT_FOREVER to KAL_WAIT_FOREVER
 * Added prototype for CN8600_application_entry
 *
 *    Rev 1.53   14 Oct 1999 14:56:20   rtilton
 * Added TTX net commands.
 *
 *    Rev 1.52   06 Oct 1999 15:58:38   dawilson
 * Changed sem_delete to return a value.
 *
 *    Rev 1.51   06 Oct 1999 15:43:24   dawilson
 * Added task_destroy prototype.
 *
 *    Rev 1.50   23 Sep 1999 14:29:08   dawilson
 * Moved timer function prototypes to HWLIB.H
 *
 *    Rev 1.49   16 Sep 1999 17:06:22   dawilson
 * Added event functions
 *
 *    Rev 1.48   14 Sep 1999 14:08:10   dawilson
 * Changes due to splitting KAL into PSOSKAL and HWLIB components.
 *
 *    Rev 1.47   13 Sep 1999 13:28:42   dawilson
 * Split out trace functions to new component.
 *
 *    Rev 1.46   16 Aug 1999 17:06:04   rtilton
 * Added another NET command for getting profile data.
 *
 *    Rev 1.45   11 Aug 1999 16:38:32   dawilson
 * Changes to completely decouple the KAL from OpenTV.
 *
 *    Rev 1.44   02 Aug 1999 15:36:08   velusws
 * Replaced definition for TRACE_FLS with TRACE_NV.
 * TRACE_FLS will now be derived from TRACE_NV.
 *
 *    Rev 1.43   02 Aug 1999 10:59:48   rossst
 * Added mem_nc_malloc() and mem_nc_free() prototypes.
 *
 *    Rev 1.42   28 Jun 1999 13:21:58   rtilton
 * Added MEMCOMMANDS struct for net commands.
 *
 *    Rev 1.41   23 Jun 1999 13:42:18   rtilton
 * Added IIC net commands.
 *
 *    Rev 1.40   21 Jun 1999 17:37:48   rtilton
 * Added more net commands.
 *
 *    Rev 1.39   15 Jun 1999 16:03:10   mustafa
 * Added set/get trace level command.
 *
 *    Rev 1.38   14 Jun 1999 10:41:24   mustafa
 * Added network command defines.
 *
 *    Rev 1.37   08 Jun 1999 14:01:54   rossst
 * Updated for non-OS dependent BSP.
 *
 *    Rev 1.36   12 May 1999 09:34:20   dawilson
 * Changes to timer API to keep same create/destroy syntax as
 * rest of objects.
 *
 *    Rev 1.35   05 May 1999 14:23:32   dawilson
 * Changed timer API to accept a user data pointer.
 *
 *    Rev 1.34   04 May 1999 15:11:06   dawilson
 * Updated for new, split KAL
 *
 *    Rev 1.33   11 Mar 1999 12:57:48   glennon
 * Added #include of hwconfig.h - was not always included before use of this
 * header file, so some #if statements were throwing warnings which were real.
 *
 *    Rev 1.32   05 Mar 1999 10:24:56   dawilson
 * A few more changes for Neches interrupt handling.
 *
 *    Rev 1.31   04 Mar 1999 18:38:46   dawilson
 * Further changes for Neches version.
 *
 *    Rev 1.30   04 Mar 1999 15:36:12   dawilson
 * Changes for Neches.
 *
 *    Rev 1.29   29 Jan 1999 17:48:34   mooreda
 * Added TRACE_MODEM
 *
 *    Rev 1.28   28 Jan 1999 16:08:28   prattac
 * Added definition for TRACE_CR, for use by system clock recovery driver.
 *
 *    Rev 1.27   07 Jan 1999 17:35:12   rtilton
 * Trace messages are not compiled for non-debug builds.
 *
 *    Rev 1.26   06 Oct 1998 14:50:10   dawilson
 * Increased the default stack size.
 *
 *    Rev 1.25   21 Sep 1998 14:30:52   rossst
 * Corrected the INT_ definitions for the Sabine serial ports.
 *
 *    Rev 1.24   04 Sep 1998 09:55:46   dawilson
 * Added definitions of INT_INVALID for PID7T builds.
 *
 *    Rev 1.23   14 Aug 1998 14:16:00   dawilson
 * Fixed up isr_error_log so that it generates the correct filename and
 * line number.
 *
 *    Rev 1.22   07 Jul 1998 09:40:30   dawilson
 *
 *    Rev 1.21   29 Jun 1998 14:19:18   dawilson
 * Added memory alloc and free functions.
 * Added new trace functions with level and source flags.
 *
 *    Rev 1.20   26 Jun 1998 18:50:08   mustafa
 * Fix for interrupt defines for netlist 06/25.
 *
 *    Rev 1.19   29 May 1998 09:56:38   dawilson
 * Added DEFAULT_PRIORITY from kalint.h
 *
 *    Rev 1.18   26 May 1998 07:34:10   dawilson
 * Added OSD scratchpad size label.
 *
 *    Rev 1.17   06 May 1998 11:43:52   dawilson
 * Added prototype for semaphore_nowait.
 *
 *    Rev 1.16   06 May 1998 04:07:34   rossst
 * Updated PIC/interrupt bit/ID definitions.
 *
 *    Rev 1.15   16 Apr 1998 15:17:12   dawilson
 * Shifted the watchdog timer to timer 1 (just to be neat).
 *
 *    Rev 1.14   15 Apr 1998 13:44:00   dawilson
 * Changed ULONG type to u_int32 to save others having to include
 * pSOS headers.
 *
 *    Rev 1.13   15 Apr 1998 11:20:50   dawilson
 * Changes for Phase3.0 and new opentvx.h
 *
 ****************************************************************************/

