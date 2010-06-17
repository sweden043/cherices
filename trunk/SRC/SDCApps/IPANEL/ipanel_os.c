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
#include "ipanel_os.h"

//--------------------------------------------------------------------------------------------------
// Types and defines
//--------------------------------------------------------------------------------------------------
//
#define IPANEL_LOWEST_TASK_PRIO             0
#define IPANEL_MAXIMAL_TASK_PRIO            31

#define IPANEL_MESSAGE_LENGTH               sizeof(IPANEL_QUEUE_MESSAGE)

/**********************************************************/
/*                    Necleus 操作系统                    */
/**********************************************************/
/*
    注意点:
        (1): ipanel_task 主任务必须mapping出来的值小于60
        (2): debug任务优先级相对低于ipanel task 
        (3): 其它任务优先级相对高于 ipanel task
        (4): 默认优先级是64, STEP=(+/-8)
*/
static int OsalToNativeOsMapTable_necleus[32][2] = 
{
       /*OSAL	native os priority */
		{0, 		0},
		{1, 		8},
		{2, 		12},
		{3,			16},
		{4,			24},
		{5,			32},  
		{6,			40},
		{7,			48},
		{8,			56},
		{9,			64},
		{10,		72},	
		{11,		80},
		{12,		88},
		{13,		96},
		{14,		104},
		{15,		112},  
		{16,		116},
		{17,		124},
		{18,		132},
		{19,		140},
		{20,		148},  
		{21,		156},
		{22,		164},
		{23,		172},
		{24,		180},
		{25,		188},  
		{26,		194},
		{27,		202},
		{28,		210},
		{29,		220},
		{30,		238},
		{31,		239},	
};

//--------------------------------------------------------------------------------------------------
// Internal Prototypes
//--------------------------------------------------------------------------------------------------
//
#define MAX_SEM_NUM			64

typedef struct tagOsSem
{
	unsigned int used;
	unsigned int sem;
	INT32_T initialTokenCount;
	UINT32_T taskWaitMode;
	char name[4];
}OsSem;

static OsSem m_semMgr[MAX_SEM_NUM];
	
//--------------------------------------------------------------------------------------------------
// Exported functions
//--------------------------------------------------------------------------------------------------
//
extern sem_id_t sem_create_ex(  unsigned int        initial_value,
                                const char         *name,
                                task_wait_order_t   wait_order  );

extern queue_id_t qu_create_ex( unsigned int        max_elements,
                                const char         *name,
                                int                 max_length,
                                qu_msg_type_t       msg_type,
                                task_wait_order_t   wait_order  );

extern int qu_send_ex(  queue_id_t      qu_id,
                        void           *message,
                        int             message_length,
                        u_int32         timeout_ms,
                        qu_priority_t   priority    );

extern int qu_receive_ex(queue_id_t qu_id, 
					     u_int32 timeout_ms, 
					     void *message, 
					     int message_length);

#define TASK_ENVELOP // 避免任务break出while(1)循环继续执行

#ifdef TASK_ENVELOP
typedef struct 
{
    UINT32_T task_id ;
	IPANEL_TASK_PROC func;
	VOID *param;
}IPANEL_TASK_PARAM;

static void task_envelop(void *param)
{
    IPANEL_TASK_PARAM *lparam;
    
    lparam = (IPANEL_TASK_PARAM *)param;

	(*lparam->func)(lparam->param);

    task_destroy(lparam->task_id);

    ipanel_porting_free(lparam);
}
#endif

