/****************************************************************************/
/*                 Rockwell Semiconductor Systems - SABINE                  */
/****************************************************************************/
/*                                                                          */
/* Filename:           C:\sabine\include\bcd.h                              */
/*                                                                          */
/* Description:        Header file for BCD conversion routines              */
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
 3    mpeg      1.2         11/29/99 9:54:42 AM    Dave Wilson     Added 
       int_to_bcd function.
       
 2    mpeg      1.1         9/15/98 1:30:08 PM     Steve Glennon   Added more 
       comments on bcdint_to_int
       
 1    mpeg      1.0         9/11/98 6:10:48 PM     Steve Glennon   
$
*/
/******************************************************************************/
/* Function Name: bcdint_to_int                                               */
/*                                                                            */
/* Description  : Converts a bcd value supplied in nibbles of an integer      */
/*                                                                            */
/* Parameters   : u32bcd      unsigned int supplying bcd nibbles. Most sig    */
/*                            digits stored in most significant nibbles. If   */
/*                            <8 digits, digits are 'right aligned' (least    */
/*                            significant digit in bits 3:0 and bits 31:x are */
/*                digits      number of digits in u32bcd.  <8 digits occupy   */
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
/* Example:  bcdint_to_int(0x01234567, 8,3,100) returns 1234 (12.34567*100)   */
/*           bcdint_to_int(0x00123456, 6,3,100) returns 12345 (123.456*100)   */
/*                                                                            */
/******************************************************************************/
u_int32 bcdint_to_int(u_int32 u32bcd, 
                      u_int16 digits, 
                      u_int16 dp, 
                      u_int32 multiplier);

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
u_int32 int_to_bcd(u_int32 value);                      

