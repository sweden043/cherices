/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*          Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002          */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*                                                                          */
/* Filename:       eefsx.h                                                  */
/*                                                                          */
/* Description:    EE Filesystem Function Prototypes                        */
/*                                                                          */
/* Author:         Senthil Veluswamy                                        */
/*                                                                          */
/* Date:           04/20/00                                                 */
/*                                                                          */
/*                                                                          */
/****************************************************************************/
/* $Header: eefsx.h, 2, 9/6/02 4:14:58 PM, Matt Korte$
 ****************************************************************************/

#ifndef _EEFSX_H_
#define _EEFSX_H_

/*****************/
/* Include Files */
/*****************/
#include <basetype.h>
#include <stdio.h>

/************/
/* Typedefs */
/************/
typedef u_int16 (*ee_rw)(u_int16 address, voidF buffer, u_int16 count, 
                        void *private);

/************************/
/* Function definitions */
/************************/
int ee_filesys_init(char *pathname, int filesys_id, char d_name[20], 
        size_t eesize, size_t secsize, ee_rw ee_r, ee_rw ee_w, void* rw_priv);

#endif // _EEFSX_H_
/****************************************************************************
 * Modifications:
 * $Log: 
 *  2    mpeg      1.1         9/6/02 4:14:58 PM      Matt Korte      SCR(s) 
 *        4498 :
 *        Fixed Warnings
 *        
 *  1    mpeg      1.0         4/20/00 12:47:48 PM    Senthil Veluswamy 
 * $
 * 
 *    Rev 1.1   06 Sep 2002 15:14:58   kortemw
 * SCR(s) 4498 :
 * Fixed Warnings
 ****************************************************************************/
