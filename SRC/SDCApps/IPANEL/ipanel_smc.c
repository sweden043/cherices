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
#include "ipanel_dsm.h"
#include "ipanel_os.h"
#include "ipanel_smc.h"

static int m_smc_cardNo = -1;

// 0: ��ʾû�п�1:��ʾ�п���δ��ʼ��
static unsigned int m_SCInitStatus = 0;  

static CNXT_SMC_HANDLE smc_handle;
static IPANEL_SC_STATUS_NOTIFY smc_notify_func = NULL;

static void ipanel_sc_set_config(void);
static void ipanel_sc_get_config(void);

INT32_T ipanel_card_status = 0;
INT32_T ipanel_reset_flag = 0;

/////////////////////////////////////////////////////////////////////

// ���ڿ�״̬��֪ͨ���� 
static void ipanel_scm_callback(CNXT_SMC_HANDLE Handle,
               				  	void *pUserData,
               				   	CNXT_SMC_EVENT Event,
               				   	void *pData,
               				  	void *Tag)
{	
#ifdef USE_NO_TFCA_CARD
	switch (Event)
	{	
		case CNXT_SMC_EVENT_CARD_INSERTED:
			m_SCInitStatus = 1;
			ipanel_reset_flag = 1;
			ipanel_card_status = 1;
			
			if (smc_notify_func)
			{
				(smc_notify_func)(m_smc_cardNo, IPANEL_CARD_IN, NULL, IPANEL_SMARTCARD_STANDARD_ISO7816);
			}
			break;

		case CNXT_SMC_EVENT_CARD_REMOVED:
			ipanel_card_status = 0;
			ipanel_reset_flag = 1;
			m_SCInitStatus = 0;
			if (smc_notify_func)
			{
				(smc_notify_func)(m_smc_cardNo,IPANEL_CARD_OUT, NULL, IPANEL_SMARTCARD_STANDARD_ISO7816);
			}
			break;

		default:
    		break;
	}
#endif

	return ;
}

/******************************************************************************************************
����˵��:
	�����ܿ�ִ�и�λ������

����˵����
	 �������:	
		cardno�����ܿ����
		msgATR�����濨���ص�ATR����buffer��ַ��ʹ��ʱ�������ַ�Ƿ��ǿ�ָ�롣
	���������
		msgATR�� ����һ��ATR����Ҳ�����ǿյġ�

��    �أ�
	0����λ�ɹ���ʹ��T0Э��ͨѶ
	-1�޿�
	-2����ʶ��
	-3ͨѶ����
*******************************************************************************************************/
INT32_T ipanel_porting_smartcard_reset(INT32_T cardno, BYTE_T *msgATR)
{
#ifdef USE_NO_TFCA_CARD
	int i=0;
	u_int32 BufLength = 16;
	unsigned char ATRBuffer[32]={0};
	CNXT_SMC_STATUS smcstat;
	CNXT_SMC_STATE  pState;
    
	ipanel_porting_dprintf("[ipanel_porting_smartcard_reset] cardno=%d, msgATR=0x%x\n",cardno,msgATR);

	// get current state of smartcard
	cnxt_smc_get_state( smc_handle, &pState );
	
	/* state when card is not in slot */
	if (CNXT_SMC_EMPTY == pState) 
	{
		ipanel_porting_dprintf("[ipanel_porting_smartcard_reset] NO card in slot! Please insert card !\n");
		return -1;
	}
	
	smcstat = cnxt_smc_reset_card (smc_handle, FALSE, NULL);
	if (CNXT_SMC_OK != smcstat) 
	{
		ipanel_porting_dprintf("[ipanel_porting_smartcard_reset] reset smartcard failed ! ret=%d\n",smcstat);
    	return -3;
	}

	smcstat = cnxt_smc_get_atr (smc_handle, (void*)ATRBuffer, &BufLength );
	if (CNXT_SMC_OK != smcstat)
	{
		ipanel_porting_dprintf("[ipanel_porting_smartcard_reset] Failed ! ret=0x%x \n",smcstat);
    	return -2;
	}
	memcpy(msgATR,ATRBuffer,32);	
	
	ipanel_porting_dprintf(" len=%d, ATR : ", BufLength);
	for (i=0; i<16; i++)  
	{
		ipanel_porting_dprintf(" 0x%02x ",ATRBuffer[i]);
	}
	ipanel_porting_dprintf("\n\n");
#endif

	return 0;
}

