/**********************************************************************
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
#include "ipanel_task.h"
#include "ipanel_tuner.h"
#include "ipanel_porting_event.h"

//--------------------------------------------------------------------------------------------------
// Types and defines
//--------------------------------------------------------------------------------------------------
//
#define TUNER_STENGTH_BASE  640
#define TUNER_QUALITY_BASE  500

#define FREQUENCY_MIN           450000
#define FREQUENCY_MAX           9000000

#define TUNER_LOCK_TIMEOUT      1500 /* 检测超时时间为1.5 s*/

#define CABLE_CONNECTED 	 	  1 /*Cable线连接上*/
#define CABLE_UNCONNECTED         0 /*Cable线未连接上*/
#define CABLE_CONNECTED_UNKNOW 	 -1 /*Cable线连接状态未知*/

/*Tuner任务每100ms运行一次*/
#define TUNER_TASK_PERIOD 100

/*每1s检测一次CABLE线状态好坏*/
#define DETECT_CABLE_STATE_PERIOD 1000

/*保存IPANEL中间件的modulatioin编号和NXPmodulation编号的对应关系的表*/
/*由于IPANEL_MODULATION_MODE_e 编号从1~5ModulationMapTable[ipanel_module -1]即为NXP modulation编号*/
static const NIM_CABLE_MODULATION  ModulationMapTable[IPANEL_MODULATION_QAM256] = 
{
	16,//MOD_QAM16,
	32,//MOD_QAM32,
	64,//MOD_QAM64,
	128,//MOD_QAM128,
	256,//MOD_QAM256
};

/*全局发送消息的队列*/
extern UINT32_T  g_main_queue;

/*Tuner模块锁频任务id*/
static UINT32_T     m_tuner_task_id;

/*锁频任务是否在运行*/
static unsigned int m_tuner_task_running = FALSE;

/*Tuner模块互斥信号量*/
static UINT32_T     m_mutex;

static STunerPara   m_tuning_para;

/*Tuner 的handle*/
static u_int32      m_demod_handle;

/*Tuner最后锁频的频点*/
static u_int32      m_last_freq = 0;

/*Tuner网络状态是否可发*/
static u_int32      m_tuner_flag = 1; 

int m_cable_connect_state = CABLE_CONNECTED_UNKNOW;

/**********************************************************	
	 为了使Tuenr 状态更好表现,可以使用Callback通知函数与
	 任务轮询同时使用，其中Callback主要报告一些Cable  状
	 况的信息(物理连接方面的状态信息) , 
	 由软件的SWI   产生.

	 暂时未使用，为以后扩展使用@!
************************************************************/
#define IPANEL_TUNER_CALLBACK  

