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
����˵����
	����һ���׽��֡�
	
����˵����

	���������
		family��ָ��Ҫ�������׽���Э���塣IPANEL_AF_INET����ʾIPv4Э�飬
		        IPANEL_AV_INET6��ʾIPv6Э�飬����ֵ����
		type��ָ��Ҫ�������׽��ֵ����͡�IPANEL_SOCK_STREAM����ʾSOCK_STREAM��
		      IPANEL_SOCK_DGRAM����ʾSOCK_DGRAM������ֵ����
		protocol��ָ������Э�顣ͨ������ΪIPANEL_IPPROTO_IP����ʾ��typeƥ���Ĭ��Э�飬����ֵ����
	�����������
	
��    �أ�
���׽��ֽ����ɹ����򷵻طǸ������������򷵻�-1��
>��0: �׽��ֽ����ɹ����򷵻طǸ���������
	  �������ֵӦ�ò�����IPANEL_NFDBITS�Ķ��巶Χ��
IPANEL_ERR: �����׽���ʧ�ܡ�

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
����˵����
	��ָ���Ķ˿ں����׽��ְ���һ��
	
����˵����
	���������
		sockfd���׽���������
		ipaddr����������IP��ַ��IP�汾�����ṹ���ָ��
		port���˿ں�
	�����������
	
��    �أ�
	IPANEL_OK:�ɹ�;
	IPANEL_ERR:ʧ�ܡ�
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

            if( ip != IP_ADDR_ANY)  // ֻ�б��ص�ַ���ܰ󶨳ɹ�
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
        	/* IPv6Э�� */
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
����˵����
	��һ���׽���ת���ɼ����׽��֡�

����˵����
	���������
		sockfd���׽���������
		backlog�����ն��е���󳤶�
	�����������
	
��    �أ�
	IPANEL_OK:�ɹ�;
	IPANEL_ERR:ʧ�ܡ�
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
����˵����
	��Ӧ�������󣬽������Ӳ�����һ���µ�socket������������������

����˵����
	���������
		sockfd�������׽���������
		ipaddr������IP�汾�����Ľṹ��ָ��
	���������
		ipaddr���������ӶԷ�IP��ַ�Ľṹ��ָ��
		port���������ӶԷ��Ķ˿ں�
		
��    �أ�
	>��0:ִ�гɹ��򷵻طǸ���������
	IPANEL_ERR:ʧ�ܡ�
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
            /* IPv6Э�� */
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
����˵����
	���׽�����ָ�����׽�����������
	
����˵����
	���������
		sockfd���׽���������
		ipaddr������������IP��ַ��IP�汾�����Ľṹ��ָ��
		port�������ӵĶ˿ں�
	�����������
	
��    �أ�
	IPANEL_OK:�ɹ�;
	IPANEL_ERR:ʧ�ܡ�
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
            /* IPv6Э�� */
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
����˵����
	�������ݵ��׽��֡�

����˵����
	���������
		sockfd���׽���������
		buf�����������ݵĻ�����
		len�������������ݵĳ���
		flags������ѡ�һ����Ϊ0������ֵ����
	�����������
	
��    �أ�
	>0:ִ�гɹ�����ʵ�ʷ��͵����ݳ��ȡ�
	IPANEL_ERR:ʧ�ܡ�
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
����˵����
	�������ݵ�ָ���׽��֡�

����˵����
	���������
		sockfd���׽���������
		buf�����������ݵĻ�����
		len�������������ݵĳ���
		flags������ѡ�һ����Ϊ0������ֵ����
		ipaddr������Ŀ��IP��ַ��IP�汾�����Ľṹ��ָ��
		port��Ŀ�Ķ˿ں�
	�����������
	
��    �أ�
	>=0:ִ�гɹ�����ʵ�ʷ��͵����ݳ��ȡ�
	IPANEL_ERR:ʧ�ܡ�
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
                /* IPv6Э�� */
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
����˵����
	��һ���׽��ֽ������ݡ�
	
����˵����
	���������
		sockfd���׽���������
		buf�����ڽ������ݵĻ�����
		len���������ĳ���
		flags������ѡ�һ����Ϊ0������ֵ����
	�����������
	
