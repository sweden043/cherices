//--------------------------------------------------------------------------------------------------
// Copyright (c) 2005 iPanel Technologies, Ltd.
// All rights reserved.
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
#include "ipanel_still.h"
#include "ipanel_adec.h"

//--------------------------------------------------------------------------------------------------
// Types and defines
//--------------------------------------------------------------------------------------------------
//
#define VALID_AUDIO_TIMEOUT             200 //1000
#define CLEAN_AUDIO_WAIT_TIME           50  //150 /* ~6 frame times at 48KHz */

UINT32_T g_mute_mode = 0; 
//--------------------------------------------------------------------------------------------------
// Global data
//--------------------------------------------------------------------------------------------------
//

extern u_int32 gDemuxInstance;
extern UINT32_T  g_ipanel_audio_channel;

u_int8  g_audio_play_status = IPANEL_AUDIO_STATUS_IDLE; 
//--------------------------------------------------------------------------------------------------
// Internal Prototypes
//--------------------------------------------------------------------------------------------------
//
static HAUDIO m_ipanel_haudio ;

/******************************************************************************	
** 	Description : Start playing audio;
**	Output : None;
**	Return : None;
******************************************************************************/
static INT32_T  ipanel_adec_start(VOID)
{
    INT32_T ret = IPANEL_OK;

    gen_audio_play(FALSE);

    //*glpIrqStatus   = MPG_IRQ_AUD_PTS;  // inherit from blanco
    //*glpIrqMask    |= MPG_IRQ_AUD_PTS;  // enable pts received interrupt for audio type check

    ipanel_WaitForAudioValid(VALID_AUDIO_TIMEOUT, CLEAN_AUDIO_WAIT_TIME);

    hw_set_aud_mute(FALSE);

	if( IPANEL_ENABLE == g_mute_mode)
	{
		hw_set_aud_mute(TRUE);	
	}
    
    g_audio_play_status = IPANEL_AUDIO_STATUS_PLAY ;
    return ret ;
}

/******************************************************************************	
**	Description : pause playing audio;
** 	Input : None;
**	Output : None;
**	Return : None;
******************************************************************************/
static INT32_T  ipanel_adec_pause(VOID)
{
	INT32_T ret = IPANEL_ERR;
	DMX_STATUS dmxstat;

	/* mute the audio */
	hw_set_aud_mute(TRUE);

	/* stop the audio */
	gen_audio_stop();

	dmxstat=cnxt_dmx_channel_control(gDemuxInstance, g_ipanel_audio_channel, 
                                     (gencontrol_channel_t) GEN_DEMUX_DISABLE);
	if( DMX_STATUS_OK != dmxstat )
	{
		ipanel_porting_dprintf("[ipanel_audio_pause] pause audio failed! \n");
		return ret ;
	}

	hw_set_aud_mute(FALSE);

	ret = IPANEL_OK ; 
	ipanel_porting_dprintf("[ipanel_audio_pause] pause audio success! \n");

    g_audio_play_status = IPANEL_AUDIO_STATUS_STOP ;
	
	return ret;
}

/******************************************************************************	
**	Description : resume playing audio;
** 	Input : None;
**	Output : None;
**	Return : None;
******************************************************************************/
static INT32_T ipanel_adec_resume(VOID)
{
	INT32_T ret= IPANEL_ERR;
	DMX_STATUS dmxstat;

	ipanel_porting_dprintf("[ipanel_audio_resume] is called \n");

	/* play the audio */
	gen_audio_play( FALSE );  

	/*play audio*/
	dmxstat=cnxt_dmx_channel_control(gDemuxInstance, g_ipanel_audio_channel,
	                                 (gencontrol_channel_t) GEN_DEMUX_ENABLE);
	if( DMX_STATUS_OK != dmxstat )
	{
		ipanel_porting_dprintf("[ipanel_audio_resume] resume audio failed! \n");
		return ret ;
	}

	ret = IPANEL_OK; 
	ipanel_porting_dprintf("[ipanel_audio_resume] resume video success! \n");

    g_audio_play_status = IPANEL_AUDIO_STATUS_PLAY ; 

	return ret;
}

/******************************************************************************	
**	Description : Stop playing audio;
** 	Input : None;
**	Output : None;
**	Return : None;
******************************************************************************/
static INT32_T  ipanel_adec_stop(VOID)
{
	INT32_T ret = IPANEL_OK;

	hw_set_aud_mute(TRUE);
	
	gen_audio_stop();

	ipanel_WaitForAudioValid(VALID_AUDIO_TIMEOUT, CLEAN_AUDIO_WAIT_TIME);

	hw_set_aud_mute(FALSE);		

    g_audio_play_status = IPANEL_AUDIO_STATUS_STOP ; 

	return ret ;
}

