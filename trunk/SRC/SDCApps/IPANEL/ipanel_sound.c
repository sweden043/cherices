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
#include "ipanel_task.h"
#include "ipanel_sound.h"

static pIPANEL_XMEMBLK ipanel_mix_get_blk(INT32_T len);
static void ipanel_mix_free_blk(VOID *pBlk);

static IPANEL_SOUND_CONTEXT_s * snd_context = IPANEL_NULL;

static INT32_T ipanel_mix_set_volume(UINT32_T volume)
{
	INT32_T ret = IPANEL_ERR ; 
	PCM_STATUS pcmStat ; 
	
	ipanel_porting_dprintf("[ipanel_mix_set_volume] set volume = %d\n", volume);

	pcmStat = cnxt_pcm_set_volume(volume,volume);
	if( pcmStat != PCM_ERROR_OK)
	{
		ipanel_porting_dprintf("[ipanel_mix_set_volume] set pcm volume failed!\n");
		return ret ; 
	}

	pcmStat = cnxt_pcm_set_volume_mpeg(volume,volume);
	if( pcmStat != PCM_ERROR_OK)
	{
		ipanel_porting_dprintf("[ipanel_mix_set_volume] set mpeg volume failed!\n");
		return ret ; 
	}

	return IPANEL_OK;		
}

INT32_T ipanel_pcm_set_format(IPANEL_PCMDES *pcmDes)
{
    int i = 0;
    PCM_STATUS error_code;
	cnxt_pcm_sample_rate_t sample_rate ;
	cnxt_pcm_bits_t        sample_bits ; 
	cnxt_pcm_format_t 	   ipanel_ps_format ;
    bool                   bstereo = FALSE ;

    ipanel_porting_dprintf("[ipanel_pcm_set_format] PCM Format:%d,%d,%d.\n",
                           pcmDes->samplerate,pcmDes->channelnum,pcmDes->bitspersample);

	switch(pcmDes->samplerate)
	{
		case 8000:
			sample_rate = SAMPLE_RATE_8K ;
			break;
		case 11025:
			sample_rate = SAMPLE_RATE_11K ;
			break;
		case 16000:
			sample_rate = SAMPLE_RATE_16K ;
			break;
		case 22050:
			sample_rate = SAMPLE_RATE_22K ;
			break;
		case 24000:
			sample_rate = SAMPLE_RATE_24K ;
			break;
		case 32000:
			sample_rate = SAMPLE_RATE_32K ;
			break;
		case 44100:
			sample_rate = SAMPLE_RATE_44K ;
			break;
		case 48000:
			sample_rate = SAMPLE_RATE_48K ;
			break;
		default:
			ipanel_porting_dprintf("[ipanel_pcm_set_format]Invalid sample rate !\n");
			break;
	}

	switch(pcmDes->bitspersample)
	{
		case 8:
			sample_bits = BITS_PER_SAMPLE_8 ; 
			break; 
		case 12:
			sample_bits = BITS_PER_SAMPLE_12 ; 
			break; 
		case 16:
			sample_bits = BITS_PER_SAMPLE_16 ; 
			break; 
		case 20:
			sample_bits = BITS_PER_SAMPLE_20 ; 
			break; 
		case 24:
			sample_bits = BITS_PER_SAMPLE_24 ; 
			break; 
		default:
			ipanel_porting_dprintf("[ipanel_pcm_set_format]Invalid per sample bits !\n");
			break;
	}

	if(pcmDes->channelnum == 1)
		bstereo = FALSE ;
	else if( pcmDes->channelnum == 2)
		bstereo = TRUE ;
	else
		ipanel_porting_dprintf("[ipanel_pcm_set_format]Invalid channels !\n");			

	ipanel_ps_format.b_signed = 0;
	ipanel_ps_format.b_stereo = bstereo ;
	ipanel_ps_format.e_rate   = sample_rate ;
	ipanel_ps_format.e_bps    = sample_bits  ;
	ipanel_ps_format.e_mpeg_level = MPEG_LEVEL_HALF ;

	error_code = cnxt_pcm_start(&ipanel_ps_format);
    
	/* 如果通道刚刚关闭，则再次启动需要稍等，这里做等待处理.(0.1秒) */
    while(error_code !=  PCM_ERROR_OK && i < 10)
    {
        task_time_sleep(10);
        error_code = cnxt_pcm_start(&ipanel_ps_format);
        if(PCM_ERROR_OK == error_code)
            break;
        
        i++;
    }
	
	if(error_code !=  PCM_ERROR_OK)
	{
		ipanel_porting_dprintf("[audio_output_init] cnxt_pcm_start is error!\n");
		return IPANEL_ERR;
	}
    
    return IPANEL_OK;
}

