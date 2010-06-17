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
#ifndef _IPANEL_MIDDLEWARE_PORTING_TASK_API_FUNCTOTYPE_H_
#define _IPANEL_MIDDLEWARE_PORTING_TASK_API_FUNCTOTYPE_H_

#define IPANEL_MAIN_TASK_NAME                   "EIST"
#define IPANEL_MAIN_TASK_PRIORITY               8
#define IPANEL_MAIN_TASK_STACK_SIZE             0x20000

#define IPANEL_REBOOT_TASK_STACK_SIZE           0x1000
#define IPANEL_REBOOT_TASK_PRIORITY             10 
#define IPANEL_REBOOT_TASK_NAME                 "REST"

#define IPANEL_TUNER_TASK_NAME                  "TNRT"
#define IPANEL_TUNER_TASK_PRIORITY              13
#define IPANEL_TUNER_TASK_STACK_SIZE            0x1000

#define IPANEL_SOUND_TASK_NAME                  "SNDT"
#define IPANEL_SOUND_TASK_PRIORITY              20
#define IPANEL_SOUND_TASK_STACK_SIZE            0x2000

#define IPANEL_AV_MONITOR_NAME                  "AVMT"
#define IPANEL_AV_MONITOR_PRIORITY              14
#define IPANEL_AV_MONITOR_STACK_SIZE            0x1000

#define IPANEL_NETCHECK_TASK_NAME               "NETS"
#define IPANEL_NETCHECK_TASK_PRIORITY           9
#define IPANEL_NETCHECK_TASK_STACK_SIZE         0x1000

#endif  // _IPANEL_MIDDLEWARE_PORTING_TASK_API_FUNCTOTYPE_H_

