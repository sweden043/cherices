/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*          Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002          */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        ecc.h
 *
 *
 * Description:     Mode macros
 *
 *
 * Author:          Sunil Cheruvu
 *
 ****************************************************************************/
/* $Header: ecc.h, 1, 4/22/04 4:17:34 PM, Sunil Cheruvu$
 ****************************************************************************/

#ifndef _SAMECC_H
#define _SAMECC_H

/*****************/
/* Include Files */
/*****************/
#include "basetype.h"


typedef enum {
	ECC_NO_ERROR		= 0,			/* no error */
	ECC_CORRECTABLE_ERROR	= 1,			/* one bit data error */
	ECC_ECC_ERROR		= 2,			/* one bit ECC error */
	ECC_UNCORRECTABLE_ERROR	= 3			/* uncorrectable error */
} eccdiff_t;


extern void make_ecc_512Byte( u_int8* temp_data, u_int16* ecc_gen);
extern eccdiff_t perform_ecc_512Byte(u_int8 *iEccdata1, 
                                    u_int8 *iEccdata2, 
		                              u_int8 *pPagedata, 
		                              u_int32 *pOffset, 
		                              u_int8 *pCorrected);

extern void make_ecc_256Word( u_int16* temp_data, u_int16* ecc_gen);
extern eccdiff_t perform_ecc_256Word(u_int16 *iEccdata1, 
                                    u_int16 *iEccdata2, 
			                           u_int16 *pPagedata, 
			                           u_int32 *pOffset, 
			                           u_int16 *pCorrected);

#endif // _SAMECC_

/****************************************************************************
 * Modifications:
 * $Log: 
 *  1    mpeg      1.0         4/22/04 4:17:34 PM     Sunil Cheruvu   CR(s) 
 *        8870 8871 : Added the Nand Flash support for Wabash(Milano rev 5 and 
 *        above) and Brazos.
 * $
 * 
 *    Rev 1.9   03 Sep 2003 15:04:08   whiteth
 * SCR(s) 7424 :
 * Added support for the dual flash aperture.
 * 
 * 
 *    Rev 1.8   10 Apr 2002 16:05:12   whiteth
 * SCR(s) 3509 :
 * Eradicate external bus (PCI) bitfield usage.
 * 
 *
 ****************************************************************************/

