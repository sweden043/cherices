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
 * Description:     Generic Smart card driver
 *
 *
 * Author:          Larry Wang
 *
 ****************************************************************************/
/* $Header: smc_util.c, 16, 10/27/03 9:32:56 AM, Larry Wang$
 ****************************************************************************/

/*****************/
/* Include Files */
/*****************/
#include <string.h>
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
static int16 baud_rate_table[] = { RFU, 1, 2, 4, 8, 16, 32, 
                                   RFU, 12, 20, RFU, RFU, 
                                   RFU, RFU, RFU, RFU};   /* Values defined in Protocol. */
/* ATR parameter Tables. */
static SMC_CLK_RATE_TAG clock_rate_table[] = { { 372, 4 },
                                               { 372, 5 },
                                               { 558, 6 },
                                               { 744, 8 },
                                               { 1116, 12 },
                                               { 1488, 16 },
                                               { 1860, 20 },
                                               { RFU, RFU },
                                               { RFU, RFU },
                                               { 512, 5 },
                                               { 768, 7 },
                                               { 1024, 10 },
                                               { 1536, 15 },
                                               { 2048, 20 },
                                               { RFU, RFU },
                                               { RFU, RFU } };  /* Fi, Fmax values defined in Protocol. */

/***************************************/
/* Smart Card Driver Utility functions */
/***************************************/
void smc_access_enable ( void )
{
#if GPIO_CONFIG == GPIOM_COLORADO

#if (CUSTOMER == VENDOR_D) && (defined(OPENTV_EN2))
   /* SC1 interfaces with the SC socket */
   *((LPREG)PLL_CONFIG1_REG) |= PLL_CONFIG1_SC1PIN_MUX_SELECT_SCSOCKET;
#else
   /* SC0 connected to PIO pins. */
   *((LPREG)PLL_CONFIG1_REG) &= ~PLL_CONFIG1_SC0_PIN_MUX_SELECT_MASK;
   *((LPREG)PLL_CONFIG1_REG) |= PLL_CONFIG1_SC0PIN_MUX_SELECT_SC0;
#endif

#if PLL_PIN_ALT_FUNC_REG_DEFAULT == NOT_DETERMINED
   /* make sure Uart2 does not interfere */
   *((LPREG)PLL_PIN_ALT_FUNC_REG) |= PLL_PIN_ALT_FUNC_UART2_SEL_MASK;
#endif /* PLL_PIN_ALT_FUNC_REG_DEFAULT == NOT_DETERMINED  */

   /* Disable the Pin Current Overload Protection */
   *((LPREG)PLL_PIN_SC1_PWR_REG) &= ~PLL_PIN_SC1_PWR_VCCCUR_OLOAD_PROT_ENABLE;

   /*
    * This is required for Colorado Rev C
    */
#if (CHIP_REV == AUTOSENSE) || (CHIP_REV == REV_C_COLO)
   if ( ISCOLORADOREVC )
   {
      /* Enable the Voltage Regulator */
      *((LPREG)PLL_PIN_SC1_PWR_REG) &= ~PLL_PIN_SC1_PWR_VR_ENABLE_MASK;
      /* Clear Power interrupts */
      *((LPREG)PLL_PIN_SC1_PWR_REG) |= PLL_PIN_SC1_PWR_CLEAR_OLOAD_STATUS_MASK;
      task_time_sleep(10);
      *((LPREG)PLL_PIN_SC1_PWR_REG) |= PLL_PIN_SC1_PWR_CLEAR_OLOAD_STATUS_MASK;
      /* Clear the PIC bit */
      *((LPREG)ITC_INTSTATCLR_REG) |= ITC_SCR1;
   }
#endif

#else /* (GPIO_CONFIG != GPIOM_COLORADO) */
   /* SC0 is not used. So let the reg bit be at default */
   /* SC1 is used. So enable Path (Clear the bit) */
   /*modified by txl ,3-29-2005*/
  // *((LPREG)PLL_CONFIG0_REG) &= ~0x40000000;
  *((LPREG)PLL_CONFIG0_REG) &= ~0x00000000;
  /*end modified*/
#endif
}

