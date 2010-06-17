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
#include "ipanel_tuner.h"
#include "ipanel_demux.h"
#include "ipanel_adec.h"
#include "ipanel_aout.h"
#include "ipanel_vdec.h"
#include "ipanel_vout.h"
#include "ipanel_av.h"
#include "ipanel_os.h"
#include "ipanel_task.h"
#include "ipanel_porting_event.h"

/* º‡øÿ“Ù ”∆µ◊¥Ã¨ */
static UINT32_T m_av_thread = IPANEL_NULL; 

extern u_int8   g_audio_play_status;
extern u_int8   g_video_play_status ; 
extern UINT32_T g_main_queue;
u_int8 msg_Flag;

#define IPANEL_AV_MONITOR_TIME    (3000) 
#define IPANEL_AV_SECOND_MONITOR_TIME	(3000)
#define IPANEL_AV_THIRD_MONITOR_TIME	(3000)


static void ipanel_av_monitor(void *param)
{
    bool avStatus = FALSE;
	bool avSecondstatus = FALSE;
	bool avTHIRDstatus = FALSE;
    IPANEL_QUEUE_MESSAGE msg ;
    static u_int8  v_curr_status = VIDEO_LINK_UNKNOWN;
    static u_int8  a_curr_status = AUDIO_LINK_UNKNOWN;

    msg.q1stWordOfMsg = EIS_EVENT_TYPE_DVB ;
    msg.q2ndWordOfMsg = 0 ;
    msg.q3rdWordOfMsg  = 0 ;
    msg.q4thWordOfMsg  = 0 ;
    
    while(1)
    {
        // VIDEO Decode status 
        if( g_video_play_status == IPANEL_VIDEO_STATUS_PLAY)
        {
            avStatus = cnxt_check_av_channel_status(VIDEO_CHANNEL);
            if( avStatus == TRUE && msg_Flag ==1)
            {
                if( VIDEO_LINK_UNKNOWN == v_curr_status || VIDEO_LINK_DOWN == v_curr_status)
                {
                    v_curr_status = VIDEO_LINK_UP ;
                    msg.q2ndWordOfMsg = EIS_DEVICE_DECODER_NORMAL ;
                    ipanel_porting_dprintf("Video decode have input data!\n");
                    ipanel_porting_queue_send(g_main_queue,&msg);
					msg_Flag =0;
                }
            }
            else if( avStatus == FALSE)
            {
            	ipanel_porting_task_sleep(IPANEL_AV_SECOND_MONITOR_TIME);
				avSecondstatus = cnxt_check_av_channel_status(VIDEO_CHANNEL);
				ipanel_porting_task_sleep(IPANEL_AV_THIRD_MONITOR_TIME);
				avTHIRDstatus = cnxt_check_av_channel_status(VIDEO_CHANNEL);
				if((avSecondstatus == FALSE) && (avTHIRDstatus == FALSE)){
                if( VIDEO_LINK_UNKNOWN == v_curr_status || VIDEO_LINK_UP == v_curr_status)
                {
                    v_curr_status = VIDEO_LINK_DOWN;
                    msg.q2ndWordOfMsg = EIS_DEVICE_DECODER_HUNGER ;
                    ipanel_porting_dprintf("[WARNING]Video decode have no input data!\n");
                    ipanel_porting_queue_send(g_main_queue,&msg);
					msg_Flag = 1;
                }
				}
            }
        }

        /* AUDIO Decode status  
        if( g_audio_play_status == IPANEL_AUDIO_STATUS_PLAY)
        {
            if( avStatus == TRUE)
            {
                if( AUDIO_LINK_UNKNOWN == a_curr_status || AUDIO_LINK_DOWN == a_curr_status)
                {
                    a_curr_status = AUDIO_LINK_UP ;
                    ipanel_porting_dprintf("Audio decode have input data!\n");
                }
            }
            else if( avStatus == FALSE)
            {
                if( AUDIO_LINK_UNKNOWN == a_curr_status || AUDIO_LINK_UP == a_curr_status)
                {
                    a_curr_status = AUDIO_LINK_DOWN;
                    ipanel_porting_dprintf("[WARNING]Audio decode have no input data!\n");
                }
            }
        }
        */

        ipanel_porting_task_sleep(IPANEL_AV_MONITOR_TIME);
    }
}

INT32_T ipanel_av_init()
{
    m_av_thread = ipanel_porting_task_create(
                                             IPANEL_AV_MONITOR_NAME,
                                             ipanel_av_monitor,
                                             (VOID*)NULL,
                                             IPANEL_AV_MONITOR_PRIORITY,
                                             IPANEL_AV_MONITOR_STACK_SIZE);
    if( IPANEL_NULL ==  m_av_thread  )
    {
        ipanel_porting_dprintf("[ipanel_av_init] create av monitor task failed!\n");
        return IPANEL_ERR;
    }

    return IPANEL_OK;
}

void ipanel_av_exit()
{
    if(m_av_thread)
    {
        ipanel_porting_task_destroy(m_av_thread);
        m_av_thread = IPANEL_NULL;
    }
}

