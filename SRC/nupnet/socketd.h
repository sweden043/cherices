/*************************************************************************
*                                                                       
*    Copyright (c) 1993 - 2001 by Accelerated Technology, Inc.          
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
*   FILE NAME                                           VERSION                                            
*                                                                       
*       SOCKETD.H                                           4.4          
*                                                                       
*   COMPONENT                                                             
*                                                                       
*       Sockets                                                          
*                                                                       
*   DESCRIPTION                                                           
*                                                                       
*       Holds the defines for data structures related to sockets.        
*                                                                       
*   DATA STRUCTURES                                                       
*                                                                       
*       id_struct
*       addr_struct
*       sockaddr_struct
*       SCK_SOCKADDR_STRUCT
*       SCK_IP_ADDR_STRUCT
*       SCK_SOCKADDR_IP_STRUCT
*       sock_struct
*       TASK_TABLE_STRUCT
*       NU_Host_Ent
*       host
*       nu_fd_set
*       _ip_mreq
*       SCK_IOCTL_OPTION
*                                                                       
*   DEPENDENCIES                                                          
*                                                                       
*       nucleus.h
*       sockdefs.h
*       target.h
*                                                                       
*************************************************************************/

#ifndef SOCKETD_H
#define SOCKETD_H

#include "nucleus.h"
#include "sockdefs.h"   /* socket definitions */
#include "target.h"
#include "mem_defs.h"

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif /* _cplusplus */

#define     NU_IGNORE_VALUE  -1 /* Null parameter value     */
#define     NULL_IP        0    /* Used to initialize ip addresses to NULL */

#define     MAX_HOST_NAME_LENGTH 32 /* Max size of the lcoal host name */


/* 32-bit structure containing 4-digit ip number */
struct id_struct
{
    UINT8 is_ip_addrs[4];        /* IP address number */
};

struct addr_struct
{
    INT16    family;             /* family = INTERNET */
    UINT16   port;               /* machine's port number */
    struct   id_struct id;       /* contains the 4-digit ip number for the host machine */
    char     *name;              /* points to machine's name */
};

struct sockaddr_struct
{
    struct  id_struct ip_num;     /* the address = the ip num */
    UINT16  port_num;             /* the process = the port num */
    INT16   pad;
};


typedef struct SCK_SOCKADDR_STRUCT
{
    UINT8       sck_len;
    UINT8       sck_family;
    INT8        sck_data[14];
}SCK_SOCKADDR;

struct SCK_IP_ADDR_STRUCT
{
    UINT32      sck_ip_addr;
};


struct SCK_SOCKADDR_IP_STRUCT
{
    UINT8           sck_len;
    UINT8           sck_family;
    UINT16          sck_port;
    UINT32          sck_addr;
    INT8            sck_unused[8];
};


/* this is the socket 5-tuple */
struct sock_struct
{
  struct sockaddr_struct    s_local_addr;
  struct sockaddr_struct    s_foreign_addr;
  NET_BUFFER_HEADER         s_recvlist;     /* List of received packets. */
  NU_TASK                   *s_RXTask;      /* Task pending on a receive. */
  NU_TASK                   *s_TXTask;      /* Task pending on a transmit. */
  NU_TASK                   *s_CLSTask;     /* Task pending on a transmit. */
  IP_MULTI_OPTIONS          *s_moptions;    /* IP multicast options. */
  UINT32                    s_recvbytes;    /* Total bytes in s_recvlist. */
  UINT32                    s_recvpackets;  /* Total packets in s_recvlist. */
  struct TASK_TABLE_STRUCT  *s_accept_list; /* Established connections that have 
                                               yet to be accepted. */
  INT                       s_accept_index;
  INT                       s_port_index;   /* Port number. */
  UINT16                    s_state;        /* Internal state flags. */
  UINT16                    s_options;      /* Socket options as defined by BSD.  Currently */
                                            /* the only implemented option is SO_BROADCAST. */
  UINT16                    s_flags;
  UINT16                    s_protocol;
};


/*
 * Socket state bits.
 */
#define SS_NOFDREF              0x0001   /* no file table ref any more */
#define SS_ISCONNECTED          0x0002   /* socket connected to a peer */
#define SS_ISCONNECTING         0x0004   /* in process of connecting to peer */
#define SS_ISDISCONNECTING      0x0008   /* in process of disconnecting */
#define SS_DEVICEDOWN           0x0010   /* Used only by UDP sockets. Indicates 
                                            that the device that was being used by
                                            a UDP socket/port has gone down. */

/* 
 *  Socket Flag bits.
 */
