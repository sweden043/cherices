/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*              Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002      */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       nal.h
 *
 *
 * Description:    Network Abstraction Layer for Conexant MPEG/IRD Software
 *
 *
 * Author:         Senthil Veluswamy
 *
 ****************************************************************************/
/* $Header: nal.h, 3, 7/16/02 1:59:36 PM, $
 ****************************************************************************/

#ifndef _NAL_H_
#define _NAL_H_

/*****************/
/* Include Files */
/*****************/

/***********/
/* Aliases */
/***********/

#define TRACE_NAL                      TRACE_KAL

#define CNXT_NAL_INTERFACE_ADDRESS_ANY 0

#define CNXT_NAL_PORT_NUMBER_ANY       0

/* Socket integrity Macros */
#define CNXT_NAL_MAGIC_NUMBER          0x74090000
#define RTOS_TO_CNXT_NAL_SD(x, y)      (x | CNXT_NAL_MAGIC_NUMBER | (y << 20))
#define CNXT_NAL_TO_RTOS_SD(x)         (x & 0xFFFF)
#define IS_VALID_CNXT_NAL_SD(x)        ((x & 0xFF0F0000) == \
                                             CNXT_NAL_MAGIC_NUMBER) ? \
                                                TRUE : FALSE
#define IS_CNXT_NAL_STREAM_SOCKET(x)   (((x & 0x00F00000) >> 20) == \
                                             CNXT_NAL_SOCKET_TYPE_STREAM) ? \
                                                TRUE : FALSE

#define IS_CNXT_NAL_DATAGRAM_SOCKET(x) (((x & 0x00F00000) >> 20) == \
                                             CNXT_NAL_SOCKET_TYPE_DATAGRAM) ? \
                                                TRUE : FALSE

#define IS_CNXT_NAL_RAW_SOCKET(x)      (((x & 0x00F00000) >> 20) == \
                                             CNXT_NAL_SOCKET_TYPE_RAW) ? \
                                                TRUE : FALSE

/* Socket layer defines */
typedef enum _CNXT_NAL_ADDRESS_FAMILY
{
   CNXT_NAL_ADDRESS_FAMILY_INET
} CNXT_NAL_ADDRESS_FAMILY;

typedef enum _CNXT_NAL_SOCKET_TYPE
{
   CNXT_NAL_SOCKET_TYPE_STREAM = 0xA,
   CNXT_NAL_SOCKET_TYPE_DATAGRAM,
   CNXT_NAL_SOCKET_TYPE_RAW
} CNXT_NAL_SOCKET_TYPE;

typedef enum _CNXT_NAL_COMMUNICATION_PROTOCOL
{
   CNXT_NAL_COMMUNICATION_PROTOCOL_DEFAULT = 0,
   CNXT_NAL_COMMUNICATION_PROTOCOL_TCP,
   CNXT_NAL_COMMUNICATION_PROTOCOL_UDP,
   CNXT_NAL_COMMUNICATION_PROTOCOL_RAW
} CNXT_NAL_COMMUNICATION_PROTOCOL;

typedef long CNXT_NAL_SOCKET_DESCRIPTOR;

typedef struct _CNXT_NAL_SOCKET_ADDRESS
{
   short          addr_family;            /* One of CNXT_NAL_ADDRESS_FAMILY */
   unsigned short network_port;           /* Unique Service Port            */
   unsigned long  network_address;        /* Network byte ordered address   */
   char           reserved[8];            /* Unused, but generally set to 0 */
} CNXT_NAL_SOCKET_ADDRESS;

// DIFF
//typedef enum _CNXT_NAL_SOCKET_RW_MSG_FLAG
//{
//   CNXT_NAL_SOCKET_RW_MSG_FLAG_OOB,
//   CNXT_NAL_SOCKET_RW_MSG_FLAGS_PEEK,
//   CNXT_NAL_SOCKET_RW_MSG_FLAGS_DONTROUTE
//} CNXT_NAL_SOCKET_RW_MSG_FLAG;

typedef int CNXT_NAL_SOCKET_RW_MSG_FLAG;

typedef enum _CNXT_NAL_SOCKET_OPTION_LEVEL
{
   CNXT_NAL_SOCKET_OPTION_LEVEL_IP,
   CNXT_NAL_SOCKET_OPTION_LEVEL_TCP,
   CNXT_NAL_SOCKET_OPTION_LEVEL_SOCKET
} CNXT_NAL_SOCKET_OPTION_LEVEL;

