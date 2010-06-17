/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                 Conexant Systems Inc. (c) 2003                           */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       apiext.c
 *
 *
 * Description:    see ATI header below.
 *                 Initial version based on Nucleus File v2.4.4
 *
 ****************************************************************************/
/* $Header: apiext.c, 2, 4/2/04 9:13:39 PM, Nagaraja Kolur$
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
*       APIEXT.C                                  2.5              
*                                                                       
* COMPONENT                                                             
*                                                                       
*       Nucleus FILE                                                   
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Extensions to the API.                                          
*                                                                       
* DATA STRUCTURES                                                       
*                                                                       
*       None                                                            
*                                                                       
* FUNCTIONS                                                             
*                                                                       
*       po_extend_file                      Extend a file by            
*                                            N contiguous clusters.     
*       pc_find_contig_clusters             Find at least MIN_CLUSTER   
*                                            clusters.                  
*                                                                       
* DEPENDENCIES                                                          
*                                                                       
*       pcdisk.h                            File common definitions     
*                                                                       
************************************************************************/

#include    "pcdisk.h"
#include	   "file_mmu.h"

/************************************************************************
* FUNCTION                                                              
*                                                                       
*       po_extend_file                                                  
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Given a file descriptor, n_clusters clusters and method,        
*       extend the file and update the file size.                       
*                                                                       
*       Method may be one of the following:                             
*       PC_FIRST_FIT  - The first free chain >= n_clusters is alloced   
*       PC_BEST_FIT   - The smallest chain   >= n_clusters is alloced   
*       PC_WORST_FIT  - The largest chain    >= n_clusters is alloced   
*                                                                       
*       Note: PC_FIRST_FIT is significantly faster than the others      
*       See: pc_find_contig_clusters()                                  
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       fd                                  File descriptor             
*       n_clusters                          Number of clusters          
*       method                              Method                      
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       0xffffffff if an error occurred.                                 
*       Returns n_clusters if the file was extended. Otherwise it       
*       returns the largest free chain available. If n_clusters is   
*       not returned the files was not extended.                        
*                                                                       
*************************************************************************/
UINT32 po_extend_file(INT fd, UINT32 n_clusters, INT16 method)
{
STATUS      ret_stat;
UINT32      ret_val;
UINT32      clno;
UINT32      n_alloced;
UINT32      largest_chain;
UINT32      first_cluster;
UINT32      last_cluster_in_chain;
UINT32      i;
UINT32      ltemp;
PC_FILE     *pfile;
DDRIVE      *pdr;


    /* Must be last line in declarations. */
    PC_FS_ENTER()
    /* Check if a valid user if multitasking. */
    CHECK_USER(UINT32, -1)

    if (!n_clusters)
    {
        fs_user->p_errno = 0;
        ret_val = 0;
        goto return_error;
    }

    /* Assume error to start. */
    fs_user->p_errno = PENOSPC;
    ret_val = (UINT32) -1;

    /* Get the FILE. Second argument is ignored. */
    pfile = pc_fd2file(fd);

    /* Make sure we have write privileges. Make sure we got a  count. */
    if ( (!pfile) || !((pfile->flag & PO_WRONLY) || 
         (pfile->flag & PO_RDWR)) )
    {
        fs_user->p_errno = PEBADF;
        goto return_error;
    }

    /* From here on we exit through alloc_done so we will unlock these 
       resources. */
    pdr = pfile->pobj->pdrive;

    /* Register drive in use. */
    PC_DRIVE_ENTER(pdr->driveno, NO)
    /* Lock the finode. */
    PC_INODE_ENTER(pfile->pobj->finode, YES)
    /* Grab exclusive access to the FAT. */
    PC_FAT_ENTER(pdr->driveno)

    /* Make sure our file pointer is ok */
    _synch_file_ptrs(pfile);

        /* Find the end of the file's chain */
    last_cluster_in_chain = 0;  
    clno = pfile->fptr_cluster;
    while (clno)
    {
        last_cluster_in_chain = clno;
        pc_clnext(&clno, pdr, clno);
    }

    /* Now allocate clusters. To find the free space we look in three 
       regions until we find space:
        1 we look from the last cluster in the file to the end of the 
          fat(skip 1 if there is no chain).
        2 we look from the beginning of the data area to the end of the 
          fat.
        3 we look from the beginning of the fat area to the end of the 
          fat.
    */

    n_alloced     = 0;
    largest_chain = 0;
    clno = last_cluster_in_chain;
    if (!clno)
        clno = pdr->free_contig_pointer;

    while (clno)
    {
        n_alloced =  pc_find_contig_clusters(pdr, clno, &first_cluster, 
                                                n_clusters, method);
        if (n_alloced == ((UINT32) -1))
            goto alloc_done;
        else if (n_alloced >= n_clusters)
            break;                      /* We got our chain */
        else
        {
            /* We didn't get enough space. keep track of the biggest 
               chain. Don't need to store first_cluster since we won't 
               alocate chains smaller than what we need. */
            if (largest_chain < n_alloced)
                largest_chain = n_alloced;
        }
        /* If we were searching between from the end of the  file and 
           end of fat look from the beginning of the file data area. */
        if (clno == last_cluster_in_chain)
            clno = pdr->free_contig_pointer;
        /* If we were searching between the beginning of the file data
           area and end of fat  look from the fat. */
        else if ( (clno == pdr->free_contig_pointer) && (clno != 2) )
            clno = 2;
        else  /* We've looked everywhere. No luck */
            break;
    }

    if (n_alloced < n_clusters)
    {
        /* We didn't get what we asked for so we return the biggest 
           free contiguous chain */
        ret_val = largest_chain;
        goto alloc_done;
    }
    /* else */

    /* We found a large enough contiguos group of clusters. */
    /* Turn them into a chain. */
    clno = first_cluster;
    for (i = 0; i < (n_clusters-1); i++, clno++)
    {
        /* Link the current cluster to the next one */
        ret_stat = pc_pfaxx(pdr, clno, (UINT32) (clno+1));
    if (ret_stat != NU_SUCCESS)
            goto alloc_done;
    }
    /* Terminate the list */
    ret_stat = pc_pfaxx(pdr, clno, ((UINT32)-1));
    if (ret_stat != NU_SUCCESS)
        goto alloc_done;

    if (last_cluster_in_chain)
    {
        /* The file already has clusters in it. Append our new chain. */
        ret_stat = pc_pfaxx(pdr, last_cluster_in_chain, first_cluster);
        if (ret_stat != NU_SUCCESS)
            goto alloc_done;
    }
    else
    {
        /* Put our chain into the directory entry. */
        pfile->pobj->finode->fcluster = first_cluster;
        /* Use synch_pointers to set our file pointers up. */
        pfile->fptr_cluster = 0L;   /* This is already true but... */
        pfile->fptr_block = 0L;
        pfile->fptr = 0L;
    }

    
    /* Now recalculate the file size. */
    ltemp = n_clusters;
    ltemp <<= (pdr->log2_secpalloc + 9);
    pfile->pobj->finode->fsize += ltemp;
    /* Call synch to take care of both the eof condition and the case 
       where we just alloced the beginning of the chain. */
    _synch_file_ptrs(pfile);

    /* Flush the fat. */
    ret_stat = pc_flushfat(pdr);
    if (ret_stat != NU_SUCCESS)
        goto alloc_done;

    /* Write the directory entry. */
    ret_stat = pc_update_inode(pfile->pobj, DSET_UPDATE);
    if (ret_stat != NU_SUCCESS)
        goto alloc_done;

    /* It worked !  Set the return to the number of clusters requested. */
    ret_val = n_clusters;
    fs_user->p_errno = 0;

    /* All code exits through here. ret_val determines if the function
       was successful. If 0xffff it's an error. If n_clusters it's a 
       success and the file is expanded. Otherwise the return value */
alloc_done:
    /* Release non-excl use of FAT. */
    PC_FAT_EXIT(pdr->driveno)
    /* Release excl use of finode. */
    PC_INODE_EXIT(pfile->pobj->finode)
    /* Release non-excl use of drive. */
    PC_DRIVE_EXIT(pdr->driveno)
return_error:
    /* Restore the kernel state. */
    PC_FS_EXIT()

    return(ret_val);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_find_contig_clusters                                         
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Using the provided method, search the FAT from start_pt to the  
*       end for a free contiguous chain of at least MIN_CLUSTERS. If    
*       less than MIN_CLUSTERS are found the largest free chain in the  
*       region is returned.                                             
*                                                                       
*       There are three possible methods:                               
*       PC_FIRST_FIT - The first free chain >= MIN_CLUSTERS is returned 
*       PC_BEST_FIT  - The smallest chain   >= MIN_CLUSTERS is returned 
*       PC_WORST_FIT - The largest chain    >= MIN_CLUSTERS is returned 
*                                                                       
*       Choose the method that will work best for you.                  
*                                                                       
*       Note: The chain is not created. The caller must convert the     
*              clusters to an allocated chain.                          
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       pdr                                 Drive management structure  
*       startpt                             Search start cluster        
*       pchain                              Free chain pointer          
*       min_clusters                        Minimum chain clusters      
*       method                              Method                    
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       Returns the number of contiguous clusters found up to           
*       MIN_CLUSTERS. *pchain contains the cluster number at the        
*       beginning of the chain.                                         
*       On error return 0xffff                                          
*       Example:                                                        
*       Get the largest free chain on the disk:                         
*       large =                                                         
*        pc_find_contig_clusters(pdr, 2, &chain, 0xffff, PC_FIRST_FIT); 
*                                                                       
*************************************************************************/
UINT32 pc_find_contig_clusters(DDRIVE *pdr, UINT32 startpt, 
                               UINT32 *pchain, UINT32 min_clusters, 
                               INT16 method) /* __fn__ */
{
STATUS      ret_stat;
UINT32      i;
UINT32      value;
UINT32      best_chain;
UINT32      best_size;
UINT32      chain_start;
UINT32      chain_size;
UINT32      largest_size;
UINT32      largest_chain;
UINT32      endpt;


    best_chain = 0;
    best_size = 0;
    chain_start = 0;
    chain_size = 0;
    largest_size  = 0;
    largest_chain = 0;
    endpt = pdr->maxfindex;

    for (i = startpt; i <= endpt; i++)
    {
        ret_stat = pc_faxx(pdr, i, &value);
        if (ret_stat != NU_SUCCESS)
            return((UINT32) -1);             /* IO error .. oops */

        if (value == 0)
        {   
            /* Cluster is free. Run some tests on it. */
            if (chain_start)
            {
                /* We're in a contiguous region already. Bump the count. */
                chain_size++;
            }
            else
            {
                /* Just starting a contiguous region. */
                chain_size = 1;
                chain_start = i;
            }
            /* If using first fit see if we crossed the threshold. */
            if (method == PC_FIRST_FIT)
            {
                if (chain_size >= min_clusters)
                {
                    best_chain = chain_start;
                    best_size = chain_size;
                    break;
                }
            }
        }       /* if value == 0*/
        /* Did we just finish scanning a contiguous chain? */
        if ( chain_size && ((value != 0) || (i == endpt)) )
        {
            /* Remember the largest chain. */
            if (chain_size > largest_size)
            {
                largest_size  = chain_size;
                largest_chain = chain_start;
            }
            if (method == PC_BEST_FIT)
            {
                if (chain_size == min_clusters)
                {
                    /* The chain is exactly the size we need take it. */
                    best_chain = chain_start;
                    best_size = chain_size;
                    break;
                }
                if (chain_size > min_clusters)
                {
                    if ( !best_chain || (chain_size < best_size) )
                    {
                        /* Chain is closest to what we need so far note 
                           it. */
                        best_size = chain_size;
                        best_chain = chain_start;
                    }
                }
            }   /* if BEST_FIT */
            else if (method == PC_WORST_FIT)
            {
                if (chain_size >= min_clusters)
                {
                    if ( !best_chain || chain_size > best_size )
                    {
                        best_size = chain_size;
                        best_chain = chain_start;
                    }
                }
            }   /* if WORST_FIT */
            chain_size = 0;
            chain_start = 0;
        } /* if (chain_size && ((value != 0) || (i == endpt)) ) */
    }     /*     for (i = startpt; i <= endpt; i++) */

    /* If we have a best chain return it here. Else return the largest 
       chain. */
    if (best_chain)
    {
        *pchain = best_chain;
        return(best_size);
    }
    else
    {
        *pchain = largest_chain;
        return(largest_size);
    }
}


/****************************************************************************
 * Modifications:
 * $Log: 
 *  2    mpeg      1.1         4/2/04 9:13:39 PM      Nagaraja Kolur  CR(s) 
 *        8762 8763 : Merged 2.5.6 Nup File System.
 *        Test: 1. pvr demo 2. nup file test 3. CR 8181
 *        
 *  1    mpeg      1.0         8/22/03 5:20:42 PM     Dave Moore      
 * $
 * 
 *    Rev 1.0   22 Aug 2003 16:20:42   mooreda
 * SCR(s) 7350 :
 * Nucleus File Source (API Extensions)
 * 
 *
 ****************************************************************************/

