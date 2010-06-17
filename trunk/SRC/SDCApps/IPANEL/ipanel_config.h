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
#ifndef _IPANEL_MIDDLEWARE_PORTING_CONFIG_API_FUNCTOTYPE_H_
#define _IPANEL_MIDDLEWARE_PORTING_CONFIG_API_FUNCTOTYPE_H_

#include "ipanel_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IPANEL_MAIN_WAIT_TIMEOUT        10 //ms

// iPanel中间件使用的内存空间大小
#define IPANEL_MIDDLEWARE_MEMORY_SIZE   (17*1024*1024)

#define IPANEL_MAIN_QUEUE_MAX           12

// 定义屏幕的显示的大小以及初始显示位置
#define IPANEL_DEST_PAL_WIDTH       (720)
#define IPANEL_DEST_PAL_HEIGHT      (576)

#define IPANEL_TV_SCREEN_WIDTH      (640) // 实际屏幕显示宽度
#define IPANEL_TV_SCREEN_HEIGHT     (526) // 实际屏幕显示高度

#define IPANEL_OSD_XOFFSET           48
#define IPANEL_OSD_YOFFSET           25

#define PORTING_ARGB1555            0   // 使用16位颜色格式
#define ARGB8888_to_ARGB1555        1   // 使用32位库而使用16位显示
#define PORTING_ARGB8888            2   // 使用32位颜色格式

//#define USE_SEMITRANSPARENT_COLOR       // 打开半透明颜色显示

// 需要更改颜色格式的话，只需要更改此宏定义即可
#define  PORTING_COLORFMT           PORTING_ARGB1555
//#define  TEST_OSD_AUTOSCALE

#define  OSD_MALLOC_SELF            1 // 使用get_info自行分配内存

//#define USE_PORTING_TRANSPARENT  //如果color值与PORTING层定义的透明色相同时, 将调整color值, 将其加1, 这样颜色损坏得最小.   

//#define DHCP_ADD_VENDOR_CLASS_ID  // 是否DHCP增加Vdendor class id

#define IPANEL_TUNER_CHECK_STATUS   // 是否每隔1s检测tuner锁频状态

// 是否打印LOG, 1为打印, 0为不打印
#define DEBUG_OUT                       0

#define USE_NO_TFCA_CARD  // 是否使用TFCA 的卡

//#define IPANEL_WRITE_UI2FLASH  // 使用外挂UI

//#define IPANEL_RELEASE_VERSION   // 是否使用RELEASE版本  

#define USE_IPANEL_UPGRADE     // 是否使用upgrade升级模式

//#define IPANEL_PRE_MALLOC_BUFFER  // 定义时使用预先分配好的2M内存分配方式

//#define IPANEL_LOADER_DEBUG
//#define IPANEL_LOADER_DEBUG_WRITE

#endif  // _IPANEL_MIDDLEWARE_PORTING_CONFIG_API_FUNCTOTYPE_H_