typedef enum _CNXT_NAL_SOCKET_OPTION_NAME
{
// DIFF
   CNXT_NAL_SOCKET_OPTION_NAME_IP_HDRINCL,
//   CNXT_NAL_SOCKET_OPTION_NAME_IP_TOS,
//   CNXT_NAL_SOCKET_OPTION_NAME_IP_TTL,
//   CNXT_NAL_SOCKET_OPTION_NAME_IP_RECVOPTS,
//   CNXT_NAL_SOCKET_OPTION_NAME_IP_RECVRETOPTS,
//   CNXT_NAL_SOCKET_OPTION_NAME_IP_RECVDSTADDR,
//   CNXT_NAL_SOCKET_OPTION_NAME_IP_RETOPTS,
   CNXT_NAL_SOCKET_OPTION_NAME_IP_MCAST_ADD,
   CNXT_NAL_SOCKET_OPTION_NAME_IP_MCAST_DROP,
//   CNXT_NAL_SOCKET_OPTION_NAME_IP_MCAST_IF,
   CNXT_NAL_SOCKET_OPTION_NAME_IP_MCAST_TTL,
//   CNXT_NAL_SOCKET_OPTION_NAME_IP_MCAST_LOOP,
//   CNXT_NAL_SOCKET_OPTION_NAME_TCP_MAXSEG,
   CNXT_NAL_SOCKET_OPTION_NAME_TCP_NODELAY,
   CNXT_NAL_SOCKET_OPTION_NAME_SOCKET_BROADCAST,
//   CNXT_NAL_SOCKET_OPTION_NAME_SOCKET_DEBUG,
//   CNXT_NAL_SOCKET_OPTION_NAME_SOCKET_DONTROUTE,
//   CNXT_NAL_SOCKET_OPTION_NAME_SOCKET_ERROR,
//   CNXT_NAL_SOCKET_OPTION_NAME_SOCKET_KEEPALIVE,
//   CNXT_NAL_SOCKET_OPTION_NAME_SOCKET_LINGER,
//   CNXT_NAL_SOCKET_OPTION_NAME_SOCKET_OOBINLINE,
   CNXT_NAL_SOCKET_OPTION_NAME_SOCKET_RCVBUF,
   CNXT_NAL_SOCKET_OPTION_NAME_SOCKET_SNDBUF,
//   CNXT_NAL_SOCKET_OPTION_NAME_SOCKET_RCVLOWAT,
//   CNXT_NAL_SOCKET_OPTION_NAME_SOCKET_SNDLOWAT,
//   CNXT_NAL_SOCKET_OPTION_NAME_SOCKET_RCVTIMEO,
//   CNXT_NAL_SOCKET_OPTION_NAME_SOCKET_SNDTIMEO,
//   CNXT_NAL_SOCKET_OPTION_NAME_SOCKET_REUSEADDR,
//   CNXT_NAL_SOCKET_OPTION_NAME_SOCKET_TYPE,
//   CNXT_NAL_SOCKET_OPTION_NAME_SOCKET_USELOOPBACK
} CNXT_NAL_SOCKET_OPTION_NAME;

typedef struct _CNXT_NAL_IP_MCAST_REQ
{
   unsigned long mcast_group_addr;
   unsigned long mcast_interface_addr;
} CNXT_NAL_IP_MCAST_REQ;

typedef unsigned long CNXT_NAL_MCAST_IFADDR;

// DIFF
//typedef struct _CNXT_NAL_SOCKET_LINGER
//{
//   int linger_onoff;
//   int linger_time;
//} CNXT_NAL_SOCKET_LINGER;

typedef enum _CNXT_NAL_SOCKET_FNCTL_CMD
{
// DIFF
//   CNXT_NAL_SOCKET_FNCTL_CMD_F_GETOWN,
//   CNXT_NAL_SOCKET_FNCTL_CMD_F_SETOWN,
//   CNXT_NAL_SOCKET_FNCTL_CMD_F_GETFL,
   CNXT_NAL_SOCKET_FNCTL_CMD_F_SETFL
} CNXT_NAL_SOCKET_FNCTL_CMD;

