/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*          Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002          */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        smcdrv.c
 *
 *
 * Description:     Interrupt Handler for Generic Smart Card Driver
 *
 *
 * Author:          Larry Wang
 *
 ****************************************************************************/
/* $Header: smc_isr.c, 10, 9/17/03 4:08:28 PM, Lucy C Allevato$
 ****************************************************************************/

/*****************/
/* Include Files */
/*****************/
#include "stbcfg.h"
#include "hwlib.h"
#include "kal.h"
#include "retcodes.h"
#include "gpioext.h"
#if CUSTOMER==VENDOR_D
#include "vendor_d.h"
#endif
#include "handle.h"
#include "smartcard.h"
#include "smc_priv.h"

/*******************/
/* Local Variables */
/*******************/        

/********************/
/* Global Variables */
/********************/

/*********************************************************************/
/*  cnxt_smc_isr ()                                                  */
/*                                                                   */
/*  PARAMETERS:                                                      */
/*                                                                   */
/*  DESCRIPTION:                                                     */
/*                                                                   */
/*  RETURNS:                                                         */
/*********************************************************************/
static void smc_stabilize_card_irq ( u_int32 slot_id )
{
   int iBank;
   
   iBank = BANK_FROM_SLOT_ID ( slot_id );
   
   /* Clear all Interrupts except CardIn and CardOut. */
   *(LPREG)(SMC_INT_STAT_REG_ADDR(iBank)) = 0x3FFC;

#ifdef SCARD_FIX
   /* Clear Loopback mode. */
   CNXT_SET_VAL ( SMC_TERM_CTRL_REG_ADDR ( iBank ), SMC_TERM_CTRL_LOOPBACK_MASK,
                  SMC_DISABLE );
#endif

   /* Set mode Tx. */
   CNXT_SET_VAL ( SMC_TERM_CTRL_REG_ADDR ( iBank ), SMC_TERM_CTRL_MODE_MASK,
                  SMC_MODE_TX);

   /* disable Rx, Tx interrupts. */
   CNXT_SET_VAL ( SMC_RXTIDE_REG_ADDR ( iBank ), SMC_TIDE_LEVEL_MASK, 0x1f );
   CNXT_SET_VAL ( SMC_TXTIDE_REG_ADDR ( iBank ), SMC_TIDE_LEVEL_MASK, 0 );

   /* flush the RxFIFO, TxFIFO buffers. */
   CNXT_SET_VAL ( SMC_RXCOUNT_REG_ADDR ( iBank ), SMC_COUNT_COUNT_MASK, 0 );
   CNXT_SET_VAL ( SMC_TXCOUNT_REG_ADDR ( iBank ), SMC_COUNT_COUNT_MASK, 0 );
}

static bool smc_rw_job_complete ( SMC_DESCRIPTOR *pCard )
{
   SMC_RW_JOB_DESCRIPTOR *pRwJob = pCard->pRwJob;
   
   if ( ( pRwJob->pRxPtr >= pRwJob->pRxBufEnd ) &&
        ( pRwJob->uBytesToTx == 0 ) )
   {
      /* RW is done */
      pCard->pRwJob = pRwJob->pNext;

      smc_term_rw_job ( pRwJob, CNXT_SMC_OK );

      return TRUE;
   }

   return FALSE;
}

static void smc_rw_job_abort ( SMC_DESCRIPTOR *pCard )
{
   SMC_RW_JOB_DESCRIPTOR *pRwJob = pCard->pRwJob;

   if ( pRwJob == NULL )
   {
      return;
   }

   /* move to the next */
   pCard->pRwJob = pRwJob->pNext;

   smc_term_rw_job ( pRwJob, CNXT_SMC_TIMEOUT );
}

