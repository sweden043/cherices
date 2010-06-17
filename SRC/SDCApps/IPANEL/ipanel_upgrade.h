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
#ifndef _IPANEL_MIDDLEWARE_PORTING_UPGRADE_API_FUNCTOTYPE_H_
#define _IPANEL_MIDDLEWARE_PORTING_UPGRADE_API_FUNCTOTYPE_H_

#include "ipanel_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
			NIT HuNan linkage description 
			
	CodeDownload_Description()
	{
		Descriptor_Tag         8bit
		Descriptor_Length      8bit
		STB_Manufacturer_ID    16bit
		for(i=0;i<N;i++)       
		{
			Delivery_system_descriptor  
			Download_pid         13bit
			Download_type        3bit
			Download_info_len    8bit
			Download_info        
		}
	}	

	Delivery System Descriptor()  //13Byte
	{
		descriptor_tag      8bit
		descriptor_length   8bit
		frequency           32bit
		reserve_use         12bit
		FEC_outer           4bit
		modulation          8bit
		symbol_rate         28bit
		FEC_inner           4bit
	}
    
*********************************************************************/
#define  LOADER_INFO_ADDRESS   0x7F00
#define  LD_IIB_ADDRESS		   0x2001F000

#define DOWNLOAD_DESCRIPTION_TAG 0xA1
#define DOWNLOAD_SUB_DES_OFFSET  (13+2)

#define DOWNLOAD_TYPE_MANUAL   0x01
#define DOWNLOAD_TYPE_FORCE    0x02

typedef struct 
{
    u_int8		hardware_id;
    u_int16		software_version;
    u_int8		serial_number[8];
    u_int8		serial_number_mask[8];    
}Download_Private_data;

typedef struct
{
    u_int8  description_tag;
    u_int8  description_length;
    u_int32 frequency ;
    u_int16 reserve_use;
    u_int8  FEC_outer;
    u_int8  modulation;
    u_int32 symbol_rate;
    u_int8  FEC_inner;
}Delivery_system_description;
typedef struct 
{
	u_int64 start_serial;
	u_int64 end_serial;
}TAG_SERIAL;


typedef struct 
{
    Delivery_system_description deliver;
    u_int16 download_pid;
    u_int8  download_type;
    u_int8  download_info_len;
    Download_Private_data PrivateData;
}CodeDownload_Sub_Description;

typedef struct 
{
    u_int8  description_tag;
    u_int8  description_length;
    u_int16 stb_manufacturer_id;
    void     *pData ;
}CodeDownload_Description;

//--------------------------------------------------------------------------------------------------
//  TYPES DEFINITION
//--------------------------------------------------------------------------------------------------
//
typedef struct tagIPANEL_LOADER_INFO
{
	unsigned int Frequency;	    // example: 2990000  
	unsigned int Symbolrate;	// example: 69000
	unsigned int Modulation;	// example: "1"(16-QAM)、"2"(32-QAM)、"3"(64-QAM)、"4"(128-QAM)、"5"(256-QAM)
	unsigned int Pid;				// example: 0x137
	
	//以下这些参数只有在通过后门升级时ipanel才会赋值并传给第三方
	unsigned int  TableId;			// 根据需要赋值，可能为0
	unsigned char Version;			// 将要升级的软件的版本(一般不传)     
	unsigned char Upgrade_Type;		// 提示更新:0 ; 强制更新:1
	unsigned char Upgrade_flag;		// currently no use,pass 1
/*______________________________________________________________________________________
|软件类别						|				 CODE 值 				|											备 注               |
|___________________|_______________________|_________________________________________|
|系统软件 					|				0X0001 					|							 系统软件与应用软件         |
|___________________|_______________________|_________________________________________|
|基础系统软件 			|				0X0002 					|						Miniload 升级、硬件诊断等     |
|___________________|_______________________|_________________________________________|
|其他 							|		0X0003~0X00CF 			|											保留                |
|___________________|_______________________|_________________________________________|
|厂商自定义			 		|		0X00D0~0X00FF 			|						保留给厂商自己定义软件类型    |
|___________________|_______________________|_________________________________________|
|独立附加外设       |     0X0100~0XFFFF     |  为终端提供的附加外设设备专用驱动程序   |                   
|驱动软件模块       |                       |       每个驱动软件CODE 值需事先单独分配 |                        
____________________|_______________________|_________________________________________|                      
*/ 
	unsigned char Software_Type;	// 表明将要升级的软件类型，应用软件:0 ; 系统软件:1
	
}IPANEL_LOADER_INFO;

