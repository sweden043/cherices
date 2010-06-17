/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                     Conexant Systems Inc. (c) 2002                       */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename: multi_demod.c
 *
 * Description: This file contains the implementation of the user-level
 *              interface to multi-instance demod.
 *
 * Author: Billy Jackman
 *
 ****************************************************************************/
/* $Id: demod.c,v 1.24, 2004-03-15 16:33:03Z, Matt Korte$
 ****************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "hwconfig.h"
#include "basetype.h"
#include "demod_user_api.h"
#include "demod_module_api.h"
#include "hwlib.h"
#include "kal.h"
#include "retcodes.h"

/* Provide an upper limit on the number of modules handled in a
   system so we can allocate some data structures statically. */
#define MAX_MODULES  (8)

/* Provide an upper limit on the number of units handled in a
   system so we can allocate some data structures statically.
   This value is based upon MAX_MODULES, but it is not magic.
   Some modules will have a single unit, some may have two or
   more.  Set it to what is needed. */
#define MAX_UNITS (MAX_MODULES * 2)

/* Allow up to 4 opens and callbacks registered per unit. */
#define MAX_CALLBACKS   (4)

/* Just a magic number to appear in a handle. */
#define HANDLE_MAGIC       (0x00c00000)
#define HANDLE_MAGIC_MASK  (0x0ff00000)

/* Define the init pointers for all the demod drivers.  NULL is
   not present.  Adding a new kind of demod to the system will
   require adding a block here and a line in the definition of
   the demod_init array. */

/* The demod_baseband driver is the module that supports the
   baseband input. */
#ifdef DRIVER_INCL_DEMOD_BASEBAND
extern INIT_FUNC  cnxt_baseband_demod_init;
#define DVB_BASEBAND & cnxt_baseband_demod_init
#else
#define DVB_BASEBAND 0
#endif

/* The satellite_110 driver supports the CX24106 (HM1221) and
   CX24110 chips. */
#ifdef DRIVER_INCL_DEMOD_VALIANT
extern INIT_FUNC  cnxt_single_cx24110_demod_init;
#define SAT_110   & cnxt_single_cx24110_demod_init
#else
#define SAT_110   0
#endif

/* The demod_cobra driver supports the CX24121 and CX24130
   Cobra chips. */
#ifdef DRIVER_INCL_DEMOD_COBRA
extern INIT_FUNC  cnxt_cobra_demod_init;
#define SAT_COBRA & cnxt_cobra_demod_init
#else
#define SAT_COBRA 0
#endif

/* The terrestrial driver supports the CX22702 chip. */
#ifdef DRIVER_INCL_DEMOD_TERRESTRIAL
extern INIT_FUNC  cnxt_terrestrial_demod_init;
#define TERRESTRIAL  & cnxt_terrestrial_demod_init
#else
#define TERRESTRIAL  0
#endif

/* The analogue terrestrial driver supports the MACO chip. */
#ifdef DRIVER_INCL_DEMOD_ANALOGUE_TERRESTRIAL
extern INIT_FUNC              cnxt_analogue_terrestrial_demod_init;
#define ANALOGUE_TERRESTRIAL  & cnxt_analogue_terrestrial_demod_init
#else
#define ANALOGUE_TERRESTRIAL  0
#endif

#ifdef DRIVER_INCL_DEMOD_CX24430
extern INIT_FUNC              cnxt_24430_demod_init;
#define CABLE_WABASH & cnxt_24430_demod_init
#else
#define CABLE_WABASH 0
#endif

/* The cable front-end driver supports the Thomson DCF8722 can tuner. */
#ifdef DRIVER_INCL_DEMOD_DCF8722
extern INIT_FUNC              cnxt_cable_demod_init;
#define CABLE_DCF8722 & cnxt_cable_demod_init
#else
#define CABLE_DCF8722 0
#endif

static INIT_FUNC              *demod_init[] =
{
   DVB_BASEBAND,
   SAT_110,
   SAT_COBRA,
   TERRESTRIAL,
   ANALOGUE_TERRESTRIAL,
   CABLE_DCF8722,
   CABLE_WABASH
};

static u_int32                total_number_units;
static u_int32                demod_initialized = 0;

static u_int32                module_base_unit[MAX_MODULES];
static DEMOD_FTABLE           function_table[MAX_MODULES];
static USER_STATUS_FUNCTION   *callback_table[MAX_UNITS][MAX_CALLBACKS];
static u_int32                unit_xlat_table[MAX_UNITS];
static u_int32                is_controlled[MAX_UNITS];
static u_int32                open_flag[MAX_UNITS][MAX_CALLBACKS];

/* stuff for state machine */
typedef enum
{
   DISCONNECTED,
   ACQUIRING_SIGNAL,
   RE_ACQUIRE,
   CALL_CONNECT,
   CONNECTED,
   CALL_DISCONNECT,
   LOCK_LOST,
   LOCK_WANTED,
   INIT_SCAN,
   SCAN_NEXT
} STATE_LIST;

typedef enum
{
   JUST_LOST,
   BOUNCING,
   BOUNCED_OUT,
   LOCKED_OUT
} DEBOUNCE_STATUS;

static int debounce_status_array[MAX_UNITS];

typedef struct
{
   bool        scanning;
   STATE_LIST  state;
   TUNING_SPEC requested_tuning_spec;  /* This is a record of the original connect */
   TUNING_SPEC tuning_spec;
   SCAN_SPEC   scanning_spec;
} UNIT_STATE;

static UNIT_STATE unit_state_machine_info[MAX_UNITS];

/* functions with file scope */
static u_int32    change_demod_state(u_int32     unit,
                                                       u_int32     module,
                                                       STATE_LIST  state);
static bool       tuning_spec_equal(TUNING_SPEC ts1, TUNING_SPEC ts2);

typedef struct
{
   int   connection_timer;
   bool  flag;
} TIMER_STR;

static TIMER_STR  unit_timer[MAX_UNITS];

/* macro to work out the global unit number */
#define GLOBAL_UNIT_NUMBER (unit + module_base_unit[module])

/*****************************************************************************/
/*  FUNCTION:    local_callback                                              */
/*                                                                           */
/*  PARAMETERS:  module - the module number of the caller of this status     */
/*                   function.                                               */
/*               unit - the unit number of the caller of this status         */
/*                   function.                                               */
/*               type - the enumerated type of the status callback.          */
/*               callback_data - pointer to data structure containing data   */
/*                   about this status callback.                             */
/*                                                                           */
/*  DESCRIPTION: This function serves as an intermediary for module level    */
/*               status callbacks.  When a user registers a status callback  */
/*               function, that callback address is saved locally.  This     */
/*               function is then registered with the module level driver as */
/*               its status function callback.  That way when the module     */
/*               makes a status callback, this function receives it and can  */
/*               translate <module,unit> to the appropriate flat unit        */
/*               callback.                                                   */
/*                                                                           */
/*  RETURNS:     nothing.                                                    */
/*                                                                           */
/*  CONTEXT:     May be called from an interrupt or non-interrupt context.   */
/*                                                                           */
/*****************************************************************************/
static void local_callback(u_int32              module,
                           u_int32              unit,
                           DEMOD_CALLBACK_TYPE  type,
                           void                 *callback_data)
{
   int   ii;

   /* Convert from module/unit to flat unit. */
   unit += module_base_unit[module];

   /* Fork this callback out for all registered clients for this unit. */
   for (ii = 0; ii < MAX_CALLBACKS; ++ii)
   {
      if (callback_table[unit][ii])
      {
         callback_table[unit][ii](unit, type, callback_data);
      }
   }
}

