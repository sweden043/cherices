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
#include "ipanel_aout.h"

extern UINT32_T g_mute_mode ;

/**********************************************************************************/
/*Description: Set Audio volume. 0 - mute, 100 - strength sound, between 0 to 100 */
/*Input      : volume value, if value is not in 0 to 100, the return fail         */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
static INT32_T ipanel_audio_set_output_volume(UINT32_T volume)
{
	INT32_T ret = IPANEL_ERR;
 	PCM_STATUS pcmstat ; 
	u_int8 left_volume , right_volume ; 
	
	ipanel_porting_dprintf("[ipanel_audio_set_output_volume] vol=%d\n",volume);

	if( IPANEL_ENABLE == g_mute_mode)
	{
		hw_set_aud_mute(FALSE);	
    }

	if (volume <= 100) 
	{
		volume += 60; 	// ����һ������ֵ,��Ȼ����û����������
		left_volume = right_volume =  (volume*31)/100;

		if(left_volume > 31)
		{
			left_volume = right_volume = 31;
		}
		else if(volume <= 60)
		{
			left_volume = right_volume = 0;
		}

		pcmstat = cnxt_pcm_set_volume_mpeg(left_volume, right_volume);
		if( pcmstat != PCM_ERROR_OK )
		{
			ipanel_porting_dprintf("[ipanel_audio_set_output_volume] set volume failed! \n");
		}
        else
		{
    		ret = IPANEL_OK;
        }
	}

	return ret;
}

/***************************************************************************************************
���ܣ�����Ƶ�������Ԫ
ԭ�ͣ�UINT32_T ipanel_porting_audio_output_open(VOID)
����˵����

  �����������

  �����������

��    �أ�

  != IPANEL_NULL���ɹ�����Ƶ�������Ԫ�����

  == IPANEL_NULL��ʧ��

****************************************************************************************************/
UINT32_T ipanel_porting_audio_output_open(VOID)
{
    UINT32_T handle = 0xA0E0;

    ipanel_porting_dprintf("[ipanel_porting_audio_output_open] is called!\n");

    return handle;
}

/***************************************************************************************************
���ܣ��ر�ָ������Ƶ�����Ԫ
ԭ�ͣ�INT32_T ipanel_porting_audio_output_close(UINT32_T handle)
����˵����

  ���������handle:    Ҫ�رյ��������Ԫ���

  �����������

��    �أ�

  IPANEL_OK:�ɹ�;

  IPANEL_ERR:ʧ�ܡ�
****************************************************************************************************/
INT32_T ipanel_porting_audio_output_close(UINT32_T handle)
{
    INT32_T ret = IPANEL_OK;

    ipanel_porting_dprintf("[ipanel_porting_audio_output_close] handle=0x%x\n", handle);

    return ret;
}

/***************************************************************************************************
���ܣ�������������ý���һ�������������������úͻ�ȡ��������豸�Ĳ���������
ԭ�ͣ�INT32_T ipanel_porting_audio_output_ioctl(UINT32_T handle, IPANEL_AOUT_IOCTL_e op, VOID *arg)
����˵����

  ���������

    audio�� ����豸���

    op �� ��������

      typedef enum

      {

        IPANEL_AOUT_SET_OUTPUT =1,

        IPANEL_AOUT_SET_VOLUME,

        IPANEL_AOUT_SET_BALANCE

      } IPANEL_AOUT_IOCTL_e;

    arg - �������������Ĳ�����������ö���ͻ�32λ����ֵʱ��arg��ǿ��ת���ɶ�Ӧ�������͡�

    op, argȡֵ���±�

    +---------------------+-------------------------+-----------------------------+
    |  op                 |   arg                   |  ˵��                       |
    +---------------------+-------------------------+-----------------------------+
    | IPANEL_AOUT_        |typedef enum  {          |ָ����������豸             |
    |   SET_OUTPUT        | IPANEL_AOUT_DEVICE_ANALOG_STERO=0x01,//���������     |
    |                     | IPANEL_AOUT_DEVICE_ANALOG_MUTI=0x02, //���������     |
    |                     | IPANEL_AOUT_DEVICE_SPDIF=0x04,       //S/PDIF���     |
    |                     | IPANEL_AOUT_DEVICE_HDMI=0x08,        //HDMI���       |
    |                     | IPANEL_AOUT_DEVICE_I2S=0x10,         //I2S���        |
    |                     | IPANEL_AOUT_DEVICE_ALL = 0xff,       //���ж˿�       |
    |                     |}IPANEL_AOUT_DEVICE_e;   |                             |

    +---------------------+-------------------------+-----------------------------+
    | IPANEL_AOUT_        |0��100����ֵ             |������Ƶ�������             |
    |   SET_VOLUME        |                         |                             |
    +---------------------+-------------------------+-----------------------------+
    | IPANEL_AOUT_        |0��100����ֵ             |������Ƶ������������������,|
    |   SET_BALANCE       |                         |Ĭ��ֵΪ0.                   |
    +---------------------+-------------------------+-----------------------------+
  �����������

��    �أ�

  IPANEL_OK:�ɹ�;

  IPANEL_ERR:ʧ�ܡ�

****************************************************************************************************/
INT32_T ipanel_porting_audio_output_ioctl(UINT32_T handle, IPANEL_AOUT_IOCTL_e op, VOID *arg)
{
    INT32_T ret = IPANEL_OK;
    UINT32_T oparg = (UINT32_T)arg;

    ipanel_porting_dprintf("[ipanel_porting_audio_output_ioctl] handle=0x%x, op=%d, \
                            arg=%p\n", handle, op, arg);

    switch (op)
    {
        case IPANEL_AOUT_SET_OUTPUT :
            break;

        case IPANEL_AOUT_SET_VOLUME :
	        ret = ipanel_audio_set_output_volume(oparg);
            break;

        case IPANEL_AOUT_SET_BALANCE :
            break;

		case IPANEL_AOUT_GET_VOLUME:
			break;

        default :
            ipanel_porting_dprintf("[ipanel_porting_audio_output_ioctl] ERROR parameter!\n");
            ret = IPANEL_ERR;
    }

    return ret;
}

