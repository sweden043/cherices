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
功能：打开一个解码单元。
原型：UINT32_T ipanel_porting_adec_open(VOID)
参数说明：

  输入参数：无

  输出参数：无

返    回：

	！＝IPANEL_NULL：成功，解码器句柄；

	＝＝IPANEL_NULL：失败

****************************************************************************************************/
UINT32_T ipanel_porting_adec_open(VOID)
{
    UINT32_T decoder = 0xA0D0;

    ipanel_porting_dprintf("[ipanel_porting_adec_open] is called!\n");

    return decoder;
}

/***************************************************************************************************
功能：关闭指定的解码单元。
原型：INT32_T ipanel_porting_adec_close(UINT32_T decoder)

参数说明：

  输入参数：decoder: 要关闭的解码单元句柄。

  输出参数：无

返    回：

  IPANEL_OK:成功;

  IPANEL_ERR:失败。
****************************************************************************************************/
INT32_T ipanel_porting_adec_close(UINT32_T decoder)
{
    INT32_T ret = IPANEL_OK;

    ipanel_porting_dprintf("[ipanel_porting_adec_close] decoder=0x%x\n", decoder);

    return ret;
}

/***************************************************************************************************
功能：对声音输出设置进行一个操作，或者用于设置和获取声音输出设备的参数和属性
原型：INT32_T ipanel_porting_audio_output_ioctl(UINT32_T handle, IPANEL_AOUT_IOCTL_e op, VOID *arg)
参数说明：

  输入参数：

    audio： 输出设备句柄

    op － 操作命令

      typedef enum

      {

        IPANEL_AOUT_SET_OUTPUT =1,

        IPANEL_AOUT_SET_VOLUME,

        IPANEL_AOUT_SET_BALANCE

      } IPANEL_AOUT_IOCTL_e;

    arg - 操作命令所带的参数，当传递枚举型或32位整数值时，arg可强制转换成对应数据类型。

    op, arg取值见下表：

    +---------------------+-------------------------+-----------------------------+
    |  op                 |   arg                   |  说明                       |
    +---------------------+-------------------------+-----------------------------+
    | IPANEL_AOUT_        |typedef enum  {          |指定声音输出设备             |
    |   SET_OUTPUT        | IPANEL_AOUT_DEVICE_ANALOG_STERO=0x01,//立体声输出     |
    |                     | IPANEL_AOUT_DEVICE_ANALOG_MUTI=0x02, //多声道输出     |
    |                     | IPANEL_AOUT_DEVICE_SPDIF=0x04,       //S/PDIF输出     |
    |                     | IPANEL_AOUT_DEVICE_HDMI=0x08,        //HDMI输出       |
    |                     | IPANEL_AOUT_DEVICE_I2S=0x10,         //I2S输出        |
    |                     | IPANEL_AOUT_DEVICE_ALL = 0xff,       //所有端口       |
    |                     |}IPANEL_AOUT_DEVICE_e;   |                             |

    +---------------------+-------------------------+-----------------------------+
    | IPANEL_AOUT_        |0～100整数值             |设置音频输出音量             |
    |   SET_VOLUME        |                         |                             |
    +---------------------+-------------------------+-----------------------------+
    | IPANEL_AOUT_        |0～100整数值             |设置音频输出左右声道均衡参数,|
    |   SET_BALANCE       |                         |默认值为0.                   |
    +---------------------+-------------------------+-----------------------------+
  输出参数：无

返    回：

  IPANEL_OK:成功;

  IPANEL_ERR:失败。

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
        case IPANEL_ADEC_SET_SOURCE :           /* 指定Audio decoder输入数据来源 */
            break;

        case IPANEL_ADEC_START :                /* 启动指定的decoder，并指出输入的数据格式*/
            ipanel_adec_start();
            break;

        case IPANEL_ADEC_STOP :                 /* 停止指定的decoder*/
	        ipanel_adec_stop();
            break;

        case IPANEL_ADEC_PAUSE :                /* 暂定已启动的decoder解码单元的解码。*/
            ret = ipanel_adec_pause();
            break;

        case IPANEL_ADEC_RESUME :               /* 恢复已暂定的decoder解码单元的解码。*/
            ret = ipanel_adec_resume();
            break;

        case IPANEL_ADEC_CLEAR :                /* 清空Decoder解码单元内部缓冲区的内容*/
            break;

        case IPANEL_ADEC_SYNCHRONIZE :          /* 禁止和允许视音频同步功能*/
            break;

        case IPANEL_ADEC_SET_CHANNEL_MODE :     /* 设置双声道输出模式。*/
            ret = ipanel_adec_set_channel_mode(decoder, oparg);
            break;

        case IPANEL_ADEC_SET_MUTE :             /* 禁止和允许静音功能。*/
            ipanel_adec_set_mute(oparg);
            break;

        case IPANEL_ADEC_SET_PASS_THROUGH :     /* 设置解码器是否对输入流解码，
                                                   不用解码的流解码器工作在旁路模式*/
            break;

        case IPANEL_ADEC_SET_VOLUME :           /* 设置输出音量大小 */
            ipanel_adec_set_volume(oparg);
            break;

        case IPANEL_ADEC_GET_BUFFER_RATE :      /* 返回音频解码器前级buffer占用情况，以百分表示 */
            break;

        case IPANEL_ADEC_SET_SYNC_OFFSET :      /* 90KHz时钟的计数值，设置音频播放单元与系统时间的差值 */
            break;

        case IPANEL_ADEC_GET_BUFFER :           /* 向音频解码设备获取数据缓冲区 */
            break;

        case IPANEL_ADEC_PUSH_STREAM :          /* 向音频解码器输入压缩音频数据。 */
            break;

        case IPANEL_ADEC_GET_BUFFER_CAP :       /* 获得解码器缓冲buffer的大小（缓冲能力） */
            break;

        case IPANEL_ADEC_SET_CONFIG_PARAMS :    /* 获取解码器工作参数，参数串符合SDP描叙格式 */
            break;

        case IPANEL_ADEC_GET_CURRENT_PTS :      /* 获取当前解码数据单元的时间戳 */
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

