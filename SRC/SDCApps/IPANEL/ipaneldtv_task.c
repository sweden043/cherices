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
#include "ipanel_task.h"
#include "ipanel_os.h"
#include "ipanel_av.h"
#include "ipanel_smc.h"
#include "ipanel_uart.h"
#include "ipanel_adec.h"
#include "ipanel_aout.h"
#include "ipanel_adec.h"
#include "ipanel_vdec.h"
#include "ipanel_vout.h"
#include "ipanel_input.h"
#include "ipanel_tuner.h"
#include "ipanel_demux.h"
#include "ipanel_nvram.h"
#include "ipanel_sound.h"
#include "ipanel_socket.h"
#include "ipanel_media.h"
#include "ipanel_product.h"
#include "ipanel_network.h"
#include "ipanel_upgrade.h"
#include "ipanel_graphics.h"
#include "ipanel_frontpanel.h"
#include "ipaneldtv_task.h"
#include "ipanel_porting_event.h"

UINT32_T  g_main_queue = IPANEL_NULL;

int       main_continue_flag   = 1;     // 定义用于主线程是否继续执行
int       reboot_destroy_flag = 0 ;    // 定义用于重启线程是否继续执行  
int       standard_by_flag = 0 ;       // 定义用于待机模式标识

UINT32_T  ipanel_task_pid = 0;       // 定义主线程的返回ID 值

extern void IICInit(void);
extern void IICInit(void);
extern bool clock_init(bool reinit);
extern void hwlib_watchdog_enable(void);
extern int panel_pio_display_string(unsigned char string[4]);

INT32_T ipanel_application_init()
{
    INT32_T ret = IPANEL_ERR;

    /* Set the all trace level and enable all modules */
    trace_set_level(TRACE_LEVEL_ALWAYS | TRACE_ALL);
    
    /* Initialise I2C */
    IICInit();
    
    /* Initialise the non-volatile storage driver */
    nv_init();

    /* Start the clock recovery driver */
    clock_init(FALSE);

#ifdef IPANEL_RELEASE_VERSION
    hwlib_watchdog_enable(); //WatchDog Enabled
    ipanel_porting_dprintf("Open watchdong for RELEASE version!\n");
#endif

    ret = ipanel_nvram_init();
    if(IPANEL_ERR == ret)
    {
        ipanel_porting_dprintf("[ipanel_application_init] ipanel_nvram_init failed!\n");
	    goto INIT_FAILED;
    }

	//清除BASE FLASE ，如果换版本引起的死机，请先清FLASH，放开下面的函数，可以清除指定BASE的FLASH
    //ipanel_erase_base_data();

    ret = ipanel_input_init();
    if (IPANEL_ERR == ret)
    {
        ipanel_porting_dprintf("[ipanel_application_init] ipanel_input_init failed!\n");
        goto INIT_FAILED;
    }

    ret = ipanel_tuner_init();
    if (IPANEL_ERR == ret)
    {
        ipanel_porting_dprintf("[ipanel_application_init] ipanel_tuner_init failed!\n");
        goto INIT_FAILED;
    }

    ret = ipanel_demux_init();
    if (IPANEL_ERR == ret)
    {
        ipanel_porting_dprintf("[ipanel_application_init] ipanel_demux_init failed!\n");
        goto INIT_FAILED;
    }

    ret = ipanel_adec_init();
    if (IPANEL_ERR == ret)
    {
        ipanel_porting_dprintf("[ipanel_application_init] ipanel_adec_init failed!\n");
        goto INIT_FAILED;
    }

    ret = ipanel_vdec_init();
    if (IPANEL_ERR == ret)
    {
        ipanel_porting_dprintf("[ipanel_application_init] ipanel_vdec_init failed!\n");
        goto INIT_FAILED;
    }

    ret = ipanel_graphics_init();
    if (IPANEL_ERR == ret)
    {
        ipanel_porting_dprintf("[ipanel_application_init] ipanel_graphics_init failed!\n");
        goto INIT_FAILED;
    }

	ret = ipanel_video_tvenc_init();
    if (IPANEL_ERR == ret)
    {
        ipanel_porting_dprintf("[ipanel_application_init] ipanel_video_tvenc_init failed!\n");
        goto INIT_FAILED;
    }

	ret = ipanel_smartcard_init();
    if(IPANEL_ERR == ret)
    {
        ipanel_porting_dprintf("[ipanel_application_init] ipanel_smartcard_init failed!\n");
        goto INIT_FAILED;
    }

	ret = ipanel_sound_init();
    if(IPANEL_ERR == ret)
    {
        ipanel_porting_dprintf("[ipanel_application_init] ipanel_sound_init failed!\n");
        goto INIT_FAILED;
    }

    ret = ipanel_socket_init();
    if(IPANEL_ERR == ret)
    {
        ipanel_porting_dprintf("[ipanel_application_init] ipanel_socket_init failed!\n");
        goto INIT_FAILED;
    }

    ret = ipanel_av_init();
    if(IPANEL_ERR == ret)
    {
        ipanel_porting_dprintf("[ipanel_application_init] ipanel_av_init failed!\n");
        goto INIT_FAILED;
    }

    ret = ipanel_frontpanel_init();
    if(IPANEL_ERR == ret)
    {
        ipanel_porting_dprintf("[ipanel_application_init] ipanel_frontpanel_init failed!\n");
        goto INIT_FAILED;
    }

    ret = ipanel_network_init();
    if(IPANEL_ERR == ret)
    {
        ipanel_porting_dprintf("[ipanel_application_init] ipanel_network_init failed!\n");
        goto INIT_FAILED;
    }
    ipanel_porting_dprintf("[ipanel_application_init] init success!\n");
    return IPANEL_OK;
    
INIT_FAILED:
    ipanel_porting_dprintf("[ipanel_application_init] init failed!\n");
    return IPANEL_ERR;
}