void smc_hw_init ( SMC_DESCRIPTOR *pCard, u_int32 uSlotId )
{
   u_int32 uChip;
   u_int8  uChipRev;

   u_int32 uBank = BANK_FROM_SLOT_ID ( uSlotId );

   /* set convention */
   if ( pCard->Config.uConvention == CNXT_SMC_CONV_DIRECT )
   {
      CNXT_SET_VAL ( SMC_CONV_REG_ADDR ( uBank ), SMC_CONV_SENSE_MASK, SMC_SENSE_DIRECT );
      CNXT_SET_VAL ( SMC_CONV_REG_ADDR ( uBank ), SMC_CONV_ORDER_MASK, SMC_ORDER_DIRECT );
   }
   else
   {
      CNXT_SET_VAL ( SMC_CONV_REG_ADDR ( uBank ), SMC_CONV_SENSE_MASK, SMC_SENSE_INVERSE );
      CNXT_SET_VAL ( SMC_CONV_REG_ADDR ( uBank ), SMC_CONV_ORDER_MASK, SMC_ORDER_INVERSE );
   }

   /* set parity */
   CNXT_SET_VAL ( SMC_PARITY_REG_ADDR ( uBank ), SMC_PARITY_TXPARITY_MASK, SMC_PARITY_EVEN );
   CNXT_SET_VAL ( SMC_PARITY_REG_ADDR ( uBank ), SMC_PARITY_ENABLETXNAK_MASK, SMC_DISABLE );
   CNXT_SET_VAL ( SMC_PARITY_REG_ADDR ( uBank ), SMC_PARITY_RXPARITY_MASK, SMC_PARITY_EVEN );
   CNXT_SET_VAL ( SMC_PARITY_REG_ADDR ( uBank ), SMC_PARITY_ENABLERXNAK_MASK, SMC_DISABLE );

   /* flush TxFIFO */
   CNXT_SET_VAL ( SMC_TXCOUNT_REG_ADDR ( uBank ), SMC_COUNT_COUNT_MASK, 0 );

   /* set RXTIDE to get ATR */
   CNXT_SET_VAL ( SMC_RXTIDE_REG_ADDR ( uBank ), SMC_TIDE_LEVEL_MASK, 0 );

   /* flush RxFIFO */
   CNXT_SET_VAL ( SMC_RXCOUNT_REG_ADDR ( uBank ), SMC_COUNT_COUNT_MASK, 0 );

   /* set RXTIME, This time should be just lesser or equal than the CHTime. */
   CNXT_SET_VAL ( SMC_RXTIME_REG_ADDR ( uBank ), SMC_RXTIME_CYCLES_MASK, 9600 );

   /* set TERM_CTRL */
   CNXT_SET_VAL ( SMC_TERM_CTRL_REG_ADDR ( uBank ), SMC_TERM_CTRL_MODE_MASK, SMC_MODE_RX );
   CNXT_SET_VAL ( SMC_TERM_CTRL_REG_ADDR ( uBank ), SMC_TERM_CTRL_STOPCLK_MASK, SMC_DISABLE );
   CNXT_SET_VAL ( SMC_TERM_CTRL_REG_ADDR ( uBank ), SMC_TERM_CTRL_LOOPBACK_MASK, SMC_DISABLE );

   /* set STABLE */
   /* Target debounce time = 50 microSec. */
   /* 50us/(1/ref_clk)                    */
   /* * & / 0xffff for accuracy           */
   CNXT_SET_VAL ( SMC_STABLE_REG_ADDR ( uBank ), SMC_STABLE_DEBOUNCECOUNT_MASK, 
                  (unsigned long)( ( 32.7675 * SMC_REF_CLK ) / 0xffff ) );

   /* set ATIME. Should be greater than the card internal reset time of 40K cycles. */
   CNXT_SET_VAL ( SMC_ATIME_REG_ADDR ( uBank ), SMC_TIME_CYCLES_MASK, 45000 );

   /* set DTIME */
   /* Should be less than 0.33 milliSec for total deactivation */
   /* time of 1milliSec.                                       */
   CNXT_SET_VAL ( SMC_DTIME_REG_ADDR ( uBank ), SMC_TIME_CYCLES_MASK, 
                  (unsigned short)( 0.000333 * SMC_REF_CLK ) );

   /* set ATRSTIME */
	CNXT_SET_VAL ( SMC_ATRSTIME_REG_ADDR ( uBank ), SMC_TIME_CYCLES_MASK, 0xFFFF );

   /* set ATRDTIME */
   CNXT_SET_VAL ( SMC_ATRDTIME_REG_ADDR ( uBank ), SMC_TIME_CYCLES_MASK, 19200 );

   /* set BLKTIME, Same as CHTime. See "Work Waiting Time" for T0. */
   CNXT_SET_VAL ( SMC_BLKTIME_REG_ADDR ( uBank ), SMC_TIME_CYCLES_MASK, 9600 );

   /* set CHTIME */
   /* Defined in protocol. See "Work Waiting Time" */
   /* and "Initial Waiting Time" T0.               */
   CNXT_SET_VAL ( SMC_CHTIME_REG_ADDR ( uBank ), SMC_TIME_CYCLES_MASK, 9600 );

   /* set CLK_DIV to get 1MHz clk */
   CNXT_SET_VAL ( SMC_CLK_DIV_REG_ADDR ( uBank ), SMC_CLK_DIV_LOTIMECOUNT_MASK, 
                  (unsigned char)( SMC_REF_CLK / 2000000 ) );
   CNXT_SET_VAL ( SMC_CLK_DIV_REG_ADDR ( uBank ), SMC_CLK_DIV_HITIMECOUNT_MASK, 
                  (unsigned char)( SMC_REF_CLK / 2000000 ) );

   /* set FD */
   CNXT_SET_VAL ( SMC_FD_REG_ADDR ( uBank ), SMC_FD_D_VALUE_MASK, 
                  baud_rate_table[pCard->Config.uDI] );
   CNXT_SET_VAL ( SMC_FD_REG_ADDR(uBank), SMC_FD_F_VALUE_MASK,
                  clock_rate_table[pCard->Config.uFI].Fi );

   /* set VCC */
   CNXT_SET_VAL ( SMC_CONFIG_REG_ADDR ( uBank ), SMC_CONFIG_VCC_VALUE_MASK, pCard->Vcc );
#if SMC_VOLT_CTRL==SMC_VOLT_CTRL_EXT
   if ( pCard->Vcc == SMC_VCC_5V )
   {
      set_gpio_output_high ( ( uSlotId == 0 ) ?
                             PIN_GPIO_SMC_0_VOLTAGE_CTRL:
                             PIN_GPIO_SMC_1_VOLTAGE_CTRL );
   }
   else
   {
      set_gpio_output_low ( ( uSlotId == 0 ) ?
                            PIN_GPIO_SMC_0_VOLTAGE_CTRL:
                            PIN_GPIO_SMC_1_VOLTAGE_CTRL );
   }
#endif

   /* set Detect pin polarity */      
   CNXT_SET_VAL ( SMC_CONFIG_REG_ADDR ( uBank ), SMC_CONFIG_DETECT_POLARITY_MASK, 
                  CARD_DETECT_SIGNAL_HIGH );

   /* enable Auto-detect convention */
   CNXT_SET_VAL ( SMC_CONFIG_REG_ADDR ( uBank ), SMC_CONFIG_AUTO_DETECT_CONV_MASK, 
                  SMC_ENABLE );

   /* set extra guard-time */
   CNXT_SET_VAL ( SMC_CHGUARD_REG_ADDR(uBank), SMC_GUARD_CYCLES_MASK, pCard->Config.uN );

   /*
    * set bit 0 & 1 of terminal control reg to correct fast turn around 
    * problem for Colorado Rev F and beyond
    */
   read_chip_id_and_revision ( &uChip, &uChipRev );

   if ( ( uChip == COLORADO ) && ( uChipRev == PCI_REV_ID_F_COLO ) )
   {
      CNXT_SET_VAL ( SMC_TERM_CTRL_REG_ADDR ( uBank ), 0x3, 0x3 );
   }

   /* set initial INT_MASK */
   *(LPREG)SMC_INT_MASK_REG_ADDR ( uBank ) = SMC_INT_CARDIN_MASK | 
                                             SMC_INT_CARDOUT_MASK | 
                                             SMC_INT_CARDUNPOWERED_MASK | 
                                             SMC_INT_ATRSTARTTIMEOUT_MASK | 
                                             SMC_INT_ATRDURATIONTIMEOUT_MASK;
                     
}

