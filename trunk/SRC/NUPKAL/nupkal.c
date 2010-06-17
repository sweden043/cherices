/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                   Conexant Systems Inc. (c) 1998-2003                    */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       nupkal.c
 *
 *
 * Description:    Implementation file for the Standard Kernel Adaptation 
 *                 layer for Nucleus+.
 *
 *
 * Author:         Senthil Veluswamy
 *
 ****************************************************************************/
/* $Header: nupkal.c, 62, 4/15/04 5:31:29 PM, Dave Wilson$
 ****************************************************************************/

/*****************/
/* Include Files */
/*****************/
#include "stbcfg.h"
#include "nup.h"
#ifdef __ARMCC_VERSION
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#else
#include "string.h"
#include "stdarg.h"
#include "stdio.h"
#endif

#ifdef OPENTV
#include <opentvx.h>
#include <otvkal.h>
#endif

#include "kal.h"
#ifdef DRIVER_INCL_KALEXINC
#include "kalex.h"
#include "kalintex.h"
#endif /* DRIVER_INCL_KALEXINC */
#include "kalprv.h"
#define INSTANTIATE_TABLES
#include "kalint.h"
#include "hwlib.h"

#define DEFINE_GLOBALS
#include "globals.h"
#include "retcodes.h"
#ifdef DRIVER_INCL_FPLEDS
 #include "fpleds.h"
#endif /* DRIVER_INCL_FPLEDS */
#include "bldver.h"
#include "iic.h"

/***********************/
/***********************/
/* Function Prototypes */
/***********************/
/***********************/
extern bool InterruptsDisabled(void);
extern u_int32 *GetStackPtr(void);

extern void CleanDCache(void);

extern void reboot_IRD(void);

extern int  GetRAMSize(void);

int time_sleep(unsigned int ms);

void kal_system_timer_handler(timer_id_t timerSystem, void *pArg);

/* N+ task cannot delete itself. So we need this */
void cleanup_task(void *parm);

#ifdef DEBUG
void stack_check_task(void *parm);
#endif

void traceout(char *string, ... );
void TraceOutTelnet(char *string);

#if CPU_TYPE == AUTOSENSE
    extern int GetCPUType();
#endif

/***********/
/***********/
/* Aliases */
/***********/
/***********/
#define TASK_MARKED_DELETE_PRIORITY (CLEN_TASK_PRIORITY - 1)
#define  SYSTEM_QUE_MEM_SIZE  0x10000
/***************/
/***************/
/* Global Data */
/***************/
/***************/

/* Copyright and Version strings */
char *szCopyright = "Copyright Conexant Systems Inc, 1999-2003. All Rights Reserved\n";
char *szBuildVersion = VERSTRING;
char *szBuildTime    = __TIME__;
char *szBuildDate    = __DATE__;
char *szConfig       = CONFIGSTRING;
char *szSWConfig     = SWCONFIGSTRING;

#ifdef DEBUG
/* Create a global here so this can be modified by the debugger */
long stack_check_delay_ticks = STACK_CHECK_DELAY_TICKS;
#endif

#define SC_PNA NO

#if SC_PNA == YES
int sdTraceSocket = -1;
#endif
unsigned long TelnetTracePID = 0;

/* Client timer tick function */
PFNTICK gTickFunc = (PFNTICK)NULL;

/* KAL Serialisation Semaphores */
NU_SEMAPHORE nplus_pool_sem;
sem_id_t kal_proc_sem;
sem_id_t kal_del_sem;
/* sem_id_t kal_gpio_sem; */
#ifdef DRIVER_INCL_KALEXINC
extern sem_id_t kal_mut_sem;
#endif /* DRIVER_INCL_KALEXINC */
sem_id_t kal_qu_sem;
sem_id_t kal_pool_sem;

/* Id of Task marked for Deletion (by task_terminate) */
task_id_t gDeleteTaskId = (task_id_t)-1;

/* Resource maxima */
u_int32 gMaxProcesses  = MAX_PROCESSES;
u_int32 gMaxSemaphores = MAX_SEMAPHORES;
u_int32 gMaxQueues     = MAX_QUEUES;
u_int32 gMaxEvents     = MAX_EVENTS;
#ifdef DRIVER_INCL_KALEXINC
extern u_int32 gMaxMutexes;
extern u_int32 gMaxMemPools;
#endif /* DRIVER_INCL_KALEXINC */

u_int32 gSemsCreated   = 0;

/* KAL Process descriptor table */
proc_descriptor processes[MAX_PROCESSES];

#ifdef DRIVER_INCL_KALEXINC
/* KAL Mutex descriptor table */
extern mut_desc mutex[MAX_MUTEXES];
#endif /* DRIVER_INCL_KALEXINC */

/* KAL Memory Pool descriptor table */
mem_pool_desc mem_pool[MAX_MEMORY_POOLS];

/* Initialisation checks */
bool bKalUp = FALSE;
bool bKalIntOk = FALSE;
bool bKalTimersOK = FALSE;
bool bEarlyInitOK = FALSE;

u_int32 uSetSemCheck = 0;

#ifdef DEBUG
u_int32 dwNumNcMalloc = 0;
u_int32 dwNumNcFree   = 0;
#endif

#if (defined MEM_ERROR_DEBUG) && (defined OPENTV) && (defined DEBUG)
bool bDumpHeapNow = FALSE;
extern void dump_direct_heap(void);
#endif

/* Size of installed RAM as calculated on startup */
extern u_int32 RAMSize;

extern volatile u_int32 IntMask;
extern u_int32 gInterrupt_State;
extern int TCD_Interrupt_Level;

/* N+ Task Entry Point Format */
typedef VOID (*TASK_ENTRY_POINT)(UNSIGNED, VOID *);

/***************/
/***************/
/* New defines */
/***************/
/***************/
/* System Memory (we do the memory alloc/dealloc from here for all the objects
   components, etc as N+ does not handle system memory */
NU_MEMORY_POOL gSystemMemory;

/* Reserved memory to guarantee pre-defined resources */
NU_MEMORY_POOL gSysTaskStackMemory;
NU_PARTITION_POOL gSysSemCtrlBlkMemory;

NU_PARTITION_POOL gSysPipeCtrlBlkMemory;
NU_MEMORY_POOL gSysPipeMesgMemory;

NU_PARTITION_POOL gSysQuCtrlBlkMemory;
NU_MEMORY_POOL gSysQuMesgMemory;


/* Root Task vars */
extern NU_TASK gRootTask;

/* To save the system pool info into KAL pool structure */
extern VOID *FirstAvailMem;

/* N+ Control Blocks */
NU_TASK NU_Task_CtrlBlk[MAX_PROCESSES];
NU_MEMORY_POOL NU_MemPool_CtrlBlk[MAX_MEMORY_POOLS];
NU_EVENT_GROUP NU_Evt_CtrlBlk[MAX_PROCESSES];

/* ID counter */
u_int8 SemNameCount;
u_int8 QuNameCount;

#ifndef DRIVER_INCL_KALEXINC
static task_id_t task_create_ex(PFNTASKEX entry_point, int argc, void **argv,
                                void *stack, size_t stack_size, int prio,
                                const char *name);
static int task_id_from_name(const char *name, task_id_t *pid);
static int task_suspend(task_id_t pid);
//static int task_resume(task_id_t pid);
int task_resume(task_id_t pid);

sem_id_t sem_create_ex(unsigned int initial_value, 
                              const char *name, 
                              task_wait_order_t wait_order);
static int sem_is_valid(sem_id_t sem_id, bool *valid);
static int sem_id_from_name(const char *name, sem_id_t *sem_id);

#define KAL_QMSG_SIZE      16
/*
static queue_id_t qu_create_ex(unsigned int max_elements, 
                               const char *name, 
                               int max_length,
                               qu_msg_type_t msg_type, 
                               task_wait_order_t wait_order);
*/
static int qu_is_valid(queue_id_t qu_id, bool *valid);
/*
static int qu_send_ex(queue_id_t qu_id, void *message, int message_length,
                      u_int32 timeout_ms, qu_priority_t priority);
static int qu_receive_ex(queue_id_t qu_id, u_int32 timeout_ms, void *message, 
*/
static int qu_id_from_name(const char *name, queue_id_t *qu_id);

#define KAL_SYS_MEM_POOL_ID  (MPID_FROM_INDEX(0))
static int mem_malloc_ex(pool_id_t pool_id, u_int32 uSize, void **ptr);
static int mem_free_ex(pool_id_t pool_id, void *ptr);

#endif /* DRIVER_INCL_KALEXINC */

/******************************/
/******************************/
/** Initialisation Functions **/
/******************************/
/******************************/

/************************************************************************/
/* Initialise all internal KAL structures - called before N+ is running */
/************************************************************************/
int kal_early_initialise(void)
{
   if(bKalIntOk)
   {
      return(FALSE);
   }

   /* Clear all the kal descriptors */
   memset(processes, 0, sizeof(proc_descriptor) * MAX_PROCESSES);
   #ifdef DRIVER_INCL_KALEXINC
   memset(mutex, 0, sizeof(mut_desc) * MAX_MUTEXES);
   #endif /* DRIVER_INCL_KALEXINC */
   memset(mem_pool, 0, sizeof(mem_pool_desc) * MAX_MEMORY_POOLS);

   /* Initialize the hardware library interrupt functions */
   bEarlyInitOK = hwlib_int_initialise();
   if(!bEarlyInitOK)
   {
      error_log(ERROR_FATAL | RC_KAL_NOTHOOKED);
      return(FALSE);
   }

   return(TRUE);
}

/*****************************************************/
/* Initialise the KAL - called during system startup */
/*****************************************************/

#if defined(SEACHANGE_VOD_ENABLE) &&  (SEACHANGE_VOD_ENABLE == YES)

	#define	ITV_TASK_MAX		4

	#define EXTEND_SIZE 	(1024*32 * ITV_TASK_MAX)
#else
	#define EXTEND_SIZE		0
#endif

int kal_initialise(void)
{
   int iRetcode;
   bool bRetcode;
   task_id_t pid;
   UNSIGNED *mem_ptr;
   UNSIGNED mem_unit_size;

   if(bKalUp)
   {
      return(1);
   }

   /* If the interrupt initialization has not been done, do it now */
   if(!bEarlyInitOK)
   {
      iRetcode = kal_early_initialise();
      if(iRetcode==FALSE)
      {
         return(iRetcode);
      }
   }

   /* Reserve memory to guarantee pre-defined resources */

   /* Task Stack Memory */

   /*********************************************************************/
   /* From the earlier statistic in "otvkal.h"                          */
   /* Amount of memory we need to set aside for process stacks. The +2  */
   /* is for safety margin. Although it is possible to specify any      */
   /* stack size when creating a task, looking at the 44 tasks in our   */
   /* largest OpenTV image shows that the vast majority use the default */
   /* and for every task that uses a larger stack there is at least one */
   /* that uses a smaller one so this calculation should be safe.       */
   /*********************************************************************/
   mem_unit_size = (( DEFAULT_STACK_SIZE + KAL_OS_POOL_ALLOC_OVERHEAD
                        + sizeof(UNSIGNED) - 1) 
                      / sizeof(UNSIGNED)) 
                     * sizeof(UNSIGNED);

   iRetcode = NU_Allocate_Memory(&gSystemMemory, 
                                 (VOID **)&mem_ptr,
                                 #if 0
   #ifdef OPENTV
   /***************************************/
   /* From JoeK's earlier Stack reduction */
   /***************************************/
                                 (UNSIGNED)((mem_unit_size * (MAX_PROCESSES+2)) - 50000),
   #else /* OPENTV */
   #ifdef MHP
                                 (UNSIGNED)(mem_unit_size * (MAX_PROCESSES*3)),
   #else /* MHP */
                                 (UNSIGNED)(mem_unit_size * (MAX_PROCESSES+2)),
   #endif /* MHP */
   #endif /* OPENTV */
   #endif
   					(UNSIGNED)(mem_unit_size * (MAX_PROCESSES+2) + EXTEND_SIZE),
                                 NU_NO_SUSPEND);
   if(iRetcode != NU_SUCCESS)
   {
      error_log(ERROR_FATAL | RC_KAL_SYSERR);
      return(0);
   }

   iRetcode = NU_Create_Memory_Pool(&gSysTaskStackMemory,
                                    "KTSM",
                                    mem_ptr,
 #if 0
   #ifdef OPENTV
   /***************************************/
   /* From JoeK's earlier Stack reduction */
   /***************************************/
                                 (UNSIGNED)((mem_unit_size * (MAX_PROCESSES+2)) - 50000),
   #else /* OPENTV */
   #ifdef MHP
                                 (UNSIGNED)(mem_unit_size * (MAX_PROCESSES*3)),
   #else /* MHP */
                                 (UNSIGNED)(mem_unit_size * (MAX_PROCESSES+2) + EXTEND_SIZE),
   #endif /* MHP */
   #endif /* OPENTV */
   #endif 			
   					(UNSIGNED)(mem_unit_size * (MAX_PROCESSES+2) + EXTEND_SIZE),
                                    (UNSIGNED)KAL_POOL_MIN_ALLOC_SIZE,
                                    NU_PRIORITY);
   
   
   if(iRetcode != NU_SUCCESS)
   {
      error_log(ERROR_FATAL | RC_KAL_SYSERR);
      return(0);
   }

   /* Semaphore Control Block */
   mem_unit_size = (( sizeof(NU_SEMAPHORE) + KAL_OS_PART_MEM_BLK_OVERHEAD
                         + sizeof(UNSIGNED) - 1) 
                       / sizeof(UNSIGNED)) 
                     * sizeof(UNSIGNED);
   iRetcode = NU_Allocate_Memory(&gSystemMemory,
                                 (VOID **)&mem_ptr,
                                 (UNSIGNED)(mem_unit_size * MAX_SEMAPHORES),
                                 NU_NO_SUSPEND);
   if(iRetcode != NU_SUCCESS)
   {
      error_log(ERROR_FATAL | RC_KAL_SYSERR);
      return(0);
   }

   iRetcode = NU_Create_Partition_Pool(&gSysSemCtrlBlkMemory,
                                       "KSCB",
                                       mem_ptr,
                                       (UNSIGNED)(mem_unit_size * MAX_SEMAPHORES),
                                       sizeof(NU_SEMAPHORE),
                                       NU_PRIORITY);
   if(iRetcode != NU_SUCCESS)
   {
      error_log(ERROR_FATAL | RC_KAL_SYSERR);
      return(0);
   }

////////////////////////////////////////////////////////////////
		// add by ÀµÔÆÁ¼2006.03 
// Pipe Control Block
   mem_unit_size = (( sizeof(NU_PIPE) + KAL_OS_PART_MEM_BLK_OVERHEAD
                         + sizeof(UNSIGNED) - 1) 
                       / sizeof(UNSIGNED)) 
                     * sizeof(UNSIGNED);
   iRetcode = NU_Allocate_Memory(&gSystemMemory,
                                 (VOID **)&mem_ptr,
                                 (UNSIGNED)(mem_unit_size * MAX_PIPES),
                                 NU_NO_SUSPEND);
   if(iRetcode != NU_SUCCESS)
   {
      error_log(ERROR_FATAL | RC_KAL_SYSERR);
      return(0);
   }

   iRetcode = NU_Create_Partition_Pool(&gSysPipeCtrlBlkMemory,
                                       "KPCB",
                                       mem_ptr,
                                       (UNSIGNED)(mem_unit_size * MAX_PIPES),
                                       sizeof(NU_PIPE),
                                       NU_PRIORITY);
   if(iRetcode != NU_SUCCESS)
   {
      error_log(ERROR_FATAL | RC_KAL_SYSERR);
      return(0);
   }

	// Pipe  Message Block
     iRetcode = NU_Allocate_Memory(&gSystemMemory,
                                 (VOID **)&mem_ptr, 

                                    (UNSIGNED)(SYSTEM_QUE_MEM_SIZE),

                                 NU_NO_SUSPEND);
   if(iRetcode != NU_SUCCESS)
   {
      error_log(ERROR_FATAL | RC_KAL_SYSERR);
      return(0);
   }

   iRetcode = NU_Create_Memory_Pool(&gSysPipeMesgMemory,
                                    "KPMM",
                                    mem_ptr,
                                    
                                    (UNSIGNED)(SYSTEM_QUE_MEM_SIZE),

                                    KAL_POOL_MIN_ALLOC_SIZE,
                                    NU_PRIORITY);
   if(iRetcode != NU_SUCCESS)
   {
      error_log(ERROR_FATAL | RC_KAL_SYSERR);
      return(0);
   }


////////////////////////////////////////////////////////////////

   /* Queue Control Block */
   mem_unit_size = (( sizeof(NU_QUEUE)+KAL_OS_PART_MEM_BLK_OVERHEAD
                         + sizeof(UNSIGNED) - 1) 
                       / sizeof(UNSIGNED)) 
                     * sizeof(UNSIGNED);
   iRetcode = NU_Allocate_Memory(&gSystemMemory,
                                 (VOID **)&mem_ptr,
                                 (UNSIGNED)(mem_unit_size * MAX_QUEUES),
                                 NU_NO_SUSPEND);
   if(iRetcode != NU_SUCCESS)
   {
      error_log(ERROR_FATAL | RC_KAL_SYSERR);
      return(0);
   }

   iRetcode = NU_Create_Partition_Pool(&gSysQuCtrlBlkMemory,
                                       "KQCB",
                                       mem_ptr,
                                       (UNSIGNED)(mem_unit_size * MAX_QUEUES),
                                       sizeof(NU_QUEUE),
                                       NU_PRIORITY);
   if(iRetcode != NU_SUCCESS)
   {
      error_log(ERROR_FATAL | RC_KAL_SYSERR);
      return(0);
   }

   /* Queue Message Block */
   /*********************************************************************/
   /* Trying to allocate 20K for our Qu Message Blocks. There is a total*/
   /* of around 1200 Messages (16Bytes/Message) requested for various   */
   /* qu_creates in an OTV build.                                       */
   /* !!Just an estimation. Open to correction!!                        */
   /*********************************************************************/
   iRetcode = NU_Allocate_Memory(&gSystemMemory,
                                 (VOID **)&mem_ptr, 
                                    #ifdef DVNCA
                                    (UNSIGNED)0x19800,
                                    #else
                                    (UNSIGNED)0x5800,
                                    #endif
                                 NU_NO_SUSPEND);
   if(iRetcode != NU_SUCCESS)
   {
      error_log(ERROR_FATAL | RC_KAL_SYSERR);
      return(0);
   }

   iRetcode = NU_Create_Memory_Pool(&gSysQuMesgMemory,
                                    "KQMM",
                                    mem_ptr,
                                    #ifdef DVNCA
                                    (UNSIGNED)0x19800,
                                    #else
                                    (UNSIGNED)0x5800,
                                    #endif
                                    KAL_POOL_MIN_ALLOC_SIZE,
                                    NU_PRIORITY);
   if(iRetcode != NU_SUCCESS)
   {
      error_log(ERROR_FATAL | RC_KAL_SYSERR);
      return(0);
   }

   /*******************************************************/
   /* Establish the system memory pool as the first pool. */
   /*******************************************************/
   mem_pool[0].flags = POOL_FLAG_STRUCT_IN_USE;
   strncpy(mem_pool[0].name, "KALSYSPL", NUP_MAX_NAME_LENGTH);
   mem_pool[0].pool_start = (void *)FirstAvailMem;
   mem_pool[0].pool_size = (u_int32)(GetRAMSize() - (int)FirstAvailMem - 4096);
   mem_pool[0].rtos_id = (u_int32)(&gSystemMemory);

   /***************************************************/
   /* Create the semaphore used for the mem_malloc_ex */
   /* routine. This must be created first before any  */
   /* other KAL routine uses system memory.           */
   /***************************************************/
   if(NU_Create_Semaphore(&nplus_pool_sem, "NPPLSEM", 
                                          1, NU_PRIORITY)!=NU_SUCCESS)
   {
      error_log(ERROR_FATAL | RC_KAL_SYSERR);
      return(0);
   }

   kal_pool_sem = (sem_id_t)&nplus_pool_sem;

   /* Update the Semaphore Created Counter */
   gSemsCreated++;

   /**************************************************/
   /* Create the semaphores we use for serialisation */
   /**************************************************/
   kal_proc_sem = sem_create(1, "KPRSEM");
   kal_del_sem = sem_create(1, "KCLSEM");
   kal_qu_sem = sem_create(1, "KQUSEM");
   #ifdef DRIVER_INCL_KALEXINC
   kal_mut_sem = sem_create(1, "KMUTSEM");
   #endif /* DRIVER_INCL_KALEXINC */
/*    kal_gpio_sem = sem_create(1, "KGPIOSEM"); */

   if(!kal_proc_sem || !kal_qu_sem 
   #ifdef DRIVER_INCL_KALEXINC
                                 || !kal_mut_sem
   #endif /* DRIVER_INCL_KALEXINC */
                                     || !kal_pool_sem) /* || !kal_gpio_sem */
   {
      error_log(ERROR_FATAL | RC_KAL_SYSERR);
      return(0);
   }

   /* Initialise the OTV KAL */
   #ifdef OPENTV
   otvkal_initialise();
   #endif

   /****************************************************/
   /* Initialise the trace and error handler component */
   /****************************************************/
   bRetcode = trace_init();
   /* I deliberately don't abort if this initialisation fails since */
   /* trace is not a vital function of the IRD.                     */

   /* If the interrupt initialisation has not been done, do it now */
   if(!bKalTimersOK)
   {
     iRetcode = kal_timer_initialise();
     if(iRetcode == FALSE)
       return(iRetcode);
   }

   if(bKalUp)
   {
      /* Error - reinitialising the KAL ! */
      error_log(ERROR_WARNING | RC_KAL_INITIALISED);
      return(0);
   }

#ifdef DEBUG
   /***************************************************************************/
   /* Create our stack checking task. This runs in the background looking for */
   /* processes which over-write the end of their stacks (or, at least, get   */
   /* close to it).                                                           */
   /***************************************************************************/

   pid = task_create(stack_check_task,
                   (void *)NULL,
                   (void *)NULL,
                   STCK_TASK_STACK_SIZE,
                   STCK_TASK_PRIORITY,
                   STCK_TASK_NAME);
   if(!pid)
   {
      trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
         "Stack probe check task failed to start\n");
      error_log(ERROR_WARNING | RC_KAL_TASKFAIL);
   }

#endif /* DEBUG */

   /***************************************************************************/
   /* Create the deleted tasks cleanup task. This runs in the background      */
   /* and destroyes any tasks marked as deleted. This is needed as tasks      */
   /* cannot themselves in N+.                                                */
   /***************************************************************************/
   pid = task_create(cleanup_task,
                     (void *)NULL,
                     (void *)NULL,
                     CLEN_TASK_STACK_SIZE,
                     CLEN_TASK_PRIORITY,
                     CLEN_TASK_NAME);
   if(!pid)
   {
      trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
         "Cleanup task failed to start\n");
      error_log(ERROR_WARNING | RC_KAL_TASKFAIL);
   }

   bKalUp = TRUE;

   return(1);
}

