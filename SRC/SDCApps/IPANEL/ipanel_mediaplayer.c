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
#include "ipanel_mediaplayer.h"

UINT32_T ipanel_mediaplayer_open(CONST CHAR_T *des, IPANEL_PLAYER_EVENT_NOTIFY cbk)
{
	return 1;
}

INT32_T ipanel_mediaplayer_close(UINT32_T player)
{
	return IPANEL_OK;
}

INT32_T ipanel_mediaplayer_play(UINT32_T player,CONST BYTE_T *mrl,
								CONST BYTE_T *des)
{
	return IPANEL_OK;
}

INT32_T ipanel_mediaplayer_stop(UINT32_T player)
{
	return IPANEL_OK;
}

INT32_T ipanel_mediaplayer_pause(UINT32_T player)
{
	return IPANEL_OK;
}

INT32_T ipanel_mediaplayer_resume(UINT32_T player)
{
	return IPANEL_OK;
}

INT32_T ipanel_mediaplayer_slow(UINT32_T player, INT32_T rate)
{
	return IPANEL_OK;
}

INT32_T ipanel_mediaplayer_forward(UINT32_T player,INT32_T rate)
{
	return IPANEL_OK;
}

INT32_T ipanel_mediaplayer_rewind(UINT32_T player, INT32_T rate)
{
	return IPANEL_OK;
}

INT32_T ipanel_mediaplayer_seek(UINT32_T player, BYTE_T *pos)
{
	return IPANEL_OK;
}

INT32_T ipanel_mediaplayer_ioctl(UINT32_T player ,INT32_T op,UINT32_T *param)
{
	return IPANEL_OK;
}

INT32_T ipanel_mediaplayer_start_record(UINT32_T player, CONST BYTE_T *mrl,
	                                    CONST BYTE_T *device)	                                    
{
	return IPANEL_OK;
}

INT32_T ipanel_mediaplayer_stop_record(UINT32_T player)
{
	return IPANEL_OK;
}

