/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                 Conexant Systems Inc. (c) 2003                           */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       pc_memry.c
 *
 *
 * Description:    see ATI header below.
 *                 Initial version based on Nucleus File v2.4.4
 *
 ****************************************************************************/
/* $Header: pc_memry.c, 2, 4/2/04 11:02:33 PM, Nagaraja Kolur$
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
*       PC_MEMRY.C                                2.5
*                                                                       
* COMPONENT                                                             
*                                                                       
*       Nucleus FILE                                                    
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       System specific memory management routines.                     
*                                                                       
* DATA STRUCTURES                                                       
*                                                                       
*       drive_locks                         Drive locks list.           
*       fat_locks                           FAT locks list.             
*       drive_io_locks                      Drive I/O locks list.       
*       finode_lock_handles                 FINODE lock list.           
*       handles_are_alloced                 Memory allocate flag        
*                                            (pc_memory_init use).      
*       user_heap                           User heap.                  
*       *inoroot                            Beginning of inode pool.     
*       Task_Map                            Task map list.              
*       *nu_to_rtfs_task_map                File system user task map.  
*       *mem_block_pool                     Memory block pool list.     
*       *mem_file_pool                      Memory file pool list.      
*       *mem_drobj_pool                     Memory DROBJ pool list.     
*       *mem_drobj_freelist                 Memory DROBJ free list.     
*       *mem_finode_pool                    Memory FINODE pool list.    
*       *mem_finode_freelist                Memory FINODE free list.    
*       *NUF_Drive_Pointers                 File system Driver pointer. 
*       NUF_Fat_Type                        Drive FAT type list.        
*       NUFP_Events                         File system event group list.
*       NUF_FILE_SYSTEM_MUTEX               File system semaphore.      
*                                                                       
* FUNCTIONS                                                             
*                                                                       
*       NUF_Alloc                           Allocate memory.            
*       pc_memory_init                      This routine must be called 
*                                            before any file system     
*                                            routines.                  
*       pc_memory_close                     Free all memory used by     
*                                            the file system and make it
*                                            ready to run again.         
*       pc_memory_drobj                     If called with a null       
*                                            pointer, allocates and     
*                                            zeros the space needed to 
*                                            store a DROBJ structure.   
*       pc_memory_finode                    If called with a null       
*                                            pointer, allocates and     
*                                            zeros the space needed to 
*                                            store a FINODE structure.  
*                                                                       
* DEPENDENCIES                                                          
*                                                                       
*       nucleus.h                           System definitions          
*       pcdisk.h                            File common definitions     
*                                                                       
*************************************************************************/

#include        "nucleus.h"
#include        "pcdisk.h"
#include "retcodes.h"
#include "stbcfg.h"
#include "kal.h"
#include "trace.h"
#include  "ata.h"


/* Things we need if using fine-grained multitasking */
#if (LOCK_METHOD == 2)
/* Semaphore handles used for reentrancy control on fats, drives, and finodes */
LOCKOBJ                 drive_locks[NDRIVES];
LOCKOBJ                 fat_locks[NDRIVES];
LOCKOBJ                 drive_io_locks[NDRIVES];
WAIT_HANDLE_TYPE        finode_lock_handles[NFINODES];
INT                     handles_are_alloced = NO;
#endif 


/* List of users. See porting guide and pc_users.c. The only user 
   structure in single tasking environments  */
PFILE_SYSTEM_USER       user_heap = 0;
FINODE                  *inoroot = 0;             /* Begining of inode pool */
INT16                   Task_Map[NUM_USERS];
INT16                   *nu_to_rtfs_task_map;
BLKBUFF                 *mem_block_pool = 0;
PC_FILE                 *mem_file_pool = 0;
DROBJ                   *mem_drobj_pool = 0;
DROBJ                   *mem_drobj_freelist = 0;
FINODE                  *mem_finode_pool = 0;
FINODE                  *mem_finode_freelist = 0;
UNSIGNED                *NUF_Drive_Pointers[NDRIVES];
INT                     NUF_Fat_Type[NDRIVES]; 

/*  The following is a definition which allows the Event IDs used
    by Nucleus PLUS.  */
NU_EVENT_GROUP      NUFP_Events[NUF_NUM_EVENTS];

/*  The following is the definition of the Semaphore used by Nucleus FILE. */
NU_SEMAPHORE            NUF_FILE_SYSTEM_MUTEX;

