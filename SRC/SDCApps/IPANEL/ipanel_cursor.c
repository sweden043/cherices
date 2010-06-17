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
#include "ipanel_cursor.h"

INT32_T ipanel_porting_cursor_get_position(INT32_T *x, INT32_T *y)
{
	return IPANEL_OK;
}

INT32_T ipanel_porting_cursor_set_position(INT32_T x, INT32_T y)
{
	return IPANEL_OK;
}

INT32_T ipanel_porting_cursor_get_shape(VOID)
{
	return IPANEL_OK;
}

INT32_T ipanel_porting_cursor_set_shape(INT32_T shape)
{
	return IPANEL_OK;
}

INT32_T ipanel_porting_cursor_show(INT32_T showflag)
{
	return IPANEL_OK;
}

INT32_T ipanel_porting_cursor_ioctl(IPANEL_CURSOR_IOCTL_e cmd, VOID *arg)
{
	return IPANEL_OK;
}

