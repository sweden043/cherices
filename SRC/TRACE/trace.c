/****************************************************************************/
/*                   Conexant Proprietary and Confidential                  */
/*                       Software File/Module Header                        */
/*              Conexant Systems Inc. (c) 1998, 1999, 2000, 2001            */
/*                               Austin, TX                                 */
/*                            All Rights Eeserved                           */
/****************************************************************************/
/*
 * Filename:       trace.c
 *
 *
 * Description:    Trace and Error Logging Functions (compiles to almost nothing
 *                 when build with the DEBUG/TRACE label undefined).
 *
 *
 * Author:         Dave Wilson
 *
 ****************************************************************************/
/* $Id: trace.c,v 1.37, 2004-04-27 20:40:00Z, Miles Bintz$
 ****************************************************************************/

/* #define TRACE_LEVEL_BEFORE_MODULE */

/********************/
/* Required Headers */
/********************/
#include "stbcfg.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "kal.h"
#include "retcodes.h"
#include "trace.h"
#if RTOS==NUP
# include "nup.h"
#endif 


/********************************/
/* Internal Function Prototypes */
/********************************/
void isr_error_handler_task(void *parm);
void init_trace_queue(void);
void trace_message(bool bNoTsFlag,const char *string, va_list arg);
void isr_trace_message(bool bNoTsFlag, char *string, u_int32 value1, u_int32 value2);

/********************/
/* Global variables */
/********************/
typedef struct _trace_time_t
{
    u_int32 min, sec, msec, nsec;
} trace_time_t;

u_int32 trace_minutes = 0;
volatile u_int32 *trace_timer_value_addr;
void trace_timer_handler(timer_id_t timer, void *userData);
timer_id_t trace_timer;

char achspfbuff[PRT_BUFF_SIZE];

bool bTraceInitialised = FALSE;

#if (defined TRACE_OUTPUT) || (ENABLE_TRACE_IN_RELEASE == YES)
task_id_t trace_tid;
sem_id_t kal_trace_sem;
queue_id_t isr_q;
queue_id_t trcmsg_q;
int iBadIsrErrors = 0;
int iBadIsrTraces = 0;
char achTraceMsgs[MAX_TRACEMSGS][MAX_TRACEMSG_CHARS];

/* Trace level */
u_int32 uTraceFlags = TRACE_LEVEL_4 | TRACE_ALL;

#endif /* TRACE_OUTPUT */

/********************************/
/* Externally Defined Variables */
/********************************/
extern task_id_t TelnetTracePID;

/********************************/
/* Externally Defined Functions */
/********************************/
extern void traceout(char *string, ...);

/********************************************************************/
/*  FUNCTION:    trace_init                                         */
/*                                                                  */
/*  PARAMETERS:  None                                               */
/*                                                                  */
/*  DESCRIPTION: Initialise trace and error handling functions.     */
/*                                                                  */
/*  RETURNS:     TRUE on success, FALSE on failure                  */
/********************************************************************/
bool trace_init(void)
{
  #if (defined TRACE_OUTPUT) || (ENABLE_TRACE_IN_RELEASE == YES)
  kal_trace_sem = sem_create(1, "TRSM");
  
  if(!kal_trace_sem)
  {
     error_log(ERROR_FATAL | MOD_KAL | RC_KAL_NORESOURCES);
     return(FALSE);
  }

   /* Create the queue for error and trace logging from ISRs */
   isr_q = qu_create(MAX_ISR_DEBUG_QUEUE, "DBGI");
   if(isr_q != (queue_id_t)0)
   {
      trace_tid = task_create(isr_error_handler_task,
                              (void *)NULL,
                              (void *)NULL,
                              ISRH_TASK_STACK_SIZE,
                              ISRH_TASK_PRIORITY,
                              ISRH_TASK_NAME);
      if(!trace_tid)
      {
         trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, "ISR error and trace handling task failed to start\n");
         error_log(ERROR_WARNING | MOD_KAL | RC_KAL_TASKFAIL);
         return(FALSE);
      }
      else
      {
         TelnetTracePID = trace_tid;
         init_trace_queue();
      }
   }
   #endif
   
   /*
    * Allocate and start a hw timer for trace since the
    * RTC is not accurate (because of OTV) to use for
    * timestamping trace
    */

   trace_timer = hwtimer_create(trace_timer_handler, 0, "XTRC");

   trace_timer_value_addr = (u_int32*)((char *)TIM_VALUE_REG +
                                       (TIM_BANK_SIZE * (trace_timer&0xffff)));

   hwtimer_set(trace_timer, 60000000, FALSE);
   hwtimer_start(trace_timer);

   bTraceInitialised = TRUE;
   
   return(TRUE);
}

