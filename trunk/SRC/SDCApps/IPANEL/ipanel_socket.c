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
#include "ipanel_os.h"
#include "ipanel_base.h"
#include "ipanel_device.h"
#include "ipanel_network.h"
#include "ipanel_socket.h"
#include "ipanel_porting_event.h"

//--------------------------------------------------------------------------------------------------
// Types and defines
//--------------------------------------------------------------------------------------------------
//
#define IPANEL_MAX_SOCKETS              16

typedef struct _SOCKPARAM_tag
{
	INT32_T sockid ; 
	INT32_T socktype ; 
	INT32_T sockport ; 
	IPANEL_IPADDR   sockip ;	
}SOCKPARAM;

typedef struct _SOCKMANAGER_tag
{
	INT32_T n_sockidle ; 
	SOCKPARAM sockparam[IPANEL_MAX_SOCKETS] ;	
}SOCKMANAGER ;

static SOCKMANAGER *ipanel_mgr = NULL ; 
static SOCKMANAGER  ipanelMgr ;

//--------------------------------------------------------------------------------------------------
// Internal functions
//--------------------------------------------------------------------------------------------------
//

static SOCKMANAGER  *ipanel_register_new()
{
	SOCKMANAGER *me ;  
	int i;
	
	me = (SOCKMANAGER*) &ipanelMgr;
	memset(me,0,sizeof(SOCKMANAGER));
	
	for (i=0;i<IPANEL_MAX_SOCKETS;i++) 
	{
		me->sockparam[i].sockid = -1;
		me->sockparam[i].socktype = -1;
		me->sockparam[i].sockport = -1; 
	}
	
	me->n_sockidle = IPANEL_MAX_SOCKETS; 
 
	return me;
}

static void ipanel_register_delete(SOCKMANAGER *me)
{
	int i;
	for (i=0;i<IPANEL_MAX_SOCKETS;i++) 
	{
    	if (me->sockparam[i].sockid != -1) 
		{
			NU_Close_Socket(me->sockparam[i].sockid);
    	}
	}
}

static int ipanel_register_register(SOCKMANAGER *me, INT32_T s)
{
	int i, indx=-1;

	for (i=0;i<IPANEL_MAX_SOCKETS;i++) 
	{
		if (me->sockparam[i].sockid == -1) 
		{
			me->sockparam[i].sockid = s;
			me->n_sockidle --;
			indx = i ;
			break;
		}
	}

	return indx;	
}

static int ipanel_register_unregister(SOCKMANAGER *me, INT32_T s)
{
	int i, indx=-1;
	
	for (i=0;i<IPANEL_MAX_SOCKETS;i++) 
	{
		if (me->sockparam[i].sockid == s)
		{
			me->sockparam[i].sockid = -1;
			me->n_sockidle ++;
			indx = i;
			break;
		}
	}
	
	return indx;	
}

static int ipanel_register_set_param(SOCKMANAGER *me,SOCKPARAM param)
{
	int i, indx=-1;
	
	for (i=0;i<IPANEL_MAX_SOCKETS;i++) 
	{
		if (me->sockparam[i].sockid == param.sockid)
		{
			indx = i;
			break;
		}
	}

	if(i == IPANEL_MAX_SOCKETS)
	{
		ipanel_porting_dprintf("[ipanel_register_set_param] set param failed!\n");
		return -1;
	}

	me->sockparam[indx].sockport = param.sockport ;
	me->sockparam[indx].socktype = param.socktype ;
	me->sockparam[indx].sockip   = param.sockip ; 

	return 0;	
}

static int ipanel_register_get_param(SOCKMANAGER *me, SOCKPARAM *param)
{
	int i, indx=-1;
	
	for (i=0;i<IPANEL_MAX_SOCKETS;i++) 
	{
		if (me->sockparam[i].sockid == param->sockid)
		{
			indx = i;
			break;
		}
	}

	if(i == IPANEL_MAX_SOCKETS)
	{
		ipanel_porting_dprintf("[ipanel_register_get_param] get param failed!\n");
		return -1;
	}

	param->sockport = me->sockparam[indx].sockport;
	param->socktype = me->sockparam[indx].socktype;
	param->sockip   =  me->sockparam[indx].sockip    ; 
	
	return 0;
}

INT32_T ipanel_get_socktype(SOCKMANAGER *me, INT32_T s)
{
	int i, indx=-1;
	
	for (i=0;i<IPANEL_MAX_SOCKETS;i++) 
	{
		if (me->sockparam[i].sockid == s)
		{
			indx = i;
			break;
		}
	}

	if(i == IPANEL_MAX_SOCKETS)
	{
		ipanel_porting_dprintf("[ipanel_get_socktype] get sock type failed!\n");
		return -1;
	}	
    ipanel_porting_dprintf("[ipanel_get_socktype]  index = %d   type  = %d \n", indx, me->sockparam[indx].socktype);

	return me->sockparam[indx].socktype ; 
}

//--------------------------------------------------------------------------------------------------
// Exported functions
//--------------------------------------------------------------------------------------------------
//

