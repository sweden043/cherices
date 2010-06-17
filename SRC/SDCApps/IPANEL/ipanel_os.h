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
#ifndef _IPANEL_MIDDLEWARE_PORTING_OS_API_FUNCTOTYPE_H_
#define _IPANEL_MIDDLEWARE_PORTING_OS_API_FUNCTOTYPE_H_

#include "ipanel_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------------------------------------
//  CONSTANTS DEFINITION
//--------------------------------------------------------------------------------------------------
//
#define IPANEL_TASK_WAIT_FIFO               0
#define IPANEL_TASK_WAIT_PRIO               1

#define IPANEL_NO_WAIT                      0
#define IPANEL_WAIT_FOREVER                -1


//--------------------------------------------------------------------------------------------------
//  TYPES DEFINITION
//--------------------------------------------------------------------------------------------------
//
typedef VOID (*IPANEL_TASK_PROC)(VOID *param);

typedef struct
{
    UINT32_T    q1stWordOfMsg;
    UINT32_T    q2ndWordOfMsg;
    UINT32_T    q3rdWordOfMsg;
    UINT32_T    q4thWordOfMsg;
} IPANEL_QUEUE_MESSAGE;


//--------------------------------------------------------------------------------------------------
//  EXTERN DATA DEFINITION
//--------------------------------------------------------------------------------------------------
//


//--------------------------------------------------------------------------------------------------
//  EXTERN FUNCTION PROTOTYPES
//--------------------------------------------------------------------------------------------------
//
UINT32_T ipanel_porting_task_create(
            CONST CHAR_T       *name,
            IPANEL_TASK_PROC    func,
            VOID               *param,
            INT32_T             priority,
            INT32_T             size
    );

INT32_T ipanel_porting_task_destroy(UINT32_T handle);

VOID ipanel_porting_task_sleep(INT32_T ms);

UINT32_T ipanel_porting_sem_create(CONST CHAR_T *name, INT32_T count, UINT32_T mode);

INT32_T ipanel_porting_sem_destroy(UINT32_T handle);

INT32_T ipanel_porting_sem_wait(UINT32_T handle, INT32_T timeout);

INT32_T ipanel_porting_sem_release(UINT32_T handle);

UINT32_T ipanel_porting_queue_create(CONST CHAR_T *name, UINT32_T len, UINT32_T mode);

INT32_T ipanel_porting_queue_destroy(UINT32_T handle);

INT32_T ipanel_porting_queue_send(UINT32_T handle, IPANEL_QUEUE_MESSAGE *msg);

INT32_T ipanel_porting_queue_recv(UINT32_T handle, IPANEL_QUEUE_MESSAGE *msg, INT32_T timeout);

int ipanel_os_init();

void ipanel_os_exit();

#ifdef __cplusplus
}
#endif

#endif  // _IPANEL_MIDDLEWARE_PORTING_OS_API_FUNCTOTYPE_H_

