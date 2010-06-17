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
#ifndef _IPANEL_MIDDLEWARE_PORTING_NVRAM_API_FUNCTOTYPE_H_
#define _IPANEL_MIDDLEWARE_PORTING_NVRAM_API_FUNCTOTYPE_H_

#include "ipanel_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IPANEL_APP_CODE_START_ADDR  (0x200F0000)

#define NVRAM_BLOCK_SIZE            (0x10000)   //64k

#define IPANEL_USER_DATA_ADDRESS    (0x204F0000)

#define IPANEL_CORE_UI_ADDRESS    (IPANEL_USER_DATA_ADDRESS)
#define IPANEL_CORE_UI_SIZE       (0x200000) //2M

#define IPANEL_CORE_FONT_ADDRESS   (IPANEL_CORE_UI_ADDRESS + IPANEL_CORE_UI_SIZE)
#define IPANEL_CORE_FONT_SIZE      (0x00)

#define IPANEL_CORE_RESERVE_ADDRESS (IPANEL_CORE_FONT_ADDRESS + IPANEL_CORE_FONT_SIZE)
#define IPANEL_CORE_RESERVE_SIZE    (0x60000) 

#define IPANEL_NVRAM_APP_MGR_ADDRESS (IPANEL_CORE_RESERVE_ADDRESS + IPANEL_CORE_RESERVE_SIZE)
#define IPANEL_NVRAM_APP_MGR_SIZE    (0x60000)  //384K

#define IPANEL_NVRAM_AUX_ADDRESS     (IPANEL_NVRAM_APP_MGR_ADDRESS + IPANEL_NVRAM_APP_MGR_SIZE )
#define IPANEL_NVRAM_AUX_SIZE        (0x10000) //64K

#define IPANEL_CA_BASE_ADDRESS     (IPANEL_NVRAM_AUX_ADDRESS + IPANEL_NVRAM_AUX_SIZE)
#define IPANEL_CA_BASE_SIZE        (0x20000)   //128K

#define IPANEL_NVRAM_QUICK_ADDRESS  (IPANEL_CA_BASE_ADDRESS + IPANEL_CA_BASE_SIZE)
#define IPANEL_NVRAM_QUICK_SIZE     (0x10000)   //64K

#define IPANEL_CORE_BASE_ADDRESS    (IPANEL_NVRAM_QUICK_ADDRESS + IPANEL_NVRAM_QUICK_SIZE)
#define IPANEL_CORE_BASE_SIZE       (0x10000)   //64k

#define IPANEL_NVRAM_UNKNOWN_ADDRESS    (IPANEL_CORE_BASE_ADDRESS + IPANEL_CORE_BASE_SIZE)
#define IPANEL_NVRAM_UNKNOWN_SIZE       (0x00)

typedef enum
{
	IPANEL_NVRAM_DATA_BASIC,		/* 基本NVRAM */
	IPANEL_NVRAM_DATA_SKIN,			/* Skin专用 */
	IPANEL_NVRAM_DATA_THIRD_PART,	/* 第三方共用 */
	IPANEL_NVRAM_DATA_QUICK,		/* 立即写入的NVRAM */
	IPANEL_NVRAM_DATA_APPMGR,  	    /* 保存下载应用程序*/
	IPANEL_NVRAM_DATA_USERDEF,		/* 保存中间件下载的用户自定义数据*/
	IPANEL_NVRAM_DATA_AUX,			/* 辅助FFS, 保存可以丢失且经常写的数据(如, cookie) */
	IPANEL_NVRAM_DATA_BOOT,			/* BOOT程序 */
	IPANEL_NVRAM_DATA_LOADER,		/* LOADER程序 */
	IPANEL_NVRAM_DATA_ROOTFFS,		/* ROOTFFS */
	IPANEL_NVRAM_DATA_SYSTEM,		/* 内核或操作系统 */
	IPANEL_NVRAM_DATA_APPSOFT,		/* APPSOFT*/
	IPANEL_NVRAM_DATA_UNKNOWN
} IPANEL_NVRAM_DATA_TYPE_e;


typedef enum
{
    IPANEL_NVRAM_FAILED     = -1,
    IPANEL_NVRAM_BURNING    =  0,
    IPANEL_NVRAM_SUCCESS    =  1,
    IPANEL_NVRAM_UNKNOWN
} IPANEL_NVRAM_STATUS_e;

typedef enum
{
    IPANEL_NVRAM_BURN_DELAY = 0,
    IPANEL_NVRAM_BURN_NOW   = 1
} IPANEL_NVRAM_BURN_MODE_e;

INT32_T ipanel_porting_nvram_info(
        BYTE_T    **addr,
        INT32_T    *num_sect,
        INT32_T    *sect_size,
        INT32_T     flag
    );

INT32_T ipanel_porting_nvram_read(UINT32_T addr, BYTE_T *buf, INT32_T len);

INT32_T ipanel_porting_nvram_burn(
        UINT32_T                    addr,
        CONST CHAR_T               *buf,
        INT32_T                     len,
        IPANEL_NVRAM_BURN_MODE_e    mode
    );

INT32_T ipanel_porting_nvram_erase(UINT32_T addr, INT32_T len);

INT32_T ipanel_porting_nvram_status(UINT32_T addr, INT32_T len);

void ipanel_erase_base_data();

INT32_T ipanel_write_ui_to_flash();

INT32_T ipanel_nvram_init( VOID );

VOID ipanel_nvram_exit( VOID );

#ifdef __cplusplus
}
#endif

#endif  // _IPANEL_MIDDLEWARE_PORTING_NVRAM_API_FUNCTOTYPE_H_

