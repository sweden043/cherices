/*************************************************************************
*                                                                       
*        Copyright (c) 1993 - 2001 Accelerated Technology, Inc.         
*                                                                       
* PROPRIETARY RIGHTS of Accelerated Technology are involved in the      
* subject matter of this material.  All manufacturing, reproduction,    
* use, and sales rights pertaining to this subject matter are governed  
* by the license agreement.  The recipient of this software implicitly  
* accepts the terms of the license.                                     
*                                                                       
*************************************************************************/

/*************************************************************************
*                                                                       
*   FILE NAME                                          VERSION                    
*                                                                               
*       NET_EXTR.H                                       4.4                     
*                                                                               
*   COMPONENT                                                             
*                                                                       
*       Net
*                                                                       
*   DESCRIPTION                                                           
*                                                                       
*       Holds the defines for the compare functions INT32_CMP and        
*       INT16_CMP.                                                       
*                                                                       
*   DATA STRUCTURES                                                       
*                                                                       
*       None.                                                            
*                                                                       
*   DEPENDENCIES                                                          
*                                                                       
*       None
*                                                                       
*************************************************************************/

#ifndef NET_EXTR_H
#define NET_EXTR_H

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++ */
#endif /* _cplusplus */

/*************************************************************************
*  INT32_CMP                                                             
*                                                                        
*  Compare two sequence numbers.  This macro handles will handle sequence
*  space wrap around.  Overflow/Underflow makes the results below        
*  correct.                                                              
*  RESULTS:            result       implies                              
*                        -          a < b                                
*                        0          a = b                                
*                        +          a > b                                
*************************************************************************/
#define INT32_CMP(a, b)      ((INT32)((a)-(b)))
#define INT16_CMP(a, b)      ((INT16)((a)-(b)))

#ifdef          __cplusplus
}
#endif /* _cplusplus */

#endif
