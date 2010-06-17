/**************************************************************************
*                                                                          
*      Copyright (c) 1993 - 2001 by Accelerated Technology, Inc.           
*                                                                          
* PROPRIETARY RIGHTS of Accelerated Technology are involved in the subject 
* matter of this material.  All manufacturing, reproduction, use and sales 
* rights pertaining to this subject matter are governed by the license     
* agreement.  The recipient of this software implicity accepts the terms   
* of the license.                                                          
*                                                                          
****************************************************************************/
/**************************************************************************
*                                                                          
* FILENAME                                         VERSION                         
*                                                                                  
*      IP.H                                          4.4                           
*                                                                                  
* DESCRIPTION                                                              
*                                                                          
*      This include file will handle defines relating to the IP layer.     
*                                                                          
* DATA STRUCTURES                                                          
*                                                                          
*      IP_LAYER
*      PSEUDOTCP
*      IP_FRAG
*      IP_QUEUE_ELEMENT                                                    
*      IP_QUEUE                                                            
*      _IP_MULTI                                                           
*      _IP_MULTI_OPTIONS                                                   
*                                                                          
* DEPENDENCIES                                                             
*                                                                          
*      No other file dependencies                                          
*                                                                          
****************************************************************************/
#ifndef IP_H
#define IP_H


#include "rtab.h"
#include "mem_defs.h"

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif /* _cplusplus */

/***********************************************************************/
/*   Internet protocol
*
*/
typedef struct iph
{
    UINT8  ip_versionandhdrlen; /* I prefer to OR them myself */
                                /* each half is four bits */
    UINT8  ip_service;          /* type of service for IP */
    UINT16 ip_tlen;             /* total length of IP packet */
    UINT16 ip_ident;            /* these are all BYTE-SWAPPED! */
    UINT16 ip_frags;            /* combination of flags and value */
    UINT8  ip_ttl;              /* time to live */
    UINT8  ip_protocol;         /* higher level protocol type */
    UINT16 ip_check;            /* header checksum, byte-swapped */
    UINT32 ip_src;              /* IP source address */
    UINT32 ip_dest;             /* IP destination address */
} IPLAYER;

/*
*  used for computing checksums in TCP
*/
struct pseudotcp
{
    UINT32  source;     /* Source IP address. */
    UINT32  dest;       /* Destination IP address. */
    UINT8   z;          /* zero */
    UINT8   proto;      /* protocol number */
    UINT16  tcplen;     /* byte-swapped length field */
};


typedef struct IP_FRAG_STRUCT {
    UINT8                   ipf_hl;        /* version and header length */
    UINT8                   ipf_mff;       /* overlays IP TOS: use low bit to 
                                              avoid destroying tos; copied from
                                              (ip service & IP_MF) */
    UINT16                  ipf_tlen;         
    UINT16                  ipf_id;         
    UINT16                  ipf_off;       /* Contains only the fragment offset,
                                              unlike the normal IP field which 
                                              contains offset and flags fields. 
                                            */
    UINT8                   ipf_ttl;
    UINT8                   ipf_buf_offset; /* The protocol field is used to 
                                               store the offset of the IP layer 
                                               from the start of the memory 
                                               buffer.
                                            */
    UINT16                  ipf_check;
    struct IP_FRAG_STRUCT   *ipf_next;
    struct IP_FRAG_STRUCT   *ipf_prev;
} HUGE IP_FRAG;

typedef struct _IP_QUEUE_ELEMENT {
    struct _IP_QUEUE_ELEMENT    *ipq_next;
    struct _IP_QUEUE_ELEMENT    *ipq_prev;
    IP_FRAG                     *ipq_first_frag;
    UINT32                      ipq_source;
    UINT32                      ipq_dest;
    UINT16                      ipq_id;
    UINT8                       ipq_protocol;
    UINT8                       ipq_pad;  /* The pad is added to make this
                                             structure an even number of long
                                             words.
                                           */
} IP_QUEUE_ELEMENT;

typedef struct _IP_QUEUE
{
     IP_QUEUE_ELEMENT       *ipq_head;
     IP_QUEUE_ELEMENT       *ipq_tail;
} IP_QUEUE;



struct _IP_MULTI
{
    UINT32              ipm_addr;
    DV_DEVICE_ENTRY     *ipm_device;
    UINT32              ipm_refcount;
    UINT32              ipm_timer;
    IP_MULTI            *ipm_next;
};

#define     IP_MAX_MEMBERSHIPS          10

struct _IP_MULTI_OPTIONS
{
    DV_DEVICE_ENTRY     *ipo_device;
    IP_MULTI            *ipo_membership[IP_MAX_MEMBERSHIPS];
    UINT8               ipo_ttl;
    UINT8               ipo_loop;
    UINT8               ipo_num_mem;
    UINT8               pad1;
};