#ifdef DEBUG
/************************************************************************/
/* Stack checking task                                                  */
/*                                                                      */
/* This task runs periodically and checks to ensure that no process has */
/* written over the end of its stack space. If an offending process is  */
/* found, a large error message is printed and the system aborts.       */
/************************************************************************/
void stack_check_task(void *parm)
{
   int iLoop;
/*  DEBUG HACK */
#if 0
   task_id_t tTemp;

   tTemp = task_id();
#endif
/*  END */

   while(1)
   {
      /* Grab the process descriptor semaphore */
      if(sem_get(kal_proc_sem, KAL_WAIT_FOREVER) != RC_OK)
      {
         error_log(ERROR_WARNING | RC_KAL_SYSERR);
      }

      /* Loop through the processes looking for active ones with */
      /* stack probes inserted.                                  */
      for(iLoop = 0; iLoop < MAX_PROCESSES; iLoop++)
      {
         if(processes[iLoop].proc_flags & PROC_FLAG_STRUCT_IN_USE)
         {
            if(processes[iLoop].proc_flags & PROC_FLAG_STACK_CHECK)
            {
               #if (BACKFIT_RTOS_TASKS_INTO_KAL == YES)
               if((u_int32 *)NULL == processes[iLoop].pstack_check)
               {
                  continue;
               }
               #endif /* (BACKFIT_RTOS_TASKS_INTO_KAL == YES) */
               if(*(processes[iLoop].pstack_check) != STACK_CHECK_MARKER)
               {
                  /* Using printf here since this assures the output is      */
                  /* displayed. If we use trace_new there is the possibility */
                  /* that the message will be lost.                          */
                  printf("**** STACK OVERFLOW! ****\n");
                  printf("Process index %d, start addr 0x%08lx, original SP 0x%08lx\n",
                          iLoop,
                          (u_int32)processes[iLoop].entry_point,
                          (u_int32)processes[iLoop].pstack_ptr);
                  printf("*************************\n");

                  /* Turn checking off for this process since, otherwise this */
                  /* function will trace continually!                         */

                  processes[iLoop].proc_flags &= ~PROC_FLAG_STACK_CHECK;
               }
            }
         }
      }

      /* Give up the process descriptor semaphore */
      sem_put(kal_proc_sem);

      /* Sleep for a while */
      NU_Sleep(stack_check_delay_ticks);
   }
}
#endif /* DEBUG */

/***************************************/
/* Kill other tasks marked for deleted */
/***************************************/
void cleanup_task(void *parm)
{
   int iRetcode;
   int iLoop;

   while(1)
   {
      /* We have been woken up. Some Culling to do! */
      //trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, "Clean Up Task Woken Up.\n");

      iLoop = 0;

      /* Get the clean up semaphore */
      if(sem_get(kal_del_sem, KAL_WAIT_FOREVER) != RC_OK)
      {
         error_log(ERROR_WARNING | RC_KAL_SYSERR);
      }

      /* Make sure we have a valid Task to be deleted */
      if(gDeleteTaskId != (task_id_t)-1)
      {
         iLoop = INDEX_FROM_PID(gDeleteTaskId);

         /* Get the process serialiation semaphore */
         sem_get(kal_proc_sem, KAL_WAIT_FOREVER);

         if((processes[iLoop].proc_flags & PROC_FLAG_STRUCT_IN_USE) &&
            (processes[iLoop].proc_flags & PROC_FLAG_DELETE_TASK))
         {
            /* Terminate this task */
            iRetcode = NU_Terminate_Task((NU_TASK *)processes[iLoop].psos_pid);
            if(iRetcode != NU_SUCCESS)
            {
               /* Error Terminating Task */
               trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
                  "Could not terminate N+ Task. Kal Pid:%x\n",
                     PID_FROM_INDEX(iLoop));
               error_log(ERROR_WARNING | RC_KAL_SYSERR);
            }

            /* Delete this task */
            iRetcode = NU_Delete_Task((NU_TASK *)processes[iLoop].psos_pid);
            if(iRetcode != NU_SUCCESS)
            {
               /* Error Deleting Task */
               trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
                  "Could not delete N+ Task. Kal Pid:%x\n",
                     PID_FROM_INDEX(iLoop));
               error_log(ERROR_WARNING | RC_KAL_SYSERR);
            }
            else
            {
               /* Free this task's resources */
               /* Delete it's sleep semapore */
               sem_delete(processes[iLoop].sleep_sem);
               /* Free it's stack memory */
               mem_free_ex(KAL_SYS_MEM_POOL_ID, 
                              (void*)processes[iLoop].pstack_mem_ptr);
               /* Free it's event group if it has one */
               if(processes[iLoop].pEvent)
               {
                  NU_Delete_Event_Group(processes[iLoop].pEvent);
                  memset(processes[iLoop].pEvent, 0, sizeof(NU_EVENT_GROUP));
               }
               /* Clear the process descriptor */
               memset(&processes[iLoop], 0, sizeof(proc_descriptor));
            }
         }
         else
         {
            /* Error ! */
            trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
               "gDeleteTaskId Task (%x) not marked for deletion or in Use!\n",
                  gDeleteTaskId);
            error_log(ERROR_WARNING | RC_KAL_SYSERR);
         }

         /* Return the process serialization semaphore */
         sem_put(kal_proc_sem);

         /* Clear the Delete Task Id var */
         gDeleteTaskId = (task_id_t)-1;
      }

      /* Return the clean semaphore */
      sem_put(kal_del_sem);

      /************************************************************************/
      /* There is a chance that a task waiting to be deleted, (if of higher   */
      /* priority than the Cleanup task) will task switch , start running now */
      /* and will try to resume the (Unsuspended) Clean up task. It will then */
      /* suspend itself and the clean up task as soon as it gets to run will  */
      /* suspend itself. Thus there is a chance of the cleanup task remaining */
      /* suspended and not running again.                                     */
      /*                                                                      */
      /* To prevent this from happening, all tasks marked for deletion will be*/
      /* changed to a Priority lower than the Cleanup Task, thus making sure  */
      /* that the Clean Task will not be preempted until it finishes and      */
      /* suspends itself                                                      */
      /************************************************************************/

      /* Deleted all the tasks marked for deletion. Hibernate. */
      task_suspend(task_id());
   }
}

/**********************************************************************/
/* Grab the system tick timer and watchdog timer. Initialise them and */
/* turn on their interrupts                                           */
/**********************************************************************/
int kal_timer_initialise(void)
{

   hwtimer_sys_register_client(kal_system_timer_handler, 
                                 SYSTIMER_CLIENT_KAL, (void *)NULL);

   bKalTimersOK = TRUE;

   return((int)TRUE);
}

/******************************************************************/
/* Allow a single client to register a system timer tick callback */
/******************************************************************/
void kal_set_timer_tick_func(PFNTICK pfnTick)
{
   not_from_critical_section();
   not_interrupt_safe();
   
   //trace_new(TRACE_KAL | TRACE_LEVEL_2, "kal_set_timer_tick_func\n");

   if((gTickFunc != NULL) && (gTickFunc != pfnTick))
   {
      trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
         "Attempt to re-hook system tick function\n");
      error_log(ERROR_WARNING | RC_KAL_INUSE);
      return;
   }

   gTickFunc = pfnTick;

   return;
}

/*************************************************/
/* Return the kernel tick period in milliseconds */
/*************************************************/
unsigned int kal_get_tick_period(void)
{
   u_int32 uCurrentRate;

   not_from_critical_section();

   //isr_trace_new(TRACE_KAL | TRACE_LEVEL_2, "kal_get_tick_period\n", 0, 0);

   uCurrentRate = hwtimer_sys_get_rate();

   if(uCurrentRate < 1000)
   {
      /* Error - OpenTV can't handle timer ticks with period < 1ms */
      error_log(ERROR_FATAL | RC_KAL_PERIOD);
      return(1);
   }
   else
   {
      return(uCurrentRate/1000);
   }
}

/***************************************************************/
/* System timer handler. Informs another client of timer ticks */
/***************************************************************/

/************************************************************************/
/* Note: This function used to call the RTOS tick function too but in   */
/* some cases (eg. vxWorks loaded as a boot rom) the KAL will not be    */
/* present in an image which requires system timers to be up and        */
/* running. As a result, HWLIB and the BSP are now responsible for      */
/* routing the tick to the appropriate RTOS function. The KAL registers */
/* as a client of the HWLIB tick handler during kal_timer_initialise    */
/************************************************************************/
void kal_system_timer_handler(timer_id_t timerSys, void *pArg)
{
   /* Tell any registered client about the timer tick */
   if(gTickFunc)
   {
      gTickFunc();
   }
}

/**************************************/
/**************************************/
/** Basic RTOS abstraction functions **/
/**************************************/
/**************************************/

/********************/
/********************/
/** Task Functions **/
/********************/
/********************/

void kal_process_wrapper(u_int32 argc, void *argv)
{
   u_int32 *argv_array =      (u_int32 *)argv;
   int arg_c =                (int)argv_array[0];
   void **arg_v =             (void **)(argv_array[1]);
   PFNTASK entry_point =      (PFNTASK)argv_array[2];
   PFNTASKEX entry_point_ex = (PFNTASKEX)argv_array[2];
   u_int32 proc_index =       (u_int32)argv_array[3];
#ifdef DEBUG
   u_int32 stack_size,
      *pStack, *pCheck;
#endif /* DEBUG */
#ifdef CHECK_STACK_USAGE
   u_int8 *ptr;
#endif /* CHECK_STACK_USAGE */

   not_from_critical_section();

   not_interrupt_safe();
   /*trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
      "KAL process wrapper called for task at 0x%08lx. Index %d\n", 
         entry_point, proc_index);*/

#ifdef DEBUG

   /* Get the serialization semaphore */
   if(sem_get(kal_proc_sem, KAL_WAIT_FOREVER) != RC_OK)
   {
      trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
         "Unable to get the serialization semaphore (%x)\n",
            kal_proc_sem);
      error_log(ERROR_FATAL | RC_KAL_SYSERR);
      return;
   }

   /* Is the stack large enough to be checked? */
   stack_size = processes[proc_index].stack_size;
   if(stack_size <= STACK_CHECK_GUARD)
   {
      trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
         "Stack too small (%d) for checking!\n", stack_size);
   }
   else
   {
      /* Get the stack Pointer */
      pStack = GetStackPtr();

#ifdef CHECK_STACK_USAGE
      /*********************************************************/
      /* Fill the stack with a pattern in order for a task to  */
      /* periodically monitor global stack usage in the system */
      /*********************************************************/
      ptr = (u_int8 *)pStack-stack_size;
     /* trace_new(TRACE_KAL | TRACE_LEVEL_2,
"Current stack pointer 0x%08lx. Bottom of stack 0x%08xlx. Filling with pattern\n",
            (u_int32)pStack, (u_int32)ptr);*/

      for(;ptr<(u_int8 *)pStack;)
      {
         *ptr++ = CHECK_STACK_USAGE_CHAR;
      }
#endif /* CHECK_STACK_USAGE */

      /*********************************************************************/
      /* Add a known value to somewhere near the bottom of the stack and   */
      /* stick a pointer to it in the process descriptor. This allows our  */
      /* extra stack check process to periodically ensure that the stack   */
      /* as not overflowed.                                                */
      /*********************************************************************/
      pCheck = (u_int32 *)((u_int8 *)pStack - stack_size + STACK_CHECK_GUARD);

      *(u_int32 *)pCheck = STACK_CHECK_MARKER;

      /* Update the process info */
      processes[proc_index].pstack_ptr = pStack;
                           /* This is the Stack visible to the actual task */
      processes[proc_index].pstack_check = (u_int32 *)pCheck;
      processes[proc_index].proc_flags  |= PROC_FLAG_STACK_CHECK;

      sem_put(kal_proc_sem);

      /*trace_new(TRACE_KAL | TRACE_LEVEL_2, 
         "Current stack pointer 0x%08lx. Inserted probe at 0x%08lx\n",
            (u_int32)pStack, (u_int32)pCheck);*/
   }

#endif /* DEBUG */

   /* Call the task entry point. If we passed a NULL for the 2nd  */
   /* task argument (i.e. argv), then we are calling an old-style */
   /* task that only takes 1 parameter.                           */
   if(arg_v == (void **)NULL)
   {
      entry_point((u_int32*)arg_c);
   }
   else
   {
      entry_point_ex(arg_c, arg_v);
   }

#ifdef DEBUG
   /* If this code ever executes, the task has exited wrongly */
   /*trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS,
       "* ERROR! Task 0x%08lx Returned *\n", entry_point);*/
//trace("temp remove error_log!!!\n");
   error_log(ERROR_FATAL | RC_KAL_SYSERR);
   do
   {
      task_terminate();
   }
   while(1);  /* Dont let the task get away! :) */
#endif /* DEBUG */
}

/*************************************************/
/* Create and activate a new thread of execution */
/*************************************************/
task_id_t task_create(PFNTASK entry_point, 
                        void *arg, 
                        void *stack, 
                        size_t stack_size, 
                        int prio, 
                        const char *name)
{
   return(task_create_ex((PFNTASKEX)entry_point, 
                           (int)arg,
                           0, 
                           stack,
                           stack_size,
                           prio,
                           name));
}

task_id_t task_create_ex(PFNTASKEX entry_point, 
                           int argc, 
                           void **argv, 
                           void *stack, 
                           size_t stack_size, 
                           int prio, 
                           const char *name)
{
   int            iLoop;
   int            iFound;
   int            iRetcode;
   unsigned char  byPriority;
   char           SemaphoreName[NUP_MAX_NAME_LENGTH];
   /* set the preemption option depending on the TASK_PRIO_NON_PREEMPTED bit */
   unsigned char  byPreemptOption = (( prio & TASK_PRIO_NON_PREEMPTED ) ? NU_NO_PREEMPT : NU_PREEMPT );
   
   not_from_critical_section();
   not_interrupt_safe();
   
   /* clear the bit in the priority that selects non-preemtive */
   /* task option so the priority value is in range */
   prio &= ( 0xFFFF - TASK_PRIO_NON_PREEMPTED );

   trace_new(TRACE_KAL | TRACE_LEVEL_2, 
      "task_create. Entry Point:0x%08lx, priority %d, stack %d\n",
          entry_point, prio, stack_size);

   /* Is the entry point valid? */
   if(entry_point == NULL)
   {
      /* Error - NULL function pointer passed   */
      error_log(ERROR_WARNING | RC_KAL_BADPTR);
      return((task_id_t)NULL);
   }

   /* Is the priority valid? */
   if((prio < KAL_MIN_TASK_PRIORITY) || (prio > KAL_MAX_TASK_PRIORITY))
   {
      error_log(ERROR_WARNING | RC_KAL_INVALID);
      return((task_id_t)NULL);
   }

   /* Grab the serialisation semaphore */
   if(sem_get(kal_proc_sem, KAL_WAIT_FOREVER) != RC_OK)
   {
      /* Unable to get the process block protection semaphore */
      sem_put(kal_proc_sem);
      error_log(ERROR_WARNING | RC_KAL_SYSERR);
      return((task_id_t)NULL);
   }

   /* Find a free descriptor. Also use this opportunity to make sure that
      the passed name doesn't already exist */
   iLoop  = 0;
   iFound = -1;
   
   while(iLoop < MAX_PROCESSES)
    {
        /* If this process table slot is already in use, check that the */
        /* name we were passed doesn't clash with the existing name (if */
        /* we were passed a non-NULL name).                             */
        if(processes[iLoop].proc_flags & PROC_FLAG_STRUCT_IN_USE)
        {   
            if((name != NULL) && (strncmp(name, processes[iLoop].name, NUP_MAX_NAME_LENGTH)==0))
            {
               /* Error - there is already a task created using same name */
               sem_put(kal_proc_sem);
               error_log(ERROR_WARNING | RC_KAL_INUSE);
               return((task_id_t)NULL);
            }
        }
        else
        {
            /* The process table slot is not full so copy the number for later use */
            iFound = iLoop;
        }
        iLoop++;
   }

   if(iFound == -1)
   {
      /* Error - no free descriptors were found */
      sem_put(kal_proc_sem);
      error_log(ERROR_WARNING | RC_KAL_NORESOURCES);
      return((task_id_t)NULL);
   }
   else
   {
      processes[iFound].proc_flags = PROC_FLAG_STRUCT_IN_USE;
   }

   /* Derive a task name from the descriptor index */
   if(name == NULL)
   {
      sprintf(processes[iFound].name, "T%03x", iFound);
   }
   else
   {
      strncpy(processes[iFound].name, name, NUP_MAX_NAME_LENGTH);
   }

   /* Make sure the name is null terminated */
   processes[iFound].name[NUP_MAX_NAME_LENGTH-1] = '\0';

   /* Fix up the Stack Size if required */
   if(stack_size == 0)
   {
      stack_size = DEFAULT_STACK_SIZE;
   }

   /* Create a stack for this task */
   iRetcode = NU_Allocate_Memory(&gSysTaskStackMemory,
                                 (void**)(&processes[iFound].pstack_mem_ptr),
                                 stack_size,
                                 NU_NO_SUSPEND);
   if(iRetcode != RC_OK)
   {
      /* Stack could not be created ! */
      NU_Deallocate_Memory((void*)processes[iFound].pstack_mem_ptr);
      memset(&processes[iFound], 0, sizeof(proc_descriptor));
      sem_put(kal_proc_sem);
      trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
                              "*** Couldn't create Task stack !\n");
      error_log(ERROR_WARNING | RC_KAL_SYSERR);
      return((task_id_t)NULL);
   }

   /* Create the sleep semaphore for this task */
   sprintf(SemaphoreName, "SS%02x", iFound);
   processes[iFound].sleep_sem = sem_create(0, SemaphoreName);
   if(processes[iFound].sleep_sem == (sem_id_t)NULL)
   {
      /* Sleep semaphore could not be created ! */
      mem_free_ex(KAL_SYS_MEM_POOL_ID, (void*)processes[iFound].pstack_mem_ptr);
      memset(&processes[iFound], 0, sizeof(proc_descriptor));
      sem_put(kal_proc_sem);
      trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
                                 "*** Couldn't create Task sleep sem !\n");
      error_log(ERROR_WARNING | RC_KAL_SYSERR);
      return((task_id_t)NULL);
   }

   /* Fix up the thread priority, Kal style [0,239] */
   if(prio == 0)
   {
      byPriority = DEFAULT_PRIORITY;
   }
   else
   {
      byPriority = (u_int8)prio;
   }

   /* Tell N+ to create a new task for us */
   processes[iFound].entry_point = entry_point;
   processes[iFound].stack_size  = stack_size;
   processes[iFound].pEvent = 0;
   processes[iFound].free_events = 0xFFFF;
   processes[iFound].priority    = byPriority;
#ifdef KAL_DO_PRIORITY_INHERITANCE
   processes[iFound].priority_original
                                = byPriority;
   processes[iFound].priority_inherited
                                = 0;
   processes[iFound].inversion_protection_interest
                                = 0;
#endif /* KAL_DO_PRIORITY_INHERITANCE */
   /* Fill in the parameters to be passed */
   processes[iFound].dwArgs[0] = (u_int32)argc;
   processes[iFound].dwArgs[1] = (u_int32)argv;
   processes[iFound].dwArgs[2] = (u_int32)entry_point;
   processes[iFound].dwArgs[3] = (u_int32)iFound;

   /* Convert the KAL priority to N+ style.                        */

   /* NOTE: Nucleus+ Range is 0-255, with 0 being highest priority */
   /*       KAL Range is 1-239 with 239 being the highest priority */
   /*                      High Low                                */
   /*       Mapping:  N+    17  255                                */
   /*                 KAL  239    1                                */
   byPriority = 255 - byPriority + 1;


   /* All processes are wrapped to present a standard task interface to 
      Software created on top of this KAL */
   iRetcode = NU_Create_Task(&NU_Task_CtrlBlk[iFound], 
                              processes[iFound].name, 
                              (TASK_ENTRY_POINT)kal_process_wrapper, 
                              4, 
                              &(processes[iFound].dwArgs),
                              processes[iFound].pstack_mem_ptr, 
                              stack_size, 
                              (OPTION)byPriority,
                              KAL_TIME_SLICE,
                              byPreemptOption,
                              NU_START);
   if(iRetcode != NU_SUCCESS)
   {
      /* Task could not be created ! */
      sem_delete(processes[iFound].sleep_sem);
      mem_free_ex(KAL_SYS_MEM_POOL_ID, (void*)processes[iFound].pstack_mem_ptr);
      memset(&processes[iFound], 0, sizeof(proc_descriptor));
      sem_put(kal_proc_sem);

      /* Output the failure message */
#ifdef NU_DETAILED_DEBUG
      switch(iRetcode)
      {
         case NU_INVALID_TASK:
            /* Invalid Ctrl Blk ptr or Ctrl Blk already in use */
            trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
               "Task Create Ctrl Blk already in Use! Index=%x\n", iFound);
            error_log(ERROR_WARNING | RC_KAL_INUSE);
         break;

         case NU_INVALID_ENTRY:
            /* Task's Entry function pointer is NULL */
            trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
               "Task Create Entry function pointer NULL!\n");
            error_log(ERROR_WARNING | RC_KAL_INVALID);
         break;

         case NU_INVALID_MEMORY:
            /* Stack Address Ptr is NULL */
            trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
               "Task Create Stack Ptr NULL. Stack Ptr=%x\n",
                  (u_int32)processes[iFound].pstack_mem_ptr);
            error_log(ERROR_WARNING | RC_KAL_INVALID);
         break;

         case NU_INVALID_SIZE:
            /* Stack Size is Insufficient */
            trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
               "Task Create Stack Size Insufficient. Size=%x\n",
                  stack_size);
            error_log(ERROR_WARNING | RC_KAL_INVALID);
         break;

         case NU_INVALID_PRIORITY:
            /* Task Priority is Invalid */
            trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
               "Task Create Prioirty Invalid. KAL Prio=%x, N+ Prio=%x\n",
                  prio, byPriority);
            error_log(ERROR_WARNING | RC_KAL_INVALID);
         break;

         case NU_INVALID_PREEMPT:
            /* Task Preempt Parameter is Invalid */
            trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
               "Task Create Preempt Param Invalid.\n");
            error_log(ERROR_WARNING | RC_KAL_INVALID);
         break;

         case NU_INVALID_START:
            /* Task Auto Start Parameter is Invalid */
            trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
               "Task Create Auto Start Param Invalid.\n");
            error_log(ERROR_WARNING | RC_KAL_INVALID);
         break;

         default:
            /* Unknown N+ error */
            trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
               "Unknown N+ Task Create Error=%x!\n", iRetcode);
            error_log(ERROR_WARNING | RC_KAL_SYSERR);
      }
#else /* NU_DETAILED_DEBUG */
      trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, "Can't create Task!\n");
      error_log(ERROR_WARNING | RC_KAL_SYSERR);
#endif /* NU_DETAILED_DEBUG */

      return((task_id_t)NULL);
   }

   /* Created new Task */
   processes[iFound].psos_pid = (unsigned long)(&NU_Task_CtrlBlk[iFound]);

   /* Return the serialization semaphore */
   sem_put(kal_proc_sem);

   trace_new(TRACE_KAL | TRACE_LEVEL_2, 
      "N+ Task %s Created. Id:0x%08x, KAL pid:0x%08x\n", 
         processes[iFound].name, processes[iFound].psos_pid, 
            PID_FROM_INDEX(iFound));

   return(PID_FROM_INDEX(iFound));
}

