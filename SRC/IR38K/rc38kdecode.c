/****************************************************************************/ 
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*          Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002          */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:    RC38KDECODE.C
 *
 * Description: Infra Red Remote control decoder module
 *              Decodes the NEC protocol, 38khz carriar of SC6122. 
 * 
 * Author: Sunbey Tu
 *
 ****************************************************************************/
/* $Header: rc38kdecode.c, 1, 7/14/04 1:52:25 ÏÂÎç, Ford Fu$
 ****************************************************************************/ 

#include "hwconfig.h"
#include "kal.h"
#include "retcodes.h"
#include "sabine.h"
#include "confmgr.h"
#include "genir.h"
#include "rc38ktable.h"
#include "rc38kdecode.h"
#include <stdio.h>
//#include "fpdisplay.h"


extern void FlashIRLED(void);
extern void FlashFPLED(int led_number);
/*********************************/
/* Local static data             */ 
/*********************************/
static bool bFrontSWInited = FALSE;
static bool bBackSWInited = FALSE;
static u_int32 Rep_Interval = 3;


#define TR_IRD    (TRACE_IRD | TRACE_LEVEL_1 | TRACE_NO_TIMESTAMP)

/*********************************/
/* Internal function prototypes  */ 
/*********************************/
void         get_pulse(       PIRDECODE_INSTANCE pinstance, 
                              PIRRX_DATAINFO     pdatainfo, 
                              u_int16            pulse_count, 
                              PPULSE             ppulse );
enum irstate ir_state_change( PIRDECODE_INSTANCE pinstance, 
                              PIRRX_DATAINFO     pdatainfo, 
                              u_int16            pulse_count, 
                              PIRRX_KEYINFO      pkeyinfo);
void         check_keyup(     PIRDECODE_INSTANCE pinstance, 
                              PIRRX_KEYINFO pkeyinfo, 
                              PPULSE ppulse );
u_int16      query_data(      u_int16 index_w, u_int16 index_r );
static void ir_return_to_waiting( PIRDECODE_INSTANCE pinstance, 
      PIRRX_KEYINFO pkeyinfo, PPULSE pulse, enum irstate *retcode );


