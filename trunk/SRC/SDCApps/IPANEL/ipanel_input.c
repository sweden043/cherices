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
#include "ipanel_porting_event.h"

extern UINT32_T g_main_queue;

static unsigned int IR_CALLBACK[3] = {0};
static int  m_get_flag = -1; 
static UINT32_T IR_SEM=IPANEL_NULL;

static void input_device_callback( IOFUNCS_MESSAGE eMsg, u_int32 uParam1, u_int32 uParam2)
{
	bool bKeyPress = TRUE ; 
	IPANEL_QUEUE_MESSAGE  keyt_event;
	
	keyt_event.q1stWordOfMsg  = 0;
	keyt_event.q2ndWordOfMsg  = 0;
	keyt_event.q3rdWordOfMsg  = 0;	
	keyt_event.q4thWordOfMsg  = 0;

	if (eMsg == IOFUNCS_MSG_KEY_PRESSED || eMsg == IOFUNCS_MSG_KEY_HOLD )
	{
		keyt_event.q1stWordOfMsg= IPANEL_EVENT_TYPE_KEYDOWN;
	}
	else if (eMsg == IOFUNCS_MSG_KEY_RELEASED)
	{
		keyt_event.q1stWordOfMsg = IPANEL_EVENT_TYPE_KEYUP;
	}

	ipanel_porting_dprintf("[input_device_callback] key code = 0x%x.\n",uParam1);

	/* 转化相应的key code为中间件键值 */
	switch(uParam1)
	{
		case CNXT_UPARROW:
			keyt_event.q2ndWordOfMsg= EIS_IRKEY_UP;
			ipanel_porting_dprintf("key=IPANEL_IRKEY_UP\n");
			break;

		case CNXT_DOWNARROW:
			keyt_event.q2ndWordOfMsg = EIS_IRKEY_DOWN;
			ipanel_porting_dprintf("key=IPANEL_IRKEY_DOWN\n");
			break;

		case CNXT_LARROW:
			keyt_event.q2ndWordOfMsg = EIS_IRKEY_LEFT;
			ipanel_porting_dprintf("key=IPANEL_IRKEY_LEFT\n");
			break;

		case CNXT_RARROW:
			keyt_event.q2ndWordOfMsg = EIS_IRKEY_RIGHT;
			ipanel_porting_dprintf("key=IPANEL_IRKEY_RIGHT\n");
			break;

		case CNXT_ENTER:
			keyt_event.q2ndWordOfMsg = EIS_IRKEY_SELECT;
			ipanel_porting_dprintf("key=IPANEL_IRKEY_SELECT\n");
			break;

		case CNXT_MENU:
			keyt_event.q2ndWordOfMsg = EIS_IRKEY_MENU;
			ipanel_porting_dprintf("key=IPANEL_IRKEY_MENU\n");
			break;

		case CNXT_BLUE:
			keyt_event.q2ndWordOfMsg = EIS_IRKEY_BLUE;
			ipanel_porting_dprintf("key=IPANEL_IRKEY_BLUE\n");
			break;

		case CNXT_YELLOW:
			keyt_event.q2ndWordOfMsg = EIS_IRKEY_YELLOW;
			ipanel_porting_dprintf("key=IPANEL_IRKEY_YELLOW\n");
			break;

		case CNXT_GREEN:
			keyt_event.q2ndWordOfMsg = EIS_IRKEY_GREEN;
			ipanel_porting_dprintf("key=IPANEL_IRKEY_GREEN\n");
			break;

		case CNXT_RED:
			keyt_event.q2ndWordOfMsg = EIS_IRKEY_RED;
			ipanel_porting_dprintf("key=IPANEL_IRKEY_RED\n");
			break;	
			
		case CNXT_MUTE:
			keyt_event.q2ndWordOfMsg = EIS_IRKEY_VOLUME_MUTE;
			ipanel_porting_dprintf("key=IPANEL_IRKEY_VOLUME_MUTE\n");
			break;

		// Digit key 
		case CNXT_0:
			keyt_event.q2ndWordOfMsg = EIS_IRKEY_NUM0;
			ipanel_porting_dprintf("key=IPANEL_IRKEY_NUM0\n");
			break;

		case CNXT_1:
			keyt_event.q2ndWordOfMsg = EIS_IRKEY_NUM1;
			ipanel_porting_dprintf("key=IPANEL_IRKEY_NUM1\n");
			break;

		case CNXT_2:
			keyt_event.q2ndWordOfMsg = EIS_IRKEY_NUM2;
			ipanel_porting_dprintf("key=IPANEL_IRKEY_NUM2\n");
			break;

		case CNXT_3:
			keyt_event.q2ndWordOfMsg = EIS_IRKEY_NUM3;
			ipanel_porting_dprintf("key=IPANEL_IRKEY_NUM3\n");
			break;

		case CNXT_4:
			keyt_event.q2ndWordOfMsg = EIS_IRKEY_NUM4;
			ipanel_porting_dprintf("key=IPANEL_IRKEY_NUM4\n");
			break;

		case CNXT_5:
			keyt_event.q2ndWordOfMsg = EIS_IRKEY_NUM5;
			ipanel_porting_dprintf("key=IPANEL_IRKEY_NUM5\n");
			break;

		case CNXT_6:
			keyt_event.q2ndWordOfMsg = EIS_IRKEY_NUM6;
			ipanel_porting_dprintf("key=IPANEL_IRKEY_NUM6\n");
			break;

		case CNXT_7:
			keyt_event.q2ndWordOfMsg = EIS_IRKEY_NUM7;
			ipanel_porting_dprintf("key=IPANEL_IRKEY_NUM7\n");
			break;

		case CNXT_8:
			keyt_event.q2ndWordOfMsg = EIS_IRKEY_NUM8;
			ipanel_porting_dprintf("key=IPANEL_IRKEY_NUM8\n");
			break;

		case CNXT_9:
			keyt_event.q2ndWordOfMsg = EIS_IRKEY_NUM9;
			ipanel_porting_dprintf("key=IPANEL_IRKEY_NUM9\n");
			break;		

		case CNXT_PAGEUP:
			keyt_event.q2ndWordOfMsg = EIS_IRKEY_PAGE_UP;
			ipanel_porting_dprintf("key=EIS_IRKEY_PAGE_UP\n");
			break;	

		case CNXT_PAGEDOWN:
			keyt_event.q2ndWordOfMsg = EIS_IRKEY_PAGE_DOWN;
			ipanel_porting_dprintf("key=EIS_IRKEY_PAGE_DOWN\n");
			break;

		case CNXT_VOLUP:
			keyt_event.q2ndWordOfMsg = EIS_IRKEY_VOLUME_UP;
			ipanel_porting_dprintf("key=IPANEL_IRKEY_VOLUME_UP\n");
			break;

		case CNXT_VOLDN:
			keyt_event.q2ndWordOfMsg = EIS_IRKEY_VOLUME_DOWN;
			ipanel_porting_dprintf("key=IPANEL_IRKEY_VOLUME_DOWN\n");
			break;		

		case CNXT_POWER:
			//keyt_event.q2ndWordOfMsg = EIS_IRKEY_POWER;
			//ipanel_porting_dprintf("key=IPANEL_IRKEY_POWER\n");
			keyt_event.q2ndWordOfMsg = EIS_IRKEY_STANDBY;
			ipanel_porting_dprintf("key=EIS_IRKEY_STANDBY\n");
			break;

		case CNXT_EXIT:
			keyt_event.q2ndWordOfMsg = EIS_IRKEY_EXIT;
			ipanel_porting_dprintf("key=IPANEL_IRKEY_EXIT\n");
			break;

		case CNXT_VOD:
			keyt_event.q2ndWordOfMsg = EIS_IRKEY_VOD;
			ipanel_porting_dprintf("key=IPANEL_IRKEY_VOD\n");
			break;

        case CNXT_INFO:
			keyt_event.q2ndWordOfMsg = EIS_IRKEY_INFO;
			ipanel_porting_dprintf("key=EIS_IRKEY_INFO\n");
			break;

        case CNXT_RECALL:
			keyt_event.q2ndWordOfMsg = EIS_IRKEY_FAVORITE;
			ipanel_porting_dprintf("key=EIS_IRKEY_FAVORITE\n");
			break;

        case CNXT_TRACK:
			keyt_event.q2ndWordOfMsg = EIS_IRKEY_AUDIO_MODE;
			ipanel_porting_dprintf("key=EIS_IRKEY_AUDIO_MODE\n");
			break;

		case CNXT_BUY:			
			keyt_event.q2ndWordOfMsg = EIS_IRKEY_STOCK;
			ipanel_porting_dprintf("key=EIS_IRKEY_STOCK\n");
			break;

		case CNXT_EPG:
			keyt_event.q2ndWordOfMsg = EIS_IRKEY_EPG;
			ipanel_porting_dprintf("key=EIS_IRKEY_EPG\n");
			break;

		case CNXT_DATEBOAST:
			keyt_event.q2ndWordOfMsg = EIS_IRKEY_DATA_BROADCAST;
			ipanel_porting_dprintf("key=IPANEL_IRKEY_DATA_BROADCAST\n");
			break;

        case CNXT_FN:
			keyt_event.q2ndWordOfMsg = EIS_IRKEY_IME;
			ipanel_porting_dprintf("key=EIS_IRKEY_IME\n");
			break;

        case CNXT_SORT:
			keyt_event.q2ndWordOfMsg = EIS_IRKEY_PROGRAM_TYPE;
			ipanel_porting_dprintf("key=EIS_IRKEY_PROGRAM_TYPE\n");
			break;            

        case CNXT_CINAME:
			keyt_event.q2ndWordOfMsg = EIS_IRKEY_NVOD;
			ipanel_porting_dprintf("key=EIS_IRKEY_NVOD\n");
			break; 

        case CNXT_RATOTV:
			keyt_event.q2ndWordOfMsg = EIS_IRKEY_AUDIO;
			ipanel_porting_dprintf("key=EIS_IRKEY_AUDIO\n");
			break;           

        case CNXT_CHANNEL:
			keyt_event.q2ndWordOfMsg = EIS_IRKEY_PLAYLIST;
			ipanel_porting_dprintf("key=EIS_IRKEY_PLAYLIST\n");
			break;  
					case CNXT_MAIL:
			keyt_event.q2ndWordOfMsg = EIS_IRKEY_INFO;
			ipanel_porting_dprintf("key=EIS_IRKEY_PLAYLIST\n");
			break;
			
		case CNXT_LAST:
			keyt_event.q2ndWordOfMsg = EIS_IRKEY_BACK;
			ipanel_porting_dprintf("key=EIS_IRKEY_PLAYLIST\n");
			break;
           

		default:
			bKeyPress = FALSE ;
			break;
  	}

    ipanel_porting_sem_wait(IR_SEM,IPANEL_WAIT_FOREVER);
    if( bKeyPress )
    {
        IR_CALLBACK[0] = keyt_event.q1stWordOfMsg;
        IR_CALLBACK[1] = keyt_event.q2ndWordOfMsg;
        IR_CALLBACK[2] = 0;
        m_get_flag = 1;
    }
    ipanel_porting_sem_release(IR_SEM);

#ifdef IPANEL_TEST_ENABLE    
	/*将按键消息发送到队列中*/
	if( bKeyPress )
	{
		ipanel_porting_queue_send(g_main_queue, &keyt_event);
	}
#endif
}