/******************************************************************************/
/* Function Name: StateTask                                                   */
/*                                                                            */
/* Description  : The KAL task which keeps the satellite NIM in acquisition.  */
/*                                                                            */
/* Parameters   : parm         Pointer to parameter block - not used          */
/*                                                                            */
/* Returns      : Nothing - Never Terminates                                  */
/*                                                                            */
/******************************************************************************/

#define DEMOD_Q_TIMEOUT (35)  /* Queue wait timeout (ms) */

#define STATE_INFO(x)   (unit_state_machine_info[x])
#define MY_MODULE(x)    (unit_xlat_table[x])
#define MY_UNIT(x)      ((x) - module_base_unit[MY_MODULE(x)])

static u_int32 debounce_time = 250;
#define NUMBER_DEBOUNCE    (5)
#define DEBOUNCE_TIMEOUT   (debounce_time / NUMBER_DEBOUNCE)
#define POLL_TIMEOUT       (250)

void cnxt_demod_set_debounce_time(u_int32 time)
{
   debounce_time = time;
}

static queue_id_t          demod_queue;
static DEMOD_CALLBACK_DATA callback_struct;

static void call_back(u_int32              unit,
                      DEMOD_CALLBACK_TYPE  type,
                      DEMOD_ASYNC_RESULT   result,
                      TUNING_SPEC          *tune_spec)
{
   callback_struct.parm.type = result;
   if (tune_spec != NULL)
   {
      callback_struct.parm.tune = *tune_spec;
   }
   else  /* No tuning spec supplied means call to get_tuning is required */
   {
      if (function_table[MY_MODULE(unit)].get_tuning != NULL)
      {
         function_table[MY_MODULE(unit)].get_tuning(MY_UNIT(unit),
                                                    &(callback_struct.parm.tune));
      }
      else
      {
         callback_struct.parm.tune = unit_state_machine_info[unit].tuning_spec;
      }
   }

   local_callback(MY_MODULE(unit), MY_UNIT(unit), type, &callback_struct);
   return;
}

static DEBOUNCE_STATUS get_debounce(u_int32 unit)
{
   DEBOUNCE_STATUS   retval;
   switch (debounce_status_array[unit])
   {
   case 1:
      retval = BOUNCED_OUT;
      debounce_status_array[unit] = 0;
      break;
   case 0:
      retval = LOCKED_OUT;
      break;
   case (-1):
      debounce_status_array[unit] = NUMBER_DEBOUNCE;
      retval = JUST_LOST;
      break;
   default:
      debounce_status_array[unit]--;
      retval = BOUNCING;
      break;
   }

   return(retval);
}

static void un_debounce(u_int32 unit)
{
   debounce_status_array[unit] = 1;
   return;
}

static void set_debounce(u_int32 unit)
{
   debounce_status_array[unit] = -1;
   return;
}

#define TIMER_RES 10/* Go down 10 ms at a time */

static void clock_func(timer_id_t id, void *pUserData)
{
   u_int32  i;
   for (i = 0; i < total_number_units; i++)
   {
      if (unit_timer[i].connection_timer > 0)
      {
         unit_timer[i].connection_timer -= TIMER_RES;
         if (unit_timer[i].connection_timer <= 0)
         {
            unit_timer[i].flag = TRUE;
         }
      }
   }
}

static bool check_timer(u_int32 unit)
{
   bool  retval;
   if (unit_timer[unit].flag == TRUE)
   {
      retval = TRUE;
      unit_timer[unit].flag = FALSE;
   }
   else
   {
      retval = FALSE;
   }

   return(retval);
}

static void start_timer(u_int32 unit, int timeout)
{
   unit_timer[unit].flag = FALSE;
   unit_timer[unit].connection_timer = timeout;
   return;
}