/********************************************************************************************************
功能：创建一个线程/任务。

原型：UINT32_T ipanel_porting_task_create(CONST CHAR_T *name, IPANEL_TASK_PROC func, VOID *param,INT32_T priority, UINT32_T stack_size)
参数说明：

  输入参数：
    name：一个最多四字节长字符串，系统中线程名称应该唯一；

    func：线程主体函数入口地址，函数原型定义如下；

    typedef VOID (*IPANEL_TASK_PROC)(VOID *param);

    param：线程主体函数的参数列表指针(可置为IPANEL_NULL)；

    priority：优先级别(ipanel优先级从0到31，0最低,31最高)；

    stack_size：栈大小，以字节为单位

  输出参数：无

返    回：

  != IPANEL_NULL：成功，返回线程实例句柄。

  == IPANEL_NULL：失败

********************************************************************************************************/
UINT32_T ipanel_porting_task_create(
            CONST CHAR_T       *name,
            IPANEL_TASK_PROC    func,
            VOID               *param,
            INT32_T             priority,
            INT32_T             size
    )
{
    UINT32_T handle = IPANEL_NULL;
	INT32_T PriorityNativeOs;
#ifdef TASK_ENVELOP    
    IPANEL_TASK_PARAM *task_param;
#endif

    ipanel_porting_dprintf("[ipanel_porting_task_create] name=%s, func=%p, param=%p, prio=%d, size=0x%x\n",\
        name, func, param, priority, size);

    if (    func && (size > 0)
        && (priority >= IPANEL_LOWEST_TASK_PRIO) && (priority <= IPANEL_MAXIMAL_TASK_PRIO)  )
    {
		PriorityNativeOs = OsalToNativeOsMapTable_necleus[priority][1];

#ifdef TASK_ENVELOP
		task_param = ipanel_porting_malloc(sizeof(IPANEL_TASK_PARAM));
		if(task_param == IPANEL_NULL)
			return IPANEL_NULL;

		task_param->func = func;
		task_param->param = param;

	    handle = (UINT32_T)task_create(
	                            (PFNTASK)task_envelop,
	                            (void*)task_param,
	                            (void*)NULL,
	                            (size_t)size,
	                            (int)PriorityNativeOs,
	                            (const char*)name
	                        );	

	    task_param->task_id = handle ;
#else
	    handle = (UINT32_T)task_create(
	                            (PFNTASK)func,
	                            (void*)param,
	                            (void*)NULL,
	                            (size_t)size,
	                            (int)PriorityNativeOs,
	                            (const char*)name
	                        );
#endif

		if(NULL == handle)
		{
			ipanel_porting_dprintf("[ipanel_porting_task_create]create task failed!\n");
			return IPANEL_NULL;
		}
    }

    return handle;
}

/********************************************************************************************************
功能：销毁一个线程/任务。

原型：INT32_T ipanel_porting_task_destroy(UINT32_T task_handle)
参数说明：

  输入参数：task_handle：线程句柄(非0且存在，有效)。
  输出参数：无

返    回：

  IPANEL_OK：成功，

  IPANEL_ERR：失败

********************************************************************************************************/
INT32_T ipanel_porting_task_destroy(UINT32_T handle)
{
    INT32_T ret = IPANEL_ERR;

    ipanel_porting_dprintf("[ipanel_porting_task_destroy] handle=0x%x\n", handle);

    if (handle)
    {
        int result;

        result = task_destroy((task_id_t)handle);
        if (NU_SUCCESS == result)
        {
            ret = IPANEL_OK;
        }
        else
        {
            ipanel_porting_dprintf("[ipanel_porting_task_destroy] task_destroy error(0x%x)\n", result);
        }
    }

    return ret;
}

VOID ipanel_porting_task_sleep(INT32_T ms)
{
    if (ms > 0)
    {
        task_time_sleep((unsigned int)ms);
    }
}

UINT32_T ipanel_porting_sem_create(CONST CHAR_T *name, INT32_T count, UINT32_T mode)
{
    UINT32_T handle = IPANEL_NULL;
	int index ;

    ipanel_porting_dprintf("[ipanel_porting_sem_create] name=%s, count=%d, mode=%d\n", name, count, mode);

	for (index=0; index<MAX_SEM_NUM; index++)
	{
		if (!m_semMgr[index].used)
		{
			m_semMgr[index].initialTokenCount = count;
			m_semMgr[index].taskWaitMode = mode;
			strncpy(m_semMgr[index].name,name,4);
			m_semMgr[index].used = 1;
			break;
		}
	}

	if( MAX_SEM_NUM == index)
	{
		ipanel_porting_dprintf("[ipanel_porting_sem_create] no use semphore!\n");
		return IPANEL_NULL;
	}

    if (count >= 0)
    {
        if (IPANEL_TASK_WAIT_FIFO == mode)
        {
            handle = (UINT32_T)sem_create_ex((unsigned int)count, name, KAL_TASK_ORDER_FIFO);
        }
        else if (IPANEL_TASK_WAIT_PRIO == mode)
        {
            handle = (UINT32_T)sem_create_ex((unsigned int)count, name, KAL_TASK_ORDER_PRIORITY);
        }
    }

	if((sem_id_t)NULL == handle)
	{
		ipanel_porting_dprintf("[ipanel_porting_sem_create] sem create failed!\n");
		return IPANEL_NULL;	
	}
	
	m_semMgr[index].sem = handle;

    return (UINT32_T)(&m_semMgr[index]);
}

