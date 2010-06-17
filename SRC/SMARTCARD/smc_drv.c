/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*       Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002, 2003       */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        smcdrv.c
 *
 *
 * Description:     API for smart card driver
 *
 *
 * Author:          Larry Wang
 *
 ****************************************************************************/
/* $Header: smc_drv.c, 9, 9/17/03 3:58:42 PM, Lucy C Allevato$
 ****************************************************************************/

/*****************/
/* Include Files */
/*****************/
#include "string.h"
#include "kal.h"
#include "retcodes.h"
#include "handle.h"
#include "smartcard.h"
#include "smc_priv.h"

/*******************/
/* Local Variables */
/*******************/

/* Smart card data */
static SMC_DESCRIPTOR Card[CNXT_SMC_NUM_UNITS];
static PFNISR previous_isr[] = { NULL, NULL };
static SMC_RW_JOB_DESCRIPTOR RwJobs[CNXT_SMC_MAX_PENDING_RW];

/* Instance data */
static CNXT_SMC_DRIVER_INST DriverInst = { 0 };
static CNXT_SMC_DRIVER_INST *pDriverInst = &DriverInst;

static CNXT_SMC_INST InstArray[CNXT_SMC_MAX_HANDLES]; 
static bool          bInstUsed[CNXT_SMC_MAX_HANDLES];
                                                   /* Memory pool to get inst */

static CNXT_SMC_UNIT_INST UnitInst[CNXT_SMC_NUM_UNITS]; 
                                                 /* for Driver Inst structure */
static CNXT_SMC_CAPS CapsArray[] = 
                        { { sizeof ( CNXT_SMC_CAPS ), FALSE, 0 },
                          { sizeof ( CNXT_SMC_CAPS ), FALSE, 1 } }; 
                                               /* Static array of actual caps */

/********************/
/* Global Variables */
/********************/
/* DO NOT USE WITHOUT COMPELLING REASON!!! */

/*************************/
/* External Declarations */
/*************************/
/* DO NOT USE WITHOUT COMPELLING REASON!!! */

/***************************/
/* Local utility functions */
/***************************/
/* ALL FUNCTION DEFINED HERE HAVE TO BE static!!! */
static bool cnxt_smc_internal_data_init(void)
{
   u_int32 i;
   bool bRetVal = TRUE;

   /* 
    * Loop through the array of available Inst to set them to
    * unused, and make the list pointer invalid. 
    */
   for (i = 0; i < CNXT_SMC_MAX_HANDLES; ++i)
   {
      bInstUsed[i] = FALSE;
      InstArray[i].Preface.pSelf = NULL;
      InstArray[i].Preface.pNext = &InstArray[i+1].Preface;
      /* Initialize any Inst values that are added */
   }

   /* Terminate the linked list of instance structures. */
   InstArray[CNXT_SMC_MAX_HANDLES-1].Preface.pNext = NULL;
   /* Initialize UnitInst */
   for (i = 0; i < CNXT_SMC_NUM_UNITS; ++i)
   {
      UnitInst[i].pFirstInst = NULL;  /* list of handles is empty */
      UnitInst[i].bExclusive = FALSE;
      UnitInst[i].pCaps      = &CapsArray[i];
      UnitInst[i].pCard      = &Card[i];
      smc_descriptor_init ( UnitInst[i].pCard, i );

      /* clear job status */
      Card[i].ResetJob.pInst     = NULL;
      Card[i].PowerdownJob.pInst = NULL;
      Card[i].pRwJob             = NULL;
   }

   for (i = 0; i < CNXT_SMC_MAX_PENDING_RW; ++i )
   {
      RwJobs[i].pTxPtr       = NULL;
      RwJobs[i].uBytesToTx   = 0;
      RwJobs[i].pRxPtr       = NULL;
      RwJobs[i].pBytesRecved = NULL;
      RwJobs[i].pRxBufEnd    = NULL;
      RwJobs[i].Tag          = NULL;
      RwJobs[i].pInst        = NULL;

      if ( RwJobs[i].SyncSem == 0 )
      {
         RwJobs[i].SyncSem = sem_create ( 0, NULL );
         if ( RwJobs[i].SyncSem == 0 )
         {
            bRetVal = FALSE;
         }
      }

      if ( i != CNXT_SMC_MAX_PENDING_RW - 1 )
      {
         RwJobs[i].pNext = &RwJobs[i+1];
      }
      else
      {
         RwJobs[i].pNext = NULL;
      }
   }
   pDriverInst->pFreeRwJobHead = &RwJobs[0];
   pDriverInst->pFreeRwJobTail = &RwJobs[CNXT_SMC_MAX_PENDING_RW-1];

   return bRetVal;
}

static u_int32 cnxt_smc_unit_num(CNXT_SMC_CAPS *pCaps)
{
   return pCaps->uUnitNumber;
}

static SMC_RW_JOB_DESCRIPTOR *cnxt_smc_new_rw_job ( void )
{
   SMC_RW_JOB_DESCRIPTOR *pRwJob = pDriverInst->pFreeRwJobHead;

   if ( pRwJob )
   {
      pDriverInst->pFreeRwJobHead = pDriverInst->pFreeRwJobHead->pNext;
      if ( pDriverInst->pFreeRwJobHead == NULL )
      {
         pDriverInst->pFreeRwJobTail = NULL;
      }
   }

   return pRwJob;
}

/*************************/
/* Private API functions */
/*************************/
/* 
 * The following API functions should be used within smart card module.
 * Do NOT call them from any application!!!
 */
SMC_DESCRIPTOR *cnxt_smc_get_card_descriptor ( u_int32 uSlotId )
{
   return &Card[uSlotId];
}

bool cnxt_smc_check_drv_init ( void )
{
   return pDriverInst->bInit;
}

void cnxt_smc_notify_unit_clients(CNXT_SMC_EVENT Event, u_int32 uUnit)
{
   CNXT_SMC_INST *pInst;

   /* 
    * Loop through the open handles, calling the pNotifyFn with 
    * Event notice
    */
   pInst = pDriverInst->pUnitInst[uUnit].pFirstInst;
   while ( pInst )
   {
      if ( pInst->pNotifyFn )
      {
         pInst->pNotifyFn((CNXT_SMC_HANDLE)pInst,
                          pInst->pUserData,
                          Event,
                          NULL,
                          NULL);
      }
      pInst = (CNXT_SMC_INST*)pInst->Preface.pNext;
   }
}

void cnxt_smc_free_rw_job ( SMC_RW_JOB_DESCRIPTOR *pRwJob )
{
   if ( pRwJob == NULL )
   {
      return;
   }

   pRwJob->pTxPtr       = NULL;
   pRwJob->uBytesToTx   = 0;
   pRwJob->pRxPtr       = NULL;
   pRwJob->pBytesRecved = NULL;
   pRwJob->pRxBufEnd    = NULL;
   pRwJob->Tag          = NULL;
   pRwJob->pNext        = NULL;
   pRwJob->pInst        = NULL;

   if ( pDriverInst->pFreeRwJobTail )
   {
      pDriverInst->pFreeRwJobTail->pNext = pRwJob;
      pDriverInst->pFreeRwJobTail = pRwJob;
   }
   else
   {
      pDriverInst->pFreeRwJobHead = pRwJob;
      pDriverInst->pFreeRwJobTail = pRwJob;
   }
}
   
