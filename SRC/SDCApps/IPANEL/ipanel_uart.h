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
#ifndef _IPANEL_MIDDLEWARE_PORTING_UART_API_FUNCTOTYPE_H_
#define _IPANEL_MIDDLEWARE_PORTING_UART_API_FUNCTOTYPE_H_

#include "ipanel_typedef.h"
#include "ipanel_socket.h"

#ifdef __cplusplus
extern "C" {
#endif

/**********************************************************************
    VxWorks����ϵͳ:
    Ϊ������ݽ��յ�ʵʱ�ԣ��ɲ����жϷ�ʽ.����VxWorks�ṩ��select����
    ���¼��������ƣ��������ڵ���������ʹ��һֱ�ȴ����ݣ���������������
    ʱ�������������Զ���Ӧ�����ϵͳ��ʵʱ�ԡ�

    ������ǿ����ն�֮��ģ�ֻ�Ǵ��ڴ������ݣ�������Ҫ����������
    ��ôʹ��ԭʼģʽ(Raw Mode)��ʽ��ͨѶ
    ʹ��LINEģʽ���Ļس����ɱ������ʹ��RAWģʽ������ߴ�ӡ������
**********************************************************************/

typedef enum
{
    IPANEL_UART_RAW	    = 1,  /* ÿ���մ��豸������ַ��Զ��߶�����Ч��,��������ģʽ  */
    IPANEL_UART_LINE    = 2   /* ���������ַ����洢��ֱ��NEWLINE�ַ����룬�ַ���ģʽ */
} IPANEL_UART_MODE;

typedef enum
{
    IPANEL_UART_BR_300      = 1,
    IPANEL_UART_BR_600      = 2,
    IPANEL_UART_BR_1200     = 3,
    IPANEL_UART_BR_2400     = 4,
    IPANEL_UART_BR_4800     = 5,
    IPANEL_UART_BR_9600     = 6,
    IPANEL_UART_BR_19200    = 7,
    IPANEL_UART_BR_38400    = 8,
    IPANEL_UART_BR_57600    = 9,
    IPANEL_UART_BR_115200   = 10
} IPANEL_UART_BAUD_RATE_e;

typedef enum
{
    IPANEL_UART_SET_BAUDRATE =1, /* ���ò����� */
    IPANEL_UART_SET_PARITY = 2,  /* Ч��λ��ֹͣλ������ */
    IPANEL_UART_SET_MODE   = 3   /* RAW �� LINE ���ڹ���ģʽ */
}IPANEL_UART_IOCTL_e;

UINT32_T ipanel_porting_uart_open(IPANEL_UART_BAUD_RATE_e band_rate, 
                                  IPANEL_UART_MODE mode);

INT32_T ipanel_porting_uart_close(UINT32_T handle);

INT32_T ipanel_porting_uart_read(UINT32_T handle, BYTE_T *buf, INT32_T len);

INT32_T ipanel_porting_uart_write(UINT32_T handle, CONST BYTE_T *buf, INT32_T len);

INT32_T ipanel_porting_uart_ioctl(UINT32_T handle,IPANEL_UART_IOCTL_e op,void *arg);

INT32_T ipanel_porting_uart_purge(UINT32_T handle);

INT32_T ipanel_porting_uart_select(INT32_T nfds,
								   IPANEL_FD_SET_S* readset, 
								   IPANEL_FD_SET_S* writeset,
								   IPANEL_FD_SET_S* exceptset,
								   INT32_T timeout );

int ipanel_uart_test();

#ifdef __cplusplus
}
#endif

#endif  // _IPANEL_MIDDLEWARE_PORTING_UART_API_FUNCTOTYPE_H_