INT32_T ipanel_get_ir_key(unsigned int event[])
{
	INT32_T ret = IPANEL_ERR;
	
    ipanel_porting_sem_wait(IR_SEM,IPANEL_WAIT_FOREVER);
    if( m_get_flag == 1)
    {
        event[0] = IR_CALLBACK[0];
        event[1] = IR_CALLBACK[1];
        event[2] = IR_CALLBACK[2];        
        m_get_flag = 0 ;
		ret = IPANEL_OK;
    }
    ipanel_porting_sem_release(IR_SEM);

	return ret;
}

INT32_T ipanel_input_init(VOID)
{
	INT32_T ret = IPANEL_ERR ;
	IOFUNCS_STATUS retCode ;

	retCode = cnxt_input_init() ;
	if (IOFUNCS_STATUS_OK  != retCode)
	{
		ipanel_porting_dprintf("[ipanel_input_init] Can't initialise OSD screen functions!\n" );
		return ret;
	}

    retCode =  cnxt_iofuncs_register_input_device_callback(input_device_callback) ;
	if(IOFUNCS_STATUS_OK != retCode)
	{
		ipanel_porting_dprintf("[ipanel_input_init] Can't register input device callback!\n" );
		return ret;
	}

    IR_SEM = ipanel_porting_sem_create("IRSM",1,IPANEL_TASK_WAIT_FIFO);
    if( IPANEL_NULL == IR_SEM)
	{
		ipanel_porting_dprintf("[ipanel_input_init] create sem failed failed!\n" );
		return ret;
	}
	
	return IPANEL_OK;
}

VOID ipanel_input_exit(VOID)
{
	ipanel_porting_dprintf("[ipanel_ir_exit] is called\n");

	cnxt_input_exit();
}