INT32_T ipanel_pcm_output_frame(unsigned char *buffer, int len)
{
	unsigned long  ulBytesCopied;      
	unsigned long  ulBytesConsumed;    
	unsigned char *pData;             
	PCM_STATUS rc ;

	ipanel_porting_dprintf("[ipanel_pcm_output_frame] buff=0x%x,len=0x%x.\n",buffer,len);

	ulBytesCopied = len ;
	pData  = (unsigned char*)buffer ;
	ulBytesConsumed = 0;

	// Send the new buffer to the driver
	while (ulBytesCopied)
	{
	   	rc = cnxt_pcm_write_data( (void *)pData, ulBytesCopied, &ulBytesConsumed);
	  	if (rc == PCM_ERROR_OK)
	   	{
	       	ulBytesCopied -= ulBytesConsumed;
	       	pData += ulBytesConsumed;
            
			/* 一个小小的延时，效果还不错:) */
			task_time_sleep(5);	
	     } 
	     else 
	     {
	       	ipanel_porting_dprintf("[ipanel_pcm_output_frame] Write data to PCM driver failed!rc = %d\n",(u_int32) rc);		   
	     } 
  	}
	
	return ulBytesConsumed;
}

static void ipanel_physical_send(IPANEL_XMEMBLK* pblk)
{
 	pIPANEL_PCMDES pcm_des = NULL;

	pcm_des = (pIPANEL_PCMDES)pblk->pdes;

    // 只需要设置一次
    if(snd_context->play_set_flag ==0)
    {                    
        snd_context->play_set_flag = 1;
        ipanel_pcm_set_format(pcm_des);
    }

	ipanel_pcm_output_frame((unsigned char*)pblk->pbuf,pblk->len);
}

static PCM_STATUS pcm_process_pcm_data_request()
{
	return PCM_ERROR_OK;
}

static void ipanel_physical_play(void* param)
{
	INT32_T  ret ; 
	UINT32_T hMixHandle ; 
	IPANEL_QUEUE_MESSAGE	mixMsg;
	
	while(1)
	{
		ret = ipanel_porting_queue_recv(snd_context->PlayQue, &mixMsg,IPANEL_WAIT_FOREVER);
		if( IPANEL_OK == ret )
		{
			hMixHandle = mixMsg.q1stWordOfMsg ; 

			if( IPANEL_MSG_DESTROY == mixMsg.q2ndWordOfMsg) // 退出播放
			{
				; //暂时对这种事件没有处理
			}
			else if( IPANEL_MSG_DATA == mixMsg.q2ndWordOfMsg)
			{
				pIPANEL_XMEMBLK plocal;

				/*play pcm data here*/
				plocal = (pIPANEL_XMEMBLK)mixMsg.q3rdWordOfMsg;
				ipanel_physical_send(plocal);
				if(snd_context->func)
					snd_context->func(0,IPANEL_AUDIO_DATA_CONSUMED,(UINT32_T*)plocal);
			}
		} 	
	}
}

