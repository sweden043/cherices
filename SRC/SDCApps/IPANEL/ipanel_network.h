/*********************************************************************
    Copyright (c) 2008 - 2010 Embedded Internet Solutions, Inc
    All rights reserved. You are not allowed to copy or distribute
    the code without permission.
    This is the demo implenment of the base Porting APIs needed by iPanel MiddleWare. 
    Maybe you should modify it accorrding to Platform.
    
    Note: the "int" in the file is 32bits
    
    History:
		version		date		name		desc
         0.01     2009/8/1     Vicegod     create
*********************************************************************/
#ifndef _IPANEL_MIDDLEWARE_PORTING_NETWORK_API_FUNCTOTYPE_H_
#define _IPANEL_MIDDLEWARE_PORTING_NETWORK_API_FUNCTOTYPE_H_

#include "ipanel_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    IPANEL_NETWORK_DEVICE_LAN               = 1,
    IPANEL_NETWORK_DEVICE_DIALUP            = 2,
    IPANEL_NETWORK_DEVICE_PPPOE             = 3,
    IPANEL_NETWORK_DEVICE_CABLEMODEM        = 4
} IPANEL_NETWORK_DEVICE_e;

typedef enum
{
    IPANEL_NETWORK_CONNECT                  = 1,
    IPANEL_NETWORK_DISCONNECT               = 2,
    IPANEL_NETWORK_SET_IFCONFIG             = 3,
    IPANEL_NETWORK_GET_IFCONFIG             = 4,
    IPANEL_NETWORK_GET_STATUS               = 5,
    IPANEL_NETWORK_SET_STREAM_HOOK          = 6,
    IPANEL_NETWORK_DEL_STREAM_HOOK          = 7,
    IPANEL_NETWORK_SET_USER_NAME            = 8,
    IPANEL_NETWORK_SET_PWD                  = 9,
    IPANEL_NETWORK_SET_DIALSTRING           = 10,
    IPANEL_NETWORK_SET_DNS_CONFIG           = 11,
    IPANEL_NETWORK_GET_DNS_CONFIG           = 12,
    IPANEL_NETWORK_SET_NIC_MODE             = 13,
    IPANEL_NETWORK_GET_NIC_MODE             = 14,
    IPANEL_NETWORK_SET_NIC_BUFFER_SIZE      = 15,
    IPANEL_NETWORK_GET_NIC_BUFFER_SIZE      = 16,
    IPANEL_NETWORK_RENEW_IP                 = 17,
    IPANEL_NETWORK_GET_MAC                  = 18,
    IPANEL_NETWORK_GET_NIC_SEND_PACKETS     = 19,
    IPANEL_NETWORK_GET_NIC_REVD_PACKETS     = 20,
    IPANEL_NETWORK_SEND_PING_REQ            = 21,
    IPANEL_NETWORK_STOP_PING_REQ            = 22
} IPANEL_NETWORK_IOCTL_e;

typedef enum
{
    IPANEL_NETWORK_DNS_FROM_SERVER          = 1,
    IPANEL_NETWORK_DNS_FROM_USER            = 2
} IPANEL_NETWORK_DNS_MODE_e;

typedef enum
{
    IPANEL_NETWORK_LAN_ASSIGN_IP            = 1,
    IPANEL_NETWORK_LAN_DHCP                 = 2
} IPANEL_NETWORK_LAN_MODE_e;

typedef struct
{
    UINT32_T    ipaddr;
    UINT32_T    netmask;
    UINT32_T    gateway;
} IPANEL_NETWORK_IF_PARAM;

typedef enum
{
    IPANEL_NETWORK_IF_CONNECTED             = 1,
    IPANEL_NETWORK_IF_CONNECTING            = 2,
    IPANEL_NETWORK_IF_DISCONNECTED          = 3
} IPANEL_NETWORK_IF_STATUS_e;


typedef enum
{
    IPANEL_IP_NETWORK_CONNECT              = 5500,
    IPANEL_IP_NETWORK_DISCONNECT,
    IPANEL_IP_NETWORK_READY,               
    IPANEL_IP_NETWORK_FAILED, 
    IPANEL_IP_NETWORK_SENT_PACKAGE         = 5520,
    IPANEL_IP_NETWORK_RECEIVED_PACKAGE     = 5521,
    IPANEL_IP_NETWORK_PING_RESPONSE        = 5530,

    IPANEL_CABLE_NETWORK_CONNECT           = 5550,
    IPANEL_CABLE_NETWORK_DISCONNECT,

    IPANEL_NETWORK_UNDEFINED
}IPANEL_NETWORK_STATUS;

typedef enum
{
    IPANEL_NETWORK_NIC_CONFIG_UNKNOW        = 0,
    IPANEL_NETWORK_AUTO_CONFIG              = 1,
    IPANEL_NETWORK_10BASE_HALF_DUMPLEX      = 2,
    IPANEL_NETWORK_10BASE_FULL_DUMPLEX      = 3,
    IPANEL_NETWORK_100BASE_HALF_DUMPLEX     = 4,
    IPANEL_NETWORK_100BASE_FULL_DUMPLEX     = 5
} IPANEL_NETWORK_NIC_MODE_e;

typedef VOID (*IPANEL_NETWORK_STREAM_HOOK)(BYTE_T *buf, INT32_T len );

//--------------------------------------------------------------------------------------------------
//  EXTERN DATA DEFINITION
//--------------------------------------------------------------------------------------------------
//


//--------------------------------------------------------------------------------------------------
//  EXTERN FUNCTION PROTOTYPES
//--------------------------------------------------------------------------------------------------
//
INT32_T ipanel_porting_network_ioctl(
        IPANEL_NETWORK_DEVICE_e     device,
        IPANEL_NETWORK_IOCTL_e      op,
        VOID                       *arg
    );

void INTTOCHAR(unsigned int IPAddr,  unsigned char *ipStr);

unsigned int CHARTOINT(unsigned char *ipAddr);

void ipanel_ping_ip(u_int32 uip, u_int32 len );

void ipanel_network_dhcpget();

void ipanel_network_ipset();

INT32_T ipanel_network_init();

void ipanel_network_exit();

#ifdef __cplusplus
}
#endif

#endif  // _IPANEL_MIDDLEWARE_PORTING_NETWORK_API_FUNCTOTYPE_H_