static void StateTask(void *parm)
{
   int32             glob_unit, force_unit = (-1), saved_unit = (-1);
   u_int32           connection_limit;
   bool              retval;
   DEBOUNCE_STATUS   debounce_status;
   tick_id_t         tick_id;
   u_int32           msg[4];
   /*SCAN_SPEC         delete_me;*/
   TUNING_SPEC       retune;

   #if 0
   #define SCAN_FREQ ((u_int32) 738000000)
   delete_me.type = DEMOD_NIM_TERRESTRIAL;
   delete_me.current.tune.nim_terrestrial_tune.Frequency = SCAN_FREQ;
   delete_me.current.tune.nim_terrestrial_tune.Frame = FRAME_AUTO;
   delete_me.current.tune.nim_terrestrial_tune.InputBW = BANDWIDTH_8_MHZ;
   delete_me.scan.stop = delete_me.current;
   delete_me.scan.stop.tune.nim_terrestrial_tune.Frequency = SCAN_FREQ + (u_int32) 16000000;
   #endif
   if ((tick_id = tick_create((PFNTIMERC) clock_func, (void *)0, "DEMD")) == 0)
   {
      trace_new((TRACE_LEVEL_3 | TRACE_DMD),
                "Can't hook the tick incrementer in.\n");
   }
   else
   {
      if (tick_set(tick_id, (u_int32) TIMER_RES, FALSE) != RC_OK)
      {
         trace_new((TRACE_LEVEL_3 | TRACE_DMD),
                   "Error setting demod tick timer.\n");
      }

      if (tick_start(tick_id) != RC_OK)
      {
         trace_new((TRACE_LEVEL_3 | TRACE_DMD),
                   "Error starting demod tick timer.\n");
      }
   }

   for (glob_unit = 0; glob_unit < (int32) total_number_units; glob_unit++)
   {
      STATE_INFO(glob_unit).state = DISCONNECTED;
      STATE_INFO(glob_unit).scanning = FALSE;
      unit_timer[glob_unit].flag = FALSE;
      unit_timer[glob_unit].connection_timer = -1;
   }

   while (1)
   {
      for (glob_unit = 0; glob_unit < (int32) total_number_units; glob_unit++)
      {
         #define VALID_UNIT(X)   ((X >= 0) && (X < (int32) total_number_units))
         if (VALID_UNIT(force_unit))
         {
            if (!(VALID_UNIT(saved_unit)))
            {
               saved_unit = glob_unit;
            }

            glob_unit = force_unit;
         }
         else if (VALID_UNIT(saved_unit))
         {
            glob_unit = saved_unit;
            saved_unit = -1;
         }

         force_unit = -1;
         if ((void *)(function_table[MY_MODULE(glob_unit)].scan_next) == NULL)
         {
            continue;
         }

         switch (STATE_INFO(glob_unit).state)
         {
         case CALL_CONNECT:
            /* Call connect with new params */
            if (function_table[MY_MODULE(glob_unit)].connect(MY_UNIT(glob_unit),
                                                             &(STATE_INFO(glob_unit).tuning_spec),
                                                             &connection_limit) != DEMOD_SUCCESS)
            {
               STATE_INFO(glob_unit).state = DISCONNECTED;
               call_back(glob_unit,
                         DEMOD_CONNECT_STATUS,
                         DEMOD_ABORTED,
                         &(STATE_INFO(glob_unit).tuning_spec));
               trace_new((TRACE_LEVEL_3 | TRACE_DMD),
                         "Callback, connect failure\n");
               break;
            }
            else
            {
               start_timer(glob_unit, (int)connection_limit);
               STATE_INFO(glob_unit).state = ACQUIRING_SIGNAL;
               /* FALLTHROUGH */
            }

         /* POSSIBLE FALLTHROUGH */
         case ACQUIRING_SIGNAL:
            /* Check for lock status & conditionally do callback */
            function_table[(MY_MODULE(glob_unit))].get_lock(MY_UNIT(glob_unit),
                                                            &retval);
            if (retval == FALSE)
            {
               trace_new((TRACE_LEVEL_3 | TRACE_DMD),
                         "acq%l",
                         (long)STATE_INFO(glob_unit).tuning_spec.tune.nim_terrestrial_tune.Frequency);
               if (check_timer(glob_unit))
               {
                  /* This 'Acquire' has failed */
                  trace_new((TRACE_LEVEL_3 | TRACE_DMD), "Acquire fail\n");
                  if (STATE_INFO(glob_unit).scanning == TRUE)
                  {
                     STATE_INFO(glob_unit).state = SCAN_NEXT;
                  }
                  else
                  {
                     /* Set to poll forever */
                     STATE_INFO(glob_unit).state = LOCK_LOST;
                     call_back(glob_unit,
                               DEMOD_CONNECT_STATUS,
                               DEMOD_FAILED,
                               (TUNING_SPEC *)NULL);
                     trace_new((TRACE_LEVEL_3 | TRACE_DMD),
                               "Callback, acquire fail\n");
                  }
               }
               else
               {
                  /* If we have neither got a lock nor timed out...   */
                  /*    ...continue to acquire, i.e. do nothing here. */
               }
               break;
            }
            else
            {
               /* We have got a signal. We shall see if we need to re-tune */
               /* to the frequency we've got. FALL THROUGH to RE_ACQUIRE   */
               trace_new((TRACE_LEVEL_3 | TRACE_DMD),
                         "We've got a lock, deciding on re-acquire next.\n");
               trace_new((TRACE_LEVEL_3 | TRACE_DMD), "Acquire succeed\n");
            }

         /* FALL THROUGH */
         case RE_ACQUIRE:
            /* Set array member tuning spec to actual tuning param */
            /* This value can then be used for re-acquisition...   */
            /* either now (to fine-tune) or in case of signal fail */
            trace_new((TRACE_LEVEL_3 | TRACE_DMD),
                      "Deciding whether to re-acquire.\n");
            if (((function_table[MY_MODULE(glob_unit)].get_tuning) != NULL) &&
                (STATE_INFO(glob_unit).scanning != TRUE))
            {
               function_table[MY_MODULE(glob_unit)].get_tuning(MY_UNIT(glob_unit),
                                                               &retune);
               STATE_INFO(glob_unit).tuning_spec = retune;
            }

            /* Go to optional re-acquire if exists and is not scanning */
            if (((function_table[MY_MODULE(glob_unit)].re_acquire) != NULL) &&
                (STATE_INFO(glob_unit).scanning != TRUE))
            {
               retval = function_table[MY_MODULE(glob_unit)].re_acquire(&(STATE_INFO(glob_unit).tuning_spec),
                                                                        &retune);
            }
            else
            {
               retval = FALSE;
            }

            if (retval == TRUE)
            {
               /* The actual tune is too far away - retune */
               trace_new((TRACE_LEVEL_3 | TRACE_DMD),
                         "Re-acquire is on - going to call_connect.\n");
               STATE_INFO(glob_unit).state = CALL_CONNECT;
               break;
            }
            else
            {
               trace_new((TRACE_LEVEL_3 | TRACE_DMD),
                         "Re-acquire unnecessary or no re_acquire fn.\n");
               STATE_INFO(glob_unit).state = CONNECTED;
               start_timer(glob_unit, POLL_TIMEOUT);
               /* Callback - Acquire Succeed */
               if (STATE_INFO(glob_unit).scanning == TRUE)
               {
                  call_back(glob_unit,
                            DEMOD_SCAN_STATUS,
                            DEMOD_SCAN_COMPLETE,
                            &retune);
                  trace_new((TRACE_LEVEL_3 | TRACE_DMD),
                            "Callback, scan succeed\n");
                  STATE_INFO(glob_unit).scanning = FALSE;
               }
               else
               {
                  call_back(glob_unit,
                            DEMOD_CONNECT_STATUS,
                            DEMOD_CONNECTED,
                            &retune);
                  trace_new((TRACE_LEVEL_3 | TRACE_DMD),
                            "Callback, acquire succeed\n");
               }
            }
            break;
         case SCAN_NEXT:
            retval = function_table[MY_MODULE(glob_unit)].scan_next(MY_UNIT(glob_unit),
                                                                    &(STATE_INFO(glob_unit).scanning_spec));
            STATE_INFO(glob_unit).tuning_spec = STATE_INFO(glob_unit).scanning_spec.current;
            if (retval == TRUE)
            {
               /* True implies come to end of allowed scanning range... */
               /* WITHOUT getting a lock! */
               call_back(glob_unit,
                         DEMOD_SCAN_STATUS,
                         DEMOD_SCAN_NO_SIGNAL,
                         (TUNING_SPEC *)NULL);
               trace_new((TRACE_LEVEL_3 | TRACE_DMD), "Callback, scan fail\n");
               STATE_INFO(glob_unit).scanning = FALSE;
               STATE_INFO(glob_unit).state = DISCONNECTED;
            }
            else
            {
               STATE_INFO(glob_unit).state = CALL_CONNECT;
            }
            break;
         case CONNECTED:
            /* Check the signal is still there */
            if (check_timer(glob_unit))
            {
               function_table[MY_MODULE(glob_unit)].get_lock(MY_UNIT(glob_unit),
                                                             &retval);
               if (retval == TRUE)
               {
                  start_timer(glob_unit, POLL_TIMEOUT);
                  break;
               }

               /* else FALLTHROUGH */
            }
            else
            {
               break;
            }

         case LOCK_LOST:
            set_debounce(glob_unit);
         /* set_debounce ensures next get_debounce will be JUST_LOST */
         /* FALLTHROUGH */
         case LOCK_WANTED:
            if ((STATE_INFO(glob_unit).state != LOCK_WANTED) ||
                (check_timer(glob_unit)))
            {
               STATE_INFO(glob_unit).state = LOCK_WANTED;   /* for fallthrough */
               /* poll forever to see if the signal returns */
               function_table[MY_MODULE(glob_unit)].get_lock(MY_UNIT(glob_unit),
                                                             &retval);
               if (retval == TRUE)
               {
                  /* We've got a lock back but we don't know how */
                  /* See if re-tuning would be helpful           */
                  trace_new((TRACE_LEVEL_3 | TRACE_DMD),
                            "Poll has been successful. Forcing state to re-acquire.\n");
                  STATE_INFO(glob_unit).state = RE_ACQUIRE;
                  force_unit = glob_unit;             /* Go around again */
               }
               else
               {
                  if (debounce_time == 0)
                  {
                     un_debounce(glob_unit);
                     /* This makes sure it won't bounce at all */
                  }

                  debounce_status = get_debounce(glob_unit);
                  if (debounce_status == BOUNCED_OUT)
                  {
                     /* One-time given up bouncing, start steady poll */
                     connection_limit = POLL_TIMEOUT; /* Now we're polling */
                     trace_new((TRACE_LEVEL_3 | TRACE_DMD),
                               "Callback signal lost & start polling.\n");
                     call_back(glob_unit,
                               DEMOD_CONNECT_STATUS,
                               DEMOD_DRIVER_LOST_SIGNAL,
                               (TUNING_SPEC *)NULL);
                  }
                  else if (debounce_status == LOCKED_OUT)
                  {
                     /* Steady state polling, longer timeout */
                     connection_limit = POLL_TIMEOUT;
                     trace_new((TRACE_LEVEL_3 | TRACE_DMD), "Poll.\n");
                  }
                  else
                  {
                     /* One of the bounces */
                     connection_limit = DEBOUNCE_TIMEOUT;
                     trace_new((TRACE_LEVEL_3 | TRACE_DMD), "Bounce.\n");
                  }

                  start_timer(glob_unit, connection_limit);
               }
            }
            break;
         case CALL_DISCONNECT:
            /* Call disconnect function */
            if (function_table[MY_MODULE(glob_unit)].disconnect(MY_UNIT(glob_unit)) == DEMOD_SUCCESS)
            {
               STATE_INFO(glob_unit).state = DISCONNECTED;
            }
            break;
         case DISCONNECTED:
            /* Do nothing */
            break;
         case INIT_SCAN:
            STATE_INFO(glob_unit).scanning = TRUE;
            STATE_INFO(glob_unit).state = CALL_CONNECT;
            /*STATE_INFO(glob_unit).scanning_spec = delete_me;*/
            STATE_INFO(glob_unit).tuning_spec = STATE_INFO(glob_unit).scanning_spec.current;
            break;
         default:
            /* Never get here - it's an array of enums. */
            break;
         }

         if (qu_receive(demod_queue, DEMOD_Q_TIMEOUT, msg) == RC_OK)
         {
            force_unit = msg[0] /*unit*/ + module_base_unit[msg[1]]; /*module*/
            STATE_INFO(force_unit).state = (STATE_LIST) msg[2];      /*state*/
         }
      }  /* endfor(all units) */
   }     /* endwhile(1) */

   task_terminate();
   return;
}

