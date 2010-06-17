/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                   Conexant Systems Inc. (c) 2003 - 2004                  */
/*                            Shanghai, CHINA                               */
/*                          All Rights Reserved                             */
/****************************************************************************/
/*
 * Filename:      DEMOD_DCF8722.C
 *
 * Description:   This file contains the module-level driver for the cable
 *                front-end interface for use with multi-instance demod.
 *
 * Author:        Steven Shen
 *
 ****************************************************************************/
/* $Header: DEMOD_DCF8722.C, 5, 5/26/04 3:12:04 AM, Steven Shen$
 * $Id: DEMOD_DCF8722.C,v 1.4, 2004-05-26 08:12:04Z, Steven Shen$
 ****************************************************************************/

/***************************/
/*       Header Files      */
/***************************/
#include "stbcfg.h"
#include "hwlib.h"
#include "kal.h"
#include "retcodes.h"
#include "demod_module_api.h"
#include "iic.h"
#if (defined(GX1001_V35_SERVICE_ENABLE) && (GX1001_V35_SERVICE_ENABLE ==YES))
#include "mw.h"
#include "confmgr.h"
#endif
/* to use the polling mode */
#define CABLE_INT_DISABLED
/* to disable the lock indicator */
#define CABLE_LKDT_DISABLED
/* header file of the cable dcf8722 can tuner driver */
#include "DRV_8722.h"
/* header file of the debug output function */
#include "DEBUGINF.h"
#if(MTG_m88dc2000==YES)
#define m88dc2000
#endif
#if(TUNER_LG==YES)
#define TDCH_G101F
#endif

/***************************/
/* Local Label Definitions */
/***************************/
static u_int8 m_get_statics =0;

/* Define the maximum number of cable front-end units in the system */
#define MAXIMUM_NUMBER_UNITS  (1)

/* Define the parameters of the tick */
#define NIM_TICK_NAME         "CF_K"
#define NIM_TICK_CALLBACK     (cable_tick_callback)
#if (defined(GX1001_V35_SERVICE_ENABLE) && (GX1001_V35_SERVICE_ENABLE ==YES))
#define NIM_CONNECT_TIMEOUT  (80)
#define NIM_MONITOR_PERIOD    (80)
#else
#define NIM_CONNECT_TIMEOUT   (100)
#define NIM_MONITOR_PERIOD    (100)
#endif
/* Define the parameters of the message queue */
#define NIM_MSGQ_NAME         "CF_Q"
#define NIM_MSGQ_MAXIMUM      (16)

/* Define the messages */
#define NIM_MSG_CONNECT       (0x00000001)
#define NIM_MSG_DISCONNECT    (0x00000002)
#define NIM_MSG_SCAN          (0x00000003)
#define NIM_MSG_TIMEOUT       (0x00000004)
#define NIM_MSG_INTERRUPT     (0x00000005)

/* Define the parameters of the task */
#define NIM_TASK_MAINFUNC     (cable_task_main)

/* Define the parameters of the semaphore */
#define NIM_SEM_NAME          "CF_S"

/* Define the states of cable front-end units in the system */
#define NIM_NO_HARDWARE       (0)
#define NIM_UNINITIALIZED     (1)
#define NIM_INITIALIZED       (2)
#define NIM_CONNECTING        (3)
#define NIM_CONNECTED         (4)
#define NIM_LOSE_LOCKED       (5)
#define NIM_DISCONNECTING     (6)
#define NIM_DISCONNECTED      (7)
#define NIM_SCANNING          (8)
#define NIM_FAILED            (9)


/***************************/
/*     Local Constants     */
/***************************/
/* The amount of time allowed for the chip to lock and stabilise in ms */
static const u_int32    LOCK_TIMEOUT = 1000;
/* The maximum frequency offset in Hz that is allowed */
static const int32      MAX_OFFSET = 150000;    /* 150 KHz */


/***************************/
/* Static Global Variables */
/***************************/
static sem_id_t         gNIMSemaphore = 0;
static tick_id_t        gNIMTimer = 0;
static queue_id_t       gNIMQueue = 0;
static task_id_t        gNIMTask = 0;

static u_int32          guLocalModule = -1;
static u_int32          guInitialized = 0;
static u_int32          guLocalUnitCount = 0; /* ONLY ONE UNIT IS SUPPORTED */

static u_int8           guTimeoutFlag = 0;

#ifndef CABLE_INT_DISABLED
static PFNISR           pfnNIMPreviousIsr = 0;
static u_int32          guNIMInterrupt = 0;
static u_int8           guInterruptFlag = 0;
#endif /* CABLE_INT_DISABLED */

static int32            gTimeoutCnt=0;
static int32            gWaitingCnt=0;
static int32            gRetryCnt=0;

/* workaround the bug of 32-QAM in the auto-QAM detection mode */
static int32            gAutoQAMMode=0;
#if (defined m88dc2000)
static int32            gQAMAuto=0;
#endif

static u_int8           guNewLOCState[MAXIMUM_NUMBER_UNITS] =
{
   NIM_NO_HARDWARE
};

static u_int8           guOldLOCState[MAXIMUM_NUMBER_UNITS] = 
{
   NIM_NO_HARDWARE
};

static u_int8           guDemodAddr[MAXIMUM_NUMBER_UNITS] =
{
   #if (CABLE_TUNER_TYPE == CTT_THOMSON_TUNER)
   #if (defined(GX1001_V35_SERVICE_ENABLE) && (GX1001_V35_SERVICE_ENABLE ==YES))
      0x18
   #else
      0x38
   #endif
   #else
      #error "ERROR! NO VALID TUNER ADDRESSS DEFINED"
   #endif
};

static MODULE_STATUS_FUNCTION    *gpfnCallbacks[MAXIMUM_NUMBER_UNITS];

static TUNING_SPEC      gLocalTuning[MAXIMUM_NUMBER_UNITS];

static u_int32          uTuningFreq;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*           ===================================================             */
/*           = ISR, TIMER, MAIN TASK OF CABLE FRONT-END DRIVER =             */
/*           ===================================================             */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void cable_tick_callback (tick_id_t hTick, void *pUser);
void cable_task_main (void *parm);

#ifndef CABLE_INT_DISABLED

int  cable_isr_handler (u_int32 uINTID, bool bIsFIQ, void *pPrevious);

#if (defined(GX1001_V35_SERVICE_ENABLE) && (GX1001_V35_SERVICE_ENABLE ==YES))
extern GX_STATE GX_Read_ALL_OK(void);
extern int mwrt_send_message(u_int32 message_id, u_int32 param1, u_int32 param2,u_int32 param3);
#endif
/*****************************************************************************/
/*  FUNCTION:    cable_isr_handler                                           */
/*                                                                           */
/*  PARAMETERS:  uIntID   - the ID of the interrupt being handled.           */
/*               bFIQ     - flag indicating FIQ or not.                      */
/*               pfnChain - function pointer to be filled in if interrupt    */
/*                          chaining is required.                            */
/*                                                                           */
/*  DESCRIPTION: This function handles interrupts from the cable front-end.  */
/*                                                                           */
/*  RETURNS:     RC_ISR_HANDLED - Interrupt fully handled by this routine.   */
/*               RC_ISR_NOTHANDLED - Interrupt not handled by this function. */
/*                        (KAL should chain to the function whose pointer is */
/*                         stored in pfnChain).                              */
/*                                                                           */
/*  CONTEXT:     Will be called from an interrupt context.                   */
/*                                                                           */
/*****************************************************************************/
int cable_isr_handler (u_int32 uIntID, bool bFIQ, void *pfnChain)
{
   u_int32     uMsg[4];
   int32       iReturnCode;
   
   /*
    * Make sure that there will be no more interrupts from the cabel front-end
    * until this one is fully handled (cleared).
    */
   int_disable (guNIMInterrupt);
   
   /* send a control message to the demod task to signal the interrupt. */
   uMsg[0] = 0;
   uMsg[1] = 0;
   uMsg[2] = 0;
   uMsg[3] = NIM_MSG_INTERRUPT;
   iReturnCode = qu_send (gNIMQueue, uMsg);
   if (RC_OK != iReturnCode)
   {
      guInterruptFlag = 1;
   }
   
   /* clear the interrupt status */
   CLEAR_GPIO_INT_BANK (CABLE_INT_GPIO_BANK, CABLE_INT_GPIO_BIT);
   
   /* re-enable interrupts. */
   int_enable (guNIMInterrupt);
   
   return (RC_ISR_HANDLED);
} /* cable_isr_handler */

#endif /* CABLE_INT_DISABLED */

extern uint8 scan_flag ;
/*****************************************************************************/
/*  FUNCTION:    cable_tick_callback                                         */
/*                                                                           */
/*  PARAMETERS:  hTick - the handle of the tick timer that expired.          */
/*               pUser - the user-specified parameter for the timeout.       */
/*                                                                           */
/*  DESCRIPTION: This function handles tick timer callbacks.                 */
/*                                                                           */
/*  RETURNS:     Nothing.                                                    */
/*                                                                           */
/*  CONTEXT:     Will be called from an interrupt context.                   */
/*                                                                           */
/*****************************************************************************/
void cable_tick_callback (tick_id_t hTick, void *pUser)
{
   u_int32     uUnit = (u_int32)pUser;
   u_int32     uMsg[4];
   int32       iReturnCode;
   
   /* Send a control message to the demod task to signal the timeout. */
   uMsg[0] = uUnit;
   uMsg[1] = 0;
   uMsg[2] = 0;
   uMsg[3] = (u_int32)(NIM_MSG_TIMEOUT);
   #if (defined(GX1001_V35_SERVICE_ENABLE) && (GX1001_V35_SERVICE_ENABLE ==YES))
   if (scan_flag==0)
   #endif
   iReturnCode = qu_send (gNIMQueue, uMsg);
   if (RC_OK != iReturnCode)
   {
      guTimeoutFlag = (1 << (uUnit));
   }
   
   return;
}
/////for siglost eric
#ifndef BOOL
typedef unsigned char	BOOL;	/* 1 byte */
#endif
//extern void desk_display_nosig_banner(BOOL enable);

