/****************************************************************************/
/*                               Conexant Systems                           */
/****************************************************************************/
/*                                                                          */
/* Filename:           TRACE_A.H                                            */
/*                                                                          */
/* Description:        Public header file for end user trace/debug support  */
/*                     routines                                             */
/*                                                                          */
/* Author:             Miles Bintz                                          */
/*                                                                          */
/* Copyright Conexant Systems Inc, 2000                                     */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/
/****************************************************************************
$Header: gentrace.c, 11, 9/2/03 7:00:08 PM, Joe Kroesche$                                      
****************************************************************************/

#include "stbcfg.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "kal.h"
#include "retcodes.h"
#include "trace.h"
#include "basetype.h"
#include "gentrace.h"

#if RTOS==NUP
#include "nup.h"
#endif

u_int32         gDbgLvl;
u_int32         gTrcLvl;
genEventtype    gEventLog[MAXEVENTS];

task_id_t       gen_trace_tid;
sem_id_t        gen_trace_sem;
queue_id_t      gen_tracein_q;
queue_id_t      gen_traceout_q;

char            trcmsgbuff[GENTRCMAXMSGS][GENDBGMAXSTRLEN];
char            trctmpbuff[GENDBGMAXSTRLEN+30]; /* leave room for file name and line no */

AlertCBTable    AlertTable;
void gen_trace_handler_task(void *parm);
void TraceMsg(char* module, u_int32 line, trctype type, char* msg, va_list ap);

void fatal_flush(void);
 
bool genDebugInit(void)
{
    int i, retcode;
    u_int32 msg[4];
    
    /* TODO: do we want a mechanism letting each module have its own
     * trace/debug level?  or should we let the end user implement
     * something on top of 32 generic masks?
     * 
     * 
     * Let all trace and debug messages through by default */
    gDbgLvl = 0xFFFFFFFF;
    gTrcLvl = 0xFFFFFFFF;

    AlertTable.Alert0 = 0;
    AlertTable.Alert1 = 0;
    AlertTable.Alert2 = 0;
    AlertTable.Alert3 = 0;
    AlertTable.Alert4 = 0;

    for (i = 0; i < MAXEVENTS; i++) {
        gEventLog[i].timestamp = 0;
        gEventLog[i].EventType = 0;
        gEventLog[i].value     = 0;
    }

    /* Is this semaphore needed? */
    gen_trace_sem = sem_create(1, "GTRS");
    if(!gen_trace_sem) {
        /* TODO: How should we print a message saying that 
         * we couldn't create the semaphore?
         */
        return FALSE;
    }

    /* Two queues.  One for incoming messages.  One for
     * outgoing messages.  Trace messages are palced
     * on the GTIQ queue by genTraceMsg and moved to
     * the GTOQ queue by gen_trace_handler_task.
     *
     * The in_q contains indexes to buffers available for 
     * storing a trace message.
     */
    gen_tracein_q = qu_create(GENTRCMAXMSGS, "GTIQ");
    gen_traceout_q = qu_create(GENTRCMAXMSGS, "GTOQ");
    if(gen_tracein_q != (queue_id_t)0) {

        for (i = 0; i < GENTRCMAXMSGS; i++) {
            msg[0] = i;
            retcode = qu_send(gen_tracein_q, msg);
            if (retcode) {
                /* TODO:  How do we handle uninitialized queue? */
            }
        }
        
        gen_trace_tid = task_create(gen_trace_handler_task,
                                    (void *)NULL,
                                    (void *)NULL,
                                     TRQH_TASK_STACK_SIZE,
                                     TRQH_TASK_PRIORITY,
                                     TRQH_TASK_NAME);
        if (!gen_trace_tid) {
            return FALSE;
        }
    }
    
    printf("Generic debug/trace functions initialized.  Event buffer is at 0x%08x\n", (int)gEventLog);
    return TRUE;
}