#ifdef IPANEL_TUNER_CALLBACK
void ipanel_demod_callback( u_int32 unit, DEMOD_CALLBACK_TYPE type, void *callback_data )
{
    DEMOD_CALLBACK_PARM *data;
	IPANEL_QUEUE_MESSAGE    msg;

	msg.q1stWordOfMsg  = EIS_EVENT_TYPE_DVB;
	msg.q2ndWordOfMsg  = 0;
	msg.q3rdWordOfMsg  = 0;
	msg.q4thWordOfMsg  = 0;

	data = (DEMOD_CALLBACK_PARM *)&((DEMOD_CALLBACK_DATA*)callback_data)->parm;

    if ( type == DEMOD_CONNECT_STATUS )
    {
        switch (data->type)
		{
    		case DEMOD_DRIVER_REACQUIRED_SIGNAL:
    		case DEMOD_CONNECTED:
                if (m_tuning_para.start_tunering)
    		    {
                    if (CABLE_CONNECTED_UNKNOW == m_cable_connect_state)	
			        {
				        m_cable_connect_state = CABLE_CONNECTED; /*此时表示已经开始锁频*/
			        }
                
    			    m_tuning_para.start_tunering = FALSE;
                    ipanel_porting_dprintf("Demod lock success.\n");                

    				msg.q2ndWordOfMsg = EIS_DVB_TUNE_SUCCESS;
    				msg.q3rdWordOfMsg = m_tuning_para.request_id;
    				ipanel_porting_queue_send(g_main_queue, &msg);
					printf("send 8001 msg\n");
                }
                else
                {
                    ipanel_porting_dprintf("Demod connected.\n");

                    if(CABLE_UNCONNECTED == m_cable_connect_state)
                    {
                        m_cable_connect_state = CABLE_CONNECTED;
        				msg.q2ndWordOfMsg = EIS_CABLE_NETWORK_CONNECT;
        				msg.q3rdWordOfMsg = m_tuning_para.request_id;
        				ipanel_porting_queue_send(g_main_queue, &msg);
						printf("send 5550 msg\n");
                    }
                }
                break;
                
            case DEMOD_DRIVER_LOST_SIGNAL:
		    case DEMOD_FAILED:
                if (m_tuning_para.start_tunering)
    		    {
                    if (CABLE_CONNECTED_UNKNOW == m_cable_connect_state)	
			        {
				        m_cable_connect_state = CABLE_UNCONNECTED; /*此时表示已经开始锁频*/
			        }

    			    m_tuning_para.start_tunering = FALSE;
                    ipanel_porting_dprintf("Demod lock failed.\n");                 

    				msg.q2ndWordOfMsg = EIS_DVB_TUNE_FAILED;
    				msg.q3rdWordOfMsg = m_tuning_para.request_id;
    				ipanel_porting_queue_send(g_main_queue, &msg);
					printf("send 8002 msg\n");
                }
                else
                {
                    ipanel_porting_dprintf("Demod disconnected.\n");

                    if(CABLE_CONNECTED == m_cable_connect_state 
						&& m_tuner_flag)
                    {
                        m_cable_connect_state = CABLE_UNCONNECTED;
                    	msg.q2ndWordOfMsg = EIS_CABLE_NETWORK_DISCONNECT;
                    	msg.q3rdWordOfMsg = m_tuning_para.request_id;
                    	ipanel_porting_queue_send(g_main_queue, &msg);
						printf("send 5551 msg\n");
                    }
                }
                break;

            case DEMOD_TIMEOUT:
                ipanel_porting_dprintf("Demod lock timeout, retry. \r\n");
                break;
            default:
                break;
        }
    }
    else if ( type == DEMOD_DISCONNECT_STATUS )
    {
        if( data->type == DEMOD_DISCONNECTED 
			&& m_tuner_flag)
        {
            ipanel_porting_dprintf("Demod disconnected.\n");
			printf("send 5551 msg\n");

        	msg.q2ndWordOfMsg = EIS_CABLE_NETWORK_DISCONNECT;
        	msg.q3rdWordOfMsg = m_tuning_para.request_id;
        	ipanel_porting_queue_send(g_main_queue, &msg);
        }
    }
    else if ( type == DEMOD_SCAN_STATUS )
    {
        ipanel_porting_dprintf("Demod scan completed.\n");
    }
}

static INT32_T ipanel_demod_tuner_frequency()
{
    DEMOD_STATUS status;
    /*锁频硬件相关参数*/
    TUNING_SPEC  tuning_spec; 

	/*设置API的接口参数*/
	tuning_spec.type                                = DEMOD_NIM_CABLE;
	tuning_spec.tune.nim_cable_tune.annex           = ANNEX_A;
	tuning_spec.tune.nim_cable_tune.auto_spectrum   = 1;
	tuning_spec.tune.nim_cable_tune.spectrum        = SPECTRUM_NORMAL;
	tuning_spec.tune.nim_cable_tune.ucSDPMin        = 0;
	tuning_spec.tune.nim_cable_tune.ucSDPMax        = 0;    

	/*转换硬件锁频参数*/
	tuning_spec.tune.nim_cable_tune.frequency   = m_tuning_para.frequency * 100;
	tuning_spec.tune.nim_cable_tune.symbol_rate = m_tuning_para.symbol_rate * 100;
	tuning_spec.tune.nim_cable_tune.modulation  = ModulationMapTable[m_tuning_para.modulation - 1];

    status = cnxt_demod_connect(m_demod_handle, &tuning_spec);
	if (DEMOD_SUCCESS != status)
	{
        ipanel_porting_dprintf("[ipanel_demod_tuner_frequency] tuner param set failed!\n");
        return IPANEL_ERR;
    }

    return IPANEL_OK;
}

