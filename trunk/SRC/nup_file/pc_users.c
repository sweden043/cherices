/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                     Conexant Systems Inc. (c) 2003                       */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       pc_users.c
 *
 *
 * Description:    see ATI header below.
 *                 Initial version based on Nucleus File v2.4.4
 *
 ****************************************************************************/
/* $Header: pc_users.c, 3, 4/2/04 11:20:23 PM, Nagaraja Kolur$
 ****************************************************************************/

/*************************************************************************/
/*                                                                       */
/*               Copyright Mentor Graphics Corporation 2003              */
/*                         All Rights Reserved.                          */
/*                                                                       */
/* THIS WORK CONTAINS TRADE SECRET AND PROPRIETARY INFORMATION WHICH IS  */
/* THE PROPERTY OF MENTOR GRAPHICS CORPORATION OR ITS LICENSORS AND IS   */
/* SUBJECT TO LICENSE TERMS.                                             */
/*                                                                       */
/*************************************************************************/

/*************************************************************************
* FILE NAME                                     VERSION                 
*                                                                       
*       PC_USERS.C                                2.5
*                                                                       
* COMPONENT                                                             
*                                                                       
*       Nucleus FILE                                                    
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Manage the fs_user structure code.                              
*                                                                       
* DATA STRUCTURES                                                       
*                                                                       
*       None.                                                           
*                                                                       
* FUNCTIONS                                                             
*                                                                       
*       NU_Become_File_User                 Register a task as an RTFS  
*                                            user.                      
*       NU_Release_File_User                Register task as no longer  
*                                            an RTFS user.              
*       NU_Check_File_User                  Check if task is a          
*                                            registered RTFS user.      
*       fs_current_user_structure           Current task's              
*                                            FILE_SYSTEM_USER structure 
*                                            pointer.                   
*       pc_free_all_users                   Free all cwd objects for a  
*                                            drive.                     
*                                                                       
* DEPENDENCIES                                                          
*                                                                       
*       nucleus.h                           System definitions      
*       pcdisk.h                            File common definitions     
*                                                                       
*************************************************************************/

#include    "nucleus.h"
#include    "pcdisk.h"
#include	   "file_mmu.h"
/*cnxt*/
#include <stdio.h>
#include "retcodes.h"
#include "stbcfg.h"
#include "kal.h"
#include "trace.h"
#include "ata.h"

extern PFILE_SYSTEM_USER    user_heap;      /* File system user 
                                                structure pointer.      */

#if (NUM_USERS > 1)
FILE_SYSTEM_USER            default_user;   /* File system default user 
                                                structure. */
/* Implement these macros for your kernel. */
/* Return CONTEXT_HANDLE_TYPE that uniquely represents the current task. 
   May not be zero. */
/* NUCLEUS - We set GET_CONTEXT_HANDLE() to (NU_Current_Task_ID()+1) so
   we are sure not to get a zero.  If we are using Nucleus PLUS, the task
   ID must be converted to a task pointer.  NUFP_Current_Task_ID performs
   this conversion. */

extern VOID NUFP_Remove_User(signed int task_id);
#define GET_CONTEXT_HANDLE() (NUFP_Current_Task_ID()+1)


/* Put the (UINT16) X into the task control block of the current task */

/* NUCLEUS - We use an array of ints to map nucleus tasks to RTFS user 
   structures. This is done because RTFS user structures use a lot of 
   core (200 or so bytes). And it it not necessary to have one RTFS user
   structure per nucleus task. We only need one structure per file 
   system user. When a nucleus task calls NU_Become_File_User() it will
   get a map index in this table. GET_RTFS_TASKNO() and SET_RTFS_TASKNO()
   are macros described below */

/* This array is allocated in pc_memory_init(). It has a slot for every
   Nucleus task */
extern INT16 *nu_to_rtfs_task_map;

/*  Nucleus PLUS uses task pointers not IDs.  The call to NUFP_Current_Task_ID
    makes the appropriate conversion.  */
#define SET_RTFS_TASKNO(X) nu_to_rtfs_task_map[NUFP_Current_Task_ID()] = X