INT32_T ipanel_porting_sem_destroy(UINT32_T handle)
{
    INT32_T ret = IPANEL_ERR;
	OsSem *pSem = (OsSem*)handle;
	int index ;

    ipanel_porting_dprintf("[ipanel_porting_sem_destroy] handle=0x%x\n", handle);

	if (!pSem || !pSem->used)
	{
		ipanel_porting_dprintf("[ipanel_porting_task_destroy] Not Exist the Semaphore, handle = 0x%x\n", handle);
		return IPANEL_ERR;
	}

	for(index=0; index<MAX_SEM_NUM; index++)
	{
		if(m_semMgr[index].sem == pSem->sem)
		{
			break;
		}
	}

	if(index == MAX_SEM_NUM)
	{
		ipanel_porting_dprintf("[ipanel_porting_task_destroy] invalid handle!\n");
		return IPANEL_ERR;
	}
	
    if (pSem->sem)
    {
        int result ;

        result = sem_delete((sem_id_t)pSem->sem);
        if (NU_SUCCESS == result)
        {
            ret = IPANEL_OK;
        }
        else
        {
            ipanel_porting_dprintf("[ipanel_porting_sem_destroy] sem_delete error(0x%x)\n", result);
        }
    }

	pSem->initialTokenCount = 0;
	pSem->taskWaitMode = 0;
	memset(pSem->name, 0, 4);
	memset(&pSem->sem, 0, sizeof(unsigned int));
	pSem->used = 0;

    return ret;
}

INT32_T ipanel_porting_sem_wait(UINT32_T handle, INT32_T timeout)
{
    INT32_T ret = IPANEL_ERR;
	OsSem *pSem = (OsSem*)handle;
	int index ;

	if (!pSem || !pSem->used)
	{
		ipanel_porting_dprintf("[ipanel_porting_sem_wait] Not Exist the Semaphore, handle = 0x%x\n", handle);
		return IPANEL_ERR;
	}

	for(index=0; index<MAX_SEM_NUM; index++)
	{
		if(m_semMgr[index].sem == pSem->sem)
		{
			break;
		}
	}

	if(index == MAX_SEM_NUM)
	{
		ipanel_porting_dprintf("[ipanel_porting_sem_wait] invalid handle!\n");
		return IPANEL_ERR;
	}

    if (pSem->sem && (timeout > 0 || 
        IPANEL_NO_WAIT == timeout || 
        IPANEL_WAIT_FOREVER == timeout))
    {
        int result;

        result = sem_get((sem_id_t)pSem->sem, (u_int32)timeout);
        if (NU_SUCCESS == result)
        {
            ret = IPANEL_OK;
        }
    }

    return ret;
}

INT32_T ipanel_porting_sem_release(UINT32_T handle)
{
    INT32_T ret = IPANEL_ERR;
	OsSem *pSem = (OsSem*)handle;
	int index ;

	if (!pSem || !pSem->used)
	{
		ipanel_porting_dprintf("[ipanel_porting_sem_release] Not Exist the Semaphore, handle = 0x%x\n", handle);
		return IPANEL_ERR;
	}

	for(index=0; index<MAX_SEM_NUM; index++)
	{
		if(m_semMgr[index].sem == pSem->sem)
		{
			break;
		}
	}

	if(index == MAX_SEM_NUM)
	{
		ipanel_porting_dprintf("[ipanel_porting_sem_release] invalid handle!\n");
		return IPANEL_ERR;
	}

    if (pSem->sem)
    {
        int result;

        result = sem_put((sem_id_t)pSem->sem);
        if (NU_SUCCESS == result)
        {
            ret = IPANEL_OK;
        }
        else
        {
            ipanel_porting_dprintf("[ipanel_porting_sem_release] sem_put failed(0x%x)\n", result);
        }
    }

    return ret;
}