#else
/*===========================================================================
Description: 该任务进行锁频处理，应用调用ipanel_porting_tuner_lock_delivery，
触发锁频, 并将锁频结果发送到消息队列，通知MiddleWare
============================================================================*/
static void ipanel_tuner_task(VOID *para)
{
	/*锁频超时时间: 3s*/	
	#define LOCK_OVER_TIME 3000

	/*开始锁频的时间，如果为-1表示没有开始锁频  */
	unsigned int lock_start_time = (unsigned int)-1; 
	
	/*当前的时间*/
	unsigned int current_time;

	/*锁频硬件相关参数*/
	TUNING_SPEC  tuning_spec; 

	IPANEL_QUEUE_MESSAGE    msg;

	bool is_locked = FALSE; /*锁频是否成功*/
	bool is_locked2 =FALSE;

	bool start_detect_cable_state = FALSE; /*是否开始进行CABLE连线状态好坏*/

	/*线程循环的次数，用于记录是否需要检测CABLE连线状态*/
	/*循环100次: 1s时，进行一次检测*/
	unsigned int loop_num = 0;	

	/*设置API的接口参数*/
	tuning_spec.type                                = DEMOD_NIM_CABLE;
	tuning_spec.tune.nim_cable_tune.annex           = ANNEX_A;
	tuning_spec.tune.nim_cable_tune.auto_spectrum   = 1;
	tuning_spec.tune.nim_cable_tune.spectrum        = SPECTRUM_NORMAL;
	tuning_spec.tune.nim_cable_tune.ucSDPMin        = 0;
	tuning_spec.tune.nim_cable_tune.ucSDPMax        = 0;    


	msg.q1stWordOfMsg  = EIS_EVENT_TYPE_DVB;
	msg.q2ndWordOfMsg  = 0;
	msg.q3rdWordOfMsg  = 0;
	msg.q4thWordOfMsg  = 0;
	
	while (m_tuner_task_running)
	{	
		ipanel_porting_sem_wait(m_mutex, IPANEL_WAIT_FOREVER);	

        #ifdef IPANEL_TUNER_CHECK_STATUS
		loop_num++;
		if ( (DETECT_CABLE_STATE_PERIOD / TUNER_TASK_PERIOD) == loop_num) /*达到1s*/
		{
			loop_num = 0;
			if (CABLE_CONNECTED_UNKNOW != m_cable_connect_state) 
			{
				start_detect_cable_state = TRUE; /*当锁频开始后，才进行Cable连线状态检测*/
			}
		}

		/*检测Cable线连接状态好坏，每秒检测一次*/
		if (start_detect_cable_state)		
		{
			cnxt_demod_get_lock_status(m_demod_handle, &is_locked);

			if ((CABLE_UNCONNECTED == m_cable_connect_state) && is_locked)
			{
				m_cable_connect_state = CABLE_CONNECTED;
				msg.q2ndWordOfMsg = EIS_CABLE_NETWORK_CONNECT;
				msg.q3rdWordOfMsg = 0;
				ipanel_porting_dprintf("Connect!!!!!!!!!!!!!!!!!!!!!!\n");
				ipanel_porting_queue_send(g_main_queue, &msg);			
			}
			else if ((CABLE_CONNECTED == m_cable_connect_state) && !is_locked )
			{	
				ipanel_porting_task_sleep(1000);
				cnxt_demod_get_lock_status(m_demod_handle, &is_locked2);
				if(!is_locked2){
					m_cable_connect_state = CABLE_UNCONNECTED;
					msg.q2ndWordOfMsg = EIS_CABLE_NETWORK_DISCONNECT;
					msg.q3rdWordOfMsg = 0;
					ipanel_porting_dprintf("!!!!!!!!!!!!!!!DisConnect!!!!!!!!!!!!!!!!!!!!!!\n");
					ipanel_porting_queue_send(g_main_queue, &msg);	
				}
			}
			
			start_detect_cable_state = FALSE;
		}
        #endif

		/*请求开始锁频*/
		if (m_tuning_para.start_tunering)
		{
			m_tuning_para.start_tunering = FALSE;

			/*转换硬件锁频参数*/
			tuning_spec.tune.nim_cable_tune.frequency   = m_tuning_para.frequency * 100;
			tuning_spec.tune.nim_cable_tune.symbol_rate = m_tuning_para.symbol_rate * 100;
			tuning_spec.tune.nim_cable_tune.modulation  = ModulationMapTable[m_tuning_para.modulation - 1];

						
			/* 记录开始锁频时间 */
			lock_start_time = ipanel_porting_time_ms();

			if (DEMOD_SUCCESS !=  cnxt_demod_connect(m_demod_handle, &tuning_spec))
			{
				msg.q2ndWordOfMsg = EIS_DVB_TUNE_FAILED;
				msg.q3rdWordOfMsg = m_tuning_para.request_id;
				ipanel_porting_queue_send(g_main_queue, &msg);
				ipanel_porting_dprintf("cnxt_demod_connect set failed\n");
				goto LOCK_END;  /*锁频设置不成功*/
			}
		}
		
		if (lock_start_time ==  (unsigned int)-1)  /*没有开始锁频*/
		{
			goto LOCK_END;
		}

		cnxt_demod_get_lock_status(m_demod_handle, &is_locked);
		if (is_locked) /*锁频成功，发送成功消息*/
		{
			msg.q2ndWordOfMsg = EIS_DVB_TUNE_SUCCESS;
			msg.q3rdWordOfMsg = m_tuning_para.request_id;
			ipanel_porting_queue_send(g_main_queue, &msg);
			lock_start_time = (unsigned int)-1; /*重新置为-1,保证只发送一次*/

			ipanel_porting_dprintf("[ipanel_tuner_task] ****** LOCK SUCCEED!\n");
			
			if (CABLE_CONNECTED_UNKNOW == m_cable_connect_state)	
			{
				m_cable_connect_state = CABLE_CONNECTED; /*此时表示已经开始锁频*/
			}
			goto LOCK_END;
		}
		
		current_time = ipanel_porting_time_ms();
		if (current_time < lock_start_time)
		{
			current_time = lock_start_time; /* 时间回溯 */
		}
		else if (lock_start_time + LOCK_OVER_TIME < current_time) /*如果超过锁频超时时间一直不成功，发送失败消息*/
		{			
			msg.q2ndWordOfMsg = EIS_DVB_TUNE_FAILED;
			msg.q3rdWordOfMsg = m_tuning_para.request_id;
			ipanel_porting_queue_send(g_main_queue, &msg);
			lock_start_time = (unsigned int)-1;
			
			if (CABLE_CONNECTED_UNKNOW == m_cable_connect_state)	
			{
				m_cable_connect_state = CABLE_UNCONNECTED; /*此时表示已经开始锁频*/
			}				
			goto LOCK_END;  /*锁频设置不成功*/				
		}
		
LOCK_END:
		ipanel_porting_sem_release(m_mutex);
		ipanel_porting_task_sleep(TUNER_TASK_PERIOD);
	}

	m_tuner_task_running = TRUE; /*此时线程正常退出，将该标志置为TRUE*/
}
#endif

