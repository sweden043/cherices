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
#include "ipanel_base.h"
#include "ipanel_avm.h"

INT32_T ipanel_avm_play_srv(IPANEL_SERVICE_INFO *srv)
{
	return IPANEL_OK;
}

INT32_T ipanel_avm_stop_srv(VOID)
{
	return IPANEL_OK;
}

INT32_T ipanel_avm_ioctl(IPANEL_AVM_IOCTL_e op, VOID *arg)
{
	return IPANEL_OK;
}

