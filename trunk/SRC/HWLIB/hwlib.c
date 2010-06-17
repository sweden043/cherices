/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*          Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002          */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        HWLIB.C
 *
 *
 * Description:     Low level, RTOS independent hardware access functions
 *
 *
 * Author:          Dave Wilson
 *
 ****************************************************************************/
/* $Header: hwlib.c, 90, 5/6/04 5:25:35 PM, Miles Bintz$
 ****************************************************************************/

/*****************/
/* Include Files */
/*****************/
#include "stbcfg.h"
#include <stdio.h>
#include <string.h>
#include "basetype.h"
#include "retcodes.h"
#include "hwfuncs.h"
#include "hwlib.h"
#include "hwlibprv.h"
#define INSTANTIATE_TABLES
#include "hwlibint.h"
#include "critsec.h"
#include "gpioext.h"
#include "iic.h"
#ifdef DRIVER_INCL_TRACE
#include "trace.h"
#endif
#include "startup.h"

#if RTOS==NUP
extern void nup_isr_mask_unmask (u_int32 uWork);
#endif

/***********************************************************************************/
/* WATCHDOG is a software config key which defines whether we should turn on       */
/* or off the watchdog timer; It is usually turned on in relase builds and turned  */
/* off in debug builds, but it can be force on or off regardless of either build   */
/***********************************************************************************/
#if (WATCHDOG == DEB_OFF_REL_ON)
  //#ifndef DEBUG 
    #define WATCHDOG_TIMER_IS_ENABLED
  //#endif
#elif (WATCHDOG == FORCE_ON)
  #define WATCHDOG_TIMER_IS_ENABLED
#endif  

/* Watchdog will reset IRD if interrupts are disabled for twice the number of */
/* microseconds defined below. In this case this equates to 2 seconds.        */
#define WATCHDOG_TIMER_PERIOD 1000000

/******************************************/
/* Local Definitions and Global Variables */
/******************************************/
#define TICKS_PER_US (TICKS_PER_MS / 1000)

int     iProcessingInt = 0;
int     iCritSecCount  = 0;
u_int32 gSysTimerRate  = 0;
#define CRIT_SEC_STACK_SIZE     8
u_int32 crit_sec_stack[CRIT_SEC_STACK_SIZE];

/* gInterrupt_State is used in NUP to not enable interrupts that were
   disabled while the ISR ran. */

u_int32 gInterrupt_State = 0xffffffff;

static u_int32 uSysTickCount = 0;
static u_int32 uMsecPerTick = 0;
u_int32 gMaxTimers     = NUM_TIMERS;
#ifdef DEBUG
u_int32 uIntDebugCheck = 0;
u_int32 gdwIntCounts[NUM_INTS_PER_PIC];
#endif

systimer_client system_timer_clients[MAX_SYSTIMER_CLIENTS];

tick_descriptor ticks[NUM_TICKS];

timer_descriptor timers[NUM_TIMERS] =
{
   {0,
    0,
    INT_TIMER0,
    0,
    (PFNTIMERC)NULL,
    (PFNISR)NULL
   },
   {0,
    0,
    INT_TIMER1,
    0,
    (PFNTIMERC)NULL,
    (PFNISR)NULL
   },
   {0,
    0,
    INT_TIMER2,
    0,
    (PFNTIMERC)NULL,
    (PFNISR)NULL
   },
   {0,
    0,
    INT_TIMER3,
    0,
    (PFNTIMERC)NULL,
    (PFNISR)NULL
   },
   {0,
    0,
    INT_TIMER4,
    0,
    (PFNTIMERC)NULL,
    (PFNISR)NULL
   },
   {0,
    0,
    INT_TIMER5,
    0,
   (PFNTIMERC)NULL,
    (PFNISR)NULL
   },
   {0,
    0,
    INT_TIMER6,
    0,
    (PFNTIMERC)NULL,
    (PFNISR)NULL
   },
   {0,
    0,
    INT_TIMER7,
    0,
    (PFNTIMERC)NULL,
    (PFNISR)NULL
   }
};
void start_tick_counter(int iIndex);
void stop_tick_counter(int iIndex);
bool update_tick_counter_timer(void);

bool gbTickCounterRunning   = FALSE;
u_int32 gdwMsToNextTick     = 0;
u_int32 gdwCountsToNextTick = 0;
u_int32 gdwLastCorrection   = 0;

#ifdef DEBUG
#ifdef INT_LATENCY_CHECK
#define NUM_LATENCY_SAMPLES 1000
u_int32 gdwIntLatency[NUM_LATENCY_SAMPLES];
u_int32 gdwLongIntLatency   = 0;
#define NUM_LONGEST_LATENCY 10
u_int32 int_time_begin = 0, int_time_end = 0, int_time_diff = 0;
typedef struct
{
   u_int32 int_num;
   u_int32 int_latency;
} INT_LATENCY_T;
INT_LATENCY_T longestINTlatency[NUM_LONGEST_LATENCY] = {0,0};
static void int_latency_check( u_int32 dwIntID ); 
#endif  /* INT_LATENCY_CHECK */

/*******************************************/
/* Interrupt Service Routine timing option */
/*******************************************/
#ifdef ISR_TIMING_CHECK
#define LONGESTISRDURATION_SIZE 10
typedef struct
{
   u_int32 caller_add;
   u_int32 duration;
   u_int32 end;
   u_int32 id;
} ISR_TIMING_T;

ISR_TIMING_T longestISRduration[LONGESTISRDURATION_SIZE];

static void isr_timing_check(u_int32 pVector, u_int32 uIntID, u_int32 uStart, u_int32 uEnd);

#define ISR_TIMING_CHECK_LOCALS u_int32 uISRTimeStart;    \
                                u_int32 uISRTimeEnd;      \
                                u_int32 uISRTimePfn;      \
                                u_int32 uISRTimeID;
  
#define ISR_TIMING_CHECK_START(pfn, id)     if(trace_timer_value_addr)                 \
                                              uISRTimeStart = *trace_timer_value_addr; \
                                            else                                       \
                                              uISRTimeStart = 0;                       \
                                            uISRTimePfn   = (u_int32)(pfn);            \
                                            uISRTimeID    = (id);
                                            
#define ISR_TIMING_CHECK_END  if(trace_timer_value_addr)                 \
                                uISRTimeEnd = *trace_timer_value_addr;   \
                              else                                       \
                                uISRTimeEnd = 0;                         \
                              isr_timing_check(uISRTimePfn, uISRTimeID, uISRTimeStart, uISRTimeEnd);

#else
  #define ISR_TIMING_CHECK_LOCALS
  #define ISR_TIMING_CHECK_START(pfn, id)
  #define ISR_TIMING_CHECK_END
#endif /* ISR_TIMING_CHECK */

/**********************************/
/* Critical section timing option */
/**********************************/
#ifdef CRIT_SEC_LATENCY_CHECK
#define LONGESTCSDURATION_SIZE 10
u_int32 time_begin = 0, time_end = 0, time_diff = 0;
u_int32 caller = 0;
int from_int_handler = 0; /*set by int_handler in NUP_IRQ.c*/
int from_lowpowtask = 0;  /*set by LowPowTask in lowpow.c*/ 
typedef struct
{
   u_int32 caller_add;
   u_int32 duration;
} CRIT_SEC_LATENCY_T;
CRIT_SEC_LATENCY_T longestCSduration[LONGESTCSDURATION_SIZE] = {0,0};
#endif /* CRIT_SEC_LATENCY_CHECK */

#if defined( INT_LATENCY_CHECK ) || defined( CRIT_SEC_LATENCY_CHECK ) || defined(ISR_TIMING_CHECK)
extern volatile u_int32 *trace_timer_value_addr;
#endif 

#else
  /*************************************/
  /* Tidy things up for release builds */
  /*************************************/
  #define ISR_TIMING_CHECK_LOCALS
  #define ISR_TIMING_CHECK_START(pfn, id)
  #define ISR_TIMING_CHECK_END

#endif /* DEBUG */

int  kal_system_timer_isr(u_int32 dwIntID, bool bFIQ, PFNISR *pfnChain);
int  kal_tick_timer_isr(u_int32 dwIntID, bool bFIQ, PFNISR *pfnChain);
int  kal_watchdog_timer_isr(u_int32 dwIntID, bool bFIQ, PFNISR *pfnChain);
int  kal_general_timer_isr(u_int32 dwIntID, bool bFIQ, PFNISR *pfnChain);
int  kal_dummy_isr(u_int32 dwIntID, bool bFIQ, PFNISR *pfnChain);

#if (INTEXP_TYPE != NOT_PRESENT)
int  kal_exp_fanout_isr(u_int32 dwIntID, bool bFIQ, PFNISR *pfnChain);
PFNISR kal_exp_chain = NULL;

#if RTOS == NUP
extern void restore_exp_int_enable_bit ( u_int32 dwInt );
#endif

#endif

int  kal_gpio_fanout_isr(u_int32 dwIntID, bool bFIQ, PFNISR *pfnChain);
PFNISR kal_gpio_chain = NULL;

int  kal_timer_fanout_isr(u_int32 dwIntID, bool bFIQ, PFNISR *pfnChain);
PFNISR kal_timer_chain = NULL;

interrupt_vector vectors[NUM_INTS_PER_PIC];
interrupt_vector exp_vectors[NUM_INTS_PER_PIC];
interrupt_vector gpio_vectors[NUM_GPIO_INTS];

interrupt_vector exptimer_vectors[NUM_TIMER_INTS];
#define NUM_EXPTIMER_INTS NUM_TIMER_INTS

interrupt_vector *vector_lists[NUM_PICS] = {vectors, exp_vectors, exptimer_vectors, gpio_vectors};
int max_vectors[NUM_PICS]                = {NUM_INTS_PER_PIC, NUM_INTS_PER_PIC, NUM_EXPTIMER_INTS, NUM_GPIO_INTS};

u_int32 gdwNoSignalInts = 0;

/***********************/
/* External References */
/***********************/
extern unsigned long GetChipID(void);
extern unsigned long GetChipRev(void);
extern u_int32 get_active_exp_int(void);

/********************************/
/* Internal Function Prototypes */
/********************************/
void pfnTimerCompatCallback(timer_id_t timer, void *pUser);
void tick_timer_state_change(bool bInt, u_int32 dwCount, u_int32 dwNumMs);
static void clear_object_name(char *pName);

/***********************************************************************/
/* Clear an object name in an interrupt-safe manner (no runtime calls) */
/***********************************************************************/
static void clear_object_name(char *pName)
{
  int iLoop;
  
  /***************************************************************************/
  /* WARNING: Called from within a critical section! Do not add any function */
  /*          calls to this function body!!!!!                               */
  /***************************************************************************/
         
  for(iLoop = 0; iLoop < MAX_OBJ_NAME_LENGTH; iLoop++)
    pName[iLoop]=(char)0;
    
}
/********************************************************************/
/*  FUNCTION:     hwlib_int_initialise                              */
/*                                                                  */
/*  PARAMETERS:   None                                              */
/*                                                                  */
/*  DESCRIPTION:  Initalise interrupt handling                      */
/*                                                                  */
/*  RETURNS:      TRUE on success, FALSE otherwise.                 */
/*                                                                  */
/*  No semaphore protection required                                */
/*                                                                  */
/********************************************************************/
bool hwlib_int_initialise(void)
{
    u_int32 iLoop;
    int     iRetcode;
    static  bool bIntsInitialised = FALSE;

   /* Check for multiple calls */
   if(bIntsInitialised)
     return(TRUE);
   else
     bIntsInitialised = TRUE;

   #ifdef DEBUG
   #ifdef INT_LATENCY_CHECK
   /* Clear the latency histogram */
   memset(gdwIntLatency, 0, NUM_LATENCY_SAMPLES * sizeof(u_int32));
   #endif
   #endif

   #ifdef DEBUG
   /* Clear the latency histogram */
   memset(gdwIntCounts, 0, NUM_INTS_PER_PIC * sizeof(u_int32));
   #endif

   /* Reset our interrupt vector table(s) */
   memset(vectors, 0, sizeof(interrupt_vector) * NUM_INTS_PER_PIC);

   for(iLoop = 0; iLoop < NUM_INTS_PER_PIC; iLoop++)
     vectors[iLoop].pfnVector = (PFNISR)kal_dummy_isr;

   memset(exp_vectors, 0, sizeof(interrupt_vector) * NUM_INTS_PER_PIC);

   for(iLoop = 0; iLoop < NUM_INTS_PER_PIC; iLoop++)
     exp_vectors[iLoop].pfnVector = (PFNISR)kal_dummy_isr;

   memset(gpio_vectors, 0, sizeof(interrupt_vector) * NUM_GPIO_INTS);

   for(iLoop = 0; iLoop < NUM_GPIO_INTS; iLoop++)
     gpio_vectors[iLoop].pfnVector = (PFNISR)kal_dummy_isr;

   memset(exptimer_vectors, 0, sizeof(interrupt_vector) * NUM_EXPTIMER_INTS);

   for(iLoop = 0; iLoop < NUM_EXPTIMER_INTS; iLoop++)
     exptimer_vectors[iLoop].pfnVector = (PFNISR)kal_dummy_isr;

   /************************************************************************/
   /* Hook the top level, fanout interrupt handlers for expansion and GPIO */
   /************************************************************************/
   /********************************************************/
   /* NOTE: These handlers must be registered here so      */
   /* that the low-level device drivers (i.e. UART, E-Net) */
   /* that are installed before ROOT is called are         */
   /* handled properly.                                    */
   /********************************************************/
#if (INTEXP_TYPE != NOT_PRESENT)
   iRetcode = int_register_isr(INT_EXP,
                               (PFNISR)kal_exp_fanout_isr,
                               FALSE,
                               FALSE,
                               &kal_exp_chain);
   if(iRetcode != RC_OK)
     return(FALSE);

 //  int_enable(INT_EXP);
#endif

   iRetcode = int_register_isr(INT_GPIO,
                               (PFNISR)kal_gpio_fanout_isr,
                               FALSE,
                               FALSE,
                               &kal_gpio_chain);
   if(iRetcode != RC_OK)
     return(FALSE);

   int_enable(INT_GPIO);

   return(TRUE);
}
/*********************************************************************/
/*  FUNCTION:     hwlib_timer_initialise                             */
/*                                                                   */
/*  PARAMETERS:   uRate       - system timer rate in microseconds    */
/*                pfnSysTimer - system timer callback function       */
/*                bStart      - start the clock if TRUE              */
/*                                                                   */
/*  DESCRIPTION:  Initialise hardware timers and start the system    */
/*                timer running                                      */
/*                                                                   */
/*  RETURNS:      TRUE on success, FALSE otherwise.                  */
/*                                                                   */
/*  No semaphore protection required                                 */
/*                                                                   */
/*********************************************************************/
bool hwlib_timer_initialise(u_int32 uRate, bool bStart)
{
   int iLoop, iPic;
   static bool bTimersInitialised = FALSE;

   /**********************************/
   /* Protect against multiple calls */
   /**********************************/
   if(bTimersInitialised)
      return(TRUE);

   /********************************************/
   /* Clear out all the tick timer descriptors */
   /********************************************/
   memset(ticks, 0, sizeof(tick_descriptor) * NUM_TICKS);

   /*******************************************/
   /* Clear out all system client descriptors */
   /*******************************************/
   memset(system_timer_clients, 0, sizeof(systimer_client) * MAX_SYSTIMER_CLIENTS);

   /**************************************/
   /* Grab the top level timer interrupt */
   /**************************************/
   iLoop = int_register_isr(INT_TIMERS,
                            (PFNISR)kal_timer_fanout_isr,
                            FALSE,
                            FALSE,
                            &kal_timer_chain);
   if(iLoop != RC_OK)
      return(FALSE);

   /********************************/
   /* Grab the hardware tick timer */
   /********************************/
   timers[TICK_TIMER].dwFlags = TIMER_FLAG_ALLOCATED | TIMER_FLAG_SYSTEM;

   /***********************************************************/
   /* Start the system timer running at the appropriate rates */
   /***********************************************************/
   iLoop = int_register_isr(INT_SYSTIMER,
                            (PFNISR)kal_system_timer_isr,
                            FALSE,
                            FALSE,
                            &timers[SYS_TIMER].pfnChain);
   if(iLoop != RC_OK)
     return(FALSE);

   timers[SYS_TIMER].dwFlags = TIMER_FLAG_ALLOCATED | TIMER_FLAG_SYSTEM;
   gSysTimerRate = uRate;
   uSysTickCount = 0;
   uMsecPerTick = gSysTimerRate / 1000;
   iLoop = hwtimer_set(TIMERID_FROM_INDEX(SYS_TIMER), uRate, FALSE);
   if((iLoop == RC_OK) && bStart)
     hwtimer_start(TIMERID_FROM_INDEX(SYS_TIMER));

   if(iLoop != RC_OK)
     return(FALSE);

   int_enable(INT_SYSTIMER);

   #ifdef WATCHDOG_TIMER_IS_ENABLED
   /******************************************/
   /* Set up the watchdog timer if requested */
   /******************************************/
   iLoop = int_register_isr(INT_WATCHDOG,
                            (PFNISR)kal_watchdog_timer_isr,
                            FALSE,
                            FALSE,
                            &timers[WATCHDOG_TIMER].pfnChain);

   if(iLoop != RC_OK)
      return(FALSE);

   timers[WATCHDOG_TIMER].dwFlags = TIMER_FLAG_ALLOCATED | TIMER_FLAG_WATCHDOG | TIMER_FLAG_SYSTEM;
   iLoop = hwtimer_set(TIMERID_FROM_INDEX(WATCHDOG_TIMER), WATCHDOG_TIMER_PERIOD, FALSE);

   if (iLoop == RC_OK)
   {
     iLoop = hwtimer_start(TIMERID_FROM_INDEX(WATCHDOG_TIMER));    
     
     /* NB: DO NOT enable the hardware watchdog function here! This function is generally */
     /*     called very early in the initialisation process with interrupt disabled. If   */
     /*     you turn on the hardware watchdog here and the boot process keeps interrupts  */
     /*     off for more than twice the watchdog timer period, the box will get stuck in  */
     /*     a never-ending loop of reboot/reset/reboot. Only enable the watchdog function */
     /*     after interrupts have been enabled!                                           */
   }
   int_enable(INT_WATCHDOG);

   if(iLoop != RC_OK)
     return(FALSE);

   #endif /* WATCHDOG_TIMER_IS_ENABLED */

   /*********************************/
   /* Grab the tick timer interrupt */
   /*********************************/
   iLoop = int_register_isr(INT_TICKTIMER,
                            (PFNISR)kal_tick_timer_isr,
                            FALSE,
                            FALSE,
                            &timers[TICK_TIMER].pfnChain);
   if(iLoop != RC_OK)
     return(FALSE);

   int_enable(INT_TICKTIMER);

   /* Grab all remaining timers */
   for(iPic = 0; iPic < NUM_TIMERS; iPic++)
   {
     if(!(timers[iPic].dwFlags & TIMER_FLAG_ALLOCATED))
     {
        stop_hw_timer(iPic);
        iLoop = int_register_isr(timers[iPic].dwIntMask,
                                 (PFNISR)kal_general_timer_isr,
                                 FALSE,
                                 FALSE,
                                 &timers[iPic].pfnChain);
        if(iLoop != RC_OK)
           return(FALSE);

        int_enable(timers[iPic].dwIntMask);
     }
   }

   /**************************************/
   /* Turn on the global timer interrupt */
   /**************************************/
   int_enable(INT_TIMERS);

   return(TRUE);
}


