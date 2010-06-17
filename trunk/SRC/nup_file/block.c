/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                      Conexant Systems Inc. (c)  2003                     */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       block.c
 *
 *
 * Description:    see ATI header below.
 *                 Initial version based on Nucleus File v2.4.4
 *
 * Author:         Author
 *
 ****************************************************************************/
/* $Header: block.c, 3, 4/2/04 9:49:28 PM, Nagaraja Kolur$
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
*       BLOCK.C                                   2.5              
*                                                                       
* COMPONENT                                                             
*                                                                       
*       Nucleus FILE                                                    
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Directory block buffering routines.                             
*                                                                       
* DATA STRUCTURES                                                       
*                                                                       
*       None.                                                           
*                                                                       
* FUNCTIONS                                                             
*                                                                       
*       pc_alloc_blk                        Set up a block buffer.      
*       pc_blkpool                          Get pointer to block pool.  
*       pc_free_all_blk                     Release all buffers         
*                                            associated with a drive.   
*       pc_free_buf                         Release a single buffer for 
*                                            possible re-use.           
*       pc_read_blk                         Read data from disk into a  
*                                            buffer.                    
*       pc_write_blk                        Write data from the buffer  
*                                            to the disk.               
*                                                                       
* DEPENDENCIES                                                          
*                                                                       
*       pcdisk.h                            File common definitions.    
*                                                                       
*************************************************************************/

#include        "pcdisk.h"
/*cnxt*/
#include <stdio.h>
#include "retcodes.h"
#include "stbcfg.h"
#include "kal.h"
#include "trace.h"
#include "ata.h"

extern BLKBUFF      *mem_block_pool;        /* Memory block pool list.  */
extern _PC_BDEVSW   pc_bdevsw[];            /* Driver dispatch table.   */

extern UINT32  FILE_Unused_Param; /* Use to remove compiler warnings */

