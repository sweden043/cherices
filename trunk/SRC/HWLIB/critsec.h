/****************************************************************************/
/*                 Rockwell Semiconductor Systems - SABINE                  */
/****************************************************************************/
/*                                                                          */
/* Filename:           CRITSEC.H                                            */
/*                                                                          */
/* Description:        C prototypes for functions in CRITSEC.S              */
/*                                                                          */
/* Author:             Dave Wilson                                          */
/*                                                                          */
/* Copyright Rockwell Semiconductor Systems, 1998                           */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/

#ifndef _CRITSEC_H_
#define _CRITSEC_H_

void EnableInterrupts(void);
bool DisableInterrupts(void);
bool InterruptsDisabled(void);
u_int32 *GetStackPtr(void);

#endif