static pIPANEL_XMEMBLK ipanel_mix_get_blk(INT32_T len)
{
	pIPANEL_XMEMBLK pblk;
	
#ifdef USE_ONE_BLOCK	
	unsigned int p;

	pblk = ipanel_porting_malloc(sizeof(IPANEL_XMEMBLK) + sizeof(IPANEL_PCMDES)+len + 8);
	if(IPANEL_NULL == pblk)
		return IPANEL_NULL;
	
	p = ((unsigned int)pblk + sizeof(IPANEL_XMEMBLK) + 3) & 0xfffffffc;
	pblk->pdes = (void *)p;
	p = (p + sizeof(IPANEL_PCMDES) + 3) & 0xfffffffc;
	pblk->pbuf = (UINT32_T *)p;
	pblk->pfree = ipanel_mix_free_blk;
#else
	pblk =ipanel_porting_malloc(sizeof(IPANEL_XMEMBLK));
	if(IPANEL_NULL == pblk)
		return IPANEL_NULL;
	
	pblk->pdes = ipanel_porting_malloc(sizeof(IPANEL_PCMDES));
	if(IPANEL_NULL == pblk->pdes)
	{
		ipanel_porting_free(pblk);
		return IPANEL_NULL;
	}
	
	pblk->pbuf = ipanel_porting_malloc(len);
	if(IPANEL_NULL == pblk->pdes)
	{
		ipanel_porting_free(pblk->pdes);
		ipanel_porting_free(pblk);
		return IPANEL_NULL;
	}

	pblk->pfree = ipanel_mix_free_blk;
#endif

	pblk->len = len;
	return pblk;
}

static void ipanel_mix_free_blk(VOID *pBlk)
{
	pIPANEL_XMEMBLK pblk = (pIPANEL_XMEMBLK)pBlk;

#ifdef USE_ONE_BLOCK	
	ipanel_porting_free(pblk);
#else
	ipanel_porting_free(pblk->pdes);
	ipanel_porting_free(pblk->pbuf);
	ipanel_porting_free(pblk);
#endif
}

/*******************************************************************************
功能说明：
    打开一个PCM播放设备实例，为了解决资源冲突问题，特约定（假定只存在一个硬件设备的情况）：
    当已存在实例是以独占方式（IPANEL_DEV_USE_EXCUSIVE）打开时，后续的所有打开操作都不成功
    当以前的实例都是以共享方式（IPANEL_DEV_USE_SHARED）打开时，可以再打开共享实例和独占实例。如果所有都是共享实例则按先入先出方式播放，如果新实例是独占方式打开，则以前所有共享实例的输出将被丢弃或阻塞。

参数说明：
输入参数：
    mode： 使用设备的方式。
    typedef enum 
    {
     IPANEL_DEV_USE_SHARED, //和其他用户共享使用设备
     IPANEL_DEV_USE_EXCUSIVE,   //独占使用设备
    }IPANEL_DEV_USE_MODE;

    func： 用户实现的事件通知函数指针，用户也可以不传入指针，这时func必须是IPANEL_NULL（0）。
    VOID (*IPANEL_AUDIO_MIXER_NOTIFY)(UINT32_T hmixer, IPANEL_AUDIO_MIXER_ENENT pcm_event, UINT32_T *param)
    函数描述:
    此函数由声音播放接口的用户实现，在打开合成器实例的时候传给底层模块，当底层模块处理完一个数据块或播放缓冲区出现下溢时通知上层模块处理。
    输入参数:
    handler － 合成器实例的句柄，在多实例的情况下用来区分消息来源，并能通过它找到相应的用户。
    pcm_event － 通知消息的类型。
    ttypedef enum
    {
    //某个数据块中的数据已经处理完毕
     IPANEL_AUDIO_DATA_CONSUMED,
    //所有的数据处理完毕，不能马上获得新的数据就会导致声音停止
     IPANEL_AUDIO_DATA_LACK
    } IPANEL_AUDIO_MIXER_EVENT;
    param － 每类事件对应的参数指针，上下层模块可协商定义。
    AUDIO_DATA_CONSUMED：param指向处理完的数据块描叙符，如果数据块描叙符，在底层释放掉，则param为空指针。
    AUDIO_DATA_LACK：param为空指针

输出参数: 无
返回值:无

输出参数：无

返 回：
！＝IPANEL_NULL：成功，PCM播放设备单元句柄；
＝＝IPANEL_NULL：失败
*******************************************************************************/
UINT32_T ipanel_porting_audio_mixer_open(IPANEL_DEV_USE_MODE mode, IPANEL_AUDIO_MIXER_NOTIFY func)
{
	ipanel_porting_dprintf("[ipanel_porting_audio_mixer_open] mode=%d, func=0x%x \n",mode,func);

	snd_context->func = func;

    return (UINT32_T)snd_context;
}

