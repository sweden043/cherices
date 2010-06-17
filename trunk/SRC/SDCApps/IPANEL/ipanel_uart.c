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
*@brief  设置串口通信速率
*@param  fd     类型 int  打开串口的文件句柄
*@param  speed  类型 int  串口速度
*@return failed:IPANEL_ERR success:IPANEL_OK
*****************************************************/
static INT32_T ipanel_uart_set_speed(int fd, int speed)
{
	return IPANEL_OK;
}

/******************************************************
*@brief   设置串口数据位，停止位和效验位
*@param  fd     类型  int  打开的串口文件句柄
*@param  databits 类型  int 数据位   取值 为 7 或者8
*@param  stopbits 类型  int 停止位   取值为 1 或者2
*@param  parity  类型  int  效验类型 取值为N,E,O,,S
*@return failed:IPANEL_ERR success:IPANEL_OK
*******************************************************/
static INT32_T ipanel_uart_set_parity(int fd,int databits,int stopbits,int parity)
{
	return IPANEL_OK;
}

/**************************************************************************************************\
\* 函数原型                                                                                       *\
\*     UINT32_T ipanel_porting_uart_open(                                                         *\
\*             IPANEL_UART_BAUD_RATE_e  band_rate,                                                *\
\*             IPANEL_UART_MODE         mode                                                      *\
\*         )
\*  在 Linux 下串口文件是位于 /dev 下的
\*  串口一 为 /dev/ttyS0
\*  串口二 为 /dev/ttyS1 
\*                                                                                                *\
\* 函数功能                                                                                       *\
\*     打开指定的串口.                                                                            *\
\*                                                                                                *\
\* 函数输入                                                                                       *\
\*     band_rate : 串口的波特率;                                                                  *\
\*     mode : 串口工作模式.                                                                       *\
\*                                                                                                *\
\* 函数输出                                                                                       *\
\*     无.                                                                                        *\
\*                                                                                                *\
\* 函数返回                                                                                       *\
\*     成功返回串口设备的句柄, 否则返回IPANEL_NULL.                                               *\
\**************************************************************************************************/
UINT32_T ipanel_porting_uart_open(IPANEL_UART_BAUD_RATE_e band_rate, 
                                  IPANEL_UART_MODE mode)
{
    INT32_T ret = IPANEL_ERR;
    INT32_T fd = IPANEL_NULL;

    ipanel_porting_dprintf("[ipanel_porting_uart_open] band_rate=%d, mode=%d\n", band_rate, mode);

	//在TracePort_Init中可以设定其波特率等值--llser_open

    /* 设置波特率 */
    ret = ipanel_uart_set_speed(fd, uart_speed[band_rate-1]);
    if( IPANEL_ERR == ret)
    {
        ipanel_porting_dprintf("[ipanel_porting_uart_open] ipanel_uart_set_speed failed!\n");
    }

    m_uart_mode = mode ;

    return (UINT32_T)fd;
}