/****************************************************************
	功能说明：得到当前频率信号误码率。
	
	这是个格式化的值，规定如下：
	高十六位表示误码率的整数部分（只取4位整数），
	低16位表示指数部分。

	如果值为{4， -5}   就表示误码率为4.00E-5;
   	如果值为{462, -7}  就表示误码率为4.62E-5;
	如果值为{0, 0}     就表示误码率为0.00E+0;
*****************************************************************/
static INT32_T ipanel_tuner_get_signal_ber(INT32_T tunerid)
{
	INT32_T result = 0x00 ,temp1 = 0, temp2 = 0, Nub=0; 
  	DEMOD_STATUS    bRetcode;
	SIGNAL_STATS statistics;
	float fError, fCount, DemodBER;

	/* Get signal statistics from the demod */
   	bRetcode = cnxt_demod_get_signal_stats(m_demod_handle,&statistics);	
	if(DEMOD_SUCCESS ==  bRetcode)
	{
		/* calculate the Bit Error Rate */
		fError = (float)(statistics.stats.c_signal.rs_uncorrected);
		fCount = (float)(statistics.stats.c_signal.rs_total);

		DemodBER = fError / fCount ; 

        if( DemodBER != 0)
        {
            // 转换成科学计数法
            do
            {
                DemodBER /= 0.1;
                ++Nub;
                if(DemodBER >= 1)
                    break;
            }while(1);

            DemodBER *= 1000;

            temp1 = ((int)DemodBER)%10;
            temp2 = ((int)DemodBER)/10;
            if(temp1 >=5)
                ++temp2;

            result |= temp2<<16;
            result |= (2 + Nub);
        }
        
		ipanel_porting_dprintf("[ipanel_porting_tuner_get_signal_ber] BER= %.2e\n",result);
        
		return result ; 
 	}

	ipanel_porting_dprintf("[ipanel_porting_tuner_get_signal_ber] ERROR: get signal BER error! \n");
	return result ; 
}