��    �أ�
	>=0: ִ�гɹ�����ʵ�ʽ��յ����ݳ��ȡ�
	IPANEL_ERR:ʧ�ܡ�

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
        			��̬��XML�ļ����͹�����ʱ��HTTP�����content-length,
        			�ϲ�Ӧ�ÿ��Ը������ֵ�ж������ǲ��ǽ�����ϣ�
        			Ȼ��͹ر�socket

        			��̬��XML�ļ���ͨ��recv����ֵ0���ж�������
        			:) ������ʵ�ǲ��Եģ��ٺ�!

        			<���recv�����ڵȴ�Э���������ʱ�����ж��ˣ���ô������0��>
        			
        			ԭ��:
        			tcp���������Ĵ���Э�� ����Զû�н����ı��
        			�����������������tcp�ϵ�Ӧ��Э����ȷ����
        			�ĵķֽ�!!!
        		*/
        		if( NU_NOT_CONNECTED == recvbyte )  // ��ʱ���������ϲ�ر�socket��
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
����˵����
	��һ��ָ�����׽��ֽ������ݡ�
	
����˵����
	���������
		sockfd���׽���������
		buf�����ڽ������ݵĻ�����
		len���������ĳ���
		flags������ѡ�һ����Ϊ0������ֵ����
		ipaddr������IP�汾�����Ľṹ��ָ��
	���������
		ipaddr�����淢�ͷ�IP��ַ�Ľṹ��ָ��
		port�����淢�ͷ��Ķ˿ں�
	
��    �أ�
	>=0: ִ�гɹ�����ʵ�ʽ��յ����ݳ��ȡ�
	IPANEL_ERR:ʧ�ܡ�

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
            /* IPv6Э�� */
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
����˵����
	��ȡ���׽����йصı���Э���ַ����������ַ��
	
����˵����
	���������
		sockfd���׽���������
		ipaddr������IP�汾�����Ľṹ��ָ��
	���������
		ipaddr����������Э��IP��ַ�Ľṹ��ָ��
		port����������Э��Ķ˿ں�

��    �أ�
	IPANEL_OK:�ɹ�;
	IPANEL_ERR:ʧ�ܡ�

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
            /* IPv6Э�� */
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
����˵����
	ȷ��һ�������׽��ֵ�״̬���ж��׽������Ƿ������ݣ������ܷ���һ���׽���д�����ݡ�
	
����˵����
	���������
		nfds��    ��Ҫ�����ļ���������������ֵӦ�ñ�readset��writeset��exceptset
		          ����������󣬶�����ʵ�ʵ��ļ�����������
		readset�� �������ɶ��Ե�һ���ļ�������
		writeset����������д�Ե�һ���ļ�������
		exceptset�������������״̬���ļ���������������������״̬
		timeout�� ����0��ʾ�ȴ����ٺ��룻ΪIPANEL_NO_WAIT(0)ʱ��ʾ���ȴ��������أ�
		          ΪIPANEL_WAIT_FOREVER(-1)��ʾ���õȴ���
	�����������
	