static const unsigned short awTable_CRC16[256] =
{
   0x0000, 0xc0c1, 0xc181, 0x0140, 0xc301, 0x03c0, 0x0280, 0xc241,
   0xc601, 0x06c0, 0x0780, 0xc741, 0x0500, 0xc5c1, 0xc481, 0x0440,
   0xcc01, 0x0cc0, 0x0d80, 0xcd41, 0x0f00, 0xcfc1, 0xce81, 0x0e40,
   0x0a00, 0xcac1, 0xcb81, 0x0b40, 0xc901, 0x09c0, 0x0880, 0xc841,
   0xd801, 0x18c0, 0x1980, 0xd941, 0x1b00, 0xdbc1, 0xda81, 0x1a40,
   0x1e00, 0xdec1, 0xdf81, 0x1f40, 0xdd01, 0x1dc0, 0x1c80, 0xdc41,
   0x1400, 0xd4c1, 0xd581, 0x1540, 0xd701, 0x17c0, 0x1680, 0xd641,
   0xd201, 0x12c0, 0x1380, 0xd341, 0x1100, 0xd1c1, 0xd081, 0x1040,
   0xf001, 0x30c0, 0x3180, 0xf141, 0x3300, 0xf3c1, 0xf281, 0x3240,
   0x3600, 0xf6c1, 0xf781, 0x3740, 0xf501, 0x35c0, 0x3480, 0xf441,
   0x3c00, 0xfcc1, 0xfd81, 0x3d40, 0xff01, 0x3fc0, 0x3e80, 0xfe41,
   0xfa01, 0x3ac0, 0x3b80, 0xfb41, 0x3900, 0xf9c1, 0xf881, 0x3840,
   0x2800, 0xe8c1, 0xe981, 0x2940, 0xeb01, 0x2bc0, 0x2a80, 0xea41,
   0xee01, 0x2ec0, 0x2f80, 0xef41, 0x2d00, 0xedc1, 0xec81, 0x2c40,
   0xe401, 0x24c0, 0x2580, 0xe541, 0x2700, 0xe7c1, 0xe681, 0x2640,
   0x2200, 0xe2c1, 0xe381, 0x2340, 0xe101, 0x21c0, 0x2080, 0xe041,
   0xa001, 0x60c0, 0x6180, 0xa141, 0x6300, 0xa3c1, 0xa281, 0x6240,
   0x6600, 0xa6c1, 0xa781, 0x6740, 0xa501, 0x65c0, 0x6480, 0xa441,
   0x6c00, 0xacc1, 0xad81, 0x6d40, 0xaf01, 0x6fc0, 0x6e80, 0xae41,
   0xaa01, 0x6ac0, 0x6b80, 0xab41, 0x6900, 0xa9c1, 0xa881, 0x6840,
   0x7800, 0xb8c1, 0xb981, 0x7940, 0xbb01, 0x7bc0, 0x7a80, 0xba41,
   0xbe01, 0x7ec0, 0x7f80, 0xbf41, 0x7d00, 0xbdc1, 0xbc81, 0x7c40,
   0xb401, 0x74c0, 0x7580, 0xb541, 0x7700, 0xb7c1, 0xb681, 0x7640,
   0x7200, 0xb2c1, 0xb381, 0x7340, 0xb101, 0x71c0, 0x7080, 0xb041,
   0x5000, 0x90c1, 0x9181, 0x5140, 0x9301, 0x53c0, 0x5280, 0x9241,
   0x9601, 0x56c0, 0x5780, 0x9741, 0x5500, 0x95c1, 0x9481, 0x5440,
   0x9c01, 0x5cc0, 0x5d80, 0x9d41, 0x5f00, 0x9fc1, 0x9e81, 0x5e40,
   0x5a00, 0x9ac1, 0x9b81, 0x5b40, 0x9901, 0x59c0, 0x5880, 0x9841,
   0x8801, 0x48c0, 0x4980, 0x8941, 0x4b00, 0x8bc1, 0x8a81, 0x4a40,
   0x4e00, 0x8ec1, 0x8f81, 0x4f40, 0x8d01, 0x4dc0, 0x4c80, 0x8c41,
   0x4400, 0x84c1, 0x8581, 0x4540, 0x8701, 0x47c0, 0x4680, 0x8641,
   0x8201, 0x42c0, 0x4380, 0x8341, 0x4100, 0x81c1, 0x8081, 0x4040
};


/*
参数为一个字符串，只有一个，这个参数是从页面传过来的，iPanel MiddleWare相当于一个通道，
原封不动的将从页面获得的字符串调用这个接口传给底层。由底层触发一些动作。是否销毁
iPanel MiddleWare由实现此函数的相关人员决定。
*/
INT32_T ipanel_start_other_app(CONST CHAR_T *name);

/*
将表中收到的升级描述符传给升级规则检测模块。
return：
-1 	– 错误，数据错误或不是本设备升级描叙符；
 0 	- 版本相等，无需升级;
 1 	- 有新版本，手动升级;
 2 	- 有新版本，强制升级。
*/
INT32_T ipanel_upgrade_check(CHAR_T *des, UINT32_T len);

/*通知升级执行模块执行升级操作。*/
INT32_T ipanel_upgrade_start(CHAR_T *des, UINT32_T len);

/******************************************************************
    功能说明：
    获取升级流入口参数。Ipanel实现，机顶盒厂家调用。当ipanel在check
    函数中传递NIT表时，此函数无效。

    参数说明：
        输入参数：
		onid：该升级数据包下传所在频道Original_network_id 值
		tsid：存放下载频点信息,QAM 信息,和符号率信息的NIT表的ID。
		srvid：下载数据流的ID，该值应为描述升级数据包的PMT 
		       在该频道的节目号，一般定义一个Service_id对应一个厂商。
		comp_tag：定义本版本的Loader PID
		info：返回升级需要参数的结构体指针。

    返    回：
    IPANEL_OK:成功;
    IPANEL_ERR:失败。
******************************************************************/
INT32_T ipanel_upgrade_getparams(UINT32_T onid, UINT32_T tsid, UINT32_T srvid, 
								 UINT32_T comp_tag, IPANEL_LOADER_INFO *info);
#ifdef __cplusplus
}
#endif

#endif //_IPANEL_MIDDLEWARE_PORTING_UPGRADE_API_FUNCTOTYPE_H_

