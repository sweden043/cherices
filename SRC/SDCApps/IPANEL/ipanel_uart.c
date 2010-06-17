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
#include "ipanel_base.h"
#include "ipanel_os.h"
#include "ipanel_uart.h"

#define UART_DBGOUT

extern int SerialGetData( void* pData,unsigned int len,int timeout );
extern int SerialPutData(void* pData,unsigned int len);

static IPANEL_UART_MODE m_uart_mode = IPANEL_UART_RAW;

static int uart_speed[] = {300,600,1200,2400,4800,9600,19200,38400,57600,115200};

/****************************************************
*@brief  ���ô���ͨ������
*@param  fd     ���� int  �򿪴��ڵ��ļ����
*@param  speed  ���� int  �����ٶ�
*@return failed:IPANEL_ERR success:IPANEL_OK
*****************************************************/
static INT32_T ipanel_uart_set_speed(int fd, int speed)
{
	return IPANEL_OK;
}

/******************************************************
*@brief   ���ô�������λ��ֹͣλ��Ч��λ
*@param  fd     ����  int  �򿪵Ĵ����ļ����
*@param  databits ����  int ����λ   ȡֵ Ϊ 7 ����8
*@param  stopbits ����  int ֹͣλ   ȡֵΪ 1 ����2
*@param  parity  ����  int  Ч������ ȡֵΪN,E,O,,S
*@return failed:IPANEL_ERR success:IPANEL_OK
*******************************************************/
static INT32_T ipanel_uart_set_parity(int fd,int databits,int stopbits,int parity)
{
	return IPANEL_OK;
}

/**************************************************************************************************\
\* ����ԭ��                                                                                       *\
\*     UINT32_T ipanel_porting_uart_open(                                                         *\
\*             IPANEL_UART_BAUD_RATE_e  band_rate,                                                *\
\*             IPANEL_UART_MODE         mode                                                      *\
\*         )
\*  �� Linux �´����ļ���λ�� /dev �µ�
\*  ����һ Ϊ /dev/ttyS0
\*  ���ڶ� Ϊ /dev/ttyS1 
\*                                                                                                *\
\* ��������                                                                                       *\
\*     ��ָ���Ĵ���.                                                                            *\
\*                                                                                                *\
\* ��������                                                                                       *\
\*     band_rate : ���ڵĲ�����;                                                                  *\
\*     mode : ���ڹ���ģʽ.                                                                       *\
\*                                                                                                *\
\* �������                                                                                       *\
\*     ��.                                                                                        *\
\*                                                                                                *\
\* ��������                                                                                       *\
\*     �ɹ����ش����豸�ľ��, ���򷵻�IPANEL_NULL.                                               *\
\**************************************************************************************************/
UINT32_T ipanel_porting_uart_open(IPANEL_UART_BAUD_RATE_e band_rate, 
                                  IPANEL_UART_MODE mode)
{
    INT32_T ret = IPANEL_ERR;
    INT32_T fd = IPANEL_NULL;

    ipanel_porting_dprintf("[ipanel_porting_uart_open] band_rate=%d, mode=%d\n", band_rate, mode);

	//��TracePort_Init�п����趨�䲨���ʵ�ֵ--llser_open

    /* ���ò����� */
    ret = ipanel_uart_set_speed(fd, uart_speed[band_rate-1]);
    if( IPANEL_ERR == ret)
    {
        ipanel_porting_dprintf("[ipanel_porting_uart_open] ipanel_uart_set_speed failed!\n");
    }

    m_uart_mode = mode ;

    return (UINT32_T)fd;
}

/**************************************************************************************************\
\* ����ԭ��                                                                                       *\
\*     INT32_T ipanel_porting_uart_close(UINT32_T handle)                                         *\
\*                                                                                                *\
\* ��������                                                                                       *\
\*     �ر�һ���򿪵Ĵ���.                                                                        *\
\*                                                                                                *\
\* ��������                                                                                       *\
\*     handle : ���ھ��.                                                                         *\
\*                                                                                                *\
\* �������                                                                                       *\
\*     ��.                                                                                        *\
\*                                                                                                *\
\* ��������                                                                                       *\
\*     �ɹ��رմ��ڷ���IPANEL_OK, ���򷵻�IPANEL_ERR.                                             *\
\**************************************************************************************************/
INT32_T ipanel_porting_uart_close(UINT32_T handle)
{
    INT32_T ret = IPANEL_ERR;

    ipanel_porting_dprintf("[ipanel_porting_uart_close] handle=0x%x\n", handle);

    if (handle)
    {
		
    }

    return IPANEL_OK;
}

