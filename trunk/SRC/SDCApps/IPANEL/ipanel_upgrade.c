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

杭州升级逻辑说明：
1、ipanel监控到有升级要求，解析NIT表获取升级描述符Linkage_Descriptior；
2、ipanel将解析出的描述符通过ipanel_upgrade_check函数传递给集成方检测升级逻辑；
3、集成方检测是否是自己的描述符，如果是则根据升级检测规则返回是否升级及升级方式；
4、ipanel根据返回值决定是否通知用户选择升级；
5、如果要执行升级，ipanel将调用ipanel_upgrade_start接口执行升级动作；
6、在ipanel_upgrade_start接口中，集成方调用ipanel提供的ipanel_upgrade_getparams接口获取升级入口参数。
7、集成方将参数存贮到flash中，并启动loader升级。

iPanel MiddleWare的升级功能有两种触发方式：
1、	iPanel MiddleWare检测到有新的软件版本
2、	用户通过界面手工触发

*********************************************************************/
#include "ipanel_config.h"
#include "ipanel_base.h"
#include "ipanel_crc.h"
#include "ipanel_eeprom.h"
#include "ipanel_nvram.h"
#include "ipanel_upgrade.h"

#define DEBUG_UPGRADE
#define daya_crc

static unsigned char m_sub_des_num =0;
static u_int64 nvram_serial_number;
static u_int64 start_serial,end_serial;

static CodeDownload_Description m_stb_param;
u_int8 soft_ver_flash[2];
void crc_2dot_test();

unsigned short CRC16_16(unsigned char * pvStartAddress, unsigned long ulSize_in_bytes)
{
	unsigned char *pbData = pvStartAddress;
	unsigned short ulCRC=0;
   while (ulSize_in_bytes--)
   {
      ulCRC = (((ulCRC >> 8) & 0xFF) ^ awTable_CRC16[((ulCRC ^*pbData++) & 0xFF)]);
   	}
   return ulCRC;
}
void Int2longlong(u_int8 *buf,u_int8 len,u_int64 *r)
{
     u_int8 i,j;
     *r=0;
     for(i=0,j=len-1;i<len;i++,j--)
		*r|=(((u_int64)(buf[i]))<<(j*8));
}


char  ld_nvm_write(unsigned char* buffer, unsigned int offset, unsigned int size)
{
   unsigned int Num_Bytes_Actually_Read;
   unsigned int	addr;
   
   trace("Loader ld_nvm_write\n");
   
   if((offset+size) > 0x7fff)
   	return -1;
   
   addr = offset;
   
   Num_Bytes_Actually_Read = ee_write(addr, buffer, size, (void*)NULL);
   if(Num_Bytes_Actually_Read != size)
       return -1;
   else
       return 0;
}


/*********************************************************************
 * NVM loader partition data:
 *
 * Syntax								Num of bits		Identifier
 * download_info_block{
 *    update_flag						8				uimsf
 *    update_aborted					8				uimsf
 *    crc16_1							16				uimsf
 *    for(i=0; i<2; i++){
 *        download_service_id			16				uimsf
 *        download_component_tag		8				uimsf
 *        delivery_system_descriptor	192				24 bytes buffer
 *        crc16_2						16				uimsf
 *    }
 * }
 *********************************************************************/
