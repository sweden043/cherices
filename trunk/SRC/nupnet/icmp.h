/***********************************************************************
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

/***********************************************************************
*                                                                       
* FILE NAME                                     VERSION                         
*                                                                               
*      ICMP.H                                     4.4                           
*                                                                               
* COMPONENT                                                             
*                                                                       
*      ICMP - Internet Control Message Protocol                         
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*      Holds the defines for the ICMP protocol.                         
*                                                                       
* DATA STRUCTURES                                                       
*                                                                       
*      ICMP_LAYER                                                       
*      ICMP_ECHO_LIST_ENTRY                                             
*      ICMP_ECHO_LIST                                                   
*                                                                       
* DEPENDENCIES                                                          
*                                                                       
*      No other file dependencies                                       
*                                                                       
*************************************************************************/

#ifndef ICMP_H
#define ICMP_H

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif /* _cplusplus */

typedef struct ICMP_LAYER_STRUCT
{
    UINT8   icmp_type;                  /* Type of ICMP message. */
    UINT8   icmp_code;                  /* Sub code. */
    UINT16  icmp_cksum;                 /* Ones complement check sum .*/

    union {                             /* ICMP header union. */
        UINT8   ih_pptr;

        UINT32  ih_gwaddr;

        struct  ih_idseq {
            INT16   icd_id;
            INT16   icd_seq;
        } ih_idseq;

        INT32   ih_void;

        struct ih_pmtu {
            INT16   ipm_void;
            INT16   ipm_nextmtu;
        }ih_pmtu;

    }icmp_hun;

    union {
        struct id_ts  {
            UINT32  its_otime;
            UINT32  its_rtime;
            UINT32  its_ttime;
        } id_ts;

        IPLAYER id_ip;

        UINT32  id_mask;

        INT8    id_data[1];

    } icmp_dun;

}ICMP_LAYER;

/* Define the structure for the ICMP echo list. */
typedef struct _icmp_echo_list_entry
{
    struct _icmp_echo_list_entry    *icmp_next;
    struct _icmp_echo_list_entry    *icmp_prev;

    NU_TASK     *icmp_requesting_task;      /* task that sent the ping */
    STATUS      icmp_echo_status;           /* did we get a reply or timeout */
    UINT16      icmp_echo_seq_num;          /* sequence number of the ping */
} ICMP_ECHO_LIST_ENTRY;

/* This is the head of the linked list for echo requests. */
typedef struct _icmp_echo_list
{
    ICMP_ECHO_LIST_ENTRY    *icmp_head;
    ICMP_ECHO_LIST_ENTRY    *icmp_tail;
}ICMP_ECHO_LIST;

/* The following definitions make accessing the fields in the unions easier. */
#define icmp_pptr       icmp_hun.ih_pptr
#define icmp_gwaddr     icmp_hun.ih_gwaddr
#define icmp_id         icmp_hun.ih_idseq.icd_id
#define icmp_seq        icmp_hun.ih_idseq.icd_seq
#define icmp_void       icmp_hun.ih_void
#define icmp_pmvoid     icmp_hun.ih_pmtu.ipm_void
#define icmp_nextmtu    icmp_hun.ih_pmtu.ipm_nextmtu

#define icmp_otime      icmp_dun.id_ts.its_otime
#define icmp_rtime      icmp_dun.id_ts.its_rtime
#define icmp_ttime      icmp_dun.id_ts.its_ttime
#define icmp_ip         icmp_dun.id_ip
#define icmp_mask       icmp_dun.id_mask
#define icmp_data       icmp_dun.id_data

/* ICMP echo request definitions. */
#define        ICMP_ECHO_REQ_ID                 0x4154 /* AT */
#define        ICMP_DEFAULT_ECHO_TIMEOUT        (5 * SCK_Ticks_Per_Second)
#define        ICMP_ECHO_REQ_HEADER_SIZE        8

/* ICMP Type definitions. */
#define        ICMP_ECHOREPLY                   0
#define        ICMP_UNREACH                     3
#define        ICMP_SOURCEQUENCH                4
#define        ICMP_REDIRECT                    5
#define        ICMP_ECHO                        8
#define        ICMP_TIMXCEED                    11
#define        ICMP_PARAPROB                    12
#define        ICMP_TIMESTAMP                   13
#define        ICMP_TIMESTAMPREPLY              14
#define        ICMP_INFOREQUEST                 15
#define        ICMP_INFOREPLY                   16

/* ICMP code definitions. */
#define        ICMP_UNREACH_NET                 0
#define        ICMP_UNREACH_HOST                1
#define        ICMP_UNREACH_PROTOCOL            2
#define        ICMP_UNREACH_PORT                3
#define        ICMP_UNREACH_NEEDFRAG            4
#define        ICMP_UNREACH_SRCFAIL             5

#define        ICMP_TIMXCEED_TTL                0
#define        ICMP_TIMXCEED_REASM              1

#define        ICMP_PARAPROB_OFFSET             0

#define        ICMP_REDIRECT_NET                0
#define        ICMP_REDIRECT_HOST               1
#define        ICMP_REDIRECT_TOSNET             2
#define        ICMP_REDIRECT_TOSHOST            3


/* Function Prototypes. */
VOID    ICMP_Send_Error(NET_BUFFER *buf_ptr, INT type, INT code, UINT32 dest,
                     DV_DEVICE_ENTRY *device);
VOID    ICMP_Reflect(NET_BUFFER *buf_ptr);
VOID    ICMP_Init(VOID);
//STATUS  ICMP_Send_Echo_Request(UINT8 *, UINT32); mod by sunbey fro big packet test
STATUS  ICMP_Send_Echo_Request(UINT8 *,UINT16, UINT32);
STATUS  ICMP_Echo_Reply (NET_BUFFER *buf_ptr);
STATUS  ICMP_Interpret (NET_BUFFER *buf_ptr, UINT32);

#define ICMP_TYPE_OFFSET            0
#define ICMP_CODE_OFFSET            1
#define ICMP_CKSUM_OFFSET           2
#define ICMP_PPTR_OFFSET            4
#define ICMP_GWADDR_OFFSET          4
#define ICMP_ID_OFFSET              4
#define ICMP_SEQ_OFFSET             6
#define ICMP_VOID_OFFSET            4
#define ICMP_PMVOID_OFFSET          4
#define ICMP_NEXTMTU_OFFSET         6
#define ICMP_OTIME_OFFSET           8
#define ICMP_RTIME_OFFSET           12
#define ICMP_TTIME_OFFSET           16
#define ICMP_IP_OFFSET              8
#define ICMP_MASK_OFFSET            8
#define ICMP_DATA_OFFSET            8

#ifdef          __cplusplus
}
#endif /* _cplusplus */


#endif /* ICMP_H */