/**********************************************************/
/* Return the process id of the process currently running */
/**********************************************************/
task_id_t task_id(void)
{
   int iLoop;
   NU_TASK *current_task_id;
   task_id_t current_kal_pid;
   #if (BACKFIT_RTOS_TASKS_INTO_KAL == YES)
   int iRetcode;
   CHAR current_task_name[8];
   DATA_ELEMENT current_task_status;
   UNSIGNED current_task_scheduled_count;
   OPTION current_task_priority, current_task_preempt;
   UNSIGNED current_task_time_slice, *current_task_stack_base;
   UNSIGNED current_task_stack_size, current_task_stack_minimum;
   char SemaphoreName[NUP_MAX_NAME_LENGTH];
   #endif /* (BACKFIT_RTOS_TASKS_INTO_KAL == YES) */

   not_from_critical_section();

   /*********************************************************/
   /* OpenTV calls this from interrupt so we need to ensure */
   /* that we can handle this gracefully!!!!                */
   /*********************************************************/

   /* Ask N+ for its current task pointer */
   current_task_id = NU_Current_Task_Pointer();

   /* Search our task descriptor table for this pid */
   if(InterruptsDisabled())
   {
      /* From pSoS kal */
      /**********************************************************************/
      /* Here's the nasty bit that pretty much does away with the           */
      /* effectiveness of the access semaphore (sorry). OpenTV frequently   */
      /* calls us with interrupts disabled. Sometimes (yes, I have seen it) */
      /* the semaphore is held by someone else and the sem_get fails if I   */
      /* use a timeout of 0. I tried just failing the call in these cases   */
      /* but this causes OpenTV to fall over in a big heap so I do this     */
      /* nasty thing instead....                                            */
      /**********************************************************************/
      iLoop = 0;
      while((iLoop < MAX_PROCESSES) && 
            (processes[iLoop].psos_pid != (u_int32)current_task_id))
      {
         iLoop++;
      }

      /* If we found the pid, return the KAL task id */
      if(processes[iLoop].psos_pid == (u_int32)current_task_id)
      {
         current_kal_pid = (task_id_t)PID_FROM_INDEX(iLoop);
      }
      else
      {
         isr_trace_new(TRACE_KAL | TRACE_LEVEL_3, 
            "Failed to find pSOS pid in KAL table!\n", 
               processes[iLoop].psos_pid, 0);
         isr_error_log(ERROR_WARNING | RC_KAL_SYSERR);
         current_kal_pid = 0;
      }
   }
   else
   {
      not_interrupt_safe();
      
      /* Get the serialization semaphore */
      sem_get(kal_proc_sem, KAL_WAIT_FOREVER);

      iLoop = 0;
      while((iLoop < MAX_PROCESSES) && 
            (processes[iLoop].psos_pid != (u_int32)current_task_id))
      {
         iLoop++;
      }

      /* If we found the pid, return the KAL task id */
      if(processes[iLoop].psos_pid != (u_int32)current_task_id)
      {
         sem_put(kal_proc_sem);

         #if (BACKFIT_RTOS_TASKS_INTO_KAL == NO)
         isr_trace_new(TRACE_KAL | TRACE_LEVEL_3, 
            "Failed to find N+ pid (%x) in KAL table!\n", 
               processes[iLoop].psos_pid, 0);
         isr_error_log(ERROR_WARNING | RC_KAL_SYSERR);
         #endif
         current_kal_pid = 0;
      }
      else
      {
         /* Get the KAL Id */
         current_kal_pid = (task_id_t)PID_FROM_INDEX(iLoop);
         /* Return the serializaton semaphore */
         sem_put(kal_proc_sem);
      }
   }

   #if (BACKFIT_RTOS_TASKS_INTO_KAL == YES)
   if(0==current_kal_pid)
   {
      /* Retrieve this task's info */
      iRetcode = NU_Task_Information(current_task_id, 
                                     current_task_name, 
                                     &current_task_status,
                                     &current_task_scheduled_count,
                                     &current_task_priority,
                                     &current_task_preempt,
                                     &current_task_time_slice,
                                     (VOID **)&current_task_stack_base,
                                     &current_task_stack_size,
                                     &current_task_stack_minimum);
      if(NU_SUCCESS != iRetcode)
      {
         /* Unable to get the process info */
         isr_trace_new(TRACE_KAL | TRACE_LEVEL_3, 
            "Failed to Get N+ task info. Retcode=%x!\n", 
               iRetcode, 0);
         error_log(ERROR_WARNING | RC_KAL_SYSERR);
         return((task_id_t)NULL);
      }

      if(!InterruptsDisabled())
      {
         /* Grab the serialisation semaphore */
         if(sem_get(kal_proc_sem, KAL_WAIT_FOREVER) != RC_OK)
         {
            /* Unable to get the process block protection semaphore */
            sem_put(kal_proc_sem);
            error_log(ERROR_WARNING | RC_KAL_SYSERR);
            return((task_id_t)NULL);
         }
      }

      /* Find a free descriptor. */
      iLoop = 0;
      while((iLoop < MAX_PROCESSES) &&
            (processes[iLoop].proc_flags & PROC_FLAG_STRUCT_IN_USE))
      {
         iLoop++;
      }

      /* Did we find one? */
      if(MAX_PROCESSES <= iLoop)
      {
         /* Error - no free descriptors were found */
         if(!InterruptsDisabled())
         {
            sem_put(kal_proc_sem);
         }
         isr_trace_new(TRACE_KAL | TRACE_LEVEL_3, 
            "KAL Pid table overflow. Index=%x! Current RTOS Id=%x\n", 
               iLoop, (u_int32)current_task_id);
         isr_error_log(ERROR_WARNING | RC_KAL_NORESOURCES);
         current_kal_pid = 0;
      }
      else
      {
         /* Clear out block */
         memset(&processes[iLoop], 0, sizeof(processes[iLoop]));

         /* Allocate new pid block and fill it */
         processes[iLoop].proc_flags = PROC_FLAG_STRUCT_IN_USE;

         /* Derive a task name from the descriptor index */
         /* Note: the prefix - NT denotes a native back fitted task */
         sprintf(processes[iLoop].name, "NT%02x", iLoop);

         /* Make sure the name is null terminated */
         processes[iLoop].name[NUP_MAX_NAME_LENGTH-1] = '\0';

         /* Create the sleep semaphore for this task */
         sprintf(SemaphoreName, "SS%02x", iLoop);
         processes[iLoop].sleep_sem = sem_create(0, SemaphoreName);
         if((sem_id_t)NULL == processes[iLoop].sleep_sem)
         {
            memset(&processes[iLoop], 0, sizeof(proc_descriptor));
            if(!InterruptsDisabled())
            {
               sem_put(kal_proc_sem);
            }
            trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
               "*** Couldn't create Task sleep sem for Native RTOS Task!\n");
            error_log(ERROR_WARNING | RC_KAL_SYSERR);
            return((task_id_t)NULL);
         }

         /* Fix up the thread priority, Kal style [0,239] */
         /* NOTE: Nucleus+ Range is 0-255, with 0 being highest priority */
         /*       KAL Range is 1-239 with 239 being the highest priority */
         /*                      High Low                                */
         /*       Mapping:  N+    17  255                                */
         /*                 KAL  239    1                                */
         processes[iLoop].priority = 255 - current_task_priority + 1;
         #ifdef KAL_DO_PRIORITY_INHERITANCE
         processes[iLoop].priority_original = processes[iLoop].priority;
         processes[iLoop].priority_inherited = 0;
         processes[iLoop].inversion_protection_interest = 0;
         #endif /* KAL_DO_PRIORITY_INHERITANCE */

         /* Fill in the other table entries */
         processes[iLoop].psos_pid = (unsigned long)current_task_id;
         processes[iLoop].entry_point = NULL;         /* We dont know what */
         processes[iLoop].dwArgs[0] = NULL;
         processes[iLoop].dwArgs[1] = NULL;
         processes[iLoop].dwArgs[2] = NULL;
         processes[iLoop].dwArgs[3] = NULL;
         processes[iLoop].pEvent = 0;
         processes[iLoop].free_events = 0xFFFF;
         /* No stack ptr */
         processes[iLoop].pstack_ptr = NULL;
         processes[iLoop].pstack_mem_ptr =  NULL;
         processes[iLoop].pstack_check = NULL;
         processes[iLoop].stack_size = NULL;

         if(!InterruptsDisabled())
         {
            /* Return the serialization semaphore */
            sem_put(kal_proc_sem);
         }

         trace_new(TRACE_KAL | TRACE_LEVEL_2, 
"KAL PID Entry Created for Native Rtos task:%s. Id:0x%08x, KAL name=%s, pid:0x%08x\n", 
               current_task_name, processes[iLoop].psos_pid, 
                  processes[iLoop].name, PID_FROM_INDEX(iLoop));

         /* Generate our KAL pid */
         current_kal_pid = PID_FROM_INDEX(iLoop);
      }
   }
   #endif /* (BACKFIT_RTOS_TASKS_INTO_KAL == YES) */

   /* Return what we have */
   return(current_kal_pid);
}

/***************************************/
/* Get a task ID from its name string. */
/***************************************/
int task_id_from_name(const char *name, task_id_t *pid)
{
   int iLoop;
   int retcode;

   not_from_critical_section();
   not_interrupt_safe();
   
   /* Check for bad input params */
   if((name == (char *)NULL) || (pid == (task_id_t *)NULL))
   {
      error_log(ERROR_WARNING | RC_KAL_BADPTR);
      return(RC_KAL_BADPTR);
   }

   /* Take the process serialisation semaphore. */
   if(sem_get(kal_proc_sem, KAL_WAIT_FOREVER) != RC_OK)
   {
      error_log(ERROR_WARNING | RC_KAL_SYSERR);
      return(RC_KAL_SYSERR);
   }

   /* Search our task descriptor table for this name string */
   iLoop = 0;
   while(iLoop < MAX_PROCESSES)
   {
      /* Only consider active task descriptors. */
      if((processes[iLoop].proc_flags & PROC_FLAG_STRUCT_IN_USE) &&
         (strncmp(processes[iLoop].name, name, NUP_MAX_NAME_LENGTH)==0))
      {
         break;
      }
      iLoop++;
   }

   /* Release the serialisation semaphore. */
   sem_put(kal_proc_sem);

   /* If we found the name, return the KAL task id */
   if(iLoop < MAX_PROCESSES)
   {
      *pid = (task_id_t)PID_FROM_INDEX(iLoop);
      retcode = RC_OK;
   }
   else
   {
      trace_new(TRACE_KAL | TRACE_LEVEL_3, 
         "Failed to find a task named %s in KAL table!\n", name);
      error_log(ERROR_WARNING | RC_KAL_SYSERR);
      retcode = RC_KAL_INVALID;
   }

   return(retcode);
}

/*******************************/
/* Destroy the calling thread. */
/*******************************/
void task_terminate( void )
{
   task_id_t dwOurPid, dwCleanTaskPid;
   unsigned char OurPriority;
   int iRetcode;

   not_from_critical_section();
   not_interrupt_safe();
   
   trace_new(TRACE_KAL | TRACE_LEVEL_2, "task_terminate\n");

   /* Find out who we are */
   dwOurPid = task_id();
   if(!IS_VALID_PID(dwOurPid))
   {
      /* Error ! - we can't get our own PID ! */
      error_log(ERROR_FATAL | RC_KAL_SYSERR);
      return;
   }

   /* Get the serialization semaphore */
   if(sem_get(kal_proc_sem, KAL_WAIT_FOREVER) != RC_OK)
   {
      error_log(ERROR_FATAL | RC_KAL_SYSERR);
      return;
   }

   /* Mark this task for deletion */
   processes[INDEX_FROM_PID(dwOurPid)].proc_flags |= PROC_FLAG_DELETE_TASK;

   /* Return the serialization semaphore */
   sem_put(kal_proc_sem);

   /* Reduce the task priority to below the Cleanup Task priority */
   OurPriority = task_priority(dwOurPid, 0);
   task_priority(dwOurPid, TASK_MARKED_DELETE_PRIORITY - OurPriority);

   /* Store our Id in the Global Delete Tid var */
   sem_get(kal_del_sem, KAL_WAIT_FOREVER);
   gDeleteTaskId = dwOurPid;
   sem_put(kal_del_sem);

   /* Get the Cleanup Task's Id */
   iRetcode = task_id_from_name(CLEN_TASK_NAME, &dwCleanTaskPid);
   if(iRetcode != RC_OK)
   {
      /* Error ! - we can't get the cleanup task's PID ! */
      error_log(ERROR_FATAL | RC_KAL_SYSERR);
      return;
   }

   /* Resume the cleanup task and let it kill us off */
   iRetcode = task_resume(dwCleanTaskPid);
   if(iRetcode != RC_OK)
   {
      /* Error resuming the Clean Up task */
      error_log(ERROR_FATAL | RC_KAL_SYSERR);
      return;
   }

   /* Put ourselves to sleep permanently and wait for the cleanup task to 
      finish us off. */
   do
   {
      task_suspend(dwOurPid);
   }
   while(1);  /* Dont let the task get away! :) */
}

/********************************************************/
/* Kill another task without even asking its permission */
/********************************************************/

/* WARNING: This function may leave the target task's resources */
/* allocated. It should only be used in cases where the task is */
/* known to have allocated no resources or when other signaling */
/* has been used to inform the task to release those resources  */
/* before killing it.                                           */
int task_destroy(task_id_t pid)
{
   int iIndex;
   int iRetcode;
   task_id_t dwOurPid;
   sem_id_t task_sleep_sem;
   NU_TASK *tid;
   NU_EVENT_GROUP *pTaskEvent;
   void *pTaskStack;

   not_from_critical_section();
   not_interrupt_safe();

   /* If we are killing ourselves off, just call task_terminate */
   dwOurPid = task_id();
   if(pid == dwOurPid)
   {
      task_terminate();
      /* If the function returned, the task ID must be invalid */
      /* (which is unlikely given we just queried it)          */
      return(RC_KAL_INVALID);
   }

   /* Get the serialization semaphore */
   if(sem_get(kal_proc_sem, KAL_WAIT_FOREVER) != RC_OK)
   {
      error_log(ERROR_WARNING | RC_KAL_SYSERR);
      return(RC_KAL_SYSERR);
   }

   /* Is the pid valid ? */
   iIndex = INDEX_FROM_PID(pid);
   if(!((IS_VALID_PID(pid)) &&
        (processes[iIndex].proc_flags & PROC_FLAG_STRUCT_IN_USE)))
   {
      sem_put(kal_proc_sem);
      trace_new(TRACE_KAL | TRACE_LEVEL_3, 
         "Invalid PID 0x%08x passed to task_destroy\n", pid);
      error_log(ERROR_WARNING | RC_KAL_INVALID);
      return(RC_KAL_INVALID);
   }

   /* Get info on this process from the process descriptor */
   /* Get the Task's Id */
   tid = (NU_TASK *)processes[iIndex].psos_pid;
   /* Get the task's sleep semaphore */
   task_sleep_sem = processes[iIndex].sleep_sem;
   /* Get the task's stack memory */
   pTaskStack = processes[iIndex].pstack_mem_ptr;
   /* Get the task's event group, if it has one */
   pTaskEvent = processes[iIndex].pEvent;
   /* Clear the process structure */
   memset(&processes[INDEX_FROM_PID(pid)], 0, sizeof(proc_descriptor));
   sem_put(kal_proc_sem);

   /* Terminate the task */
   iRetcode = NU_Terminate_Task(tid);
   if(iRetcode != NU_SUCCESS)
   {
      trace_new(TRACE_KAL | TRACE_LEVEL_3, 
         "Error Terminating N+ Task. Kal Pid:%x\n", pid);
      error_log(ERROR_FATAL | RC_KAL_SYSERR);
      return(RC_KAL_SYSERR);
   }

   /* Delete the task */
   iRetcode = NU_Delete_Task(tid);
   if(iRetcode != NU_SUCCESS)
   {
      trace_new(TRACE_KAL | TRACE_LEVEL_3, 
         "Error Destroying N+ Task. Kal Pid=%x\n", pid);
      error_log(ERROR_FATAL | RC_KAL_SYSERR);
      return(RC_KAL_SYSERR);
   }

   /* Free the task's resources */
   /* Delete it's sleep semapore */
   sem_delete(task_sleep_sem);
   /* Free it's stack memory */
   mem_free_ex(KAL_SYS_MEM_POOL_ID, pTaskStack);
   /* Free it's event group if it has one */
   if(pTaskEvent)
   {
      NU_Delete_Event_Group(pTaskEvent);
      memset(pTaskEvent, 0, sizeof(NU_EVENT_GROUP));
   }

   return(RC_OK);
}

/***********************************************************************/
/* Place the current thread in the sleeping queue and wake it up again */
/* after a given time period.                                          */
/***********************************************************************/
void task_time_sleep(unsigned int ms)
{
   not_from_critical_section();
   not_interrupt_safe();
   trace_new(TRACE_KAL | TRACE_LEVEL_2, "task_time_sleep\n");
   time_sleep(ms);
}

/***********************************************************************/
/* Place the current thread in the sleeping queue and wake it up again */
/* after a given time period.                                          */
/* The "_ex" is that it returns a return code                          */
/* RC_OK if it was awoken, RC_KAL_TIMEOUT if specified timeout occured */
/***********************************************************************/
int task_time_sleep_ex(unsigned int ms)
{
   not_from_critical_section();
   not_interrupt_safe();
   trace_new(TRACE_KAL | TRACE_LEVEL_2, "task_time_sleep\n");
   return(time_sleep(ms));
}

/************************************************************/
/* Implement task_sleep and task_time_sleep functions       */
/************************************************************/
int time_sleep(unsigned int ms)
{
   task_id_t dwOurPid;
   int iIndex;
   int iRetcode;
   sem_id_t task_sleep_sem;

   not_from_critical_section();
   not_interrupt_safe();

   /* Find out who we are */
   dwOurPid = task_id();

   /* Get the serialization semaphore */
   if(sem_get(kal_proc_sem, KAL_WAIT_FOREVER) != RC_OK)
   {
      error_log(ERROR_WARNING | RC_KAL_SYSERR);
      return(RC_KAL_SYSERR);
   }

   if((IS_VALID_PID(dwOurPid)) &&
      (processes[INDEX_FROM_PID(dwOurPid)].proc_flags & PROC_FLAG_STRUCT_IN_USE))
   {
      iIndex = INDEX_FROM_PID(dwOurPid);
      if(!(processes[iIndex].proc_flags & PROC_FLAG_SLEEPING))
      {
         /* Get ready to start sleeping. Update Flags to indicate state */
         processes[iIndex].proc_flags |= PROC_FLAG_SLEEPING;
         task_sleep_sem = processes[iIndex].sleep_sem;

         sem_put(kal_proc_sem);

         /* Sleep for a while by waiting on the sleep sem */
         iRetcode = sem_get(task_sleep_sem, ms);
         if((iRetcode != RC_OK) && (iRetcode != RC_KAL_TIMEOUT))
         {
            /* Error ! sem_wait failed. What do I do ? */
            error_log(ERROR_WARNING | RC_KAL_SYSERR);
            iRetcode = RC_KAL_SYSERR;
         }

         /* Even in the error cases, we need to tidy up... */

         /* Process is no longer sleeping. Clear relevant flag */
         sem_get(kal_proc_sem, KAL_WAIT_FOREVER);
         processes[iIndex].proc_flags &= ~PROC_FLAG_SLEEPING;

         /* Also clear the wakeup mask (required by spec) */
         processes[iIndex].proc_flags &= ~PROC_FLAG_WAKEUP_MASK;
         sem_put(kal_proc_sem);
      }
      else
      {
         /* Error ! Task already marked as asleep. Handling ? */
         sem_put(kal_proc_sem);
         error_log(ERROR_WARNING | RC_KAL_ASLEEP);
         iRetcode = RC_KAL_ASLEEP;
      }
   }
   else
   {
      /* Error ! - we can't get our own PID ! */
      sem_put(kal_proc_sem);
      error_log(ERROR_FATAL | RC_KAL_SYSERR);
      iRetcode = RC_KAL_SYSERR;
   }

   return iRetcode;
}

/**********************************************************************/
/* Move a given process to the active queue and resumes its execution */
/**********************************************************************/
void task_wakeup(task_id_t pid)
{
   int iRetcode;
   int iIndex;

   not_from_critical_section();

   isr_trace_new(TRACE_KAL | TRACE_LEVEL_2, "task_wakeup\n", 0, 0);

   iIndex = INDEX_FROM_PID(pid);
   if((IS_VALID_PID(pid)) && 
      (processes[iIndex].proc_flags & PROC_FLAG_STRUCT_IN_USE))
   {
      /* If the task is already sleeping or is awake but has the wakeup */
      /* events mask set, send the wakeup event, otherwise just return  */
      if((processes[iIndex].proc_flags & PROC_FLAG_SLEEPING) ||
         (!(processes[iIndex].proc_flags & PROC_FLAG_SLEEPING) &&
           (processes[iIndex].proc_flags & PROC_FLAG_WAKEUP_MASK)))
      {
         /* Task sleeping or wakeup mask is set so send a wakeup event */

         /* Ensure semaphore count never goes above 1 - we are trying */
         /* to emulate a 1-bit event using semaphores here.           */
         iRetcode = sem_get(processes[iIndex].sleep_sem, 0);
         sem_put(processes[iIndex].sleep_sem);
      }
   }
   else
   {
      /* Error ! Invalid PID passed. */
      isr_error_log(ERROR_WARNING | RC_KAL_INVALID);
   }

   return;
}

/****************************************/
/* Set or clear the process wakeup mask */
/****************************************/
void task_mask(bool on_off)
{
   unsigned int dwPid;
   int iIndex;

   not_from_critical_section();
   not_interrupt_safe();
   
   trace_new(TRACE_KAL | TRACE_LEVEL_2, "task_mask\n");

   dwPid = task_id();

   /* Get the serialization semaphore */
   if(sem_get(kal_proc_sem, KAL_WAIT_FOREVER) != RC_OK)
   {
      error_log(ERROR_WARNING | RC_KAL_SYSERR);
      return;
   }

   iIndex = INDEX_FROM_PID(dwPid);
   if((IS_VALID_PID(dwPid)) &&
      (processes[iIndex].proc_flags & PROC_FLAG_STRUCT_IN_USE))
   {
      processes[iIndex].proc_flags = 
                     (processes[iIndex].proc_flags & ~PROC_FLAG_WAKEUP_MASK)
                               | (on_off ? PROC_FLAG_WAKEUP_MASK : 0);
      sem_put(kal_proc_sem);
   }
   else
   {
      /* Error - task_id returned an invalid PID ! */
      sem_put(kal_proc_sem);
      error_log(ERROR_WARNING | RC_KAL_INVALID);
   }

   return;
}

/****************************************/
/* Set or clear the process wakeup mask */
/****************************************/
bool task_get_mask(void)
{
   unsigned int dwPid;
   int iIndex;
   bool bMask;

   not_from_critical_section();
   not_interrupt_safe();
   
   trace_new(TRACE_KAL | TRACE_LEVEL_2, "task_get_mask\n");

   dwPid = task_id();

   /* Get the serialization semaphore */
   if(sem_get(kal_proc_sem, KAL_WAIT_FOREVER) != RC_OK)
   {
      error_log(ERROR_WARNING | RC_KAL_SYSERR);
   }

   iIndex = INDEX_FROM_PID(dwPid);
   if((IS_VALID_PID(dwPid)) &&
      (processes[iIndex].proc_flags & PROC_FLAG_STRUCT_IN_USE))
   {
      bMask = ((processes[iIndex].proc_flags & PROC_FLAG_WAKEUP_MASK) 
                                                      ? TRUE : FALSE);
      sem_put(kal_proc_sem);
   }
   else
   {
      /* Error - task_id returned an invalid PID ! */
      sem_put(kal_proc_sem);
      error_log(ERROR_WARNING | RC_KAL_INVALID);
   }

   return bMask;
}