/***************************************************************************************************
功能说明：
	建立一个套接字。
	
参数说明：

	输入参数：
		family：指定要创建的套接字协议族。IPANEL_AF_INET：表示IPv4协议，
		        IPANEL_AV_INET6表示IPv6协议，其他值保留
		type：指定要创建的套接字的类型。IPANEL_SOCK_STREAM：表示SOCK_STREAM，
		      IPANEL_SOCK_DGRAM：表示SOCK_DGRAM，其他值保留
		protocol：指定哪种协议。通常设置为IPANEL_IPPROTO_IP，表示与type匹配的默认协议，其他值保留
	输出参数：无
	
返    回：
若套接字建立成功，则返回非负描述符，否则返回-1。
>＝0: 套接字建立成功，则返回非负描述符，
	  描叙符的值应该不超过IPANEL_NFDBITS的定义范围；
IPANEL_ERR: 创建套接字失败。

***************************************************************************************************/
INT32_T ipanel_porting_socket_open ( INT32_T domain, INT32_T type, INT32_T protocol )
{
	INT32_T ret =  IPANEL_ERR;
	STATUS socketd = -1; 
	INT32_T sockdomain, socktype ;
	SOCKPARAM tmpSockParam = {0} ;
	
    ipanel_porting_dprintf("[ipanel_porting_socket_open] domain=%d, type=%d, protocol=%d\n",
   			domain, type, protocol );

	if (  (IPANEL_AF_INET == domain) ||
		(IPANEL_AF_INET6 == domain))
	{
	    sockdomain = NU_FAMILY_IP;
	}
	else
	{
		ipanel_porting_dprintf("[ipanel_porting_socket_open]ERROR: domain para error ! \n");
	    return ret;
	}

	if (IPANEL_SOCK_STREAM == type)
	{
    	socktype = NU_TYPE_STREAM;
	}
	else if (IPANEL_SOCK_DGRAM == type)
	{
    	socktype = NU_TYPE_DGRAM;
	}
	else
	{
		ipanel_porting_dprintf("[ipanel_porting_socket_open]ERROR:  stream(TCP&UDP) para error ! \n");
	    return ret;
	}

	if (IPANEL_IPPROTO_IP == protocol)
	{
		socketd = NU_Socket(sockdomain, socktype, protocol);	
		if ( socketd < 0) 
		{
			ipanel_porting_dprintf("[ipanel_porting_socket_open] ERROR: create socket error ! \n");
			return ret ; 
		}
	}

	ipanel_register_register(ipanel_mgr, socketd);
	tmpSockParam.sockid = socketd ;
	tmpSockParam.socktype  = type ;
	ipanel_register_set_param(ipanel_mgr, tmpSockParam);

	ipanel_porting_dprintf("[ipanel_porting_socket_open] open socket id = %d\n", socketd);

    return socketd;
}


/***************************************************************************************************
功能说明：
	将指定的端口号与套接字绑定在一起。
	
参数说明：
	输入参数：
		sockfd：套接字描述符
		ipaddr：包含待绑定IP地址和IP版本描述结构体的指针
		port：端口号
	输出参数：无
	
返    回：
	IPANEL_OK:成功;
	IPANEL_ERR:失败。
***************************************************************************************************/
INT32_T ipanel_porting_socket_bind ( INT32_T sockfd, IPANEL_IPADDR *ipaddr,INT32_T port )
{
    INT32_T ret = IPANEL_ERR;
    STATUS bindstat ; 
    struct addr_struct serv_addr;

    if ( ( sockfd >= 0 ) && ( NULL != ipaddr ) )
    {
    	if (IPANEL_IP_VERSION_4 == ipaddr->version)
    	{
            unsigned int ip = ipaddr->addr.ipv4;	

            ipanel_porting_dprintf("[ipanel_porting_socket_bind] ip = 0x%x ,port = %d\n" , ip , port );

            if( ip != IP_ADDR_ANY)  // 只有本地地址才能绑定成功
            {
            	ip = IP_ADDR_ANY ; 
            }

            serv_addr.family = NU_FAMILY_IP;
            serv_addr.port   = port ;  

            serv_addr.id.is_ip_addrs[0]=(UINT8)((ip>>24)&0xFF);
            serv_addr.id.is_ip_addrs[1]=(UINT8)((ip>>16)&0xFF);
            serv_addr.id.is_ip_addrs[2]=(UINT8)((ip>>8)&0xFF);
            serv_addr.id.is_ip_addrs[3]=(UINT8)(ip&0xFF);

            bindstat = NU_Bind( sockfd, ( struct addr_struct* ) ( &serv_addr ), sizeof ( serv_addr ) );
            if( bindstat < 0)
            {
            	ipanel_porting_dprintf("[ipanel_porting_socket_bind] bind failed! \n");
            	return ret ;
            }

            ipanel_porting_dprintf("[ipanel_porting_socket_bind] bind success. IP addr = 0x%x \n", ip);

            ret = IPANEL_OK ;
    	}
        else if(IPANEL_IP_VERSION_6 == ipaddr->version)
        {
        	/* IPv6协议 */
        	ipanel_porting_dprintf("[ipanel_porting_socket_bind] ERROR: NOT suppurt IPV6 !!! \n");
        	return ret ; 
        }
        else
        {
        	ipanel_porting_dprintf("[ipanel_porting_socket_bind] ERROR: please set  IP_VERSION_4 !\n");
        	return ret ;
        }
    }

    return ret;
}

/***************************************************************************************************
功能说明：
	将一个套接字转换成监听套接字。

参数说明：
	输入参数：
		sockfd：套接字描述符
		backlog：接收队列的最大长度
	输出参数：无
	
返    回：
	IPANEL_OK:成功;
	IPANEL_ERR:失败。
***************************************************************************************************/
INT32_T ipanel_porting_socket_listen ( INT32_T sockfd, INT32_T backlog )
{
    INT32_T ret = IPANEL_OK;
    STATUS lisstat ; 

    ipanel_porting_dprintf("[ipanel_porting_socket_listen] sockfd=%d, backlog=%d\n",sockfd, backlog);

    if ( sockfd >= 0 )
    {
        lisstat = NU_Listen(sockfd, backlog);
        if( lisstat < 0) 
        {
            ipanel_porting_dprintf("[ipanel_porting_socket_listen] listen socket failed! \n");
            ret = IPANEL_ERR ; 
        }
    }

    return ret;
}

