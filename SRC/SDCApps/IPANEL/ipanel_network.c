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
#include "ipanel_task.h"
#include "ipanel_tuner.h"
#include "ipanel_network.h"
#include "ipanel_device.h"
#include "ipaneldtv_task.h"
#include "ipanel_porting_event.h"

extern UINT32_T g_main_queue;

static u_int8 m_dns_addr[4];
static u_int8 m_dns_mode=1;
static CHAR   g_ln_name[] = "ethernet" ;

static DHCP_STRUCT *m_dhcp_ptr = NU_NULL;
INT32_T ipanel_network_set_dns_server();

static int g_dhcp_flag = 0 ; // 只有是利用DHCP 获取的地址才利用dhcp release it 

static UINT32_T m_netcheck_task_id = IPANEL_NULL;
#define IPANEL_NETWORK_CHECK_TIME     10000

//--------------------------------------------------------------------------------------------------
// Internal Prototypes
//--------------------------------------------------------------------------------------------------
//
union ORDERBYTE
{
	unsigned int 	dwords;
	struct
	{
		unsigned char	byte0;
		unsigned char	byte1;
		unsigned char	byte2;
		unsigned char	byte3;
	}bytes;
};

/******************************************************************************/
/*Description: convert char  to INT data type                                 */
/*Input      : no                                                             */
/*Output     : no                                                             */
/*Return     : no                                     				          */
/******************************************************************************/
static unsigned int Conver_IP_Address(unsigned char *p)
{
	return ((unsigned int)p[0] << 24) + 
	       ((unsigned int)p[1] << 16) + 
	       ((unsigned int)p[2] << 8) + 
	        (unsigned int)p[3];
}

/******************************************************************************/
/*Description: convert string  to char* data type                             */
/*Input      : no                                                             */
/*Output     : no                                                             */
/*Return     : no                                     				          */
/******************************************************************************/
static void ipanel_str2ip(const char *str, unsigned char* ipaddr)
{
	const char *p=str;
	char ip[4][3]={0};
	int32 digi_num=0;
	int32 ip_seg=0;
	int32 j;
	int32 tmp=0;

	// 去掉不合理的前缀字符
	while( *p>'9' && *p<'0' )
		p++;

	while(1)
	{
		while(*p!='.' && *p)
		{
			p++;
			digi_num++;
		}

		p--;
		switch (digi_num)
		{
			case 3:
				ip[ip_seg][0]=*(p-2);
				ip[ip_seg][1]=*(p-1);
				ip[ip_seg][2]=*(p);
				break;
			case 2:
				ip[ip_seg][0]='0';
				ip[ip_seg][1]=*(p-1);
				ip[ip_seg][2]=*(p);
				break;
			default:
				ip[ip_seg][0]='0';
				ip[ip_seg][1]='0';
				ip[ip_seg][2]=*(p);
				break;
		}

		ip_seg++;
		p++;

		if (*p=='.')
			p++;

		if (*p=='\0')
			break;

		digi_num=0;
	}

	// 填入所有的IP 地址值
	for (ip_seg=0;ip_seg<4;ip_seg++)
	{
		for (digi_num=0,j=100;digi_num<3;digi_num++,j/=10)
			tmp+=((ip[ip_seg][digi_num]-'0')*j);

		ipaddr[ip_seg] = tmp ; 
		tmp=0;
	}
}