/*************************************/
/* Set a thread's execution priority */
/*************************************/
unsigned char task_priority (task_id_t pid, signed short delta)
{
   int      iIndex;
   OPTION   iOldPriority;
   int      iPriority;
   NU_TASK  *task_rtos_id;

   not_from_critical_section();
   not_interrupt_safe();
  
   trace_new(TRACE_KAL | TRACE_LEVEL_2, "task_priority\n");

   /* Get the serialization semaphore */
   if(sem_get(kal_proc_sem, KAL_WAIT_FOREVER) != RC_OK)
   {
      error_log(ERROR_WARNING | RC_KAL_SYSERR);
      return((unsigned char)NULL);
   }

   /* Has a valid task ID been passed ? */
   iIndex = INDEX_FROM_PID(pid);
   if(!(IS_VALID_PID(pid) &&
       (processes[iIndex].proc_flags & PROC_FLAG_STRUCT_IN_USE)))
   {
      sem_put(kal_proc_sem);
      error_log(ERROR_FATAL | RC_KAL_INVALID);
   }
   else
   {
#ifdef KAL_DO_PRIORITY_INHERITANCE

      /******************************************************************/
      /* The proc struct's "priority_original" variable reflects how the*/
      /* task's priority would be if not for priority inheritance. While*/
      /* Inheritance is in play, this variable keeps track of the task's*/
      /* priority from when the task was created, during owning         */
      /* Inversion Protected Objects (and consequently having to        */
      /* maintain a floor at an inherited priority) and also inbetween  */
      /* holding Priority Inversion protected Objects                   */
      /******************************************************************/

      /* Store the current Original Priority. */
      iOldPriority = processes[iIndex].priority_original;

      /* Update the Original Priority */
      /* New priority is old priority + delta passed to function */
      iPriority = (int)processes[iIndex].priority_original + (int)delta;
      /* Ensure new priority is inside valid range.*/
      iPriority = max(iPriority, 1);
      iPriority = min(iPriority, 239);
      /* Update the proc struct */
      processes[iIndex].priority_original = (unsigned char)iPriority;

      /* See if the task's priority has to be changed */
      if(processes[iIndex].priority > iOldPriority)
      {

#ifdef DEBUG_PRIORITY_INHERITANCE
         /* If not at orignal priority, task has to be at Inherited priority.
            If the priority is different from these two values, how did we 
            arrive at this priority?
         */
         if(processes[iIndex].priority != 
                                 processes[iIndex].priority_inherited)
         {
            /* How did we get this priority ? */
            isr_trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS,
               "Error! Changing Priority for Task:%x. Inherited Prio=%x",
                  pid, processes[iIndex].priority_inherited);
            isr_trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS,
               "Old Original Prio:%x. Running Prio=%x\n",
                  iOldPriority, processes[iIndex].priority);
            isr_error_log(ERROR_WARNING | RC_KAL_SYSERR);
         }
#endif

         /*****************************************************************/
         /* An Inherited priority is in use. Dont change the task priority*/
         /* now. Task has to be maintained above the Inherited Priority   */
         /* for the duration on "locking" shared Objects. Task will be    */
         /* changed to desired priority (saved in orignal_priority) after */
         /* it releases ALL it's shared Objects                           */
         /*****************************************************************/

         return((unsigned char)iOldPriority);
      }

#ifdef DEBUG_PRIORITY_INHERITANCE
      else
      {
         /* If not at Inherited priority, task has to be at orignal priority.
            If the priority is different from these two values, how did we 
            arrive at this priority?
         */
         if(processes[iIndex].priority != iOldPriority)
         {
            /* How did we get this priority ? */
            isr_trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS,
               "Error! Changing Priority for Task:%x. Inherited Prio=%x",
                  pid, processes[iIndex].priority_inherited);
            isr_trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS,
               "Original Prio:%x. Running Prio=%x\n",
                  processes[iIndex].priority_original, 
                     processes[iIndex].priority);
            isr_error_log(ERROR_WARNING | RC_KAL_SYSERR);
         }
      }
#endif /* DEBUG_PRIORITY_INHERITANCE */

#endif /* KAL_DO_PRIORITY_INHERITANCE */

      /* Change the task's priority */

      /* NOTE: Nucleus+ Range is 0-255, with 0 being highest priority */
      /*       KAL Range is 1-239 with 239 being the highest priority */
      /*                      High Low                                */
      /*       Mapping:  N+    17  255                                */
      /*                 KAL  239    1                                */

      /* New (KAL) priority is old priority + delta passed to function */
      iPriority = (int)processes[iIndex].priority + (int)delta;

      /*trace("task_priority: old Kal %d, delta %d, new KAL task pri=%d\n"*/
      /* (int)processes[iIndex].priority,delta,iPriority); */

      /* Ensure new priority is inside valid (KAL) range.*/
      iPriority = max(iPriority, 1); 
      iPriority = min(iPriority, 239);

      /*trace("task_priority: update (bounded) KAL task pri to %d\n",iPriority);*/

      /* Update the process struct */
      processes[iIndex].priority = (unsigned char)iPriority; 

      /* Get the task RTOS Id */
      task_rtos_id = (NU_TASK *)processes[iIndex].psos_pid;
      sem_put(kal_proc_sem);

      /* Convert the priority to N+ Style */
      iPriority = 255 - iPriority + 1;

      /* trace("task_priority: N+ equiv pri=%d\n",iPriority); */

      /* Change the Task's priority */
      iOldPriority = NU_Change_Priority(task_rtos_id, (OPTION)iPriority);

      /* trace("task_priority: NU_Change_Priority returns old N+ pri=%d\n",iOldPriority); */

      /* Convert the priority to KAL Style */
      iPriority = 255 - iPriority + 1;

      /* trace("task_priority: return old KAL pri=%d and exit\n",iPriority); */

      return((unsigned char)iPriority);

   }

   return((unsigned char)NULL);
}

/*******************/
/* Suspend a task. */
/*******************/
int task_suspend(task_id_t pid)
{
   int iIndex;
   int iRetcode = RC_OK;

   not_from_critical_section();
   not_interrupt_safe();
   
   /* Get the serialization semaphore */
   if(sem_get(kal_proc_sem, KAL_WAIT_FOREVER) != RC_OK)
   {
      error_log(ERROR_WARNING | RC_KAL_SYSERR);
      return(RC_KAL_SYSERR);
   }

   /* Has a valid task ID been passed ? */
   iIndex = INDEX_FROM_PID(pid);
   if((IS_VALID_PID(pid)) &&
      (processes[iIndex].proc_flags & PROC_FLAG_STRUCT_IN_USE))
   {
      /* Has task already been suspended? */
      if(!(processes[iIndex].proc_flags & PROC_FLAG_SUSPENDED))
      {
         /* Indicate that the task has been suspended */
         processes[iIndex].proc_flags |= PROC_FLAG_SUSPENDED;
         /* Return the serialization semaphore */
         sem_put(kal_proc_sem);

         iRetcode = NU_Suspend_Task((NU_TASK *)processes[iIndex].psos_pid);
         if(iRetcode != NU_SUCCESS)
         {
            /* Task was not suspened. Undo previous suspension indication */
            if(sem_get(kal_proc_sem, KAL_WAIT_FOREVER) != RC_OK)
            {
               error_log(ERROR_WARNING | RC_KAL_SYSERR);
            }
            processes[iIndex].proc_flags &= ~PROC_FLAG_SUSPENDED;
            sem_put(kal_proc_sem);
            error_log(ERROR_WARNING | RC_KAL_SYSERR);
            iRetcode = RC_KAL_SYSERR;
         }
      }
      else
      {
      /* Return the serialization semaphore */
      sem_put(kal_proc_sem);

      iRetcode = RC_KAL_SUSPENDED;
      }
            
   }
   else
   {
      /* Return the serialization semaphore */
      sem_put(kal_proc_sem);
      error_log(ERROR_WARNING | RC_KAL_INVALID);
      iRetcode = RC_KAL_INVALID;
   }

   return(iRetcode);
}

/******************/
/* Resume a task. */
/******************/
int task_resume(task_id_t pid)
{
   int iIndex;
   int iRetcode;

   not_from_critical_section();
   not_interrupt_safe();
   
   /* Get the serialization semaphore */
   if(sem_get(kal_proc_sem, KAL_WAIT_FOREVER) != RC_OK)
   {
      error_log(ERROR_WARNING | RC_KAL_SYSERR);
      return(RC_KAL_SYSERR);
   }

   /* Has a valid task ID been passed ? */
   iIndex = INDEX_FROM_PID(pid);
   if(!((IS_VALID_PID(pid)) &&
       (processes[iIndex].proc_flags & PROC_FLAG_STRUCT_IN_USE)))
   {
      sem_put(kal_proc_sem);
      error_log(ERROR_WARNING | RC_KAL_INVALID);
      iRetcode = RC_KAL_INVALID;
   }
   else
   {
      /* Has task been suspended? */
      //if(processes[iIndex].proc_flags&PROC_FLAG_SUSPENDED)
      if (1)
      {
         /* Clear the Suspended indication */
         processes[iIndex].proc_flags &= ~PROC_FLAG_SUSPENDED;
         /* Return the serialization semaphore */
         sem_put(kal_proc_sem);

         iRetcode = NU_Resume_Task((NU_TASK *)processes[iIndex].psos_pid);
         if (iRetcode != NU_SUCCESS)
         {
            /* Task was not Reset. Undo previous updation. */
            if(sem_get(kal_proc_sem, KAL_WAIT_FOREVER) != RC_OK)
            {
               error_log(ERROR_WARNING | RC_KAL_SYSERR);
            }
            processes[iIndex].proc_flags |= PROC_FLAG_SUSPENDED;
            sem_put(kal_proc_sem);
            error_log(ERROR_WARNING | RC_KAL_SYSERR);
            iRetcode = RC_KAL_SYSERR;
         }
      }
      else{
         /* Return the serialization semaphore */
         sem_put(kal_proc_sem);
         /* Error! Trying to resume a running task */
         error_log(ERROR_WARNING | RC_KAL_INUSE);
         iRetcode = RC_KAL_INUSE;
      }
   }

   return(iRetcode);
}

/************************************************/
/* Save a pointer to this task's private memory */
/************************************************/
void* task_set_pointer(task_id_t dwPid, void *ptr)
{
   int   iIndex;
   void  *pOldPtr;

   not_from_critical_section();
   not_interrupt_safe();

   trace_new(TRACE_KAL | TRACE_LEVEL_2, "private_memory_attach\n");

   /* Get the serialization semaphore */
   if(sem_get(kal_proc_sem, KAL_WAIT_FOREVER) != RC_OK)
   {
      error_log(ERROR_WARNING | RC_KAL_SYSERR);
      return((void *)NULL);
   }

   iIndex = INDEX_FROM_PID(dwPid);
   if((IS_VALID_PID(dwPid)) &&
      (processes[iIndex].proc_flags & PROC_FLAG_STRUCT_IN_USE))
   {
      pOldPtr = processes[iIndex].private_mem;
      processes[iIndex].private_mem = ptr;
      sem_put(kal_proc_sem);
      return(pOldPtr);
   }
   else
   {
      sem_put(kal_proc_sem);
      trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
         "Invalid pid passed: 0x%08lx\n", dwPid);
      error_log(ERROR_WARNING | RC_KAL_INVALID);
      return((void *)NULL);
   }
}

/******************************************/
/* Retrieve a task private memory pointer */
/******************************************/
void* task_get_pointer(task_id_t task_id)
{
   int iIndex;
   void *mem_ptr;

   not_from_critical_section();

   isr_trace_new(TRACE_KAL | TRACE_LEVEL_1, "task_get_pointer\n", 0, 0);

   iIndex = INDEX_FROM_PID(task_id);
   if((IS_VALID_PID(task_id)) &&
      (processes[iIndex].proc_flags & PROC_FLAG_STRUCT_IN_USE))
   {
      mem_ptr = processes[iIndex].private_mem;
      return(mem_ptr);
   }
   else
   {
      isr_trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
         "Invalid PID passed: 0x%08lx\n", task_id, 0);
      isr_error_log(ERROR_WARNING | RC_KAL_INVALID);
      return((void *)NULL);
   }
}

/*************************/
/*************************/
/** Semaphore Functions **/
/*************************/
/*************************/

/*************************************************/
/* Create a semaphore and sets its initial value */
/*************************************************/
sem_id_t sem_create(unsigned int initial_value, const char *name)
{
   return(sem_create_ex(initial_value, name, KAL_TASK_ORDER_PRIORITY));
}

sem_id_t sem_create_ex(unsigned int initial_value, 
                       const char *name, 
                       task_wait_order_t wait_order)
{
   int iRetcode;
   bool ks;
   sem_id_t dummy_id;
   CHAR sem_name[NUP_MAX_NAME_LENGTH];
   OPTION task_suspend_type;
   NU_SEMAPHORE *pSemCtrlBlk;
   bool valid = FALSE;

   not_from_critical_section();
   not_interrupt_safe();

   //trace_new(TRACE_KAL | TRACE_LEVEL_1, "sem_create\n");

   /* If caller did not specifiy a name, make one up. */
   if(name == NULL)
   {
      ks = critical_section_begin();
      sprintf(sem_name, "S%03x", SemNameCount++);
      critical_section_end(ks);
   }
   else
   {
      /* make sure the name is unique */
      if(sem_id_from_name(name, &dummy_id) != RC_KAL_INVALID)
      {
         trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
            "sem_create. Name error!\n");
         error_log(ERROR_WARNING | RC_KAL_SYSERR);
         return((sem_id_t)NULL);
      }

      /* Copy the name */
      strncpy(sem_name, name, NUP_MAX_NAME_LENGTH);
   }

   /* Make sure string is null terminated */
   sem_name[NUP_MAX_NAME_LENGTH-1] = '\0';

   /* Translate task wait order. */
   switch(wait_order)
   {
      case KAL_TASK_ORDER_FIFO:
         task_suspend_type = NU_FIFO;
      break;

      case KAL_TASK_ORDER_PRIORITY:
         task_suspend_type = NU_PRIORITY;
      break;

      default:
         /* Unknown Kal Priority */
         trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
            "Invalid task wait type passed to sem_create_ex(). Type=%d.\n",
               wait_order);
         error_log(ERROR_FATAL | RC_KAL_INVALID);
         return((sem_id_t)NULL);
   }

   /* Get memory to store the Sem Control Block */
   iRetcode = NU_Allocate_Partition(&gSysSemCtrlBlkMemory,
                                    (VOID **)&pSemCtrlBlk,
                                    NU_NO_SUSPEND);
   if(iRetcode != RC_OK)
   {
      trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
         "sem_create. Ctrl Blk Allocate error!\n");
      error_log(ERROR_WARNING | RC_KAL_SYSERR);
      return((sem_id_t)NULL);
   }

   /* Has this block already been allocated? */
   sem_is_valid((sem_id_t)pSemCtrlBlk, &valid);
   if(valid)
   {
      if(NU_Deallocate_Partition((void *)pSemCtrlBlk) != NU_SUCCESS)
      {
         error_log(ERROR_WARNING | RC_KAL_SYSERR);
      }
      trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
         "Sem Ctrl block:%x already allocated!\n", pSemCtrlBlk);
      error_log(ERROR_FATAL | RC_KAL_SYSERR);
      return((sem_id_t)NULL);
   }
   else
   {
      memset(pSemCtrlBlk, 0, sizeof(NU_SEMAPHORE));
   }

   /* Create the new semaphore */
   iRetcode = NU_Create_Semaphore(pSemCtrlBlk,
                                  sem_name,
                                  (UNSIGNED)initial_value, 
                                  (OPTION)task_suspend_type);
   if(iRetcode != NU_SUCCESS)
   {
      /* Error ! Can't create RTOS semaphore */
      memset(pSemCtrlBlk, 0, sizeof(NU_SEMAPHORE));
      if(NU_Deallocate_Partition((void *)pSemCtrlBlk) != NU_SUCCESS)
      {
         error_log(ERROR_WARNING | RC_KAL_SYSERR);
      }

      /* Output the failure message */
#ifdef NU_DETAILED_DEBUG
      switch(iRetcode)
      {
         case NU_INVALID_SEMAPHORE:
            /* Invalid Ctrl Blk ptr or Ctrl Blk already in use */
            trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
               "Sem Create Ctrl Blk:%x already in Use! \n", pSemCtrlBlk);
            error_log(ERROR_WARNING | RC_KAL_INUSE);
         break;

         case NU_INVALID_SUSPEND:
            /* Suspend Parameter Invalid */
            trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
               "Sem Create Suspend Type not Valid. Kal Type=%d, N+ Type=%d! \n",
                  wait_order, task_suspend_type);
            error_log(ERROR_WARNING | RC_KAL_INVALID);
         break;

         default:
            /* Unknown N+ error */
            trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
               "Unknown N+ Sem Create Error=%x!\n", iRetcode);
            error_log(ERROR_WARNING | RC_KAL_SYSERR);
      }
#else /* NU_DETAILED_DEBUG */
      trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, "Can't create Semaphore!\n");
      error_log(ERROR_WARNING | RC_KAL_SYSERR);
#endif /* NU_DETAILED_DEBUG */

      return((sem_id_t)NULL);
   }

   /* Update the Semaphore Created Counter */
   gSemsCreated++;

   trace_new(TRACE_KAL | TRACE_LEVEL_2, 
      "N+ Semaphore %s Created. Id:0x%08x\n", sem_name, pSemCtrlBlk);

   return((sem_id_t)pSemCtrlBlk);
}

/***************************************************************/
/* Does the sem_id refer to a valid semaphore currently in use */
/***************************************************************/
int sem_is_valid(sem_id_t sem_id, bool *valid)
{
   static NU_SEMAPHORE *sem_ptr[MAX_SEMAPHORES
   #ifdef DRIVER_INCL_KALEXINC
                                       +MAX_MUTEXES
   #endif /* DRIVER_INCL_KALEXINC */
                                       ];
   UNSIGNED num_sem_ptr;

   not_from_critical_section();
   not_interrupt_safe();

   if(valid == (bool*)NULL)
   {
      error_log(ERROR_WARNING | RC_KAL_BADPTR);
      return(RC_KAL_BADPTR);
   }

   /* Get a list of the established semaphores */
   num_sem_ptr = NU_Semaphore_Pointers(sem_ptr, sizeof(sem_ptr));
   if(num_sem_ptr == 0)
   {
      trace_new(TRACE_KAL | TRACE_LEVEL_3, 
         "Failed to find any semaphore Ptrs!\n");
      error_log(ERROR_WARNING | RC_KAL_INVALID);
      return(RC_KAL_INVALID);
   }

   /* Search for the given sem_id */
   do
   {
      if(sem_ptr[num_sem_ptr-1] == (NU_SEMAPHORE *)sem_id)
      {
         break;
      }
   }
   while(--num_sem_ptr);

   if(num_sem_ptr == 0)
   {
      *valid = FALSE;
   }
   else
   {
      *valid = TRUE;
   }

   return(RC_OK);
}

/********************************/
/* Delete an existing semaphore */
/********************************/
int sem_delete(sem_id_t sem_id)
{
   STATUS iRetcode;

   not_from_critical_section();
   not_interrupt_safe();
   
   trace_new(TRACE_KAL | TRACE_LEVEL_2, "sem_delete\n");

   /* Delete the Semaphore */
   iRetcode = NU_Delete_Semaphore((NU_SEMAPHORE *)sem_id);
   if(iRetcode != NU_SUCCESS)
   {

#ifdef NU_DETAILED_DEBUG
      switch(iRetcode)
      {
         case NU_INVALID_SEMAPHORE:
            /* Sem Id is Invalid. */
            trace_new(TRACE_KAL | TRACE_LEVEL_2, 
               "Invalid Delete Sem Id=%d.\n", sem_id);
            error_log(ERROR_WARNING | RC_KAL_INVALID);
            iRetcode = RC_KAL_INVALID;
         break;

         default:
            /* Unknown N+ Error */
            trace_new(TRACE_KAL | TRACE_LEVEL_2, 
               "Delete Sem. Unknown N+ Error=%x\n", iRetcode);
            error_log(ERROR_WARNING | RC_KAL_SYSERR);
            iRetcode = RC_KAL_SYSERR;
      }
#else /* NU_DETAILED_DEBUG */
      /* Error ! Can't delete RTOS semaphore */
      trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
         "Can't delete Semaphore!\n");
      error_log(ERROR_WARNING | RC_KAL_SYSERR);
      iRetcode = RC_KAL_SYSERR;
#endif
   }
   else
   {
      /* Update Created Semaphore count */
      gSemsCreated--;

      /* Semaphore deleted. Release the Ctrl Blk. */
      memset((void *)sem_id, 0, sizeof(NU_SEMAPHORE));
      if(NU_Deallocate_Partition((void *)sem_id) != NU_SUCCESS)
      {
         error_log(ERROR_WARNING | RC_KAL_SYSERR);
      }
      trace_new(TRACE_KAL | TRACE_LEVEL_2, 
         "Deleted Semaphore. Id=%x\n", sem_id);
   }

   return(iRetcode);
}

/***********************************************************************/
/* Get a token for the given semaphore or sleep until one is available */
/***********************************************************************/
int sem_get(sem_id_t sem_id, u_int32 timeout_ms)
{
   STATUS iRetcode;
   UNSIGNED dwTimeoutTicks;
   
   not_from_critical_section();

   #ifdef DEBUG
   /* It is not safe to call this function with anything other than a 0 */
   /* timeout if calling from interrupt context!                        */
   if(timeout_ms != 0)
   {
     not_interrupt_safe();
   }
   #endif
   
   /* Translate the suspend parameter */
   if(timeout_ms == KAL_WAIT_FOREVER)
   {
      dwTimeoutTicks = NU_SUSPEND;
   }
   else if(timeout_ms == 0)
   {
      dwTimeoutTicks = NU_NO_SUSPEND;
   }
   else
   {
      /* Convert the specified time (milliSecs) to equivalent ticks */
      dwTimeoutTicks = (timeout_ms * KAL_TICKS2SEC)/1000;
      if(dwTimeoutTicks == 0)
      {
         dwTimeoutTicks++;
      }
   }

   /* Make the system call */
   iRetcode = NU_Obtain_Semaphore((NU_SEMAPHORE *)sem_id, dwTimeoutTicks);
   switch(iRetcode)
   {
      case NU_SUCCESS:
      break;

      case NU_TIMEOUT: /* Not an Error */
         /* Task timed out */
         iRetcode = RC_KAL_TIMEOUT;
      break;

      case NU_UNAVAILABLE: /* Not an Error */
         /* Sema is Unavailable */
         iRetcode = RC_KAL_NORESOURCES;
      break;

      case NU_INVALID_SEMAPHORE:
         /* Invalid Semaphore Id */
         /* modified to match the API */
         iRetcode = RC_KAL_SYSERR;
#ifdef NU_DETAILED_DEBUG
      isr_error_log(ERROR_WARNING | RC_KAL_INVALID);
#endif /* NU_DETAILED_DEBUG */
      break;

      case NU_SEMAPHORE_DELETED:
         /* Semaphore destroyed while task was waiting */
         iRetcode = RC_KAL_DESTROYED;
#ifdef NU_DETAILED_DEBUG
         isr_error_log(ERROR_WARNING | RC_KAL_DESTROYED);
#endif /* NU_DETAILED_DEBUG */
      break;

      case NU_INVALID_SUSPEND:
         /* N+ does this only if called from invalid context (aka ISR) */
         iRetcode = RC_KAL_INVALID;
#ifdef NU_DETAILED_DEBUG
         isr_error_log(ERROR_WARNING | RC_KAL_INVALID);
#endif /* NU_DETAILED_DEBUG */
      break;

      default:
         /* Unknown N+ error! */
         iRetcode = RC_KAL_SYSERR;
#ifdef NU_DETAILED_DEBUG
         isr_error_log(ERROR_WARNING | RC_KAL_SYSERR);
#endif /* NU_DETAILED_DEBUG */
   }

   return((int)iRetcode);
}

/*******************************************/
/* Release a token for the given semaphore */
/*******************************************/
int sem_put(sem_id_t sem_id)
{
   STATUS iRetcode;
   int iKALRetcode = RC_OK;

   not_from_critical_section();

   /* Release the semaphore */
   iRetcode = NU_Release_Semaphore((NU_SEMAPHORE *)sem_id);
#ifdef NU_DETAILED_DEBUG
   if(iRetcode != NU_SUCCESS)
   {
      switch(iRetcode)
      {
         case NU_INVALID_SEMAPHORE:
            /* Invalid Semaphore Id */
            isr_trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS,
               "sem_put. Invalid Id! Id=%x\n", sem_id, 0);
            isr_error_log(ERROR_WARNING | RC_KAL_INVALID);
            iKALRetcode = RC_KAL_INVALID;
         break;

         default:
            /* Unknown N+ error! */
            isr_trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS,
               "sem_put. Unknown N+ Error=%x!\n", iRetcode, 0);
            isr_error_log(ERROR_WARNING | RC_KAL_SYSERR);
            iKALRetcode = RC_KAL_SYSERR;
      }
   }
#endif /* NU_DETAILED_DEBUG */

   return( iKALRetcode );
}

