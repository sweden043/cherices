/*************************************************************************
*                                                                       
*      Copyright (c) 1993 - 2001 by Accelerated Technology, Inc.        
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
*   FILE NAME                                     VERSION                         
*                                                                                 
*       IPRAW.H                                     4.4   
*                                                                                 
*   DESCRIPTION                                                           
*                                                                         
*       This file contains the structure definitions required by the 
*       IPRAW module of Nucleus NET.
*                                                                         
*   DATA STRUCTURES       
*
*       IPR_PORT
*                                                                         
*   DEPENDENCIES                                                          
*                                                                         
*       None                                                              
*                                                                         
*************************************************************************/

#ifndef IPRAW_H
#define IPRAW_H

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif /* _cplusplus */

#if INCLUDE_IP_RAW

/* Maximum IP raw message size */
#define IMAXLEN (MTU  - (NET_MAX_MAC_HEADER_SIZE + sizeof(IPLAYER)))

/* PCB for IP Raw Datagrams */
struct iport
{
    RTAB_ROUTE              ip_route;       /* A cached route. */
    UINT32                  iportFlags;     /* What type of events are
                                               currently pending. */
    UINT32                  ip_laddr;       /* Local IP address */
    UINT32                  ip_faddr;       /* Foreign IP address */
    UINT16                  ip_lport;       /* Local port number */
    UINT16                  listen;         /* what port should this one
                                               listen to? */
    UINT16                  length;         /* how much data arrived in
                                               last packet? */
    UINT16                  ip_protocol;    /* Protocol number */
    INT                     ip_socketd;     /* the socket associated with 
                                               this port. */
};

typedef struct iport IPR_PORT;

extern struct iport *IPR_Ports[IPR_MAX_PORTS];

/* IP Raw prototypes. */
STATUS  IPRaw_Interpret (NET_BUFFER *buf_ptr, DV_DEVICE_ENTRY *device);
INT16   IPRaw_Read (struct iport *, CHAR *, struct addr_struct *, 
                  struct sock_struct *);
INT16   IPRaw_Send (struct iport *, UINT8 *, UINT16, UINT16);
INT16   IPRaw_Make_Port(INT);
INT16   IPRaw_Get_PCB(INT socketd, struct sock_struct *);

#endif /* INCLUDE_IP_RAW */

#ifdef          __cplusplus
}
#endif /* _cplusplus */


#endif /* IPRAW_H */