/***************************************************************************************************
功能说明：
	响应连接请求，建立连接并产生一个新的socket描述符来描述该连接

参数说明：
	输入参数：
		sockfd：监听套接字描述符
		ipaddr：包含IP版本描述的结构体指针
	输出参数：
		ipaddr：保存连接对方IP地址的结构体指针
		port：保存连接对方的端口号
		
返    回：
	>＝0:执行成功则返回非负描述符。
	IPANEL_ERR:失败。
***************************************************************************************************/
INT32_T ipanel_porting_socket_accept ( INT32_T sockfd,IPANEL_IPADDR *ipaddr,INT32_T *port )
{
    INT32_T ret = IPANEL_ERR;
    STATUS newsock = -1; 
    struct addr_struct serv_addr = {0};		
    INT16 len = sizeof (serv_addr);
    SOCKPARAM tmpSockParam = {0} ;

    ipanel_porting_dprintf("[ipanel_porting_socket_accept] sockfd=%d, ip=0x%p, port=%p \n",
    sockfd, ipaddr, port);

    if ( sockfd >= 0 )
    {
        if (IPANEL_IP_VERSION_4 == ipaddr->version)
        {
            newsock = NU_Accept(sockfd, (struct addr_struct*)&serv_addr, &len);
            if ( newsock >= 0 )
            {
                if ( ipaddr != NULL )
                {
                    ipaddr->addr.ipv4 = CHARTOINT(serv_addr.id.is_ip_addrs);
                }

                if ( port  != NULL)
                {
                    *port = serv_addr.port ;
                }

                ipanel_porting_dprintf("[ipanel_porting_socket_accept] ip = 0x%x , port = %d \n", 
                                        ipaddr->addr.ipv4 , *port); 

                ipanel_register_register(ipanel_mgr, newsock);
                tmpSockParam.sockid = newsock ;
                tmpSockParam.socktype  = IPANEL_SOCK_STREAM ;
                tmpSockParam.sockip = *ipaddr;
                tmpSockParam.sockport = *port;

                ipanel_register_set_param(ipanel_mgr, tmpSockParam);

            }
            else
            {
                ipanel_porting_dprintf("[ipanel_porting_socket_accept] accept socket failed! \n");
                return ret ; 
            }   
        }
        else if (IPANEL_IP_VERSION_6 == ipaddr->version)
        {
            /* IPv6协议 */
            ipanel_porting_dprintf("[ipanel_porting_socket_accept]ERROR: NOT suppurt IPV6 !!! \n");
            return ret ;
        }
        else
        {
            ipanel_porting_dprintf("[ipanel_porting_socket_accept] ERROR: please set  IP_VERSION_4 !\n");
            return ret ;
        }		  
    }

    return newsock;
}