/********************************************************************************************************
功能：锁定到指定的频点。锁定成功时，发送成功事件给iPanel MiddleWare，
锁定失败时发送锁定失败事件给iPanel MiddleWare。
该函数需要立即返回，是非阻塞的。由于锁频是要花时间的，
一般做法是把锁频参数传到一个专门的线程，在这个 线程中进行锁频。
当底层调用锁频后，通过消息返回锁频结果，及id号。
这个接口可能会连续调用，如果有多次调用，只执行最后一次，
中间的调用丢弃，尽可能中断正在进行的锁频操作，
进行最后一次调用参数的锁频。正常工作过程中，如果tuner失锁，
必须发送失败消息给iPanel MiddleWare

  原型：INT32_T ipanel_porting_tuner_lock_delivery(INT32_T tunerid, INT32_T frequency,INT32_T symbol_rate, INT32_T modulation, INT32_T id)
  
	参数说明：
	
	  输入参数：
	  tunerid:要操作的tuner编号，从0 开始。
	  frequency: 频率，单位是Hz的十六进制数值；
	  symbol_rate: 符号率，单位是sym/s的十六进制数值；
	  modulation: 调制方式，
	  typedef enum
	  {
      IPANEL_MODULATION_QAM16 = 1,
      IPANEL_MODULATION_QAM32,
      IPANEL_MODULATION_QAM64,
      IPANEL_MODULATION_QAM128,
      IPANEL_MODULATION_QAM256
	  } IPANEL_MODULATION_MODE_e；
	  id：一个标志号,驱动程序向iPanel MiddleWare发送事件时的参数。
	  
		输出参数：无
		
		  返    回：
		  IPANEL_OK:成功;
		  IPANEL_ERR:失败。
		  
			事件说明：
			当锁频成功或失败时需要通过ipanel_proc将下列消息发到iPanel MiddleWare中。
			锁频成功消息：
			event[0] = IPANEL_EVENT_TYPE_DVB,
			event[1] = IPANEL_DVB_TUNE_SUCCESS
			event[2] = id
			锁频失败消息：
			event[0] = IPANEL_EVENT_TYPE_DVB,
			event[1] = IPANEL_DVB_TUNE_FAILED
			event[2] = id
********************************************************************************************************/
INT32_T ipanel_porting_tuner_lock_delivery(
										   INT32_T tunerid,
										   INT32_T frequency,
										   INT32_T symbol_rate,
										   INT32_T modulation,
										   INT32_T request_id
										   )
{
	ipanel_porting_dprintf("[ipanel_porting_tuner_lock_delivery] id=%d, freq=%d, rate=%d, mod=%d, req_id=%d\n",\
						   tunerid, frequency, symbol_rate, modulation, request_id);
	
	/*进入临界区*/
	ipanel_porting_sem_wait(m_mutex, IPANEL_WAIT_FOREVER);
	
	/*锁频参数合法性判断*/
	if ( ((frequency  >= FREQUENCY_MIN) && (frequency <= FREQUENCY_MAX))
		&&  (symbol_rate>  0)
		&& ((modulation >= IPANEL_MODULATION_QAM16) && (modulation <= IPANEL_MODULATION_QAM256)) )
	{
		m_tuning_para.frequency   = frequency;
		m_tuning_para.symbol_rate = symbol_rate;
		m_tuning_para.modulation  = modulation;
		
		m_tuning_para.request_id = request_id;

		m_tuning_para.start_tunering = TRUE; /*置开始锁频标记*/
		
		if(m_last_freq != frequency)
		{			
			m_tuner_flag = 0;
		}
		else
		{
			m_tuner_flag = 1;
		}
		m_last_freq = frequency;   /*记录最后锁频的频点*/
		
	}
	else
	{
		goto ERROR_EXIT; 	/*锁频参数不合法*/
	}
	
#ifdef IPANEL_TUNER_CALLBACK 
    ipanel_demod_tuner_frequency();
#endif
    
	ipanel_porting_sem_release(m_mutex);	 /*退出临界区*/
	
	return IPANEL_OK;
	
ERROR_EXIT:	
	ipanel_porting_sem_release(m_mutex);	 /*退出临界区*/
	return IPANEL_ERR;
}