#undef STATE_INFO
#undef MY_MODULE
#undef MY_UNIT

/*****************************************************************************/
/*  FUNCTION:    change_demod_state                                          */
/*                                                                           */
/*  PARAMETERS:  unit & module number, and the state to change to.           */
/*                                                                           */
/*  DESCRIPTION:                                                             */
/*                                                                           */
/*  RETURNS:                                                                 */
/*                                                                           */
/*  CONTEXT:     May be called from any context, is designed as an interrupt */
/*               callback function.                                          */
/*                                                                           */
/*****************************************************************************/
static u_int32 change_demod_state(u_int32     unit,
                                  u_int32     module,
                                  STATE_LIST  state)
{
   u_int32  msg[4] = { 0 };
   msg[0] = unit;
   msg[1] = module;
   msg[2] = (u_int32) state;
   return(u_int32) (qu_send(demod_queue, msg));
}

static bool tuning_spec_equal(TUNING_SPEC ts1, TUNING_SPEC ts2)
{
   bool  bResult = FALSE;

   if (ts1.type == ts2.type)
   {
      switch (ts1.type)
      {
      case DEMOD_NIM_TERRESTRIAL:
         if (ts1.tune.nim_terrestrial_tune.Frequency == ts2.tune.
                nim_terrestrial_tune.Frequency)
         {
            bResult = TRUE;
         }
         break;

      default:
         trace_new((TRACE_LEVEL_3 | TRACE_DMD),
                   "Cannot check if we are already connected - tuning_spec_equal is not impemented for this tuning spec type (%d)\n",
                   ts1.type);
         break;
      }
   }

   return bResult;
}

u_int32 demod_lock_lost(u_int32 unit, u_int32 module)
{
   return(change_demod_state(unit, module, LOCK_LOST));
}

u_int32 demod_lock_found(u_int32 unit, u_int32 module)
{
   return(change_demod_state(unit, module, RE_ACQUIRE));
}

/*****************************************************************************/
/*  FUNCTION:    cnxt_demod_init                                             */
/*                                                                           */
/*  PARAMETERS:  number_units - pointer to an integer to be filled out with  */
/*                   the total number of units in this module.               */
/*                                                                           */
/*  DESCRIPTION: This function iterates through the demod_init table and     */
/*               initializes all the modules that have defined               */
/*               initialization functions there.  This function keeps track  */
/*               of the number of units each module initialization reports   */
/*               and returns a count of units in the system.                 */
/*                                                                           */
/*  RETURNS:     DEMOD_INITIALIZED - the system has already been             */
/*                   initialized.                                            */
/*               DEMOD_MODULE_COUNT - there are more initialization          */
/*                   functions in the demod_init table than local data       */
/*                   structures can accomodate.  Expand the structures.      */
/*               DEMOD_BAD_FTABLE - a module did not correctly fill out the  */
/*                   interface function table.                               */
/*               DEMOD_BAD_UNIT - a module reported successful               */
/*                   initialization but a bad number of units in the module. */
/*               DEMOD_UNIT_COUNT - there are more units in the component    */
/*                   modules than local data structures can accomodate.      */
/*                   Expand the structures.                                  */
/*               DEMOD_SUCCESS - the function completed successfully.        */
/*               The error indication from a module initialization routine.  */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
DEMOD_STATUS cnxt_demod_init(u_int32 *number_units)
{
   u_int32        number_demod_slots, number_demod_modules;
   u_int32        ii, jj, unit_count, current_base_unit = 0;
   DEMOD_STATUS   ret_val;
   bool           need_task = FALSE;
   u_int32        module_unit_count[MAX_MODULES], current_module, *ptr;

   if (!number_units)
   {
      return DEMOD_BAD_PARAMETER;
   }

   if (demod_initialized)
   {
      return DEMOD_INITIALIZED;
   }
   else
   {
      demod_initialized = 1;
   }

   /* how many drivers to initialize? */
   number_demod_slots = sizeof(demod_init) / sizeof(INIT_FUNC *);
   number_demod_modules = 0;
   for (ii = 0; ii < number_demod_slots; ++ii)
   {
      if (demod_init[ii])
      {
         ++number_demod_modules;
      }
   }

   if (number_demod_modules > MAX_MODULES)
   {
      return DEMOD_MODULE_COUNT;
   }

   /* initialize the modules */
   current_module = 0;
   for (ii = 0; ii < number_demod_slots; ++ii)
   {
      if (demod_init[ii])
      {
         unit_count = 0;
         ret_val = demod_init[ii](current_module, &unit_count, &function_table[current_module]);

         /* Check for the special case of no hardware present for the demod.
            A return of DEMOD_NO_HARDWARE means to continue initialization and
            allows autoconfiguration of interfaces. */
         if (ret_val != DEMOD_NO_HARDWARE)
         {
            if (ret_val != DEMOD_SUCCESS)
            {
               /* fatal error on initialization */
               return ret_val;
            }
            else
            {
               if (unit_count)
               {
                  module_unit_count[current_module] = unit_count;
                  module_base_unit[current_module] = current_base_unit;
                  current_base_unit += unit_count;
                  if (!function_table[current_module].unit_type ||
                      !function_table[current_module].ioctl ||
                      !function_table[current_module].connect ||
                      !function_table[current_module].disconnect ||
                      (
                         (
                            !function_table[current_module].get_tuning ||
                            !function_table[current_module].set_callback ||
                            !function_table[current_module].clear_callback
                         ) && !function_table[current_module].scan_next
                      ) ||
                      !function_table[current_module].get_signal ||
                      !function_table[current_module].get_lock)
                  {
                     /* did not fill out the function table correctly */
                     return DEMOD_BAD_FTABLE;
                  }
               }
               else
               {
                  /* why a successful init with no units? */
                  return DEMOD_BAD_UNIT;
               }

               ++current_module;
            }
         }
         else
         {
            --number_demod_modules;
         }
      }
   }

   *number_units = total_number_units = current_base_unit;
   if (total_number_units > MAX_UNITS)
   {
      return DEMOD_UNIT_COUNT;
   }

   /* initialize the unit translation table */
   memset(unit_xlat_table, 0, sizeof(unit_xlat_table));
   ptr = unit_xlat_table;
   for (ii = 0; ii < number_demod_modules; ++ii)
   {
      for (jj = 0; jj < module_unit_count[ii]; ++jj)
      {
         *ptr++ = ii;
      }
   }

   /* Create the state machine task */
   need_task = FALSE;
   for (ii = 0; ((ii < number_demod_modules) && (need_task == FALSE)); ++ii)
   {
      if (function_table[ii].scan_next)
      {
         need_task = TRUE;
      }
   }

   if (need_task == TRUE)
   {
      demod_queue = qu_create(10, "DMQU");
      task_create(&StateTask,
                  (void *)NULL,
                  (void *)NULL,
                  DMOD_TASK_STACK_SIZE,
                  DMOD_TASK_PRIORITY,
                  DMOD_TASK_NAME);
   }

   /* initialize the local callback table */
   memset(callback_table, 0, sizeof(callback_table));

   /* initialize the open flag table */
   memset(open_flag, 0, sizeof(open_flag));

   return DEMOD_SUCCESS;
}