/***************************************************************************************************
功能说明：
	将套接字与指定的套接字连接起来
	
参数说明：
	输入参数：
		sockfd：套接字描述符
		ipaddr：包含待连接IP地址及IP版本描述的结构体指针
		port：待连接的端口号
	输出参数：无
	
返    回：
	IPANEL_OK:成功;
	IPANEL_ERR:失败。
***************************************************************************************************/
INT32_T ipanel_porting_socket_connect (INT32_T sockfd,IPANEL_IPADDR *ipaddr,INT32_T port )
{
    INT32_T ret = IPANEL_ERR;
    STATUS connstat ; 
    struct addr_struct serv_addr;

    if ( ( sockfd >= 0 ) && ( NULL != ipaddr ) )
    {
        if (IPANEL_IP_VERSION_4 == ipaddr->version)
        {
            unsigned int ip = ipaddr->addr.ipv4;	
            int len = sizeof(serv_addr);	
            INT32_T socktype ; 

            ipanel_porting_dprintf("[ipanel_porting_socket_connect] ip = 0x%x , port=%d \n", ip, port);

            socktype = ipanel_get_socktype(ipanel_mgr,sockfd);

            if( IPANEL_SOCK_DGRAM ==  socktype) 
            {
                SOCKPARAM tmpSockParam = {0} ;

                tmpSockParam.sockid = sockfd ;
                tmpSockParam.sockport = port ; 
                tmpSockParam.sockip = *ipaddr ;
                tmpSockParam.socktype = socktype ;
                ipanel_register_set_param(ipanel_mgr, tmpSockParam);

                ret = ipanel_porting_socket_bind(sockfd,ipaddr,port);
                return ret ; 			
            }
            else if( IPANEL_SOCK_STREAM == socktype )
            {
                serv_addr.family= NU_FAMILY_IP;
                serv_addr.port = port;  

                serv_addr.id.is_ip_addrs[0]=(UINT8)((ip>>24)&0xFF);
                serv_addr.id.is_ip_addrs[1]=(UINT8)((ip>>16)&0xFF);
                serv_addr.id.is_ip_addrs[2]=(UINT8)((ip>>8)&0xFF);
                serv_addr.id.is_ip_addrs[3]=(UINT8)(ip&0xFF);				

                connstat = NU_Connect(sockfd,(struct addr_struct*)&serv_addr, len);
                if ( connstat < 0)
                {
                	ipanel_porting_dprintf("[ipanel_porting_socket_connect] connect failed!\n");
                	return ret;
                }

                ret = IPANEL_OK ; 
            }
        }
        else if (IPANEL_IP_VERSION_6 == ipaddr->version)
        {
            /* IPv6协议 */
            ipanel_porting_dprintf("[ipanel_porting_socket_connect] ERROR: NOT suppurt IPV6 !!! \n");
            return ret ;
        }
        else
        {
            ipanel_porting_dprintf("[ipanel_porting_socket_connect] ERROR: please set  IP_VERSION_4 !\n");
            return ret ;
        }
    }

    ipanel_porting_dprintf("[ipanel_porting_socket_connect] connect success . ret = %d\n", ret);

    return ret;
}
/***************************************************************************************************
功能说明：
	发送数据到套接字。

参数说明：
	输入参数：
		sockfd：套接字描述符
		buf：待发送数据的缓冲区
		len：缓冲区中数据的长度
		flags：操作选项，一般设为0，其他值保留
	输出参数：无
	
返    回：
	>0:执行成功返回实际发送的数据长度。
	IPANEL_ERR:失败。
***************************************************************************************************/
INT32_T ipanel_porting_socket_send ( INT32_T sockfd,CHAR_T *buf,INT32_T buflen,INT32_T flags )
{
    INT32_T ret = IPANEL_ERR;
    INT32 sendbyte ; 

    ipanel_porting_dprintf("[ipanel_porting_socket_send] sockfd=%d, buf=0x%p, buflen=0x%x, flags=%d\n",
    	                    sockfd, buf, buflen, flags);

    if ( ( sockfd >= 0 ) && ( NULL != buf ) )
    {
    	INT32_T socktype ; 
    	socktype = ipanel_get_socktype(ipanel_mgr,sockfd);
    	if( IPANEL_SOCK_DGRAM ==  socktype)
    	{
    		SOCKPARAM tmpSockParam = {0} ;
    		INT32_T port ; 
    		IPANEL_IPADDR   *ipAddr ; 
        	tmpSockParam.sockid  = sockfd ;
    		ipanel_register_get_param(ipanel_mgr,&tmpSockParam);
    		port = tmpSockParam.sockport ;
    		ipAddr = &tmpSockParam.sockip ; 

    		sendbyte = ipanel_porting_socket_sendto(sockfd,buf,buflen,0, ipAddr, port );
    		if( sendbyte < 0)
    		{
    			ipanel_porting_dprintf("[ipanel_porting_socket_send] send byte failed! \n");
    			return ret; 
    		}
    	}
    	else if( IPANEL_SOCK_STREAM == socktype)
    	{
            sendbyte = NU_Send(sockfd, (char *)buf, buflen, flags);
            if( sendbyte < 0)
            {
                ipanel_porting_dprintf("[ipanel_porting_socket_send] send byte failed! \n");
                return ret; 
            }
    	}
    }

    ipanel_porting_dprintf("[ipanel_porting_socket_send] send data byte = %d \n",sendbyte);
    return sendbyte;
}

/***************************************************************************************************
功能说明：
	发送数据到指定套接字。

参数说明：
	输入参数：
		sockfd：套接字描述符
		buf：待发送数据的缓冲区
		len：缓冲区中数据的长度
		flags：操作选项，一般设为0，其他值保留
		ipaddr：包含目的IP地址和IP版本描述的结构体指针
		port：目的端口号
	输出参数：无
	
返    回：
	>=0:执行成功返回实际发送的数据长度。
	IPANEL_ERR:失败。
****************************************************************************************************/
INT32_T ipanel_porting_socket_sendto (  
        INT32_T         sockfd,
        CHAR_T          *buf,
        INT32_T         buflen,
        INT32_T         flags,
        IPANEL_IPADDR   *ipaddr,
        INT32_T         port
        )
{
    INT32_T ret = IPANEL_ERR;
    INT32 sendtobyte ; 
    struct addr_struct serv_addr;

    ipanel_porting_dprintf("[ipanel_porting_socket_sendto] sockfd=%d, buf=0x%p, buflen=0x%x, flags=%d,\
                            ipaddr=0x%p, port=%d\n", sockfd, buf, buflen, flags, ipaddr, port);

    if ( ( sockfd >= 0 ) && ( NULL != buf ) )
    {
        if ( NULL != ipaddr )
        {
            if (IPANEL_IP_VERSION_4 == ipaddr->version)
            {
                unsigned int ip = ipaddr->addr.ipv4;

                ipanel_porting_dprintf("[ipanel_porting_socket_sendto] ip = 0x%x , port = %d \n", ip, port); 

                serv_addr.family= NU_FAMILY_IP;
                serv_addr.port = port ;                 

                serv_addr.id.is_ip_addrs[0]=(UINT8)((ip>>24)&0xFF);
                serv_addr.id.is_ip_addrs[1]=(UINT8)((ip>>16)&0xFF);
                serv_addr.id.is_ip_addrs[2]=(UINT8)((ip>>8)&0xFF);
                serv_addr.id.is_ip_addrs[3]=(UINT8)(ip&0xFF);

                sendtobyte = NU_Send_To(sockfd, (char*)buf, buflen, NULL, (struct addr_struct*)&serv_addr, sizeof(struct addr_struct));			
                if( sendtobyte < 0) 
                {
                    ipanel_porting_dprintf("[ipanel_porting_socket_sendto] sendto byte failed! \n");
                    return ret ; 
                }
            }
            else if (IPANEL_IP_VERSION_6 == ipaddr->version)
            {
                /* IPv6协议 */
                ipanel_porting_dprintf("[ipanel_porting_socket_sendto] ERROR: NOT suppurt IPV6 !!! \n");
                return ret ; 
            }	
            else
            {
                ipanel_porting_dprintf("[ipanel_porting_socket_sendto] ERROR: please set  IP_VERSION_4 !\n");
                return ret ;
            }				  
        }
    }

    ipanel_porting_dprintf("[ipanel_porting_socket_sendto] sendto data byte = %d \n",sendtobyte);

    return sendtobyte;
}
/***************************************************************************************************
功能说明：
	从一个套接字接收数据。
	
参数说明：
	输入参数：
		sockfd：套接字描述符
		buf：用于接收数据的缓冲区
		len：缓冲区的长度
		flags：操作选项，一般设为0，其他值保留
	输出参数：无
	
返    回：
	>=0: 执行成功返回实际接收的数据长度。
	IPANEL_ERR:失败。

***************************************************************************************************/
INT32_T ipanel_porting_socket_recv ( INT32_T sockfd,CHAR_T *buf,INT32_T buflen,INT32_T flags )
{
    INT32_T ret = IPANEL_ERR;
    INT32 recvbyte = 0 ; 


    ipanel_porting_dprintf("[ipanel_porting_socket_recv] sockfd=%d, buf=0x%p, buflen=0x%x, flags=%d\n",
                            sockfd, buf, buflen, flags);

    if ( sockfd >= 0 )
    {
        INT32_T socktype ; 
        IPANEL_IPADDR ipAddr ;
        INT32_T port ;
        socktype = ipanel_get_socktype(ipanel_mgr,sockfd);
        if(IPANEL_SOCK_DGRAM ==  socktype) 
        {
        	ipAddr.version = IPANEL_IP_VERSION_4 ;
        	ret = ipanel_porting_socket_recvfrom(sockfd,buf,buflen,0,&ipAddr,&port);
        	return ret ;
        }
        else if( IPANEL_SOCK_STREAM == socktype)
        {
            recvbyte = NU_Recv(sockfd, buf, buflen, 0);
        	if ( recvbyte < 0) 
        	{
        		/*
        			静态的XML文件发送过来的时候，HTTP会加上content-length,
        			上层应该可以根据这个值判断数据是不是接收完毕，
        			然后就关闭socket

        			动态的XML文件就通过recv返回值0来判定。。。
        			:) 我想其实是不对的，嘿嘿!

        			<如果recv函数在等待协议接收数据时网络中断了，那么它返回0。>
        			
        			原因:
        			tcp是面向流的传输协议 它永远没有结束的标记
        			你必须来分析承载在tcp上的应用协议来确定报
        			文的分界!!!
        		*/
        		if( NU_NOT_CONNECTED == recvbyte )  // 暂时这样做让上层关闭socket吧
        			return 0;
        	
        		ipanel_porting_dprintf("[ipanel_porting_socket_recv] recv byte failed! \n");
        		return ret ;
        	}
        }else
        {
            return ret;
        }      
    }

    return recvbyte;
}