/********************************************************************************************************
功能说明：
查询tuner的参数和状态
参数说明：
输入参数：
tunerid：
op － 操作命令
typedef enum
{
IPANEL_TUNER_GET_QUALITY =1,
IPANEL_TUNER_GET_STRENGTH,
IPANEL_TUNER_GET_BER,
IPANEL_TUNER_GET_LEVEL,
IPANEL_TUNER_GET_SNR
} IPANEL_TUNER _IOCTL_e;

		arg – 操作命令所带的参数，当传递枚举型或32位整数值时，arg可强制转换成对应数据类型。
		
		  op, arg取值关系见下表：
		  +---------------------+-------------------------+-----------------------------------+
		  |  op                 |   arg                   |  说明                       	    |
		  +---------------------+-------------------------+-----------------------------------+
		  |IPANEL_TUNER_GET_	  |0～100整数值				|得到当前频率信号质量， 质量的范围	|
		  |		QUALITY		  |						    |是0到 100。 0 无信号，100 最强信号	|
		  +---------------------+-------------------------+-----------------------------------+
		  |IPANEL_TUNER_GET_    |0～100整数值             |得到当前频率信号强度。强度的范围	|
		  |   STRENGTH       	  |                       |是0到 100。0 无信号，100 最强信号    |
		  +---------------------+-------------------------+-----------------------------------+
		  | IPANEL_TUNER_GET_	  |整数值             		|得到当前频率信号误码率。这是个格式	|
		  |   BER	       		  |                         |化的值，规定如下： 高十六位表示误码|
		  |					  |							|率的整数部分（只取4位整数），低16位|
		  |					  |							|表示指数部分。						|
		  +---------------------+-------------------------+-----------------------------------+
		  |IPANEL_TUNER_GET_    |整数值             		|得到当前频点信号电平。			    |
		  |   LEVEL       	  |                         |以db为单位的整数值。  			    |
		  +---------------------+-------------------------+-----------------------------------+
		  |IPANEL_TUNER_GET_    |整数值             		|得到当前频点信号信噪比。			|
		  |   SNR       		  |                         |以db为单位的整数值。  				|
		  +---------------------+-------------------------+-----------------------------------+
		  输出参数：
		  无
		  返 回：
		  IPANEL_OK: 函数执行成功;
		  IPANEL_ERR: 函数执行失败。
********************************************************************************************************/
INT32_T ipanel_porting_tuner_ioctl(INT32_T tunerid, IPANEL_TUNER_IOCTL_e op, VOID *arg)
{
	INT32_T ret = IPANEL_OK;
	DEMOD_STATUS status;
	SIGNAL_STATS signal_status;
	
	ipanel_porting_dprintf("[ipanel_porting_tuner_ioctl] id=%d, ip=%d, arg=%p\n", tunerid, op, arg);

	switch (op)
	{
	case IPANEL_TUNER_GET_QUALITY :
		status = cnxt_demod_get_signal_stats(m_demod_handle, &signal_status);
		if (DEMOD_SUCCESS == status)
		{
	        ipanel_porting_dprintf("[%d]\n",signal_status.stats.c_signal.signal_quality); 
			if (signal_status.stats.c_signal.signal_quality >= 850)
			{
				*((INT32_T*)arg) = 100;
			}
			else
			{
				*((INT32_T*)arg) = signal_status.stats.c_signal.signal_quality * 100 / 850;
			}

			ipanel_porting_dprintf("quality=%d.\n",*((INT32_T*)arg));
		}
		else
		{
			ret = IPANEL_ERR;			
		}
		break;
		
	case IPANEL_TUNER_GET_STRENGTH :	      
            status = cnxt_demod_get_signal_stats(m_demod_handle, &signal_status);
            if (DEMOD_SUCCESS == status)
            {
		        ipanel_porting_dprintf("[%d]\n",signal_status.stats.c_signal.signal_strength); 

    			if (signal_status.stats.c_signal.signal_strength >= TUNER_STENGTH_BASE)
    			{
    				*((INT32_T*)arg) = 100;
    			}
    			else
    			{
    	            *((INT32_T*)arg) = signal_status.stats.c_signal.signal_strength * 100 / TUNER_STENGTH_BASE;
    			}

    			ipanel_porting_dprintf("strength=%d.\n",*((INT32_T*)arg));
            }			
		else
		{
			ret = IPANEL_ERR;			
		}
        break;
		
	case IPANEL_TUNER_GET_BER :
		ret = ipanel_tuner_get_signal_ber(tunerid);
        *((INT32_T*)arg) = ret;
		break;
		
	case IPANEL_TUNER_GET_LEVEL :
		break;
		
	case IPANEL_TUNER_GET_SNR :
		status = cnxt_demod_get_signal_stats(m_demod_handle, &signal_status);
		if (DEMOD_SUCCESS == status)
		{
	        ipanel_porting_dprintf("[%d]\n",signal_status.stats.c_signal.signal_quality); 
			if (signal_status.stats.c_signal.signal_quality >= TUNER_QUALITY_BASE)
			{
				*((INT32_T*)arg) = 100;
			}
			else
			{
				*((INT32_T*)arg) = signal_status.stats.c_signal.signal_quality * 100 / TUNER_QUALITY_BASE;
			}

			ipanel_porting_dprintf("SNR=%d.\n",*((INT32_T*)arg));
		}
		else
		{
			ret = IPANEL_ERR;			
		}
		break;
		
	case IPANEL_TUNER_GET_STATUS :
		break;
		
	case IPANEL_TUNER_LOCK :
		break;
		
	default :
		ret = IPANEL_ERR;
	}

	return ret;
}


