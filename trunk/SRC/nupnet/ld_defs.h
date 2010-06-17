/****************************************************************************
*
*        Copyright (c) 1993 - 2001 Accelerated Technology, Inc.
*
* PROPRIETARY RIGHTS of Accelerated Technology are involved in the
* subject matter of this material.  All manufacturing, reproduction,
* use, and sales rights pertaining to this subject matter are governed
* by the license agreement.  The recipient of this software implicitly
* accepts the terms of the license.
*
****************************************************************************/

/****************************************************************************
*
*   FILE NAME                                           VERSION 
*        
*       ld_defs.h                                         4.4   
*        
*   COMPONENT
*
*       LD - Loopback Device
*
*   DESCRIPTION
*
*       This file contains macro definitions to support the loopback
*       module LDC.C
*
*   DATA STRUCTURES
*
*       None
*
*   DEPENDENCIES
*
*       None
*
****************************************************************************/

#ifndef LD_DEFS_H
#define LD_DEFS_H

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif /* _cplusplus */

#define LD_ADDRESS_LENGTH   6
#define LD_HEADER_LENGTH    14
#define LD_MTU              1500
#define LD_INVALID_VECTOR   65000UL

#ifdef          __cplusplus
}
#endif /* _cplusplus */

#endif
