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
#include "ipanel_dsm.h"

extern u_int32 gDemuxInstance;

/*******************************************************************************
����һ��������ͨ����������ָ���趨����ģʽ��
input:	encryptmode������ģʽ����TS��PES
output:	None
return:	> 0 �ɹ��� ���ط���Ľ������ľ��; 0 ʧ�ܡ�
*******************************************************************************/
UINT32_T ipanel_porting_descrambler_allocate(IPANEL_ENCRY_MODE_e mode)
{
	UINT32_T handle = IPANEL_NULL;
	DMX_STATUS status ; 
	u_int32 deschandle;
	static unsigned int av_channel = AUDIO_CHANNEL ;   // Video : 1  Audio: 2  

	// NOTE: �ڴ˼ٶ����ȴ�������Audio Channel �ٴ���Video Channel !!!
	if( av_channel == AUDIO_CHANNEL) 
	{
		status = cnxt_dmx_descrambler_open(gDemuxInstance, AUDIO_CHANNEL, &deschandle);
		if ( DMX_STATUS_OK != status)
		{
			ipanel_porting_dprintf("[ipanel_porting_descrambler_allocate] allocate descrambler handle failed! \n");
			return handle;
		}
		av_channel = VIDEO_CHANNEL ;
	}
	else
	{
		status = cnxt_dmx_descrambler_open(gDemuxInstance, VIDEO_CHANNEL, &deschandle);
		if ( DMX_STATUS_OK != status)
		{
			ipanel_porting_dprintf("[ipanel_porting_descrambler_allocate] allocate descrambler handle failed! \n");
			return handle;
		}
		av_channel = AUDIO_CHANNEL ;
	}

	handle = deschandle ;
	
 	return handle;
}

/*******************************************************************************
�ͷ�һ��ͨ��ipanel_porting_descrambler_allocate����Ľ�����ͨ����
input:	deschandle:�������ľ��
output:	None
return:	>= 0 �ɹ�, < 0 ʧ��
*******************************************************************************/
INT32_T ipanel_porting_descrambler_free(UINT32_T handle)
{
    INT32_T ret = IPANEL_ERR;
	DMX_STATUS status ;

    ipanel_porting_dprintf("[ipanel_porting_descrambler_free] handle=0x%x\n", handle);

    if (handle)
    {
		status = cnxt_dmx_descrambler_close(gDemuxInstance, handle);
		if( DMX_STATUS_OK != status)
		{
			ipanel_porting_dprintf("[ipanel_porting_descrambler_free] free descrambler handle failed! \n");
			return ret ;
		}

		ret = IPANEL_OK;
    }

    return ret;
}

/*******************************************************************************
���ý�����ͨ��PID
input:	deschandle:�������ľ��
		pid:��Ҫ���ŵ�����PID(AudioPid,VideoPid)
output:	None
return:	>= 0 �ɹ�; < 0 ʧ��
*******************************************************************************/
INT32_T ipanel_porting_descrambler_set_pid(UINT32_T handle, UINT16_T pid)
{
    INT32_T ret = IPANEL_ERR;
	DMX_STATUS status ; 

    ipanel_porting_dprintf("[ipanel_porting_descrambler_set_pid] handle=0x%x, pid=0x%x\n", handle, pid);

    if (handle)
    {
		status = cnxt_dmx_descrambler_control(gDemuxInstance, handle,GEN_DEMUX_ENABLE);
		if( DMX_STATUS_OK != status) 
		{
			ipanel_porting_dprintf("[ipanel_porting_descrambler_set_pid] enable descrambling failed!\n");
			return ret;
		}		

		ret = IPANEL_OK;
    }

    return ret;
}

/*******************************************************************************
����������֡�
input:	deschandle:�������ľ��
		key:�����ֵ�ֵ
		keylen:�����ֳ���
output:	None
return:	>= 0 �ɹ�; < 0 ʧ��
*******************************************************************************/
INT32_T ipanel_porting_descrambler_set_oddkey(UINT32_T handle, BYTE_T *key, INT32_T len)
{
	int i;
    INT32_T ret = IPANEL_ERR;
	DMX_STATUS status ;

    ipanel_porting_dprintf("[ipanel_porting_descrambler_set_oddkey] handle=0x%x, key=%p, len=%d\n",\
        				   handle, key, len);

	for(i=0;i<len;i++)
	{
		ipanel_porting_dprintf(" 0x%02x ",key[i]);
	}
	ipanel_porting_dprintf("\n\n");

    if (handle && key && (len > 0))
    {
		status = cnxt_dmx_descrambler_set_odd_keys(gDemuxInstance, handle, len, (char*)key);
		if ( DMX_STATUS_OK != status)
		{
			ipanel_porting_dprintf("[ipanel_porting_descrambler_set_oddkey] descrable set odd key failed!\n");
			return IPANEL_ERR ;
		}

		ret = IPANEL_OK;
    }

    return ret;
}

/*******************************************************************************
����ż�����֡�
input:	deschandle:�������ľ��
		key:�����ֵ�ֵ
		keylen:�����ֳ���
output:	None
return:	>= 0 �ɹ�; < 0 ʧ��
*******************************************************************************/
INT32_T ipanel_porting_descrambler_set_evenkey(UINT32_T handle, BYTE_T *key, INT32_T len)
{
	int i;
    INT32_T ret = IPANEL_ERR;
	DMX_STATUS status ;

    ipanel_porting_dprintf("[ipanel_porting_descrambler_set_evenkey] handle=0x%x, key=%p, len=%d\n",\
        				   handle, key, len);

	for(i=0;i<len;i++)
	{
		ipanel_porting_dprintf(" 0x%02x ",key[i]);
	}
	ipanel_porting_dprintf("\n\n");

    if (handle && key && (len > 0))
    {
		status = cnxt_dmx_descrambler_set_even_keys(gDemuxInstance,handle,  len, (char*)key);
		if ( DMX_STATUS_OK != status)
		{
			ipanel_porting_dprintf("[ipanel_porting_descrambler_set_evenkey] descrable set even key failed!\n");
			return IPANEL_ERR ; 
		}	

		ret = IPANEL_OK;
    }

    return ret;
}

