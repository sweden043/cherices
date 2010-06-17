/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*          Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002          */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        ECC.C
 *
 *
 * Description:     ECC Error Checking and Correcting functions
 *
 *
 * Author:          Sunil Cheruvu
 *
 ****************************************************************************/
/* $ $
 ****************************************************************************/


/*****************/
/* Include Files */
/*****************/
#include <string.h>

/******************************************************************************/
/*								 	                                                   */
/*     ECC Code for NANDUSB						                                 */
/*     Developed by Flash S/W Team of Product Planning Group                  */
/*     Copyright (C) 2003, SAMSUNG Electronics.		 		                     */
/*     All Rights Reserved.						                                 */
/*							 		                                                   */
/*									                                                   */
/*	   DESCRIPTION :						                                          */
/*									                                                   */
/*        # author1 : Young Gon Kim					                              */
/*        # author2 : Jong Baek Jang					                           */
/*        # author3 : Jae Bum Lee					                              */
/*        # Version : $Revision: 3$					                              */
/*									                                                   */
/******************************************************************************/

#include <string.h>
#include "stbcfg.h"
#include "basetype.h"
#ifndef DLOAD
#if RTOS == VXWORKS
    #include <cacheLib.h>
#endif
#include "kal.h"
#endif
#ifdef DLOAD
#include "defs.h"
#endif
#include "startup.h"
#include "retcodes.h"
#include "flashrw.h"
#include "ecc.h"

#ifdef DLOAD
   #if DOWNLOAD_SERIAL_SUPPORT==YES
      #include "libfuncs.h"
   #else
      #include <stdio.h>
   #endif
#endif


/*****************************************************************************/
/* Address Types                                                             */
/*****************************************************************************/


/*****************************************************************************/
/* Integer Types                                                             */
/*****************************************************************************/



/*****************************************************************************/
/*                                                                           */
/* # NAME : ECC Code for 512 Byte                                            */
/*						                                                           */
/* # DESCRIPTION :                                                           */
/* 		    This function generates 3 byte ECC for 512 byte data.           */
/*		    (Software ECC)                                                     */
/*			 ECC operating based on the ECC algorithm. 						        */
/* # PARAMETERS :                                                            */
/* 		    ecc_buf	the location where ECC should be stored                 */
/*		    data_buf	given data                                              */
/*								 	                                                  */
/* # RETURN VALUES : None                                                    */
/*			                                                                    */
/*****************************************************************************/

