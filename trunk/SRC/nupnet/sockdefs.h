/***************************************************************************
*                                                                          
*      Copyright (c) 1993 - 2001 by Accelerated Technology, Inc.           
*                                                                          
* PROPRIETARY RIGHTS of Accelerated Technology are involved in the subject 
* matter of this material.  All manufacturing, reproduction, use and sales 
* rights pertaining to this subject matter are governed by the license     
* agreement.  The recipient of this software implicity accepts the terms   
* of the license.                                                          
*                                                                          
***************************************************************************/

/***************************************************************************
*                                                                          
*   FILENAME                                       VERSION                           
*                                                                                  
*       sockdefs.h                                   4.4                            
*                                                                                  
*   COMPONENTS
*
*       Sockets
*
*   DESCRIPTION                                                              
*                                                                          
*       This include file will define socket type error return codes, socket
*       options, and socket protocol types.                                 
*                                                                          
*   DATA STRUCTURES                                                          
*                                                                          
*       None
*                                                                          
*   DEPENDENCIES                                                             
*                                                                          
*       None
*                                                                          
***************************************************************************/

#ifndef SOCKDEFS_H
#define SOCKDEFS_H

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif /* _cplusplus */

/* A generic catch-all for unused parameters. */
#define NU_NONE         0

/* Address family equates */
#define SK_FAM_UNSPEC   0               /* unspecified */
#define SK_FAM_LOCAL    1
#define SK_FAM_UNIX     SK_FAM_LOCAL
#define SK_FAM_IP       2               /* Internet:  UDP, TCP, etc. */
#define SK_FAM_ROUTE    17              /* Internal routing protocol */
#define SK_FAM_LINK     18              /* Link layer interface.     */

/* These equates are for backwards compatability */
#define NU_FAMILY_UNIX SK_FAM_UNIX        /* Unix */
#define NU_FAMILY_IP   SK_FAM_IP          /* Internet       - valid entry */

/* TYPE equates */
#define NU_TYPE_STREAM    0     /* stream Socket             - valid entry */
#define NU_TYPE_DGRAM     1     /* datagram Socket           - valid entry */
#define NU_TYPE_RAW       2     /* raw Socket                - valid entry */
#define NU_TYPE_SEQPACKET 3     /* sequenced packet Socket */
#define NU_TYPE_RDM       4     /* reliably delivered msg Socket */

/* PROTOCOL equates */
#define NU_PROTO_INVALID  0
#define NU_PROTO_TCP      1
#define NU_PROTO_UDP      2
#define NU_PROTO_ICMP     3

/***************************  SOCKET OPTIONS  *****************************/
/* SOCKET OPTION control flags */
#define NU_SETFLAG        1
#define NU_BLOCK          1

/* PROTOCOL LEVELS */
#define NU_SOCKET_LEVEL   0
   
/* Levels used in the call to NU_Setsockopt */
#define IPPROTO_IP      0
#define IPPROTO_ICMP    1
#define IPPROTO_IGMP    2
#define IPPROTO_GGP     3
#define IPPROTO_TCP     6
#define IPPROTO_EGP     8
#define IPPROTO_PUP     12
#define IPPROTO_UDP     17
#define SOL_SOCKET      100

/* Protocol used int call to NU_Socket with a Raw IP socket */
#define IPPROTO_HELLO   63
#define IPPROTO_RAW     255
#define IPPROTO_OSPF    89


/*
 * Options for use with [gs]etsockopt at the socket level.
 * First word of comment is data type; bool is stored in int.
 */
#define SO_BROADCAST        1  /* permission to transmit broadcast messages? */

/*
 * Options for use with [gs]etsockopt at the IP level.
 * First word of comment is data type; bool is stored in int.
 */
#define IP_OPTIONS          1    /* buf/ip_opts; set/get IP options */
#define IP_HDRINCL          2    /* int; header is included with data */
#define IP_TOS              3    /* int; IP type of service and preced. */
#define IP_TTL              4    /* int; IP time to live */
#define IP_RECVOPTS         5    /* bool; receive all IP opts w/dgram */
#define IP_RECVRETOPTS      6    /* bool; receive IP opts for response */
#define IP_RECVDSTADDR      7    /* bool; receive IP dst addr w/dgram */
#define IP_RETOPTS          8    /* ip_opts; set/get IP options */
#define IP_MULTICAST_IF     9    /* u_char; set/get IP multicast i/f  */
#define IP_MULTICAST_TTL    10   /* u_char; set/get IP multicast ttl */
#define IP_MULTICAST_LOOP   11   /* u_char; set/get IP multicast loopback */
#define IP_ADD_MEMBERSHIP   12   /* ip_mreq; add an IP group membership */
#define IP_DROP_MEMBERSHIP  13   /* ip_mreq; drop an IP group membership */

/*******************  SOCKET ERROR CODES  ****************************

 The range for Nucleus NET error codes is -251 to -500.

*/

