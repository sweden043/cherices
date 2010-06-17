/************************************************************************
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

/************************************************************************
*                                                                       
*   FILE NAME                                           VERSION
*                                                                    
*       TCPDEFS.H                                           4.4             
*                                                                       
*   COMPONENT                                                             
*                                                                       
*       TCP - Transmission Control Protocol                              
*                                                                       
*   DESCRIPTION                                                           
*                                                                       
*       Holds the defines related to the TCP protocol.                   
*                                                                       
*   DATA STRUCTURES                                                       
*                                                                       
*       Global component data stuctures defined in this file             
*                                                                       
*   DEPENDENCIES                                                          
*                                                                       
*       nucleus.h                                                        
*                                                                       
*************************************************************************/

              /* the definitions for Nucleus - TCP/IP program */

#ifndef TCPDEFS_H
#define TCPDEFS_H

#include "nucleus.h"

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif /* _cplusplus */


/************ RESOURCES ************/
extern NU_SEMAPHORE             TCP_Resource;
extern NU_PARTITION_POOL        Mace_Pool;


/************* EVENTS **************/
extern NU_QUEUE        eQueue;
extern NU_MEMORY_POOL  System_Memory;
extern NU_EVENT_GROUP  Buffers_Available;

#ifdef          __cplusplus
}
#endif /* _cplusplus */

#endif /* TCPDEFS.H */
