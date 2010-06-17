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
#include "ipanel_config.h"
#include "ipanel_base.h"
#include "ipanel_os.h"
#include "ipanel_tuner.h"
#include "ipanel_demux.h"
#include "ipanel_porting_event.h"

extern UINT32_T g_main_queue ;

static ChannelTableStruc m_DmxChannelTable[MAX_DMX_CHANNEL_NUMBER];

//--------------------------------------------------------------------------------------------------
// Global data
//--------------------------------------------------------------------------------------------------
/*Video通道和Audio通道*/
extern UINT32_T  g_ipanel_video_channel, g_ipanel_audio_channel; 

extern u_int32 gDemuxInstance;

//--------------------------------------------------------------------------------------------------
// Internal Prototypes
//--------------------------------------------------------------------------------------------------
//
static ChannelMemoryManager     m_channel_mem_mgr;
static IPANEL_DMX_NOTIFY_FUNC   m_ipanel_dmx_data_func;
static u_int8                   m_section_buffer[4096];
//--------------------------------------------------------------------------------------------------
// Internal functions
//--------------------------------------------------------------------------------------------------
//

static INT32_T dmx_allocate_channel_buffer(UINT32_T channel, UINT32_T *buf, INT32_T len);
static VOID    dmx_free_channel_buffer(UINT32_T channel);

static genfilter_mode demux_section_header_notify(pgen_notify_t pNotifyData);
static genfilter_mode demux_section_data_notify(pgen_notify_t pNotifyData);

/********************************************************************************************************
功能：设置demux数据到达时的通知回调函数。
      当数据来的时侯通过这个回调函数，将数据发送给iPanel MiddleWare。
      底层应该保证回调函数返回之前，缓存中
      的数据不被改动，完整Section(PES packet)应该在一个连续的buffer中，
      如果底层使用循环buffer管理数据，当
      遇到循环buffer的尾部时，必须要做处理，完成section(PES packet)的组装工作。
      由于iPanel MiddleWare的回调函数中使用了信号量互斥机制，
      所以这个函数不能在中断中被调用，建议客户实现
      demux数据的接收管理线程，然后再此线程中调用iPanel MiddleWare的回调函数。

      回调函数的定义是：
      typedef VOID (*IPANEL_DMX_NOTIFY_FUNC)(UINT32_T channel, UINT32_T filter, BYTE_T *buf, INT32_T len)；
      各个参数说明如下：
      channel - 通道过滤器句柄；
      filter  - 二级过滤器句柄；
      buf     - 底层存放的完整Section(PES packet)数据的起始地址；
      len     - buf中有效数据的长度。

原型：INT32_T ipanel_porting_demux_set_notify(IPANEL_DMX_NOTIFY_FUNC func)
参数说明：
  输入参数：func： 回调函数的入口地址.
  输出参数：无
返    回：
  IPANEL_OK:成功;
  IPANEL_ERR:失败。
********************************************************************************************************/
INT32_T ipanel_porting_demux_set_notify(IPANEL_DMX_NOTIFY_FUNC func)
{
    INT32_T ret = IPANEL_ERR;

    ipanel_porting_dprintf("[ipanel_porting_demux_set_notify] func=%p\n", func);

    if (func)
    {
        m_ipanel_dmx_data_func = func;

        ret = IPANEL_OK;
    }

    return ret;
}

