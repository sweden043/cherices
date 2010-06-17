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
#include "ipanel_adec.h"
#include "ipanel_vdec.h"
#include "ipanel_vout.h"
#include "ipanel_graphics.h"
#include "ipanel_task.h"
#include "ipanel_nvram.h"
#include "ipanel_device.h"
#include "ipanel_product.h"
#include "ipanel_porting_event.h"

extern UINT32_T g_main_queue ;

extern u_int8  m_stb_mac[6];

extern UINT32_T  ipanel_task_pid ;      // 定义主线程的返回ID 值

extern int       main_continue_flag;    // 定义用于主线程是否继续执行
extern int       reboot_destroy_flag ;  // 定义用于重启线程是否继续执行
extern int       standard_by_flag ;     // 定义用于待机模式标识
u_int8 iibBuf[18]; 
u_int64 nvram_serial_number;





INT32_T ipanel_porting_system_get_info(IPANEL_STB_INFO_e name, CHAR_T *buf, INT32_T len)
{
    INT32_T ret = IPANEL_ERR;

    ipanel_porting_dprintf("[ipanel_porting_system_get_info] name=%d, buf=%p, len=%d\n", name, buf, len);

    switch (name)
    {
        case IPANEL_LOADER_NAME :
	        strcpy(buf, "IPANEL_LOADER");
	        ret = strlen(buf);
            break;

        case IPANEL_LOADER_VERSION :
	        strcpy(buf, "V0.01");
	        ret = strlen(buf);
            break;

        case IPANEL_LOADER_PROVIDER :
	        strcpy(buf, "IPANEL");
	        ret = strlen(buf);
            break;

        case IPANEL_LOADER_SIZE :
	        strcpy(buf, "0x2000");
	        ret = strlen(buf);
            break;

        case IPANEL_DRIVER_NAME :
	        strcpy(buf, "IPANEL_DRIVER");
	        ret = strlen(buf);
            break;

        case IPANEL_DRIVER_VERSION :
	        strcpy(buf, "V3.0");
	        ret = strlen(buf);
            break;

        case IPANEL_DRIVER_PROVIDER :
	        strcpy(buf, "IPANEL");
	        ret = strlen(buf);
            break;

        case IPANEL_DRIVER_SIZE :
	        strcpy(buf, "0x40000");
	        ret = strlen(buf);
            break;

        case IPANEL_HARDWARE_SERIAL :
	        strcpy(buf, "Z:0017-1000-11115");
	        ret = strlen(buf);
            break;

        case IPANEL_HARDWARE_PROVIDER :
	        strcpy(buf, "DAYA");
	        ret = strlen(buf);
            break;

        case IPANEL_PRODUCT_DESC :
	        strcpy(buf, "IPANEL_DEMO_PRODUCT");
	        ret = strlen(buf);
            break;

        case IPANEL_PRODUCT_SERIAL :
			/*
			read_flash((void *)LD_IIB_ADDRESS,18,iibBuf))
			Int2longlong(&iibBuf[6],8,&nvram_serial_number);
			sprintf(buf,"%ld",nvram_serial_number);
			*/
	        strcpy(buf, "IPANEL_123456");
	        ret = strlen(buf);
            break;

        case IPANEL_PRODUCT_MAC_ADDR :
			ipanel_network_get_ioctl(IPANEL_NET_MAC,NULL);
			sprintf(buf, "%02x-%02x-%02x-%02x-%02x-%02x", 
				    m_stb_mac[0], m_stb_mac[1], m_stb_mac[2], m_stb_mac[3],m_stb_mac[4],m_stb_mac[5]);
		    ipanel_porting_dprintf("MAC Address: %s \n", buf);

	        ret = strlen(buf);
            break;

        case IPANEL_PRODUCT_FLASH :
	        strcpy(buf, "0x800000");
	        ret = strlen(buf);
            break;

        case IPANEL_PRODUCT_RAM :
	        strcpy(buf, "0x2000000");
	        ret = strlen(buf);
            break;

        case IPANEL_SMART_CARD_ID :
            break;

        case IPANEL_PRODUCT_CA_NAME :
            strcpy(buf,"TFCA");
	        ret = strlen(buf);
            break;

        case IPANEL_PRODUCT_CA_VERSION :
            strcpy(buf,"TFCA V2.1");
	        ret = strlen(buf);
            break;

        case IPANEL_PRODUCT_CA_PROVIDER :
            strcpy(buf,"TONGFANG");
	        ret = strlen(buf);
            break;

        case IPANEL_PRODUCT_CA_SIZE :
    		strcpy(buf, "0x4000");
    		ret = strlen(buf);
            break;

        case IPANEL_PORTING_VERSION :
            strcpy(buf,"iPanel V3.2");
    		ret = strlen(buf);
            break;

        case IPANEL_SOFTWARE_VERSION :
            strcpy(buf,"iPanel V3.0");
    		ret = strlen(buf);
            break;

        case IPANEL_JFT_CARD_INFO :
            break;

        case IPANEL_JFT_CARD_STATUS :
            break;

        case IPANEL_HARDWARE_PRODUCTIONBATCH :
            break;

        case IPANEL_SIHUA_REGION_ID :
            break;

        case IPANEL_OC_SERVICE_ID :
            break;

        case IPANEL_GD_TABLE_ID :
            break;

        case IPANEL_GD_PID :
            break;

        case IPANEL_BOOT_TYPE :
            break;

        case IPANEL_BOOT_STRING :
            break;

        case IPANEL_PRODUCT_STARTUP_MODE :
            break;

        case IPANEL_PRODUCT_INI_FILENAME :
            break;

        case IPANEL_GET_TIME_ZONE :
            strcpy(buf,"UTC-8.0");
            ret = strlen(buf);
            break;

        case IPANEL_GET_UTC_TIME :
            strcpy(buf,"197001010000");
            ret = strlen(buf);
            break;

        case IPANEL_GET_START_FREQUENCY :
            break;

        case IPANEL_HARDWARE_VERSION :
            GetOuterSerialNumber(buf,20);
    		ret = strlen(buf);
            break;

        case IPANEL_BOOT_VERSION:
            strcpy(buf,"U-BOOT1.4");
    		ret = strlen(buf);
            break;

        case IPANEL_ROOT_FFS_VERSION:
            strcpy(buf,"JFFS2.0");
    		ret = strlen(buf);
            break;

        case IPANEL_OS_VERSION:
            strcpy(buf,"LINUX2.16");
    		ret = strlen(buf);
            break;

        default :
			ipanel_porting_dprintf("[ipanel_porting_get_system_info] ERROR: not a supported value! \n");
            break;
    }

    return ret;
}