/********************************************************************/
/*  FUNCTION:    hwlib_watchdog_enable                              */
/*                                                                  */
/*  PARAMETERS:  None                                               */
/*                                                                  */
/*  DESCRIPTION: If the watchdog timer is enabled in this build,    */
/*               flip the hardware bit that enables the watchdog    */
/*               function.                                          */
/*                                                                  */
/*  RETURNS:     Nothing                                            */
/*                                                                  */
/*  CONTEXT:     May be called from any context but interrupt MUST  */
/*               be enabled within twice the selected watchdog      */
/*               period of making this call or the IRD will reboot. */
/*                                                                  */
/********************************************************************/
void hwlib_watchdog_enable(void)
{
  #ifdef WATCHDOG_TIMER_IS_ENABLED
  make_timer_watchdog(WATCHDOG_TIMER, TRUE);
  #endif
}

/********************************************************************/
/*  FUNCTION:    hw_critical_section_begin                          */
/*                                                                  */
/*  PARAMETERS:  Return address of caller                           */
/*                                                                  */
/*  DESCRIPTION: Turn off all interrupts (or leave them off if      */
/*               already disabled.                                  */
/*                                                                  */
/*  RETURNS:     State of interrupts on entry.                      */
/********************************************************************/
bool hw_critical_section_begin( u_int32 ra )
{
   bool bIntsOn;

   /* Disable interrupts */
   bIntsOn = DisableInterrupts();

   /* Perform checks */
#ifdef DEBUG
#ifdef DRIVER_INCL_TRACE
   if(bIntsOn && (iProcessingInt || iCritSecCount))
   {
      cs_trace(TRACE_KAL|TRACE_LEVEL_ALWAYS, "cs: Turning off intrs in err! iProcessingInt=%x iCritSecCount=%x\n", iProcessingInt, iCritSecCount);
   }
#endif
#endif

   /* Save return address */
   if(iCritSecCount < CRIT_SEC_STACK_SIZE)
   {
      crit_sec_stack[iCritSecCount] = ra;
   }

   /* Increment nesting count */
   ++iCritSecCount;

   /* Return whether we disabled interrupts or not */
   return(bIntsOn);
}

/********************************************************************/
/*  FUNCTION:    c_critical_section_begin                           */
/*                                                                  */
/*  PARAMETERS:  LR of caller of 'critical_section_begin'           */
/*               which is an ASM function in CRITSEC.S              */
/*                                                                  */
/*  DESCRIPTION: Turn off all interrupts (or leave them off if      */
/*               already disabled.                                  */
/*                                                                  */
/*  RETURNS:     State of interrupts on entry.                      */
/********************************************************************/
bool c_critical_section_begin( u_int32 ra )
{
#if defined(DEBUG) && defined(CRIT_SEC_LATENCY_CHECK)

   bool return_value;
   
   return_value =hw_critical_section_begin( ra );
   if( return_value && !from_int_handler && !from_lowpowtask )/* first time in */
   {
      time_begin = *trace_timer_value_addr;
      caller = ra;
   }   
   return( return_value );

#else
   return( hw_critical_section_begin( ra ) );
#endif   
}

/********************************************************************/
/*  FUNCTION:    critical_section_end                               */
/*                                                                  */
/*  PARAMETERS:  previous_state - value returned by matching call   */
/*                                to critical_section_begin         */
/*                                                                  */
/*  DESCRIPTION: Restore the state of the interrupts after          */
/*               temporarily disabling them.                        */
/*                                                                  */
/*  RETURNS:     Nothing                                            */
/********************************************************************/
void critical_section_end( bool previous_state)
{
#ifdef DEBUG
#ifdef CRIT_SEC_LATENCY_CHECK
   int i, insertIdx;
#endif
#endif
   
   /* Decrement nesting count */
   iCritSecCount--;

   /* Zero out saved return address */
   if(iCritSecCount < CRIT_SEC_STACK_SIZE)
   {
      crit_sec_stack[iCritSecCount] = 0;
   }

   /* If this is the bottom of the nest, re-enable interrupts */
   if(previous_state)
   {
      /* Perform checks */
#ifdef DEBUG
#ifdef DRIVER_INCL_TRACE
      if(iCritSecCount || iProcessingInt)
      {
         cs_trace(TRACE_KAL|TRACE_LEVEL_ALWAYS, "cs: Turning on intrs in err! iProcessingInt=%x iCritSecCount=%x\n", iProcessingInt, iCritSecCount);
      }
#endif
#endif
#ifdef DEBUG
#ifdef CRIT_SEC_LATENCY_CHECK
      if( !from_int_handler && !from_lowpowtask)
      {
         time_end = *trace_timer_value_addr;
         if( time_end >= time_begin )
            time_diff = (time_end - time_begin) /54;
         else /* timer limit has been reached sometime during this critical section*/
            time_diff = 60000000 - ( ( time_begin - time_end ) /54);
         if( time_diff != 0 )
         {
             /* keep it in order */
            i=0; insertIdx =0;
            while( time_diff >longestCSduration[i].duration && i< LONGESTCSDURATION_SIZE )
            {
               i++;
               insertIdx = i;
            }
            if( insertIdx  )                 
            {
               for( i=0; i<insertIdx;i++ )
               {
                  if( longestCSduration[i+1].duration != 0 )
                  {
                     longestCSduration[i].duration = longestCSduration[i+1].duration;
                     longestCSduration[i].caller_add = longestCSduration[i+1].caller_add;
                  }
               }
               longestCSduration[insertIdx -1].duration = time_diff;
               longestCSduration[insertIdx -1].caller_add = caller;
            }
         }

      }   
#endif
#endif

      EnableInterrupts();
   }

   return;
}

#if (defined ISR_TIMING_CHECK)  && (defined DEBUG)
/********************************************************************/
/*  FUNCTION:    isr_timing_check                                   */
/*                                                                  */
/*  PARAMETERS:  pVector - ISR function to which the times refer    */
/*               uIntID  - Interrupt ID to which the times refer    */
/*               uStart  - ISR start timestamp                      */
/*               uEnd    - ISR end timestamp                        */
/*                                                                  */
/*  DESCRIPTION: This table calculates the time elapsed during an   */
/*               ISR and uses this to maintain a sorted list of the */
/*               longest ISR durations.                             */
/*                                                                  */
/*  RETURNS:     Nothing                                            */
/*                                                                  */
/*  CONTEXT:     May be called in any context (but is designed to   */
/*               used solely from low level KAL interrupt handlers) */
/*                                                                  */
/********************************************************************/
static void isr_timing_check(u_int32 pVector, u_int32 uIntID, u_int32 uStart, u_int32 uEnd)
{
  u_int32 uElapsed;
  int i, insertIdx;
  
  if(!uStart)
    return;
    
  if( uEnd >= uStart )
     uElapsed = (uEnd - uStart) /54;
  else /* timer limit has been reached sometime during this critical section*/
     uElapsed = 60000000 - ( ( uStart - uEnd ) /54);
     
  if( uElapsed != 0 )
  {
      /* keep it in order */
     i=0; insertIdx =0;
     while( (uElapsed >longestISRduration[i].duration) && (i< LONGESTISRDURATION_SIZE) )
     {
        i++;
        insertIdx = i;
     }
     
     if( insertIdx  )                 
     {
        for( i=0; i<insertIdx;i++ )
        {
           if( longestISRduration[i+1].duration != 0 )
           {
              longestISRduration[i] = longestISRduration[i+1];
           }
        }
        longestISRduration[insertIdx-1].duration   = uElapsed;
        longestISRduration[insertIdx-1].caller_add = pVector;
        longestISRduration[insertIdx-1].id         = uIntID;
        longestISRduration[insertIdx-1].end        = uEnd;
     }
   }
}
#endif /* ISR_TIMING_CHECK */

/********************************************************************/
/*  FUNCTION:    c_not_interrupt_safe                               */
/*                                                                  */
/*  PARAMETERS:  LR of caller of 'not_interrupt_safe' which is an   */
/*               assembler function in CRITSEC.S / VXCRIT.S         */
/*                                                                  */
/*  DESCRIPTION: Checks that interrupts are not disabled            */
/*               Complains if they are                              */
/*                                                                  */
/*  RETURNS:     Nothing                                            */
/********************************************************************/
bool c_not_interrupt_safe( u_int32 ra )
{

   bool bRet = FALSE;

   if (InterruptsDisabled())
   {
#if defined(DEBUG) && defined (DRIVER_INCL_TRACE)
     isr_trace_new(TRACE_ALL | TRACE_LEVEL_ALWAYS, "****** Called a non-interrupt-safe func (called from 0x%x) with interrupts disabled! ********\n", ra, 0);
     if(iProcessingInt)
     {
       bRet = TRUE;
       isr_trace_new(TRACE_ALL | TRACE_LEVEL_ALWAYS, "****** Inside a non-interrupt-safe func (called from 0x%x) at interrupt time! ***********\n", ra, 0);
       isr_error_log(ERROR_FATAL | MOD_KAL | RC_KAL_INTSAFETY);
     }
#endif
   }
   return bRet;
}

/********************************************************************/
/*  FUNCTION:    c_not_from_critical_section                        */
/*                                                                  */
/*  PARAMETERS:  LR of caller of 'not_from_critical_section' which  */
/*               is an assembler function in CRITSEC.S / VXCRIT.S   */
/*                                                                  */
/*  DESCRIPTION: This function will generate a fatal_exit if        */
/*               called within a critical section.                  */
/*                                                                  */
/*  RETURNS:     Nothing                                            */
/********************************************************************/
void c_not_from_critical_section( u_int32 ra )
{
   if (iCritSecCount)
   {
     /* We just hang things up here. Inside a critical section, we can't call fatal_exit */
     /* or any of the trace functions. With this loop, though, it will be obvious what   */
     /* has happened if you stop the debugger (unless, of course, you have the hardware  */
     /* watchdog enabled in which case you will see the box reboot shortly after this    */
     /* loop is entered)                                                                 */
     while(1);
   }
   return;
}

/********************************************************************/
/*  FUNCTION:    only_at_interrupt_safe                             */
/*                                                                  */
/*  PARAMETERS:  none                                               */
/*                                                                  */
/*  DESCRIPTION: Checks that interrupts are disabled and that we    */
/*               inside an interrupt routine. Complains if not      */
/*                                                                  */
/*  RETURNS:     Nothing                                            */
/********************************************************************/
void only_at_interrupt_safe( void )
{
   
   if (FALSE == InterruptsDisabled())
   {
     #ifdef DEBUG
     #ifdef DRIVER_INCL_TRACE
     isr_trace_new(TRACE_ALL | TRACE_LEVEL_ALWAYS, "****** Called an interrupt-only func with interrupts enabled! ************\n", 0, 0);
     if(0 == iProcessingInt)
     {
       isr_trace_new(TRACE_ALL | TRACE_LEVEL_ALWAYS, "****** Called an interrupt-only function outside an interrupt! ***********\n", 0, 0);
     }
     #endif
     #endif

   }
   return;
}

/********************************************************************/
/*  FUNCTION:    read_board_and_vendor_codes                        */
/*                                                                  */
/*  PARAMETERS:  pBoard  - ptr to storage for board ID.             */
/*               pVendor - ptr to storage for vendor code.          */
/*                                                                  */
/*  DESCRIPTION: Read the board and vendor code from this IRD's     */
/*               hardware configuration register.                   */
/*                                                                  */
/*  RETURNS:     Board & vendor codes stored at supplied ptrs.      */
/*                                                                  */
/*  No semaphore protection required.                               */
/*                                                                  */
/********************************************************************/
void read_board_and_vendor_codes(u_int8 *pBoard, u_int8 *pVendor)
{
  /* The values VendorID and BoardID are read at startup from    */
  /* startupc.c (api.s).  Use these values instead of rereading. */

  /* Set the vendor ID */
  if (pVendor)
  {
    *pVendor = (u_int8) VendorID;
  }

  /* Set the board ID */
  if (pBoard)
  {
    *pBoard = (u_int8) BoardID;
  }

  return;
}

/********************************************************************/
/*  FUNCTION: read_chip_id_and_revision                             */
/*                                                                  */
/*  PARAMETERS:   pChip - Storage for returned chip ID value        */
/*                pRev  - Storage for returned revision value       */
/*                                                                  */
/*  DESCRIPTION:  Read the current chip ID and revision number      */
/*                                                                  */
/*  RETURNS:      Chip ID and revision codes for the Conexant       */
/*                processor/demux/decoder IC in use.                */
/********************************************************************/
void read_chip_id_and_revision(u_int32 *pChip, u_int8 *pRev)
{
  if(pChip)
    *pChip = GetChipID();

  if(pRev)
    *pRev = GetChipRev();
}

/************************/
/************************/
/** Timer Service APIs **/
/************************/
/************************/

/********************************************************************/
/*  FUNCTION:    hwtimer_create                                     */
/*                                                                  */
/*  PARAMETERS:  pfnCallback - ptr to callback function             */
/*               pUserData   - value passed to callback on timeout  */
/*               name        - 4 char name of timer                 */
/*                                                                  */
/*  DESCRIPTION: Allocate a general purpose timer                   */
/*                                                                  */
/*  RETURNS:     ID of the timer if successful, 0 otherwise         */
/*                                                                  */
/********************************************************************/
timer_id_t hwtimer_create(PFNTIMERC pfnCallback, void *pUserData, const char *name)
{
   int iLoop;
   bool ks;

   #ifdef DRIVER_INCL_TRACE
   isr_trace_new(TRACE_KAL | TRACE_LEVEL_2, "hwtimer_create\n", 0, 0);
   #endif

   /************************************/
   /* Is the callback provided valid ? */
   /************************************/
   if(pfnCallback == (PFNTIMERC)NULL)
   {
     #ifdef DRIVER_INCL_TRACE
     error_log(ERROR_WARNING | MOD_KAL | RC_KAL_INVALID);
     #endif
     return(0);
   }

   /*********************************************************/
   /* Run through the list of timers looking for a free one */
   /*********************************************************/
   ks = critical_section_begin();
   for(iLoop = 0; iLoop < NUM_TIMERS; iLoop++)
   {
      /* If this timer is free, hand it to the caller */
      if(!(timers[iLoop].dwFlags & TIMER_FLAG_ALLOCATED))
      {
         timers[iLoop].dwFlags |= TIMER_FLAG_ALLOCATED;
         timers[iLoop].pfnCallback = pfnCallback;
         timers[iLoop].pUserData = pUserData;
         critical_section_end(ks);
         if(name != NULL)
           strncpy(timers[iLoop].name, name, MAX_OBJ_NAME_LENGTH);
         else
           sprintf(timers[iLoop].name, "KAL_TIM%03x", iLoop);
         return(TIMERID_FROM_INDEX(iLoop));
      }
   }

   /*********************************************/
   /* If we got here, there were no free timers */
   /*********************************************/
   critical_section_end(ks);
   return(0);
}