/*********************************************************************/
/*  cnxt_smc_isr ()                                                  */
/*                                                                   */
/*  PARAMETERS:                                                      */
/*                                                                   */
/*  DESCRIPTION:                                                     */
/*                                                                   */
/*  RETURNS:                                                         */
/*********************************************************************/
u_int32 cnxt_smc_isr ( u_int32 int_id, bool fiq, PFNISR *fn_chain )
{
   u_int32 slot_id, iBank, bytes_in_fifo;
   u_int32 int_stat;
   HW_DWORD incoming_data;
   SMC_DESCRIPTOR *pCard;
   CNXT_SMC_INST  *pInst;
   SMC_RW_JOB_DESCRIPTOR *pRwJob;

   /* figure out card slot */
   if ( int_id == INT_VENDOR_SLOT_0 )
   {
      slot_id = 0;
   }
   else if ( int_id == INT_VENDOR_SLOT_1 )
   {
      slot_id = 1;
   }
   else
   {
      return RC_ISR_NOTHANDLED;
   }

   pCard = cnxt_smc_get_card_descriptor ( slot_id );
   iBank = BANK_FROM_SLOT_ID ( slot_id );

   /* get value of interrupt status register */
   int_stat = *(LPREG)SMC_INT_QSTAT_REG_ADDR ( iBank );

   /* clear all the interrupts */
   *(LPREG)SMC_INT_STAT_REG_ADDR ( iBank ) = int_stat;

   /* check if the attached driver is initialized */
   if ( cnxt_smc_check_drv_init () == FALSE )
   {
      return RC_ISR_HANDLED;
   }

   /*                   */
   /* handle interrupts */
   /*                   */

   /* CARD_IN */
   if ( int_stat & SMC_INT_CARDIN_MASK )
   {
      pCard->State = SMC_INSERT;
      cnxt_smc_notify_unit_clients ( CNXT_SMC_EVENT_CARD_INSERTED, slot_id ); 
   }

   /* CARD_OUT */
   if ( int_stat & SMC_INT_CARDOUT_MASK )
   {
      pCard->State = SMC_NOT_INSERT;
      cnxt_smc_notify_unit_clients ( CNXT_SMC_EVENT_CARD_REMOVED, slot_id );
       
      /* clear all pending jobs */
      while ( pCard->pRwJob )
      {
         pRwJob = pCard->pRwJob;
         pCard->pRwJob = pRwJob->pNext;

         smc_term_rw_job ( pRwJob, CNXT_SMC_TIMEOUT );
      }
      pCard->ResetJob.pInst = NULL;
      pCard->PowerdownJob.pInst = NULL;

      smc_stabilize_card_irq ( slot_id );
   }

   /* CARD powered off */
   if ( int_stat & SMC_INT_CARDUNPOWERED_MASK )
   {
      if ( pCard->State != SMC_NOT_INSERT )
      {
         cnxt_smc_notify_unit_clients ( CNXT_SMC_EVENT_CARD_POWER_DOWN_COMPLETE, 
                                        slot_id );
         /* release sync powerdown job */
         if ( pCard->PowerdownJob.pInst && 
              ( pCard->PowerdownJob.bAsync == FALSE ) &&
              pCard->PowerdownJob.SyncSem )
         {
            sem_put ( pCard->PowerdownJob.SyncSem );
         }
         pCard->State = SMC_MUTE;
      }

      pCard->PowerdownJob.pInst = NULL;
   }

   /* ATR timeout */
   if ( ( int_stat & SMC_INT_ATRSTARTTIMEOUT_MASK ) || 
        ( int_stat & SMC_INT_ATRDURATIONTIMEOUT_MASK ) )
   {
      /* check voltage */
      if ( pCard->Vcc == SMC_VCC_3V )
      {
         /* try 5V */
         pCard->Vcc = SMC_VCC_5V;
         CNXT_SET_VAL ( SMC_CONFIG_REG_ADDR ( iBank ), 
                        SMC_CONFIG_VCC_VALUE_MASK, 
                        SMC_VCC_5V );

#if SMC_VOLT_CTRL==SMC_VOLT_CTRL_EXT
         /* 8004 voltage control on Abilene/Athens (CNXT HW) only */
         set_gpio_output_high ( ( slot_id == 0 ) ?
                                PIN_GPIO_SMC_0_VOLTAGE_CTRL:
                                PIN_GPIO_SMC_1_VOLTAGE_CTRL );
#endif
         /* update card descriptor */
         pCard->State      = SMC_INSERT;
         pCard->uAtrLength = 0;

         /* warm reset */
         *(LPREG)( SMC_ICC_CTRL_REG_ADDR ( iBank ) ) 
                                        = SMC_ICC_CTRL_WARMRESET_MASK;
      }
      else
      {
         /* card is bad, power it down */
         *(LPREG)SMC_INT_MASK_REG_ADDR ( iBank ) = SMC_INT_CARDIN_MASK | 
                                                   SMC_INT_CARDOUT_MASK | 
                                                   SMC_INT_CARDUNPOWERED_MASK;

         *(LPREG)( SMC_ICC_CTRL_REG_ADDR ( iBank ) ) 
                                        = SMC_ICC_CTRL_DEACTIVATECARD_MASK;

         /* notification the resetting client */
         pInst = (pCard->ResetJob).pInst;
         if ( pInst && pCard->ResetJob.bAsync && pInst->pNotifyFn )
         {
            pInst->pNotifyFn ( (CNXT_SMC_HANDLE)pInst,
                               pInst->pUserData,
                               CNXT_SMC_EVENT_CARD_RESET_TIMEOUT,
                               NULL,
                               pCard->ResetJob.Tag );
         }
         (pCard->ResetJob).pInst = NULL;
      }
      cnxt_smc_notify_unit_clients ( CNXT_SMC_EVENT_ATR_TIMEOUT, slot_id );
   }

   /* RXREAD */
   if ( int_stat & SMC_INT_RXREAD_MASK )
   {
      bytes_in_fifo = CNXT_GET_VAL ( SMC_RXCOUNT_REG_ADDR ( iBank ),
                                     SMC_COUNT_COUNT_MASK );

      if ( pCard->State < SMC_ATR_RECEIVED )
      {
         /* receive ATR */
         while ( bytes_in_fifo -- )
         {
            incoming_data = *(LPREG)SMC_DATA_REG_ADDR ( iBank );

            /* search for valid TS byte */
            if ( ( pCard->uAtrLength == 0 ) &&
                 ( ( incoming_data & SMC_DATA_DATA_MASK ) == 0x60 ) )
            {
               /* NULL byte, skip */
               continue;
            }

            if ( pCard->uAtrLength == 0 )
            {
               if ( ( ( incoming_data & SMC_DATA_DATA_MASK ) != 0x3b ) &&
                    ( ( incoming_data & SMC_DATA_DATA_MASK ) != 0x3f ) )
               {
                  /*
                   * oops, the card can be irdeto card (T14), initial etu has
                   * to be 625/clk
                   */
//                  CNXT_SET_VAL ( SMC_FD_REG_ADDR(iBank), SMC_FD_F_VALUE_MASK,
//                                 625 );
                  /* flush Rx fifo */
                  CNXT_SET_VAL ( SMC_RXCOUNT_REG_ADDR ( iBank ), 
                                 SMC_COUNT_COUNT_MASK, 0 );
                  /* re-initiate reset */
                  *(LPREG)(SMC_ICC_CTRL_REG_ADDR (iBank)) 
                                          = SMC_ICC_CTRL_WARMRESET_MASK;

                  return RC_ISR_HANDLED;
               }

               /* disable atr timers. */
               CNXT_SET_VAL ( SMC_INT_MASK_REG_ADDR ( iBank ), 
                              SMC_INT_ATRSTARTTIMEOUT_MASK, SMC_DISABLE );
            }
#ifdef SMC_PARITY_CHECK_ON_ATR
            else if ( incoming_data & SMC_DATA_PARITY_MASK )
            {
#ifdef DRIVER_INCL_TRACE
               isr_trace_new ( TRACE_LEVEL_3 | TRACE_SMC,
                               "cnxt_smc_isr: ATR Parity Error!\n", 0, 0 );                                                                
#endif
               /* power the card down */
               *(LPREG)SMC_INT_MASK_REG_ADDR ( iBank ) = SMC_INT_CARDIN_MASK | 
                                                         SMC_INT_CARDOUT_MASK | 
                                                         SMC_INT_CARDUNPOWERED_MASK; 
               *(LPREG)( SMC_ICC_CTRL_REG_ADDR ( iBank ) ) 
                                             = SMC_ICC_CTRL_DEACTIVATECARD_MASK;

               /* notify resetting client */
               pInst = (pCard->ResetJob).pInst;
               if ( pInst && pCard->ResetJob.bAsync && pInst->pNotifyFn )
               {
                  pInst->pNotifyFn ( (CNXT_SMC_HANDLE)pInst,
                                     pInst->pUserData,
                                     CNXT_SMC_EVENT_CARD_RESET_TIMEOUT,
                                     NULL,
                                     pCard->ResetJob.Tag );
               }
               (pCard->ResetJob).pInst = NULL;
               return RC_ISR_HANDLED;
            }
#endif
            
            if ( pCard->uAtrLength < SMC_ATR_MAX_LENGTH )
            {
               pCard->uAtr[pCard->uAtrLength++]
                                           = incoming_data & SMC_DATA_DATA_MASK;
            }
         }

         /* check integrity of ATR */
         if ( smc_atr_complete ( pCard ) )
         {
            /* disable atr timers. */
            CNXT_SET_VAL ( SMC_INT_MASK_REG_ADDR ( iBank ), 
                           SMC_INT_ATRDURATIONTIMEOUT_MASK, SMC_DISABLE );

            /* got all bytes of ATR */
            pCard->State = SMC_ATR_RECEIVED;

            /* parse ATR */
            if ( smc_parse_atr ( slot_id, pCard ) )
            {
               pCard->State = SMC_ATR_PARSED;

               /* notify resetting client */
               pInst = (pCard->ResetJob).pInst;
               //For CTI CA system
               cnxt_smc_notify_unit_clients ( CNXT_SMC_EVENT_CARD_ATR_RECIEVED, slot_id ); 
 
               if ( pInst )
               {
                  if ( pCard->ResetJob.bAsync )
                  {
                     if ( pInst->pNotifyFn )
                     {
                        pInst->pNotifyFn ( (CNXT_SMC_HANDLE)pInst,
                                           pInst->pUserData,
                                           CNXT_SMC_EVENT_CARD_RESET_COMPLETE,
                                           NULL,
                                           pCard->ResetJob.Tag );
                     }
                  }
                  else
                  {
                     if ( pCard->ResetJob.SyncSem )
                     {
                        sem_put ( pCard->ResetJob.SyncSem );
                     }
                  }
               }
            }
            else
            {
               /* bad ATR, power down the card */
               *(LPREG)SMC_INT_MASK_REG_ADDR ( iBank ) = SMC_INT_CARDIN_MASK | 
                                                         SMC_INT_CARDOUT_MASK | 
                                                         SMC_INT_CARDUNPOWERED_MASK; 
               *(LPREG)( SMC_ICC_CTRL_REG_ADDR ( iBank ) ) 
                                             = SMC_ICC_CTRL_DEACTIVATECARD_MASK;

               /* notify resetting client */
               pInst = (pCard->ResetJob).pInst;
               if ( pInst && pCard->ResetJob.bAsync && pInst->pNotifyFn )
               {
                  pInst->pNotifyFn ( (CNXT_SMC_HANDLE)pInst,
                                     pInst->pUserData,
                                     CNXT_SMC_EVENT_CARD_RESET_TIMEOUT,
                                     NULL,
                                     pCard->ResetJob.Tag );
               }
            }

            /* clear reset job */
            (pCard->ResetJob).pInst = NULL;
         }
         else
         {
            if ( pCard->uAtrLength == SMC_ATR_MAX_LENGTH )
            {
#ifdef DRIVER_INCL_TRACE
               isr_trace_new ( TRACE_LEVEL_3 | TRACE_SMC,
                               "cnxt_smc_isr: ATR exceeds max length!\n", 0, 0 );                                                                
#endif
               /* power the card down */
               *(LPREG)SMC_INT_MASK_REG_ADDR ( iBank ) = SMC_INT_CARDIN_MASK | 
                                                         SMC_INT_CARDOUT_MASK | 
                                                         SMC_INT_CARDUNPOWERED_MASK; 
               *(LPREG)( SMC_ICC_CTRL_REG_ADDR ( iBank ) ) = SMC_ICC_CTRL_DEACTIVATECARD_MASK;

               /* notify resetting client */
               pInst = (pCard->ResetJob).pInst;
               if ( pInst && pCard->ResetJob.bAsync && pInst->pNotifyFn )
               {
                  pInst->pNotifyFn ( (CNXT_SMC_HANDLE)pInst,
                                     pInst->pUserData,
                                     CNXT_SMC_EVENT_CARD_RESET_TIMEOUT,
                                     NULL,
                                     pCard->ResetJob.Tag );
               }
               (pCard->ResetJob).pInst = NULL;
            }
         }
      }
      else
      {
         /* receive response from card */
         while ( bytes_in_fifo -- )
         {
            /* get incoming byte */
            incoming_data = *(LPREG)SMC_DATA_REG_ADDR ( iBank );

            /* throw it away if there no pending reading or not buffer available */
            if ( ( pCard->pRwJob == NULL ) ||
                 ( pCard->pRwJob->pInst == NULL ) ||
                 ( pCard->pRwJob->pRxPtr == NULL ) ||
                 ( pCard->pRwJob->pRxPtr >= pCard->pRwJob->pRxBufEnd ) )
            {
               smc_stabilize_card_irq ( slot_id );
               break;
            }

            /* check parity error */
            if ( pCard->Config.uProtocol != CNXT_SMC_PROTOCOL_T14 )
            {
               if ( incoming_data & SMC_DATA_PARITY_MASK )
               {
#ifdef DRIVER_INCL_TRACE
                  isr_trace_new ( TRACE_LEVEL_3 | TRACE_SMC,
                                  "cnxt_smc_isr: ATR Parity Error!\n", 0, 0 );                                                                
#endif

                  /* abort reading */
                  smc_rw_job_abort ( pCard );

                  smc_stabilize_card_irq ( slot_id );

                  /* start next RW job if there is any */
                  if ( pCard->pRwJob )
                  {
                     while ( smc_start_rw_job ( pCard->pRwJob ) == FALSE )
                     {
                        /* pCard->pRwJob is done */
                        pRwJob = pCard->pRwJob;
                        pCard->pRwJob = pRwJob->pNext;

                        smc_term_rw_job ( pRwJob, CNXT_SMC_OK );

                        if ( pCard->pRwJob == NULL )
                        {
                           break;
                        }
                     }
                  }

                  break;
               }
            }

            /* save the byte */
            #ifdef TFCAS
            if( ((incoming_data&0xFF) == 0x60) && (*(pCard->pRwJob->pBytesRecved) == 0) )
            	continue;
			if(*(pCard->pRwJob->pBytesRecved) == 0)
			{
				CNXT_SET_VAL ( SMC_CHTIME_REG_ADDR ( iBank ), SMC_TIME_CYCLES_MASK, 200 );
				CNXT_SET_VAL ( SMC_BLKTIME_REG_ADDR ( iBank ), SMC_TIME_CYCLES_MASK, 200 );
			}
			#endif
            *(pCard->pRwJob->pRxPtr++) = incoming_data & SMC_DATA_DATA_MASK;
            *(pCard->pRwJob->pBytesRecved) = *(pCard->pRwJob->pBytesRecved) + 1;

            /* check if all expected bytes are received */
            if ( smc_rw_job_complete ( pCard ) )
            {
               smc_stabilize_card_irq ( slot_id );
               /* start next RW job if there is any */
               if ( pCard->pRwJob )
               {
                  while ( smc_start_rw_job ( pCard->pRwJob ) == FALSE )
                  {
                     /* pCard->pRwJob is done */
                     pRwJob = pCard->pRwJob;
                     pCard->pRwJob = pRwJob->pNext;

                     smc_term_rw_job ( pRwJob, CNXT_SMC_OK );

                     if ( pCard->pRwJob == NULL )
                     {
                        break;
                     }
                  }
               }

               break;
            }
         }
      }
   }

   if ( int_stat & SMC_INT_TXTIDE_MASK )
   {
   #ifdef DVNCA
      if ( pCard->pRwJob->uBytesToTx != 0 )
      	{
      	#endif
      if ( pCard->pRwJob && !CNXT_GET_VAL ( SMC_TXCOUNT_REG_ADDR ( iBank ), SMC_COUNT_COUNT_MASK ))
      {
         bytes_in_fifo = pCard->pRwJob->uBytesToTx;

         if ( bytes_in_fifo > NUM_SMC_DATA )
         {
            bytes_in_fifo = NUM_SMC_DATA;
         }
#ifdef SCARD_FIX
         else if ( bytes_in_fifo > 1 )
         {
            bytes_in_fifo --;
         }

         if ( pCard->pRwJob->uBytesToTx > 1 )
         {
#endif
            while ( bytes_in_fifo -- )
            {
               CNXT_SET_VAL ( SMC_DATA_REG_ADDR ( iBank ), SMC_DATA_DATA_MASK,
                              *(pCard->pRwJob->pTxPtr) );
               pCard->pRwJob->uBytesToTx --;
               pCard->pRwJob->pTxPtr ++;
            }
#ifdef SCARD_FIX
         }
         else if ( pCard->pRwJob->uBytesToTx == 1 )
         {
            /* last char */
            smc_tx_last_char ( pCard, iBank );
         }
#endif /* SCARD_FIX */

         if ( pCard->pRwJob->uBytesToTx == 0 )
         {
            /* write is done, start read */
            if ( pCard->pRwJob->pRxPtr && 
                 ( pCard->pRwJob->pRxPtr < pCard->pRwJob->pRxBufEnd ) )
            {
               CNXT_SET_VAL ( SMC_TXTIDE_REG_ADDR ( iBank ), SMC_TIDE_LEVEL_MASK, 0 );
               CNXT_SET_VAL ( SMC_TERM_CTRL_REG_ADDR ( iBank ),
                              SMC_TERM_CTRL_MODE_MASK, SMC_MODE_RX );
               CNXT_SET_VAL ( SMC_RXTIDE_REG_ADDR ( iBank ), SMC_TIDE_LEVEL_MASK, 0 );
            }
            else
            {
               /* this rw job is done */
               pRwJob = pCard->pRwJob;
               pCard->pRwJob = pRwJob->pNext;

               smc_term_rw_job ( pRwJob, CNXT_SMC_OK );

               smc_stabilize_card_irq ( slot_id );

               if ( pCard->pRwJob )
               {
                  while ( smc_start_rw_job ( pCard->pRwJob ) == FALSE )
                  {
                     /* pCard->pRwJob is done */
                     pRwJob = pCard->pRwJob;
                     pCard->pRwJob = pRwJob->pNext;

                     smc_term_rw_job ( pRwJob, CNXT_SMC_OK );

                     if ( pCard->pRwJob == NULL )
                     {
                        break;
                     }
                  }
               }
            }
         }
      }
      #ifdef DVNCA
      	}
      #endif
   }

   if ( ( int_stat & SMC_INT_CHARRXTIMEOUT_MASK ) && pCard->pRwJob )
   {
      /* RW is done */
      pRwJob = pCard->pRwJob;
      pCard->pRwJob = pRwJob->pNext;

      smc_term_rw_job ( pRwJob, CNXT_SMC_OK );

      smc_stabilize_card_irq ( slot_id );

      /* start next RW job if there is any */
      if ( pCard->pRwJob )
      {
         while ( smc_start_rw_job ( pCard->pRwJob ) == FALSE )
         {
            /* pCard->pRwJob is done */
            pRwJob = pCard->pRwJob;
            pCard->pRwJob = pRwJob->pNext;

            smc_term_rw_job ( pRwJob, CNXT_SMC_OK );

            if ( pCard->pRwJob == NULL )
            {
               break;
            }
         }
      }
   }
      
   if ( int_stat & ( SMC_INT_BLKRXTIMEOUT_MASK |
                     SMC_INT_TXERROR_MASK | SMC_INT_RXERROR_MASK ) )
   {
      /* abort rw job */
      smc_rw_job_abort ( pCard );

      smc_stabilize_card_irq ( slot_id );

      /* start next RW job if there is any */
      if ( pCard->pRwJob )
      {
         while ( smc_start_rw_job ( pCard->pRwJob ) == FALSE )
         {
            /* pCard->pRwJob is done */
            pRwJob = pCard->pRwJob;
            pCard->pRwJob = pRwJob->pNext;

            smc_term_rw_job ( pRwJob, CNXT_SMC_OK );

            if ( pCard->pRwJob == NULL )
            {
               break;
            }
         }
      }
   }

   
   return RC_ISR_HANDLED;
}