/*************************/
/* Function definitions  */ 
/*************************/
/********************************************************************/
/*  FUNCTION:    IR_Init                                            */
/*                                                                  */
/*  PARAMETERS:  port         PORT_FRONT, PORT_BACK or PORT_UNKNOWN */
/*                            Which port to initialize              */
/*                                                                  */
/*  DESCRIPTION: This function is called by GENIR to ininitialize   */
/*               the hardware for the port specified. The init is   */
/*               specific to the algorithm as it specifies which    */
/*               edges to pay attention to, and the carrier timing. */
/*                                                                  */
/*  RETURNS:     Nothing                                            */
/*                                                                  */
/*  CONTEXT:     May be called from interrupt or non-interrupt      */
/*               contexts.                                          */
/*                                                                  */
/********************************************************************/
void IR_Init(IRRX_PORTID portid)
{
   bool ks;
   /* Registers for front IR */
   LPREG lpFrontCtrl;
   LPREG lpFrontClkCtrl;
   LPREG lpFrontFilter;
   LPREG lpFrontIntEnable;
   /* Registers for back IR */   
   LPREG lpBackCtrl; 
   LPREG lpBackClkCtrl;
   LPREG lpBackFilter;

   if (portid == PORT_FRONT)
   {
      /*******************************/
      /* Initalize Front IR hardware */
      /*******************************/
      lpFrontCtrl = (LPREG)IRD_CONTROL_REG;/*30560000*/
      lpFrontClkCtrl = (LPREG)IRD_RX_CLK_CONTROL_REG;/*30560008*/
      lpFrontFilter = (LPREG)IRD_LOWPASS_FILTER_REG;/*30560018*/
      lpFrontIntEnable = (LPREG)IRD_INT_ENABLE_REG;     

      /* Critical session added because txir test might be access these registers at the same time. */
      ks = critical_section_begin();
      * lpFrontCtrl = IRD_WINDOW_33 | (IRD_EDGE_ANY<<IRD_EDGE_CTRL_SHIFT)//|IRD_CARRIER_POL 
                      | IRD_ENABLE_RX_FIFO | IRD_ENABLE_RX | IRD_RX_INT_CTRL;
      * lpFrontClkCtrl = IRRX_CLK_CTRL;
      * lpFrontFilter = IR_FILTER_VALUE;   
      * lpFrontIntEnable = IRD_INT_RX_FIFO;  /* interrupt will be generated when FIFO is not empty */
      critical_section_end(ks); 
   }
   else if (portid == PORT_BACK)
   {
      /*******************************/
      /* Initalize Back IR hardware */
      /*******************************/
      lpBackCtrl = (LPREG)PLS_CONTROL_REG;   
      lpBackClkCtrl = (LPREG)PLS_PULSE_TIMER_DIV_REG;   
      lpBackFilter = (LPREG)PLS_FILTER_CLK_DIV_REG;   

      ks = critical_section_begin();   
      CNXT_SET_VAL(lpBackCtrl, PLS_CONTROL_TIMEOUT_INT_ENABLE_MASK, 0);
      CNXT_SET_VAL(lpBackCtrl, PLS_CONTROL_OVERRUN_INT_ENABLE_MASK, 0);
      CNXT_SET_VAL(lpBackCtrl, PLS_CONTROL_FIFO_INT_ENABLE_MASK, 1);
      CNXT_SET_VAL(lpBackCtrl, PLS_CONTROL_EDGE_CNTL_MASK, PLS_EDGE_EITHER);
      CNXT_SET_VAL(lpBackCtrl, PLS_CONTROL_FIFO_INT_CNTL_MASK, PLS_FIFO_INT_NOT_EMPTY);
      CNXT_SET_VAL(lpBackCtrl, PLS_CONTROL_ENABLE_MASK, 1);
      * lpBackClkCtrl = IRRX_CLK_CTRL;   
      * lpBackFilter = IR_FILTER_VALUE;      
      critical_section_end(ks);    
   }
} /* IR_Init() */

/********************************************************************/
/*  FUNCTION:    IR_SW_Init                                         */
/*                                                                  */
/*  PARAMETERS:  portid       PORT_FRONT, PORT_BACK or PORT_UNKNOWN */
/*                            Which port to initialize              */
/*                                                                  */
/*               ppinstance   Pointer to location to receive        */
/*                            address of instance data allocated.   */
/*                                                                  */
/*  DESCRIPTION: This function is called by GENIR to allocate and   */
/*               initialize the software decode (specifically the   */
/*               decoder instance data) prior to any decode taking  */
/*               place.                                             */
/*                                                                  */
/*               Protects against multiple initialization.          */
/*                                                                  */
/*  RETURNS:     IRRX_OK if all OK                                  */
/*               IRRX_ERROR if any error (alloc fail, re-init etc)  */
/*                                                                  */
/*  CONTEXT:     Must be called from non-interrupt context.         */
/*                                                                  */
/********************************************************************/
IRRX_STATUS IR_SW_Init(IRRX_PORTID portid, void **ppinstance)
{
   IRRX_STATUS        rc = IRRX_OK;
   PIRDECODE_INSTANCE pinstance;
   
   /* Check this is a recognized port */
   if ((portid == PORT_FRONT) || (portid == PORT_BACK))
   {
   
      /* Check this port is not already initialized */
      if (((portid == PORT_FRONT) && !bFrontSWInited) ||
          ((portid == PORT_BACK) &&  !bBackSWInited)) 
      {
         /* Allocate the instance data */
         pinstance = mem_malloc(sizeof(IRDECODE_INSTANCE));
         if (pinstance)
         {
            pinstance->current_state = STATE_WAITING_START;
            pinstance->packet = 0;    
            pinstance->last_packet = 0;
            pinstance->num_bits = 1;
            pinstance->prev_bit = 1;
            pinstance->mid_bit = 0;
            pinstance->last_cnxtcode = CNXT_NUL;
            pinstance->sent_key_up = TRUE;
            pinstance->last_matched = TRUE;
   
            *ppinstance = pinstance;

            /* indicate the port is initialized to prevent re-init */
            if (portid == PORT_FRONT)
            {
               bFrontSWInited = TRUE;
            } else {
               bBackSWInited = TRUE;
            } /* endif front port */
   
         } else {
            rc = IRRX_ERROR;
         } /* endif malloc succeeded */

      } else {
         rc = IRRX_ERROR;   
      } /* endif re-initing existing port*/
      
   } else {
      rc = IRRX_ERROR;
   } /* endif valid port */
   
   return rc;

} /* IR_SW_Init() */