/*****************************************************************************/
/*  FUNCTION:    cnxt_demod_get_number_of_units                              */
/*                                                                           */
/*  PARAMETERS:  number_units - pointer to the variable to be set with the   */
/*                   number of units available in the system.                */
/*                                                                           */
/*  DESCRIPTION: This function returns the count of units present in the     */
/*               system.                                                     */
/*                                                                           */
/*  RETURNS:     DEMOD_UNINITIALIZED - this module has not been initialized. */
/*               DEMOD_SUCCESS - the function completed successfully.        */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
DEMOD_STATUS cnxt_demod_get_number_of_units(u_int32 *number_units)
{
   if (!demod_initialized)
   {
      return DEMOD_UNINITIALIZED;
   }

   if (!number_units)
   {
      return DEMOD_BAD_PARAMETER;
   }

   *number_units = total_number_units;
   return DEMOD_SUCCESS;
}

/*****************************************************************************/
/*  FUNCTION:    cnxt_demod_get_unit_type                                    */
/*                                                                           */
/*  PARAMETERS:  unit - the unit number for which type information is        */
/*                   requested.                                              */
/*               unit_type - pointer to the DEMOD_NIM_TYPE variable to be    */
/*                   filled out.                                             */
/*                                                                           */
/*  DESCRIPTION: This function gathers information about the type of the     */
/*               unit that is the subject of the request by calling the      */
/*               appropriate module function to supply the information.      */
/*                                                                           */
/*  RETURNS:     DEMOD_UNINITIALIZED - this module has not been initialized. */
/*               DEMOD_BAD_UNIT - the unit number is not valid.              */
/*               The return value from the called module function.           */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
DEMOD_STATUS cnxt_demod_get_unit_type(u_int32 unit, DEMOD_NIM_TYPE *unit_type)
{
   u_int32  module;

   if (!demod_initialized)
   {
      return DEMOD_UNINITIALIZED;
   }

   if (unit >= total_number_units)
   {
      return DEMOD_BAD_UNIT;
   }

   if (!unit_type)
   {
      return DEMOD_BAD_PARAMETER;
   }

   module = unit_xlat_table[unit];
   unit -= module_base_unit[module];
   return function_table[module].unit_type(unit, unit_type);
}

/*****************************************************************************/
/*  FUNCTION:    cnxt_demod_get_unit_num                                     */
/*                                                                           */
/*  PARAMETERS:  handle - the handle for which the connect operation is      */
/*                   requested.                                              */
/*               unitnum - pointer to the u_int32 which will be written with */
/*                   the unit number corresponding to the supplied handle    */
/*                                                                           */
/*  DESCRIPTION: This function allows the caller to determine the unit       */
/*               number associated with the supplied handle.                 */
/*                                                                           */
/*  RETURNS:     DEMOD_UNINITIALIZED - the system has not been initialized.  */
/*               DEMOD_SUCCESS - unit number retrieved successfully          */
/*               DEMOD_BAD_PARAMETER - NULL pointer passed                   */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
DEMOD_STATUS cnxt_demod_get_unit_num(u_int32 handle, u_int32 *unitnum)
{
   if (!demod_initialized)
   {
      return DEMOD_UNINITIALIZED;
   }

   if ((handle & HANDLE_MAGIC_MASK) != HANDLE_MAGIC)
   {
      return DEMOD_BAD_HANDLE;
   }

   if (!unitnum)
   {
      return DEMOD_BAD_PARAMETER;
   }

   *unitnum = module_base_unit[(handle >> 8) & 0xFF] + (handle & 0xff);

   return DEMOD_SUCCESS;
}

/*****************************************************************************/
/*  FUNCTION:    cnxt_demod_open                                             */
/*                                                                           */
/*  PARAMETERS:  unit - the unit number for which the open operation is      */
/*                   requested.                                              */
/*               controlled - flag to indicate whether the client requests   */
/*                   control capability for the interface (ability to order  */
/*                   ioctl, connect, disconnect, or scan operations).        */
/*               handle - pointer to a variable to be set with the value of  */
/*                   a handle to be used for subsequent operations.          */
/*                                                                           */
/*  DESCRIPTION: This function links together a client application and a     */
/*               unit in the system by assigning a handle for the client to  */
/*               use in subsequent calls.                                    */
/*                                                                           */
/*  RETURNS:     DEMOD_UNINITIALIZED - the system has not been initialized.  */
/*               DEMOD_BAD_UNIT - the unit number is not valid.              */
/*               DEMOD_CONTROLLED - control access is requested to a device  */
/*                   that is already controlled by another client.           */
/*               DEMOD_OPEN_COUNT - the system is already handling the       */
/*                   maximum number of clients for the unit.                 */
/*               DEMOD_SUCCESS - the function completed successfully.        */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
DEMOD_STATUS cnxt_demod_open(u_int32 unit, bool controlled, u_int32 *handle)
{
   u_int32  ii, flat_unit, module;

   if (!demod_initialized)
   {
      return DEMOD_UNINITIALIZED;
   }

   if (unit >= total_number_units)
   {
      return DEMOD_BAD_UNIT;
   }

   if (controlled && is_controlled[unit])
   {
      return DEMOD_CONTROLLED;
   }

   if (!handle)
   {
      return DEMOD_BAD_PARAMETER;
   }

   /* find a handle to give back for this unit */
   *handle = 0;
   for (ii = 0; ii < MAX_CALLBACKS; ++ii)
   {
      if (open_flag[unit][ii] == 0)
      {
         open_flag[unit][ii] = 1;
         *handle = HANDLE_MAGIC + (ii << 16);
         break;
      }
   }

   if (*handle == 0)
   {
      return DEMOD_OPEN_COUNT;
   }

   flat_unit = unit;
   module = unit_xlat_table[unit];
   unit -= module_base_unit[module];

   is_controlled[flat_unit] |= controlled;
   *handle |= (module << 8) | unit;
   if (controlled)
   {
      *handle |= 0x80000000;
   }

   return DEMOD_SUCCESS;
}

/*****************************************************************************/
/*  FUNCTION:    cnxt_demod_close                                            */
/*                                                                           */
/*  PARAMETERS:  handle - the handle to be closed.                           */
/*                                                                           */
/*  DESCRIPTION: This function terminates operation of a unit by a client.   */
/*               Once calling cnxt_demod_close, a client must not use the    */
/*               handle value to access the system again or the access may   */
/*               cause undefined operation.                                  */
/*                                                                           */
/*  RETURNS:     DEMOD_UNINITIALIZED - the system has not been initialized.  */
/*               DEMOD_SUCCESS - the function completed successfully.        */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
DEMOD_STATUS cnxt_demod_close(u_int32 handle)
{
   u_int32  module, unit, flat_unit, controlled, slot;

   if (!demod_initialized)
   {
      return DEMOD_UNINITIALIZED;
   }

   if ((handle & HANDLE_MAGIC_MASK) != HANDLE_MAGIC)
   {
      return DEMOD_BAD_HANDLE;
   }

   module = (handle >> 8) & 0xff;
   unit = handle & 0xff;
   flat_unit = unit + module_base_unit[module];
   controlled = (handle & 0x80000000) != 0;

   is_controlled[flat_unit] &= (controlled == 0);

   slot = (handle >> 16) & 0x3f;
   if (slot >= MAX_CALLBACKS)
   {
      return DEMOD_BAD_UNIT;
   }

   open_flag[flat_unit][slot] = 0;

   #if 0
   for (ii = 0; ii < MAX_CALLBACKS; ++ii)
   {
      if (open_flag[unit][ii] == handle)
      {
         open_flag[unit][ii] = 0;
         break;
      }
   }
   #endif

   return DEMOD_SUCCESS;
}

