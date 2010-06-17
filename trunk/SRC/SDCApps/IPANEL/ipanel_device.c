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
#include "time.h"
#include "ipanel_base.h"
#include "ipanel_device.h"

extern u_int16 ee_read(u_int16 address, voidF buffer, u_int16 count, void* private);
extern u_int16 ee_write(u_int16 address, voidF buffer, u_int16 count, void* private);

//--------------------------------------------------------------------------------------------------
// Global data
//--------------------------------------------------------------------------------------------------
//

NU_MEMORY_POOL          net_mem_pool;
extern NU_MEMORY_POOL   gSystemMemory;
#define System_Memory   gSystemMemory
#define ETHNET_PROT_MEM_POOL_SIZE	(400*1024)

#define EEPROM_ADDR_MAC	0x7B00
#define MAC_ADDR_LEN	6

u_int8  m_stb_mac[6] = {0};

u_int8 LoadNetSeting()
{	
	memset(m_stb_mac,0x00,MAC_ADDR_LEN);
	
	// MAC 地址存储在EEPROM 中，单独读出:
	ee_read(EEPROM_ADDR_MAC, m_stb_mac, MAC_ADDR_LEN, NULL);
	
	ipanel_porting_dprintf("mac: %02x %02x %02x %02x %02x %02x \n", 
		m_stb_mac[0], m_stb_mac[1], m_stb_mac[2], m_stb_mac[3], m_stb_mac[4], m_stb_mac[5]);
	
	if (m_stb_mac[0]==0xFF&&m_stb_mac[1]==0xFF&&m_stb_mac[2]==0xFF
		&&m_stb_mac[3]==0xFF&&m_stb_mac[4]==0xFF&&m_stb_mac[5]==0xFF)
	{
		m_stb_mac[0]=0x00;
		m_stb_mac[1]=0x80;
		m_stb_mac[2]=0x0f;
		m_stb_mac[3]=0x11;
		m_stb_mac[4]=0x80;
		m_stb_mac[5]=0x31;

		ee_write(EEPROM_ADDR_MAC,m_stb_mac, MAC_ADDR_LEN, NULL);
	}
	
	return 1;
}

int set_net_mac(u_int8 *pMac)
{
	DV_DEVICE_ENTRY *dev;
	DV_REQ req;
	
	dev=DEV_Get_Dev_By_Name("ethernet"); 
	 
	req.dvr_dvru.dvru_data = pMac;

#if(ETH_CHIPSET == SMSC)
	return  smsc_ioctl(dev,DEV_SET_MAC,&req);
#elif(ETH_CHIPSET == DAVICOM9000A)
	return  dm9ks_ioctl(dev,DEV_SET_MAC,&req);
#endif
}

int set_net_ip(u_int8 *pIP_addr)
{
	DV_DEVICE_ENTRY *dev;
	DV_REQ req;
	
	dev=DEV_Get_Dev_By_Name("ethernet"); 
	 
	req.dvr_dvru.dvru_data = pIP_addr;

#if(ETH_CHIPSET == SMSC)
	smsc_ioctl(dev,DEV_SET_IP,&req);
#elif(ETH_CHIPSET == DAVICOM9000A)
	dm9ks_ioctl(dev,DEV_SET_IP,&req);
#endif

	return IPANEL_OK;
}

int set_net_gw(u_int8 *pGW)
{
	return IPANEL_OK;
}

int set_net_mask(u_int8 *pMask)
{
	DV_DEVICE_ENTRY *dev;
	DV_REQ req;
	
	dev=DEV_Get_Dev_By_Name("ethernet"); 
	 
	req.dvr_dvru.dvru_data = pMask;

#if(ETH_CHIPSET == SMSC)
	smsc_ioctl(dev,DEV_SET_MASK,&req);	
#elif(ETH_CHIPSET == DAVICOM9000A)
	dm9ks_ioctl(dev,DEV_SET_MASK,&req);	
#endif

	return IPANEL_OK;
}

bool get_net_mac(u_int8 *pMac)
{
	DV_REQ req;
	DV_DEVICE_ENTRY *dev;

   	dev = DEV_Get_Dev_By_Name("ethernet");
	
   	req.dvr_dvru.dvru_data = pMac;
	
#if(ETH_CHIPSET == SMSC)
	smsc_ioctl(dev,DEV_GET_MAC,&req);
#elif(ETH_CHIPSET == DAVICOM9000A)
	dm9ks_ioctl(dev,DEV_GET_MAC,&req);
#endif

	return TRUE;		
}