/************************/
/* Public API functions */
/************************/

/*********************************************************************/
/*  cnxt_smc_init ()                                                 */
/*                                                                   */
/*  PARAMETERS: CNXT_SMC_CONFIG *pCfg                                */
/*                                                                   */
/*  DESCRIPTION: This function initializes smc driver                */
/*                                                                   */
/*  RETURNS: CNXT_SMC_OK if success                                  */
/*           CNXT_SMC_ALREADY_INIT if driver has be init'ed          */
/*           CNXT_SMC_RESOURCE_ERROR if lack of resource             */
/*           CNXT_SMC_INTERNAL_ERROR if internal error occurs        */
/*********************************************************************/
CNXT_SMC_STATUS cnxt_smc_init ( CNXT_SMC_CONFIG *pCfg )
{
   static bool ReEntry = FALSE;
   bool bKs;
   u_int32 i;

   /*
    * Need to ensure that this routine only called once, so
    * start crit section, test the semaphore for existence.
    * If not exist, then create.
    * get out of the critical section, and see if the semaphore
    * exists. If it doesn't, then return an error.
    * Now get the semaphore. If this fails then
    * someone else grapped the semaphore, or there is a bigger
    * system resource problem
    */
   while (1)
   {
      bKs = critical_section_begin();
      if ( ReEntry == FALSE )
      {
         ReEntry = TRUE;
         critical_section_end(bKs);
         break;
      }
      critical_section_end(bKs);
      task_time_sleep(5);
   }

   if (pDriverInst->DriverSem == 0)
   {
      pDriverInst->DriverSem = sem_create(1, NULL);
   }

   if (pDriverInst->DriverSem == 0)
   {
      ReEntry = FALSE;
      return CNXT_SMC_RESOURCE_ERROR;
   }

   if (sem_get(pDriverInst->DriverSem, KAL_WAIT_FOREVER) != RC_OK)
   {
      ReEntry = FALSE;
      return CNXT_SMC_INTERNAL_ERROR;
   }

   if (pDriverInst->bInit == TRUE)
   {
      sem_put(pDriverInst->DriverSem);
      ReEntry = FALSE;
      return CNXT_SMC_ALREADY_INIT;
   }

   /* initialize internal data */
   if ( cnxt_smc_internal_data_init() == FALSE )
   {
      sem_put(pDriverInst->DriverSem);
      ReEntry = FALSE;
      return CNXT_SMC_RESOURCE_ERROR;
   }

   /* connect internal data to the driver instance structure */
   pDriverInst->pInstList = InstArray;
   pDriverInst->pUnitInst  = UnitInst;

   /*
    * Put Driver Specific code here.
    */


   /* enable smart card access */
   smc_access_enable ();

   for ( i = 0 ; i < CNXT_SMC_NUM_UNITS ; i ++ )
   {
      /* create semaphore for synchronous implementations of asynchronous op */
      if ( Card[i].ResetJob.SyncSem == 0 )
      {
         Card[i].ResetJob.SyncSem = sem_create ( 0, NULL );
         if ( Card[i].ResetJob.SyncSem == 0 )
         {
            sem_put(pDriverInst->DriverSem);
            ReEntry = FALSE;
            return CNXT_SMC_RESOURCE_ERROR;
         }
      }

      if ( Card[i].PowerdownJob.SyncSem == 0 )
      {
         Card[i].PowerdownJob.SyncSem = sem_create ( 0, NULL );
         if ( Card[i].PowerdownJob.SyncSem == 0 )
         {
            sem_put(pDriverInst->DriverSem);
            ReEntry = FALSE;
            return CNXT_SMC_RESOURCE_ERROR;
         }
      }

      /* intialize card HW */
      smc_hw_init ( (pDriverInst->pUnitInst[i]).pCard, i );

      /* Hook Interrupts */
      if ( int_register_isr ( ( ( i == 0 ) ? INT_VENDOR_SLOT_0 : INT_VENDOR_SLOT_1 ),
                              (PFNISR)cnxt_smc_isr, FALSE, FALSE, 
                              &previous_isr[i] ) != RC_OK )
      {
         return CNXT_SMC_INTERNAL_ERROR;
      }

      /* enable interrupts */
      if ( int_enable ( ( i == 0 ) ? INT_VENDOR_SLOT_0 : INT_VENDOR_SLOT_1 ) != RC_OK )
      {
         return CNXT_SMC_INTERNAL_ERROR;
      }
   }

   pDriverInst->bInit = TRUE; 
   sem_put(pDriverInst->DriverSem); /* must be last line of routine! */
   ReEntry = FALSE;
   return CNXT_SMC_OK;
} /* end cnxt_smc_init() */

/*********************************************************************/
/*  cnxt_smc_term ()                                                 */
/*                                                                   */
/*  PARAMETERS: None                                                 */
/*                                                                   */
/*  DESCRIPTION: This function terminates all operation of smc       */
/*               driver                                              */
/*                                                                   */
/*  RETURNS: CNXT_SMC_OK if success                                  */
/*           CNXT_SMC_NOT_INIT if driver is not initialized          */
/*           CNXT_SMC_INTERNAL_ERROR if internal error occurs        */
/*********************************************************************/
CNXT_SMC_STATUS cnxt_smc_term ( void )
{
   u_int32 i;

   IS_DRIVER_INITED(SMC, pDriverInst->bInit);
   
   if (sem_get(pDriverInst->DriverSem, KAL_WAIT_FOREVER) != RC_OK)
   {
      return CNXT_SMC_INTERNAL_ERROR;
   }

   /* Mark the driver as not initialized */
   pDriverInst->bInit = FALSE;

   /* notify all clients of termination of the driver */
   for ( i = 0 ; i < CNXT_SMC_NUM_UNITS ; i ++ )
   {
      cnxt_smc_notify_unit_clients(CNXT_SMC_EVENT_TERM, i);

      if ( Card[i].ResetJob.SyncSem )
      {
         sem_delete ( Card[i].ResetJob.SyncSem );
         Card[i].ResetJob.SyncSem = 0;
      }

      if ( Card[i].PowerdownJob.SyncSem )
      {
         sem_delete ( Card[i].PowerdownJob.SyncSem );
         Card[i].PowerdownJob.SyncSem = 0;
      }
   }

   /* destroy RW job semaphores */
   for ( i = 0 ; i < CNXT_SMC_MAX_PENDING_RW ; i ++ )
   {
      if ( RwJobs[i].SyncSem )
      {
         sem_delete ( RwJobs[i].SyncSem );
      }
   }

   /* Destroy semaphore */
   if (pDriverInst->DriverSem)
   {
      sem_delete(pDriverInst->DriverSem);
      pDriverInst->DriverSem = 0;
   }

   return CNXT_SMC_OK;
} /* end cnxt_smc_term() */

