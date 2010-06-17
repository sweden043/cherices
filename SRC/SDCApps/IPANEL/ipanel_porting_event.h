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
#ifndef _IPANEL_MIDDLEWARE_PORTING_EVENT_API_FUNCTOTYPE_H_
#define _IPANEL_MIDDLEWARE_PORTING_EVENT_API_FUNCTOTYPE_H_

//--------------------------------------------------------------------------------------------------
//  CONSTANTS DEFINITION
//--------------------------------------------------------------------------------------------------
//


//--------------------------------------------------------------------------------------------------
//  TYPES DEFINITION
//--------------------------------------------------------------------------------------------------
//
enum
{
    EIS_EVENT_TYPE_TIMER        = 0,
    EIS_EVENT_TYPE_SYSTEM       = 1,
    EIS_EVENT_TYPE_KEYDOWN      = 2,
    EIS_EVENT_TYPE_KEYUP        = 3,
    EIS_EVENT_TYPE_NETWORK      = 4,
    EIS_EVENT_TYPE_CHAR         = 5,
    EIS_EVENT_TYPE_MOUSE        = 6,
    EIS_EVENT_TYPE_IRKEY        = 7,
    EIS_EVENT_TYPE_DVB          = 0x100
};

enum
{
    EIS_IRKEY_NULL                          = 0x0100,   /* bigger than largest ASCII*/
    EIS_IRKEY_POWER                         = 0x0101,
    EIS_IRKEY_STANDBY,

    /* generic remote controller keys */
    EIS_IRKEY_NUM0                          = 0x110,
    EIS_IRKEY_NUM1,
    EIS_IRKEY_NUM2,
    EIS_IRKEY_NUM3,
    EIS_IRKEY_NUM4,
    EIS_IRKEY_NUM5,                         // 0x115
    EIS_IRKEY_NUM6,
    EIS_IRKEY_NUM7,
    EIS_IRKEY_NUM8,
    EIS_IRKEY_NUM9,
    EIS_IRKEY_UP,                           // 0x11A
    EIS_IRKEY_DOWN,
    EIS_IRKEY_LEFT,
    EIS_IRKEY_RIGHT,
    EIS_IRKEY_SELECT,                       // 0x11E

    /* generic editting/controling keys */
    EIS_IRKEY_INSERT                        = 0x130,
    EIS_IRKEY_DELETE,
    EIS_IRKEY_HOME,
    EIS_IRKEY_END,
    EIS_IRKEY_ESC,
    EIS_IRKEY_CAPS,                         // 0x135

    EIS_IRKEY_PREVLINK                      = 0x150,
    EIS_IRKEY_NEXTLINK,
    EIS_IRKEY_REFRESH,
    EIS_IRKEY_EXIT,
    EIS_IRKEY_BACK,                         /*backspace*/
    EIS_IRKEY_CANCEL,

    /* generic navigating keys */
    EIS_IRKEY_SCROLL_UP                     = 0x170,
    EIS_IRKEY_SCROLL_DOWN,
    EIS_IRKEY_SCROLL_LEFT,
    EIS_IRKEY_SCROLL_RIGHT,
    EIS_IRKEY_PAGE_UP,
    EIS_IRKEY_PAGE_DOWN,                    // 0x175
    EIS_IRKEY_HISTORY_FORWARD,
    EIS_IRKEY_HISTORY_BACKWARD,
    EIS_IRKEY_SHOW_URL,

    /* function remote controller keys */
    EIS_IRKEY_HOMEPAGE                      = 0x200,
    EIS_IRKEY_MENU,
    EIS_IRKEY_EPG,
    EIS_IRKEY_HELP,
    EIS_IRKEY_MOSAIC,
    EIS_IRKEY_VOD,                          // 0x205
    EIS_IRKEY_NVOD,
    EIS_IRKEY_SETTING,
    EIS_IRKEY_STOCK,

    /* special remote controller keys */
    EIS_IRKEY_SOFT_KEYBOARD                 = 0x230,
    EIS_IRKEY_IME,
    EIS_IRKEY_DATA_BROADCAST,
    EIS_IRKEY_VIDEO,                        /*视频键*/
    EIS_IRKEY_AUDIO,                        /*音频键*/
    EIS_IRKEY_LANGUAGE,                     // 0x235
    EIS_IRKEY_SUBTITLE,
    EIS_IRKEY_INFO,
    EIS_IRKEY_RECOMMEND,                    /*推荐键*/
    EIS_IRKEY_FORETELL,                     /*预告键*/
    EIS_IRKEY_FAVORITE,                     /*收藏键*/