/**************************************************************************************************\
\* ����ԭ��                                                                                       *\
\*     INT32_T ipanel_porting_uart_read(UINT32_T handle, BYTE_T *buf, INT32_T len)                *\
\*                                                                                                *\
\* ��������                                                                                       *\
\*     �Ӵ��ڶ�ȡ����.                                                                            *\
\*                                                                                                *\
\* ��������                                                                                       *\
\*     handle : �����豸�ľ��;                                                                   *\
\*     len : ָ��buf�ĳ���.                                                                       *\
\*                                                                                                *\
\* �������                                                                                       *\
\*     buf : �����ȡ��������.                                                                    *\
\*                                                                                                *\
\* ��������                                                                                       *\
\*     �ɹ�����ʵ�ʶ�ȡ�������ݳ���, ���򷵻�IPANEL_ERR.                                          *\
\**************************************************************************************************/
INT32_T ipanel_porting_uart_read(UINT32_T handle, BYTE_T *buf, INT32_T len)
{
    int nByte ;

    if (handle && buf && (len > 0))
    {
        nByte = SerialGetData((void*)buf, len,0);
        if (nByte > 0)
        {
            #ifdef UART_DBGOUT
            ipanel_hex_printout("Read", buf, nByte, 16);
            #endif
        }
        else
        {
            return IPANEL_ERR;
        }
    }

    return nByte;
}

/**************************************************************************************************\
\* ����ԭ��                                                                                       *\
\*     INT32_T ipanel_porting_uart_write(UINT32_T handle, CONST BYTE_T *buf, INT32_T len)         *\
\*                                                                                                *\
\* ��������                                                                                       *\
\*     д���ݵ�����.                                                                              *\
\*                                                                                                *\
\* ��������                                                                                       *\
\*     handle : �����豸�ľ��;                                                                   *\
\*     buf : ָ����д�������;                                                                    *\
\*     len : ��д�����ݵĳ���.                                                                    *\
\*                                                                                                *\
\* �������                                                                                       *\
\*     ��.                                                                                        *\
\*                                                                                                *\
\* ��������                                                                                       *\
\*     �ɹ�д�����ݷ���ʵ��д������ݳ���, ���򷵻�IPANEL_ERR.                                    *\
\**************************************************************************************************/
INT32_T ipanel_porting_uart_write(UINT32_T handle, CONST BYTE_T *buf, INT32_T len)
{
    int nByte ;

    if (handle && buf && (len > 0))
    {
        #ifdef UART_DBGOUT
        ipanel_hex_printout("Write", buf, len, 16);
        #endif
        
        nByte = SerialPutData((void*)buf, len);
		if(nByte < 0)
			return IPANEL_ERR;
    }

    return nByte;
}

/**************************************************************************************************\
\* ����ԭ��                                                                                       *\
\*     INT32_T ipanel_porting_uart_ioctl(UINT32_T handle,IPANEL_UART_IOCTL_e op,void *arg)        *\
\*                                                                                                *\
\* ��������                                                                                       *\
\*     ���д��ڵ���ز����趨                                                                     *\
\*                                                                                                *\
\* ��������                                                                                       *\
\*     handle : �����豸�ľ��;                                                                   *\
\*     IPANEL_UART_IOCTL_e : ��Ҫ������ѡ��                                                       *\
\*     arg : �ɿ��Ʋ���                                                                           *\
\*                                                                                                *\
\* �������                                                                                       *\
\*     ��.                                                                                        *\
\*                                                                                                *\
\* ��������                                                                                       *\
\*     �ɹ��رմ��ڷ���IPANEL_OK, ���򷵻�IPANEL_ERR.                                             *\
\**************************************************************************************************/
INT32_T ipanel_porting_uart_ioctl(UINT32_T handle,IPANEL_UART_IOCTL_e op,void *arg)
{
    INT32_T ret = IPANEL_OK;
    INT32_T oparg = (INT32_T)arg ;

    ipanel_porting_dprintf("[ipanel_porting_uart_ioctl] handle=0x%x,op=%d,arg=0x%x.\n",
                           handle,op,arg);

    switch(op)
    {
        case IPANEL_UART_SET_BAUDRATE:
            ret = ipanel_uart_set_speed(handle, uart_speed[oparg-1]);
            break;

        case IPANEL_UART_SET_PARITY: // ��ʽ"8,N,1"
            ret = ipanel_uart_set_parity(handle,8,1,'N');
            break;

        case IPANEL_UART_SET_MODE:
            m_uart_mode = (IPANEL_UART_MODE)oparg;
            break;

        default:
            ipanel_porting_dprintf("[ipanel_porting_uart_ioctl] error parameter!\n");
            ret = IPANEL_ERR;
            break;
    }
    
    return ret;    
}