/********************************************/
/* Get a semaphore ID from its name string. */
/********************************************/
int sem_id_from_name(const char *name, sem_id_t *sem_id)
{
   int iRetcode;
   static NU_SEMAPHORE *sem_ptr[MAX_SEMAPHORES
   #ifdef DRIVER_INCL_KALEXINC
                                                +MAX_MUTEXES
   #endif /* DRIVER_INCL_KALEXINC */
                                                            ];
   UNSIGNED num_sem_ptr;
   CHAR sem_name[NUP_MAX_NAME_LENGTH];
   UNSIGNED sem_current_count;
   OPTION sem_suspend_type;
   UNSIGNED sem_tasks_suspended;
   NU_TASK *sem_first_task;

   not_from_critical_section();
   not_interrupt_safe();
   
   trace_new(TRACE_KAL | TRACE_LEVEL_1, "sem_id\n");

   /* Check for bad input params */
   if((name == (char *)NULL) || (sem_id == (sem_id_t *)NULL))
   {
      error_log(ERROR_WARNING | RC_KAL_BADPTR);
      return(RC_KAL_BADPTR);
   }

   /* Get a list of the established semaphores */
   num_sem_ptr = NU_Semaphore_Pointers(sem_ptr, sizeof(sem_ptr));
   if(num_sem_ptr == 0)
   {
      trace_new(TRACE_KAL | TRACE_LEVEL_3, 
         "Failed to find any semaphore Ptrs!\n");
      error_log(ERROR_WARNING | RC_KAL_INVALID);
      return(RC_KAL_INVALID);
   }

   /* Search for the given name */
   do
   {
      memset(sem_name, 0, sizeof(sem_name));
      /* Get the semaphore Info */
      iRetcode = NU_Semaphore_Information(sem_ptr[num_sem_ptr-1], 
                                          sem_name,
                                          &sem_current_count,
                                          &sem_suspend_type,
                                          &sem_tasks_suspended,
                                          &sem_first_task);
      if(strncmp(sem_name, name, NUP_MAX_NAME_LENGTH) == 0)
      {
         break;
      }
   }
   while(--num_sem_ptr);

   if(num_sem_ptr == 0)
   {
      trace_new(TRACE_KAL | TRACE_LEVEL_3, 
         "Failed to find a semaphore named %s!\n", name);
/*       error_log(ERROR_WARNING | RC_KAL_INVALID); */
      return(RC_KAL_INVALID);
   }

   /* A match was found */
   *sem_id = (sem_id_t)sem_ptr[num_sem_ptr-1];

   return(RC_OK);
}

#if 0
int sem_id_from_name(const char *name, sem_id_t *sem_id)
{
   int iLoop;
   int iRetcode;
   
   not_from_critical_section();
   not_interrupt_safe();

   trace_new(TRACE_KAL | TRACE_LEVEL_1, "sem_id\n", 0, 0);

   /* Check for bad input params */
   if((name == (char *)NULL) || (sem_id == (sem_id_t *)NULL))
   {
      error_log(ERROR_WARNING | RC_KAL_BADPTR);
      return(RC_KAL_BADPTR);
   }

   /* Get the serialisation semaphore. */
   if(NU_Obtain_Semaphore(&nplus_sem_sem, NU_SUSPEND) != NU_SUCCESS)
   {
      isr_error_log(ERROR_WARNING | RC_KAL_SYSERR);
      return(RC_KAL_SYSERR);
   }

   /* Search our semaphore descriptor table for this name string */
   iLoop = 0;
   while(iLoop < MAX_SEMAPHORES)
   {
      /* Only consider active task descriptors. */
      if((semaphore[iLoop].flags & SEM_FLAG_STRUCT_IN_USE) &&
         (strncmp(semaphore[iLoop].name, name, NUP_MAX_NAME_LENGTH)==0))
      {
         break;
      }
      iLoop++;
   }

   /* Release the serialisation semaphore. */
   if(NU_Release_Semaphore(&nplus_sem_sem) != NU_SUCCESS)
   {
      error_log(ERROR_WARNING | RC_KAL_SYSERR);
      return(RC_KAL_SYSERR);
   }

   /* If we found the name, return the KAL task id */
   if(iLoop < MAX_SEMAPHORES)
   {
      *sem_id = (sem_id_t)SID_FROM_INDEX(iLoop);
      iRetcode = RC_OK;
   }
   else
   {
      trace_new(TRACE_KAL | TRACE_LEVEL_3, 
         "Failed to find a semaphore named %s in KAL table!\n", name);
      error_log(ERROR_WARNING | RC_KAL_SYSERR);
      iRetcode = RC_KAL_INVALID;
   }

   return(iRetcode);
}
#endif

////////////////////////////////////////////////////////////////
		// Pipe Fuctions	 add by ÀµÔÆÁ¼ 2006.03

#define PIPE_SIZE	(1024*6)

pipe_id_t pipe_create(const char *name)
{
	int ret;
	NU_PIPE *pPipeCtrlBlk;
	u_int32 *start_addr;
	CHAR pipe_name[NUP_MAX_NAME_LENGTH+1];

	if (!name)
		return((pipe_id_t)NULL);
	
	ret  = NU_Allocate_Memory(&gSysPipeMesgMemory,
                                       (void**)(&start_addr),
                                       PIPE_SIZE,
                                       NU_NO_SUSPEND);

	if (ret != RC_OK)
	{
		trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
	     "pipe_create. Mesg Mem Allocate error!\n");
		return((pipe_id_t)NULL);
	}

	ret = NU_Allocate_Partition(&gSysPipeCtrlBlkMemory,
                                    (VOID **)&pPipeCtrlBlk,
                                    NU_NO_SUSPEND);
	if(ret != RC_OK)
	{
	  trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
	     "pipe_create. Ctrl Blk Allocate error!\n");
	  error_log(ERROR_WARNING | RC_KAL_SYSERR);

	  NU_Deallocate_Memory(start_addr);
	  return((pipe_id_t)NULL);
	}

	memset(pPipeCtrlBlk, 0, sizeof(NU_PIPE));

		/* Copy the name */
	strncpy(pipe_name, name, NUP_MAX_NAME_LENGTH);

	ret =NU_Create_Pipe(pPipeCtrlBlk, pipe_name, start_addr, PIPE_SIZE, NU_VARIABLE_SIZE, PIPE_SIZE, NU_FIFO);
	if (NU_SUCCESS != ret)
	{
		trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
		 "pipe_create. Ctrl Blk Allocate error!\n");
		error_log(ERROR_WARNING | RC_KAL_SYSERR);

		NU_Deallocate_Memory(start_addr);
		NU_Deallocate_Partition(pPipeCtrlBlk);
		return ((pipe_id_t)NULL);	
	}

	return ((pipe_id_t)pPipeCtrlBlk);
}

int pipe_destroy(pipe_id_t pipe_id)
{
	NU_PIPE *pPipe = (NU_PIPE *)pipe_id;

	char name[NUP_MAX_NAME_LENGTH];
	void *start_addr;
	UNSIGNED pipe_size, available, messages, message_size, tasks_waiting; 
	OPTION message_type, suspend_type;
	NU_TASK *pTask;

	int ret;
	
	if (!pPipe)
		return RC_KAL_INVALID;


	ret = NU_Pipe_Information(pPipe, name, &start_addr, &pipe_size, &available, &messages, 
		&message_type, &message_size, &suspend_type, &tasks_waiting, &pTask);

	if (ret != NU_SUCCESS)
	{
		if(ret == NU_INVALID_PIPE)
		{
		 /* Pipe Ctrl Blk Ptr is Invalid. */
		 trace_new(TRACE_KAL | TRACE_LEVEL_2, 
		    "Invalid Delete Pipe Id=%d\n", pipe_id);
		 error_log(ERROR_WARNING | RC_KAL_INVALID);
		 return(RC_KAL_INVALID);
		}
		else
		{
		 trace_new(TRACE_KAL | TRACE_LEVEL_2, 
		    "Error getting Pipe Info. Id=%d\n", pipe_id);
		 error_log(ERROR_WARNING | RC_KAL_SYSERR);
		 return(RC_KAL_SYSERR);
		}
	}

	
	ret = NU_Delete_Pipe(pPipe);

	if(ret != NU_SUCCESS)
	{
		/* Error ! Can't delete RTOS Pipe */
		trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
		"Can't delete Pipe!\n");
		error_log(ERROR_WARNING | RC_KAL_SYSERR);
		ret = RC_KAL_SYSERR;
	}
	else
	{
		/* Pipe Deleted. Release the Ctrl Blk & resources. */
		NU_Deallocate_Memory(start_addr);
		if(NU_Deallocate_Partition(pPipe) != NU_SUCCESS)
		{
		error_log(ERROR_WARNING | RC_KAL_SYSERR);
		}
		trace_new(TRACE_KAL | TRACE_LEVEL_2, 
		"Deleted Qu. Kal Id=%x\n", pipe_id);
	}

   return ret;
}

int pipe_read(pipe_id_t pipe_id, void *message, u_int32 size, u_int32 *actual_size, u_int32 timeout_ms)
{
	int ret;

	if ((!pipe_id ) || (!message) || (!actual_size))
	{
		return RC_KAL_INVALID;
	}
	
	ret = NU_Receive_From_Pipe((NU_PIPE *)pipe_id, message, size, actual_size, timeout_ms);

	return ret;
}

int pipe_write(pipe_id_t pipe_id, void *message, u_int32 size, u_int32 timeout_ms)
{
	int ret;

	if ((!pipe_id ) || (!message))
	{
		return RC_KAL_INVALID;
	}
	
	ret = NU_Send_To_Pipe((NU_PIPE *)pipe_id, message, size, timeout_ms);

	return ret;
}

////////////////////////////////////////////////////////////////


/*********************/
/*********************/
/** Queue Functions **/
/*********************/
/*********************/

/**************************/
/* Create a message queue */
/**************************/
queue_id_t qu_create(unsigned int max_elements, const char *name)
{
   return(qu_create_ex(max_elements, 
                       name,
                       KAL_QMSG_SIZE,
                       KAL_FIXED_LENGTH,
                       KAL_TASK_ORDER_PRIORITY));
}

queue_id_t qu_create_ex(unsigned int max_elements, const char *name, 
                        int max_length, qu_msg_type_t msg_type, 
                        task_wait_order_t wait_order)
{
   int iRetcode;
   queue_id_t dummy_id;
   CHAR qu_name[NUP_MAX_NAME_LENGTH];
   OPTION qu_msg_type;
   OPTION task_suspend_type;
   NU_QUEUE *pQuCtrlBlk;
   u_int32 *qu_mem_start_address;
   bool valid = FALSE;

   not_from_critical_section();
   not_interrupt_safe();

   trace_new(TRACE_KAL | TRACE_LEVEL_1, "qu_create\n");

   /* Check for a valid number of qu elements */
   if(max_elements == 0)
   {
      return((queue_id_t)NULL);
   }

   /* If caller did not specifiy a name, make one up. */
   if(name == NULL)
   {
      sem_get(kal_qu_sem, KAL_WAIT_FOREVER);
      sprintf(qu_name, "Q%03x", QuNameCount++);
      sem_put(kal_qu_sem);
   }
   else
   {
      /* make sure the name is unique */
      if(qu_id_from_name(name, &dummy_id) != RC_KAL_INVALID)
      {
         trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
            "qu_create. Name error!\n");
         error_log(ERROR_WARNING | RC_KAL_SYSERR);
         return((queue_id_t)NULL);
      }

      /* Copy the name */
      strncpy(qu_name, name, NUP_MAX_NAME_LENGTH);
   }

   /* Make sure string is null terminated */
   qu_name[NUP_MAX_NAME_LENGTH-1] = '\0';

   /* Translate Qu Message Type & Allocate Enough memory for this type of Qu */
   switch(msg_type)
   {
      case KAL_FIXED_LENGTH:
         qu_msg_type = NU_FIXED_SIZE;
         iRetcode = NU_Allocate_Memory(&gSysQuMesgMemory,
                                       (void**)(&qu_mem_start_address),
                                       (max_elements*max_length),
                                       NU_NO_SUSPEND);
         if(iRetcode != RC_OK)
         {
            /* Qu Memory could not be created ! */
            NU_Deallocate_Memory(qu_mem_start_address);
            trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
                                    "*** Couldn't create Fix Queue Memory !\n");
            error_log(ERROR_WARNING | RC_KAL_SYSERR);
            return((queue_id_t)NULL);
         }
      break;

      case KAL_VARIABLE_LENGTH:
         qu_msg_type = NU_VARIABLE_SIZE;
         iRetcode = mem_malloc_ex(KAL_SYS_MEM_POOL_ID, 
                                  (max_elements*(max_length+sizeof(UNSIGNED))), 
                                    /* Each Variable length Q requires an extra 
                                    UNSIGNED data element of overhead */
                                  (void**)(&qu_mem_start_address));
         if(iRetcode != RC_OK)
         {
            /* Qu Memory could not be created ! */
            mem_free(qu_mem_start_address);
            trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
                                    "*** Couldn't create Var Queue Memory !\n");
            error_log(ERROR_WARNING | RC_KAL_SYSERR);
            return((queue_id_t)NULL);
         }
      break;

      default:
         trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
            "Qu Invalid Message Type=%d!\n", msg_type);
         error_log(ERROR_WARNING | RC_KAL_INVALID);
         return((queue_id_t)NULL);
   }

   /* Translate the Task wait order */
   switch(wait_order)
   {
      case KAL_TASK_ORDER_FIFO:
         task_suspend_type = NU_FIFO;
      break;

      case KAL_TASK_ORDER_PRIORITY:
         task_suspend_type = NU_PRIORITY;
      break;

      default:
         mem_free(qu_mem_start_address);
         trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
            "Qu Invalid Task Wait order=%d!\n", wait_order);
         error_log(ERROR_WARNING | RC_KAL_INVALID);
         return((queue_id_t)NULL);
   }

   /* Get memory to store the Qu Control Block */
   iRetcode = NU_Allocate_Partition(&gSysQuCtrlBlkMemory,
                                    (VOID **)&pQuCtrlBlk,
                                    NU_NO_SUSPEND);
   if(iRetcode != RC_OK)
   {
      trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
         "qu_create. Ctrl Blk Allocate error!\n");
      error_log(ERROR_WARNING | RC_KAL_SYSERR);
      return((queue_id_t)NULL);
   }

   /* Has this block already been allocated? */
   qu_is_valid((queue_id_t)pQuCtrlBlk, &valid);
   if(valid)
   {
      if(NU_Deallocate_Partition((void *)pQuCtrlBlk) != NU_SUCCESS)
      {
         error_log(ERROR_WARNING | RC_KAL_SYSERR);
      }
      trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
         "Memory block:%x already allocated!\n", pQuCtrlBlk);
      error_log(ERROR_FATAL | RC_KAL_SYSERR);
      return((queue_id_t)NULL);
   }
   else
   {
      memset(pQuCtrlBlk, 0, sizeof(NU_QUEUE));
   }

   /* Max Message length will be in number of bytes (the pSOS std) convert to 
      the N+ std - UNSIGNED */
   max_length /= sizeof(UNSIGNED);

   /* Create the new queue */
   iRetcode = NU_Create_Queue(pQuCtrlBlk, 
                              qu_name,
                              qu_mem_start_address,
                              (max_elements*max_length),
                              qu_msg_type,
                              max_length,
                              task_suspend_type);
   /* Return handle or error */
   if(iRetcode != NU_SUCCESS)
   {
      /* Free the resources used */
      memset(pQuCtrlBlk, 0, sizeof(NU_QUEUE));
      mem_free(qu_mem_start_address);
      if(NU_Deallocate_Partition((void *)pQuCtrlBlk) != NU_SUCCESS)
      {
         error_log(ERROR_WARNING | RC_KAL_SYSERR);
      }

      /* Output the failure message */
#ifdef NU_DETAILED_DEBUG
      switch(iRetcode)
      {
         case NU_INVALID_QUEUE:
            /* CtrlBlk Ptr NULL or CtrBlk aready in use */
         trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
            "Sem Create Ctrl Blk:%x already in Use! \n", pQuCtrlBlk);
         error_log(ERROR_WARNING | RC_KAL_INVALID);
         break;

         case NU_INVALID_MEMORY:
            /* Qu Memory area Invalid */
            trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
               "Invalid Qu Memory! StartAddress=%x\n", 
                  qu_mem_start_address);
            error_log(ERROR_WARNING | RC_KAL_INVALID);
         break;

         case NU_INVALID_MESSAGE:
            /* Message type is invalid */
            trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
               "Invalid Qu Mesg Type! Kal Type=%d, N+ type=%d\n", 
                  qu_msg_type, msg_type);
            error_log(ERROR_WARNING | RC_KAL_INVALID);
         break;

         case NU_INVALID_SIZE:
            /* Invalid Message Size (> Qu Size or Zero) */
            trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
               "Invalid Qu Mesg Size! Size=%x\n", max_length);
            error_log(ERROR_WARNING | RC_KAL_INVALID);
         break;

         case NU_INVALID_SUSPEND:
            /* Invalid Suspend Type */
            trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
               "Invalid Qu Suspend Type! Kal Type=%d, N+ type=%d\n", 
                  wait_order, task_suspend_type);
            error_log(ERROR_WARNING | RC_KAL_INVALID);
         break;

         default:
            trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
               "Unknown N+ Qu Create Error! Error=%x\n", 
                  iRetcode);
            error_log(ERROR_WARNING | RC_KAL_SYSERR);
      }
#else /* NU_DETAILED_DEBUG */
      /* Error ! Can't create RTOS queue */
      trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
         "Can't create queue! RC=%x\n", iRetcode);
      error_log(ERROR_WARNING | RC_KAL_SYSERR);
#endif /* NU_DETAILED_DEBUG */

      return((queue_id_t)NULL);
   }

   trace_new(TRACE_KAL | TRACE_LEVEL_2, 
      "N+ Queue %s Created. Id:0x%08x\n", qu_name, pQuCtrlBlk);

   return((queue_id_t)pQuCtrlBlk);
}

/*******************************************************************/
/* Does the passed qu_id refer to a valid Queue, currently in use? */
/*******************************************************************/
int qu_is_valid(queue_id_t qu_id, bool *valid)
{
   static NU_QUEUE *qu_ptr[MAX_QUEUES];
   UNSIGNED num_qu_ptr;

   not_from_critical_section();
   not_interrupt_safe();

   if(valid == (bool*)NULL)
   {
      error_log(ERROR_WARNING | RC_KAL_BADPTR);
      return(RC_KAL_BADPTR);
   }

   /* Get a list of the established queues */
   num_qu_ptr = NU_Queue_Pointers(qu_ptr, sizeof(qu_ptr));
   if(num_qu_ptr == 0)
   {
      trace_new(TRACE_KAL | TRACE_LEVEL_3, 
         "Failed to find any qus Ptrs!\n");
/*       error_log(ERROR_WARNING | RC_KAL_INVALID); */
      return(RC_KAL_INVALID);
   }

   /* Search for the given qu_id */
   do
   {
      if(qu_ptr[num_qu_ptr-1] == (NU_QUEUE *)qu_id)
      {
         break;
      }
   }
   while(--num_qu_ptr);

   if(num_qu_ptr == 0)
   {
      *valid = FALSE;
   }
   else
   {
      *valid = TRUE;
   }

   return(RC_OK);
}

/***************************/
/* Destroy a message queue */
/***************************/
int qu_destroy(queue_id_t qu_id)
{
   STATUS iRetcode;
   CHAR qu_name[NUP_MAX_NAME_LENGTH];
   UNSIGNED *qu_mem_start_address,
      qu_size,
      qu_available,
      qu_messages;
   OPTION qu_message_type;
   UNSIGNED qu_message_size;
   OPTION qu_suspend_type;
   UNSIGNED qu_tasks_waiting;
   NU_TASK *qu_first_task;
   
   not_from_critical_section();
   not_interrupt_safe();

   trace_new(TRACE_KAL | TRACE_LEVEL_2, "qu_destroy\n");

   /* Get the Queue's Memory start address */
   iRetcode = NU_Queue_Information((NU_QUEUE *)qu_id,
                                   qu_name,
                                   (VOID **)&qu_mem_start_address,
                                   &qu_size,
                                   &qu_available,
                                   &qu_messages,
                                   &qu_message_type,
                                   &qu_message_size,
                                   &qu_suspend_type,
                                   &qu_tasks_waiting,
                                   &qu_first_task);
   if(iRetcode != NU_SUCCESS)
   {
      if(iRetcode == NU_INVALID_QUEUE)
      {
         /* Qu Ctrl Blk Ptr is Invalid. */
         trace_new(TRACE_KAL | TRACE_LEVEL_2, 
            "Invalid Delete Qu Id=%d\n", qu_id);
         error_log(ERROR_WARNING | RC_KAL_INVALID);
         return(RC_KAL_INVALID);
      }
      else
      {
         trace_new(TRACE_KAL | TRACE_LEVEL_2, 
            "Error getting Qu Info. Id=%d\n", qu_id);
         error_log(ERROR_WARNING | RC_KAL_SYSERR);
         return(RC_KAL_SYSERR);
      }
   }

   /* Delete the Queue */
   iRetcode = NU_Delete_Queue((NU_QUEUE *)qu_id);
   if(iRetcode != NU_SUCCESS)
   {

#ifdef NU_DETAILED_DEBUG
      switch(iRetcode)
      {
         case NU_INVALID_QUEUE:
            /* Qu Ctrl Blk Ptr is Invalid. */
            trace_new(TRACE_KAL | TRACE_LEVEL_2, 
               "Invalid Delete Qu Id=%d\n", qu_id);
            error_log(ERROR_WARNING | RC_KAL_INVALID);
            iRetcode = RC_KAL_INVALID;
         break;

         default:
            /* Unknown N+ Error */
            trace_new(TRACE_KAL | TRACE_LEVEL_2, 
               "Delete Qu. Unknown N+ Error=%x\n", iRetcode);
            error_log(ERROR_WARNING | RC_KAL_SYSERR);
            iRetcode = RC_KAL_SYSERR;
      }
#else /* NU_DETAILED_DEBUG */
      /* Error ! Can't delete RTOS Queue */
      trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
         "Can't delete Queue!\n");
      error_log(ERROR_WARNING | RC_KAL_SYSERR);
      iRetcode = RC_KAL_SYSERR;
#endif /* NU_DETAILED_DEBUG */
   }
   else
   {
      /* Qu Deleted. Release the Ctrl Blk & resources. */
      mem_free((void *)qu_mem_start_address);
      if(NU_Deallocate_Partition((void *)qu_id) != NU_SUCCESS)
      {
         error_log(ERROR_WARNING | RC_KAL_SYSERR);
      }
      trace_new(TRACE_KAL | TRACE_LEVEL_2, 
         "Deleted Qu. Kal Id=%x\n", qu_id);
   }

   return(iRetcode);
}

/**********************************/
/* Put a message on a given queue */
/**********************************/
int qu_send(queue_id_t qu_id, void *message)
{
   return(qu_send_ex(qu_id, 
                     message, 
                     KAL_QMSG_SIZE, 
                     KAL_FIXED_LENGTH, 
                     KAL_QMSG_PRI_NORM));
}