/*******************************************************************************
功能说明：
　关闭通过ipanel_porting_audio_mixer_open打开的PCM播放设备实例，同时要释放可能存在的待处理数据块。

参数说明：
	输入参数：
		handle： PCM播放设备实例句柄。
	输出参数：无

返 回：
IPANEL_OK: 函数执行成功;
IPANEL_ERR: 函数执行失败。
*******************************************************************************/
INT32_T ipanel_porting_audio_mixer_close(UINT32_T handle)
{
    PCM_STATUS error_code;

    error_code = cnxt_pcm_stop();
    if(PCM_ERROR_OK != error_code)
    {
        ipanel_porting_dprintf("[ipanel_porting_audio_mixer_close]cnxt_pcm_stop failed!\n");
        return IPANEL_ERR;
    }
    
    return IPANEL_OK;
}

/*******************************************************************************
功能说明：从PCM播放设备获取内存块

参数说明：
	输入参数：
		handle：PCM播放设备实例句柄 　　　　
		size： 需要的内存块大小。
	输出参数：无

返 回：
!=IPANEL_NULL:返回IPANEL_XMEMBLK结构，底层分配的内存指针放在这个结构中返回；
==IPANEL_NULL:失败
*******************************************************************************/
IPANEL_XMEMBLK *ipanel_porting_audio_mixer_memblk_get(UINT32_T handle, UINT32_T size)
{
	pIPANEL_XMEMBLK pointer = IPANEL_NULL;
	
	if(size != 0 )
	{
		pointer = ipanel_mix_get_blk(size);
	}
	
	return pointer;
}

/*******************************************************************************
功能说明：
向PCM播放设备实例句柄发送音频数据，该函数必须立即返回。IPANEL_XMEMBLK包含有实际的数据。而由实际播放的进程/线程来处理该数据。

参数说明：
	输入参数：
		handle –PCM播放设备实例句柄
		pcmblk - IPANEL_XMEMBLK结构体见前页：
	输出参数：无

返 回：
IPANEL_OK: 函数执行成功;
IPANEL_ERR: 函数执行失败。
*******************************************************************************/
INT32_T ipanel_porting_audio_mixer_memblk_send(UINT32_T handle, IPANEL_XMEMBLK *pcmblk)
{
	INT32_T ret = IPANEL_ERR;
	IPANEL_QUEUE_MESSAGE mixMsg ; 
	IPANEL_SOUND_CONTEXT_s* cxt = (IPANEL_SOUND_CONTEXT_s*)handle;

    ipanel_porting_dprintf("[ipanel_porting_audio_mixer_memblk_send] is called!\n");

	mixMsg.q1stWordOfMsg = handle ; 
	mixMsg.q2ndWordOfMsg = IPANEL_MSG_DATA ;
	mixMsg.q3rdWordOfMsg = (unsigned int)pcmblk ;
	mixMsg.q4thWordOfMsg = 0 ;

	ret = ipanel_porting_queue_send(cxt->PlayQue, &mixMsg);
	if( IPANEL_ERR == ret)
	{
		ipanel_porting_dprintf("[ipanel_porting_aduio_mixer_memblk_send] send data failed!\n");
		return ret;
	}

    return IPANEL_OK;
}

