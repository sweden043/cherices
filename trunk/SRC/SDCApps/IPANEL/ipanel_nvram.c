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
#include "ipanel_nvram.h"
#define  LD_IIB_ADDRESS		   0x2001F000

extern  int data_read ( void *addr, int count, void *buffer );
extern  int block_erase ( void *addr, int block_size );
extern  int data_write(void *addr, int count, void *buffer);

/***************************************************************************************************
功能：获得提供给上层的NVRAM(通常就是flash)内存区的起始地址，块数和每块的大小，建议给
      iPanel MiddleWare 用作 NVRAM的Flash Memory 每块合适尺寸是64k/128k/ 256kBYTE_Ts。
      但NVRAM的最小空间不得小于64k。

      注意：如果当该函数得不到NVRAM的地址和尺寸时，其他NVRAM接口函数可以放空。当flag为
      IPANEL_NVRAM_DATA_PSI_SI时，用于存放一些PSI/SI数据，一般给64K就够了。如果有换肤功能，
      当flag为IPANEL_NVRAM_DATA_SKIN时，用于存储下载的skin，一般要1到2M。当flag为
      IPANEL_NVRAM_DATA_THIRD_PART时表示提供给和iPanel MiddleWare 集成的第三方使用的NVRAM。

原型：
    INT32_T ipanel_porting_nvram_info(
            BYTE_T    **addr,
            INT32_T    *num_sect,
            INT32_T    *sect_size,
            INT32_T     flag
        )
参数说明：

  输入参数：

    flag:　要获取的数据类型

    typedef enum

    {

  	IPANEL_NVRAM_DATA_PSI_SI,

  	IPANEL_NVRAM_DATA_SKIN,

  	IPANEL_NVRAM_DATA_THIRD_PART,

    } IPANEL_NVRAM_DATA_TYPE_e;

    IPANEL_NVRAM_DATA_PSI_SI -　表示返回供PSI/SI存储的NVRAM的起始地址，块个数和每个块大小。

    IPANEL_NVRAM_DATA_SKIN -　表示返回供skin文件存储的NVRAM的起始地址，块个数和每个块的大小。

    IPANEL_NVRAM_DATA_THIRD_PART -  表示返回供第三方存储的NVRAM的起始地址，块个数和每个块的大小。

  输出参数：

    *addr：FLASH空间的起始地址；

    *sect_num：FLASH空间的块数

    *sect_size：FLASH的每块大小。

返    回：

  IPANEL_OK:成功;

  IPANEL_ERR:失败

****************************************************************************************************/
INT32_T ipanel_porting_nvram_info(
        BYTE_T    **addr,
        INT32_T    *num_sect,
        INT32_T    *sect_size,
        INT32_T     flag
    )
{
    INT32_T ret = IPANEL_ERR;

    if (addr && num_sect && sect_size)
    {
        if (IPANEL_NVRAM_DATA_BASIC == flag)
        {
            *addr       = ( unsigned char* ) IPANEL_CORE_BASE_ADDRESS;
            *num_sect   = IPANEL_CORE_BASE_SIZE / NVRAM_BLOCK_SIZE;
            *sect_size  = NVRAM_BLOCK_SIZE;

            ret         = IPANEL_OK;
        }
		else if( IPANEL_NVRAM_DATA_SKIN == flag )
		{
             *addr      = ( unsigned char* ) IPANEL_CORE_UI_ADDRESS;
             *num_sect  = IPANEL_CORE_UI_SIZE / NVRAM_BLOCK_SIZE;
             *sect_size = NVRAM_BLOCK_SIZE;

             ret        = IPANEL_OK;
		}
        else if (IPANEL_NVRAM_DATA_THIRD_PART == flag)
        {
             *addr      = ( unsigned char* ) IPANEL_CA_BASE_ADDRESS;
             *num_sect  = IPANEL_CA_BASE_SIZE / NVRAM_BLOCK_SIZE;
             *sect_size = NVRAM_BLOCK_SIZE;

             ret        = IPANEL_OK;
        }
        else if (IPANEL_NVRAM_DATA_QUICK == flag)
        {
            *addr       = (unsigned char *)IPANEL_NVRAM_QUICK_ADDRESS;
            *num_sect   = IPANEL_NVRAM_QUICK_SIZE / NVRAM_BLOCK_SIZE;
            *sect_size  = NVRAM_BLOCK_SIZE;
            ret = IPANEL_OK;
        }
		else if( IPANEL_NVRAM_DATA_APPMGR == flag)
		{
            *addr       = (unsigned char *)IPANEL_NVRAM_APP_MGR_ADDRESS;
            *num_sect   = IPANEL_NVRAM_APP_MGR_SIZE / NVRAM_BLOCK_SIZE;
            *sect_size  = NVRAM_BLOCK_SIZE;
            ret = IPANEL_OK;
		}
		else if( IPANEL_NVRAM_DATA_AUX == flag)
		{
            *addr       = (unsigned char *)IPANEL_NVRAM_AUX_ADDRESS;
            *num_sect   = IPANEL_NVRAM_AUX_SIZE / NVRAM_BLOCK_SIZE;
            *sect_size  = NVRAM_BLOCK_SIZE;
            ret = IPANEL_OK;
		}
    }

    ipanel_porting_dprintf("[ipanel_porting_nvram_info] addr=%p, num_sect=%d, sect_size=0x%x, \
                           flag=%d, ret=%d\n",*addr, *num_sect, *sect_size, flag, ret);

    return ret;
}