UINT32_T ipanel_porting_queue_create(CONST CHAR_T *name, UINT32_T len, UINT32_T mode)
{
    UINT32_T handle = IPANEL_NULL;

    ipanel_porting_dprintf("[ipanel_porting_queue_create] name=%s, len=%d, mode=%d\n", name, len, mode);

    if (len > 0)
    {
        if (IPANEL_TASK_WAIT_FIFO == mode)
        {
            handle = (UINT32_T)qu_create_ex(
                            (unsigned int)len,
                            name,
                            IPANEL_MESSAGE_LENGTH,
                            KAL_FIXED_LENGTH,
                            KAL_TASK_ORDER_FIFO
                        );
        }
        else if (IPANEL_TASK_WAIT_PRIO == mode)
        {
            handle = (UINT32_T)qu_create_ex(
                            (unsigned int)len,
                            name,
                            IPANEL_MESSAGE_LENGTH,
                            KAL_FIXED_LENGTH,
                            KAL_TASK_ORDER_PRIORITY
                        );
        }
    }

    return handle;
}

INT32_T ipanel_porting_queue_destroy(UINT32_T handle)
{
    INT32_T ret = IPANEL_ERR;

    ipanel_porting_dprintf("[ipanel_porting_queue_destroy] handle=0x%x\n", handle);

    if (handle)
    {
        int result;

        result = qu_destroy((queue_id_t)handle);
        if (NU_SUCCESS == result)
        {
            ret = IPANEL_OK;
        }
        else
        {
            ipanel_porting_dprintf("[ipanel_porting_queue_destroy] qu_destroy error(0x%x)\n", result);
        }
    }

    return ret;
}

INT32_T ipanel_porting_queue_send(UINT32_T handle, IPANEL_QUEUE_MESSAGE *msg)
{
    INT32_T ret = IPANEL_ERR;

    if (handle && msg)
    {
        int result;

        result = qu_send_ex(
                        (queue_id_t)handle,
                        (void*)msg,
                        IPANEL_MESSAGE_LENGTH,
                        0,
                        KAL_QMSG_PRI_NORM
                    );
        if (NU_SUCCESS == result)
        {
            ret = IPANEL_OK;
        }
        else
        {
            ipanel_porting_dprintf("[ipanel_porting_queue_send] qu_send failed(0x%x)\n", result);
        }
    }

    return ret;
}

INT32_T ipanel_porting_queue_recv(UINT32_T handle, IPANEL_QUEUE_MESSAGE *msg, INT32_T timeout)
{
    INT32_T ret = IPANEL_ERR;

    if (   handle && msg
        && ((timeout > 0) || (IPANEL_NO_WAIT == timeout) || (IPANEL_WAIT_FOREVER == timeout)) )
    {
        int result;

        result = qu_receive_ex(
                        (queue_id_t)handle,
                        (u_int32)timeout,
                        (void*)msg,
                        IPANEL_MESSAGE_LENGTH
                    );
        if (NU_SUCCESS == result)
        {
            ret = IPANEL_OK;
        }
    }

    return ret;
}

int ipanel_os_init()
{
	int i;
	
	for (i = 0; i < MAX_SEM_NUM; i++)
	{
		m_semMgr[i].used = 0;
	}

	return IPANEL_OK;
}

void ipanel_os_exit()
{
	int i;
	
	for (i = 0; i < MAX_SEM_NUM; i++)
	{
		if (m_semMgr[i].used)
		{
			sem_delete((sem_id_t)m_semMgr[i].sem);
			m_semMgr[i].initialTokenCount = 0;
			m_semMgr[i].taskWaitMode = 0;
			memset(m_semMgr[i].name, 0, 4);
			memset(&m_semMgr[i].sem, 0, sizeof(unsigned int));
			m_semMgr[i].used = 0;
		}
	}	
}