static void tongfang_save_dib() //eric at beingjing 1019
{
	u_int16 crc16;
	int i;
	u_int8 DownloadBuffer[20],printBuffer[20];
    CodeDownload_Sub_Description *stb_sub_info;

    stb_sub_info = (CodeDownload_Sub_Description *)m_stb_param.pData;

	//store DIB
	DownloadBuffer[0]=1; //update_flag
	DownloadBuffer[1]=0; //update_aborted

	DownloadBuffer[2]=(unsigned char )((stb_sub_info->download_pid>>8) & 0x00ff);
	DownloadBuffer[3]=(unsigned char )(stb_sub_info->download_pid & 0x00ff); 

	DownloadBuffer[4]=(unsigned char )((stb_sub_info->PrivateData.software_version>>8) & 0x00ff);
	DownloadBuffer[5]=(unsigned char )(stb_sub_info->PrivateData.software_version & 0x00ff);

	// frequency 
	DownloadBuffer[9] = (stb_sub_info->deliver.frequency>>24)&0xff;
	DownloadBuffer[8] = (stb_sub_info->deliver.frequency>>16)&0xff;
	DownloadBuffer[7] = (stb_sub_info->deliver.frequency>>8)&0xff;
	DownloadBuffer[6] = (stb_sub_info->deliver.frequency)&0xff;

	// modulation
	DownloadBuffer[10] = stb_sub_info->deliver.modulation;

	// symbol rate
	DownloadBuffer[14] = (stb_sub_info->deliver.symbol_rate>>24)&0xff;
	DownloadBuffer[13] = (stb_sub_info->deliver.symbol_rate>>16)&0xff;
	DownloadBuffer[12] = (stb_sub_info->deliver.symbol_rate>>8)&0xff;
	DownloadBuffer[11] = (stb_sub_info->deliver.symbol_rate)&0xff;
#ifdef ipanel_crc
	crc16 = ipanel_GetCrc16((unsigned char *)&DownloadBuffer[0],15);
#endif

#ifdef daya_crc
	crc16 = CRC16_16((unsigned char *)&DownloadBuffer[0],15);
#endif

	DownloadBuffer[15]=(unsigned char)((crc16&0xff00)>>8);
	DownloadBuffer[16]=(unsigned char)(crc16&0xff);

	DownloadBuffer[17]=0;//flag
	DownloadBuffer[18]=0x00;//old soft_version
	DownloadBuffer[19]=0x09;

	ipanel_porting_eeprom_burn((UINT32_T)LOADER_INFO_ADDRESS,(CONST BYTE_T*)DownloadBuffer,20);
	ipanel_porting_task_sleep(2000);
	ee_read(LOADER_INFO_ADDRESS, printBuffer, 20,(void * )NULL);
	for(i =0;i<20;i++)
		printf("read from LOADER Data:%0x\n",printBuffer[i]);
}

static float get_bcd_4(u_int8 * buffer, u_int8 len,u_int8 dec_point)
{
     u_int32 num=0;
     float ret;
     int val=1,temp;
     int i,j;
     for(i = 0; i < len; i += 2)
     {
      val=1;
     	for(j=1;j<len-i;j++)
     	{
     	   val = 10*val;
     	}
     	temp = (((u_int32)(buffer[i/2]>>4)&0x0f)*val); 
     	num += temp;
     	if(val > 1)
     	{
     	    temp = (((u_int32)(buffer[i/2]&0x0f))*(val/10));
     	    num += temp;
     	}   
     }
     val = 1;	
     for(i=0;i<dec_point;i++)
     {
          val = val*10;
     }
     ret = (float)(num / (float)val); 
     return ret;    
    
}


