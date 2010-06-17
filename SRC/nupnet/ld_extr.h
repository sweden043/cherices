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
*       ld_extr.h                                         4.4   
*        
*   COMPONENT
*
*       LD - Loopback Device
*
*   DESCRIPTION
*
*       This file contains function prototypes for the loopback module.
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

#ifndef LD_EXTR_H
#define LD_EXTR_H

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif /* _cplusplus */

STATUS LDC_Init (DV_DEVICE_ENTRY *);
STATUS LDC_TX_Packet (DV_DEVICE_ENTRY *, NET_BUFFER *);
STATUS LDC_Ioctl(DV_DEVICE_ENTRY *, INT, DV_REQ *);

#ifdef          __cplusplus
}
#endif /* _cplusplus */

#endif /* LD_EXTR_H */