#if (defined(GX1001_V35_SERVICE_ENABLE) && (GX1001_V35_SERVICE_ENABLE ==YES))
/*****************************************************************************/
/*  FUNCTION:    cable_task_main                                             */
/*                                                                           */
/*  PARAMETERS:  pParam - (unused)                                           */
/*                                                                           */
/*  DESCRIPTION: This function implements the acquisition task for the cable */
/*               front-end driver.  The task runs forever, checking an input */
/*               message queue for work to do.                               */
/*                                                                           */
/*  RETURNS:     Never.                                                      */
/*                                                                           */
/*  CONTEXT:     Will be run in task context.                                */
/*                                                                           */
/*****************************************************************************/
void cable_task_main (void *pParam)
{
   int32                   iReturnCode, rc;
   bool                    bLockState;
   u_int32                 uRxMsg[4], uUnit, uMessage;
   DEMOD_CALLBACK_DATA     CallbackData;
   sabine_config_data *pConfig;
   extern void GX1001_set_to_cable(sabine_config_data *pConfig);

   extern u_int8 scan_flag;
   debug_out(TL_FUNC, "CABLE TASK: Started the task\n");
   
   while (1)
   {
      /* check for new message, process the message received. */
      iReturnCode = qu_receive (gNIMQueue, 9000, uRxMsg);
      /* if the message receive timed out, monitor current state. */
      if ( iReturnCode == RC_KAL_TIMEOUT )
      {
         debug_out (TL_VERBOSE, "CABLE TASK: Timeout on message queue\n");
         /* do nothing */
      }
      else if ( iReturnCode == RC_OK )
      {
         /* process the incoming message. */
         uUnit = uRxMsg[0];
         uMessage = uRxMsg[3];
         switch (uMessage)
         {
            case NIM_MSG_INTERRUPT:
               #if (defined CABLE_INT_DISABLED)
               /* if use the POLLING method, there is no NIM_MSG_INTERRUPT message. */
               #else /* CABLE_INT_DISABLED is not defined, use the INTERRUPT method */
               debug_out (TL_INFO, "CABLE TASK: received NIM_MSG_INTERRUPT message\n");
               /* check the interrupt type */
		
               /* get the current lock status of the cable front end */	 
               #endif /* CABLE_INT_DISABLED */
               break;
            
            case NIM_MSG_TIMEOUT:
		//    trace("NIM_MSG_TIMEOUT\n");
		    debug_out (TL_INFO, "CABLE TASK: received NIM_MSG_TIMEOUT message\n");
	           //      #if (defined CABLE_INT_DISABLED)
               /* get the current lock status of the cable front end */
      //         dcf872x_get_lockstatus (uUnit, &bLockState);
		   if((GX_Read_ALL_OK())==1)
		    {
		        /* stop the monitor timer. */
 	               rc = tick_stop (gNIMTimer);
 	               debug_out (TL_INFO, "CABLE TASK: Lost signal\n");
			
	       	            	                       
 	               #ifndef CABLE_LKDT_DISABLED
 	               /* lost the signal lock, turn off the lock indicator */
 	               CABLE_LKDT_TURN_OFF (CABLE_LKDT_GPIO_BANK, CABLE_LKDT_GPIO_BIT);
 	               #endif
 	                        
                      /* call the callback function to indicate LOSE_LOCKED. */
			//trace("...ximei...scan_flag=%d\n",scan_flag);
			if(scan_flag==0)	   /*	正在搜索中时不能发送这个消息，不然NIT搜索无法继续进行*/
	                      mwrt_send_message(MWM_NIM_LOCKED,0,0,0);
             //           trace("MWM_NIM_LOCKED\n");	                     
                     if (gAutoQAMMode == 1)
                        {
                           gLocalTuning[uUnit].tune.nim_cable_tune.modulation = MOD_QAMAUTO;
                        } /* endif (gAutoQAMMode) */
                        
                        /* try to connect the lost signal */
                    //    if (DRV_OK == dcf872x_connect(uUnit, (TUNING_SPEC *)(&gLocalTuning[uUnit]), TRUE))
                        {
                           /* clear the waiting counter and the retry counter */
                           gWaitingCnt = 0;
                           gRetryCnt   = 0;
                        }
                     
                      /* we need start a timer to monitor the connection. */
                     rc = tick_set (gNIMTimer, (u_int32)(NIM_CONNECT_TIMEOUT), FALSE);
                     rc = tick_start (gNIMTimer);
					  
	       //        desk_display_nosig_banner(FALSE);//for siglost
		   }
		   else
		   { 	
		      	gWaitingCnt++;		                          
			/*这里可以加上频偏检测*/
              	if (gWaitingCnt == 1)  /* 100 ms */
                     {
                          if((GX_Read_ALL_OK())==1)
                          {
                    //         debug_out (TL_INFO, "CABLE TASK: DEMOD+ offset\n");
                          } 
			     else
			     {
				  pConfig = config_lock_data();
				  GX1001_set_to_cable( pConfig);
				  config_unlock_data(pConfig);
			     	} 	
                     }
                     if (gWaitingCnt == 2)  /* 200 ms */
                     {
                          if((GX_Read_ALL_OK())==1)
                          {
                    //         debug_out (TL_INFO, "CABLE TASK: DEMOD- offset\n");
                          } 
			     else
			     {
				  pConfig = config_lock_data();
				  GX1001_set_to_cable( pConfig);
				  config_unlock_data(pConfig);
			     	} 	
                     }
			if (gWaitingCnt == 3)  /* 300 ms */
                     {                 
                         gWaitingCnt = 0;
                         
                         /* wakeup the sleeping task */
                         task_wakeup(gNIMTask);
	  		    if(scan_flag==0)	 
	  			    mwrt_send_message(MWM_NIM_FAILED,0,0,0);
	//		    trace("MWM_NIM_FAILED\n");
    			    debug_out (TL_INFO, "CABLE TASK: DEMOD_FAILED\n");
  		       }
                  }
         	break;       
            
            case NIM_MSG_CONNECT:
	//	 trace("NIM_MSG_CONNECT\n");
               debug_out (TL_INFO, "CABLE TASK: received NIM_MSG_CONNECT message\n");
               /* update the state */
               guOldLOCState[uUnit] = guNewLOCState[uUnit];
               guNewLOCState[uUnit] = NIM_CONNECTING;
               #ifndef CABLE_LKDT_DISABLED
               /* the new signal is to be tuned, turn off the lock indicator */
               CABLE_LKDT_TURN_OFF (CABLE_LKDT_GPIO_BANK, CABLE_LKDT_GPIO_BIT);
               #endif
               /* connect to the required signal. */
       
               /* start a timer to monitor the connection timeout. */
               rc = tick_set (gNIMTimer, (u_int32)(NIM_CONNECT_TIMEOUT), FALSE);
               rc = tick_start (gNIMTimer);
               /* release the semaphore to have the connection function return */
               //sem_put(gNIMSemaphore);
               break;
            
            case NIM_MSG_DISCONNECT:
               debug_out (TL_INFO, "CABLE TASK: received NIM_MSG_DISCONNECT message\n");
               /* do something to disconnect the current signal. */
               /* after disconnection, update the state. */
               guOldLOCState[uUnit] = guNewLOCState[uUnit];
               guNewLOCState[uUnit] = NIM_DISCONNECTED;
               #ifndef CABLE_LKDT_DISABLED
               /* disconnect with the signal, turn off the lock indicator */
               CABLE_LKDT_TURN_OFF (CABLE_LKDT_GPIO_BANK, CABLE_LKDT_GPIO_BIT);
               #endif
               /* call the callback function to indicate DISCONNECTED. */
               if (gpfnCallbacks[uUnit])
               {
                  CallbackData.type      = DEMOD_DISCONNECT_STATUS;
                  CallbackData.parm.type = DEMOD_DISCONNECTED;
                  gpfnCallbacks[uUnit] (guLocalModule, uUnit, DEMOD_DISCONNECT_STATUS, &CallbackData);
               }
		//	   desk_display_nosig_banner(TRUE);
               break;
            
            case NIM_MSG_SCAN:
               debug_out (TL_INFO, "CABLE TASK: received NIM_MSG_SCAN message\n");
               break;
            
            default:
               debug_out (TL_ERROR, "CABLE TASK: Unknown message 0x%x for unit %d\n", uMessage, uUnit);
               break;
         
         } /* endswitch (uMessage) */
      } /* end of processing messages */
      else
      {
         debug_out (TL_ERROR, "CABLE TASK: error code 0x%x when receiving message\n", iReturnCode);
      } /* end of receiving messages */
   } /* endwhile (1) */
}
#else
void cable_task_main (void *pParam)
{
   int32                   iReturnCode, rc;
   bool                    bLockState;
   u_int32                 uRxMsg[4], uUnit, uMessage;
   DEMOD_CALLBACK_DATA     CallbackData;
   
   debug_out(TL_FUNC, "CABLE TASK: Started the task\n");
   
   while (1)
   {
      /* check for new message, process the message received. */
      iReturnCode = qu_receive (gNIMQueue, 9000, uRxMsg);
      /* if the message receive timed out, monitor current state. */
      if ( iReturnCode == RC_KAL_TIMEOUT )
      {
         debug_out (TL_VERBOSE, "CABLE TASK: Timeout on message queue\n");
         /* do nothing */
      }
      else if ( iReturnCode == RC_OK )
      {
         /* process the incoming message. */
         uUnit = uRxMsg[0];
         uMessage = uRxMsg[3];
         switch (uMessage)
         {
            case NIM_MSG_INTERRUPT:
               #if (defined CABLE_INT_DISABLED)
               /* if use the POLLING method, there is no NIM_MSG_INTERRUPT message. */
               #else /* CABLE_INT_DISABLED is not defined, use the INTERRUPT method */
               debug_out (TL_INFO, "CABLE TASK: received NIM_MSG_INTERRUPT message\n");
               /* check the interrupt type */
               /* get the current lock status of the cable front end */
               dcf872x_get_lockstatus (uUnit, &bLockState);
               #endif /* CABLE_INT_DISABLED */
               break;
            
            case NIM_MSG_TIMEOUT:
               // debug_out (TL_INFO, "CABLE TASK: received NIM_MSG_TIMEOUT message\n");
               #if (defined CABLE_INT_DISABLED)
               /* get the current lock status of the cable front end */
               dcf872x_get_lockstatus (uUnit, &bLockState);
               // debug_out (TL_INFO, "CABLE TASK: current bLockState=0x%08X\n", bLockState);
               /* accroding to the current state, to do some processing. */
               switch (guNewLOCState[uUnit])
               {
                  case NIM_CONNECTING:
                  case NIM_FAILED:
                     if (TRUE == bLockState)
                     {
                        /* stop the connection timeout timer. */
                        rc = tick_stop (gNIMTimer);
                        debug_out (TL_INFO, "CABLE TASK: Locked\n");
		//	    trace(" Demod lock   gWaitingCnt = ........%d\n",gWaitingCnt+1);
                        /* update the state */
                        guOldLOCState[uUnit] = guNewLOCState[uUnit];
                        guNewLOCState[uUnit] = NIM_CONNECTED;
                        
                        #ifndef CABLE_LKDT_DISABLED
                        /* The new signal has been locked, turn on the lock indicator */
                        CABLE_LKDT_TURN_ON (CABLE_LKDT_GPIO_BANK, CABLE_LKDT_GPIO_BIT);
                        #endif
                        
                        /* wakeup the sleeping task */
                        task_wakeup(gNIMTask);
                        
                        /* call the callback function to indicate LOCKED. */
                        if (gpfnCallbacks[uUnit])
                        {
                           CallbackData.type      = DEMOD_CONNECT_STATUS;
                           CallbackData.parm.type = DEMOD_CONNECTED;
                           CallbackData.parm.tune = gLocalTuning[uUnit];
                           gpfnCallbacks[uUnit] (guLocalModule, uUnit, DEMOD_CONNECT_STATUS, &CallbackData);
                        }
                        /* clear and start all block counters. */
                        gTimeoutCnt = 0;
                        dcf872x_init_statics (uUnit);
                        /* we need start a timer to monitor the connection. */
                        rc = tick_set (gNIMTimer, (u_int32)(NIM_MONITOR_PERIOD), FALSE);
                        rc = tick_start (gNIMTimer);
                     }
                     else /* FALSE == bLockState */
                     {
                        gWaitingCnt++;
	//		    trace(" gWaitingCnt = ........%d\n",gWaitingCnt);
			   #if (defined m88dc2000)
			   if(0/*gAutoQAMMode == 1*/)
			   {
	                        if (guNewLOCState[uUnit] == NIM_CONNECTING||guNewLOCState[uUnit] == NIM_FAILED)
	                        {
	                           if (gWaitingCnt >= 10 && gWaitingCnt <50)  /* 1000 ms */
	                           {
	                               if ((gWaitingCnt % 10) == 0)
	                               {
		                                gQAMAuto = (gQAMAuto + 1) % 5;
		                                gLocalTuning[uUnit].tune.nim_cable_tune.modulation = gQAMAuto;
		                                gLocalTuning[uUnit].tune.nim_cable_tune.frequency= uTuningFreq;//the offset should be according to the spec
		                                debug_out (TL_INFO, "CABLE TASK: +0\n");
		                                if (DRV_OK == dcf872x_connect(uUnit, (TUNING_SPEC *)(&gLocalTuning[uUnit]), TRUE))
		                                {
		                                   debug_out (TL_INFO, "CABLE TASK: DEMOD\n");
		                                } 
	                                }
	                           }
	                           else if (gWaitingCnt >= 50 && gWaitingCnt <100)  /* 2000 ms */
	                           {
	                               if ((gWaitingCnt % 10) == 0)
	                               {
		                                gQAMAuto = (gQAMAuto + 1) % 5;
		                                gLocalTuning[uUnit].tune.nim_cable_tune.modulation = gQAMAuto;
		                                gLocalTuning[uUnit].tune.nim_cable_tune.frequency=uTuningFreq + 500000;//the offset should be according to the spec
		                                debug_out (TL_INFO, "CABLE TASK: +500000\n");
		                                if (DRV_OK == dcf872x_connect(uUnit, (TUNING_SPEC *)(&gLocalTuning[uUnit]), TRUE))
		                                {
		                                   debug_out (TL_INFO, "CABLE TASK: DEMOD+ offset\n");
		                                } 
	                                }
	                           }                           
	                           else if (gWaitingCnt >= 100 && gWaitingCnt <150)  /* 3000 ms */
	                           {
	                               if ((gWaitingCnt % 10) == 0)
	                               {
		                                gQAMAuto = (gQAMAuto + 1) % 5;
		                                gLocalTuning[uUnit].tune.nim_cable_tune.modulation = gQAMAuto;
		                                gLocalTuning[uUnit].tune.nim_cable_tune.frequency=uTuningFreq - 500000;//the offset should be according to the spec
		                                debug_out (TL_INFO, "CABLE TASK: -500000\n");
		                                if (DRV_OK == dcf872x_connect(uUnit, (TUNING_SPEC *)(&gLocalTuning[uUnit]), TRUE))
		                                {
		                                   debug_out (TL_INFO, "CABLE TASK: DEMOD- offset\n");
		                                } 
	                                }
	                           }                           
	                          
	                           else if (gWaitingCnt == 150)  /* 4000 ms */
	                           {
	                              /* update the state */
	                              guOldLOCState[uUnit] = guNewLOCState[uUnit];
	                              guNewLOCState[uUnit] = NIM_FAILED;
	                              /* wakeup the sleeping task */
	                              task_wakeup(gNIMTask);
	                              /* if the front-end fails to acquire lock within N ms,
	                               * call the callback function to indicate DEMOD_FAILED.
	                               */
	                              if (gpfnCallbacks[uUnit])
	                              {
	                                 CallbackData.type      = DEMOD_CONNECT_STATUS;
	                                 CallbackData.parm.type = DEMOD_FAILED;
	                                 CallbackData.parm.tune = gLocalTuning[uUnit];
	                                 gpfnCallbacks[uUnit] (guLocalModule, uUnit, DEMOD_CONNECT_STATUS, &CallbackData);
	                              }
	                              debug_out (TL_INFO, "CABLE TASK: DEMOD_FAILED\n");
	                           }   
	                           
	                        }
	                        
	                        if (gWaitingCnt >= 150) /* >4000 ms */
	                        {
	                           /* if the front end hasn't been SYNC for about M ms,
	                            * we need try to connect the signal again.
	                            */
	                          // gWaitingCnt = 0;   /* for the next M ms interval */
	                           
	                           /* if the auto-detect spectrum inversion is required,
	                            * we also need change the spectrum inversion option before re-connection.
	                            */
	                           if (1 == gLocalTuning[uUnit].tune.nim_cable_tune.auto_spectrum  && gWaitingCnt == 150 )
	                           {
	                              switch (gLocalTuning[uUnit].tune.nim_cable_tune.spectrum)
	                              {
	                                 case SPECTRUM_NORMAL:
	                                    gLocalTuning[uUnit].tune.nim_cable_tune.spectrum = SPECTRUM_INVERTED;
	                                    break;
	                                 case SPECTRUM_INVERTED:
	                                    gLocalTuning[uUnit].tune.nim_cable_tune.spectrum = SPECTRUM_NORMAL;
	                                    break;
	                              }
	                           }
	                           
	                           if(gWaitingCnt ==150)
	                           {
	                              gQAMAuto = (gQAMAuto + 1) % 5;
	                              gLocalTuning[uUnit].tune.nim_cable_tune.modulation = gQAMAuto;
	                              gLocalTuning[uUnit].tune.nim_cable_tune.frequency =uTuningFreq;//the offset should be according to the spec
	                              debug_out (TL_INFO, "CABLE TASK: 000000\n");
	                              if (DRV_OK == dcf872x_connect(uUnit, (TUNING_SPEC *)(&gLocalTuning[uUnit]), TRUE))
	                              {
	                                 debug_out (TL_INFO, "CABLE TASK: Frequnce\n");
	                              }
	                           }
	                           gWaitingCnt = 0;   /* for the next M ms interval */
	                           gLocalTuning[uUnit].tune.nim_cable_tune.frequency = uTuningFreq;
	                           gRetryCnt++;
	                           if (gRetryCnt == 2) /* 3000 ms */
	                           {
	                              /* if the front-end fails to acquire lock after 3 retries,
	                               * call the callback function to indicate DEMOD_TIMEOUT.
	                               */
	                              if (gpfnCallbacks[uUnit])
	                              {
	                                 CallbackData.type      = DEMOD_CONNECT_STATUS;
	                                 CallbackData.parm.type = DEMOD_TIMEOUT;
	                                 CallbackData.parm.tune = gLocalTuning[uUnit];
	                                 gpfnCallbacks[uUnit] (guLocalModule, uUnit, DEMOD_CONNECT_STATUS, &CallbackData);
	                              }
	                              gRetryCnt = 0;
	                              debug_out (TL_INFO, "CABLE TASK: DEMOD_TIMEOUT\n");
	                              
	                              /* IF TIMEOUT IN AUTO-QAM DETECTION MODE, TRY 32-QAM MANUALLY */
	                              if (gAutoQAMMode == 1)
	                              {
	                                 if (MOD_QAMAUTO == gLocalTuning[uUnit].tune.nim_cable_tune.modulation)
	                                 {
	                                    gLocalTuning[uUnit].tune.nim_cable_tune.modulation = MOD_QAM32;
	                                 }
	                                 else
	                                 {
	                                    gLocalTuning[uUnit].tune.nim_cable_tune.modulation = MOD_QAMAUTO;
	                                 }
	                              } /* endif (gAutoQAMMode) */
	                           } /* endif (gRetryCnt) */
	                        }
			   }
			   else
			   {
	                        if (guNewLOCState[uUnit] == NIM_CONNECTING||guNewLOCState[uUnit] == NIM_FAILED)
	                        {
	                           if (1 == gLocalTuning[uUnit].tune.nim_cable_tune.auto_spectrum  && gWaitingCnt == 4 )
	                           {
	                              switch (gLocalTuning[uUnit].tune.nim_cable_tune.spectrum)
	                              {
	                                 case SPECTRUM_NORMAL:
	                                    gLocalTuning[uUnit].tune.nim_cable_tune.spectrum = SPECTRUM_INVERTED;
	                                    break;
	                                 case SPECTRUM_INVERTED:
	                                    gLocalTuning[uUnit].tune.nim_cable_tune.spectrum = SPECTRUM_NORMAL;
	                                    break;
	                              }
	                           }
	                           if (gWaitingCnt == 4)  /* 500 ms */
	                           {
	                                gLocalTuning[uUnit].tune.nim_cable_tune.frequency=uTuningFreq;//the offset should be according to the spec
	                                debug_out (TL_INFO, "CABLE TASK: +500000\n");
	                                if (DRV_OK == dcf872x_connect(uUnit, (TUNING_SPEC *)(&gLocalTuning[uUnit]), TRUE))
	                                {
	                                   debug_out (TL_INFO, "CABLE TASK: DEMOD+ offset\n");
	                                } 
	                           }                           

	                           if (1 == gLocalTuning[uUnit].tune.nim_cable_tune.auto_spectrum  && gWaitingCnt == 8 )
	                           {
	                              switch (gLocalTuning[uUnit].tune.nim_cable_tune.spectrum)
	                              {
	                                 case SPECTRUM_NORMAL:
	                                    gLocalTuning[uUnit].tune.nim_cable_tune.spectrum = SPECTRUM_INVERTED;
	                                    break;
	                                 case SPECTRUM_INVERTED:
	                                    gLocalTuning[uUnit].tune.nim_cable_tune.spectrum = SPECTRUM_NORMAL;
	                                    break;
	                              }
	                           }
	                           if (gWaitingCnt == 8)  /* 500 ms */
	                           {
	                                gLocalTuning[uUnit].tune.nim_cable_tune.frequency=uTuningFreq + 500000;//the offset should be according to the spec
	                                debug_out (TL_INFO, "CABLE TASK: +500000\n");
	                                if (DRV_OK == dcf872x_connect(uUnit, (TUNING_SPEC *)(&gLocalTuning[uUnit]), TRUE))
	                                {
	                                   debug_out (TL_INFO, "CABLE TASK: DEMOD+ offset\n");
	                                } 
	                           }                    
							   
	                           if (1 == gLocalTuning[uUnit].tune.nim_cable_tune.auto_spectrum  && gWaitingCnt == 12 )
	                           {
	                              switch (gLocalTuning[uUnit].tune.nim_cable_tune.spectrum)
	                              {
	                                 case SPECTRUM_NORMAL:
	                                    gLocalTuning[uUnit].tune.nim_cable_tune.spectrum = SPECTRUM_INVERTED;
	                                    break;
	                                 case SPECTRUM_INVERTED:
	                                    gLocalTuning[uUnit].tune.nim_cable_tune.spectrum = SPECTRUM_NORMAL;
	                                    break;
	                              }
	                           }
	                           if (gWaitingCnt == 12)  /* 500 ms */
	                           {
	                                gLocalTuning[uUnit].tune.nim_cable_tune.frequency=uTuningFreq + 500000;//the offset should be according to the spec
	                                debug_out (TL_INFO, "CABLE TASK: +500000\n");
	                                if (DRV_OK == dcf872x_connect(uUnit, (TUNING_SPEC *)(&gLocalTuning[uUnit]), TRUE))
	                                {
	                                   debug_out (TL_INFO, "CABLE TASK: DEMOD+ offset\n");
	                                } 
	                           }                    

	                           if (1 == gLocalTuning[uUnit].tune.nim_cable_tune.auto_spectrum  && gWaitingCnt == 16 )
	                           {
	                              switch (gLocalTuning[uUnit].tune.nim_cable_tune.spectrum)
	                              {
	                                 case SPECTRUM_NORMAL:
	                                    gLocalTuning[uUnit].tune.nim_cable_tune.spectrum = SPECTRUM_INVERTED;
	                                    break;
	                                 case SPECTRUM_INVERTED:
	                                    gLocalTuning[uUnit].tune.nim_cable_tune.spectrum = SPECTRUM_NORMAL;
	                                    break;
	                              }
	                           }
	                           if (gWaitingCnt == 16)  /* 500 ms */
	                           {
	                                gLocalTuning[uUnit].tune.nim_cable_tune.frequency=uTuningFreq - 500000;//the offset should be according to the spec
	                                debug_out (TL_INFO, "CABLE TASK: +500000\n");
	                                if (DRV_OK == dcf872x_connect(uUnit, (TUNING_SPEC *)(&gLocalTuning[uUnit]), TRUE))
	                                {
	                                   debug_out (TL_INFO, "CABLE TASK: DEMOD+ offset\n");
	                                } 
	                           }                    


	                           if (1 == gLocalTuning[uUnit].tune.nim_cable_tune.auto_spectrum  && gWaitingCnt == 20 )
	                           {
	                              switch (gLocalTuning[uUnit].tune.nim_cable_tune.spectrum)
	                              {
	                                 case SPECTRUM_NORMAL:
	                                    gLocalTuning[uUnit].tune.nim_cable_tune.spectrum = SPECTRUM_INVERTED;
	                                    break;
	                                 case SPECTRUM_INVERTED:
	                                    gLocalTuning[uUnit].tune.nim_cable_tune.spectrum = SPECTRUM_NORMAL;
	                                    break;
	                              }
	                           }
	                           if (gWaitingCnt == 20)  /* 500 ms */
	                           {
	                                gLocalTuning[uUnit].tune.nim_cable_tune.frequency=uTuningFreq - 500000;//the offset should be according to the spec
	                                debug_out (TL_INFO, "CABLE TASK: +500000\n");
	                                if (DRV_OK == dcf872x_connect(uUnit, (TUNING_SPEC *)(&gLocalTuning[uUnit]), TRUE))
	                                {
	                                   debug_out (TL_INFO, "CABLE TASK: DEMOD+ offset\n");
	                                } 
	                           }                    

	                          
	                           if (gWaitingCnt == 24)  /* 1500 ms */
	                           {
	                              /* update the state */
	                              guOldLOCState[uUnit] = guNewLOCState[uUnit];
	                              guNewLOCState[uUnit] = NIM_FAILED;
	                              /* wakeup the sleeping task */
	                              task_wakeup(gNIMTask);
	                              /* if the front-end fails to acquire lock within N ms,
	                               * call the callback function to indicate DEMOD_FAILED.
	                               */
	                              if (gpfnCallbacks[uUnit])
	                              {
	                                 CallbackData.type      = DEMOD_CONNECT_STATUS;
	                                 CallbackData.parm.type = DEMOD_FAILED;
	                                 CallbackData.parm.tune = gLocalTuning[uUnit];
	                                 gpfnCallbacks[uUnit] (guLocalModule, uUnit, DEMOD_CONNECT_STATUS, &CallbackData);
	                              }
	                              debug_out (TL_INFO, "CABLE TASK: DEMOD_FAILED\n");
	                           }   
	                           
	                        }
	                        
	                        if (gWaitingCnt >= 24) /* >1500 ms */
	                        {
	                           /* if the front end hasn't been SYNC for about M ms,
	                            * we need try to connect the signal again.
	                            */
	                          // gWaitingCnt = 0;   /* for the next M ms interval */
	                           
	                           /* if the auto-detect spectrum inversion is required,
	                            * we also need change the spectrum inversion option before re-connection.
	                            */
	                           if (1 == gLocalTuning[uUnit].tune.nim_cable_tune.auto_spectrum  && gWaitingCnt == 24 )
	                           {
	                              switch (gLocalTuning[uUnit].tune.nim_cable_tune.spectrum)
	                              {
	                                 case SPECTRUM_NORMAL:
	                                    gLocalTuning[uUnit].tune.nim_cable_tune.spectrum = SPECTRUM_INVERTED;
	                                    break;
	                                 case SPECTRUM_INVERTED:
	                                    gLocalTuning[uUnit].tune.nim_cable_tune.spectrum = SPECTRUM_NORMAL;
	                                    break;
	                              }
	                           }
	                           
	                           if(gWaitingCnt ==24)
	                           {
	                              gLocalTuning[uUnit].tune.nim_cable_tune.frequency =uTuningFreq;//the offset should be according to the spec
	                              debug_out (TL_INFO, "CABLE TASK: 000000\n");
	                              if (DRV_OK == dcf872x_connect(uUnit, (TUNING_SPEC *)(&gLocalTuning[uUnit]), TRUE))
	                              {
	                                 debug_out (TL_INFO, "CABLE TASK: Frequnce\n");
	                              }
	                           }
	                           gWaitingCnt = 0;   /* for the next M ms interval */
	                           gLocalTuning[uUnit].tune.nim_cable_tune.frequency = uTuningFreq;
	                           gRetryCnt++;
	                           if (gRetryCnt == 2) /* 3000 ms */
	                           {
	                              /* if the front-end fails to acquire lock after 3 retries,
	                               * call the callback function to indicate DEMOD_TIMEOUT.
	                               */
	                              if (gpfnCallbacks[uUnit])
	                              {
	                                 CallbackData.type      = DEMOD_CONNECT_STATUS;
	                                 CallbackData.parm.type = DEMOD_TIMEOUT;
	                                 CallbackData.parm.tune = gLocalTuning[uUnit];
	                                 gpfnCallbacks[uUnit] (guLocalModule, uUnit, DEMOD_CONNECT_STATUS, &CallbackData);
	                              }
	                              gRetryCnt = 0;
	                              debug_out (TL_INFO, "CABLE TASK: DEMOD_TIMEOUT\n");
	                              
	                              /* IF TIMEOUT IN AUTO-QAM DETECTION MODE, TRY 32-QAM MANUALLY */
	                              if (gAutoQAMMode == 1)
	                              {
	                                 if (MOD_QAMAUTO == gLocalTuning[uUnit].tune.nim_cable_tune.modulation)
	                                 {
	                                    gLocalTuning[uUnit].tune.nim_cable_tune.modulation = MOD_QAM32;
	                                 }
	                                 else
	                                 {
	                                    gLocalTuning[uUnit].tune.nim_cable_tune.modulation = MOD_QAMAUTO;
	                                 }
	                              } /* endif (gAutoQAMMode) */
	                           } /* endif (gRetryCnt) */
	                        }
			   	}						
			   #else
                        if (guNewLOCState[uUnit] == NIM_CONNECTING||guNewLOCState[uUnit] == NIM_FAILED)
                        {
                           if (gWaitingCnt == 2)  /* 200 ms */
                           {
                                gLocalTuning[uUnit].tune.nim_cable_tune.frequency= uTuningFreq + 250000;//the offset should be according to the spec
                                debug_out (TL_INFO, "CABLE TASK: +150000\n");
                                if (DRV_OK == dcf872x_connect(uUnit, (TUNING_SPEC *)(&gLocalTuning[uUnit]), TRUE))
                                {
                                   debug_out (TL_INFO, "CABLE TASK: DEMOD+ offset\n");
                                } 
                           }
                           if (gWaitingCnt == 4)  /* 400 ms */
                           {
                                gLocalTuning[uUnit].tune.nim_cable_tune.frequency=uTuningFreq - 500000;//the offset should be according to the spec
                                debug_out (TL_INFO, "CABLE TASK: -150000\n");
                                if (DRV_OK == dcf872x_connect(uUnit, (TUNING_SPEC *)(&gLocalTuning[uUnit]), TRUE))
                                {
                                   debug_out (TL_INFO, "CABLE TASK: DEMOD- offset\n");
                                } 
                           }                           
                          
                           if (gWaitingCnt == 6)  /* 600 ms */
                           {
                              /* update the state */
                              guOldLOCState[uUnit] = guNewLOCState[uUnit];
                              guNewLOCState[uUnit] = NIM_FAILED;
                              /* wakeup the sleeping task */
                              task_wakeup(gNIMTask);
                              /* if the front-end fails to acquire lock within N ms,
                               * call the callback function to indicate DEMOD_FAILED.
                               */
                              if (gpfnCallbacks[uUnit])
                              {
                                 CallbackData.type      = DEMOD_CONNECT_STATUS;
                                 CallbackData.parm.type = DEMOD_FAILED;
                                 CallbackData.parm.tune = gLocalTuning[uUnit];
                                 gpfnCallbacks[uUnit] (guLocalModule, uUnit, DEMOD_CONNECT_STATUS, &CallbackData);
                              }
                              debug_out (TL_INFO, "CABLE TASK: DEMOD_FAILED\n");
                           }   
                           
                        }
                        
                        if (gWaitingCnt >= 6) /* >600 ms */
                        {
                           /* if the front end hasn't been SYNC for about M ms,
                            * we need try to connect the signal again.
                            */
                          // gWaitingCnt = 0;   /* for the next M ms interval */
                           
                           /* if the auto-detect spectrum inversion is required,
                            * we also need change the spectrum inversion option before re-connection.
                            */
                           if (1 == gLocalTuning[uUnit].tune.nim_cable_tune.auto_spectrum  && gWaitingCnt == 6 )
                           {
                              switch (gLocalTuning[uUnit].tune.nim_cable_tune.spectrum)
                              {
                                 case SPECTRUM_NORMAL:
                                    gLocalTuning[uUnit].tune.nim_cable_tune.spectrum = SPECTRUM_INVERTED;
                                    break;
                                 case SPECTRUM_INVERTED:
                                    gLocalTuning[uUnit].tune.nim_cable_tune.spectrum = SPECTRUM_NORMAL;
                                    break;
                              }
                           }
                           
                           if(gWaitingCnt ==6)
                           {
                              gLocalTuning[uUnit].tune.nim_cable_tune.frequency =uTuningFreq;//the offset should be according to the spec
                              debug_out (TL_INFO, "CABLE TASK: 000000\n");
                              if (DRV_OK == dcf872x_connect(uUnit, (TUNING_SPEC *)(&gLocalTuning[uUnit]), TRUE))
                              {
                                 debug_out (TL_INFO, "CABLE TASK: Frequnce\n");
                              }
                           }
                           if(gWaitingCnt ==8)
                           {
                              gLocalTuning[uUnit].tune.nim_cable_tune.frequency = uTuningFreq + 250000;//the offset should be according to the spec
                              debug_out (TL_INFO, "CABLE TASK: +150000\n");
                              if (DRV_OK == dcf872x_connect(uUnit, (TUNING_SPEC *)(&gLocalTuning[uUnit]), TRUE))
                              {
                                 debug_out (TL_INFO, "CABLE TASK: Frequnce+ offset\n");
                              }
                           }
                           if(gWaitingCnt ==10)
                           {
                              gLocalTuning[uUnit].tune.nim_cable_tune.frequency = uTuningFreq - 500000;//the offset should be according to the spec
                              debug_out (TL_INFO, "CABLE TASK: -150000\n");
                              if (DRV_OK == dcf872x_connect(uUnit, (TUNING_SPEC *)(&gLocalTuning[uUnit]), TRUE))
                              {
                                 debug_out (TL_INFO, "CABLE TASK: Frequnce - offset\n");
                              }
                           }
                           gWaitingCnt = 0;   /* for the next M ms interval */
                           gLocalTuning[uUnit].tune.nim_cable_tune.frequency = uTuningFreq;
                           gRetryCnt++;
                           if (gRetryCnt == 3) /* 3000 ms */
                           {
                              /* if the front-end fails to acquire lock after 3 retries,
                               * call the callback function to indicate DEMOD_TIMEOUT.
                               */
                              if (gpfnCallbacks[uUnit])
                              {
                                 CallbackData.type      = DEMOD_CONNECT_STATUS;
                                 CallbackData.parm.type = DEMOD_TIMEOUT;
                                 CallbackData.parm.tune = gLocalTuning[uUnit];
                                 gpfnCallbacks[uUnit] (guLocalModule, uUnit, DEMOD_CONNECT_STATUS, &CallbackData);
                              }
                              gRetryCnt = 0;
                              debug_out (TL_INFO, "CABLE TASK: DEMOD_TIMEOUT\n");
                              
                              /* IF TIMEOUT IN AUTO-QAM DETECTION MODE, TRY 32-QAM MANUALLY */
                              if (gAutoQAMMode == 1)
                              {
                                 if (MOD_QAMAUTO == gLocalTuning[uUnit].tune.nim_cable_tune.modulation)
                                 {
                                    gLocalTuning[uUnit].tune.nim_cable_tune.modulation = MOD_QAM32;
                                 }
                                 else
                                 {
                                    gLocalTuning[uUnit].tune.nim_cable_tune.modulation = MOD_QAMAUTO;
                                 }
                              } /* endif (gAutoQAMMode) */
                           } /* endif (gRetryCnt) */
                        }
			   #endif
                     } /* endif bLockState */
                     break;
                  
                  case NIM_CONNECTED:
                     if (TRUE == bLockState)
                     {
                        gTimeoutCnt ++;
                        if ( gTimeoutCnt == 10 ) /* 2000 ms */
                        {
                           gTimeoutCnt = 0;
                           /* monitor the performance of the front end. */
                           dcf872x_get_statics (uUnit);
                           m_get_statics = 1;
                        }
                     }
                     else
                     {
                        /* stop the monitor timer. */
                        rc = tick_stop (gNIMTimer);
                        debug_out (TL_INFO, "CABLE TASK: Lost signal\n");
                        /* update the state */
                        guOldLOCState[uUnit] = guNewLOCState[uUnit];
                        guNewLOCState[uUnit] = NIM_FAILED;
                        
                        #ifndef CABLE_LKDT_DISABLED
                        /* lost the signal lock, turn off the lock indicator */
                        CABLE_LKDT_TURN_OFF (CABLE_LKDT_GPIO_BANK, CABLE_LKDT_GPIO_BIT);
                        #endif
                        
                        /* call the callback function to indicate LOSE_LOCKED. */
                        if (gpfnCallbacks[uUnit])
                        {
                           CallbackData.type      = DEMOD_CONNECT_STATUS;
                           CallbackData.parm.type = DEMOD_DRIVER_LOST_SIGNAL;
                           CallbackData.parm.tune = gLocalTuning[uUnit];
                           gpfnCallbacks[uUnit] (guLocalModule, uUnit, DEMOD_CONNECT_STATUS, &CallbackData);
                        }
                        
                        /* IF LOST SIGNAL IN AUTO-QAM DETECTION MODE, ALWAYS RETRY WITH AUTO-QAM MODE. */
                        if (gAutoQAMMode == 1)
                        {
                           gLocalTuning[uUnit].tune.nim_cable_tune.modulation = MOD_QAMAUTO;
                        } /* endif (gAutoQAMMode) */
                        
                        /* try to connect the lost signal */
                        if (DRV_OK == dcf872x_connect(uUnit, (TUNING_SPEC *)(&gLocalTuning[uUnit]), TRUE))
                        {
                           /* clear the waiting counter and the retry counter */
                           gWaitingCnt = 0;
                           gRetryCnt   = 0;
                        }
                        /* start a timer to monitor the reconnection timeout */
                        rc = tick_set (gNIMTimer, (u_int32)(NIM_CONNECT_TIMEOUT), FALSE);
                        rc = tick_start (gNIMTimer);
                     }
                     
          //           desk_display_nosig_banner(FALSE);//for siglost

                     break;
                  
                  default:
                     break;
               
               } /* endswitch guNewLOCState[uUnit] */
               #else /* CABLE_INT_DISABLED is not defined, use the INTERRUPT method */
               
               #endif /* CABLE_INT_DISABLED */
               break;
            
            case NIM_MSG_CONNECT:
               debug_out (TL_INFO, "CABLE TASK: received NIM_MSG_CONNECT message\n");
               /* update the state */
               guOldLOCState[uUnit] = guNewLOCState[uUnit];
               guNewLOCState[uUnit] = NIM_CONNECTING;
               #ifndef CABLE_LKDT_DISABLED
               /* the new signal is to be tuned, turn off the lock indicator */
               CABLE_LKDT_TURN_OFF (CABLE_LKDT_GPIO_BANK, CABLE_LKDT_GPIO_BIT);
               #endif
               /* connect to the required signal. */
               if (DRV_OK == dcf872x_connect(uUnit, (TUNING_SPEC *)(uRxMsg[1]), TRUE))
               {
                  /* clear the waiting counter and the retry counter */
                  gWaitingCnt = 0;
                  gRetryCnt   = 0;
               }
               /* start a timer to monitor the connection timeout. */
               rc = tick_set (gNIMTimer, (u_int32)(NIM_CONNECT_TIMEOUT), FALSE);
               rc = tick_start (gNIMTimer);
               /* release the semaphore to have the connection function return */
               //sem_put(gNIMSemaphore);
               break;
            
            case NIM_MSG_DISCONNECT:
               debug_out (TL_INFO, "CABLE TASK: received NIM_MSG_DISCONNECT message\n");
               /* do something to disconnect the current signal. */
               /* after disconnection, update the state. */
               guOldLOCState[uUnit] = guNewLOCState[uUnit];
               guNewLOCState[uUnit] = NIM_DISCONNECTED;
               #ifndef CABLE_LKDT_DISABLED
               /* disconnect with the signal, turn off the lock indicator */
               CABLE_LKDT_TURN_OFF (CABLE_LKDT_GPIO_BANK, CABLE_LKDT_GPIO_BIT);
               #endif
               /* call the callback function to indicate DISCONNECTED. */
               if (gpfnCallbacks[uUnit])
               {
                  CallbackData.type      = DEMOD_DISCONNECT_STATUS;
                  CallbackData.parm.type = DEMOD_DISCONNECTED;
                  gpfnCallbacks[uUnit] (guLocalModule, uUnit, DEMOD_DISCONNECT_STATUS, &CallbackData);
               }
               break;
            
            case NIM_MSG_SCAN:
               debug_out (TL_INFO, "CABLE TASK: received NIM_MSG_SCAN message\n");
               break;
            
            default:
               debug_out (TL_ERROR, "CABLE TASK: Unknown message 0x%x for unit %d\n", uMessage, uUnit);
               break;
         
         } /* endswitch (uMessage) */
      } /* end of processing messages */
      else
      {
         debug_out (TL_ERROR, "CABLE TASK: error code 0x%x when receiving message\n", iReturnCode);
      } /* end of receiving messages */
   } /* endwhile (1) */
}