/********************************************************************************************************
功能：创建一个通道过滤器。
原型：UINT32_T ipanel_porting_demux_create_channel(INT32_T poolsize, IPANEL_DMX_CHANNEL_TYPE_e type)
参数说明：
  输入参数：
    poolsize：底层设置的该通道的缓存池的尺寸（如0x10000(64kB)等，可能实际分配缓存的时侯需要根据这个值调整）；建议每个通道的buffer不小于32KB。(64KB?)
    type:     通道类型，从下面的枚举类型中取值。
      typedef enum
      {
        IPANEL_DMX_DATA_PSI_CHANNEL = 0,
        IPANEL_DMX_DATA_PES_CHANNEL,
        IPANEL_DMX_VIDEO_CHANNEL,
        IPANEL_DMX_AUDIO_CHANNEL,
        IPANEL_DMX_PCR_CHANNEL
      } IPANEL_DMX_CHANNEL_TYPE_e; 
  输出参数：无
返    回：
  == IPANEL_NULL: 创建通道不成功；
  != IPANEL_NULL: 返回一个通道过滤器的句柄。
********************************************************************************************************/
UINT32_T ipanel_porting_demux_create_channel(INT32_T poolsize, IPANEL_DMX_CHANNEL_TYPE_e type)
{
    DMX_STATUS status;
    u_int32    chid = PCR_CHANNEL - 1;
    
    ipanel_porting_dprintf("[ipanel_porting_demux_create_channel] size=0x%x, type=%d\n", poolsize, type);

    if (IPANEL_DMX_DATA_PSI_CHANNEL == type)
    {
    	
        UINT32_T buffer = 0;
        INT32_T  ret = IPANEL_ERR;

		//add by ccc for enlarge buffer size of oc poolsize
    	if (0x40000 == poolsize)
    	{
			poolsize = 0x80000;
		}
		
        status = cnxt_dmx_channel_open(gDemuxInstance, 0, PSI_CHANNEL_TYPE, &chid);
        if (DMX_STATUS_OK != status)
        {
            ipanel_porting_dprintf("[ipanel_porting_demux_create_channel] cnxt_dmx_channel_open \
                                    failed(%d)\n", status);

            goto CREATE_CHANNEL_FAILED;
        }

		/*注册回调函数*/
        status = cnxt_dmx_set_section_channel_attributes(   gDemuxInstance,
                                                            chid,
                                                            demux_section_header_notify,
                                                            demux_section_data_notify,
                                                            0,
                                                            8   );
        if (DMX_STATUS_OK != status)
        {
            ipanel_porting_dprintf("[ipanel_porting_demux_create_channel] set callback function \
                                    failed(%d)\n", status);

            status = cnxt_dmx_channel_close(gDemuxInstance, chid);
            if (DMX_STATUS_OK != status)
            {
                ipanel_porting_dprintf("[ipanel_porting_demux_create_channel] cnxt_dmx_channel_close \
                                        failed(%d)\n", status);
            }

            goto CREATE_CHANNEL_FAILED;
        }

		/*分配通道对应的poolsize*/
        #ifdef IPANEL_PRE_MALLOC_BUFFER
        ret = dmx_allocate_channel_buffer(chid, &buffer, poolsize);
        if (IPANEL_ERR == ret)
        #else
        buffer = (UINT32_T)ipanel_porting_malloc(poolsize);
        if(buffer == IPANEL_NULL)
        #endif
        {
            ipanel_porting_dprintf("[ipanel_porting_demux_create_channel] dmx_allocate_channel_buffer failed\n");

            status = cnxt_dmx_channel_close(gDemuxInstance, chid);
            if (DMX_STATUS_OK != status)
            {
                ipanel_porting_dprintf("[ipanel_porting_demux_create_channel] cnxt_dmx_channel_close \
                                        failed(%d)\n", status);
            }

            goto CREATE_CHANNEL_FAILED;
        }

		/*设置对应通道的buffer和硬件关联*/
        status = cnxt_dmx_set_channel_buffer(gDemuxInstance, chid, (void*)buffer, poolsize);
        if (DMX_STATUS_OK != status)
        {
            ipanel_porting_dprintf("[ipanel_porting_demux_create_channel] set channel buffer \
                                    failed(%d)\n", status);

            status = cnxt_dmx_channel_close(gDemuxInstance, chid);
            if (DMX_STATUS_OK != status)
            {
                ipanel_porting_dprintf("[ipanel_porting_demux_create_channel] cnxt_dmx_channel_close \
                                        failed(%d)\n", status);
            }

            #ifdef IPANEL_PRE_MALLOC_BUFFER
            dmx_free_channel_buffer(chid);
            #endif

            goto CREATE_CHANNEL_FAILED;
        }

        ipanel_porting_dprintf("[ipanel_porting_demux_create_channel] channel=%d, buf=0x%x\n", chid + 1, buffer);

        if( m_DmxChannelTable[chid].bUse == TRUE)
        {
            ipanel_porting_dprintf("[ipanel_porting_demux_create_channel] have error (create same channel multi times)!\n");
        }

        m_DmxChannelTable[chid].bUse = TRUE;
        m_DmxChannelTable[chid].bOccupied  = TRUE;
        m_DmxChannelTable[chid].buffer = buffer ;

        return chid + 1;
    }

	/*创建视频通道*/
    if (IPANEL_DMX_VIDEO_CHANNEL == type)
    {
        status = cnxt_dmx_channel_open(gDemuxInstance, DMX_CH_CAP_DESCRAMBLING, VIDEO_PES_TYPE, &chid);
        if (DMX_STATUS_OK != status)
        {
            ipanel_porting_dprintf("[ipanel_porting_demux_create_channel] vedio cnxt_dmx_channel_open \
                                    failed(%d)\n", status);

            goto CREATE_CHANNEL_FAILED;
        }

        ipanel_porting_dprintf("[ipanel_porting_demux_create_channel] vedio channel=%d\n", chid + 1);

        g_ipanel_video_channel = chid;
 
        goto CREATE_CHANNEL_SUCCESS;
    }
	

	/*创建音频通道*/
    if (IPANEL_DMX_AUDIO_CHANNEL == type)
    {
        status = cnxt_dmx_channel_open(gDemuxInstance, DMX_CH_CAP_DESCRAMBLING, AUDIO_PES_TYPE, &chid);
        if (DMX_STATUS_OK != status)
        {
            ipanel_porting_dprintf("[ipanel_porting_demux_create_channel] audio cnxt_dmx_channel_open \
                                    failed(%d)\n", status);

            goto CREATE_CHANNEL_FAILED;
        }

        ipanel_porting_dprintf("[ipanel_porting_demux_create_channel] audio channel=%d\n", chid + 1);

        g_ipanel_audio_channel = chid;
 
        goto CREATE_CHANNEL_SUCCESS;
    }


    if (IPANEL_DMX_PCR_CHANNEL == type)
    {
        ipanel_porting_dprintf("[ipanel_porting_demux_create_channel] pcr channel=0x%x\n", PCR_CHANNEL);

        goto CREATE_CHANNEL_SUCCESS;
    }

CREATE_CHANNEL_SUCCESS:
    return chid + 1;
   
CREATE_CHANNEL_FAILED:
    return IPANEL_NULL;
}

/********************************************************************************************************
功能：将pid设置到通道过滤器
原型：INT32_T ipanel_porting_demux_set_channel_pid(UINT32_T channel, INT16_T pid)
参数说明：
  输入参数：
    channel：通道过滤器句柄
    pid：    要过滤的pid
  输出参数：无
返    回：
  IPANEL_OK:成功;
  IPANEL_ERR:失败。
********************************************************************************************************/
INT32_T ipanel_porting_demux_set_channel_pid(UINT32_T channel, INT16_T pid)
{
    INT32_T     ret = IPANEL_ERR;
    DMX_STATUS  status;
    UINT32_T    cur_channel =0;

    ipanel_porting_dprintf("[ipanel_porting_demux_set_channel_pid] channel=%d, pid=0x%x\n", channel, pid);

    if ((PCR_CHANNEL == channel) && (pid > 0))
    {
        status = cnxt_dmx_set_pcr_pid(gDemuxInstance, pid);
        if (DMX_STATUS_OK != status)
        {
            ipanel_porting_dprintf("[ipanel_porting_demux_set_channel_pid] set pcr pid failed(%d)\n", status);
        }
        else
        {
            ret = IPANEL_OK;
        }
    }
    else if (channel)
    {
        cur_channel = channel -1;
        status = cnxt_dmx_set_section_channel_tag(gDemuxInstance, cur_channel,pid);
        
        status = cnxt_dmx_channel_set_pid(gDemuxInstance, cur_channel, (u_int16)pid);
        if (DMX_STATUS_OK != status)
        {
            ipanel_porting_dprintf("[ipanel_porting_demux_set_channel_pid] set pid failed(%d)\n", status);
        }
        else
        {
            ret = IPANEL_OK;
        }
    }

    ipanel_porting_dprintf("[ipanel_porting_demux_set_channel_pid] ret=%d\n", ret);

    return ret;
}



