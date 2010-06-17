/***********************************************************************
*                                                                       
*        Copyright (c) 1993 - 2001 Accelerated Technology, Inc.           
*                                                                       
* PROPRIETARY RIGHTS of Accelerated Technology are involved in the      
* subject matter of this material.  All manufacturing, reproduction,    
* use, and sales rights pertaining to this subject matter are governed  
* by the license agreement.  The recipient of this software implicitly  
* accepts the terms of the license.                                     
*                                                                       
************************************************************************

************************************************************************
*                                                                       
*   FILE NAME                                       VERSION           
*                                                                    
*       NCL.H                                         4.4          
*                                                                               
*   COMPONENT                                                           
*                                                                       
*       NET - NET 'C' Library 
*                                                                       
*   DESCRIPTION                                                         
*                                                                       
*       This file contains prototypes and macros for the NCL.C 
*       module.
*                                                                       
*   DATA STRUCTURES
*
*       None
*
*   DEPENDENCIES                                                        
*           
*       None                                                            
*                                                                       
***********************************************************************/
#ifndef NCL_H
#define NCL_H

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++  */
#endif /* _cplusplus */

/***********************************************************************
*   Macros to negate an unsigned long, and convert to a signed long 
*   and to negate an unsigned int, and convert to a signed int.     
*   It is needed to prevent possible overflows for large negatives. 
*   These macros should work on any form of integer representation. 
************************************************************************/
#define INT_MAX     32767
#define LONG_MAX    2147483647UL

#define NCL_SNEGATE(uvalue)    ( ( uvalue <= LONG_MAX )         \
                ?  ( - (long) uvalue )                          \
                :  ( - (long)(uvalue-LONG_MAX) - LONG_MAX ) )

#define NCL_SINEGATE(uvalue)   ( ( uvalue <= INT_MAX )          \
                ?  ( - (int) uvalue )                           \
                :  ( - (int)(uvalue-INT_MAX ) - INT_MAX ) )

/**********************************************************************
* Macros to test if a character is a digit and to test if
*  a character is a space. These are used by functions within the
*  NCL.C module.
***********************************************************************/
#define NCL_IS_DIGIT(c) ( (int) (c >= '0' && c <= '9') )
#define NCL_IS_SPACE(c) ( (int) (c == 0x20 || (c >= 0x09 && c <= 0x0D) ))
#define NCL_IS_ASCII(c) ( (unsigned int) (c <= 0x7f) )


/**********************************************************************
* Function prototypes for the NCL.C module.
***********************************************************************/
char *NCL_Ultoa(unsigned long val, char buf[], int base);
char *NCL_Itoa(int value, char *string, int radix);
int   NCL_To_Upper(int ch);
int   NCL_Stricmp(register const char *s1, register const char *s2);
long  NCL_Atol (const char *nptr);
int   NCL_Atoi (const char *nptr);

#ifdef          __cplusplus
}
#endif /* _cplusplus */

#endif /* NCL_H */

