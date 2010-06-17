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
*       tftpextc.h                                          4.4        
*                                                                       
*   COMPONENT                                                            
*                                                                       
*       TFTP -  Trivial File Transfer Protocol                           
*                                                                       
*   DESCRIPTION                                                          
*                                                                       
*       This file contains function prototypes of all TFTP functions.    
*                                                                       
*   DATA STRUCTURES                                                      
*                                                                       
*       None                                                             
*                                                                       
*   DEPENDENCIES                                                         
*                                                                       
*       None
*                                                                       
*************************************************************************/

#ifndef NU_TFTPEXTC_H
#define NU_TFTPEXTC_H

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif /* _cplusplus */

#define NU_TFTPC_Get  TFTPC_Get
#define NU_TFTPC_Put  TFTPC_Put

#ifndef NU_TFTP_OPTIONS
#define NU_TFTP_OPTIONS TFTP_OPTIONS
#endif

#ifdef FILE_SYSTEM_SUPPORT	 
INT32  TFTPC_Get(UINT8 *, CHAR *, CHAR *, TFTP_OPTIONS *);
INT32  TFTPC_Put(UINT8 *, CHAR *, CHAR *, TFTP_OPTIONS *);
#else
INT32  TFTPC_Get(UINT8 *, CHAR *, UINT8*, TFTP_OPTIONS *);
INT32  TFTPC_Put(UINT8 *, CHAR *, UINT8 *, TFTP_OPTIONS *);
#endif
STATUS TFTPC_Request(UINT8 *, CHAR *, TFTP_CB *);
INT32  TFTPC_Recv(TFTP_CB *);
STATUS TFTPC_Process_Data(TFTP_CB *, INT32);
STATUS TFTPC_Ack(TFTP_CB *);
INT32  TFTPC_Send_Data(TFTP_CB *);
STATUS TFTPC_Process_Ack(TFTP_CB *, INT32);
STATUS TFTPC_Retransmit(TFTP_CB *, INT32);
STATUS TFTPC_Error(TFTP_CB *, INT16, CHAR *);
STATUS TFTPC_Check_Options(TFTP_CB *, INT32);
STATUS TFTPC_Set_Options(UINT32 buf_size, TFTP_OPTIONS *ops);

/* Error codes for the TFTP Client component. */
#define TFTP_ERROR              -1751  /*   Not defined, see error message    */
#define TFTP_FILE_NFOUND        -1752  /*   File not found                    */
#define TFTP_ACCESS_VIOLATION   -1753  /*   Access violation                  */
#define TFTP_DISK_FULL          -1754  /*   Disk full or allocation execeeded */
#define TFTP_FILE_ERROR         -1755
#define TFTP_BAD_OPERATION      -1756  /*   Illegal TFTP operation            */
#define TFTP_UNKNOWN_TID        -1757  /*   Unknown transfer ID               */
#define TFTP_FILE_EXISTS        -1758  /*   File already exists               */
#define TFTP_NO_SUCH_USER       -1759  /*   No such user                      */
#define TFTP_BAD_OPTION         -1760  /*   Illegal option negotiation       */
#define TFTP_NO_MEMORY          -1761
#define TFTP_CON_FAILURE        -1762

#ifdef          __cplusplus
}
#endif /* _cplusplus */

#endif /* NU_TFTPEXTC_H */