void smc_descriptor_init ( SMC_DESCRIPTOR *pCard, u_int32 uSlotId )
{
   u_int32 uBank = BANK_FROM_SLOT_ID ( uSlotId );

   /* check if a card is in the slot */
   if ( CNXT_GET ( SMC_ICC_STAT_REG_ADDR ( uBank ),
                   SMC_ICC_STAT_CARDPRESENT_MASK ) == 0 )
   {
      pCard->State = SMC_NOT_INSERT;
   }
   else
   {
      pCard->State = SMC_INSERT;
   }

   pCard->Vcc = SMC_VCC_5V;

   pCard->Config.uConvention        = CNXT_SMC_CONV_DIRECT;
   pCard->Config.uProtocol          = CNXT_SMC_PROTOCOL_T0;
   pCard->Config.uFI                = 0;
   pCard->Config.uDI                = 1;
   pCard->Config.uPI1               = 5;
   pCard->Config.uPI2               = 0;
   pCard->Config.uII                = 50;
   pCard->Config.uN                 = 0;
   pCard->Config.pHistorical        = NULL;
   pCard->Config.uHistoricalLength  = 0;
   pCard->Config.uRetry             = 0;
   pCard->Config.uTimeout           = 2000;  /* 2 sec for synchronous reset and powerdown */

   pCard->uAtrLength = 0;
}

bool smc_atr_complete ( SMC_DESCRIPTOR *pCard )
{
   u_int32 i, k, index;
   u_int32 tck_present = 0;
   u_int8 td;

   if ( pCard->uAtrLength < 2 )
   {
      return 0;
   }

   /* point to T0 */
   i = 1;
   k = (u_int32)( pCard->uAtr[i] & 0xf );  /* K value */

   index = 0;
   while ( i < pCard->uAtrLength )
   {
      td = pCard->uAtr[i];

      if ( ( td & 0xf ) && ( index != 0 ) )
      {
         tck_present = 1;
      }

      if ( td & 0x10 )
      {
         i ++;       /* TAi */
      }

      if ( td & 0x20 )
      {
         i ++;       /* TBi */
      }

      if ( td & 0x40 )
      {
         i ++;       /* TCi */
      }

      if ( td & 0x80 )
      {
         i ++;       /* TDi */
         index ++;
      }
      else
      {
         break;      /* end of T chars */
      }
   }

   i += ( k + tck_present );
   if ( i < pCard->uAtrLength )
   {
      pCard->uAtrLength = i + 1;
      return TRUE;
   }
   else
   {
      return FALSE;
   }
}

