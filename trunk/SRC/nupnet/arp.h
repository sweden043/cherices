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
* FILENAME                                      VERSION                            
*                                                                                  
*      ARP.H                                      4.4                              
*                                                                                  
* DESCRIPTION                                                              
*                                                                          
*      This include file will handle ARP and RARP protocol defines.        
*                                                                          
* DATA STRUCTURES                                                          
*                                                                          
*       ARP_ENTRY
*       ARP_LAYER
*       ARP_RESOLVE_ENTRY
*       ARP_RESOLVE_LIST
*       ARP_MAC_HEADER                            
*
* DEPENDENCIES                                                             
*                                                                          
*      No other file dependencies                                          
*                                                                          
****************************************************************************/

#ifndef ARP_H
#define ARP_H

#include "mem_defs.h"
#include "ip.h"

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif /* _cplusplus */


/***********************************************************************/
/*  ARP cache
*   Data structure for saving low-level information until needed
*/
typedef struct ARP_ENTRY_STRUCT
{
    UINT32  arp_ip_addr;            /* the IP # in question                 */
    INT     arp_flags;              /* is this a gateway?                   */
                                /* Is this a valid entry.  For gateways an  */
                                /* entry is created before the HW addr is   */
                                /* known.  This flag indicates when the HW  */
                                /* is resolved.                             */
    UINT32  arp_time;               /* time information                     */
    UINT8   arp_mac_addr[DADDLEN];  /* hardware address for this IP address */
    UINT8   pad[2];
} ARP_ENTRY;

#define ARP_UP          0x1         /* Is this entry valid. */
#define ARP_GATEWAY     0x2         /* Is this entry for a gateway. */
#define ARP_PERMANENT   0x4         /* Is this entry permanent. */

/* This structure defines an ARP packet. */
typedef struct ARP_LAYER_STRUCT
{
    UINT16  arp_hrd;                /* hardware type, Ethernet = 1 */
    UINT16  arp_pro;                /* protocol type to resolve for */
    UINT8   arp_hln;                /* byte length of hardware addr = 6 for ETNET */
    UINT8   arp_pln;                /* byte length of protocol = 4 for IP */
    UINT16  arp_op;                 /* opcode, request = 1, reply = 2, RARP = 3,4 */
    UINT8   arp_sha[DADDLEN];       /* Source Hardware Address */
    UINT8   arp_spa[IP_ADDR_LEN];   /* Source protocol address. */
    UINT8   arp_tha[DADDLEN];       /* Target Hardware Address */
    UINT8   arp_tpa[IP_ADDR_LEN];   /* Target protocol address. */
/*
*   the final four fields (contained in 'rest') are:
*     sender hardware address:   sha       hln bytes
*     sender protocol address:   spa       pln bytes
*     target hardware address:   tha       hln bytes
*     target protocol address:   tpa       pln bytes
*/
} ARP_LAYER;


/*************************************************************************/
/*  Dave Plummer's  Address Resolution Protocol (ARP) (RFC-826) and 
*   Finlayson, Mann, Mogul and Theimer's Reverse ARP packets.
*
*   Note that the 2 byte ints are byte-swapped.  The protocols calls for
*   in-order bytes, and the PC is lo-hi ordered.
*   
*/
#define RARPR   0x0004       /*  RARP reply, from host, needs swap */
#define RARPQ   0x0003       /*  RARP request, needs swapping */
#define ARPREP  0x0002       /*  reply, byte swapped when used */
#define ARPREQ  0x0001       /*  request, byte-swapped when used */
#define ARPPRO  0x0800       /*  IP protocol, needs swapping */

#define     RARP_MAX_ATTEMPTS   5

/* ARP_RESOLVE_STRUCT is used to keep track the resolution of MAC layer
   addresses.
 */
typedef struct ARP_RESOLVE_STRUCT
{
    struct ARP_RESOLVE_STRUCT   *ar_next;
    struct ARP_RESOLVE_STRUCT   *ar_prev;
    DV_DEVICE_ENTRY             *ar_device;
    UINT32                      ar_dest;
    NU_TASK                     *ar_task;
    NET_BUFFER                  *ar_buf_ptr;
    INT                         ar_send_count;
    INT                         ar_pkt_type;
    UINT16                      ar_id;
} ARP_RESOLVE_ENTRY;

typedef struct _ARP_RESOLVE_LIST
{
    struct ARP_RESOLVE_STRUCT   *ar_head;
    struct ARP_RESOLVE_STRUCT   *ar_tail;
} ARP_RESOLVE_LIST;

/* This structure is used by ARP to build the MAC layer header in before 
   passing it to the MAC layer send routine. */
typedef struct ARP_MAC_HEADER_STRUCT
{
    UINT8           ar_len;
    UINT8           ar_family;
    union {
        /* Right now the only MAC layer supported with ARP is ethernet. However,
           an entry for other such as token ring could be added to this union. */
        DLAYER      ar_mac_ether;
    } ar_mac;

} ARP_MAC_HEADER;

/* Function Prototypes */
ARP_ENTRY *ARP_Find_Entry(SCK_SOCKADDR_IP *dest);
STATUS ARP_Resolve(DV_DEVICE_ENTRY *int_face, SCK_SOCKADDR_IP *ip_dest,
                   UINT8 *mac_dest, NET_BUFFER *buf_ptr);
STATUS ARP_Request(DV_DEVICE_ENTRY *device, UINT32 *tip, UINT8 *thardware,
                   UINT16 protocol_type, INT16 arp_type);
INT     ARP_Cache_Update(UINT32 ipn, UINT8 *hrdn, INT flags);
STATUS  ARP_Reply(UINT8 *, UINT32, DV_DEVICE_ENTRY *);
VOID    ARP_Event(UINT16 id);
STATUS  ARP_Interpret(ARP_LAYER *a_pkt, DV_DEVICE_ENTRY *device);
NET_BUFFER  *ARP_Build_Pkt (DV_DEVICE_ENTRY *dev, UINT32 tipnum, const UINT8 *thardware,
                        INT16 pkt_type);
STATUS ARP_Rarp(CHAR *device_name);

/* Offsets of the ARP header fields. */
#define ARP_HRD_OFFSET              0
#define ARP_PRO_OFFSET              2
#define ARP_HLN_OFFSET              4
#define ARP_PLN_OFFSET              5
#define ARP_OP_OFFSET               6
#define ARP_SHA_OFFSET              8
#define ARP_SPA_OFFSET              14
#define ARP_THA_OFFSET              18
#define ARP_TPA_OFFSET              24

#ifdef          __cplusplus
}
#endif /* _cplusplus */


#endif /* ARP_H */
