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
*      DNS.H                                         4.4                           
*                                                                                  
* DESCRIPTION                                                              
*                                                                          
*      This include file will handle domain processing defines.            
*                                                                          
* DATA STRUCTURES                                                          
*                                                                          
*       DNS_PKT_HEADER
*       DNS_RR
*       DNS_HOST
*       DNS_HOST_LIST
*       DNS_SERVER
*       DNS_SERVER_LIST
*                                                                         
* DEPENDENCIES                                                             
*                                                                          
*      No other file dependencies                                          
*                                                                          
***************************************************************************/

#ifndef DNS_H
#define DNS_H

#include "target.h"

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif /* _cplusplus */

/* The max number of DNS servers that can be registered with Nucleus NET. */
#define     DNS_MAX_DNS_SERVERS     5

/* Max and Min size defintions. */
#define     DNS_MAX_LABEL_SIZE      63
#define     DNS_MAX_NAME_SIZE       255
#define     DNS_MAX_MESSAGE_SIZE    512
#define     DNS_MAX_ATTEMPTS        5
#define     DNS_MIN_NAME_ALLOC      51  /* 50 characters + NULL terminator. */

/* This is the port DNS servers listen for queries on. */
#define     DNS_PORT                53

/* Resource Record (RR) type codes: */
#define DNS_TYPE_A      1           /* A host address (RR)    */
#define DNS_TYPE_PTR    12          /* A domain name ptr (RR) */

/* RR Class definitions.  The only one we care about is the Internet class. */
#define DNS_CLASS_IN    1           /* Internet class */

/*
 *  flag masks for the flags field of the DNS header
 */
#define DNS_QR         0x8000          /* query=0, response=1 */
#define DNS_OPCODE     0x7800          /* opcode, see below */
#define DNS_AA         0x0400          /* Authoritative answer */
#define DNS_TC         0x0200          /* Truncation, response was cut off at 512 */
#define DNS_RD         0x0100          /* Recursion desired */
#define DNS_RA         0x0080          /* Recursion available */
#define DNS_RCODE_MASK 0x000F

/* opcode possible values: */
#define DNS_OPQUERY    0    /* a standard query */
#define DNS_OPIQ       1    /* an inverse query */
#define DNS_OPCQM      2    /* a completion query, multiple reply */
#define DNS_OPCQU      3    /* a completion query, single reply */

/* the rest reserved for future */
#define DNS_ROK        0    /* okay response */
#define DNS_RFORM      1    /* format error */
#define DNS_RFAIL      2    /* their problem, server failed */
#define DNS_RNAME      3    /* name error, we know name doesn't exist */
#define DNS_RNOPE      4    /* no can do request */
#define DNS_RNOWAY     5    /* name server refusing to do request */
#define DNS_WILD       255  /* wildcard for several of the classifications */

/* These definitions are used to control where new DNS servers are added into 
   the list of servers. */
#define DNS_ADD_TO_FRONT        1 /* Add to the front of the list. */
#define DNS_ADD_TO_END          2 /* Add to the end of the list. */


/* All DNS messages have a header defined as follows. */
typedef struct _DNS_PKT_HEADER
{
    UINT16      dns_id;
    UINT16      dns_flags;
    UINT16      dns_qdcount;
    UINT16      dns_ancount;
    UINT16      dns_nscount;
    UINT16      dns_arcount;

} DNS_PKT_HEADER;

#define DNS_ID_OFFSET                   0
#define DNS_FLAGS_OFFSET                2
#define DNS_QDCOUNT_OFFSET              4
#define DNS_ANCOUNT_OFFSET              6
#define DNS_NSCOUNT_OFFSET              8
#define DNS_ARCOUNT_OFFSET              10

/*
 *  A resource record is made up of a compressed domain name followed by
 *  this structure.  All of these ints need to be byteswapped before use.
 */
typedef struct _DNS_RR
{
    UINT16      dns_type;           /* resource record type=DTYPEA */
    UINT16      dns_class;          /* RR class=DIN */
    UINT32      dns_ttl;            /* time-to-live, changed to 32 bits */
    UINT16      dns_rdlength;       /* length of next field */
    CHAR        dns_rdata[1];       /* data field */
    UINT8       pad1;
} DNS_RR;

#define DNS_TYPE_OFFSET                 0
#define DNS_CLASS_OFFSET                2
#define DNS_TTL_OFFSET                  4
#define DNS_RDLENGTH_OFFSET             8
#define DNS_RDATA_OFFSET                10


/* This structure is defines what a host looks like. */
typedef struct _DNS_HOST
{
    struct _DNS_HOST    *dns_next;
    struct _DNS_HOST    *dns_previous;
    UNSIGNED            dns_ttl;            /* Time To Live for this entry.  A
                                               value of 0 is used to indicate a
                                               permanent entry. */
    INT                 dns_name_size;      /* The size of the name in this
                                               entry. */
    CHAR                *dns_name;          /* Host name. */
    CHAR                dns_ipaddr[4];      /* Host IP address. */
} DNS_HOST;

/* Define the head of the linked list of HOSTs. */
typedef struct _DNS_HOST_LIST
{
    DNS_HOST    *dns_head;
    DNS_HOST    *dns_tail;
} DNS_HOST_LIST;


/* Define DNS Server list structure */
typedef struct _DNS_SERVER
{
    struct _DNS_SERVER      *dnss_next;
    struct _DNS_SERVER      *dnss_previous;
    UINT8                    dnss_ip[4];
} DNS_SERVER;

typedef struct _DNS_SERVER_LIST
{
    DNS_SERVER      *dnss_head;
    DNS_SERVER      *dnss_tail;
}DNS_SERVER_LIST;


/* Function prototypes. */
STATUS    DNS_Initialize(VOID);
STATUS    DNS_Find_Host_By_Name(CHAR *name, DNS_HOST **host);
STATUS    DNS_Find_Host_By_Addr(CHAR *addr, DNS_HOST **host);
STATUS    NU_Add_DNS_Server (UINT8 *new_dns_server, INT where);
STATUS    NU_Delete_DNS_Server ( UINT8 *dns_ip );
INT       NU_Get_DNS_Servers(UINT8 *dest, INT size);


#ifdef          __cplusplus
}
#endif /* _cplusplus */

#endif  /* DNS_H */
