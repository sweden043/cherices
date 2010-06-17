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
*       TCP.H                                               4.4          
*                                                                       
*   COMPONENT                                                             
*                                                                       
*       TCP - Transmission Control Protocol                              
*                                                                       
*   DESCRIPTION                                                           
*                                                                       
*       Holds the defines for the TCP protocol.                          
*                                                                       
*   DATA STRUCTURES                                                       
*                                                                       
*       TCPLAYER
*       _TCP_Window
*       _TCP_Port
*                                                                       
*   DEPENDENCIES                                                          
*                                                                       
*       ip.h
*                                                                       
*************************************************************************/

#ifndef TCP_H
#define TCP_H

#include "ip.h"

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif /* _cplusplus */

#define DATA           1
#define ACK            2

#define TCP_RTTDFLT         (18 * SCK_Ticks_Per_Second)
#define TCP_RTT_SHIFT       2
#define TCP_RTTVAR_SHIFT    2

/* Used for the size of the TCP_Backoff array. This array holds 
   values that will be used for TCP and other protocol retransmit
   times. Therefore this array MUST be larger in size than the
   max number of retranmits. */
#define TCP_MAX_BACKOFFS    8

/* The above define must be larger than MAX_RETRANSMITS. */
#if ( (TCP_MAX_BACKOFFS) < (MAX_RETRANSMITS) )
#error TCP_MAX_BACKOFFS must be larger in size than MAX_RETRANMITS!
#endif


#if (INCLUDE_TCP == NU_TRUE)

/*****************************************************************************/
/* IN_CLASSA,  IN_CLASSB, IN_CLASSC                                          */
/*                                                                           */
/* These macros are used to determine the class of an IP address.  Note that */
/* they will only work if i is unsigned.  The class of the address is        */
/* determined by the first two bits of the IP address.                       */
/*                                                                           */
/*****************************************************************************/
#define IN_CLASSA(i)        (((long)(i) & 0x80000000) == 0)
#define IN_CLASSA_NET       0xff000000
#define IN_CLASSA_NSHIFT    24
#define IN_CLASSA_HOST      0x00ffffff
#define IN_CLASSA_MAX       128

#define IN_CLASSB(i)        (((long)(i) & 0xc0000000) == 0x80000000)
#define IN_CLASSB_NET       0xffff0000
#define IN_CLASSB_NSHIFT    16
#define IN_CLASSB_HOST      0x0000ffff
#define IN_CLASSB_MAX       65536

#define IN_CLASSC(i)        (((long)(i) & 0xe0000000) == 0xc0000000)
#define IN_CLASSC_NET       0xffffff00
#define IN_CLASSC_NSHIFT    8
#define IN_CLASSC_HOST      0x000000ff

#define IN_CLASSD(i)        (((long)(i) & 0xf0000000) == 0xe0000000)
#define IN_CLASSD_NET       0xf0000000  /* These ones aren't really */
#define IN_CLASSD_NSHIFT    28          /* net and host fields, but */
#define IN_CLASSD_HOST      0x0fffffff  /* routing needn't know.    */
#define IN_MULTICAST(i)     IN_CLASSD(i)

#define IN_EXPERIMENTAL(i)  (((long)(i) & 0xf0000000) == 0xf0000000)
#define IN_BADCLASS(i)      (((long)(i) & 0xf0000000) == 0xf0000000)


/**************************************************************************/
/*  TCP protocol
*     define both headers required and create a data type for a typical
*     outgoing TCP packet (with IP header)
*   
*  Note:  So far, there is no way to handle IP options fields
*   which are associated with a TCP packet.  They are mutually exclusive
*   for both receiving and sending.  Support may be added later.
*
*   The tcph and iph structures can be included in many different types of
*   arbitrary data structures and will be the basis for generic send and
*   receive subroutines later.  For now, the packet structures are optimized 
*   for packets with no options fields.  (seems to be almost all of them from
*   what I've observed.
*/

typedef struct tcph
{
    UINT16 tcp_src;             /* TCP Source port number */
    UINT16 tcp_dest;            /* TCP destination port number */
    UINT32 tcp_seq;             /* TCP sequence number */ 
    UINT32 tcp_ack;             /* TCP Acknowledgment number */
    UINT8  tcp_hlen;            /* length of TCP header in 4 byte words */
    UINT8  tcp_flags;           /* flag fields */
    UINT16 tcp_window;          /* advertised window, byte-swapped */
    UINT16 tcp_check;           /* TCP checksum of whole packet */
    UINT16 tcp_urgent;          /* urgent pointer, when flag is set */
} TCPLAYER;


struct _TCP_Window
{
    UINT32  nxt;                  /* sequence number, not byte-swapped */
    UINT32  ack;                  /* what the other machine acked */
    INT32   lasttime;             /* (signed) used for timeout checking */
    NET_BUFFER_HEADER packet_list;
    NET_BUFFER        *nextPacket;
    NET_BUFFER_HEADER ooo_list;   /* Contains out of order packets. */
    INT32   size;                 /* size of window advertised */
    INT32   contain;              /* how many bytes in queue? */
    UINT16  port;                 /* port numbers from one host or another */
    UINT16  num_packets;
    UINT8   push;                 /* flag for TCP push */
    UINT8   tcp_flags;
    UINT8   pad[2];               /* correct alignment for 32 bits CPU */
};

typedef struct _TCP_Window TCP_WINDOW;

