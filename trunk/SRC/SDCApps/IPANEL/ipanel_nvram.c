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
���ܣ�����ṩ���ϲ��NVRAM(ͨ������flash)�ڴ�������ʼ��ַ��������ÿ��Ĵ�С�������
      iPanel MiddleWare ���� NVRAM��Flash Memory ÿ����ʳߴ���64k/128k/ 256kBYTE_Ts��
      ��NVRAM����С�ռ䲻��С��64k��

      ע�⣺������ú����ò���NVRAM�ĵ�ַ�ͳߴ�ʱ������NVRAM�ӿں������Էſա���flagΪ
      IPANEL_NVRAM_DATA_PSI_SIʱ�����ڴ��һЩPSI/SI���ݣ�һ���64K�͹��ˡ�����л������ܣ�
      ��flagΪIPANEL_NVRAM_DATA_SKINʱ�����ڴ洢���ص�skin��һ��Ҫ1��2M����flagΪ
      IPANEL_NVRAM_DATA_THIRD_PARTʱ��ʾ�ṩ����iPanel MiddleWare ���ɵĵ�����ʹ�õ�NVRAM��

ԭ�ͣ�
    INT32_T ipanel_porting_nvram_info(
            BYTE_T    **addr,
            INT32_T    *num_sect,
            INT32_T    *sect_size,
            INT32_T     flag
        )
����˵����

  ���������

    flag:��Ҫ��ȡ����������

    typedef enum

    {

  	IPANEL_NVRAM_DATA_PSI_SI,

  	IPANEL_NVRAM_DATA_SKIN,

  	IPANEL_NVRAM_DATA_THIRD_PART,

    } IPANEL_NVRAM_DATA_TYPE_e;

    IPANEL_NVRAM_DATA_PSI_SI -����ʾ���ع�PSI/SI�洢��NVRAM����ʼ��ַ���������ÿ�����С��

    IPANEL_NVRAM_DATA_SKIN -����ʾ���ع�skin�ļ��洢��NVRAM����ʼ��ַ���������ÿ����Ĵ�С��

    IPANEL_NVRAM_DATA_THIRD_PART -  ��ʾ���ع��������洢��NVRAM����ʼ��ַ���������ÿ����Ĵ�С��

  ���������

    *addr��FLASH�ռ����ʼ��ַ��

    *sect_num��FLASH�ռ�Ŀ���

    *sect_size��FLASH��ÿ���С��

��    �أ�

  IPANEL_OK:�ɹ�;

  IPANEL_ERR:ʧ��

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
���ܣ���NVRAM (ͨ������flash)�ж�ȡָ�����ֽ��������ݻ������У�ʵ��ʱע������ռ�Խ�硣

ԭ�ͣ�INT32_T ipanel_porting_nvram_read(UINT32_T flash_addr, BYTE_T *buff_addr, INT32_T nbytes)
����˵����

  ���������
    flash_addr����Ҫ��ȡ��Ŀ��Flash Memory����ʼ��ַ��

    buf_addr�������ȡ���ݵĻ�������

    len����Ҫ��ȡ���ֽ�����

  ���������buf_addr����ȡ�����ݡ�

��    �أ�

  >=0:ʵ�ʶ�ȡ�����ݳ���;

  IPANEL_ERR��ʧ��

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
����: ����FLASH�е�����

����˵����

  ���������
    flash_addr�����������ʼ��ַ��

    len�������ռ�ĳ��ȡ�

  �����������

��    �أ�

  IPANEL_OK:�ɹ�;

  IPANEL_ERR:ʧ��

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
���ܣ�������д��NVRAM(ͨ������flash)�У������ֲ�ͬ�Ĳ���ģʽ����̨д�ͼ�ʱдģʽ��

      ʵ��ʱע������ռ�Խ�����⣬���ں�̨д��ģʽ������ʹ���µ��߳���ͬ�������Ҹú��������������أ�
      ������������ʵ�ʴ���ʱ��ͨ���Ὠ��һ�������ڴ������Ƚ�����д���ڴ澵��ȥ���������أ�Ȼ���̨
      ����/�߳̽������ڴ����е�����д��NVRAM�С���̨д���flash�ռ�Ĳ��������ɵײ㸺��

      ���ڼ�ʱд��ģʽ����Ҫ�ڽӿ����������д������������첽ʵ�֡���ʱд��ģʽ��flash�ռ�Ĳ�����
      ����ipanel����

ԭ�ͣ�INT32_T ipanel_porting_nvram_burn(
        UINT32_T                    addr,
        CONST CHAR_T               *buf,
        INT32_T                     len,
        IPANEL_NVRAM_BURN_MODE_e    mode
    )
����˵����

  ���������
    flash_addr����Ҫд���Ŀ����ʼ��ַ��

    buf_addr��д�����ݿ����ʼ��ַ��

    len����Ҫд����ֽ�����

    mode��д�����ģʽ

    typedef enum

    {

      IPANEL_NVRAM_BURN_DELAY,

      IPANEL_NVRAM_BURN_NOW

    }IPANEL_NVRAM_BURN_MODE_e;

  �����������

��    �أ�

  >=0:ʵ��д������ݳ���;

  IPANEL_ERR��ʧ��

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
���ܣ��ж�������NVRAM(ͨ������flash)��д��״̬��

ԭ�ͣ�INT32_T ipanel_porting_nvram_status(UINT32_T flash_addr, INT32_T len)
����˵����

  ���������
    flash_addr - burn����ʼ��ַ��

    len        - д����ֽ���

  �����������

��    �أ�

  IPANEL_NVRAM_BURNING������д��

  IPANEL_NVRAM_SUCCESS���Ѿ�д�ɹ���

  IPANEL_NVRAM_FAILED��дʧ�ܡ�

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