#endif
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*               =========================================                   */
/*               =  FUNCTIONS OF CABLE FRONT-END DRIVER  =                   */
/*               =========================================                   */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define CABLE_UNIT_TYPE_FUNC           cnxt_c_get_unit_type
#define CABLE_IOCTL_FUNC               cnxt_c_ioctl
#define CABLE_CONNECT_FUNC             cnxt_c_connect
#define CABLE_DISCONNECT_FUNC          cnxt_c_disconnect
#define CABLE_GET_SIGNAL_FUNC          cnxt_c_get_signal_stats
#define CABLE_GET_LOCK_FUNC            cnxt_c_get_lock_status

#define CABLE_GET_TUNING_FUNC          cnxt_c_get_tuning
#define CABLE_SET_CALLBACK_FUNC        cnxt_c_set_callback
#define CABLE_CLEAR_CALLBACK_FUNC      cnxt_c_clear_callback

#define CABLE_SCAN_FUNC                (0)
#define CABLE_SCAN_NEXT_FUNC           (0)
#define CABLE_RE_ACQUIRE_FUNC          (0)

static DEMOD_STATUS cnxt_c_get_unit_type (u_int32 uUnit, DEMOD_NIM_TYPE *pUnitType);
static DEMOD_STATUS cnxt_c_ioctl (u_int32 uUnit, DEMOD_IOCTL_TYPE eType, void *pData);
static DEMOD_STATUS cnxt_c_connect (u_int32 uUnit, TUNING_SPEC *pTuning, u_int32 *pTimeLimit);
static DEMOD_STATUS cnxt_c_disconnect (u_int32 uUnit);
static DEMOD_STATUS cnxt_c_get_signal_stats (u_int32 uUnit, SIGNAL_STATS *pSignalStats);
static DEMOD_STATUS cnxt_c_get_lock_status (u_int32 uUnit, bool *pLocked);
static DEMOD_STATUS cnxt_c_get_tuning (u_int32 uUnit, TUNING_SPEC *pTuning);
static DEMOD_STATUS cnxt_c_set_callback (u_int32 uUnit, MODULE_STATUS_FUNCTION *pfnCallback);
static DEMOD_STATUS cnxt_c_clear_callback (u_int32 uUnit);
/*
static bool cnxt_c_scan_next (u_int32 uUnit, SCAN_SPEC *pScan);
static bool cnxt_c_reacquire (TUNING_SPEC *pOriginal, TUNING_SPEC *pActual);
*/