/******************************************************************************	
**	Description : mute or unmute playing audio;
** 	Input : None;
**	Output : None;
**	Return : None;
******************************************************************************/
static INT32_T ipanel_adec_set_mute(UINT32_T mutemode)
{
	INT32_T ret = IPANEL_OK ;

	hw_set_aud_mute(mutemode);	

    g_mute_mode = mutemode ;
	
	return ret ;
}

/******************************************************************************	
**	Description : set audio decode format 
** 	Input : None;
**	Output : None;
**	Return : None;
******************************************************************************/
static INT32_T ipanel_adec_set_channel_mode(UINT32_T decoder, UINT32_T mode)
{
	INT32_T ret = IPANEL_ERR ;
	PCM_STATUS status;
	cnxt_pcm_channel_mode_t pcm_mode;

	hw_set_aud_mute(TRUE);
 
	switch(mode)
	{
		case IPANEL_AUDIO_MODE_STEREO:
			pcm_mode = PCM_CM_STEREO ;
			break;

		case IPANEL_AUDIO_MODE_LEFT_MONO:
			pcm_mode = PCM_CM_MONO_LEFT ;
			break;

		case IPANEL_AUDIO_MODE_RIGHT_MONO:
			pcm_mode = PCM_CM_MONO_RIGHT ;
			break;

		case IPANEL_AUDIO_MODE_MIX_MONO:
			pcm_mode = PCM_CM_MONO_MIX ;
			break;

		case IPANEL_AUDIO_MODE_STEREO_REVERSE:
			pcm_mode = PCM_CM_STEREO_SWAP ;
			break;

		default:
			pcm_mode = PCM_CM_STEREO ; 
			break; 
	}

    status=cnxt_pcm_set_channel_mode_mpeg(pcm_mode);
	if( PCM_ERROR_OK != status )
	{
		ipanel_porting_dprintf("[ipanel_set_audio_channel_mode] set audio channel mode failed!\n");
		return ret ;
	}

   	ipanel_WaitForAudioValid(VALID_AUDIO_TIMEOUT, CLEAN_AUDIO_WAIT_TIME);
   	hw_set_aud_mute(FALSE);

	return (ret = IPANEL_OK);
}

/******************************************************************************	
**	Description : set volume playing audio;
** 	Input : None;
**	Output : None;
**	Return : None;
******************************************************************************/
static INT32_T ipanel_adec_set_volume(UINT32_T volume)
{
	#define VOLUME_MAX_PERC             (100)
	#define VOLUME_START_PERC         (68)/* minimal volume displayed in UI */

	int volume_percentage=0;

	if( IPANEL_ENABLE == g_mute_mode)
	{
		hw_set_aud_mute(FALSE);	
    }

    g_mute_mode = 0 ;
    
	if(volume < 6 )
	{
        volume = 0;
	}
    
	if(volume > 100)
	{
		volume = 100;
	}

    volume_percentage =  VOLUME_START_PERC +
                        (VOLUME_MAX_PERC-VOLUME_START_PERC)*volume/100;
    if ( VOLUME_START_PERC == volume_percentage)
    {
        audio_set_volume(0);
    }
    else
    {
        audio_set_volume(volume_percentage);		 
    }

    return IPANEL_OK;
}

/***************************************************************************************************
���ܣ���һ�����뵥Ԫ��
ԭ�ͣ�UINT32_T ipanel_porting_adec_open(VOID)
����˵����

  �����������

  �����������

��    �أ�

	����IPANEL_NULL���ɹ��������������

	����IPANEL_NULL��ʧ��

****************************************************************************************************/
UINT32_T ipanel_porting_adec_open(VOID)
{
    UINT32_T decoder = 0xA0D0;

    ipanel_porting_dprintf("[ipanel_porting_adec_open] is called!\n");

    return decoder;
}

