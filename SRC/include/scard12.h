/***************************************************************************/
/*                            Conexant Systems                             */
/***************************************************************************/
/*                                                                         */
/* Filename:       scard12.h                                               */
/*                                                                         */
/* Description:    OTV 1.2 & EN2 Smart card driver public header           */
/*                                                                         */
/* Author:         Senthil Veluswamy                                       */
/*                                                                         */
/* Date:           08/31/00                                                */
/*                                                                         */
/* Copyright Conexant Systems, 2000, 2002                                  */
/* All Rights Reserved.                                                    */
/*                                                                         */
/***************************************************************************/
/* $Header: scard12.h, 2, 11/4/02 5:39:54 PM, Billy Jackman$
****************************************************************************/

#ifndef _SCARD12_H_
#define _SCARD12_H_

/*****************/
/* Include Files */
/*****************/

/***********/
/* Aliases */
/***********/

#define SC_STATUS_OK            (0)
#define SC_STATUS_BAD_ID        (-1)
#define SC_STATUS_BAD_OPTION    (-2)

#define SC_OPTION_FAST_TURNAROUND       (1)

/***********************/
/* Function prototypes */
/***********************/

int sc12_set_option( int slot_id, u_int32 option, u_int32 value );
int sc12_get_option( int slot_id, u_int32 option, u_int32 *value );


#endif /* _SCARD12_H_ */

 /****************************************************************************
 * Modifications:
 *
 * $Log: 
 *  2    mpeg      1.1         11/4/02 5:39:54 PM     Billy Jackman   SCR(s) 
 *        4886 :
 *        Added defines and prototypes to allow dynamic setting of the fast
 *        turnaround fix at run time.
 *        
 *  1    mpeg      1.0         8/31/00 6:27:30 PM     Senthil Veluswamy 
 * $
 * 
 *    Rev 1.1   04 Nov 2002 17:39:54   jackmaw
 * SCR(s) 4886 :
 * Added defines and prototypes to allow dynamic setting of the fast
 * turnaround fix at run time.
 *
 ****************************************************************************/
