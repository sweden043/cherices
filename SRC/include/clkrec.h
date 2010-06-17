/****************************************************************************/
/*                   Conexant Proprietary and Confidential                  */
/*                       Software File/Module Header                        */
/*              Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002      */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       clkrec.h
 *
 *
 * Description:    Definitions and prototypes for the nechescr driver, which
 *                 is the general clock recovery driver.
 *
 *
 * Author:         Matthew W. Korte
 *
 ****************************************************************************/
/* $Header: clkrec.h, 4, 7/29/03 10:55:46 AM, Tim Ross$
 ****************************************************************************/
#ifndef _CLKREC_H_
#define _CLKREC_H_

typedef void (*clk_rec_ISRNotify_t)(void);
typedef void (*clk_rec_PCRNotify_t)(u_int32 PCR_High, u_int32 PCR_Low);
typedef void (*clk_rec_PCRFailCB_t)(void);
void clk_rec_suspend(void);
void clk_rec_resume(void);

#endif
/****************************************************************************
 * Modifications:
 * $Log: 
 *  4    mpeg      1.3         7/29/03 10:55:46 AM    Tim Ross        SCR(s) 
 *        7064 :
 *        Add clk_rec_suspend() and clk_rec_resume() to API.
 *        
 *  3    mpeg      1.2         9/3/02 7:32:12 PM      Matt Korte      SCR(s) 
 *        4498 :
 *        Remove warnings
 *        
 *  2    mpeg      1.1         8/16/00 5:07:56 PM     Amy Pratt       added 
 *        type definition for callback at PCR stream failure.
 *        
 *  1    mpeg      1.0         6/15/00 7:42:58 PM     Amy Pratt       
 * $
 * 
 *    Rev 1.3   29 Jul 2003 09:55:46   rossst
 * SCR(s) 7064 :
 * Add clk_rec_suspend() and clk_rec_resume() to API.
 * 
 *    Rev 1.2   03 Sep 2002 18:32:12   kortemw
 * SCR(s) 4498 :
 * Remove warnings
 ****************************************************************************/