void make_ecc_512Byte( u_int8* temp_data, u_int16* ecc_gen)
{
	u_int32	iIndex;

	u_int32	iLsum, iParity;

	u_int32	iParityLo;
	u_int32	iParityHi;

	u_int8		iInvLow, iInvHigh;
	u_int8		iLow, iHigh;

	u_int8		iP1_1, iP1_2, iP2_1, iP2_2, iP4_1, iP4_2, iP8_1, iP8_2;
	u_int8		iP16_1, iP16_2, iP32_1, iP32_2, iP64_1, iP64_2, iP128_1, iP128_2;
	u_int8		iP256_1, iP256_2, iP512_1, iP512_2, iP1024_1, iP1024_2, iP2048_1, iP2048_2;

	u_int32	*temp_datal = (u_int32 *)temp_data;

	iParityLo = iParityHi = iLsum = 0;

	// bit position [0 ~ 63]
	for (iIndex = 0; iIndex < 512/8; iIndex++ ) {
	    iParityLo = iParityLo ^ (*(temp_datal + iIndex * 2));
	    iParityHi = iParityHi ^ (*(temp_datal + iIndex * 2 + 1));
	}

	iParity = iParityLo ^ iParityHi;
	for (iIndex = 0; iIndex < 32; iIndex++)
	{
	    iLsum ^= ((iParity >> iIndex) & 0x1);	
	}

	iParity = iParityLo ^ iParityHi;		// bit0
	iParity &= 0x55555555;

	iParity = (iParity >> 16) ^ iParity;
	iParity = (iParity >> 8) ^ iParity;
	iParity = (iParity >> 4) ^ iParity;
	iParity = (iParity >> 2) ^ iParity;
	iParity = (iParity >> 1) ^ iParity;

	iLow = (iParity & 0x1);

	iParity = iParityLo ^ iParityHi;		// bit1
	iParity &= 0x33333333;

	iParity = (iParity >> 16) ^ iParity;
	iParity = (iParity >> 8) ^ iParity;
	iParity = (iParity >> 4) ^ iParity;
	iParity = (iParity >> 2) ^ iParity;
	iParity = (iParity >> 1) ^ iParity;

	iLow |= ((iParity & 0x1) << 1);

	iParity = iParityLo ^ iParityHi;		// bit2
	iParity &= 0x0F0F0F0F;

	iParity = (iParity >> 16) ^ iParity;
	iParity = (iParity >> 8) ^ iParity;
	iParity = (iParity >> 4) ^ iParity;
	iParity = (iParity >> 2) ^ iParity;
	iParity = (iParity >> 1) ^ iParity;

	iLow |= ((iParity & 0x1) << 2);

	iParity = iParityLo ^ iParityHi;		// bit3
	iParity &= 0x00FF00FF;

	iParity = (iParity >> 16) ^ iParity;
	iParity = (iParity >> 8) ^ iParity;
	iParity = (iParity >> 4) ^ iParity;
	iParity = (iParity >> 2) ^ iParity;
	iParity = (iParity >> 1) ^ iParity;

	iLow |= ((iParity & 0x1) << 3);

	iParity = iParityLo ^ iParityHi;		// bit4
	iParity &= 0x0000FFFF;

	iParity = (iParity >> 16) ^ iParity;
	iParity = (iParity >> 8) ^ iParity;
	iParity = (iParity >> 4) ^ iParity;
	iParity = (iParity >> 2) ^ iParity;
	iParity = (iParity >> 1) ^ iParity;

	iLow |= ((iParity & 0x1) << 4);

	iParity = iParityLo;				// bit5

	iParity = (iParity >> 16) ^ iParity;
	iParity = (iParity >> 8) ^ iParity;
	iParity = (iParity >> 4) ^ iParity;
	iParity = (iParity >> 2) ^ iParity;
	iParity = (iParity >> 1) ^ iParity;

	iLow |= ((iParity & 0x1) << 5);
	iLow &= 0x3f;

	if (iLsum == 0) 
	   iInvLow = iLow;
	else 
	   iInvLow = ~iLow & 0x3f;

	iParityLo = iParityHi = iLsum = 0;

	// byte position [0 ~ 63]
	for (iIndex = 0; iIndex < (512/8); iIndex++)
	{
	    u_int32 iDataLo, iDataHi;

   	    iDataLo = (*(temp_datal + iIndex * 2));
	    iDataHi = (*(temp_datal + iIndex * 2 + 1));
	
	    iDataLo = iDataLo ^ iDataHi;
	    iDataLo = (iDataLo >> 16) ^ iDataLo;
	    iDataLo = (iDataLo >> 8) ^ iDataLo;
	    iDataLo = (iDataLo >> 4) ^ iDataLo;
	    iDataLo = (iDataLo >> 2) ^ iDataLo;
	    iDataLo = (iDataLo >> 1) ^ iDataLo;

    	    if (iIndex < 32)
	       iParityLo |= ((iDataLo & 0x01) << iIndex);
	    else
	       iParityHi |= ((iDataLo & 0x01) << (iIndex - 32));
  	       iLsum ^= (iDataLo & 0x1);
	}

	iParity = iParityLo ^ iParityHi;		// bit0
	iParity &= 0x55555555;

	iParity = (iParity >> 16) ^ iParity;
	iParity = (iParity >> 8) ^ iParity;
	iParity = (iParity >> 4) ^ iParity;
	iParity = (iParity >> 2) ^ iParity;
	iParity = (iParity >> 1) ^ iParity;

	iHigh = (iParity & 0x1);

	iParity = iParityLo ^ iParityHi;		// bit1
	iParity &= 0x33333333;

	iParity = (iParity >> 16) ^ iParity;
	iParity = (iParity >> 8) ^ iParity;
	iParity = (iParity >> 4) ^ iParity;
	iParity = (iParity >> 2) ^ iParity;
	iParity = (iParity >> 1) ^ iParity;

	iHigh |= ((iParity & 0x1) << 1);

	iParity = iParityLo ^ iParityHi;		// bit2
	iParity &= 0x0F0F0F0F;

	iParity = (iParity >> 16) ^ iParity;
	iParity = (iParity >> 8) ^ iParity;
	iParity = (iParity >> 4) ^ iParity;
	iParity = (iParity >> 2) ^ iParity;
	iParity = (iParity >> 1) ^ iParity;

	iHigh |= ((iParity & 0x1) << 2);

	iParity = iParityLo ^ iParityHi;		// bit3
	iParity &= 0x00FF00FF;

	iParity = (iParity >> 16) ^ iParity;
	iParity = (iParity >> 8) ^ iParity;
	iParity = (iParity >> 4) ^ iParity;
	iParity = (iParity >> 2) ^ iParity;
	iParity = (iParity >> 1) ^ iParity;

	iHigh |= ((iParity & 0x1) << 3);

	iParity = iParityLo ^ iParityHi;		// bit4
	iParity &= 0x0000FFFF;

	iParity = (iParity >> 16) ^ iParity;
	iParity = (iParity >> 8) ^ iParity;
	iParity = (iParity >> 4) ^ iParity;
	iParity = (iParity >> 2) ^ iParity;
	iParity = (iParity >> 1) ^ iParity;

	iHigh |= ((iParity & 0x1) << 4);

	iParity = iParityLo;					// bit5

	iParity = (iParity >> 16) ^ iParity;
	iParity = (iParity >> 8) ^ iParity;
	iParity = (iParity >> 4) ^ iParity;
	iParity = (iParity >> 2) ^ iParity;
	iParity = (iParity >> 1) ^ iParity;

	iHigh |= ((iParity & 0x1) << 5);
	iHigh &= 0x3f;

	if (iLsum == 0) 
	   iInvHigh = iHigh;
	else 
	   iInvHigh = ~iHigh & 0x3f;

	iP2048_2 = (iHigh & 0x20) >> 5;
	iP2048_1 = (iInvHigh & 0x20) >> 4;
	iP1_2 = (iLow & 0x1) << 2;
	iP1_1 = (iInvLow & 0x1) << 3;
	iP2_2 = (iLow & 0x2) << 3;
	iP2_1 = (iInvLow & 0x2) << 4;
	iP4_2 = (iLow & 0x4) << 4;
	iP4_1 = (iInvLow & 0x4) << 5;

	iP8_2 = (iLow & 0x8) >> 3;
	iP8_1 = (iInvLow & 0x8) >> 2;
	iP16_2 = (iLow & 0x10) >> 2;
	iP16_1 = (iInvLow & 0x10) >> 1;
	iP32_2 = (iLow & 0x20) >> 1;
	iP32_1 = (iInvLow & 0x20);
	iP64_2 = (iHigh & 0x1) << 6;
	iP64_1 = (iInvHigh & 0x1) << 7;

	iP128_2 = (iHigh & 0x2) >> 1;
	iP128_1 = (iInvHigh & 0x2);
	iP256_2 = (iHigh & 0x4);
	iP256_1 = (iInvHigh & 0x4) << 1;
	iP512_2 = (iHigh & 0x8) << 1;
	iP512_1 = (iInvHigh & 0x8) << 2;
	iP1024_2 = (iHigh & 0x10) << 2;
	iP1024_1 = (iInvHigh & 0x10) << 3;

	*(ecc_gen + 0) = ~( iP64_1|iP64_2|iP32_1|iP32_2
			  |iP16_1|iP16_2|iP8_1|iP8_2 );
	*(ecc_gen + 1) = ~( iP1024_1|iP1024_2|iP512_1|iP512_2
			  |iP256_1|iP256_2|iP128_1|iP128_2 );
	*(ecc_gen + 2) = ~( iP4_1|iP4_2|iP2_1|iP2_2|iP1_1|iP1_2
			  |iP2048_1|iP2048_2 );
}

