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
*   FILENAME                                        VERSION                          
*                                                                                  
*       netevent.h                                    4.4                           
*                                                                                  
*   DESCRIPTION                                                              
*                                                                          
*       This include file will define the events used for the NCSA Telnet.  
*                                                                          
*   DATA STRUCTURES                                                          
*                                                                          
*       tqhdr
*       tqe
*                                                                          
*   DEPENDENCIES                                                             
*                                                                          
*       None
*                                                                          
***************************************************************************/

#ifndef NETEVENT_H
#define NETEVENT_H

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif /* _cplusplus */


/* Events processed by the Events Dispatcher. */
#define CONOPEN             1   /* connection has opened, CONCLASS */
#define CONCLOSE            2   /* the other side has closed its side of the connection */
#define CONFAIL             3   /* connection open attempt has failed */
#define UNUSED_EVENT_SPOT   4   /* This used to be CONRX, but this has been removed.  */
#define CONNULL             5   /*      Just a null event...     */
                                /*      Used to wake up the dispatcher from a
                                 *      indefinite sleep should a timer queue
                                 *      entry be posted...could be used for other
                                 *      purposes also.
                                 */

#define UDPDATA             6   /* UDP data has arrived on listening port, USERCLASS */
#define TCPRETRANS          7   /* TCP segment retransmission event */
#define WINPROBE            8   /* Window Probe event. */
#define TCPACK              9   /* TCP ACK transmission event */
#define CONTX               10  /* buffer needs to be sent. */
#define SELECT              11  /* TCP Select timeout event. */
#define ARPRESOLVE          12  /* ARP event. */
#define RARP_REQUEST        13  /* A RARP request event. */

/* PPP Events */
#define LCP_RESEND          14  /* Resend the last sent LCP packet, PPP_CLASS */
#define LCP_SEND_CONFIG     15  /* Send a LCP config req packet, PPP_CLASS */
#define MDM_HANGUP          16  /* Hangup the mdm up, PPP_CLASS */
#define LCP_ECHO_REQ        17  /* Send an echo request packet, PPP_CLASS */
#define NCP_RESEND          18  /* Resend the last send NCP packet, PPP_CLASS */
#define NCP_SEND_CONFIG     19  /* Send a NCP config req packet, PPP_CLASS */
#define LCP_CLOSE_LINK      20  /* Close the link and all sockets, PPP_CLASS */
#define PAP_SEND_AUTH       21  /* Send a PAP authentication packet, PPP_CLASS */
#define CHAP_RESEND         22  /* Resend the last CHAP response, PPP_CLASS */
#define CHAP_CHALL          23  /* Send a challenge to the host, PPP_CLASS */

#define EV_IGMP_REPORT      24  /* Send an IGMP Group Report. */
#define EV_IP_REASSEMBLY    25  /* Timeout the reassembly of an IP datagram. */

/* ICMP events */
#define ICMP_ECHO_TIMEOUT   26  /* Timeout waiting for a ping reply. */

#define IPDATA              27  /* IP data has arrived */
#define TCPCLOSETIMEOUTSFW2 28  /* Timeout the closing of a connection. */
#define TCPTIMEWAIT         29

/* DHCP events. */
#define DHCP_RENEW          30
#define DHCP_REBIND         31
#define DHCP_NEW_LEASE      32

/* NAT events */
#define NAT_CLEANUP_TCP     33
#define NAT_CLEANUP_UDP     34
#define NAT_TIMEOUT         35
#define NAT_CLOSE           36
#define NAT_CLEANUP_ICMP    37

/* ATI - UP: Negotiation timeout enchancement */

#define PPP_STOP_NEGOTIATION 38

/* end */    


#define NU_CLEAR       0
#define NU_SET         1

struct tqhdr
{
    struct tqe      *flink, *blink;
};

/* Define a timer queue element for TCP and IP timer events */
struct tqe
{
    struct tqe      *flink,
                    *blink;
    struct tqhdr    duplist;
    UNSIGNED        tqe_event,
                    tqe_data;
    UNSIGNED        duetime;
    INT32           tqe_ext_data;
};
typedef struct      tqe tqe_t;

/* External References */
extern struct tqhdr EQ_Event_Freelist;
extern struct tqhdr EQ_Event_List;

/* timer.c -- new double link list for the timer stuff, and will be used
   for the defragmentation later */
INT16   tqpost (tqe_t *tqlist, tqe_t *tqe);

STATUS  EQ_Clear_Matching_Timer (struct tqhdr *, UNSIGNED, UNSIGNED, INT32);

#ifdef          __cplusplus
}
#endif /* _cplusplus */

#endif  /* NETEVENT_H */