/******************************************************************************/
/* Description: ping the appointed IP address and len is 32byte               */
/* Input      : no                                                            */
/* Output     : no                                                            */
/* Return     : no                                     				          */
/******************************************************************************/
void ipanel_ping_ip(u_int32 uip, u_int32 len )
{
	int status;
	int time1, time;
	u_int8 ip[4];

	IPANEL_QUEUE_MESSAGE PingMsg;

	PingMsg.q1stWordOfMsg = IPANEL_EVENT_TYPE_NETWORK;
	PingMsg.q2ndWordOfMsg = IPANEL_IP_NETWORK_PING_RESPONSE ;
	PingMsg.q3rdWordOfMsg = 0;
	PingMsg.q4thWordOfMsg = 0 ;

	ip[0] = (uip>>24)&0xff;
	ip[1] = (uip>>16)&0xff;
	ip[2] = (uip>>8)&0xff;
	ip[3] = uip&0xff;

	time1   = NU_Retrieve_Clock();
	status  = NU_Ping( ip, 32,30*TICKS_PER_SECOND);
	
	if( status == NU_TIMEOUT )
	{
	  	ipanel_porting_dprintf("Request timeout.\n");
	}
	else if( status == NU_SUCCESS )
	{
		time = NU_Retrieve_Clock() - time1;
		time = time*1000/TICKS_PER_SECOND;
		if( time<= 5 )
		{	
			PingMsg.q3rdWordOfMsg =1;
			ipanel_porting_dprintf("Reply from %d.%d.%d.%d: bytes=%d time<1ms TTL=%d\n", ip[0],ip[1],ip[2],ip[3],len,IP_TIME_TO_LIVE);
		}
		else
		{	
			PingMsg.q3rdWordOfMsg =time;
			ipanel_porting_dprintf("Reply from %d.%d.%d.%d: bytes=%d time=%dms TTL=%d\n", ip[0],ip[1],ip[2],ip[3],len,time,IP_TIME_TO_LIVE);
		}
	}
	else if( status == NU_HOST_UNREACHABLE )
	{	
		PingMsg.q3rdWordOfMsg =2;
	  	ipanel_porting_dprintf("Destination host unreachable.\n");
	}
	else if( status == NU_ACCESS )
	{	
		PingMsg.q3rdWordOfMsg =3;
	  	ipanel_porting_dprintf("The attempted operation is not allowed on the socket\n");
	}
	else if( status == NU_MSGSIZE )
	{
		PingMsg.q3rdWordOfMsg =4;
	  	ipanel_porting_dprintf("Packet if too large for interface\n");
	}
	else
	{	
		PingMsg.q3rdWordOfMsg = 5;
	  	ipanel_porting_dprintf("Destination host unreachable. No network device\n");
	}
	ipanel_porting_queue_send(g_main_queue, &PingMsg);
	
}

void INTTOCHAR(unsigned int IPAddr,  unsigned char *ipStr)
{
	ipStr[0] = (UINT8)((IPAddr>>24)&0xFF);
	ipStr[1] = (UINT8)((IPAddr>>16)&0xFF);
	ipStr[2] = (UINT8)((IPAddr>>8)&0xFF);
	ipStr[3] = (UINT8)(IPAddr&0xFF);
}

unsigned int CHARTOINT(unsigned char *ipAddr)
{
	union  ORDERBYTE  IPAddr ;

	IPAddr.bytes.byte0  = ipAddr[3];
	IPAddr.bytes.byte1  = ipAddr[2];
	IPAddr.bytes.byte2  = ipAddr[1] ;
	IPAddr.bytes.byte3  = ipAddr[0] ;

	return (IPAddr.dwords) ;
}

/******************************************************************************/
/*	convert IP address format :  "xxx.xxx.xxx.xxx" to int address ...         */
/*  Input      : no                                                           */
/*  Output     : no                                                           */
/*  Return     : no                                     				      */
/******************************************************************************/
unsigned int ipanel_ip_convert(char *str, int len) 
{
	int i ;
	char *p;
	long value ; 

	union 
  	{
  		unsigned   int   val;
  		unsigned   char ch[4];
  	}ipVal = {0} ; 

	p = str ; 
	for(i = 0 ; i<len; i++)
	{
		value = strtol(p,&p,10);
		ipVal.ch[3-i] = (unsigned char)value ;
		p++;
	}

	return ipVal.val ; 
}