/*****************************************************************************/
/*                                                                           */
/* # NAME : Perform ECC for 512 Byte                                         */
/*				                                             */
/* # DESCRIPTION :                                                           */
/*	This function compares two ECCs and corrects if there is an error.  */
/*								 	     */
/* # PARAMETERS :                                                            */
/*		ecc_data1		first ECC to be compared               */
/*		ecc_data2		second ECC to be compared         */
/*		page_data		content of data page                 */
/*		offset			returned location where the error occurred             */
/*		corrected		correct data                         */
/*									     */
/* # RETURN VALUES :                                                         */
/*		Upon successful completion, compare_ecc returns SSR_SUCCESS. */
/*      Otherwise, corresponding error code is returned.                     */
/*                                                                           */
/*****************************************************************************/

eccdiff_t perform_ecc_512Byte(u_int8 *iEccdata1, 
                              u_int8 *iEccdata2, 
		                        u_int8 *pPagedata, 
		                        u_int32 *pOffset, 
		                        u_int8 *pCorrected)
{
	u_int8	*pEcc1 = (u_int8 *)iEccdata1;
	u_int8	*pEcc2 = (u_int8 *)iEccdata2;
	u_int32	iCompecc = 0, iEccsum = 0;
	u_int8	iNewvalue;
	u_int8	iFindbit;
	u_int32	iFindbyte;
	u_int32	iIndex;
	
	for (iIndex = 0; iIndex < 3; iIndex++)
	    iCompecc |= (((~*pEcc1++) ^ (~*pEcc2++)) << iIndex * 8);
	
	for(iIndex = 0; iIndex < 24; iIndex++) {
	   iEccsum += ((iCompecc >> iIndex) & 0x01);
	}

	switch (iEccsum) {
	case 0 :	/* no error */
			/* actually not reached. */
			/* if this function is not called because two ecc's are equal. */
#if defined(DLOAD)
		printf("RESULT : no error\n");
#endif 
		return ECC_NO_ERROR;

	case 1 :	/* one bit ECC error */
#if defined(DLOAD) 
		printf("RESULT : ecc error\n");
#endif 
		return ECC_ECC_ERROR;

	case 12 :
		iFindbyte = ((iCompecc >> 17 & 1) << 8) + ((iCompecc >> 15 & 1) << 7) + ((iCompecc >> 13 & 1) << 6)
			    + ((iCompecc >> 11 & 1) << 5) + ((iCompecc >> 9 & 1) << 4) + ((iCompecc >> 7 & 1) << 3)
			    + ((iCompecc >> 5 & 1) << 2) + ((iCompecc >> 3 & 1) << 1) + (iCompecc >> 1 & 1);
		iFindbit = ((iCompecc >> 23 & 1) << 2) + ((iCompecc >> 21 & 1) << 1) + (iCompecc >> 19 & 1);
		
		iNewvalue = (pPagedata[iFindbyte] ^ (1 << iFindbit));
		
		if (pOffset != NULL) {
		   *pOffset = iFindbyte;
		}

		if (pCorrected != NULL) {
		   *pCorrected = iNewvalue;
		}
		
		return ECC_CORRECTABLE_ERROR;	/* one bit data error */

	default :				/* one bit ECC error */
#if defined(DLOAD) 
		printf("RESULT : unrecoverable error\n");
#endif 
		return ECC_ECC_ERROR;

	}
}