/********************************************************************/
/*  FUNCTION:    IR_Decode                                          */
/*                                                                  */
/*  PARAMETERS:  pinst_decode Pointer to instance information       */
/*               pdatainfo    Pointer to information on the data    */
/*                            being presented for decode            */
/*               pkeyinfo     Pointer to location to receive        */
/*                            decoded key information when decode   */
/*                            completed successfully                */
/*                                                                  */
/*  DESCRIPTION: This function is called by GENIR whenever there is */
/*               some pulse data to decode. It will normally be     */
/*               called with less than enough data to do a full     */
/*               decode, and is expected to maintain state in       */
/*               pinst_decode to allow it to know where it is up to */
/*               in the decode process.                             */
/*                                                                  */
/*               When it has completed a decode successfully, it    */
/*               sets the components of pkeyinfo to indicate the    */
/*               decoded key.                                       */
/*                                                                  */
/*  RETURNS:     IRRX_OK if all OK                                  */
/*               IRRX_ERROR if any error (alloc fail, re-init etc)  */
/*                                                                  */
/*  CONTEXT:     Must be called from non-interrupt context.         */
/*                                                                  */
/********************************************************************/
void IR_Decode(void * pinst_decode, PIRRX_DATAINFO pdatainfo, PIRRX_KEYINFO pkeyinfo)
{
   PIRDECODE_INSTANCE pinstance;
   u_int16 count;

   pinstance = (PIRDECODE_INSTANCE)pinst_decode;
   count = query_data(pdatainfo->iwrite, pdatainfo->iread);      
   while (count > 0)   
   {   
      trace_new(TR_IRD, "\nRC38KDECODE: IRData 0x%8.8X ", pdatainfo->data[pdatainfo->iread]);  
      pinstance->current_state = ir_state_change(pinstance, pdatainfo, count, pkeyinfo);   
      count = query_data(pdatainfo->iwrite, pdatainfo->iread);   
   }   
} /* IR_Decode() */