/******************************************************************************/
/*Description: get dhcp network parameter setting    */
/*Input      : no                                                             */
/*Output     : no                                                             */
/*Return     : Block handle                                                   */
/*NOTE: DHCP只能使用阻塞方式获取IP地址，需要更改底层驱动，有些麻烦           */
/******************************************************************************/
INT32_T ipanel_get_dhcp_ip()
{
    INT32_T ret = IPANEL_ERR ; 
	unsigned int ts,te;
	STATUS status;
	u_int8 failes = 0;
	u_int8 FailTimes=1;

#ifdef DHCP_ADD_VENDOR_CLASS_ID
	UINT8 dhcp_options[] = {DHCP_REQUEST_LIST, 
		10,
		DHCP_NETMASK,
		DHCP_ROUTE, 
		DHCP_DNS,
		DHCP_VENDOR_CLASS_ID,

		DHCP_CLIENT_CLASS_ID,
		DHCP_IP_LEASE_TIME, 
		DHCP_RENEWAL_T1, 
		DHCP_REBINDING_T2, 
		DHCP_REQUEST_IP, 
		DHCP_MSG_TYPE, 
		
		DHCP_VENDOR_CLASS_ID, 10,
		
		'Z', 'B', 'I','T','V','S','T','B','V','1'
	};
		
	u_int8 pServer_vender_class_id[] = "ZBITVSERV1";
	char *pStr_vender_class_id = (char *)pServer_vender_class_id;
#else
	UINT8 dhcp_options[] = {DHCP_REQUEST_LIST, 
		10,
		DHCP_NETMASK,
		DHCP_ROUTE, 
		DHCP_DNS,

		DHCP_CLIENT_CLASS_ID,
		DHCP_IP_LEASE_TIME, 
		DHCP_RENEWAL_T1, 
		DHCP_REBINDING_T2, 
		DHCP_REQUEST_IP, 
		DHCP_MSG_TYPE, 
		DHCP_VENDOR_CLASS_ID,
	};

	u_int8 pServer_vender_class_id[] = "";
	char *pStr_vender_class_id = (char *)pServer_vender_class_id;
#endif

	ipanel_porting_dprintf("[ipanel_get_dhcp_ip] is called!\n");

	ts = ipanel_porting_time_ms();

	/* set all DHCP fields to zero value */
	if(m_dhcp_ptr != NU_NULL)
	{
		memset(m_dhcp_ptr->dhcp_vclass_id, 0, sizeof(m_dhcp_ptr->dhcp_vclass_id));
	}
	else
	{
		m_dhcp_ptr = (DHCP_STRUCT *)ipanel_porting_malloc(sizeof(NU_DHCP_STRUCT));
		if(m_dhcp_ptr != NU_NULL)
        	memset(m_dhcp_ptr, 0, sizeof(NU_DHCP_STRUCT));	
	}

	if(m_dhcp_ptr == NU_NULL)
	{
		ipanel_porting_dprintf("malloc dhcp pointer failed!\n");
		return IPANEL_ERR;
	}

fails_retry:
	
	m_dhcp_ptr->dhcp_opts = dhcp_options;
	m_dhcp_ptr->dhcp_opts_len = sizeof(dhcp_options);

	NU_DHCP_SetVenderClassID(pServer_vender_class_id, strlen(pStr_vender_class_id));

	status = NU_Dhcp(m_dhcp_ptr, g_ln_name); 

	if(NU_DHCP_REQUEST_FAILED == status)
	{
		ipanel_porting_dprintf("DHCP: Get IP failed\n");
		
		failes++;
		if (failes < FailTimes) /*因为获得ip失败，增加可失败次数2到20,modified by ybc*/
			goto fails_retry;
	}

	if(NU_SUCCESS == status)
	{
		te = ipanel_porting_time_ms();
		
		ipanel_porting_dprintf("[ipanel_get_dhcp_ip] time span = %d ms\n", (te- ts));
   	
		ipanel_porting_dprintf("DHCP success!\n");
		
		ipanel_porting_dprintf("IP address: %d.%d.%d.%d\n", m_dhcp_ptr->dhcp_yiaddr[0], m_dhcp_ptr->dhcp_yiaddr[1],\
				 m_dhcp_ptr->dhcp_yiaddr[2],  m_dhcp_ptr->dhcp_yiaddr[3]);
		ipanel_porting_dprintf("Subnet address: %d.%d.%d.%d\n", m_dhcp_ptr->dhcp_net_mask[0], m_dhcp_ptr->dhcp_net_mask[1],\
				 m_dhcp_ptr->dhcp_net_mask[2],  m_dhcp_ptr->dhcp_net_mask[3]);
		ipanel_porting_dprintf("Gateway address: %d.%d.%d.%d\n", m_dhcp_ptr->dhcp_giaddr[0], m_dhcp_ptr->dhcp_giaddr[1],\
				 m_dhcp_ptr->dhcp_giaddr[2],  m_dhcp_ptr->dhcp_giaddr[3]);
		ipanel_porting_dprintf("Dhcp server address: %d.%d.%d.%d\n", m_dhcp_ptr->dhcp_siaddr[0], m_dhcp_ptr->dhcp_siaddr[1],\
				 m_dhcp_ptr->dhcp_siaddr[2],  m_dhcp_ptr->dhcp_siaddr[3]);
		ipanel_porting_dprintf("Dns Server: %d.%d.%d.%d\n", m_dhcp_ptr->dhcp_yiaddr[0], m_dhcp_ptr->dhcp_yiaddr[1],\
				m_dhcp_ptr->dhcp_yiaddr[2],  m_dhcp_ptr->dhcp_yiaddr[3]);
		ipanel_network_set_dns_server();
		ipanel_porting_dprintf("liaosha of IFCONFIG");
        ret = IPANEL_OK ;
	}

    return ret ; 
}

