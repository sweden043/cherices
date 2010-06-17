/****************************************************************************/
/*                 Rockwell Semiconductor Systems - SABINE                  */
/****************************************************************************/
/*                                                                          */
/* Filename:           C:\sabine\demod\bcd.c                                */
/*                                                                          */
/* Description:        BCD to integer conversion routines                   */
/*                                                                          */
/* Author:             Steve Glennon                                        */
/*                                                                          */
/* Copyright Rockwell Semiconductor Systems, 1998                           */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/
/*
$Header $
$Log: 
 1    mpeg      1.0         3/25/02 6:20:58 PM     Dave Wilson     
$
*/

/*****************/
/* Include files */
/*****************/
#include "basetype.h"
#include "bcd.h"

/*********************/
/* local definitions */
/*********************/
u_int32 factors[] = {1,10,100,1000,10000,100000,1000000,10000000,100000000};

/**********************/
/* internal functions */
/**********************/
u_int32 get_bcd_digit(u_int32 bcd, u_int16 digit);

/******************************************************************************/
/* Function Name: bcdint_to_int                                               */
/*                                                                            */
/* Description  : Converts a bcd value supplied in nibbles of an integer      */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/* Parameters   : u32bcd      unsigned int supplying bcd nibbles. Most sig    */
/*                            digits stored in most significant nibbles. If   */
/*                            <8 digits, digits are 'right aligned' (least    */
/*                            significant digit in bits 3:0 and bits 31:x are */
/*                digits      number of digits in u32bcd. < 8 digits occupy   */
/*                            the least significant part of the u32.          */
/*                                                                            */
/*                dp          Number of digits before decimal point. eg dp=3  */
/*                            indicates 012.34567                             */
/*                                                                            */
/*                multiplier  value to multiple integer and fraction to get   */
/*                            return value                                    */
/*                                                                            */
/* Returns      : int32       converted value, 0xFFFFFFFF if error            */
/*                                                                            */
/******************************************************************************/
u_int32 bcdint_to_int(u_int32 u32bcd,
                      u_int16 digits,
                      u_int16 dp,
                      u_int32 multiplier)
{
   u_int32 result = 0xFFFFFFFF;
   u_int32 our_mult;
   u_int32 val;
   u_int16 i;

   /******************************************/
   /* Check that digits and dp make sense    */
   /******************************************/
   if ((digits == 0) || (digits > 8) || (dp>digits))
      return result;

   /************************************************/
   /* Check that only valid BCD digits are present */
   /************************************************/
   val = u32bcd;
   for (i = 0;i < digits;i++ )
   {
      if ((val & 0x0000000f) >9)
         return result;

      val >>= 4;

   } /* endfor */

   /*******************************************************/
   /* Calculate the factor for the most significant digit */
   /*******************************************************/
   if (dp == 0)
   {
      our_mult = multiplier / 10;
   } else {
      our_mult = multiplier * factors[dp-1];
   }

   /*****************************************************************/
   /* Now do the math on the digits. Start with the multiplier for  */
   /* the most significant digit, and divide the multiplier by 10   */
   /* for each subsequent digit.                                    */
   /*****************************************************************/
   result = 0;
   for (i = digits; i>0 ;i-- )
   {
     result += get_bcd_digit(u32bcd,i) * our_mult;
     our_mult /= 10;
   } /* endfor */

   return result;


} /* bcd_to_int */

/******************************************************************************/
/* Function Name: int_to_bcd                                                  */
/*                                                                            */
/* Description  : Converts an unsigned integer to bcd format                  */
/*                                                                            */
/* Parameters   : value       unsigned int to convert.                        */
/*                                                                            */
/* Returns      : int32       converted value, 0xFFFFFFFF if error            */
/*                                                                            */
/* Example:  int_to_bcd(1234) returns 0x1234                                  */
/*                                                                            */
/******************************************************************************/ 
u_int32 int_to_bcd(u_int32 value)
{
  u_int32 uBCD = 0;
  u_int32 uRemainder;
  u_int32 uWork = value;
  int     iLoop;
  
  /* Numbers larger than 0x99999999 can't be represented in 8 BCD digits */
  if(value > 0x99999999)
    return(0xFFFFFFFF);
  
  /* We convert a maximum of 8 digits of information */
  for (iLoop = 0; iLoop < 8; iLoop++)
  {
    uRemainder = uWork - ((uWork / 10) * 10);
    uBCD = (uBCD >> 4) | (uRemainder << 28);
    uWork = (uWork-uRemainder)/10;
  }
  
  return(uBCD);
}

/******************************************************************************/
/* Function Name: get_bcd_digit                                               */
/*                                                                            */
/* Description  : Gets a numerical value of a BCD digit in the u32bcd value   */
/*                passed. If out of range value, returns 0.                   */
/*                                                                            */
/* Parameters   : u32bcd      unsigned int supplying bcd nibbles.             */
/*                                                                            */
/*                digit       digit to return, 1-8. 1 is least sig, 8 is most */
/*                                                                            */
/* Returns      : value of digit specified (0-9). Returns 0 for invalid val.  */
/*                                                                            */
/******************************************************************************/
u_int32 get_bcd_digit(u_int32 bcd, u_int16 digit)
{
   u_int32 val;
   val = (bcd >> (4*(digit-1))) & 0x0000000F;
   if (val > 9)
      val = 0;

   return val;

} /* get_bcd_digit */