bool get_net_ip(u_int8 *pIP_addr)
{
	DV_REQ req;
	DV_DEVICE_ENTRY *dev;

   	dev = DEV_Get_Dev_By_Name("ethernet");
	
   	req.dvr_dvru.dvru_data = pIP_addr;

#if(ETH_CHIPSET == SMSC)
	smsc_ioctl(dev,DEV_GET_IP,&req);
#elif(ETH_CHIPSET == DAVICOM9000A)
	dm9ks_ioctl(dev,DEV_GET_IP,&req);
#endif

	return TRUE;	
}

bool get_net_getway(u_int8 *pGW)
{
	return TRUE;
}

bool get_net_mask(u_int8 *pMask)
{
	DV_REQ req;
	DV_DEVICE_ENTRY *dev;

   	dev = DEV_Get_Dev_By_Name("ethernet");
	
   	req.dvr_dvru.dvru_data = pMask;

#if(ETH_CHIPSET == SMSC)
	smsc_ioctl(dev,DEV_GET_MASK,&req);	
#elif(ETH_CHIPSET == DAVICOM9000A)
	dm9ks_ioctl(dev,DEV_GET_MASK,&req);
#endif

	return TRUE;
}

int get_net_link_status()
{
	DV_DEVICE_ENTRY *dev;
	DV_REQ req;
	
	dev=DEV_Get_Dev_By_Name("ethernet"); 

	if (dev)
	{
#if(ETH_CHIPSET == SMSC)
		if (smsc_ioctl(dev,DEV_GET_LINK_STATE,&req) >= 0)
#elif(ETH_CHIPSET == DAVICOM9000A)
		if (dm9ks_ioctl(dev,DEV_GET_LINK_STATE,&req) >= 0)
#endif
		return req.dvr_dvru.drvu_flags;
	}
	return LINK_OFF;
}

int ipanel_network_set_ioctl(IPANEL_NET_OP op,IPANEL_NETWORK_IF_PARAM *arg)
{
	int ret = IPANEL_OK;
	u_int8 netValue[4];

	switch(op)
	{
		case IPANEL_NET_NONE:
			ipanel_porting_dprintf("[ipanel_network_set_ioctl] Error network param!\n");
			break;

		case IPANEL_NET_IP:
			INTTOCHAR(arg->ipaddr,netValue);
			set_net_ip(netValue);
			break;

		case IPANEL_NET_GW:
			INTTOCHAR(arg->gateway,netValue);
			set_net_gw(netValue);
			break;

		case IPANEL_NET_MASK:
			INTTOCHAR(arg->netmask,netValue);
			set_net_mask(netValue);
			break;

		case IPANEL_NET_MAC:
			set_net_mac(m_stb_mac);
			break;

		case IPANEL_NET_ALL:		
			INTTOCHAR(arg->ipaddr,netValue);
			set_net_ip(netValue);

			INTTOCHAR(arg->gateway,netValue);
			set_net_gw(netValue);
			
			INTTOCHAR(arg->netmask,netValue);
			set_net_ip(netValue);	
			
			break;
	}
	
	return ret;
}

int ipanel_network_get_ioctl(IPANEL_NET_OP op,IPANEL_NETWORK_IF_PARAM *arg)
{
	int ret = IPANEL_OK;
	u_int8 netValue[4]={0};

	switch(op)
	{
		case IPANEL_NET_NONE:
			ipanel_porting_dprintf("[ipanel_network_get_ioctl] Error network param!\n");
			break;

		case IPANEL_NET_IP:
			get_net_ip(netValue);
			arg->ipaddr = CHARTOINT(netValue);
			break;

		case IPANEL_NET_GW:
			get_net_getway(netValue);
			arg->gateway = CHARTOINT(netValue);
			break;

		case IPANEL_NET_MASK:
			get_net_mask(netValue);
			arg->netmask = CHARTOINT(netValue);
			break;

		case IPANEL_NET_MAC:
			get_net_mac(m_stb_mac);
			break;

		case IPANEL_NET_ALL:			
			get_net_ip(netValue);
			arg->ipaddr = CHARTOINT(netValue);

			get_net_getway(netValue);
			arg->gateway = CHARTOINT(netValue);
			
			get_net_mask(netValue);
			arg->netmask = CHARTOINT(netValue);
			
			break;
	}
	
	return ret;
}