/***************************************************************************************************
功能说明：
	从一个指定的套接字接收数据。
	
参数说明：
	输入参数：
		sockfd：套接字描述符
		buf：用于接收数据的缓冲区
		len：缓冲区的长度
		flags：操作选项，一般设为0，其他值保留
		ipaddr：包含IP版本描述的结构体指针
	输出参数：
		ipaddr：保存发送方IP地址的结构体指针
		port：保存发送方的端口号
	
返    回：
	>=0: 执行成功返回实际接收的数据长度。
	IPANEL_ERR:失败。

***************************************************************************************************/
INT32_T ipanel_porting_socket_recvfrom ( 
        INT32_T sockfd,
        CHAR_T *buf,
        INT32_T buflen,
        INT32_T flags,
        IPANEL_IPADDR *ipaddr,
        INT32_T *port 
    )
{
    INT32_T ret = IPANEL_ERR;
    INT32 recvfbyte ;  
    struct addr_struct serv_addr;

    ipanel_porting_dprintf("[ipanel_porting_socket_recvfrom] sockfd=%d, buf=0x%p, buflen=0x%x,\
                            flags=%d .\n", sockfd, buf, buflen, flags);

    if ( ( sockfd >= 0 ) && ( NULL != buf ) )
    {	
        if (IPANEL_IP_VERSION_4 == ipaddr->version)
        {
            INT16 len = sizeof(serv_addr);

            recvfbyte = NU_Recv_From(sockfd, buf, buflen, 0, (struct addr_struct*)&serv_addr, &len);

            if ( recvfbyte > 0 )
            {
                if ( ipaddr != NULL)
                {
                    ipaddr->addr.ipv4 = CHARTOINT(serv_addr.id.is_ip_addrs);
                }

                if ( port  != NULL )
                {
                    *port = serv_addr.port;
                }

                ipanel_porting_dprintf("[ipanel_porting_socket_recvfrom] recv data length = %d, \
                                        ip = 0x%x \n",recvfbyte,  ipaddr->addr.ipv4); 
            }
            else
            {
                ipanel_porting_dprintf("[ipanel_porting_socket_recvfrom] recv from byte failed! \n");
                return ret ; 
            }			  
        }
        else if (IPANEL_IP_VERSION_6 == ipaddr->version)
        {
            /* IPv6协议 */
            ipanel_porting_dprintf("[ipanel_porting_socket_recvfrom] ERROR: NOT suppurt IPV6 !!! \n");
            return ret ; 
        }
        else
        {
            ipanel_porting_dprintf("[ipanel_porting_socket_recvfrom] ERROR: please set  IP_VERSION_4 !\n");
            return ret ;
        }			 
    }

    return recvfbyte;
}

