/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                 Conexant Systems Inc. (c) 2003                           */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       pc_locks.c
 *
 *
 * Description:    see ATI header below.
 *                 Initial version based on Nucleus File v2.4.4
 *
 ****************************************************************************/
/* $Header: pc_locks.c, 2, 4/2/04 10:44:09 PM, Nagaraja Kolur$
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
*       PC_LOCKS.C                                2.5              
*                                                                       
* COMPONENT                                                             
*                                                                       
*       Nucleus FILE                                                    
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       System specific locking code (user supplied).                   
*                                                                       
* DATA STRUCTURES                                                       
*                                                                       
*                                                                       
* FUNCTIONS                                                             
*                                                                       
*       pc_fs_enter                         Claim exclusive access of   
*                                            all RTFS rewsources.       
*       pc_fs_exit                          Release exclusive access of 
*                                            all RTFS resources.        
*       fs_suspend_task                     Suspend a task for one or    
*                                            more ticks.                
*       fs_lock_task                        Lock a task so it will not  
*                                            be preempted.              
*       fs_unlock_task                      Restore a task's lock state  
*                                            to what it was before      
*                                            FS_LOCK_TASK was called.   
*       fs_release                          Release all RTFS resources  
*                                            while we wait for a        
*                                            resource to become free or 
*                                            we wait for I/O completion. 
*       fs_reclaim                          Reclaim exclusive access to 
*                                            all RTFS resources after   
*                                            returning from a resource  
*                                            wait or for an I/O completion.
*       pc_alloc_lock                       Return an event handle which
*                                             may be used as an argument
*                                             to pc_wait_lock and       
*                                             pc_wake_lock.             
*       pc_wait_lock                        Wait for a message on the   
*                                            lock channel.              
*       pc_wake_lock                        Wake up anyone waiting on   
*                                            the channel.               
*       pc_drive_enter                      An API call will use a drive.
*       pc_drive_exit                       An API call has completed   
*                                            and is returning.          
*       pc_inode_enter                      The file system will be     
*                                            touching a file or         
*                                            directory at pinode.       
*       pc_inode_exit                       The directory entry at      
*                                            pinode is not being used.  
*       pc_fat_enter                        An API call will use a FAT. 
*       pc_fat_exit                         An API call has completed   
*                                            using the FAT.             
*       pc_drive_io_enter                   We are calling the block I/O 
*                                            routines.                  
*       pc_drive_io_exit                    I/O has completed.           
*       pc_generic_enter                    Generic lock routine called 
*                                            by others.                 
*       pc_generic_exit                     Exit a lock                 
*                                                                       
* DEPENDENCIES                                                          
*                                                                       
*       nucleus.h                           System definitions          
*       pcdisk.h                            File common definitions     
*                                                                       
*************************************************************************/

#include        "nucleus.h"
#include        "pcdisk.h"

#if (NUM_USERS > 1)
extern NU_SEMAPHORE NUF_FILE_SYSTEM_MUTEX;  /* File system semaphore.   */
extern _PC_BDEVSW   pc_bdevsw[];            /* Driver dispatch table.   */
extern LOCKOBJ      drive_locks[];          /* Drive locks list.        */
extern LOCKOBJ      drive_io_locks[];       /* Drive I/O locks list.    */
extern LOCKOBJ      fat_locks[];            /* FAT locks list.          */

extern UINT32 FILE_Unused_Param; /* Used to prevent compiler warnings */