/*********************************************************************/
/*  cnxt_smc_reset ()                                                */
/*                                                                   */
/*  PARAMETERS: CNXT_SMC_CONFIG *pCfg                                */
/*                                                                   */
/*  DESCRIPTION: This function resets the internal state of smc      */
/*               driver                                              */
/*                                                                   */
/*  RETURNS: CNXT_SMC_OK if success                                  */
/*           CNXT_SMC_NOT_INIT if driver is not initialized          */
/*           CNXT_SMC_INTERNAL_ERROR if internal error occurs        */
/*********************************************************************/
CNXT_SMC_STATUS cnxt_smc_reset ( CNXT_SMC_CONFIG *pCfg )
{
   u_int32 i;

   IS_DRIVER_INITED(SMC, pDriverInst->bInit);

   if (sem_get(pDriverInst->DriverSem, KAL_WAIT_FOREVER) != RC_OK)
   {
      return CNXT_SMC_INTERNAL_ERROR;
   }

   /* Mark the driver as not initialized */
   pDriverInst->bInit = FALSE;

   /* notify all clients of reset of the driver */
   for ( i = 0 ; i < CNXT_SMC_NUM_UNITS ; i ++ )
   {
      cnxt_smc_notify_unit_clients(CNXT_SMC_EVENT_RESET, i);
   }


   /* initialize internal data */
   if ( cnxt_smc_internal_data_init() == FALSE )
   {
      sem_put(pDriverInst->DriverSem);
      return CNXT_SMC_RESOURCE_ERROR;
   }
   
   /*
    * Put Driver Specific code here.
    */

   pDriverInst->bInit = TRUE; 
   sem_put(pDriverInst->DriverSem); /* must be last line of routine! */
   return CNXT_SMC_OK;
} /* end cnxt_smc_reset() */

/*********************************************************************/
/*  cnxt_smc_get_units ()                                            */
/*                                                                   */
/*  PARAMETERS: u_int32 *puCount                                     */
/*                                                                   */
/*  DESCRIPTION: This function gets the number of device units       */
/*                                                                   */
/*  RETURNS: CNXT_SMC_OK if success                                  */
/*           CNXT_SMC_NOT_INIT if driver is not initialized          */
/*********************************************************************/
CNXT_SMC_STATUS cnxt_smc_get_units ( u_int32 *puCount )
{
   IS_DRIVER_INITED(SMC, pDriverInst->bInit);

   *puCount = CNXT_SMC_NUM_UNITS;
   return CNXT_SMC_OK;
} /* end cnxt_smc_get_units() */

/*********************************************************************/
/*  cnxt_smc_get_unit_caps ()                                        */
/*                                                                   */
/*  PARAMETERS:  u_int32       uUnitNumber                           */
/*               CNXT_SMC_CAPS *pCaps                                */
/*                                                                   */
/*  DESCRIPTION: This function gets capability parameters of a       */
/*               device unit                                         */
/*                                                                   */
/*  RETURNS: CNXT_SMC_OK if success                                  */
/*           CNXT_SMC_NOT_INIT if driver is not initialized          */
/*           CNXT_SMC_BAD_PARAMETER if pCaps is NULL or              */
/*                                     uLength field in pCaps is 0   */
/*           CNXT_SMC_BAD_UNIT if uUnitNumber is invalid             */
/*********************************************************************/
CNXT_SMC_STATUS cnxt_smc_get_unit_caps ( u_int32       uUnitNumber, 
                                         CNXT_SMC_CAPS *pCaps )
{
   u_int32 uLength;

   IS_DRIVER_INITED(SMC, pDriverInst->bInit);
   IS_NOT_NULL_POINTER(SMC, pCaps);
   uLength = pCaps->uLength;

   /* Check that the module requested exists */
   if (uUnitNumber >= CNXT_SMC_NUM_UNITS)
   {
      return CNXT_SMC_BAD_UNIT;
   }

   /* Check that the passed length was not zero */
   if (uLength == 0)
   {
      return CNXT_SMC_BAD_PARAMETER;   
   }

   /* Copy the caps structure to the clients buffer with length protection */
   uLength = min(uLength, sizeof(CNXT_SMC_CAPS));
   memcpy(pCaps, pDriverInst->pUnitInst[uUnitNumber].pCaps, uLength);
   pCaps->uLength = uLength;

   return CNXT_SMC_OK;
} /* end cnxt_smc_get_unit_caps() */

/*********************************************************************/
/*  cnxt_smc_open ()                                                 */
/*                                                                   */
/*  PARAMETERS: CNXT_SMC_HANDLE    *pHandle                          */
/*              CNXT_SMC_CAPS      *pCaps                            */
/*              CNXT_SMC_PFNNOTIFY pNotifyFn                         */
/*              void               *pUserData                        */
/*                                                                   */
/*  DESCRIPTION: This function opens an instance of a device         */
/*                                                                   */
/*  RETURNS: CNXT_SMC_OK if success                                  */
/*           CNXT_SMC_NOT_INIT if driver is not initialized          */
/*           CNXT_SMC_BAD_PARAMETER if any input argument is         */
/*                                          invalid                  */
/*           CNXT_SMC_RESOURCE_ERROR if lack of resource             */
/*           CNXT_SMC_INTERNAL_ERROR if internal error occurs        */
/*           CNXT_SMC_NOT_AVAILABLE if exclusive violated            */
/*********************************************************************/
CNXT_SMC_STATUS cnxt_smc_open ( CNXT_SMC_HANDLE    *pHandle,
                                CNXT_SMC_CAPS      *pCaps,
                                CNXT_SMC_PFNNOTIFY pNotifyFn,
                                void               *pUserData )
{
   CNXT_SMC_INST *pInst;
   CNXT_SMC_UNIT_INST *pUnitInst;
   u_int32 uUnit;

   IS_DRIVER_INITED(SMC,pDriverInst->bInit);
   IS_NOT_NULL_POINTER(SMC, pHandle);
   IS_NOT_NULL_POINTER(SMC, pCaps);

   if (sem_get(pDriverInst->DriverSem, KAL_WAIT_FOREVER) != RC_OK)
   {
      return CNXT_SMC_INTERNAL_ERROR;
   }

   /* figure out unit number based on info in pCaps */
   uUnit = cnxt_smc_unit_num(pCaps);

   if (uUnit >= CNXT_SMC_NUM_UNITS)
   {
      return CNXT_SMC_BAD_UNIT;
   }

   /* check bExclusive */
   pUnitInst = &(pDriverInst->pUnitInst[uUnit]);
   if ((pUnitInst->pFirstInst != NULL) && 
       (pUnitInst->bExclusive || pCaps->bExclusive))
   {
      sem_put(pDriverInst->DriverSem); /* must be last line of routine! */
      return CNXT_SMC_NOT_AVAILABLE;
   }
   /* 
    * There is a possibility that the pCaps needs to be checked further.
    * Add that here.
    */


   /* create an instance */
   if ( !CREATE_HANDLE(&(pDriverInst->pInstList), &pInst) )
   {
      *pHandle = NULL;
      sem_put(pDriverInst->DriverSem); /* must be last line of routine! */
      return CNXT_SMC_RESOURCE_ERROR;
   }

   /* add the instance into the list */
   pInst->uUnitNumber = uUnit;
   if (ADD_HANDLE (&(pUnitInst->pFirstInst), pInst) == FALSE)
   {
      DESTROY_HANDLE(&(pDriverInst->pInstList), pInst);
      sem_put(pDriverInst->DriverSem); /* must be last line of routine! */
      return CNXT_SMC_INTERNAL_ERROR;
   }

   pInst->Preface.pSelf = (CNXT_HANDLE_PREFACE*)pInst;
   pInst->pNotifyFn = pNotifyFn;    /* Use this fcn to notify appl of events */
   pInst->pUserData = pUserData;    /* Store data the inst needs */

   /*
    * Put Driver Specific code here.
    */

   /* set driver bExclusive field */
   pUnitInst->bExclusive = pCaps->bExclusive;

   *pHandle = (CNXT_SMC_HANDLE)pInst;

   sem_put(pDriverInst->DriverSem); /* must be last line of routine! */
   return CNXT_SMC_OK;
} /* end cnxt_smc_open() */