void trace_timer_handler(timer_id_t timer, void *userData)
{
   trace_minutes += 1;
}

void trace_timestamp(trace_time_t *time)
{
    u_int32 m54, msec;

    m54  = *trace_timer_value_addr;
    msec = m54 / 54000;

    time->min  = trace_minutes + (msec / 60000);
    time->sec  = (msec / 1000) % 60;
    time->msec = msec % 1000;
    time->nsec = ((m54 % 54) * 185) / 10;
}

#if (defined TRACE_OUTPUT) || (ENABLE_TRACE_IN_RELEASE == YES)
/**********************************************/
/* Initialise the trace message buffer queue. */
/**********************************************/
void init_trace_queue(void)
{
   u_int32 trc_msg[4];
   u_int32 i;
   int     iRetcode;


   trcmsg_q = qu_create(MAX_TRACEMSGS, "TRC1");
   if(trcmsg_q != (queue_id_t)0)
   {
     /* First/only word of msg holds index of a free msg buffer */
     for (i = 0; i < MAX_TRACEMSGS; i++)
     {
       trc_msg[0] = i;
       iRetcode = qu_send(trcmsg_q,trc_msg);
       if (iRetcode)
          error_log(ERROR_WARNING | MOD_KAL | RC_KAL_QFULL);
     }
   }
   else
     error_log(ERROR_WARNING | MOD_KAL | RC_KAL_NORESOURCES);
}

#endif /* TRACE_OUTPUT */

/*************************************************/
/*************************************************/
/** Error Reporting and Trace Logging Functions **/
/*************************************************/
/*************************************************/

/***************************************************/
/* Output a checkpoint number to the parallel port */
/***************************************************/
#if (defined TRACE_OUTPUT) || (ENABLE_TRACE_IN_RELEASE == YES)
void checkpoint(int checkpoint_num)
{
   // To be completed
}
#endif /* TRACE_OUTPUT */

/********************************************************************/
/* Log an error and, optionally, abort execution of the application */
/********************************************************************/

/********************************************************************/
/* In DEBUG/TRACE builds, error_log is a macro that calls debug_error_log */
/* and includes file and line number information. For release, this */
/* is reduced to a simple function call with the error number.      */
/********************************************************************/
#if !(defined TRACE_OUTPUT) && (ENABLE_TRACE_IN_RELEASE == NO)
/* Release version - just keep note of the error count and reboot if */
/* a fatal error is reported.                                        */
void error_log(int error_num)
{
   static int iErrorLogCount = 0;
   static int iErrorLogLast;
   
   iErrorLogCount++;
   iErrorLogLast = error_num;
   
   /* If this is a fatal error, do whatever we do when fatal things happen */
   if(error_num & ERROR_FATAL)
     fatal_exit(error_num);
}

