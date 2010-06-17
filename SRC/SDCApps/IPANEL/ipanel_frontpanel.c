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
#include "ipanel_frontpanel.h"

extern int standard_by_flag ;     // 定义用于待机模式标识

extern bool cnxt_btn_led_number(u_int32 word, unsigned char pos, bool bHex,u_int8 TA);
extern int panel_pio_display_string(unsigned char string[4]);
extern int panel_pio_led_set(u_int32 num, bool bOn);

static unsigned char m_led_buff[5] = {0};
static unsigned char m_time_buff[5] = {0};

static timer_id_t     ipanel_led_timer ; 

static void ipanel_led_timer_handler(timer_id_t timer, void *userData)
{
    static unsigned char hour=0;
    static unsigned char minutes=0;
    static bool b_start_flag = 0;
        
	// 每分钟更新一下时间,由porting层自已实现
    if(standard_by_flag)
    {
        if(b_start_flag ==0)
        {
            hour = (m_time_buff[0]-'0')*10 + (m_time_buff[1]-'0'); 
            minutes = (m_time_buff[2]-'0')*10 + (m_time_buff[3]-'0'); 
            b_start_flag = 1;
        }
        else
        {
            minutes++;
            if(minutes >= 60)
            {
                minutes = 0;
                hour += 1;

                if(hour >= 24)
                   hour = 0; 
            }

            m_time_buff[0] = (hour/10)+'0';
            m_time_buff[1] = (hour%10)+'0';
            m_time_buff[2] = (minutes/10)+'0';
            m_time_buff[3] = (minutes%10)+'0';

            panel_pio_display_string(m_time_buff);
        }
    }

	return ; 
}

static INT32_T ipanel_led_time_start()
{
	INT32_T ret = IPANEL_ERR ;
	#define TIME_1MINUTE (60*1000*1000) //us	
	
   	ipanel_led_timer = hwtimer_create(ipanel_led_timer_handler, 0, "LEDS");
	if( 0 == ipanel_led_timer)
	{
		ipanel_porting_dprintf("[ipanel_led_time_start] create time handle failed!\n");
		return ret ; 
	}

   	hwtimer_set(ipanel_led_timer, TIME_1MINUTE, FALSE);  // 每1分钟更新一下

	hwtimer_start(ipanel_led_timer);	

	return IPANEL_OK; 
}

static void ipanel_led_time_stop()
{
	hwtimer_stop(ipanel_led_timer);
	hwtimer_destroy(ipanel_led_timer);
}