/*********************************************************************/
/*  cnxt_smc_close ()                                                */
/*                                                                   */
/*  PARAMETERS: CNXT_SMC_HANDLE Handle                               */
/*                                                                   */
/*  DESCRIPTION: This function closes a instance of a device         */
/*                                                                   */
/*  RETURNS: CNXT_SMC_OK if success                                  */
/*           CNXT_SMC_NOT_INIT if driver is not initialized          */
/*           CNXT_SMC_BAD_HANDLE if Handle is invalid                */
/*           CNXT_SMC_INTERNAL_ERROR if internal error occurs        */
/*********************************************************************/
CNXT_SMC_STATUS cnxt_smc_close ( CNXT_SMC_HANDLE Handle )
{
   CNXT_SMC_INST *pInst = (CNXT_SMC_INST*)Handle;
   CNXT_SMC_UNIT_INST *pUnitInst;

   IS_DRIVER_INITED(SMC,pDriverInst->bInit);
   IS_VALID_HANDLE(SMC, pInst, &(pDriverInst->pUnitInst[pInst->uUnitNumber].pFirstInst));

   if (sem_get(pDriverInst->DriverSem, KAL_WAIT_FOREVER) != RC_OK)
   {
      return CNXT_SMC_INTERNAL_ERROR;
   }

   /*
    * Put Driver Specific code here.
    */

   pUnitInst = &(pDriverInst->pUnitInst[pInst->uUnitNumber]);

   /* release the resource of the instance */
   if (REMOVE_HANDLE(&(pUnitInst->pFirstInst), pInst) == FALSE)
   {
      sem_put(pDriverInst->DriverSem); /* must be last line of routine! */
      return CNXT_SMC_INTERNAL_ERROR;
   }

   DESTROY_HANDLE(&(pDriverInst->pInstList), pInst);

   /* set the unit to be shared if no instance opened for the unit */
   if ( pUnitInst->pFirstInst == NULL )
   {
      pUnitInst->bExclusive = FALSE;
   }

   sem_put(pDriverInst->DriverSem); /* must be last line of routine! */
   return CNXT_SMC_OK;
} /* end cnxt_smc_close() */

/*********************************************************************/
/*  cnxt_smc_reset_card ()                                           */
/*                                                                   */
/*  PARAMETERS: CNXT_SMC_HANDLE Handle                               */
/*              bool            bAsync                               */
/*              void            *Tag                                 */
/*                                                                   */
/*  DESCRIPTION:  This function resets card referenced by Handle.    */
/*                                                                   */
/*  RETURNS: CNXT_SMC_OK if success                                  */
/*           CNXT_SMC_NOT_INIT if driver is not initialized          */
/*           CNXT_SMC_BAD_HANDLE if Handle is invalid                */
/*           CNXT_SMC_NOT_AVAILABLE if no card or card is in         */
/*                                     resetting                     */
/*********************************************************************/
CNXT_SMC_STATUS cnxt_smc_reset_card ( CNXT_SMC_HANDLE Handle,
                                      bool            bAsync,
                                      void            *Tag )
{
   CNXT_SMC_INST *pInst = (CNXT_SMC_INST*)Handle;
   SMC_RW_JOB_DESCRIPTOR *pRwJob;
   u_int32 uUnit, uBank;
   bool bKs;

   IS_DRIVER_INITED(SMC,pDriverInst->bInit);
   IS_VALID_HANDLE(SMC, pInst, &(pDriverInst->pUnitInst[pInst->uUnitNumber].pFirstInst));

   /*
    * Put Driver Specific code here.
    */

   uUnit = pInst->uUnitNumber;

   /* check if there is another resetting pending */
   bKs = critical_section_begin ();
   if ( Card[uUnit].ResetJob.pInst ) 
   {
      critical_section_end ( bKs );
      return CNXT_SMC_OK;
   }
   Card[uUnit].ResetJob.pInst = pInst;
   critical_section_end ( bKs );

   /* clear other pending transactions */
   bKs = critical_section_begin ();
   if ( Card[uUnit].PowerdownJob.pInst ) 
        
   {
      if ( ( Card[uUnit].PowerdownJob.bAsync ) && 
           ( (Card[uUnit].PowerdownJob.pInst)->pNotifyFn ) )
      {
         (Card[uUnit].PowerdownJob.pInst)->pNotifyFn ( 
                               (CNXT_SMC_HANDLE)Card[uUnit].PowerdownJob.pInst,
                               (Card[uUnit].PowerdownJob.pInst)->pUserData,
                               CNXT_SMC_EVENT_CARD_POWER_DOWN_TIMEOUT,
                               NULL,
                               Card[uUnit].PowerdownJob.Tag );
      }
      Card[uUnit].PowerdownJob.pInst = NULL;
   }
   critical_section_end ( bKs );
                                                   
   /* clear all pending RW jobs */
   while ( Card[uUnit].pRwJob )
   {
      bKs = critical_section_begin ();
      pRwJob = Card[uUnit].pRwJob;
      Card[uUnit].pRwJob = pRwJob->pNext;
      critical_section_end ( bKs );

      smc_term_rw_job ( pRwJob, CNXT_SMC_TIMEOUT );
   }


   /* save bAsync & Tag */
   Card[uUnit].ResetJob.bAsync = bAsync;
   Card[uUnit].ResetJob.Tag    = Tag;

   uBank = BANK_FROM_SLOT_ID ( uUnit );

   /* initialize card descriptor */
   smc_descriptor_init ( &Card[uUnit], uUnit );

   /* check if there is card in the slot */
   if ( Card[uUnit].State == SMC_NOT_INSERT )
   {
      Card[uUnit].ResetJob.pInst = NULL;
      return CNXT_SMC_NOT_AVAILABLE;
   }

   /* initialize card HW */
   smc_hw_init ( &Card[uUnit], uUnit );

   /* set receive mode to receive ATR */
   CNXT_SET_VAL ( SMC_TERM_CTRL_REG_ADDR ( uBank ), 
                  SMC_TERM_CTRL_MODE_MASK, 
                  SMC_MODE_RX );

   /* Enable Rx Interrupt */
   CNXT_SET_VAL ( SMC_INT_MASK_REG_ADDR ( uBank ), 
                  SMC_INT_RXREAD_MASK, 
                  SMC_ENABLE );

   /* initiate reset */
   if (CNXT_GET (SMC_ICC_STAT_REG_ADDR (uBank), SMC_ICC_STAT_CARDPOWER_MASK))
   {
      /* do a warm reset */
      *(LPREG)(SMC_ICC_CTRL_REG_ADDR (uBank)) = SMC_ICC_CTRL_WARMRESET_MASK;
   }
   else
   {
      /* do a cold reset */
      *(LPREG)(SMC_ICC_CTRL_REG_ADDR (uBank)) = ( SMC_ICC_CTRL_ACTIVATECARD_MASK |
                                                  SMC_ICC_CTRL_DEACTIVATECARD_MASK );
   }

   if ( Card[uUnit].ResetJob.bAsync == FALSE )
   {
      if ( sem_get ( Card[uUnit].ResetJob.SyncSem,
                     Card[uUnit].Config.uTimeout ) != RC_OK )
      {
         return CNXT_SMC_TIMEOUT;
      }
   }
   
   return CNXT_SMC_OK;
}  /* end cnxt_smc_reset_card */