typedef enum _CNXT_NAL_SOCKET_FNCTL_CMD_ARG
{
// DIFF
//   CNXT_NAL_SOCKET_FNCTL_CMD_ARG_FLAG_APPEND,
//   CNXT_NAL_SOCKET_FNCTL_CMD_ARG_FLAG_ASYNC,
//   CNXT_NAL_SOCKET_FNCTL_CMD_ARG_FLAG_CREAT,
//   CNXT_NAL_SOCKET_FNCTL_CMD_ARG_FLAG_EXCL,
   CNXT_NAL_SOCKET_FNCTL_CMD_ARG_FLAG_NDELAY//,
//   CNXT_NAL_SOCKET_FNCTL_CMD_ARG_FLAG_TRUNC
} CNXT_NAL_SOCKET_FNCTL_CMD_ARG;

typedef enum _CNXT_NAL_SOCKET_IOCTL_REQUEST
{
/*
   CNXT_NAL_SOCKET_IOCTL_REQUEST_FIOCLEX,
   CNXT_NAL_SOCKET_IOCTL_REQUEST_FIONCLEX,
*/
   CNXT_NAL_SOCKET_IOCTL_REQUEST_FIONBIO,
/*
   CNXT_NAL_SOCKET_IOCTL_REQUEST_FIOASYNC,
   CNXT_NAL_SOCKET_IOCTL_REQUEST_FIONREAD,
   CNXT_NAL_SOCKET_IOCTL_REQUEST_FIOSETOWN,
   CNXT_NAL_SOCKET_IOCTL_REQUEST_FIOGETOWN,
   CNXT_NAL_SOCKET_IOCTL_REQUEST_SIOCSHIWAT,
   CNXT_NAL_SOCKET_IOCTL_REQUEST_SIOCGHIWAT,
   CNXT_NAL_SOCKET_IOCTL_REQUEST_SIOCSLOWAT,
   CNXT_NAL_SOCKET_IOCTL_REQUEST_SIOCGLOWAT,
   CNXT_NAL_SOCKET_IOCTL_REQUEST_SIOCATMARK,
   CNXT_NAL_SOCKET_IOCTL_REQUEST_SIOCSPGRP,
   CNXT_NAL_SOCKET_IOCTL_REQUEST_SIOCGPGRP,
   CNXT_NAL_SOCKET_IOCTL_REQUEST_SIOCADDRT,
   CNXT_NAL_SOCKET_IOCTL_REQUEST_SIOCDELRT,
   CNXT_NAL_SOCKET_IOCTL_REQUEST_SIOCSIFADDR,
*/
   CNXT_NAL_SOCKET_IOCTL_REQUEST_SIOCGIFADDR,
/*
   CNXT_NAL_SOCKET_IOCTL_REQUEST_SIOCSIFFLAGS,
   CNXT_NAL_SOCKET_IOCTL_REQUEST_SIOCGIFFLAGS,
   CNXT_NAL_SOCKET_IOCTL_REQUEST_SIOCGIFCONF,
   CNXT_NAL_SOCKET_IOCTL_REQUEST_SIOCSIFDSTADDR,
*/
   CNXT_NAL_SOCKET_IOCTL_REQUEST_SIOCGIFDSTADDR,
/*
   CNXT_NAL_SOCKET_IOCTL_REQUEST_SIOCGIFBRDADDR,
   CNXT_NAL_SOCKET_IOCTL_REQUEST_SIOCSIFBRDADDR,
   CNXT_NAL_SOCKET_IOCTL_REQUEST_SIOCGIFNETMASK,
   CNXT_NAL_SOCKET_IOCTL_REQUEST_SIOCSIFNETMASK,
   CNXT_NAL_SOCKET_IOCTL_REQUEST_SIOCGIFMETRIC,
   CNXT_NAL_SOCKET_IOCTL_REQUEST_SIOCSIFMETRIC,
   CNXT_NAL_SOCKET_IOCTL_REQUEST_SIOCSARP,
   CNXT_NAL_SOCKET_IOCTL_REQUEST_SIOCGARP,
   CNXT_NAL_SOCKET_IOCTL_REQUEST_SIOCDARP
*/
} CNXT_NAL_SOCKET_IOCTL_REQUEST;