/*************************************************************
	功能: 外部使用API 函数
	             初始化整个TCP&IP 协议栈内存及结构
	             初始化网络驱动代码正常工作
*************************************************************/
int cnxt_nuptst_init( unsigned int ip,unsigned int subnet_mask,unsigned int gateway )
{
	void *pointer;
	UINT8 p1[4]={0};
	UINT8 p2[4]={0};

	p2[0]=(UINT8)((gateway>>24)&0xFF);
	p2[1]=(UINT8)((gateway>>16)&0xFF);
	p2[2]=(UINT8)((gateway>>8)&0xFF);
	p2[3]=(UINT8)((gateway)&0xFF);

	if ( NU_Allocate_Memory(&System_Memory, &pointer,
	                           ETHNET_PROT_MEM_POOL_SIZE, (u_int32)NU_NO_SUSPEND) )
   	{
      	ipanel_porting_dprintf("No enough memory for NUPNET!!\n") ;	
   	}
      
   	if ( NU_Create_Memory_Pool(&net_mem_pool, "net_mem",
                              pointer, ETHNET_PROT_MEM_POOL_SIZE, 4, NU_PRIORITY) )
   	{
      	ipanel_porting_dprintf("No enough memory for NUPNET!!\n") ;	
   	}

   	if(0 != NU_Init_Net(&net_mem_pool))
   	{
        ipanel_porting_dprintf("NU_Init_Net failed \n");
    }

	// 获取MAC地址，失败的话设定固定MAC地址
	LoadNetSeting();

	// 初始化逻辑网络设备及物理网卡设备
#if(ETH_CHIPSET == SMSC)//add by wlk,2006.11.29
	eth_init(smsc_init,ip,subnet_mask, gateway);
#elif(ETH_CHIPSET == DAVICOM9000A)//add by wlk,2006.11.29
	eth_init(dm9ks_init,ip,subnet_mask, gateway);
#endif

	set_net_mac(m_stb_mac);

	NU_Add_Route(p1,p1,p2);

   	return IPANEL_OK;
}

INT32_T	ipanel_porting_device_read(CONST CHAR_T *params, 
								   CHAR_T *buf, INT32_T length)
{
	return IPANEL_OK;
}