void ipanel_release_dhcp_ip()
{
	NU_Dhcp_Release(m_dhcp_ptr,g_ln_name);
}

/*****************************************************
   协议栈只能获取一个DNS Server 地址，如果需要多个的话
   需要修改一下底层代码
*****************************************************/
INT32_T ipanel_network_set_dns_server()
{
	int ret,r_dns;
    char buff[16];
	u_int8 dns_addr[4*5]={0};


	ipanel_porting_dprintf("ipanel_network_set_dns_server begain\n");
	memset(m_dns_addr,0x00,sizeof(m_dns_addr));

	ret = NU_Get_DNS_Servers(dns_addr, sizeof(dns_addr));
	if (ret > 0)
	{	
		memcpy(m_dns_addr,dns_addr,4);
		memcpy(buff,m_dns_addr,16*sizeof(char));
		ipanel_porting_dprintf("DNS server Add:%d,%d,%d,%d\n",m_dns_addr[0],m_dns_addr[1],m_dns_addr[2],m_dns_addr[3]);
	}
	
	else
	{
		ipanel_porting_dprintf("liaosha : can't get the DNS server Add\n");
		memset(m_dns_addr, 0, 4);
	}
	
    if(g_dhcp_flag)
    {
    	
        sprintf(buff, "%d.%d.%d.%d",m_dns_addr[0], m_dns_addr[1],\
                m_dns_addr[2],  m_dns_addr[3]);
		ipanel_porting_dprintf("DNS Addr:%d.%d.%d.%d\n",m_dns_addr[0], m_dns_addr[1],\
                m_dns_addr[2],  m_dns_addr[3]);
		
		if(m_dns_mode == 1){
        r_dns=ipanel_set_dns_server(1,buff);
		if(r_dns == 0){
			ipanel_porting_dprintf("ipanel_set_dns_server failed\n");
			return r_dns;
			}
		}
    }
	ipanel_porting_dprintf("ipanel_network_set_dns_server end\n");
    return IPANEL_OK;
}

INT32_T ipanel_network_get_dns_server()
{
	int ret;
	u_int8 dns_addr[4*5]={0};

	memset(m_dns_addr,0x00,sizeof(m_dns_addr));
	
	ret = NU_Get_DNS_Servers(dns_addr, sizeof(dns_addr));
	if (ret > 0)
	{	
		ipanel_porting_dprintf("liaosha got the DNS server IP\n");
		memcpy(m_dns_addr,dns_addr,4);
		ipanel_porting_dprintf("DNS server Add:%d,%d,%d,%d\n",m_dns_addr[0],m_dns_addr[1],m_dns_addr[2],m_dns_addr[3]);
	}
	else
	{
		ipanel_porting_dprintf("liaosha : can't get the DNS server Add\n");
		memset(m_dns_addr, 0, 4);
	}

	return IPANEL_OK;
}