/********************************************************************************************************
功能：销毁一个通道过滤器
原型：INT32_T ipanel_porting_demux_destroy_channel(UINT32_T channel)
参数说明：
  输入参数：
    channel：通道过滤器句柄
  输出参数：无
返    回：
  IPANEL_OK:成功;
  IPANEL_ERR:失败。
********************************************************************************************************/
INT32_T ipanel_porting_demux_destroy_channel(UINT32_T channel)
{
    INT32_T     ret = IPANEL_ERR;
    DMX_STATUS  status;
	UINT32_T    ch_idx = channel;

    ipanel_porting_dprintf("[ipanel_porting_demux_destroy_channel] channel=%d\n", channel);

    if (PCR_CHANNEL == channel)
    {
        ret = IPANEL_OK;
    }
    else if (channel)
    {
        --channel;

		if(TRUE == m_DmxChannelTable[channel].bOccupied)
		{
			// 如果channel没有stop，则强制将其stop先再destroy
			if( m_DmxChannelTable[channel].bEnable == TRUE)
			{
				ipanel_porting_dprintf("[ipanel_porting_demux_destroy_channel] disable channel firstly.\n");
				ipanel_porting_demux_stop_channel(ch_idx);
				m_DmxChannelTable[channel].bEnable = FALSE;
			}
			m_DmxChannelTable[channel].bOccupied = FALSE;
        }

        if(m_DmxChannelTable[channel].bUse == FALSE)
        {
            ipanel_porting_dprintf("[ipanel_porting_demux_destroy_channel] have error!(delete channel multi times)\n");
        }

        m_DmxChannelTable[channel].bUse = FALSE ;

        status = cnxt_dmx_channel_close(gDemuxInstance, channel);
        if (DMX_STATUS_OK != status)
        {
            ipanel_porting_dprintf("[ipanel_porting_demux_destroy_channel] destroy channel failed(%d)\n", status);
        }

        #ifdef IPANEL_PRE_MALLOC_BUFFER
        dmx_free_channel_buffer(channel);
        #else
        if(m_DmxChannelTable[channel].buffer)
        {
            ipanel_porting_free((void*)m_DmxChannelTable[channel].buffer);
            m_DmxChannelTable[channel].buffer = IPANEL_NULL;
        }
        #endif

        ret = IPANEL_OK;
    }

    ipanel_porting_dprintf("[ipanel_porting_demux_destroy_channel] ret=%d\n", ret);

    return ret;
}

/********************************************************************************************************
功能：启动一个通道过滤器来过滤数据
原型：INT32_T ipanel_porting_demux_start_channel(UINT32_T channel)
参数说明：
  输入参数：
    channel：通道过滤器句柄
  输出参数：无
返    回：
  IPANEL_OK:成功;
  IPANEL_ERR:失败。
********************************************************************************************************/
INT32_T ipanel_porting_demux_start_channel(UINT32_T channel)
{
    INT32_T     ret = IPANEL_ERR;
    DMX_STATUS  status;

    ipanel_porting_dprintf("[ipanel_porting_demux_start_channel] channel=%d\n", channel);

    if (PCR_CHANNEL == channel)
    {
        ret = IPANEL_OK;
    }
    else if (channel)
    {
        --channel;
        
        status = cnxt_dmx_channel_control(gDemuxInstance, channel, GEN_DEMUX_ENABLE);
        if (DMX_STATUS_OK != status)
        {
            ipanel_porting_dprintf("[ipanel_porting_demux_start_channel] start channel failed(%d)\n", status);
        }
        else
        {
            ret = IPANEL_OK;
        }

		m_DmxChannelTable[channel].bEnable = TRUE;
    }

    ipanel_porting_dprintf("[ipanel_porting_demux_start_channel] ret=%d\n", ret);

    return ret;
}

/********************************************************************************************************
功能：停止一个接收数据的通道过滤器，让它不再过滤数据
原型：INT32_T ipanel_porting_demux_stop_channel(UINT32_T channel)
参数说明：
  输入参数：
    channel：通道过滤器句柄
  输出参数：无
返    回：
  IPANEL_OK:成功;
  IPANEL_ERR:失败。
********************************************************************************************************/
INT32_T ipanel_porting_demux_stop_channel(UINT32_T channel)
{
    INT32_T     ret = IPANEL_ERR;
    DMX_STATUS  status;

    ipanel_porting_dprintf("[ipanel_porting_demux_stop_channel] channel=%d\n", channel);

    if (PCR_CHANNEL == channel)
    {
        cnxt_dmx_set_pcr_pid(gDemuxInstance, 0x1fff);
        
        ret = IPANEL_OK;
    }
    else if (channel)
    {
        --channel ;
            
        status = cnxt_dmx_channel_control(gDemuxInstance, channel, GEN_DEMUX_DISABLE);
        if (DMX_STATUS_OK != status)
        {
            ipanel_porting_dprintf("[ipanel_porting_demux_stop_channel] stop channel failed(%d)\n", status);
        }
        else
        {
            ret = IPANEL_OK;
        }

		m_DmxChannelTable[channel].bEnable = FALSE;
    }

    ipanel_porting_dprintf("[ipanel_porting_demux_stop_channel] ret=%d\n", ret);

    return ret;
}

/********************************************************************************************************
功能：创建一个二级过滤器
原型：UINT32_T ipanel_porting_demux_create_filter(UINT32_T channel)
参数说明：
  输入参数：
    channel：通道过滤器句柄
  输出参数：无
返    回：
  == IPANEL_NULL: 创建不成功；
  != IPANEL_NULL: 返回一个二级过滤器的句柄。
*******************************************************************************************************/
UINT32_T ipanel_porting_demux_create_filter(UINT32_T channel)
{
    UINT32_T    filter_id = IPANEL_NULL;
    DMX_STATUS  status;

    ipanel_porting_dprintf("[ipanel_porting_demux_create_filter] channel=%d\n", channel);

    if (channel)
    {
        status = cnxt_dmx_filter_open(gDemuxInstance, --channel, FILTER_DEPTH, &filter_id);
        if (DMX_STATUS_OK != status)
        {
            ipanel_porting_dprintf("[ipanel_porting_demux_create_filter] create filter failed(%d)\n", status);

            return filter_id;
        }
    }

    ipanel_porting_dprintf("[ipanel_porting_demux_create_filter] filter=%d\n", filter_id + 1);

    return filter_id + 1;
}