/* Return the UINT16 assigned to this task by SET_RTFS_TASK_NO() If  
   SET_RTFS_TASK_NO() was never called, this routine may return a random value*/
#define GET_RTFS_TASKNO()  nu_to_rtfs_task_map[NUFP_Current_Task_ID()]

extern PFILE_SYSTEM_USER user_heap;


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       NU_Become_File_User                                             
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       In a multitasking environment this function must be called by a 
*       task before it may use the API. This reserves a user structure  
*       from the pool of user structures for the task.                  
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       None.                                                           
*                                                                       
* OUTPUTS                                                               
*                                                                       
*      YES if the task may use the file system                          
*      NO  if too many users already.                                   
*                                                                       
*************************************************************************/
INT NU_Become_File_User(VOID)                                   /*__fn__*/
{
UINT16              i;
PFILE_SYSTEM_USER   p;
OPTION              preempt_status;
CONTEXT_HANDLE_TYPE context_handle;
STATUS              ret_stat = NO;

NU_SUPERV_USER_VARIABLES  

    NU_SUPERVISOR_MODE();

    /* Change to no preemtion.  */
    preempt_status = NU_Change_Preemption(NU_NO_PREEMPT);
    /* Move file system user structure to local. */
    p = user_heap;
    if (p)
    {
        /* Get the number of context. */
        context_handle = GET_CONTEXT_HANDLE();

        /* Check NU_Become_File_User call. */
        for (i = 0; i < NUM_USERS; i++, p++)
        {
            if (p->context_handle == context_handle)
            {
                NU_Change_Preemption(preempt_status);
                ret_stat = YES;
                break;
            }
        }

        if (ret_stat == NO)
        {
            /* Remove user_heap to local. */
            p = user_heap;
            for (i = 0; i < NUM_USERS; i++, p++)
            {
                if (!p->context_handle)
                {
                    /* Initialize FILE_SYSTEM_USER structure. */
                    pc_memfill(p, sizeof(FILE_SYSTEM_USER), (UINT8) 0);

                    /* Set the number of context. */
                    p->context_handle = context_handle;
                    /* Set task id. */
                    SET_RTFS_TASKNO(i);

                    /* Restore the previous preemption state. */
                    NU_Change_Preemption(preempt_status);

                    ret_stat = YES;
                    break;
                }
            }
        }
    }

    if (ret_stat == NO)
        /* Restore the previous preemption state. */
        NU_Change_Preemption(preempt_status);

	NU_USER_MODE();

    return(ret_stat);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       NU_Check_File_User                                              
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       The API PROLOG code calls this if NUM_USERS > 1 is turned on.   
*       If the the current task is registered as an RTFS user it        
*       returns YES otherwise it returns NO and the PROLOG code causes  
*       the API call to fail.                                           
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       None.                                                           
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       YES                                 if the task may use the file
*                                            system.                    
*       NO                                  if not registered.          
*                                                                       
*************************************************************************/
INT NU_Check_File_User(VOID)                                    /*__fn__*/
{
UINT16      i;
STATUS      ret_stat;

NU_SUPERV_USER_VARIABLES  

    NU_SUPERVISOR_MODE();

    /* Get task id. */
    i = GET_RTFS_TASKNO();

    /* Check Nucleus File user. */
    if ( (i < NUM_USERS) &&
         (user_heap[i].context_handle ==  GET_CONTEXT_HANDLE()) )
    {
        ret_stat = YES;
    }
    else
    {
        pc_report_error(PCERR_BAD_USER);
        ret_stat = NO;
    }
   
      NU_USER_MODE();

    return(ret_stat);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       NU_Release_File_User                                            
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       When a task is through with RTFS it should call here. This frees
*       up a user structure so it may be used by other tasks callers to 
*       (NU_Become_File_User).                                          
*                                                                       
*       Subsequent API calls will fail if this routine has been called. 
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       None.                                                           
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       None.                                                           
*                                                                       
*************************************************************************/
VOID NU_Release_File_User(VOID)                                 /*__fn__*/
{
INT16       driveno;

NU_SUPERV_USER_VARIABLES  

    NU_SUPERVISOR_MODE();

    /* Check Nucleus File user. */
    if (NU_Check_File_User())
    {
        for (driveno = 0; driveno < NDRIVES; driveno++)
        {
            if (fs_user->lcwd[driveno])
            {
                /* Free the current directory object. */
                pc_freeobj(fs_user->lcwd[driveno]);

                /* Clear the current working directory. */
                fs_user->lcwd[driveno] = NU_NULL;
            }
        }

        /* Initialize FILE_SYSTEM_USER structure. */
        pc_memfill(fs_user, sizeof(FILE_SYSTEM_USER), (UINT8) 0);

        /*  Release the Task Pointer to Task ID conversion.  */
        NUFP_Remove_User(GET_RTFS_TASKNO());
    }

	NU_USER_MODE();
}

#else      

/************************************************************************
* FUNCTION                                                              
*                                                                       
*       NU_Become_File_User (Single User Version) 
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Has no effect in single user mode
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       None.                                                           
*                                                                       
* OUTPUTS                                                               
*                                                                       
*      Always return YES 
*                                                                       
*************************************************************************/
INT NU_Become_File_User(VOID)                                   /*__fn__*/
{
    return(YES);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       NU_Check_File_User  (Single User Version)  
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Has no effect in single user mode
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       None.                                                           
*                                                                       
* OUTPUTS                                                               
*                                                                       
*      Always return YES 
*                                                                       
*************************************************************************/
INT NU_Check_File_User(VOID)                                    /*__fn__*/
{

    return(YES);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       NU_Release_File_User  (Single User Version) 

*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Has no effect in single user mode
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       None.                                                           
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       None.                                                           
*                                                                       
*************************************************************************/
VOID NU_Release_File_User(VOID)                                 /*__fn__*/
{
	return;
}


#endif      /* NUM_USERS > 1 */


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       fs_current_user_structure                                       
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       This function is called every time the user or the file system  
*       kernel invokes the fs_user macro. It returns the task's private 
*       FILE_SYSTEM_USER structure that was allocated by calling        
*       NU_Become_File_User(). If the program has a bug in it and calls 
*       fs_user without having first called NU_Become_File_User(), a    
*       default structure is returned so the system doesn't crash, and  
*       pc_report_error() is called.                                    
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       None.                                                           
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       Current task's FILE_SYSTEM_USER structure pointer.              
*                                                                       
*************************************************************************/
PFILE_SYSTEM_USER fs_current_user_structure(VOID)
{

#if (NUM_USERS > 1)
INT16       i;


    /* Get task id. */
    i = GET_RTFS_TASKNO();

    /* Check Nucleus File user. */
    if ( (i < NUM_USERS) && (user_heap[i].context_handle 
        == GET_CONTEXT_HANDLE()) )
    {
        return(user_heap + i);
    }
    else
    {
        pc_report_error(PCERR_BAD_USER);
        /* Return the default user structure. */
        return(&default_user);
    }

#else   /* NUM_USERS == 1 */

    return(user_heap);

#endif
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_free_all_users                                               
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Free all cwd objects for a drive.                               
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       driveno                             Drive number               
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       None.                                                           
*                                                                       
*************************************************************************/
VOID pc_free_all_users(INT16 driveno)
{
PFILE_SYSTEM_USER   p; 
INT16               i;


    /* Move file system user structure to local. */
    p = user_heap;

    for (i = 0; i < NUM_USERS; i++, p++)
    {
        if (p->lcwd[driveno])
        {
            /* Free the current directory object. */
            pc_freeobj(p->lcwd[driveno]);
            /* Clear the current working directory. */
            p->lcwd[driveno] = NU_NULL;
        }
    }
}


/****************************************************************************
 * Modifications:
 * $Log: 
 *  3    mpeg      1.2         4/2/04 11:20:23 PM     Nagaraja Kolur  CR(s) 
 *        8762 8763 : Merged 2.5.6 Nup File System.
 *        Test: 1. pvr demo 2. nup file test 3. CR 8181
 *        
 *  2    mpeg      1.1         10/15/03 4:57:33 PM    Tim White       CR(s): 
 *        7660 Remove ATA header files from NUP_FILE code.
 *        
 *  1    mpeg      1.0         8/22/03 5:47:38 PM     Dave Moore      
 * $
 ****************************************************************************/