/********************************************************************************************************
���ܣ�ȷ��һ�������׽��ֵ�״̬���ж��׽������Ƿ������ݣ������ܷ���һ���׽���д�����ݡ�
ԭ�ͣ�INT32_T ipanel_porting_uart_select(INT32_T nfds, IPANEL_FD_SET_S*   readset,
                   IPANEL_FD_SET_S*  writeset, IPANEL_FD_SET_S* exceptset, INT32_T   timeout)
����˵����
  ���������
    nfds��    ��Ҫ�����ļ���������������ֵӦ�ñ�readset��writeset��exceptset����������󣬶�����ʵ�ʵ��ļ�����������
    readset�� �������ɶ��Ե�һ���ļ�������
    writeset����������д�Ե�һ���ļ�������
    exceptset�������������״̬���ļ���������������������״̬
    timeout�� ����0��ʾ�ȴ����ٺ��룻ΪIPANEL_NO_WAIT(0)ʱ��ʾ���ȴ��������أ�ΪIPANEL_WAIT_FOREVER(-1)��ʾ���õȴ���
  �������: ��
��    �أ�
  ��Ӧ�����Ķ�Ӧ�����ļ�����������������readset��writeset��exceptset�������ݾ���ǡ��λ�ñ��޸ģ�
  IPANEL_OK:Ҫ��ѯ���ļ��������׼����;
  IPANEL_ERR:ʧ�ܣ��ȴ���ʱ�����
********************************************************************************************************/
INT32_T ipanel_porting_uart_select(INT32_T nfds,
								   IPANEL_FD_SET_S* readset, 
								   IPANEL_FD_SET_S* writeset,
								   IPANEL_FD_SET_S* exceptset,
								   INT32_T timeout )
{
	return IPANEL_OK;
}

/**************************************************************************************************\
\* ����ԭ��                                                                                       *\
\*     INT32_T ipanel_porting_uart_purge(UINT32_T handle)		                                  *\
\*                                                                                                *\
\* ��������                                                                                       *\
\*     ���д��ڵ�buffer�������                                                                   *\
\*                                                                                                *\
\* ��������                                                                                       *\
\*     handle : �����豸�ľ��;                                                                   *\
\*                                                                                                *\
\* �������                                                                                       *\
\*     ��.                                                                                        *\
\*                                                                                                *\
\* ��������                                                                                       *\
\*     �ɹ��رմ��ڷ���IPANEL_OK, ���򷵻�IPANEL_ERR.                                             *\
\*                                                                                                *\
\*  ��ע:                                                                                         *\
\*  tcflush����ˢ�壨���������뻺�棨�ն����������ѽ��յ������û�����                             *\
\*  ��δ������������棨�û������Ѿ�д������δ���ͣ���                                            *\
\*  queue����Ӧ����������������֮һ��                                                             *\
\*  . TCIFLUSH ˢ��������С�                                                                     *\
\*  . TCOFLUSH ˢ��������С�                                                                     *\
\*  . TCIOFLUSH ˢ�����롢������С�                                                              *\
\**************************************************************************************************/
INT32_T ipanel_porting_uart_purge(UINT32_T handle)
{
	return IPANEL_OK;
}

int ipanel_uart_test()
{
	UINT32_T handle;
	int nread;
    INT32_T ret ;
	char buff[512];
	IPANEL_FD_SET_S fds;
	int timeout = 5000 ;
	
    handle = ipanel_porting_uart_open(IPANEL_UART_BR_115200,IPANEL_UART_UPLOAD);
    if( IPANEL_NULL == handle )
    {
        ipanel_porting_dprintf("[ipanel_uart_test] open uart failed!");
        return IPANEL_ERR;
    }
    
    ret = ipanel_porting_uart_ioctl((INT32_T)handle,IPANEL_UART_SET_PARITY,(VOID*)NULL);
    if( IPANEL_ERR == ret)
    {
        ipanel_porting_dprintf("[ipanel_uart_test] set uart failed!");
        return IPANEL_ERR;
    }
    
    while (1) //ѭ����ȡ����
    {   
		buff[0] = 0x43 ;
		ipanel_porting_uart_write(handle,buff,1);
		
		IPANEL_FD_ZERO(&fds);
	    IPANEL_FD_SET(handle, &fds);
		ret = ipanel_porting_uart_select(handle,&fds,NULL,NULL,timeout);	    	
		if(IPANEL_OK == ret)
		{
			//if(IPANEL_FD_ISSET(handle,&fds))
			{
		        ipanel_porting_uart_read(handle,buff,1);
			}
		}
    }

    ipanel_porting_uart_close(handle);  

    return IPANEL_OK;
}