static INT32_T STB_SIParseDownloadInfo(CONST BYTE_T *des,UINT32_T length)
{
    INT32_T ret = IPANEL_ERR;
    u_int8  *ptr = (BYTE_T*)des ;
    u_int8  *subptr;
    u_int8  description_length=0;
    u_int8  download_info_len;
    u_int16 stb_manufacturer_id ;
    u_int16 reserved_1;
    u_int8  FEC_outer;
    u_int8  modulation ;
    u_int32 frequency,symbol_rate;
    u_int8	hardware_id;
    u_int16 download_pid;
	u_int8  download_type;
    u_int16	software_version;  
    CodeDownload_Sub_Description  *subDes;
    int i ,iCycle;
      
    if( DOWNLOAD_DESCRIPTION_TAG != ptr[0])
    {
        ipanel_porting_dprintf("[STB_SIParseDownloadInfo] error des type!\n");
        return ret;
    }

    description_length = ptr[1]&0xff;
	
    stb_manufacturer_id = ((UINT16_T)ptr[2]<<8) | ((UINT16_T)ptr[3]);
	if(stb_manufacturer_id != 0x19F){
		printf("stb_manufacturer_id isn't 0x019F,just %0x\n",stb_manufacturer_id);
		return ret;
	}
    download_info_len = ptr[4+DOWNLOAD_SUB_DES_OFFSET]&0xff;

    m_stb_param.description_tag = ptr[0];
    m_stb_param.description_length = description_length;
    m_stb_param.stb_manufacturer_id = stb_manufacturer_id;
    
    iCycle = (description_length-2)/(download_info_len+DOWNLOAD_SUB_DES_OFFSET+1);
    subDes = (CodeDownload_Sub_Description*)ipanel_porting_malloc(sizeof(CodeDownload_Sub_Description)*iCycle);
    m_stb_param.pData = subDes;
    m_sub_des_num = iCycle;
    subptr = ptr+4;

    for(i=0;i<iCycle;i++)
    {
        subDes[i].deliver.description_tag = subptr[0];
        subDes[i].deliver.description_length = subptr[1];

        frequency = (UINT32_T)(get_bcd_4(&subptr[2],8,4)*1000000);
        reserved_1 = ((UINT16_T)subptr[6]<<8) | ((UINT16_T)subptr[7]&0xF0);
        FEC_outer  = subptr[7]&0x0F;
        modulation = subptr[8];

        symbol_rate = (UINT32_T)(get_bcd_4(&subptr[9],7,4)*1000000);
        subDes[i].deliver.frequency = frequency;
        subDes[i].deliver.reserve_use = reserved_1;
        subDes[i].deliver.modulation = modulation;
        subDes[i].deliver.symbol_rate = symbol_rate;
        subDes[i].deliver.FEC_inner = subptr[12]&0x0F;

        download_pid = ((UINT16_T)subptr[13]<<8)|(subptr[14]&0xF1);
        download_pid = (download_pid>>3);
		download_type = subptr[14]&0x07;

        subDes[i].download_pid = download_pid;
        subDes[i].download_type= download_type;
        subDes[i].download_info_len = subptr[15];

        hardware_id = subptr[16];
        software_version = ((UINT16_T)subptr[17]<<8)|subptr[18];
        subDes[i].PrivateData.hardware_id = hardware_id;
        subDes[i].PrivateData.software_version = software_version;

        subDes[i].PrivateData.serial_number[0] = subptr[19]&0xff;
        subDes[i].PrivateData.serial_number[1] = subptr[20]&0xff;
        subDes[i].PrivateData.serial_number[2] = subptr[21]&0xff;
        subDes[i].PrivateData.serial_number[3] = subptr[22]&0xff;
        subDes[i].PrivateData.serial_number[4] = subptr[23]&0xff;
        subDes[i].PrivateData.serial_number[5] = subptr[24]&0xff;
        subDes[i].PrivateData.serial_number[6] = subptr[25]&0xff;
        subDes[i].PrivateData.serial_number[7] = subptr[26]&0xff;

        subDes[i].PrivateData.serial_number_mask[0] = subptr[27]&0xff;
        subDes[i].PrivateData.serial_number_mask[1] = subptr[28]&0xff;
        subDes[i].PrivateData.serial_number_mask[2] = subptr[29]&0xff;
        subDes[i].PrivateData.serial_number_mask[3] = subptr[30]&0xff;
        subDes[i].PrivateData.serial_number_mask[4] = subptr[31]&0xff;
        subDes[i].PrivateData.serial_number_mask[5] = subptr[32]&0xff;
        subDes[i].PrivateData.serial_number_mask[6] = subptr[33]&0xff;
        subDes[i].PrivateData.serial_number_mask[7] = subptr[34]&0xff;

        subptr += (DOWNLOAD_SUB_DES_OFFSET+download_info_len+1);
    }

    printf("[OTA ParseDownloadInfo] upgrade info:");
    printf("Frequency = %d, Symbolrate = %d, Modulation = %d.\n",
                           frequency,symbol_rate,modulation);
    printf("Download pid = %d.download type = %d.\n",
						   download_pid,download_type);

    return IPANEL_OK;    
}