int qu_send_ex(queue_id_t qu_id, void *message, int message_length,
               u_int32 timeout_ms, qu_priority_t priority)
{
   int iRetcode;
   UNSIGNED dwTimeoutTicks;
   
   not_from_critical_section();

   /* NB: Do NOT call isr_trace from this function. If you do, you end up */
   /* in an infinite loop since isr_trace calls qu_send !                 */

   if(message == (void*)NULL)
   {
      return(RC_KAL_BADPTR);
   }

   /* Translate the suspend parameter */
   if(timeout_ms == KAL_WAIT_FOREVER)
   {
      dwTimeoutTicks = NU_SUSPEND;
   }
   else if(timeout_ms == 0)
   {
      dwTimeoutTicks = NU_NO_SUSPEND;
   }
   else
   {
      /* Convert the specified time (milliSecs) to equivalent ticks */
      dwTimeoutTicks = (timeout_ms * KAL_TICKS2SEC)/1000;
      if(dwTimeoutTicks == 0)
      {
         dwTimeoutTicks++;
      }
   }

   /* Message length will be in number of bytes (the pSOS std) convert to the 
      N+ std - UNSIGNED */
   message_length /= sizeof(UNSIGNED);

   /* Send the message */
   if(priority == KAL_QMSG_PRI_NORM)
   {
      iRetcode = NU_Send_To_Queue((NU_QUEUE *)qu_id, 
                                  message, 
                                  message_length,
                                  dwTimeoutTicks);
   }
   else if(priority == KAL_QMSG_PRI_HI)
   {
      iRetcode = NU_Send_To_Front_Of_Queue((NU_QUEUE *)qu_id, 
                                           message, 
                                           message_length,
                                           dwTimeoutTicks);
   }
   else
   {
      /* Invalid Q Send Priority */
      return(RC_KAL_INVALID);
   }

   if(iRetcode != NU_SUCCESS)
   {
      switch(iRetcode)
      {
         case NU_INVALID_QUEUE:
            /* Queue pointer is invalid */
            iRetcode = RC_KAL_INVALID;
         break;

         case NU_INVALID_POINTER:
            /* Message pointer is Invalid */
            iRetcode = RC_KAL_INVALID;
         break;

         case NU_INVALID_SIZE:
            /* Message Size Invalid (Fixed Message Size Qu)*/
            iRetcode = RC_KAL_INVALID;
         break;

         case NU_INVALID_SUSPEND:
            /* Invalid Suspend Context (aka ISR?) */
            iRetcode = RC_KAL_INVALID;
         break;

         case NU_QUEUE_FULL:
            /* Qu is Full */
            iRetcode = RC_KAL_QFULL;
         break;

         case NU_TIMEOUT:
            /* Send timed out */
            iRetcode = RC_KAL_TIMEOUT;
         break;

         case NU_QUEUE_DELETED:
            /* Qu was deleted while task was waiting */
            iRetcode = RC_KAL_DESTROYED;
         break;

         case NU_QUEUE_RESET:
            /* Qu was reset while Task was waiting */
            /* Kal does not support Qu Reset */
            iRetcode = RC_KAL_SYSERR;
         break;

         default:
            /* Unknown N+ error */
            iRetcode = RC_KAL_SYSERR;
      }
   }

   return(iRetcode);
}

/************************************/
/* Get a message from a given queue */
/************************************/
int qu_receive(queue_id_t qu_id, u_int32 timeout_ms, void *message)
{
   return(qu_receive_ex(qu_id, 
                        timeout_ms, 
                        message, 
                        KAL_QMSG_SIZE));
}

int qu_receive_ex(queue_id_t qu_id, u_int32 timeout_ms, void *message, 
                  int message_length)
{
   int      iRetcode;
   UNSIGNED qu_actual_msg_length;
   UNSIGNED dwTimeoutTicks;
   
   not_from_critical_section();

   /* NB: Do NOT call isr_trace from this function. If you do, you end up */
   /* in an infinite loop since isr_trace causes the trace task to call   */
   /* qu_receive.                                                         */

   #ifdef DEBUG
   /* If called under interrupt context, this function must NOT block! */
   if(timeout_ms != 0)
   {
     not_interrupt_safe();
   }
   #endif
   
   if(message == (void *)NULL)
   {
      return(RC_KAL_BADPTR);
   }

   /* Translate the suspend parameter */
   if(timeout_ms == KAL_WAIT_FOREVER)
   {
      dwTimeoutTicks = NU_SUSPEND;
   }
   else if(timeout_ms == 0)
   {
      dwTimeoutTicks = NU_NO_SUSPEND;
   }
   else
   {
      /* Convert the specified time (milliSecs) to equivalent ticks */
      dwTimeoutTicks = (timeout_ms * KAL_TICKS2SEC)/1000;
      if(dwTimeoutTicks == 0)
      {
         dwTimeoutTicks++;
      }
   }

   /* Message length will be in number of bytes (the pSOS std) convert to the 
      N+ std - UNSIGNED */
   message_length /= sizeof(UNSIGNED);

   /* Receive the message */
   iRetcode = NU_Receive_From_Queue((NU_QUEUE *)qu_id, 
                                    message,
                                    message_length, 
                                    &qu_actual_msg_length,
                                    dwTimeoutTicks);

   /* In case of Variable message size queues "message_length" would be 
      the size of the buffer passed in. We would not be able to return the 
      actual number of bytes copied "qu_actual_msg_length", ie, how much of 
      the buffer got filled in.
   */

   switch(iRetcode)
   {
      case NU_SUCCESS:
      break;

      case NU_QUEUE_EMPTY: /* Not an error */
         /* Qu is Empty */
         iRetcode = RC_KAL_EMPTY;
      break;

      case NU_TIMEOUT: /* Not an error */
         /* Receive timed out */
         iRetcode = RC_KAL_TIMEOUT;
      break;

      case NU_INVALID_QUEUE:
         /* Queue pointer is invalid */
         iRetcode = RC_KAL_INVALID;
      break;

      case NU_INVALID_POINTER:
         /* Message pointer is Invalid */
         iRetcode = RC_KAL_INVALID;
      break;

      case NU_INVALID_SIZE:
         /* Message Size Invalid (Fixed Message Size Qu)*/
         iRetcode = RC_KAL_INVALID;
      break;

      case NU_INVALID_SUSPEND:
         /* Invalid Suspend Context (aka ISR?) */
         iRetcode = RC_KAL_INVALID;
      break;

      case NU_QUEUE_DELETED:
         /* Qu was deleted while task was waiting */
         iRetcode = RC_KAL_DESTROYED;
      break;

      case NU_QUEUE_RESET:
         /* Qu was reset while Task was waiting */
         /* Kal does not support Qu Reset */
         iRetcode = RC_KAL_SYSERR;
      break;

      default:
         /* Unknown N+ error */
         iRetcode = RC_KAL_SYSERR;
   }

   return(iRetcode);
}


int qu_reset(queue_id_t qu_id)
{
   return(qu_reset_ex(qu_id));
}
int qu_reset_ex(queue_id_t qu_id)
{
   int iRetcode;
   
   not_from_critical_section();

   /* Send the message */
   iRetcode = NU_Reset_Queue((NU_QUEUE *)qu_id);

   if(iRetcode != NU_SUCCESS)
   {
      switch(iRetcode)
      {
         case NU_INVALID_QUEUE:
            /* Queue pointer is invalid */
            iRetcode = RC_KAL_INVALID;
         break;
      }
   }

   return(iRetcode);
}

/****************************************/
/* Get a queue ID from its name string. */
/****************************************/
int qu_id_from_name(const char *name, queue_id_t *qu_id)
{
   int iRetcode;
   static NU_QUEUE *qu_ptr[MAX_QUEUES];
   UNSIGNED num_qu_ptr;
   CHAR qu_name[NUP_MAX_NAME_LENGTH];
   UNSIGNED *qu_mem_start_address,
      qu_size,
      qu_available,
      qu_messages;
   OPTION qu_message_type;
   UNSIGNED qu_message_size;
   OPTION qu_suspend_type;
   UNSIGNED qu_tasks_waiting;
   NU_TASK *qu_first_task;

   not_from_critical_section();
   not_interrupt_safe();
   
   trace_new(TRACE_KAL | TRACE_LEVEL_1, "qu_id\n");

   /* Check for bad input params */
   if((name == (const char *)NULL) || (qu_id == (queue_id_t *)NULL))
   {
      error_log(ERROR_WARNING | RC_KAL_BADPTR);
      return(RC_KAL_BADPTR);
   }

   /* Get a list of the established semaphores */
   num_qu_ptr = NU_Queue_Pointers(qu_ptr, sizeof(qu_ptr));
   if(num_qu_ptr == 0)
   {
      trace_new(TRACE_KAL | TRACE_LEVEL_3, 
         "Failed to find any queue Ptrs!\n");
/*       error_log(ERROR_WARNING | RC_KAL_INVALID); */
      return(RC_KAL_INVALID);
   }

   /* Search for the given name */
   do
   {
      memset(qu_name, 0, sizeof(qu_name));
      /* Get the queue Info */
      iRetcode = NU_Queue_Information(qu_ptr[num_qu_ptr-1],
                                      qu_name,
                                      (VOID **)&qu_mem_start_address,
                                      &qu_size,
                                      &qu_available,
                                      &qu_messages,
                                      &qu_message_type,
                                      &qu_message_size,
                                      &qu_suspend_type,
                                      &qu_tasks_waiting,
                                      &qu_first_task);
      if(strncmp(qu_name, name, NUP_MAX_NAME_LENGTH) == 0)
      {
         break;
      }
   }
   while(--num_qu_ptr);

   if(num_qu_ptr == 0)
   {
      trace_new(TRACE_KAL | TRACE_LEVEL_3, 
         "Failed to find a queue named %s!\n", name);
/*       error_log(ERROR_WARNING | RC_KAL_INVALID); */
      return(RC_KAL_INVALID);
   }

   /* A match was found */
   *qu_id = (queue_id_t)qu_ptr[num_qu_ptr-1];

   return(RC_OK);
}

/***********************/
/***********************/
/**                   **/
/** Memory Management **/
/**                   **/
/***********************/
/***********************/

/***********************************************/
/* Get a block of memory of the specified size */
/***********************************************/
void *mem_malloc(u_int32 size)
{
   void *ptr=NULL;
   int retcode = RC_OK;

   not_from_critical_section();
   not_interrupt_safe();

   #if (defined MEM_ERROR_DEBUG) && (defined OPENTV) && (defined DEBUG)
   /* In specially marked builds, allow someone debugging to get a complete    */
   /* dump of the direct heap by setting the bDumpHeapNow variable and waiting */
   /* for the next call to mem_malloc or mem_free.                             */
   if(bDumpHeapNow)
   {
      dump_direct_heap();
      bDumpHeapNow = FALSE;
   }
   #endif

   if(size == 0)
   {
      return(NULL);
   }

   #if (defined OPENTV) && (OPENTV_DIRECT_HEAP_SIZE == 0)
      ptr = (void *)direct_memory_alloc(size);
   #else 
      retcode = mem_malloc_ex(KAL_SYS_MEM_POOL_ID, size, &ptr);
   #endif

   if((ptr == NULL) || (retcode != RC_OK))
   {
      trace_new(TRACE_KAL | TRACE_LEVEL_3, 
         "mem_malloc failed for size=0x%lX\n", size);
      error_log(ERROR_WARNING | RC_KAL_NORESOURCES);
   }
    //  trace( "**************mem_malloc address=0x%X*************\n\n\n", ptr);

   return(ptr);
}

/**********************************************************/
/* Get a block of Non-Cached memory of the specified size */
/**********************************************************/
void *mem_nc_malloc(u_int32 size)
{
   void *ptr, *nc_ptr=NULL;
   #if ((CPU_TYPE==CPU_ARM940T) || (CPU_TYPE==AUTOSENSE)) && (!MMU_CACHE_DISABLE)
   bool prev_state;
   #endif
    
   /* Allocate cached memory.
    *
    * Why 2 cache lines?  We can guaruntee that the beginning of 
    * our data will not be corrupted by cache aligning it (which 
    * requires one cache line).
    *
    * HOWEVER, we have no way of guarunteeing that the end of our
    * data will not be corrupted without allocating an extra cache 
    * line.
    *
    * You can modify mem_malloc to cache-align it's alloc's but 
    * that is not sufficient because mem_malloc may not be the 
    * only call allocating memory in the system.  mem_malloc boils
    * down to a call to NU_Allocate_Memory.  Since we have no control
    * over who calls NU_Allocate_Memory (ex. the RTOS itself), we
    * can not guaruntee the cleanliness of our data other than to 
    * push out the next malloc past the end of our last cache line.
    * This is done by allocating the second cache line.
    *
    */

   if(size == 0)
   {
      return(NULL);
   }

   ptr = mem_malloc(size + 2*CACHE_LINE_SIZE + sizeof(void *));

   if(ptr)
   {
      /* Flush & invalidate D-cache & drain write buffer. */
      #if !MMU_CACHE_DISABLE

         #if (CPU_TYPE==CPU_ARM940T) || (CPU_TYPE==AUTOSENSE)

            #if CPU_TYPE == AUTOSENSE
               if ( GetCPUType() == CPU_ARM940T )
               {
            #endif

                  /* On 940, we can't flush & invalidate just the buffer, */
                  /* we must do the entire cache.                         */
                  
                  /* This must be done inside a critical section to prevent potential   */
                  /* problems of interrupts writing to memory between the clean and the */
                  /* flush operations. This could cause memory corruption.              */
                  
                  prev_state = critical_section_begin();
                  CleanDCache();
                  FlushDCache();
                  DrainWriteBuffer();    
                  critical_section_end(prev_state); 

            #if CPU_TYPE == AUTOSENSE
               }
            #endif

         #endif /* (CPU_TYPE==CPU_ARM940T) || (CPU_TYPE==AUTOSENSE) */

         #if (CPU_TYPE==CPU_ARM920T) || (CPU_TYPE==AUTOSENSE)

            #if CPU_TYPE == AUTOSENSE
               if ( GetCPUType() == CPU_ARM920T )
               {
            #else
               {
            #endif

                  /* On 920, we flush & invalidate just the buffer allocated. */
                  int i;
                  for (i = (int)size + 2*CACHE_LINE_SIZE + sizeof(void *) - 1;
                       i >= 0;
                       i = i - 1)
                  /*   Could we do this instead?  (MFB)            
                       i = i - CACHE_LINE_SIZE) */
                  {
                     CleanAndInvalDCacheMVA((u_int32)ptr + i);
                  }
                  DrainWriteBuffer();

            /* this looks a bit silly, but keeps parentheses balanced for editors */
            #if CPU_TYPE == AUTOSENSE
               }
            #else
               }
            #endif

         #endif /* (CPU_TYPE==CPU_ARM920T) || (CPU_TYPE==AUTOSENSE) */

         #if (CPU_TYPE!=CPU_ARM940T) && (CPU_TYPE!=CPU_ARM920T) && (CPU_TYPE!=AUTOSENSE)
            #error CPU_TYPE must be either CPU_ARM940T, CPU_ARM920T, or AUTOSENSE!
         #endif /* CPU_TYPE */

      #endif /* MMU_CACHE_DISABLE */

      /* Align to cache line boundary. */
      nc_ptr = (void *)(((u_int32)ptr + (CACHE_LINE_SIZE-1) + 
                                 sizeof(void *)) & ~(CACHE_LINE_SIZE-1));

      /* store the pointer to the memory we originally allocated 
         at the aligned address */
      *(void**)nc_ptr = ptr;

      /* now move the pointer up one word */
      nc_ptr = (void*)((int)nc_ptr + sizeof(void*));

      /* Modify the ptr to reference the NC aperture. */
      nc_ptr = (void*)SET_FLAGS(nc_ptr, NCR_BASE);
   }

#ifdef DEBUG
   dwNumNcMalloc++;
#endif

   return(nc_ptr);
}

/*************************************************/
/* Return a previously allocated block of memory */
/*************************************************/
void mem_free(void *ptr)
{
   void *lpFixedUp;
   #if (!defined OPENTV) || (OPENTV_DIRECT_HEAP_SIZE != 0)
   int retcode;
   #endif
   
   not_from_critical_section();
   not_interrupt_safe();

   if(ptr == (void *)NULL){
      error_log(ERROR_WARNING | RC_KAL_BADPTR);
      return;
   }

   #if (defined MEM_ERROR_DEBUG) && (defined OPENTV) && (defined DEBUG)
   /* In specially marked builds, allow someone debugging to get a complete    */
   /* dump of the direct heap by setting the bDumpHeapNow variable and waiting */
   /* for the next call to mem_malloc or mem_free.                             */
   if(bDumpHeapNow)
   {
      dump_direct_heap();
      bDumpHeapNow = FALSE;
   }
   #endif

   /* Remove cache and DMA aperture bits */
   lpFixedUp = (void *)((u_int32)ptr & NONCACHEABLE_RAM_MASK);

   #if (defined OPENTV) && (OPENTV_DIRECT_HEAP_SIZE == 0)
      direct_memory_free(lpFixedUp);
   #else
      retcode = mem_free_ex(KAL_SYS_MEM_POOL_ID, lpFixedUp);
      if(retcode != RC_OK)
      {
      trace_new(TRACE_KAL | TRACE_LEVEL_3, 
         "mem_free failed for ptr(%x)=0x%lX\n", ptr, lpFixedUp);
      }
   #endif

   return;
}

/************************************************************/
/* Return a previously allocated block of Non-Cached memory */
/************************************************************/
void mem_nc_free(void *nc_ptr)
{
   void *ptr;

   if(nc_ptr == (void *)NULL){
      error_log(ERROR_WARNING | RC_KAL_BADPTR);
      return;
   }

   /* our "real" pointer is one word before the address 
   which is pointer at */
   ptr = *(void **)(((u_int32)nc_ptr & NONCACHEABLE_RAM_MASK) - sizeof(void *));

   mem_free(ptr);

#ifdef DEBUG
   dwNumNcFree++;
#endif
}

/******************************************************************************/
/* Get a block of memory of the specified size from the Specified Memory Pool */
/******************************************************************************/
#undef MEM_TRACE
#ifdef MEM_TRACE
u_int32 TotalMallocMemeory=0;
u_int32 MemoryLogSize[1024]={0};
u_int32 MemoryLogPtr[1024]={0};
int GetFreeMemLogId(void)
{
	int i;
	for(i=0;i<1024;i++)
	{
		if(MemoryLogSize[i]==0)
			break;
	}
	return i;
}
#endif
int mem_malloc_ex(pool_id_t pool_id, u_int32 size, void **pptr)
{

   int            iIndex;
   int            iRetcode;
   NU_MEMORY_POOL *pool_rtos_id;

   not_from_critical_section();
   not_interrupt_safe();

   /* Check for bad input */
   if(pptr == (void **)NULL)
   {
      error_log(ERROR_WARNING | RC_KAL_BADPTR);
      return(RC_KAL_BADPTR);
   }

   /* Get serialisation semaphore. */
   if(sem_get(kal_pool_sem, KAL_WAIT_FOREVER) != RC_OK)
   {
      error_log(ERROR_WARNING | RC_KAL_SYSERR);
      return(RC_KAL_SYSERR);
   }

   iIndex = INDEX_FROM_MPID(pool_id);
   if((IS_VALID_MPID(pool_id)) &&
      (mem_pool[iIndex].flags & POOL_FLAG_STRUCT_IN_USE))
   {
      /* Get the Pool RTOS Id */
      pool_rtos_id = (NU_MEMORY_POOL *)mem_pool[iIndex].rtos_id;

      /* Return the serialization semaphore */
      sem_put(kal_pool_sem);

      /* Get memory */
      iRetcode = NU_Allocate_Memory(pool_rtos_id, 
                                    (VOID **)pptr, 
                                    (UNSIGNED)size,
                                    NU_NO_SUSPEND);
      if(iRetcode != NU_SUCCESS)
      {
#ifdef NU_DETAILED_DEBUG
         switch(iRetcode)
         {
            case NU_INVALID_POOL:
               /* Pool pointer is invalid */
               trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
                  "mem_alloc_ex. Invalid Pool. KAL Id=%x, RTOS Id=%x\n",
                     pool_id, pool_rtos_id);
               error_log(ERROR_WARNING | RC_KAL_INVALID);
               iRetcode = RC_KAL_INVALID;
            break;

            case NU_INVALID_POINTER:
               /* Mem pointer is Invalid */
               trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
                  "mem_alloc_ex. Invalid Mem Ptr. KAL Id=%x, Ptr=%x\n",
                     pool_id, pptr);
               error_log(ERROR_WARNING | RC_KAL_INVALID);
               iRetcode = RC_KAL_INVALID;
            break;

            case NU_INVALID_SIZE:
               /* Mem Size Invalid (Fixed Message Size Qu)*/
               trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
                  "mem_alloc_ex. Invalid Size. KAL Id=%x, Size=%x\n",
                     pool_id, size);
               error_log(ERROR_WARNING | RC_KAL_INVALID);
               iRetcode = RC_KAL_NORESOURCES;
            break;

            case NU_INVALID_SUSPEND:
               /* Invalid Suspend Context (aka ISR?) */
               trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
                  "mem_alloc_ex. Invalid Suspend Context. KAL Id=%x\n", 
                     pool_id);
               error_log(ERROR_WARNING | RC_KAL_INVALID);
               iRetcode = RC_KAL_INVALID;
            break;

            case NU_NO_MEMORY:
               /* Mem Request could not be immediately satisfied */
               /* !! We suspend forever. how did we get this? !! */
               trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
                  "mem_alloc_ex. No Memory. KAL Id=%x\n",
                     pool_id);
               error_log(ERROR_WARNING | RC_KAL_EMPTY);
               iRetcode = RC_KAL_EMPTY;
            break;

            case NU_TIMEOUT:
               /* Request timed out */
               /* !! We suspend forever. how did we get this? !! */
               trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
                  "mem_alloc_ex. Request Timeout. KAL Id=%x\n",
                     pool_id);
               error_log(ERROR_WARNING | RC_KAL_TIMEOUT);
               iRetcode = RC_KAL_TIMEOUT;
            break;

            case NU_POOL_DELETED:
               /* Pool was deleted while task was waiting */
               trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
                  "mem_alloc_ex. Pool Deleted. KAL Id=%x\n",
                     pool_id);
               error_log(ERROR_WARNING | RC_KAL_DESTROYED);
               iRetcode = RC_KAL_DESTROYED;
            break;

            default:
               /* Unknown N+ error */
               trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
                  "mem_alloc_ex. Unknown N+ Error=%x. KAL Id=%x\n",
                     iRetcode, pool_id);
               error_log(ERROR_WARNING | RC_KAL_SYSERR);
               iRetcode = RC_KAL_SYSERR;
         }
#else /* NU_DETAILED_DEBUG */
         /* Error Sending message */
         trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
            "mem_alloc_ex. N+ Error=%x. KAL Id=%x\n",
               iRetcode, pool_id);
         error_log(ERROR_WARNING | RC_KAL_SYSERR);
         iRetcode = RC_KAL_SYSERR;
#endif /* NU_DETAILED_DEBUG */
      }
   }
   else
   {
      /* Invalid Pool Id */
      sem_put(kal_pool_sem);
      error_log(ERROR_WARNING | RC_KAL_INVALID);
      iRetcode = RC_KAL_INVALID;
   }
   #ifdef MEM_TRACE
   if(iRetcode == NU_SUCCESS)
   {
	   TotalMallocMemeory+=size;
	   trace("--->Malloc Total %d Memory malloc\n",TotalMallocMemeory);
	   {
		int rtn=0;
		rtn=GetFreeMemLogId();
		if(rtn<1024)
		{
			MemoryLogSize[rtn]=size;
			MemoryLogPtr[rtn]=(u_int32)(*pptr);
		}else
		{
			trace("--->MEM Log pool full\n");
		}
	   }
   }
   #endif
   return(iRetcode);
}