/********************************************************************/
/*  FUNCTION:    ir_state_change                                    */
/*                                                                  */
/*  PARAMETERS:  pinstance    Instance data on the decode. Used to  */
/*                            maintain the state of the decoder.    */
/*               pdatainfo    Pointer to the structure which lets   */
/*                            us know the available data for        */
/*                            decode.                               */
/*               pulse_count  Number of pulses available to decode. */
/*               pkeyinfo     Structure to return decoded keys.     */
/*                                                                  */
/*  DESCRIPTION: This function is called repeatedly by IR_Decode to */
/*               decode the incoming pulse data. It consumes a      */
/*               single pulse each time it is called.               */
/*               This is the routine which actually implements the  */
/*               algorithm as a simple state machine. It has two    */
/*               states, WAITING_START and GETTING_BITS.            */
/*                                                                  */
/*               See the file prolog for detailed algorithmic info. */
/*                                                                  */
/*               In addition to looking for valid data, it sends    */
/*               key up information on a non-valid pulse.           */
/*                                                                  */
/*  RETURNS:     STATE_WAITING_START for next state                 */
/*               STATE_GETTING_BITS  for next state                 */
/*                                                                  */
/*  CONTEXT:     Must be called from non-interrupt context.         */
/*                                                                  */
/********************************************************************/
enum irstate ir_state_change(PIRDECODE_INSTANCE pinstance, 
                             PIRRX_DATAINFO pdatainfo, 
                             u_int16 pulse_count, 
                             PIRRX_KEYINFO pkeyinfo)
{
   PULSE   pulse;
   u_int32 new_bit;
   
   /* a flag to indicate received 9ms pulse */
   static bool head_getted = FALSE;
   /* a flag to indicate received 0.56ms high pulse */
   static bool bit_head_getted = FALSE;
   
   static enum inerstate repeat_state=HEAD_GETTING;
   static u_int32 repeat_count =0;
   u_int32 packet_middle;
   u_int32 check_code;
   u_int32 raw_code;
   u_int32 raw_code_conver;
   int i;
   u_int32 packet_sys_code;//add by wlk ,extract system code of IR remote control ,12-30-2004
   enum irstate retcode;

   switch (pinstance->current_state)
   {
   case STATE_WAITING_START:
      /* Assume stay in the same state */
      retcode = STATE_WAITING_START; 
      repeat_state = HEAD_GETTING;
      repeat_count = 0;      
      /* get the pulse information */
      get_pulse(pinstance, pdatainfo, pulse_count, &pulse);
      
      /* Start should look for a  start pulse */
      if( !head_getted )
      {
         if( pulse.head && pulse.high )
			{
            head_getted = TRUE;
         	}
			/*else
         {
            check_keyup(pinstance, pkeyinfo, &pulse);
            trace_new(TR_IRD, "WS no match\n");      
         }*/
      }
      else
      {
         /* A 4.5ms following a 9ms signal indicates  is 'start' */
         if( pulse.begin && !pulse.high )
         {
            retcode = STATE_GETTING_BITS;       /* switch to next state  */
            pinstance->num_bits = BITS_PER_PACKET;   
            pinstance->packet =0;
            bit_head_getted = FALSE;            
         }

         head_getted = FALSE;
      }
      break; /* STATE_WAITING_START */
      
   case STATE_GETTING_BITS:
      /* Assume stay in same state       */
      retcode = STATE_GETTING_BITS;
      get_pulse(pinstance, pdatainfo, pulse_count, &pulse);
      if( !bit_head_getted )
      {
         /* look for bit head (0.56ms high pulse) */
         if( pulse.bit_head )
         {
            bit_head_getted = TRUE;
         }
         else
         {
            ir_return_to_waiting( pinstance, pkeyinfo, &pulse, &retcode );
         }
      }
      else
      {
          /* decide what to do based on pulse type */
         if( pulse.longpulse && !pulse.high )
         {
            /* a 1.69ms pulse of low logic level indicates receive a bit '1' */
            trace_new(TR_IRD,"1");
            new_bit = 1;
            pinstance->prev_bit = new_bit;
            if( BITS_PER_PACKET == pinstance->num_bits )
            {
               pinstance->packet = new_bit;
            }
            else
            {
               pinstance->packet = (pinstance->packet << 1) | (new_bit);
            }
            pinstance->num_bits--;
         }
         else if( pulse.shortpulse && !pulse.high )
         {
            /* a 0.565ms pulse of low logic level indicates receiving a '0' */
            trace_new(TR_IRD,"0 ");
            new_bit = 0;
            pinstance->prev_bit = new_bit;
            if(BITS_PER_PACKET == pinstance->num_bits)
            {
                pinstance->packet = new_bit;
            }
            else
            {
                pinstance->packet = (pinstance->packet << 1) | (new_bit);
            }
            
            pinstance->num_bits--;
         }
         else if( pulse.f_end )
         {
            /* Switch to Repeating State */
            retcode = STATE_REPEAT;
         }
         else
         {
            ir_return_to_waiting( pinstance, pkeyinfo, &pulse, &retcode );
         }
         
         bit_head_getted = FALSE;
      }
  
      /* OK, we decided what to do with the bit. Now see if we are done */
      if (pinstance->num_bits == 0)
      {
         pinstance->num_bits =BITS_PER_PACKET;
         /* Extract the bottom 16 data bits of the command */
         packet_middle = pinstance->packet&0xffff;
         packet_sys_code=pinstance->packet&0xffff0000;//add by wlk, extract system code of IR,12-30-2004
         packet_sys_code=(packet_sys_code>>16);
         raw_code = ((packet_middle>>8) & DEVICE_DATA_MASK);
         check_code = (~packet_middle) & DEVICE_DATA_MASK;
          
         /*if( raw_code != check_code)*/
         /*modify by wlk, add system code checck,12-30-2004*/
         if( (raw_code != check_code)||((packet_sys_code!=REMOTE_SYS_CODE)&&(packet_sys_code!=REMOTE_SYS_CODE2)))
         {
            
            trace_new(TR_IRD,"BAD, restart");
            trace("\nIR: Data error, Check failed\n");
            pinstance->last_matched = FALSE;
         }
         else
         {
            FlashIRLED();
            pinstance->last_packet = pinstance->packet;
            pinstance->last_matched = TRUE;
            raw_code_conver =0x00;
            
            /* convert for the rc38k_xlate_table to be smaller */
            for(i=0;i<8;i++)
            {
            	raw_code_conver|=(((raw_code>>(7-i))&0x01)<<i);
            }
            raw_code =raw_code_conver;
               
            /* Translate the raw code and set the pkeyinfo structure */
            pkeyinfo->key_code = rc38k_xlate_table[raw_code];
           
            pkeyinfo->key_state = IRRX_KEYDOWN;
            /* have to update device_type to validate key information */
            pkeyinfo->device_type = IRRX_REMOTEBUTTON;
    
            /* Make note that we sent a keydown code */
            pinstance->last_cnxtcode = pkeyinfo->key_code;
            pinstance->sent_key_up = FALSE;
         }
      } /* endif no bits left to get */
      break;  /* STATE_GETTING_BITS */
   case STATE_REPEAT:
      /* Assume stay in same state       */
      retcode = STATE_REPEAT;
      get_pulse(pinstance, pdatainfo, pulse_count, &pulse);
      switch( repeat_state )
      {
         case HEAD_GETTING:
            /* Looking for 9ms repeat leading pulse */
            if( pulse.head )
            {
               repeat_state = REPEAT_BEGIN_GETTING;
            }
            else
            {
               repeat_state = HEAD_GETTING;
               ir_return_to_waiting( pinstance, pkeyinfo, &pulse, &retcode );
            }
            break;
         case REPEAT_BEGIN_GETTING:
            /* Looking for 2.25ms begin repeat pulse */
            if( pulse.repeat_begin )
            {
               repeat_state = BIT_HEAD_GETTING;
            }
            else
            {
               repeat_state = HEAD_GETTING;
               ir_return_to_waiting( pinstance, pkeyinfo, &pulse, &retcode );
            }
            break;
         case BIT_HEAD_GETTING:
            /* Looking for 0.56ms bit leading pulse */
            if( pulse.bit_head )
            {
               repeat_state = REPEAT_GETTING;
            }
            else
            {
               repeat_state = HEAD_GETTING;
               ir_return_to_waiting( pinstance, pkeyinfo, &pulse, &retcode );
            }
            break;
         case REPEAT_GETTING:
            /* Looking for 96.19ms long repeat pulse */
            if( pulse.repeat )                                                         
            {                                                                          
                repeat_count++;                                                         
                if( (repeat_count%Rep_Interval)== 0 )                                  
                {                                                                      
                   /* send out a 'key hold' message after special interval */
                   pinstance->num_bits =BITS_PER_PACKET;                               
                   FlashIRLED();                                                       
                   pkeyinfo->key_state = IRRX_KEYHOLD;                                 
                   pkeyinfo->device_type = IRRX_REMOTEBUTTON;                          
                   pkeyinfo->key_code = pinstance->last_cnxtcode ;                     
                   pinstance->sent_key_up = FALSE;                                     
                }/* (repeat_count%Rep_Interval)== 0 */
            }/* if( pulse.repeat ) */  
            else
            {
               	ir_return_to_waiting( pinstance, pkeyinfo, &pulse, &retcode );					
            }
			  repeat_state = HEAD_GETTING;
            break;
         default:
            break;
      }/* switch( repeat_state )*/
      break;
   default:
      break;   
   } /* switch state */

   return retcode;
   
} /* ir_state_change() */