/*********************************************************************/
/*  cnxt_smc_powerdown_card ()                                       */
/*                                                                   */
/*  PARAMETERS: CNXT_SMC_HANDLE Handle                               */
/*              bool            bAsync                               */
/*              void            *Tag                                 */
/*                                                                   */
/*  DESCRIPTION:  This function shuts down card referenced by Handle.*/
/*                                                                   */
/*  RETURNS: CNXT_SMC_OK if success                                  */
/*           CNXT_SMC_NOT_INIT if driver is not initialized          */
/*           CNXT_SMC_BAD_HANDLE if Handle is invalid                */
/*           CNXT_SMC_NOT_AVAILABLE if no card or card is in         */
/*                                     resetting or is shuting down  */
/*********************************************************************/
CNXT_SMC_STATUS cnxt_smc_powerdown_card ( CNXT_SMC_HANDLE Handle,
                                          bool            bAsync,
                                          void            *Tag )
{
   CNXT_SMC_INST *pInst = (CNXT_SMC_INST*)Handle;
   SMC_RW_JOB_DESCRIPTOR *pRwJob;
   u_int32 uUnit;
   bool bKs;

   IS_DRIVER_INITED(SMC,pDriverInst->bInit);
   IS_VALID_HANDLE(SMC, pInst, &(pDriverInst->pUnitInst[pInst->uUnitNumber].pFirstInst));

   /*
    * Put Driver Specific code here.
    */

   uUnit = pInst->uUnitNumber;

   /* check if there is resetting/power down pending */
   bKs = critical_section_begin ();
   if ( Card[uUnit].ResetJob.pInst )
   {
      critical_section_end ( bKs );
      return CNXT_SMC_NOT_AVAILABLE;
   }
   if ( Card[uUnit].PowerdownJob.pInst )
   {
      critical_section_end ( bKs );
      return CNXT_SMC_OK;
   }
   Card[uUnit].PowerdownJob.pInst = pInst;
   critical_section_end ( bKs );

   /* clear all pending RW jobs */
   bKs = critical_section_begin ();
   while ( Card[uUnit].pRwJob )
   {
      pRwJob = Card[uUnit].pRwJob;
      Card[uUnit].pRwJob = pRwJob->pNext;

      smc_term_rw_job ( pRwJob, CNXT_SMC_TIMEOUT );
   }
   critical_section_end ( bKs );

   if ( Card[uUnit].State == SMC_NOT_INSERT )
   {
      Card[uUnit].PowerdownJob.pInst = NULL;
      return CNXT_SMC_NOT_AVAILABLE;
   }

   /* save bAsync & Tag */
   Card[uUnit].PowerdownJob.bAsync = bAsync;
   Card[uUnit].PowerdownJob.Tag    = Tag;

   /* check if the card is up */
   if ( CNXT_GET ( SMC_ICC_STAT_REG_ADDR ( BANK_FROM_SLOT_ID ( uUnit ) ),
                   SMC_ICC_STAT_CARDPOWER_MASK ) == 0 )
   {
      /* card is down, clear power down job */
      Card[uUnit].PowerdownJob.pInst = NULL;
   }
   else
   {
      /* initiate deactivate */
      *(LPREG)( SMC_ICC_CTRL_REG_ADDR ( BANK_FROM_SLOT_ID ( uUnit ) ) ) 
                                          = SMC_ICC_CTRL_DEACTIVATECARD_MASK;
   }

   if ( Card[uUnit].PowerdownJob.bAsync == FALSE )
   {
      if ( sem_get ( Card[uUnit].PowerdownJob.SyncSem,
                     Card[uUnit].Config.uTimeout ) != RC_OK )
      {
         return CNXT_SMC_TIMEOUT;
      }
   }
      

   return CNXT_SMC_OK;
}  /* end cnxt_smc_reset_card */

/*********************************************************************/
/*  cnxt_smc_get_state ()                                            */
/*                                                                   */
/*  PARAMETERS: CNXT_SMC_HANDLE Handle                               */
/*              CNXT_SMC_STATE  *pState                              */
/*                                                                   */
/*  DESCRIPTION: This function queries the state of a card.          */
/*                                                                   */
/*  RETURNS: CNXT_SMC_OK if success                                  */
/*           CNXT_SMC_NOT_INIT if driver is not initialized          */
/*           CNXT_SMC_BAD_HANDLE if Handle is invalid                */
/*********************************************************************/
CNXT_SMC_STATUS cnxt_smc_get_state ( CNXT_SMC_HANDLE Handle,
                                     CNXT_SMC_STATE  *pState )
{
   CNXT_SMC_INST *pInst = (CNXT_SMC_INST*)Handle;
   u_int32 uUnit;

   IS_DRIVER_INITED(SMC,pDriverInst->bInit);
   IS_VALID_HANDLE(SMC, pInst, &(pDriverInst->pUnitInst[pInst->uUnitNumber].pFirstInst));

   /*
    * Put Driver Specific code here.
    */
   uUnit = pInst->uUnitNumber;

   /* check if a card is in the slot */
   if ( CNXT_GET ( SMC_ICC_STAT_REG_ADDR ( BANK_FROM_SLOT_ID ( uUnit ) ),
                   SMC_ICC_STAT_CARDPRESENT_MASK ) == 0 )
   {
      *pState = CNXT_SMC_EMPTY;
   }
   else
   {
      if ( Card[uUnit].State == SMC_ATR_PARSED )
      {
         *pState = CNXT_SMC_READY;
      }
      else
      {
         *pState = CNXT_SMC_NOT_READY;
      }
   }

   return CNXT_SMC_OK;
}  /* end cnxt_smc_query_state */