/******************************************************************************/
/*Description: release dhcp network parameter setting                         */
/*Input      : no                                                             */
/*Output     : no                                                             */
/*Return     : no , Block handle                                              */
/******************************************************************************/
INT32_T ipanel_set_network_config(IPANEL_NETWORK_IF_PARAM netParam)
{
	INT32_T ret = IPANEL_ERR;
	INT32_T devStatus;
	BYTE_T dv_ip_addr[4];
	BYTE_T dv_net_mask[4];

	memset(dv_ip_addr,0x00,sizeof(dv_ip_addr));
	memset(dv_net_mask,0x00,sizeof(dv_net_mask));

	// 网关设进去也没什么用，因为设备层没有用它
	INTTOCHAR(netParam.ipaddr , dv_ip_addr);  // 转化IP 地址
	ipanel_porting_dprintf("[ipanel_set_network_config] IP address = %d.%d.%d.%d \n", \
		                   dv_ip_addr[0],dv_ip_addr[1],dv_ip_addr[2],dv_ip_addr[3]) ; 

	INTTOCHAR(netParam.netmask, dv_net_mask); // 转化IP 掩码地址
	ipanel_porting_dprintf("[ipanel_set_network_config] IP net mask  = %d.%d.%d.%d \n", \
		                    dv_net_mask[0],dv_net_mask[1],dv_net_mask[2],dv_net_mask[3]) ; 

	devStatus = DEV_Attach_IP_To_Device(g_ln_name, dv_ip_addr, dv_net_mask);
	if(-1  ==  devStatus )
	{
		ipanel_porting_dprintf("[ipanel_set_network_config] set network config failed! \n");
		return ret ; 
	}

	ret = IPANEL_OK ;

	return ret ;
}

INT32_T  ipanel_get_network_config(IPANEL_NETWORK_IF_PARAM *netParam)
{
	INT32_T ret = IPANEL_ERR ; 
   	DV_DEVICE_ENTRY   *dev;

	dev = DEV_Get_Dev_By_Name(g_ln_name);
	if( dev != NULL) 
	{
		netParam->ipaddr    = dev->dev_addr.dev_ip_addr ;
		netParam->netmask   = dev->dev_addr.dev_netmask;
		netParam->gateway   = (dev->dev_addr.dev_ip_addr&0xffffff00)|0x1;
		ret = IPANEL_OK ;
	}

	ipanel_porting_dprintf("[ipanel_get_network_config] IP address =0x%x ,  IP net mask = 0x%x , \
                            IP getway = 0x%x \n", netParam->ipaddr,netParam->netmask,netParam->gateway );

	return ret ; 
}