/*****************************************************************************************
����˵����
	����Ҫ���������ܿ���ע�����ܿ��ص�������
	ʵ���߿����ڴ˺�������ɶ���Ӧ���ĳ�ʼ��������
	��ʹ�ܿ���Ӳ���ӿڣ��Կ���Active�����ȡ�
	����ʱ̽�⵽������п�ʱ������Կ�����ϵ縴λ������

����˵����
	���������
		cardno�����ܿ����
		sc_notify�����ܿ�״̬�仯֪ͨ�ص������ĵ�ַ
	�����������
	
��    �أ�
	0���ӿڴ򿪲����ɹ���������п�����λʧ��
	1���ӿڴ򿪲����ɹ���������޿�
	2���ӿڴ򿪲����ɹ���������п�����λ�ɹ���ʹ��T0Э��ͨѶ
	<0 ʧ�ܡ�
*****************************************************************************************/
INT32_T ipanel_porting_smartcard_open(INT32_T cardno, IPANEL_SC_STATUS_NOTIFY func)
{
	ipanel_porting_dprintf("[ipanel_porting_smartcard_open] cardno=%d, sc_atr_notify=0x%x\n",
							cardno,func);
#ifdef USE_NO_TFCA_CARD
	if (NULL != func)	
	{		
		smc_notify_func = func;
	}
	else
	{
		ipanel_porting_dprintf("[ipanel_porting_smartcard_open] SMC notify func is NULL !\n");
		return -1;
	}

	m_smc_cardNo = cardno;
	
	if ( 0 == m_SCInitStatus ) 
	{
		if (smc_notify_func)
		{
			(smc_notify_func)(cardno, IPANEL_CARD_OUT, NULL, IPANEL_SMARTCARD_STANDARD_ISO7816);
		}

		ipanel_porting_dprintf("[ipanel_porting_smartcard_open] NO Card in slot \n");
		return 1;
	}

	if (smc_notify_func) 
	{
		(smc_notify_func)(cardno, IPANEL_CARD_IN, NULL, IPANEL_SMARTCARD_STANDARD_ISO7816);
	}

	ipanel_porting_dprintf("[ipanel_porting_smartcard_open] Card in slot and reset OK!@ \n");
	
	return 2;
#endif	
    
}

/***************************************************************************
����˵����
	�ر����ڲ��������ܿ��ӿڡ�
	ʵ���߿����ڴ˺�������ɶ���Ӧ�����ͷŲ�����
	��ֹͣ����Ӳ���ӿڣ��Կ���DeActive�����ȡ�

����˵����
	���������cardno�����ܿ����
	�����������

��    �أ�
	>=0  �ɹ���<0 ʧ�ܡ�
****************************************************************************/
INT32_T ipanel_porting_smartcard_close(INT32_T cardno)
{
	ipanel_porting_dprintf("[ipanel_porting_smartcard_close] cardno=%d\n",cardno);
	
	smc_notify_func = NULL;
	m_smc_cardNo = -1;
	
	return IPANEL_OK;
}

