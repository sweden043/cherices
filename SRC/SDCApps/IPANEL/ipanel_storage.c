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
#include "ipanel_storage.h"

INT32_T ipanel_porting_storage_ioctl(IPANEL_STORAGE_IOCTL_e cmd, VOID *arg)
{
	return IPANEL_OK;
}