/********************************************************************/
/*  FUNCTION:    hwtimer_destroy                                    */
/*                                                                  */
/*  PARAMETERS:  timer - ID of the timer to destroy                 */
/*                                                                  */
/*  DESCRIPTION: Free a general purpose timer allocated using a     */
/*               previous call to hwtimer_create                    */
/*                                                                  */
/*  RETURNS:     RC_OK            - Success                         */
/*               RC_KAL_NOTHOOKED - Timer not already allocated     */
/*               RC_KAL_INVALID   - Invalid handle supplied         */
/*                                                                  */
/********************************************************************/
int hwtimer_destroy(timer_id_t timer)
{
  int iIndex;
  bool ks;

  #ifdef DRIVER_INCL_TRACE
  trace_new(TRACE_KAL | TRACE_LEVEL_2, "hwtimer_destroy 0x%08lx\n", timer);
  #endif

  if(IS_VALID_TIMERID(timer))
  {
     /* Has this timer been allocated ? */
     iIndex = INDEX_FROM_TIMERID(timer);
     ks = critical_section_begin();

     if(timers[iIndex].dwFlags & TIMER_FLAG_ALLOCATED)
     {
       /* Yes - so stop it and return it to the pool */
       hwtimer_stop(timer);

       timers[iIndex].dwFlags    &= ~(TIMER_FLAG_ALLOCATED | TIMER_FLAG_SET);
       timers[iIndex].pfnCallback = (PFNTIMERC)NULL;
       timers[iIndex].pUserData   = NULL;
       clear_object_name(timers[iIndex].name); /* This is critical section safe */
       critical_section_end(ks);

       return(RC_OK);
     }
     else /* No - timer not already allocated */
     {
       critical_section_end(ks);
       #ifdef DRIVER_INCL_TRACE
       error_log(ERROR_WARNING | MOD_KAL | RC_KAL_NOTHOOKED);
       #endif
       return(RC_KAL_NOTHOOKED);
     }
  }
  else /* Invalid timer handle passed */
  {
    #ifdef DRIVER_INCL_TRACE
    error_log(ERROR_WARNING | MOD_KAL | RC_KAL_INVALID);
    #endif
    return(RC_KAL_INVALID);
  }
}

/********************************************************************/
/*  FUNCTION:    hwtimer_set                                        */
/*                                                                  */
/*  PARAMETERS:  timer     - ID of the timer to destroy             */
/*               period_us - Desired period in microseconds         */
/*               bOneShot  - If TRUE, one shot timer, else periodic */
/*                                                                  */
/*  DESCRIPTION: Set the period and type for a timer previously     */
/*               allocated using a call to hwtimer_create           */
/*                                                                  */
/*  RETURNS:     RC_OK            - Success                         */
/*               RC_KAL_NOTHOOKED - Timer not already allocated     */
/*               RC_KAL_INVALID   - Invalid handle supplied         */
/*               RC_KAL_PERIOD    - Invalid period specified        */
/*                                                                  */
/********************************************************************/
int hwtimer_set(timer_id_t timer, u_int32 period_us, bool bOneShot)
{
  int iIndex;
  bool ks;

  #ifdef DRIVER_INCL_TRACE
  isr_trace_new(TRACE_KAL | TRACE_LEVEL_2, "hwtimer_set 0x%08lx\n", timer, 0);
  #endif

  if(IS_VALID_TIMERID(timer))
  {
     /* Has this timer been allocated ? */
     iIndex = INDEX_FROM_TIMERID(timer);

     ks = critical_section_begin();
     if(timers[iIndex].dwFlags & TIMER_FLAG_ALLOCATED)
     {
       /* The maximum period can be set is 79s. */
       if (period_us > 79000000)
       {
          critical_section_end(ks);
          #ifdef DRIVER_INCL_TRACE
          isr_trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, "Invalid timer period (%dus) specified.\n", period_us, 0);
          isr_error_log(ERROR_WARNING | MOD_KAL | RC_KAL_PERIOD);
          #endif
          return(RC_KAL_PERIOD);
       }

       /* Yes - so set the timer (this is safe within a critical section) */
       timers[iIndex].dwPeriod = set_hw_timer(iIndex,
                                              period_us);

       if(timers[iIndex].dwPeriod)
       {
         timers[iIndex].dwFlags |= TIMER_FLAG_SET;
         if(bOneShot)
           timers[iIndex].dwFlags |= TIMER_FLAG_ONESHOT;
         else
           timers[iIndex].dwFlags &= ~TIMER_FLAG_ONESHOT;
         critical_section_end(ks);
         return(RC_OK);
       }
       else /* Timer could not be set to specified period */
       {
          critical_section_end(ks);
          #ifdef DRIVER_INCL_TRACE
          isr_trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, "Invalid timer period (%dus) specified.\n", period_us, 0);
          isr_error_log(ERROR_WARNING | MOD_KAL | RC_KAL_PERIOD);
          #endif
          return(RC_KAL_PERIOD);
       }
     }

     else /* No - timer not already allocated */
     {
       critical_section_end(ks);
       #ifdef DRIVER_INCL_TRACE
       isr_error_log(ERROR_WARNING | MOD_KAL | RC_KAL_NOTHOOKED);
       #endif
       return(RC_KAL_NOTHOOKED);
     }
  }
  else /* Invalid timer handle passed */
  {
    #ifdef DRIVER_INCL_TRACE
    isr_error_log(ERROR_WARNING | MOD_KAL | RC_KAL_INVALID);
    #endif
    return(RC_KAL_INVALID);
  }
}

/********************************************************************/
/*  FUNCTION:    hwtimer_start                                      */
/*                                                                  */
/*  PARAMETERS:  timer - ID of the timer to start                   */
/*                                                                  */
/*  DESCRIPTION: Start a timer counting.                            */
/*                                                                  */
/*  RETURNS:     RC_OK            - Success                         */
/*               RC_KAL_NOTHOOKED - Timer not already allocated     */
/*               RC_KAL_INVALID   - Invalid handle supplied         */
/*               RC_KAL_NOTSET    - Timer period not set            */
/*                                                                  */
/********************************************************************/
int hwtimer_start(timer_id_t timer)
{
  int  iIndex;
  bool ks;

  #ifdef DRIVER_INCL_TRACE
  isr_trace_new(TRACE_KAL | TRACE_LEVEL_2, "hwtimer_start 0x%08lx\n", timer, 0);
  #endif

  if(IS_VALID_TIMERID(timer))
  {
     /* Has this timer been allocated ? */
     iIndex = INDEX_FROM_TIMERID(timer);
     ks = critical_section_begin();
     if(timers[iIndex].dwFlags & TIMER_FLAG_ALLOCATED)
     {
       /* Timer is allocated. Has period been set ? */
       if(timers[iIndex].dwFlags & TIMER_FLAG_SET)
       {
         /* Yes - so start the timer (call safe within a critical section) */

         start_hw_timer(iIndex);

         timers[iIndex].dwFlags |= TIMER_FLAG_RUNNING;

         critical_section_end(ks);

         return(RC_OK);
       }
       else /* Timer period has not been set */
       {
         critical_section_end(ks);
         #ifdef DRIVER_INCL_TRACE
         isr_error_log(ERROR_WARNING | MOD_KAL | RC_KAL_NOTSET);
         #endif
         return(RC_KAL_NOTSET);
       }
     }
     else /* No - timer not already allocated */
     {
       critical_section_end(ks);
       #ifdef DRIVER_INCL_TRACE
       isr_error_log(ERROR_WARNING | MOD_KAL | RC_KAL_NOTHOOKED);
       #endif
       return(RC_KAL_NOTHOOKED);
     }
  }
  else /* Invalid timer handle passed */
  {
    #ifdef DRIVER_INCL_TRACE
    isr_error_log(ERROR_WARNING | MOD_KAL | RC_KAL_INVALID);
    #endif
    return(RC_KAL_INVALID);
  }
}

/********************************************************************/
/*  FUNCTION:    hwtimer_stop                                       */
/*                                                                  */
/*  PARAMETERS:  timer - ID of the timer to stop                    */
/*                                                                  */
/*  DESCRIPTION: Stop a timer from counting.                        */
/*                                                                  */
/*  RETURNS:     RC_OK            - Success                         */
/*               RC_KAL_NOTHOOKED - Timer not already allocated     */
/*               RC_KAL_INVALID   - Invalid handle supplied         */
/*                                                                  */
/********************************************************************/
int hwtimer_stop(timer_id_t timer)
{
  int  iIndex;
  bool ks;

  #ifdef DRIVER_INCL_TRACE
  cs_trace(TRACE_KAL | TRACE_LEVEL_2, "hwtimer_stop 0x%08lx\n", timer, 0);
  #endif

  if(IS_VALID_TIMERID(timer))
  {
     /* Has this timer been allocated ? */
     iIndex = INDEX_FROM_TIMERID(timer);

     ks = critical_section_begin();

     if(timers[iIndex].dwFlags & TIMER_FLAG_ALLOCATED)
     {
       /* Yes - so stop the timer if it's actually running */
       if(timers[iIndex].dwFlags & TIMER_FLAG_RUNNING)
       {
         /* This call is safe from within a critical section */
         stop_hw_timer(iIndex);

         timers[iIndex].dwFlags &= ~TIMER_FLAG_RUNNING;
       }
       critical_section_end(ks);
       return(RC_OK);
     }
     else /* No - timer not already allocated */
     {
       critical_section_end(ks);
       #ifdef DRIVER_INCL_TRACE
       isr_error_log(ERROR_WARNING | MOD_KAL | RC_KAL_NOTHOOKED);
       #endif
       return(RC_KAL_NOTHOOKED);
     }
  }
  else /* Invalid timer handle passed */
  {
    #ifdef DRIVER_INCL_TRACE
    isr_error_log(ERROR_WARNING | MOD_KAL | RC_KAL_INVALID);
    #endif
    return(RC_KAL_INVALID);
  }
}

/********************************************************************/
/*  FUNCTION:     hwtimer_sys_set                                   */
/*                                                                  */
/*  PARAMETERS:   period_us - desired system timer period in        */
/*                            microseconds.                         */
/*                                                                  */
/*  DESCRIPTION:  Set the system timer tick rate.                   */
/*                                                                  */
/*  RETURNS:      RC_OK            - Success                        */
/*                RC_KAL_NOTHOOKED - Timer not already allocated    */
/*                RC_KAL_PERIOD    - Invalid period specified       */
/*                                                                  */
/********************************************************************/
int hwtimer_sys_set(u_int32 period_us)
{
   gSysTimerRate = period_us;
   return(hwtimer_set(TIMERID_FROM_INDEX(SYS_TIMER), period_us, FALSE));
}

/********************************************************************/
/*  FUNCTION:     hwtimer_sys_start                                 */
/*                                                                  */
/*  PARAMETERS:   None                                              */
/*                                                                  */
/*  DESCRIPTION:  Start the system timer counting.                  */
/*                                                                  */
/*  RETURNS:      RC_OK            - Success                        */
/*                RC_KAL_NOTHOOKED - Timer not already allocated    */
/*                RC_KAL_NOTSET    - Timer period not set           */
/*                                                                  */
/********************************************************************/
int hwtimer_sys_start(void)
{
    return(hwtimer_start(TIMERID_FROM_INDEX(SYS_TIMER)));
}

/********************************************************************/
/*  FUNCTION:    hwtimer_sys_stop                                   */
/*                                                                  */
/*  PARAMETERS:  None                                               */
/*                                                                  */
/*  DESCRIPTION: Stop the system timer from counting.               */
/*                                                                  */
/*  RETURNS:     RC_OK            - Success                         */
/*               RC_KAL_NOTHOOKED - Timer not already allocated     */
/*               RC_KAL_INVALID   - Invalid handle supplied         */
/*                                                                  */
/********************************************************************/
int hwtimer_sys_stop(void)
{
    return(hwtimer_stop(TIMERID_FROM_INDEX(SYS_TIMER)));
}

/********************************************************************/
/*  FUNCTION:    hwtimer_sys_get_rate                               */
/*                                                                  */
/*  PARAMETERS:  None                                               */
/*                                                                  */
/*  DESCRIPTION: Return the current system timer rate               */
/*                                                                  */
/*  RETURNS:     0 - system timer not initialised                   */
/*               other value - rate in microseconds                 */
/*                                                                  */
/********************************************************************/
u_int32 hwtimer_sys_get_rate(void)
{
    return(gSysTimerRate);
}

/********************************************************************/
/*  FUNCTION:    hwtimer_sys_register_client                        */
/*                                                                  */
/*  PARAMETERS:  iIndex - index of client to remove                 */
/*               pfnSysFunc - pointer to callback function          */
/*               pArg   - Argument passed to callback               */
/*                                                                  */
/*  DESCRIPTION: Register a function that will be called whenever   */
/*               the system timer interrupt fires.                  */
/*                                                                  */
/*  RETURNS:     RC_OK            - Success                         */
/*               RC_KAL_NOTHOOKED - Client already registered       */
/*               RC_KAL_INVALID   - Invalid index supplied          */
/*                                                                  */
/********************************************************************/
int hwtimer_sys_register_client(PFNTIMERC pfnSysFunc, int iIndex, void *pArg)
{
  bool ks;
  bool bRegistered;

  /* Ensure that a valid index has been passed */
  if ((iIndex < 0) || (iIndex >= MAX_SYSTIMER_CLIENTS))
  {
    #ifdef DRIVER_INCL_TRACE
    error_log(ERROR_WARNING | MOD_KAL | RC_KAL_INVALID);
    #endif
    return(RC_KAL_INVALID);
  }

  /* Stop anyone else from mucking with things while we are */
  ks = critical_section_begin();

  /* Is this client index already taken? */
  bRegistered = (system_timer_clients[iIndex].pfnHandler == (PFNTIMERC)NULL) ? FALSE : TRUE;
  if (!bRegistered)
  {
    /* It's free so grab it for the caller */
    system_timer_clients[iIndex].pfnHandler = pfnSysFunc;
    system_timer_clients[iIndex].pArg       = pArg;
  }
  critical_section_end(ks);

  return(bRegistered ? RC_KAL_NOTHOOKED : RC_OK);

}

/********************************************************************/
/*  FUNCTION:    hwtimer_sys_remove_client                          */
/*                                                                  */
/*  PARAMETERS:  iIndex - index of client to remove                 */
/*                                                                  */
/*  DESCRIPTION: Remove a system timer client callback              */
/*                                                                  */
/*  RETURNS:     RC_OK            - Success                         */
/*               RC_KAL_NOTHOOKED - Client not already registered   */
/*               RC_KAL_INVALID   - Invalid index supplied          */
/*                                                                  */
/********************************************************************/
int hwtimer_sys_remove_client(int iIndex)
{
  bool ks;
  bool bRegistered;

  /* Ensure that a valid index has been passed */
  if ((iIndex < 0) || (iIndex >= MAX_SYSTIMER_CLIENTS))
  {
    #ifdef DRIVER_INCL_TRACE
    error_log(ERROR_WARNING | MOD_KAL | RC_KAL_INVALID);
    #endif
    return(RC_KAL_INVALID);
  }

  /* Stop anyone else from mucking with things while we are */
  ks = critical_section_begin();

  bRegistered = (system_timer_clients[iIndex].pfnHandler == (PFNTIMERC)NULL) ? FALSE : TRUE;
  system_timer_clients[iIndex].pfnHandler = (PFNTIMERC)NULL;
  system_timer_clients[iIndex].pArg       = (void *)NULL;

  critical_section_end(ks);

  return(bRegistered ? RC_OK : RC_KAL_NOTHOOKED);
}

/********************************************************************/
/*  FUNCTION:    tick_create                                        */
/*                                                                  */
/*  PARAMETERS:  pfnCallback - ptr to callback function             */
/*               pUserData   - value passed to callback on timeout  */
/*               name        - 4 char name of timer                 */
/*                                                                  */
/*  DESCRIPTION: Create an application tick timer using the shared  */
/*               hardware tick timer.                               */
/*                                                                  */
/*  RETURNS:     ID of the tick timer if successful, 0 otherwise    */
/*                                                                  */
/********************************************************************/
tick_id_t tick_create(PFNTIMERC pfnCallback, void *pUserData, const char *name)
{
   int iLoop;
   bool ks;

   #ifdef DRIVER_INCL_TRACE
   isr_trace_new(TRACE_KAL | TRACE_LEVEL_2, "tick_create\n", 0, 0);
   #endif

   /************************************/
   /* Is the callback provided valid ? */
   /************************************/
   if(pfnCallback == (PFNTIMERC)NULL)
   {
     #ifdef DRIVER_INCL_TRACE
     error_log(ERROR_WARNING | MOD_KAL | RC_KAL_INVALID);
     #endif
     return(0);
   }

   /*********************************************************/
   /* Run through the list of timers looking for a free one */
   /*********************************************************/
   ks = critical_section_begin();
   for(iLoop = 0; iLoop < NUM_TICKS; iLoop++)
   {
      /* If this timer is free, hand it to the caller */
      if(!(ticks[iLoop].dwFlags & TIMER_FLAG_ALLOCATED))
      {
         ticks[iLoop].dwFlags |= TIMER_FLAG_ALLOCATED;
         ticks[iLoop].pfnCallback = pfnCallback;
         ticks[iLoop].pUserData = pUserData;
         critical_section_end(ks);
         if(name != NULL)
           strncpy(ticks[iLoop].name, name, MAX_OBJ_NAME_LENGTH);
         else
           sprintf(ticks[iLoop].name, "KAL_TIC%03x", iLoop);
         return(TICKID_FROM_INDEX(iLoop));
      }
   }

   /*********************************************/
   /* If we got here, there were no free timers */
   /*********************************************/
   critical_section_end(ks);
   return(0);
}