/*****************************************************************************************************
����˵����
	�����ܿ�ͨѶ��
	����������Ҫ���ݲ�ͬ��Э�����;�����δ���reqdata�е����ݡ�
	T0Э�飺
	д���ݣ�
	reqdata = header + data to write
	�����ݣ�
	reqdata = header
	repdata = data from smartcard

����˵����
	���������
		cardno�����ܿ����
		reqdata�������ֶ�
		reqlen�������ֶγ���
		repdata�������ֶ�
		replen�������ֶγ���( exclude status word )
		StatusWord�����汾�β������ܿ����ص�״̬�ֵĵ�ַ,*StatusWordS �� SW1 << 8 | SW2
	�����������

��    �أ�
	>=0 �ɹ��� 
	-1�޿�
	-2����ʶ��
	-3ͨѶ����
******************************************************************************************************/
INT32_T ipanel_porting_smartcard_transfer_data(
        INT32_T         cardno,
        CONST BYTE_T   *reqdata,
        INT32_T         reqlen,
        BYTE_T         *repdata,
        INT32_T        *replen,
        UINT16_T       *statusword
    )
{
	u_int32  RetLen=256, DataLen=0;
	unsigned char RetData[256];
	unsigned char *pCmd=NULL, *pData=NULL;
	CNXT_SMC_STATUS smcstat;

    ipanel_porting_dprintf("[ipanel_porting_smartcard_transfer_data] no=%d, req=%p, len=%d, rep=%p,\
                            len=%p, sw=%p\n",cardno, reqdata, reqlen, repdata, replen, statusword);

#ifdef USE_NO_TFCA_CARD
	if ( reqlen < 5 ) 
	{
		ipanel_porting_dprintf("[ipanel_porting_smartcard_transfer_data] Error ! (*reqlen < 5) \n");
		*replen = 0;
		return -3;
	}
	
	// step-1: Send Command ( first 5 bytes in pbyCommand)
	if (reqlen > 5)  
	{
		RetLen = 1;
	}
	else 
	{
		RetLen = reqdata[4]+3;
	}
	
	pCmd = (unsigned char*)reqdata;		
	memset(RetData, 0x0, 256);	
	smcstat=cnxt_smc_read_write(smc_handle,
								FALSE,
								pCmd,
								5,
								RetData,
								&RetLen,
								NULL);
	if (CNXT_SMC_OK != smcstat) 
	{
		ipanel_porting_dprintf("[ipanel_porting_smartcard_transfer_data] cnxt_smc_read_write(1) failed ! ret=0x%x\n",smcstat);
		*replen = 0;
		return -3;
	}	

	if ( 0x60 == RetData[0] ) 
	{
		ipanel_porting_dprintf("[ipanel_porting_smartcard_transfer_data] NO need to send data ! RetData=0x%x\n",RetData[0]);
		*replen = 0;
		return 0;
	}	
	
	if ((6 == (RetData[0] >> 4)) || (9 == (RetData[0] >> 4)))
	{
		*replen = 0;
		*statusword = (unsigned short)(RetData[0]<<8)|RetData[1];
		ipanel_porting_dprintf("[ipanel_porting_smartcard_transfer_data] StatusWord=0x%04x\n",*statusword);
		return 0;
	}		
	
	if ( RetData[0] != reqdata[1] ) 
	{
		ipanel_porting_dprintf("[ipanel_porting_smartcard_transfer_data] PW Error !\n");
		*replen = 0;
		return -3;
	}

	// step-2: Send Data ( following first 5 bytes in pbyCommand)	
	if (5 == reqlen) 
	{		
		memcpy(repdata, &RetData[1], RetLen-1-2);
		*replen = RetLen-1-2;
	}
	else
	{	
		pData = (unsigned char*)reqdata+5;
		DataLen = reqdata[4];
		RetLen = 2;		
		smcstat=cnxt_smc_read_write(smc_handle,
									FALSE,
									pData,
									DataLen,
									repdata,
									&RetLen,
									NULL);
		if (CNXT_SMC_OK != smcstat) 
		{
			ipanel_porting_dprintf("[ipanel_porting_smartcard_transfer_data] cnxt_smc_read_write(2) failed ! ret=0x%x\n",smcstat);
			*replen = 0;
			return -3;
		}
		
		*replen = RetLen-2;		
	}	
	
	*statusword = (unsigned short)(repdata[*replen]<<8)|repdata[*replen+1];	

#endif	

	ipanel_porting_dprintf("[ipanel_porting_smartcard_transfer_data] StatusWord=0x%04x\n",*statusword);
	return 0;
}

