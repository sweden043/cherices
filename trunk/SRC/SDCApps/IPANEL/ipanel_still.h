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
#ifndef _IPANEL_MIDDLEWARE_PORTING_STILL_API_FUNCTOTYPE_H_
#define _IPANEL_MIDDLEWARE_PORTING_STILL_API_FUNCTOTYPE_H_

#include "ipanel_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

#define STILL_BOTTOM 576
#define STILL_TOP    0
#define STILL_LEFT   0 
#define STILL_RIGHT  720

bool ipanel_WaitForAudioValid(u_int32 Timeoutms, u_int32 WaitAfterValidms);

void ipanel_clear_still_plane();

void ipanel_decode_still(voidF pImage,u_int32 uSize);

#ifdef __cplusplus
}
#endif

#endif  // _IPANEL_MIDDLEWARE_PORTING_STILL_API_FUNCTOTYPE_H_