/********************************************************************************************************
功能：修改/设置一个二级过滤器的过滤参数
原型：INT32_T ipanel_porting_demux_set_filter(UINT32_T channel, UINT32_T filter,
            UINT32_T wide, BYTE_T coef[], BYTE_T mask[], BYTE_T excl[])
参数说明：
  输入参数：
    channel： 通道过滤器句柄。
    filter：  二级过滤器的句柄；
    wide：过滤宽度；
　　coef：需要过滤的section头的匹配数组，该数组包含wide个有效元素
		      coef[0]~coef[wide－1]：数据满足data&mask == coef的条件。
　　　　coef[0]：二级过滤中的section头的第一个字节，即table_id；
　　　　coef[1]：二级过滤中的section头的第二个字节；
　　　　coef[2]：二级过滤中的section头的第三个字节；
　　　　coef[3]：二级过滤中的section头的第四个字节；
　　　　余此类推，直到第wide个字节。

　  mask：需要过滤的section头的掩码数组，
		     该数组包含wide个有效元素mask[0]~mask[wide－1]：
　　　　mask[0]：二级过滤中的section头的第一个字节的mask；
　　　　mask[1]：二级过滤中的section头的第二个字节的mask；
　　　　mask[2]：二级过滤中的section头的第三个字节的mask；
　　　　mask[3]：二级过滤中的section头的第四个字节的mask；
　　　　余此类推，直到第wide个字节。

	excl：需要过滤的section头的掩码数组，
		      该数组包含wide个有效元素excl[0]~excl[wide－1]：对条件进行反向选择。
　　　　excl[0]：二级过滤中的section头的第一个字节的excl；
　　　　excl[1]：二级过滤中的section头的第二个字节的excl；
　　　　excl[2]：二级过滤中的section头的第三个字节的excl；
　　　　excl[3]：二级过滤中的section头的第四个字节的excl；
　　　　余此类推，直到第wide个字节。

   过滤逻辑满足下面公式：
   在excl==0时：src&mask == coef&mask
   在excl!＝0时：(src & (mask & ~excl))==(coef & (mask & ~excl)) && (src & (mask & excl))!=(coef & (mask & excl))
   使用方法举例：针对过滤section数据的过滤器，传的wide为11。
   如果需要将table id为0x4E且Section长度域后第一个字节的低4bit不为1100B的数据过滤出来，
   过滤器的过滤条件应该设置为：
       coef：[0x4E 0x00 0x00 0x0C 0x00 0x00(后面5byte的0)]
       mask：[0xFF 0x00 0x00 0xFF 0x00 0x00(后面5byte的0)]
       excl：[0x00 0x00 0x00 0x0C 0x00 0x00(后面5byte的0)]

输出参数：无
  
返    回：
  IPANEL_OK:成功;
  IPANEL_ERR:失败。
********************************************************************************************************/
INT32_T ipanel_porting_demux_set_filter(
        UINT32_T    channel,
        UINT32_T    filter,
        UINT32_T    wide,
        BYTE_T      coef[],
        BYTE_T      mask[],
        BYTE_T      excl[]
    )
{
    INT32_T     ret = IPANEL_ERR;
    DMX_STATUS  status;

    ipanel_porting_dprintf("[ipanel_porting_demux_set_filter] channel=%d, filter=%d, wide=%d\n",\
        channel, filter, wide);

    if (channel && filter && (coef || mask || excl) && (wide <= 16))
    {
        UINT32_T    i;
        u_int8      cf[FILTER_DEPTH]    = {0};
        u_int8      msk[FILTER_DEPTH]   = {0};
        u_int8      notmsk[FILTER_DEPTH] = {0};

        if (coef)
        {
            ipanel_porting_dprintf("\tcoef: %02x %02x %02x %02x %02x %02x %02x %02x\n",\
                coef[0], coef[1], coef[2], coef[3], coef[4], coef[5], coef[6], coef[7]);

            for (i = 0; ((i < wide) && (i < FILTER_DEPTH)); i++)
            {
                cf[i] = coef[i];
            }
        }

        if (mask)
        {
            ipanel_porting_dprintf("\tmask: %02x %02x %02x %02x %02x %02x %02x %02x\n",\
                mask[0], mask[1], mask[2], mask[3], mask[4], mask[5], mask[6], mask[7]);

            for (i = 0; ((i < wide) && (i < FILTER_DEPTH)); i++)
            {
                msk[i] = mask[i];
            }
        }

        if (excl)
        {
            ipanel_porting_dprintf("\texcl: %02x %02x %02x %02x %02x %02x %02x %02x\n",\
                excl[0], excl[1], excl[2], excl[3], excl[4], excl[5], excl[6], excl[7]);

            for (i = 0; ((i < wide) && (i < FILTER_DEPTH)); i++)
            {
                notmsk[i] = excl[i];
            }
        }

        status = cnxt_dmx_set_filter(gDemuxInstance, --channel, --filter, cf, msk, NULL);
        if (DMX_STATUS_OK != status)
        {
            ipanel_porting_dprintf("[ipanel_porting_demux_set_filter] set filter failed(%d)\n", status);
        }
        else
        {
            ret = IPANEL_OK;
        }
    }

    ipanel_porting_dprintf("[ipanel_porting_demux_set_filter] ret=%d\n", ret);

    return ret;
}

/********************************************************************************************************
功能：销毁指定的二级过滤器
原型：INT32_T ipanel_porting_demux_destroy_filter(UINT32_T channel, UINT32_T filter)
参数说明：
  输入参数：
    channel： 通道过滤器句柄。
    filter：  二级过滤器的句柄；
  输出参数：无
返    回：
  IPANEL_OK:成功;
  IPANEL_ERR:失败。
********************************************************************************************************/
INT32_T ipanel_porting_demux_destroy_filter(UINT32_T channel, UINT32_T filter)
{
    INT32_T     ret = IPANEL_ERR;
    DMX_STATUS  status;

    ipanel_porting_dprintf("[ipanel_porting_demux_destroy_filter] channel=%d, filter=%d\n", channel, filter);

    if (channel && filter)
    {
        status = cnxt_dmx_filter_close(gDemuxInstance, --channel, --filter);
        if (DMX_STATUS_OK != status)
        {
            ipanel_porting_dprintf("[ipanel_porting_demux_destroy_filter] destroy filter failed(%d)\n", status);
        }
        else
        {
            ret = IPANEL_OK;
        }
    }

    ipanel_porting_dprintf("[ipanel_porting_demux_destroy_filter] ret=%d\n", ret);

    return ret;
}