    EIS_IRKEY_PLAYLIST = 0x23D,  /* 频道键 */
    EIS_IRKEY_PROGRAM_TYPE = 0x23E, /* 分类键 */

    /* user controling remote controller keys */
    EIS_IRKEY_LAST_CHANNEL                  = 0x250,
    EIS_IRKEY_CHANNEL_UP,
    EIS_IRKEY_CHANNEL_DOWN,
    EIS_IRKEY_VOLUME_UP,
    EIS_IRKEY_VOLUME_DOWN,
    EIS_IRKEY_VOLUME_MUTE,
    EIS_IRKEY_AUDIO_MODE,

    /* virtual function keys */
    EIS_IRKEY_VK_F1                         = 0x300,
    EIS_IRKEY_VK_F2,
    EIS_IRKEY_VK_F3,
    EIS_IRKEY_VK_F4,
    EIS_IRKEY_VK_F5,
    EIS_IRKEY_VK_F6,                        // 0x305
    EIS_IRKEY_VK_F7,
    EIS_IRKEY_VK_F8,
    EIS_IRKEY_VK_F9,
    EIS_IRKEY_VK_F10,
    EIS_IRKEY_VK_F11,                       // 0x30A
    EIS_IRKEY_VK_F12,

    /* special function keys class A */
    EIS_IRKEY_FUNCTION_A                    = 0x320,
    EIS_IRKEY_FUNCTION_B,
    EIS_IRKEY_FUNCTION_C,
    EIS_IRKEY_FUNCTION_D,
    EIS_IRKEY_FUNCTION_E,
    EIS_IRKEY_FUNCTION_F,

    /* special function keys class B */
    EIS_IRKEY_RED                           = 0x340,
    EIS_IRKEY_GREEN,
    EIS_IRKEY_YELLOW,
    EIS_IRKEY_BLUE,

    EIS_IRKEY_ASTERISK                      = 0x350,    /*  功能键'*'       */

    /* VOD/DVD controling keys */
    EIS_IRKEY_PLAY                          = 0x400,
    EIS_IRKEY_STOP,
    EIS_IRKEY_PAUSE,
    EIS_IRKEY_RECORD,
    EIS_IRKEY_FASTFORWARD,
    EIS_IRKEY_REWIND,
    EIS_IRKEY_STEPFORWARD,
    EIS_IRKEY_STEPBACKWARD,
    EIS_IRKEY_DVD_AB,
    EIS_IRKEY_DVD_MENU,
    EIS_IRKEY_DVD_TITILE,
    EIS_IRKEY_DVD_ANGLE,
    EIS_IRKEY_DVD_ZOOM,
    EIS_IRKEY_DVD_SLOW,
    EIS_IRKEY_TV_SYSTEM,
    EIS_IRKEY_DVD_EJECT
};

enum
{
    EIS_DVB_TUNE_SUCCESS    = 8001,
    EIS_DVB_TUNE_FAILED     = 8002
};

enum
{
    EIS_IP_NETWORK_CONNECT              = 5500,
    EIS_IP_NETWORK_DISCONNECT,
    EIS_IP_NETWORK_READY,               
    EIS_IP_NETWORK_FAILED, 
    EIS_IP_NETWORK_SENT_PACKAGE         = 5520,
    EIS_IP_NETWORK_RECEIVED_PACKAGE     = 5521,
    EIS_IP_NETWORK_PING_RESPONSE        = 5530,

    EIS_CABLE_NETWORK_CONNECT           = 5550,
    EIS_CABLE_NETWORK_DISCONNECT        = 5551,

    EIS_NETWORK_UNDEFINED
};

enum
{
    EIS_DEVICE_DECODER_NORMAL = 6050,   //decoder 正常工作,没有异常
                                                                    //状态.解码器从任何异常恢
                                                                    //复后发送此消息.
    EIS_DEVICE_DECODER_HUNGER = 6051    // decoder 没有数据输入，处
                                                                    //于饥饿状态
};

#endif  // _IPANEL_MIDDLEWARE_PORTING_EVENT_API_FUNCTOTYPE_H_