#define NU_INVALID_PROTOCOL     -251    /*  Invalid network protocol */
#define NU_NO_DATA_TRANSFER     -252    /*  Data was not written/read
                                            during send/receive function */
#define NU_NO_PORT_NUMBER       -253    /*  No local port number was stored
                                            in the socket descriptor */
#define NU_NO_TASK_MATCH        -254    /*  No task/port number combination
                                            existed in the task table */
#define NU_NO_SOCKET_SPACE      -255    /*  The socket structure list was full
                                            when a new socket descriptor was
                                            requested */
#define NU_NO_ACTION            -256    /*  No action was processed by
                                            the function */
#define NU_NOT_CONNECTED        -257    /*  A connection has been closed
                                            by the network.  */
#define NU_INVALID_SOCKET       -258    /*  The socket ID passed in was
                                            not in a valid range.  */
#define NU_NO_SOCK_MEMORY       -259    /*  Memory allocation failed for
                                            internal sockets structure.  */
#define NU_INVALID_ADDRESS      -260    /*  An incomplete address was sent */
#define NU_NO_HOST_NAME         -261    /*  No host name specified in a  */
#define NU_RARP_INIT_FAILED     -262    /*  During initialization RARP failed. */
#define NU_BOOTP_INIT_FAILED    -263    /*  During initialization BOOTP failed. */
#define NU_INVALID_PORT         -264    /*  The port number passed in was
                                            not in a valid range. */
#define NU_NO_BUFFERS           -265    /*  There were no buffers to place */
                                        /*  the outgoing packet in. */
#define NU_NOT_ESTAB            -266    /*  A connection is open but not in
                                            an established state. */
#define NU_WINDOW_FULL          -267    /*  The foreign host's in window is
                                            full. */
#define NU_NO_SOCKETS           -268    /*  No sockets were specified. */
#define NU_NO_DATA              -269    /*  None of the specified sockets were
                                            data ready.  NU_Select. */



/* The following errors are reported by the NU_Setsockopt and NU_Getsockopt
   service calls. */
#define NU_INVALID_LEVEL        -270    /*  The specified level is invalid. */
#define NU_INVALID_OPTION       -271    /*  The specified option is invalid. */
#define NU_INVAL                -272    /*  General purpose error condition. */
#define NU_ACCESS               -273    /*  The attempted operation is not   */
                                        /*  allowed on the  socket           */
#define NU_ADDRINUSE            -274

#define NU_HOST_UNREACHABLE     -275    /*  Host unreachable */
#define NU_MSGSIZE              -276    /*  Packet is to large for interface. */
#define NU_NOBUFS               -277    /*  Could not allocate a memory buffer. */
#define NU_UNRESOLVED_ADDR      -278    /*  The MAC address was not resolved.*/
#define NU_CLOSING              -279    /*  The other side in a TCP connection*/
                                        /*  has sent a FIN */
#define NU_MEM_ALLOC            -280    /* Failed to allocate memory. */
#define NU_RESET                -281  
#define NU_DEVICE_DOWN          -282    /* A device being used by the socket has
                                           gone down. Most likely because a PPP 
                                           link has been disconnected or a DHCP
                                           IP address lease has expired. */
/* These error codes are returned by DNS. */
#define NU_INVALID_LABEL        -283    /* Indicates a domain name with an
                                           invalid label.                   */
#define NU_FAILED_QUERY         -284    /* No response received for a DNS
                                           Query. */
#define NU_DNS_ERROR            -285    /* A general DNS error status. */
#define NU_NOT_A_HOST           -286    /* The host name was not found. */
#define NU_INVALID_PARM         -287    /*  A parameter has an invalid value. */

#define NU_NO_DNS_SERVER        -288    /* No DNS server has been registered with
                                            the stack. */

/* Error codes for DHCP */
#define NU_DHCP_INIT_FAILED     -289    /*  During initialization DHCP failed. */
#define NU_DHCP_REQUEST_FAILED  -290

/*  Error codes for BOOTP */
#define NU_BOOTP_SEND_FAILED          -291
#define NU_BOOTP_RECV_FAILED          -292
#define NU_BOOTP_ATTACH_IP_FAILED     -293
#define NU_BOOTP_SELECT_FAILED        -294
#define NU_BOOTP_FAILED               -295

#define NU_NO_ROUTE_TO_HOST     -296    /* ICMP Destination Unreachable specific error */
#define NU_CONNECTION_REFUSED   -297    /* ICMP Destination Unreachable specific error */
#define NU_MSG_TOO_LONG         -298    /* ICMP Destination Unreachable specific error */

#define NU_BAD_SOCKETD          -299
#define NU_BAD_LEVEL            -300
#define NU_BAD_OPTION           -301


#ifdef          __cplusplus
}
#endif /* _cplusplus */

#endif  /* SOCKDEFS_H */
