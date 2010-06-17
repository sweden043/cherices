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
#ifndef _IPANEL_MIDDLEWARE_PORTING_BASE_API_FUNCTOTYPE_H_
#define _IPANEL_MIDDLEWARE_PORTING_BASE_API_FUNCTOTYPE_H_

#include "ipanel_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

VOID *ipanel_porting_malloc(INT32_T size);

VOID ipanel_porting_free(VOID *ptr);

INT32_T ipanel_porting_dprintf(CONST CHAR_T *fmt, ...);


UINT32_T ipanel_porting_time_ms(VOID);

int ipanel_hex_printout(const char* msg,const char* buf,
						unsigned int  len,int  wide);

void ipanel_test_time_ms();

void ipanel_mem_test();

#ifdef __cplusplus
}
#endif

#endif  // _IPANEL_MIDDLEWARE_PORTING_BASE_API_FUNCTOTYPE_H_