/************************************************************************************************
	INT32_T ipanel_porting_network_ioctl(IPANEL_NETWORK_DEVICE_e device, IPANEL_NETWORK_IOCTL_e op, VOID *arg)

	功能说明：
	操作网络接口，包括建立网络连接、断开网络连接、设置和
	获取网络参数等。目前仅支持LAN， Dialup，PPPoE，CableModem四种。

    DHCP及静态获取IP地址逻辑顺序:

    DHCP方式:
        IPANEL_NETWORK_DISCONNECT
        IPANEL_NETWORK_CONNECT
        IPANEL_NETWORK_GET_IFCONFIG

    static方式:
        IPANEL_NETWORK_DISCONNECT
        IPANEL_NETWORK_SET_IFCONFIG
	    IPANEL_NETWORK_LAN_ASSIGN_IP
************************************************************************************************/
INT32_T ipanel_porting_network_ioctl(IPANEL_NETWORK_DEVICE_e device, IPANEL_NETWORK_IOCTL_e op, VOID *arg)
{
	INT32_T ret = IPANEL_OK ;
	UINT32_T  oparg = (UINT32_T)arg;	
	IPANEL_QUEUE_MESSAGE NetworkMsg;

	NetworkMsg.q1stWordOfMsg = IPANEL_EVENT_TYPE_NETWORK;
	NetworkMsg.q2ndWordOfMsg = 0;
	NetworkMsg.q3rdWordOfMsg = 0;
	NetworkMsg.q4thWordOfMsg = 0 ;

    ipanel_porting_dprintf("[ipanel_porting_network_ioctl] device = %d,op=%d.\n",device,op);

	switch(device)
	{
		case IPANEL_NETWORK_DEVICE_LAN:	// LAN
			switch(op)
			{
				case IPANEL_NETWORK_CONNECT:
					switch(oparg)
					{
						case IPANEL_NETWORK_LAN_DHCP:
							if(!g_dhcp_flag)
							{
								g_dhcp_flag = 1 ; 
								ret = ipanel_get_dhcp_ip();
                                if( IPANEL_OK == ret )
                                {
                                    NetworkMsg.q2ndWordOfMsg = IPANEL_IP_NETWORK_READY ;
    				                ipanel_porting_dprintf("DHCP: [IPANEL_IP_NETWORK_READY] send success message!\n");
       								g_dhcp_flag = 1 ; 
                                }
                                else
                                {
                                    NetworkMsg.q2ndWordOfMsg = IPANEL_IP_NETWORK_FAILED ;
    				                ipanel_porting_dprintf("DHCP: [IPANEL_IP_NETWORK_FAILED] send failed message!\n");
                                }
                                ipanel_porting_queue_send(g_main_queue, &NetworkMsg);

    				            ipanel_porting_dprintf("DHCP: [IPANEL_NETWORK_LAN_DHCP]is called!\n");
							}

							break;

						case IPANEL_NETWORK_LAN_ASSIGN_IP:
    				        ipanel_porting_dprintf("STATIC: [IPANEL_NETWORK_LAN_ASSIGN_IP]is called!\n");
							break; 

						default:
							break;
					}
					break; 

				case IPANEL_NETWORK_DISCONNECT:
    				if( g_dhcp_flag)
    				{
    				    ipanel_porting_dprintf("DHCP: [IPANEL_NETWORK_DISCONNECT]is called!\n");
    					ipanel_release_dhcp_ip();
    					g_dhcp_flag = 0 ; 
    				}

  				    ipanel_porting_dprintf("[IPANEL_NETWORK_DISCONNECT]is called!\n");

					break; 

				case IPANEL_NETWORK_SET_IFCONFIG:
					ret = ipanel_set_network_config(*((IPANEL_NETWORK_IF_PARAM*)arg));
                    if( IPANEL_OK == ret )
                    {
    				    ipanel_porting_dprintf("STATIC: [IPANEL_IP_NETWORK_READY] send success message!\n");
                        NetworkMsg.q2ndWordOfMsg = IPANEL_IP_NETWORK_READY ;
                    }
                    else
                    {
    				    ipanel_porting_dprintf("STATIC: [IPANEL_IP_NETWORK_FAILED] send failed message!\n");
                        NetworkMsg.q2ndWordOfMsg = IPANEL_IP_NETWORK_FAILED ;
                    }
                    ipanel_porting_queue_send(g_main_queue, &NetworkMsg);
                    
					break; 

				case IPANEL_NETWORK_GET_IFCONFIG:
					ret = ipanel_get_network_config((IPANEL_NETWORK_IF_PARAM*)arg);
					break ; 

				case IPANEL_NETWORK_GET_STATUS: // 获取网线连接状态
                    ret = get_net_link_status(); 
                    switch (ret)
                    {
                        case 0:
                            *((int32 *)arg) = IPANEL_NETWORK_IF_DISCONNECTED;
							ipanel_porting_dprintf("liaosha:IPANEL_NETWORK_IF_DISCONNECTED\n");
                            break;
                        case 1:
                            *((int32 *)arg) = IPANEL_NETWORK_IF_CONNECTED;
							ipanel_porting_dprintf("liaosha:IPANEL_NETWORK_IF_CONNECTED\n");
                            break;
                        default:
                            *((int32 *)arg) = IPANEL_NETWORK_IF_CONNECTED;
							ipanel_porting_dprintf("liaosha:IPANEL_NETWORK_IF_CONNECTING\n");
                            break;
                    } 
                    break;
					break; 

				case IPANEL_NETWORK_SET_STREAM_HOOK:  //设置HOOK函数
					break;

				case IPANEL_NETWORK_DEL_STREAM_HOOK:  //删除指定的HOOK
					break;

				case IPANEL_NETWORK_SET_USER_NAME:
					break;

				case IPANEL_NETWORK_SET_PWD:
					break;

				case IPANEL_NETWORK_SET_DIALSTRING:
					break; 

                case IPANEL_NETWORK_SET_DNS_CONFIG:
					ipanel_porting_dprintf("liaosha IPANEL_NETWORK_SET_DNS_CONFIG,oparg=%d\n",oparg);
                    switch(oparg)
                    {
                        case IPANEL_NETWORK_DNS_FROM_SERVER:
							m_dns_mode = 1;
                            ret = ipanel_network_set_dns_server();
                            break;

                        case IPANEL_NETWORK_DNS_FROM_USER:
							//NU_Add_DNS_Server (UINT8 *new_dns_server, INT where)
							m_dns_mode = 0;
							ret = ipanel_network_set_dns_server();
                            break;

                        default:
                            break;
                    }
                    break;

                case IPANEL_NETWORK_GET_DNS_CONFIG:
                    break; 

                case IPANEL_NETWORK_SET_NIC_MODE:
					ipanel_porting_dprintf("liaosha IPANEL_NETWORK_SET_NIC_MODE\n");
                    break;

                case IPANEL_NETWORK_GET_NIC_MODE:
                    *((int32 *)arg) = IPANEL_NETWORK_100BASE_HALF_DUMPLEX;
					ipanel_porting_dprintf("IPANEL_NETWORK_GET_NIC_MODE IPANEL_NETWORK_100BASE_HALF_DUMPLEX\n");
                    break;                    

				default:
					ipanel_porting_dprintf("[ipanel_porting_network_ioctl] ERROR : param error . \n");
					ret = IPANEL_ERR ;
					break;
				}

			break; 

		case IPANEL_NETWORK_DEVICE_DIALUP:	 // Dialup
			break;

		case IPANEL_NETWORK_DEVICE_PPPOE:	// PPPoE
			break; 

		case IPANEL_NETWORK_DEVICE_CABLEMODEM:   // CableModem
			break; 

		default:
			ipanel_porting_dprintf("[ipanel_porting_network_ioctl] ERROR : param error . \n");
			ret = IPANEL_ERR ; 
			break;
	}
	
	return ret ; 
}