/***************************************************************************************************
功能说明：
	获取与套接字有关的本地协议地址，即主机地址。
	
参数说明：
	输入参数：
		sockfd：套接字描述符
		ipaddr：包含IP版本描述的结构体指针
	输出参数：
		ipaddr：保存主机协议IP地址的结构体指针
		port：保存主机协议的端口号

返    回：
	IPANEL_OK:成功;
	IPANEL_ERR:失败。

***************************************************************************************************/
INT32_T ipanel_porting_socket_getsockname ( INT32_T sockfd,IPANEL_IPADDR *ipaddr,INT32_T *port )
{
    INT32_T ret = IPANEL_ERR;
    STATUS getstat ; 
	struct sockaddr_struct serv_addr;

    ipanel_porting_dprintf("[ipanel_porting_socket_getsockname] sockfd=%d, ipaddr=0x%p,\
                          port=%p\n", sockfd, ipaddr, port);

    if ( sockfd >= 0 )
    {
        if (IPANEL_IP_VERSION_4 == ipaddr->version)
        {
            INT16 len = sizeof (serv_addr);
            getstat = NU_Get_Sock_Name(sockfd, (struct sockaddr_struct*)&serv_addr, &len);
            if ( getstat >= 0 )
            {
                if ( ipaddr )
                {
                    ipaddr->addr.ipv4 = CHARTOINT(serv_addr.ip_num.is_ip_addrs);
                }

                if ( port )
                {
                    *port = serv_addr.port_num;
                }

                ipanel_porting_dprintf("[ipanel_porting_socket_getsockname] ip = 0x%x , port = %d n",
                                        ipaddr->addr.ipv4, *port); 
                ret = IPANEL_OK; 
            }
        }
        else if (IPANEL_IP_VERSION_6 == ipaddr->version)
        {
            /* IPv6协议 */
            ipanel_porting_dprintf("[ipanel_porting_socket_getsockname] ERROR: NOT suppurt IPV6 !!! \n");
            return ret ;
        }		
        else
        {
        	ipanel_porting_dprintf("[ipanel_porting_socket_getsockname] ERROR: please set  IP_VERSION_4 !\n");
        	return ret ;
        }
    }

    return ret;
}

/***************************************************************************************************
功能说明：
	确定一个或多个套接字的状态，判断套接字上是否有数据，或者能否向一个套接字写入数据。
	
参数说明：
	输入参数：
		nfds：    需要检查的文件描述符个数，数值应该比readset、writeset、exceptset
		          中最大数更大，而不是实际的文件描述符总数
		readset： 用来检查可读性的一组文件描述符
		writeset：用来检查可写性的一组文件描述符
		exceptset：用来检查意外状态的文件描述符，错误不属于意外状态
		timeout： 大于0表示等待多少毫秒；为IPANEL_NO_WAIT(0)时表示不等待立即返回，
		          为IPANEL_WAIT_FOREVER(-1)表示永久等待。
	输出参数：无
	
返    回：
	响应操作的对应操作文件描述符的总数，且readset、writeset、exceptset三组数据均在恰当位置被修改；
	IPANEL_ERR:失败，等待超时或出错。
***************************************************************************************************/
INT32_T ipanel_porting_socket_select ( 
        INT32_T             nfds,
        IPANEL_FD_SET_S     *readset,
        IPANEL_FD_SET_S     *writeset,
        IPANEL_FD_SET_S     *exceptset,
        INT32_T timeout 
    )
{
    INT32_T i;
    INT32_T ret  = IPANEL_ERR;
    STATUS selstat ; 
    UNSIGNED timespan ; 
    FD_SET fds_r ,fds_w ,fds_e;

    NU_FD_Init(&fds_r);
    NU_FD_Init(&fds_w);
    NU_FD_Init(&fds_e);

    if ( nfds <= 0 )
    {
        return ret;
    }

    if( timeout == -1)
    {
        timespan = NU_SUSPEND ;  // 一直挂起
    }
    else if( timeout == 0)
    {
        timespan = NU_NO_SUSPEND ;  // 立即返回
    }
    else
    {
        timespan = timeout ;    // 等待超时值
    }

    for (i = 0; i < sizeof(FD_SET) / sizeof(fds_r.words[0]); i++)
    {
        if ( readset )
        {
            fds_r.words[i] = readset->fds_bits[i];
        }

        if ( writeset )
        {
            fds_w.words[i] = writeset->fds_bits[i];
        }

        if ( exceptset )
        {
            fds_e.words[i] = exceptset->fds_bits[i];
        }
    }

    selstat = NU_Select(nfds, &fds_r, &fds_w, &fds_e, timespan); 		
    if ( selstat < 0 )
    {
    	return ret;
    }

    for (i = 0; i < sizeof(FD_SET) / sizeof(fds_r.words[0]); i++)
    {
        if ( readset )
        {
            readset->fds_bits[i] = fds_r.words[i];
        }

        if ( writeset )
        {
            writeset->fds_bits[i] = 0x00 ; //fds_w.words[i];
        }

        if ( exceptset )
        {
            exceptset->fds_bits[i] = 0x00 ; //fds_e.words[i];
        }
    }

    if ( selstat == NU_SUCCESS) 
    {
        //ipanel_porting_dprintf("[ipanel_porting_socket_select] select success. \n");
        //ipanel_porting_dprintf("[ipanel_porting_socket_select] read/write/except   0x%x   0x%x   
        //                        0x%x timeout = %d  \n", readset, writeset, exceptset, timeout);   
        ret = IPANEL_OK;
    }

    return ret;
}


