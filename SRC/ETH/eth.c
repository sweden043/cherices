#include "basetype.h"
#include "kal.h"

#include "..\nupnet\target.h"
#include "..\nupnet\dev.h"
#include "..\nupnet\net.h"
#include "..\nupnet\mem_defs.h"
#include "..\nupnet\externs.h"
#include "eth.h"

CHAR   eth_device_name[] = "ethernet";
u_int8 mac_addr[6]={0x00,0x11,0x22,0x33,0x44,0x55};

static void str2ip(char *str, u_int32 *ipaddr)
{
	char *p=str;
	char ip[4][3]={0};
	int32 digi_num=0;
	int32 ip_seg=0;
	int32 j;
	int32 tmp=0;

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

	for (ip_seg=0;ip_seg<4;ip_seg++)
	{
		for (digi_num=0,j=100;digi_num<3;digi_num++,j/=10)
				tmp+=((ip[ip_seg][digi_num]-'0')*j);
		*ipaddr|=tmp<<(24-ip_seg*8);
		tmp=0;
	}	
	*ipaddr=*ipaddr;	

}

void eth_console(u_int8 *macaddr,u_int32 *ipaddr,u_int32 *subnet,u_int32 *gateway )
{
	char str[128];
	//int32 i=0,j=0,k=0;
	//u_int8 ip[4];
	//bool bOldStatus;
	
	trace ("-----------Ethernet Configuration --------------- \n");
	trace ("IP Address: xxx.xxx.xxx.xxx \n");
	gets(str);

	str2ip(str,ipaddr);

	trace ("IP Subnet Mask: xxx.xxx.xxx.xxx \n");
	gets(str);
	str2ip(str,subnet);
	
	trace ("IP Gateway: xxx.xxx.xxx.xxx \n");
	gets(str);
	str2ip(str,gateway);	

}



STATUS eth_init(STATUS (*dev_init) (DV_DEVICE_ENTRY *), u_int32 ipaddr,u_int32 subnet_mask,u_int32 gateway)
{

	NU_DEVICE                   eth_device;
	STATUS result;
 	//u_int8                        null_ip[4]={0};
	eth_device.dv_ip_addr[0]       = (ipaddr>>24)&0xFF;
	eth_device.dv_ip_addr[1]       = (ipaddr>>16)&0xFF;
	eth_device.dv_ip_addr[2]       = (ipaddr>>8)&0xFF;
	eth_device.dv_ip_addr[3]       = (ipaddr&0xFF);

	eth_device.dv_subnet_mask[0]   = (subnet_mask>>24)&0xFF;
	eth_device.dv_subnet_mask[1]   = (subnet_mask>>16)&0xFF;
	eth_device.dv_subnet_mask[2]   = (subnet_mask>>8)&0xFF;
	eth_device.dv_subnet_mask[3]   = (subnet_mask&0xFF);

	eth_device.dv_gw[0]   = (gateway>>24)&0xFF;
	eth_device.dv_gw[1]   = (gateway>>16)&0xFF;
	eth_device.dv_gw[2]   = (gateway>>8)&0xFF;
	eth_device.dv_gw[3]   = (gateway&0xFF);
	if((gateway>>24)&0xFF== 0)
	{
	eth_device.dv_gw[0]   = eth_device.dv_ip_addr[0] & eth_device.dv_subnet_mask[0] ;
	eth_device.dv_gw[1]   = eth_device.dv_ip_addr[1] & eth_device.dv_subnet_mask[1] ;
	eth_device.dv_gw[2]   = eth_device.dv_ip_addr[2] & eth_device.dv_subnet_mask[2] ;
	eth_device.dv_gw[3]   = 1;
	}
	
	eth_device.dv_name          = eth_device_name;
	eth_device.dv_flags            = DV_MULTICAST|DV_UP|DV_PROMISC;
	eth_device.dv_driver_options   = NU_NULL;
	eth_device.dv_init             = dev_init;

	eth_device.dv_hw.ether.dv_irq          = 0;
	eth_device.dv_hw.ether.dv_io_addr      = 0;
	eth_device.dv_hw.ether.dv_shared_addr  = NU_NULL;

	/* Add the device to the system. */
	result = DEV_Init_Devices (&eth_device, 1);

	//if(eth_device.dv_gw[0]!=0)
	//	NU_Add_Route(null_ip,null_ip, eth_device.dv_gw);

	trace("Static IP address :  %d.%d.%d.%d \n",
				(ipaddr>>24)&0xFF,
				(ipaddr>>16)&0xFF,
				(ipaddr>>8)&0xFF,
				(ipaddr>>0)&0xFF);
	trace("Subnet Mask       :  %d.%d.%d.%d \n",
				(subnet_mask>>24)&0xFF,
				(subnet_mask>>16)&0xFF,
				(subnet_mask>>8)&0xFF,
				(subnet_mask>>0)&0xFF);
	trace("GateWay             :  %d.%d.%d.%d \n",
				(gateway>>24)&0xFF,
				(gateway>>16)&0xFF,
				(gateway>>8)&0xFF,
				(gateway>>0)&0xFF);
	return result;
}