#else /* TRACE_OUTPUT */
void debug_error_log(int error_num, char *file, int linenum)
{
   char *strError;
   trace_time_t  time;

   trace_timestamp(&time);

   switch(error_num & ERROR_SOURCE)
   {
     case MOD_GEN: strError = STR_GEN; break;
     case MOD_AUD: strError = STR_AUD; break;
     case MOD_DPS: strError = STR_DPS; break;
     case MOD_DRM: strError = STR_DRM; break;
     case MOD_ETH: strError = STR_ETH; break;
     case MOD_FPU: strError = STR_FPU; break;
     case MOD_GCP: strError = STR_GCP; break;
     case MOD_I2C: strError = STR_I2C; break;
     case MOD_IRD: strError = STR_IRD; break;
     case MOD_MOV: strError = STR_MOV; break;
     case MOD_MPG: strError = STR_MPG; break;
     case MOD_PAR: strError = STR_PAR; break;
     case MOD_RST: strError = STR_RST; break;
     case MOD_RTC: strError = STR_RTC; break;
     case MOD_SDM: strError = STR_SDM; break;
     case MOD_SER: strError = STR_SER; break;
     case MOD_SMC: strError = STR_SMC; break;
     case MOD_TUN: strError = STR_TUN; break;
     case MOD_KAL: strError = STR_KAL; break;
     case MOD_KEY: strError = STR_KEY; break;
     case MOD_FSH: strError = STR_FSH; break;
     case MOD_OTV: strError = STR_OTV; break;
     case MOD_ISR: strError = STR_ISR; break;
     default:      strError = STR_UNK; break;

   }

   printf("%02lu.%02lu.%03lu: 0x%08x: %s from %s (file %s, line %d)\n",
          time.min, time.sec, time.msec, error_num,
          (error_num & ERROR_FATAL) ? "Fatal error" : "Warning",
          strError,
          file,
          linenum);

   /* If this is a fatal error, do whatever we do when fatal things happen */
   if(error_num & ERROR_FATAL)
     fatal_exit(error_num);
}
#endif /* TRACE_OUTPUT */

/*************************************************************/
/* Set the current trace level and enable particular modules */
/* (Compiled out in release builds using #define in header)  */
/*************************************************************/
#if (defined TRACE_OUTPUT) || (ENABLE_TRACE_IN_RELEASE == YES)
void trace_set_level(u_int32 flags)
{
   uTraceFlags = flags;
}
#endif /* TRACE_OUTPUT */

/**********************************************************/
/* Send a trace message depending upon the flags provided */
/**********************************************************/
#if (defined TRACE_OUTPUT) || (ENABLE_TRACE_IN_RELEASE == YES) /* TRACE */
void trace_new(u_int32 flags, char *string, ...)
{
   va_list arg;
   u_int32 level;
   #if (DISABLE_TRACE_TIMESTAMPS == NO)
   bool bNoTsFlag = FALSE;
   #else
   bool bNoTsFlag = TRUE;
   #endif

   /* Only do anything if we are not at interrupt time */
   not_interrupt_safe();

   /* Flags indicate the source of the trace and the trace level. */
   /* If our global trace flags are set to enable this level of   */
   /* trace from this source, the message is printed, else it is  */
   /* ignored.                                                    */

   if ( ( (flags|uTraceFlags) & TRACE_TIMESTAMP_MASK) == TRACE_NO_TIMESTAMP) {
      bNoTsFlag = TRUE;
   }

#ifdef TRACE_LEVEL_BEFORE_MODULE
   level = flags & TRACE_LEVEL_MASK;
   /* Is the level enabled ? */
   if((level >= (uTraceFlags & TRACE_LEVEL_MASK)) || (level == TRACE_LEVEL_ALWAYS))
   {
      /* Is trace from this module enabled ? */
      if(flags & TRACE_MASK_MODULE & uTraceFlags)
      {

         /* OK - source and level are OK so print the message */
         va_start(arg,string);
         trace_message(bNoTsFlag, string, arg);
         va_end(arg);
      }
   }
#else
   /* Is trace from this module enabled ? */
   if(flags & TRACE_MASK_MODULE & uTraceFlags)
   {
      level = flags & TRACE_LEVEL_MASK;

      /* Is the level enabled ? */
      if((level >= (uTraceFlags & TRACE_LEVEL_MASK)) || (level == TRACE_LEVEL_ALWAYS))
      {
         /* OK - source and level are OK so print the message */
         va_start(arg,string);
         trace_message(bNoTsFlag, string, arg);
         va_end(arg);
      }
   }
#endif      
   
   return;
} /* trace_new */

/***********************************************/
/* Basic trace which always prints the message */
/***********************************************/
void trace(char*string, ...)
{
   va_list arg;
   not_interrupt_safe();
   va_start(arg, string);
   #if (DISABLE_TRACE_TIMESTAMPS == NO)
   trace_message(FALSE, string, arg);
   #else
   trace_message(TRUE, string, arg);
   #endif
   va_end(arg);
   return;
   
}
#endif /* TRACE_OUTPUT */