/********************************************************************/
/*  FUNCTION:    get_pulse                                          */
/*                                                                  */
/*  PARAMETERS:  pinstance    Instance data on the decode. Used to  */
/*                            maintain the state of the decoder.    */
/*               pdatainfo    Pointer to the structure which lets   */
/*                            us know the available data for        */
/*                            decode.                               */
/*               pulse_count  Number of pulses available to decode. */
/*               ppulse       Pointer to structure to return info.  */
/*                                                                  */
/*  DESCRIPTION: This function is called to extract information on  */
/*               the next available pulse. It determines whether    */
/*               the pulse is short, long, interwork or none, and   */
/*               whether the pulse is high or not. It also extracts */
/*               the duration so the pulse structure is nice and    */
/*               complete.                                          */
/*                                                                  */
/*  RETURNS:     Nothing                                            */
/*                                                                  */
/*  CONTEXT:     Must be called from non-interrupt context.         */
/*                                                                  */
/********************************************************************/
void get_pulse(PIRDECODE_INSTANCE pinstance, PIRRX_DATAINFO pdatainfo, u_int16 pulse_count, PPULSE ppulse)
{

   u_int32 FifoCount0;
   u_int32 LogicLevel0;

   /* Get the information from the queue */
   LogicLevel0 = (pdatainfo->data[pdatainfo->iread] & IRD_BIT_LEVEL) >> 16;
   FifoCount0 = pdatainfo->data[pdatainfo->iread] & IRD_DATA_MASK;
   //trace("\nIR: Received %x, logic level: %d\n", FifoCount0, LogicLevel0 );
   pulse_count--;
   pdatainfo->iread++;   
   pdatainfo->iread &= IR_INDEX_MASK;

   /* Initialize the pulse structure */
   ppulse->length         = FifoCount0;
   ppulse->high           = LogicLevel0;
   ppulse->head           = FALSE;
   ppulse->begin          = FALSE;
   ppulse->shortpulse     = FALSE;
   ppulse->longpulse      = FALSE;
   ppulse->f_end          = FALSE;
   ppulse->bit_head       = FALSE;
   ppulse->repeat_begin   = FALSE;
   ppulse->repeat         = FALSE;
    
  
   /* Now check against our Start/0'/'1'/repeat criteria */
   if ((FifoCount0 >= BEGIN_MIN) && (FifoCount0 <= BEGIN_MAX))
   {
      ppulse->begin = TRUE;
      trace_new(TR_IRD, "Start");
   } 
   else if ((FifoCount0 >= LPULSE_MIN) && (FifoCount0 <= LPULSE_MAX))
   {
      ppulse->longpulse= TRUE;
      trace_new(TR_IRD, "1 ");
   }   
   else if ((FifoCount0 >= SPULSE_MIN) && (FifoCount0 <= SPULSE_MAX))
   {
      if( ppulse->high )
      {
         ppulse->bit_head = TRUE;
      }
      else
      {
         ppulse->shortpulse = TRUE;
         trace_new(TR_IRD, "0 ");
      }
   }
   else if ((FifoCount0 > F_END_MIN) && (FifoCount0 < F_END_MAX))
   {
    ppulse->f_end = TRUE;
   }
   else if ((FifoCount0 > HEAD_MIN) && (FifoCount0 < HEAD_MAX))
   {
    ppulse->head = TRUE;
   }
   else if ((FifoCount0 > REPEAT_MIN) && (FifoCount0 < REPEAT_MAX))
   {
    ppulse->repeat = TRUE;
   }
    else if ((FifoCount0 > REPEAT_BEGIN_MIN) && (FifoCount0 < REPEAT_BEGIN_MAX))
   {
    ppulse->repeat_begin = TRUE;
   }
   else 
   {
      trace_new(TR_IRD, "BAD ", FifoCount0);
   } /* endif pulse criteria */
   
   return;
   
} /* get_pulse() */

