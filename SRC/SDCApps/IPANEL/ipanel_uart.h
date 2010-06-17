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
    VxWorks操作系统:
    为提高数据接收的实时性，可采用中断方式.利用VxWorks提供的select函数
    的事件触发机制，将读串口的任务阻塞使其一直等待数据，当有数据来到的
    时候该任务会立刻自动响应，提高系统的实时性。

    如果不是开发终端之类的，只是串口传输数据，而不需要串口来处理，
    那么使用原始模式(Raw Mode)方式来通讯
    使用LINE模式最后的回车符可被输出，使用RAW模式边输入边打印出内容
**********************************************************************/

typedef enum
{
    IPANEL_UART_RAW	    = 1,  /* 每个刚从设备输入的字符对读者都是有效的,二进制流模式  */
    IPANEL_UART_LINE    = 2   /* 所有输入字符被存储，直到NEWLINE字符输入，字符流模式 */
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
    IPANEL_UART_SET_BAUDRATE =1, /* 设置波特率 */
    IPANEL_UART_SET_PARITY = 2,  /* 效验位和停止位的设置 */
    IPANEL_UART_SET_MODE   = 3   /* RAW 或 LINE 串口工作模式 */
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