/***************************************************************/
/* Dump a trace message via the internal trace/error log queue */
/***************************************************************/
#if (defined TRACE_OUTPUT) || (ENABLE_TRACE_IN_RELEASE == YES) /* TRACE */
void trace_message(bool bNoTsFlag,const char *string, va_list arg)
{
    trace_time_t time;

#ifndef UNBUFFERED_TRACE
   /******************************************************************/
   /* Normal buffered version puts all trace through a queue. If the */
   /* queue fills up, messages are discarded. This poses a problem   */
   /* when trace is important, for example in OpenTV VTS testing so  */
   /* the unbuffered option is also included for these cases.        */
   /******************************************************************/
   u_int32 trc_msg[4];
   u_int32 isr_msg[4];
   u_int32 slot;
   int     iRetcode;

   not_interrupt_safe();
   if(isr_q && trcmsg_q)
   {
     /* Obtain a trace buffer from trcmsg_q (buffer queue) */
     /* Buffer will be returned by isr_error_handler_task  */

     iRetcode = qu_receive(trcmsg_q, 0, trc_msg);
     if (iRetcode)
     {
//     error_log(ERROR_WARNING | MOD_KAL | RC_KAL_QFULL);
       iBadIsrTraces ++;
       return;
     }
     else
     {
       slot = trc_msg[0];
       if (slot >= MAX_TRACEMSGS)
       {
         error_log(ERROR_FATAL | MOD_KAL | RC_KAL_SYSERR);
       }
       else
       {
         /* Get the trace semaphore */

         sem_get(kal_trace_sem, KAL_WAIT_FOREVER);

         /* format message into intermediate store */
         if (bNoTsFlag) {
            vsprintf(&achspfbuff[0], string, arg);
         }
         else {
            trace_timestamp(&time);
            sprintf(achspfbuff, "%02lu.%02lu.%03lu: ", time.min, time.sec, time.msec);
            vsprintf(&achspfbuff[11], string, arg);
         }

         /* copy to slot in buffer queue */
         strncpy(&(achTraceMsgs[slot][0]),
                 achspfbuff,
                 MAX_TRACEMSG_CHARS);

         /* Hand back the trace semaphore */

         sem_put(kal_trace_sem);

         achTraceMsgs[slot][MAX_TRACEMSG_CHARS-1] = '\0';

         isr_msg[0] = (u_int32)&(achTraceMsgs[slot][0]);
         isr_msg[1] = slot;
         isr_msg[2] = 0;
         isr_msg[3] = TRACE_Q_MSG;

         iRetcode = qu_send(isr_q, isr_msg);
         if(iRetcode)
         {
           qu_send(trcmsg_q,trc_msg);
         }
       }

     }
   }
#else
   /********************************************************************/
   /* Unbuffered version in case we want never to loose trace messages */
   /********************************************************************/
   not_interrupt_safe();
   if ( bNoTsFlag ) {
        vsprintf(&achspfbuff[0], string, arg);
   }
   else {
        trace_timestamp(&time);
        sprintf(achspfbuff, "%02lu.%02lu.%03lu: ", time.min, time.sec, time.msec);
        vsprintf(&achspfbuff[11], string, arg);
   }
   traceout(achspfbuff);
#endif /* UNBUFFERED_TRACE */
}
#endif /* TRACE_OUTPUT */

/**********************************************************************/
/* Send a trace message from an ISR depending upon the flags provided */
/**********************************************************************/
#if (defined TRACE_OUTPUT) || (ENABLE_TRACE_IN_RELEASE == YES) /* TRACE */
void isr_trace_new(u_int32 flags, char*string, u_int32 value1, u_int32 value2)
{
   u_int32 level;
   #if (DISABLE_TRACE_TIMESTAMPS == NO)
   bool bNoTsFlag = FALSE;
   #else
   bool bNoTsFlag = TRUE;
   #endif

   /* Flags indicate the source of the trace and the trace level. */
   /* If our global trace flags are set to enable this level of   */
   /* trace from this source, the message is printed, else it is  */
   /* ignored.                                                    */

   if ( ( (flags|uTraceFlags) & TRACE_TIMESTAMP_MASK) == TRACE_NO_TIMESTAMP) {
      bNoTsFlag = TRUE;
   }

   /* Is trace from this module enabled ? */
   if(flags & TRACE_MASK_MODULE & uTraceFlags)
   {
      /* Is the level enabled ? */

      level = flags & TRACE_LEVEL_MASK;

      if((level >= (uTraceFlags & TRACE_LEVEL_MASK)) || (level == TRACE_LEVEL_ALWAYS))
      {
         /* OK - source and level are OK so print the message */
         isr_trace_message(bNoTsFlag, string, value1, value2);
      }
   }
}
/************************************/
/* Dump a trace message from an ISR */
/************************************/
void isr_trace(char *string, u_int32 value1, u_int32 value2)
{
    #if (DISABLE_TRACE_TIMESTAMPS == NO)
    isr_trace_message (FALSE, string, value1, value2);
    #else
    isr_trace_message (TRUE, string, value1, value2);
    #endif
}