/********************************************************************************************************
功能说明：
  对PCM播放设备实例句柄进行一个操作，或者用于设置和获取PCM播放设备实例句柄的参数和属性。

参数说明：
  输入参数：
    handle –PCM播放设备实例句柄
    op － 操作命令 
    typedef enum
    {
     IPANEL_MIXER_SET_VOLUME =1,
     IPANEL_MIXER_CLEAR_BUFFER
    } IPANEL_MIXER_IOCTL_e;

    arg – 操作命令所带的参数，当传递枚举型或32位整数值时，arg可强制转换成对应数据类型。

    op和arg的取值关系见下表：
  
    +---------------------+-------------------------+------------------------------+
    |  op                 |   arg                   |  说明                        |
    +---------------------+-------------------------+------------------------------+
    | IPANEL_MIXER_       |0～100整数值             |设置输出音量和解码器输出音频的|
    |   SET_VOLUME        |                         |混和比例，0最小，100最大      |
    +---------------------+-------------------------+------------------------------+
    | IPANEL_MIXER_       |IPANEL_NULL              |清空播放设备缓冲区的数据      |
    |   CLEAR_BUFFER      |                         |                              |
    +---------------------+-------------------------+------------------------------+
输出参数：
  无

返 回：
  IPANEL_OK: 函数执行成功;
  IPANEL_ERR: 函数执行失败。
********************************************************************************************************/
INT32_T ipanel_porting_audio_mixer_ioctl(UINT32_T handle, IPANEL_MIXER_IOCTL_e op, VOID *arg)
{
    INT32_T ret = IPANEL_OK;
	PCM_STATUS error_code;
	INT32_T oparg = (INT32_T)arg;

    ipanel_porting_dprintf("[ipanel_porting_audio_mixer_ioctl] handle=0x%x, op=%d, arg=%p\n", handle, op, arg);

    switch (op)
    {
        case IPANEL_MIXER_SET_VOLUME:
			ret = ipanel_mix_set_volume(oparg);
            break;

        case IPANEL_MIXER_CLEAR_BUFFER :
            break;

        case IPANEL_MIXER_PAUSE  :
			error_code = cnxt_pcm_pause();
			if( PCM_ERROR_OK != error_code)
			{
				ipanel_porting_dprintf("[ipanel_porting_audio_mixer_ioctl] cnxt_pcm_pause failed!\n");
				ret = IPANEL_ERR;
			}
            break;

        case IPANEL_MIXER_RESUME :			
			error_code = cnxt_pcm_resume();
			if( PCM_ERROR_OK != error_code)
			{
				ipanel_porting_dprintf("[ipanel_porting_audio_mixer_ioctl] cnxt_pcm_pause failed!\n");
				ret = IPANEL_ERR;
			}
            break;

        default :
			ipanel_porting_dprintf("[ipanel_porting_audio_mixer_ioctl] error parameter!\n");
            ret = IPANEL_ERR;
    }

    return ret;
}

UINT32_T ipanel_porting_mic_open(IPANEL_MIC_NOTIFY func)
{
	UINT32_T handle = IPANEL_NULL ; 

	return 1 ;
}

INT32_T ipanel_porting_mic_close(UINT32_T handle)
{
	INT32_T ret = IPANEL_OK ;

	return ret ; 
}


INT32_T ipanel_porting_mic_ioctl(UINT32_T handle, IPANEL_MIC_IOCTL_e op, VOID *arg)
{
	INT32_T ret = IPANEL_OK ;

	return ret ; 
}

INT32_T ipanel_porting_mic_read(UINT32_T handle, BYTE_T *buf)
{
	INT32_T ret = IPANEL_OK ;

	return ret ; 
}