typedef struct _CNXT_NAL_SOCKET_IOCTL_IF_REQUEST
{
#define CNXT_NAL_SOCKET_IOCTL_IF_REQUEST_NAMESIZE 16
   union {
      char *pname_buf;
      char name[CNXT_NAL_SOCKET_IOCTL_IF_REQUEST_NAMESIZE];
   } if_dev_name;
   unsigned long if_dev_addr;
} CNXT_NAL_SOCKET_IOCTL_IF_REQUEST;

// DIFF
//typedef void* pCNXT_NAL_IOVEC;

// DIFF
//typedef enum _CNXT_NAL_SOCKET_SHUTDOWN_OPTION
//{
//   CNXT_NAL_SOCKET_SHUTDOWN_OPTION_SEND,
//   CNXT_NAL_SOCKET_SHUTDOWN_OPTION_RECEIVE,
//   CNXT_NAL_SOCKET_SHUTDOWN_OPTION_BOTH
//} CNXT_NAL_SOCKET_SHUTDOWN_OPTION;

typedef struct _CNXT_NAL_TIMEVAL
{
   unsigned long timeval_sec;
   long timeval_microsec;
} CNXT_NAL_TIMEVAL;

typedef enum _CNXT_NAL_STATUS
{
   CNXT_NAL_STATUS_OK = 0,
   CNXT_NAL_STATUS_NOT_SUPPORTED,
   CNXT_NAL_STATUS_INVALID,
   CNXT_NAL_STATUS_BADPTR,
   CNXT_NAL_STATUS_SYS_ERROR,
   CNXT_NAL_STATUS_UNKNOWN_ERROR
} CNXT_NAL_STATUS;

// DIFF
//typedef void* pCNXT_NAL_MSGHDR;

typedef unsigned long *pCNXT_NAL_FDSET;
#define CNXT_NAL_FD_SETSIZE FD_SETSIZE

/***********************/
/* Function Prototypes */
/***********************/

/* Socket layer routines */
CNXT_NAL_STATUS cnxt_nal_socket_create(CNXT_NAL_ADDRESS_FAMILY addr_family, 
                                       CNXT_NAL_SOCKET_TYPE sock_type, 
                                       CNXT_NAL_COMMUNICATION_PROTOCOL comm_protocol,
                                       CNXT_NAL_SOCKET_DESCRIPTOR *psock_desc);

CNXT_NAL_STATUS cnxt_nal_socket_bind(CNXT_NAL_SOCKET_DESCRIPTOR sock_desc, 
                                     CNXT_NAL_SOCKET_ADDRESS *plocal_addr,
                                     int local_addr_len);

CNXT_NAL_STATUS cnxt_nal_socket_connect(CNXT_NAL_SOCKET_DESCRIPTOR sock_desc,
                                        CNXT_NAL_SOCKET_ADDRESS *pserver_addr,
                                        int server_addr_len);
// DIFF
//                                        ,
//                                        CNXT_NAL_TIMEVAL *ptime_out);

CNXT_NAL_STATUS cnxt_nal_socket_listen(CNXT_NAL_SOCKET_DESCRIPTOR sock_desc,
                                       int listen_queue_length);

CNXT_NAL_STATUS cnxt_nal_socket_accept(CNXT_NAL_SOCKET_DESCRIPTOR parent_sock_desc,
                                      CNXT_NAL_SOCKET_ADDRESS *ppeer_addr,
                                      int *ppeer_addr_len,
                                      CNXT_NAL_SOCKET_DESCRIPTOR *pchild_sock_desc);

CNXT_NAL_STATUS cnxt_nal_socket_send(CNXT_NAL_SOCKET_DESCRIPTOR sock_desc,
                                     char *pbuf,
                                     int *pnum_bytes,
                                     CNXT_NAL_SOCKET_RW_MSG_FLAG flags);

CNXT_NAL_STATUS cnxt_nal_socket_send_to(CNXT_NAL_SOCKET_DESCRIPTOR sock_desc,
                                        char *pbuf,
                                        int *pnum_bytes,
                                        CNXT_NAL_SOCKET_RW_MSG_FLAG flags,
                                        CNXT_NAL_SOCKET_ADDRESS *ppeer_addr,
                                        int peer_addr_len);

CNXT_NAL_STATUS cnxt_nal_socket_receive(CNXT_NAL_SOCKET_DESCRIPTOR sock_desc,
                                        char *pbuf,
                                        int *pnum_bytes,
                                        CNXT_NAL_SOCKET_RW_MSG_FLAG flags);