/********************************************************************/
/*  FUNCTION:    check_keyup                                        */
/*                                                                  */
/*  PARAMETERS:  pinstance    Instance data on the decode. Used to  */
/*                            maintain the state of the decoder.    */
/*               pkeyinfo     Pointer to the structure to receive   */
/*                            key up information, if any.           */
/*               pulse_count  Number of pulses available to decode. */
/*               ppulse       Pointer to pulse information.         */
/*                                                                  */
/*  DESCRIPTION: This function is called to on any pulses which do  */
/*               not meet the criteria of "I was expecting this".   */
/*               It is used whenever a pulse which is not short or  */
/*               long or interword length to check whether we need  */
/*               to send a "key up" command. If needed, sets the    */
/*               pkeyinfo to reflect a key up of the last key.      */
/*                                                                  */
/*  RETURNS:     Nothing                                            */
/*                                                                  */
/*  CONTEXT:     Must be called from non-interrupt context.         */
/*                                                                  */
/********************************************************************/
void check_keyup(PIRDECODE_INSTANCE pinstance, PIRRX_KEYINFO pkeyinfo, PPULSE ppulse)
{

   /* See if not a valid pulse for signal or interword gap */
   if ( !ppulse->head && !ppulse->begin && !ppulse->longpulse && !ppulse->shortpulse &&  
            !ppulse->f_end && !ppulse->bit_head && !ppulse->repeat_begin && !ppulse->repeat )
   {
      if (!pinstance->sent_key_up)
      {
         pkeyinfo->key_code = pinstance->last_cnxtcode;
         pkeyinfo->key_state = IRRX_KEYUP;
         /* have to update device_type to validate key information */
         pkeyinfo->device_type = IRRX_REMOTEBUTTON;

         /* Make note that we sent a keyup code */
         pinstance->sent_key_up = TRUE;
         
      } /* endif have not yet sent key up */
   } /* endif not a valid pulse*/
 
   return;
} /* check_keyup() */


