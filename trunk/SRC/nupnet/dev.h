/***********************************************************************
*                                                                       
*      Copyright (c) 1993 - 2001 by Accelerated Technology, Inc.        
*                                                                       
* PROPRIETARY RIGHTS of Accelerated Technology are involved in the      
* subject matter of this material.  All manufacturing, reproduction,    
* use, and sales rights pertaining to this subject matter are governed  
* by the license agreement.  The recipient of this software implicitly  
* accepts the terms of the license.                                     
*                                                                       
*************************************************************************/

/****************************************************************************
*                                                                            
* FILE NAME                                   VERSION                                
*                                                                                    
*   DEV.H                                       4.4                                  
*                                                                                    
* DESCRIPTION                                                                
*                                                                            
*   Definitions for multiple device driver interface.                        
*                                                                            
* DATA STRUCTURES                                                            
*                                                                            
*      DEV_IF_ADDRESS                                                        
*      _DV_DEVICE_ENTRY                                                      
*      DV_DEVICE_LIST                                                        
*      _DV_REQ                                                               
*      URT_DEV                                                               
*      ETHER_DEV                                                             
*      _DEV_DEVICE                                                           
*                                                                            
* DEPENDENCIES                                                               
*                                                                            
*      None                                                                  
*                                                                            
******************************************************************************/

#ifndef DEV_H
#define DEV_H

#include "mem_defs.h"
#include "net.h"

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif /* _cplusplus */

/* This is the maximum length of a device name. This value should be a 
   multiple of 4. */
#define DEV_NAME_LENGTH         16

typedef struct dev_if_address {
        UINT32  dev_ip_addr;                    /* address of interface */
        UINT32  dev_dst_ip_addr;                /* other end of p-to-p link */
        UINT32  dev_netmask;                    /* used to determine subnet */
        UINT32  dev_net;                        /* Network number. */
        UINT32  dev_net_brdcast;                /* Network broadcast. */
        IP_MULTI *dev_multiaddrs;

#if (INCLUDE_DHCP == NU_TRUE)
        /* These fields are used only if the IP address is obtained via DHCP. */
        UINT32  dev_dhcp_lease;             /* Lease time for the IP addr. */
        UINT32  dev_dhcp_renew;             /* Renwal time for IP addr. */
        UINT32  dev_dhcp_rebind;            /* Rebinding time for the IP addr. */
        UINT32  dev_dhcp_state;             /* The DHCP state. */
        UINT32  dev_dhcp_server_addr;       /* The DHCP server's IP address. */
        INT32   dev_dhcp_opts_length;       /* Length of the options */
        CHAR   *dev_dhcp_options;           /* DHCP Options */
#endif /* INCLUDE_DHCP */

} DEV_IF_ADDRESS;

struct  _DV_DEVICE_ENTRY {

    struct _DV_DEVICE_ENTRY *dev_next;
    struct _DV_DEVICE_ENTRY *dev_previous;
    CHAR                    dev_net_if_name[DEV_NAME_LENGTH];   /* Must be unique. */
    INT32                   dev_flags;
    UINT32                  dev_index;          /* Unique identifier. */
    UINT32                  dev_irq;
    UINT16                  dev_vect;
    UINT32                  dev_sm_addr;       /* shared memmory address */
    UINT32                  dev_io_addr;
    UINT8                   dev_mac_addr[6];   /* Address of device. */

    /* procedure handles */
    STATUS  (*dev_open) (DV_DEVICE_ENTRY *);
    STATUS  (*dev_start) (DV_DEVICE_ENTRY *, NET_BUFFER *);
    STATUS  (*dev_receive) (DV_DEVICE_ENTRY *);
    STATUS  (*dev_output) (NET_BUFFER *, DV_DEVICE_ENTRY *,
                            SCK_SOCKADDR_IP *, RTAB_ROUTE *);
    STATUS  (*dev_input) (VOID);
    STATUS  (*dev_ioctl) (DV_DEVICE_ENTRY *, INT, DV_REQ *);
    STATUS  (*dev_event) (DV_DEVICE_ENTRY *, UNSIGNED);

    /* transmit list pointer. */
    NET_BUFFER_HEADER       dev_transq;
    UINT32                  dev_transq_length;
    DEV_IF_ADDRESS          dev_addr;           /* Address information.         */
    NET_MULTI               *dev_ethermulti;    /* List of multicast ethernet   */
    VOID                    *dev_ppp_layer;     /* All information regarding a  */
                                                /* PPP link. Including events,  */
                                                /* timers, ect.                 */

    /* generic interface information */
    UINT8           dev_type;       /* ethernet, tokenring, etc */
    UINT8           dev_addrlen;    /* media address length */
    UINT8           dev_hdrlen;     /* media header length */
    UINT32          dev_mtu;        /* maximum transmission unit, excluding media
                                       header length, i.e., 1500 for ethernet */
    UINT32          dev_metric;     /* routing metric (external only) */
    UINT32          dev_com_port;
    UINT32          dev_baud_rate;
    UINT32          dev_data_mode;
    UINT32          dev_parity;
    UINT32          dev_stop_bits;
    UINT32          dev_data_bits;
    UINT32          dev_driver_options;

