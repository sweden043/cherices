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
#include "ipanel_still.h"
#include "ipanel_vdec.h"
#include "ipanel_vout.h"

extern u_int32 gDemuxInstance;

u_int8  g_video_play_status = IPANEL_VIDEO_STATUS_IDLE ;

UINT32_T  g_ipanel_video_channel, g_ipanel_audio_channel;

extern bool gen_video_decode_still_image(voidF *pImage, u_int32 uSize);
extern void gen_video_unblank_internal(void);


//--------------------------------------------------------------------------------------------------
// Internal Prototypes
//--------------------------------------------------------------------------------------------------
//
static u_int8 m_stop_mode = 0;

static VIDEO_STATUS  m_video_status = VIDEO_NULL_STATUS ; 

static int frame_num = 0;

static void gen_video_frame_cb()
{
	frame_num++;

	if (frame_num==2)
	{
		gen_video_unblank_internal();
		gen_video_set_frame_callback(NULL);
		frame_num = 0;
	}
}

VIDEO_STATUS ipanel_get_video_status(void)
{
	return m_video_status;
}

/******************************************************************************	
** 	Description : Start playing video;
**	Output : None;
**	Return : None;
******************************************************************************/
static INT32_T ipanel_vdec_start(VOID)
{
	INT32_T ret = IPANEL_OK;

	frame_num = 0;
	gen_video_set_frame_callback(gen_video_frame_cb);
    
	gen_video_play (PLAY_LIVE_STATE_NO_SYNC, NULL);
	
	m_video_status = IPANEL_VDEC_STARTED ;

    g_video_play_status = IPANEL_VIDEO_STATUS_PLAY ;

	return ret ;
}

/******************************************************************************	
**	Description : Stop playing video;
** 	Input : None;
**	Output : None;
**	Return : None;
******************************************************************************/
static INT32_T  ipanel_vdec_stop(UINT32_T mode)
{
	INT32_T ret = IPANEL_OK;

    if( IPANEL_VDEC_LAST_FRAME == mode)
    {
	    gen_video_unblank(NULL);
        m_stop_mode = IPANEL_VDEC_LAST_FRAME;
    }
    else if ( IPANEL_VDEC_BLANK == mode)
    {
	    gen_video_blank(NULL);
        m_stop_mode = IPANEL_VDEC_BLANK;
    }

	gen_video_stop(NULL);			

	m_video_status = IPANEL_VDEC_STOPPED ;

    g_video_play_status = IPANEL_VIDEO_STATUS_STOP ;
	
	return ret ;
}

/******************************************************************************	
**	Description : pause playing video;
** 	Input : None;
**	Output : None;
**	Return : None;
******************************************************************************/
static INT32_T ipanel_vdec_pause(VOID)
{
	INT32_T ret = IPANEL_ERR;
	DMX_STATUS dmxstat;

	/* stop the video, wait for completion */
	gen_video_stop( NULL );

	dmxstat = cnxt_dmx_channel_control(gDemuxInstance,  g_ipanel_video_channel, 
                                       (gencontrol_channel_t) GEN_DEMUX_DISABLE);
	if( DMX_STATUS_OK != dmxstat )
	{
		ipanel_porting_dprintf("[ipanel_vdec_pause] pause video failed! \n");
	}
	else
	{
        m_video_status =  IPANEL_VDEC_PAUSED ;

        ret = IPANEL_OK ; 
        ipanel_porting_dprintf("[ipanel_vdec_pause] pause video success! \n");   	

        g_video_play_status = IPANEL_VIDEO_STATUS_STOP ;
    }

    return ret;	
}