//static int smc_chtime_tmp = 100;
bool smc_parse_atr ( u_int32 slot_id, SMC_DESCRIPTOR *pCard )
{
   u_int8 *ptr = pCard->uAtr;

   int t0_protocol = 0, t_protocol = 0, tck_present = 0;

   u_int8 cwi_value = 13, bwi_value = 4;
   u_int8 wwi_value = 10, edc_type = 0;

   u_int8 y_value, t_value;
   int index, uBank;
#ifdef  CTICA   
   int len; /*added by J. Mao*/ 
   char History[50];
#else
   u_int32 sc_frequency;
#endif

   if ( pCard->State < SMC_ATR_RECEIVED )
   {
      return FALSE;
   }

   /* check convention */
   if ( *ptr == 0x3b )
   {
      pCard->Config.uConvention = CNXT_SMC_CONV_DIRECT;
   }
   else if ( *ptr == 0x3f )
   {
      pCard->Config.uConvention = CNXT_SMC_CONV_INVERSE;
   }
   else
   {
      /* invalid convention */
      return FALSE;
   }
   ptr ++;

   /* get number of historical bytes */
   pCard->Config.uHistoricalLength = *ptr & 0x0f;

   /* parse fields of next index, starting from index = 1 */
   index = 1;
   y_value = *ptr++ & 0xf0;
   t_value = 0xff;    /* no T value for T0 */

   while ( 1 )
   {
      /* take care t_value */
      if ( t_value == 0 )
      {
         if ( index == 2 )
         {
            t0_protocol = 1;
         }
      }
      else 
      {
         t_protocol = t_value;

         if ( index != 1 )
         {
            tck_present = 1;
         }
      }

      /* parse TA, TB, TC, TD */
      if ( y_value & 0x10 )
      {
         if ( index == 1 )
         {
            /* TA1 */
            pCard->Config.uDI = *ptr & 0xf;
            pCard->Config.uFI = ( *ptr >> 4 ) & 0x0f;
         }
         else
         {
            /* TAi (i>1) */
#ifdef DRIVER_INCL_TRACE
            isr_trace_new ( TRACE_LEVEL_1 | TRACE_SMC,
                        "parse_card_atr: TA[%d] = 0x%x\n", index, *ptr );
#endif
         }

         ptr ++;
      }

      if ( y_value & 0x20 )
      {
         if ( index == 1 )
         {
            /* TB1 */
            pCard->Config.uPI1 = *ptr & 0x1f;
            pCard->Config.uII = ( *ptr >> 5 ) & 0x03;
         }
         else if ( index == 2 )
         {
            pCard->Config.uPI2 = *ptr;
         }
         else if ( t_value == 1 )
         {
            /* TBi (i>2) T=1 */
            /* get CWI and BWI */
            cwi_value = *ptr & 0x0f;
            bwi_value = ( *ptr >> 4 ) & 0x0f;

            if ( bwi_value > 9 )
            {
               bwi_value = 4;
            }
         }
         else
         {
            /* TBi (i>2) T!=1 */
#ifdef DRIVER_INCL_TRACE
            isr_trace_new ( TRACE_LEVEL_1 | TRACE_SMC,
                        "parse_card_atr: TB[%d] = 0x%x\n", index, *ptr );
#endif
         }

         ptr ++;
      }

      if ( y_value & 0x40 )
      {
         if ( index == 1 )
         {
            /* TC1 */
            pCard->Config.uN = *ptr;
         }
         else if ( index == 2 )
         {
            /* TC2 */
            if ( t_value == 1 )
            {
               wwi_value = *ptr;
            }
         }
         else  if ( t_value == 1 )
         {
            /* TCi (i>2) T=1 */
            edc_type = *ptr & 0x01;
         }
         else
         {
            /* TCi (i>2) T!=1 */
#ifdef DRIVER_INCL_TRACE
            isr_trace_new ( TRACE_LEVEL_1 | TRACE_SMC,
                        "parse_card_atr: TC[%d] = 0x%x\n", index, *ptr );
#endif
         }
            
         ptr ++;
      }

      if ( y_value & 0x80 )
      {
         index ++;
         y_value = *ptr & 0xf0;
         t_value = *ptr++ & 0x0f;
      }
      else
      {
         break;
      }
   }

   /* get historical bytes */
   pCard->Config.pHistorical = ptr;

   /* make sure we have correct historical bytes */
   if ( ( ( (int)ptr - (int)pCard->uAtr ) 
            + (int)pCard->Config.uHistoricalLength + (int)tck_present ) 
        != pCard->uAtrLength )
   {
      return FALSE;
   }

   /* determine protocol */
   if ( t0_protocol == 1 )
   {
      pCard->Config.uProtocol = CNXT_SMC_PROTOCOL_T0;
   }
   else if ( t_protocol == 1 )
   {
      pCard->Config.uProtocol = CNXT_SMC_PROTOCOL_T1;
   }
   else if ( t_protocol == 14 )
   {
      pCard->Config.uProtocol = CNXT_SMC_PROTOCOL_T14;
   }
   else
   {
#ifdef DRIVER_INCL_TRACE
      isr_trace_new ( TRACE_LEVEL_1 | TRACE_SMC,
                      "parse_card_atr ( %d ): Unsupported protocol!\n", slot_id, 0 );
#endif
   }

   /* check TCK */
   if ( tck_present )
   {
      y_value = 0;      /* re-use t_value and y_value */
      for ( t_value = 1 ; t_value < pCard->uAtrLength ; t_value ++ )
      {
         y_value ^= pCard->uAtr[t_value];
      }
      if ( ( y_value !=0 ) && ( ( y_value ^ pCard->uAtr[0] ) != 0 ) )
      {
#ifdef DRIVER_INCL_TRACE
         isr_trace_new ( TRACE_LEVEL_1 | TRACE_SMC,
                         "parse_card_atr ( %d ): ATR CKS failed!\n", slot_id, 0 );
#endif
         return FALSE;
      }
   }

   /*                            */      
   /* update HW according to ATR */
   /*                            */

   uBank = BANK_FROM_SLOT_ID ( slot_id );

   /* flush Tx fifo */
   CNXT_SET_VAL ( SMC_TXCOUNT_REG_ADDR ( uBank ), SMC_COUNT_COUNT_MASK, 0 );

   /* disable RxIRQ */
   CNXT_SET_VAL ( SMC_RXTIDE_REG_ADDR ( uBank ), SMC_TIDE_LEVEL_MASK, 0xf );

   /* flush Rx fifo */
   CNXT_SET_VAL(SMC_RXCOUNT_REG_ADDR(uBank), SMC_COUNT_COUNT_MASK, 0);

   /* switch to Tx mode */
   CNXT_SET_VAL ( SMC_TERM_CTRL_REG_ADDR ( uBank ), SMC_TERM_CTRL_MODE_MASK, SMC_MODE_TX );

#ifndef CTICA
   /* set CLK_DIV */
   if ( pCard->Config.uProtocol == CNXT_SMC_PROTOCOL_T14 )
   {
      /* has to be 6MHz */
      t_value = SMC_REF_CLK / ( 6 * 2000000 );  /* re-use t_value */

      if ( SMC_REF_CLK % ( 6 * 2000000 ) )
      {
         t_value ++;
      }
   }
   else
   {
      t_value = SMC_REF_CLK / ( clock_rate_table[pCard->Config.uFI].Fmax * 2000000 ); /* re-use t_value */

      if( SMC_REF_CLK % ( clock_rate_table[pCard->Config.uFI].Fmax * 2000000 ) )
      {
         t_value ++;
      }
   }

   CNXT_SET_VAL ( SMC_CLK_DIV_REG_ADDR ( uBank ), SMC_CLK_DIV_LOTIMECOUNT_MASK, t_value );
   CNXT_SET_VAL ( SMC_CLK_DIV_REG_ADDR ( uBank ), SMC_CLK_DIV_HITIMECOUNT_MASK, t_value );

   sc_frequency = SMC_REF_CLK / ( t_value + t_value );

#ifdef DRIVER_INCL_TRACE
   isr_trace_new ( TRACE_LEVEL_1 | TRACE_SMC,
               "parse_card_atr ( %d ): SMCFreq=%ld\n", slot_id, sc_frequency );
#endif
#endif   
   /* set FD */
   if ( pCard->Config.uProtocol == CNXT_SMC_PROTOCOL_T14 )
   {
      /* etu has to be 625/clk */
      CNXT_SET_VAL ( SMC_FD_REG_ADDR ( uBank ), SMC_FD_D_VALUE_MASK, 1 );
      CNXT_SET_VAL ( SMC_FD_REG_ADDR ( uBank ), SMC_FD_F_VALUE_MASK, 625 );
   }
   else
   {
      CNXT_SET_VAL ( SMC_FD_REG_ADDR ( uBank ), SMC_FD_D_VALUE_MASK, 
                     baud_rate_table[pCard->Config.uDI] );
      CNXT_SET_VAL ( SMC_FD_REG_ADDR ( uBank ), SMC_FD_F_VALUE_MASK, 
                     clock_rate_table[pCard->Config.uFI].Fi );
   }

   if ( pCard->Config.uProtocol == CNXT_SMC_PROTOCOL_T1 )
   {
      /* set parity */
      CNXT_SET_VAL ( SMC_PARITY_REG_ADDR ( uBank ), 
                     SMC_PARITY_ENABLETXNAK_MASK, SMC_DISABLE );
      CNXT_SET_VAL ( SMC_PARITY_REG_ADDR ( uBank ), 
                     SMC_PARITY_ENABLERXNAK_MASK, SMC_DISABLE );
      
      /* set BLKTIME */
      CNXT_SET_VAL ( SMC_BLKTIME_REG_ADDR ( uBank ), SMC_TIME_CYCLES_MASK, 
		               11 + ( 1 << bwi_value ) * 960 * 372 
		                       * baud_rate_table[pCard->Config.uDI] 
		                       / clock_rate_table[pCard->Config.uFI].Fi );

      /* set CHTIME */
      CNXT_SET_VAL ( SMC_CHTIME_REG_ADDR ( uBank ), SMC_TIME_CYCLES_MASK,
                     11 + ( 1 << cwi_value ) );

      /* set BKGUARD */
      CNXT_SET_VAL ( SMC_BKGUARD_REG_ADDR ( uBank ), SMC_GUARD_CYCLES_MASK, 22 );

      /* set extra guard-time */
      if ( pCard->Config.uN != 255 )
      {
         CNXT_SET_VAL ( SMC_CHGUARD_REG_ADDR ( uBank ), 
                        SMC_GUARD_CYCLES_MASK, 
                        pCard->Config.uN + 1 );
      }
      else
      {
         CNXT_SET_VAL ( SMC_CHGUARD_REG_ADDR ( uBank ), 
                        SMC_GUARD_CYCLES_MASK, 
                        0 );
      }
   }
   else
   {
      /* set parity */
      if ( pCard->Config.uProtocol == CNXT_SMC_PROTOCOL_T0 )
      {
         CNXT_SET_VAL ( SMC_PARITY_REG_ADDR ( uBank ), 
                        SMC_PARITY_ENABLETXNAK_MASK, SMC_ENABLE );
         CNXT_SET_VAL ( SMC_PARITY_REG_ADDR ( uBank ), 
                        SMC_PARITY_ENABLERXNAK_MASK, SMC_ENABLE );
      }
      else
      {
#ifdef SMC_PARITY_DISABLETX_GEN_MASK
         CNXT_SET_VAL ( SMC_PARITY_REG_ADDR ( uBank ), 
                        SMC_PARITY_DISABLETX_GEN_MASK, SMC_ENABLE );
#endif
      }         

      /* set TXRETRY */
      CNXT_SET_VAL ( SMC_TXRETRY_REG_ADDR ( uBank ), SMC_RETRY_RETRIES_MASK, 3 );

      /* set RXRETRY */
      CNXT_SET_VAL ( SMC_RXRETRY_REG_ADDR ( uBank ), SMC_RETRY_RETRIES_MASK, 3 );

      /* set CHTIME & BLKTIME */
      index = wwi_value * baud_rate_table[pCard->Config.uDI];   /* re-use index */
      if ( index > 67 )
      {
         index = 0x1999;
      }
      else
      {
         index *= 960;
      }

	#ifdef DVNCA
      CNXT_SET_VAL ( SMC_CHTIME_REG_ADDR ( uBank ), SMC_TIME_CYCLES_MASK, 200 );
      CNXT_SET_VAL ( SMC_BLKTIME_REG_ADDR ( uBank ), SMC_TIME_CYCLES_MASK, 19200*30 );
      #else
      CNXT_SET_VAL ( SMC_CHTIME_REG_ADDR ( uBank ), SMC_TIME_CYCLES_MASK, index );
      CNXT_SET_VAL ( SMC_BLKTIME_REG_ADDR ( uBank ), SMC_TIME_CYCLES_MASK, index );
	#endif
#ifdef TFCAS
	pCard->Config.uN = 0x20;
#endif
      /* set extra guard-time */
      if ( pCard->Config.uN != 255 )
      {
         CNXT_SET_VAL ( SMC_CHGUARD_REG_ADDR ( uBank ), 
                        SMC_GUARD_CYCLES_MASK, 
                        pCard->Config.uN );
      }
      else
      {
         CNXT_SET_VAL ( SMC_CHGUARD_REG_ADDR ( uBank ), 
                        SMC_GUARD_CYCLES_MASK, 
                        0 );
      }
   }

   /* set RXTIME */
   CNXT_SET_VAL ( SMC_RXTIME_REG_ADDR ( uBank ), SMC_RXTIME_CYCLES_MASK, 
		            CNXT_GET_VAL ( SMC_CHTIME_REG_ADDR ( uBank ), SMC_TIME_CYCLES_MASK ) );

   /* set int mask */
#if 0
   CNXT_SET_VAL ( SMC_INT_MASK_REG_ADDR ( uBank ), SMC_INT_CARDIN_MASK, SMC_ENABLE );
   CNXT_SET_VAL ( SMC_INT_MASK_REG_ADDR ( uBank ), SMC_INT_CARDOUT_MASK, SMC_ENABLE );
   CNXT_SET_VAL ( SMC_INT_MASK_REG_ADDR ( uBank ), SMC_INT_CARDPOWERED_MASK, SMC_DISABLE );
   CNXT_SET_VAL ( SMC_INT_MASK_REG_ADDR ( uBank ), SMC_INT_CARDUNPOWERED_MASK, SMC_ENABLE );
   CNXT_SET_VAL ( SMC_INT_MASK_REG_ADDR ( uBank ), SMC_INT_CARDONLINE_MASK, SMC_ENABLE );
   CNXT_SET_VAL ( SMC_INT_MASK_REG_ADDR ( uBank ), SMC_INT_CARDOFFLINE_MASK, SMC_DISABLE );
   CNXT_SET_VAL ( SMC_INT_MASK_REG_ADDR ( uBank ), SMC_INT_ATRSTARTTIMEOUT_MASK, SMC_DISABLE );
   CNXT_SET_VAL ( SMC_INT_MASK_REG_ADDR ( uBank ), SMC_INT_ATRDURATIONTIMEOUT_MASK, SMC_DISABLE );
   CNXT_SET_VAL ( SMC_INT_MASK_REG_ADDR ( uBank ), SMC_INT_BLKRXTIMEOUT_MASK, SMC_ENABLE );
   CNXT_SET_VAL ( SMC_INT_MASK_REG_ADDR ( uBank ), SMC_INT_CHARRXTIMEOUT_MASK, SMC_ENABLE );
   CNXT_SET_VAL ( SMC_INT_MASK_REG_ADDR ( uBank ), SMC_INT_TXERROR_MASK, SMC_ENABLE );
   CNXT_SET_VAL ( SMC_INT_MASK_REG_ADDR ( uBank ), SMC_INT_RXERROR_MASK, SMC_ENABLE );
   CNXT_SET_VAL ( SMC_INT_MASK_REG_ADDR ( uBank ), SMC_INT_TXTIDE_MASK, SMC_DISABLE );
   CNXT_SET_VAL ( SMC_INT_MASK_REG_ADDR ( uBank ), SMC_INT_RXREAD_MASK, SMC_DISABLE );
#else
   *(LPREG)SMC_INT_MASK_REG_ADDR ( uBank ) = SMC_INT_CARDIN_MASK |
                                             SMC_INT_CARDOUT_MASK |
                                             SMC_INT_CARDUNPOWERED_MASK |
                                             SMC_INT_BLKRXTIMEOUT_MASK |
                                             SMC_INT_CHARRXTIMEOUT_MASK |
                                             SMC_INT_TXERROR_MASK |
                                             SMC_INT_RXERROR_MASK |
                                             SMC_INT_TXTIDE_MASK |
                                             SMC_INT_RXREAD_MASK;
#endif

#ifdef  CTICA   
  /*For CTI card ,the protocol will be set to T14 despite that the TDi was set the T1 flag,by J. Mao*/
   ptr=pCard->Config.pHistorical;
   for(len=0;len<(int)pCard->Config.uHistoricalLength;len++)
   {
      History[len]=*ptr;
      ptr++;
   }
   if(strstr(History,"cti"))
      pCard->Config.uProtocol = CNXT_SMC_PROTOCOL_T14;
   else
      return FALSE;
#endif      
   return TRUE;
}

