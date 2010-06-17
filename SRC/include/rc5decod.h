/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                 Conexant Systems Inc. (c) 1998-2002                      */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:      rc5decod.h
 *
 * Description:   Header file for the new RC5 IR decoder driver (irrc5new)
 *
 * Author:        Steve Glennon
 *
 ****************************************************************************/
/* $Header: rc5decod.h, 1, 10/15/02 5:00:36 PM, Steve Glennon$
 ****************************************************************************/

#ifndef _RC5DECOD_H_
#define _RC5DECOD_H_

/*********************************************************************************** 
 * NOTE: Please refer to Infra Remote Controller section of the CX22490/1/6 specification  
 *       for the register descriptions.                                            
 **********************************************************************************/ 

/* Value for Receive Clock Divider Register. See spec for calulation formula. */
#define IRRX_CLK_CTRL  93  
/* Value for Low Pass Filter. Any pulse that is shorter than RC5 short pulse is filtered. */
/* 889uSec is the RC5 short pulse value.  I choose 700usec to program the register.       */
#define IR_FILTER_VALUE 0x93a8

/* Debug messaging functions */
#define IR_ERROR_MSG    (TRACE_IRD|TRACE_LEVEL_ALWAYS)
#define IR_FUNC_TRACE   (TRACE_IRD|TRACE_LEVEL_2)
#define IR_MSG          (TRACE_IRD|TRACE_LEVEL_2)
#define IR_ISR_MSG      (TRACE_IRD|TRACE_LEVEL_1)

#define BITS_PER_PACKET 13    /* Number of bits to decode for RC5 protocol. */

/* Following are the precalculated FIFO count values for RC5 short and long pulses.              */
/* According to Philips spec, long pulse lasts 1.778ms and short pulse lasts 0.889ms.            */
/* They were calculated by using formula: PulseDuration = RxFIFOCount * 4 / RxClockFreq,         */
/* where RxClockFreq is 16 times the carrier frequency(36k). A 10 percent tolerance is allowed.  */

/* Mid point for short is count of 128 or 0x80 */
#define SPULSE_MIN   0x58   /* was 0x73 at 10% tolerance - made wider to work better */
#define SPULSE_MAX   0x94   /* was 0x8d at 10% tolerance - made wider to work better */

/* Mid point for short is count of 256 or 0x100 */
#define LPULSE_MIN   0xe6
#define LPULSE_MAX   0x11a

/* Mid point for interword gap is 0x3200 */
#define REPEAT_MIN   0x2d00  /* 90% of interword gap  */
#define REPEAT_MAX   0x3700  /* 110% of interword gap */

#define KEYUP_PACKET 0xffffffff

/* Internal state machine states */
enum irstate
{
   STATE_WAITING_START,
   STATE_GETTING_BITS
};

/**********************************************************/
/* The following struct contains IR decode instance infos */
/* that are to be passed to cnxt_irrx_register() call.    */
/**********************************************************/
typedef struct IRDECODE_INSTANCE {
   enum irstate current_state;       /* Current state of decoder          */
   u_int32 packet;                   /* Current partially recvd packet    */
   u_int32 last_packet;              /* Last complete recvd packet        */
   u_int32 num_bits;                 /* Number of bits received so far    */
   u_int32 prev_bit;                 /* Value of previous bit decoded     */
   u_int32 mid_bit;                  /* Are we in the middle of the bit?  */
   u_int32 last_cnxtcode;            /* Last conexant key code sent       */
   bool    sent_key_up;              /* Have sent key up for last key     */
   bool    last_matched;             /* Last packet matched decode OK     */
} IRDECODE_INSTANCE, * PIRDECODE_INSTANCE;

typedef struct PULSE {
   u_int32 length;                   /* Pulse length                      */
   bool    high;                     /* Is pulse 1?                       */
   bool    shortpulse;               /* Is it in our "short" bracket?     */
   bool    longpulse;                /* Is it in our "long" bracket?      */
   bool    interwordpulse;           /* Is it in our "interword" bracket? */
} PULSE, * PPULSE;   

/*********************/
/* Public Prototypes */
/*********************/
void        IR_Init(    IRRX_PORTID portid);
IRRX_STATUS IR_SW_Init( IRRX_PORTID portid, void **ppInstance);
void        IR_Decode(  void * pinst_decode, 
                        PIRRX_DATAINFO pdatainfo, 
                        PIRRX_KEYINFO pkeyinfo );
#endif /* _RC5DECOD_H_ */
 /****************************************************************************
 * Modifications:
 * $Log: 
 *  1    mpeg      1.0         10/15/02 5:00:36 PM    Steve Glennon   
 * $
 * 
 *    Rev 1.0   15 Oct 2002 16:00:36   glennon
 * SCR(s): 4796 
 * Header file for new-style RC5 IR decode
 * 
 ****************************************************************************/ 
