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
#ifndef _IPANEL_MIDDLEWARE_PORTING_AV_API_FUNCTOTYPE_H_
#define _IPANEL_MIDDLEWARE_PORTING_AV_API_FUNCTOTYPE_H_

#include "ipanel_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

enum
{
    VIDEO_LINK_UNKNOWN,
    VIDEO_LINK_UP,
    VIDEO_LINK_DOWN
};

enum
{
    AUDIO_LINK_UNKNOWN,
    AUDIO_LINK_UP,
    AUDIO_LINK_DOWN
};

INT32_T ipanel_av_init();

void ipanel_av_exit();

#ifdef __cplusplus
}
#endif

#endif  // _IPANEL_MIDDLEWARE_PORTING_AV_API_FUNCTOTYPE_H_

