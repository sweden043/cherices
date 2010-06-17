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

#define TUNER_LOCK_TIMEOUT      1500 /* ��ⳬʱʱ��Ϊ1.5 s*/

#define CABLE_CONNECTED 	 	  1 /*Cable��������*/
#define CABLE_UNCONNECTED         0 /*Cable��δ������*/
#define CABLE_CONNECTED_UNKNOW 	 -1 /*Cable������״̬δ֪*/

/*Tuner����ÿ100ms����һ��*/
#define TUNER_TASK_PERIOD 100

/*ÿ1s���һ��CABLE��״̬�û�*/
#define DETECT_CABLE_STATE_PERIOD 1000

/*����IPANEL�м����modulatioin��ź�NXPmodulation��ŵĶ�Ӧ��ϵ�ı�*/
/*����IPANEL_MODULATION_MODE_e ��Ŵ�1~5ModulationMapTable[ipanel_module -1]��ΪNXP modulation���*/
static const NIM_CABLE_MODULATION  ModulationMapTable[IPANEL_MODULATION_QAM256] = 
{
	16,//MOD_QAM16,
	32,//MOD_QAM32,
	64,//MOD_QAM64,
	128,//MOD_QAM128,
	256,//MOD_QAM256
};

/*ȫ�ַ�����Ϣ�Ķ���*/
extern UINT32_T  g_main_queue;

/*Tunerģ����Ƶ����id*/
static UINT32_T     m_tuner_task_id;

/*��Ƶ�����Ƿ�������*/
static unsigned int m_tuner_task_running = FALSE;

/*Tunerģ�黥���ź���*/
static UINT32_T     m_mutex;

static STunerPara   m_tuning_para;

/*Tuner ��handle*/
static u_int32      m_demod_handle;

/*Tuner�����Ƶ��Ƶ��*/
static u_int32      m_last_freq = 0;

/*Tuner����״̬�Ƿ�ɷ�*/
static u_int32      m_tuner_flag = 1; 

int m_cable_connect_state = CABLE_CONNECTED_UNKNOW;