void TraceMsg(char* module, u_int32 line, trctype type, char* msg, va_list ap)
{
    int retcode,slot;
    char temp[GENDBGMAXSTRLEN+30]; /* leave room for fn and line no */
    u_int32 in_msg[4], out_msg[4];

    /* if our queues exist... */
    if (gen_tracein_q && gen_traceout_q) {
        /* get the index of the next available buffer (stored in the message) */
        retcode = qu_receive(gen_tracein_q, 0, in_msg);
        if (retcode) {
            /* TODO:  Should we log (use genEvent?) to keep track of 
             * failed trace messages?
             */
            return;
        } else {
            slot = in_msg[0];

            /* If slot is a legit value... */
            if (slot < GENTRCMAXMSGS) {

                /* Get the trace semaphore -
                 * TODO:
                 * -is this required?  The queue will ensure that
                 * unique buffers are given out, so two tasks should never
                 * be writing the same piece of memory
                 
                sem_get(gen_trace_sem, KAL_WAIT_FOREVER);
                 */

                /* get the __FILE__ and __LINE__ into the trace string */
                sprintf(temp, "%s: %d: %s", module, (int)line, msg);
                /* get arguments into trace string */
                vsprintf(trctmpbuff, temp, ap);

                strncpy(&(trcmsgbuff[slot][0]), trctmpbuff, GENDBGMAXSTRLEN);
                trcmsgbuff[slot][GENDBGMAXSTRLEN-1] = '\0';
                
                /* 
                sem_put(gen_trace_sem);
                */
                
                /* the outgoing message contains a pointer to:
                 * 1. our last trace string
                 * 2. which index was used for this message (so we can free it)
                 * 3. Don't know if 0 represents anything yet.
                 * 4. Indicate whether this was a trace, alert, or debug string
                 */
                out_msg[0] = (u_int32)&(trcmsgbuff[slot][0]);
                out_msg[1] = slot;
                out_msg[2] = 0;
                out_msg[3] = type;

                retcode = qu_send(gen_traceout_q, out_msg);
                if(retcode) {
                    /* TODO: if we fail when posting the out_msg
                     * to traceout the corresponding tracein will be
                     * lost forever.
                     * do we worry about handling this?
                     */
                }
            } /* good slot number */
        } /* successful qu_receive */
    } /*  gen_tracein_q && gen_traceout_q */
}
                
/*
 * All of the debug/trace routines get their messages
 * processed by this task
 * 
 */

void gen_trace_handler_task(void *parm)
{
    int retcode;
    u_int32 out_msg[4], in_msg[4];
    
    do {
        retcode = qu_receive(gen_traceout_q, KAL_WAIT_FOREVER, out_msg);
        if (retcode == RC_OK) {
            
            switch(out_msg[3]) {
                case DBG: case TRC: case ALERT: case ASSRT:
                    printf("%s\n",(char*)out_msg[0]);

                    in_msg[0] = out_msg[1];
                    qu_send(gen_tracein_q, in_msg);

                    break;
                    
                default:
                    
                    in_msg[0] = out_msg[1];
                    qu_send(gen_tracein_q, in_msg);

                    break;
            }
        }
    } while ((retcode == RC_OK) && (out_msg[3] != TRCDBG_SHUTDOWN));

    task_terminate();
}

void genTraceMsg(char *module, u_int32 line, u_int32 trclvl, char* msg, ...) {
    va_list ap;

    va_start(ap, msg);
    
    if (trclvl & gTrcLvl) {
        TraceMsg(module, line, TRC, msg, ap);
    }
    
    va_end(ap);
}            

u_int32 genTraceSetLevel(u_int32 lvl)
{
    u_int32 oldlvl = gTrcLvl;
    gTrcLvl = lvl;
    return oldlvl;
}

#ifdef GENDEBUG
void genDebugMsg(char *module, u_int32 line, u_int32 dbglvl, char* msg, ...)
{
    va_list ap;

    va_start(ap, msg);

    if (dbglvl & gDbgLvl) {
        TraceMsg(module, line, DBG, msg, ap);
    }

    va_end(ap);
}