int judge_download(BYTE_T *pDes,UINT32_T len){
	u_int16 sv_flash;
	int i;
	int pri_f;
	u_int8 iibBuf[18]; 

	u_int16	software_version = ((UINT16_T)pDes[21]<<8) | ((UINT16_T)pDes[22]);
	u_int8	download_type = pDes[18]&0x07;

	ee_read( LOADER_INFO_ADDRESS+4, soft_ver_flash,2,(void *) NULL);
	sv_flash= (((u_int16)(soft_ver_flash[0]))<<8) | (u_int16)(soft_ver_flash[1]);

	if((pri_f=read_flash((void *)LD_IIB_ADDRESS,18,iibBuf))==0)
		trace("### read_flash() fail ! ###\n");

	Int2longlong(&pDes[13],8,&start_serial);
    Int2longlong(&pDes[21],8,&end_serial);
	Int2longlong(&iibBuf[6],8,&nvram_serial_number);
	for(i=0;i<8;i++){
		printf("nvram_serial_num %d,:%0x\n",i,iibBuf[6+i]);
	}
	
	if(sv_flash!=0xffff)
		{
			if(software_version <= sv_flash )
			{	
				printf("OTA soft version:%0x\n",software_version);
				printf("flash soft version:%0x\n",sv_flash);
				printf("OTA soft version not higher than flash soft version\n");
				return 0;
			}else if(download_type ==1 || download_type ==2){
				printf("OTA TYPE :%d\n",download_type);
				if((nvram_serial_number>end_serial) ||(nvram_serial_number <start_serial)){
						printf("serial_number not include in\n %d,%d,%d",nvram_serial_number,start_serial,end_serial);
						return download_type;
					}

				return download_type;
			}
				
		}
	return  -1;

}
int Get_Codedowndes_from_section(CHAR_T *des,UINT32_T len)
{
	int ret = -1;
	int i = 0;
	unsigned char *p=(unsigned char *)des;

	
	unsigned char table_id = *p;

	unsigned short section_length = ((((p[1]&0xf)<<8)&0xff00)|p[2]);
	unsigned short network_id = (p[3]<<8 )|p[4];

	unsigned char version_number =(( p[5]&0x3e)>>1)&0x1f;
	unsigned char current_next_indicator = p[5]&0x1;
	unsigned char section_number = p[6];
	unsigned char last_section_number = p[7];

	signed short network_descriptors_length = ((p[8]&0xf)<<8)|p[9];

	unsigned char tag = 0x00;
	unsigned char solid_version= 0x0;
	unsigned char des_length = 0;
	unsigned char *pDes = &p[10];
	u_int16 stb_manufacturer_id;

	while(network_descriptors_length>2)
	{
		tag = pDes[0];
		des_length = pDes[1];
		solid_version = pDes[20];
		stb_manufacturer_id= ((UINT16_T)pDes[2]<<8) | ((UINT16_T)pDes[3]);
		printf(" tag =%02x %d\n",tag,des_length);

		if((0xA1 == tag) && (stb_manufacturer_id ==0x019F) && (solid_version ==0xF))
		{	
			ret = judge_download(pDes,(des_length+2));
			if(1== ret || 2 ==ret)
				STB_SIParseDownloadInfo(pDes,(des_length+2));
			return ret;			
		}
	
		pDes += (des_length+2);
		network_descriptors_length -= (des_length+2);
		
	}
	return -1;
}