void smc_set_fi ( u_int32 uBank, u_int32 uFi )
{
   CNXT_SET_VAL ( SMC_FD_REG_ADDR ( uBank ), SMC_FD_F_VALUE_MASK, 
                  clock_rate_table[uFi].Fi );
}

void smc_set_di ( u_int32 uBank, u_int32 uDi )
{
   CNXT_SET_VAL ( SMC_FD_REG_ADDR ( uBank ), SMC_FD_D_VALUE_MASK, 
                  baud_rate_table[uDi] );
}

bool smc_start_rw_job ( SMC_RW_JOB_DESCRIPTOR *pRwJob )
{
   u_int32 uBank;
   u_int32 uBytesToFifo;

   uBank = BANK_FROM_SLOT_ID ( pRwJob->pInst->uUnitNumber );

   pRwJob->Status = CNXT_SMC_OK;

   if ( ( pRwJob->pTxPtr ) && ( pRwJob->uBytesToTx ) )
   {
      /* flush RxFIFO */
      CNXT_SET_VAL ( SMC_RXCOUNT_REG_ADDR ( uBank ), SMC_COUNT_COUNT_MASK, 0 );

      /* start writing transaction */
      CNXT_SET_VAL ( SMC_TERM_CTRL_REG_ADDR ( uBank ), 
                     SMC_TERM_CTRL_MODE_MASK, SMC_MODE_TX );

      /* fill up Tx fifo */
      if ( pRwJob->uBytesToTx > NUM_SMC_DATA )
      {
         uBytesToFifo = NUM_SMC_DATA;
      }
      else
      {
#ifdef SCARD_FIX
         /* don't transmit the last char */
         uBytesToFifo = pRwJob->uBytesToTx - 1;
#else
         uBytesToFifo = pRwJob->uBytesToTx;
#endif
      }

      while ( uBytesToFifo -- )
      {
         CNXT_SET_VAL ( SMC_DATA_REG_ADDR ( uBank ), SMC_DATA_DATA_MASK,
                        *(pRwJob->pTxPtr) );
         pRwJob->uBytesToTx --;
         pRwJob->pTxPtr ++;
      }
   }

   if ( ( pRwJob->uBytesToTx == 0 ) || ( pRwJob->pTxPtr == NULL ) )
   {
      /* write is done, switch to read */
      CNXT_SET_VAL ( SMC_TXTIDE_REG_ADDR ( uBank ), SMC_TIDE_LEVEL_MASK, 0 );
      CNXT_SET_VAL ( SMC_TERM_CTRL_REG_ADDR ( uBank ),
                     SMC_TERM_CTRL_MODE_MASK, SMC_MODE_RX );
      
      if ( ( pRwJob->pRxPtr == NULL ) || ( pRwJob->pRxPtr >= pRwJob->pRxBufEnd ) )
      {
         /* no response expected */
         return FALSE;
      }
      else
      {
         CNXT_SET_VAL ( SMC_RXTIDE_REG_ADDR ( uBank ), SMC_TIDE_LEVEL_MASK, 0 );
      }
   }
   else
   {
      /* enable TXTIDE interrupt */
#ifdef SCARD_FIX
      CNXT_SET_VAL ( SMC_INT_MASK_REG_ADDR ( uBank ), SMC_INT_TXTIDE_MASK, 
                     SMC_ENABLE );
#endif
      CNXT_SET_VAL ( SMC_TXTIDE_REG_ADDR ( uBank ), SMC_TIDE_LEVEL_MASK, 1 );
   }

   return TRUE;
}