static void ipanel_sc_set_config(void)
{
	CNXT_SMC_STATUS smcstat;

	smcstat = cnxt_smc_set_config(smc_handle, CNXT_SMC_CONFIG_PROTOCOL, 0);
	if (CNXT_SMC_OK != smcstat)
	{
		ipanel_porting_dprintf("[ipanel_sc_set_config] cnxt_smc_set_config (PROTOCOL) failed ! ret=0x%x\n",smcstat);		
	}

	smcstat = cnxt_smc_set_config(smc_handle, CNXT_SMC_CONFIG_DI, 1);
	if (CNXT_SMC_OK != smcstat)
	{
		ipanel_porting_dprintf("[ipanel_sc_set_config] cnxt_smc_set_config (DI) failed ! ret=0x%x\n",smcstat);		
	}

	smcstat = cnxt_smc_set_config(smc_handle, CNXT_SMC_CONFIG_TIMEOUT, 2000);
	if (CNXT_SMC_OK != smcstat)
	{
		ipanel_porting_dprintf("[ipanel_sc_set_config] cnxt_smc_set_config (TIMEOUT) failed ! ret=0x%x\n",smcstat);		
	}	
}

static void ipanel_sc_get_config(void)
{
	CNXT_SMC_STATUS ret;
	u_int32 Value = 0;	
	
	ret = cnxt_smc_get_config(smc_handle, CNXT_SMC_CONFIG_CONVENTION, (u_int32*)&Value);
	ipanel_porting_dprintf("[ipanel_sc_get_config] CONVENTION : Value=0x%x, ret=0x%x\n",Value, ret);

	ret = cnxt_smc_get_config(smc_handle, CNXT_SMC_CONFIG_PROTOCOL, (u_int32*)&Value);
	ipanel_porting_dprintf("[ipanel_sc_get_config] PROTOCOL : Value=0x%x, ret=0x%x\n",Value, ret);

	ret = cnxt_smc_get_config(smc_handle, CNXT_SMC_CONFIG_FI, (u_int32*)&Value);
	ipanel_porting_dprintf("[ipanel_sc_get_config] FI : Value=0x%x, ret=0x%x\n",Value, ret);

	ret = cnxt_smc_get_config(smc_handle, CNXT_SMC_CONFIG_DI, (u_int32*)&Value);
	ipanel_porting_dprintf("[ipanel_sc_get_config] DI : Value=0x%x, ret=0x%x\n",Value, ret);

	ret = cnxt_smc_get_config(smc_handle, CNXT_SMC_CONFIG_PI1, (u_int32*)&Value);
	ipanel_porting_dprintf("[ipanel_sc_get_config] PI1 : Value=0x%x, ret=0x%x\n",Value, ret);

	ret = cnxt_smc_get_config(smc_handle, CNXT_SMC_CONFIG_PI2, (u_int32*)&Value);
	ipanel_porting_dprintf("[ipanel_sc_get_config] PI2 : Value=0x%x, ret=0x%x\n",Value, ret);

	ret = cnxt_smc_get_config(smc_handle, CNXT_SMC_CONFIG_II, (u_int32*)&Value);
	ipanel_porting_dprintf("[ipanel_sc_get_config] II : Value=0x%x, ret=0x%x\n",Value, ret);

	ret = cnxt_smc_get_config(smc_handle, CNXT_SMC_CONFIG_N, (u_int32*)&Value);
	ipanel_porting_dprintf("[ipanel_sc_get_config] N : Value=0x%x, ret=0x%x\n",Value, ret);

	ret = cnxt_smc_get_config(smc_handle, CNXT_SMC_CONFIG_HISTORICAL, (u_int32*)&Value);
	ipanel_porting_dprintf("[ipanel_sc_get_config] HISTORICAL : Value=0x%x, ret=0x%x\n",Value, ret);

	ret = cnxt_smc_get_config(smc_handle, CNXT_SMC_CONFIG_HISTORICAL_LEN, (u_int32*)&Value);
	ipanel_porting_dprintf("[ipanel_sc_get_config] HISTORICAL_LEN : Value=0x%x, ret=0x%x\n",Value, ret);

	ret = cnxt_smc_get_config(smc_handle, CNXT_SMC_CONFIG_TIMEOUT, (u_int32*)&Value);
	ipanel_porting_dprintf("[ipanel_sc_get_config] TIMEOUT : Value=0x%x, ret=0x%x\n",Value, ret);

	ret = cnxt_smc_get_config(smc_handle, CNXT_SMC_CONFIG_RETRY, (u_int32*)&Value);
	ipanel_porting_dprintf("[ipanel_sc_get_config] RETRY : Value=0x%x, ret=0x%x\n",Value, ret);
}