/*********************************************************************/
/*  cnxt_smc_get_atr ()                                              */
/*                                                                   */
/*  PARAMETERS: CNXT_SMC_HANDLE Handle                               */
/*              void            *pAtr                                */
/*              u_int32         *puBufLength                          */
/*                                                                   */
/*  DESCRIPTION: This function queries the state of a card.          */
/*                                                                   */
/*  RETURNS: CNXT_SMC_OK if success                                  */
/*           CNXT_SMC_NOT_INIT if driver is not initialized          */
/*           CNXT_SMC_BAD_HANDLE if Handle is invalid                */
/*           CNXT_SMC_NOT_AVAILABLE if card reset not done           */
/*           CNXT_SMC_BAD_PARAMETER if any of input argument is      */
/*                                     invalid                       */
/*********************************************************************/
CNXT_SMC_STATUS cnxt_smc_get_atr ( CNXT_SMC_HANDLE Handle,
                                   void            *pAtr,
                                   u_int32         *puBufLength )
{
   CNXT_SMC_INST *pInst = (CNXT_SMC_INST*)Handle;
   u_int32 uUnit;

   IS_DRIVER_INITED(SMC,pDriverInst->bInit);
   IS_VALID_HANDLE(SMC, pInst, &(pDriverInst->pUnitInst[pInst->uUnitNumber].pFirstInst));
   IS_NOT_NULL_POINTER(SMC, puBufLength);

   /*
    * Put Driver Specific code here.
    */
   uUnit = pInst->uUnitNumber;

   /* check state of the card */
   if ( Card[uUnit].State < SMC_ATR_RECEIVED )
   {
      return CNXT_SMC_NOT_AVAILABLE;
   }

   if ( pAtr == NULL )
   {
      /* get size of ATR only */
      *puBufLength = Card[uUnit].uAtrLength;
      return CNXT_SMC_OK;
   }

   if ( *puBufLength < Card[uUnit].uAtrLength )
   {
      /* input buffer is too small */
      *puBufLength = Card[uUnit].uAtrLength;
      return CNXT_SMC_BAD_PARAMETER;
   }

   /* copy ATR */
   memcpy ( pAtr, Card[uUnit].uAtr, Card[uUnit].uAtrLength );
   *puBufLength = Card[uUnit].uAtrLength;

   return CNXT_SMC_OK;
}  /* end cnxt_smc_get_atr */

/*********************************************************************/
/*  cnxt_smc_set_config ()                                           */
/*                                                                   */
/*  PARAMETERS: CNXT_SMC_HANDLE      Handle                          */
/*              CNXT_SMC_CONFIG_ITEM Item                            */
/*              u_int32              uValue                          */
/*                                                                   */
/*  DESCRIPTION: This function sets the config parameters of a card. */
/*                                                                   */
/*  RETURNS: CNXT_SMC_OK if success                                  */
/*           CNXT_SMC_NOT_INIT if driver is not initialized          */
/*           CNXT_SMC_BAD_HANDLE if Handle is invalid                */
/*           CNXT_SMC_NOT_AVAILABLE if card reset not done           */
/*           CNXT_SMC_BAD_PARAMETER if any of input argument is      */
/*                                     invalid                       */
/*********************************************************************/
CNXT_SMC_STATUS cnxt_smc_set_config ( CNXT_SMC_HANDLE      Handle,
                                      CNXT_SMC_CONFIG_ITEM Item,
                                      u_int32              uValue )
{
   CNXT_SMC_INST *pInst = (CNXT_SMC_INST*)Handle;
   u_int32 uUnit, uBank;

   IS_DRIVER_INITED(SMC,pDriverInst->bInit);
   IS_VALID_HANDLE(SMC, pInst, &(pDriverInst->pUnitInst[pInst->uUnitNumber].pFirstInst));

   /*
    * Put Driver Specific code here.
    */
   uUnit = pInst->uUnitNumber;

   /* check the state of the card */
   if ( Card[uUnit].State != SMC_ATR_PARSED )
   {
      return CNXT_SMC_NOT_AVAILABLE;
   }

   uBank = BANK_FROM_SLOT_ID ( uUnit );

   switch ( Item )
   {
      case CNXT_SMC_CONFIG_CONVENTION:
         if ( (CNXT_SMC_CONVENTION)uValue == CNXT_SMC_CONV_DIRECT )
         {
            CNXT_SET_VAL ( SMC_CONV_REG_ADDR ( uBank ), 
                           SMC_CONV_SENSE_MASK, 
                           SMC_SENSE_DIRECT );
            CNXT_SET_VAL ( SMC_CONV_REG_ADDR ( uBank ), 
                           SMC_CONV_ORDER_MASK, 
                           SMC_ORDER_DIRECT );
         }
         else if ( (CNXT_SMC_CONVENTION)uValue == CNXT_SMC_CONV_INVERSE )
         {
            CNXT_SET_VAL ( SMC_CONV_REG_ADDR ( uBank ),
                           SMC_CONV_SENSE_MASK,
                           SMC_SENSE_INVERSE );
            CNXT_SET_VAL ( SMC_CONV_REG_ADDR ( uBank ),
                           SMC_CONV_ORDER_MASK,
                           SMC_ORDER_INVERSE );
         }
         else
         {
            return CNXT_SMC_BAD_PARAMETER;
         }
         Card[uUnit].Config.uConvention = (u_int8)uValue;
         break;

      case CNXT_SMC_CONFIG_PROTOCOL:
         /* Protocol is a software/driver state. No change required to the hw */
         if ( ( (CNXT_SMC_PROTOCOL)uValue == CNXT_SMC_PROTOCOL_T0 ) || 
              ( (CNXT_SMC_PROTOCOL)uValue == CNXT_SMC_PROTOCOL_T1 ) )
         {
            Card[uUnit].Config.uProtocol = (u_int8)uValue;
         }
         else
         {
            return CNXT_SMC_BAD_PARAMETER;
         }
         break;
            
      case CNXT_SMC_CONFIG_FI:
         if ( ( uValue > 0 ) && ( uValue <= 15 ) )
         {
            /* set the new F */
            smc_set_fi ( uBank, uValue );
            Card[uUnit].Config.uFI = (u_int8)uValue;
         }
         else
         {
            return CNXT_SMC_BAD_PARAMETER;
         }
         break;

      case CNXT_SMC_CONFIG_DI:
         if ( ( uValue > 0 ) && ( uValue <= 15 ) )
         {
            /* set the new D */
            smc_set_di ( uBank, uValue );
            Card[uUnit].Config.uDI = (u_int8)uValue;
         }
         else
         {
            return CNXT_SMC_BAD_PARAMETER;
         }
         break;

      case CNXT_SMC_CONFIG_PI1:
      case CNXT_SMC_CONFIG_PI2:
      case CNXT_SMC_CONFIG_II:
         /* these config parameters cannot be set */
         return CNXT_SMC_BAD_PARAMETER;

      case CNXT_SMC_CONFIG_N:
         if ( ( uValue > 0 ) && ( uValue <= 255 ) )
         {
            /* set new N */
            CNXT_SET_VAL ( SMC_CHGUARD_REG_ADDR ( uBank ), SMC_GUARD_CYCLES_MASK, uValue );
            Card[uUnit].Config.uN = (u_int8)uValue;
         }
         else
         {
            return CNXT_SMC_BAD_PARAMETER;
         }
         break;
            
      case CNXT_SMC_CONFIG_TIMEOUT:
         Card[uUnit].Config.uTimeout = (u_int16)uValue;
         break;

      case CNXT_SMC_CONFIG_RETRY:
         Card[uUnit].Config.uRetry = (u_int8)uValue;
         break;

      default:
         return CNXT_SMC_BAD_PARAMETER;
   }


   return CNXT_SMC_OK;
}  /* end cnxt_smc_set_config */

