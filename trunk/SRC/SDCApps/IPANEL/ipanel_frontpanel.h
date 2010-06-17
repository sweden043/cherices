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
#ifndef _IPANEL_MIDDLEWARE_PORTING_FRONTPANEL_API_FUNCTOTYPE_H_
#define _IPANEL_MIDDLEWARE_PORTING_FRONTPANEL_API_FUNCTOTYPE_H_

#include "ipanel_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------------------------------------
//  CONSTANTS DEFINITION
//--------------------------------------------------------------------------------------------------
//
#define IPANEL_KEY_MAP_MAX                  256

//--------------------------------------------------------------------------------------------------
//  TYPES DEFINITION
//--------------------------------------------------------------------------------------------------
//
typedef struct
{
    UINT32_T    value;
    UINT32_T    mask;
} IPANEL_FRONT_PANEL_INDICATOR;

typedef struct
{
    UINT16_T    mapping[IPANEL_KEY_MAP_MAX];
} IPANEL_KEY_TABLE;

typedef enum
{
    IPANEL_FRONT_PANEL_SHOW_TEXT        = 1,
    IPANEL_FRONT_PANEL_SHOW_TIME        = 2,
    IPANEL_FRONT_PANEL_SET_INDICATOR    = 3,
    IPANEL_FRONT_PANEL_REMAP_KEYS       = 4
} IPANEL_FRONT_PANEL_IOCTL_e;

INT32_T ipanel_porting_front_panel_ioctl(IPANEL_FRONT_PANEL_IOCTL_e cmd, VOID *arg);

INT32_T ipanel_frontpanel_init();
	
void ipanel_frontpanel_exit();

#ifdef __cplusplus
}
#endif

#endif  // _IPANEL_MIDDLEWARE_PORTING_FRONTPANEL_API_FUNCTOTYPE_H_