/******************************************************************
    功能说明：
		将表中收到的升级描述符或NIT表传给升级规则检测模块。
		什么时候传递哪种类型的数据是和实际项目相关的。
		当ipanel传递NIT表时，不在提供参数查询接口，NIT表中的信息
		应该满足用户的升级检测和获取升级参数的需求。

    参数说明：
        输入参数：
		des：升级描述符地址
		len：升级描述符长度

    返    回：
		-1 - 错误，数据错误或不是本设备升级描述符；
		0 - 版本相等，无需升级;
		1 - 有新版本，手动升级;
		2 - 有新版本，强制升级。
******************************************************************/
INT32_T ipanel_upgrade_check(CHAR_T *des, UINT32_T len)
{
    int i,ret;

#ifdef USE_IPANEL_UPGRADE

#ifdef DEBUG_UPGRADE
	ipanel_porting_dprintf("[ipanel_upgrade_check] des = 0x%x, len = %d\n",des,len);
	for(i = 0; i < len; i++)
	{
		if(i % 16 == 0)
			ipanel_porting_dprintf("\n");
		ipanel_porting_dprintf("%2x ",des[i]);
	}
	ipanel_porting_dprintf("\n");
#endif
	ret = Get_Codedowndes_from_section(des,len);
	ipanel_porting_dprintf("ipanel_upgrade_check return:%d\n",ret);
#endif

	return ret;
}

/******************************************************************
    功能说明：
   		通知升级执行模块执行升级操作。
   		
    参数说明：
        输入参数：
		des：升级描述符地址，一般为空
		len：升级描述符长度，des为空时，len为0

    返    回：
    IPANEL_OK:成功;
    IPANEL_ERR:失败。
******************************************************************/
INT32_T ipanel_upgrade_start(CHAR_T *des, UINT32_T len)
{
#ifdef USE_IPANEL_UPGRADE	
    tongfang_save_dib();
	ipanel_porting_task_sleep(2500);
	//reboot();
	reboot_IRD();
#endif

	return IPANEL_OK;
}

#define IPANEL_APP_NAME_NUMBER     11
#define IPANEL_APP_NAME_LEN        20
#define IPANEL_APP_VALUE_LEN       16

static char m_app_name[][IPANEL_APP_NAME_LEN] =
{
    "action",
    "frequency",
    "symbol_rate",
    "modulation",
    "serviceID",
    "tableID",
    "PID",
    "boot_version",
    "loader_version",
    "root_ffs_version",
    "software_version"
};

typedef struct
{
    unsigned char action[IPANEL_APP_VALUE_LEN];
    unsigned int  frequency ;
    unsigned int  symbol_rate;
    unsigned int  modulation;
    unsigned int  serviceID;
    unsigned int  tableID;
    unsigned int  PID;
    unsigned char  boot_version[8];
    unsigned char  loader_version[8];
    unsigned char  root_ffs_version[8];
    unsigned char  software_version[8];
}APP_LOADER_INFO_s;

static APP_LOADER_INFO_s m_app_info = {0};
static char m_app_name_array[IPANEL_APP_NAME_NUMBER][IPANEL_APP_VALUE_LEN]={0};

int ipanel_str2int(char *str)
{
    int i,n,sign;

    /* 跳过空格符 */
    for(i=0;isspace(str[i]);i++)
    {
        ;
    }

    /* 判定正负号 */
    sign = (str[i] == '-')?-1:1;

    /* 跳过正负号 */
    if(str[i] == '+' || str[i] == '-')
        i++;

    /* 求值 */
    for(n=0;n<isdigit(str[i]);i++)
    {
        n = 10*n + (str[i] - '0');
    }

    return sign*n;
}

char *ipanel_app_next_string(char *string,char *match_name,char *result_value)
{
    char *ptrs=NULL,*ptre=NULL;

    ptrs = strstr(string,match_name);
    if(NULL == ptrs )
    {
        ipanel_porting_dprintf("[ipanel_app_parse_string] %s parse failed!\n",m_app_name[0]);
        return NULL;
    }

    /* 查找到其值并拷贝到相关存储区域 */
    ptrs = strstr(ptrs,"=");
    ptre = strstr(ptrs,";");
    strncpy(result_value,ptrs+1,(ptre-ptrs-1));

    return ptrs;
}