/********************************************************************/
/*  FUNCTION:    tick_destroy                                       */
/*                                                                  */
/*  PARAMETERS:  tick - ID of the tick timer to destroy             */
/*                                                                  */
/*  DESCRIPTION: Free a shared tick timer allocated using a         */
/*               previous call to tick_create                       */
/*                                                                  */
/*  RETURNS:     RC_OK            - Success                         */
/*               RC_KAL_NOTHOOKED - Timer not already allocated     */
/*               RC_KAL_INVALID   - Invalid handle supplied         */
/*                                                                  */
/********************************************************************/
int tick_destroy(tick_id_t tick)
{
  int iIndex;
  bool ks;

  #ifdef DRIVER_INCL_TRACE
  trace_new(TRACE_KAL | TRACE_LEVEL_2, "tick_destroy 0x%08lx\n", tick);
  #endif

  if(IS_VALID_TICKID(tick))
  {
     /* Has this timer been allocated ? */
     iIndex = INDEX_FROM_TICKID(tick);

     if(ticks[iIndex].dwFlags & TIMER_FLAG_ALLOCATED)
     {
       /* Get exclusive access */
       ks = critical_section_begin();

       /* Yes - so stop it and return it to the pool */
       tick_stop(tick);

       ticks[iIndex].dwFlags    &= ~(TIMER_FLAG_ALLOCATED | TIMER_FLAG_SET);
       ticks[iIndex].pfnCallback = (PFNTIMERC)NULL;
       ticks[iIndex].pUserData   = NULL;
       
       /* Clear the name (critical section safe) */
       clear_object_name(ticks[iIndex].name);
       
       /* UnLock access */
       critical_section_end(ks);
       

       return(RC_OK);
     }
     else /* No - timer not already allocated */
     {
       #ifdef DRIVER_INCL_TRACE
       error_log(ERROR_WARNING | MOD_KAL | RC_KAL_NOTHOOKED);
       #endif
       return(RC_KAL_NOTHOOKED);
     }
  }
  else /* Invalid timer handle passed */
  {
    #ifdef DRIVER_INCL_TRACE
    error_log(ERROR_WARNING | MOD_KAL | RC_KAL_INVALID);
    #endif
    return(RC_KAL_INVALID);
  }
}

/********************************************************************/
/*  FUNCTION:    tick_set                                           */
/*                                                                  */
/*  PARAMETERS:  tick      - ID of the timer to destroy             */
/*               period_ms - Desired period in milliseconds         */
/*               bOneShot  - If TRUE, one shot timer, else periodic */
/*                                                                  */
/*  DESCRIPTION: Set the period and type for a timer previously     */
/*               allocated using a call to tick_create             */
/*                                                                  */
/*  RETURNS:     RC_OK            - Success                         */
/*               RC_KAL_NOTHOOKED - Timer not already allocated     */
/*               RC_KAL_INVALID   - Invalid handle supplied         */
/*               RC_KAL_PERIOD    - Invalid period specified        */
/*                                                                  */
/********************************************************************/
int tick_set(tick_id_t tick, u_int32 period_ms, bool bOneShot)
{
  int iIndex;
  bool bRestart = FALSE;
  bool ks;

  #ifdef DRIVER_INCL_TRACE
  cs_trace(TRACE_KAL | TRACE_LEVEL_2, "tick_set 0x%08lx\n", tick, 0);
  #endif

  if(IS_VALID_TICKID(tick))
  {
     /* Make sure that the period is valid, between 1 and MAX_TICK_PERIOD */
     if((period_ms > MAX_TICK_PERIOD) || (period_ms == 0))
     {
        #ifdef DRIVER_INCL_TRACE
        cs_trace(TRACE_KAL | TRACE_LEVEL_ALWAYS, "Invalid tick period (%dms) specified.\n", period_ms, 0);
        cs_error_log(ERROR_WARNING | MOD_KAL | RC_KAL_PERIOD);
        #endif
        return(RC_KAL_PERIOD);
     }

     /* Has this timer been allocated ? */
     iIndex = INDEX_FROM_TICKID(tick);

     /* Get exclusive access */
     ks = critical_section_begin();

     if(ticks[iIndex].dwFlags & TIMER_FLAG_ALLOCATED)
     {
       /* If the tick timer is already running, stop it */
       if (ticks[iIndex].dwFlags & TIMER_FLAG_RUNNING)
       {
         bRestart = TRUE;
         tick_stop(tick);
       }

       ticks[iIndex].iPeriodMs = (int)period_ms;

       ticks[iIndex].dwFlags |= TIMER_FLAG_SET;
       if(bOneShot)
         ticks[iIndex].dwFlags |= TIMER_FLAG_ONESHOT;
       else
         ticks[iIndex].dwFlags &= ~TIMER_FLAG_ONESHOT;

       if(bRestart)
        tick_start(tick);

       /* UnLock access */
       critical_section_end(ks);

       return(RC_OK);
     }

     else /* No - timer not already allocated */
     {
       /* UnLock access */
       critical_section_end(ks);

       #ifdef DRIVER_INCL_TRACE
       cs_error_log(ERROR_WARNING | MOD_KAL | RC_KAL_NOTHOOKED);
       #endif
       return(RC_KAL_NOTHOOKED);
     }
  }
  else /* Invalid timer handle passed */
  {
    #ifdef DRIVER_INCL_TRACE
    cs_error_log(ERROR_WARNING | MOD_KAL | RC_KAL_INVALID);
    #endif
    return(RC_KAL_INVALID);
  }
}

/********************************************************************/
/*  FUNCTION:    tick_start                                         */
/*                                                                  */
/*  PARAMETERS:  tick - ID of the timer to start                    */
/*                                                                  */
/*  DESCRIPTION: Start a timer counting.                            */
/*                                                                  */
/*  RETURNS:     RC_OK            - Success                         */
/*               RC_KAL_NOTHOOKED - Timer not already allocated     */
/*               RC_KAL_INVALID   - Invalid handle supplied         */
/*               RC_KAL_NOTSET    - Timer period not set            */
/*                                                                  */
/********************************************************************/
int tick_start(tick_id_t tick)
{
  int  iIndex;
  bool ks;

  #ifdef DRIVER_INCL_TRACE
  cs_trace(TRACE_KAL | TRACE_LEVEL_2, "tick_start 0x%08lx\n", tick, 0);
  #endif

  if(IS_VALID_TICKID(tick))
  {
     /* Has this timer been allocated ? */
     iIndex = INDEX_FROM_TICKID(tick);

     /* Get exclusive access */
     ks = critical_section_begin();

     if(ticks[iIndex].dwFlags & TIMER_FLAG_ALLOCATED)
     {
       /* Timer is allocated. Has period been set ? */
       if(ticks[iIndex].dwFlags & TIMER_FLAG_SET)
       {
         /* Yes - so start the timer (critical section safe) */
         start_tick_counter(iIndex);

         /* UnLock access */
         critical_section_end(ks);

         return(RC_OK);
       }
       else /* Timer period has not been set */
       {
         /* UnLock access */
         critical_section_end(ks);

         #ifdef DRIVER_INCL_TRACE
         cs_error_log(ERROR_WARNING | MOD_KAL | RC_KAL_NOTSET);
         #endif
         return(RC_KAL_NOTSET);
       }
     }
     else /* No - timer not already allocated */
     {
       /* UnLock access */
       critical_section_end(ks);

       #ifdef DRIVER_INCL_TRACE
       cs_error_log(ERROR_WARNING | MOD_KAL | RC_KAL_NOTHOOKED);
       #endif
       return(RC_KAL_NOTHOOKED);
     }
  }
  else /* Invalid timer handle passed */
  {
    #ifdef DRIVER_INCL_TRACE
    cs_error_log(ERROR_WARNING | MOD_KAL | RC_KAL_INVALID);
    #endif
    return(RC_KAL_INVALID);
  }
}

/********************************************************************/
/*  FUNCTION:    tick_stop                                         */
/*                                                                  */
/*  PARAMETERS:  tick  - ID of the timer to stop                    */
/*                                                                  */
/*  DESCRIPTION: Stop a timer from counting.                        */
/*                                                                  */
/*  RETURNS:     RC_OK            - Success                         */
/*               RC_KAL_NOTHOOKED - Timer not already allocated     */
/*               RC_KAL_INVALID   - Invalid handle supplied         */
/*                                                                  */
/********************************************************************/
int tick_stop(tick_id_t tick)
{
  int  iIndex;
  bool ks;

  #ifdef DRIVER_INCL_TRACE
  cs_trace(TRACE_KAL | TRACE_LEVEL_2, "tick_stop 0x%08lx\n", tick, 0);
  #endif

  if(IS_VALID_TICKID(tick))
  {
     /* Has this timer been allocated ? */
     iIndex = INDEX_FROM_TICKID(tick);

     /* Get exclusive access */
     ks = critical_section_begin();

     if(ticks[iIndex].dwFlags & TIMER_FLAG_ALLOCATED)
     {
       /* Yes - so stop the timer if it's actually running */
       if (ticks[iIndex].dwFlags & TIMER_FLAG_RUNNING)
         stop_tick_counter(iIndex);

       /* UnLock access */
       critical_section_end(ks);

       return(RC_OK);
     }
     else /* No - timer not already allocated */
     {
       /* UnLock access */
       critical_section_end(ks);

       #ifdef DRIVER_INCL_TRACE
       cs_error_log(ERROR_WARNING | MOD_KAL | RC_KAL_NOTHOOKED);
       #endif
       return(RC_KAL_NOTHOOKED);
     }
  }
  else /* Invalid timer handle passed */
  {
    #ifdef DRIVER_INCL_TRACE
    cs_error_log(ERROR_WARNING | MOD_KAL | RC_KAL_INVALID);
    #endif
    return(RC_KAL_INVALID);
  }
}

/********************************************************************/
/*  FUNCTION:    tick_get_info                                      */
/*                                                                  */
/*  PARAMETERS:  tick - ID of the timer to start                    */
/*               info - pointer to a structure to return the Timer  */
/*                        information                               */
/*  DESCRIPTION: Send back Information about the timer.             */
/*                                                                  */
/*  RETURNS:     RC_OK            - Success                         */
/*               RC_KAL_NOTHOOKED - Timer not already allocated     */
/*               RC_KAL_INVALID   - Invalid handle supplied         */
/*                                                                  */
/********************************************************************/
int tick_get_info(tick_id_t tick, tick_info_t *info)
{

  int  iIndex;
  bool ks;

  #ifdef DRIVER_INCL_TRACE
  cs_trace(TRACE_KAL | TRACE_LEVEL_2, "tick_get_info 0x%08lx\n", tick, 0);
  #endif

  if(IS_VALID_TICKID(tick))
  {
     /* Has this timer been allocated ? */
     iIndex = INDEX_FROM_TICKID(tick);

     /* Get exclusive access before reading Timer Info */
     ks = critical_section_begin();

     if(ticks[iIndex].dwFlags & TIMER_FLAG_ALLOCATED)
     {
       /* Yes - load and return timer information */
       info->bRunning = ticks[iIndex].dwFlags & TIMER_FLAG_RUNNING;
       info->flags = ticks[iIndex].dwFlags;
       info->pfnCallback = ticks[iIndex].pfnCallback;
       info->pUserData = ticks[iIndex].pUserData;

       /* UnLock access */
       critical_section_end(ks);

       return(RC_OK);
     }
     else /* No - timer not already allocated */
     {
       /* UnLock access */
       critical_section_end(ks);

       #ifdef DRIVER_INCL_TRACE
       cs_error_log(ERROR_WARNING | MOD_KAL | RC_KAL_NOTHOOKED);
       #endif
       return(RC_KAL_NOTHOOKED);
     }
  }
  else /* Invalid timer handle passed */
  {
    #ifdef DRIVER_INCL_TRACE
    cs_error_log(ERROR_WARNING | MOD_KAL | RC_KAL_INVALID);
    #endif
    return(RC_KAL_INVALID);
  }
}

/********************************************************************/
/*  FUNCTION:    tick_id_from_name                                  */
/*                                                                  */
/*  PARAMETERS:  name - name of the timer whose id is to be         */
/*                          determined                              */
/*                                                                  */
/*  DESCRIPTION: Get the ID of the timer whose Name is passed.      */
/*                                                                  */
/*  RETURNS:     RC_OK if the tick object is found                  */
/*               RC_KAL_INVALID if the object is not found          */
/*                                                                  */
/********************************************************************/
int tick_id_from_name(const char *name, tick_id_t *timer)
{

  int  i;
  bool ks;

  #ifdef DRIVER_INCL_TRACE
  cs_trace(TRACE_KAL | TRACE_LEVEL_2, "tick_id_from_name %s\n", (u_int32)name, 0);
  #endif

  /* Get exclusive access before reading Timer Info */
  ks = critical_section_begin();

  for(i=0;i<NUM_TICKS;i++)
  {
     if((ticks[i].dwFlags & TIMER_FLAG_ALLOCATED) && (strcmp(name, ticks[i].name)==0))
     {
       *timer = TICKID_FROM_INDEX(i);

       /* UnLock access */
       critical_section_end(ks);

       return(RC_OK);
     }
  }

  /* UnLock access */
  critical_section_end(ks);

  /* If it comes here, the Name was not found */
  return(RC_KAL_INVALID);
}

/*****************************************************************************/
/* Additional timer callback which maps between old and new callback formats */
/*****************************************************************************/
void pfnTimerCompatCallback(timer_id_t timer, void * uUser)
{
  PFNTIMER pfnCallback;

  if (uUser)
  {
    pfnCallback=(PFNTIMER)uUser;

    pfnCallback(timer);
  }
}

/********************************/
/* Start a tick counter running */
/********************************/
void start_tick_counter(int iIndex)
{
  /**********************************************************/
  /* WARNING: Must be called from within a critical section */
  /**********************************************************/

  ticks[iIndex].dwFlags |= TIMER_FLAG_STARTING;

  /*
   * Must ensure that the tick_timer_state_change() handler
   * will not make any callouts since we'll be calling the
   * handler not from an ISR and under a critical section
   * which means no callouts.  Let the ISR handler take care
   * of the callouts exclusively...
   */

  tick_timer_state_change(FALSE, 0, 0); /* Critical section safe
                                           only when the last two
                                           arguments are both 0's */
}

/***********************/
/* Stop a tick counter */
/***********************/
void stop_tick_counter(int iIndex)
{
  /**********************************************************/
  /* WARNING: Must be called from within a critical section */
  /**********************************************************/

  ticks[iIndex].dwFlags |= TIMER_FLAG_STOPPING;

  /*
   * Must ensure that the tick_timer_state_change() handler
   * will not make any callouts since we'll be calling the
   * handler not from an ISR and under a critical section
   * which means no callouts.  Let the ISR handler take care
   * of the callouts exclusively...
   */

  tick_timer_state_change(FALSE, 0, 0); /* Critical section safe
                                           only when the last two
                                           arguments are both 0's */
}

/********************************************************************/
/* Watchdog timer ISR. Required to prevent spontaneous system reset */
/********************************************************************/
int kal_watchdog_timer_isr(u_int32 dwIntID, bool bFIQ, PFNISR *pfnChain)
{
   /* Clear the timer interrupt */
   clear_timer_int(WATCHDOG_TIMER);

   return(RC_ISR_HANDLED);
}

/**************************************************************/
/* System timer ISR. Call all registered system timer clients */
/**************************************************************/
int kal_system_timer_isr(u_int32 dwIntID, bool bFIQ, PFNISR *pfnChain)
{
   int iLoop;
   ISR_TIMING_CHECK_LOCALS;

   ++uSysTickCount;

   for (iLoop = MAX_SYSTIMER_CLIENTS-1; iLoop >= 0; iLoop--)
   {
     if(system_timer_clients[iLoop].pfnHandler != NULL)
     {
       ISR_TIMING_CHECK_START(system_timer_clients[iLoop].pfnHandler, TIMERID_FROM_INDEX(SYS_TIMER));
       system_timer_clients[iLoop].pfnHandler(TIMERID_FROM_INDEX(SYS_TIMER),
                                             system_timer_clients[iLoop].pArg);
       ISR_TIMING_CHECK_END;                                      
     }                                        
   }

   /* Clear the timer interrupt */
   clear_timer_int(SYS_TIMER);

   return(RC_ISR_HANDLED);
}