/*********************************************************************/
/*  cnxt_smc_get_config ()                                           */
/*                                                                   */
/*  PARAMETERS: CNXT_SMC_HANDLE      Handle                          */
/*              CNXT_SMC_CONFIG_ITEM Item                            */
/*              u_int32              *puValue                        */
/*                                                                   */
/*  DESCRIPTION: This function gets the config parameters of a card. */
/*                                                                   */
/*  RETURNS: CNXT_SMC_OK if success                                  */
/*           CNXT_SMC_NOT_INIT if driver is not initialized          */
/*           CNXT_SMC_BAD_HANDLE if Handle is invalid                */
/*           CNXT_SMC_NOT_AVAILABLE if card reset not done           */
/*           CNXT_SMC_BAD_PARAMETER if any of input argument is      */
/*                                     invalid                       */
/*********************************************************************/
CNXT_SMC_STATUS cnxt_smc_get_config ( CNXT_SMC_HANDLE      Handle,
                                      CNXT_SMC_CONFIG_ITEM Item,
                                      u_int32              *puValue )
{
   CNXT_SMC_INST *pInst = (CNXT_SMC_INST*)Handle;
   u_int32 uUnit;

   IS_DRIVER_INITED(SMC,pDriverInst->bInit);
   IS_VALID_HANDLE(SMC, pInst, &(pDriverInst->pUnitInst[pInst->uUnitNumber].pFirstInst));

   /*
    * Put Driver Specific code here.
    */
   uUnit = pInst->uUnitNumber;

   /* check the state of the card */
   if ( Card[uUnit].State != SMC_ATR_PARSED )
   {
      return CNXT_SMC_NOT_AVAILABLE;
   }

   switch ( Item )
   {
      case CNXT_SMC_CONFIG_CONVENTION:
         *puValue = (u_int32)Card[uUnit].Config.uConvention;
         break;

      case CNXT_SMC_CONFIG_PROTOCOL:
         *puValue = (u_int32)Card[uUnit].Config.uProtocol;
         break;
            
      case CNXT_SMC_CONFIG_FI:
         *puValue = (u_int32)Card[uUnit].Config.uFI;
         break;

      case CNXT_SMC_CONFIG_DI:
         *puValue = (u_int32)Card[uUnit].Config.uDI;
         break;

      case CNXT_SMC_CONFIG_PI1:
         *puValue = (u_int32)Card[uUnit].Config.uPI1;
         break;

      case CNXT_SMC_CONFIG_PI2:
         *puValue = (u_int32)Card[uUnit].Config.uPI2;
         break;

      case CNXT_SMC_CONFIG_II:
         *puValue = (u_int32)Card[uUnit].Config.uII;
         break;

      case CNXT_SMC_CONFIG_N:
         *puValue = (u_int32)Card[uUnit].Config.uN;
         break;
            
      case CNXT_SMC_CONFIG_HISTORICAL:
         *puValue = (u_int32)Card[uUnit].Config.pHistorical;
         break;

      case CNXT_SMC_CONFIG_HISTORICAL_LEN:
         *puValue = (u_int32)Card[uUnit].Config.uHistoricalLength;
         break;

      case CNXT_SMC_CONFIG_TIMEOUT:
         *puValue = (u_int32)Card[uUnit].Config.uTimeout;
         break;

      case CNXT_SMC_CONFIG_RETRY:
         *puValue = (u_int32)Card[uUnit].Config.uRetry;
         break;

      default:
         return CNXT_SMC_BAD_PARAMETER;
   }


   return CNXT_SMC_OK;
}  /* end cnxt_smc_get_config */