#if 0
/*****************************************************************************/
/* 									     */
/*  # NAME : Random 1Bit-Error ECC Test :				     */
/*									     */
/*	# DESCRIPTION :							     */
/*		     1. Choose random byte and bit.	     		     */
/*                   2. Make a selected bit move as far as 1bit.	     */
/*                   3. Check the 1bit ECC function for 10K times,	     */
/*	                whether the ECC generates 1bit error.		     */
/*									     */
/*  # PARAMETER :							     */
/*	    	     ran_byte		Selected byte for ECC test at random */
/*		     ran_bit		Selected bit for ECC test at random  */
/*		     datab_corrupted    1bit-moved Data for ECC test	     */
/*									     */
/*  # RETURN VALUES :							     */
/*  		     On successful for 1bit ECC, no value returns.	     */
/*		     Otherwise, return Error message.			     */
/*									     */
/*****************************************************************************/

int main(int argc, char* argv[])
{
    int i, test_number;
    u_int8 datab[512],ecca_save[3],eccb_save[3], datab_corrupted[512];
    u_int32 offset;
    u_int8 corrected;
    eccdiff_t rc;
    u_int16 ran_byte, ran_bit;		  // Selected random byte and bit to test the 1bit ECC
	 u_int16 ecc_gen[3];

    srand( (unsigned)time( NULL ) );      // Random Fuction
	printf("\n\n********* 1bit ECC Example in case of 512Byte ******* \n\n");

	for(test_number=0;test_number<10000;test_number++){    // ECC Test for 10000 times
		
	   ran_byte=rand()%512;		  // Choose byte and bit at random
	   ran_bit=rand()%8;	
	   printf("Test number : %d\r", test_number);

		for (i=3; i<512; i++)
		    datab[i] = i;

		make_ecc_512Byte(datab, &ecc_gen);

		for (i=0; i<3; i++)
		    ecca_save[i]=(u_int8)ecc_gen[i];
		
		for (i=0; i<512; i++)
		    datab_corrupted[i] = datab[i];

		datab_corrupted[ran_byte] = (datab_corrupted[ran_byte] ^ (1 << ran_bit)); 
		// Make a chosen bit move as far as 1bit

		make_ecc_512Byte(datab_corrupted, &ecc_gen);

		for (i=0; i<3; i++)
		{
		    eccb_save[i]=(u_int8)ecc_gen[i];
		}

		rc = perform_ecc_512Byte(ecca_save, eccb_save, datab_corrupted, &offset, &corrected);

		if((offset==ran_byte)&&(corrected==datab[ran_byte])) 
		  continue;

		else printf(" \n Error!!");
	}
	return 0;
}