/***************************************************************************************************
功能：从NVRAM (通常就是flash)中读取指定的字节数到数据缓存区中，实现时注意操作空间越界。

原型：INT32_T ipanel_porting_nvram_read(UINT32_T flash_addr, BYTE_T *buff_addr, INT32_T nbytes)
参数说明：

  输入参数：
    flash_addr：想要读取的目标Flash Memory的起始地址；

    buf_addr：保存读取数据的缓冲区。

    len：想要读取的字节数。

  输出参数：buf_addr：读取的数据。

返    回：

  >=0:实际读取的数据长度;

  IPANEL_ERR：失败

****************************************************************************************************/
INT32_T ipanel_porting_nvram_read(UINT32_T addr, BYTE_T *buf, INT32_T len)
{
    INT32_T ret = IPANEL_ERR;
    INT32_T readbyte ;

    ipanel_porting_dprintf("[ipanel_porting_nvram_read]  addr=0x%x, buf=%p, len=0x%x.\n", addr, buf, len);


    if( (addr < IPANEL_CORE_UI_ADDRESS) ||
	    (addr > IPANEL_NVRAM_UNKNOWN_ADDRESS -len ) ) 
    {
        ipanel_porting_dprintf("[ipanel_porting_nvram_read] ERROR : address = 0x%p is valid! \n", addr);
        goto NVRAM_READ_FAILED;
    }

    readbyte = data_read( ( void* )addr,len, buf);
    if( readbyte == 0 )
    {
        ipanel_porting_dprintf("[ipanel_porting_nvram_read] read data failed! \n");
        goto NVRAM_READ_FAILED;
    }

    return readbyte;

NVRAM_READ_FAILED:
    return ret;

}


/***************************************************************************************************
功能: 擦除FLASH中的数据

参数说明：

  输入参数：
    flash_addr：擦除块的起始地址；

    len：擦除空间的长度。

  输出参数：无

返    回：

  IPANEL_OK:成功;

  IPANEL_ERR:失败

****************************************************************************************************/
INT32_T ipanel_porting_nvram_erase(UINT32_T addr, INT32_T len)
{
    INT32_T ret = IPANEL_ERR;

    ipanel_porting_dprintf("[ipanel_porting_nvram_erase] addr=0x%x, len=0x%x\n", addr, len);

    if( (addr < IPANEL_CORE_UI_ADDRESS) ||
	    (addr > IPANEL_NVRAM_UNKNOWN_ADDRESS -len ) ) 
    {
        ipanel_porting_dprintf("[ipanel_porting_nvram_burn] ERROR: address = 0x%p is valid \n", addr);
        goto NVRAM_ERASE_FAILED;
    }

    if (addr)
    {
        ret=block_erase((void *) addr, len);
        if(ret!=0) 
        {
        	ipanel_porting_dprintf("[ipanel_porting_nvram_erase] failed\n");
            goto NVRAM_ERASE_FAILED;
        }
    }

    return ret=IPANEL_OK;

NVRAM_ERASE_FAILED:
    return ret; 
}

