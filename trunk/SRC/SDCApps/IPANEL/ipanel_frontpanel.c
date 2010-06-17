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

extern int standard_by_flag ;     // �������ڴ���ģʽ��ʶ

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
        
	// ÿ���Ӹ���һ��ʱ��,��porting������ʵ��
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

   	hwtimer_set(ipanel_led_timer, TIME_1MINUTE, FALSE);  // ÿ1���Ӹ���һ��

	hwtimer_start(ipanel_led_timer);	

	return IPANEL_OK; 
}

static void ipanel_led_time_stop()
{
	hwtimer_stop(ipanel_led_timer);
	hwtimer_destroy(ipanel_led_timer);
}

/********************************************************************************************************
���ܣ�����STB��ǰ��������LED��ָʾ�Ƶ���ʾ����Ҫ��������ʾʱ�䣬������С��Ƶ���ţ�������С��������״̬֮�����Ϣ
ԭ�ͣ�INT32_T ipanel_porting_front_panel_ioctl(IPANEL_FRONT_PANEL_IOCTL_e cmd, VOID *arg)
����˵����
  ���������
    cmd ����������
      typedef enum
      {
        IPANEL_FRONT_PANEL_SHOW_TEXT=1,
        IPANEL_FRONT_PANEL_SHOW_TIME,
        IPANEL_FRONT_PANEL_SET_INDICATOR
      } IPANEL_FRONT_PANEL_IOCTL_e;
    arg - �������������Ĳ�����������ö���ͻ�32λ����ֵʱ��arg��ǿ��ת���ɶ�Ӧ��������
    cmd, argȡֵ���±�
    +---------------------+-------------------------+-----------------------------+
    |  cmd                |   arg                   |  ˵��                       |
    +---------------------+-------------------------+-----------------------------+
    | IPANEL_FRONT_       |ָ���ַ�����ָ�룬�ַ��� |ָ����ǰ�����Ҫ��ʾ���ַ���,|
    |   PANEL_SHOW_TEXT   |������\0������           |������ʾ���ַ�������LED�й�, |
    |                     |                         |һ������ʾ�����ֵ����,���޵�|
    |                     |                         |��ĸ���,�����ַ���.�ַ����� |
    |��Ҳ���������ʾ��λ���й�.�����û������Զ�����ʾ���뷽ʽ�������ַ�����ʾ����|
    +---------------------+-------------------------+-----------------------------+
    | IPANEL_FRONT_       |ָ���ʾʱ����ַ���ָ�� |����LED��ʾʱ����Ϣ,�ַ����� |
    |   PANEL_SHOW_TIME   |                         |��ʽ��"yyyymmddhhmmss",�û���|
    |                     |                         |��ѡ������ʾ����ĳ�ε�����.��|
    |                     | �����ݳ������ǵ�ǰʱ�̵�ֵ,���ʱ��ĸ���������Ҫ��ʵ |
    |                     | ����Ŀ����Ӧ�úͼ��ɷ�Э�̶��塣yyyy:��,��2007; mm:��,|
    |                     | ��12; dd:��,��06; hh:Сʱ,��13,04; mm:����,��05; ss:��|
    |                     | ,��03,59; �������ݲ����涨λ�������ڸ�λ��0,ʱ�����24|
    |                     | Сʱ��ʾ.                                             |
    +---------------------+-------------------------+-----------------------------+
    | IPANEL_FRONT_PANEL  |ָ�����½ṹ��ָ��:      |IPANEL����֧�����32������led||
    |   _SET_INDICATOR    |typedef struct {         |ָʾ��.mask��valueʹ��λ���� |
    |                     | UINT32_T value;         |��ʽ,maskλΪ1��ʾ��ǰҪ���� |
    |                     | UINT32_T mask;          |��ָʾ��,valueλ��ʾ�Ƶ�״̬,|
    |                     |}IPANEL_FRONT_PANEL      |1��ʾ��,0��ʾ��.�����ĸ�λ�� |
    |                     |   _INDICATOR;           |���ĸ���,��ҪӦ�úͼ��ɷ�Э��|
    |                     |                         |����.                        |
    +---------------------+-------------------------+-----------------------------+
  �����������
��    �أ�
  IPANEL_OK���ɹ���
  IPANEL_ERR��ʧ��
********************************************************************************************************/
INT32_T ipanel_porting_front_panel_ioctl(IPANEL_FRONT_PANEL_IOCTL_e cmd, VOID *arg)
{
    INT32_T ret = IPANEL_OK;
    unsigned char *buff = (unsigned char *)arg;

    ipanel_porting_dprintf("[ipanel_porting_front_panel_ioctl] cmd=%d, arg=%p\n", cmd, arg);

    switch (cmd)
    {
        case IPANEL_FRONT_PANEL_SHOW_TEXT :
			/* ֻ��ʾ��Ч�ַ�4�� */
			memcpy(m_led_buff,buff,4);
			panel_pio_display_string(m_led_buff);
            break;

        case IPANEL_FRONT_PANEL_SHOW_TIME :
			/* �ַ����ĸ�ʽ��"0,yyyymmdd,hhmmss",ֻ��ʾСʱ�ͷ���*/
			memcpy(m_time_buff,buff+11, 4);
            panel_pio_display_string(m_time_buff);
            break;

        case IPANEL_FRONT_PANEL_SET_INDICATOR :
		/* ��������ܿ��� */			
		{
			int index ; 
			IPANEL_FRONT_PANEL_INDICATOR *ledStatus ; 
			UINT32_T ledValue , ledMask ; 

			ledStatus = (IPANEL_FRONT_PANEL_INDICATOR*)arg ; 

			ledValue = ledStatus->value ;
			ledMask = ledStatus->mask ;
			for(index= 0 ; index < 8; index++)  // ���ڽ���8���ƿ���
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