INT32_T ipanel_porting_system_set_info(IPANEL_STB_INFO_e name, CHAR_T *buf, INT32_T len)
{
	return IPANEL_OK;	
}

INT32_T ipanel_porting_get_outside_dat_info(
        CHAR_T                    **address,
        INT32_T                    *size,
        IPANEL_RESOURCE_TYPE_e      type
    )
{
    INT32_T ret = IPANEL_ERR;

    if (address && size)
    {
        if (IPANEL_RESOUCE_UI == type)
        {
    		*address = (char*)IPANEL_CORE_UI_ADDRESS;
    		*size = IPANEL_CORE_UI_SIZE;
    		ret = IPANEL_OK ;
        }
        else if (IPANEL_RESOUCE_FONT == type)
        {
        }
    }

	ipanel_porting_dprintf("[ipanel_porting_get_outside_dat_info] address = 0x%x, size = %d, type = %d \n ",
						   *address, *size, type );

    return ret;
}

static void ipanel_reboot_task(void *param)
{	
    int sleep_time;
	ipanel_porting_dprintf("[ipanel_porting_system_reboot] iPanel system is reboot...");

    sleep_time = *((int*)param);
    
    ipanel_porting_task_sleep(sleep_time*1000);
    main_continue_flag = 0;
	
	while(1)
	{
        // 稍等会让主任务退出
		ipanel_porting_task_sleep(1000);
        
		if(reboot_destroy_flag)
		{
			ipanel_porting_task_destroy(ipanel_task_pid);
			ipanel_task_pid = IPANEL_NULL;
            
			reboot_IRD(); //reset stb
		}
	}
}

INT32_T ipanel_porting_system_reboot(INT32_T s)
{
	reboot_IRD();
	return IPANEL_OK;
	/*

    INT32_T ret = IPANEL_ERR;
	UINT32_T reboot_task_id = IPANEL_NULL;

    ipanel_porting_dprintf("[ipanel_porting_system_reboot] s=%d\n", s);
	reboot_task_id = ipanel_porting_task_create(
	                                            IPANEL_REBOOT_TASK_NAME,
	                                            ipanel_reboot_task,
	                                            (void*)&s,
	                                            IPANEL_REBOOT_TASK_PRIORITY,
	                                            IPANEL_REBOOT_TASK_STACK_SIZE
	                                            );
	if( IPANEL_NULL == reboot_task_id )
	{
        ipanel_porting_dprintf("[ipanel_porting_system_reboot] create reboot thread failed!\n");
        ret = IPANEL_ERR ; 
	}

	ipanel_porting_dprintf("[ipanel_porting_system_reboot] create reboot thread success!\n");

    return IPANEL_OK;
	*/
}

static int ipanel_entry_standby_mode()
{
    UINT32_T aDecHandle,vDechandle;

    aDecHandle = ipanel_porting_adec_open();
    vDechandle = ipanel_porting_vdec_open();

    // 静音功能
    ipanel_porting_adec_ioctl(aDecHandle,IPANEL_ADEC_SET_MUTE,0);
    
    // 关闭掉音视频
    ipanel_porting_adec_ioctl(aDecHandle,IPANEL_ADEC_STOP,0);
    ipanel_porting_vdec_ioctl(vDechandle,IPANEL_VDEC_STOP,0);

    // 输出蓝屏
    ipanel_osd_hide_region();
    ipanel_set_encoder_output();

    return IPANEL_OK;
}

INT32_T ipanel_porting_system_standby(INT32_T s)
{
    INT32_T ret = IPANEL_ERR;
    IPANEL_QUEUE_MESSAGE msg = {0};

    ipanel_porting_dprintf("[ipanel_porting_system_standby] s=%d\n", s);

    if( standard_by_flag == 0)
    {
        ipanel_entry_standby_mode();        

        if( IPANEL_WAIT_FOREVER == s)
        {
            s = 0;
        }

        standard_by_flag = 1;

        msg.q1stWordOfMsg = EIS_EVENT_TYPE_SYSTEM;
        msg.q2ndWordOfMsg = IPANEL_STB_SUSPEND ;
        msg.q3rdWordOfMsg = s ;
        msg.q4thWordOfMsg = 0 ;

        ret = ipanel_porting_queue_send(g_main_queue,&msg);
        if(ret == IPANEL_ERR)
        {
            ipanel_porting_dprintf("[ipanel_porting_system_standby] send msg failed!\n");
            ret = IPANEL_ERR;
        }
    }

    return ret;
}