/*****************************************************************************/
/*  FUNCTION:    cnxt_c_get_unit_type                                        */
/*                                                                           */
/*  PARAMETERS:  uUnit - unit number for which type information is requested */
/*                                                                           */
/*               pUnitType - pointer to the DEMOD_NIM_TYPE variable to be    */
/*                           filled out.                                     */
/*                                                                           */
/*  DESCRIPTION: This function returns information about the type of the     */
/*               unit that is the subject of the request.                    */
/*                                                                           */
/*  RETURNS:     DEMOD_UNINITIALIZED - this module has not been initialized. */
/*               DEMOD_BAD_UNIT - the unit number is not valid.              */
/*               DEMOD_BAD_PARAMETER - there is a bad parameter.             */
/*               DEMOD_SUCCESS - the function completed successfully.        */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
static DEMOD_STATUS cnxt_c_get_unit_type (u_int32 uUnit, DEMOD_NIM_TYPE *pUnitType)
{
   if (!guInitialized)
   {
      return (DEMOD_UNINITIALIZED);
   }
   
   if (NIM_NO_HARDWARE == guNewLOCState[uUnit])
   {
      return (DEMOD_BAD_UNIT);
   }
   
   if (NULL == pUnitType)
   {
      return (DEMOD_BAD_PARAMETER);
   }
   
   *pUnitType = DEMOD_NIM_CABLE;
   return (DEMOD_SUCCESS);
}


