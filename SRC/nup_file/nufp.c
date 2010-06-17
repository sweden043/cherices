/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                 Conexant Systems Inc. (c) 2003                           */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       nufp.c
 *
 *
 * Description:    see ATI header below.
 *                 Initial version based on Nucleus File v2.4.4
 *
 *
 ****************************************************************************/
/* $Header: nufp.c, 2, 4/2/04 10:07:48 PM, Nagaraja Kolur$
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
*       NUFP.C                                    2.5              
*                                                                       
* COMPONENT                                                             
*                                                                       
*       Nucleus FILE                                                    
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       This file contains the routines necessary to intitialize        
*       the Nucleus PLUS environment for file system usage.             
*                                                                       
* DATA STRUCTURES                                                       
*                                                                       
*       NUFP_TASK_POINTER                    Task ID list.              
*                                                                       
* FUNCTIONS                                                             
*                                                                       
*       NUFP_Current_Task_ID                Replaces the Nucleus PLUS   
*                                            Task Pointer to a Task ID. 
*       NUFP_Remove_User                    Responsible for removing a 
*                                            user from the              
*                                            NUFP_TASK_POINTER table.   
*                                                                       
* DEPENDENCIES                                                          
*                                                                       
*       nucleus.h                           System definitions          
*       pcdisk.h                            File common definitions     
*                                                                       
*************************************************************************/

#include        "nucleus.h"
#include        "pcdisk.h"
#if (RAMDISK)
#if (!RAMDISK_FROMPOOL)
#include        <malloc.h>
#endif
#endif

/*  The following table maintains a mapping between Nucleus PLUS pointers
    and Task IDs.  */
NU_TASK         *NUFP_TASK_POINTER[NUM_USERS];

/*  The following declarations are used for intialization of the Nucleus
    PLUS tasking environment for the Nucleus FILE system.  */


/************************************************************************
*  FUNCTION                                                             
*                                                                       
*       NUFP_Current_Task_ID                                            
*                                                                       
*  DESCRIPTION                                                          
*                                                                       
*       This function converts a Task Pointer to a Task ID.              
*                                                                       
*                                                                       
*                                                                       
*  INPUTS                                                               
*                                                                       
*       None.                                                           
*                                                                       
*  OUTPUTS                                                              
*                                                                       
*       task_id                             The id of the task that     
*                                           represents the task pointer.
*                                                                       
*************************************************************************/
INT NUFP_Current_Task_ID(VOID)
{
NU_TASK     *current_task_ptr;
INT         task_id;
STATUS      done = 0;

    /*  Get the Task Pointer to the current task. */
    current_task_ptr = NU_Current_Task_Pointer();

    /*  Search for the Task Pointer.  */
    for (task_id = 0; task_id < NUM_USERS; task_id++)
    {
        /*  If we found the entry, then return the ID.  */
        if (NUFP_TASK_POINTER[task_id] == current_task_ptr)
        {
            done = 1;
            break;
        }
    }

    if (!done)
    {
        /*  There is not one already established, so find a blank spot,
            set up the pointer, and return the ID. */
        for (task_id = 0; task_id < NUM_USERS; task_id++)
        {
            /*  If we found a blank entry, then return the ID.  */
            if (NUFP_TASK_POINTER[task_id] == NU_NULL)
            {
                /*  Save the entry so that we can find it next time.  */
                NUFP_TASK_POINTER[task_id] = current_task_ptr;

                /*  Return the associated ID.  */
                done = 1;
                break;
            }
        }
    }

    /*  We did not find an empty entry for a new task.  That means that the
        user did a no no. */
    if (!done)
        task_id = -1;

    return(task_id);
}   /*  end of NUFP_Current_Task_ID.  */


/************************************************************************
*  FUNCTION                                                             
*                                                                       
*       NUFP_Remove_User                                                
*                                                                       
*  DESCRIPTION                                                          
*                                                                       
*       This function is responsible for removing a user from the      
*       NUFP_TASK_POINTER table.                                        
*                                                                       
*                                                                       
*  INPUTS                                                               
*                                                                       
*       task_id                             Converted task ID from PLUS 
*                                            task pointer               
*                                                                       
*  OUTPUTS                                                              
*                                                                       
*       NUFP_TASK_POINTER                   Updated to remove the task  
*                                            pointed to by the ID.      
*                                                                       
*************************************************************************/
VOID NUFP_Remove_User(INT task_id)
{

    /*  Remove the task pointer.  */
    NUFP_TASK_POINTER[task_id] = NU_NULL;

}


/****************************************************************************
 * Modifications:
 * $Log: 
 *  2    mpeg      1.1         4/2/04 10:07:48 PM     Nagaraja Kolur  CR(s) 
 *        8762 8763 : Merged 2.5.6 Nup File System.
 *        Test: 1. pvr demo 2. nup file test 3. CR 8181
 *        
 *  1    mpeg      1.0         8/22/03 5:34:40 PM     Dave Moore      
 * $
 * 
 *    Rev 1.0   22 Aug 2003 16:34:40   mooreda
 * SCR(s) 7350 :
 * Nucleus File sourcecode (Inits Nucleus+ for Nucleus File usage)
 * 
 *
 ****************************************************************************/