static INT32_T ipanel_app_parse_string(const char *string)
{
    int i;
    INT32_T ret = IPANEL_OK;
    char *pString = (char*)string ;
    char *ptr = NULL;

    /* 解析到相关字符串数组中 */
    for(i=0;i<IPANEL_APP_NAME_NUMBER;i++)
    {
        pString = ipanel_app_next_string(pString,&m_app_name[i][0],
                                         &m_app_name_array[i][0]);
        if(NULL == pString)
        {
            ipanel_porting_dprintf("[ipanel_app_parse_string]parse failed!\n");
            return IPANEL_ERR;
        }

        #ifdef USE_IPANEL_UPGRADE
        ipanel_porting_dprintf("%s:%s]\n",
                               &m_app_name[i][0],&m_app_name_array[i][0]);
        #endif
    }

    /* 转换相关字符串为相关数值 */
    strncpy((char*)m_app_info.action,&m_app_name_array[0][0],IPANEL_APP_VALUE_LEN);
    m_app_info.frequency = atoi(&m_app_name_array[1][0]);
    m_app_info.symbol_rate = atoi(&m_app_name_array[2][0]);

    ptr = &m_app_name_array[3][0];
    if(strcmp(ptr,"16-QAM") == 0)
        m_app_info.modulation = 1;
    else if(strcmp(ptr,"32-QAM") == 0)
        m_app_info.modulation = 2;
    else if(strcmp(ptr,"64-QAM") == 0)
        m_app_info.modulation = 3;
    else if(strcmp(ptr,"128-QAM") == 0)
        m_app_info.modulation = 4;
    else if(strcmp(ptr,"256-QAM") == 0)
        m_app_info.modulation = 5;

    m_app_info.serviceID = atoi(&m_app_name_array[4][0]);
    m_app_info.tableID= atoi(&m_app_name_array[5][0]);
    m_app_info.PID = atoi(&m_app_name_array[6][0]);
        
    strncpy((char*)m_app_info.boot_version,&m_app_name_array[7][0],IPANEL_APP_VALUE_LEN);
    strncpy((char*)m_app_info.loader_version,&m_app_name_array[8][0],IPANEL_APP_VALUE_LEN);
    strncpy((char*)m_app_info.root_ffs_version,&m_app_name_array[9][0],IPANEL_APP_VALUE_LEN);
    strncpy((char*)m_app_info.software_version,&m_app_name_array[10][0],IPANEL_APP_VALUE_LEN);

    return IPANEL_OK;
}

/******************************************************************
    功能说明：
	参数为一个字符串，只有一个，这个参数是从页面传过来的，
	iPanel MiddleWare相当于一个通道，原封不动的将从页面获得的字符串
	调用这个接口传给底层。由底层触发一些动作。是否销毁iPanel MiddleWare
	由实现此函数的相关人员决定。

    参数说明：
        输入参数：
		name：底层所知道的一个功能描述符，传到底层，由底层根据这个字符
		串来来决定需要什么动作。

    返    回：
    IPANEL_OK:成功;
    IPANEL_ERR:失败。
******************************************************************/
INT32_T ipanel_start_other_app(CONST CHAR_T *name)
{   
	char p[4];
	unsigned int ch;
	unsigned char ipstr[4];
	
    memset(&m_app_info.action,0x00,sizeof(m_app_info));
    memset(&m_app_name_array[0][0],0x00,sizeof(m_app_name_array));

#ifdef USE_IPANEL_UPGRADE
	strncpy(p,name,4);
	if(strcmp(p,"ping") == 0){
		name=name+5;
		ch = ipanel_ip_convert(name,4);
		return ipanel_ping_ip(ch,32);
		//ipanel.misc.startotherapp("ping:192.168.10.25");
	}
	else
    	return ipanel_app_parse_string(name);	
#endif

    return IPANEL_OK;
}

