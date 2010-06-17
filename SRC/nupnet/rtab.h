/*************************************************************************
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

/*************************************************************************
*                                                                       
*   FILE NAME                                       VERSION                       
*                                                                               
*       RTAB.H                                        4.4                        
*                                                                               
*   COMPONENT                                                             
*                                                                       
*       Routing                                                          
*                                                                       
*   DESCRIPTION                                                           
*                                                                       
*       Holds the defines for routing.                                   
*                                                                       
*   DATA STRUCTURES                                                       
*                                                                       
*       route_node
*       destination_addr
*       route_entry
*       _RTAB_ROUTE
*                                                                       
*   DEPENDENCIES                                                          
*                                                                       
*       socketd.h
*       dev.h
*       rip2.h
*                                                                       
*************************************************************************/

#ifndef _RTAB_H
#define _RTAB_H

#include "socketd.h"
#include "dev.h"
#include "rip2.h"

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif /* _cplusplus */


#define SEMA_WAIT_TIME      ((UINT32)(3 * SCK_Ticks_Per_Second))
#define RT_LIFE_TIME        ((UINT32)(180 * SCK_Ticks_Per_Second))

#define MORE        0       /* RIGHT */
#define LESS        1       /* LEFT */
#define MIDDLE      2
#define CMP_ERROR   3

#define ISLEAF(n)       (n->rt_child[LESS] == 0 && n->rt_child[MORE] == 0)
#define ISNODE(n)       (n->rt_child[LESS] != 0 || n->rt_child[MORE] != 0)
#define ISROOT(n)       (n->rt_parent == 0)

#define NULL_ROUTE_NODE     (ROUTE_NODE *)0

struct route_node {
    struct route_node *rt_parent;   /* parent to this node */
    struct route_node *rt_child[2]; /* left and right child of node */

    INT16           rt_flags;       /* Up/Down, Host/Net */
    INT16           rt_refcnt;      /* # held references */
    UINT32          rt_use;         /* # of packets sent over this route */
    UINT32          rt_clock;       /* number of clock ticks of last update */
    UINT32          rt_lastsent;    /* if clock != lastsent then broadcast */
    UINT32          rt_sendcnt;     /* controls if a route has been sent */
    SCK_SOCKADDR_IP rt_gateway;     /* gateway for route, if any */
    RIP2_ENTRY      *rt_rip2;       /* holds rip2 entry structure */
    RIP2_AUTH_ENTRY *rt_auth;       /* holds rip2 auth entry structure */
    DV_DEVICE_ENTRY *rt_device;     /* pointer to the interface structure */
};

typedef struct route_node ROUTE_NODE;

struct destination_addr {
    UINT8 ip_addr[4];
    UINT8 ip_mask[4];
};

typedef struct destination_addr DEST_ADDR;

struct route_entry {
    RIP2_ENTRY *rip2;
};

typedef struct route_entry ROUTE_ENTRY;

struct _RTAB_Route
{
    ROUTE_NODE          *rt_route;
    SCK_SOCKADDR_IP     rt_ip_dest;
};

/* Route Flags */
#define RT_UP          0x1         /* route usable */
#define RT_GATEWAY     0x2         /* destination is a gateway */
#define RT_HOST        0x4         /* host entry (net otherwise) */
#define RT_REJECT      0x8         /* host or net unreachable */
#define RT_DYNAMIC     0x10        /* created dynamically (by redirect) */
#define RT_MODIFIED    0x20        /* modified dynamically (by redirect) */
#define RT_DONE        0x40        /* message confirmed */
#define RT_MASK        0x80        /* subnet mask present */
#define RT_CLONING     0x100       /* generate new routes on use */
#define RT_XRESOLVE    0x200       /* external daemon resolves name */
#define RT_LLINFO      0x400       /* generated by ARP or ESIS */
#define RT_STATIC      0x800       /* manually added */
#define RT_BLACKHOLE   0x1000      /* just discard pkts (during updates) */
#define RT_SILENT      0x2000      /* this route is kept silent from routing protocol info. 
                                      such as the loopback device's route */
#define RT_USED        0x4000      /* This entry in the routing table is */
                                   /* being used. */
#define RT_PROTO1      0x8000      /* protocol specific routing flag */


/* The function prototypes known to the outside world. */
ROUTE_NODE *RTAB_Root_Node(VOID);
INT RTAB_Insert_Node( ROUTE_NODE * );
INT RTAB_Delete_Node( ROUTE_NODE * );

VOID RTAB_Init(VOID);
ROUTE_NODE *RTAB_Find_Route( SCK_SOCKADDR_IP * );
STATUS RTAB_Add_Route(DV_DEVICE_ENTRY *, UINT32, UINT32, UINT32, INT16);
STATUS RTAB_Delete_Route (UINT8 *);
VOID   RTAB_Free( ROUTE_NODE * );
STATUS RTAB_Set_Default_Route(DV_DEVICE_ENTRY *device, UINT32 gw, UINT16 flags);
ROUTE_NODE *RTAB_Get_Default_Route(VOID);
VOID RTAB_Redirect(UINT32, UINT32, INT, UINT32);
  
#ifdef          __cplusplus
}
#endif /* _cplusplus */


#endif /* RTAB_H */