/***************************************************************************************************
���ܣ��ر�ָ���Ľ��뵥Ԫ��
ԭ�ͣ�INT32_T ipanel_porting_adec_close(UINT32_T decoder)

����˵����

  ���������decoder: Ҫ�رյĽ��뵥Ԫ�����

  �����������

��    �أ�

  IPANEL_OK:�ɹ�;

  IPANEL_ERR:ʧ�ܡ�
****************************************************************************************************/
INT32_T ipanel_porting_adec_close(UINT32_T decoder)
{
    INT32_T ret = IPANEL_OK;

    ipanel_porting_dprintf("[ipanel_porting_adec_close] decoder=0x%x\n", decoder);

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

INT32_T ipanel_porting_adec_ioctl(UINT32_T decoder, IPANEL_ADEC_IOCTL_e op, VOID *arg)
{
    INT32_T ret = IPANEL_OK;
    u_int32 value;
    UINT32_T oparg = (UINT32_T)arg;

    ipanel_porting_dprintf("[ipanel_porting_adec_ioctl] decoder=0x%x, \
                            op=%d, arg=%p\n", decoder, op, arg);

    switch (op)
    {
        case IPANEL_ADEC_SET_SOURCE :           /* ָ��Audio decoder����������Դ */
            break;

        case IPANEL_ADEC_START :                /* ����ָ����decoder����ָ����������ݸ�ʽ*/
            ipanel_adec_start();
            break;

        case IPANEL_ADEC_STOP :                 /* ָֹͣ����decoder*/
	        ipanel_adec_stop();
            break;

        case IPANEL_ADEC_PAUSE :                /* �ݶ���������decoder���뵥Ԫ�Ľ��롣*/
            ret = ipanel_adec_pause();
            break;

        case IPANEL_ADEC_RESUME :               /* �ָ����ݶ���decoder���뵥Ԫ�Ľ��롣*/
            ret = ipanel_adec_resume();
            break;

        case IPANEL_ADEC_CLEAR :                /* ���Decoder���뵥Ԫ�ڲ�������������*/
            break;

        case IPANEL_ADEC_SYNCHRONIZE :          /* ��ֹ����������Ƶͬ������*/
            break;

        case IPANEL_ADEC_SET_CHANNEL_MODE :     /* ����˫�������ģʽ��*/
            ret = ipanel_adec_set_channel_mode(decoder, oparg);
            break;

        case IPANEL_ADEC_SET_MUTE :             /* ��ֹ�����������ܡ�*/
            ipanel_adec_set_mute(oparg);
            break;

        case IPANEL_ADEC_SET_PASS_THROUGH :     /* ���ý������Ƿ�����������룬
                                                   ���ý��������������������·ģʽ*/
            break;

        case IPANEL_ADEC_SET_VOLUME :           /* �������������С */
            ipanel_adec_set_volume(oparg);
            break;

        case IPANEL_ADEC_GET_BUFFER_RATE :      /* ������Ƶ������ǰ��bufferռ��������԰ٷֱ�ʾ */
            break;

        case IPANEL_ADEC_SET_SYNC_OFFSET :      /* 90KHzʱ�ӵļ���ֵ��������Ƶ���ŵ�Ԫ��ϵͳʱ��Ĳ�ֵ */
            break;

        case IPANEL_ADEC_GET_BUFFER :           /* ����Ƶ�����豸��ȡ���ݻ����� */
            break;

        case IPANEL_ADEC_PUSH_STREAM :          /* ����Ƶ����������ѹ����Ƶ���ݡ� */
            break;

        case IPANEL_ADEC_GET_BUFFER_CAP :       /* ��ý���������buffer�Ĵ�С������������ */
            break;

        case IPANEL_ADEC_SET_CONFIG_PARAMS :    /* ��ȡ��������������������������SDP�����ʽ */
            break;

        case IPANEL_ADEC_GET_CURRENT_PTS :      /* ��ȡ��ǰ�������ݵ�Ԫ��ʱ��� */
            ipanel_get_apts(&value);
            *((UINT32_T*)arg) = value ;
            break;

        default :
            ipanel_porting_dprintf("[ipanel_porting_adec_ioctl] ERROR parameter!\n");
            ret = IPANEL_ERR;
    }

    return ret;
}

INT32_T ipanel_adec_init(VOID)
{
    INT32_T ret = IPANEL_ERR;
	CNXT_AUDIO_STATUS audstat ;
	bool status ;

	/* enable AC3 pass through */
	cnxt_ac3_passthru (TRUE);

	status = gen_audio_init(FALSE, FALSE);
    if (FALSE == status)
    {
        ipanel_porting_dprintf("[iPanelAudioInit] gen_audio_init failed\n");
        goto AUDIO_INIT_FAILED;
    }

	audstat = cnxt_audio_open(&m_ipanel_haudio);
	if( audstat != CNXT_AUDIO_OK)
	{
		ipanel_porting_dprintf("[ipanel_porting_adec_open] open audio handle failed! \n");
        goto AUDIO_INIT_FAILED;
	}
    
    return IPANEL_OK;

AUDIO_INIT_FAILED:
    return ret;
}

VOID ipanel_adec_exit(VOID)
{
    cnxt_audio_close(m_ipanel_haudio);
}

