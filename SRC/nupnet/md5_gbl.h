/***************************************************************************
*
*        Copyright (c) 1993 - 2001 Accelerated Technology, Inc.
*
* PROPRIETARY RIGHTS of Accelerated Technology are involved in the
* subject matter of this material.  All manufacturing, reproduction,
* use, and sales rights pertaining to this subject matter are governed
* by the license agreement.  The recipient of this software implicitly
* accepts the terms of the license.
*
***************************************************************************/

/***************************************************************************
*
* FILE NAME                                             VERSION 
*        
*      MD5_GBL.H                                          4.4   
*        
* COMPONENT
*
*      Prototyping
*
* DESCRIPTION
*
*      This file has been renamed to MD5_GBL.H to provide better compatibilty
*      with other ATI source files.
*
* DATA STRUCTURES
*
*      None
*
* DEPENDENCIES
*
*      None
*
***************************************************************************/

/* GLOBAL.H - RSAREF types and constants
*/

/* PROTOTYPES should be set to one if and only if the compiler supports
   function argument prototyping.
   The following makes PROTOTYPES default to 0 if it has not already
   been defined with C compiler flags.
*/
#ifndef MD5_GBL_H
#define MD5_GBL_H

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif /* _cplusplus */

#ifndef PROTOTYPES
#define PROTOTYPES 1
#endif

/* POINTER defines a generic pointer type */
typedef unsigned char *POINTER;

/* UINT2 defines a two byte word */
typedef unsigned short int UINT2;

/* UINT4 defines a four byte word */
typedef unsigned long int UINT4;

/* PROTO_LIST is defined depending on how PROTOTYPES is defined above.
   If using PROTOTYPES, then PROTO_LIST returns the list, otherwise it
   returns an empty list.
*/

#if PROTOTYPES
#define PROTO_LIST(list) list
#else
#define PROTO_LIST(list) ()
#endif

#ifdef          __cplusplus
}
#endif /* _cplusplus */

#endif /* MD5_GBL_H */