/********************************************************************************************************
功能：控制STB的前面板的数码LED和指示灯的显示。主要是用来显示时间，声音大小，频道号，声音大小，机顶盒状态之类的信息
原型：INT32_T ipanel_porting_front_panel_ioctl(IPANEL_FRONT_PANEL_IOCTL_e cmd, VOID *arg)
参数说明：
  输入参数：
    cmd ：控制命令
      typedef enum
      {
        IPANEL_FRONT_PANEL_SHOW_TEXT=1,
        IPANEL_FRONT_PANEL_SHOW_TIME,
        IPANEL_FRONT_PANEL_SET_INDICATOR
      } IPANEL_FRONT_PANEL_IOCTL_e;
    arg - 操作命令所带的参数，当传递枚举型或32位整数值时，arg可强制转换成对应数据类型
    cmd, arg取值见下表
    +---------------------+-------------------------+-----------------------------+
    |  cmd                |   arg                   |  说明                       |
    +---------------------+-------------------------+-----------------------------+
    | IPANEL_FRONT_       |指向字符串的指针，字符串 |指定在前面板上要显示的字符串,|
    |   PANEL_SHOW_TEXT   |必须以\0结束。           |所能显示的字符与数码LED有关, |
    |                     |                         |一般能显示出数字的组合,有限的|
    |                     |                         |字母组合,特殊字符等.字符串长 |
    |度也与盒子能显示的位数有关.集成用户可以自定义显示对齐方式、超常字符串显示处理|
    +---------------------+-------------------------+-----------------------------+
    | IPANEL_FRONT_       |指向表示时间的字符串指针 |数码LED显示时间信息,字符串的 |
    |   PANEL_SHOW_TIME   |                         |格式是"yyyymmddhhmmss",用户可|
    |                     |                         |以选择性显示其中某段的内容.用|
    |                     | 户传递出来的是当前时刻的值,面板时间的更新问题需要在实 |
    |                     | 际项目中由应用和集成方协商定义。yyyy:年,如2007; mm:月,|
    |                     | 如12; dd:日,如06; hh:小时,如13,04; mm:分钟,如05; ss:秒|
    |                     | ,如03,59; 以上数据不够规定位数都是在高位补0,时间采用24|
    |                     | 小时表示.                                             |
    +---------------------+-------------------------+-----------------------------+
    | IPANEL_FRONT_PANEL  |指向如下结构的指针:      |IPANEL定义支持最多32个独立led||
    |   _SET_INDICATOR    |typedef struct {         |指示灯.mask和value使用位控制 |
    |                     | UINT32_T value;         |方式,mask位为1表示当前要操作 |
    |                     | UINT32_T mask;          |的指示灯,value位表示灯的状态,|
    |                     |}IPANEL_FRONT_PANEL      |1表示亮,0表示灭.具体哪个位控 |
    |                     |   _INDICATOR;           |制哪个灯,需要应用和集成方协商|
    |                     |                         |定义.                        |
    +---------------------+-------------------------+-----------------------------+
  输出参数：无
返    回：
  IPANEL_OK：成功，
  IPANEL_ERR：失败
********************************************************************************************************/
INT32_T ipanel_porting_front_panel_ioctl(IPANEL_FRONT_PANEL_IOCTL_e cmd, VOID *arg)
{
    INT32_T ret = IPANEL_OK;
    unsigned char *buff = (unsigned char *)arg;

    ipanel_porting_dprintf("[ipanel_porting_front_panel_ioctl] cmd=%d, arg=%p\n", cmd, arg);

    switch (cmd)
    {
        case IPANEL_FRONT_PANEL_SHOW_TEXT :
			/* 只显示有效字符4个 */
			memcpy(m_led_buff,buff,4);
			panel_pio_display_string(m_led_buff);
            break;

        case IPANEL_FRONT_PANEL_SHOW_TIME :
			/* 字符串的格式是"0,yyyymmdd,hhmmss",只显示小时和分钟*/
			memcpy(m_time_buff,buff+11, 4);
            panel_pio_display_string(m_time_buff);
            break;

        case IPANEL_FRONT_PANEL_SET_INDICATOR :
		/* 单个数码管控制 */			
		{
			int index ; 
			IPANEL_FRONT_PANEL_INDICATOR *ledStatus ; 
			UINT32_T ledValue , ledMask ; 

			ledStatus = (IPANEL_FRONT_PANEL_INDICATOR*)arg ; 

			ledValue = ledStatus->value ;
			ledMask = ledStatus->mask ;
			for(index= 0 ; index < 8; index++)  // 现在仅有8个灯可用
			{				
				if( ledValue & 0x01 )
				{
					panel_pio_led_set(index,ledMask&0x01);
				}
				
				ledValue= ledValue>>1 ; 
				ledMask= ledMask>>1 ; 
			}
			
			break;
		}

        case IPANEL_FRONT_PANEL_REMAP_KEYS :
            break;

        default :
			ipanel_porting_dprintf("[ipanel_porting_front_panel_ioctl] Unknown parameter!\n");
            ret = IPANEL_ERR;
    }

    m_led_buff[4] = '\0';
    ipanel_porting_dprintf("[ipanel_porting_front_panel_ioctl] m_led_buff = %s.\n",m_led_buff);
    
    return ret;
}

INT32_T ipanel_frontpanel_init()
{
	INT32_T ret = IPANEL_ERR ; 
	
	ret = ipanel_led_time_start();
	if( IPANEL_ERR == ret) 
	{
		ipanel_porting_dprintf("[ipanel_product_init] start time failed!\n");
		return ret ; 
	}

	return (ret=IPANEL_OK);
}

void ipanel_frontpanel_exit()
{
	ipanel_led_time_stop();
}

void ipanel_fp_show_test()
{
	unsigned char test_buff[4];

	memcpy(test_buff,"load",4);
	ipanel_porting_front_panel_ioctl(IPANEL_FRONT_PANEL_SHOW_TEXT,(void*)test_buff);
	ipanel_porting_task_sleep(2000);

	memcpy(test_buff,"0000",4);
	ipanel_porting_front_panel_ioctl(IPANEL_FRONT_PANEL_SHOW_TEXT,(void*)test_buff);
	ipanel_porting_task_sleep(2000);

	memcpy(test_buff,"data",4);
	ipanel_porting_front_panel_ioctl(IPANEL_FRONT_PANEL_SHOW_TEXT,(void*)test_buff);
	ipanel_porting_task_sleep(2000);

	memcpy(test_buff,"c001",4);
	ipanel_porting_front_panel_ioctl(IPANEL_FRONT_PANEL_SHOW_TEXT,(void*)test_buff);
	ipanel_porting_task_sleep(2000);

	memcpy(test_buff,"r002",4);
	ipanel_porting_front_panel_ioctl(IPANEL_FRONT_PANEL_SHOW_TEXT,(void*)test_buff);
	ipanel_porting_task_sleep(2000);

	memcpy(test_buff,"0927",4);
	ipanel_porting_front_panel_ioctl(IPANEL_FRONT_PANEL_SHOW_TEXT,(void*)test_buff);
	ipanel_porting_task_sleep(2000);
}

