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
    
	/* ���ͨ���ոչرգ����ٴ�������Ҫ�Եȣ��������ȴ�����.(0.1��) */
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
            
			/* һ��СС����ʱ��Ч��������:) */
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

    // ֻ��Ҫ����һ��
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

			if( IPANEL_MSG_DESTROY == mixMsg.q2ndWordOfMsg) // �˳�����
			{
				; //��ʱ�������¼�û�д���
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
����˵����
    ��һ��PCM�����豸ʵ����Ϊ�˽����Դ��ͻ���⣬��Լ�����ٶ�ֻ����һ��Ӳ���豸���������
    ���Ѵ���ʵ�����Զ�ռ��ʽ��IPANEL_DEV_USE_EXCUSIVE����ʱ�����������д򿪲��������ɹ�
    ����ǰ��ʵ�������Թ���ʽ��IPANEL_DEV_USE_SHARED����ʱ�������ٴ򿪹���ʵ���Ͷ�ռʵ����������ж��ǹ���ʵ���������ȳ���ʽ���ţ������ʵ���Ƕ�ռ��ʽ�򿪣�����ǰ���й���ʵ�����������������������

����˵����
���������
    mode�� ʹ���豸�ķ�ʽ��
    typedef enum 
    {
     IPANEL_DEV_USE_SHARED, //�������û�����ʹ���豸
     IPANEL_DEV_USE_EXCUSIVE,   //��ռʹ���豸
    }IPANEL_DEV_USE_MODE;

    func�� �û�ʵ�ֵ��¼�֪ͨ����ָ�룬�û�Ҳ���Բ�����ָ�룬��ʱfunc������IPANEL_NULL��0����
    VOID (*IPANEL_AUDIO_MIXER_NOTIFY)(UINT32_T hmixer, IPANEL_AUDIO_MIXER_ENENT pcm_event, UINT32_T *param)
    ��������:
    �˺������������Žӿڵ��û�ʵ�֣��ڴ򿪺ϳ���ʵ����ʱ�򴫸��ײ�ģ�飬���ײ�ģ�鴦����һ�����ݿ�򲥷Ż�������������ʱ֪ͨ�ϲ�ģ�鴦��
    �������:
    handler �� �ϳ���ʵ���ľ�����ڶ�ʵ�������������������Ϣ��Դ������ͨ�����ҵ���Ӧ���û���
    pcm_event �� ֪ͨ��Ϣ�����͡�
    ttypedef enum
    {
    //ĳ�����ݿ��е������Ѿ��������
     IPANEL_AUDIO_DATA_CONSUMED,
    //���е����ݴ�����ϣ��������ϻ���µ����ݾͻᵼ������ֹͣ
     IPANEL_AUDIO_DATA_LACK
    } IPANEL_AUDIO_MIXER_EVENT;
    param �� ÿ���¼���Ӧ�Ĳ���ָ�룬���²�ģ���Э�̶��塣
    AUDIO_DATA_CONSUMED��paramָ����������ݿ��������������ݿ���������ڵײ��ͷŵ�����paramΪ��ָ�롣
    AUDIO_DATA_LACK��paramΪ��ָ��

�������: ��
����ֵ:��

�����������

�� �أ�
����IPANEL_NULL���ɹ���PCM�����豸��Ԫ�����
����IPANEL_NULL��ʧ��
*******************************************************************************/
UINT32_T ipanel_porting_audio_mixer_open(IPANEL_DEV_USE_MODE mode, IPANEL_AUDIO_MIXER_NOTIFY func)
{
	ipanel_porting_dprintf("[ipanel_porting_audio_mixer_open] mode=%d, func=0x%x \n",mode,func);

	snd_context->func = func;

    return (UINT32_T)snd_context;
}

/*******************************************************************************
����˵����
���ر�ͨ��ipanel_porting_audio_mixer_open�򿪵�PCM�����豸ʵ����ͬʱҪ�ͷſ��ܴ��ڵĴ��������ݿ顣

����˵����
	���������
		handle�� PCM�����豸ʵ�������
	�����������

�� �أ�
IPANEL_OK: ����ִ�гɹ�;
IPANEL_ERR: ����ִ��ʧ�ܡ�
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
����˵������PCM�����豸��ȡ�ڴ��

����˵����
	���������
		handle��PCM�����豸ʵ����� ��������
		size�� ��Ҫ���ڴ���С��
	�����������

�� �أ�
!=IPANEL_NULL:����IPANEL_XMEMBLK�ṹ���ײ������ڴ�ָ���������ṹ�з��أ�
==IPANEL_NULL:ʧ��
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
����˵����
��PCM�����豸ʵ�����������Ƶ���ݣ��ú��������������ء�IPANEL_XMEMBLK������ʵ�ʵ����ݡ�����ʵ�ʲ��ŵĽ���/�߳�����������ݡ�

����˵����
	���������
		handle �CPCM�����豸ʵ�����
		pcmblk - IPANEL_XMEMBLK�ṹ���ǰҳ��
	�����������

�� �أ�
IPANEL_OK: ����ִ�гɹ�;
IPANEL_ERR: ����ִ��ʧ�ܡ�
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
����˵����
  ��PCM�����豸ʵ���������һ�������������������úͻ�ȡPCM�����豸ʵ������Ĳ��������ԡ�

����˵����
  ���������
    handle �CPCM�����豸ʵ�����
    op �� �������� 
    typedef enum
    {
     IPANEL_MIXER_SET_VOLUME =1,
     IPANEL_MIXER_CLEAR_BUFFER
    } IPANEL_MIXER_IOCTL_e;

    arg �C �������������Ĳ�����������ö���ͻ�32λ����ֵʱ��arg��ǿ��ת���ɶ�Ӧ�������͡�

    op��arg��ȡֵ��ϵ���±�
  
    +---------------------+-------------------------+------------------------------+
    |  op                 |   arg                   |  ˵��                        |
    +---------------------+-------------------------+------------------------------+
    | IPANEL_MIXER_       |0��100����ֵ             |������������ͽ����������Ƶ��|
    |   SET_VOLUME        |                         |��ͱ�����0��С��100���      |
    +---------------------+-------------------------+------------------------------+
    | IPANEL_MIXER_       |IPANEL_NULL              |��ղ����豸������������      |
    |   CLEAR_BUFFER      |                         |                              |
    +---------------------+-------------------------+------------------------------+
���������
  ��

�� �أ�
  IPANEL_OK: ����ִ�гɹ�;
  IPANEL_ERR: ����ִ��ʧ�ܡ�
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