/**********************************************************	
	 Ϊ��ʹTuenr ״̬���ñ���,����ʹ��Callback֪ͨ������
	 ������ѯͬʱʹ�ã�����Callback��Ҫ����һЩCable  ״
	 ������Ϣ(�������ӷ����״̬��Ϣ) , 
	 �������SWI   ����.

	 ��ʱδʹ�ã�Ϊ�Ժ���չʹ��@!
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
				        m_cable_connect_state = CABLE_CONNECTED; /*��ʱ��ʾ�Ѿ���ʼ��Ƶ*/
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
				        m_cable_connect_state = CABLE_UNCONNECTED; /*��ʱ��ʾ�Ѿ���ʼ��Ƶ*/
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
    /*��ƵӲ����ز���*/
    TUNING_SPEC  tuning_spec; 

	/*����API�Ľӿڲ���*/
	tuning_spec.type                                = DEMOD_NIM_CABLE;
	tuning_spec.tune.nim_cable_tune.annex           = ANNEX_A;
	tuning_spec.tune.nim_cable_tune.auto_spectrum   = 1;
	tuning_spec.tune.nim_cable_tune.spectrum        = SPECTRUM_NORMAL;
	tuning_spec.tune.nim_cable_tune.ucSDPMin        = 0;
	tuning_spec.tune.nim_cable_tune.ucSDPMax        = 0;    

	/*ת��Ӳ����Ƶ����*/
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
Description: �����������Ƶ����Ӧ�õ���ipanel_porting_tuner_lock_delivery��
������Ƶ, ������Ƶ������͵���Ϣ���У�֪ͨMiddleWare
============================================================================*/
static void ipanel_tuner_task(VOID *para)
{
	/*��Ƶ��ʱʱ��: 3s*/	
	#define LOCK_OVER_TIME 3000

	/*��ʼ��Ƶ��ʱ�䣬���Ϊ-1��ʾû�п�ʼ��Ƶ  */
	unsigned int lock_start_time = (unsigned int)-1; 
	
	/*��ǰ��ʱ��*/
	unsigned int current_time;

	/*��ƵӲ����ز���*/
	TUNING_SPEC  tuning_spec; 

	IPANEL_QUEUE_MESSAGE    msg;

	bool is_locked = FALSE; /*��Ƶ�Ƿ�ɹ�*/
	bool is_locked2 =FALSE;

	bool start_detect_cable_state = FALSE; /*�Ƿ�ʼ����CABLE����״̬�û�*/

	/*�߳�ѭ���Ĵ��������ڼ�¼�Ƿ���Ҫ���CABLE����״̬*/
	/*ѭ��100��: 1sʱ������һ�μ��*/
	unsigned int loop_num = 0;	

	/*����API�Ľӿڲ���*/
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
		if ( (DETECT_CABLE_STATE_PERIOD / TUNER_TASK_PERIOD) == loop_num) /*�ﵽ1s*/
		{
			loop_num = 0;
			if (CABLE_CONNECTED_UNKNOW != m_cable_connect_state) 
			{
				start_detect_cable_state = TRUE; /*����Ƶ��ʼ�󣬲Ž���Cable����״̬���*/
			}
		}

		/*���Cable������״̬�û���ÿ����һ��*/
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

		/*����ʼ��Ƶ*/
		if (m_tuning_para.start_tunering)
		{
			m_tuning_para.start_tunering = FALSE;

			/*ת��Ӳ����Ƶ����*/
			tuning_spec.tune.nim_cable_tune.frequency   = m_tuning_para.frequency * 100;
			tuning_spec.tune.nim_cable_tune.symbol_rate = m_tuning_para.symbol_rate * 100;
			tuning_spec.tune.nim_cable_tune.modulation  = ModulationMapTable[m_tuning_para.modulation - 1];

						
			/* ��¼��ʼ��Ƶʱ�� */
			lock_start_time = ipanel_porting_time_ms();

			if (DEMOD_SUCCESS !=  cnxt_demod_connect(m_demod_handle, &tuning_spec))
			{
				msg.q2ndWordOfMsg = EIS_DVB_TUNE_FAILED;
				msg.q3rdWordOfMsg = m_tuning_para.request_id;
				ipanel_porting_queue_send(g_main_queue, &msg);
				ipanel_porting_dprintf("cnxt_demod_connect set failed\n");
				goto LOCK_END;  /*��Ƶ���ò��ɹ�*/
			}
		}
		
		if (lock_start_time ==  (unsigned int)-1)  /*û�п�ʼ��Ƶ*/
		{
			goto LOCK_END;
		}

		cnxt_demod_get_lock_status(m_demod_handle, &is_locked);
		if (is_locked) /*��Ƶ�ɹ������ͳɹ���Ϣ*/
		{
			msg.q2ndWordOfMsg = EIS_DVB_TUNE_SUCCESS;
			msg.q3rdWordOfMsg = m_tuning_para.request_id;
			ipanel_porting_queue_send(g_main_queue, &msg);
			lock_start_time = (unsigned int)-1; /*������Ϊ-1,��ֻ֤����һ��*/

			ipanel_porting_dprintf("[ipanel_tuner_task] ****** LOCK SUCCEED!\n");
			
			if (CABLE_CONNECTED_UNKNOW == m_cable_connect_state)	
			{
				m_cable_connect_state = CABLE_CONNECTED; /*��ʱ��ʾ�Ѿ���ʼ��Ƶ*/
			}
			goto LOCK_END;
		}
		
		current_time = ipanel_porting_time_ms();
		if (current_time < lock_start_time)
		{
			current_time = lock_start_time; /* ʱ����� */
		}
		else if (lock_start_time + LOCK_OVER_TIME < current_time) /*���������Ƶ��ʱʱ��һֱ���ɹ�������ʧ����Ϣ*/
		{			
			msg.q2ndWordOfMsg = EIS_DVB_TUNE_FAILED;
			msg.q3rdWordOfMsg = m_tuning_para.request_id;
			ipanel_porting_queue_send(g_main_queue, &msg);
			lock_start_time = (unsigned int)-1;
			
			if (CABLE_CONNECTED_UNKNOW == m_cable_connect_state)	
			{
				m_cable_connect_state = CABLE_UNCONNECTED; /*��ʱ��ʾ�Ѿ���ʼ��Ƶ*/
			}				
			goto LOCK_END;  /*��Ƶ���ò��ɹ�*/				
		}
		
LOCK_END:
		ipanel_porting_sem_release(m_mutex);
		ipanel_porting_task_sleep(TUNER_TASK_PERIOD);
	}

	m_tuner_task_running = TRUE; /*��ʱ�߳������˳������ñ�־��ΪTRUE*/
}
#endif