void ipanel_network_dhcpget()
{
	IPANEL_NETWORK_IF_PARAM arg;

	// DHCP动态IP地址测试
	ipanel_porting_dprintf("[network_test]    ---------------- dhcp  -----------\n");
	ipanel_porting_network_ioctl(IPANEL_NETWORK_DEVICE_LAN, IPANEL_NETWORK_CONNECT, (VOID *)IPANEL_NETWORK_LAN_DHCP);

	ipanel_porting_dprintf("[network_test]    ---------------- get ipaddr   -----------\n");
	ipanel_porting_network_ioctl(IPANEL_NETWORK_DEVICE_LAN, IPANEL_NETWORK_GET_IFCONFIG, (void *)&arg);

	ipanel_porting_dprintf("[network_test] ip  = 0x%x   mask = 0x%x  gw = 0x%x \n", 
						   arg.ipaddr, arg.netmask, arg.gateway);

	ipanel_porting_dprintf("[network_test]    ---------------- ping dhcp ip addr  -----------\n");
	ipanel_ping_ip(arg.ipaddr, 32);
	ipanel_network_get_dns_server();
}

void ipanel_network_ipset()
{

	IPANEL_NETWORK_IF_PARAM arg;
	
	// 静态IP地址测试
	//arg.ipaddr  = 0xC0A81B03;
	//arg.gateway = 0xC0A81B01;
	//arg.netmask = 0xffffff00;

	// IP :10.240.0.144 MASK: 255.255.255.0 
	arg.ipaddr  = 0x0AF00390;
	arg.gateway = 0x0AF00381;
	arg.netmask = 0xffffff80;

	ipanel_porting_network_ioctl(IPANEL_NETWORK_DEVICE_LAN, IPANEL_NETWORK_SET_IFCONFIG, (void *)&arg);

	ipanel_porting_dprintf("[network_test]   2 ---------------- get ipaddr   -----------\n");
	ipanel_porting_network_ioctl(IPANEL_NETWORK_DEVICE_LAN, IPANEL_NETWORK_GET_IFCONFIG, (void *)&arg);

	ipanel_porting_dprintf("[network_test] ip  = 0x%x   mask = 0x%x  gw = 0x%x \n", arg.ipaddr, arg.netmask, arg.gateway);

	ipanel_porting_dprintf("[network_test]  2  ---------------- ping static ip addr  -----------\n");
	ipanel_ping_ip(arg.ipaddr, 32);
	ipanel_network_get_dns_server();
}