void ipanel_application_exit()
{
    ipanel_network_exit();
    
    ipanel_frontpanel_exit();

    ipanel_av_exit();

    ipanel_socket_exit();

    ipanel_smartcard_exit();

    ipanel_graphics_exit();

    ipanel_video_tvenc_exit();

    ipanel_vdec_exit();

    ipanel_adec_exit();

    ipanel_demux_exit();

    ipanel_tuner_exit();

    ipanel_input_exit();

    ipanel_nvram_exit();    
}

static VOID ipanel_main_task(VOID *para)
{
    VOID                   *pDtvHandle  = IPANEL_NULL;
    IPANEL_QUEUE_MESSAGE    msg         = {0, 0, 0, 0};
    INT32_T ret = IPANEL_ERR ;
    unsigned int event[3] = {0};
    unsigned int ts,te;
    unsigned char send_msg_flag = 0;

	g_main_queue = ipanel_porting_queue_create("MANQ", 
                                               IPANEL_MAIN_QUEUE_MAX, 
                                               IPANEL_TASK_WAIT_FIFO);
	if (IPANEL_NULL == g_main_queue)
	{
        ipanel_porting_dprintf("[ipanel_main_task] create main queue failed!\n");
	    goto INIT_FAILED;
	}

    ret = ipanel_application_init();
    if( IPANEL_ERR == ret)
    {
        ipanel_porting_dprintf("[ipanel_main_task] ipanel_application_init failed!\n");
        goto INIT_FAILED;
    }

    panel_pio_display_string((unsigned char*)"load");
    
//    ipanel_network_dhcpget();

#ifdef IPANEL_WRITE_UI2FLASH
    ipanel_write_ui_to_flash();
	ipanel_porting_dprintf("[ipanel_write_ui_to_flash] cccc!\n");
	ipanel_porting_front_panel_ioctl(1,"UI");
    while(1)
    {
        ipanel_porting_task_sleep(1000);
    }
#endif

#ifdef IPANEL_LOADER_DEBUG
	ipanel_eeprom_test();
	ipanel_porting_dprintf("[ipanel get eeprom data 20 char!] cccc!\n");
#endif

#ifdef IPANEL_LOADER_DEBUG_WRITE
	ipanel_eeprom_write();
	ipanel_porting_dprintf("[ipanel get eeprom data 20 char!] cccc!\n");
#endif





STANDY_BY:
    if( standard_by_flag )
    {
        while(1)
        {
            ipanel_porting_task_sleep(500);
            if( ipanel_get_ir_key(event) == IPANEL_OK)
            {
                if( EIS_EVENT_TYPE_KEYDOWN == event[0] &&
                    EIS_IRKEY_STANDBY      == event[1])
                {
                    // 重新启动
                    ipanel_porting_dprintf("iPanel Set-top box will restart!\n");
                    reboot_IRD();
                }
            }
        }
    }

	/*创建中间件实例*/
	pDtvHandle = ipanel_create(IPANEL_MIDDLEWARE_MEMORY_SIZE);
	if (!pDtvHandle)
	{
	    ipanel_porting_dprintf("[ipanel_main_task] ipanel_create failed\n");
	    goto INIT_FAILED;
	}

	while (main_continue_flag)
	{
        send_msg_flag = 0;

        //ipanel_porting_dprintf("[ipanel_main_task] is running!\n");
        
        /* 按键消息 */
        if( ipanel_get_ir_key(event) == IPANEL_OK)
        {
            send_msg_flag = 1;
	        ipanel_proc(pDtvHandle, event[0], event[1], event[2]);
			//printf("event[0]:%d,event[1]:%d,event[2]:%d",event[0],event[1], event[2]);
#if 0
            if(event[0] == IPANEL_EVENT_TYPE_KEYDOWN &&
               event[1] == EIS_IRKEY_YELLOW)
            {
            		//ipanel_porting_dprintf("open url: [http://172.16.0.37] ");
                //ipanel_open_uri(pDtvHandle,"http://172.16.0.37");

            		ipanel_porting_dprintf("open url: [http://172.16.0.37] ");
                ipanel_open_uri(pDtvHandle,"http://172.16.0.37:8080/");
            }
            
            if(event[0] == IPANEL_EVENT_TYPE_KEYDOWN &&
               event[1] == EIS_IRKEY_BLUE)
            {
                ipanel_network_ipset();
            }
#endif
        }

        /*从全局消息队列中接收到数据*/
        ret = ipanel_porting_queue_recv(g_main_queue, &msg, IPANEL_NO_WAIT);
	    if (IPANEL_OK == ret) // tuner&network message 
	    {
            if( EIS_EVENT_TYPE_SYSTEM == msg.q1stWordOfMsg)
            {
                if( IPANEL_STB_SUSPEND == msg.q2ndWordOfMsg)
                {
                    ipanel_porting_dprintf("Enter stand-by mode...\n");
                    if(standard_by_flag ==1)
                    {						
                        goto STANDY_BY;
                    }
                }
            }

            send_msg_flag = 1;
	        ipanel_proc(pDtvHandle, msg.q1stWordOfMsg, msg.q2ndWordOfMsg, msg.q3rdWordOfMsg);
	    }

        if( send_msg_flag == 0)
        {
            ts = ipanel_porting_time_ms();
            ipanel_proc(pDtvHandle,EIS_EVENT_TYPE_TIMER,0,0);
            ipanel_proc(pDtvHandle,EIS_EVENT_TYPE_TIMER,0,0);
            te = ipanel_porting_time_ms();

            if( te - ts < 20) // 让出时间片给其它低任务者工作
            {	
            	//ipanel_porting_dprintf("te - ts < 20");
                ipanel_porting_task_sleep(20);
            }
        }
	}

	ipanel_destroy(pDtvHandle);

	ipanel_porting_dprintf("[ipanel_main_task] exit normal!\n");

INIT_FAILED:
	ipanel_porting_dprintf("[ipanel_main_task] exit abnormality!\n");

    ipanel_application_exit();

	if (IPANEL_NULL != g_main_queue)
	{
		ipanel_porting_queue_destroy(g_main_queue);
        g_main_queue = IPANEL_NULL;
	}

    reboot_destroy_flag = 1 ;
}