/******************************************/
/* Handler for all tick timer interrupts  */
/******************************************/
int kal_tick_timer_isr(u_int32 dwIntID, bool bFIQ, PFNISR *pfnChain)
{
  u_int32 dwCount;
  u_int32 dwNumMs;
  #ifdef DEBUG
  #ifdef INT_LATENCY_CHECK
  u_int32 dwLatency;
  #endif
  #endif

  /* Read the timer and let it run so we can correct for elapsed time later */
  dwCount = read_hw_timer(TICK_TIMER);
  dwNumMs = dwCount / TICKS_PER_MS;

  /* If required, track the interrupt latency */
  #ifdef DEBUG
  #ifdef INT_LATENCY_CHECK
  dwLatency = (dwCount/TICKS_PER_US) - gdwCountsToNextTick;

  if(dwLatency < NUM_LATENCY_SAMPLES)
    gdwIntLatency[dwLatency]++;
  else
    gdwLongIntLatency++;
  #endif
  #endif

  /* Clear the timer interrupt before calling callback function. */
  clear_timer_int(TICK_TIMER);   

  /* Handle all the tick timers */
  tick_timer_state_change(TRUE, dwCount, dwNumMs);

  return(RC_ISR_HANDLED);
}

/***********************************************************************/
/* This function is called both by the tick timer ISR and whenever a   */
/* tick timer is to be started or stopped. If the ISR calls it, the    */
/* function will make all necessary callbacks to tick timers that have */
/* expired.                                                            */
/***********************************************************************/
void tick_timer_state_change(bool bInt, u_int32 dwCount, u_int32 dwNumMs)
{
  u_int32 dwCountEnd;
  int     iNextTimeout = MAX_TICK_PERIOD;
  int     iLoop;
  int     iNumRunning = 0;
  ISR_TIMING_CHECK_LOCALS;
  
/*********************************************************************************/
/* WARNING: This function is called from a critical section and during interrupt */
/*          processing. DO NOT ADD ANY TRACE, RTOS OR C RUNTIME CALLS!           */
/*********************************************************************************/

  /*
   * If called from the critical section, and not the ISR, read the timer
   * value since we need it for calculating the elapsed time since the
   * last time we were in this function or else tick timers will be starved.
   * While this isn't the most accurate, it's MUCH better than not taking
   * it into account at all (i.e. starved tick timers).
   */
  if(!bInt)
  {
    /* Read the timer and let it run so we can correct for elapsed time later */
    dwCount = read_hw_timer(TICK_TIMER);
    dwNumMs = dwCount / TICKS_PER_MS;
  }

  /*
   * Stop timer before calling callback function (critical section safe)
   */
  stop_hw_timer(TICK_TIMER);

  /* Look through the tick timers for any that have expired and */
  /* call them back                                             */
  for (iLoop = NUM_TICKS-1; iLoop >= 0; iLoop--)
  {
    /* Look for timers that are signalled to start and set them up */
    if (ticks[iLoop].dwFlags & TIMER_FLAG_STARTING)
    {
      ticks[iLoop].dwFlags &= ~TIMER_FLAG_STARTING;
      ticks[iLoop].dwFlags |= TIMER_FLAG_RUNNING;
      ticks[iLoop].iMsToNextTick = ticks[iLoop].iPeriodMs + (int)dwNumMs;
    }

    /* Look for stopping timers and shut them down */
    if (ticks[iLoop].dwFlags & TIMER_FLAG_STOPPING)
    {
      ticks[iLoop].dwFlags &= ~(TIMER_FLAG_RUNNING | TIMER_FLAG_STOPPING);
    }

    /* Process all timers that are running */
    if (ticks[iLoop].dwFlags & TIMER_FLAG_RUNNING)
    {

      /* Keep track of the number of running timers */
      iNumRunning++;

      /* Subtract the elapsed time from the number of milliseconds till */
      /* this tick timer times out.                                     */
      ticks[iLoop].iMsToNextTick -= dwNumMs;

      /*
       * If called from the ISR and if this tick timer time-ed out, handle it
       * Cannot make callbacks from critical section so let the timeouts be
       * handled the next time we come back in from the ISR
       */
      if (bInt && (ticks[iLoop].iMsToNextTick <= 0))
      {
        if (ticks[iLoop].dwFlags & TIMER_FLAG_ONESHOT) /* One-shot timer case */
        {
          ticks[iLoop].dwFlags &= ~(TIMER_FLAG_RUNNING | TIMER_FLAG_ONESHOT);
          iNumRunning--;
          /* Call the tick timer's callback function */
          ISR_TIMING_CHECK_START(ticks[iLoop].pfnCallback, TICKID_FROM_INDEX(iLoop));
          ticks[iLoop].pfnCallback(TICKID_FROM_INDEX(iLoop), ticks[iLoop].pUserData);
          ISR_TIMING_CHECK_END;
        }
        else /* Repeating timer case */
        {
          /* This loop ensures that callbacks don't miss ticks. If, for some reason, the    */
          /* tick interrupt latency is longer than the tick timer period, we call the       */
          /* callback multiple times in quick succession to ensure that it gets the correct */
          /* number of ticks (even if they are not equally spaced)                          */
          do
          {
            /* If this was a one-shot timer, turn it off before its callback (since the */
            /* callback may turn it back on)                                            */
            if (ticks[iLoop].dwFlags & TIMER_FLAG_ONESHOT)
            {
              ticks[iLoop].dwFlags &= ~(TIMER_FLAG_RUNNING | TIMER_FLAG_ONESHOT);
              iNumRunning--;
              ticks[iLoop].iMsToNextTick = ticks[iLoop].iPeriodMs; /* Cause loop to exit */
            }

            /* Call the tick timer's callback function */
            ISR_TIMING_CHECK_START(ticks[iLoop].pfnCallback, TICKID_FROM_INDEX(iLoop));
            ticks[iLoop].pfnCallback(TICKID_FROM_INDEX(iLoop), ticks[iLoop].pUserData);
            ISR_TIMING_CHECK_END;
            
          } while ((ticks[iLoop].dwFlags & TIMER_FLAG_RUNNING) && (ticks[iLoop].iMsToNextTick += ticks[iLoop].iPeriodMs ) <= 0 );
        }
      }

      /*
       * If this timer's next timeout is the shortest yet found, keep a track of it
       * but only if it's not a ONE_SHOT tick timer and it's been shut off!
       */
      if((ticks[iLoop].dwFlags & TIMER_FLAG_RUNNING) &&
         (ticks[iLoop].iMsToNextTick < iNextTimeout))
      {
        /*
         * Ensure iNextTimeout is never set less than 0.  This could happen if
         * called from critical section (i.e. not ISR, but from tick_start or
         * tick_stop).
         */
        iNextTimeout = (ticks[iLoop].iMsToNextTick>0) ? ticks[iLoop].iMsToNextTick : 0;
      }
    }
  }

  /* By this time, we have called back all functions that need called, we have calculated */
  /* the minimum time to the next tick and we know if any timers are running. All that    */
  /* remains to be done is to reset the timer timeout value and restart the timer at zero.*/

  dwCountEnd = read_hw_timer(TICK_TIMER);
  gdwLastCorrection = dwCountEnd - dwCount;

  /*
   * To do. Currently we don't correct for the interrupt latency or the time
   * it takes to process all callbacks. We should use gdwLastCorrection for
   * this. Without this correction, all tick timers will run slightly slow
   * but this is probably acceptable for the timer being.
   */
  if (iNumRunning)
  {
    gbTickCounterRunning = TRUE;
    gdwCountsToNextTick = iNextTimeout * 1000;
    /*
     * If the count is 0, make it 1us (nearly 0) so the set_hw_timer() function
     * will not re-use the old value.
     */
    if(!gdwCountsToNextTick)
    {
        gdwCountsToNextTick = 1;
    }
    set_hw_timer(TICK_TIMER, gdwCountsToNextTick);
    start_hw_timer_nowrap(TICK_TIMER);
  }
  else
  {
    /* Leave a marker telling us that the timer is stopped */
    gbTickCounterRunning = FALSE;
  }

}

/*************************************************************/
/* General ISR for all timers other than system and watchdog */
/*************************************************************/
int kal_general_timer_isr(u_int32 dwIntID, bool bFIQ, PFNISR *pfnChain)
{
   int iLoop;
   ISR_TIMING_CHECK_LOCALS;

   /* If this timer has been claimed, call the relevant handler */
   for(iLoop = 0; iLoop < NUM_TIMERS; iLoop++)
   {
      if(timers[iLoop].dwIntMask == dwIntID)
      {
        if(timers[iLoop].dwFlags & TIMER_FLAG_ALLOCATED)
        {
          /* If the timer is one shot, turn it off */
          if(timers[iLoop].dwFlags & TIMER_FLAG_ONESHOT)
            hwtimer_stop(TIMERID_FROM_INDEX(iLoop));

          /************************************************************/
          /* We must turn a one shot timer off before calling the ISR */
          /* since the handler may wish to turn the timer back on. If */
          /* we turn it off afterwards, we prevent ISRs from being    */
          /* able to do this.                                         */
          /************************************************************/
          ISR_TIMING_CHECK_START (timers[iLoop].pfnCallback, TIMERID_FROM_INDEX(iLoop));
          timers[iLoop].pfnCallback(TIMERID_FROM_INDEX(iLoop), timers[iLoop].pUserData);
          ISR_TIMING_CHECK_END;
          
          *pfnChain = timers[iLoop].pfnChain;

        }

        /* We found the interrupt so clear it and return */

        clear_timer_int(iLoop);
        return(RC_ISR_HANDLED);

      }
   }

   /* If we get here, we have a problem. We got an interrupt from an */
   /* unknown timer !                                                */

   return(RC_ISR_NOTHANDLED);
}

/**********************************************************************/
/* Dummy ISR that merely flags an error if an unhooked interrupt hits */
/**********************************************************************/
int kal_dummy_isr(u_int32 dwIntID, bool bFIQ, PFNISR *pfnChain)
{
   u_int32 dwPic, dwInt;
   interrupt_vector *pVector;

   dwPic = PIC_FROM_INT_ID(dwIntID);
   dwInt = INT_FROM_INT_ID(dwIntID);

   if((dwPic < (u_int32)NUM_PICS) && (dwInt < (u_int32)max_vectors[dwPic]))
   {
     pVector = vector_lists[dwPic];
     pVector[dwInt].dwFlags |= INT_FLAG_ERROR;
     pVector[dwInt].dwCount++;
   }

   return(RC_ISR_HANDLED);
}

#if (INTEXP_TYPE != NOT_PRESENT)
/*************************************************/
/* Top level expander interrupt handler function */
/*************************************************/
int kal_exp_fanout_isr(u_int32 dwIntID, bool bFIQ, PFNISR *pfnChain)
{
   u_int32           dwInt;
   u_int32           uIntId;
   u_int32           iRetcode;
   interrupt_vector *pVector;
   PFNISR            pfnChainInt;
   
   /* Find out which is the lowest GPIO requesting service */
   dwInt = get_active_exp_int();

   if(dwInt == (u_int32)-1)
   {
      /* Problem - no expansion interrupts are active ! */
     *pfnChain = kal_exp_chain;
     return(RC_ISR_NOTHANDLED);
   }

   /***************************************************/
   /* Do we have a vector hooked for this interrupt ? */
   /***************************************************/
   pVector = vector_lists[PIC_EXPANSION];
   uIntId = CALC_INT_ID(PIC_EXPANSION, dwInt);

   if(pVector[dwInt].dwFlags & INT_FLAG_HOOKED)
   {
      /**************************/
      /* Yes - call the handler */
      /**************************/
      pVector[dwInt].dwCount++;
      pfnChainInt = pVector[dwInt].pfnVector;

      do
      {
        /* If the handler returns RC_INT_NOTHANDLED and a new pfnChain */
        /* value, we call it, and keep going until someone handles the */
        /* interrupt or a NULL pointer is returned for pfnChain        */
        iRetcode = (u_int32)pfnChainInt(uIntId, bFIQ, &pfnChainInt);

      } while((iRetcode != RC_ISR_HANDLED) && (pfnChainInt != NULL));
      #ifdef DEBUG
      #ifdef INT_LATENCY_CHECK
      int_latency_check( uIntId );
      #endif
      #endif 
   }
   #ifdef DRIVER_INCL_TRACE
   else
   {
     /**************/
     /* No - error */
     /**************/
     isr_error_log(ERROR_WARNING | MOD_KAL | RC_KAL_NOTHOOKED);
   }
   #endif

   /*********************************************/
   /* Clear the interrupt in the expanded PIC -
      this must be done after servicing it due 
      to the nature of some of these coprocessor 
      interrupts - they continue to cause bits 
      to be set (i.e. clears are useless) until 
      they're truly serviced...                 */
   /*********************************************/
   clear_pic_interrupt(PIC_EXPANSION, dwInt);

#if RTOS == NUP
   /* re-enable the expanded interrupt */
   restore_exp_int_enable_bit ( dwInt );
#endif

   /* Return to the main KAL interrupt handler */

   *pfnChain = kal_exp_chain;
   return(RC_ISR_HANDLED);
}
#endif

/*********************************************/
/* Top level GPIO interrupt handler function */
/*********************************************/
int kal_gpio_fanout_isr(u_int32 dwIntID, bool bFIQ, PFNISR *pfnChain)
{
   u_int32           dwInt;
#ifdef COLORADO_GPIO_EDGE_INTR_BUG
   u_int32           bit, bank, pos_en, neg_en;
   u_int32           intr_en[NUM_GPI_BANKS];
   u_int32           pre_clear_stat[NUM_GPI_BANKS];
   u_int32           post_clear_stat[NUM_GPI_BANKS];
   bool              gpio_workaround_complete;
#endif // COLORADO_GPIO_EDGE_INTR_BUG
   u_int32           uIntId;
   u_int32           iRetcode;
   interrupt_vector *pVector;
   PFNISR            pfnChainInt;
   ISR_TIMING_CHECK_LOCALS;

   /********************************************************/
   /* Find out which is the lowest GPIO requesting service */
   /********************************************************/
   dwInt = get_active_gpio_int();

   /************************************/
   /* Do we have a spurrious interrupt */
   /************************************/
   if(dwInt == (u_int32)-1)
   {
      /* Problem - no GPIO interrupts are active ! */
     *pfnChain = kal_gpio_chain;
     return(RC_ISR_NOTHANDLED);
   }

#ifdef COLORADO_GPIO_EDGE_INTR_BUG

/*
 * Errata:
 *
 * Message to customer, 4/24/01
 *
 * Upon closer inspection, an edge can be missed by the gpio interrupt status
 * register on the CX22490.  This can occur when a write to the IRQ_STATUS
 * register occurs at the same time as an edge event.
 *
 * Design Specifics
 * ----------------
 *
 * The gpio interrupts are edge detected.  An edge is detected by flopping the
 * incoming gpio and comparing the previous and current value.
 *
 * The IRQ_STATUS register, conditional on the IRQ_ENABLE register, generates the
 * interrupts to the ARM microprocessor.  The IRQ_STATUS register is updated once
 * per asxclk cycle (54Mhz):
 *
 * first priority  - write to IRQ_STATUS bank1(Clearing an IRQ),
 * second priority - write to IRQ_STATUS bank2,
 * third priority  - write to IRQ_STATUS bank3,
 * last priority by edge detection.
 *
 * Therefore, if a write to an -ANY- IRQ_STATUS register and -ANY- edge event
 * occur during the same asxclk cycle, the edge will be missed.
 *
 * The proper hardware fix would be to "or" a write to clean and an incoming edge
 * together, essentially ignoring the write to clear, if the two events happen a
 * the same time.
 *
 * Possible Workaround
 * -------------------
 *
 * 1) Check the pin value READ register of the possible interrupt sources
 * 2) Write the IRQ_STATUS register to clear the interrupt
 * 3) Sample the gpio pin were an interrupt can occur and compare to value in (1)
 * 4) If a gpio line has toggled in the proper edge direction, go to appropriate
 * interrupt service routine
 */

   /*
    * Read the intr enable registers and the read status registers for any
    * banks which have at least one enable bit enabled
    */
   for(bank=0;bank<NUM_GPI_BANKS;bank++)
   {
      /*
       * Read intr enable since we only care about interrupts that are enabled
       */
      intr_en[bank] = *((LPREG)(GPI_INT_ENABLE_REG+(bank*GPI_BANK_SIZE)));
      if(intr_en[bank])
      {
         /*
          * Read status before the clear
          */
         pre_clear_stat[bank] = *((LPREG)(GPI_READ_REG+(bank*GPI_BANK_SIZE))) & intr_en[bank];
      }
   }
#endif // COLORADO_GPIO_EDGE_INTR_BUG

   /*********************************************/
   /* Clear the interrupt in the PIC and return */
   /*********************************************/
   clear_pic_interrupt(PIC_GPIO, dwInt);

#ifdef COLORADO_GPIO_EDGE_INTR_BUG
   /*
    * Re-read the read status registers for any banks which have at least one
    * enable bit enabled
    */
   for(bank=0;bank<NUM_GPI_BANKS;bank++)
   {
      if(intr_en[bank])
      {
         /*
          * Read status after the clear
          */
         post_clear_stat[bank] = *((LPREG)(GPI_READ_REG+(bank*GPI_BANK_SIZE))) & intr_en[bank];
         /*
          * Mask off real (i.e. not missed) interrupts and unchanged bits
          */
         intr_en[bank] &= (~(*((LPREG)(GPI_INT_REG+(bank*GPI_BANK_SIZE)))) &
                          (pre_clear_stat[bank] ^ post_clear_stat[bank]));
      }
   }

   /*
    * For the workaround, we loop calling the handlers for any missed interrupts.
    * Call the handler for the original interrupt (dwInt) first of course.
    * At this point, the intr_en array contains only potentially missed
    * interrupt bits.
    */
   bank = 0; bit = 0;
   gpio_workaround_complete = FALSE;
   do
   {
#endif // COLORADO_GPIO_EDGE_INTR_BUG

   /***************************************************/
   /* Do we have a vector hooked for this interrupt ? */
   /***************************************************/
   pVector = vector_lists[PIC_GPIO];
   uIntId = CALC_INT_ID(PIC_GPIO, dwInt);

   if(pVector[dwInt].dwFlags & INT_FLAG_HOOKED)
   {
      /**************************/
      /* Yes - call the handler */
      /**************************/
      pVector[dwInt].dwCount++;
      pfnChainInt = pVector[dwInt].pfnVector;
      /*#ifdef DEBUG
      #ifdef INT_LATENCY_CHECK
      int_time_begin = *trace_timer_value_addr;
      #endif
      #endif  */
      do
      {
        /* If the handler returns RC_INT_NOTHANDLED and a new pfnChain */
        /* value, we call it, and keep going until someone handles the */
        /* interrupt or a NULL pointer is returned for pfnChain        */
        ISR_TIMING_CHECK_START(pfnChainInt, uIntId);
        iRetcode = (u_int32)pfnChainInt(uIntId, bFIQ, &pfnChainInt);
        ISR_TIMING_CHECK_END;

      } while((iRetcode != RC_ISR_HANDLED) && (pfnChainInt != NULL));
      #ifdef DEBUG
      #ifdef INT_LATENCY_CHECK
      if( pfnChainInt != NULL )
      {
         int_latency_check( uIntId );
      }
      #endif
      #endif 
   }
   #ifdef DRIVER_INCL_TRACE
   else
   {
     /**************/
     /* No - error */
     /**************/
     isr_error_log(ERROR_WARNING | MOD_KAL | RC_KAL_NOTHOOKED);
   }
   #endif

#ifdef COLORADO_GPIO_EDGE_INTR_BUG
   /*
    * Loop handling callouts for all missed interrupts
    */
   dwInt = 0;
   while(!dwInt && !gpio_workaround_complete)
   {
      /*
       * First, see if there are any interrupts enabled by bank
       */
      while(!intr_en[bank])
      {
         /*
          * None in this bank, goto the next and reset the bit to 0
          */
         ++bank;
         bit = 0;
         if(bank >= NUM_GPI_BANKS)
         {
            /*
             * No more missed interrupts, return
             */
            gpio_workaround_complete = TRUE;
            break;
         }
      }

      /*
       * Is there at least one interrupt enabled?
       */
      if(!gpio_workaround_complete)
      {
         /*
          * Look for the next missed interrupt bit set
          */
         while((bit<32) && !(intr_en[bank]&(1<<bit)))
         {
            ++bit;
         }
         /*
          * If there are any bits different in the pre-clear read status vs
          * the post-clear status, read the positive and negative interrupt
          * enable registers
          */
         pos_en = *((LPREG)(GPI_POS_EDGE_REG+(bank*GPI_BANK_SIZE))) & (1<<bit);
         neg_en = *((LPREG)(GPI_NEG_EDGE_REG+(bank*GPI_BANK_SIZE))) & (1<<bit);
   
         /*
          * A bit is different, now look at the positive and
          * negative enables to see if the value changed in the
          * correct direction to trigger an interrupt (i.e. if
          * positive enable, the bit changes from a 0->1.  If
          * negative enable, the bit changes from a 1->0.
          */
         if((pos_en && (post_clear_stat[bank]&(1<<bit))) ||
            (neg_en && (pre_clear_stat[bank]&(1<<bit))))
         {
            /*
             * We have a missed interrupt. Setup dwInt and loop back to the
             * top of the handler callouts
             */
            dwInt = (bank<<5)+bit;
         }
         /*
          * Remove bit from intr_en array
          */
         intr_en[bank] &= ~(1<<bit);
      }
   }

   /*
    * Loop handling all missed interrupts
    */
   }
   while(!gpio_workaround_complete);
#endif // COLORADO_GPIO_EDGE_INTR_BUG

   /*
    * Return to the main KAL interrupt handler
    */
   *pfnChain = kal_gpio_chain;
   return(RC_ISR_HANDLED);
}