void isr_trace_message(bool bNoTsFlag, char *string, u_int32 value1, u_int32 value2)
{
  u_int32       rec_msg[4];
  int           iRetcode;
  trace_time_t  time;

  if(isr_q)
  {
    if (!bNoTsFlag) {
      trace_timestamp(&time);
      rec_msg[0] = time.min;
      rec_msg[1] = time.sec;
      rec_msg[2] = time.msec;
      rec_msg[3] = TIME_MSG;
      iRetcode = qu_send(isr_q, rec_msg);
      if(iRetcode)
        iBadIsrTraces ++;
    }
    rec_msg[0] = (u_int32)string;
    rec_msg[1] = value1;
    rec_msg[2] = value2;
    if (bNoTsFlag) {
      rec_msg[3] = TRACE_MSG_NO_TS;
    }
    else {
      rec_msg[3] = TRACE_MSG;
    }
    iRetcode = qu_send(isr_q, rec_msg);
    if(iRetcode)
      iBadIsrTraces ++;
  }
}

#endif /* TRACE_OUTPUT */

/**********************************************************************/
/* Send a trace message from a critical_section                       */
/* (Must use some form of polling and no OS calls (e.g. q_send)       */
/**********************************************************************/
#if (defined TRACE_OUTPUT) || (ENABLE_TRACE_IN_RELEASE == YES) 
void cs_trace(u_int32 flags, char*string, u_int32 value1, u_int32 value2)
{
   //
   // For trace, stop using the OS queue management services and
   // devise a non critical_section required shared memory area
   // for passing trace information between the caller and the trace
   // handler task.  This will also allow the isr_trace calls to have
   // more arguments.
   //
}

/***********************************************/
/* Log an error from within a critical section */
/***********************************************/

/* Debug version - dump the error info to the trace output. */
/* Release version maps to error_log                        */
void debug_cs_error_log(int error_num, char *filename, int linenum)
{
}
#endif /* TRACE_OUTPUT */

/****************************/
/* Log an error from an ISR */
/****************************/

#if (defined TRACE_OUTPUT) || (ENABLE_TRACE_IN_RELEASE == YES)
/* Debug version - dump the error info to the trace output. */
/* Release version maps to error_log                        */
void debug_isr_error_log(int error_num, char *filename, int linenum)
{
  u_int32       rec_msg[4];
  int           iRetcode;

  if(isr_q)
  {
    rec_msg[0] = error_num;
    rec_msg[1] = (int)filename;
    rec_msg[2] = linenum;
    rec_msg[3] = ERROR_MSG;
    iRetcode = qu_send(isr_q, rec_msg);
    if(iRetcode)
      iBadIsrErrors ++;
  }
}
#endif /* TRACE */