/********************************************************************/
/*  FUNCTION:    query_data                                         */
/*                                                                  */
/*  PARAMETERS:  index_w      Write index into the IR data array    */
/*                            which is passed by GENIR.             */
/*               index_r      Read Index into the IR data array     */
/*                            which is passed by GENIR.             */
/*                                                                  */
/*  DESCRIPTION: Returns the number of valid entries in the data    */
/*               array, taking into account buffer wrap.            */
/*                                                                  */
/*  RETURNS:     Number of valid entries                            */
/*                                                                  */
/*  CONTEXT:     May be called from any context.                    */
/*                                                                  */
/********************************************************************/
u_int16 query_data(u_int16 index_w, u_int16 index_r)
{
   u_int16 count;
   
   if(index_w >= index_r)
	   count = index_w - index_r;
	else
      count = IR_BUFFER_SIZE - (index_r - index_w);
   return count;  
    
}  /* query_data() */

static void ir_return_to_waiting( PIRDECODE_INSTANCE pinstance, 
      PIRRX_KEYINFO pkeyinfo, PPULSE pulse, enum irstate *retcode )
{
   check_keyup(pinstance, pkeyinfo, pulse);
   pinstance->packet =0;
   trace_new(TR_IRD,"BAD, restart");
   *retcode = STATE_WAITING_START;
}

void ir_set_rep_interval( u_int32 value )
{
   if( value >= 1 )
      Rep_Interval = value;
}

u_int32 ir_get_rep_interval()
{
   return Rep_Interval;
}


 /****************************************************************************
 * Modifications:
 *    Rev 1.0   12 Jul 2004 10:59:30   sunbey
 * SCR(s): 
 * Implementation of NEC Infrared protocol decode according to GENIR interfaces.
 * 
 ****************************************************************************/ 