/********************************************************************************************************
功能：查询tuner是否在锁定状态。
原型：INT32_T ipanel_porting_tuner_get_status(INT32_T tunerid)
参数说明：
输入参数：tunerid:要查询的tuner编号.
输出参数：无
返    回：
IPANEL_TUNER_LOST：没有锁定频点；
IPANEL_TUNER_LOCKED：成功锁定频点；
IPANEL_ERR: 函数执行错误。
********************************************************************************************************/
INT32_T ipanel_porting_tuner_get_status(INT32_T tunerid)
{
	/*只有一个Tuner，tunerid永远为0*/
	int ret = IPANEL_ERR;
	bool is_locked = 0;
	
	ipanel_porting_sem_wait(m_mutex, IPANEL_WAIT_FOREVER);
	
	if (DEMOD_UNINITIALIZED == cnxt_demod_get_lock_status(m_demod_handle, &is_locked))
	{
		goto ERROR_EXIT;
	}
	
	if (is_locked)
	{
		ret = IPANEL_TUNER_LOCKED;
	}		
	else
	{
		ret = IPANEL_TUNER_LOST;
	}
	ipanel_porting_sem_release(m_mutex);
	
	return ret;
	
ERROR_EXIT:
	ipanel_porting_sem_release(m_mutex);
	return IPANEL_ERR;
}

