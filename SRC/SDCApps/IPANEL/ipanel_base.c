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

#if DISABLE_TRACE_TIMESTAMPS
#define LOG_INFO_MAX_LEN            161
#else
#define LOG_INFO_MAX_LEN            150
#endif

#define LOG_BUFFER_SIZE             4096

VOID *ipanel_porting_malloc(INT32_T size)
{
    VOID *ptr = IPANEL_NULL;

    if (size > 0)
    {
        ptr = (VOID *)mem_malloc(size);
        if (!ptr)
        {
            ipanel_porting_dprintf("[ipanel_porting_malloc] allocate memory failure.\n");
        }
    }

    ipanel_porting_dprintf("[ipanel_porting_malloc] size=0x%x  start addr = 0x%x \n", size, ptr);

    return ptr;
}

VOID ipanel_porting_free(VOID *ptr)
{
    ipanel_porting_dprintf("[ipanel_porting_free] ptr=0x%x\n", ptr);

    if (ptr)
    {
        mem_free(ptr);
    }
}

static char  log_info[LOG_BUFFER_SIZE];

INT32_T ipanel_porting_dprintf(CONST CHAR_T *fmt, ...)
{
    INT32_T ret = 0;

#if DEBUG_OUT
    int     len = 0;
    char   *ptr;
    char    ch;
    va_list args;

    va_start(args, fmt);
    ret = vsprintf(log_info, fmt, args);
    va_end(args);

    ptr = log_info;
    len = ret;

    do
    {
        if (len < LOG_INFO_MAX_LEN)
        {
            trace((char*)ptr);

            len = 0;
        }
        else
        {
            ch = ptr[LOG_INFO_MAX_LEN];
            ptr[LOG_INFO_MAX_LEN] = '\0';

            trace((char*)ptr);

            ptr[LOG_INFO_MAX_LEN] = ch;

            ptr += LOG_INFO_MAX_LEN;
            len -= LOG_INFO_MAX_LEN;
        }
    } while (len > 0);
#endif

    return ret;
}

UINT32_T ipanel_porting_time_ms(VOID)
{
    static UINT32_T system_start_time = 0;

    UINT32_T msec = 0;

    msec = get_system_time();
    if (0 == system_start_time)
    {
        system_start_time = msec;
    }

    return (msec - system_start_time);
}

int ipanel_hex_printout(const char* msg,const char* buf,
						unsigned int  len,int  wide)
{
    int i;
    int ret     = 0;
    int nTabs   = 0;
    int nlines  = 0;
    char msgbuf[128];
    const char hex_char[] = "0123456789ABCDEF";
    const unsigned char *ptr = (const unsigned char*)buf;

    if ((NULL == msg) || (NULL == buf) || ((8 != wide) && (16 != wide)))
    {
        return ret;
    }

    ipanel_porting_dprintf("%s buf=%08lx %ld bytes\n", msg, buf, len);

    /* calculate how many prefixing Tab */
    while ((*msg == '\r') || (*msg == '\n'))
    {
        msg++;              // skip prefixing '\r' or '\n'
    }

    while (*msg++ == '\t')
    {
        nTabs++;
        if (6 == nTabs)
        {
            break;
        }
    }

    if (8 == wide)
    {
        nlines = (len + 0x07) >> 3;
    }
    else if (16 == wide)
    {
        nlines = (len + 0x0f) >> 4;
    }

    for (i = 0; i < nlines; i++)
    {
        int j;
        char *dst  = msgbuf;
        int nbytes = ((int)len < wide) ? len : wide;

        for (j = 0; j < nTabs; j++)
        {
            *dst++ = '\t';
        }

        *dst++ = ' ';
        *dst++ = ' ';
        *dst++ = ' ';
        *dst++ = ' ';

        for (j = 0; j < nbytes; j++)
        {
            unsigned char ival = *ptr++;

            //一个char型变量用16进制输出，然后再输出一个空格，共占用三个字符空间
            *dst++ = hex_char[(ival >> 4) & 0x0F];
            *dst++ = hex_char[ival & 0x0F];
            *dst++ = ' ';
        }

        //输出一行数值后，输出三个空格，再输出该行数值的ASCII码字符
        //若nbytes小于一行的长度，即小于8或16，则以三个空格代替一个数值输出
        for (j = 0; j < wide - nbytes + 1; j++)
        {
            *dst++ = ' ';
            *dst++ = ' ';
            *dst++ = ' ';
        }

        ptr -= nbytes;
        for (j = 0; j < nbytes; j++)
        {
            if ((*ptr >= 0x20) && (*ptr <= 0x7e))
            {
                *dst = *ptr;
            }
            else
            {
                *dst = '.';
            }
            ptr++;
            dst++;
        }
        *dst = 0;
        ret += ipanel_porting_dprintf("%s\n", msgbuf);
        len -= nbytes;
    }

    return ret;
}

void ipanel_test_time_ms()
{
    int t0, t1, cnt = 0;
	
	t0 = ipanel_porting_time_ms();
	while (1)
	{
		t1 = ipanel_porting_time_ms();
		if ((t1 - t0) >= 1000)
	    {
	    	ipanel_porting_dprintf("time second: %d\n", cnt++);
	    	t0 = t1;
	    }
    }
}

/***********************************************************
    0802 log:
    [ipanel_porting_malloc]malloc 0x16E8000 bytes start from 0x009128F4.
    pointer=0x009128F4,max size = 0x16E8000. //22.9M 
***********************************************************/
void ipanel_mem_test()
{
#define IPANEL_MAX_MEM 0x2000000 //32M
#define IPANEL_STEP_MEM 0x8000   //512K 
    int mem_size ;

    void *pointer = NULL ;

    mem_size = IPANEL_MAX_MEM ;
    while(1)
    {
        pointer = ipanel_porting_malloc(mem_size);

        if( IPANEL_NULL == pointer)
        {
            mem_size -= IPANEL_STEP_MEM; 
        }
        else
        {
            ipanel_porting_dprintf("pointer=0x%x,max size = 0x%x.\n", pointer,mem_size);
            break;
        }
    }
}

