/****************************************************************************/ 
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*          Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002          */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:    RC5DECOD.C
 *
 * Description: Infra Red Remote control decoder module
 *              Decodes the Philips RC5 protocol. 
 * 
 * Author: Steve Glennon
 *
 ****************************************************************************/
/* $Header: ir38kdecode.c, 1, 6/24/04 10:06:59 PM, Xiao Guang Yan$
 ****************************************************************************/ 

#include "hwconfig.h"
#include "kal.h"
#include "retcodes.h"
#include "sabine.h"
#include "confmgr.h"
#include "genir.h"
#include "ir38ktable.h"
#include "ir38kdecode.h"
// #include "fpdisplay.h"

// extern void FlashIRLED(void);
// extern void FlashFPLED(int led_number);

/*********************************/
/* Local static data             */ 
/*********************************/
static bool bFrontSWInited = FALSE;
static bool bBackSWInited = FALSE;

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
     // * lpFrontCtrl = IRD_WINDOW_33 | (IRD_EDGE_ANY<<IRD_EDGE_CTRL_SHIFT) 
 * lpFrontCtrl = IRD_WINDOW_33 | (IRD_EDGE_FALLING<<IRD_EDGE_CTRL_SHIFT) //|IRD_CARRIER_POL //waw change the edge and carrier_pol     
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
      trace_new(TR_IRD, "\nRC5DECOD: IRData 0x%8.8X ", pdatainfo->data[pdatainfo->iread]);
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
   u_int32 raw_code;
   u_int32 raw_code_conver;
   int i;
   

   enum irstate retcode;

   switch (pinstance -> current_state)
   {
   case STATE_WAITING_START:
      pinstance->num_bits = BITS_PER_PACKET;   
      pinstance->packet = 0;              /* First bit data is  a 0 */
      retcode = STATE_GETTING_BITS;       /* Assume switch to next state  */

      /* get the pulse information */
      get_pulse(pinstance, pdatainfo, pulse_count, &pulse);
      #if 0
      /* Start should look for a short or long high pulse */
      /* A short high indicates S1 is '1' */
      if ((pulse.high) && (pulse.shortpulse))
      {
         new_bit = 1;
         pinstance->prev_bit = new_bit;
         pinstance->packet = (pinstance->packet << 1) | (new_bit);
         pinstance->num_bits--;
         pinstance->mid_bit = 0;
         trace_new(TR_IRD,"WS, Start SB");
      } 
      /* A long high indicates S1 is '0' */
      else if ((pulse.high) && (pulse.longpulse))
      {
         new_bit = 0;
         pinstance->prev_bit = new_bit;
         pinstance->packet = (pinstance->packet << 1) | (new_bit);
         pinstance->num_bits--;
         pinstance->mid_bit = 1;
         trace_new(TR_IRD,"WS, Start LB");
      } 
      #endif
	 /* Start should look for a  start   pulse */
      /* A short high indicates S1 is '1' */
      if ((pulse.shortpulse))
      {
        pinstance->mid_bit= 1;
         trace_new(TR_IRD,"WS, Start SB");
      } 
      
      else 
      {
         /* Check whether this is a bad pulse and we may need to send key up */
         check_keyup(pinstance, pkeyinfo, &pulse);

         /* If no match, go back to looking for start */
         retcode = STATE_WAITING_START;
         trace_new(TR_IRD, "WS no match\n");
      
      } /* endif pulse length match */
      break; /* STATE_WAITING_START */
      
   case STATE_GETTING_BITS:
      
      /* Assume stay in same state       */
      retcode = STATE_GETTING_BITS;              
      
      /* get the pulse information */
      get_pulse(pinstance, pdatainfo, pulse_count, &pulse);

      trace_new(TR_IRD,"GB, nb=%d, mb = %d ", pinstance->num_bits, pinstance->mid_bit);
 /* decide what to do based on pulse type */
      if (pulse.middlepules)
      {
         trace_new(TR_IRD,"1");
         
         /* If we were already mid-bit, use this short pulse to provide data. */
         /* If we were not, then we can effectively ignore the short pulse    */
         /* since we already got the data.                                    */
         if (pinstance->mid_bit != 0)
         {
            /* Since this is a short pulse, we know that the new bit is the same as the last */
            new_bit = 1;
            pinstance->prev_bit = new_bit;
            pinstance->packet = (pinstance->packet << 1) | (new_bit);
            pinstance->num_bits--;
            if(pinstance->num_bits <= 10)
            	{
		trace_new(TR_IRD,"num-bits<10");
            }
            trace_new(TR_IRD,"MB, pkt=0x%4.4X", pinstance->packet);
         
         } /* endif mid bit */
         
         /* since this was a short pulse, we alternated from being mid-bit */
  //       pinstance->mid_bit ^= 1;
      }
       
      else if (pulse.longpulse)
      {
         trace_new(TR_IRD,"LP ");
         /* We should only get long pulses from mid-bit */
         if (pinstance->mid_bit != 0)
         {
            /* Long pulse means the bit alternated from the previous bit */
            /* Since this is a short pulse, we know that the new bit is the same as the last */
            new_bit = 0;
            pinstance->prev_bit = new_bit;
            pinstance->packet = (pinstance->packet << 1) | (new_bit);
            pinstance->num_bits--;
            if(pinstance->num_bits <= 10)
            	{
		trace_new(TR_IRD,"num-bits<10");
            }
            trace_new(TR_IRD,"MB, pkt=0x%4.4X", pinstance->packet);
        
            trace_new(TR_IRD,"MB, pkt=0x%4.4X ", pinstance->packet);
         } 
         else 
         {
            /* invalid state - return to looking for start */
            trace_new(TR_IRD, "Not MB - BAD!!! LP not at mid bit\n");
            retcode = STATE_WAITING_START;
         } /* endif mid bit */
      
      }
      
      else 
      {
         /* Check whether this is a bad pulse and we may need to send key up */
         check_keyup(pinstance, pkeyinfo, &pulse);

         trace_new(TR_IRD,"BAD, restart");
         retcode = STATE_WAITING_START;
      } /* endif pulse long or short */
      
      
      /* OK, we decided what to do with the bit. Now see if we are done */
      if (pinstance->num_bits == 0)
      {

         /* We now have a fully captured packet - next state is start again  */             
         retcode = STATE_WAITING_START;

         /* Discard the packet if it is the same as the last one */
//         if (pinstance->packet != pinstance->last_packet)
         {
            trace_new(TR_IRD,"\nRC5DECOD: ALL Bits, decode.");

            trace_new(TRACE_IRD | TRACE_LEVEL_2 | TRACE_NO_TIMESTAMP,
                      "\nRC5DECOD: Address %d, Command 0x%2.2X\n",
                      ((pinstance->packet >> 6) & 0x1F),
                      ((pinstance->packet & 0x3F) | ((pinstance->packet & 0x00001000) ? 0 : 0x40)));
            
            pinstance->last_packet = pinstance->packet;
            /* Now check the IR device data matches our criteria */
//            if (((pinstance->packet >> 7) & DEVICE_DATA_MASK) == DEVICE_ADDRESS_MATCH)
            {
               // FlashIRLED();
                // FlashFPLED(0x04);
               //fp_blink(LED_IR,0x01,0x01);
       //        fp_blink(0x04,0x01,0x01);
       //        delay(10000);
              // fp_blink(LED_IR,0x00,0x01);
             //  fp_blink(0x04,0x00,0x01);
               pinstance->last_matched = TRUE;
               /* Extract the bottom 6 bits of the command */
//               raw_code = (pinstance->packet & 0x0000003F);
               raw_code = ((pinstance->packet>>7) & DEVICE_DATA_MASK);
               raw_code_conver =0x00;
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
    
         }
         #if 0
         else
         {
            trace_new(TR_IRD,"\nRC5DECOD: ALL Bits, discard (same as last).");
            /* if the packet is the same as the last time, and the address */
            /* matched OK (we generated a keypress) flash the LED again.   */
            if (pinstance->last_matched)
            {
               // FlashIRLED();
            } /* endif */
            
         } /* endif packet not same as last */
         #endif
         
      } /* endif no bits left to get */
      break;  /* STATE_GETTING_BITS */
   default:
      trace_new(TR_IRD,"BAD STATE!!!!\n");
      break;   
    
  // #endif      
   } /* endswitch state */

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
   //u_int32 LogicLevel0;

   /* Get the information from the queue */
  // LogicLevel0 = (pdatainfo->data[pdatainfo->iread] & IRD_BIT_LEVEL) >> 16;
   FifoCount0 = pdatainfo->data[pdatainfo->iread] & IRD_DATA_MASK;
   pulse_count--;
   pdatainfo->iread++;   
   pdatainfo->iread &= IR_INDEX_MASK;

   /* Initialize the pulse structure */
   ppulse->length         = FifoCount0;
   //ppulse->high           = LogicLevel0;
   ppulse->shortpulse     = FALSE;
   ppulse->middlepules  =  FALSE;
   ppulse->longpulse      = FALSE;
   ppulse->interwordpulse = FALSE;

   /* Now check against our Start/0'/'1'/interword criteria */
   if ((FifoCount0 >= SPULSE_MIN) && (FifoCount0 <= SPULSE_MAX))
   {
      ppulse->shortpulse = TRUE;
      trace_new(TR_IRD, "Start");
      
   } 
   else if ((FifoCount0 >= MPULSE_MIN) && (FifoCount0 <= MPULSE_MAX))
   {
      ppulse->middlepules= TRUE;
      trace_new(TR_IRD, "1 ");
      
   } 
   
   else if ((FifoCount0 >= LPULSE_MIN) && (FifoCount0 <= LPULSE_MAX)) 
   {
      ppulse->longpulse = TRUE;
      trace_new(TR_IRD, "0 ");
   } 
   else if ((FifoCount0 > REPEAT_MIN) && (FifoCount0 < REPEAT_MAX))
   {  
      ppulse->interwordpulse = TRUE;
      trace_new(TR_IRD, "IW ");
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
   if (!ppulse->shortpulse && !ppulse->longpulse && !ppulse->middlepules && !ppulse->interwordpulse)
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

 /****************************************************************************
 * Modifications:
 * $Log: 
 *  1    mpeg      1.0         6/24/04 10:06:59 PM    Xiao Guang Yan  CR(s) 
 *        9583 9584 : Ir38k decoder driver implementation
 * $
 * 
 *    Rev 1.2   25 Oct 2002 17:51:30   glennon
 * SCR(s): 4845 
 * Fixed comments to show how to reduce time before key up is reported.
 * 
 * 
 *    Rev 1.1   24 Oct 2002 13:10:46   glennon
 * SCR(s): 4834 
 * Modified decode function to output the device address and command code irrespective of whether it is in the allowable device address range. This is to allow us to more easily create new code tables with a new remote.
 * 
 * 
 *    Rev 1.0   15 Oct 2002 17:23:40   glennon
 * SCR(s): 4796 
 * Implementation of RC5 decode decode according to the GENIR interfaces
 * 
 * 
 *    Rev 1.0   15 Oct 2002 16:17:30   glennon
 * SCR(s): 4796 
 * Implementation of RC5 Infrared protocol decode according to GENIR interfaces.
 * 
 ****************************************************************************/ 