#endif

/*****************************************************************************/
/*                                                                           */
/* # NAME : ECC Code for 256 Word                                            */
/*						                             */
/* # DESCRIPTION :                                                           */
/*		This function generates 22 bit ECC for 256 word data.        */
/*		(Software ECC)                                               */
/*			 ECC operating based on the ECC algorithm. 						     */
/*									     */
/* # PARAMETERS :                                                            */
/*		ecc_buf		the location where ECC should be stored      */
/*		data_buf	given data                                   */
/*									     */
/* # RETURN VALUES : None                                                    */
/*			                                                     */
/*****************************************************************************/

void make_ecc_256Word( u_int16* temp_data, u_int16* ecc_gen)
{
	u_int32		iIndex;

	u_int32		iLsum, iParity;

	u_int32		iParityLo;
	u_int32		iParityHi;

	u_int8		iInvLow, iInvHigh;
	u_int8		iLow, iHigh;

	u_int16		iP1_1, iP1_2, iP2_1, iP2_2, iP4_1, iP4_2, iP8_1, iP8_2;
	u_int16		iP16_1, iP16_2, iP32_1, iP32_2, iP64_1, iP64_2, iP128_1, iP128_2;
	u_int16		iP256_1, iP256_2, iP512_1, iP512_2, iP1024_1, iP1024_2, iP2048_1, iP2048_2;

	u_int32		*temp_datal = (u_int32 *)temp_data;

	iParityLo = iParityHi = iLsum = 0;

	// bit position [0 ~ 63]
	for (iIndex = 0; iIndex < (256/4); iIndex++ ) {
		iParityLo = iParityLo ^ (*(temp_datal + iIndex * 2));
		iParityHi = iParityHi ^ (*(temp_datal + iIndex * 2 + 1));
	}

	iParity = iParityLo ^ iParityHi;
	for (iIndex = 0; iIndex < 32; iIndex++)
	{
		iLsum ^= ((iParity >> iIndex) & 0x1);	
	}

	iParity = iParityLo ^ iParityHi;		// bit0
	iParity &= 0x55555555;

	iParity = (iParity >> 16) ^ iParity;    
	iParity = (iParity >> 8) ^ iParity;     
	iParity = (iParity >> 4) ^ iParity;     
	iParity = (iParity >> 2) ^ iParity;
	iParity = (iParity >> 1) ^ iParity;

	iLow = (iParity & 0x1);

	iParity = iParityLo ^ iParityHi;		// bit1
	iParity &= 0x33333333;

	iParity = (iParity >> 16) ^ iParity;
	iParity = (iParity >> 8) ^ iParity;
	iParity = (iParity >> 4) ^ iParity;
	iParity = (iParity >> 2) ^ iParity;
	iParity = (iParity >> 1) ^ iParity;

	iLow |= ((iParity & 0x1) << 1);

	iParity = iParityLo ^ iParityHi;		// bit2
	iParity &= 0x0F0F0F0F;

	iParity = (iParity >> 16) ^ iParity;
	iParity = (iParity >> 8) ^ iParity;
	iParity = (iParity >> 4) ^ iParity;
	iParity = (iParity >> 2) ^ iParity;
	iParity = (iParity >> 1) ^ iParity;

	iLow |= ((iParity & 0x1) << 2);

	iParity = iParityLo ^ iParityHi;		// bit3
	iParity &= 0x00FF00FF;

	iParity = (iParity >> 16) ^ iParity;
	iParity = (iParity >> 8) ^ iParity;
	iParity = (iParity >> 4) ^ iParity;
	iParity = (iParity >> 2) ^ iParity;
	iParity = (iParity >> 1) ^ iParity;

	iLow |= ((iParity & 0x1) << 3);

	iParity = iParityLo ^ iParityHi;		// bit4
	iParity &= 0x0000FFFF;

	iParity = (iParity >> 16) ^ iParity;
	iParity = (iParity >> 8) ^ iParity;
	iParity = (iParity >> 4) ^ iParity;
	iParity = (iParity >> 2) ^ iParity;
	iParity = (iParity >> 1) ^ iParity;

	iLow |= ((iParity & 0x1) << 4);

	iParity = iParityLo;				// bit5

	iParity = (iParity >> 16) ^ iParity;
	iParity = (iParity >> 8) ^ iParity;
	iParity = (iParity >> 4) ^ iParity;
	iParity = (iParity >> 2) ^ iParity;
	iParity = (iParity >> 1) ^ iParity;

	iLow |= ((iParity & 0x1) << 5);
	iLow &= 0x3f;

	if (iLsum == 0) 
		iInvLow = iLow;
	else 
		iInvLow = ~iLow & 0x3f;

	iParityLo = iParityHi = iLsum = 0;

	// word position [0 ~ 63]
	for (iIndex = 0; iIndex < (256/4); iIndex++)
	{
		u_int32 iDataLo, iDataHi;

		iDataLo = (*(temp_datal + iIndex * 2));
		iDataHi = (*(temp_datal + iIndex * 2 + 1));
	
		iDataLo = iDataLo ^ iDataHi;

		iDataLo = (iDataLo >> 16) ^ iDataLo;
		iDataLo = (iDataLo >> 8) ^ iDataLo;
		iDataLo = (iDataLo >> 4) ^ iDataLo;
		iDataLo = (iDataLo >> 2) ^ iDataLo;
		iDataLo = (iDataLo >> 1) ^ iDataLo;

		if (iIndex < 32)
			iParityLo |= ((iDataLo & 0x01) << iIndex);
		else
			iParityHi |= ((iDataLo & 0x01) << (iIndex - 32));

		iLsum ^= (iDataLo & 0x1);
	}

	iParity = iParityLo ^ iParityHi;		// bit0
	iParity &= 0x55555555;

	iParity = (iParity >> 16) ^ iParity;
	iParity = (iParity >> 8) ^ iParity;
	iParity = (iParity >> 4) ^ iParity;
	iParity = (iParity >> 2) ^ iParity;
	iParity = (iParity >> 1) ^ iParity;

	iHigh = (iParity & 0x1);

	iParity = iParityLo ^ iParityHi;		// bit1
	iParity &= 0x33333333;

	iParity = (iParity >> 16) ^ iParity;
	iParity = (iParity >> 8) ^ iParity;
	iParity = (iParity >> 4) ^ iParity;
	iParity = (iParity >> 2) ^ iParity;
	iParity = (iParity >> 1) ^ iParity;

	iHigh |= ((iParity & 0x1) << 1);

	iParity = iParityLo ^ iParityHi;		// bit2
	iParity &= 0x0F0F0F0F;

	iParity = (iParity >> 16) ^ iParity;
	iParity = (iParity >> 8) ^ iParity;
	iParity = (iParity >> 4) ^ iParity;
	iParity = (iParity >> 2) ^ iParity;
	iParity = (iParity >> 1) ^ iParity;

	iHigh |= ((iParity & 0x1) << 2);

	iParity = iParityLo ^ iParityHi;		// bit3
	iParity &= 0x00FF00FF;

	iParity = (iParity >> 16) ^ iParity;
	iParity = (iParity >> 8) ^ iParity;
	iParity = (iParity >> 4) ^ iParity;
	iParity = (iParity >> 2) ^ iParity;
	iParity = (iParity >> 1) ^ iParity;

	iHigh |= ((iParity & 0x1) << 3);

	iParity = iParityLo ^ iParityHi;		// bit4
	iParity &= 0x0000FFFF;

	iParity = (iParity >> 16) ^ iParity;
	iParity = (iParity >> 8) ^ iParity;
	iParity = (iParity >> 4) ^ iParity;
	iParity = (iParity >> 2) ^ iParity;
	iParity = (iParity >> 1) ^ iParity;

	iHigh |= ((iParity & 0x1) << 4);

	iParity = iParityLo;				// bit5

	iParity = (iParity >> 16) ^ iParity;
	iParity = (iParity >> 8) ^ iParity;
	iParity = (iParity >> 4) ^ iParity;
	iParity = (iParity >> 2) ^ iParity;
	iParity = (iParity >> 1) ^ iParity;

	iHigh |= ((iParity & 0x1) << 5);
	iHigh &= 0x3f;

	if (iLsum == 0) 
		iInvHigh = iHigh;
	else 
		iInvHigh = ~iHigh & 0x3f;


	iP1_2 = (iLow & 0x1);
	iP1_1 = (iInvLow & 0x1) << 1;
	iP2_2 = (iLow & 0x2) << 1;
	iP2_1 = (iInvLow & 0x2) << 2;
	iP4_2 = (iLow & 0x4) << 2;
	iP4_1 = (iInvLow & 0x4) << 3;

	iP8_2 = (iLow & 0x8) << 3;
	iP8_1 = (iInvLow & 0x8) << 4;
	iP16_2 = (iLow & 0x10) >> 4;
	iP16_1 = (iInvLow & 0x10) >> 3;
	iP32_2 = (iLow & 0x20) >> 3;
	iP32_1 = (iInvLow & 0x20) >> 2;

	iP64_2 = (iHigh & 0x1) << 4;
	iP64_1 = (iInvHigh & 0x1) << 5;
	
	iP128_2 = (iHigh & 0x2) << 5;
	iP128_1 = (iInvHigh & 0x2) << 6;
	iP256_2 = (iHigh & 0x4) << 6;
	iP256_1 = (iInvHigh & 0x4) << 7;
	iP512_2 = (iHigh & 0x8) << 7;
	iP512_1 = (iInvHigh & 0x8) << 8;
	
	iP1024_2 = (iHigh & 0x10) << 8;
	iP1024_1 = (iInvHigh & 0x10) << 9;
	iP2048_2 = (iHigh & 0x20) << 9;
	iP2048_1 = (iInvHigh & 0x20) << 10;

	*(ecc_gen + 0) = ~( iP2048_1|iP2048_2|iP1024_1|iP1024_2|iP512_1|iP512_2|iP256_1|iP256_2
			  |iP128_1|iP128_2|iP64_1|iP64_2|iP32_1|iP32_2|iP16_1|iP16_2);

	*(ecc_gen + 1) = 0xFF00 | ~( iP8_1|iP8_2|iP4_1|iP4_2|iP2_1|iP2_2|iP1_1|iP1_2);
}