/**********************************************/
/* Top level timer interrupt handler function */
/**********************************************/
int kal_timer_fanout_isr(u_int32 dwIntID, bool bFIQ, PFNISR *pfnChain)
{
   u_int32           dwInt;
   u_int32           uIntId;
   u_int32           iRetcode;
   interrupt_vector *pVector;
   PFNISR            pfnChainInt;
   ISR_TIMING_CHECK_LOCALS;

   /* Find out which is the lowest timer requesting service */
   dwInt = get_active_timer_int();
   if(dwInt == (u_int32)-1)
   {
      /* Problem - no timer interrupts are active ! */
     *pfnChain = kal_timer_chain;
     return(RC_ISR_NOTHANDLED);
   }

   /***************************************************/
   /* Do we have a vector hooked for this interrupt ? */
   /***************************************************/
   pVector = vector_lists[PIC_TIMERS];
   uIntId = CALC_INT_ID(PIC_TIMERS, dwInt);

   if(pVector[dwInt].dwFlags & INT_FLAG_HOOKED)
   {
      /**************************/
      /* Yes - call the handler */
      /**************************/
      pVector[dwInt].dwCount++;
      pfnChainInt = pVector[dwInt].pfnVector;
      /*#ifdef DEBUG
      #ifdef INT_LATENCY_CHECK
      int_time_begin = *trace_timer_value_addr;
      #endif
      #endif */
      do
      {
        /* If the handler returns RC_INT_NOTHANDLED and a new pfnChain */
        /* value, we call it, and keep going until someone handles the */
        /* interrupt or a NULL pointer is returned for pfnChain        */
        ISR_TIMING_CHECK_START (pfnChainInt, uIntId);
        iRetcode = (u_int32)pfnChainInt(uIntId, bFIQ, &pfnChainInt);
        #ifdef ISR_TIMING_CHECK
        /* Check interrupt time for timers which are not fanned out */
        if(uIntId != INT_SYSTIMER)
        {   
          ISR_TIMING_CHECK_END;
        }  
        #endif

      } while((iRetcode != RC_ISR_HANDLED) && (pfnChainInt != NULL));
      #ifdef DEBUG
      #ifdef INT_LATENCY_CHECK
      int_latency_check( uIntId );
      #endif
      #endif 

   }
   #ifdef DRIVER_INCL_TRACE
   else
   {
     /**************/
     /* No - error */
     /**************/
     isr_error_log(ERROR_WARNING | MOD_KAL | RC_KAL_NOTHOOKED);
   }
   #endif

   /*********************************************/
   /* Clear the interrupt in the PIC and return */
   /*********************************************/
   clear_pic_interrupt(PIC_TIMERS, dwInt);

   /* Return to the main KAL interrupt handler */

   *pfnChain = kal_timer_chain;
   return(RC_ISR_HANDLED);
}

/*****************************************************************/
/* Register a handler for a given interrupt source. Interrupt is */
/* NOT enabled on exit from this call. New algorithm.            */
/*****************************************************************/
int int_register_isr(u_int32 dwIntID,
                     PFNISR  pfnHandler,
                     bool    bFIQ,
                     bool    bUnused,
                     PFNISR *pfnChain)
{
   u_int32 uPic;
   u_int32 uInt;
   interrupt_vector *pVector;

   #ifdef DRIVER_INCL_TRACE
   trace_new(TRACE_KAL | TRACE_LEVEL_2, "int_register_isr 0x%08lx\n", dwIntID);
   #endif

   if ((pfnHandler == (PFNISR)NULL) || (pfnChain == (PFNISR *)NULL))
   {
      isr_error_log(ERROR_WARNING | MOD_KAL | RC_KAL_BADPTR);
      return(RC_KAL_BADPTR);
   }
   
   /********************************************/
   /* For the time being, we don't handle FIQs */
   /********************************************/
   if(bFIQ)
   {
      #ifdef DRIVER_INCL_TRACE
      isr_error_log(ERROR_WARNING | MOD_KAL | RC_KAL_INVALID);
      #endif
      return(RC_KAL_INVALID);
   }

   /*******************************************/
   /* Have we been passed a valid interrupt ? */
   /*******************************************/
   uPic = PIC_FROM_INT_ID(dwIntID);
   uInt = INT_FROM_INT_ID(dwIntID);

   if((uPic >= (u_int32)NUM_PICS) || (uInt >= (u_int32)max_vectors[uPic]))
   {
      #ifdef DRIVER_INCL_TRACE
      isr_error_log(ERROR_WARNING | MOD_KAL | RC_KAL_INVALID);
      #endif
      return(RC_KAL_INVALID);
   }

   pVector = vector_lists[uPic];

#if (INTEXP_TYPE != NOT_PRESENT)
   /* Allow only one (fanout) ISR to register with expansion interrupt */
   if ((uPic == PIC_CENTRAL) &&
       (uInt == ITC_EXPANSION_POS) && 
       (pVector[uInt].pfnVector != (PFNISR)kal_dummy_isr))
   {
     #ifdef DRIVER_INCL_TRACE
       #ifdef DEBUG
         isr_error_log(ERROR_FATAL | MOD_KAL | RC_KAL_INUSE);
       #else
         isr_error_log(ERROR_WARNING | MOD_KAL | RC_KAL_INUSE);
       #endif
     #endif
     return(RC_KAL_INUSE);
   }
#endif

   /* Allow only one (fanout) ISR to register with GPIO interrupt */
   if ((uPic == PIC_CENTRAL) &&
       (uInt == ITC_GPIO_POS) && 
       (pVector[uInt].pfnVector != (PFNISR)kal_dummy_isr))
   {
     #ifdef DRIVER_INCL_TRACE
       #ifdef DEBUG
         isr_error_log(ERROR_FATAL | MOD_KAL | RC_KAL_INUSE);
       #else
         isr_error_log(ERROR_WARNING | MOD_KAL | RC_KAL_INUSE);
       #endif
     #endif
     return(RC_KAL_INUSE);
   }
   
   /* Place ISR on chain... */
   *pfnChain = pVector[uInt].pfnVector;

   if(*pfnChain != (PFNISR)kal_dummy_isr)
     pVector[uInt].dwFlags |= INT_FLAG_CHAINED;
   else
     *pfnChain = (PFNISR)0;

   pVector[uInt].pfnVector = pfnHandler;
   pVector[uInt].dwFlags |= INT_FLAG_HOOKED;

   return(RC_OK);
}

/**************************************************************************/
/* Enable (unmask) a particular interrupt source that has previously been */
/* registered using a call to int_register_isr. New algorithm             */
/*                                                                        */
/* (NOTE: This function needs to be critical_section safe for calls in    */
/* the serial port handling code under NUP in NUPBSP)                     */
/**************************************************************************/
int int_enable(u_int32 dwIntID)
{
   u_int32 uPic;
   u_int32 uInt;
   interrupt_vector *pVector;

   #ifdef DRIVER_INCL_TRACE
   cs_trace(TRACE_KAL | TRACE_LEVEL_2, "int_enable 0x%08lx\n", dwIntID, 0);
   #endif

   /***********************************/
   /* Have we hooked this interrupt ? */
   /***********************************/
   uPic = PIC_FROM_INT_ID(dwIntID);
   uInt = INT_FROM_INT_ID(dwIntID);

   if((uPic >= (u_int32)NUM_PICS) || (uInt >= (u_int32)max_vectors[uPic]))
   {
      #ifdef DRIVER_INCL_TRACE
      cs_error_log(ERROR_WARNING | MOD_KAL | RC_KAL_INVALID);
      #endif
      return(RC_KAL_INVALID);
   }

   pVector = vector_lists[uPic];

   if(!(pVector[uInt].dwFlags & INT_FLAG_HOOKED))
   {
      #ifdef DRIVER_INCL_TRACE
      cs_error_log(ERROR_WARNING | MOD_KAL | RC_KAL_NOTHOOKED);
      #endif
      return(RC_KAL_NOTHOOKED);
   }

   pVector[uInt].dwCount = 0;
   pVector[uInt].dwFlags |= INT_FLAG_ACTIVE;

   enable_hw_int(uPic, uInt);

   return(RC_OK);
}

/**************************************************************************/
/* Disable (mask) a particular interrupt source that has previously been  */
/* registered using a call to int_registeR_isr. New algorithm.            */
/**************************************************************************/
int int_disable(u_int32 dwIntID)
{
   u_int32 uPic;
   u_int32 uInt;
   interrupt_vector *pVector;

   #ifdef DRIVER_INCL_TRACE
   isr_trace_new(TRACE_KAL | TRACE_LEVEL_2, "int_disable 0x%08lx\n", dwIntID, 0);
   #endif

   /***********************************/
   /* Have we hooked this interrupt ? */
   /***********************************/
   uPic = PIC_FROM_INT_ID(dwIntID);
   uInt = INT_FROM_INT_ID(dwIntID);

   if((uPic >= (u_int32)NUM_PICS) || (uInt >= (u_int32)max_vectors[uPic]))
   {
      #ifdef DRIVER_INCL_TRACE
      isr_error_log(ERROR_WARNING | MOD_KAL | RC_KAL_INVALID);
      #endif
      return(RC_KAL_INVALID);
   }

   pVector = vector_lists[uPic];
   if(!(pVector[uInt].dwFlags & INT_FLAG_HOOKED))
   {
      #ifdef DRIVER_INCL_TRACE
      isr_error_log(ERROR_WARNING | MOD_KAL | RC_KAL_NOTHOOKED);
      #endif
      return(RC_KAL_NOTHOOKED);
   }

   disable_hw_int(uPic, uInt);

   pVector[uInt].dwFlags &= ~INT_FLAG_ACTIVE;

   return(RC_OK);
}

/***********************************/
/* Top level KAL interrupt handler */
/***********************************/
void kal_int_handler(u_int32 uInts, u_int32 uPic, bool bFIQ)
{
   u_int32 uInterrupt;
   u_int32 uWork;
   u_int32 uNewPic;
   int     iRetcode;
   PFNISR  pfnChain;
   interrupt_vector *pVector;
   #ifdef DEBUG
   u_int32 uDummy;
   #endif
   ISR_TIMING_CHECK_LOCALS;

   iProcessingInt++;

   /* This is a very simplistic handler. No thought is given to        */
   /* priority and enabling/disabling interrupts just now but          */
   /* this will likely be added later. The handler services the        */
   /* interrupt with the highest bit position in the uInts parameter   */
   /* chaining if necessary, before returning. If more than 1 IRQ      */
   /* source is active, it is assumed that another interrupt will      */
   /* occur immediately on return or during the running of the handler */
   /* so only a single interrupt bit need be considered each time we   */
   /* are called.                                                      */

   /**************************************************/
   /* Get the highest bit set in the uInts parameter */
   /**************************************************/
   uInterrupt = find_highest_bit(uInts);

   /******************************************************/
   /* No interrupt was signalled ! (Must indicate a bug) */
   /******************************************************/
   if(uInterrupt == (u_int32)-1)
   {
     gdwNoSignalInts++;
     #ifdef DRIVER_INCL_TRACE
     isr_error_log(ERROR_WARNING | MOD_KAL | RC_KAL_INVALID);
     #endif
     iProcessingInt--;
     return;
   }

   /***************************************************/
   /* Do we have a vector hooked for this interrupt ? */
   /***************************************************/
   uInterrupt = CALC_INT_ID(uPic, uInterrupt);
   uWork      = INT_FROM_INT_ID(uInterrupt);
   uNewPic    = PIC_FROM_INT_ID(uInterrupt);

   /*****************************************************/
   /* In debug builds, this gives us somewhere to break */
   /* to find non-timer interrupts.                     */
   /*****************************************************/
   #ifdef DEBUG
   if (uInterrupt != INT_TIMERS)
   {
      uDummy = 0;
   }

   /****************************************/
   /* ...or to find a particular interrupt */
   /****************************************/
   if(uInterrupt == uIntDebugCheck)
   {
     uDummy = 0;
   }
   #endif

   pVector = vector_lists[uNewPic];

   if(pVector[uWork].dwFlags & INT_FLAG_HOOKED)
   {
      /**************************/
      /* Yes - call the handler */
      /**************************/
      pVector[uWork].dwCount++;

      #ifdef DEBUG
      /* This is duplication of information but it makes looking at the */
      /* counts a whole lot easier in the debugger.                     */
      gdwIntCounts[uWork]++;
      #endif
      pfnChain = pVector[uWork].pfnVector;
      #ifdef DEBUG
      #ifdef INT_LATENCY_CHECK
      int_time_begin = *trace_timer_value_addr;
      #endif
      #endif
      do
      {
        /* If the handler returns RC_INT_NOTHANDLED and a new pfnChain */
        /* value, we call it, and keep going until someone handles the */
        /* interrupt or a NULL pointer is returned for pfnChain        */
        ISR_TIMING_CHECK_START(pfnChain, uInterrupt);
        iRetcode = (u_int32)pfnChain(uInterrupt, bFIQ, &pfnChain);
        
        #ifdef ISR_TIMING_CHECK
        /* Only check the interrupt time if this is not fanned out */
        if((uInterrupt != INT_TIMERS) &&
           #ifdef ITC_EXPANSION_POS
           (uInterrupt != INT_EXP) &&
           #endif
           (uInterrupt != INT_GPIO))
        {
          ISR_TIMING_CHECK_END;
        }
        #endif
        
      } while((iRetcode != RC_ISR_HANDLED) && (pfnChain != NULL));
      #ifdef DEBUG
      #ifdef INT_LATENCY_CHECK
      if( uInterrupt != 0x1e && uInterrupt != 0x1f ) /* not INT_TIMERS or INT_GPIO */
         int_latency_check( uInterrupt );
      #endif
      #endif 

   }
   #ifdef DRIVER_INCL_TRACE
   else
   {
     /**************/
     /* No - error */
     /**************/
     isr_error_log(ERROR_WARNING | MOD_KAL | RC_KAL_NOTHOOKED);
   }
   #endif

   /*********************************************/
   /* Clear the interrupt in the PIC and return */
   /*********************************************/
   clear_pic_interrupt(uNewPic, uWork);
#if RTOS == NUP
   nup_isr_mask_unmask (uWork);
#endif

   iProcessingInt--;
}

