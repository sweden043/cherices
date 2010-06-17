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

/* Portions of this program were written by:       */
/***************************************************************************
*                                                                          *
*                                                                          *
*      NCSA Telnet                                                         *
*      by Tim Krauskopf, VT100 by Gaige Paulsen, Tek by Aaron Contorer     *
*                                                                          *
*      National Center for Supercomputing Applications                     *
*      152 Computing Applications Building                                 *
*      605 E. Springfield Ave.                                             *
*      Champaign, IL  61820                                                *
*                                                                          *
****************************************************************************/
/****************************************************************************
*                                                                            
* FILE NAME                                     VERSION                              
*                                                                                    
*   EXTERNS.H                                     4.4                                
*                                                                                    
* DESCRIPTION                                                                
*                                                                            
*   External definitions for functions in Nucleus NET.                       
*   This include file needs to go after other include files                  
*                                                                            
* DATA STRUCTURES                                                            
*                                                                            
*                                                                            
* DEPENDENCIES                                                               
*                                                                            
*      None                                                                  
*                                                                            
******************************************************************************/

#ifndef EXTERNS_H
#define EXTERNS_H

#include "tcpdefs.h"
#include "target.h"
#include "mem_defs.h"
#include "bootp.h"
#include "dns.h"
#include "dev.h"
#include "ip.h"
#include "socketd.h"
#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif /* _cplusplus */

/***** NET.C *****/

VOID    statcheck (VOID);
STATUS  NET_Add_Multi(DV_DEVICE_ENTRY *dev, DV_REQ *d_req);
STATUS  NET_Del_Multi(DV_DEVICE_ENTRY *dev, DV_REQ *d_req);
extern  NET_BUFFER_HEADER MEM_Buffer_List;
extern  NET_BUFFER_HEADER MEM_Buffer_Freelist;
extern  NU_MEMORY_POOL    *MEM_Non_Cached;

/***/

/***** BOOTP.C *****/

STATUS  NU_Bootp ( CHAR  *dv_name, BOOTP_STRUCT *bp_ptr);

/***/

/***** TOOLS.C *****/

UINT16  TLS_IP_Check (UINT16 *s, UINT16 len);
UINT32  TLS_Longswap (UINT32);
UINT16  TLS_Intswap (UINT16);
INT     TLS_Comparen (VOID *s1, VOID *s2, unsigned int len);
UINT16  TLS_IP_Check_Buffer_Chain (NET_BUFFER *buf_ptr);
UINT16  TLS_TCP_Check (UINT16 *, NET_BUFFER *);
STATUS  TLS_Put_Event (UINT16 event, UNSIGNED dat);
VOID   *TLS_Normalize_Ptr(VOID *);
void    TLS_Put32(unsigned char *, unsigned int, unsigned long);
void    TLS_Put16(unsigned char *, unsigned int, unsigned short);
void   *TLS_Put_String(unsigned char *dest, unsigned int offset,
                       unsigned char *src, unsigned int size);
void   *TLS_Get_String(unsigned char *src, unsigned int offset,
                       unsigned char *dest, unsigned int size);
int     TLS_Eq_String(unsigned char *packet, unsigned int offset,
                      unsigned char *local, unsigned int size);

unsigned long   TLS_Get32(unsigned char *, unsigned int);
unsigned short  TLS_Get16(unsigned char *, unsigned int);
unsigned long   TLS_IP_Address(unsigned char *p);

extern UINT32  TLS_Unused_Parameter;


/***/


/***** ARP.C *****/

VOID    ARP_Init (VOID);
#define NU_Rarp     ARP_Rarp

/***/

/***** UTIL.C *****/

STATUS  UTL_Timerset (UNSIGNED, UNSIGNED, UNSIGNED, INT32);
STATUS  UTL_Timerunset (UNSIGNED, UNSIGNED, INT32);
VOID    UTL_Zero(VOID *ptr, UINT32 size);
UINT16  UTL_Checksum (NET_BUFFER *, UINT32, UINT32, UINT8);
UINT32  UTL_Rand(VOID);
UINT16  UTL_Get_Unique_Port_Number(VOID);
INT     UTL_Is_Unique_Port_Number(UINT16, UINT16);

/***/

/***** DLL.C *****/

VOID    *DLL_Dequeue(VOID *h);
VOID    *DLL_Insert(VOID *h, VOID *i, VOID *l);
VOID    *DLL_Remove(VOID *h, VOID *i);
VOID    *DLL_Enqueue(VOID *h, VOID *i);

/***/

/***** MEM.C *****/

STATUS MEM_Init(VOID);
NET_BUFFER *MEM_Buffer_Dequeue(NET_BUFFER_HEADER *hdr);
NET_BUFFER *MEM_Buffer_Enqueue (NET_BUFFER_HEADER *hdr, NET_BUFFER *item);
NET_BUFFER *MEM_Buffer_Chain_Dequeue (NET_BUFFER_HEADER *header, INT32 nbytes);
NET_BUFFER *MEM_Update_Buffer_Lists (NET_BUFFER_HEADER *source, 
                                     NET_BUFFER_HEADER *dest);
NET_BUFFER *MEM_Buffer_Insert(NET_BUFFER_HEADER *, NET_BUFFER *, 
                              NET_BUFFER*, NET_BUFFER *);