    UINT32          user_defined_1; /* Available for users for anything. */
    UINT32          user_defined_2; /* Available for users for anything. */
    UINT32          system_use_1;   /* Reserverd for System use. */
    UINT32          system_use_2;   /* Reserverd for System use. */
};


typedef struct _DV_DEVICE_LIST
{
    struct _DV_DEVICE_ENTRY   *dv_head;
    struct _DV_DEVICE_ENTRY   *dv_tail;
} DV_DEVICE_LIST;

#define     DV_NAME_SIZE    16

/* The device request structure is used when making requests to a driver via
   the driver's ioctl function. */
struct _DV_REQ
{
    char        dvr_name[DV_NAME_SIZE];
    union 
    {
        UINT32      dvru_addr;
        UINT32      dvru_dstaddr;
        UINT32      dvru_broadaddr;
        UINT16      drvu_flags;
        int         dvru_metric;
        UINT8       *dvru_data;
    } dvr_dvru;
};

/* These macros simplify access to the fields in struct _DV_REQ. */
#define dvr_addr        dvr_dvru.dvru_addr
#define dvr_dstaddr     dvr_dvru.dvru_dstaddr
#define dvr_broadaddr   dvr_dvru.dvru_broadaddr
#define dvr_flags       dvr_dvru.dvru_flags
#define dvr_metric      dvr_dvru.dvru_metric
#define dvr_data        dvr_dvru.dvru_data

/* Device ioctl options. */
#define DEV_ADDMULTI	1
#define DEV_DELMULTI	2
#define DEV_GET_MAC	3
#define DEV_SET_MAC	4	
#define DEV_GET_IP	5
#define DEV_SET_IP	6	
#define DEV_GET_MASK	7
#define DEV_SET_MASK	8
#define DEV_GET_LINK_STATE 9
/*  Defines for the DEV_NET_IF.dev_flags field.  */

#define DV_UP           0x1     /* interface is up                  */
#define DV_BROADCAST    0x2     /* broadcast address valid          */
#define DV_DEBUG        0x4     /* turn on debugging                */
#define DV_LOOPBACK     0x8     /* is a loopback net                */
#define DV_POINTTOPOINT 0x10    /* interface is point-to-point link */
#define DV_NOTRAILERS   0x20    /* avoid use of trailers            */
#define DV_RUNNING      0x40    /* resources allocated              */
#define DV_NOARP        0x80    /* no address resolution protocol   */
#define DV_PROMISC      0x100   /* receive all packets              */
#define DV_ALLMULTI     0x200   /* receive all multicast packets    */
#define DV_OACTIVE      0x400   /* transmission in progress         */
#define DV_SIMPLEX      0x800   /* can't hear own transmissions     */
#define DV_LINK0        0x1000  /* per link layer defined bit       */
#define DV_LINK1        0x2000  /* per link layer defined bit       */
#define DV_LINK2        0x4000  /* per link layer defined bit       */
#define DV_MULTICAST    0x8000  /* supports multicast               */

/* Device types. */
#define DVT_OTHER       1
#define DVT_ETHER       2       /* Ethernet */
#define DVT_LOOP        3       /* Loop back interface. */
#define DVT_SLIP        4       /* Serial Line IP */
#define DVT_PPP         5       /* Point to Point Protocol */

/* The default routing metric to be used for this device. */
#define DEV_DEFAULT_METRIC    1

typedef struct _URT_INIT_STRUCT
{
    INT32       com_port;
    INT32       baud_rate;
    INT32       data_mode;
    INT32       parity;
    INT32       stop_bits;
    INT32       data_bits;
} URT_DEV;

typedef struct _ETHER_INIT_STRUCT
{
    UINT32      dv_irq;
    UINT32      dv_io_addr;
    UINT32      dv_shared_addr;
} ETHER_DEV;

struct _DEV_DEVICE
{
    CHAR        *dv_name;
    INT32       dv_flags;
    UINT32      dv_driver_options;
    STATUS      (*dv_init) (DV_DEVICE_ENTRY *);
    UINT8       dv_ip_addr[4];
    UINT8       dv_subnet_mask[4];
    UINT8       dv_gw[4];

    /* This union defines the hardware specific portion of the device
       initialization structure. */
    union _dv_hw
    {
        URT_DEV     uart;
        ETHER_DEV   ether;
    } dv_hw;

};

extern DV_DEVICE_LIST    DEV_Table;

/*  DEV.C Function Prototypes.  */
DV_DEVICE_ENTRY *DEV_Get_Dev_For_Vector( INT vector );
DV_DEVICE_ENTRY *DEV_Get_Dev_By_Name( CHAR *name );
DV_DEVICE_ENTRY *DEV_Get_Dev_By_Addr( UINT8 *addr );

STATUS  DEV_Init_Devices (DEV_DEVICE *devices, INT dev_count);
STATUS  DEV_Init_Route(DV_DEVICE_ENTRY *device);
INT     DEV_Get_Ether_Address(CHAR *name, UINT8 *ether_addr);
INT     DEV_Attach_IP_To_Device(CHAR *name, UINT8 *ip_addr, UINT8 *subnet);
INT     DEV_Detach_IP_From_Device(CHAR *name);
VOID    DEV_Recover_TX_Buffers (DV_DEVICE_ENTRY *);

#ifdef          __cplusplus
}
#endif /* _cplusplus */

#endif /* DEV_H */
