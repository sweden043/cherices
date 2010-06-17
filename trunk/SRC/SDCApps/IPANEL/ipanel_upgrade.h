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
	unsigned int Modulation;	// example: "1"(16-QAM)��"2"(32-QAM)��"3"(64-QAM)��"4"(128-QAM)��"5"(256-QAM)
	unsigned int Pid;				// example: 0x137
	
	//������Щ����ֻ����ͨ����������ʱipanel�Żḳֵ������������
	unsigned int  TableId;			// ������Ҫ��ֵ������Ϊ0
	unsigned char Version;			// ��Ҫ����������İ汾(һ�㲻��)     
	unsigned char Upgrade_Type;		// ��ʾ����:0 ; ǿ�Ƹ���:1
	unsigned char Upgrade_flag;		// currently no use,pass 1
/*______________________________________________________________________________________
|������						|				 CODE ֵ 				|											�� ע               |
|___________________|_______________________|_________________________________________|
|ϵͳ��� 					|				0X0001 					|							 ϵͳ�����Ӧ�����         |
|___________________|_______________________|_________________________________________|
|����ϵͳ��� 			|				0X0002 					|						Miniload ������Ӳ����ϵ�     |
|___________________|_______________________|_________________________________________|
|���� 							|		0X0003~0X00CF 			|											����                |
|___________________|_______________________|_________________________________________|
|�����Զ���			 		|		0X00D0~0X00FF 			|						�����������Լ������������    |
|___________________|_______________________|_________________________________________|
|������������       |     0X0100~0XFFFF     |  Ϊ�ն��ṩ�ĸ��������豸ר����������   |                   
|�������ģ��       |                       |       ÿ���������CODE ֵ�����ȵ������� |                        
____________________|_______________________|_________________________________________|                      
*/ 
	unsigned char Software_Type;	// ������Ҫ������������ͣ�Ӧ�����:0 ; ϵͳ���:1
	
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
����Ϊһ���ַ�����ֻ��һ������������Ǵ�ҳ�洫�����ģ�iPanel MiddleWare�൱��һ��ͨ����
ԭ�ⲻ���Ľ���ҳ���õ��ַ�����������ӿڴ����ײ㡣�ɵײ㴥��һЩ�������Ƿ�����
iPanel MiddleWare��ʵ�ִ˺����������Ա������
*/
INT32_T ipanel_start_other_app(CONST CHAR_T *name);

/*
�������յ���������������������������ģ�顣
return��
-1 	�C �������ݴ�����Ǳ��豸�����������
 0 	- �汾��ȣ���������;
 1 	- ���°汾���ֶ�����;
 2 	- ���°汾��ǿ��������
*/
INT32_T ipanel_upgrade_check(CHAR_T *des, UINT32_T len);

/*֪ͨ����ִ��ģ��ִ������������*/
INT32_T ipanel_upgrade_start(CHAR_T *des, UINT32_T len);

/******************************************************************
    ����˵����
    ��ȡ��������ڲ�����Ipanelʵ�֣������г��ҵ��á���ipanel��check
    �����д���NIT��ʱ���˺�����Ч��

    ����˵����
        ���������
		onid�����������ݰ��´�����Ƶ��Original_network_id ֵ
		tsid���������Ƶ����Ϣ,QAM ��Ϣ,�ͷ�������Ϣ��NIT���ID��
		srvid��������������ID����ֵӦΪ�����������ݰ���PMT 
		       �ڸ�Ƶ���Ľ�Ŀ�ţ�һ�㶨��һ��Service_id��Ӧһ�����̡�
		comp_tag�����屾�汾��Loader PID
		info������������Ҫ�����Ľṹ��ָ�롣

    ��    �أ�
    IPANEL_OK:�ɹ�;
    IPANEL_ERR:ʧ�ܡ�
******************************************************************/
INT32_T ipanel_upgrade_getparams(UINT32_T onid, UINT32_T tsid, UINT32_T srvid, 
								 UINT32_T comp_tag, IPANEL_LOADER_INFO *info);
#ifdef __cplusplus
}
#endif

#endif //_IPANEL_MIDDLEWARE_PORTING_UPGRADE_API_FUNCTOTYPE_H_

