/***************************************************************************/
/*                            Conexant Systems                             */
/***************************************************************************/
/*                                                                         */
/* Filename:       scdata.h                                                */
/*                                                                         */
/* Description:    Smart card Data tables                                  */
/*                                                                         */
/* Author:         Senthil Veluswamy                                       */
/*                                                                         */
/* Date:           09/01/00                                                */
/*                                                                         */
/* Copyright Conexant Systems, 2000                                        */
/* All Rights Reserved.                                                    */
/*                                                                         */
/***************************************************************************/

#ifndef _SCDATA_H_
#define _SCDATA_H_

/*****************/
/* Include Files */
/*****************/
#include <basetype.h>
#ifdef OPENTV_12
#include <opentv_12.h>
#elif defined OPENTV_EN2
#include <opentv_en2.h>
#include <otxsmcrd.h>
#endif

/***********/
/* Aliases */
/***********/
#define RFU                  -1

/* Clock rate table - indicates the F value to be used. */
typedef struct{
    int Fi;
    int Fmax;
}clock_rate_tag;

/* ATR parameter Tables. */
static clock_rate_tag clock_rate_table[] = /* Fi, Fmax values defined in Protocol. */
            {   {372, 4},
                {372, 5},
                {558, 6},
                {744, 8},
                {1116, 12},
                {1488, 16},
                {1860, 20},
                {RFU, RFU},
                {RFU, RFU},
                {512, 5},
                {768, 7},
                {1024, 10},
                {1536, 15},
                {2048, 20},
                {RFU, RFU},
                {RFU, RFU}  };

static int16 baud_rate_table[] =                 /* Values defined in Protocol. */
        {RFU, 1, 2, 4, 8, 16, 32, RFU, 12, 20, RFU, RFU, RFU, RFU, RFU, RFU};

typedef enum 
{
   SC_CONVENTION_USED_DIRECT=0,
   SC_CONVENTION_USED_INVERSE
} scd12_convention;

typedef enum 
{
   SC_PROTOCOL_USED_T0=0,
   SC_PROTOCOL_USED_T1
} scd12_protocol;

/* Direction to be used for a T=0 Read or Write */
typedef enum
{
   DIRECTION_WRITE = 0,
   DIRECTION_READ,
   DIRECTION_NONE          /* To be used for T=1 Protocol */
} T0_Direction;

#ifndef NO_SCARD_DEFAULT_COMM  /* To avoid warnings when included in 
				  sc12isr, which doesn't use
				  default_comm_params */
/* Specify a set of default comm parameters to be used after reset */
static comm_params default_comm_params = 
            {   /* most of the values are 7816-3 ATR defaults     */
               SC_CONVENTION_USED_DIRECT,    /* convention        */
               SC_PROTOCOL_USED_T0,          /* protocol          */
               0,                            /* FI                */
               1,                            /* DI                */
               5,                            /* PI1               */
               0,                            /* PI2               */
               50,                           /* II                */
               0,                            /* N                 */
               (u_int8*)NULL,                /* *historical       */
               0,                            /* historical_length */
               0,                            /* retry             */
               0                             /* timeout           */
            };

#endif

/* To be used with SQC */
#define CARD_VOLT_33TENS   33
#define CARD_VOLT_50TENS   50

#endif /* _SCDATA_H_ */