/********************************************************************************************************
功能：使指定的二级过滤器可以接收数据
原型：INT32_T ipanel_porting_demux_enable_filter(UINT32_T channel, UINT32_T filter)
参数说明：
  输入参数：
    channel： 通道过滤器句柄。
    filter：  二级过滤器的句柄；
  输出参数：无
返    回：
  IPANEL_OK:成功;
  IPANEL_ERR:失败。
********************************************************************************************************/
INT32_T ipanel_porting_demux_enable_filter(UINT32_T channel, UINT32_T filter)
{
    INT32_T     ret = IPANEL_ERR;
    DMX_STATUS  status;

    ipanel_porting_dprintf("[ipanel_porting_demux_enable_filter] channel=%d, filter=%d\n", channel, filter);

    if (channel && filter)
    {
        status = cnxt_dmx_filter_control(gDemuxInstance, --channel, --filter, GEN_DEMUX_ENABLE);
        if (DMX_STATUS_OK != status)
        {
            ipanel_porting_dprintf("[ipanel_porting_demux_enable_filter] enable filter failed(%d)\n", status);
        }
        else
        {
            ret = IPANEL_OK;
        }
    }

    ipanel_porting_dprintf("[ipanel_porting_demux_enable_filter] ret=%d\n", ret);

    return ret;
}

/********************************************************************************************************
功能：使指定的二级过滤器不能再接收数据
原型：INT32_T ipanel_porting_demux_disable_filter(UINT32_T channel, UINT32_T filter)
参数说明：
  输入参数：
    channel： 通道过滤器句柄。
    filter：  二级过滤器的句柄；
  输出参数：无
返    回：
  IPANEL_OK:成功;
  IPANEL_ERR:失败。
********************************************************************************************************/
INT32_T ipanel_porting_demux_disable_filter(UINT32_T channel, UINT32_T filter)
{
    INT32_T     ret = IPANEL_ERR;
    DMX_STATUS  status;

    ipanel_porting_dprintf("[ipanel_porting_demux_disable_filter] channel=%d, filter=%d\n", channel, filter);

    if (channel && filter)
    {
        status = cnxt_dmx_filter_control(gDemuxInstance, --channel, --filter, GEN_DEMUX_DISABLE);
        if (DMX_STATUS_OK != status)
        {
            ipanel_porting_dprintf("[ipanel_porting_demux_disable_filter] disable filter failed(%d)\n", status);
        }
        else
        {
            ret = IPANEL_OK;
        }
    }

    ipanel_porting_dprintf("[ipanel_porting_demux_disable_filter] ret=%d\n", ret);

    return ret;
}

/********************************************************************************************************
功能：对Demux进行一个操作，或者用于设置和获取Demux设备的参数和属性
原型：INT32_T ipanel_porting_demux_ioctl(IPANEL_DEMUX_IOCTL_e op, VOID *arg)
参数说明：
  输入参数：
    op － 操作命令
      typedef enum
      {
        IPANEL_DEMUX_SET_CHANNEL_NOTIFY =1,
        IPANEL_DEMUX_SET_SRC,
        IPANEL_DEMUX_SET_STREAM_TYPE,
        IPANEL_DEMUX_GET_BUFFER,
        IPANEL_DEMUX_PUSH_STREAM,
        IPANEL_DEMUX_STC_FETCH_MODE,
      } IPANEL_DEMUX_IOCTL_e;
    arg - 操作命令所带的参数，当传递枚举型或32位整数值时，arg可强制转换成对应数据类型。
    op, arg取值见下表
    +---------------------+-------------------------+-----------------------------+
    |  op                 |   arg                   |  说明                       |
    +---------------------+-------------------------+-----------------------------+
    | IPANEL_DEMUX_       |未定义                   |按每个channel设置回调函数，是|
    |   SET_CHANNEL_NOTIFY|                         |对全局回调函数的扩展.        |
    +---------------------+-------------------------+-----------------------------+
    | IPANEL_DEMUX_SET_SRC|typedef enum {           |设置demux模块的数据输入来源  |
    |                     | //从芯片硬件接口输入数据|                             |
    |                     | IPANEL_DEMUX_SOURCE_DEVICE=0x10,                      |
    |                     | //系统软件推入的数据    |                             |
    |                     | IPANEL_DEMUX_SOURCE_HOST=0x20                         |
    |                     |} IPANEL_DEMUX_SOURCE_TYPE_e;                          |
    +---------------------+-------------------------+-----------------------------+
    | IPANEL_DEMUX_       |typedef enum {           |设置输入demux设备的流格式    |
    |   SET_STREAM_TYPE   | IPANEL_STREAM_TS,       |                             |
    |                     | IPANEL_STREAM_ES,       |                             |
    |                     | IPANEL_STREAM_PS        |                             |
    |                     |} IPANEL_STREAM_TYPE_e;  |                             |
    +---------------------+-------------------------+-----------------------------+
    | IPANEL_DEMUX_       |指向IPANEL_XMEMBLK结构的 |如果demux管理buffer，接收模块|
    |   GET_BUFFER        |指针,其中的描叙符(pdes)指|可以向demux申请buffer.       |
    |                     |向下面的结构：           |                             |
    |                     |typedef struct {         |                             |
    |                     | IPANEL_XMEM_PAYLOAD_    |                             |
    |                     |         TYPE_e destype; |                             |
    |                     | UINT32_T len;           |                             |
    |                     |} IPANEL_XMEM_GEN_DES;   |                             |
    |                     |IPANEL_XMEMBLK结构定义见PCM模块说明                    |
    +---------------------+-------------------------+-----------------------------+
    | IPANEL_DEMUX_       |指向IPANEL_XMEMBLK结构的 |包含纯媒体流数据             |
    |   PUSH_STREAM       |指针.                    |                             |
    +---------------------+-------------------------+-----------------------------+
    | IPANEL_DEMUX_       |typedef enum {           |                             |
    |   STC_FETCH_MODE    | IPANEL_DEMUX_STC _ONCE_PCR =0x1,                      |
    |                     | IPANEL_DEMUX_STC_FOLLOW_PCR,                          |
    |                     | IPANEL_DEMUX_STC_ONCE_APTS,                           |
    |                     | IPANEL_DEMUX_STC_ONCE_VPTS                            |
    |                     |}IPANEL_SWITCH_e;        |                             |
    +---------------------+-------------------------+-----------------------------+
  输出参数：无
返    回：
  IPANEL_OK：成功
  IPANEL_ERR：失败
********************************************************************************************************/
INT32_T ipanel_porting_demux_ioctl(IPANEL_DEMUX_IOCTL_e op, VOID *arg)
{
    INT32_T ret = IPANEL_ERR;

    ipanel_porting_dprintf("[ipanel_porting_demux_ioctl] op=%d, arg=%p\n", op, arg);

    switch (op)
    {
        case IPANEL_DEMUX_SET_CHANNEL_NOTIFY :
            break;

        case IPANEL_DEMUX_SET_SRC :
            break;

        case IPANEL_DEMUX_SET_STREAM_TYPE :
            break;

        case IPANEL_DEMUX_GET_BUFFER :
            break;

        case IPANEL_DEMUX_PUSH_STREAM :
            break;

        case IPANEL_DEMUX_STC_FETCH_MODE :
            break;

        case IPANEL_DEMUX_STC_SET_TIMEBASE :
            break;

        case IPANEL_DEMUX_GET_CURRENT_PCR :
            break;

        case IPANEL_DEMUX_GET_CURRENT_STC :
            break;

        default :
            break;
    }

    return ret;
}