int  ipanel_tuner_init(void)
{
	DEMOD_STATUS    status;
	
	/*tuner子模块类型,初始化为0，无效*/
	DEMOD_NIM_TYPE  demod_nim_type  = 0;
	
	/*需要初始化的模块数*/
	u_int32 unit_module_number;
	
	u_int32  loop;

	m_cable_connect_state = CABLE_CONNECTED_UNKNOW;
	
	m_tuning_para.start_tunering = FALSE;
	m_tuning_para.frequency = 0;
	m_tuning_para.symbol_rate = 0;
	m_tuning_para.modulation = 0;
	m_tuning_para.request_id = 0;
	
	status = (DEMOD_STATUS)cnxt_ts_route_init();
	if (TSROUTE_STATUS_OK != status)
	{
		ipanel_porting_dprintf("[iPanelTunerInit] cnxt_ts_route_init error(%d)\n", status);
		goto ERROR_EXIT;
	}	
	
	/*初始化各个demod子模块*/
	status = cnxt_demod_init(&unit_module_number);
	if (DEMOD_SUCCESS != status)
	{
		ipanel_porting_dprintf("[iPanelTunerInit] cnxt_demod_init error(%d)\n", status);
		goto ERROR_EXIT;
	}
	
	for (loop = 0; loop < unit_module_number; loop++)
	{
		status = cnxt_demod_get_unit_type(loop, &demod_nim_type); /*得到子模块类型*/
		if (DEMOD_SUCCESS == status)
		{
			if (DEMOD_NIM_CABLE == demod_nim_type)
			{
				unit_module_number = loop;
				break;
			}
		}
	}
	
	/*如果没有CABLE子模块，初始化失败*/
	if (DEMOD_NIM_CABLE != demod_nim_type) 
	{
		ipanel_porting_dprintf("[iPanelTunerInit] demod_nim_type error(%d)\n", demod_nim_type);
		goto ERROR_EXIT;
	}
	
	/*创建TS  Source和TS destion 间的路径*/
    status = cnxt_ts_route_input(TS_PORT_DVB, HSDP_DEMUX0);
    if (TSROUTE_STATUS_OK != status)
	{
		ipanel_porting_dprintf("[iPanelTunerInit] cnxt_hsdp_route error(%d)\n", status);
		goto ERROR_EXIT;
	}	
	
	/*打开Tuner，得到handle*/
	status = cnxt_demod_open(unit_module_number, TRUE, &m_demod_handle);
	if (DEMOD_SUCCESS != status)
	{
		ipanel_porting_dprintf("[iPanelTunerInit] cnxt_demod_open error(%d)\n", status);
		goto ERROR_EXIT;
	}	
	
	m_mutex = ipanel_porting_sem_create("TNRS", 1, IPANEL_TASK_WAIT_FIFO);	
	if (IPANEL_NULL == m_mutex)
	{
		ipanel_porting_dprintf("[iPanelTunerInit] g_tuner_sem creation failed\n");
		goto ERROR_EXIT;
	}

#ifdef IPANEL_TUNER_CALLBACK
	status = cnxt_demod_set_callback(m_demod_handle,ipanel_demod_callback);
	if ( status != DEMOD_SUCCESS )
	{
		ipanel_porting_dprintf("[iPanelTunerInit] register notify func failed\n");
		goto ERROR_EXIT;
	}	
#else
	/*创建锁频任务或监控TUNER连接状态任务*/
	m_tuner_task_running = TRUE;
	m_tuner_task_id = ipanel_porting_task_create(
												IPANEL_TUNER_TASK_NAME, 
												ipanel_tuner_task, 
												NULL, 
												IPANEL_TUNER_TASK_PRIORITY, 
												IPANEL_TUNER_TASK_STACK_SIZE);
	if (IPANEL_NULL == m_tuner_task_id)
	{
		ipanel_porting_dprintf("[iPanelTunerInit] task creation failed\n");
		goto ERROR_EXIT;
	}	
#endif

	return IPANEL_OK;
	
ERROR_EXIT:
#ifdef IPANEL_TUNER_CALLBACK
	cnxt_demod_clear_callback(m_demod_handle);
#else
	if (IPANEL_NULL != m_tuner_task_id)
	{
		ipanel_porting_task_destroy(m_tuner_task_id);
		m_tuner_task_running = FALSE;
	}
#endif

	if (IPANEL_NULL != m_mutex)
	{
		ipanel_porting_sem_destroy(m_mutex);
		m_mutex = NULL;
	}
	
	return IPANEL_ERR;
}

void  ipanel_tuner_exit(void)
{
	unsigned int index;
	
	m_tuner_task_running = FALSE;	  
	
	for (index = 0; index < 3; index++)
	{
		if (TRUE == m_tuner_task_running) /*线程已正常退出*/
		{
			break;
		}
		
		ipanel_porting_task_sleep(10); /*延时等待线程正常退出*/
	}

#ifdef IPANEL_TUNER_CALLBACK
	cnxt_demod_clear_callback(m_demod_handle);
#else
	if (IPANEL_NULL != m_tuner_task_id) /*先删除任务再释放资源*/
	{
		ipanel_porting_task_destroy(m_tuner_task_id);
		m_tuner_task_id = NULL;
	}
#endif

	if (IPANEL_NULL != m_mutex)
	{
		ipanel_porting_sem_destroy(m_mutex);
		m_mutex = NULL;		
	}
}

