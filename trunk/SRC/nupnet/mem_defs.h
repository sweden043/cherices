/****************************************************************************
*                                                                            
*      Copyright (c) 1993 - 2001 by Accelerated Technology, Inc.             
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
*   FILE NAME                                       VERSION                          
*                                                                                    
*       MEM_DEFS.H                                    4.4                            
*                                                                                    
*   DESCRIPTION                                                              
*                                                                            
*       This file contains the linked list structure definitions used by NET.
*       These lists are used for buffering of incoming and outgoing packets. 
*                                                                            
*   DATA STRUCTURES                                                          
*                                                                            
*       _me_bufhdr                                                           
*       packet_queue_element                                                 
*       packet_queue_header                                                  
*       queue_header                                                         
*       queue_element                                                        
*       _mem_suspension_element                                              
*       _mem_suspension_list                                                 
*                                                                            
*   DEPENDENCIES                                                             
*                                                                            
*       None                                                                 
*                                                                            
****************************************************************************/

#ifndef MEM_DEFS_H
#define MEM_DEFS_H

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif /* _cplusplus */

/* Flags for indicating the destination address of a packet in a buffer. */
#define NET_BCAST       0x01
#define NET_MCAST       0x02
#define NET_IP          0x04
#define NET_LCP         0x08
#define NET_IPCP        0x10
#define NET_PAP         0x20
#define NET_CHAP        0x40

typedef struct packet_queue_header NET_BUFFER_HEADER;

/* Header sizes of other protocols that affect the max. */
#define PPE_HEADER_SIZE 6
#define PPP_HEADER_SIZE 2

#ifndef INCLUDE_PPPOE
#define INCLUDE_PPPOE   NU_FALSE
#endif

#if (defined(INCLUDE_PPPOE) && (INCLUDE_PPPOE == NU_TRUE))
/* This is a definition of the largest Media Access Layer header. It is used in the 
   definitions below when deciding how far to offset into a buffer. */
#define NET_MAX_MAC_HEADER_SIZE (sizeof(DLAYER) + PPE_HEADER_SIZE +  \
  PPP_HEADER_SIZE)
#else
/* This is a definition of the largest Media Access Layer header. It is used in the 
   definitions below when deciding how far to offset into a buffer. */
#define NET_MAX_MAC_HEADER_SIZE (DADDLEN + DADDLEN + sizeof(UINT16))
#endif

/* Define the size of the header for the parent buffer header */
#define NET_MAX_TCP_HEADER_SIZE \
  (NET_MAX_MAC_HEADER_SIZE + sizeof (IPLAYER) + sizeof (TCPLAYER))

#define NET_MAX_UDP_HEADER_SIZE \
  (NET_MAX_MAC_HEADER_SIZE + sizeof (IPLAYER) + sizeof (UDPLAYER))

#define NET_MAX_ARP_HEADER_SIZE \
  (NET_MAX_MAC_HEADER_SIZE + sizeof (ARP_LAYER))

#define NET_MAX_ICMP_HEADER_SIZE \
  (NET_MAX_MAC_HEADER_SIZE + sizeof (IPLAYER) + sizeof (ICMP_LAYER))

#define NET_PPP_HEADER_OFFSET_SIZE NET_MAX_MAC_HEADER_SIZE

#define NET_ETHER_HEADER_OFFSET_SIZE NET_MAX_MAC_HEADER_SIZE


#define NET_SLIP_HEADER_OFFSET_SIZE NET_MAX_MAC_HEADER_SIZE

#define NET_MAX_IP_HEADER_SIZE \
  (NET_MAX_MAC_HEADER_SIZE + sizeof (IPLAYER))

#define NET_MAX_IGMP_HEADER_SIZE \
  (NET_MAX_MAC_HEADER_SIZE + sizeof (IPLAYER) + sizeof (IGMP_LAYER))

#define NET_FREE_BUFFER_THRESHOLD       10
#define NET_PARENT_BUFFER_SIZE          512
#define NET_MAX_BUFFER_SIZE \
   (NET_PARENT_BUFFER_SIZE + sizeof(struct _me_bufhdr))


typedef struct packet_queue_element NET_BUFFER;