/*===========================================================================
Description: 分配一个通道对应的buffer

Input: UINT32_T channel : 通道号,UINT32_T *buf buffer区指针，INT32_T len数据长度

Ouput: void

Return: void
============================================================================*/
static INT32_T dmx_allocate_channel_buffer(UINT32_T channel, UINT32_T *buf, INT32_T len)
{
    INT32_T                 ret = IPANEL_ERR;
    ChannelMemoryManager  *pmgr = &m_channel_mem_mgr;

    ipanel_porting_dprintf("[dmx_allocate_channel_buffer] entry\n");

    ipanel_porting_sem_wait(pmgr->SemId, IPANEL_WAIT_FOREVER);
    if (pmgr->used_buf_tail)
    {
        ChannelInfo *curr   = pmgr->used_buf_head;
        ChannelInfo *prev   = pmgr->used_buf_head;
        ChannelInfo *ins    = NULL;
        ChannelInfo *tmp;

        /* 寻找一个最合适的buffer间隙*/
        while (curr)
        {
            if (curr->gap > len)
            {
                if (ins)
                {
                    if (ins != pmgr->used_buf_head)
                    {
                        if (curr->gap < ins->next->gap)
                        {
                            ins = prev;
                        }
                    }
                    else
                    {
                        if (curr->gap < ins->gap)
                        {
                            ins = prev;
                        }
                    }
                }
                else
                {
                    ins = prev;
                }
            }
            else if (curr->gap == len)
            {
                ins = prev;

                break;
            }
            prev = curr;
            curr = curr->next;
        }


        tmp         = pmgr->available_buf_head;
        tmp->ch     = channel;
        tmp->len    = len;
        tmp->gap    = 0;

        pmgr->available_buf_head = pmgr->available_buf_head->next;
        if (!pmgr->available_buf_head)
        {
            pmgr->available_buf_tail = NULL;
        }

        if (ins)   // 找到一个合适的插入点
        {
            if (pmgr->used_buf_head == ins)
            {
                tmp->buf    = pmgr->addr;
                tmp->next   = ins;

                ins->gap = ins->buf - (tmp->buf + tmp->len);

                pmgr->used_buf_head = tmp;

                *buf = tmp->buf;

                ret = IPANEL_OK;
            }
            else
            {
                tmp->buf    = ins->buf + ins->len;
                tmp->next   = ins->next;
                ins->next   = tmp;

                tmp->next->gap = tmp->next->buf - (tmp->buf + tmp->len);

                *buf = tmp->buf;

                ret = IPANEL_OK;
            }
        }
        else    // 只能检查一下最后是否还有剩余的空间
        {
            if ((pmgr->used_buf_tail->buf + pmgr->used_buf_tail->len) <= (pmgr->addr + pmgr->size))
            {
                tmp->buf    = pmgr->used_buf_tail->buf + pmgr->used_buf_tail->len;
                tmp->next   = NULL;

                pmgr->used_buf_tail->next   = tmp;
                pmgr->used_buf_tail         = tmp;

                *buf = tmp->buf;

                ret = IPANEL_OK;
            }
        }
    }
    else //初始状态，从预定的内存中分配 buf 空间
    {
        pmgr->used_buf_head   = pmgr->available_buf_head;
        pmgr->used_buf_tail   = pmgr->available_buf_head;

        pmgr->available_buf_head    = pmgr->available_buf_head->next;

        pmgr->used_buf_tail->ch     = channel;
        pmgr->used_buf_tail->buf    = pmgr->addr;
        pmgr->used_buf_tail->len    = len;
        pmgr->used_buf_tail->next   = NULL;

        *buf = pmgr->addr;

        ret = IPANEL_OK;
    }
    ipanel_porting_sem_release(pmgr->SemId);

    ipanel_porting_dprintf("[dmx_allocate_channel_buffer] leave \n");

    return ret;
}

/*===========================================================================
Description: 释放Demux对应通道的Buffer

Input: UINT32_T channel : 通道号

Ouput: void

Return: void
============================================================================*/
static VOID dmx_free_channel_buffer(UINT32_T channel)
{
    ChannelMemoryManager   *pmgr    = &m_channel_mem_mgr;
    ChannelInfo            *curr    = m_channel_mem_mgr.used_buf_head;
    ChannelInfo            *prev    = NULL;

    ipanel_porting_dprintf("[dmx_free_channel_buffer] entry\n");

    ipanel_porting_sem_wait(pmgr->SemId, IPANEL_WAIT_FOREVER);
    while (curr)
    {
        if (curr->ch == channel)  /*找到对应通道*/
        {
            if (prev)
            {
                prev->next = curr->next;
                if (!prev->next)
                {
                    pmgr->used_buf_tail = prev;
                }
                else
                {
                    prev->next->gap = prev->next->buf - (prev->buf + prev->len);
                }
            }
            else
            {
                if (curr->next)
                {
                    pmgr->used_buf_head      = curr->next;
                    pmgr->used_buf_head->gap = pmgr->used_buf_head->buf - pmgr->addr;
                }
                else
                {
                    pmgr->used_buf_head = NULL;
                    pmgr->used_buf_tail = NULL;
                }
            }

			/*注销该通道*/
            curr->ch    = INVALID_CHANNEL;
            curr->buf   = 0;
            curr->len   = 0;
            curr->gap   = 0;
            curr->next  = NULL;

            if (pmgr->available_buf_tail)
            {
                pmgr->available_buf_tail->next  = curr;
                pmgr->available_buf_tail        = curr;
            }
            else
            {
                pmgr->available_buf_head = curr;
                pmgr->available_buf_tail = curr;
            }

            break;
        }
        prev = curr;
        curr = curr->next;
    }
    
    ipanel_porting_sem_release(pmgr->SemId);

    ipanel_porting_dprintf("[dmx_free_channel_buffer] leave\n");
}