��    �أ�
	��Ӧ�����Ķ�Ӧ�����ļ�����������������readset��writeset��exceptset�������ݾ���ǡ��λ�ñ��޸ģ�
	IPANEL_ERR:ʧ�ܣ��ȴ���ʱ�����
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
        timespan = NU_SUSPEND ;  // һֱ����
    }
    else if( timeout == 0)
    {
        timespan = NU_NO_SUSPEND ;  // ��������
    }
    else
    {
        timespan = timeout ;    // �ȴ���ʱֵ
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
����˵����
	��ȡ�׽��ֵ����ԡ��ɻ�ȡ�׽��ֵ����Լ��±���get��ΪY�������
	
����˵����

	���������
		sockfd�� �׽���������
		level�� ѡ���Ĳ��
		optname����Ҫ��ȡ��������\

		           level            	    optname              get  set   ��������
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
	���������
	    optval��ָ�򱣴�����ֵ�Ļ�����
	    optlen��ָ�򱣴�����ֵ�ĳ���
	    
��    �أ�
IPANEL_OK:�ɹ�
IPANEL_ERR:ʧ�ܡ�

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
����˵����
	�����׽��ֵ����ԡ������õ��׽��ֵ����Լ��±���set��ΪY�������

����˵����
	���������
		sockfd���׽���������
		level�� ѡ���Ĳ��
		optname����Ҫ���õ�������
		optval��ָ�򱣴�����ֵ�Ļ�����
		optlen�� optval�������ĳ���
	�����������

��    �أ�
	IPANEL_OK:�ɹ���
	IPANEL_ERR:ʧ�ܡ�
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
			// NOTE: ������ϵ�ַת������!!!
			ip_mreq.sck_inaddr = LONGSWAP(ipanel_ip_mreq->imr_interface.addr);
			ip_mreq.sck_multiaddr = LONGSWAP(ipanel_ip_mreq->imr_multiaddr.addr);
			optvalue = (CHAR_T*)&ip_mreq ;
			
			optlen = sizeof(IP_MREQ);
			switch(name)
			{
 				case IPANEL_IP_ADD_MEMBERSHIP :  // �����鲥��ַ
					optname = IP_ADD_MEMBERSHIP;
					break ; 

				case IPANEL_IP_DROP_MEMBERSHIP:  // ɾ���鲥��ַ
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
����˵����
	�ı��׽�������Ϊ��������ע���˺���ֻ�ṩ���׽�����������Ϊ��������
	���ṩ���׽�����������Ϊ�����Ĺ��ܡ�

����˵����

	���������
		sockfd���׽���������
		cmd���������ͣ�ΪIPANEL_FIONBIO
		arg�����������1��ʾ������
	�����������

��    �أ�
	IPANEL_OK:�ɹ�;
	IPANEL_ERR:ʧ�ܡ�
***************************************************************************************************/
INT32_T ipanel_porting_socket_ioctl ( INT32_T sockfd, INT32_T cmd, INT32_T arg )
{
    INT32_T ret = IPANEL_ERR;
    STATUS ioctlstat ; 
		
    ipanel_porting_dprintf("[ipanel_porting_socket_ioctl] sockfd=%d, cmd=%d, arg=%d \n",
                            sockfd, cmd, arg);
					              
    if ( ( sockfd >= 0 ) && ( IPANEL_FIONBIO == cmd ) )
    {
		ioctlstat = NU_Fcntl(sockfd,NU_SETFLAG,NU_FALSE);	  // ������ģʽ 	     
		if( ioctlstat >= 0) 
		{
		  	ret = IPANEL_OK; 
		}
    }

    return ret;
}               

/***************************************************************************************************
����˵����
	��ֹ��һ���׽����Ͻ������ݵĽ����뷢�͡�������Ҫʹ�ô˺�����

����˵����
	���������
		sockfd���׽�����������
		what��ΪIPANEL_DIS_RECEIVE��ʾ��ֹ���գ�ΪIPANEL_DIS_SEND��ʾ��ֹ���ͣ�
		      ΪIPANEL_DIS_ALL��ʾͬʱ��ֹ���ͺͽ��ա�
	�����������

��    �أ�
	IPANEL_OK:�ɹ�;
	IPANEL_ERR:ʧ�ܡ�
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
����˵����
	�ر�ָ�����׽���
	
����˵����

	���������
	    sockfd���׽���������
	�����������

��    �أ�
	IPANEL_OK:�ɹ�;
	IPANEL_ERR:ʧ�ܡ�

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
����˵����
    ��ȡ׼����iPanel MiddleWareͬʱ����socket����������ע�⣺����Ŀ������8�����ϣ�
    16�����£���[8~16]��ȱʡΪ8����iPanel MiddleWareֻ�������ҳ�Ļ���8��socket
    ���㹻�ˣ���iPanel MiddleWare�������ҳ��ʱ��ͬʱ����Ҫʹ��VOIP��VOD�㲥��
    Ӧ�þ���Ҫ���socket������ٶȣ�������ܳ���socket���Ѿ�ʹ�ã�
    ��Ҫ�ر�һЩsocket��������ĳ��Ӧ�õ������
    
����˵����
	�����������
	�����������

��    �أ�
    >0:����iPanel MiddleWare�ɴ��������socket��Ŀ��
	IPANEL_ERR:ʧ�ܡ�

***************************************************************************************************/
INT32_T ipanel_porting_socket_get_max_num(void)
{
    return IPANEL_MAX_SOCKETS;
}

INT32_T ipanel_socket_init(VOID)
{
	INT32_T ret = IPANEL_OK; 

	// �ڴ˴�����������������.
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