struct _me_bufhdr
{
    INT32                       seqnum;
    NET_BUFFER_HEADER           *dlist;
    struct _DV_DEVICE_ENTRY     *buf_device;
    UINT16                      option_len;
    INT16                       retransmits;
    UINT16                      flags;
    UINT16                      tcp_data_len;           /* size of the data in a TCP packet. */
    UINT32                      total_data_len;         /* size of the entire buffer,
                                                           sum of all in the chain    */
    INT32                       port_index;
};

/* Define the queue element used to hold a packet */
struct packet_queue_element
{
    union 
    {
        UINT8 packet[NET_MAX_BUFFER_SIZE];

        struct _me_pkthdr
        {
            UINT8                   parent_packet[NET_PARENT_BUFFER_SIZE];
            struct  _me_bufhdr      me_buf_hdr;
        } me_pkthdr;

    } me_data;

    NET_BUFFER                          *next;        /* next buffer chain in the list */  
    NET_BUFFER                          *next_buffer; /* next buffer in this chain */
    UINT8                               *data_ptr;
    UINT32                              data_len;     /* size of this buffer */
};

/* These definitions make it easier to access fields within a packet. */
#define mem_seqnum              me_data.me_pkthdr.me_buf_hdr.seqnum
#define mem_dlist               me_data.me_pkthdr.me_buf_hdr.dlist
#define mem_buf_device          me_data.me_pkthdr.me_buf_hdr.buf_device
#define mem_option_len          me_data.me_pkthdr.me_buf_hdr.option_len
#define mem_retransmits         me_data.me_pkthdr.me_buf_hdr.retransmits
#define mem_flags               me_data.me_pkthdr.me_buf_hdr.flags
#define mem_tcp_data_len        me_data.me_pkthdr.me_buf_hdr.tcp_data_len
#define mem_total_data_len      me_data.me_pkthdr.me_buf_hdr.total_data_len
#define mem_port_index          me_data.me_pkthdr.me_buf_hdr.port_index
#define mem_parent_packet       me_data.me_pkthdr.parent_packet
#define mem_packet              me_data.packet

/* This definition is for use by an application. It is the minimum
   size of the NET memory pool, that is the memory pool passed
   to NET via the NU_Init_Net service. NOTE: the DM_OVERHEAD
   macro is taken from the Nucleus PLUS source file 'dm_defs.h'
   and specifies the overhead associated with a memory pool. Thus
   it is required for computing the minimum size of NET memory
   pool. */
#ifndef DM_OVERHEAD
#define         DM_OVERHEAD         ((UINT32)((NU_MEMORY_POOL_SIZE + \
                                    sizeof(UNSIGNED)                 \
                                    - 1)/sizeof(UNSIGNED)) *         \
                                    sizeof(UNSIGNED))
#endif

#define NU_NET_BUFFER_POOL_SIZE     ((UINT32)(MAX_BUFFERS * (sizeof(NET_BUFFER) + \
                                    (REQ_ALIGNMENT - 4) + DM_OVERHEAD) +          \
                                    (2 * DM_OVERHEAD)))


/* Define the header for the buffer queue */
struct packet_queue_header
{
     NET_BUFFER *head;
     NET_BUFFER *tail;
};

/* Define a generic queue header */
struct queue_header
{
    struct queue_element *head, *tail;
};
typedef struct queue_header NET_QUEUE_HEADER;

/* Define a generic queue element */
struct queue_element
{
        struct queue_element *next, *next_buffer;
};
typedef struct queue_element NET_QUEUE_ELEMENT;

/* Define the buffer suspension list structure. This list will
   hold tasks that are waiting to transmit because of lack of
   memory buffers. */
struct _mem_suspension_element
{
    struct _mem_suspension_element *flink;
    struct _mem_suspension_element *blink;
    NU_TASK                        *waiting_task;
};

struct _mem_suspension_list
{
    struct _mem_suspension_element *head;
    struct _mem_suspension_element *tail;
};

typedef struct _mem_suspension_list     NET_BUFFER_SUSPENSION_LIST;
typedef struct _mem_suspension_element  NET_BUFFER_SUSPENSION_ELEMENT;

/* Global data structures declared in MEM.C */
extern UINT16                       MEM_Buffers_Used;
extern NET_BUFFER_SUSPENSION_LIST   MEM_Buffer_Suspension_List;


#ifdef          __cplusplus
}
#endif /* _cplusplus */


#endif /* MEM_DEFS_H */