/************************************************************/
/* Carry out a soft reset of the IRD under software control */
/************************************************************/
void reboot_IRD(void)
{
  LPREG lpReset = (LPREG)RST_SOFTRESET_REG;

  /* Any write to this register resets the IRD! */
  *lpReset = 0x00;
}

/************************************************************/
/* Dealy for a fixed period of time then reboot the IRD.    */
/* Tracker #2485                                            */
/************************************************************/
void delay_and_reboot_IRD(void)
{
  LPREG lpReset = (LPREG)RST_SOFTRESET_REG;
  LPREG lpTValue = (LPREG)TIMER_NUM_VALUE_REG(0);
  LPREG lpTMode  = (LPREG)TIMER_NUM_MODE_REG(0);
  
  *lpTMode = 0x3; /* Timer enable & continue to count past limit */
  *lpTValue = 0;
  
  /* timer is fed by a 54 MHz.  54,000,000 ticks is one second */
  while(*lpTValue < 54000000);
  
  /* Any write to this register resets the IRD! */
  *lpReset = 0x00;
}

#if RTOS == PSOS || RTOS==NUP || RTOS==UCOS
/*************************************************************************/
/* For pSOS, we define the basic reboot() function as required by OpenTV */
/* 1.2. This is a predefined system call for vxWorks so we don't define  */
/* it again in this case.                                                */
/*************************************************************************/
void reboot(void)
{
  reboot_IRD();
}
#endif
/********************************************************************/
/*  FUNCTION:    get_system_time                                    */
/*                                                                  */
/*  PARAMETERS:  none                                               */
/*                                                                  */
/*  DESCRIPTION: Get the system low level time, in milliseconds.    */
/*               This is the number of system timer ticks since     */
/*               initialization, converted to milliseconds.         */
/*               NOTE: this will not work correctly if the system   */
/*               time tick period is not some even multiple of 1 ms */
/*                                                                  */
/*  RETURNS:     Low level system time, in milliseconds.            */
/*                                                                  */
/********************************************************************/
u_int32 get_system_time( void )
{
   return( uSysTickCount * uMsecPerTick );
}

u_int32 get_system_time_us( void )
{
   bool critical;
   u_int32 time_us;

/* Prevent Interrupts from happening */
   critical = critical_section_begin();

/* Read the raw hardware timer register */
   time_us = read_hw_timer(SYS_TIMER);

/* Now, check for interrupts pending */
   if (is_timer_pending(SYS_TIMER))
   {
   /* Here, if an interrupt is pending, we need to re-read the register */
   /* to guarantee that we have a "rolled-over" value, and make an adjustment */
   /* to the uSysTickCount value */
      time_us = (read_hw_timer(SYS_TIMER) / TICKS_PER_US)
         + (((uSysTickCount + 1) * uMsecPerTick) * 1000);
   }
   else
   {
   /* Here, an interrupt is not pending, so we can use the current value */
   /* of the uSysTickCount, and don't have to re-read the register */
      time_us = (time_us / TICKS_PER_US)
         + ((uSysTickCount * uMsecPerTick) * 1000) ;
   }

   critical_section_end(critical);

   return( time_us );
}

/********************************************************************/
/*  FUNCTION:    in_pci_mode                                        */
/*                                                                  */
/*  PARAMETERS:  none                                               */
/*                                                                  */
/*  DESCRIPTION: Returns true if chip is wired up/operating in      */
/*               PCI mode                                           */
/*                                                                  */
/*  RETURNS:     TRUE if PCI, FALSE if not                          */
/*                                                                  */
/********************************************************************/
bool in_pci_mode( void )
{
   LPREG cfg0 = (LPREG)PLL_CONFIG0_REG;

   if (*cfg0 & PLL_CONFIG0_CNXT_GENERIC_IO_PCI_MODE_MASK) 
   {
      return FALSE;
   } 
   else
   {
      return TRUE;
   }
   
}

/********************************************************************/
/*  FUNCTION:    pci_get_bar                                        */
/*                                                                  */
/*  PARAMETERS:  device and vendor ID, bar number, pointer to       */
/*               integer                                            */
/*                                                                  */
/*  DESCRIPTION: given a device and vendor ID number and a bar      */
/*               number, this function will return the value from   */
/*               that particular pci config register                */
/*                                                                  */
/*  RETURNS:     1 = success, 0 = failure                           */
/*                                                                  */
/********************************************************************/
int pci_get_bar( unsigned int dev_ven, unsigned int bar_no, u_int32 *bar )
{
   LPREG pPCICfgAddr = (LPREG)PCI_CFG_ADDR_REG;
   LPREG pPCICfgData = (LPREG)PCI_CFG_DATA_REG;
   u_int32 loc;
   int     i;

   if( bar_no > (unsigned int)5 ) 
        return( FALSE ); /* bogus input data */

   for ( i = 1 ; i < 31 ; i ++ )
   {
      loc = ( ( 0 << PCI_CFG_ADDR_BUS_SHIFT )      |
              ( 0 << PCI_CFG_ADDR_FUNCTION_SHIFT ) |
              ( i << PCI_CFG_ADDR_DEVICE_SHIFT ) );
      *pPCICfgAddr = loc | ( PCI_ID_OFF << PCI_CFG_ADDR_REGISTER_SHIFT );

      if ( dev_ven == *pPCICfgData )
      {
         *pPCICfgAddr = loc | ( ( PCI_BASE_ADDR_0_OFF + bar_no ) << PCI_CFG_ADDR_REGISTER_SHIFT );
         *bar = *pPCICfgData;
         return TRUE;
      }
   }

   return FALSE;
}

/********************************************************************/
/*  FUNCTION:    int_latency_check                                  */
/*                                                                  */
/*  PARAMETERS:  interrupt id                                       */
/*                                                                  */
/*  DESCRIPTION: Calculate the latency of interrupt, store          */
/*               the interrupt ID and iterrupt latency in a global  */
/*               array longestINTlatency if its latency is bigger   */
/*               than the smallest one in the array                 */
/*                                                                  */
/*  RETURNS:     none                                               */
/*                                                                  */
/********************************************************************/
#ifdef DEBUG
#ifdef INT_LATENCY_CHECK
static void int_latency_check( u_int32 dwIntID ) 
{        
   int i, insertIdx;

   int_time_end = *trace_timer_value_addr;
   if( int_time_end >= int_time_begin )
      int_time_diff = (int_time_end - int_time_begin) /54;
   else /* timer limit has reached sometime during the interrupt service routine */
      int_time_diff = 60000000 - ( ( int_time_begin - int_time_end ) /54);
   if( int_time_diff != 0 )
   {
      /* keep it in order */
      i=0; insertIdx =0;
      while( int_time_diff >longestINTlatency[i].int_latency && i< NUM_LONGEST_LATENCY )
      {
         i++;
         insertIdx = i;
      }
      if( insertIdx  )                 
      {
         for( i=0; i<insertIdx;i++ )
         {
            if( longestINTlatency[i+1].int_latency != 0 )
            {
               longestINTlatency[i].int_latency = longestINTlatency[i+1].int_latency;
               longestINTlatency[i].int_num = longestINTlatency[i+1].int_num;
            }
         }
         longestINTlatency[insertIdx -1].int_latency = int_time_diff;
         longestINTlatency[insertIdx -1].int_num = dwIntID;
      }
   }
}   
#endif
#endif