void smc_term_rw_job ( SMC_RW_JOB_DESCRIPTOR *pRwJob, CNXT_SMC_STATUS Status )
{
   CNXT_SMC_INST *pInst;
   CNXT_SMC_EVENT Event;

   if ( Status == CNXT_SMC_OK )
   {
      Event = CNXT_SMC_EVENT_CARD_RW_COMPLETE;
   }
   else
   {
      Event = CNXT_SMC_EVENT_CARD_RW_TIMEOUT;
   }

   pRwJob->Status = Status;
   if ( pRwJob->bAsync )
   {
      /* notify the client */
      pInst = pRwJob->pInst;
      if ( pInst && pInst->pNotifyFn )
      {
         pInst->pNotifyFn ( (CNXT_SMC_HANDLE)pInst,
                            pInst->pUserData,
                            Event,
                            NULL,
                            pRwJob->Tag );
      }

      /* clear RW job */
      cnxt_smc_free_rw_job ( pRwJob );
   }
   else
   {
      if ( pRwJob->SyncSem )
      {
         sem_put ( pRwJob->SyncSem );
      }
   }
}   

#ifdef SCARD_FIX
void smc_tx_last_char ( SMC_DESCRIPTOR *pCard, u_int32 iBank )
{
#if 1
   u_int32 target_time;
   while ( !CNXT_GET_VAL ( SMC_RAW_DAT_REG_ADDR ( iBank ), 
                           SMC_RAW_DATA_MASK ) );
#endif
   /* Send the last char */
   CNXT_SET_VAL ( SMC_DATA_REG_ADDR ( iBank ), SMC_DATA_DATA_MASK,
                     *(pCard->pRwJob->pTxPtr) );
   pCard->pRwJob->uBytesToTx --;
   pCard->pRwJob->pTxPtr ++;
#if 1
   /*
    * Make sure character has begun transmission, 
    * then delay an extra 1/2 ETU to align xmit and receive.
    */
   while ( CNXT_GET_VAL ( SMC_RAW_DAT_REG_ADDR ( iBank ),
                          SMC_RAW_DATA_MASK ) );
   target_time = get_system_time_us() 
                  + ( clock_rate_table[pCard->Config.uFI].Fi /
                      (baud_rate_table[pCard->Config.uDI] * clock_rate_table[pCard->Config.uFI].Fmax * 2));
   while ( get_system_time_us() < target_time ) ;

   /* Go to special mode to do fix */
   CNXT_SET_VAL ( SMC_TERM_CTRL_REG_ADDR ( iBank ), 
                  SMC_TERM_CTRL_LOOPBACK_MASK, SMC_ENABLE );

   /* We need to know when the byte was sent out */
   CNXT_SET_VAL ( SMC_INT_MASK_REG_ADDR ( iBank ), SMC_INT_TXTIDE_MASK, 
                  SMC_DISABLE );
   CNXT_SET_VAL ( SMC_TXTIDE_REG_ADDR ( iBank ), SMC_TIDE_LEVEL_MASK, 1 );

   /* Wait for Byte to be sent out */
   while ( !CNXT_GET_VAL ( SMC_INT_STAT_REG_ADDR ( iBank ),
                           SMC_INT_TXTIDE_MASK ) );

   /* clear it */
   CNXT_SET_VAL ( SMC_INT_STAT_REG_ADDR ( iBank ), SMC_INT_TXTIDE_MASK, 1 ); 

#endif
}
#endif