/********************************************************************/
/* Return a previously allocated block of memory from a Memory Pool */
/********************************************************************/
int mem_free_ex(pool_id_t pool_id, void *ptr)
{
   int            iIndex;
   int            iRetcode;
   NU_MEMORY_POOL *pool_rtos_id;
   
   not_from_critical_section();
   not_interrupt_safe();

   /* Check for bad input */
   if(ptr == (void *)NULL)
   {
      error_log(ERROR_WARNING | RC_KAL_BADPTR);
      return(RC_KAL_BADPTR);
   }

   /* Get serialisation semaphore. */
   if(sem_get(kal_pool_sem, KAL_WAIT_FOREVER) != RC_OK)
   {
      error_log(ERROR_WARNING | RC_KAL_SYSERR);
      return(RC_KAL_SYSERR);
   }

   iIndex = INDEX_FROM_MPID(pool_id);
   if((IS_VALID_MPID(pool_id)) &&
      (mem_pool[iIndex].flags & POOL_FLAG_STRUCT_IN_USE))
   {
      /* Get the Pool RTOS Id */
      pool_rtos_id = (NU_MEMORY_POOL *)mem_pool[iIndex].rtos_id;

      /* Return the serialization semaphore */
      sem_put(kal_pool_sem);

      /* Free the memory */
      iRetcode = NU_Deallocate_Memory((VOID *)ptr);
      if(iRetcode != NU_SUCCESS)
      {
#ifdef NU_DETAILED_DEBUG
         switch(iRetcode)
         {
            case NU_INVALID_POINTER:
               /* Mem pointer is Invalid */
               trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
                  "mem_free_ex. Invalid Mem Ptr. KAL Id=%x, Ptr=%x\n",
                     pool_id, ptr);
               error_log(ERROR_WARNING | RC_KAL_INVALID);
               iRetcode = RC_KAL_INVALID;
            break;

            default:
               /* Unknown N+ error */
               trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
                  "mem_free_ex. Unknown N+ Error=%x. KAL Id=%x\n",
                     iRetcode, pool_id);
               error_log(ERROR_WARNING | RC_KAL_SYSERR);
               iRetcode = RC_KAL_SYSERR;
         }
#else /* NU_DETAILED_DEBUG */
         /* Error Sending message */
         trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
            "mem_free_ex. N+ Error=%x. KAL Id=%x\n",
               iRetcode, pool_id);
         error_log(ERROR_WARNING | RC_KAL_SYSERR);
         iRetcode = RC_KAL_SYSERR;
#endif /* NU_DETAILED_DEBUG */
      }
   }
   else
   {
      /* Invalid Pool Id */
      sem_put(kal_pool_sem);
      error_log(ERROR_WARNING | RC_KAL_INVALID);
      iRetcode = RC_KAL_INVALID;
   }
#ifdef MEM_TRACE   
{
	int i=0;
	for(i=0;i<1024;i++)
	{
		if(MemoryLogPtr[i]==(u_int32)ptr)
		{
			MemoryLogPtr[i]=0;
			TotalMallocMemeory-=MemoryLogSize[i];
			MemoryLogSize[i]=0;
			trace("--->Free Total %d Memory malloc\n",TotalMallocMemeory);
		}
	}

   }
#endif
   #ifdef DEBUG
   dwNumNcFree++;
   #endif

   return(iRetcode);
}

/***********************************************************************/
/* Calculate the size of the largest contiguous free block of memory   */
/* This is only currently used by the OpenTV mapping layer to set up   */
/* the direct segment. Alter size and granularity values appropriately */
/* for your system.                                                    */
/***********************************************************************/
u_int32 mem_inquire_largest_block(void)
{
   SIGNED   iLoop;
   void     *pBuffer;
   STATUS   iRetcode;
   
   not_from_critical_section();

   isr_trace_new(TRACE_KAL | TRACE_LEVEL_2, "Finding largest memory\n", 0, 0);

   for(iLoop = (RAMSize - HWBUF_TOP) & BLOCK_SEARCH_MASK;
      iLoop > 0;                                   
      iLoop -= BLOCK_SEARCH_GRANULARITY)
   {
      /* Try to allocate the memory */
      iRetcode = NU_Allocate_Memory(&gSystemMemory, 
                                    &pBuffer, 
                                    (UNSIGNED)iLoop,
                                    NU_NO_SUSPEND);
      if(iRetcode == NU_SUCCESS)
      {
         /* We got the memory. Free it up and calculate the size we can use */
         iRetcode = NU_Deallocate_Memory(pBuffer);
         isr_trace_new(TRACE_KAL | TRACE_LEVEL_4, 
            "Largest block available is 0x%08x bytes\n", iLoop, 0);
         return(iLoop);
      }
   }

   isr_trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
      "Can't find any free block >= 0x%X bytes!\n", 
         BLOCK_SEARCH_GRANULARITY, 0);

   return(0);
}

/********************************************************************/
/* Get a chunk of memory directly from the underlying RTOS. This is */
/* designed for clients who want to set up and manage their own     */
/* heaps                                                            */
/********************************************************************/
void *mem_get_os_segment(u_int32 uSize)
{
   int iRetcode;
   void *pMem;

   not_from_critical_section();

   /* Get a segment from the N+ System Heap */
   iRetcode = mem_malloc_ex(KAL_SYS_MEM_POOL_ID, uSize, &pMem);
   if(iRetcode != RC_OK)
   {
#ifdef DEBUG
      isr_trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
         "Error: %x allocating 0x%X byte RTOS segment\n", iRetcode, uSize);
      isr_error_log(ERROR_WARNING | RC_KAL_SYSERR);
#endif
      return((void *)NULL);
   }

   return(pMem);
}

/******************************************************************/
/* Release a block of memory previously allocated using a call to */
/* mem_get_os_segment.                                            */
/******************************************************************/
void mem_release_os_segment(void *ptrSegment)
{
   int iRetcode;
   
   not_from_critical_section();

   /* Check for bad input */
   if(ptrSegment == (void *)NULL)
   {
#ifdef DEBUG
      isr_error_log(ERROR_WARNING | RC_KAL_BADPTR);
#endif
      return;
   }

   /* Return the segment */
   iRetcode = mem_free_ex(KAL_SYS_MEM_POOL_ID, ptrSegment);
#ifdef DEBUG
   if(iRetcode != RC_OK)
   {
      isr_trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
         "Error: %x Returning byte RTOS segment 0x%X.\n", 
            iRetcode, (u_int32)ptrSegment);
      isr_error_log(ERROR_WARNING | RC_KAL_SYSERR);
   }
#endif

   return;
}

/*******************/
/*******************/
/* Event functions */
/*******************/
/*******************/

/************************************************/
/* Allocate one of this task's available events */
/************************************************/
events_t event_create(void)
{
   int iIndex, iRetcode;
   task_id_t dwOurTid;
   u_int32 uBitNum;
   events_t eRetcode = (events_t)0;

   not_from_critical_section();
   not_interrupt_safe();

   /* Get our task ID */
   dwOurTid = task_id();
   if(!IS_VALID_PID(dwOurTid))
   {
      /* Error ! - we can't get our own PID ! */
      error_log(ERROR_FATAL | RC_KAL_SYSERR);
      return((events_t)NULL);
   }
   else
   {
      iIndex = INDEX_FROM_PID(dwOurTid);

      /* Get the process table access lock */
      if(sem_get(kal_proc_sem, KAL_WAIT_FOREVER) != RC_OK)
      {
         error_log(ERROR_WARNING | RC_KAL_SYSERR);
         return((events_t)NULL);
      }

      /* Has an event been created for this task? */
      if(processes[iIndex].pEvent == (NU_EVENT_GROUP *)NULL)
      {
         /* Create an Event group for this task */
         iRetcode = NU_Create_Event_Group(&NU_Evt_CtrlBlk[iIndex], NULL);
         if(iRetcode == NU_SUCCESS)
         {
            processes[iIndex].pEvent = &NU_Evt_CtrlBlk[iIndex];
         }
         else
         {
            /* Return the serialization semaphore */
            sem_put(kal_proc_sem);

            /* Output the failure message */
#ifdef NU_DETAILED_DEBUG
            switch(iRetcode)
            {
               case NU_INVALID_GROUP:
                  /* CtrlBlk Ptr NULL or CtrBlk aready in use */
               trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
                  "Invalid Evt CtrlBlk! Index=%d\n", iIndex);
               error_log(ERROR_WARNING | RC_KAL_INVALID);
               break;

               default:
                  trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
                     "Unknown N+ Evt Create Error! Error=%x\n", 
                        iRetcode);
                  error_log(ERROR_WARNING | RC_KAL_SYSERR);
            }
#else /* NU_DETAILED_DEBUG */
            /* Error ! Can't create RTOS queue */
            trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, 
               "Can't create Event! RC=%x\n", iRetcode);
            error_log(ERROR_WARNING | RC_KAL_SYSERR);
#endif /* NU_DETAILED_DEBUG */

            return((events_t)NULL);
         }
      }

      /* Find a free event */
      uBitNum = find_lowest_bit((u_int32)(processes[iIndex].free_events));
      if(uBitNum == (u_int32)-1)
      {
         /* No events are available - return an error */
         error_log(ERROR_WARNING | RC_KAL_NORESOURCES);
      }
      else
      {
         /* We got an event. Mark it as used and give it to the caller */
         eRetcode = (events_t)(1 << uBitNum);
         processes[iIndex].free_events &= ~(eRetcode);
      }

      /* Release the access lock */
      sem_put(kal_proc_sem);
   }

   return(eRetcode);
}

/*************************************/
/* Free a previously allocated event */
/*************************************/
int event_destroy(events_t pEvents)
{
   int       iIndex;
   int       iRetcode;
   task_id_t dwOurTid;

   not_from_critical_section();
   not_interrupt_safe();

   /* Get our task ID */
   dwOurTid = task_id();
   if(!IS_VALID_PID(dwOurTid))
   {
      /* Error ! - we can't get our own PID ! */
      error_log(ERROR_FATAL | RC_KAL_SYSERR);
      return(RC_KAL_SYSERR);
   }
   else
   {
      iIndex = INDEX_FROM_PID(dwOurTid);

      /* Get the process table access lock */
      iRetcode = sem_get(kal_proc_sem, KAL_WAIT_FOREVER);
      if(iRetcode == RC_OK)
      {
         /* Mark the events passed as free. NB: Freeing unallocated */
         /* events is not considered an error.                      */
         processes[iIndex].free_events |= pEvents;

         /* Release the access lock */
         sem_put(kal_proc_sem);
      }
      else
      {
         error_log(ERROR_WARNING | iRetcode);
      }
   }

   return(iRetcode);
}

/********************************************/
/* Wait for at least one of a set of events */
/********************************************/
int event_receive(events_t eEvents, events_t *pReceived, 
                                          u_int32 timeout_ms, bool bWaitAll)
{
   OPTION NumEventsWait;
   UNSIGNED dwTimeoutTicks;
   task_id_t dwOurTid;
   int iIndex, iRetcode;
   NU_EVENT_GROUP *evt_rtos_id;

   not_from_critical_section();
   not_interrupt_safe();

   /* Was a valid pointer passed? */
   if(pReceived == (events_t *)NULL)
   {
      error_log(ERROR_WARNING | RC_KAL_INVALID);
      return(RC_KAL_INVALID);
   }

   /* Identify ourselves */
   dwOurTid = task_id();
   if(!IS_VALID_PID(dwOurTid))
   {
      /* Error ! - we can't get our own PID ! */
      error_log(ERROR_FATAL | RC_KAL_SYSERR);
      return(RC_KAL_SYSERR);
   }
   else
   {
      iIndex = INDEX_FROM_PID(dwOurTid);

      /* Get the process table access lock */
      iRetcode = sem_get(kal_proc_sem, KAL_WAIT_FOREVER);
      if(iRetcode == RC_OK)
      {
         /* Check that events passed are allocated */
         if(processes[iIndex].free_events & eEvents)
         {
            iRetcode = RC_KAL_NOTSET;
         }

         /* Get the Event's RTOS Id */
         evt_rtos_id = (NU_EVENT_GROUP *)processes[iIndex].pEvent;

         /* Release the access lock */
         sem_put(kal_proc_sem);

         /* If any of the requested events were not allocated */
         /* return an error.                                  */
         if(iRetcode == RC_KAL_NOTSET)
         {
            error_log(ERROR_WARNING | RC_KAL_NOTSET);
            return(iRetcode);
         }
      }
      else
      {
         /* Error ! - we can't get the process semaphore ! */
         error_log(ERROR_WARNING | RC_KAL_SYSERR);
         return(RC_KAL_SYSERR);
      }
   }

   /* How many of the Events to wait for ? */
   NumEventsWait = (bWaitAll) ? NU_AND_CONSUME : NU_OR_CONSUME;

   /* Translate the suspend parameter */
   if(timeout_ms == KAL_WAIT_FOREVER)
   {
      dwTimeoutTicks = NU_SUSPEND;
   }
   else if(timeout_ms == 0)
   {
      dwTimeoutTicks = NU_NO_SUSPEND;
   }
   else
   {
      /* Convert the specified time (milliSecs) to equivalent ticks */
      dwTimeoutTicks = (timeout_ms * KAL_TICKS2SEC)/1000;
      if(dwTimeoutTicks == 0)
      {
         dwTimeoutTicks++;
      }
   }

   /* Wait for the events */
   iRetcode = NU_Retrieve_Events(evt_rtos_id,
                                 (UNSIGNED)eEvents,
                                 NumEventsWait,
                                 (UNSIGNED *)pReceived,
                                 dwTimeoutTicks);
   switch(iRetcode)
   {
      case NU_SUCCESS:
         /* N+ returns all logged events. So return only the requested events.
            This is safe as N+ clears only the requested events */
            *pReceived &= eEvents;
      break;

#ifdef NU_DETAILED_DEBUG
      case NU_INVALID_GROUP:
         /* Evt pointer is invalid */
         error_log(ERROR_WARNING | RC_KAL_INVALID);
         iRetcode = RC_KAL_INVALID;
      break;

      case NU_INVALID_POINTER:
         /* Retrieved Event pointer is Invalid */
         error_log(ERROR_WARNING | RC_KAL_INVALID);
         iRetcode = RC_KAL_INVALID;
      break;

      case NU_INVALID_OPERATION:
         /* Operation(NumEventsWait) paramter Invalid */
         error_log(ERROR_WARNING | RC_KAL_INVALID);
         iRetcode = RC_KAL_INVALID;
      break;

      case NU_INVALID_SUSPEND:
         /* Invalid Suspend Context (aka ISR?) */
         error_log(ERROR_WARNING | RC_KAL_INVALID);
         iRetcode = RC_KAL_INVALID;
      break;

      case NU_NOT_PRESENT:
         /* Requested Event(s) not present */
         error_log(ERROR_WARNING | RC_KAL_EMPTY);
         iRetcode = RC_KAL_NOTSET;
      break;

      case NU_TIMEOUT:
         /* Evt Request timed out */
         //error_log(ERROR_WARNING | RC_KAL_TIMEOUT);
         iRetcode = RC_KAL_TIMEOUT;
      break;

      case NU_GROUP_DELETED:
         /* Evt Group was deleted while task was waiting */
         error_log(ERROR_WARNING | RC_KAL_DESTROYED);
         iRetcode = RC_KAL_DESTROYED;
      break;
#endif /* NU_DETAILED_DEBUG */

      default:
         error_log(ERROR_WARNING | RC_KAL_SYSERR);
         iRetcode = RC_KAL_SYSERR;
   }

   return(iRetcode);
}

/**********************************/
/* Set an event in the given task */
/**********************************/
int event_send(task_id_t target_task, events_t eEvents)
{
   int iIndex, iRetcode;
   bool cs;

   not_from_critical_section();

   /* If called with a NULL event */ 
   if(eEvents == (events_t)NULL) 
   {
      isr_error_log(ERROR_WARNING | RC_KAL_NOTSET);
      return(RC_KAL_NOTSET);
   }

   /* Check parameter validity */
   if(!IS_VALID_PID(target_task))
   {
      /* Error ! - invalid process ID passed ! */
      isr_error_log(ERROR_WARNING | RC_KAL_INVALID);
      return(RC_KAL_INVALID);
   }

   iIndex = INDEX_FROM_PID(target_task);

   /* Lock access to the process table */
   cs = critical_section_begin();

   /* Check that events passed are allocated */
   if(processes[iIndex].free_events & eEvents)
   {
      iRetcode = RC_KAL_NOTSET;
   }
   else
   {
      /* Send the required events */
      iRetcode = NU_Set_Events((NU_EVENT_GROUP *)processes[iIndex].pEvent, 
                               eEvents, 
                               NU_OR);
      if(iRetcode != NU_SUCCESS)
      {
#ifdef NU_DETAILED_DEBUG
         switch(iRetcode)
         {
            case NU_INVALID_GROUP:
               /* Evt pointer is invalid */
               isr_error_log(ERROR_WARNING | RC_KAL_INVALID);
               iRetcode = RC_KAL_INVALID;
            break;

            case NU_INVALID_OPERATION:
               /* Operation paramter Invalid */
               isr_error_log(ERROR_WARNING | RC_KAL_INVALID);
               iRetcode = RC_KAL_INVALID;
            break;

            default:
               /* Unknown N+ error */
               isr_error_log(ERROR_WARNING | RC_KAL_SYSERR);
               iRetcode = RC_KAL_SYSERR;
         }
#else /* NU_DETAILED_DEBUG */
         /* Error Sending message */
         isr_error_log(ERROR_WARNING | RC_KAL_SYSERR);
         iRetcode = RC_KAL_SYSERR;
#endif /* NU_DETAILED_DEBUG */
      }
   }

   /* Release the access lock */
   critical_section_end(cs);

   return(iRetcode);
}

/****************************************************/
/****************************************************/
/** RTOS-dependent functions used in trace logging **/
/****************************************************/
/****************************************************/
 
#if SC_PNA == YES
/************************************************************************/
/* telnet server task                                                   */
/*                                                                      */
/* This task  runs in the background waiting for a                      */
/* connection to the telnet port, then sets up a socket for trace       */
/* messages to go out.                                                  */
/************************************************************************/
#define SERVER_PORT     23
#define BUFFLEN         80
#define TCP_CHECK_DELAY 1000

void telnet_server_task(void *parm)
{
   int sock;
   int ChildSock;
   struct sockaddr_in our_addr;
   struct sockaddr_in their_addr;
   int addr_len;
   char buff[BUFFLEN];
   int nbytes;
   int i;
   unsigned long TelnetTraceTID = 0;
   int iIndex;

   trace_new( TRACE_KAL | TRACE_LEVEL_ALWAYS, "----------------------------------------\n");

   sock = socket(AF_INET, SOCK_STREAM, TCP);

   if( sock < 0)
   {
      trace_new( TRACE_KAL | TRACE_LEVEL_ALWAYS, "TCP: Could not open socket!\n");
      trace_new( TRACE_KAL | TRACE_LEVEL_ALWAYS, "TCP: errno = 0x%X\n", errno);
      task_terminate();
   }
   trace_new( TRACE_KAL | TRACE_LEVEL_ALWAYS, "Opened socket.\n");


   our_addr.sin_family = AF_INET; /* must be AF_INET */
   our_addr.sin_port = htons(SERVER_PORT); /* 16-bit port number */
   our_addr.sin_addr.s_addr = htonl(SysVars.Lan1IP); /* 32-bit IP address */

   for(i = 0; i < 8; i++)
   {
      our_addr.sin_zero[i] = 0;           /* must be 0 */
   }

   trace_new( TRACE_KAL | TRACE_LEVEL_ALWAYS, "----------------------------------------\n");

   if(bind(sock, &our_addr, sizeof(our_addr)) == -1)
   {
      trace_new( TRACE_KAL | TRACE_LEVEL_ALWAYS, "TCP: Could not bind() socket!\n");
      trace_new( TRACE_KAL | TRACE_LEVEL_ALWAYS, "TCP: errno = 0x%X\n", errno);
      task_terminate();
   }
   trace_new( TRACE_KAL | TRACE_LEVEL_ALWAYS, "TCP: Bound socket.\n");

   trace_new( TRACE_KAL | TRACE_LEVEL_ALWAYS, "----------------------------------------\n");

   if (listen(sock, 5) == -1)
   {
      trace_new( TRACE_KAL | TRACE_LEVEL_ALWAYS, "TCP: Could not listen() to socket!\n");
      trace_new( TRACE_KAL | TRACE_LEVEL_ALWAYS, "TCP: errno = 0x%X\n", errno);
      task_terminate();
   }
   trace_new( TRACE_KAL | TRACE_LEVEL_ALWAYS, "TCP: Listening to socket.\n");
   trace_new( TRACE_KAL | TRACE_LEVEL_ALWAYS, "----------------------------------------\n");

   /* Convert the KAL PID to a PSOS TID */
   iIndex = INDEX_FROM_PID(TelnetTracePID);
   TelnetTraceTID = processes[iIndex].psos_pid;

   while(1)
   {
      ChildSock = accept(sock, &their_addr, &addr_len);
      if (ChildSock == -1)
      {
         trace_new( TRACE_KAL | TRACE_LEVEL_ALWAYS, "TCP: Could not accept() socket!\n");
         trace_new( TRACE_KAL | TRACE_LEVEL_ALWAYS, "TCP: errno = 0x%X\n", errno);
         task_terminate();
      }
      trace_new( TRACE_KAL | TRACE_LEVEL_ALWAYS, "TCP: Accepted socket.\n");

      sdTraceSocket = shr_socket(ChildSock, TelnetTraceTID);
      if (sdTraceSocket == -1)
      {
         trace_new( TRACE_KAL | TRACE_LEVEL_ALWAYS, "TCP: Could not share socket!\n");
         trace_new( TRACE_KAL | TRACE_LEVEL_ALWAYS, "TCP: errno = 0x%X\n", errno);
         task_terminate();
      }
      trace_new( TRACE_KAL | TRACE_LEVEL_ALWAYS, "TCP: Now sharing socket.\n");

      trace_new( TRACE_KAL | TRACE_LEVEL_ALWAYS, "----------------------------------------\n");
      do
      {
         task_time_sleep(TCP_CHECK_DELAY);
         nbytes = recv(ChildSock, buff, BUFFLEN, 0);
         trace_new( TRACE_KAL | TRACE_LEVEL_ALWAYS, "TCP: recv() returned %d.\n", nbytes);
      } while (nbytes >= 0);
      trace_new( TRACE_KAL | TRACE_LEVEL_ALWAYS, "TCP: errno = 0x%X\n", errno);
      /* Expecting ECONNRESET 0x5036 The connection has been reset  by the peer */
      close(ChildSock);
      ChildSock = -1;
   }

   trace_new( TRACE_KAL | TRACE_LEVEL_ALWAYS, "TCP: Exiting.\n");
   task_terminate();
}
#endif

/************************************************************************************/
/* Dump a trace message to the debug port (low level entry point - not for app use) */
/************************************************************************************/
void traceout(char *string, ...)
{
   va_list arg;
#if SC_PNA == YES
   char buff[LC_SSIZE];
#endif

   va_start(arg, string);

#if SC_PNA == YES
   if(sdTraceSocket != -1)
   {
      vsprintf(buff, string, arg);
      TraceOutTelnet(buff);
   } else
   {
      vprintf(string, arg);
   }
#else
   vprintf(string, arg);
#endif

   va_end(arg);
}

/*************************************************/
/* Dump a trace message via telnet to another PC */
/*************************************************/
#if SC_PNA == YES
void TraceOutTelnet(char *string)
{
   char buff[LC_SSIZE];
   int i;
   int j;

   j = 0;
   for(i=0; i <= strlen(string); i++)
   {
      if(string[i] != 0x0A)
      {
         buff[j++] = string[i];
      } else
      {
         buff[j++] = 0x0D;
         buff[j++] = 0x0A;
      }
   }
   if(send(sdTraceSocket, buff,
      strlen(buff), 0) == -1)
   {
      close(sdTraceSocket);
      sdTraceSocket = -1;
   }
}
#else
void TraceOutTelnet(char *string)
{
  /*  Do nothing - no networking code in the build. */
}
#endif

/*********************************/
/*********************************/
/** Interrupt Service Functions **/
/*********************************/
/*********************************/

void nup_isr_mask_unmask (u_int32 uWork)
{
   u_int32 ks;
   LPREG pic_int_mask;
   pic_int_mask = (LPREG)ITC_INTENABLE_REG;

   ks = critical_section_begin();


   IntMask = IntMask & ~((u_int32)0x00000001 << uWork);

   /* gInterrupt_State contains a bitmap of which interrupts are
      enabled or disabled on the central PIC chip.  If it is disabled
      it will mask off the interrupt in uWork.  This prevents us
      from turning back on an interrupt that was turned off during
      in its handler */

   /* This enable_pic_interrupt does (roughly) what is done below.
      We shouldn't have to worry about anything other than the main
      PIC since that is the only place we disable interrupts
      in NUP's IRQ handler
      enable_pic_interrupt(uWork & gInterrupt_State); */

    *pic_int_mask |= (gInterrupt_State & ((u_int32)0x00000001 << uWork));

     critical_section_end(ks);
}