INT32_T ipanel_porting_device_write(CONST CHAR_T *params, 
		 							CHAR_T *buf, INT32_T length)
{
	return IPANEL_OK;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/

#define _DAY_SEC           (24L * 60L * 60L)    /* secs in a day */

#define _YEAR_SEC          (365L * _DAY_SEC)    /* secs in a year */

#define _FOUR_YEAR_SEC     (1461L * _DAY_SEC)   /* secs in a 4 year interval */

#define _DEC_SEC           315532800L           /* secs in 1970-1979 */

#define _BASE_YEAR         70L                  /* 1970 is the base year */

#define _BASE_DOW          4                    /* 01-01-70 was a Thursday */

#define _LEAP_YEAR_ADJUST  17L                  /* Leap years 1900 - 1970 */

#define _MAX_YEAR          138L                 /* 2038 is the max year */

static int _lpdays[] = 
{
	-1, 30, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365
};

static int _days[] = 
{
	-1, 30, 58, 89, 119, 150, 180, 211, 242, 272, 303, 333, 364
};


// 
// gmtime 没有实现，此函数拷贝自 vc6.0 
//
static struct tm * my_gmtime( const time_t *timp, struct tm *ptb )
{
    long caltim = *timp;            /* calendar time to convert */
    int islpyr = 0;                 /* is-current-year-a-leap-year flag */
    int tmptim;
    int *mdays;						/* pointer to days or lpdays */

    if ( caltim < 0L )
	{
		return(NULL);
	}

    /*
     * Determine years since 1970. First, identify the four-year interval
     * since this makes handling leap-years easy (note that 2000 IS a
     * leap year and 2100 is out-of-range).
     */
    tmptim = (int)(caltim / _FOUR_YEAR_SEC);
    caltim -= ((long)tmptim * _FOUR_YEAR_SEC);

    /*
     * Determine which year of the interval
     */
    tmptim = (tmptim * 4) + 70;         /* 1970, 1974, 1978,...,etc. */

    if ( caltim >= _YEAR_SEC ) 
	{
        tmptim++;                       /* 1971, 1975, 1979,...,etc. */
        caltim -= _YEAR_SEC;

        if ( caltim >= _YEAR_SEC ) 
		{
            tmptim++;                   /* 1972, 1976, 1980,...,etc. */
            caltim -= _YEAR_SEC;

            /*
             * Note, it takes 366 days-worth of seconds to get past a leap
             * year.
             */
            if ( caltim >= (_YEAR_SEC + _DAY_SEC) ) 
			{
                    tmptim++;           /* 1973, 1977, 1981,...,etc. */
                    caltim -= (_YEAR_SEC + _DAY_SEC);
            }
            else 
			{
                    /*
                     * In a leap year after all, set the flag.
                     */
                    islpyr++;
            }
        }
    }

    /*
     * tmptim now holds the value for tm_year. caltim now holds the
     * number of elapsed seconds since the beginning of that year.
     */
    ptb->tm_year = tmptim;

    /*
     * Determine days since January 1 (0 - 365). This is the tm_yday value.
     * Leave caltim with number of elapsed seconds in that day.
     */
    ptb->tm_yday = (int)(caltim / _DAY_SEC);
    caltim -= (long)(ptb->tm_yday) * _DAY_SEC;

    /*
     * Determine months since January (0 - 11) and day of month (1 - 31)
     */
    if ( islpyr )
        mdays = _lpdays;
    else
        mdays = _days;


    for ( tmptim = 1 ; mdays[tmptim] < ptb->tm_yday ; tmptim++ ) ;

    ptb->tm_mon = --tmptim;

    ptb->tm_mday = ptb->tm_yday - mdays[tmptim];

    /*
     * Determine days since Sunday (0 - 6)
     */
    ptb->tm_wday = ((int)(*timp / _DAY_SEC) + _BASE_DOW) % 7;

    /*
     *  Determine hours since midnight (0 - 23), minutes after the hour
     *  (0 - 59), and seconds after the minute (0 - 59).
     */
    ptb->tm_hour = (int)(caltim / 3600);
    caltim -= (long)ptb->tm_hour * 3600L;

    ptb->tm_min = (int)(caltim / 60);
    ptb->tm_sec = (int)(caltim - (ptb->tm_min) * 60);

    ptb->tm_isdst = 0;
    return( (struct tm *)ptb );

}

#define EEPROM_ADDR_SERIALNUMBER	0x7B20
#define SERIALNUMBER_LEN			4

static u_int32 EEPROM_LoadSerialNumber()
{
	u_int32 uSN = 0;
	ee_read( EEPROM_ADDR_SERIALNUMBER, &uSN, SERIALNUMBER_LEN, NULL );
		
	return uSN;
}

#define EEPROM_ADDR_PRODUCEDATE	0x7B24
#define PRODUCEDATE_LEN			4

static u_int32 EEPROM_LoadProduceDate()
{
	u_int32 uDate = 0;
	ee_read( EEPROM_ADDR_PRODUCEDATE, &uDate, PRODUCEDATE_LEN, NULL );
		
	return uDate;
}

//
// 得到内部序列号
// 此序列号为16进制，共4字节；即 0x00000000 – 0xFFFFFFFF
//
INT32_T	GetInnerSerialNumber( char* pBuffer, int nSize )
{		
	u_int32 uSN = EEPROM_LoadSerialNumber();
	sprintf( pBuffer, "0x%08lX", uSN );

    return IPANEL_OK;
}

//
// 得到外部序列号
// 此序列号为十进制共19位，从左向右，左位为最高位, 内置序列号和外置序列号的最后七位十进制数值相等
//
struct tm * my_gmtime ( const time_t *timp, struct tm *ptb );

INT32_T GetOuterSerialNumber( char* pBuffer, int nSize )
{
	u_int32 uSN = 0;
	time_t  tmDate = 0;
	struct tm *ptm = NULL;
	struct tm tmBuff;

	char	strTemp[30] = { 0 };
	int		nLength = 0;
	
	if( !pBuffer && nSize < 20 )
	{		
	    ipanel_porting_dprintf("[GetOuterSerialNumber] len(>=20)=%d.\n",nSize);
		return IPANEL_ERR;	
	}

	// 序列号
	uSN = EEPROM_LoadSerialNumber();

	nLength = sprintf( strTemp, "%07lu", uSN );
	nLength -= 7;


	// 生产日期
	tmDate = EEPROM_LoadProduceDate();
	ptm = my_gmtime ( &tmDate, &tmBuff );

	if( ptm == NULL )
	{
		sprintf( pBuffer, "062100020700%s", strTemp+nLength );
	}
	else
	{
		char strDate[11] = { 0 };
		strftime( strDate, 10, "%y%W", ptm );

		sprintf( pBuffer, "06210002%s%s", strDate, strTemp+nLength );
	}

    return IPANEL_OK;    
}

