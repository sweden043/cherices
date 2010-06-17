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
#include "ipanel_eeprom.h"

#if EEPROM_TYPE == EEPROM_32KB
#define IPANEL_EEPROM_SIZE  (0x8000>>3) //32Kbit or 16Kbit
#else
#define IPANEL_EEPROM_SIZE  (0x4000>>3) //32Kbit or 16Kbit
#endif

#define IPANEL_EEPROM_ADDR  (0x0000)

extern u_int16 ee_read(u_int16 address, voidF buffer, u_int16 count, void* private);
extern u_int16 ee_write(u_int16 address, voidF buffer, u_int16 count, void* private);

INT32_T ipanel_porting_eeprom_info(BYTE_T **addr, INT32_T *size)
{
    INT32_T ret = IPANEL_ERR;

    if (addr && size)
    {
		*addr = IPANEL_EEPROM_ADDR;
		*size = IPANEL_EEPROM_SIZE;
    }

    ipanel_porting_dprintf("[ipanel_porting_eeprom_info] addr=%p, size=0x%x\n", *addr,* size);

    return ret;
}

INT32_T ipanel_porting_eeprom_read(UINT32_T addr, BYTE_T *buf, INT32_T len)
{
    INT32_T ret = IPANEL_ERR;

    ipanel_porting_dprintf("[ipanel_porting_eeprom_read] addr=0x%x, buf=%p, len=0x%x\n", addr, buf, len);

    if (addr && buf && (len > 0))
    {
		ee_read((u_int16)addr,(void*)buf,(u_int16)len,NULL);
    }

    return ret;
}

INT32_T ipanel_porting_eeprom_burn(UINT32_T addr, CONST BYTE_T *buf, INT32_T len)
{
    INT32_T ret = IPANEL_ERR;

    ipanel_porting_dprintf("[ipanel_porting_eeprom_burn] addr=0x%x, buf=%p, len=0x%x\n", addr, buf, len);

    if (addr && buf && (len > 0))
    {
		ee_write((u_int16)addr,(void*)buf,(u_int16)len,NULL);
    }

    return ret;
}