/***************************************************************************************************
功能说明：
	获取套接字的属性。可获取套接字的属性见下表中get项为Y的属性项。
	
参数说明：

	输入参数：
		sockfd： 套接字描述符
		level： 选项定义的层次
		optname：需要获取的属性名\

		           level            	    optname              get  set   数据类型
		|---------------------------|--------------------------|----|-----|-------
	    |                           |IPANEL_SO_BROADCAST	   |Y	|  Y  |INT32_T
	    | IPANEL_SOL_SOCKET         |--------------------------|----|-----|-----
		|                   	    |IPANEL_SO_KEEPALIVE       |Y	|  Y  |INT32_T
		|                   	    |--------------------------|----|-----|-----
		|                   	    |IPANEL_SO_REUSEADDR       |Y	|  Y  |INT32_T
		|---------------------------|--------------------------|----|-----|-----
	    |                   	    |IPANEL_IP_ADD_MEMBERSHIP  |N	|  Y  |PANEL_IP_MREQ
	    |IPANEL_IPPROTO_IP		    |--------------------------|----|-----|-----
		|                   	  	|IPANEL_IP_DROP_MEMBERSHIP |N	|  Y  |PANEL_IP_MREQ
		|___________________________|__________________________|____|_____|_________
	输出参数：
	    optval：指向保存属性值的缓冲区
	    optlen：指向保存属性值的长度
	    
返    回：
IPANEL_OK:成功
IPANEL_ERR:失败。

***************************************************************************************************/
INT32_T ipanel_porting_socket_getsockopt(
        INT32_T sockfd,
        INT32_T level,
        INT32_T name,
        CHAR_T *value,
        INT32_T *len
    )
{
	INT32_T ret = IPANEL_OK;
	STATUS sockret ; 
	INT32_T optlevel , optname;

	switch(level)
	{
		case IPANEL_SOL_SOCKET:
			optlevel = SOL_SOCKET;
			switch(name)
			{
				case IPANEL_SO_BROADCAST:
					optname = SO_BROADCAST ;
					break;
					
				case IPANEL_SO_REUSEADDR:
					break;
					
				case IPANEL_SO_KEEPALIVE:
					break; 

				default:
					break; 
			}
			break;

		case IPANEL_IPPROTO_IP: 
			optlevel = IPPROTO_IP ;
			switch(name)
			{
 				case IPANEL_IP_ADD_MEMBERSHIP :
					optname = IP_ADD_MEMBERSHIP;
					break ; 

				case IPANEL_IP_DROP_MEMBERSHIP:
					optname = IP_DROP_MEMBERSHIP;
					break;

				default:
					break;
			}
			break;

		default:
			break;
	}
	
	sockret = NU_Getsockopt(sockfd,optlevel,optname,(VOID *)value,(INT *)len);
	if( sockret != NU_SUCCESS)
	{
		ipanel_porting_dprintf("[ipanel_porting_socket_getsockopt] get socket opt failed! \n");
		ret = IPANEL_ERR ;
	}

	return ret;
}

/***************************************************************************************************
功能说明：
	设置套接字的属性。可设置的套接字的属性见下表中set项为Y的属性项。

参数说明：
	输入参数：
		sockfd：套接字描述符
		level： 选项定义的层次
		optname：需要设置的属性名
		optval：指向保存属性值的缓冲区
		optlen： optval缓冲区的长度
	输出参数：无

返    回：
	IPANEL_OK:成功；
	IPANEL_ERR:失败。
***************************************************************************************************/
INT32_T ipanel_porting_socket_setsockopt(
        INT32_T sockfd,
        INT32_T level,
        INT32_T name,
        CHAR_T *value,
        INT32_T len
    )
{
	INT32_T ret = IPANEL_OK;
	STATUS sockret ; 
	INT32_T optlevel =0, optname =0, optlen =0; 	
	CHAR_T  *optvalue = NULL ; 
	IPANEL_IP_MREQ *ipanel_ip_mreq = {0};
	IP_MREQ  ip_mreq;


    ipanel_porting_dprintf("[ipanel_porting_socket_setsockopt] level %d  name = %d len = %d \n", 
                            level, name , len); 

	switch(level)
	{
		case IPANEL_SOL_SOCKET:
			optlevel = SOL_SOCKET;
			switch(name)
			{
				case IPANEL_SO_BROADCAST:
					optname = SO_BROADCAST;
					(*optvalue) = SO_BROADCAST ; 
					optlen   =  sizeof(INT16_T) ; 
					break;
					
				case IPANEL_SO_REUSEADDR:
					break;
					
				case IPANEL_SO_KEEPALIVE:
					break; 

				default:
					break; 
			}
			break;

		case IPANEL_IPPROTO_IP: 
			optlevel = IPPROTO_IP ;
			
			ipanel_ip_mreq = (IPANEL_IP_MREQ*)value ; 
			// NOTE: 必须加上地址转换函数!!!
			ip_mreq.sck_inaddr = LONGSWAP(ipanel_ip_mreq->imr_interface.addr);
			ip_mreq.sck_multiaddr = LONGSWAP(ipanel_ip_mreq->imr_multiaddr.addr);
			optvalue = (CHAR_T*)&ip_mreq ;
			
			optlen = sizeof(IP_MREQ);
			switch(name)
			{
 				case IPANEL_IP_ADD_MEMBERSHIP :  // 增加组播地址
					optname = IP_ADD_MEMBERSHIP;
					break ; 

				case IPANEL_IP_DROP_MEMBERSHIP:  // 删除组播地址
					optname = IP_DROP_MEMBERSHIP;
					break;

				default:
					break;
			}
			break;

		default:
			break;
	} 
	
	sockret = NU_Setsockopt(sockfd,optlevel,optname,(VOID *)optvalue,(INT)optlen);
	if( sockret != NU_SUCCESS)
	{
		ipanel_porting_dprintf("[ipanel_porting_socket_setsockopt] set socket opt failed! \n");
		ret = IPANEL_ERR ;
	}

    ipanel_porting_dprintf("[ipanel_porting_socket_setsockopt]     success  \n");

	return ret;
}