#if (defined TRACE_OUTPUT) || (ENABLE_TRACE_IN_RELEASE == YES) 
/*****************************************/
/* ISR error and trace logging task body */
/*****************************************/
void isr_error_handler_task(void *parm)
{
  u_int32       rec_msg[4];
  u_int32       trc_msg[4];
  int           iRetcode;
  u_int32       min, sec, msec;
  char string[PRT_BUFF_SIZE];

  trace_new(TRACE_KAL | TRACE_LEVEL_4, "ISR error and trace handler running...\n");

  do
  {
    iRetcode = qu_receive(isr_q, KAL_WAIT_FOREVER, rec_msg);
    if(iRetcode == RC_OK)
    {
       switch(rec_msg[3])
       {
           case TRACE_MSG_NO_TS:
               sprintf(string, "%s",(char*)rec_msg[0]);
               traceout(string, rec_msg[1], rec_msg[2]);
               break;
           case TRACE_MSG:
               sprintf(string, "%02lu.%02lu.%03lu: %s", min, sec, msec, (char*)rec_msg[0]);
               traceout(string, rec_msg[1], rec_msg[2]);
               break;
           case TIME_MSG:
               min  = rec_msg[0];
               sec  = rec_msg[1];
               msec = rec_msg[2];
               break;
           case TRACE_Q_MSG:
               traceout((char *)rec_msg[0]);

               /* return message buffer to pool, managed by trcmsg_q */
               trc_msg[0] = rec_msg[1];
               qu_send(trcmsg_q, trc_msg);
               break;
           case ERROR_MSG:
               debug_error_log(rec_msg[0], (char *)rec_msg[1], rec_msg[2]);
               break;
       }
    }
  } while((iRetcode == RC_OK) && (rec_msg[3] != PROCESS_EXIT));

  trace_new(TRACE_KAL | TRACE_LEVEL_4, "ISR error and trace handler exiting 0x%08lx\n", iRetcode);

  task_terminate();
}
#endif /* TRACE_OUTPUT */

#if ((!defined TRACE_OUTPUT) && (ENABLE_TRACE_IN_RELEASE != YES))

/********************************************************************/
/*  FUNCTION:    timestamp_message                                  */
/*                                                                  */
/*  PARAMETERS:  str - String to be output along with timestamp     */
/*                                                                  */
/*  DESCRIPTION: This function outputs a timestamped message to the */
/*               serial port. Note that this is only used for       */
/*               release builds or builds with the normal trace     */
/*               functions removed. In cases where trace is         */
/*               included, this function aliases to trace_new.      */
/*                                                                  */
/*  RETURNS:     Nothing                                            */
/*                                                                  */
/*  CONTEXT:     Must be called in non-interrupt context.           */
/*                                                                  */
/********************************************************************/
void timestamp_message(char *str)
{
  trace_time_t time;
  
  if(!bTraceInitialised)
    trace_init();
    
  trace_timestamp(&time);
  sprintf(achspfbuff, "%02lu.%02lu.%03lu: %s", time.min, time.sec, time.msec, str);
  traceout(achspfbuff);
}
#endif

/*******************************************************************************/
/* Before resetting everything in the case of a fatal error, dump the current  */
/* contents of the trace queue.                                                */
/*******************************************************************************/
#if (defined TRACE_OUTPUT) || (ENABLE_TRACE_IN_RELEASE == YES)
void flush_trace_on_abort(void)
{
  int iPriority;
  int iRetcode;
  u_int32 rec_msg[4];
  u_int32       trc_msg[4];
  u_int32       min, sec, msec;
  char string[PRT_BUFF_SIZE];
  
  /* Effectively suspend the trace task so that we don't have 2 people */
  /* fighting for queue messages.                                      */
  
  /* Get this task's priority */
  iPriority = task_priority(task_id(), 0);

  /* compute delta to make it top priority and set */
  iPriority = KAL_MAX_TASK_PRIORITY - iPriority ;
  iPriority = task_priority(task_id(), iPriority );
  
  /* Get the trace task priority */
  iPriority = task_priority(trace_tid, 0);
  /* Set the new priority to 0*/
  iPriority = task_priority(trace_tid, -iPriority);


  /* Read all the messages currently on the trace queue and */
  /* dump them.                                             */
  do
  {
    iRetcode = qu_receive(isr_q, 0, rec_msg);
    if(iRetcode == RC_OK)
    {
       switch(rec_msg[3])
       {
           case TRACE_MSG_NO_TS:
               sprintf(string, "%s",(char*)rec_msg[0]);
               traceout(string, rec_msg[1], rec_msg[2]);
               break;
           case TRACE_MSG:
               sprintf(string, "%02lu.%02lu.%03lu: %s", min, sec, msec, (char*)rec_msg[0]);
               traceout(string, rec_msg[1], rec_msg[2]);
               break;
           case TIME_MSG:
               min  = rec_msg[0];
               sec  = rec_msg[1];
               msec = rec_msg[2];
               break;
           case TRACE_Q_MSG:
               traceout((char *)rec_msg[0]);
               /* return message buffer to pool, managed by trcmsg_q */
               trc_msg[0] = rec_msg[1];
               qu_send(trcmsg_q, trc_msg);
			   break;
	   }
    }
  } while( iRetcode != RC_KAL_EMPTY );

  task_time_sleep(2000); /* wait here a while since fatal_exit will shut off interrupts */
                         /* which kills the serial port driver                          */
}
#endif