/*****************************************************************************/
/*                                                                           */
/* # NAME : Perform ECC for 256 Word                                         */
/*				                                             */
/* # DESCRIPTION :                                                           */
/*	This function compares two ECCs and corrects if there is an error.  */
/*									     */
/* # PARAMETERS :                                                            */
/*		ecc_data1		one ECC to be compared               */
/*		ecc_data2		the other ECC to be compared         */
/*		page_data		content of data page                 */
/*		offset			where the error occurred             */
/*		corrected		correct data                         */
/*									     */
/* # RETURN VALUES :                                                         */
/*	Upon successful completion, compare_ecc 256word returns SSR_SUCCESS. */
/*      Otherwise, corresponding error code is returned.                     */
/*                                                                           */
/*****************************************************************************/

eccdiff_t perform_ecc_256Word(u_int16 *iEccdata1, 
                              u_int16 *iEccdata2, 
			                     u_int16 *pPagedata, 
			                     u_int32 *pOffset, 
			                     u_int16 *pCorrected)
{
	u_int16	*pEcc1 = (u_int16 *)iEccdata1;
	u_int16	*pEcc2 = (u_int16 *)iEccdata2;
	u_int32	iCompecc = 0, iEccsum = 0;
	u_int16	iNewvalue;
	u_int16	iFindbit;
	u_int32	iFindword;
	u_int32	iIndex;
	
	for (iIndex = 0; iIndex < 2; iIndex++)
		iCompecc |= (((~*pEcc1++) ^ (~*pEcc2++)) << iIndex * 16);
	
	for(iIndex = 0; iIndex < 24; iIndex++) {
		iEccsum += ((iCompecc >> iIndex) & 0x01);
	}

	switch (iEccsum) {
	
	case 0 :	/* no error */
			/* actually not reached. */
			/* if this function is not called because two ecc's are equal. */
#if defined(DLOAD) 
		printf("RESULT : no error\n");
#endif
		return ECC_NO_ERROR;

	case 1 :	/* one bit ECC error */
#if defined(DLOAD) 
		printf("RESULT : ecc error\n");
#endif
		return ECC_ECC_ERROR;

	case 12 :
		iFindword = ((iCompecc >> 15 & 1) << 7) + ((iCompecc >> 13 & 1) << 6)
			    + ((iCompecc >> 11 & 1) << 5) + ((iCompecc >> 9 & 1) << 4) 
			    + ((iCompecc >> 7 & 1) << 3)  + ((iCompecc >> 5 & 1) << 2) 
			    + ((iCompecc >> 3 & 1) << 1) + (iCompecc >> 1 & 1);
		
		iFindbit = ((iCompecc >> 23 & 1) << 3) + ((iCompecc >> 21 & 1) << 2) 
			    + ((iCompecc >> 19 & 1) << 1) + (iCompecc >> 17 & 1);
		
		iNewvalue = (pPagedata[iFindword] ^ (1 << iFindbit));
		
		if (pOffset != NULL) {
			*pOffset = iFindword;
		}

		if (pCorrected != NULL) {
			*pCorrected = iNewvalue;
		}
		
		return ECC_CORRECTABLE_ERROR;	/* one bit data error */
	
	default :				/* one bit ECC error */
#if defined(DLOAD) 
		printf("RESULT : unrecoverable error\n");
#endif
		return ECC_ECC_ERROR;

	}

}

