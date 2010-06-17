/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                 Conexant Systems Inc. (c) 1998-2002                      */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:      rc38kdecod.h
 *
 * Description:   Header file for the new NEC RC38K IR decoder driver (ir)
 *
 * Author:        Sunbey Tu
 *
 ****************************************************************************/
/* $Header:ir38kdecode.h, 1, 2004-6-25 11:13:33, Sunbey Tu$
 ****************************************************************************/

#ifndef _RC38KDECOD_H_
#define _RC38KDECOD_H_

/*********************************************************************************** 
 * NOTE: Please refer to Infra Remote Controller section of the CX22490/1/6 specification  
 *       for the register descriptions.                                            
 **********************************************************************************/ 

/* Value for Receive Clock Divider Register FOR 38k. See spec for calulation formula. */
/* 38k = 54000000/16/(rx_clk+1) */
#define IRRX_CLK_CTRL  88
  
/* Value for Low Pass Filter. Any pulse that is shorter than NEC short pulse is filtered. */
/* 560usec is the short pulse RC38k, use a smaller number to set LPF                      */
#define IR_FILTER_VALUE 0x4000 

/* Debug messaging functions */
#define IR_ERROR_MSG    (TRACE_IRD|TRACE_LEVEL_ALWAYS)
#define IR_FUNC_TRACE   (TRACE_IRD|TRACE_LEVEL_2)
#define IR_MSG          (TRACE_IRD|TRACE_LEVEL_2)
#define IR_ISR_MSG      (TRACE_IRD|TRACE_LEVEL_1)

/* Number of bits to decode for RC38k protocol. this number related the algorithm of decode */
#define BITS_PER_PACKET 32    

/* Following are the precalculated FIFO count values for RC38k short and long pulses.            */
/* According to Nec spec,  pulse have lasts 9ms ,4.5ms,0.65ms,...                                */
/* They were calculated by using formula: PulseDuration = RxFIFOCount * 4 / RxClockFreq,         */
/* where RxClockFreq is 16 times the carrier frequency(38k). A 20 percent tolerance is allowed.  */


/* repeat 9ms point for interword gap is 1368 or 0x558 */
#define HEAD_MIN   0x4CF  /* 90% of interword gap  */
#define HEAD_MAX   0x5E1  /* 110% of interword gap */


/* Start point for 4.5ms is count of 684 or 0x2ac */
#define BEGIN_MIN   0x23a 
#define BEGIN_MAX   0x335

/* '0' point for logical low 0.565ms is count of 86 or 0x56  20% left */
/* 0.565ms of logical high is the leading pulse of every bit or repeat pulse */
#define SPULSE_MIN   0x30  
#define SPULSE_MAX   0x90  

/* '1' point for 1.69ms  is count of 257 or  0x101   20% of offset*/
#define LPULSE_MIN   0xd6  
#define LPULSE_MAX   0x134  

/* repeat 40.5ms point for interword gap is 6156 or 0x180c */
#define F_END_MIN   0x1220  
#define F_END_MAX   0x1a73  

/* SECOND LOW point for 96.19ms  is count of 14621 or 0x391C  */
#define REPEAT_MIN   0x3366
#define REPEAT_MAX   0x3ED2

/* one repeat start point for 2.25ms  is count of 343 or 0x156  */
#define REPEAT_BEGIN_MIN   0x134
#define REPEAT_BEGIN_MAX   0x178


#define KEYUP_PACKET 0xffffffff
#define REMOTE_SYS_CODE 0x3334 //add by wlk ,ir romote control,12-30-2004
#define REMOTE_SYS_CODE2 0x0234
/* Internal state machine states */
enum irstate
{
   STATE_WAITING_START,
   STATE_GETTING_BITS,
   STATE_REPEAT
};

/* Iner state machine for STATE_REPEAT */
enum inerstate
{
   HEAD_GETTING,
   REPEAT_BEGIN_GETTING,
   BIT_HEAD_GETTING,
   REPEAT_GETTING
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
   bool    head;                     /* Is a 9ms head received? */
   bool    begin;                    /* Is a 4.5ms start pulse received */
   bool    shortpulse;               /* Is it a short pulse of 0.56ms */
   bool    longpulse;                /* Is it a long pulse of 1.69ms */
   bool    f_end;                    /* Is it an end pulse of 40.5ms followed by a repeat message */
   bool    bit_head;                 /* Is it a leading pulse of a bit or a repeat message */
   bool    repeat_begin;             /* Is it repeat begin */
   bool    repeat;                   /* Is it repeat long */
} PULSE, * PPULSE;   

/*********************/
/* Public Prototypes */
/*********************/
void        IR_Init(    IRRX_PORTID portid);
IRRX_STATUS IR_SW_Init( IRRX_PORTID portid, void **ppInstance);
void        IR_Decode(  void * pinst_decode, 
                        PIRRX_DATAINFO pdatainfo, 
                        PIRRX_KEYINFO pkeyinfo );
u_int32 ir_get_rep_interval();
void ir_set_rep_interval( u_int32 value );

#endif /* _RC38kECOD_H_ */
 /****************************************************************************
 * Modifications:
 * $Log:
 *  1    mpeg      1.0         2004-6-25 11:13:33     Xiao Guang Yan  CR(s)
 *       9583 9584 : Header file for IR38K remote control driver.
 * $
 * 
 *    Rev 1.0   15 Oct 2002 16:00:36   glennon
 * SCR(s): 4796 
 * Header file for new-style RC5 IR decode
 * 
 ****************************************************************************/ 

