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
#ifndef _IPANEL_MIDDLEWARE_PORTING_DTVTASK_API_FUNCTOTYPE_H_
#define _IPANEL_MIDDLEWARE_PORTING_DTVTASK_API_FUNCTOTYPE_H_

#include "ipanel_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------------------------------------
//  TYPES DEFINITION
//--------------------------------------------------------------------------------------------------
//
typedef INT32_T (*IPANEL_PROTOCOL_EXT_CBK)(CONST CHAR_T* protocolurl);

//--------------------------------------------------------------------------------------------------
//  EXTERN FUNCTION PROTOTYPES
//--------------------------------------------------------------------------------------------------
//
extern VOID *ipanel_create(UINT32_T mem_size);

extern INT32_T ipanel_proc(VOID* handle, UINT32_T msg, UINT32_T p1, UINT32_T p2);

extern INT32_T ipanel_destroy(VOID* handle);

extern INT32_T ipanel_register_protocol(CONST CHAR_T *protocol, IPANEL_PROTOCOL_EXT_CBK func);

extern INT32_T ipanel_open_uri(VOID *handle, CHAR_T *uri);

extern INT32_T ipanel_set_dns_server(INT32_T no,  CHAR_T *ipaddr);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // _IPANEL_MIDDLEWARE_PORTING_DTVTASK_API_FUNCTOTYPE_H_