/***************************************************************************************************
功能：将数据写入NVRAM(通常就是flash)中，有两种不同的操作模式：后台写和即时写模式。

      实现时注意操作空间越界问题，对于后台写入模式，建议使用新的线程来同步，并且该函数必须立即返回，
      不能阻塞，在实际处理时，通常会建立一个镜像内存区，先将数据写到内存镜像去，立即返回，然后后台
      进程/线程将镜像内存区中的数据写到NVRAM中。后台写入的flash空间的擦除操作由底层负责。

      对于即时写入模式，需要在接口中立即完成写入操作，不能异步实现。及时写入模式的flash空间的擦除操
      作由ipanel负责。

原型：INT32_T ipanel_porting_nvram_burn(
        UINT32_T                    addr,
        CONST CHAR_T               *buf,
        INT32_T                     len,
        IPANEL_NVRAM_BURN_MODE_e    mode
    )
参数说明：

  输入参数：
    flash_addr：想要写入的目标起始地址；

    buf_addr：写入数据块的起始地址。

    len：想要写入的字节数。

    mode：写入操作模式

    typedef enum

    {

      IPANEL_NVRAM_BURN_DELAY,

      IPANEL_NVRAM_BURN_NOW

    }IPANEL_NVRAM_BURN_MODE_e;

  输出参数：无

返    回：

  >=0:实际写入的数据长度;

  IPANEL_ERR：失败

********************************************************************************************************/
INT32_T ipanel_porting_nvram_burn(
        UINT32_T                    addr,
        CONST CHAR_T               *buf,
        INT32_T                     len,
        IPANEL_NVRAM_BURN_MODE_e    mode
    )
{
    INT32_T ret = IPANEL_ERR;
    int	   num_written=0;

    ipanel_porting_dprintf("[ipanel_porting_nvram_burn] addr=0x%x, buf=0x%x, len=0x%x, mode=%d\n",
        addr, buf, len, mode);


	if( (addr < IPANEL_CORE_UI_ADDRESS) ||
		(addr > IPANEL_NVRAM_UNKNOWN_ADDRESS -len ) ) 
	{
		ipanel_porting_dprintf("[ipanel_porting_nvram_burn] ERROR: address = 0x%p is valid \n", addr);
		goto NVRAM_BURN_FAILDED;
	}

    if(IPANEL_NVRAM_BURN_DELAY == mode)
    {
    	if(ipanel_porting_nvram_erase(addr, len)!=IPANEL_OK)
    	{
    		ipanel_porting_dprintf("[ipanel_porting_nvram_erase] ERROR: address = 0x%p \n", addr);
    		goto NVRAM_BURN_FAILDED;
    	}
    }

	num_written = data_write((void *)addr,len,(void *)buf ); 

    return num_written;

NVRAM_BURN_FAILDED:
    return ret;
}

/***************************************************************************************************
功能：判断真正的NVRAM(通常就是flash)的写入状态。

原型：INT32_T ipanel_porting_nvram_status(UINT32_T flash_addr, INT32_T len)
参数说明：

  输入参数：
    flash_addr - burn的起始地址；

    len        - 写入的字节数

  输出参数：无

返    回：

  IPANEL_NVRAM_BURNING：正在写；

  IPANEL_NVRAM_SUCCESS：已经写成功；

  IPANEL_NVRAM_FAILED：写失败。

***************************************************************************************************/
INT32_T ipanel_porting_nvram_status(UINT32_T addr, INT32_T len)
{
    ipanel_porting_dprintf("[ipanel_porting_nvram_status] addr=0x%x, len=0x%x\n", addr, len);

    return IPANEL_NVRAM_SUCCESS;
}

