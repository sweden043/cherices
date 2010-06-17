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
#ifndef _IPANEL_MIDDLEWARE_PORTING_GRAPHICS_API_FUNCTOTYPE_H_
#define _IPANEL_MIDDLEWARE_PORTING_GRAPHICS_API_FUNCTOTYPE_H_

#include "ipanel_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------------------------------------
//  CONSTANTS DEFINITION
//--------------------------------------------------------------------------------------------------
//


//--------------------------------------------------------------------------------------------------
//  TYPES DEFINITION
//--------------------------------------------------------------------------------------------------
//
typedef struct
{
    int x;
    int y;
    int w;
    int h;
} IPANEL_GRAPHICS_WIN_RECT;

typedef enum
{
    IPANEL_GRAPHICS_AVAIL_WIN_NOTIFY    = 1
} IPANEL_GRAPHICS_IOCTL_e;

INT32_T ipanel_porting_graphics_get_info(
        INT32_T    *width,
        INT32_T    *height,
        VOID      **pbuf,
        INT32_T    *buf_width,
        INT32_T    *buf_height
    );

INT32_T ipanel_porting_graphics_install_palette(UINT32_T *pal, INT32_T npals);

INT32_T ipanel_porting_graphics_draw_image(
        INT32_T     x,
        INT32_T     y,
        INT32_T     w,
        INT32_T     h,
        BYTE_T     *bits,
        INT32_T     w_src
    );

INT32_T ipanel_porting_graphics_set_alpha(INT32_T alpha);

INT32_T ipanel_porting_graphics_ioctl(IPANEL_GRAPHICS_IOCTL_e op, VOID *arg);

INT32_T ipanel_graphics_init(VOID);

VOID ipanel_graphics_exit(VOID);

INT32_T ipanel_osd_show_region();

INT32_T ipanel_osd_hide_region();

#ifdef __cplusplus
}
#endif

#endif  // _IPANEL_MIDDLEWARE_PORTING_GRAPHICS_API_FUNCTOTYPE_H_