/****************************************************************************
 * Modifications:
 * $Log: 
 *  90   mpeg      1.89        5/6/04 5:25:35 PM      Miles Bintz     CR(s) 
 *        9016 9017 : fixed login in in_pci_mode
 *  89   mpeg      1.88        5/6/04 10:58:10 AM     Miles Bintz     CR(s) 
 *        9016 9017 : fix dvb baseband support in bronco/looneyville
 *  88   mpeg      1.87        5/4/04 10:17:33 AM     Miles Bintz     CR(s) 
 *        9016 9017 : added function to query pci mode
 *  87   mpeg      1.86        2/13/04 3:25:01 PM     Craig Dry       CR(s) 
 *        8409 : Changed "&& INT_FLAG_HOOKED" to "& INT_FLAG_HOOKED".  
 *        Fortunately
 *        this coding mistake has not yet caused any problems.
 *  86   mpeg      1.85        9/16/03 4:33:04 PM     Angela Swartz   SCR(s) 
 *        7477 :
 *        Use WATCHDOG sw config key to determine whether the Watchdog timer 
 *        should be turned on or off for the build; the default is on for 
 *        release build, and off for debug build
 *        
 *  85   mpeg      1.84        9/2/03 7:05:24 PM      Joe Kroesche    SCR(s) 
 *        7415 :
 *        removed unneeded header files and reordered to eliminate warnings 
 *        when
 *        building for PSOS
 *        
 *  84   mpeg      1.83        6/26/03 3:28:40 PM     Dave Wilson     SCR(s) 
 *        6826 :
 *        Fix to tick timer state change function to ensure that if a tick 
 *        timer
 *        callback function stops the timer, no more callbacks are generated. 
 *        Previously,
 *        if more than 1 callback would be generated, all callbacks would be 
 *        made even
 *        if the first one disabled the timer.
 *        
 *  83   mpeg      1.82        6/3/03 6:17:18 PM      Dave Wilson     SCR(s) 
 *        6701 6702 :
 *        c_not_from_critical_section now just hangs in a while(1) loop if 
 *        called from
 *        within a critical section. This allows you to see what went wrong (if
 *         you
 *        stop the debugger you will be stuck in this loop) and does not 
 *        recurse as the
 *        previous version did.
 *        
 *  82   mpeg      1.81        6/3/03 4:47:56 PM      Dave Wilson     SCR(s) 
 *        6652 6651 :
 *        Added code to optionally include ISR duration checking. If 
 *        ISR_TIMING_CHECK
 *        is defined when the code is built, a 10 entry array is created which 
 *        will
 *        store details of the 10 longest ISRs running in the system.
 *        
 *  81   mpeg      1.80        5/8/03 1:07:40 PM      Angela Swartz   SCR(s) 
 *        6265 6264 6263 :
 *        Correct build error for Critical Section latency checking code;
 *        Restore the correct value to the variable caller for critical section
 *         latency checking in c_critical_section_begin(); 
 *        remove the tabs
 *        
 *  80   mpeg      1.79        12/11/02 4:03:32 PM    Dave Wilson     SCR(s) 
 *        5150 :
 *        Removed all the conditional checks whose logic was reversed in the 
 *        last fix.
 *        This reversal of the logic caused Colorado builds to break since 
 *        INT_EXP is no
 *        longer defined for chips which have INTEXP_TYPE set to NOT_PRESENT.
 *        
 *  79   mpeg      1.78        12/11/02 1:38:18 PM    Brendan Donahe  SCR(s) 
 *        5132 :
 *        Fixed 2 == vs. != logic problems with INTEXP_TYPE (NOT_PRESENT vs. 
 *        INTEXP_WABASH)
 *        
 *        
 *  78   mpeg      1.77        12/11/02 11:18:26 AM   Dave Wilson     SCR(s) 
 *        5131 :
 *        Fixed a few cases where ITC_EXPANSION_POS (a bit position definition 
 *        in the
 *        chip header) had been used in place of INT_EXP (an HWLIB interrupt 
 *        identifier).
 *        Both had the same value but using the wrong label could have been 
 *        confusing.
 *        
 *  77   mpeg      1.76        12/10/02 3:00:10 PM    Dave Wilson     SCR(s) 
 *        5091 :
 *        Changed cases where INTEXP_TYPE was used to ensure that they catch 
 *        Brazos
 *        as well as Wabash cases.
 *        
 *  76   mpeg      1.75        10/30/02 10:45:32 AM   Dave Moore      SCR(s) 
 *        4833 :
 *        Changed trace level on hwtimer_start to LEVEL_2.
 *        
 *  75   mpeg      1.74        10/29/02 5:21:58 PM    Dave Moore      SCR(s) 
 *        4833 :
 *        Changed critical_section_begin to c_critical_section_begin which
 *        now takes LR as a parameter passed in from the new assembler version
 *        of critical_section_begin, Same treatment was also done to 
 *        not_interrupt_safe.
 *        Changes traces in these 2 c functions to TRACE_ALL.
 *        
 *        
 *  74   mpeg      1.73        10/1/02 12:21:18 PM    Miles Bintz     SCR(s) 
 *        4644 :
 *        added pci_get_bar from startup\pci.c
 *        
 *  73   mpeg      1.72        9/6/02 4:12:56 PM      Matt Korte      SCR(s) 
 *        4498 :
 *        Fixed Warnings
 *        
 *  72   mpeg      1.71        9/4/02 4:51:50 PM      Bobby Bradford  SCR(s) 
 *        4522 :
 *        Modified the critical_section_begin, and not_interrupt_safe
 *        functions to obtain the Link Register (Calling Function Address)
 *        without using in-line ASM code.  This is based on a method that
 *        was already being used in the critical_section_begin function to
 *        pass the calling function to the hw_critical_section_begin function
 *        
 *  71   mpeg      1.70        9/3/02 7:51:46 PM      Matt Korte      SCR(s) 
 *        4498 :
 *        Remove warnings
 *        
 *  70   mpeg      1.69        9/3/02 10:40:16 AM     Larry Wang      SCR(s) 
 *        4506 :
 *        re-enable interrupt expansion after it has been served.
 *        
 *  69   mpeg      1.68        8/30/02 3:24:08 PM     Larry Wang      SCR(s) 
 *        4499 :
 *        Implement the interrupt expansion.
 *        
 *  68   mpeg      1.67        8/15/02 7:02:40 PM     Tim Ross        SCR(s) 
 *        4409 :
 *        CHanged ChipID and ChipRevID from u_int32's to unsigned longs.
 *        
 *  67   mpeg      1.66        7/10/02 3:12:24 PM     Tim White       SCR(s) 
 *        4151 :
 *        The tick_timer driver did not take into account the amount of elapsed
 *        time when tick_start() and tick_stop() are called from the last time
 *        the ISR fired.  This caused long tick timers to be starved for long
 *        periods of time.  This coupled with another bug where hw_set_timer()
 *        is called with a timeout of 0 caused the old value to be used which
 *        could be anything.  A third bug is that for a ONE_SHOT timer, the
 *        fact that it's timeout was set to 0 would cause the iNextTimeout to
 *        be set to 0.  A ONE_SHOT timer should not effect the next timeout
 *        period since it's not going to fire again until tick_start() is
 *        called.
 *        
 *        
 *  66   mpeg      1.65        7/10/02 10:43:30 AM    Tim White       SCR(s) 
 *        1768 1769 :
 *        Added the workaround for the Colorado GPIO edge triggered interrupt 
 *        chip
 *        errata which can cause lost interrupts.  The #define 
 *        COLORADO_GPIO_EDGE_INTR_BUG
 *        is used to compile in the workaround and is defined only in 
 *        colorado.h.
 *        This problem does not appear in the Hondo or Wabash chips.
 *        
 *        
 *  65   mpeg      1.64        6/25/02 2:53:34 PM     Bobby Bradford  SCR(s) 
 *        4085 :
 *        Modified the sequence of saving the Link Register (R14) to remove
 *        a warning in SDT builds.  Couldn't get rid of it in ADS builds ...
 *        don't know why, but the disassembled code looks OK, so I don't
 *        believe that it is an issue.
 *        
 *  64   mpeg      1.63        6/11/02 11:48:40 AM    Dave Moore      SCR(s) 
 *        3972 :
 *        Print out LR in not_interrupt_safe
 *        
 *        
 *  63   mpeg      1.62        4/2/02 4:00:44 PM      Tim White       SCR(s) 
 *        3491 :
 *        Allow tick_xxxx functions to be used from within critical sections.
 *        
 *        
 *  62   mpeg      1.61        2/8/02 11:14:02 AM     Bobby Bradford  SCR(s) 
 *        3131 :
 *        Added new function, get_system_time_us(), which returns the current 
 *        system time, in micro-seconds.  Includes checks for hardware timer 
 *        roll-over, which can happen when the system timer interrupt is 
 *        pending, but hasn't been serviced yet.
 *        
 *  61   mpeg      1.60        2/1/02 4:06:42 AM      Ian Mitchell    SCR(s): 
 *        3101 
 *        Enable the reboot_IRD() function if RTOS==UCOS
 *        
 *  60   mpeg      1.59        9/11/01 5:46:50 PM     Miles Bintz     SCR(s) 
 *        2485 :
 *        added delay_and_reboot_IRD function for doing SW resets in release 
 *        builds after short delays
 *        
 *        
 *  59   mpeg      1.58        9/8/01 8:22:16 AM      Dave Wilson     SCR(s) 
 *        1999 1998 :
 *        Added hwlib_watchdog_enable and reworked watchdog handling inside
 *        hwlib_timer_initialise.
 *        
 *  58   mpeg      1.57        8/9/01 1:42:00 PM      Angela          SCR(s) 
 *        2448 2449 2450 :
 *        added Interrupt Latency Check
 *        
 *  57   mpeg      1.56        7/17/01 4:41:04 PM     Angela          SCR(s) 
 *        2327 2328 :
 *        added critical section latency check
 *        
 *  56   mpeg      1.55        5/15/01 3:21:50 PM     Tim White       
 *        DCS#1903,1904,1905 -- Fix internal compiler error generated from 
 *        building dmfront optimized
 *        
 *  55   mpeg      1.54        5/4/01 2:54:18 PM      Tim White       DCS#1822,
 *         DCS#1824, DCS31825 -- Critical Section Overhaul
 *        
 *  54   mpeg      1.53        5/1/01 8:28:04 AM      Dave Wilson     SCD1817: 
 *        Removed unsafe code from within critics laections.
 *        
 *  53   mpeg      1.52        4/25/01 10:53:54 AM    Tim White       DCS#1777,
 *         DCS#1778, DCS#1779 - Fix nucleus critical section end problem.
 *        Interrupts are not really off when calling critical_section_begin() 
 *        from
 *        an ISR under Nucleus hence the matching critical_section_end() will 
 *        really
 *        turn interrupts back on from within an ISR.  So you can't detect this
 *        case from Nucleus like we can from pSOS.
 *        
 *  52   mpeg      1.51        4/20/01 10:15:50 AM    Tim White       DCS#1687,
 *         DCS#1747, DCS#1748 - Add pSOS task switch prevention, move OS 
 *        specific
 *        functionality out of hwlib.c and into psoskal.c/nupkal.c, and update 
 *        critical
 *        section debugging function for both release and debug without using 
 *        assembly code.
 *        
 *  51   mpeg      1.50        4/11/01 3:33:46 PM     Tim White       DCS#1683,
 *         DCS#1684, DCS#1685 -- Add critical section debugging support.
 *        
 *  50   mpeg      1.49        3/30/01 9:18:14 AM     Steve Glennon   Fix for 
 *        DCS#1504/1505
 *        Changed not_interrupt_safe to return a bool. Also made it call 
 *        error_log if
 *        it is actually called at interrupt time.
 *        This is to allow caller to decide on behavior based upon having been 
 *        called 
 *        at interrupt time so as not to do anything bad.
 *        
 *  49   mpeg      1.48        3/23/01 12:08:16 PM    Steve Glennon   Added 
 *        functions "not_interrupt_safe" and "only_at_interrupt_safe"
 *        
 *  48   mpeg      1.47        1/16/01 8:37:44 PM     Miles Bintz     put back 
 *        changes that were lost between revision 1.38 and 1.39
 *        (NUP w/ critical_section_begin/end)
 *        
 *  47   mpeg      1.46        11/28/00 7:37:20 AM    Dave Wilson     Added 
 *        watchdog timer support for release builds
 *        
 *  46   mpeg      1.45        11/13/00 5:40:02 PM    Anzhi Chen      Removed 
 *        some comments.
 *        
 *  45   mpeg      1.44        11/13/00 5:36:26 PM    Anzhi Chen      Modified 
 *        kal_tick_timer_isr and tick_timer_state_change so timer is stopped
 *        before calling callback function.  This enabled the calling of 
 *        tick_start
 *        inside the callback function to be successful.
 *        
 *  44   mpeg      1.43        11/2/00 3:27:22 PM     Miles Bintz     Fixed 
 *        what should have been fixed in v1.40
 *        
 *  43   mpeg      1.42        10/27/00 10:59:10 AM   Ismail Mustafa  
 *        get_system_time is now used by VENDOR_B.
 *        
 *  42   mpeg      1.41        10/20/00 5:00:52 PM    Anzhi Chen      Added 
 *        checking for maximum period allowed (79secs) in hwtimer_set().
 *        
 *  41   mpeg      1.40        10/20/00 11:25:32 AM   Miles Bintz     added 
 *        gInterrupt_State so NUP doesn't re-enable interrupts that were 
 *        disabled
 *        during kal_int_handler
 *        
 *  40   mpeg      1.39        10/16/00 6:28:32 PM    Anzhi Chen      Added 
 *        checking for invalid dwIntID for int_register_isr(), int_enable() and
 *        int_disable().
 *        
 *  39   mpeg      1.38        10/11/00 6:39:38 PM    Miles Bintz     fixed the
 *         problem for real this time.  I mean it.  Really.
 *        
 *  38   mpeg      1.37        10/11/00 5:22:32 PM    Miles Bintz     fixed 
 *        interrupt contorl in critsecbeg/end 
 *        
 *  37   mpeg      1.36        10/11/00 3:31:34 PM    Miles Bintz     changed 
 *        critical section begin/end to use nucleus interrupt control routines
 *        
 *  36   mpeg      1.35        8/30/00 5:06:16 PM     Miles Bintz     added 
 *        ||RTOS==NUP around reboot #ifdef
 *        
 *  35   mpeg      1.34        8/30/00 11:41:26 AM    Ismail Mustafa  Added 
 *        extern IntMask for Nucleus Plus.
 *        
 *  34   mpeg      1.33        8/30/00 11:22:46 AM    Ismail Mustafa  Added 
 *        Nucleus Plus changes.
 *        
 *  33   mpeg      1.32        7/5/00 3:47:18 PM      Joe Kroesche    added 
 *        get_system_time function
 *        
 *  32   mpeg      1.31        6/19/00 9:33:26 AM     Ray Mack        fix for 
 *        race condition in GPIO interrupt handling
 *        
 *  31   mpeg      1.30        5/1/00 7:31:00 PM      Senthil Veluswamy removed
 *         extra null termination while naming KAL Timers and KAL Tics
 *        
 *  30   mpeg      1.29        4/28/00 5:23:16 PM     Dave Wilson     Added 
 *        debug code to count number of each interrupt processed
 *        
 *  29   mpeg      1.28        4/24/00 11:19:24 AM    Tim White       Moved 
 *        strapping CONFIG0 from chip header file(s) to board header file(s).
 *        
 *  28   mpeg      1.27        4/13/00 9:20:10 PM     Senthil Veluswamy added 
 *        critical sections to prevent simultaneous access to Tick Info
 *        
 *  27   mpeg      1.26        4/13/00 10:05:14 AM    Dave Wilson     Changed 
 *        PLL_CONFIG0 to use a union.
 *        
 *  26   mpeg      1.25        4/11/00 11:42:36 AM    Senthil Veluswamy no 
 *        change
 *        
 *  25   mpeg      1.24        4/6/00 4:13:20 PM      Senthil Veluswamy changed
 *         MAX_NAME_LENGTH to MAX_OBJ_NAME_LENGTH
 *        
 *  24   mpeg      1.23        4/6/00 3:09:22 PM      Senthil Veluswamy 
 *        included "hwlibprv.h"
 *        
 *  23   mpeg      1.22        4/4/00 12:34:52 AM     Senthil Veluswamy changed
 *         Tick and Timer Names to 15byte strings from Union
 *        
 *  22   mpeg      1.21        4/3/00 12:24:32 PM     Dave Wilson     Changed 
 *        tick_id_from_name to return a normal KAL return code.
 *        Added prototypes for GetChipId and GetChipRev.
 *        
 *  21   mpeg      1.20        3/30/00 7:22:22 PM     Senthil Veluswamy added 
 *        Extension functions:
 *        tick_get_info() and tick_id_from_name()
 *        
 *  20   mpeg      1.19        3/10/00 5:26:42 PM     Dave Wilson     Added 
 *        pSOS-only reboot function
 *        
 *  19   mpeg      1.18        3/8/00 5:24:58 PM      Miles Bintz     changed 
 *        function reboot() to reboot_IRD() because of conflict with VxWorks
 *        
 *  18   mpeg      1.17        1/26/00 9:39:28 AM     Dave Wilson     Added 
 *        interrupt latency statistics gathering code
 *        
 *  17   mpeg      1.16        1/18/00 10:12:22 AM    Dave Wilson     Added 
 *        reboot function for OpenTV 1.2
 *        
 *  16   mpeg      1.15        11/18/99 8:09:12 PM    Dave Wilson     Changed 
 *        system timer handling to keep it inside HWLIB.
 *        
 *  15   mpeg      1.14        11/15/99 9:57:42 AM    Dave Wilson     Fixed an 
 *        uninitialised variable in tick_set.
 *        
 *  14   mpeg      1.13        11/2/99 10:15:28 AM    Dave Wilson     Removed 
 *        all cases for Sabine, PID board and early emulation phases.
 *        
 *  13   mpeg      1.12        11/1/99 4:12:00 PM     Tim Ross        Made 
 *        watchgdog timer init conditionally compiled based on 
 *        frequency screening switch. 
 *        
 *  12   mpeg      1.11        10/29/99 10:59:52 AM   Dave Wilson     Moved 
 *        GPIO extender APIs to IIC.
 *        
 *  11   mpeg      1.10        10/28/99 1:02:32 PM    Dave Wilson     Renamed 
 *        timer APIs
 *        
 *  10   mpeg      1.9         10/28/99 12:22:22 PM   Dave Wilson     Fixed 
 *        get_chip_id_and_rev function (or whatever it's called) to call 
 *        appropriate functions in STARTUP.
 *        
 *  9    mpeg      1.8         10/28/99 10:55:08 AM   Dave Wilson     Added 
 *        some #ifdef DRIVER_INCL_TRACE lines around unprotected trace calls.
 *        
 *  8    mpeg      1.7         10/14/99 7:15:14 PM    Dave Wilson     Removed 
 *        dependence on OpenTV headers.
 *        
 *  7    mpeg      1.6         10/6/99 5:51:52 PM     Dave Wilson     Added 
 *        stub for new read_chip_id_and_revision function.
 *        
 *  6    mpeg      1.5         10/6/99 11:32:14 AM    Senthil Veluswamy No 
 *        change.
 *        
 *  5    mpeg      1.4         10/6/99 11:30:48 AM    Senthil Veluswamy added 
 *        missing local var iPic.
 *        
 *  4    mpeg      1.3         9/29/99 7:13:04 PM     Senthil Veluswamy No 
 *        change.
 *        
 *  3    mpeg      1.2         9/28/99 1:57:34 PM     Dave Wilson     Added 
 *        tick timer functions.
 *        
 *  2    mpeg      1.1         9/23/99 3:31:08 PM     Dave Wilson     Changed 
 *        names of all timer APIs since semaphore protection is no longer
 *        required. Critical sections allow safer operation when APIs are 
 *        called from
 *        interrupt and non-interrupt contexts.
 *        
 *  1    mpeg      1.0         9/14/99 3:11:56 PM     Dave Wilson     
 * $
 * 
 *    Rev 1.85   16 Sep 2003 15:33:04   swartzwg
 * SCR(s) 7477 :
 * Use WATCHDOG sw config key to determine whether the Watchdog timer should be turned on or off for the build; the default is on for release build, and off for debug build
 * 
 *    Rev 1.84   02 Sep 2003 18:05:24   kroescjl
 * SCR(s) 7415 :
 * removed unneeded header files and reordered to eliminate warnings when
 * building for PSOS
 * 
 *    Rev 1.83   26 Jun 2003 14:28:40   dawilson
 * SCR(s) 6826 :
 * Fix to tick timer state change function to ensure that if a tick timer
 * callback function stops the timer, no more callbacks are generated. Previously,
 * if more than 1 callback would be generated, all callbacks would be made even
 * if the first one disabled the timer.
 * 
 *    Rev 1.82   03 Jun 2003 17:17:18   dawilson
 * SCR(s) 6701 6702 :
 * c_not_from_critical_section now just hangs in a while(1) loop if called from
 * within a critical section. This allows you to see what went wrong (if you
 * stop the debugger you will be stuck in this loop) and does not recurse as the
 * previous version did.
 * 
 *    Rev 1.81   03 Jun 2003 15:47:56   dawilson
 * SCR(s) 6652 6651 :
 * Added code to optionally include ISR duration checking. If ISR_TIMING_CHECK
 * is defined when the code is built, a 10 entry array is created which will
 * store details of the 10 longest ISRs running in the system.
 * 
 *    Rev 1.80   08 May 2003 12:07:40   swartzwg
 * SCR(s) 6265 6264 6263 :
 * Correct build error for Critical Section latency checking code;
 * Restore the correct value to the variable caller for critical section latency checking in c_critical_section_begin(); 
 * remove the tabs
 * 
 *    Rev 1.79   11 Dec 2002 16:03:32   dawilson
 * SCR(s) 5150 :
 * Removed all the conditional checks whose logic was reversed in the last fix.
 * This reversal of the logic caused Colorado builds to break since INT_EXP is no
 * longer defined for chips which have INTEXP_TYPE set to NOT_PRESENT.
 * 
 *    Rev 1.78   11 Dec 2002 13:38:18   donaheb
 * SCR(s) 5132 :
 * Fixed 2 == vs. != logic problems with INTEXP_TYPE (NOT_PRESENT vs. 
 * INTEXP_WABASH)
 * 
 * 
 *    Rev 1.77   11 Dec 2002 11:18:26   dawilson
 * SCR(s) 5131 :
 * Fixed a few cases where ITC_EXPANSION_POS (a bit position definition in the
 * chip header) had been used in place of INT_EXP (an HWLIB interrupt identifier).
 * Both had the same value but using the wrong label could have been confusing.
 * 
 *    Rev 1.76   10 Dec 2002 15:00:10   dawilson
 * SCR(s) 5091 :
 * Changed cases where INTEXP_TYPE was used to ensure that they catch Brazos
 * as well as Wabash cases.
 * 
 *    Rev 1.75   30 Oct 2002 10:45:32   mooreda
 * SCR(s) 4833 :
 * Changed trace level on hwtimer_start to LEVEL_2.
 * 
 *    Rev 1.74   29 Oct 2002 17:21:58   mooreda
 * SCR(s) 4833 :
 * Changed critical_section_begin to c_critical_section_begin which
 * now takes LR as a parameter passed in from the new assembler version
 * of critical_section_begin, Same treatment was also done to not_interrupt_safe.
 * Changes traces in these 2 c functions to TRACE_ALL.
 * 
 * 
 *    Rev 1.73   01 Oct 2002 11:21:18   bintzmf
 * SCR(s) 4644 :
 * added pci_get_bar from startup\pci.c
 * 
 *    Rev 1.72   06 Sep 2002 15:12:56   kortemw
 * SCR(s) 4498 :
 * Fixed Warnings
 * 
 *    Rev 1.71   04 Sep 2002 15:51:50   bradforw
 * SCR(s) 4522 :
 * Modified the critical_section_begin, and not_interrupt_safe
 * functions to obtain the Link Register (Calling Function Address)
 * without using in-line ASM code.  This is based on a method that
 * was already being used in the critical_section_begin function to
 * pass the calling function to the hw_critical_section_begin function
 * 
 *    Rev 1.70   03 Sep 2002 18:51:46   kortemw
 * SCR(s) 4498 :
 * Remove warnings
 * 
 *    Rev 1.69   03 Sep 2002 09:40:16   wangl2
 * SCR(s) 4506 :
 * re-enable interrupt expansion after it has been served.
 * 
 *    Rev 1.68   30 Aug 2002 14:24:08   wangl2
 * SCR(s) 4499 :
 * Implement the interrupt expansion.
 * 
 *    Rev 1.67   15 Aug 2002 18:02:40   rossst
 * SCR(s) 4409 :
 * CHanged ChipID and ChipRevID from u_int32's to unsigned longs.
 * 
 *    Rev 1.66   10 Jul 2002 14:12:24   whiteth
 * SCR(s) 4151 :
 * The tick_timer driver did not take into account the amount of elapsed
 * time when tick_start() and tick_stop() are called from the last time
 * the ISR fired.  This caused long tick timers to be starved for long
 * periods of time.  This coupled with another bug where hw_set_timer()
 * is called with a timeout of 0 caused the old value to be used which
 * could be anything.  A third bug is that for a ONE_SHOT timer, the
 * fact that it's timeout was set to 0 would cause the iNextTimeout to
 * be set to 0.  A ONE_SHOT timer should not effect the next timeout
 * period since it's not going to fire again until tick_start() is
 * called.
 * 
 *
 *
 *
 ****************************************************************************/