/*********************************************************************/
/*  cnxt_smc_read_write ()                                           */
/*                                                                   */
/*  PARAMETERS: CNXT_SMC_HANDLE Handle,                              */
/*              bool            bAsync,                              */
/*              void            *pOutBuf,                            */
/*              u_int32         uOutLength                           */
/*              void            *pInBuf                              */
/*              u_int32         *pInLength                           */
/*              void            *Tag )                               */
/*                                                                   */
/*  DESCRIPTION: This function performs data transaction between     */
/*               application and smart card.                         */
/*                                                                   */
/*  RETURNS: CNXT_SMC_OK if success                                  */
/*           CNXT_SMC_NOT_INIT if driver is not initialized          */
/*           CNXT_SMC_BAD_HANDLE if Handle is invalid                */
/*           CNXT_SMC_NOT_AVAILABLE if card reset not done           */
/*           CNXT_SMC_BAD_PARAMETER if any of input argument is      */
/*                                     invalid                       */
/*           CNXT_SMC_TIMEOUT if transaction failed in sync mode     */
/*********************************************************************/
CNXT_SMC_STATUS cnxt_smc_read_write ( CNXT_SMC_HANDLE Handle,
                                      bool            bAsync,
                                      void            *pOutBuf,
                                      u_int32         uOutLength,
                                      void            *pInBuf,
                                      u_int32         *pInLength,
                                      void            *Tag )
{
   CNXT_SMC_INST *pInst = (CNXT_SMC_INST*)Handle;
   u_int32 uUnit;
   SMC_RW_JOB_DESCRIPTOR *pRwJob, *pJobNode;
   bool bKs;
   CNXT_SMC_STATUS RetCode = CNXT_SMC_OK;

   IS_DRIVER_INITED(SMC,pDriverInst->bInit);
   IS_VALID_HANDLE(SMC, pInst, &(pDriverInst->pUnitInst[pInst->uUnitNumber].pFirstInst));

   if ( ( ( pOutBuf == NULL ) || ( uOutLength == 0 ) ) && 
        ( ( pInBuf == NULL ) || ( pInLength == NULL ) || ( *pInLength == 0 ) ) )
   {
      return CNXT_SMC_BAD_PARAMETER;
   }

   uUnit = pInst->uUnitNumber;

   /* check the state of the card */
   if ( Card[uUnit].State != SMC_ATR_PARSED )
   {
      return CNXT_SMC_NOT_AVAILABLE;
   }

   /* set up Rw job */
   bKs = critical_section_begin ();
   if ( ( pRwJob = cnxt_smc_new_rw_job () ) == NULL )
   {
      critical_section_end(bKs);
      return CNXT_SMC_RESOURCE_ERROR;
   }
   critical_section_end(bKs);
  
#ifdef TFCAS
	CNXT_SET_VAL ( SMC_CHTIME_REG_ADDR ( uUnit ), SMC_TIME_CYCLES_MASK, 9600 );
	CNXT_SET_VAL ( SMC_BLKTIME_REG_ADDR ( uUnit ), SMC_TIME_CYCLES_MASK, 9600 );
#endif

   pRwJob->pInst        = pInst;
   pRwJob->bAsync       = bAsync;
   pRwJob->pTxPtr       = pOutBuf;
   pRwJob->uBytesToTx   = uOutLength;
   pRwJob->pRxPtr       = pInBuf;
   pRwJob->pBytesRecved = pInLength;
   pRwJob->pRxBufEnd    = (u_int8*)pInBuf + *pInLength;
   pRwJob->Tag          = Tag;
   pRwJob->pNext        = NULL;
   *(pRwJob->pBytesRecved) = 0;

   /* add the job to the execution list */
   bKs = critical_section_begin ();
   if ( Card[uUnit].pRwJob )
   {
      pJobNode = Card[uUnit].pRwJob;
      while ( pJobNode->pNext )
      {
         pJobNode = pJobNode->pNext;
      }
      pJobNode->pNext = pRwJob;
   }
   else
   {
      Card[uUnit].pRwJob = pRwJob;
      smc_start_rw_job ( pRwJob );
   }
   critical_section_end ( bKs );

   if ( ( bAsync == FALSE ) && pRwJob->SyncSem )
   {
      if ( sem_get ( pRwJob->SyncSem, KAL_WAIT_FOREVER ) == RC_OK )
      {
         /* rw job finished */
         RetCode = pRwJob->Status;
         bKs = critical_section_begin ();
         cnxt_smc_free_rw_job ( pRwJob );
         critical_section_end ( bKs );
      }
      else
      {
         RetCode = CNXT_SMC_INTERNAL_ERROR;
      }
   }

#ifdef TFCAS
	CNXT_SET_VAL ( SMC_CHTIME_REG_ADDR ( uUnit ), SMC_TIME_CYCLES_MASK, 9600 );
	CNXT_SET_VAL ( SMC_BLKTIME_REG_ADDR ( uUnit ), SMC_TIME_CYCLES_MASK, 9600 );
#endif

   return RetCode;
}

/* the following function is for testing only */
#ifdef DRIVER_INCL_SMCTEST
int printf ( char *, ... );
   
void show_rw_jobs ( void )
{
   SMC_RW_JOB_DESCRIPTOR *pRwJob;
   int i;

   for ( i = 0 ; i < CNXT_SMC_NUM_UNITS; i ++ )
   {
      pRwJob = Card[i].pRwJob;

      while ( pRwJob )
      {
         printf ( "job 0x%x pending on card[%d]\n", pRwJob, i );
         pRwJob = pRwJob->pNext;
      }
   }

   pRwJob = pDriverInst->pFreeRwJobHead;

   while ( pRwJob )
   {
      printf ( "job 0x%x is free\n", pRwJob );
      pRwJob = pRwJob->pNext;
   }

   printf ( "tail of free job is 0x%x\n", pDriverInst->pFreeRwJobTail );
}
#endif      
      
/****************************************************************************
 * Modifications:
 * $Log: 
 *  9    mpeg      1.8         9/17/03 3:58:42 PM     Lucy C Allevato SCR(s) 
 *        7482 :
 *        update the smartcard module to use the new handle lib.
 *        
 *  8    mpeg      1.7         8/28/03 2:45:32 PM     Larry Wang      SCR(s) 
 *        7078 :
 *        Write both ACTIVATE and DEACTIVATE bit at cold reset of the card to 
 *        reset activation timer.
 *        
 *  7    mpeg      1.6         4/23/03 4:20:40 PM     Larry Wang      SCR(s) 
 *        6085 :
 *        In cnxt_smc_term(), remove critical section and delete semaphore 
 *        without putting it first.
 *        
 *  6    mpeg      1.5         2/4/03 2:36:42 PM      Larry Wang      SCR(s) 
 *        5324 :
 *        
 *        
 *  5    mpeg      1.4         2/4/03 1:46:14 PM      Larry Wang      SCR(s) 
 *        5324 :
 *        
 *        
 *  4    mpeg      1.3         2/4/03 12:17:38 PM     Larry Wang      SCR(s) 
 *        5324 :
 *        
 *        
 *  3    mpeg      1.2         1/31/03 3:39:42 PM     Larry Wang      SCR(s) 
 *        5324 :
 *        
 *        
 *  2    mpeg      1.1         1/28/03 9:28:32 AM     Larry Wang      SCR(s) 
 *        5324 :
 *        
 *        
 *  1    mpeg      1.0         1/27/03 12:43:20 PM    Larry Wang      
 * $
 * 
 *    Rev 1.8   17 Sep 2003 14:58:42   goldenx
 * SCR(s) 7482 :
 * update the smartcard module to use the new handle lib.
 * 
 *    Rev 1.7   28 Aug 2003 13:45:32   wangl2
 * SCR(s) 7078 :
 * Write both ACTIVATE and DEACTIVATE bit at cold reset of the card to reset activation timer.
 * 
 *    Rev 1.6   23 Apr 2003 15:20:40   wangl2
 * SCR(s) 6085 :
 * In cnxt_smc_term(), remove critical section and delete semaphore without putting it first.
 * 
 *    Rev 1.5   04 Feb 2003 14:36:42   wangl2
 * SCR(s) 5324 :
 * 
 * 
 *    Rev 1.4   04 Feb 2003 13:46:14   wangl2
 * SCR(s) 5324 :
 * 
 * 
 *    Rev 1.3   04 Feb 2003 12:17:38   wangl2
 * SCR(s) 5324 :
 * 
 * 
 *    Rev 1.2   31 Jan 2003 15:39:42   wangl2
 * SCR(s) 5324 :
 * 
 * 
 ****************************************************************************/