CNXT_NAL_STATUS cnxt_nal_socket_receive_from(CNXT_NAL_SOCKET_DESCRIPTOR sock_desc,
                                             char *pbuf,
                                             int *pnum_bytes,
                                             CNXT_NAL_SOCKET_RW_MSG_FLAG flags,
                                             CNXT_NAL_SOCKET_ADDRESS *ppeer_addr,
                                             int *ppeer_addr_len);

// SKV - want to do this call? 
//CNXT_NAL_STATUS cxnt_nal_get_host_name(char *phost_name, 
//                                       int host_name_length);

CNXT_NAL_STATUS cnxt_nal_socket_get_peer_name(CNXT_NAL_SOCKET_DESCRIPTOR sock_desc,
                                              CNXT_NAL_SOCKET_ADDRESS *ppeer_addr,
                                              int *ppeer_addr_len);

CNXT_NAL_STATUS cnxt_nal_socket_get_my_name(CNXT_NAL_SOCKET_DESCRIPTOR sock_desc,
                                            CNXT_NAL_SOCKET_ADDRESS *plocal_addr,
                                            int *plocal_addr_len);

CNXT_NAL_STATUS cnxt_nal_socket_get_opt(CNXT_NAL_SOCKET_DESCRIPTOR sock_desc,
                                        CNXT_NAL_SOCKET_OPTION_LEVEL option_level,
                                        CNXT_NAL_SOCKET_OPTION_NAME option_name,
                                        int *poption_value,
                                        int *poption_length);

CNXT_NAL_STATUS cnxt_nal_socket_set_opt(CNXT_NAL_SOCKET_DESCRIPTOR sock_desc,
                                        CNXT_NAL_SOCKET_OPTION_LEVEL option_level,
                                        CNXT_NAL_SOCKET_OPTION_NAME option_name,
                                        int *poption_value,
                                        int option_length);

// DIFF
//CNXT_NAL_STATUS cnxt_nal_socket_shutdown(CNXT_NAL_SOCKET_DESCRIPTOR sock_desc,
//                                         CNXT_NAL_SOCKET_SHUTDOWN_OPTION shutdown_opt);

/* Socket File I/O Style calls */
CNXT_NAL_STATUS cnxt_nal_socket_close(CNXT_NAL_SOCKET_DESCRIPTOR sock_desc);

CNXT_NAL_STATUS cnxt_nal_socket_read(CNXT_NAL_SOCKET_DESCRIPTOR sock_desc,
                                     char *pbuf,
                                     unsigned int *pnum_bytes);

CNXT_NAL_STATUS cnxt_nal_socket_write(CNXT_NAL_SOCKET_DESCRIPTOR sock_desc,
                                      char *pbuf,
                                      unsigned int *pnum_bytes);

// DIFF
//CNXT_NAL_STATUS cnxt_nal_socket_readv(CNXT_NAL_SOCKET_DESCRIPTOR sock_desc,
//                                     pCNXT_NAL_IOVEC soc_iov,
//                                     int iov_count);
//
//CNXT_NAL_STATUS cnxt_nal_socket_writev(CNXT_NAL_SOCKET_DESCRIPTOR sock_desc,
//                                      pCNXT_NAL_IOVEC soc_iov,
//                                      int iov_count);
//
//CNXT_NAL_STATUS cnxt_nal_socket_read_mesg(CNXT_NAL_SOCKET_DESCRIPTOR sock_desc,
//                                          pCNXT_NAL_MSGHDR pmsg_hdr,
//                                          int flags);
//
//CNXT_NAL_STATUS cnxt_nal_socket_write_mesg(CNXT_NAL_SOCKET_DESCRIPTOR sock_desc,
//                                           pCNXT_NAL_MSGHDR pmsg_hdr,
//                                           int flags);

CNXT_NAL_STATUS cnxt_nal_socket_fcntl(CNXT_NAL_SOCKET_DESCRIPTOR sock_desc,
                                      CNXT_NAL_SOCKET_FNCTL_CMD cmd,
                                      CNXT_NAL_SOCKET_FNCTL_CMD_ARG arg);