/*****************************************************************************/
/*  FUNCTION:    cnxt_c_ioctl                                                */
/*                                                                           */
/*  PARAMETERS:  uUnit - the unit number for which the ioctl operation is    */
/*                       requested.                                          */
/*               eType - the type of ioctl operation requested.              */
/*               pData - pointer to the DEMOD_IOCTL_TYPE structure that      */
/*                       contains data for the operation or will be filled   */
/*                       out with data by the operation.                     */
/*                                                                           */
/*  DESCRIPTION: This function implements various module-specific control or */
/*               status functions.                                           */
/*                                                                           */
/*  RETURNS:     DEMOD_UNINITIALIZED - this module has not been initialized. */
/*               DEMOD_BAD_UNIT - the unit number is not valid.              */
/*               DEMOD_UNIMPLEMENTED - The IOCTL is not used in this driver  */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
static DEMOD_STATUS cnxt_c_ioctl (u_int32 uUnit, DEMOD_IOCTL_TYPE eType, void *pData)
{
   if (!guInitialized)
   {
      return (DEMOD_UNINITIALIZED);
   }
   
   if (NIM_NO_HARDWARE == guNewLOCState[uUnit])
   {
      return (DEMOD_BAD_UNIT);
   }
   
   return (DEMOD_UNIMPLEMENTED);
}