/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_alloc_blk                                                    
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Use pdrive and blockno to search for a buffer in the buffer     
*       pool. If not found create a new buffer entry by discarding the  
*       least recently used buffer in the buffer pool. The buffer is    
*       locked in core. A pointer to the buffer is returned in ppblk.   
*       If all buffers are in use it returns NU_NULL in *ppblk.         
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       pblk                                Read block buffer.          
*       pdriver                             Pointer to DDRIVE structure.
*       blockno                             Block number to read.       
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       1                                   Buffer was found in the pool.
*       0                                   New buffer was assigned.    
*       NUF_NO_BLOCK                        No block buffer available.  
*                                                                       
*************************************************************************/
INT pc_alloc_blk(BLKBUFF **ppblk, DDRIVE *pdrive, UINT32 blockno)
{
BLKBUFF     *pblk;
BLKBUFF     *oldest = NU_NULL;
BLKBUFF     *freeblk = NU_NULL;
UINT32      lru = (UINT32) ~0L;
static UINT32 useindex = 0L;
STATUS      ret_val = 0;

    /* Get or init the block pool. */
    pblk = pc_blkpool(pdrive); 

#ifdef DEBUG3
            DEBUG_PRINT("Alloc block %d\n", blockno);
#endif

    /* Initialize ppblk. */
    *ppblk = NU_NULL;

    /* Increment least recently used index. */
    useindex += 1;

    while (pblk)
    {
        if (!pblk->pdrive)
        {
            /* This buffer's free. */
            freeblk = pblk;
        }
        else
        {
            if ( (pblk->pdrive == pdrive) && (pblk->blockno == blockno) )
            {
                /* Found it. */
                *ppblk = pblk;
                /* Update the least recently used stuff. */
                pblk->lru = useindex;
                pblk->use_count += 1;
                /* useindex wraps around. */
                if (useindex == 0)
                {
                    /* Get or init the block pool. */
                    pblk = pc_blkpool(pdrive); 
                    /* Reset least recently used stuff. */
                    while (pblk)
                    {
                        pblk->lru = 0;
                        pblk = pblk->pnext;
                    }
                }
                ret_val = 1;
                break;
            }
            else
            {
                /* No match. See if its a candidate for swapping if we 
                   run out of pointers. */
                if (!pblk->use_count)
                {
                    if (pblk->lru < lru)
                    {
                        lru = pblk->lru;
                        oldest = pblk;
                    }
                }
            }
        }

        /* Move to next BLKBUFF. */
        pblk = pblk->pnext;
    }

    if (!ret_val)
    {
        /* If off the end of the list we have to bump somebody. */
        if (freeblk)
            pblk = freeblk;
        else
            pblk = oldest;
    }

    if ((!pblk) && (!ret_val))
    {
        pc_report_error(PCERR_BLOCKCLAIM);
        /* Panic. */
        *ppblk = NU_NULL;
        ret_val = NUF_NO_BLOCK;
    }

    if (ret_val == 0)
    {
        /* We will return NO since we didn't 
           find it in the buffer pool. */

        /* Initialize BLKBUFF. */
        pblk->lru = useindex;
        pblk->pdrive = pdrive;
        pblk->blockno = blockno;
        pblk->use_count = 1;
        pblk->io_pending = NO;

        /* Set BLKBUFF pointer. */
        *ppblk = pblk;
    }

    return(ret_val);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_blkpool                                                      
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Return the beginning of the buffer pool for a drive. If the     
*       pool is uninitialized report it. The buffer pool should have    
*       been initialized in pc_memory_init.                             
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       pdrive                              Drive management structure. 
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       BLKBUFF *                           Pointer to block pool.      
*                                                                       
*************************************************************************/
BLKBUFF *pc_blkpool(DDRIVE *pdrive)
{

    FILE_Unused_Param = (UINT32)pdrive;

    /* If not initialized somebody didn't call pc_meminit. */
    if (!mem_block_pool)
        pc_report_error(PCERR_BLOCKALLOC);

    /* Return BLKBUFF memory pool pointer. */
    return(mem_block_pool);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_free_all_blk                                                 
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Use pdrive to find all buffers in the buffer pool associated    
*       with the drive. Mark them as unused, called by dsk_close.       
*       If any are locked, print a debug message in debug mode to warn  
*       the programmer.                                                 
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       pdrive                              Drive management structure. 
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       None.                                                           
*                                                                       
*************************************************************************/
VOID pc_free_all_blk(DDRIVE *pdrive)
{
BLKBUFF     *pblk;


    /* Get or init the block pool. */
    pblk = pc_blkpool(pdrive);

    while (pblk)
    {
        /* If this block has specified drive's data. */
        if (pblk->pdrive == pdrive)
        {
            /* If someone is using this block, report error for 
               debugging. */
            if (pblk->use_count)
                pc_report_error(PCERR_BLOCKLOCK);

            /* Free work block. */
            pc_free_buf(pblk, YES);
        }

        /* Move to next BLKBUFF. */
        pblk = pblk->pnext;
    }
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_free_buf                                                     
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Give back a buffer to the system buffer pool so that it may be  
*       re-used. If waserr is YES this means that the data in the      
*       buffer is invalid so discard the buffer from the buffer pool.   
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       pblk                                Block pointer to be freed.  
*       waserr                              If it is 0, the cache is       
*                                            discarded.                 
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       None.                                                           
*                                                                       
*************************************************************************/
VOID pc_free_buf(BLKBUFF *pblk, INT waserr)
{

    if (pblk)
    {
#ifdef DEBUG3
            DEBUG_PRINT("Free block %d\n", pblk->blockno);
#endif
        /* Decrement use_count. */
        if (pblk->use_count)
            pblk->use_count -= 1;
        /* If the buffer is corrupted we null the buffer. This is safe
           even in a multitasking environment because the region of the 
           disk containing the block is always locked exclusively when 
           buffer writes are taking place. */
        if (waserr)
            pblk->pdrive = NU_NULL;
    }
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_init_blk                                                     
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Zero a BLKBUFF.                                                 
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       pdrive                              Drive management structure. 
*       blockno                             Block number to clear.      
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       NU_SUCCESS                          If service is successful.   
*       NUF_NO_BLOCK                        No block buffer available.  
*       NUF_INTERNAL                        Nucleus FILE internal error.
*                                                                       
*************************************************************************/
STATUS pc_init_blk(DDRIVE *pdrive, UINT32 blockno)
{
STATUS      status;
BLKBUFF     *pblk;


    if ( !pdrive || (blockno >= pdrive->numsecs) )
        return(NUF_INTERNAL);
    else
    {
        /* Set up a buffer to do surgery. */
        status = pc_alloc_blk(&pblk, pdrive, blockno);
        if (status < 0)
        {
            /* Error. Chuck the buffer. */
            pc_free_buf(pblk, YES);

            return(status);
        }
        /* Clear the block buffer. */
        pc_memfill(pblk->data, 512, '\0');
        /* Free current buffer. */
        pc_free_buf(pblk, NO);
    }

    return(NU_SUCCESS);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_read_blk                                                     
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Use pdrive and blockno to determine what block to read. Read    
*       the block or get it from the buffer pool and return the buffer. 
*                                                                       
*       Note: After reading, you "own" the buffer. You must release it  
*       by calling pc_free_buff() before it may be used for other       
*       blocks.                                                         
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       pblk                                Read block buffer.          
*       pdriver                             Pointer to DDRIVE structure.
*       blockno                             Block number to read.       
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       NU_SUCCESS                          If service is successful.   
*       NUF_NO_BLOCK                        No block buffer available.  
*       NUF_IO_ERROR                        Driver IO error.            
*       NUF_INTERNAL                        Nucleus FILE internal error.
*                                                                       
*************************************************************************/
STATUS pc_read_blk(BLKBUFF **pblk, DDRIVE *pdrive, UINT32 blockno)
{
INT         found_buffer = 0;
STATUS      ret_status = NU_SUCCESS;


    /* Check parameter. */
    if ( !pdrive || (blockno >= pdrive->numsecs) )
	{
        ret_status = NUF_INTERNAL;
	    trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"pc_read_blk NUF_INTERNAL, blockno=%d, pdrive->numsecs=%d, line %d\n",blockno,pdrive->numsecs,__LINE__);
        trace_new( TRACE_ATA|TRACE_LEVEL_ALWAYS,"pdrive->secpfat=%d,pdrive->numfats=%d\n",pdrive->secpfat,pdrive->numfats );
    }

    if (ret_status == NU_SUCCESS)
    {
        /* Set up a buffer. */
        found_buffer = pc_alloc_blk(pblk, pdrive, blockno);
        if (found_buffer < 0)
		{
            ret_status = found_buffer;
	        trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"pc_read_blk: pc_alloc_blk returned %d line %d\n",found_buffer,__LINE__);
	    }
    }

    if (found_buffer && (ret_status == NU_SUCCESS))
    {
        /* Found it. */

        /* If the block is being read we loop until the io_pending
           condition is gone. */
#if (LOCK_METHOD == 2)
        while ((*pblk)->io_pending)
        {
            fs_release();
            fs_suspend_task();
            fs_reclaim();
        }
#endif  /* LOCK_METHOD == 2 */
    }
    else if (ret_status == NU_SUCCESS)
    {
        /* New BLKBUFF case.
            Not found. Read it in. */

        /* Set I/O pending flag. */
        (*pblk)->io_pending = YES;

        /* Grab the device driver. */
        PC_DRIVE_IO_ENTER(pdrive->driveno)

        /* READ */
        if ( !pc_bdevsw[pdrive->driveno].io_proc(pdrive->driveno, 
                                blockno, (*pblk)->data, (UINT16) 1, YES) )
        {
            /* oops: Drop the use count and mark an error.
                     if anybody else is waiting do a wakeup. */
            (*pblk)->use_count =  0;
            /* Now the block doesn't exist in the buffer pool. */
            (*pblk)->pdrive = NU_NULL;

	        trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"pc_read_blk: block %d NUF_IO_ERROR line %d\n",blockno,__LINE__);
            ret_status = NUF_IO_ERROR;
        }
        else
        {
            /* The read worked. */
            ret_status = NU_SUCCESS;

        }

        /* Clear I/O pending flag. */
        (*pblk)->io_pending =  NO;

        /* Release the drive io locks. */
        PC_DRIVE_IO_EXIT(pdrive->driveno)
    }

    return(ret_status);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_write_blk                                                    
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Use pdrive and blockno information in pblk to flush it's data   
*       buffer to disk.                                                 
*                                                                       
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       pblk                                Write block buffer.         
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       NU_SUCCESS                          If the write succeeded.     
*       NUF_IO_ERROR                        Driver IO error.            
*       NUF_INTERNAL                        Nucleus FILE internal error.
*                                                                       
*************************************************************************/
STATUS pc_write_blk(BLKBUFF *pblk)
{
INT         ret_val = NU_SUCCESS;


	/*printf("pc_write_blk %d\n",__LINE__);*/
    /* Check parameter. */
    if ( !pblk || !pblk->pdrive )
	{
	    trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"pc_write_blk NUF_INTERNAL %d\n",__LINE__);
        ret_val = NUF_INTERNAL;
    }

    if ( ret_val == NU_SUCCESS )
    {
        /* Grab the device driver. */
        PC_DRIVE_IO_ENTER(pblk->pdrive->driveno)

        /* WRITE */
        if ( !pc_bdevsw[pblk->pdrive->driveno].io_proc(pblk->pdrive->driveno,
                                   pblk->blockno, pblk->data, (UINT16) 1, NO) )
        {
	        trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"pc_write_blk IO_ERROR %d\n",__LINE__);
            ret_val = NUF_IO_ERROR;
        }

        /* Release the drive io locks. */
        PC_DRIVE_IO_EXIT(pblk->pdrive->driveno)
    }
    return(ret_val);
}


/****************************************************************************
 * Modifications:
 * $Log: 
 *  3    mpeg      1.2         4/2/04 9:49:28 PM      Nagaraja Kolur  CR(s) 
 *        8762 8763 : Merged 2.5.6 Nup File System.
 *        Test: 1. pvr demo 2. nup file test 3. CR 8181
 *        
 *  2    mpeg      1.1         10/15/03 4:55:44 PM    Tim White       CR(s): 
 *        7660 Remove ATA header files from NUP_FILE code.
 *        
 *  1    mpeg      1.0         8/22/03 5:27:56 PM     Dave Moore      
 * $
 ****************************************************************************/