/* extern  NU_MEMORY_POOL  System_Memory; */
extern  NU_MEMORY_POOL  gSystemMemory;
extern  INT             NUF_Drive_Fat_Size[NDRIVES];


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_memory_init                                                  
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       This routine must be called before any file system routines.    
*       Its job is to allocate tables needed by the file system. We     
*       chose to implement memory management this way to provide maximum
*       flexibility for embedded system developers. In the reference    
*       port we use malloc to allocate the various chunks of memory we  
*       need, but we could just have easily compiled the tables into the 
*       BSS section of the program.                                     
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       None.                                                           
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       YES on success or no ON Failure.                                
*                                                                       
*************************************************************************/
INT pc_memory_init(VOID)
{
INT16       i;
INT16       j;
UNSIGNED    pool_size;
UINT16      event_id;
DROBJ       *pobj;
FINODE      *pfi;
OPTION      preempt_status;
#if (RAMDISK)
#if (RAMDISK_FROMPOOL)
            void *pointer;
#endif
#endif
    preempt_status = NU_Change_Preemption(NU_NO_PREEMPT);

    /* Check if already initialized. If so don't do it again */
    if (user_heap)
    {
        /* Restore the previous preemption state. */
        NU_Change_Preemption(preempt_status);
        return(YES);
    }

    /*  Initialize all of the Events.  We don't know how many events the
        user is going to define because he may or may not use the IDE,
        FLOPPY, or RAMDISK drivers. */
    for (event_id = 0; event_id < NUF_NUM_EVENTS; event_id++)
    {

        /*  Create the Event Group. */
        if (NU_Create_Event_Group(&NUFP_Events[event_id],
                                    "EVENT") != NU_SUCCESS)
            return(NO);

    }

    /*  Initialize all of the Semaphores.  */
    if (NU_Create_Semaphore(&NUF_FILE_SYSTEM_MUTEX, "SEM 0", 1,
                                    NU_FIFO) != NU_SUCCESS)
        return(NO);


/* NUCLEUS. Event handles are simple indeces under Nucleus */

    mem_block_pool      = NU_NULL;
    mem_file_pool       = NU_NULL;
    mem_drobj_pool      = NU_NULL;
    mem_drobj_freelist  = NU_NULL;
    mem_finode_pool     = NU_NULL;
    mem_finode_freelist = NU_NULL;
    user_heap           = NU_NULL;
    nu_to_rtfs_task_map = NU_NULL;

/* NUCLEUS - The current user is a macro not a constant */

#if (RAMDISK)
#if (RAMDISK_FROMPOOL)
    /*  Create the RAMDISK Partition. */
#define POOL_SIZE \
    ((unsigned)(((unsigned)NUM_RAMDISK_PAGES) * \
                (((unsigned)NUF_RAMDISK_PARTITION_SIZE) + \
                 ((unsigned)PARTITION_SIZE))))

    /*if (NU_Allocate_Memory(&System_Memory, &pointer,*/
    if (NU_Allocate_Memory(&gSystemMemory, &pointer,
                           POOL_SIZE + ALLOC_SIZE,
                           NU_NO_SUSPEND) != NU_SUCCESS)
        return(NO);

    if (NU_Create_Partition_Pool(&NUF_RAMDISK_PARTITION, "RAMDISK",
                                pointer, POOL_SIZE,
                                NUF_RAMDISK_PARTITION_SIZE,
                                NU_FIFO) != NU_SUCCESS)
        return(NO);
#endif
#endif

    /* Allocate all event handles */
#if (LOCK_METHOD == 2)
    /* Allocate all message handles needed by RTFS. */
    /* If they were already alloced, we don't allocate them here, we just
       check that they are still valid */
    for (i = 0; i < NDRIVES; i++)
    {
        NUF_Drive_Pointers[i] = (UNSIGNED *)0;
        NUF_Fat_Type[i] = 0;

        if (!handles_are_alloced)
            drive_locks[i].wait_handle = pc_alloc_lock();
        drive_locks[i].opencount = 0;
        drive_locks[i].exclusive = NO;
        if (!handles_are_alloced)
            fat_locks[i].wait_handle =  pc_alloc_lock();
        fat_locks[i].opencount = 0;
        fat_locks[i].exclusive = NO;
        if (!handles_are_alloced)
            drive_io_locks[i].wait_handle =  pc_alloc_lock();
        drive_io_locks[i].opencount = 0;
        drive_io_locks[i].exclusive = NO;
    }
    for (i = 0; i < NFINODES; i++)
    {
        if (!handles_are_alloced)
            finode_lock_handles[i] = pc_alloc_lock();
    }
    handles_are_alloced = YES;

#endif  /* LOCK_METHOD == 2 */

    /* Initialize our user list - managed by code in pc_users.c */
      pool_size =sizeof(FILE_SYSTEM_USER);
      pool_size *= NUM_USERS;
      
    /*if (NU_Allocate_Memory(&System_Memory, */
    if (NU_Allocate_Memory(&gSystemMemory, 
                                (VOID **)&user_heap, 
                                pool_size,
                                NU_NO_SUSPEND) != NU_SUCCESS)

    {
        user_heap = NU_NULL;
        goto meminit_failed;
    }
    pc_memfill(user_heap, (INT)pool_size, (UINT8) 0);
    /* Point fs_user at the heap. If NUM_USERS is one it will stay forever */
    /* NUCLEUS - The current user is a macro not a constant */

    nu_to_rtfs_task_map = Task_Map;



    /* Allocate user file structures. Set each structure's is_free 
       field to YES. The file structure allocator uses this field to
       determine if a file is available for use */
    pool_size = sizeof(PC_FILE); 
    pool_size *= NUSERFILES;
    /*if (NU_Allocate_Memory(&System_Memory, */
    if (NU_Allocate_Memory(&gSystemMemory, 
                            (VOID **)&mem_file_pool,
                            pool_size,
                            NU_NO_SUSPEND) != NU_SUCCESS)
    {
        mem_file_pool = NU_NULL;
        goto meminit_failed;
    }
    for (i = 0; i < NUSERFILES; i++)
        mem_file_pool[i].is_free = YES;

    /* Allocate block buffer pool and make a null terminated list
       linked with pnext. The block buffer pool code manages this list
       directly */
    pool_size = sizeof(BLKBUFF);
    pool_size *= NBLKBUFFS;
	mem_block_pool = (BLKBUFF *)mem_nc_malloc(pool_size);
	if( !mem_block_pool )
    {
        mem_block_pool = NU_NULL;
        goto meminit_failed;
    }

    for (i = 0, j = 1; i < (NBLKBUFFS-1); i++, j++)
    {
        pc_memfill(&mem_block_pool[i], sizeof(BLKBUFF), (UINT8) 0);
        mem_block_pool[i].pnext = mem_block_pool + j;
    }
    pc_memfill(&mem_block_pool[NBLKBUFFS-1], sizeof(BLKBUFF), (UINT8) 0);
    mem_block_pool[NBLKBUFFS-1].pnext = NU_NULL;

    /* Allocate DROBJ structures and make a NULL terminated freelist using
       pdrive as the link. This linked freelist structure is used by the
       DROBJ memory allocator routine. */
    pool_size = sizeof(DROBJ); 
    pool_size *= NDROBJS;
    if (NU_Allocate_Memory(&gSystemMemory, 
                            (VOID **)&mem_drobj_pool,
                            pool_size,
                            NU_NO_SUSPEND) != NU_SUCCESS)
    {
        mem_drobj_pool = NU_NULL;
        goto meminit_failed;
    }
    mem_drobj_freelist = mem_drobj_pool;
    for (i = 0, j = 1; i < (NDROBJS-1); i++, j++)
    {
        pobj = mem_drobj_freelist + j;
        mem_drobj_freelist[i].pdrive = (DDRIVE *) pobj;
    }
    mem_drobj_freelist[NDROBJS-1].pdrive = (DDRIVE *) NU_NULL;

    /* Allocate FINODE structures and make a NULL terminated freelist using
       pnext as the link. This linked freelist is used by the FINODE 
       memory allocator routine */
    pool_size = sizeof(FINODE);
    pool_size *= NFINODES;
    if (NU_Allocate_Memory(&gSystemMemory, 
                            (VOID **)&mem_finode_pool,
                            pool_size,
                            NU_NO_SUSPEND) != NU_SUCCESS)
    {
        mem_finode_pool = NU_NULL;
        goto meminit_failed;
    }
    mem_finode_freelist = mem_finode_pool;

#if (LOCK_METHOD == 2)
    /* Copy lock handles into our new finode structures */
    for (i = 0,pfi = mem_finode_freelist; i < NFINODES; i++, pfi++)
        pfi->lock_object.wait_handle = finode_lock_handles[i];
#endif  /* LOCK_METHOD == 2 */

    pfi = mem_finode_freelist = mem_finode_pool;
    for (i = 0; i < (NFINODES-1); i++)
    {
        pfi++;
        mem_finode_freelist->pnext = pfi;
        mem_finode_freelist++;
        mem_finode_freelist->pnext = NU_NULL;
    }
    mem_finode_freelist = mem_finode_pool;

   /* Restore the previous preemption state. */
   NU_Change_Preemption(preempt_status);

   return(YES);

meminit_failed:
    /* Now deallocate all of our internal structures. */
    if (mem_block_pool)
        NU_Deallocate_Memory((VOID *)mem_block_pool);
    if (mem_file_pool)
        NU_Deallocate_Memory((VOID *)mem_file_pool);
    if (mem_drobj_pool)
        NU_Deallocate_Memory((VOID *)mem_drobj_pool);
    if (mem_finode_pool)
        NU_Deallocate_Memory((VOID *)mem_finode_pool);
    if (user_heap)
        NU_Deallocate_Memory((VOID *)user_heap);
    /* Clear all pointer. */
    user_heap = NU_NULL;
    nu_to_rtfs_task_map = NU_NULL;
    mem_block_pool = NU_NULL;
    mem_file_pool = NU_NULL;
    mem_drobj_pool = NU_NULL;
    mem_drobj_freelist = NU_NULL;
    mem_finode_pool = NU_NULL;
    mem_finode_freelist = NU_NULL;
/* NUCLEUS - The current user is a macro not a constant */

    NU_Change_Preemption(preempt_status);
    pc_report_error(PCERR_INITALLOC);
    return(NO);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_memory_close                                                 
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Free all memory used by the file system and make it ready to run
*       again.                                                          
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
VOID pc_memory_close(VOID)                                      /*__fn__*/
{

    /* Clear a few values. This allows us to close down all memory used
       by the file system an then re-activate it by calling 
       pc_memory_init. */
    inoroot = NU_NULL;

    /* Now deallocate all of our internal structures */
    if (mem_block_pool)
	mem_nc_free(mem_block_pool);
    if (mem_file_pool)
        NU_Deallocate_Partition((VOID *)mem_file_pool);
    if (mem_drobj_pool)
        NU_Deallocate_Partition((VOID *)mem_drobj_pool);
    if (mem_finode_pool)
        NU_Deallocate_Partition((VOID *)mem_finode_pool);
    if (user_heap)
        NU_Deallocate_Partition((VOID *)user_heap);
    /* Clear all pointer. */
    user_heap           = NU_NULL;
    nu_to_rtfs_task_map = NU_NULL;
    mem_block_pool      = NU_NULL;
    mem_file_pool       = NU_NULL;
    mem_drobj_pool      = NU_NULL;
    mem_drobj_freelist  = NU_NULL;
    mem_finode_pool     = NU_NULL;
    mem_finode_freelist = NU_NULL;
/* NUCLEUS - The current user is a macro not a constant */
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_memory_drobj                                                 
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       If called with a null pointer, allocates and zeros the space   
*       needed to store a DROBJ structure. If called with a NON-NULL    
*       pointer, the DROBJ structure is returned to the heap.            
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *pobj                               Drive object structure      
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       If an ALLOC, returns a valid pointer or NU_NULL if no more core. 
*       If a free, the return value is the input.                        
*                                                                       
*************************************************************************/
DROBJ *pc_memory_drobj(DROBJ *pobj)
{
DROBJ       *preturn;
DROBJ       *ret_val;

    if (pobj)
    {
        /* Free it by putting it at the head of the freelist 
           NOTE: pdrive is used to link the freelist */
        /* Set the next DDRIVE pointer. */
        pobj->pdrive = (DDRIVE *) mem_drobj_freelist;

        /* Set DROBJ memory pool free list. */
        mem_drobj_freelist = pobj;

        ret_val = pobj;
    }
    else
    {
        /* Alloc: return the first structure from the freelist */

        /* Get DROBJ memory pool pointer. */
        preturn =  mem_drobj_freelist;
        if (preturn)
        {
            /* Move to the next DROBJ memory pool. */
            mem_drobj_freelist = (DROBJ *) preturn->pdrive;

            /* Initialize DROBJ. */
            pc_memfill(preturn, sizeof(DROBJ), (UINT8) 0);

            ret_val = preturn;
        }
        else
        {
            pc_report_error(PCERR_DROBJALLOC);
            ret_val = NU_NULL;
        }
    }

    return(ret_val);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_memory_finode                                                
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       If called with a null pointer, allocates and zeros the space   
*       needed to store a FINODE structure. If called with a NON-NULL   
*       pointer, the FINODE structure is returned to the heap.           
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *pinode                             If NULL is specified, it    
*                                            allocates FINODE memory.    
*                                            If valid pointer is input, 
*                                            it is freed.               
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       If an ALLOC, returns a valid pointer or NU_NULL if no more core. 
*       If a free, the return value is the input.                        
*                                                                       
*************************************************************************/
FINODE *pc_memory_finode(FINODE *pinode)
{
FINODE      *preturn;
#if (LOCK_METHOD == 2)
WAIT_HANDLE_TYPE wait_handle;
#endif  /* LOCK_METHOD == 2 */
FINODE      *ret_val;

    if (pinode)
    {
        /* Free it by putting it at the head of the freelist */
        /* Set the next FINODE pointer. */
        pinode->pnext = mem_finode_freelist;

        /* Set FINODE memory pool free list. */
        mem_finode_freelist = pinode;
#ifdef DEBUG_FI
        trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"Free FINODE 0x%08x \n",(unsigned int)pinode);
#endif
        ret_val = pinode;
    }
    else
    {
        /* Alloc: return the first structure from the freelist */

        /* Get FINODE memory pool pointer. */
        preturn =  mem_finode_freelist;
        if (preturn)
        {
            /* Move to the next FINODE memory pool. */
            mem_finode_freelist = preturn->pnext;
            /* Zero the structure. wait_handle can't be zeroed so
               push it and pop it after zeroing */
#if (LOCK_METHOD == 2)
            /* Evacuate the number of wait handle. */
            wait_handle = preturn->lock_object.wait_handle;
            /* Initialize FINODE. */
            pc_memfill(preturn, sizeof(FINODE), (UINT8) 0);
            /* Set the number of wait handle. */
            preturn->lock_object.wait_handle = wait_handle;

#else   /* LOCK_METHOD != 2 */

            /* Initialize FINODE. */
            pc_memfill(preturn, sizeof(FINODE), (UINT8) 0);

#endif  /* LOCK_METHOD == 2 */
#ifdef DEBUG_FI
            trace_new( TRACE_ATA | TRACE_LEVEL_4,"Alloc FINODE 0x%08x \n",(unsigned int)preturn);
#endif

            ret_val = preturn;
        }
        else
        {
            pc_report_error(PCERR_FINODEALLOC);

            ret_val = NU_NULL;
        }
    }

    return(ret_val);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       NUF_Alloc                                                       
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Nucleus - NU_Allocate_Memory call function.                     
*       See Nucleus PLUS manual.                                        
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       nbytes                              Allocate memory size(byte). 
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       None.                                                           
*                                                                       
*************************************************************************/
VOID *NUF_Alloc(INT nbytes)
{
VOID        *return_ptr;
INT         alloc_status;


    /* Allocate memory. */
    alloc_status = NU_SUCCESS;
    return_ptr = (VOID *)mem_nc_malloc( nbytes );
    if ( !return_ptr )
	alloc_status = !NU_SUCCESS;

    if (alloc_status != NU_SUCCESS)
    {
        trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NUF_Alloc: Can not allocate memory line %d\n",__LINE__);
        return_ptr = NU_NULL;
    }
    return((void *) return_ptr);

}



/****************************************************************************
 * Modifications:
 * $Log: 
 *  2    mpeg      1.1         4/2/04 11:02:33 PM     Nagaraja Kolur  CR(s) 
 *        8762 8763 : Merged 2.5.6 Nup File System.
 *        Test: 1. pvr demo 2. nup file test 3. CR 8181
 *        
 *  1    mpeg      1.0         8/22/03 5:39:10 PM     Dave Moore      
 * $
 * 
 *    Rev 1.0   22 Aug 2003 16:39:10   mooreda
 * SCR(s) 7350 :
 * Nucleus File sourcecode (Dynamic Memory Allocation)
 * 
 *
 ****************************************************************************/