/*****************************************************************************/
/*  FUNCTION:    cnxt_c_connect                                              */
/*                                                                           */
/*  PARAMETERS:  uUnit - the unit number for which the connect operation is  */
/*                       requested                                           */
/*               pTuning - pointer to the TUNING_SPEC structure containing   */
/*                         parameters for the requested connection.          */
/*               pTimeLimit - timeout value (ms) to be filled-out. if it's   */
/*                            OK for state machine to do it, allow plenty.   */
/*                                                                           */
/*  DESCRIPTION: This function connects to a stream by tuning the interface  */
/*               to the requested specification.                             */
/*                                                                           */
/*  RETURNS:     DEMOD_SUCCESS - the function completed successfully.        */
/*               DEMOD_UNINITIALIZED - this module has not been initialized. */
/*               DEMOD_BAD_UNIT      - the unit is not valid.                */
/*               DEMOD_BAD_PARAMETER - there is a bad parameter.             */
/*               DEMOD_ERROR - there has been an error.                      */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
static DEMOD_STATUS cnxt_c_connect (
   u_int32        uUnit,
   TUNING_SPEC    *pTuning,
   u_int32        *pTimeLimit
)
{
   u_int32     uTxMsg[4];
   int32       iReturnCode;
   
   debug_out (TL_INFO, "DEMOD CONNECTION FUNC: Enter.\n");
   
   if (!guInitialized)
   {
      return (DEMOD_UNINITIALIZED);
   }
   
   if (NIM_NO_HARDWARE == guNewLOCState[uUnit])
   {
      return (DEMOD_BAD_UNIT);
   }
   
   /* the NIM type must be always DEMOD_NIM_CABLE */
   if ( (pTuning == NULL) || (pTuning->type != DEMOD_NIM_CABLE) )
   {
      return (DEMOD_BAD_PARAMETER);
   }
   
   /* ONLY FOR TEST */
   /*
   pTuning->tune.nim_cable_tune.frequency   = 682000000;
   pTuning->tune.nim_cable_tune.symbol_rate = 6900000;
   pTuning->tune.nim_cable_tune.modulation  = MOD_QAM64;
   pTuning->tune.nim_cable_tune.spectrum    = SPECTRUM_NORMAL;
   pTuning->tune.nim_cable_tune.annex       = ANNEX_A;
   */
   /* save the tuning parameters to local memory */
   gLocalTuning[uUnit].type                              = pTuning->type;
   gLocalTuning[uUnit].tune.nim_cable_tune.frequency     = pTuning->tune.nim_cable_tune.frequency;
   gLocalTuning[uUnit].tune.nim_cable_tune.symbol_rate   = pTuning->tune.nim_cable_tune.symbol_rate;
   gLocalTuning[uUnit].tune.nim_cable_tune.modulation    = pTuning->tune.nim_cable_tune.modulation;
   gLocalTuning[uUnit].tune.nim_cable_tune.auto_spectrum = pTuning->tune.nim_cable_tune.auto_spectrum;
   gLocalTuning[uUnit].tune.nim_cable_tune.spectrum      = pTuning->tune.nim_cable_tune.spectrum;
   gLocalTuning[uUnit].tune.nim_cable_tune.annex         = pTuning->tune.nim_cable_tune.annex;
   
   /*save the frequnency to local varible */
   uTuningFreq = pTuning->tune.nim_cable_tune.frequency;
   /* check the AUTO-QAM DETECTION mode */
   if (MOD_QAMAUTO == gLocalTuning[uUnit].tune.nim_cable_tune.modulation)
   {
      gAutoQAMMode = 1;
   #if (defined m88dc2000)
  //    gLocalTuning[uUnit].tune.nim_cable_tune.modulation = gQAMAuto;
   #endif
   }
   else
   {
      gAutoQAMMode = 0;
   }
   
   /* send a control message to the demod task to signal the connection. */
   uTxMsg[0] = uUnit;
   uTxMsg[1] = (u_int32)(&gLocalTuning[uUnit]);
   uTxMsg[2] = (u_int32)(0);
   uTxMsg[3] = (u_int32)(NIM_MSG_CONNECT);
   //add by wm,修改消息队列中可能存在丢失消息现象
   //qu_reset(gNIMQueue);
   iReturnCode = qu_send (gNIMQueue, uTxMsg);
   if (RC_OK != iReturnCode)
   {
    	qu_reset(gNIMQueue);
	 iReturnCode = qu_send (gNIMQueue, uTxMsg);
	 if(RC_OK != iReturnCode)
	 {
	  return (DEMOD_ERROR);
	 }
   }
   
   //task_time_sleep ((u_int32)(LOCK_TIMEOUT));
   
   /*
    * wait for finishing the NIM_MSG_CONNECT process by
    * getting the semaphore that protects the NIM operation
    */
   //if (RC_OK != sem_get(gNIMSemaphore, KAL_WAIT_FOREVER))
   if (0)
   {
      return (DEMOD_ERROR);
   }

   m_get_statics = 0;
   
   /* return the connection timeout */
   /* *pTimeLimit = (2 * LOCK_TIMEOUT); */
   
   debug_out (TL_INFO, "DEMOD CONNECTION FUNC: Return.\n");
   return (DEMOD_SUCCESS);
}


/*****************************************************************************/
/*  FUNCTION:    cnxt_c_disconnect                                           */
/*                                                                           */
/*  PARAMETERS:  uUnit - the unit number for which the disconnect operation  */
/*                       is requested.                                       */
/*                                                                           */
/*  DESCRIPTION: This function disconnects from a stream.                    */
/*                                                                           */
/*  RETURNS:     DEMOD_SUCCESS - the function completed successfully.        */
/*               DEMOD_BAD_UNIT - the unit number is not valid.              */
/*               DEMOD_UNINITIALIZED - this module has not been initialized. */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
static DEMOD_STATUS cnxt_c_disconnect (u_int32 uUnit)
{
   u_int32     uMsg[4];
   int32       iReturnCode;
   
   if (!guInitialized)
   {
      return (DEMOD_UNINITIALIZED);
   }
   
   if (NIM_NO_HARDWARE == guNewLOCState[uUnit])
   {
      return (DEMOD_BAD_UNIT);
   }
   
   /* send a control message to the demod task to signal the disconnection. */
   uMsg[0] = uUnit;
   uMsg[1] = 0;
   uMsg[2] = 0;
   uMsg[3] = (u_int32)(NIM_MSG_DISCONNECT);
   iReturnCode = qu_send (gNIMQueue, uMsg);
   if (RC_OK != iReturnCode)
   {
      return (DEMOD_ERROR);
   }
   
   /* update the state */
   guOldLOCState[uUnit] = guNewLOCState[uUnit];
   guNewLOCState[uUnit] = NIM_DISCONNECTING;
   
   return (DEMOD_SUCCESS);
}


/*****************************************************************************/
/*  FUNCTION:    cnxt_c_get_tuning                                           */
/*                                                                           */
/*  PARAMETERS:  uUnit - the unit number for which the get tuning operation  */
/*                       is requested.                                       */
/*               pTuning - pointer to the TUNING_SPEC structure to be filled */
/*                         out with parameters for the current connection.   */
/*                                                                           */
/*  DESCRIPTION: This function returns the tuning parameters for the         */
/*               specified unit.                                             */
/*                                                                           */
/*  RETURNS:     DEMOD_SUCCESS - the function completed successfully.        */
/*               DEMOD_UNINITIALIZED - this module has not been initialized. */
/*               DEMOD_BAD_PARAMETER - there is a bad parameter.             */
/*               DEMOD_BAD_UNIT - the unit number is not valid.              */
/*               DEMOD_ERROR - there has been an error.                      */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*  Since there is no actual underlying tuner, this function just returns    */
/*  the last tuning parameters set.                                          */
/*                                                                           */
/*****************************************************************************/
static DEMOD_STATUS cnxt_c_get_tuning (u_int32 uUnit, TUNING_SPEC *pTuning)
{
   if (!guInitialized)
   {
      return (DEMOD_UNINITIALIZED);
   }
   
   if (NIM_NO_HARDWARE == guNewLOCState[uUnit])
   {
      return (DEMOD_BAD_UNIT);
   }
   
   if (NULL == pTuning)
   {
      return (DEMOD_BAD_PARAMETER);
   }
   
   /* get the semaphore that protects the NIM operation */
   if (RC_OK != sem_get(gNIMSemaphore, KAL_WAIT_FOREVER))
   {
      return (DEMOD_ERROR);
   }
   
   /* get the parameters of the current tuning signal */
   if ( dcf872x_get_tuning(uUnit, pTuning) )
   {
      /* Release the semaphore */
      sem_put(gNIMSemaphore);
      return (DEMOD_ERROR);
   }
   else
   {
      /* Release the semaphore */
      sem_put(gNIMSemaphore);
      return (DEMOD_SUCCESS);
   }
}