/********************************************************************/
/*  FUNCTION:    application_entry                                  */
/*                                                                  */
/*  PARAMETERS:  None                                               */
/*                                                                  */
/*  DESCRIPTION: This is the main application entry point. It is    */
/*               called by the KAL as soon as the RTOS is up and    */
/*               running. Note that this is NOT called in the       */
/*               context of a KAL process so no KAL calls other     */
/*               than kal_initialise may be made from this function.*/
/*               The function should do as little initialisation    */
/*               as possible then spawn a KAL task to form the main */
/*               thread of execution of the application.            */
/*                                                                  */
/*  RETURNS:     Nothing                                            */
/*                                                                  */
/*  CONTEXT:     Will be called by the BSP after RTOS initialisation*/
/*               completes. Non-KAL task context.                   */
/*                                                                  */
/********************************************************************/
void application_entry(void)
{
    int retcode;
    
    retcode = kal_initialise();
    ipanel_porting_dprintf("[kal_initialise] retcode = %d.\n",retcode);

    ipanel_task_pid = ipanel_porting_task_create(IPANEL_MAIN_TASK_NAME, 
		                                         ipanel_main_task, 
		                                         NULL, 
		                                         IPANEL_MAIN_TASK_PRIORITY, 
		                                         IPANEL_MAIN_TASK_STACK_SIZE);
    if (IPANEL_NULL == ipanel_task_pid)
    {
        ipanel_porting_dprintf("[application_entry] ipanel_task create failed\n");
    }

    return ;
}

