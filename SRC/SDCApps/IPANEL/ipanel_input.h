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
#ifndef _IPANEL_MIDDLEWARE_PORTING_INPUT_API_FUNCTOTYPE_H_
#define _IPANEL_MIDDLEWARE_PORTING_INPUT_API_FUNCTOTYPE_H_

#include "ipanel_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

INT32_T ipanel_get_ir_key(unsigned event[]);

INT32_T  ipanel_input_init(void);

void ipanel_input_exit(void);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // _IPANEL_MIDDLEWARE_PORTING_INPUT_API_FUNCTOTYPE_H_