/****************************************************************************
 * Modifications:
 * $Log: 
 *  10   mpeg      1.9         9/17/03 4:08:28 PM     Lucy C Allevato SCR(s) 
 *        7482 :
 *        update smartcard module to use the new handle lib.
 *        
 *  9    mpeg      1.8         8/28/03 2:29:32 PM     Larry Wang      SCR(s) 
 *        7078 :
 *        Clear reset/powerdown jobs on card removal interrupt.
 *        
 *  8    mpeg      1.7         2/17/03 3:59:26 PM     Larry Wang      SCR(s) 
 *        5525 :
 *        try Irdeto (T=14) card if ATR is invalid.
 *        
 *  7    mpeg      1.6         2/7/03 12:36:32 PM     Larry Wang      SCR(s) 
 *        5324 :
 *        Consider CHARRXTIMEOUT as the end of response instead of an error.
 *        
 *  6    mpeg      1.5         2/6/03 8:34:00 AM      Larry Wang      SCR(s) 
 *        5324 :
 *        Add "#ifdef SCARD_FIX" segments to fix "fast turnaround" problem.
 *        
 *  5    mpeg      1.4         2/5/03 12:50:30 PM     Larry Wang      SCR(s) 
 *        5324 :
 *        
 *        
 *  4    mpeg      1.3         2/4/03 2:36:42 PM      Larry Wang      SCR(s) 
 *        5324 :
 *        
 *        
 *  3    mpeg      1.2         2/4/03 12:17:40 PM     Larry Wang      SCR(s) 
 *        5324 :
 *        
 *        
 *  2    mpeg      1.1         1/31/03 3:39:44 PM     Larry Wang      SCR(s) 
 *        5324 :
 *        
 *        
 *  1    mpeg      1.0         1/27/03 12:43:40 PM    Larry Wang      
 * $
 * 
 *    Rev 1.9   17 Sep 2003 15:08:28   goldenx
 * SCR(s) 7482 :
 * update smartcard module to use the new handle lib.
 * 
 *    Rev 1.8   28 Aug 2003 13:29:32   wangl2
 * SCR(s) 7078 :
 * Clear reset/powerdown jobs on card removal interrupt.
 * 
 *    Rev 1.7   17 Feb 2003 15:59:26   wangl2
 * SCR(s) 5525 :
 * try Irdeto (T=14) card if ATR is invalid.
 * 
 *    Rev 1.6   07 Feb 2003 12:36:32   wangl2
 * SCR(s) 5324 :
 * Consider CHARRXTIMEOUT as the end of response instead of an error.
 * 
 *    Rev 1.5   06 Feb 2003 08:34:00   wangl2
 * SCR(s) 5324 :
 * Add "#ifdef SCARD_FIX" segments to fix "fast turnaround" problem.
 * 
 *    Rev 1.4   05 Feb 2003 12:50:30   wangl2
 * SCR(s) 5324 :
 * 
 * 
 *    Rev 1.3   04 Feb 2003 14:36:42   wangl2
 * SCR(s) 5324 :
 * 
 * 
 *    Rev 1.2   04 Feb 2003 12:17:40   wangl2
 * SCR(s) 5324 :
 * 
 * 
 *    Rev 1.1   31 Jan 2003 15:39:44   wangl2
 * SCR(s) 5324 :
 * 
 * 
 ****************************************************************************/