/**************************************************************************/
/* N+ Application Entry point. Remap it to our own function to keep all   */
/* apps RTOS independent.                                                 */
/**************************************************************************/
VOID root(UNSIGNED argc, VOID *argv)
{

   /* Call our own app start API */
   application_entry();

   do{
      /* Suspend the task */
      if(NU_Suspend_Task(&gRootTask)!=NU_SUCCESS)
      {
         /* Put something out for debug as we dont use the process wrapper */
         trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, "Could not suspend Root Task\n");
         error_log(ERROR_FATAL | RC_KAL_SYSERR);
      }
   }
   while(1); /* Dont let the task get away! :) */

   return;
}

/********************************/
/********************************/
/** Fall over and die horribly **/
/********************************/
/********************************/

/*********************************************************/
/* Fatal exit - tell N+ to shut down (or enter debugger) */
/*********************************************************/
void fatal_exit(u_int32 cosmic_constant)
{
   flush_trace_on_abort();
   #ifdef DEBUG
   printf("Call made to fatal_exit ! 0x%08lx\n", cosmic_constant);
   #endif
   #ifdef DRIVER_INCL_FPLEDS
   if (LEDIsMatrixDisplay())
   {
     LEDClearDisplay();
     LEDStringAt("*** Fatal error! ***", 0, 0);
     LEDStringAt("Code: ", 0, 1);
     LEDLongIntAt(cosmic_constant, TRUE, 6, 1);
     LEDStringAt("********************", 0, 3);
   }
   else
   {
     LEDString("Err ");
   }  
   #endif

   #ifdef DEBUG

   /* In the debug version, we stop everything dead but don't reboot. */
   /* This allows someone who is debugging to get into the system and */
   /* try to figure out what happened. We turn off interrupts then    */
   /* sit in a tight loop.                                            */

   critical_section_begin();
   while(1);

   #else

   /* In the release version, fatal exit will reboot the IRD. */
   reboot_IRD();

   #endif
}


/****************************************************************************
 * Modifications:
 * $Log: 
 *  62   mpeg      1.61        4/15/04 5:31:29 PM     Dave Wilson     CR(s) 
 *        8878 8879 : task_create_ex previously had two problems:
 *        1. It would dereference the name pointer prior to checking to see if 
 *        it is NULL. NULL is a perfectly valid value for the parameter.
 *        2. While checking for name clashes, the entire processes list was not
 *         checked. Only processed up to the first unused descriptor were 
 *        checked, thus leaving the door open to attempts to create a new task 
 *        while another already has the same name.
 *  61   mpeg      1.60        10/24/03 2:41:21 PM    Xin Golden      CR(s): 
 *        7714 initialize nc_ptr to NULL and return NULL pointer when size is 0
 *         in mem_nc_malloc.
 *  60   mpeg      1.59        10/22/03 4:01:50 AM    Dave Wilson     CR(s): 
 *        7684 
 *        Added  appropriate ifdefs around isr_trace_new and isr_error_log in 
 *        task_id to prevent a spurious error message in cases where the KAL is
 *         compiled to allow mixed KAL/native RTOS calls. Previously, if 
 *        task_id was called from a task which had been created using a native 
 *        NUP call, you would see an error reported but this is, in fact, 
 *        completely legal and handled within the KAL.
 *  59   mpeg      1.58        7/28/03 5:09:24 PM     Senthil Veluswamy SCR(s) 
 *        5404 :
 *        Fixed typo in passing task argc and argv. Now correct params are 
 *        passed to the newly created task. 
 *        
 *  58   mpeg      1.57        7/22/03 6:03:54 PM     Senthil Veluswamy SCR(s) 
 *        6971 :
 *        1)Wrapped fpleds.h to be used only if FPLEDS is in the build, 
 *        2) Exported required symbols from ..\hwlib\critsec.h and remove the 
 *        header include.
 *        
 *  57   mpeg      1.56        7/14/03 2:37:12 PM     Senthil Veluswamy SCR(s) 
 *        5354 :
 *        updated the sem_create_ex and qu_create_ex prototypes in nupkal.c
 *        
 *  56   mpeg      1.55        7/14/03 12:31:48 PM    Senthil Veluswamy SCR(s) 
 *        5354 :
 *        Changed initial_value to unsigned for sem_create and num_queues for 
 *        qu_create to disallow negative initial values (as do the RTOSes).
 *        
 *  55   mpeg      1.54        7/1/03 6:00:42 PM      Senthil Veluswamy SCR(s) 
 *        6862 :
 *        Added check for N+ Task Info call's return code before trying to add 
 *        native task to KAL table.
 *        
 *  54   mpeg      1.53        6/27/03 6:48:04 PM     Senthil Veluswamy SCR(s) 
 *        6862 :
 *        Modified task_id to create a KAL entry for a RTOS native task. 
 *        Changed stack_check_task to not check stack usage for native rtos 
 *        tasks.
 *        
 *  53   mpeg      1.52        6/3/03 5:59:12 PM      Dave Wilson     SCR(s) 
 *        6647 :
 *        
 *        
 *        
 *        
 *        
 *        Added calls to not_from_critical_section from all functions which 
 *        call the 
 *        underlying RTOS. Also tightened up the use of not_interrupt_safe to 
 *        catch
 *        blocking calls to functions which have a non-blocking form too.
 *        
 *  52   mpeg      1.51        5/21/03 12:30:20 PM    Dave Wilson     SCR(s) 
 *        6520 6521 :
 *        Removed critical section around cache flushing code for ARM920 inside
 *        mem_nc_malloc(_ex). This is not required and was causing interrupt 
 *        latencies
 *        up to 200mS in some cases!
 *        
 *  51   mpeg      1.50        4/30/03 5:03:14 PM     Billy Jackman   SCR(s) 
 *        6113 :
 *        Modified conditional compilation directives and code to add the 
 *        capability to
 *        detect the CPU type (920 vs. 940) at runtime when CPU_TYPE=AUTOSENSE.
 *        
 *  50   mpeg      1.49        4/11/03 10:29:08 AM    Matt Korte      SCR(s) 
 *        5998 :
 *        Changed Copyright string, added HW and SW config string pointers.
 *        
 *  49   mpeg      1.48        4/10/03 7:49:18 PM     Miles Bintz     SCR(s) 
 *        6006 :
 *        forced mem_nc_malloc to return memory that is cache aligned
 *        
 *  48   mpeg      1.47        3/4/03 4:52:24 PM      Senthil Veluswamy SCR(s) 
 *        5340 :
 *        Fixed typo that was invalidating check for a valid task id.
 *        
 *  47   mpeg      1.46        3/4/03 2:12:12 PM      Senthil Veluswamy SCR(s) 
 *        5340 :
 *        removed // comments
 *        
 *  46   mpeg      1.45        3/4/03 1:56:08 PM      Senthil Veluswamy SCR(s) 
 *        5417 :
 *        Added check for a qu_create_ex call with max elements 0.
 *        
 *  45   mpeg      1.44        2/13/03 1:52:46 PM     Bobby Bradford  SCR(s) 
 *        5479 :
 *        Fixed warnings ... moved "stbcfg.h" to beginning of list
 *        also changed <nup.h> to "nup.h"
 *        
 *  44   mpeg      1.43        2/13/03 12:44:50 PM    Matt Korte      SCR(s) 
 *        5479 :
 *        Removed old header reference
 *        
 *  43   mpeg      1.42        1/23/03 11:32:22 AM    Joe Kroesche    SCR(s) 
 *        5289 :
 *        fixed variable declaration error
 *        
 *  42   mpeg      1.41        1/22/03 9:35:00 AM     Matt Korte      SCR(s) 
 *        5258 :
 *        Removed unused variable.
 *        
 *        
 *  41   mpeg      1.40        1/16/03 11:45:38 AM    Dave Wilson     SCR(s) 
 *        5146 :
 *        >  The direct segment size is now determined using a software config 
 *        file
 *        parameter (OPENTV_DIRECT_HEAP_SIZE). If set to 0, operation is as 
 *        before and
 *        the direct heap occupies all available RAM after setting aside some 
 *        space for
 *        RTOS resources and stacks. In this case, the drivers allocate from 
 *        the direct
 *        heap. If set to a non-zero value, this specifies the number of KB to 
 *        allocate
 *        for the direct heap and also causes all drivers to allocate from the 
 *        main
 *        system heap rather than the direct heap.^D
 *        
 *        
 *  40   mpeg      1.39        1/9/03 10:08:00 AM     Billy Jackman   SCR(s) 
 *        5223 :
 *        Made declaration of variable use same condiitons as its use to avoid 
 *        warning.
 *        
 *  39   mpeg      1.38        12/20/02 9:53:50 PM    Tim Ross        SCR(s) 
 *        5206 :
 *        Modified mem_nc_malloc/mem_nc_free, and mem_malloc_ex/mem_free_ex to 
 *        properly
 *        flush & invalidate buffer & drain write posting buffer.
 *        
 *  38   mpeg      1.37        12/18/02 3:39:10 PM    Senthil Veluswamy SCR(s) 
 *        4785 :
 *        Changed the task_priority delta to size short to enable a better 
 *        range for task priority change.
 *        
 *  37   mpeg      1.36        10/29/02 5:33:14 PM    Dave Moore      SCR(s) 
 *        4833 :
 *        fixed N+ task priority calculation in task_create_ex and
 *        in task_priority.
 *        
 *        
 *  36   mpeg      1.35        10/24/02 11:39:56 AM   Ian Mitchell    SCR(s): 
 *        4828 
 *        Added extra return code to task_suspend.
 *        Added the ability to select if a new task is created in preemptive or
 *        non-preemptive modes.
 *        Changed function API to sem_put so there it returns a success code.
 *        
 *  35   mpeg      1.34        9/19/02 3:52:36 PM     Lucy C Allevato SCR(s) 
 *        4622 :
 *        assign the correct stack ptr in task_destroy so no memory leak there
 *        
 *  34   mpeg      1.33        9/3/02 4:43:34 PM      Senthil Veluswamy SCR(s) 
 *        4505 :
 *        Modified task_resume to return the process table serialization 
 *        semaphore before returning error if task is not suspended
 *        
 *  33   mpeg      1.32        8/22/02 5:04:24 PM     Senthil Veluswamy SCR(s) 
 *        3012 :
 *        Modified event_send to enable it to be called from an interrupt 
 *        context
 *        
 *  32   mpeg      1.31        7/24/02 2:35:26 PM     Senthil Veluswamy SCR(s) 
 *        4269 :
 *        Fixed typos: same name for Sem & Qu Control blocks, removed MOD_KAL 
 *        from error_log() calls
 *        
 *  31   mpeg      1.30        6/13/02 2:30:58 PM     Lucy C Allevato SCR(s) 
 *        3591 :
 *        modified sem_get in nucleus so it will match the API and pSOS.
 *        
 *  30   mpeg      1.29        6/11/02 7:05:24 AM     Ian Mitchell    SCR(s): 
 *        3956 
 *        Increased the ammount of memory allocated for semaphores and tasks if
 *         the build is for the MHP app
 *        
 *  29   mpeg      1.28        5/31/02 5:01:20 PM     Senthil Veluswamy SCR(s) 
 *        3885 :
 *        Fixed Warnings in Release builds
 *        
 *  28   mpeg      1.27        2/19/02 10:29:20 AM    Senthil Veluswamy SCR(s) 
 *        3013 :
 *        Replaced checks for EXTENDED_KAL with DRIVER_INCL_KALEXINC
 *        
 *  27   mpeg      1.26        2/15/02 4:35:36 PM     Senthil Veluswamy SCR(s) 
 *        3013 :
 *        This is the the new Stardard KAL (not using the pSOS Transkit). This 
 *        a drop from nupkalex\nupkalex.c (the part supporting the standard KAL
 *         interfaces)
 *        
 *  26   mpeg      1.25        12/12/01 3:47:30 PM    Billy Jackman   SCR(s) 
 *        2969 :
 *        Forgot to translate internal task id to psos task id.  Fixed.
 *        
 *  25   mpeg      1.24        12/12/01 2:56:12 PM    Billy Jackman   SCR(s) 
 *        2969 :
 *        Modified function task_priority to use the requested tsk id for the 
 *        priority
 *        function instead of the 0 value indicating current task.
 *        
 *  24   mpeg      1.23        7/18/01 2:25:02 PM     Bobby Bradford  SCR(s) 
 *        2093 :
 *        Added conditional support for ADS11 (modified the include statements 
 *        for std*.h from
 *        "std*.h" to <std*.h>
 *        
 *        
 *  23   mpeg      1.22        5/4/01 3:31:10 PM      Tim White       DCS#1824,
 *         DCS31825 -- Critical Section Overhaul
 *        
 *  22   mpeg      1.21        4/20/01 10:18:22 AM    Tim White       DCS#1687,
 *         DCS#1747, DCS#1748 - Add pSOS task switch prevention, move OS 
 *        specific
 *        functionality out of hwlib.c and into psoskal.c/nupkal.c, and update 
 *        critical
 *        section debugging function for both release and debug without using 
 *        assembly code.
 *        
 *  21   mpeg      1.20        4/16/01 12:52:42 PM    Miles Bintz     tracker 
 *        #1701:  added task_get_mask function, added task_time_sleep_ex
 *        
 *  20   mpeg      1.19        3/19/01 7:00:48 PM     Joe Kroesche    #1452 - 
 *        replaced a trace_new with isr_trace_new where it could be called
 *        from interrupt context
 *        
 *  19   mpeg      1.18        3/5/01 9:28:14 AM      Tim White       DCS#1374:
 *         Added task name to stack overflow message.
 *        
 *  18   mpeg      1.17        3/2/01 4:54:56 PM      Tim White       DCS#1366:
 *         Globally decrease stack memory usage.
 *        
 *  17   mpeg      1.16        3/1/01 2:16:34 PM      Senthil Veluswamy 
 *        DCS#1326 - Added semaphore protection to task_set_pointer() - Fix 
 *        Note 3.
 *        
 *  16   mpeg      1.15        3/1/01 8:48:20 AM      Tim White       DCS#1337:
 *         Implement a check stack size tool.
 *        
 *  15   mpeg      1.14        3/1/01 1:49:18 AM      Dave Wilson     DCS1317: 
 *        Added direct heap dumping code for memory debug
 *        
 *  14   mpeg      1.13        1/17/01 1:01:32 PM     Miles Bintz     tracker 
 *        #714.  Cleaned up warnings
 *        
 *  13   mpeg      1.12        12/14/00 1:35:00 PM    Angela          fixed 
 *        bugs in qu_receive and sem_delete
 *        
 *  12   mpeg      1.11        11/29/00 11:30:04 AM   Joe Kroesche    fixed bug
 *         in traceout that was goofing up stack
 *        
 *  11   mpeg      1.10        11/21/00 7:32:44 PM    Angela          added 
 *        cleanup task server and fixed tracker entry #709, 707,710,714,757
 *        
 *  10   mpeg      1.9         10/20/00 3:51:20 PM    QA - Roger Taylor Changed
 *         the return value type of task_priority() from "char" to "unsigned
 *        char".
 *        
 *  9    mpeg      1.8         10/8/00 5:20:08 PM     Dave Wilson     Fixed a 
 *        bug in the OpenTV case within mem_malloc
 *        
 *  8    mpeg      1.7         10/8/00 4:08:02 PM     Dave Wilson     Added C 
 *        runtime library hooks and removed calls not supported in embedded
 *        ARM library
 *        
 *  7    mpeg      1.6         10/2/00 9:26:20 PM     Miles Bintz     modified 
 *        memory functions to pull from region 0 (P2N_Memory in the transkit)
 *        
 *  6    mpeg      1.5         10/2/00 7:15:42 PM     Miles Bintz     fixed 
 *        some memory management bugs.  
 *        
 *  5    mpeg      1.4         10/2/00 5:00:02 PM     Miles Bintz     avoided 
 *        some task_terminate problems
 *        
 *  4    mpeg      1.3         8/29/00 8:46:02 PM     Miles Bintz     fixed 
 *        things
 *        
 *  3    mpeg      1.2         8/29/00 7:48:02 PM     Miles Bintz     removed 
 *        the psos includes
 *        
 *  2    mpeg      1.1         8/29/00 5:38:38 PM     Ismail Mustafa  Fixed 
 *        memory management.
 *        
 *  1    mpeg      1.0         8/25/00 4:30:16 PM     Ismail Mustafa  
 * $
 * 
 *    Rev 1.58   28 Jul 2003 16:09:24   velusws
 * SCR(s) 5404 :
 * Fixed typo in passing task argc and argv. Now correct params are passed to the newly created task. 
 * 
 *    Rev 1.57   22 Jul 2003 17:03:54   velusws
 * SCR(s) 6971 :
 * 1)Wrapped fpleds.h to be used only if FPLEDS is in the build, 
 * 2) Exported required symbols from ..\hwlib\critsec.h and remove the header include.
 * 
 *    Rev 1.56   14 Jul 2003 13:37:12   velusws
 * SCR(s) 5354 :
 * updated the sem_create_ex and qu_create_ex prototypes in nupkal.c
 * 
 *    Rev 1.55   14 Jul 2003 11:31:48   velusws
 * SCR(s) 5354 :
 * Changed initial_value to unsigned for sem_create and num_queues for qu_create to disallow negative initial values (as do the RTOSes).
 * 
 *    Rev 1.54   01 Jul 2003 17:00:42   velusws
 * SCR(s) 6862 :
 * Added check for N+ Task Info call's return code before trying to add native task to KAL table.
 * 
 *    Rev 1.53   27 Jun 2003 17:48:04   velusws
 * SCR(s) 6862 :
 * Modified task_id to create a KAL entry for a RTOS native task. Changed stack_check_task to not check stack usage for native rtos tasks.
 * 
 *    Rev 1.52   03 Jun 2003 16:59:12   dawilson
 * SCR(s) 6647 :
 * 
 * 
 * 
 * 
 * 
 * Added calls to not_from_critical_section from all functions which call the 
 * underlying RTOS. Also tightened up the use of not_interrupt_safe to catch
 * blocking calls to functions which have a non-blocking form too.
 * 
 *    Rev 1.51   21 May 2003 11:30:20   dawilson
 * SCR(s) 6520 6521 :
 * Removed critical section around cache flushing code for ARM920 inside
 * mem_nc_malloc(_ex). This is not required and was causing interrupt latencies
 * up to 200mS in some cases!
 * 
 *    Rev 1.50   30 Apr 2003 16:03:14   jackmaw
 * SCR(s) 6113 :
 * Modified conditional compilation directives and code to add the capability to
 * detect the CPU type (920 vs. 940) at runtime when CPU_TYPE=AUTOSENSE.
 * 
 *    Rev 1.49   11 Apr 2003 09:29:08   kortemw
 * SCR(s) 5998 :
 * Changed Copyright string, added HW and SW config string pointers.
 * 
 *    Rev 1.48   10 Apr 2003 18:49:18   bintzmf
 * SCR(s) 6006 :
 * forced mem_nc_malloc to return memory that is cache aligned
 * 
 *    Rev 1.47   04 Mar 2003 16:52:24   velusws
 * SCR(s) 5340 :
 * Fixed typo that was invalidating check for a valid task id.
 * 
 *    Rev 1.46   04 Mar 2003 14:12:12   velusws
 * SCR(s) 5340 :
 * removed // comments
 * 
 *    Rev 1.45   04 Mar 2003 13:56:08   velusws
 * SCR(s) 5417 :
 * Added check for a qu_create_ex call with max elements 0.
 * 
 *    Rev 1.44   13 Feb 2003 13:52:46   bradforw
 * SCR(s) 5479 :
 * Fixed warnings ... moved "stbcfg.h" to beginning of list
 * also changed <nup.h> to "nup.h"
 * 
 *    Rev 1.43   13 Feb 2003 12:44:50   kortemw
 * SCR(s) 5479 :
 * Removed old header reference
 * 
 *    Rev 1.42   23 Jan 2003 11:32:22   kroescjl
 * SCR(s) 5289 :
 * fixed variable declaration error
 * 
 *    Rev 1.41   22 Jan 2003 09:35:00   kortemw
 * SCR(s) 5258 :
 * Removed unused variable.
 * 
 * 
 *    Rev 1.40   16 Jan 2003 11:45:38   dawilson
 * SCR(s) 5146 :
 * >  The direct segment size is now determined using a software config file
 * parameter (OPENTV_DIRECT_HEAP_SIZE). If set to 0, operation is as before and
 * the direct heap occupies all available RAM after setting aside some space for
 * RTOS resources and stacks. In this case, the drivers allocate from the direct
 * heap. If set to a non-zero value, this specifies the number of KB to allocate
 * for the direct heap and also causes all drivers to allocate from the main
 * system heap rather than the direct heap.^D
 * 
 * 
 *    Rev 1.39   09 Jan 2003 10:08:00   jackmaw
 * SCR(s) 5223 :
 * Made declaration of variable use same condiitons as its use to avoid warning.
 * 
 *    Rev 1.38   20 Dec 2002 21:53:50   rossst
 * SCR(s) 5206 :
 * Modified mem_nc_malloc/mem_nc_free, and mem_malloc_ex/mem_free_ex to properly
 * flush & invalidate buffer & drain write posting buffer.
 * 
 *    Rev 1.37   18 Dec 2002 15:39:10   velusws
 * SCR(s) 4785 :
 * Changed the task_priority delta to size short to enable a better range for task priority change.
 * 
 *    Rev 1.36   29 Oct 2002 17:33:14   mooreda
 * SCR(s) 4833 :
 * fixed N+ task priority calculation in task_create_ex and
 * in task_priority.
 * 
 * 
 *    Rev 1.35   24 Oct 2002 10:39:56   mitchei
 * SCR(s): 4828 
 * Added extra return code to task_suspend.
 * Added the ability to select if a new task is created in preemptive or
 * non-preemptive modes.
 * Changed function API to sem_put so there it returns a success code.
 * 
 *    Rev 1.34   19 Sep 2002 14:52:36   goldenx
 * SCR(s) 4622 :
 * assign the correct stack ptr in task_destroy so no memory leak there
 * 
 *    Rev 1.33   03 Sep 2002 15:43:34   velusws
 * SCR(s) 4505 :
 * Modified task_resume to return the process table serialization semaphore before returning error if task is not suspended
 * 
 *    Rev 1.32   22 Aug 2002 16:04:24   velusws
 * SCR(s) 3012 :
 * Modified event_send to enable it to be called from an interrupt context
 * 
 *    Rev 1.31   24 Jul 2002 13:35:26   velusws
 * SCR(s) 4269 :
 * Fixed typos: same name for Sem & Qu Control blocks, removed MOD_KAL from error_log() calls
 * 
 *    Rev 1.30   13 Jun 2002 13:30:58   goldenx
 * SCR(s) 3591 :
 * modified sem_get in nucleus so it will match the API and pSOS.
 * 
 *    Rev 1.29   Jun 11 2002 07:05:24   mitchei
 * SCR(s): 3956 
 * Increased the ammount of memory allocated for semaphores and tasks if the build is for the MHP app
 * 
 *    Rev 1.28   31 May 2002 16:01:20   velusws
 * SCR(s) 3885 :
 * Fixed Warnings in Release builds
 * 
 *    Rev 1.27   19 Feb 2002 10:29:20   velusws
 * SCR(s) 3013 :
 * Replaced checks for EXTENDED_KAL with DRIVER_INCL_KALEXINC
 * 
 *    Rev 1.26   15 Feb 2002 16:35:36   velusws
 * SCR(s) 3013 :
 * This is the the new Stardard KAL (not using the pSOS Transkit). This a drop from nupkalex\nupkalex.c (the part supporting the standard KAL interfaces)
 *
 *
 ****************************************************************************/