/******************************************************************************	
**	Description : resume playing video;
** 	Input : None;
**	Output : None;
**	Return : None;
******************************************************************************/
static INT32_T ipanel_vdec_resume(VOID)
{	
	INT32_T ret= IPANEL_ERR;
	DMX_STATUS dmxstat;

	ipanel_porting_dprintf("[ipanel_vdec_resume] is called \n");

	/* play the audio */
	gen_video_play( PLAY_STILL_STATE, NULL);  	

	/*play video*/
	dmxstat=cnxt_dmx_channel_control(gDemuxInstance,  g_ipanel_video_channel, 
	                                 (gencontrol_channel_t) GEN_DEMUX_ENABLE);
	if( DMX_STATUS_OK != dmxstat )
	{
		ipanel_porting_dprintf("[ipanel_vdec_resume] resume video failed! \n");
	}
	else
	{
        gen_video_play( PLAY_LIVE_STATE_NO_SYNC, NULL);

        m_video_status = IPANEL_VDEC_HUNGERING ;

        ret = IPANEL_OK; 
        ipanel_porting_dprintf("[ipanel_vdec_resume] resume video success! \n");

        g_video_play_status = IPANEL_VIDEO_STATUS_PLAY;
    }

    return ret;
}

static INT32_T ipanel_vdec_get_status()
{
    bool avStatus = FALSE;
    INT32_T ret = -1;
	ipanel_porting_dprintf("iPanel  get dec status step 1!\n");
    avStatus = cnxt_check_av_channel_status(VIDEO_CHANNEL);
	ipanel_porting_dprintf("iPanel  get dec status step 2!\n");
    if(avStatus == TRUE)
    {
        ret = IPANEL_VDEC_DECORDING;
    }
    else if(avStatus == FALSE)
    {
        ret = IPANEL_VDEC_HUNGERING;
    }
	ipanel_porting_dprintf("iPanel get dec status success!\n");

    return ret;
}
static INT32_T ipanel_iframe_stop_av()
{
#define VALID_AUDIO_TIMEOUT   1000
#define CLEAN_AUDIO_WAIT_TIME 150 /* ~6 frame times at 48KHz */

	int ret = IPANEL_OK;

	/* mute the audio */
	hw_set_aud_mute(TRUE);
	
	/* stop the audio */
	gen_audio_stop();

	/* stop the video, wait for completion */
	gen_video_stop( NULL );

	/* disable the pids */
	cnxt_dmx_channel_control(gDemuxInstance, g_ipanel_audio_channel,
							(gencontrol_channel_t)GEN_DEMUX_DISABLE);

	cnxt_dmx_channel_control(gDemuxInstance, g_ipanel_video_channel,
						    (gencontrol_channel_t) GEN_DEMUX_DISABLE);

	cnxt_dmx_set_pcr_pid(gDemuxInstance, 0x1FFF);

	/* Wait for valid and clean audio before unmuting */
	ipanel_WaitForAudioValid(VALID_AUDIO_TIMEOUT, CLEAN_AUDIO_WAIT_TIME);
	
	hw_set_aud_mute(FALSE);

	cnxt_dmx_channel_set_pid(gDemuxInstance, g_ipanel_audio_channel, 0x1FFF);

	cnxt_dmx_channel_set_pid(gDemuxInstance, g_ipanel_video_channel, 0x1FFF);

	return ret;
}

static INT32_T ipanel_vdec_iframe_play(IPANEL_IOCTL_DATA *still_data)
{
	INT32_T ret = IPANEL_OK ;

	ipanel_porting_dprintf("[ipanel_vdec_iframe_play] len=0x%x, buffer=0x%x \n",
						   still_data->len, still_data->data);

	ipanel_iframe_stop_av();
	
	gen_video_decode_still_image((voidF)still_data->data, still_data->len);

	cnxt_dmx_channel_control( gDemuxInstance, g_ipanel_audio_channel, 
							  (gencontrol_channel_t)GEN_DEMUX_ENABLE );

	cnxt_dmx_channel_control( gDemuxInstance, g_ipanel_video_channel, 
							  (gencontrol_channel_t)GEN_DEMUX_ENABLE );

	return ret ; 
}

static INT32_T ipanel_vdec_iframe_stop(void)
{
	INT32_T ret = IPANEL_OK ; 

	ipanel_porting_dprintf("[ipanel_vdec_iframe_stop] is called!\n");
	
	return ret ; 
}