u_int32 genDebugSetLevel(u_int32 lvl)
{
    u_int32 oldlvl = gDbgLvl;
    gDbgLvl = lvl;
    return oldlvl;
}

#endif

void genAlertSetCallback(unsigned char alrtlvl, u_int32 (*alrtcb)(void))
{
    switch(alrtlvl) {
        case LVL_A0:
            AlertTable.Alert0 = alrtcb;
            break;
        case LVL_A1:
            AlertTable.Alert1 = alrtcb;
            break;
        case LVL_A2:
            AlertTable.Alert2 = alrtcb;
            break;
        case LVL_A3:
            AlertTable.Alert3 = alrtcb;
            break;
        case LVL_A4:
            AlertTable.Alert4 = alrtcb;
            break;
    }
}

u_int32 genAlertMsg(char* module, u_int32 line, unsigned char alrt_lvl, char* msg, ...)
{
    u_int32 retcode = 0;

    if (alrt_lvl <= LVL_A4) {
        va_list ap;
        va_start(ap, msg);
        TraceMsg(module, line, ALERT, msg, ap);
        va_end(ap);
        switch(alrt_lvl) {
            case LVL_A0:
                retcode = AlertTable.Alert0();
                break;
            case LVL_A1:
                retcode = AlertTable.Alert1();
                break;
            case LVL_A2:
                retcode = AlertTable.Alert2();
                break;
            case LVL_A3:
                retcode = AlertTable.Alert3();
                break;
            case LVL_A4:
                retcode = AlertTable.Alert4();
                break;
        }
    }
    return retcode;
}

void genAssert(char *module, u_int32 line, char* msg, u_int32 expr, ...) {
    char temp[80];
    /* Assert doesn't actually use variable argument lists but we
     * need to pass it (nothing) to TraceMsg
     */
    va_list ap;

    /* If the assert fails... */
    if (!expr) {
        sprintf(temp, "ASSERTION FAILED: ( %s )", msg);
        va_start(ap, expr);
        TraceMsg(module, line, ASSRT, temp, ap);
        va_end(ap);
        /* TODO:  Consider a better value to pass to fatal exit. */
        fatal_flush();
        fatal_exit(0xdeadbeef);
    }
}

void genEvent(u_int32 typ, u_int32 val) {
    static int eventcount = 0;

    gEventLog[eventcount].timestamp = 0; /* TODO: how do we get the time? */
    gEventLog[eventcount].EventType = typ;
    gEventLog[eventcount++].value   = val;

    if (eventcount > MAXEVENTS-1) eventcount = 0;
}

/*******************************************************************************/
/* Before resetting everything in the case of a fatal error, dump the current  */
/* contents of the trace queue.                                                */
/*******************************************************************************/
#ifdef GENDEBUG
void fatal_flush(void)
{
  int iPriority;
  int iRetcode;
  u_int32 rec_msg[4];
  
  /* Effectively suspend the trace task so that we don't have 2 people */
  /* fighting for queue messages.                                      */
  
  /* Get this task's priority */
  iPriority = task_priority(task_id(), 0);
  /* Work out how much to bump it to make it top priority */
  iPriority = 127-iPriority;
  /* Set the new priority */
  iPriority = task_priority(task_id(), iPriority);
  
  /* Get the trace task priority */
  iPriority = task_priority(gen_trace_tid, 0);
  /* Set the new priority to 0*/
  iPriority = task_priority(gen_trace_tid, -iPriority);

  /* Read all the messages currently on the trace queue and */
  /* dump them.                                             */
  do
  {
    iRetcode = qu_receive(gen_traceout_q, 0, rec_msg);
    if(iRetcode == RC_OK)
    {
        switch(rec_msg[3]) {
            case DBG: case TRC: case ALERT: case ASSRT:
                printf("%s\n",(char*)rec_msg[0]);
                break;
        }
    }
  } while(iRetcode == RC_OK);
}
#endif   
/****************************************************************************
$log:    $
****************************************************************************/