/*****************************************************************************/
/*  FUNCTION:    cnxt_demod_ioctl                                            */
/*                                                                           */
/*  PARAMETERS:  handle - the handle for which the ioctl operation is        */
/*                   requested.                                              */
/*               type - the type of ioctl operation requested.               */
/*               data - pointer to the DEMOD_IOCTL_TYPE structure that       */
/*                   contains data for the operation or will be filled out   */
/*                   with data by the operation.                             */
/*                                                                           */
/*  DESCRIPTION: This function implements various module-specific control or */
/*               status functions.                                           */
/*                                                                           */
/*  RETURNS:     DEMOD_UNINITIALIZED - the system has not been initialized.  */
/*               DEMOD_NOT_CONTROLLED - the handle for which the operation   */
/*                   is requested does not have control of the device.       */
/*               The return value from the called module function.           */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
DEMOD_STATUS cnxt_demod_ioctl(u_int32 handle, DEMOD_IOCTL_TYPE type, void *data)
{
   u_int32  module, unit;

   if (!demod_initialized)
   {
      return DEMOD_UNINITIALIZED;
   }

   if ((handle & HANDLE_MAGIC_MASK) != HANDLE_MAGIC)
   {
      return DEMOD_BAD_HANDLE;
   }

   module = (handle >> 8) & 0xff;
   unit = handle & 0xff;

   if (!(handle & 0x80000000))
   {
      return DEMOD_NOT_CONTROLLED;
   }

   return function_table[module].ioctl(unit, type, data);
}

/*****************************************************************************/
/*  FUNCTION:    cnxt_demod_connect                                          */
/*                                                                           */
/*  PARAMETERS:  handle - the handle for which the connect operation is      */
/*                   requested.                                              */
/*               tuning - pointer to the TUNING_SPEC structure containing    */
/*                   parameters for the requested connection.                */
/*                                                                           */
/*  DESCRIPTION: This function connects to a stream by calling the module    */
/*               driver to perform the actual connection.                    */
/*                                                                           */
/*  RETURNS:     DEMOD_UNINITIALIZED - the system has not been initialized.  */
/*               DEMOD_NOT_CONTROLLED - the handle for which the operation   */
/*                   is requested does not have control of the device.       */
/*               The return value from the called module function.           */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
DEMOD_STATUS cnxt_demod_connect(u_int32 handle, TUNING_SPEC *tuning)
{
   u_int32  module, unit, dummy;

   if (!demod_initialized)
   {
      return DEMOD_UNINITIALIZED;
   }

   if ((handle & HANDLE_MAGIC_MASK) != HANDLE_MAGIC)
   {
      return DEMOD_BAD_HANDLE;
   }

   if (!tuning)
   {
      return DEMOD_BAD_PARAMETER;
   }

   module = (handle >> 8) & 0xff;
   unit = handle & 0xff;

   if (!(handle & 0x80000000))
   {
      return DEMOD_NOT_CONTROLLED;
   }

   if (function_table[module].scan_next)
   {
      /* manipulate the state machine */

      if ((unit_state_machine_info[GLOBAL_UNIT_NUMBER].state == CONNECTED) ||
          (unit_state_machine_info[GLOBAL_UNIT_NUMBER].state == DISCONNECTED) ||
          (unit_state_machine_info[GLOBAL_UNIT_NUMBER].state == LOCK_WANTED))
      {
         unit_state_machine_info[GLOBAL_UNIT_NUMBER].scanning = FALSE;

         /* Check if we are already connected to this multiplex */
         if (tuning_spec_equal(*tuning,
                               unit_state_machine_info[GLOBAL_UNIT_NUMBER].requested_tuning_spec) &&
             (unit_state_machine_info[GLOBAL_UNIT_NUMBER].state == CONNECTED))
         {
            /* Call callback function */
            call_back(GLOBAL_UNIT_NUMBER,
                      DEMOD_CONNECT_STATUS,
                      DEMOD_CONNECTED,
                      &unit_state_machine_info[GLOBAL_UNIT_NUMBER].tuning_spec);
            trace_new((TRACE_LEVEL_3 | TRACE_DMD),
                      "Callback, acquire succeed\n");

            return DEMOD_SUCCESS;
         }
         else
         {
            /* set the TUNING_SPEC */
            unit_state_machine_info[GLOBAL_UNIT_NUMBER].tuning_spec = *tuning;

            /* keep a record of the tuning spec */
            unit_state_machine_info[GLOBAL_UNIT_NUMBER].requested_tuning_spec = *tuning;

            /* change the state */
            if (change_demod_state(unit, module, CALL_CONNECT) == RC_OK)
            {
               return DEMOD_SUCCESS;
            }
            else
            {
               return DEMOD_IS_BUSY;
            }
         }
      }
      else
      {
         /* the state machine is busy */
         return DEMOD_IS_BUSY;
      }
   }
   else
   {
      /* pass the call down to the connect function */
      return function_table[module].connect(unit, tuning, &dummy);
   }
}

/*****************************************************************************/
/*  FUNCTION:    cnxt_demod_disconnect                                       */
/*                                                                           */
/*  PARAMETERS:  handle - the handle for which the disconnect operation is   */
/*                   requested.                                              */
/*                                                                           */
/*  DESCRIPTION: This function disconnects from a stream by calling the      */
/*               module driver to perform the actual disconnection.          */
/*                                                                           */
/*  RETURNS:     DEMOD_UNINITIALIZED - the system has not been initialized.  */
/*               DEMOD_NOT_CONTROLLED - the handle for which the operation   */
/*                   is requested does not have control of the device.       */
/*               The return value from the called module function.           */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
DEMOD_STATUS cnxt_demod_disconnect(u_int32 handle)
{
   u_int32  module, unit;

   if (!demod_initialized)
   {
      return DEMOD_UNINITIALIZED;
   }

   if ((handle & HANDLE_MAGIC_MASK) != HANDLE_MAGIC)
   {
      return DEMOD_BAD_HANDLE;
   }

   module = (handle >> 8) & 0xff;
   unit = handle & 0xff;

   if (!(handle & 0x80000000))
   {
      return DEMOD_NOT_CONTROLLED;
   }

   if (function_table[module].scan_next)
   {

      /* manipulate the state machine */
      /* change the state */
      if (change_demod_state(unit, module, CALL_DISCONNECT) == RC_OK)
      {
         /* Clear the requested tuning spec... */
         memset(&unit_state_machine_info[GLOBAL_UNIT_NUMBER].requested_tuning_spec,
                0,
                sizeof(TUNING_SPEC));

         return DEMOD_SUCCESS;
      }
      else
      {
         return DEMOD_IS_BUSY;
      }
   }
   else
   {
      /* pass the call down to the connect function */

      return function_table[module].disconnect(unit);
   }
}