/****************************************************************
	����˵�����õ���ǰƵ���ź������ʡ�
	
	���Ǹ���ʽ����ֵ���涨���£�
	��ʮ��λ��ʾ�����ʵ��������֣�ֻȡ4λ��������
	��16λ��ʾָ�����֡�

	���ֵΪ{4�� -5}   �ͱ�ʾ������Ϊ4.00E-5;
   	���ֵΪ{462, -7}  �ͱ�ʾ������Ϊ4.62E-5;
	���ֵΪ{0, 0}     �ͱ�ʾ������Ϊ0.00E+0;
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
            // ת���ɿ�ѧ������
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
���ܣ�������ָ����Ƶ�㡣�����ɹ�ʱ�����ͳɹ��¼���iPanel MiddleWare��
����ʧ��ʱ��������ʧ���¼���iPanel MiddleWare��
�ú�����Ҫ�������أ��Ƿ������ġ�������Ƶ��Ҫ��ʱ��ģ�
һ�������ǰ���Ƶ��������һ��ר�ŵ��̣߳������ �߳��н�����Ƶ��
���ײ������Ƶ��ͨ����Ϣ������Ƶ�������id�š�
����ӿڿ��ܻ��������ã�����ж�ε��ã�ִֻ�����һ�Σ�
�м�ĵ��ö������������ж����ڽ��е���Ƶ������
�������һ�ε��ò�������Ƶ���������������У����tunerʧ����
���뷢��ʧ����Ϣ��iPanel MiddleWare

  ԭ�ͣ�INT32_T ipanel_porting_tuner_lock_delivery(INT32_T tunerid, INT32_T frequency,INT32_T symbol_rate, INT32_T modulation, INT32_T id)
  
	����˵����
	
	  ���������
	  tunerid:Ҫ������tuner��ţ���0 ��ʼ��
	  frequency: Ƶ�ʣ���λ��Hz��ʮ��������ֵ��
	  symbol_rate: �����ʣ���λ��sym/s��ʮ��������ֵ��
	  modulation: ���Ʒ�ʽ��
	  typedef enum
	  {
      IPANEL_MODULATION_QAM16 = 1,
      IPANEL_MODULATION_QAM32,
      IPANEL_MODULATION_QAM64,
      IPANEL_MODULATION_QAM128,
      IPANEL_MODULATION_QAM256
	  } IPANEL_MODULATION_MODE_e��
	  id��һ����־��,����������iPanel MiddleWare�����¼�ʱ�Ĳ�����
	  
		�����������
		
		  ��    �أ�
		  IPANEL_OK:�ɹ�;
		  IPANEL_ERR:ʧ�ܡ�
		  
			�¼�˵����
			����Ƶ�ɹ���ʧ��ʱ��Ҫͨ��ipanel_proc��������Ϣ����iPanel MiddleWare�С�
			��Ƶ�ɹ���Ϣ��
			event[0] = IPANEL_EVENT_TYPE_DVB,
			event[1] = IPANEL_DVB_TUNE_SUCCESS
			event[2] = id
			��Ƶʧ����Ϣ��
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
	
	/*�����ٽ���*/
	ipanel_porting_sem_wait(m_mutex, IPANEL_WAIT_FOREVER);
	
	/*��Ƶ�����Ϸ����ж�*/
	if ( ((frequency  >= FREQUENCY_MIN) && (frequency <= FREQUENCY_MAX))
		&&  (symbol_rate>  0)
		&& ((modulation >= IPANEL_MODULATION_QAM16) && (modulation <= IPANEL_MODULATION_QAM256)) )
	{
		m_tuning_para.frequency   = frequency;
		m_tuning_para.symbol_rate = symbol_rate;
		m_tuning_para.modulation  = modulation;
		
		m_tuning_para.request_id = request_id;

		m_tuning_para.start_tunering = TRUE; /*�ÿ�ʼ��Ƶ���*/
		
		if(m_last_freq != frequency)
		{			
			m_tuner_flag = 0;
		}
		else
		{
			m_tuner_flag = 1;
		}
		m_last_freq = frequency;   /*��¼�����Ƶ��Ƶ��*/
		
	}
	else
	{
		goto ERROR_EXIT; 	/*��Ƶ�������Ϸ�*/
	}
	
#ifdef IPANEL_TUNER_CALLBACK 
    ipanel_demod_tuner_frequency();
#endif
    
	ipanel_porting_sem_release(m_mutex);	 /*�˳��ٽ���*/
	
	return IPANEL_OK;
	
ERROR_EXIT:	
	ipanel_porting_sem_release(m_mutex);	 /*�˳��ٽ���*/
	return IPANEL_ERR;
}