/*****************************************************************************/
/*  FUNCTION:    cnxt_c_set_callback                                         */
/*                                                                           */
/*  PARAMETERS:  uUnit - the unit number for which the callback is to be set */
/*               pfnCallback - the function address to be used for the       */
/*                             callback for this unit.                       */
/*                                                                           */
/*  DESCRIPTION: This function sets the callback pointer for the specified   */
/*               unit.                                                       */
/*                                                                           */
/*  RETURNS:     DEMOD_SUCCESS - the function completed successfully.        */
/*               DEMOD_UNINITIALIZED - this module has not been initialized. */
/*               DEMOD_BAD_PARAMETER - there is a bad parameter.             */
/*               DEMOD_BAD_UNIT - the unit number is not valid.              */
/*               DEMOD_ERROR - there has been an error.                      */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
static DEMOD_STATUS cnxt_c_set_callback (
   u_int32                 uUnit,
   MODULE_STATUS_FUNCTION  *pfnCallback
)
{
   if (!guInitialized)
   {
      return (DEMOD_UNINITIALIZED);
   }
   
   if (NIM_NO_HARDWARE == guNewLOCState[uUnit])
   {
      return (DEMOD_BAD_UNIT);
   }
   
   if (NULL == pfnCallback)
   {
      return (DEMOD_BAD_PARAMETER);
   }
   
   /* save the callback function pointer */
   gpfnCallbacks[uUnit] = pfnCallback;
   return (DEMOD_SUCCESS);
}


/*****************************************************************************/
/*  FUNCTION:    cnxt_c_clear_callback                                       */
/*                                                                           */
/*  PARAMETERS:  uUnit - the unit number for which the callback is to be     */
/*                       cleared.                                            */
/*                                                                           */
/*  DESCRIPTION: This function clears the callback pointer for the specified */
/*               unit.                                                       */
/*                                                                           */
/*  RETURNS:     DEMOD_UNINITIALIZED - this module has not been initialized. */
/*               DEMOD_BAD_PARAMETER - there is a bad parameter.             */
/*               DEMOD_BAD_UNIT - the unit number is not valid.              */
/*               DEMOD_ERROR - there has been an error.                      */
/*               DEMOD_SUCCESS - the function completed successfully.        */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
static DEMOD_STATUS cnxt_c_clear_callback (u_int32 uUnit)
{
   if (!guInitialized)
   {
      return (DEMOD_UNINITIALIZED);
   }
   
   if (NIM_NO_HARDWARE == guNewLOCState[uUnit])
   {
      return (DEMOD_BAD_UNIT);
   }
   
   /* clear the callback function pointer */
   gpfnCallbacks[uUnit] = 0;
   return (DEMOD_SUCCESS);
}


/*****************************************************************************/
/*  FUNCTION:    cnxt_c_get_signal_stats                                     */
/*                                                                           */
/*  PARAMETERS:  uUnit - the unit number for which the get signal stats      */
/*                       operation is requested.                             */
/*               pSignalStats - pointer to the SIGNAL_STATS structure to be  */
/*                       filled out with parameters for the specified unit.  */
/*                                                                           */
/*  DESCRIPTION: This function returns the signal statistics for the         */
/*               specified unit.                                             */
/*                                                                           */
/*  RETURNS:     DEMOD_SUCCESS - If all went well.                           */
/*               DEMOD_UNINITIALIZED - this module has not been initialized. */
/*               DEMOD_BAD_PARAMETER - there is a bad parameter.             */
/*               DEMOD_BAD_UNIT - the unit number is not valid.              */
/*               DEMOD_ERROR - there has been an error.                      */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
static DEMOD_STATUS cnxt_c_get_signal_stats (u_int32 uUnit, SIGNAL_STATS *pSignalStats)
{
   #ifndef STV0297_CNS
   float       fCNE;
   #endif
   
   if (!guInitialized)
   {
      return (DEMOD_UNINITIALIZED);
   }
   
   if (NIM_NO_HARDWARE == guNewLOCState[uUnit])
   {
      return (DEMOD_BAD_UNIT);
   }
   
   if (NULL == pSignalStats)
   {
      return (DEMOD_BAD_PARAMETER);
   }

   /* monitor the performance of the front end. */
   if(m_get_statics == 0)
   {
      dcf872x_get_statics(uUnit);
      m_get_statics = 1;
   }
   
   /* Fill stats (points to pSignalStats) with the status information */
   pSignalStats->type = DEMOD_NIM_CABLE;
   #ifndef STV0297_CNS
   fCNE = ((float)(gCNimCfg.DemodCNE) * 100.0 / 40.0);
   pSignalStats->stats.c_signal.signal_quality  = (int)(fCNE);
   #else
   pSignalStats->stats.c_signal.signal_quality = gCNimCfg.DemodCNE;
   #endif
   /* We could not get the AGC(s) values of Thomson cable front-end.  */
   /* so now the signal strength is equal to the signal quality.      */
   pSignalStats->stats.c_signal.signal_strength = gCNimCfg.SignalStrength;
   pSignalStats->stats.c_signal.rs_uncorrected  = gCNimCfg.DemodBERTerr;
   pSignalStats->stats.c_signal.rs_total        = gCNimCfg.DemodBERTnb;
   
   return (DEMOD_SUCCESS);
}


/*****************************************************************************/
/*  FUNCTION:    cnxt_c_get_lock_status                                      */
/*                                                                           */
/*  PARAMETERS:  uUnit - the unit number for which the get lock status       */
/*                       operation is requested.                             */
/*               pLocked - pointer to the boolean to be filled out indicated */
/*                       in locked/not locked for the specified unit.        */
/*                                                                           */
/*  DESCRIPTION: This function returns the lock status for the specified     */
/*               unit.                                                       */
/*                                                                           */
/*  RETURNS:     DEMOD_UNINITIALIZED - this module has not been initialized. */
/*               DEMOD_BAD_UNIT - the unit number is not valid.              */
/*               DEMOD_BAD_PARAMETER - there is a bad parameter.             */
/*               DEMOD_SUCCESS - the function completed successfully.        */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
static DEMOD_STATUS cnxt_c_get_lock_status (u_int32 uUnit, bool *pLocked)
{
   if (!guInitialized)
   {
      return (DEMOD_UNINITIALIZED);
   }
   
   if (NIM_NO_HARDWARE == guNewLOCState[uUnit])
   {
      return (DEMOD_BAD_UNIT);
   }
   
   if (NULL == pLocked)
   {
      return (DEMOD_BAD_PARAMETER);
   }
   
   /* get the current lock status of the cable NIM */
   dcf872x_get_lockstatus(uUnit, pLocked);
   dcf872x_get_bert(uUnit); //eric for mtg 10-12
   return (DEMOD_SUCCESS);
}


/*****************************************************************************/
/*  FUNCTION:    cnxt_c_scan_next                                            */
/*                                                                           */
/*  PARAMETERS:  uUnit - the unit number for which the scan_next operation   */
/*                       is requested.                                       */
/*               pScan - pointer to the SCAN_SPEC structure                  */
/*                                                                           */
/*  DESCRIPTION: This function manipulates the current TUNING_SPEC inside    */
/*               the SCAN_SPEC to the next scan parameters.                  */
/*                                                                           */
/*  RETURNS:     FALSE - OK                                                  */
/*               TRUE  - Scan passed the end or invalid bandwidth            */
/*                                                                           */
/*****************************************************************************/
/*
static bool cnxt_c_scan_next (u_int32 uUnit, SCAN_SPEC *pScan)
{
   if (!guInitialized)
   {
      return (FALSE);
   }
   
   if (NIM_NO_HARDWARE == guNewLOCState[uUnit])
   {
      return (FALSE);
   }
   
   if (NULL == pScan)
   {
      return (FALSE);
   }
   
   // TODO
   return (TRUE);
}
*/