int ipanel_sound_init()
{
	PCM_STATUS errCode ;

	errCode = cnxt_pcm_set_volume(CNXT_PCM_MAX_VOLUME, CNXT_PCM_MAX_VOLUME);
	if (errCode != PCM_ERROR_OK)
	{
		ipanel_porting_dprintf("[ipanel_sound_init] cnxt_pcm_set_volume failed: rc = %ld!\n", (u_int32)errCode);
		goto INIT_FAIL;
	} 

	errCode = cnxt_pcm_set_volume_mpeg(CNXT_PCM_MAX_VOLUME, CNXT_PCM_MAX_VOLUME);
	if (errCode!= PCM_ERROR_OK)
	{
		ipanel_porting_dprintf("[ipanel_sound_init] cnxt_pcm_set_volume_mpeg failed: rc = %ld!\n", (u_int32)errCode);
		goto INIT_FAIL;
	} 

   	errCode = cnxt_pcm_data_req_fcn_register(pcm_process_pcm_data_request);
  	if (errCode != PCM_ERROR_OK)
	{
	 	ipanel_porting_dprintf("[ipanel_sound_init]cnxt_pcm_register_callback: rc = %ld!\n", (u_int32)errCode);
 	}

	errCode = cnxt_pcm_set_thresholds(320,320);
	if( errCode != PCM_ERROR_OK)
	{
		ipanel_porting_dprintf("[ipanel_sound_init]cnxt_pcm_set_thresholds failed!\n ");
		goto INIT_FAIL;
	}

	if(snd_context != IPANEL_NULL)
		goto INIT_FAIL;
	
	snd_context = (IPANEL_SOUND_CONTEXT_s*)ipanel_porting_malloc(sizeof(IPANEL_SOUND_CONTEXT_s));
	if(snd_context == IPANEL_NULL)
		return IPANEL_ERR;
	
	memset(snd_context,0,sizeof(IPANEL_SOUND_CONTEXT_s));
	snd_context->PlayQue = ipanel_porting_queue_create(NULL, 
									     IPANEL_SOUND_PLAYS_COUNT, 
									     IPANEL_TASK_WAIT_FIFO);
	if( IPANEL_NULL == snd_context->PlayQue )
	{
		ipanel_porting_dprintf("[ipanel_sound_init] create play queue failed!\n");
		goto INIT_FAIL;
	}	
	
	snd_context->taskId =ipanel_porting_task_create(
		            	                          IPANEL_SOUND_TASK_NAME,
		            			 			      ipanel_physical_play, 
		            			 			      (void *)NULL,
		            			 			      IPANEL_SOUND_TASK_PRIORITY,
		            			 			      IPANEL_SOUND_TASK_STACK_SIZE			 			      
		            			 			      );

	snd_context->play_set_flag = 0 ;
		
	if( IPANEL_NULL ==  snd_context->taskId )
	{
		ipanel_porting_dprintf("[ipanel_sound_init] create sound task failed! \n ");
		goto INIT_FAIL;
	}

	return  IPANEL_OK;
	
INIT_FAIL:
	if(snd_context)
	{
		if(snd_context->PlayQue)
		{
			ipanel_porting_queue_destroy(snd_context->PlayQue);
			snd_context->PlayQue = IPANEL_NULL;
		}

		if(snd_context->taskId)
		{
			ipanel_porting_task_destroy(snd_context->taskId);
			snd_context->taskId = IPANEL_NULL;
		}

		ipanel_porting_free(snd_context);
		snd_context = IPANEL_NULL;
	}
	
	return  IPANEL_ERR;	
}

void ipanel_sound_exit()
{
	ipanel_porting_dprintf("[ipanel_sound_exit] is called\n");

	if(snd_context)
	{
		if(snd_context->PlayQue)
		{
			ipanel_porting_queue_destroy(snd_context->PlayQue);
			snd_context->PlayQue = IPANEL_NULL;
		}

		if(snd_context->taskId)
		{
			ipanel_porting_task_destroy(snd_context->taskId);
			snd_context->taskId = IPANEL_NULL;
		}

		ipanel_porting_free(snd_context);
		snd_context = IPANEL_NULL;

		snd_context->play_set_flag = 0 ;
	}
}

