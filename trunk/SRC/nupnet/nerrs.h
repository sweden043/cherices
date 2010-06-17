/***************************************************************************
*                                                                          
*      Copyright (c) 1993 - 2001 by Accelerated Technology, Inc.           
*                                                                          
* PROPRIETARY RIGHTS of Accelerated Technology are involved in             
* the subject matter of this material.  All manufacturing, reproduction,   
* use, and sales rights pertaining to this subject matter are governed     
* by the license agreement.  The recipient of this software inplicitly     
* accepts the terms of the license.                                        
*                                                                          
***************************************************************************/

/***************************************************************************
*
*   FILE NAME                                         VERSION                        
*                                                                                  
*       NERRS.H                                         4.4                 
*                                                                                  
*   COMPONENT                                                                
*                                                                          
*       Prototyping
*                                                                          
*   DESCRIPTION                                                              
*                                                                          
*       This file will hold all the global error codes and funtion 
*       prototyping needed for all errors.  The error codes and functions 
*       here are used by the TCP/IP code only.
*                                                                          
*   DATA STRUCTURES                                                          
*                                                                          
*       None
*                                                                          
*   DEPENDENCIES                                                             
*                                                                          
*       target.h
*
***************************************************************************/
#ifndef NERRS_H
#define NERRS_H

#include "target.h"

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif /* _cplusplus */

/*
 * This area will be used for the prototyping of all the TCP/IP error
 * routines and functions.
 */
VOID NERRS_Log_Error (INT stat, INT8 *file, INT line);
VOID NERRS_Clear_All_Errors (VOID);

/*
 * This variable will be the index to the NU_Tcp_Err_List array, and will
 * tell the next available location that may be used for storing errors.
 * This index will handle the wrap around of the array based on the current
 * define value in MAX_TCP_ERRORS.
 */
extern INT NERRS_Avail_Index;


/* These defines are used by the error stuff for severity */
#define NERR_INFORMATIONAL 0      /* error may cause some trivial problems */
#define NERR_RECOVERABLE   1      /* error is recoverable */
#define NERR_SEVERE        2      /* error will cause severe problems */
#define NERR_FATAL         3      /* error is fatal and not recoverable */

#ifdef          __cplusplus
}
#endif /* _cplusplus */

#endif  /* NERRS_H */