/*****************************************************************************/
/*  FUNCTION:    cnxt_c_demod_reacquire                                      */
/*                                                                           */
/*  PARAMETERS:  pOriginal - a pointer to the tuning spec that was           */
/*                           originally tuned to.                            */
/*               pActual   - a pointer to the actual TUNING_SPEC that the    */
/*                           demod is tuned to.                              */
/*                                                                           */
/*  DESCRIPTION: This function compaires the two TUNING_SPEC's and decides   */
/*               if it is required to tune again to the actual TUNING_SPEC.  */
/*                                                                           */
/*  RETURNS:     TRUE - we need to re tune to the actual TUNING_SPEC.        */
/*               FALSE - we do NOT need to tune.                             */
/*                                                                           */
/*****************************************************************************/
/*
static bool cnxt_c_reacquire (TUNING_SPEC *pOriginal, TUNING_SPEC *pActual)
{
   int32    offset;
   
   if ( pOriginal->tune.nim_cable_tune.frequency >
        pActual->tune.nim_cable_tune.frequency
   )
   {
      offset = (pOriginal->tune.nim_cable_tune.frequency) -
               (pActual->tune.nim_cable_tune.frequency);
   }
   else
   {
      offset = (pActual->tune.nim_cable_tune.frequency) -
               (pOriginal->tune.nim_cable_tune.frequency);
   }
   
   if ( offset > MAX_OFFSET )
   {
      return (TRUE);
   }
   else
   {
      return (FALSE);
   }
}
*/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*               =========================================                   */
/*               = ENTRY POINT OF CABLE FRONT-END DRIVER =                   */
/*               =========================================                   */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*****************************************************************************/
/*  FUNCTION:    cnxt_cable_demod_init                                       */
/*                                                                           */
/*  PARAMETERS:  uModule      - the module designation to be used by this    */
/*                              driver in subsequent callbacks.              */
/*                                                                           */
/*               pNumberUnits - the pointer to an integer to be filled with  */
/*                              the total number of units in this module.    */
/*                                                                           */
/*               pfnFTable    - the pointer to the DEMOD_FTABLE structure to */
/*                              be filled out with function pointers for the */
/*                              module.                                      */
/*                                                                           */
/*  DESCRIPTION: This function initializes the module and returns a count of */
/*               units in the module and the function table for module       */
/*               functions.                                                  */
/*                                                                           */
/*  RETURNS:     DEMOD_SUCCESS - the function completed successfully.        */
/*               DEMOD_INITIALIZED - this module has been initialized.       */
/*               DEMOD_BAD_PARAMETER - there is a bad parameter.             */
/*               DEMOD_NO_HARDWARE - there is no NIM hardware in the system. */
/*               DEMOD_ERROR - An error was received from a call             */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
DEMOD_STATUS cnxt_cable_demod_init (
   u_int32        uModule,
   u_int32        *pNumberUnits,
   DEMOD_FTABLE   *pfnFTable
)
{
   int32          ii;
   
   debug_out (TL_FUNC, "CABLE INIT: Entered initialization uModule 0x%02X\n", uModule);
   
   if (guInitialized)
   {
      debug_out (TL_ERROR, "CABLE INIT: Already been initialized\n");
      return (DEMOD_INITIALIZED);
   }
   
   /**************************************************************************/
   /*   check whether the input parameters are valid.
    */
   if (NULL == pNumberUnits)
   {
      debug_out (TL_ERROR, "CABLE INIT: Bad parameter - pNumberUnits\n");
      return (DEMOD_BAD_PARAMETER);
   }
   
   if (NULL == pfnFTable)
   {
      debug_out (TL_ERROR, "CABLE INIT: Bad parameter - pfnFTable\n");
      return (DEMOD_BAD_PARAMETER);
   }
   
   /**************************************************************************/
   /*   check whether the NIM hardwares to be initialized are valid.
    */
   guLocalUnitCount = 0;
   
   /* Is there a GPIO pin used to reset the cable front-end? */
   #if (defined CABLE_RESET_DISABLED)
   /*
    * If the driver is for the NIM board on the Bronco IRD,
    * no GPIO pin is used to reset the NIM board.
    */
   #else /* CABLE_RESET_DISABLED is not defined. */
   /*
    * If the driver is for the Cable Front End module on the PuDong IRD,
    * we should set the RESET GPIO pin to HIGH level to get the Cable Front
    * End module out of reset.
    */
   DRIVE_GPIO_HIGH_BANK (CABLE_RST_GPIO_BANK, CABLE_RST_GPIO_BIT);
   #endif /* CABLE_RESET_DISABLED */
   
   for (ii=0; ii<(MAXIMUM_NUMBER_UNITS); ii++)
   {
      if (FALSE == iicAddressTest(guDemodAddr[ii], I2C_BUS_CABLE_FE, 0))
      {
         debug_out (TL_INFO, "CABLE INIT: Unit %d is not found at i2c addr 0x%03X\n", ii, guDemodAddr[ii]);
         guOldLOCState[ii] = guNewLOCState[ii];
         guNewLOCState[ii] = NIM_NO_HARDWARE;
      }
      else
      {
         debug_out (TL_INFO, "CABLE INIT: Unit %d is found at i2c addr 0x%03X\n", ii, guDemodAddr[ii]);
         guOldLOCState[ii] = guNewLOCState[ii];
         guNewLOCState[ii] = NIM_UNINITIALIZED;
         guLocalUnitCount++;
      }
   }
   #if (defined(GX1001_V35_SERVICE_ENABLE) && (GX1001_V35_SERVICE_ENABLE ==YES))
   #else
   if (0 == guLocalUnitCount)
   {
      debug_out (TL_ERROR, "CABLE INIT: No hardware is detected\n");
      return (DEMOD_NO_HARDWARE);
   }
   #endif
   /**************************************************************************/
   /*   allocate system resources (semaphore, queue, task ...) for the driver.
    */
   
   /* create the semaphore to be used to protect the cable front-end */
   gNIMSemaphore = sem_create(1, NIM_SEM_NAME);
   if (gNIMSemaphore == (sem_id_t)(0))
   {
      debug_out (TL_ERROR, "CABLE INIT: Can't create semaphore\n");
      return (DEMOD_ERROR);
   }
   
   /* create the tick timer to be used to timeout state transitions. */
   gNIMTimer = tick_create (NIM_TICK_CALLBACK, (void *)0, NIM_TICK_NAME);
   if (gNIMTimer == (tick_id_t)(0))
   {
      debug_out (TL_ERROR, "CABLE INIT: Can't create tick timer\n");
      return (DEMOD_ERROR);
   }
   #if (defined(GX1001_V35_SERVICE_ENABLE) && (GX1001_V35_SERVICE_ENABLE ==YES))
   tick_set (gNIMTimer, (u_int32)(NIM_MONITOR_PERIOD*3), FALSE);
   tick_start (gNIMTimer);
   #endif
   /* create the message queue to be used to communicate with the DEMOD task */
   gNIMQueue = qu_create (NIM_MSGQ_MAXIMUM, NIM_MSGQ_NAME);
   if (gNIMQueue == (queue_id_t)(0))
   {
      debug_out (TL_ERROR, "CABLE INIT: Can't create message queue\n");
      return (DEMOD_ERROR);
   }
   
   /* create the task to be used to process the messages from the DEMOD ISR */
   gNIMTask = task_create (NIM_TASK_MAINFUNC, NULL, NULL, (NIM_TASK_STK_SIZE),
                           (NIM_TASK_PRIORITY), (NIM_TASK_NAME));
   if (gNIMTask == (task_id_t)(0))
   {
      debug_out (TL_ERROR, "CABLE INIT: Can't create task\n");
      return (DEMOD_ERROR);
   }
   
   /**************************************************************************/
   /*   initialize the found NIM interfaces.
    */
   for (ii=0; ii<(MAXIMUM_NUMBER_UNITS); ii++)
   {
      /* ignore all lost NIM interfaces */
      if (NIM_NO_HARDWARE == guNewLOCState[ii])
      {
         continue;
      }
      /* get the semaphore that protects the NIM operation */
      if (RC_OK != sem_get(gNIMSemaphore, KAL_WAIT_FOREVER))
      {
         debug_out (TL_ERROR, "CABLE INIT: Can't get the semaphore\n");
         return (DEMOD_ERROR);
      }
      
      /* clear the callback function pointer */
      gpfnCallbacks[ii] = 0;
     #if (defined(GX1001_V35_SERVICE_ENABLE) && (GX1001_V35_SERVICE_ENABLE ==YES))
     #else
      /* initialize the Cable NIM */
      if ( dcf872x_init(ii) != DRV_OK )
      {
         debug_out (TL_ERROR, "CABLE INIT: Unknown type of the cable front-end\n");
         return (DEMOD_ERROR);
      }
      #endif
      /* update the state */
      guOldLOCState[ii] = guNewLOCState[ii];
      guNewLOCState[ii] = NIM_INITIALIZED;
      
      /* release the semaphore */
      sem_put(gNIMSemaphore);
      debug_out (TL_INFO, "CABLE INIT: Initialized unit 0x%02X\n", ii);
   }
   
   /* Which method is used to detect the events, POLLING or INTERRUPT? */
   #if (defined CABLE_INT_DISABLED)
   /* the POLLING method is used.   */
   #else /* CABLE_INT_DISABLED is not defined */
   /* the INTERRUPT method is used. */
   {
      int32          rc;
      
      /* set a GPIO pin as a interrupt source for the cable front-end driver */
      guNIMInterrupt = (u_int32)(INT_CABLE_NIM);
      /* claim a GPIO pin and set it up for input, interrupt on both edges. */
      MAKE_GPIO_INPUT_BANK (CABLE_INT_GPIO_BANK, CABLE_INT_GPIO_BIT);
      SET_GPIO_INT_EDGE_BANK (CABLE_INT_GPIO_BANK, CABLE_INT_GPIO_BIT, BOTH_EDGES);
      /* register the ISR handler for the interrupt */
      rc = int_register_isr (guNIMInterrupt, (PFNISR)cable_isr_handler,
                             FALSE, FALSE, (PFNISR *)&pfnNIMPreviousIsr);
      if (RC_OK != rc)
      {
         debug_out (TL_ERROR, "CABLE INIT: Can't register the interrupt handler\n");
         return (DEMOD_ERROR);
      }
      /* enable the interrupt */
      rc = int_enable (guNIMInterrupt);
      if (RC_OK != rc)
      {
         debug_out (TL_ERROR, "CABLE INIT: Can't enable the GPIO interrupt\n");
         return (DEMOD_ERROR);
      }
   }
   #endif /* CABLE_INT_DISABLED */
   
   /* Is a hardware lock indicator used to indicate the current lock status? */
   #if (defined CABLE_LKDT_DISABLED)
   /* no hardware lock indicator */
   #else /* CABLE_LKDT_DISABLED is not defined */
   /* during the driver initialization, turn off the lock indicator. */
   CABLE_LKDT_TURN_OFF (CABLE_LKDT_GPIO_BANK, CABLE_LKDT_GPIO_BIT);
   #endif
   
   /* initialize the functions table for this module type */
   pfnFTable->unit_type      = CABLE_UNIT_TYPE_FUNC;
   pfnFTable->ioctl          = CABLE_IOCTL_FUNC;
   pfnFTable->connect        = CABLE_CONNECT_FUNC;
   pfnFTable->disconnect     = CABLE_DISCONNECT_FUNC;
   pfnFTable->get_signal     = CABLE_GET_SIGNAL_FUNC;
   pfnFTable->get_lock       = CABLE_GET_LOCK_FUNC;
   pfnFTable->scan           = CABLE_SCAN_FUNC;
   pfnFTable->get_tuning     = CABLE_GET_TUNING_FUNC;
   pfnFTable->set_callback   = CABLE_SET_CALLBACK_FUNC;
   pfnFTable->clear_callback = CABLE_CLEAR_CALLBACK_FUNC;
   pfnFTable->scan_next      = CABLE_SCAN_NEXT_FUNC;
   pfnFTable->re_acquire     = CABLE_RE_ACQUIRE_FUNC;
   /* how many units are supported -- MUST BE ONLY ONE */
   *pNumberUnits = guLocalUnitCount;
   guLocalModule = uModule;
   guInitialized = 1;
   
   debug_out (TL_FUNC, "CABLE INIT: Finished initialization uModule 0x%02X\n", uModule);
   return (DEMOD_SUCCESS);
}


/****************************************************************************
 * Modifications:
 * $Log: 
 *  5    mpeg      1.4         5/26/04 3:12:04 AM     Steven Shen     CR(s) 
 *        9022 9023 : The DEMOD_DCF8722 driver version 1.20. Fix some bugs and 
 *        Add the support of getting the signal strength.
 *  4    mpeg      1.3         5/21/04 2:52:58 AM     Steven Shen     CR(s) 
 *        9273 9274 : Remove the task information into taskprio.h and 
 *        ucosconf.h.
 *  3    mpeg      1.2         5/20/04 3:35:08 AM     Steven Shen     CR(s) 
 *        9254 9255 : Support the Auto-QAM detection mode.
 *  2    mpeg      1.1         4/4/04 12:58:43 AM     Steven Shen     CR(s) 
 *        8674 8675 : Added support for auto-detect spectrum inversion (both 
 *        NORMAL and INVERTED).
 *  1    mpeg      1.0         3/15/04 10:30:36 AM    Matt Korte      CR(s) 
 *        8566 : Initial version of Thomson Cable Tuner/Demod
 * $
 *
 ****************************************************************************/