/*===========================================================================
Description: Demux模块Section回调数据注册函数

Input: void

Ouput: pgen_notify_t pNotifyData,回调数据结构指针

Return: genfilter_mode 过滤模式
============================================================================*/
static genfilter_mode demux_section_header_notify(pgen_notify_t pNotifyData)
{
    //u_int8 i;

    //ipanel_porting_dprintf("[demux_section_header_notify] is called!\n");

    /* 去掉包头的数据检验过程 */
#if 0 
    u_int8 fid = 0xff;

    /* 查找一个未用的Filter，最多有32个Filter*/
    for (i = 0; i < 32; i++) 
    {
        if (((pNotifyData->fid_mask >> i) & 0x01) == 0x01)
        {
            fid = i;

            break;
        }
    }

    if (0xff == fid)
    {
        return GENDEMUX_CONTINUOUS/*GENDEMUX_ONE_SHOT*/;
    }

    if (pNotifyData->condition & GENDMX_ERROR)
    {
        pNotifyData->write_ptr  = NULL;
        pNotifyData->length     = 0;

        return GENDEMUX_CONTINUOUS;
    }
#endif

    if (pNotifyData->condition & GENDMX_CHANNEL_TIMEOUT)
    {
        pNotifyData->write_ptr  = NULL;
        pNotifyData->length     = 0;

        return GENDEMUX_ONE_SHOT;
    }

    pNotifyData->write_ptr  = m_section_buffer;
    pNotifyData->skip       = 0;

    return GENDEMUX_CONTINUOUS;
}

/* 是否使用crc32进行检验数据是否正确 */
#define USE_CRC32

#ifdef USE_CRC32
unsigned int crc_table[256] = 
{
	0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc, 0x17c56b6b,
	0x1a864db2, 0x1e475005, 0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
	0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd, 0x4c11db70, 0x48d0c6c7,
	0x4593e01e, 0x4152fda9, 0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
	0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011, 0x791d4014, 0x7ddc5da3,
	0x709f7b7a, 0x745e66cd, 0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
	0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5, 0xbe2b5b58, 0xbaea46ef,
	0xb7a96036, 0xb3687d81, 0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
	0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49, 0xc7361b4c, 0xc3f706fb,
	0xceb42022, 0xca753d95, 0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
	0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d, 0x34867077, 0x30476dc0,
	0x3d044b19, 0x39c556ae, 0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
	0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16, 0x018aeb13, 0x054bf6a4,
	0x0808d07d, 0x0cc9cdca, 0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
	0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02, 0x5e9f46bf, 0x5a5e5b08,
	0x571d7dd1, 0x53dc6066, 0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
	0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e, 0xbfa1b04b, 0xbb60adfc,
	0xb6238b25, 0xb2e29692, 0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
	0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a, 0xe0b41de7, 0xe4750050,
	0xe9362689, 0xedf73b3e, 0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
	0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686, 0xd5b88683, 0xd1799b34,
	0xdc3abded, 0xd8fba05a, 0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
	0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb, 0x4f040d56, 0x4bc510e1,
	0x46863638, 0x42472b8f, 0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
	0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47, 0x36194d42, 0x32d850f5,
	0x3f9b762c, 0x3b5a6b9b, 0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
	0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623, 0xf12f560e, 0xf5ee4bb9,
	0xf8ad6d60, 0xfc6c70d7, 0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
	0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f, 0xc423cd6a, 0xc0e2d0dd,
	0xcda1f604, 0xc960ebb3, 0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
	0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b, 0x9b3660c6, 0x9ff77d71,
	0x92b45ba8, 0x9675461f, 0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
	0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640, 0x4e8ee645, 0x4a4ffbf2,
	0x470cdd2b, 0x43cdc09c, 0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
	0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24, 0x119b4be9, 0x155a565e,
	0x18197087, 0x1cd86d30, 0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
	0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088, 0x2497d08d, 0x2056cd3a,
	0x2d15ebe3, 0x29d4f654, 0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
	0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c, 0xe3a1cbc1, 0xe760d676,
	0xea23f0af, 0xeee2ed18, 0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
	0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0, 0x9abc8bd5, 0x9e7d9662,
	0x933eb0bb, 0x97ffad0c, 0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
	0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4
};

static unsigned int crc32 (char *data, int len)
{
    int i;
    unsigned long crc = 0xffffffff;

    for (i = 0; i < len; i++)
    {
        crc = (crc << 8) ^ crc_table[((crc >> 24) ^ *data) & 0xff];
        data++;
    }

    return crc;
}

#endif