INT32_T  ipanel_smartcard_init(void)
{	
	INT32_T ret = IPANEL_ERR; 
	CNXT_SMC_STATUS status;	
	CNXT_SMC_CAPS   smc_Caps;
	CNXT_SMC_STATE pState;

#ifdef USE_NO_TFCA_CARD
	smc_Caps.bExclusive = 0;
	smc_Caps.uUnitNumber = 0;	
	smc_Caps.uLength = sizeof(CNXT_SMC_CAPS);

	status = cnxt_smc_init(NULL);
	if (CNXT_SMC_OK != status) 
	{
		ipanel_porting_dprintf("[ipanel_smartcard_init] init smartcard failed! \n");
		return ret;
	}		
	
	status =  cnxt_smc_get_unit_caps(0, &smc_Caps);
	if (CNXT_SMC_OK != status)
	{
		ipanel_porting_dprintf("[ipanel_smartcard_init] get slot capability failed! \n");
		return ret;
	}
	
	status = cnxt_smc_open(&smc_handle, 
								&smc_Caps, 
								(CNXT_SMC_PFNNOTIFY)ipanel_scm_callback, 
								NULL);
	if (CNXT_SMC_OK != status)
	{
		ipanel_porting_dprintf("[ipanel_smartcard_init] smartcard open failed! \n");
		return ret;
	}	

	// get current state of smartcard
	cnxt_smc_get_state( smc_handle, &pState );
	
	/* state when card is not in slot */
	if (CNXT_SMC_EMPTY == pState) 
	{
		m_SCInitStatus = 0;
		ipanel_porting_dprintf("[ipanel_smartcard_init] NO card in slot! Please insert card !\n");
	}
	else if (CNXT_SMC_NOT_READY == pState)  /* state when card is inserted, but not reset or powered down */
	{
		m_SCInitStatus = 1;
		ipanel_porting_dprintf("[ipanel_smartcard_init] SmartCard IN Slot but not reset ok!\n");		
	}
	else
	{
		ipanel_porting_dprintf("[ipanel_smartcard_init] SmartCard IN slot and ready ok! \n");
	}

	// reset smartcard
	if (1 == m_SCInitStatus) 
	{
		CNXT_SMC_STATUS smcstat;
		// ʹ��������ʽ��λ���ܿ�
		smcstat = cnxt_smc_reset_card (smc_handle, FALSE, NULL);
		if (CNXT_SMC_OK != smcstat) 
		{
			ipanel_porting_dprintf("[ipanel_smartcard_init]  reset smartcard failed ! ret=%d \n",smcstat);			
		}

    	// get current state of smartcard
    	cnxt_smc_get_state( smc_handle, &pState );
        if( CNXT_SMC_READY == pState)
        {
            ipanel_porting_dprintf("[ipanel_smartcard_init] have CA card and reset success!\n");
        }
	}	
#endif

	return IPANEL_OK;
}

void ipanel_smartcard_exit(void)
{
	CNXT_SMC_STATUS status;

	ipanel_porting_dprintf("[ipanel_smartcard_exit] is called\n");
#ifdef USE_NO_TFCA_CARD
	status = cnxt_smc_close (smc_handle);
	if (CNXT_SMC_OK != status)
	{
		ipanel_porting_dprintf("[ipanel_smartcard_exit] reset smardcard failed! \n");
	}

	status = cnxt_smc_term();
	if( CNXT_SMC_OK != status)
	{
		ipanel_porting_dprintf("[ipanel_smartcard_exit] exit smartcard failed! \n");
	}
#endif	
}

