/****************************************************************************/
/*                 Conexant Systems, Inc.                                   */
/****************************************************************************/
/*                                                                          */
/* Filename:           CFG_DVB.H                                            */
/*                                                                          */
/* Description:        Satellite Demodulator Configuration Enumerations     */
/*                                                                          */
/* Author:             Raymond Mack                                         */
/*                                                                          */
/* Copyright Conexant Systems, Inc. 2002                                    */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/
/*
$Header $
*/
#ifndef ___CFG_DVB_INCLUDE___

#define ___CFG_DVB_INCLUDE___

/*Viterbi encoding */

/* Internal representation */
enum CodeRates { 
                 RATE_1_2=0x10,
                 RATE_2_3=0x20, 
                 RATE_3_4=0x40, 
                 RATE_5_6=0x100, 
                 RATE_7_8=0x200, 
                 RATE_4_5=0x400, 
                 RATE_3_5=0x800,
                 RATE_6_7=0x1000, 
                 RATE_5_11=0x2000, 
                 RATE_K_N=0x4000,
                 RATE_UNKNOWN=0x8000 
};

/* FEC as defined in DVB delivery descriptor */
#define DVB_FEC_12                  1
#define DVB_FEC_23                  2
#define DVB_FEC_34                  3
#define DVB_FEC_56                  4
#define DVB_FEC_78                  5
#define DVB_FEC_11                  6
#define DVB_FEC_67                  7

#define DVB_FEC_MASK  0x0F

/* LNB Polarity selection configuration.  Controls 12/18 volt selection
   These are duplicates.  An LNB does either left/right or horizontal/vertical
   but never both sets at the same time. */

#define RIGHT_IS_LOW   1
#define RIGHT_IS_HIGH  2
#define VERT_IS_LOW    1
#define VERT_IS_HIGH   2

/* Polarization definitions */

/* Standard DVB position and shift. These are NOT used with the   */
/* definitions below which are bit-masks used by the demod driver */
#define DVB_POL_MASK       (3 << 9)
#define DVB_POL_SHIFT       9
#define DVB_POL_HORIZONTAL (0 << 9)
#define DVB_POL_VERTICAL   (1 << 9)
#define DVB_POL_LEFT       (2 << 9)
#define DVB_POL_RIGHT      (3 << 9)

/* Internal definitions which can be ORed together */
#define HORIZONTAL      1
#define VERTICAL        2
#define LEFT            4
#define RIGHT           8


#endif
/*
$Log: 
 2    mpeg      1.1         2/13/03 1:24:08 PM     Billy Jackman   SCR(s) 5077 
       :
       Add FEC rate 6/7 as legal.  This is used in DSS tuning mode.
       
 1    mpeg      1.0         4/12/02 2:26:30 PM     Ray Mack        
$
*/

