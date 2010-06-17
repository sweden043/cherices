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
#ifndef _IPANEL_MIDDLEWARE_PORTING_DEMUX_API_FUNCTOTYPE_H_
#define _IPANEL_MIDDLEWARE_PORTING_DEMUX_API_FUNCTOTYPE_H_

#include "ipanel_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------------------------------------
// Types and defines
//--------------------------------------------------------------------------------------------------
//

#define INVALID_CHANNEL                 0xFFFFFFFF
#define PCR_CHANNEL                     0xFFFF

#define DEMUX_MEMORY_SIZE               (2 * 1024 * 1024)

#define FILTER_DEPTH                    8

#define MAX_PSI_CHANNEL_NUMBER          29  // 32 - video、audio and audio1 channel

#define MAX_DMX_CHANNEL_NUMBER          32

typedef struct tagChannelInfo
{
    UINT32_T                ch;
    UINT32_T                buf;
    UINT32_T                len;
    UINT32_T                gap; // 当前buf与上一个buf的间隙, 当为头结点时表示buf与起始地址的间隙
    struct tagChannelInfo  *next;
} ChannelInfo;

typedef struct
{
    UINT32_T        addr;
    UINT32_T        size;
    ChannelInfo    *used_buf_head;
    ChannelInfo    *used_buf_tail;
    ChannelInfo    *available_buf_head;
    ChannelInfo    *available_buf_tail;
    ChannelInfo     ch_info[MAX_PSI_CHANNEL_NUMBER];
    UINT32_T        SemId;
    unsigned char	*buffer;
} ChannelMemoryManager;

typedef struct tagChanelTableStruc
{
	BYTE_T          bUse;
    BYTE_T          bEnable;
    BYTE_T          bOccupied;
	UINT32_T	    buffer;
}ChannelTableStruc;

typedef enum
{
    IPANEL_DMX_DATA_PSI_CHANNEL         = 0,
    IPANEL_DMX_DATA_PES_CHANNEL         = 1,
    IPANEL_DMX_VIDEO_CHANNEL            = 2,
    IPANEL_DMX_AUDIO_CHANNEL            = 3,
    IPANEL_DMX_PCR_CHANNEL              = 4
} IPANEL_DMX_CHANNEL_TYPE_e;

typedef enum
{
    IPANEL_DEMUX_SET_CHANNEL_NOTIFY     = 1,
    IPANEL_DEMUX_SET_SRC                = 2,
    IPANEL_DEMUX_SET_STREAM_TYPE        = 3,
    IPANEL_DEMUX_GET_BUFFER             = 4,
    IPANEL_DEMUX_PUSH_STREAM            = 5,
    IPANEL_DEMUX_STC_FETCH_MODE         = 6,
    IPANEL_DEMUX_STC_SET_TIMEBASE       = 7,
    IPANEL_DEMUX_GET_CURRENT_PCR        = 8,
    IPANEL_DEMUX_GET_CURRENT_STC        = 9
} IPANEL_DEMUX_IOCTL_e;

typedef enum
{
    IPANEL_DEMUX_STC_ONCE_PCR           = 1,
    IPANEL_DEMUX_STC_FOLLOW_PCR         = 2,
    IPANEL_DEMUX_STC_ONCE_APTS          = 3,
    IPANEL_DEMUX_STC_ONCE_VPTS          = 4,
    IPANEL_DEMUX_STC_FREE_RUN           = 5
} IPANEL_DEMUX_STC_FETCH_MODE_e;

typedef enum
{
    IPANEL_STREAM_TS                    = 0,
    IPANEL_STREAM_ES                    = 1,
    IPANEL_STREAM_PS                    = 2
} IPANEL_STREAM_TYPE_e;

typedef enum
{
    IPANEL_DEMUX_SOURCE_DEVICE          = 0x10, // 从芯片硬件接口输入数据
    IPANEL_DEMUX_SOURCE_HOST            = 0x20  // 系统软件推入的数据
} IPANEL_DEMUX_SOURCE_TYPE_e;

typedef struct
{
    UINT32_T    timestamp_h;
    UINT32_T    timestamp_l;
} IPANEL_MEDIA_TIMESTAMP;

typedef VOID (*IPANEL_DMX_NOTIFY_FUNC)(UINT32_T channel, UINT32_T filter, BYTE_T *buf, INT32_T len);

INT32_T ipanel_porting_demux_set_notify(IPANEL_DMX_NOTIFY_FUNC func);

UINT32_T ipanel_porting_demux_create_channel(INT32_T poolsize, IPANEL_DMX_CHANNEL_TYPE_e type);

INT32_T ipanel_porting_demux_set_channel_pid(UINT32_T channel, INT16_T pid);

INT32_T ipanel_porting_demux_destroy_channel(UINT32_T channel);

INT32_T ipanel_porting_demux_start_channel(UINT32_T channel);

INT32_T ipanel_porting_demux_stop_channel(UINT32_T channel);

UINT32_T ipanel_porting_demux_create_filter(UINT32_T channel);

INT32_T ipanel_porting_demux_set_filter(
        UINT32_T    channel,
        UINT32_T    filter,
        UINT32_T    wide,
        BYTE_T      coef[],
        BYTE_T      mask[],
        BYTE_T      excl[]
    );

INT32_T ipanel_porting_demux_destroy_filter(UINT32_T channel, UINT32_T filter);

INT32_T ipanel_porting_demux_enable_filter(UINT32_T channel, UINT32_T filter);

INT32_T ipanel_porting_demux_disable_filter(UINT32_T channel, UINT32_T filter);

INT32_T ipanel_porting_demux_ioctl(IPANEL_DEMUX_IOCTL_e op, VOID *arg);

INT32_T ipanel_demux_init(VOID);

VOID ipanel_demux_exit(VOID);

#ifdef __cplusplus
}
#endif

#endif  // _IPANEL_MIDDLEWARE_PORTING_DEMUX_API_FUNCTOTYPE_H_

