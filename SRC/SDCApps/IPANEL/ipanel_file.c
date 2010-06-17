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
#include "ipanel_file.h"

//--------------------------------------------------------------------------------------------------
// Exported functions
//--------------------------------------------------------------------------------------------------
//
UINT32_T ipanel_porting_file_open(CONST CHAR_T *name, CONST CHAR_T *mode)
{
    UINT32_T fd = IPANEL_NULL;

    ipanel_porting_dprintf("[ipanel_porting_file_open] name=%s, mode=%s\n", name, mode);

    if (name && mode)
    {
    }

    return fd;
}

INT32_T ipanel_porting_file_close(UINT32_T fd)
{
    INT32_T ret = IPANEL_ERR;

    ipanel_porting_dprintf("[ipanel_porting_file_close] fd=0x%x\n", fd);

    if (fd)
    {
    }

    return ret;
}

INT32_T ipanel_porting_file_delete(CONST CHAR_T *name)
{
    INT32_T ret = IPANEL_ERR;

    ipanel_porting_dprintf("[ipanel_porting_file_delete] name=%s\n", name);

    if (name)
    {
    }

    return ret;
}

INT32_T ipanel_porting_file_read(UINT32_T fd, BYTE_T *buf, INT32_T len)
{
    INT32_T ret = IPANEL_ERR;

    ipanel_porting_dprintf("[ipanel_porting_file_read] fd=0x%x, buf=%p, len=0x%x\n", fd, buf, len);

    if (fd && buf && (len > 0))
    {
    }

    return ret;
}

INT32_T ipanel_porting_file_write(UINT32_T fd, CONST BYTE_T *buf, INT32_T len)
{
    INT32_T ret = IPANEL_ERR;

    ipanel_porting_dprintf("[ipanel_porting_file_write] fd=0x%x, buf=%p, len=0x%x\n", fd, buf, len);

    if (fd && buf && (len > 0))
    {
    }

    return ret;
}

INT32_T ipanel_porting_file_ioctl(UINT32_T fd, IPANEL_FILE_IOCTL_e op, VOID *arg)
{
    INT32_T ret = IPANEL_OK;

    ipanel_porting_dprintf("[ipanel_porting_file_ioctl] fd=0x%x, op=%d, arg=%p\n", fd, op, arg);

    switch (op)
    {
        case IPANEL_FILE_SEEK_SET :
            break;

        case IPANEL_FILE_SEEK_CUR :
            break;

        case IPANEL_FILE_SEEK_END :
            break;

        case IPANEL_FILE_TRUNCATE :
            break;

        case IPANEL_FILE_FLUSH :
            break;

        case IPANEL_FILE_GET_POSITION :
            break;

        case IPANEL_FILE_GET_CREATE_TIME :
            break;

        case IPANEL_FILE_GET_MODIFY_TIME :
            break;

        default :
            ret = IPANEL_ERR;
    }

    return ret;
}

INT32_T ipanel_porting_dir_create(CONST CHAR_T *name)
{
    INT32_T ret = IPANEL_ERR;

    ipanel_porting_dprintf("[ipanel_porting_dir_create] name=%s\n", name);

    if (name)
    {
    }

    return ret;
}

INT32_T ipanel_porting_dir_remove(CONST CHAR_T *name)
{
    INT32_T ret = IPANEL_ERR;

    ipanel_porting_dprintf("[ipanel_porting_dir_remove] name=%s\n", name);

    if (name)
    {
    }

    return ret;
}

UINT32_T ipanel_porting_dir_open(CONST CHAR_T *name)
{
    UINT32_T dd = IPANEL_NULL;

    ipanel_porting_dprintf("[ipanel_porting_dir_open] name=%s\n", name);

    if (name)
    {
    }

    return dd;
}

INT32_T ipanel_porting_dir_close(UINT32_T dd)
{
    INT32_T ret = IPANEL_ERR;

    ipanel_porting_dprintf("[ipanel_porting_dir_close] dd=0x%x\n", dd);

    if (dd)
    {
    }

    return ret;
}

INT32_T ipanel_porting_dir_rewind(UINT32_T dd)
{
    INT32_T ret = IPANEL_ERR;

    ipanel_porting_dprintf("[ipanel_porting_dir_rewind] dd=0x%x\n", dd);

    if (dd)
    {
    }

    return ret;
}

INT32_T ipanel_porting_dir_read(UINT32_T dd, IPANEL_DIR *pdir)
{
    INT32_T ret = IPANEL_ERR;

    ipanel_porting_dprintf("[ipanel_porting_dir_read] dd=0x%x, pdir=%p\n", dd, pdir);

    if (dd)
    {
    }

    return ret;
}

INT32_T ipanel_porting_dir_ioctl(IPANEL_DIR_IOCTL_e op, VOID *arg)
{
    INT32_T ret = IPANEL_OK;

    ipanel_porting_dprintf("[ipanel_porting_dir_ioctl] op=%d, arg=%p\n", op, arg);

    switch (op)
    {
        case IPANEL_DIR_RENAMCE_NODE :
            break;

        default :
            ret = IPANEL_ERR;
    }

    return ret;
}