/*===========================================================================
Description: Demux模块Filter数据回调处理函数，由底层任务周期性调用

Input: pgen_notify_t pNotifyData，回调处理数据指针

Ouput: void

Return: genfilter_mode 过滤模式
============================================================================*/
static genfilter_mode demux_section_data_notify(pgen_notify_t pNotifyData)
{
    u_int8  i;
    u_int8  fid = 0xff;
    u_int32 fid_mask;
    //u_int8  *pData;
    //UINT32_T ts,te;

    if (!pNotifyData->write_ptr || !pNotifyData->length || !m_ipanel_dmx_data_func)
    {
        return GENDEMUX_CONTINUOUS;
    }

	/* 根据filterMask 计算出哪个filter 正在接收数据 */
	fid_mask = pNotifyData->fid_mask;

    for (i = 0; i < 32; i++)
    {
        if (((fid_mask >> i) & 0x01) == 0x01)
        {
            fid = i;
            break;
        }
    }

    if (0xff == fid)
    {
        return GENDEMUX_CONTINUOUS;
    }

    if ((GENDMX_SECTION_AVAILABLE + GENDMX_CRC_CHECKED) == pNotifyData->condition)
    {
        //ts = ipanel_porting_time_ms();
        #ifdef USE_CRC32
        unsigned int pid = 0;
        unsigned int oricrc = 0,newcrc = 0;
        unsigned char *ptr = NULL;
        unsigned short sect_len = 0;
        
        pid = pNotifyData->tag;
        ptr = pNotifyData->write_ptr;
        sect_len = pNotifyData->length;
        
        /* not need CRC ---TDT(0x0014 0x70) RST(0x0013 0x71)*/
        if((pid == 0x14 && ptr[0] == 0x70)||
           (pid == 0x13 && ptr[0] == 0x71)||(ptr[0] >0x7F && ptr[0] < 0x90))
        {
            ;
        }
        else
        {
            //oricrc = crc32(ptr, sect_len - 4);
			oricrc = crc32((char*)ptr, sect_len - 4);
            newcrc = (unsigned int)(ptr[sect_len-4] << 24) | 
                     (unsigned int)(ptr[sect_len-3] << 16) | 
                     (unsigned int)(ptr[sect_len-2] << 8)  | 
                     (unsigned int)(ptr[sect_len-1]) ;

            if(oricrc != newcrc)
            {
                printf("CRC value error,lost it!\n");
                return GENDEMUX_CONTINUOUS;
            }
        }
        
        #endif
        
    	/*调用注册的回调函数处理Section中的数据*/
        (m_ipanel_dmx_data_func)( 
                pNotifyData->chid + 1,
                fid + 1,
                pNotifyData->write_ptr,
                pNotifyData->length
            );

        //te = ipanel_porting_time_ms();
            
        //pData = pNotifyData->write_ptr;
        //ipanel_porting_dprintf("0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x (tm=%d)\n",
        //     pData[0],pData[1],pData[2],pData[3],pData[4],pData[5],pData[6],pData[7],te-ts);
    }

    return GENDEMUX_CONTINUOUS;
}

INT32_T ipanel_demux_init(VOID)
{
    DMX_STATUS  status;
    INT32_T     i;

	for(i=0; i<MAX_DMX_CHANNEL_NUMBER;i++)
	{
        m_DmxChannelTable[i].bUse = FALSE;
		m_DmxChannelTable[i].bEnable = FALSE;
		m_DmxChannelTable[i].bOccupied = FALSE;
        m_DmxChannelTable[i].buffer = NULL;
	}

    status = cnxt_dmx_open(FALSE, DMX_CAP_DESCRAMBLE, &gDemuxInstance);
    if (DMX_STATUS_OK != status)
    {
        ipanel_porting_dprintf("[ipanel_demux_init] cnxt_dmx_open error(%d)\n", status);

        goto INIT_FAILED;
    }

	status = cnxt_dmx_descrambler_init(gDemuxInstance);
    if (DMX_STATUS_OK != status)
    {
        ipanel_porting_dprintf("[ipanel_demux_init] cnxt_dmx_descrambler_init error(%d)\n", status);

        goto INIT_FAILED;
    }	

#ifdef IPANEL_PRE_MALLOC_BUFFER
    m_channel_mem_mgr.SemId = ipanel_porting_sem_create("DMXM", 1, IPANEL_TASK_WAIT_FIFO);
    if (IPANEL_NULL == m_channel_mem_mgr.SemId)
    {
        ipanel_porting_dprintf("[ipanel_demux_init] SemId creation failed\n");

        goto INIT_FAILED;
    }

    m_channel_mem_mgr.addr = (UINT32_T)ipanel_porting_malloc(DEMUX_MEMORY_SIZE);
    if (IPANEL_NULL == m_channel_mem_mgr.addr)
    {
        ipanel_porting_dprintf("[ipanel_demux_init] memory allocate failed(0x%x)\n", DEMUX_MEMORY_SIZE);

        goto INIT_FAILED;
    }

    m_channel_mem_mgr.size = DEMUX_MEMORY_SIZE;

    m_channel_mem_mgr.used_buf_head       = NULL;
    m_channel_mem_mgr.used_buf_tail       = NULL;
    m_channel_mem_mgr.available_buf_head  = NULL;
    m_channel_mem_mgr.available_buf_tail  = NULL;

    for (i = 0; i < MAX_PSI_CHANNEL_NUMBER; i++)
    {
        m_channel_mem_mgr.ch_info[i].ch   = INVALID_CHANNEL;
        m_channel_mem_mgr.ch_info[i].buf  = 0;
        m_channel_mem_mgr.ch_info[i].len  = 0;
        m_channel_mem_mgr.ch_info[i].gap  = 0;
        m_channel_mem_mgr.ch_info[i].next = NULL;

        if (m_channel_mem_mgr.available_buf_tail)
        {
            m_channel_mem_mgr.available_buf_tail->next = &m_channel_mem_mgr.ch_info[i];
            m_channel_mem_mgr.available_buf_tail       = &m_channel_mem_mgr.ch_info[i];
        }
        else
        {
            m_channel_mem_mgr.available_buf_head  = &m_channel_mem_mgr.ch_info[i];
            m_channel_mem_mgr.available_buf_tail  = &m_channel_mem_mgr.ch_info[i];
        }
    }
#endif

    return IPANEL_OK;

INIT_FAILED:
    return IPANEL_ERR;
}

void ipanel_demux_exit(VOID)
{
	int i;

    if (NULL != m_channel_mem_mgr.SemId)
    {
	    ipanel_porting_sem_destroy(m_channel_mem_mgr.SemId);
		m_channel_mem_mgr.SemId = NULL;
    }

#ifdef IPANEL_PRE_MALLOC_BUFFER
	for(i=0; i<MAX_DMX_CHANNEL_NUMBER; i++)
	{
		// 强制将channel释放干净
		if(m_DmxChannelTable[i].bEnable)
		{
			ipanel_porting_dprintf("ipanel_demux_exit:Disable dmx channel [%d] before exit!\n");
			ipanel_porting_demux_stop_channel(i+1);
			ipanel_porting_demux_destroy_channel(i+1);
		}
	}

	if (NULL != m_channel_mem_mgr.addr)
	{
		ipanel_porting_free((void *)m_channel_mem_mgr.addr); /*释放内存资源*/
		m_channel_mem_mgr.addr = NULL;			
	}
#endif

}