/****************************************************************************
 * Modifications:
 * $Log: 
 *  38   mpeg      1.37        4/27/04 3:40:00 PM     Miles Bintz     CR(s) 
 *        8979 8980 : switch on TRACE_OUTPUT define instead of debug
 *  37   mpeg      1.36        10/22/03 7:26:28 AM    Dave Wilson     CR(s): 
 *        7693 Added code for timestamp_message. Removed ifdefs round several 
 *        existing functions since they are now required in a release build to 
 *        support the new timestamp_message function.
 *  36   mpeg      1.35        9/16/03 4:43:48 PM     Angela Swartz   SCR(s) 
 *        7477 :
 *        Enable trace output if ENABLE_TRACE_IN_RELEASE is set to YES in the 
 *        sw config file for release build
 *        
 *  35   mpeg      1.34        9/2/03 7:00:04 PM      Joe Kroesche    SCR(s) 
 *        7415 :
 *        reordered header file to eliminate psos warnings
 *        
 *  34   mpeg      1.33        2/13/03 12:34:42 PM    Matt Korte      SCR(s) 
 *        5479 :
 *        Removed old header reference
 *        
 *  33   mpeg      1.32        1/30/03 4:39:30 PM     Dave Wilson     SCR(s) 
 *        5358 :
 *        Forgot to apply the last fix to trace() and isr_trace() so timestamps
 *         still
 *        appeared on lines generated using these calls!
 *        
 *  32   mpeg      1.31        1/30/03 12:34:28 PM    Dave Wilson     SCR(s) 
 *        5358 :
 *        Added code to disable timestamps if DISABLE_TRACE_TIMESTAMPS is set 
 *        to YES
 *        in the software configuration file.
 *        
 *  31   mpeg      1.30        10/29/02 5:31:28 PM    Dave Moore      SCR(s) 
 *        4833 :
 *        Fixed task priority calculations in flush_trace_on_abort. 
 *        Also added switch statement in same to take care of cases
 *        TRACE_MSG_NO_TS and TIME_MSG.
 *        
 *        
 *  30   mpeg      1.29        6/7/02 11:07:04 AM     Matt Korte      SCR(s) 
 *        3961 :
 *        Added trace_get_level, made trace_set_level return void.
 *        
 *        
 *  29   mpeg      1.28        5/23/02 9:25:38 AM     Bobby Bradford  SCR(s) 
 *        3840 :
 *        Modified the trace_new and isr_trace_new routines to support
 *        the TRACE_NO_TIMESTAMP bit flag.
 *        
 *  28   mpeg      1.27        4/2/02 4:01:36 PM      Tim White       SCR(s) 
 *        3491 :
 *        Allow tick_xxxx functions to be used from within critical sections.
 *        
 *        
 *  27   mpeg      1.26        3/20/02 4:58:10 PM     Dave Wilson     SCR(s) 
 *        3416 :
 *        Removed a few compiler warnings.
 *        
 *  26   mpeg      1.25        2/28/02 1:53:20 PM     Miles Bintz     SCR(s) 
 *        3271 :
 *        Removed all preprocessor switches which included vxworks specific 
 *        code
 *        
 *  25   mpeg      1.24        2/11/02 5:35:00 PM     Ray Mack        SCR(s) 
 *        3167 :
 *        Added a call to get the tick count from the O/S.  Note that this 
 *        provides only 2 ms resolution on trace messages for VxWorks.
 *        
 *  24   mpeg      1.23        6/8/01 10:08:18 AM     Tim White       SCR(s) 
 *        2045 2046 :
 *        Fixed minor bug in the trace formatting.
 *        
 *        
 *  23   mpeg      1.22        6/7/01 3:45:22 PM      Tim White       SCR(s) 
 *        1958 1990 :
 *        Added a timestamp to all trace messages which works off a HW
 *        timer so as not to interfere with OTV and also be accurate.
 *        
 *        
 *  22   mpeg      1.21        5/4/01 2:52:30 PM      Tim White       DCS#1822,
 *         DCS#1824, DCS31825 -- Critical Section Overhaul
 *        
 *  21   mpeg      1.20        4/4/01 3:23:08 PM      Tim White       
 *        DCS#1631,DCS#1632,DCS#1633: Bug in #1504/1505/1634 causing a compile 
 *        error.
 *        
 *  20   mpeg      1.19        4/4/01 1:03:44 PM      Tim White       
 *        DCS#1631,DCS#1632,DCS#1633 Merge.
 *        
 *  19   mpeg      1.18        3/30/01 9:35:46 AM     Steve Glennon   DCS# 
 *        1504/1505 - Added a bunch of calls to not_interrupt_safe
 *        to ensure nothing unsafe gets called during interrupt time.
 *        Also added a variant of level/module handling so that you can choose
 *        to get all TRACE_LEVEL_ALWAYS traces regardless of module by
 *        #defining TRACE_LEVEL_BEFORE_MODULE.
 *        Defaults to old behavior of gating by module before by level
 *        
 *  18   mpeg      1.17        3/2/01 4:53:06 PM      Tim White       DCS#1366:
 *         Globally decrease stack memory usage.
 *        
 *  17   mpeg      1.16        3/2/01 4:08:30 AM      Dave Wilson     DCS1354: 
 *        flush_trace_on_Abort no longer modifies trace task priority.
 *        
 *  16   mpeg      1.15        1/23/01 10:25:06 AM    Dave Wilson     DCS987: 
 *        Made sure that no unused code is included in the release build.
 *        Saves 1.2K of ROM and 16K of RAM space in release builds
 *        
 *  15   mpeg      1.14        12/13/00 12:17:48 PM   Dave Moore      Changed 
 *        "MDM" to "MODEM"
 *        
 *  14   mpeg      1.13        11/20/00 1:03:16 PM    Miles Bintz     added 
 *        #ifdef to use unbuffered trace output
 *        
 *  13   mpeg      1.12        8/29/00 9:10:20 PM     Miles Bintz     added 
 *        nup.h if RTOS==NUP
 *        
 *  12   mpeg      1.11        8/29/00 8:45:48 PM     Miles Bintz     added 
 *        stdarg to list of includes
 *        
 *  11   mpeg      1.10        7/5/00 12:38:26 PM     Dave Wilson     Added 
 *        strings for a collection of newish modules
 *        
 *  10   mpeg      1.9         6/11/00 4:47:24 PM     Ray Mack        changes 
 *        to eliminate warnings from the compiler
 *        
 *  9    mpeg      1.8         3/21/00 4:23:00 PM     Dave Wilson     Fixed 
 *        incorrect OS #ifdefs.
 *        
 *  8    mpeg      1.7         3/3/00 1:18:52 PM      Tim Ross        Replaced 
 *        OS switch with RTOS switch.
 *        
 *  7    mpeg      1.6         12/15/99 10:59:38 AM   Dave Wilson     Added 
 *        #ifdef DEBUG round the new flush function
 *        
 *  6    mpeg      1.5         12/6/99 3:15:58 PM     Dave Wilson     Added 
 *        flush_trace_on_abort
 *        
 *  5    mpeg      1.4         12/1/99 11:44:00 AM    Dave Wilson     Changes 
 *        for VxWorks.
 *        
 *  4    mpeg      1.3         11/4/99 3:06:06 PM     Dave Wilson     Changed 
 *        and abort_report call to fatal_exit
 *        
 *  3    mpeg      1.2         11/1/99 4:56:52 PM     Dave Wilson     Fixed 
 *        header file include order for VxWorks
 *        
 *  2    mpeg      1.1         10/27/99 5:01:06 PM    Dave Wilson     Changed 
 *        WAIT_FOREVER to KAL_WAIT_FOREVER
 *        
 *  1    mpeg      1.0         9/13/99 2:29:42 PM     Dave Wilson     
 * $
 *
 ****************************************************************************/

