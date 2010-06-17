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
 *
 *              Decodes the Philips RC5 protocol. 
 *              The RC5 protocol is in the public domain with lots of info
 *              on the web. This decoder is not derived from any Philips  
 *              confidential information. 
 *
 *              The RC5 protocol has 14 bits total, and uses the sense of the
 *              signal transition at the mid-bit point to determine whether 
 *              the bit is a 0 or 1.
 *
 *              S0 S1 T  A4 A3 A2 A1 A0 C5 C4 C3 C2 C1 C0
 *
 *              Originally the protocol had two start bits, both '1', but
 *              to add expansion of the number of command bits, the second
 *              start bit was used to create another command bit. 
 *
 *              If S0 S1 = '11' then virtual C6 = '0'
 *              If S0 S1 = '10' then virtual C6 = '1'
 *              
 *              T is the toggle bit which remains the same if a key is held
 *              down, but changes if you press a key twice - this allows you
 *              to distinguish between repeated sending when a key is held 
 *              and a repeated key press.
 *
 *              A4 - A0 indicate the device address, indicating whether this 
 *              is for a TV, TV2, VCR, Sat, Cable etc.
 *
 *              C5-C0 encode the key being pressed. Note there are really
 *              C6-C0 based on the sense of start bit S1.
 *
 *              There seem to be some fairly standard key codes used by RC5,
 *              some of which appear to be (brackets indicate varies):
 *
 *              Hex Command Code        Meaning
 *              0x00-0x09               Numbers 0-9
 *              0x0C                    Power/Standby
 *              0x0D                    Mute
 *              0x0F                    Clear
 *              0x10                    Vol+
 *              0x11                    Vol-
 *             (0x1C                    Up)
 *             (0x1D                    Down)
 *              0x20                    Ch+
 *              0x21                    Ch-
 *              0x22                    Prev Ch
 *             (0x2B                    Right)
 *             (0x2C                    Left)
 *             (0x2E                    Select)
 *
 *             The translation between received codes and CNXT_ defines 
 *             is done by a translation table in RC5KTABLE.H
 *
 * Algorithmic Notes:
 *             A 1 bit is defined like this:
 *             |    |   Bit start/end
 *             __|--    Transition
 * 
 *             A 0 bit is defined like this:
 *             |    |   Bit start/end
 *             --|__    Transition
 * 
 * 
 *             Bit Combinations:
 *             |    |    |     Bit start/end
 *
 *             __|--|__|--     11
 *             __|-----|__     10
 *             --|_____|--     01
 *             --|__|--|__     00
 *
 *            The code looks for short and long pulses to decode the data.
 *            The bins for accepting periods as short or long are defined in
 *            RC5DECOD.H using SPULSE_MIN/MAX and LPULSE_MIN/MAX.
 *
 *            The algorithm starts by looking for a start (which can be either
 *            11 or 10 above. Hence it looks for a short high (indicates 11)
 *            of a long high (indicates 10) It keeps track of whether if is in
 *            the middle of the bit time (false if it got a short, true if a 
 *            long). It then ignores the sense of the period between 
 *            transitions. It should never get a "long" when NOT at the middle
 *            of the bit time - this is an error and causes decode to stop.
 *
 *            If it gets a short starting at the middle of the bit time, it 
 *            knows the next bit will be the same sense as the current bit.
 *            It also knows that it can ignore the next pulse, which must be
 *            short to be valid. It toggles the "mid bit" flag to (making it 
 *            false to ensure the next pulse is ignored.
 *
 *            If it gets a long starting at the middle of the bit time, it
 *            knows the next bit will be the opposite sense to the current 
 *            one. It also knows that it is still at the mid bit position 
 *            for the next transition.
 *
 *            When the code it is at the mid-point of D1 and gets the next 
 *            pulse (putting it at the start of D0 bit time if short, or
 *            at the mid point of D0 bit time if long) it knows the value 
 *            of D0, and can decode the entire packet. There is one short
 *            high packet to come to terminate D0, but this is ignored.
 *
 *            The code as is will attempt to recognize this as a short 
 *            start (indicating S0-S1 = 11), but the following pulse duration
 *            will be the interword gap, or a timeout, and resets the code
 *            to be looking for a start as it was not a valid short or long 
 *            pulse.
 *
 *  Key Down/Up:
 *            The RC5 protocol does not generate any easy means of 
 *            determining key up. Some other protocols actually send a 
 *            key up code. RC5 doesn't. Our general remote support has 
 *            a model of key down/up processing, so we have to generate
 *            a fake key up if we see another key pressed, of if we see
 *            a timeout with no data presented. The IR hardware generates
 *            a timout (duration of 0xFFFF) in around 1 sec, which is not
 *            too objectionable.
 *
 *            This implementation uses the arrival of a pulse which is not
 *            short, long or interword to generate a key up message. We 
 *            cannot use the arrival of another key, as we can only get
 *            one set of key information into the keyinfo structure, and
 *            we need it for the newly arrived key.
 *
 *            To change the band of accuracy at determining an interword
 *            gap to have less chance of missing the opportunity to send a
 *            key-up, modify REPEAT_MIN and REPEAT_MAX in RC5DECOD.H.
 *            
 *            One further way to reduce the time before generating a keyup
 *            is to change the IR clock divider so the timing clock is 
 *            running faster - hence the timeout will be achieved sooner.
 *            To do this, modify the clock divider and compensate by adjusting
 *            the SPULSE_MIN/MAX and other timings. We have tried this reducing 
 *            the divider to 23 (reduce by a factor of 4) and multiplying the 
 *            pulse boundaries by 4 to compensate. This reduces the timeout to
 *            roughly 1/4 second. This is not possible if you are using the
 *            internal demodulator, as in that case the clock must be 16x the
 *            carrier frequency. The defines for these are in 
 *            INCLUDE\RC5DECOD.H
 * 
 *            A less elegant solution to this is to change GENIR to make  
 *            periodic calls (longer than the interword gap) to the decoder
 *            with no data, and changing the code in IR_Decode to look for   
 *            these calls and use them to generate the key up messages.
 *
 * Hardware Control:
 *            There is a hardware pulse filter which is set to discard any
 *            pulses less than the threshold. The threshold is controlled by
 *            IR_FILTER_VALUE in RC5DECOD.H. The default is to filter out
 *            pulses 700us or less.
 *
 *            The filter has an unfortunate behavior that means when it filters
 *            out a glitch it does not store the glitch time and polarity, but
 *            just stores the next pulse as the same polarity as the previous 
 *            one. It would be possible to amalgamate two like-polarity pulses
 *            prior to decoding to try to get rid of this, but this would only 
 *            work for very short glitches (or the missing counts from the 
 *            glitch would upset the count and take it out of the "valid" bins).
 *
 *            There is no logic in this implementation to perform this
 *            pulse amalgamation.
 *
 *            It would also be possible to turn off the glitch filter and do 
 *            the filtering in software, but there is a danger of overflowing
 *            the hardware fifo with lots of short glitches. It might make 
 *            sense to set the glitch filter to a small-ish value to avoid
 *            fifo overruns, and then perform pulse amalgamation.
 *
 * Tracing/Debug:
 *            Setting breakpoints on the code to debug why a pulse is not being
 *            decoded is futile in this type of real-time decode, as FIFO's get
 *            overrun quickly. In order to facilitate trace, this decode module
 *            prints out information about the ongoing decode state using the 
 *            trace flag "TR_IRD" which is defined to be TRACE_IRD (IR Decode)
 *            at TRACE_LEVEL_1. To see what is being received and the decode
 *            process, it is best to redefine this to be TRACE_LEVEL_2
 *            which is automatically set up for output by GENIRTST.
 * 
 * Author: Steve Glennon
 *
 ****************************************************************************/
/* $Header: rc5decod.c, 4, 4/19/04 11:20:30 AM, Matt Korte$
 ****************************************************************************/ 

#include "hwconfig.h"
#include "kal.h"
#include "retcodes.h"
#include "sabine.h"
#include "confmgr.h"
#include "genir.h"
#include "rc5ktabl.h"
#include "rc5decod.h"

extern void FlashIRLED(void);

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
      lpFrontCtrl = (LPREG)IRD_CONTROL_REG;
      lpFrontClkCtrl = (LPREG)IRD_RX_CLK_CONTROL_REG;
      lpFrontFilter = (LPREG)IRD_LOWPASS_FILTER_REG;
      lpFrontIntEnable = (LPREG)IRD_INT_ENABLE_REG;     

      /* Critical session added because txir test might be access these registers at the same time. */
      ks = critical_section_begin();
      * lpFrontCtrl = IRD_WINDOW_33 | (IRD_EDGE_ANY<<IRD_EDGE_CTRL_SHIFT) 
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

   enum irstate retcode = pinstance -> current_state;

   switch (pinstance -> current_state)
   {
   case STATE_WAITING_START:
      pinstance->num_bits = BITS_PER_PACKET;   
      pinstance->packet = 1;              /* First bit (S0) is always a 1 */
      retcode = STATE_GETTING_BITS;       /* Assume switch to next state  */

      /* get the pulse information */
      get_pulse(pinstance, pdatainfo, pulse_count, &pulse);
      
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
      if (pulse.shortpulse)
      {
         trace_new(TR_IRD,"SP ");
         
         /* If we were already mid-bit, use this short pulse to provide data. */
         /* If we were not, then we can effectively ignore the short pulse    */
         /* since we already got the data.                                    */
         if (pinstance->mid_bit != 0)
         {
            /* Since this is a short pulse, we know that the new bit is the same as the last */
            new_bit = pinstance->prev_bit;
            pinstance->packet = (pinstance->packet << 1) | (new_bit);
            pinstance->num_bits--;
            trace_new(TR_IRD,"MB, pkt=0x%4.4X", pinstance->packet);
         
         } /* endif mid bit */
         
         /* since this was a short pulse, we alternated from being mid-bit */
         pinstance->mid_bit ^= 1;
      } 
      else if (pulse.longpulse)
      {
         trace_new(TR_IRD,"LP ");
         /* We should only get long pulses from mid-bit */
         if (pinstance->mid_bit != 0)
         {
            /* Long pulse means the bit alternated from the previous bit */
            new_bit = pinstance->prev_bit ^ 1;
            pinstance->prev_bit = new_bit;
            pinstance->packet = (pinstance->packet << 1) | (new_bit);
            pinstance->num_bits--;
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
         if (pinstance->packet != pinstance->last_packet)
         {
            trace_new(TR_IRD,"\nRC5DECOD: ALL Bits, decode.");

            trace_new(TRACE_IRD | TRACE_LEVEL_2 | TRACE_NO_TIMESTAMP,
                      "\nRC5DECOD: Address %d, Command 0x%2.2X\n",
                      ((pinstance->packet >> 6) & 0x1F),
                      ((pinstance->packet & 0x3F) | ((pinstance->packet & 0x00001000) ? 0 : 0x40)));
            
            pinstance->last_packet = pinstance->packet;
            /* Now check the IR device address matches our criteria */
            if (((pinstance->packet >> 6) & DEVICE_ADDRESS_MASK) == DEVICE_ADDRESS_MATCH)
            {
               FlashIRLED();
               pinstance->last_matched = TRUE;
               /* Extract the bottom 6 bits of the command */
               raw_code = (pinstance->packet & 0x0000003F);
         
               /* If the second start bit is 0, set 7th bit of command */
               if ((pinstance->packet & 0x00001000) == 0)
               {
                  raw_code |= 0x0040;
               } /* endif */
         
               /* Translate the raw code and set the pkeyinfo structure */
               pkeyinfo->key_code = rc5_xlate_table[raw_code];
               pkeyinfo->key_state = IRRX_KEYDOWN;
               /* have to update device_type to validate key information */
               pkeyinfo->device_type = IRRX_REMOTEBUTTON;

               /* Make note that we sent a keydown code */
               pinstance->last_cnxtcode = pkeyinfo->key_code;
               pinstance->sent_key_up = FALSE;
            }
            else
            {
               pinstance->last_matched = FALSE;
            } /* endif */
         }
         else
         {
            trace_new(TR_IRD,"\nRC5DECOD: ALL Bits, discard (same as last).");
            /* if the packet is the same as the last time, and the address */
            /* matched OK (we generated a keypress) flash the LED again.   */
            if (pinstance->last_matched)
            {
               FlashIRLED();
            } /* endif */
            
         } /* endif packet not same as last */
         
      } /* endif no bits left to get */
      break;  /* STATE_GETTING_BITS */
   default:
      trace_new(TR_IRD,"BAD STATE!!!!\n");
      break;   
      
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
   u_int32 LogicLevel0;

   /* Get the information from the queue */
   LogicLevel0 = (pdatainfo->data[pdatainfo->iread] & IRD_BIT_LEVEL) >> 16;
   FifoCount0 = pdatainfo->data[pdatainfo->iread] & IRD_DATA_MASK;
   pulse_count--;
   pdatainfo->iread++;   
   pdatainfo->iread &= IR_INDEX_MASK;

   /* Initialize the pulse structure */
   ppulse->length         = FifoCount0;
   ppulse->high           = LogicLevel0;
   ppulse->shortpulse     = FALSE;
   ppulse->longpulse      = FALSE;
   ppulse->interwordpulse = FALSE;

   /* Now check against our long/short/interword criteria */
   if ((FifoCount0 >= SPULSE_MIN) && (FifoCount0 <= SPULSE_MAX))
   {
      ppulse->shortpulse = TRUE;
      trace_new(TR_IRD, "S ");
      
   } 
   else if ((FifoCount0 >= LPULSE_MIN) && (FifoCount0 <= LPULSE_MAX)) 
   {
      ppulse->longpulse = TRUE;
      trace_new(TR_IRD, "L ");
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
   if (!ppulse->shortpulse && !ppulse->longpulse && !ppulse->interwordpulse)
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
 *  4    mpeg      1.3         4/19/04 11:20:30 AM    Matt Korte      CR(s) 
 *        8862 8863 : Fix Warnings
 *  3    mpeg      1.2         10/25/02 6:51:30 PM    Steve Glennon   SCR(s): 
 *        4845 
 *        Fixed comments to show how to reduce time before key up is reported.
 *        
 *        
 *  2    mpeg      1.1         10/24/02 2:10:46 PM    Steve Glennon   SCR(s): 
 *        4834 
 *        Modified decode function to output the device address and command 
 *        code irrespective of whether it is in the allowable device address 
 *        range. This is to allow us to more easily create new code tables with
 *         a new remote.
 *        
 *        
 *  1    mpeg      1.0         10/15/02 6:23:40 PM    Steve Glennon   
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