#define SF_BLOCK                0x0001  /* Indicates blocking or non-blocking */
#define SF_LISTENER             0x0002  /* Is a TCP server listening */
    
/* task table structure - created during an NU_Listen call to
   store status on x number of connections for a single port number
   from a single task id */

struct TASK_TABLE_STRUCT
{
  struct TASK_TABLE_STRUCT *next;  /* pointer to the next task structure */
  NU_TASK *Task_ID;
  INT     *stat_entry;      /* status of each connection */
  INT     *socket_index;    /* portlist entry number of each connection */
  INT     socketd;
  UINT16  local_port_num;   /* port number of server */
  UINT16  current_idx;      /* points to oldest entry in the table; a task
                              should service this connection before the others */
  UINT16  total_entries;    /* number of backlog queues possible */
  INT8    acceptFlag;       /* Used to indicate that the task is suspended in the
                              NU_Accept service. */
  INT8    pad[1];
};

/* host structure */
typedef struct NU_Host_Ent
{
  CHAR   *h_name;
  CHAR   **h_alias;        /* unused */
  INT16  h_addrtype;
  INT16  h_length;
  CHAR   *h_addr;          /* contains the host's 4-digit ip number */
} NU_HOSTENT;

#define NU_Get_Host_by_NAME   NU_Get_Host_By_Name

/* Host information.  Used to match a host name with an address.  Used in
   hosts.c to setup information on foreign hosts. */
struct host
{
    CHAR   name[32];
    UINT8  address[4];
};

/* Defines added for the NU_Select service call. */
#define FD_BITS                 32
#define FD_ELEMENTS     (NSOCKETS/FD_BITS)+1

#define SCK_EVENT_Q_ELEMENT_SIZE    3       /* event queue element size 
                                               do not change the size */
#define SCK_EVENT_Q_NUM_ELEMENTS    100    /* number of elements in the
                                              event queue */

typedef struct nu_fd_set
{
  UINT32 words[FD_ELEMENTS];
} FD_SET;


/* Clear the connecting flag and set the connected flag. */
#define     SCK_CONNECTED(desc) \
        if (SCK_Sockets[desc])                              \
        {                                                   \
          SCK_Sockets[desc]->s_state &= (~SS_ISCONNECTING); \
          SCK_Sockets[desc]->s_state |= SS_ISCONNECTED;     \
        }

/* Change the the socket state to disconnecting. */
#define     SCK_DISCONNECTING(desc) \
            if ( (desc >= 0) && (SCK_Sockets[desc]) )               \
            {                                                       \
                SCK_Sockets[desc]->s_state &= (~SS_ISCONNECTED);    \
                SCK_Sockets[desc]->s_state |= SS_ISDISCONNECTING;   \
            }


/* IP Multicast Request structure. This structure is used when using 
   NU_Setsockopt or NU_Getsockopt to set or get IP multicasting options. */
typedef struct _ip_mreq {
    UINT32      sck_multiaddr;      /* IP multicast address. */
    UINT32      sck_inaddr;         /* IP address of the interface. */
} IP_MREQ;


/*  Miscellaneuos Defines for application layer interface to DEV, DHCP and BOOTP structures. */
#define  NU_DEVICE              DEV_DEVICE
#define  NU_BOOTP_STRUCT        BOOTP_STRUCT
#define  NU_DHCP_STRUCT         DHCP_STRUCT
#define  NU_Init_Devices        DEV_Init_Devices
#define  NU_Get_Host_Name       SCK_Get_Host_Name

/* Option commands for the NU_Ioctl service call. */
#define IOCTL_GETIFADDR     1       /* Get the IP address associated with an 
                                       interface. */
#define IOCTL_GETIFDSTADDR  2       /* Get the IP address on the foreign side
                                       of a PPP link. Only valid for PPP links. */

typedef struct _SCK_IOCTL_OPTION
{
    UINT8       *s_optval;
    
    union 
    {
        UINT8   s_ipaddr[4];
    } s_ret;

} SCK_IOCTL_OPTION;

#define NU_IOCTL_OPTION     SCK_IOCTL_OPTION


/* Function Prototypes */
INT  SCK_Check_Listeners(UINT16 port_num);
VOID SCK_Clear_Accept_Entry(struct TASK_TABLE_STRUCT *task_entry, INT index);

/* External References */
extern struct sock_struct *SCK_Sockets[NSOCKETS];
extern        UINT32       SCK_Ticks_Per_Second;

#ifdef          __cplusplus
}
#endif /* _cplusplus */

#endif  /* SOCKETD_H */