struct _TCP_Port
{
    TCP_WINDOW      in, out;
    UINT32          tcp_laddr;      /* Local IP address. */
    UINT32          tcp_faddr;      /* Foreign IP address */
    UINT32          maxSendWin;     /* Max send window. */
    RTAB_ROUTE      tp_route;       /* A cached route. */
    UINT16  credit;             /* choked-down window for fast hosts */
    UINT16  sendsize;           /* MTU value for this connection */
    UINT32  p_rtseq;            /* Sequence # of packet being timed. */
    INT32   p_rto;              /* retrans timeout */
    INT32   p_srtt;
    INT32   p_rttvar;
    UINT32  p_rtt;
    INT     p_socketd;          /* The socket associated with this port. */
    UINT16  pindex;             /* added by Randy */
    UINT16  portFlags;
    INT16   icmp_error;         /* holds the ICMP error that was received */
    UINT8   pad[2];
    UINT8   state;              /* connection state */
    INT8    xmitFlag;           /* Used to indicate an timer event has been
                                   created to transmit a partial packet. */
    INT8    probeFlag;          /* This flag will be set when a window probe
                                   is required. */
    INT8    selectFlag;         /* This flag is set whenever a call to
                                    NU_Select results in a task timing out. */
};

typedef struct _TCP_Port TCP_PORT;


/* 
*  flag field definitions, first two bits undefined
*/

#define TURG    0x20
#define TACK    0x10
#define TPUSH   0x08
#define TRESET  0x04
#define TSYN    0x02
#define TFIN    0x01


/***************************************************************************/
/*  Port Flags                                                             */
/*                                                                         */
#define ACK_TIMER_SET     0x0001
#define SELECT_SET        0x0002

/*************************************************************************/
/*  TCP states
*    each connection has an associated state in the connection flow.
*    the order of the states now matters, those less than a certain
*    number are the "inactive" states.
*/
#define SCLOSED         1
#define SLISTEN         2
#define SSYNR           3
#define SSYNS           4
#define SEST            5
#define SFW1            6
#define SFW2            7
#define SCLOSING        8
#define STWAIT          9
#define SCWAIT          10
#define SLAST           11

#define TCP_SRC_OFFSET              0
#define TCP_DEST_OFFSET             2
#define TCP_SEQ_OFFSET              4
#define TCP_ACK_OFFSET              8
#define TCP_HLEN_OFFSET             12
#define TCP_FLAGS_OFFSET            13
#define TCP_WINDOW_OFFSET           14
#define TCP_CHECK_OFFSET            16
#define TCP_URGENT_OFFSET           18

/* Define return codes for internal use by TCP. */
#define TCP_NO_ACK                  -255
#define TCP_INVALID_ACK             -254

/*
 * Options for use with [gs]etsockopt at the TCP level.
 */
#define TCP_NODELAY                 1

/*
 * The Maximum amount of data that can be sent in a TCP segment is some what
 * less than the MTU.  This is because a header is added to the packet
 * at each layer (TCP layer, IP layer, and Hardware layer).
 * So the max amount of data that can be sent in a segment
 * is MTU - (sizeof all headers);
 */
/* Maximum TCP message size */
#define MAX_SEGMENT_LEN \
  (MTU  - (NET_MAX_MAC_HEADER_SIZE + sizeof(IPLAYER) + sizeof(TCPLAYER)))


/* TCP function prototypes. */
INT16   TCP_Interpret (NET_BUFFER *buf_ptr, struct pseudotcp *tcp_chk);
STATUS  TCP_Send (TCP_PORT *pport, NET_BUFFER*);
INT16   TCP_Send_ACK(TCP_PORT *pport);
INT16   TCP_Update_Headers (TCP_PORT *, NET_BUFFER *, UINT16);
INT16   TCP_Retransmit(TCP_PORT *, INT32);
VOID    TCP_ACK_It(TCP_PORT *prt, INT force);
STATUS  TCP_Xmit(TCP_PORT *, NET_BUFFER *);
STATUS  TCP_Cleanup(TCP_PORT *prt);
INT     TCP_Make_Port (VOID);
STATUS  TCP_Set_Opt(INT socketd, INT optname, VOID *optval, INT optlen);
STATUS  TCP_Get_Opt(INT socketd, INT optname, VOID *optval, INT *optlen);

/***** TCPSS.C *****/

INT32   TCPSS_Net_Read(struct sock_struct *, CHAR *, UINT16);
INT32   TCPSS_Net_Write(struct sock_struct *sockptr, UINT8 HUGE *buffer, UINT16 nbytes,
                 INT *status);
STATUS  TCPSS_Net_Listen(UINT16 serv, struct pseudotcp *tcp_check);
STATUS  TCPSS_Net_Xopen (UINT8 *machine, UINT16 service, INT socketd);
STATUS  TCPSS_Send_SYN_FIN (TCP_PORT *, INT16);
STATUS  TCPSS_Net_Close (INT, struct sock_struct *);

/***/

/***** TOOLS.C *****/
UINT16  TLS_Enqueue (struct sock_struct *);
INT32   TLS_Dequeue (struct sock_struct *, CHAR *, UINT32);
UINT16  TLS_Rmqueue (TCP_WINDOW *wind, INT32);


/* External references */
extern struct _TCP_Port *TCP_Ports[TCP_MAX_PORTS];
extern INT16 tasks_waiting_to_send;

#endif /* INCLUDE_TCP == NU_TRUE */

#ifdef          __cplusplus
}
#endif /* _cplusplus */


#endif /* TCP_H */