/************************************************************************
*                                                                       
* FUNCTION                                                              
*                                                                       
*       pc_fs_enter                                                     
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       This routine is called by the file manager each time an API     
*       function is entered. Its complement function pc_fs_exit is called
*       when the API is exitted.                                        
*                                                                       
*       In the reference port we grab a mutex semaphore providing       
*       exclusive access to RTFS. If LOCK_METHOD is one we will release 
*       the semaphore when we call pc_fs_exit().  If LOCK_METHOD is two 
*       we will release the semaphore  when we call pc_fs_exit() AND    
*       also every time we yield waiting for I/O completion or for       
*       another thread to release a resource that we need.              
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       None.                                                           
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       A UINT16 which is later passed into pc_fs_exit() when the API   
*       call completes. In the reference port we do not use the return 
*       code.                                                           
*                                                                       
*************************************************************************/
UINT16 pc_fs_enter(VOID)                                        /*__fn__*/
{

    /* Note: in a single threaded OS kernel this is not needed. */
    NU_Obtain_Semaphore(&NUF_FILE_SYSTEM_MUTEX, NU_SUSPEND);

    return(1);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_fs_exit                                                      
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       This routine is called by the file manager each time an API     
*       funtion is left. It should provide a complemetary function to   
*       pc_fs_enter.                                                    
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       restore_state                       Not used in reference port. 
*                                                                       
* OUTPUTS                                                               
*                                                                       
*      None.                                                            
*                                                                       
*************************************************************************/
VOID pc_fs_exit(UINT16 restore_state)                           /*__fn__*/
{
    /* Note: in a single threaded OS kernel this is not needed. */
    FILE_Unused_Param = (UINT32)restore_state;  

    NU_Release_Semaphore(&NUF_FILE_SYSTEM_MUTEX);
}


/* Things we need if using fine-grained multitasking. */
#if (LOCK_METHOD == 2)
/************************************************************************
* FUNCTION                                                              
*                                                                       
*       fs_suspend_task                                                 
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       pc_read_block() has a possible race condition when two or more  
*       tasks are attempting I/O on a block which is currently being read
*       from the disk by another task. To compensate for this rare race 
*       condition the tasks NOT performing the I/O suspend themselves and
*       check the buffer when they resume. They loop this way until the 
*       task performing the I/O set the buffer valid or in error.        
*                                                                       
*       This call should suspend the current task for one or more ticks.
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
VOID fs_suspend_task(VOID)
{

    /* We can't call NU_Reliquish() here because we need to allow tasks
       of even a lower priority to run. */
    NU_Sleep(1);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       fs_lock_task                                                    
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       This is called by RTFS when a preemption would cause a race     
*       condition. The call should signal to the kernel that the current
*       task should run until it is unlocked or it yields.              
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       None.                                                           
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       A BOOLEAN that will be passed to FS_UNLOCK_TASK when RTFS       
*       releases. If the kernel function supports pushing/popping of   
*       the lock state this INT is irrelevent. Otherwise use it to      
*       decide what action if any to take in FS_UNLOCK_TASK             
*                                                                       
*************************************************************************/
INT fs_lock_task(VOID)
{
    /* This sample code asks if the task is already locked. If not it 
       locks the task. It then returns YES if the task was already
       locked. This flag will be passed to fs_unlock_task() later. 
       fs_unlock_task() will then restore the task state. 

       Note: If your kernel has a nested lock/unlock call you may just 
             use it. In this case you may simply return a constant 
             value an ignore it.
    */
OPTION      preempt_status;


    /* Change to no preemtion.  */
    preempt_status = NU_Change_Preemption(NU_NO_PREEMPT);

    return((INT) preempt_status);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       fs_unlock_task                                                  
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       This is called by RTFS preemption is safe again. The kernel     
*       should restore the lock state to what it was before FS_LOCK_TASK
*       was called.                                                     
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       state_var                           Not used                    
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       None.                                                           
*                                                                       
*************************************************************************/
VOID fs_unlock_task(INT state_var)
{

    /* This sample code complents the code in pc_lock_task()

       Note: If your kernel has a nested lock/unlock call you may just 
             use it. In this case you may simply return a constant value
             an ignore it.
      */
    state_var = state_var;

    /* Restore the previous preemption state. */
    NU_Change_Preemption((OPTION)state_var);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       fs_release                                                      
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       This is called by RTFS before it blocks on a resource. The      
*       resource may be either an I/O event in a device driver, or a     
*       locked data object such as the fat or a directory that is open  
*       for write. It releases the RTFS semaphore.                      
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
VOID fs_release(VOID)
{

    /* Note: in a single threaded OS kernel this is not needed. */
    /* Note: If you can you should try not to cause a context switch here.
             The routine calling fs_release() is going to yield 
             momentarilly. If you do cause a switch here no harm is done,
             however. */

    NU_Release_Semaphore(&NUF_FILE_SYSTEM_MUTEX);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       fs_reclaim                                                      
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       This is called by RTFS after it returns from a resource block. 
*       It reclaims RTFS resources that were released in ps_release.    
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
VOID fs_reclaim(VOID)
{

    /* Note: in a single threaded OS kernel this is not needed. */
    NU_Obtain_Semaphore(&NUF_FILE_SYSTEM_MUTEX, NU_SUSPEND);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_alloc_lock                                                   
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       This routine is called repeatedly by pc_memory_init() to        
*       allocate all of the event handles needed by RTFS.               
*                                                                       
*       It returns an event handle. This handle will be used to block   
*       and wake as resources are needed/finished.                      
*                                                                       
*       The handle type returned may be one of two types.               
*        1. A mutex semaphore for which only one task awakes from       
*           pc_wait_lock() when signaled from pc_wake_lock(). If this   
*           method is used there will be a slight performance           
*           degradation in some cases since all resource accesses will  
*           be exclusive.                                               
*        2. An event handle for which all tasks awake from pc_wait_lock()
*           when signaled from pc_wake_lock(). If this method is used   
*           RTFS will arbitrate for the resource after wait has returned.
*           This is the preferred method because it allows RTFS to      
*           differentiate between exclusive and shared access to a      
*           resource.                                                   
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       None.                                                           
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       A handle of type WAIT_HANDLE_TYPE. This is defined by you in    
*       pcdisk.h and is the native handle type of your kernel           
*       environment.                                                    
*                                                                       
*************************************************************************/
WAIT_HANDLE_TYPE pc_alloc_lock(VOID)                            /*__fn__*/
{
WAIT_HANDLE_TYPE h;
static WAIT_HANDLE_TYPE current_h = 0;


    if (current_h < NUF_FIRST_EVENT_NUMBER)
        current_h = NUF_FIRST_EVENT_NUMBER;

    h = current_h;
    current_h += 1;

    return(h);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_wait_lock                                                    
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       The file system code needs to block on handle. Wait for a wakeup
*       on the channel.                                                 
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       wait_handle                         Event group number.         
*                                                                       
* OUTPUTS                                                               
*                                                                       
*      None.                                                            
*                                                                       
*************************************************************************/
VOID pc_wait_lock(WAIT_HANDLE_TYPE wait_handle)                 /*__fn__*/
{
UNSIGNED    current_events;
UNSIGNED    events;


    events = (UNSIGNED) ~0;

    /* First release the File system semaphore. */
    fs_release();

    /* Now block waiting for an evbent on the channel. */
    NU_Retrieve_Events(&NUFP_Events[wait_handle], events,
                        NU_OR_CONSUME, &current_events, NU_SUSPEND);

    /* Now reclaim the file system semaphore. */
    fs_reclaim();
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_wake_lock                                                    
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Decrement the semaphore at lock_handle. If any callers to       
*       pc_wake_lock(handle, YES) are waiting on the semaphore they will
*       wake up if the sem count drops to zero.                         
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       wait_handle                         Event group number.         
*                                                                       
* OUTPUTS                                                               
*                                                                       
*      None.                                                            
*                                                                       
*************************************************************************/
VOID pc_wake_lock(WAIT_HANDLE_TYPE wait_handle)                 /*__fn__*/
{

    /* Signal tasks waiting in wait_lock. */
    NU_Set_Events(&NUFP_Events[wait_handle],
                    (UNSIGNED) ~0, NU_OR);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_drive_enter                                                  
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       The file manager is entering code which will touch the drive at 
*       driveno.If exclusive is true we must wait until the the drive is
*       unused by any other process. If the drive is being used         
*       exclusively by another process we must wait for the other       
*       process to release it.                                          
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       driveno                             Drive number.               
*       exclusive                           If exclusive is YES wait for
*                                            access and lock the drive. 
*                                                                       
* OUTPUTS                                                               
*                                                                       
*      None.                                                            
*                                                                       
*************************************************************************/
VOID pc_drive_enter(INT16 driveno, INT exclusive)               /*__fn__*/
{

    /* Lock a drive object. */
    pc_generic_enter(&drive_locks[driveno], exclusive);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_drive_exit                                                   
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       The file manager is leaving code which touched the drive at     
*       driveno. Makes sure any process waiting for drive is awake.     
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       driveno                             Drive number.               
*                                                                       
* OUTPUTS                                                               
*                                                                       
*      None.                                                            
*                                                                       
*************************************************************************/
VOID  pc_drive_exit(INT16 driveno)                              /*__fn__*/
{

    /* Restore a lock state. */
    pc_generic_exit(&drive_locks[driveno]);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_inode_enter                                                  
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       The file system will be touching the file or directory at pinode
*       If exclusive is true the file or directory will modified and    
*       must be locked. Otherwise we wait until no-one else has         
*       exclusive access and increase the open count.                   
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *pinode                             File directory entry.       
*       exclusive                           If exclusive is YES lock the
*                                            finode.                    
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       None.                                                           
*                                                                       
*************************************************************************/
VOID pc_inode_enter(FINODE *pinode, INT exclusive)              /*__fn__*/
{

    /* Lock a FINODE object. */
    pc_generic_enter(&pinode->lock_object, exclusive);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_inode_exit                                                   
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       The directory entry at pinode is not being used. Wake up anyone 
*       who might be waiting for it.                                    
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *pinode                             File directory entry.       
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       None.                                                           
*                                                                       
*************************************************************************/
VOID pc_inode_exit(FINODE *pinode)                              /*__fn__*/
{

    /* Restore a lock state. */
    pc_generic_exit(&pinode->lock_object);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_fat_enter                                                    
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       The file manager is entering code which will touch the FAT at   
*       driveno. Wait until the the FAT is unused by any other process. 
*       And then claim it for the operation to be performed.            
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       driveno                             Drive number.               
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       None.                                                           
*                                                                       
*************************************************************************/
VOID pc_fat_enter(INT16 driveno)                                /*__fn__*/
{

    /* Lock a FAT object. */
    pc_generic_enter(&fat_locks[driveno], YES);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_fat_exit                                                     
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       The file manager is leaving code which touched the FAT driveno. 
*       Makes sure any process waiting for FAT is awake.                
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       driveno                             Drive number.               
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       None.                                                           
*                                                                       
*************************************************************************/
VOID  pc_fat_exit(INT16 driveno)                                /*__fn__*/
{

    /* Restore a lock state. */
    pc_generic_exit(&fat_locks[driveno]);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_drive_io_enter                                               
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       The file manager is entering a block read/write call. Others may
*       wait for the I/O to complete. This routine is called to establish
*       a channel which others may wait on. pc_drive_io_exit will wake  
*       the others up.                                                  
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       driveno                             Drive number.               
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       None.                                                           
*                                                                       
*************************************************************************/
VOID pc_drive_io_enter(INT16 driveno)                           /*__fn__*/
{

    /* Convert driveno to lockno. When a non re-entrant driver shares 
       more than one drive the locks will be the same for each drive. */
    pc_generic_enter(&drive_io_locks[pc_bdevsw[driveno].lock_no], YES);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_drive_io_exit                                                
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       The file manager completed a read/write call. We call here to   
*       wake anyone blocked.                                            
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       driveno                             Drive number.               
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       None.                                                           
*                                                                       
*************************************************************************/
VOID pc_drive_io_exit(INT16 driveno)                            /*__fn__*/
{

    /* Restore a lock state. */
    pc_generic_exit(&drive_io_locks[pc_bdevsw[driveno].lock_no]);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_generic_enter                                                
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Two locking operations are provided.                            
*       One, exclusive access to the lock is needed                     
*       Two, Non-exclusive access is needed. But we must not return if  
*            someone has exclusive access.                              
*                                                                       
*       To accomplish this we observe the following.                   
*       For case one, the access count of the lock must be zero before we
*       may claim exclusive access.                                     
*       For case two, the exclusive flag must not be set.                 
*       We OR these conditions to get our wait condition. Then we up the
*       access count and set the exclusive flag if we are seeking       
*       exclusive access.                                               
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *plock                              Lock object structure.      
*       exclusive                           If exclusive is YES, wait for
*                                            access and lock the drive. 
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       None.                                                           
*                                                                       
*************************************************************************/
VOID pc_generic_enter(LOCKOBJ *plock, INT exclusive)            /*__fn__*/
{
INT         lock_state;


    /* Lock a task. */
    lock_state =  fs_lock_task();

    /* Need lock object? */
    while ( plock->exclusive || (plock->opencount && exclusive) )
    {
        /* Wait for a wait_handle to be restored. */
        pc_wait_lock(plock->wait_handle);
    }

    /* Set exclusive flag. */
    plock->exclusive = exclusive;

    /* Increment lock object opencount. */
    plock->opencount += 1;

    /* Restore a task's lock state. */
    fs_unlock_task(lock_state);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_generic_exit                                                 
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       We are releasing an object which we had already locked. We      
*       decrement the access count. If the Access count drops to zero we
*       clear the exclusive flag.                                       
*       Then we issue a wakeup on the channel to see if any calls to    
*       pc_generic_enter may return.                                    
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *plock                              Lock object structure.      
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       None.                                                           
*                                                                       
*************************************************************************/
VOID pc_generic_exit(LOCKOBJ *plock)                            /*__fn__*/
{
INT         lock_state;


    /* Lock a task. */
    lock_state =  fs_lock_task();

    /* Decrement lock object opencount. */
    plock->opencount -= 1;

    /* Check opencount. */
    if (!plock->opencount)
        plock->exclusive = NO;

    /* Restore a wait_handle. */
    pc_wake_lock(plock->wait_handle);

    /* Restore a task's lock state. */
    fs_unlock_task(lock_state);

    NU_Relinquish();
}

#endif /* LOCK_METHOD == 2 */
#endif /* NUM_USERS > 1 */


/****************************************************************************
 * Modifications:
 * $Log: 
 *  2    mpeg      1.1         4/2/04 10:44:09 PM     Nagaraja Kolur  CR(s) 
 *        8762 8763 : Merged 2.5.6 Nup File System.
 *        Test: 1. pvr demo 2. nup file test 3. CR 8181
 *        
 *  1    mpeg      1.0         8/22/03 5:37:40 PM     Dave Moore      
 * $
 * 
 *    Rev 1.0   22 Aug 2003 16:37:40   mooreda
 * SCR(s) 7350 :
 * Nucleus File sourcecode (File Locking implementation)
 * 
 *
 ****************************************************************************/