/****************************************************************************
 * Modifications:
 * $Log: 
 *  16   mpeg      1.15        10/27/03 9:32:56 AM    Larry Wang      CR(s): 
 *        7717 If TC1=255, set char-to-char extra guard time to be zero.
 *        
 *        
 *  15   mpeg      1.14        10/15/03 4:10:40 PM    Larry Wang      CR(s): 
 *        7649 For Colorado Rev F, set bits 0 and 1 to enable HW "fast turn 
 *        around" fix.
 *  14   mpeg      1.13        9/17/03 4:08:34 PM     Lucy C Allevato SCR(s) 
 *        7482 :
 *        update smartcard module to use the new handle lib.
 *        
 *  13   mpeg      1.12        5/2/03 6:29:10 PM      Craig Dry       SCR(s) 
 *        5521 :
 *        Conditionaly remove GPIO Pin Mux register accesses.
 *        
 *  12   mpeg      1.11        2/17/03 3:58:44 PM     Larry Wang      SCR(s) 
 *        5525 :
 *        for T=14 disable parity check and generation, also set fixed clk=6MHz
 *         and etu=625/clk.
 *        
 *  11   mpeg      1.10        2/7/03 4:44:20 PM      Larry Wang      SCR(s) 
 *        5324 :
 *        set RW job status to be CNXT_SMC_OK before start it.
 *        
 *  10   mpeg      1.9         2/7/03 12:34:54 PM     Larry Wang      SCR(s) 
 *        5324 :
 *        Set correct BLKTIME value for T1.
 *        
 *  9    mpeg      1.8         2/6/03 8:34:04 AM      Larry Wang      SCR(s) 
 *        5324 :
 *        Add "#ifdef SCARD_FIX" segments to fix "fast turnaround" problem.
 *        
 *  8    mpeg      1.7         2/5/03 2:53:08 PM      Larry Wang      SCR(s) 
 *        5324 :
 *        
 *        
 *  7    mpeg      1.6         2/5/03 12:50:28 PM     Larry Wang      SCR(s) 
 *        5324 :
 *        
 *        
 *  6    mpeg      1.5         2/4/03 2:36:44 PM      Larry Wang      SCR(s) 
 *        5324 :
 *        
 *        
 *  5    mpeg      1.4         2/4/03 1:46:16 PM      Larry Wang      SCR(s) 
 *        5324 :
 *        
 *        
 *  4    mpeg      1.3         2/4/03 12:17:42 PM     Larry Wang      SCR(s) 
 *        5324 :
 *        
 *        
 *  3    mpeg      1.2         1/31/03 3:39:44 PM     Larry Wang      SCR(s) 
 *        5324 :
 *        
 *        
 *  2    mpeg      1.1         1/28/03 9:28:34 AM     Larry Wang      SCR(s) 
 *        5324 :
 *        
 *        
 *  1    mpeg      1.0         1/27/03 12:44:14 PM    Larry Wang      
 * $
 ****************************************************************************/

