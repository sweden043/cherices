
/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*           Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002         */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       nup_cfg.h
 *
 *
 * Description:    Nucleus+ BSP configuration definitions
 *
 *
 * Author:         Miles Bintz
 *
 ****************************************************************************/
/* $Header: nup_cfg.h, 4, 10/10/02 2:24:40 PM, Dave Wilson$
 ****************************************************************************/

#ifndef _NUP_CFG_H_
#define _NUP_CFG_H_

#define KAL_INT_HISR_STACK_SIZE     2048

#define TM_HISR_STACK_SIZE          240
#define TM_HISR_PRIORITY            2

/* Call Nucleus on every hardware timer tick */
#define SYS_TIMER_DIVIDE            1

#endif /* _NUP_CFG_H_ */

/****************************************************************************
 * Modifications:
 * $Log: 
 *  4    mpeg      1.3         10/10/02 2:24:40 PM    Dave Wilson     SCR(s) 
 *        4765 :
 *        Clarified and corrected some commenting. Also replaced a few 
 *        hardcoded
 *        numbers with the correct labels from the chip header file.
 *        
 *  3    mpeg      1.2         10/2/00 8:25:54 PM     Miles Bintz     removed 
 *        transkit_memory_size define
 *        
 *  2    mpeg      1.1         10/2/00 7:16:08 PM     Miles Bintz     added 
 *        define which controls how much memory to give Transkit
 *        
 *  1    mpeg      1.0         8/25/00 4:25:46 PM     Ismail Mustafa  
 * $
 * 
 *    Rev 1.3   10 Oct 2002 13:24:40   dawilson
 * SCR(s) 4765 :
 * Clarified and corrected some commenting. Also replaced a few hardcoded
 * numbers with the correct labels from the chip header file.
 *
 ****************************************************************************/