#if 0
/*****************************************************************************/
/*									     */
/*  # NAME : Random 1Bit-Error ECC Test :				     */
/*									     */
/*	# DESCRIPTION :							     */
/*		    1. Choose random byte and bit.			     */
/*                  2. Make a selected bit move as far as 1bit.		     */
/*                  3. Check the 1bit ECC function for 10K times,	     */
/*	                   whether the ECC generates 1bit error.	     */
/*									     */
/*  # PARAMETER :							     */
/*		    ran_byte	       Selected byte for ECC test at random  */
/*		    ran_bit	       Selected bit for ECC test at random   */
/*		    datab_corrupted    1bit-moved Data for ECC test	     */
/*									     */
/*  # RETURN VALUES :							     */
/*		    On successful for 1bit ECC, no value returns.	     */
/*		    Otherwise, return Error message.			     */
/*									     */
/*****************************************************************************/


int main(){

	int	i, test_number;
	u_int16  datab[256],ecca_save[2],eccb_save[2], datab_corrupted[256];
	u_int32    offset;
	u_int16  corrected;
	u_int16 ran_byte, ran_bit;     // Selected random byte and bit to test the 1bit ECC
	eccdiff_t rc;
   u_int16 ecc_gen[3];

    srand( (unsigned)time( NULL ) );  // Random Fuction
	printf("\n\n********* 1bit ECC Example in case of 256 Word ******* \n\n");

	for(test_number=0;test_number<10000;test_number++){   // ECC Test for 10000 times
		
		ran_byte=rand()%256;  // Choose byte and bit at random
		ran_bit=rand()%8;	
		printf("Test number : %d\r", test_number);

		for (i=2; i<256; i++)
			datab[i] = i;

		make_ecc_256word(datab, &ecc_gen);
	
		for (i=0; i<2; i++){
			ecca_save[i] = ecc_gen[i];
		}

		for (i=0; i<256; i++)
			datab_corrupted[i] = datab[i];

		datab_corrupted[ran_byte] = (datab_corrupted[ran_byte] ^ (1 << ran_bit)); 
		// Make a chosen bit move as far as 1bit/

		make_ecc_256word(datab_corrupted, &ecc_gen);

		for (i=0; i<2; i++){
			eccb_save[i] = ecc_gen[i];
		}

		rc = perform_ecc_256word(ecca_save, eccb_save, datab_corrupted, &offset, &corrected);

		if((offset==ran_byte)&&(corrected==datab[ran_byte])) 
			continue;

		else printf(" \n Error!!");
	}

	return 0;
}
#endif

/****************************************************************************
 * $Log: 
 *  3    mpeg      1.2         4/23/04 7:24:31 PM     Sunil Cheruvu   CR(s) 
 *        8870 8871 : Fix the DL_SER ADS and SDT build breaks
 *  2    mpeg      1.1         4/23/04 4:44:34 PM     Sunil Cheruvu   CR(s) 
 *        8870 8871 : Fixed the pSoS Build errors.
 *  1    mpeg      1.0         4/22/04 4:17:24 PM     Sunil Cheruvu   CR(s) 
 *        8870 8871 : Added the Nand Flash support for Wabash(Milano rev 5 and 
 *        above) and Brazos.
 * $
 ****************************************************************************/