/*****************************************************************************/
/*  FUNCTION:    cnxt_demod_scan                                             */
/*                                                                           */
/*  PARAMETERS:  handle - the handle for which the scan operation is         */
/*                   requested.                                              */
/*               scanning - pointer to the SCAN_SPEC structure containing     */
/*                   parameters for the requested scan.                      */
/*                                                                           */
/*  DESCRIPTION: This function scans the carrier using the parameters        */
/*                  set in SCAN_SPEC                                         */
/*                                                                           */
/*  RETURNS:     DEMOD_UNINITIALIZED - the system has not been initialized.  */
/*               DEMOD_NOT_CONTROLLED - the handle for which the operation   */
/*                   is requested does not have control of the device.       */
/*               The return value from the called module function.           */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
DEMOD_STATUS cnxt_demod_scan(u_int32 handle, SCAN_SPEC *scanning)
{
   u_int32  module, unit;

   if (!demod_initialized)
   {
      return DEMOD_UNINITIALIZED;
   }

   if ((handle & HANDLE_MAGIC_MASK) != HANDLE_MAGIC)
   {
      return DEMOD_BAD_HANDLE;
   }

   if (!scanning)
   {
      return DEMOD_BAD_PARAMETER;
   }

   module = (handle >> 8) & 0xff;
   unit = handle & 0xff;

   if (!(handle & 0x80000000))
   {
      return DEMOD_NOT_CONTROLLED;
   }

   if (function_table[module].scan_next)
   {
      /* manipulate the state machine */

      if ((unit_state_machine_info[GLOBAL_UNIT_NUMBER].state == CONNECTED) ||
          (unit_state_machine_info[GLOBAL_UNIT_NUMBER].state == DISCONNECTED) ||
          (unit_state_machine_info[GLOBAL_UNIT_NUMBER].state == LOCK_WANTED))
      {
         /* set the TUNING_SPEC */
         unit_state_machine_info[GLOBAL_UNIT_NUMBER].scanning_spec = *scanning;
         /* change the state */
         if (change_demod_state(unit, module, INIT_SCAN) == RC_OK)
         {
            /* Clear the requested tuning spec... */
            memset(&unit_state_machine_info[GLOBAL_UNIT_NUMBER].requested_tuning_spec,
                   0,
                   sizeof(TUNING_SPEC));
            return DEMOD_SUCCESS;
         }
         else
         {
            return DEMOD_IS_BUSY;
         }
      }
      else
      {
         /* the state machine is busy */
         return DEMOD_IS_BUSY;
      }
   }
   else
   {
      /* pass the call down to the scan function */
      return function_table[module].scan(unit, scanning);
   }
}

/*****************************************************************************/
/*  FUNCTION:    cnxt_demod_get_tuning                                       */
/*                                                                           */
/*  PARAMETERS:  handle - the handle for which the get tuning operation is   */
/*                   requested.                                              */
/*               tuning - pointer to the TUNING_SPEC structure to be filled  */
/*                   out with parameters for the current connection.         */
/*                                                                           */
/*  DESCRIPTION: This function returns the tuning parameters for the         */
/*               specified handle by calling the appropriate module function */
/*               to supply the information.                                  */
/*                                                                           */
/*  RETURNS:     DEMOD_UNINITIALIZED - the system has not been initialized.  */
/*               The return value from the called module function.           */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
DEMOD_STATUS cnxt_demod_get_tuning(u_int32 handle, TUNING_SPEC *tuning)
{
   u_int32  module, unit;

   if (!demod_initialized)
   {
      return DEMOD_UNINITIALIZED;
   }

   if ((handle & HANDLE_MAGIC_MASK) != HANDLE_MAGIC)
   {
      return DEMOD_BAD_HANDLE;
   }

   if (!tuning)
   {
      return DEMOD_BAD_PARAMETER;
   }

   module = (handle >> 8) & 0xff;
   unit = handle & 0xff;

   if (function_table[module].get_tuning != NULL)
   {
      return function_table[module].get_tuning(unit, tuning);
   }
   else
   {
      *tuning = unit_state_machine_info[GLOBAL_UNIT_NUMBER].tuning_spec;
      return DEMOD_SUCCESS;
   }
}

/*****************************************************************************/
/*  FUNCTION:    cnxt_demod_set_callback                                     */
/*                                                                           */
/*  PARAMETERS:  handle - the handle for which the set callback operation is */
/*                   requested.                                              */
/*               callback - the function address to be used for the callback */
/*                   for this handle.                                        */
/*                                                                           */
/*  DESCRIPTION: This function records the callback function to be used for  */
/*               the specified handle and then registers its local callback  */
/*               function with the specified module.                         */
/*                                                                           */
/*  RETURNS:     DEMOD_UNINITIALIZED - the system has not been initialized.  */
/*               The return value from the called module function.           */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
DEMOD_STATUS cnxt_demod_set_callback(u_int32              handle,
                                     USER_STATUS_FUNCTION *callback)
{
   u_int32  module, unit, callback_number, physical_unit;

   if (!demod_initialized)
   {
      return DEMOD_UNINITIALIZED;
   }

   if ((handle & HANDLE_MAGIC_MASK) != HANDLE_MAGIC)
   {
      return DEMOD_BAD_HANDLE;
   }

   if (!callback)
   {
      return DEMOD_BAD_PARAMETER;
   }

   module = (handle >> 8) & 0xff;            /* decode the module */
   unit = handle & 0xff;                     /* decode the relative unit number */
   callback_number = (handle >> 16) & 0x3f;  /* decode which user application number */
   physical_unit = module_base_unit[module] + unit;   /* make the unit into a flat(physical) unit number */

   callback_table[physical_unit][callback_number] = callback;
   if (function_table[module].set_callback != NULL)
   {
      return function_table[module].set_callback(unit, local_callback);
   }
   else
   {
      return DEMOD_SUCCESS;
   }
}

/*****************************************************************************/
/*  FUNCTION:    cnxt_demod_clear_callback                                   */
/*                                                                           */
/*  PARAMETERS:  handle - the handle for which the callback function is to   */
/*                   cleared.                                                */
/*                                                                           */
/*  DESCRIPTION: This function clears the callback pointer for the specified */
/*               handle.  If there are no more user callbacks registered for */
/*               the module, the module callback is cleared.                 */
/*                                                                           */
/*  RETURNS:     DEMOD_UNINITIALIZED - the system has not been initialized.  */
/*               DEMOD_SUCCESS - the function completed successfully.        */
/*               The return value from the called module function.           */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
DEMOD_STATUS cnxt_demod_clear_callback(u_int32 handle)
{
   u_int32  module, unit, callback_number, ii, physical_unit;

   if (!demod_initialized)
   {
      return DEMOD_UNINITIALIZED;
   }

   if ((handle & HANDLE_MAGIC_MASK) != HANDLE_MAGIC)
   {
      return DEMOD_BAD_HANDLE;
   }

   module = (handle >> 8) & 0xff;
   unit = handle & 0xff;
   callback_number = (handle >> 16) & 0x3f;
   physical_unit = module_base_unit[module] + unit;   /* make the unit into a flat(physical) unit number */

   callback_table[physical_unit][callback_number] = 0;

   for (ii = 0; ii < MAX_CALLBACKS; ++ii)
   {
      if (callback_table[unit][ii])
      {
         return DEMOD_SUCCESS;
      }
   }

   if (function_table[module].clear_callback != NULL)
   {
      return function_table[module].clear_callback(unit);
   }
   else
   {
      return DEMOD_SUCCESS;
   }
}

/*****************************************************************************/
/*  FUNCTION:    cnxt_demod_get_signal_stats                                 */
/*                                                                           */
/*  PARAMETERS:  handle - the handle for which the get signal stats          */
/*                   operation is requested.                                 */
/*               signal_stats - pointer to the SIGNAL_STATS structure to be  */
/*                   filled out with parameters for the specified unit.      */
/*                                                                           */
/*  DESCRIPTION: This function returns the signal statistics for the         */
/*               specified handle by calling the appropriate module function */
/*               to supply the information.                                  */
/*                                                                           */
/*  RETURNS:     DEMOD_UNINITIALIZED - the system has not been initialized.  */
/*               The return value from the called module function.           */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
DEMOD_STATUS cnxt_demod_get_signal_stats(u_int32        handle,
                                         SIGNAL_STATS   *signal_stats)
{
   u_int32  module, unit;

   if (!demod_initialized)
   {
      return DEMOD_UNINITIALIZED;
   }

   if ((handle & HANDLE_MAGIC_MASK) != HANDLE_MAGIC)
   {
      return DEMOD_BAD_HANDLE;
   }

   if (!signal_stats)
   {
      return DEMOD_BAD_PARAMETER;
   }

   module = (handle >> 8) & 0xff;
   unit = handle & 0xff;

   return function_table[module].get_signal(unit, signal_stats);
}

