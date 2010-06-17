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
#ifndef _IPANEL_MIDDLEWARE_PORTING_CONFIG_API_FUNCTOTYPE_H_
#define _IPANEL_MIDDLEWARE_PORTING_CONFIG_API_FUNCTOTYPE_H_

#include "ipanel_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IPANEL_MAIN_WAIT_TIMEOUT        10 //ms

// iPanel�м��ʹ�õ��ڴ�ռ��С
#define IPANEL_MIDDLEWARE_MEMORY_SIZE   (17*1024*1024)

#define IPANEL_MAIN_QUEUE_MAX           12

// ������Ļ����ʾ�Ĵ�С�Լ���ʼ��ʾλ��
#define IPANEL_DEST_PAL_WIDTH       (720)
#define IPANEL_DEST_PAL_HEIGHT      (576)

#define IPANEL_TV_SCREEN_WIDTH      (640) // ʵ����Ļ��ʾ���
#define IPANEL_TV_SCREEN_HEIGHT     (526) // ʵ����Ļ��ʾ�߶�

#define IPANEL_OSD_XOFFSET           48
#define IPANEL_OSD_YOFFSET           25

#define PORTING_ARGB1555            0   // ʹ��16λ��ɫ��ʽ
#define ARGB8888_to_ARGB1555        1   // ʹ��32λ���ʹ��16λ��ʾ
#define PORTING_ARGB8888            2   // ʹ��32λ��ɫ��ʽ

//#define USE_SEMITRANSPARENT_COLOR       // �򿪰�͸����ɫ��ʾ

// ��Ҫ������ɫ��ʽ�Ļ���ֻ��Ҫ���Ĵ˺궨�弴��
#define  PORTING_COLORFMT           PORTING_ARGB1555
//#define  TEST_OSD_AUTOSCALE

#define  OSD_MALLOC_SELF            1 // ʹ��get_info���з����ڴ�

//#define USE_PORTING_TRANSPARENT  //���colorֵ��PORTING�㶨���͸��ɫ��ͬʱ, ������colorֵ, �����1, ������ɫ�𻵵���С.   

//#define DHCP_ADD_VENDOR_CLASS_ID  // �Ƿ�DHCP����Vdendor class id

#define IPANEL_TUNER_CHECK_STATUS   // �Ƿ�ÿ��1s���tuner��Ƶ״̬

// �Ƿ��ӡLOG, 1Ϊ��ӡ, 0Ϊ����ӡ
#define DEBUG_OUT                       0

#define USE_NO_TFCA_CARD  // �Ƿ�ʹ��TFCA �Ŀ�

//#define IPANEL_WRITE_UI2FLASH  // ʹ�����UI

//#define IPANEL_RELEASE_VERSION   // �Ƿ�ʹ��RELEASE�汾  

#define USE_IPANEL_UPGRADE     // �Ƿ�ʹ��upgrade����ģʽ

//#define IPANEL_PRE_MALLOC_BUFFER  // ����ʱʹ��Ԥ�ȷ���õ�2M�ڴ���䷽ʽ

//#define IPANEL_LOADER_DEBUG
//#define IPANEL_LOADER_DEBUG_WRITE

#endif  // _IPANEL_MIDDLEWARE_PORTING_CONFIG_API_FUNCTOTYPE_H_