/* Define the MAX size of data that can be contained in one IP packet. */
#define IP_MAX_DATA_SIZE                65496


/* Multicast definitions. */
#define IP_DEFAULT_MULTICAST_TTL        1  /* By default keep multicast packets
                                              on the local segment. */
#define IP_DEFAULT_MULTICAST_LOOP       0  /* Loop back is not yet supported. */


/* RFC 1122 recommends a default ttl for fragments of 60 to 120 seconds. */
#define IP_FRAG_TTL     ((UINT32)(SCK_Ticks_Per_Second * 20))


/* The following macro returns a pointer to the IP header within a buffer. */
#define IP_BUFF_TO_IP(buff_ptr) \
        (IPLAYER *)(buff_ptr->data_ptr - sizeof(IPLAYER))


#define IP_FORWARDING       (1<<0)
#define IP_RAWOUTPUT        (1<<1)
#define IP_ROUTETOIF        (1<<2)
#define IP_ALLOWBROADCAST   (1<<3)


#define IP_VERSION      0x4

#define IP_DF           0x4000      /* don't fragment flag */
#define IP_MF           0x2000      /* more fragments flag */

#define IP_ADDR_ANY             0x0UL
#define IP_ADDR_BROADCAST       0xFFFFFFFFUL

#define IP_TIME_TO_LIVE     30
#define IP_TYPE_OF_SERVICE  0

/* Length of IP address in bytes */
#define IP_ADDR_LEN        4

#define IP_CLASSA_ADDR(i)        (((long)(i) & 0x80000000ul) == 0)
#define IP_CLASSB_ADDR(i)        (((long)(i) & 0xc0000000ul) == 0x80000000ul)
#define IP_CLASSC_ADDR(i)        (((long)(i) & 0xe0000000ul) == 0xc0000000ul)
#define IP_CLASSD_ADDR(i)        (((long)(i) & 0xf0000000ul) == 0xe0000000ul)
#define IP_MULTICAST_ADDR(i)     IP_CLASSD_ADDR(i)
#define IP_EXPERIMENTAL_ADDR(i)  (((long)(i) & 0xf0000000ul) == 0xf0000000ul)

#define IP_ADDR(a)  TLS_IP_Address(a)
#define IP_ADDR_COPY(dest, src)  ((UINT8 *)(dest))[0] = ((UINT8 *)(src))[0]; \
                                 ((UINT8 *)(dest))[1] = ((UINT8 *)(src))[1]; \
                                 ((UINT8 *)(dest))[2] = ((UINT8 *)(src))[2]; \
                                 ((UINT8 *)(dest))[3] = ((UINT8 *)(src))[3]

#define IP_HEADER_COPY(dest, src)                                           \
        /* Copy the IP header over. We will do this 32 bits at a time. */   \
                                                                            \
        /* Version, header length, TOS, and Total Length. */                \
        PUT32 ((UINT8 *) dest, IP_VERSIONANDHDRLEN_OFFSET,                  \
                    GET32 ((UINT8 *)src, IP_VERSIONANDHDRLEN_OFFSET));      \
                                                                            \
        /* ID, Flags, and Fragments */                                      \
        PUT32 ((UINT8 *)dest, IP_IDENT_OFFSET,                              \
                    GET32 ((UINT8 *)src, IP_IDENT_OFFSET));                 \
                                                                            \
        /* Time To Live, Protocol, and Header checksum. */                  \
        PUT32 ((UINT8 *)dest, IP_TTL_OFFSET,                                \
                    GET32 ((UINT8 *)src, IP_TTL_OFFSET));                   \
                                                                            \
        /* Source Address. */                                               \
        PUT32 ((UINT8 *)dest, IP_SRC_OFFSET,                                \
                    GET32 ((UINT8 *)src, IP_SRC_OFFSET));                   \
                                                                            \
        /* Destination Address. */                                          \
        PUT32 ((UINT8 *)dest, IP_DEST_OFFSET,                               \
                    GET32 ((UINT8 *)src, IP_DEST_OFFSET));

/* The Standard protocol types for IP packets. */
#define IP_UDP_PROT     17
#define IP_TCP_PROT     6
#define IP_ICMP_PROT    1
#define IP_IGMP_PROT    2
#define IP_RAW_PROT     255
#define IP_HELLO_PROT   63
#define IP_OSPF_PROT    89

/*
 * Definitions for options.
 */
#define IP_OPT_COPIED(o)        ((o)&0x80)
#define IP_OPT_CLASS(o)         ((o)&0x60)
#define IP_OPT_NUMBER(o)        ((o)&0x1f)

#define IP_OPT_CONTROL      0x00
#define IP_OPT_RESERVED1    0x20
#define IP_OPT_DEBMEAS      0x40
#define IP_OPT_RESERVED2    0x60