/***************************************************************************************************
功能说明：
	改变套接字属性为非阻塞。注：此函数只提供将套接字属性设置为非阻塞，
	不提供将套接字属性设置为阻塞的功能。

参数说明：

	输入参数：
		sockfd：套接字描述符
		cmd：命令类型，为IPANEL_FIONBIO
		arg：命令参数，1表示非阻塞
	输出参数：无

返    回：
	IPANEL_OK:成功;
	IPANEL_ERR:失败。
***************************************************************************************************/
INT32_T ipanel_porting_socket_ioctl ( INT32_T sockfd, INT32_T cmd, INT32_T arg )
{
    INT32_T ret = IPANEL_ERR;
    STATUS ioctlstat ; 
		
    ipanel_porting_dprintf("[ipanel_porting_socket_ioctl] sockfd=%d, cmd=%d, arg=%d \n",
                            sockfd, cmd, arg);
					              
    if ( ( sockfd >= 0 ) && ( IPANEL_FIONBIO == cmd ) )
    {
		ioctlstat = NU_Fcntl(sockfd,NU_SETFLAG,NU_FALSE);	  // 非阻塞模式 	     
		if( ioctlstat >= 0) 
		{
		  	ret = IPANEL_OK; 
		}
    }

    return ret;
}               

/***************************************************************************************************
功能说明：
	禁止在一个套接字上进行数据的接收与发送。尽量不要使用此函数。

参数说明：
	输入参数：
		sockfd：套接字描述符。
		what：为IPANEL_DIS_RECEIVE表示禁止接收；为IPANEL_DIS_SEND表示禁止发送，
		      为IPANEL_DIS_ALL表示同时禁止发送和接收。
	输出参数：无

返    回：
	IPANEL_OK:成功;
	IPANEL_ERR:失败。
***************************************************************************************************/
INT32_T ipanel_porting_socket_shutdown ( INT32_T sockfd, INT32_T what )
{
    INT32_T ret = IPANEL_OK;
	INT32_T flags   = -1;

    ipanel_porting_dprintf("[ipanel_porting_socket_shutdown] sockfd=%d, what=%d\n", sockfd, what);
	
    if ( ( sockfd >= 0 ) && ( ( what >= 0 ) && ( what <= 2 ) ) )
    {
        if (IPANEL_DIS_RECEIVE == what)
        {
            flags = 0;
        }
        else if (IPANEL_DIS_SEND == what)
        {
            flags = 1;
        }
        else if (IPANEL_DIS_ALL == what)
        {
            flags = 2;
        }
    }

    return ret;
}

/***************************************************************************************************
功能说明：
	关闭指定的套接字
	
参数说明：

	输入参数：
	    sockfd：套接字描述符
	输出参数：无

返    回：
	IPANEL_OK:成功;
	IPANEL_ERR:失败。

***************************************************************************************************/
INT32_T ipanel_porting_socket_close ( INT32_T sockfd )
{
    INT32_T ret = IPANEL_ERR;
    STATUS closestat ; 
	  
    ipanel_porting_dprintf("[ipanel_porting_socket_close] sockfd=%d\n", sockfd);

    if ( sockfd >= 0 )
    {
        ipanel_register_unregister(ipanel_mgr,sockfd);
        closestat = NU_Close_Socket (sockfd );
        if ( closestat >= 0)
        {
            ret = IPANEL_OK; 
        }
    }

    return ret;
}

/***************************************************************************************************
功能说明：
    获取准备让iPanel MiddleWare同时处理socket的最大个数。注意：该数目建议在8个以上，
    16个以下，即[8~16]，缺省为8。若iPanel MiddleWare只是浏览网页的话，8个socket
    就足够了，若iPanel MiddleWare在浏览网页的时候同时又需要使用VOIP、VOD点播等
    应用就需要多个socket以提高速度，否则可能出现socket都已经使用，
    需要关闭一些socket才能运行某个应用的情况。
    
参数说明：
	输入参数：无
	输出参数：无

返    回：
    >0:返回iPanel MiddleWare可创建的最大socket数目。
	IPANEL_ERR:失败。

***************************************************************************************************/
INT32_T ipanel_porting_socket_get_max_num(void)
{
    return IPANEL_MAX_SOCKETS;
}

INT32_T ipanel_socket_init(VOID)
{
	INT32_T ret = IPANEL_OK; 

	// 在此处可以设置网络配置.
	// IP Address : 10.240.3.223  subway:255.255.255.128 Get way : 10.240.3.129
	//cnxt_nuptst_init(0x0af003DF,0xFFFFFF80,0x0af00381);

	// IP Address : 192.168.27.100  subway:255.255.255.0 Get way : 192.168.27.1
	cnxt_nuptst_init(0xC0A81B64,0xFFFFFF00,0xC0A81B01);
    
	ipanel_mgr = ipanel_register_new();
	if( NULL == ipanel_mgr )
	{
		ipanel_porting_dprintf("[ipanel_socket_init] network init failed! \n");
		ret = IPANEL_ERR ; 
	}

	return ret;
}

VOID ipanel_socket_exit(VOID)
{
	ipanel_porting_dprintf("[ipanel_socket_exit] is called\n");

	ipanel_register_delete(ipanel_mgr);
	ipanel_mgr = NULL;
}