/*****************************************************************************/
/*  FUNCTION:    cnxt_demod_get_lock_status                                  */
/*                                                                           */
/*  PARAMETERS:  handle - the handle for which the get lock status operation */
/*                   is requested.                                           */
/*               locked - pointer to the boolean to be filled out indicating */
/*                   locked/not locked for the specified handle.             */
/*                                                                           */
/*  DESCRIPTION: This function returns the lock status for the specified     */
/*               handle by calling the appropriate module function to supply */
/*               the information.                                            */
/*                                                                           */
/*  RETURNS:     DEMOD_UNINITIALIZED - the system has not been initialized.  */
/*               The return value from the called module function.           */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
DEMOD_STATUS cnxt_demod_get_lock_status(u_int32 handle, bool *locked)
{
   u_int32  module, unit;

   if (!demod_initialized)
   {
      return DEMOD_UNINITIALIZED;
   }

   if ((handle & HANDLE_MAGIC_MASK) != HANDLE_MAGIC)
   {
      return DEMOD_BAD_HANDLE;
   }

   if (!locked)
   {
      return DEMOD_BAD_PARAMETER;
   }

   module = (handle >> 8) & 0xff;
   unit = handle & 0xff;

   return function_table[module].get_lock(unit, locked);
}

/****************************************************************************
 * Modifications:
 *
 * $Log: 
 *  25   mpeg      1.24        3/15/04 10:33:03 AM    Matt Korte      CR(s) 
 *        8566 : Add support for Thomson Cable Tuner/Demod
 *  24   mpeg      1.23        12/2/03 5:03:19 AM     Ian Mitchell    CR(s): 
 *        7739 Added support for the new analogue terrestrial demod.
 *        
 *  23   mpeg      1.22        10/3/03 9:19:14 AM     Billy Jackman   SCR(s) 
 *        7579 :
 *        Added some parameter checking to detect some errors tested for by the
 *         test
 *        harness satellite demod tests.
 *        
 *  22   mpeg      1.21        6/9/03 11:20:16 AM     Dave Wilson     SCR(s) 
 *        6751 :
 *        Corrected cnxt_demod_get_unit_num. Previously, it returned a unit 
 *        number
 *        within the module in use rather than a flat unit number. As a result,
 *         the 
 *        number returned was not the same as the number passed on the original
 *         call
 *        to cnxt_demod_open.
 *        
 *  21   mpeg      1.20        5/15/03 3:27:40 PM     Dave Wilson     SCR(s) 
 *        6362 6105 :
 *        Removed an "unreferenced local variable" warning that crept in on the
 *         last
 *        edit.
 *        
 *  20   mpeg      1.19        5/15/03 12:33:34 PM    Dave Wilson     SCR(s) 
 *        6105 6362 :
 *        Added cnxt_demod_get_unit_num API to allow the unit number to be 
 *        retrieved
 *        given a demod instance handle.
 *        
 *  19   mpeg      1.18        2/11/03 2:39:38 PM     Billy Jackman   SCR(s) 
 *        5143 :
 *        Changed DEMOD_BUSY to DEMOD_IS_BUSY to avoid conflict with OpenTV 
 *        include
 *        file demod.h.
 *        
 *  18   mpeg      1.17        12/12/02 11:13:22 AM   Ian Mitchell    SCR(s): 
 *        5016 
 *        If a connect fails to lock go into the LOCK_LOST state so it polls 
 *        for lock.
 *        
 *  17   mpeg      1.16        12/9/02 5:14:50 AM     Ian Mitchell    SCR(s): 
 *        5016 
 *        Reset the scanning flag when the connect function is called.
 *        
 *  16   mpeg      1.15        11/29/02 8:39:42 AM    Ian Mitchell    SCR(s): 
 *        5016 
 *        Only reconect if the demodulator is not scanning
 *        
 *  15   mpeg      1.14        11/27/02 2:11:56 PM    Billy Jackman   SCR(s) 
 *        4977 :
 *        Check for an explicit return code indicating that NIM hardware is not
 *         present
 *        to allow specification of multiple satellite NIMs in the 
 *        configuration file
 *        and use of whichever NIM is found at run time.
 *        
 *  14   mpeg      1.13        11/19/02 10:54:08 AM   Ian Mitchell    SCR(s): 
 *        4985 
 *        Added element to UNIT_INFO for remembering the original tuning spec 
 *        requested to connect to.
 *        If annother request is made that is the same as the last original 
 *        request and its state is connected the callback function is called 
 *        straight away.
 *        
 *  13   mpeg      1.12        9/10/02 8:03:18 AM     Ian Mitchell    SCR(s): 
 *        4563 
 *        Callback structure that the genral demod state machine sends with a 
 *        callback is not consistent with the other demod drivers. I have 
 *        changed it to demod_callback_data and changed the refrences to it.
 *        
 *  12   mpeg      1.11        9/6/02 4:58:18 PM      Miles Bintz     SCR(s) 
 *        4556 :
 *        changed unit to flat_unit in cnxt_demod_close
 *        
 *        
 *  11   mpeg      1.10        7/24/02 12:12:34 PM    Steven Jones    SCR(s): 
 *        4093 
 *        Remove dummy code which was only ever included for testing purposes.
 *        
 *  10   mpeg      1.9         7/8/02 1:45:34 PM      Billy Jackman   SCR(s) 
 *        4148 :
 *        Change usage of 'cnxt_bool' in demod interface to 'bool'.
 *        
 *  9    mpeg      1.8         6/26/02 5:33:56 AM     Steven Jones    SCR(s): 
 *        4084 
 *        Cosmetic changes only.
 *        
 *  8    mpeg      1.7         6/18/02 11:55:54 AM    Steven Jones    SCR(s): 
 *        3960 
 *        Major update, featuring the supplying of a new common demod task to 
 *        serve all possible demod driver plug-ins.
 *        
 *  7    mpeg      1.6         6/13/02 4:40:50 PM     Miles Bintz     SCR(s) 
 *        4001 :
 *        Added support for demod_cx24430
 *        
 *        
 *  6    mpeg      1.5         4/17/02 3:22:44 PM     Ray Mack        SCR(s) 
 *        3569 :
 *        fixed same problem as set_callback in clear_callback.
 *        
 *  5    mpeg      1.4         4/17/02 11:19:58 AM    Ray Mack        SCR(s) 
 *        3569 :
 *        The calculations in local_callback and set_callback were done 
 *        inconsistently.  The callback was being stored in the wrong place by 
 *        set_callback.
 *        
 *  4    mpeg      1.3         4/12/02 2:17:36 PM     Ray Mack        SCR(s) 
 *        3545 :
 *        Change satellite driver name from SATELLITE_110 to DEMOD_VALIANT
 *        
 *  3    mpeg      1.2         4/4/02 2:28:08 AM      Ian Mitchell    SCR(s): 
 *        3466 
 *        Defined the initialisation function for the terrestrial instance
 *        
 *  2    mpeg      1.1         4/3/02 9:51:34 AM      Billy Jackman   SCR(s) 
 *        3445 :
 *        Fixed a problem with enumerating modules during initialization.
 *        Modified the prototype for cnxt_demod_get_unit_type so that the 
 *        return
 *        parameter is the interface type, instead of a single member 
 *        structure.
 *        
 *  1    mpeg      1.0         3/25/02 4:16:34 PM     Billy Jackman   
 * $
 * 
 ****************************************************************************/