void ipanel_erase_base_data()
{
    int i,n;
    unsigned int start_addr;
    
    ipanel_porting_dprintf("[ipanel_erase_base_data] is called!\n"); 

    // App MGR data
    start_addr = IPANEL_NVRAM_APP_MGR_ADDRESS;
    n = IPANEL_NVRAM_APP_MGR_SIZE / NVRAM_BLOCK_SIZE;
    for(i=0;i<n;i++)
    {        
        ipanel_porting_nvram_erase(start_addr,NVRAM_BLOCK_SIZE);
        start_addr += NVRAM_BLOCK_SIZE;
    }

    // Aux data 
    ipanel_porting_nvram_erase(IPANEL_NVRAM_AUX_ADDRESS,NVRAM_BLOCK_SIZE);

    // CA data 
    ipanel_porting_nvram_erase(IPANEL_CA_BASE_ADDRESS, NVRAM_BLOCK_SIZE);
    ipanel_porting_nvram_erase(IPANEL_CA_BASE_ADDRESS + NVRAM_BLOCK_SIZE, NVRAM_BLOCK_SIZE);

    // Quick data
    ipanel_porting_nvram_erase(IPANEL_NVRAM_QUICK_ADDRESS, NVRAM_BLOCK_SIZE);

    // Base data
    ipanel_porting_nvram_erase(IPANEL_CORE_BASE_ADDRESS, NVRAM_BLOCK_SIZE);
}

INT32_T ipanel_nvram_init ( VOID )
{
    int ret = IPANEL_OK;

    return ret;
}

VOID ipanel_nvram_exit( VOID )
{
    
}

#ifdef IPANEL_WRITE_UI2FLASH
static unsigned char xbrowser_data[] =
{
    #include "xbrowser.h"
};

INT32_T ipanel_write_ui_to_flash()
{
	u_int8 serial[8];
	int i;
	u_int8 iibBuf[18]; 
	u_int64 nvram_serial_num;
    INT32_T  ret = IPANEL_ERR ;
    INT32_T iii, length,nblocks;
    UINT32_T start_addr;
    INT32_T  totallen ;


	read_flash((void *)LD_IIB_ADDRESS,18,iibBuf);
		
	Int2longlong(&iibBuf[6],8,&nvram_serial_num);
	for(i=0;i<18;i++){
		printf("nvram_serial_num %d,:%0x\n",i,iibBuf[i]);
	}
	printf("nvram_serial_num64:%lld\n",nvram_serial_num);
    
    totallen = sizeof(xbrowser_data);

    printf("\n FLASH_UI_ADDRESS = %x, len = %d. \n", 
                           IPANEL_CORE_UI_ADDRESS, totallen);

    length = 0;
    nblocks = totallen/NVRAM_BLOCK_SIZE;
    printf("[ipanel_write_ui_to_flash]   nblocks = %d \n", nblocks);

    start_addr = (IPANEL_CORE_UI_ADDRESS); 
    for( iii = 0 ; iii< nblocks ; iii++) 
    {
		ret = block_erase((void *)start_addr, NVRAM_BLOCK_SIZE);
		if( ret != 0)
		{
			ipanel_porting_dprintf("[ipanel_write_ui_to_flash]erase data failed!\n");
            break; 
		}

        ret = data_write((void *)start_addr,NVRAM_BLOCK_SIZE,(BYTE_T*)&xbrowser_data[length]);
		if( ret != NVRAM_BLOCK_SIZE)
		{
			ipanel_porting_dprintf("[ipanel_write_ui_to_flash]write data failed!\n");
            break; 
		}        
        start_addr += NVRAM_BLOCK_SIZE;
        length += NVRAM_BLOCK_SIZE;
        printf("[ipanel_write_ui_to_flash] sum length = %d.\n",length);
    }

	ret = block_erase((void *)start_addr, totallen - length);
	if( ret != 0)
	{
		printf("[ipanel_write_ui_to_flash]erase data failed!\n");
		return IPANEL_ERR;
	}

    ret = data_write((void *)start_addr,totallen - length,(BYTE_T*)&xbrowser_data[length]);
	if( ret != (totallen - length))
	{
		ipanel_porting_dprintf("[ipanel_write_ui_to_flash]write data failed!\n");
    }

    printf("[ipanel_write_ui_to_flash] write flash complete !\n");
	

    return IPANEL_OK;
}
#endif