/**************************************************************************************************\
\* 函数原型                                                                                       *\
\*     INT32_T ipanel_porting_uart_close(UINT32_T handle)                                         *\
\*                                                                                                *\
\* 函数功能                                                                                       *\
\*     关闭一个打开的串口.                                                                        *\
\*                                                                                                *\
\* 函数输入                                                                                       *\
\*     handle : 串口句柄.                                                                         *\
\*                                                                                                *\
\* 函数输出                                                                                       *\
\*     无.                                                                                        *\
\*                                                                                                *\
\* 函数返回                                                                                       *\
\*     成功关闭串口返回IPANEL_OK, 否则返回IPANEL_ERR.                                             *\
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
\* 函数原型                                                                                       *\
\*     INT32_T ipanel_porting_uart_read(UINT32_T handle, BYTE_T *buf, INT32_T len)                *\
\*                                                                                                *\
\* 函数功能                                                                                       *\
\*     从串口读取数据.                                                                            *\
\*                                                                                                *\
\* 函数输入                                                                                       *\
\*     handle : 串口设备的句柄;                                                                   *\
\*     len : 指出buf的长度.                                                                       *\
\*                                                                                                *\
\* 函数输出                                                                                       *\
\*     buf : 保存读取到的数据.                                                                    *\
\*                                                                                                *\
\* 函数返回                                                                                       *\
\*     成功返回实际读取到的数据长度, 否则返回IPANEL_ERR.                                          *\
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
\* 函数原型                                                                                       *\
\*     INT32_T ipanel_porting_uart_write(UINT32_T handle, CONST BYTE_T *buf, INT32_T len)         *\
\*                                                                                                *\
\* 函数功能                                                                                       *\
\*     写数据到串口.                                                                              *\
\*                                                                                                *\
\* 函数输入                                                                                       *\
\*     handle : 串口设备的句柄;                                                                   *\
\*     buf : 指向欲写入的数据;                                                                    *\
\*     len : 欲写入数据的长度.                                                                    *\
\*                                                                                                *\
\* 函数输出                                                                                       *\
\*     无.                                                                                        *\
\*                                                                                                *\
\* 函数返回                                                                                       *\
\*     成功写入数据返回实际写入的数据长度, 否则返回IPANEL_ERR.                                    *\
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
\* 函数原型                                                                                       *\
\*     INT32_T ipanel_porting_uart_ioctl(UINT32_T handle,IPANEL_UART_IOCTL_e op,void *arg)        *\
\*                                                                                                *\
\* 函数功能                                                                                       *\
\*     进行串口的相关参数设定                                                                     *\
\*                                                                                                *\
\* 函数输入                                                                                       *\
\*     handle : 串口设备的句柄;                                                                   *\
\*     IPANEL_UART_IOCTL_e : 需要操作的选项                                                       *\
\*     arg : 可控制参数                                                                           *\
\*                                                                                                *\
\* 函数输出                                                                                       *\
\*     无.                                                                                        *\
\*                                                                                                *\
\* 函数返回                                                                                       *\
\*     成功关闭串口返回IPANEL_OK, 否则返回IPANEL_ERR.                                             *\
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

        case IPANEL_UART_SET_PARITY: // 格式"8,N,1"
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
功能：确定一个或多个套接字的状态，判断套接字上是否有数据，或者能否向一个套接字写入数据。
原型：INT32_T ipanel_porting_uart_select(INT32_T nfds, IPANEL_FD_SET_S*   readset,
                   IPANEL_FD_SET_S*  writeset, IPANEL_FD_SET_S* exceptset, INT32_T   timeout)
参数说明：
  输入参数：
    nfds：    需要检查的文件描述符个数，数值应该比readset、writeset、exceptset中最大数更大，而不是实际的文件描述符总数
    readset： 用来检查可读性的一组文件描述符
    writeset：用来检查可写性的一组文件描述符
    exceptset：用来检查意外状态的文件描述符，错误不属于意外状态
    timeout： 大于0表示等待多少毫秒；为IPANEL_NO_WAIT(0)时表示不等待立即返回，为IPANEL_WAIT_FOREVER(-1)表示永久等待。
  输出参数: 无
返    回：
  响应操作的对应操作文件描述符的总数，且readset、writeset、exceptset三组数据均在恰当位置被修改；
  IPANEL_OK:要查询的文件描叙符已准备好;
  IPANEL_ERR:失败，等待超时或出错。
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
\* 函数原型                                                                                       *\
\*     INT32_T ipanel_porting_uart_purge(UINT32_T handle)		                                  *\
\*                                                                                                *\
\* 函数功能                                                                                       *\
\*     进行串口的buffer数据清除                                                                   *\
\*                                                                                                *\
\* 函数输入                                                                                       *\
\*     handle : 串口设备的句柄;                                                                   *\
\*                                                                                                *\
\* 函数输出                                                                                       *\
\*     无.                                                                                        *\
\*                                                                                                *\
\* 函数返回                                                                                       *\
\*     成功关闭串口返回IPANEL_OK, 否则返回IPANEL_ERR.                                             *\
\*                                                                                                *\
\*  备注:                                                                                         *\
\*  tcflush函数刷清（抛弃）输入缓存（终端驱动程序已接收到，但用户程序                             *\
\*  尚未读）或输出缓存（用户程序已经写，但尚未发送）。                                            *\
\*  queue参数应当是下列三个常数之一：                                                             *\
\*  . TCIFLUSH 刷清输入队列。                                                                     *\
\*  . TCOFLUSH 刷清输出队列。                                                                     *\
\*  . TCIOFLUSH 刷清输入、输出队列。                                                              *\
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
    
    while (1) //循环读取数据
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

