/***************************************************************************
*                                                                       
*      Copyright (c) 1993 - 2001 by Accelerated Technology, Inc.        
*                                                                       
* PROPRIETARY RIGHTS of Accelerated Technology are involved in the      
* subject matter of this material.  All manufacturing, reproduction,    
* use, and sales rights pertaining to this subject matter are governed  
* by the license agreement.  The recipient of this software implicitly  
* accepts the terms of the license.                                     
*                                                                       
***************************************************************************/

/*Portions of this program were written by:       */
/***************************************************************************
*                                                                          
*      part of:                                                            
*      TCP/IP kernel for NCSA Telnet                                       
*      by Tim Krauskopf                                                    
*                                                                          
*      National Center for Supercomputing Applications                     
*      152 Computing Applications Building                                 
*      605 E. Springfield Ave.                                             
*      Champaign, IL  61820                                                
*                                                                          
*                                                                          
***************************************************************************/

/***************************************************************************
*                                                                           
*   FILE NAME                                           VERSION                                                           
*                                                                           
*       UDP.H                                               4.4
*                                                                           
*   DESCRIPTION                                                             
*                                                                           
*       This file contains the structure definitions required by the UDP 
*       module of Nucleus NET.
*                                                                           
*   DATA STRUCTURES                                                         
*                                                                           
*       udph
*       uport
*                                                                           
*   DEPENDENCIES                                                            
*                                                                           
*       ip.h                                                                
*                                                                           
***************************************************************************/
#ifndef UDP_H
#define UDP_H

#include "ip.h"

#if INCLUDE_UDP

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif /* _cplusplus */

/* Maximum UDP message size */
#define UMAXLEN \
  (MTU  - (NET_MAX_MAC_HEADER_SIZE + sizeof(IPLAYER) + sizeof(UDPLAYER)))

/*************************************************************************/
/*  UDP
*   User Datagram Protocol
*   Each packet is an independent datagram, no sequencing information
*
*   UDP uses the identical checksum to TCP
*/

typedef struct udph
{
    UINT16 udp_src;         /* UDP source port number. */
    UINT16 udp_dest;        /* UDP destination port number. */
    UINT16 udp_length;      /* Length of packet, including hdr */
    UINT16 udp_check;       /* UDP checksum - optional */
} UDPLAYER;


#define UDP_SRC_OFFSET              0
#define UDP_DEST_OFFSET             2
#define UDP_LENGTH_OFFSET           4
#define UDP_CHECK_OFFSET            6

struct uport
{
    RTAB_ROUTE              up_route;       /* A cached route. */
    UINT32                  uportFlags;     /* What type of events are
                                               currently pending. */
    UINT16                  listen;         /* what port should this one
                                               listen to? */
    UINT16                  length;         /* how much data arrived in
                                               last packet? */
    UINT32                  up_laddr;       /* Local IP address */
    UINT32                  up_faddr;       /* Foreign IP address */
    UINT16                  up_lport;       /* Local port number */
    UINT16                  up_fport;       /* Foreign port number */
    INT                     up_socketd;     /* the socket associated with 
                                               this port. */
};
typedef struct uport UDP_PORT;

extern struct uport *UDP_Ports[UDP_MAX_PORTS];  /* allocate like iobuffers in UNIX */

/* Prototypes */
STATUS UDP_Port_Cleanup(INT uport_index);
INT16  UDP_Interpret (NET_BUFFER *, struct pseudotcp *);
INT32  UDP_Read (struct sock_struct *, CHAR *, struct addr_struct *);
INT32  UDP_Send (UDP_PORT *, CHAR *, INT32);
INT16  UDP_Make_Port (UINT16, INT);

#ifdef          __cplusplus
}
#endif /* _cplusplus */

#endif /* UDP_H */

#endif /* INCLUDE_UDP */
