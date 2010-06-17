/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                Copyright Conexant Systems Inc. 1998-2003                 */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        davic_adaptation_layer.h
 *
 *
 * Description:     helper file for davic_oobfe_demo driver
 *
 *
 * Author:          Ram B.
 *
 ****************************************************************************/
/* $Header: davic_adaptation_layer.h, 1, 7/11/03 2:46:22 PM, $
 ****************************************************************************/
#ifndef __DAVIC_ADAPTATION_LAYER_H__
#define __DAVIC_ADAPTATION_LAYER_H__

#include <basetype.h>

typedef enum
{
   DAVIC_ADAPT_LAYER_OK = 0,
   DAVIC_ADAPT_LAYER_ERROR,
   DAVIC_ADAPT_LAYER_BAD_ARG,
}DAVIC_ADAPT_LAYER_STATUS;

typedef enum
{
   DAVIC_ADAPT_LAYER_UNKNOWN = (-1),
   DAVIC_ADAPT_LAYER_DIRECTIP,
   DAVIC_ADAPT_LAYER_ETHERNET,
   DAVIC_ADAPT_LAYER_PPP
} DAVIC_ADAPT_LAYER_ENCAPSULATION_TYPE;

void davic_adapt_layer_encapsulation_set(DAVIC_ADAPT_LAYER_ENCAPSULATION_TYPE encapsulation);
DAVIC_ADAPT_LAYER_ENCAPSULATION_TYPE davic_adapt_layer_encapsulation_get(void);
DAVIC_ADAPT_LAYER_STATUS davic_adapt_layer_receive(u_int8 *data_ptr, u_int16 data_len, u_int8 *mBlkPtr);
DAVIC_ADAPT_LAYER_STATUS davic_adapt_layer_send(u_int8 *data_ptr, u_int16 data_len, u_int8 *mblk_ptr);

#endif /* DAVIC_ADAPTATION_LAYER_H */
/*****************************************************************************
 * Modifications:
 * $Log: 
 *  1    mpeg      1.0         7/11/03 2:46:22 PM     Sahil Bansal    
 * $
 * 
 *    Rev 1.0   11 Jul 2003 13:46:22   bansals
 * SCR(s): 6926 
 * New file for davi oobfe demo
 ****************************************************************************/