/***************************************************************************************************
���ܣ���һ����Ƶ���뵥Ԫ
ԭ�ͣ�UINT32_T ipanel_porting_vdec_open(VOID)
����˵����

  �����������

  �����������

��    �أ�

  != IPANEL_NULL���ɹ�����Ƶ�����������

  == IPANEL_NULL��ʧ��

***************************************************************************************************/
UINT32_T ipanel_porting_vdec_open(VOID)
{
    UINT32_T decoder = 0xE0D0;

    ipanel_porting_dprintf("[ipanel_porting_vdec_open] is called!\n");

    return decoder;
}

/***************************************************************************************************
���ܣ��ر�ָ������Ƶ���뵥Ԫ
ԭ�ͣ�INT32_T ipanel_porting_vdec_close(UINT32_T decoder)
����˵����

  �����������Ƶ���������

  �����������

��    �أ�

  IPANEL_OK���ɹ�

  IPANEL_ERR��ʧ��

***************************************************************************************************/
INT32_T ipanel_porting_vdec_close(UINT32_T decoder)
{
    INT32_T ret = IPANEL_ERR;

    ipanel_porting_dprintf("[ipanel_porting_vdec_close] decoder=0x%x\n", decoder);

    return ret;
}

/***************************************************************************************************
���ܣ���decoder����һ�������������������úͻ�ȡdecoder�豸�Ĳ���������
ԭ�ͣ�INT32_T ipanel_porting_vdec_ioctl(UINT32_T decoder, IPANEL_VDEC_IOCTL_e op, VOID *arg)
����˵����

  ���������

    decoder -- ���뵥Ԫ���

    op -- ��������

      typedef enum

      {

        IPANEL_VDEC_SET_SOURCE =1,

        IPANEL_VDEC_START,

        IPANEL_VDEC_STOP,

        IPANEL_VDEC_PAUSE,

        IPANEL_VDEC_RESUME,

        IPANEL_VDEC_CLEAR,

        IPANEL_VDEC_SYNCHRONIZE,

        IPANEL_VDEC_GET_BUFFER_RATE,

      } IPANEL_VDEC_IOCTL_e;

    arg -- �������������Ĳ�����������ö���ͻ�32λ����ֵʱ��arg��ǿ��ת���ɶ�Ӧ�������͡�

    op, argȡֵ���±�

    +---------------------+-------------------------+-----------------------------+
    |  op                 |   arg                   |  ˵��                       |
    +---------------------+-------------------------+-----------------------------+
    | IPANEL_VDEC_        |typedef enum  {          |ָ����Ƶ��������������Դ     |
    |   SET_SOURCE        | IPANEL_AV_SOURCE_DEMUX, |  ��Demuxģ����������        |
    |                     | IPANEL_AV_SOURCE_MANUAL |  ϵͳ������������         |
    |                     |}IPANEL_AV_SOURCE_TYPE_e;|                             |

    +---------------------+-------------------------+-----------------------------+
    | IPANEL_VDEC_START   |IPANEL_NULL              |����ָ����decoder��          |
    +---------------------+-------------------------+-----------------------------+
    | IPANEL_VDEC_STOP    |typedef enum {           |ָֹͣ����decoder����ָ��    |
    |                     | IPANEL_VDEC_LAST_FRAME, |�Ƿ������Ļ��档         |
    |                     | IPANEL_VDEC_BLANK       |                             |
    |                     |}IPANEL_VDEC_STOP_MODE_e;|                             |
    +---------------------+-------------------------+-----------------------------+
    | IPANEL_VDEC_PAUSE   |IPANEL_NULL              |�ݶ�decoder��Ľ���          |
    +---------------------+-------------------------+-----------------------------+
    | IPANEL_VDEC_RESUME  |IPANEL_NULL              |�ָ�decoder��Ľ���          |
    +---------------------+-------------------------+-----------------------------+
    | IPANEL_VDEC_CLEAR   |IPANEL_NULL              |���Decoder�ڲ�������������  |
    +---------------------+-------------------------+-----------------------------+
    | IPANEL_VDEC_        |typedef enum {           |��ֹ����������Ƶͬ������     |
    |   SYNCHRONIZE       | IPANEL_DISABLE,         |  ��ֹ                       |
    |                     | IPANEL_ENABLE           |  ����                       |
    |                     |}IPANEL_SWITCH_e;        |                             |
    +---------------------+-------------------------+-----------------------------+
    | IPANEL_VDEC_        |0~100����ֵ              |������Ƶ������ǰ��bufferռ�� |
    |   GET_BUFFER_RATE   |                         |���,�԰ٷֱ�ʾ              |
    +---------------------+-------------------------+-----------------------------+
  �����������

��    �أ�

  IPANEL_OK���ɹ�

  IPANEL_ERR��ʧ��
***************************************************************************************************/
INT32_T ipanel_porting_vdec_ioctl(UINT32_T decoder, IPANEL_VDEC_IOCTL_e op, VOID *arg)
{
    INT32_T ret = IPANEL_OK;
    u_int32 value ;
    UINT32_T  oparg = (UINT32_T)arg;

    ipanel_porting_dprintf("[ipanel_porting_vdec_ioctl] decoder=0x%x, \
                           op=%d, arg=%p\n", decoder, op, arg);

    switch (op)
    {
        case IPANEL_VDEC_SET_SOURCE :
            break;

        case IPANEL_VDEC_START :
            ret = ipanel_vdec_start();
            break;

        case IPANEL_VDEC_STOP :
            ret = ipanel_vdec_stop(oparg);
            break;

        case IPANEL_VDEC_PAUSE :
            ret = ipanel_vdec_pause();
            break;

        case IPANEL_VDEC_RESUME :
            ret = ipanel_vdec_resume();
            break;

        case IPANEL_VDEC_CLEAR :
            gen_video_blank(NULL);
            break;

        case IPANEL_VDEC_SYNCHRONIZE :
            break;

        case IPANEL_VDEC_GET_BUFFER_RATE :
            break;

        case IPANEL_VDEC_PLAY_I_FRAME :
			ret = ipanel_vdec_iframe_play((IPANEL_IOCTL_DATA*)arg);
            break;

        case IPANEL_VDEC_STOP_I_FRAME :
			ret = ipanel_vdec_iframe_stop();
            break;

        case IPANEL_VDEC_SET_STREAM_TYPE :
            break;

        case IPANEL_VDEC_SET_SYNC_OFFSET :
            break;

        case IPANEL_VDEC_GET_BUFFER :
            break;

        case IPANEL_VDEC_PUSH_STREAM :
            break;

        case IPANEL_VDEC_GET_BUFFER_CAP :
            break;

        case IPANEL_VDEC_SET_CONFIG_PARAMS :
            break;

        case IPANEL_VDEC_SET_RATE :
            break;

        case IPANEL_VDEC_GET_CURRENT_DTS :
            ipanel_get_vpts(&value);
            *((UINT32_T*)arg) = value ;
            break;

        case IPANEL_VDEC_GET_DEC_STATUS :
            *((INT32_T*)arg) = ipanel_vdec_get_status();
            break;

        default :
            ipanel_porting_dprintf("[ipanel_porting_vdec_ioctl] ERROR parameter!\n");
            ret = IPANEL_ERR;
    }

    return ret;
}

INT32_T ipanel_vdec_init(VOID)
{
    INT32_T ret = IPANEL_OK;
	bool status;

	status = gen_video_init(FALSE, NULL, NULL);
	if( FALSE == status)
	{
		ret = IPANEL_ERR;
        ipanel_porting_dprintf("[ipanel_vdec_init] gen_video_init failed\n");
	}

	gen_video_blank(NULL);
    return ret;
}

VOID ipanel_vdec_exit(VOID)
{
    return ;
}