static CHAR_T test_app_string[] = 
{
    "action=upgrade;frequency=259000000;symbol_rate=6875000;\
     modulation=64-QAM;serviceID=10001;tableID=1234;PID=1000;\
     boot_version=1.2;loader_version=1.2; root_ffs_version =1.1;\
     software_version =2.0;"
};

void ipanel_other_app_test()
{
    ipanel_start_other_app(test_app_string);
}

void ipanel_eeprom_test(){
	int i;
	unsigned char flash_loader[20];
	ee_read(LOADER_INFO_ADDRESS,flash_loader,20,(void * )NULL);
	for(i=0 ;i<20;i++ ){
		printf("flash_loader[%d]:%0x\n",i,flash_loader[i]);
	}
	printf("crc calc start\n");
	crc_2dot_test();
	printf("crc calc end\n");
	
}

void ipanel_eeprom_write(){
	u_int16 crc;
	u_int32 frequency;
	unsigned char flash_loader[20];
	flash_loader[0] = 0x1;
	flash_loader[1] = 0x0;
	flash_loader[2] = 0x1b;
	flash_loader[3] = 0x68;
	flash_loader[4] = 0x0;
	flash_loader[5] = 0xa;
	
	flash_loader[6] = 0xc0;			//freq
	flash_loader[7] = 0xa8;
	flash_loader[8] = 0xba;
	flash_loader[9] = 0x13;			//freq

	frequency = ((UINT32_T)flash_loader[6]<<24)|((UINT32_T)flash_loader[7]<<16)|
                     ((UINT32_T)flash_loader[8]<<8) |((UINT32_T)flash_loader[9]);
	printf("freq:%ld\n",frequency);
	
	flash_loader[10] = 0x3;
	flash_loader[11] = 0x78;
	flash_loader[12] = 0xe7;
	flash_loader[13] = 0x68;
	flash_loader[14] = 0x0;

	crc = CRC16_16((unsigned char *)&flash_loader[0],15);
	flash_loader[15]=(unsigned char)((crc&0xff00)>>8);
	flash_loader[16]=(unsigned char)(crc&0xff);
	printf("flash_loader[15]:%0x\n",flash_loader[15]);
	printf("flash_loader[16]:%0x\n",flash_loader[16]);

	//flash_loader[15] = 0x0;   //crc
	
	//flash_loader[16] = 0x0;   //crc
	flash_loader[17] = 0x0;
	flash_loader[18] = 0x0;
	flash_loader[19] = 0xa;
	
	ee_write(LOADER_INFO_ADDRESS,flash_loader,20,(void * )NULL);
	ipanel_eeprom_test();
}
void crc_2dot_test(){
	//u_int8 Data[15]={0x0,0x0,0x1b,0x68,0x0,0xa,0xc0,0xa8,0xba,0x13,0x3,0x78,0xe7,0x68,0x0};
	u_int8 Data[20];
	u_int16 crc1,crc2;
	u_int8 data2[4];
	int i;
	ee_read(LOADER_INFO_ADDRESS, Data, 15,(void * )NULL);

	crc1 = ipanel_GetCrc16((unsigned char *)&Data[0],15);
	crc2 = CRC16_16((unsigned char *)&Data[0],15);
	data2[0]=(unsigned char)((crc1&0xff00)>>8);
	data2[1]=(unsigned char)(crc1&0xff);
	data2[2]=(unsigned char)((crc2&0xff00)>>8);
	data2[3]=(unsigned char)(crc2&0xff);

	for(i=0;i<4;i++){
		printf("crc value%d :%0x\n",i,data2[i]);
	}

	for(i=0;i<20;i++){
		printf("flash loader value%d :%0x\n",i,Data[i]);
	}
}