CNXT_NAL_STATUS cnxt_nal_socket_ioctl(CNXT_NAL_SOCKET_DESCRIPTOR sock_desc,
                                      CNXT_NAL_SOCKET_IOCTL_REQUEST request,
                                      char *parg);

/* I/O Multiplexing */
CNXT_NAL_STATUS cnxt_nal_socket_select(int maxfdpl,
                                       pCNXT_NAL_FDSET pread_fds,
                                       pCNXT_NAL_FDSET pwrite_fds,
                                       pCNXT_NAL_FDSET pexcept_fds,
                                       CNXT_NAL_TIMEVAL *ptimeout_value,
                                       int *pnum_fds_active);

CNXT_NAL_STATUS cnxt_nal_fd_create(pCNXT_NAL_FDSET *ppfd_set);

CNXT_NAL_STATUS cnxt_nal_fd_init(pCNXT_NAL_FDSET pfd_set);

CNXT_NAL_STATUS cnxt_nal_fd_set(CNXT_NAL_SOCKET_DESCRIPTOR sd, 
                                pCNXT_NAL_FDSET pfd_set);

CNXT_NAL_STATUS cnxt_nal_fd_clear(CNXT_NAL_SOCKET_DESCRIPTOR sd, 
                                  pCNXT_NAL_FDSET pfd_set);

CNXT_NAL_STATUS cnxt_nal_fd_query(CNXT_NAL_SOCKET_DESCRIPTOR sd, 
                                  pCNXT_NAL_FDSET pfd_set, 
                                  int *pfd_value);

/* Byte ordering Routines */
CNXT_NAL_STATUS cnxt_nal_host_to_net_long(unsigned long host_long, 
                                          unsigned long *pnet_long);

CNXT_NAL_STATUS cnxt_nal_host_to_net_short(unsigned short host_short, 
                                           unsigned short *pnet_short);

CNXT_NAL_STATUS cnxt_nal_net_to_host_long(unsigned long net_long, 
                                          unsigned long *phost_long);

CNXT_NAL_STATUS cnxt_nal_net_to_host_short(unsigned short net_short, 
                                           unsigned short *phost_short);

/* Address Converstion Routines */
CNXT_NAL_STATUS cnxt_nal_str_to_inet4_addr(char          *pstr_addr,
                                           unsigned long *pinet_addr);

CNXT_NAL_STATUS cnxt_nal_inet4_to_str_addr(unsigned long *pinet_addr,
                                           char          *pstr_addr);

#endif /* _NAL_H_ */

/****************************************************************************
 * Modifications:
 * $Log: 
 *  3    mpeg      1.2         7/16/02 1:59:36 PM     Senthil Veluswamy SCR(s) 
 *        3942 :
 *        1) Added parameter to cnxt_nal_socket_select() to be able to return 
 *        number of active FDs.
 *        2) Changed secs member of time structure to be unsigned long
 *        3) Added define for CNXT_NAL_FD_SETSIZE
 *        4) Added "p" to some formal argument names to clarify that their were
 *         pointer types
 *        
 *  2    mpeg      1.1         12/12/01 12:06:52 PM   Senthil Veluswamy SCR(s) 
 *        2817 :
 *        Changed Socket read/write to take 3 parameters.
 *        Using VxW Socket Option SOCKET SENDBUF/RCVBUF to enable getting 
 *        receiving packet  sizes upto 64K
 *        
 *  1    mpeg      1.0         11/6/01 3:12:28 PM     Senthil Veluswamy 
 * $
 * 
 *    Rev 1.2   16 Jul 2002 12:59:36   velusws
 * SCR(s) 3942 :
 * 1) Added parameter to cnxt_nal_socket_select() to be able to return number of active FDs.
 * 2) Changed secs member of time structure to be unsigned long
 * 3) Added define for CNXT_NAL_FD_SETSIZE
 * 4) Added "p" to some formal argument names to clarify that their were pointer types
 * 
 *    Rev 1.1   12 Dec 2001 12:06:52   velusws
 * SCR(s) 2817 :
 * Changed Socket read/write to take 3 parameters.
 * Using VxW Socket Option SOCKET SENDBUF/RCVBUF to enable getting receiving packet  sizes upto 64K
 * 
 *    Rev 1.0   06 Nov 2001 15:12:28   velusws
 * SCR(s) 2817 :
 * 
 *
 ****************************************************************************/