static int ipanel_network_get_status()
{
    int ret;
    
    ret = get_net_link_status(); 
    if(LINK_OFF == ret )
        return 0;
    else
        return 1;
}

static VOID ipanel_network_check_process(VOID *param)
{
	int cur_cable_status=-1, pre_cable_status=-1, cable_off_count=0;
	IPANEL_QUEUE_MESSAGE NetworkMsg;

	NetworkMsg.q1stWordOfMsg = IPANEL_EVENT_TYPE_NETWORK;
	NetworkMsg.q2ndWordOfMsg = 0;
	NetworkMsg.q3rdWordOfMsg = 0;
	NetworkMsg.q4thWordOfMsg = 0 ;

    while(1)
    {
		/*检测网线连接情况并发对应消息给MiddleWare*/
        cur_cable_status = ipanel_network_get_status();
		if (cur_cable_status != pre_cable_status)
		{
			/*detect cable off 3 times*/
			if ((cur_cable_status==0) && (cable_off_count++>3))
			{
				cable_off_count = 0;
				pre_cable_status = cur_cable_status;
                
				ipanel_porting_dprintf("[detect_task] CABLE is %s\n", cur_cable_status?"ON":"OFF");

                if(cur_cable_status == 1)
                {
                    NetworkMsg.q2ndWordOfMsg = EIS_IP_NETWORK_CONNECT;
                }
                else
                {
                    NetworkMsg.q2ndWordOfMsg = EIS_IP_NETWORK_DISCONNECT;
                }

                ipanel_porting_queue_send(g_main_queue, &NetworkMsg);
			}
            
			if (cur_cable_status)
			{
				pre_cable_status = cur_cable_status;
                
				ipanel_porting_dprintf("[detect_task] CABLE is %s\n", cur_cable_status?"ON":"OFF");

                if(cur_cable_status == 1)
                {
                    NetworkMsg.q2ndWordOfMsg = EIS_IP_NETWORK_CONNECT;
                }
                else
                {
                    NetworkMsg.q2ndWordOfMsg = EIS_IP_NETWORK_DISCONNECT;
                }

                ipanel_porting_queue_send(g_main_queue, &NetworkMsg);
			}
		}
        
        ipanel_porting_task_sleep(IPANEL_NETWORK_CHECK_TIME);
    }
}

INT32_T ipanel_network_init()
{
    m_netcheck_task_id = ipanel_porting_task_create(
                            IPANEL_NETCHECK_TASK_NAME,
                            ipanel_network_check_process,
                            (VOID*)NULL,
                            IPANEL_NETCHECK_TASK_PRIORITY,
                            IPANEL_NETCHECK_TASK_STACK_SIZE);
    if(IPANEL_NULL == m_netcheck_task_id)
    {
        ipanel_porting_dprintf("[ipanel_network_init] create task failed!\n");
        return IPANEL_ERR;   
    }   

    return IPANEL_OK;
}

void ipanel_network_exit()
{
    if(m_netcheck_task_id)
    {
        ipanel_porting_task_destroy(m_netcheck_task_id);
        m_netcheck_task_id = IPANEL_NULL;
    }
}