#define IP_OPT_EOL          0       /* end of option list */
#define IP_OPT_NOP          1       /* no operation */

#define IP_OPT_RR           7       /* record packet route */
#define IP_OPT_TS           68      /* timestamp */
#define IP_OPT_SECURITY     130     /* provide s,c,h,tcc */
#define IP_OPT_LSRR         131     /* loose source route */
#define IP_OPT_SATID        136     /* satnet id */
#define IP_OPT_SSRR         137     /* strict source route */

/*
 * Offsets to fields in options other than EOL and NOP.
 */
#define IP_OPT_OPTVAL       0       /* option ID */
#define IP_OPT_OLEN         1       /* option length */
#define IP_OPT_OFFSET       2       /* offset within option */
#define IP_OPT_MINOFF       4       /* min value of above */


/* External declarations. */
extern UINT8 IP_Brd_Cast[IP_ADDR_LEN];
extern UINT8 IP_Null[IP_ADDR_LEN];
extern const UINT8   IP_A_Mask[4];
extern const UINT8   IP_B_Mask[4];
extern const UINT8   IP_C_Mask[4];

/* Function prototypes. */
VOID   IP_Initialize(VOID);
STATUS IP_Interpret (IPLAYER *p, DV_DEVICE_ENTRY *device, NET_BUFFER *buf_ptr);
STATUS IP_Send(NET_BUFFER *buf_ptr, RTAB_ROUTE *ro, UINT32 dest_ip, UINT32 src_ip,
               INT32 flags, INT ttl, INT protocol, INT tos, 
               IP_MULTI_OPTIONS *mopt);
VOID   IP_Find_Route(RTAB_ROUTE *ro);
STATUS IP_Forward(NET_BUFFER *buf_ptr);
STATUS IP_Canforward(UINT32 dest);
STATUS IP_Get_Net_Mask(UINT8 *ip_addr, UINT8 *mask);
NET_BUFFER *IP_Reassembly(IP_FRAG *, IP_QUEUE_ELEMENT *, NET_BUFFER *);
VOID IP_Reassembly_Event(IP_QUEUE_ELEMENT *);
VOID IP_Free_Queue_Element(IP_QUEUE_ELEMENT *);
VOID IP_Insert_Frag(IP_FRAG *ip, IP_QUEUE_ELEMENT *);
VOID IP_Remove_Frag(IP_FRAG *ip, IP_QUEUE_ELEMENT *);
INT  IP_Broadcast_Addr(UINT32 dest, DV_DEVICE_ENTRY *int_face);
STATUS IP_Fragment (NET_BUFFER *, IPLAYER *, DV_DEVICE_ENTRY *, 
                    SCK_SOCKADDR_IP *, RTAB_ROUTE *);
INT IP_Option_Copy (IPLAYER *dip, IPLAYER *sip);

STATUS   IP_Set_Opt (INT, INT, void *, INT);
STATUS IP_Set_Raw_Opt(INT socketd, INT optname, void *optval, INT optlen);
IP_MULTI *IP_Lookup_Multi(UINT32 m_addr, DEV_IF_ADDRESS *if_addr);
STATUS   IP_Set_Multi_Opt(INT, INT, void *, INT);
IP_MULTI *IP_Add_Multi(UINT32 m_addr, DV_DEVICE_ENTRY *dev);
STATUS   IP_Delete_Multi(IP_MULTI *ipm);
STATUS   IP_Get_Multi_Opt(INT, INT, void *, INT *);
STATUS   IP_Get_Opt (INT, INT, void *, INT *);
INT      IP_Localaddr(UINT32 ip_addr);


#define RTFREE(x)

#define IP_VERSIONANDHDRLEN_OFFSET  0
#define IP_SERVICE_OFFSET           1
#define IP_TLEN_OFFSET              2
#define IP_IDENT_OFFSET             4
#define IP_FRAGS_OFFSET             6
#define IP_TTL_OFFSET               8
#define IP_PROTOCOL_OFFSET          9
#define IP_CHECK_OFFSET             10
#define IP_SRC_OFFSET               12
#define IP_DEST_OFFSET              16

#define IPF_HL_OFFSET               0
#define IPF_MFF_OFFSET              1
#define IPF_TLEN_OFFSET             2
#define IPF_ID_OFFSET               4
#define IPF_OFF_OFFSET              6
#define IPF_TTL_OFFSET              8
#define IPF_BUF_OFFSET              9
#define IPF_CHECK_OFFSET            10
#define IPF_NEXT_OFFSET             12
#define IPF_PREV_OFFSET             16

#ifdef          __cplusplus
}
#endif /* _cplusplus */

#endif /* IP_H */