/********************************************************************************************************
����˵����
��ѯtuner�Ĳ�����״̬
����˵����
���������
tunerid��
op �� ��������
typedef enum
{
IPANEL_TUNER_GET_QUALITY =1,
IPANEL_TUNER_GET_STRENGTH,
IPANEL_TUNER_GET_BER,
IPANEL_TUNER_GET_LEVEL,
IPANEL_TUNER_GET_SNR
} IPANEL_TUNER _IOCTL_e;

		arg �C �������������Ĳ�����������ö���ͻ�32λ����ֵʱ��arg��ǿ��ת���ɶ�Ӧ�������͡�
		
		  op, argȡֵ��ϵ���±�
		  +---------------------+-------------------------+-----------------------------------+
		  |  op                 |   arg                   |  ˵��                       	    |
		  +---------------------+-------------------------+-----------------------------------+
		  |IPANEL_TUNER_GET_	  |0��100����ֵ				|�õ���ǰƵ���ź������� �����ķ�Χ	|
		  |		QUALITY		  |						    |��0�� 100�� 0 ���źţ�100 ��ǿ�ź�	|
		  +---------------------+-------------------------+-----------------------------------+
		  |IPANEL_TUNER_GET_    |0��100����ֵ             |�õ���ǰƵ���ź�ǿ�ȡ�ǿ�ȵķ�Χ	|
		  |   STRENGTH       	  |                       |��0�� 100��0 ���źţ�100 ��ǿ�ź�    |
		  +---------------------+-------------------------+-----------------------------------+
		  | IPANEL_TUNER_GET_	  |����ֵ             		|�õ���ǰƵ���ź������ʡ����Ǹ���ʽ	|
		  |   BER	       		  |                         |����ֵ���涨���£� ��ʮ��λ��ʾ����|
		  |					  |							|�ʵ��������֣�ֻȡ4λ����������16λ|
		  |					  |							|��ʾָ�����֡�						|
		  +---------------------+-------------------------+-----------------------------------+
		  |IPANEL_TUNER_GET_    |����ֵ             		|�õ���ǰƵ���źŵ�ƽ��			    |
		  |   LEVEL       	  |                         |��dbΪ��λ������ֵ��  			    |
		  +---------------------+-------------------------+-----------------------------------+
		  |IPANEL_TUNER_GET_    |����ֵ             		|�õ���ǰƵ���ź�����ȡ�			|
		  |   SNR       		  |                         |��dbΪ��λ������ֵ��  				|
		  +---------------------+-------------------------+-----------------------------------+
		  ���������
		  ��
		  �� �أ�
		  IPANEL_OK: ����ִ�гɹ�;
		  IPANEL_ERR: ����ִ��ʧ�ܡ�
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
���ܣ���ѯtuner�Ƿ�������״̬��
ԭ�ͣ�INT32_T ipanel_porting_tuner_get_status(INT32_T tunerid)
����˵����
���������tunerid:Ҫ��ѯ��tuner���.
�����������
��    �أ�
IPANEL_TUNER_LOST��û������Ƶ�㣻
IPANEL_TUNER_LOCKED���ɹ�����Ƶ�㣻
IPANEL_ERR: ����ִ�д���
********************************************************************************************************/
INT32_T ipanel_porting_tuner_get_status(INT32_T tunerid)
{
	/*ֻ��һ��Tuner��tunerid��ԶΪ0*/
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
	
	/*tuner��ģ������,��ʼ��Ϊ0����Ч*/
	DEMOD_NIM_TYPE  demod_nim_type  = 0;
	
	/*��Ҫ��ʼ����ģ����*/
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
	
	/*��ʼ������demod��ģ��*/
	status = cnxt_demod_init(&unit_module_number);
	if (DEMOD_SUCCESS != status)
	{
		ipanel_porting_dprintf("[iPanelTunerInit] cnxt_demod_init error(%d)\n", status);
		goto ERROR_EXIT;
	}
	
	for (loop = 0; loop < unit_module_number; loop++)
	{
		status = cnxt_demod_get_unit_type(loop, &demod_nim_type); /*�õ���ģ������*/
		if (DEMOD_SUCCESS == status)
		{
			if (DEMOD_NIM_CABLE == demod_nim_type)
			{
				unit_module_number = loop;
				break;
			}
		}
	}
	
	/*���û��CABLE��ģ�飬��ʼ��ʧ��*/
	if (DEMOD_NIM_CABLE != demod_nim_type) 
	{
		ipanel_porting_dprintf("[iPanelTunerInit] demod_nim_type error(%d)\n", demod_nim_type);
		goto ERROR_EXIT;
	}
	
	/*����TS  Source��TS destion ���·��*/
    status = cnxt_ts_route_input(TS_PORT_DVB, HSDP_DEMUX0);
    if (TSROUTE_STATUS_OK != status)
	{
		ipanel_porting_dprintf("[iPanelTunerInit] cnxt_hsdp_route error(%d)\n", status);
		goto ERROR_EXIT;
	}	
	
	/*��Tuner���õ�handle*/
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
	/*������Ƶ�������TUNER����״̬����*/
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
		if (TRUE == m_tuner_task_running) /*�߳��������˳�*/
		{
			break;
		}
		
		ipanel_porting_task_sleep(10); /*��ʱ�ȴ��߳������˳�*/
	}

#ifdef IPANEL_TUNER_CALLBACK
	cnxt_demod_clear_callback(m_demod_handle);
#else
	if (IPANEL_NULL != m_tuner_task_id) /*��ɾ���������ͷ���Դ*/
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