VOID    MEM_Buffer_Chain_Free (NET_BUFFER_HEADER *source, 
                             NET_BUFFER_HEADER *dest);
VOID    MEM_One_Buffer_Chain_Free (NET_BUFFER *source, NET_BUFFER_HEADER *dest);
VOID    MEM_Cat (NET_BUFFER *dest, NET_BUFFER *src);
VOID    MEM_Trim (NET_BUFFER *buf_ptr, INT32 length);
VOID    MEM_Buffer_Remove (NET_BUFFER_HEADER *hdr, NET_BUFFER *item);
VOID    MEM_Buffer_Cleanup (NET_BUFFER_HEADER *hdr);
VOID    MEM_Chain_Copy(NET_BUFFER *dest, NET_BUFFER *src, INT32 off, INT32 len);
VOID    MEM_Buffer_Suspension_HISR (VOID);

/***/

/***** PROTINIT.C *****/

STATUS  PROT_Net_Init (VOID);

/***/

/***** SOCKETS.C *****/

#if (NET_VERSION_COMP >= NET_4_2)
STATUS  NU_Init_Net(NU_MEMORY_POOL *);
#else
STATUS  NU_Init_Net(VOID);
#endif

STATUS  NU_Socket (INT16, INT16, INT16);
STATUS  NU_Bind (INT, struct addr_struct *, INT16);
STATUS  NU_Connect (INT, struct addr_struct *, INT16);
STATUS  NU_Listen (INT, UINT16);
STATUS  NU_Accept (INT, struct addr_struct *, INT16 *);
INT32   NU_Send (INT, CHAR *, UINT16, INT16);
INT32   NU_Send_To (INT, CHAR *, UINT16, INT16, struct addr_struct *, INT16);
STATUS  NU_Send_To_Raw(INT socketd, CHAR *buff, UINT16 nbytes, INT16 flags,
                      struct addr_struct *to, INT16 addrlen);
INT32   NU_Recv (INT, CHAR *, UINT16, INT16);
INT32   NU_Recv_From (INT, CHAR *, UINT16, INT16, struct addr_struct *, INT16 *);
STATUS  NU_Recv_From_Raw (INT, CHAR *, INT16, INT16, struct addr_struct *, INT16 *);
STATUS  NU_Push (INT);
STATUS  NU_Is_Connected (INT);
INT16   NU_Get_UDP_Pnum (struct sock_struct *);
STATUS  NU_Fcntl (INT socketd, INT16 command, INT16 arguement);

/* This definition was added to maintain compatability with older versions of
   Nucleus NET. */
#define NU_fcntl(p1, p2, p3, p4)      NU_Fcntl(p1, p2, p3)

STATUS  NU_Get_Host_By_Name(CHAR *name, NU_HOSTENT *host_entry);
STATUS  NU_Get_Host_By_Addr(CHAR *, INT, INT, NU_HOSTENT *);
STATUS  NU_Get_Peer_Name(INT, struct sockaddr_struct *, INT16 *);
STATUS  NU_Get_Sock_Name(INT, struct sockaddr_struct *, INT16 *);
STATUS  NU_Close_Socket (INT);
VOID    SCK_Suspend_Task(NU_TASK *);
INT     SCK_SearchTaskList(struct TASK_TABLE_STRUCT *, INT16, INT);
STATUS  SCK_TaskTable_Entry_Delete(INT);

STATUS  NU_Socket_Connected(INT);
STATUS  NU_Setsockopt(INT, INT, INT, VOID *, INT);
STATUS  NU_Getsockopt(INT, INT, INT, VOID *, INT *);
STATUS  NU_Abort(INT socketd);
INT     SCK_Create_Socket(INT protocol);
STATUS  NU_Ioctl(INT optname, SCK_IOCTL_OPTION *option, INT optlen);
STATUS  NU_Add_Route(UINT8 *dest, UINT8* mask, UINT8 *gw_addr);
VOID    SCK_Kill_All_Open_Sockets (DV_DEVICE_ENTRY *dev_ptr);
INT     SCK_Get_Host_Name (CHAR *name, INT name_length);


/***/

/***** SELECT.C *****/

STATUS  NU_Select(INT, FD_SET *, FD_SET *, FD_SET *, UNSIGNED);
INT     NU_FD_Check(INT, FD_SET *);
VOID    NU_FD_Set(INT, FD_SET *);
VOID    NU_FD_Init(FD_SET *fd);
VOID    NU_FD_Reset(INT, FD_SET *);
STATUS  SEL_Check_Recv(INT, FD_SET *);
INT     SEL_Setup_Recv_Ports(INT, FD_SET *, NU_TASK *);
STATUS  SEL_Enable_Timeout(INT16, UNSIGNED);
STATUS  SEL_Clear_Timeout(INT16);

/***/

/***** ICMP.C *****/

VOID    ICMP_Init(VOID);
//STATUS  ICMP_Send_Echo_Request(UINT8 *, UINT32); mod by sunbe for large ping packet
STATUS  ICMP_Send_Echo_Request(UINT8 *,UINT16, UINT32);
#define NU_Ping ICMP_Send_Echo_Request

/***/

/***** RTAB.C *****/

#define NU_Delete_Route RTAB_Delete_Route 

/***/

#ifdef          __cplusplus
}
#endif /* _cplusplus */

#endif  /* EXTERNS_H */
