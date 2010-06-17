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
*   FILE NAME                                           VERSION          
*                                                                       
*       tftpdefc.h                                          4.4            
*                                                                       
*   COMPONENT                                                             
*                                                                       
*       Net - TFTP Client
*                                                                       
*   DESCRIPTION                                                           
*                                                                       
*       This file contains data structure definitions used by the TFTP   
*       client and server routines.                                      
*                                                                       
*   DATA STRUCTURES                                                       
*                                                                       
*       TFTP_CB         TFTP Control Block           
*       TFTP_OPTIONS    User indicated options requested of the server
*                                                                       
*   DEPENDENCIES                                                          
*                                                                       
*       fal.h
*                                                                       
*************************************************************************/

#ifndef NU_TFTPDEFC_H
#define NU_TFTPDEFC_H

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif /* _cplusplus */


#ifdef FILE_SYSTEM_SUPPORT
#include "fal.h"
#endif

/* TFTP Opcodes defined by RFC 783 */
#define TFTP_RRQ_OPCODE       1
#define TFTP_WRQ_OPCODE       2
#define TFTP_DATA_OPCODE      3
#define TFTP_ACK_OPCODE       4
#define TFTP_ERROR_OPCODE     5

/* TFTP Opcode defined by RFC 2347 */
#define TFTP_OACK_OPCODE      6

/* Connection Status Values */
#define TRANSFERRING_FILE     100
#define TRANSFER_COMPLETE     102

/* RFC 1350 defaults */
#define TFTP_BLOCK_SIZE_DEFAULT     512
#define TFTP_HEADER_SIZE            4
#define TFTP_ACK_SIZE               4

#define TFTP_BLOCK_SIZE_MAX         65464UL
#define TFTP_BLOCK_SIZE_MIN         8
#define TFTP_BUFFER_SIZE_MIN        50
#define TFTP_PARSING_LENGTH         128

/* Type Of File Transfer */
#define READ_TYPE   0   
#define WRITE_TYPE  1   

/* User defined defaults */
#define TFTP_TIMEOUT_DEFAULT        60
#define TFTP_NUM_RETRANS   3

/* RFC 2348, 2349 options */
typedef struct tftp_options 
{
    UINT32  tsize;                  /* If sent as 0 on a RRQ, server returns the size 
                                       of the file being requested - if value 
                                       specified on a WRQ, server accepts/declines 
                                       the size of the file to be transmitted */
    UINT16  timeout;                /* Specify the server's timeout value */
    UINT16  blksize;                /* Specify the size of the data block
                                       to be transmitted at a time */
    INT16   timeout_acknowledged;   /* NU_TRUE or NU_FALSE if the server
                                       acknowledges the timeout requested */
    INT16   blksize_acknowledged;   /* NU_TRUE or NU_FALSE if the server
                                       acknowledges the blksize requested */
    INT16   tsize_acknowledged;     /* NU_TRUE or NU_FALSE if the server
                                       acknowledges the tsize requested */
} TFTP_OPTIONS;

/* TFTP Control Block  */
typedef struct tftp_cb
{
    INT             socket_number;          /* The socket that is being used. */
    struct          addr_struct server_addr;/* Server's address. */
    TFTP_OPTIONS    options;                /* Options requested of the server */
    UINT16          tid;                    /* Server's Transfer ID */
    INT16           status;                 /* Status of communication. */
    INT16           type;
    UINT16          block_number;           /* Block number of expected packet. */
    CHAR            *trans_buf;             /* Last packet sent. */
#ifdef FILE_SYSTEM_SUPPORT	 
    CHAR            *file_name;
    FAL_FILE        file_desc;              /* File descriptor */
#else
	UINT8			*rd_buffer;		/*the buffer store the file*/
	UINT32		byte_received;
#endif
    UINT32          cur_buf_size;           /* How much space is left in buffer. */    
    CHAR            *mode;                  /* File Transfer Mode Requested by Client. */
} TFTP_CB;

#ifdef          __cplusplus
}
#endif /* _cplusplus */

#endif /* NU_TFTPDEFC_H */
