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
#ifndef _IPANEL_MIDDLEWARE_PORTING_DEVICE_API_FUNCTOTYPE_H_
#define _IPANEL_MIDDLEWARE_PORTING_DEVICE_API_FUNCTOTYPE_H_

#include "ipanel_typedef.h"
#include "ipanel_network.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum 
{
	IPANEL_NET_NONE,
	IPANEL_NET_IP=1,
	IPANEL_NET_GW=2,
	IPANEL_NET_MASK=3,
	IPANEL_NET_MAC =4,
	IPANEL_NET_ALL =5
}IPANEL_NET_OP;

int get_net_link_status();

int ipanel_network_set_ioctl(IPANEL_NET_OP op,IPANEL_NETWORK_IF_PARAM *arg);

int ipanel_network_get_ioctl(IPANEL_NET_OP op,IPANEL_NETWORK_IF_PARAM *arg);

int cnxt_nuptst_init( unsigned int ip,unsigned int subnet_mask,unsigned int gateway );

#define IPANEL_DEVICE_PARAMS_MAX_LENGTH	64
#define IPANEL_DEVICE_BUFFER_MAX_LENGTH	64

typedef struct
{
	CHAR_T params[IPANEL_DEVICE_PARAMS_MAX_LENGTH];
	CHAR_T buffer[IPANEL_DEVICE_BUFFER_MAX_LENGTH];
}IPANEL_DEVICE_INFO;

/* EIS_MULTIFNIRKEY_MOTION P2��չ���� */
typedef struct 
{
	unsigned char 	button;
	unsigned char 	dx;		/* delta_x; */
	unsigned char 	dy;		/* delta_y */
	unsigned char 	dwheel;	/* delta_wheel*/
	unsigned int  	seq;
	unsigned short 	ax;  	/* acceleration_x */
	unsigned short 	ay;		/* acceleration_y */
	unsigned short 	az;		/* acceleration_z */
	unsigned short 	angular_velocity_x;
	unsigned short 	angular_velocity_y;
	unsigned short 	angular_velocity_z;	
}MultiFnIrkeyMot;

/* EIS_MULTIFNIRKEY_POSITION P2��չ���� */
typedef struct 
{
	unsigned char 	button;
	unsigned char 	dx;		/* delta_x; */
	unsigned char 	dy;		/* delta_y */
	unsigned char 	dwheel;	/* delta_wheel*/
	unsigned int  	seq;
	unsigned short 	x;
	unsigned short 	y;
	unsigned short 	z;
	unsigned short 	angular_position_a;
	unsigned short 	angular_position_b;
	unsigned short 	angular_position_c;
	unsigned short 	angular_position_d;
}MultiFnIrkeyPos;

/*��ȡ���磬�����ʼ��Ȳ���ֵ����ͬӲ�����в�ͬ����;��һ�㶼�Ǻ�ipanel�Զ����JS���ʹ��*/
/*	���������params���Զ��������; length���ַ���ֵ�ĳ���.
	���������buf����ǰ���ƶ�Ӧ��ֵ.
	��    �أ������ַ���ֵ�ĳ��ȡ�*/
INT32_T	ipanel_porting_device_read(CONST CHAR_T *params, CHAR_T *buf, INT32_T length);

/*�������磬�����ʼ��Ȳ���ֵ����ͬӲ�����в�ͬ����;��һ�㶼�Ǻ�ipanel�Զ����JS���ʹ��*/
/*	�����������
	���������params���Զ��������; buf�������Ʋ�����Ӧ��ֵ; length���ַ���ֵ�ĳ���.
	��    �أ�ͨ�����ز���ֵ�ĳ��ȣ�-1 ��ʾδ֪�Ĳ������ߴ���*/
INT32_T ipanel_porting_device_write(CONST CHAR_T *params, CHAR_T *buf, INT32_T length);

INT32_T GetInnerSerialNumber(char* pBuffer, int nSize);

INT32_T GetOuterSerialNumber(char* pBuffer, int nSize);

#ifdef __cplusplus
}
#endif

#endif  // _IPANEL_MIDDLEWARE_PORTING_DEVICE_API_FUNCTOTYPE_H_

