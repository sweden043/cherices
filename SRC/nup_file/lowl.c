/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                 Conexant Systems Inc. (c) 2003                           */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       lowl.c
 *
 *
 * Description:    see ATI header below.
 *                 Initial version based on Nucleus File v2.4.4
 *
 *
 ****************************************************************************/
/* $Header: lowl.c, 3, 4/3/04 12:47:21 AM, Nagaraja Kolur$
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
*       LOWL.C                                    2.5                  
*                                                                       
* COMPONENT                                                             
*                                                                       
*       Nucleus FILE                                                    
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Low level File allocation table management functions.           
*                                                                       
* DATA STRUCTURES                                                       
*                                                                       
*       None.                                                           
*                                                                       
* FUNCTIONS                                                             
*                                                                       
*       pc_alloc_chain                      Allocate a chain from the   
*                                            FAT.                       
*       pc_find_free_cluster                Find the first free cluster 
*                                            in a given range.          
*       pc_clalloc                          Allocate a single cluster   
*                                            in the fragmented region.  
*       pc_clgrow                           Grow a directory chain in   
*                                            the fragmented region.     
*       pc_clnext                           Get the next cluster in a   
*                                            chain.                     
*       pc_clrelease                        Return a cluster to the free
*                                            list.                      
*       pc_faxx                             Get a value from the FAT.   
*       pc_flushfat                         Make sure the FAT is up to  
*                                            date on disk.              
*       pc_freechain                        Release a chain to the free 
*                                            list.                      
*       pc_get_chain                        Return contiguous clusters  
*                                            in a chain.                
*       pc_pfaxx                            Put a value to the FAT.     
*       pc_pfswap                           Swap a block of the FAT into
*                                            the cache.                 
*       pc_pfpword                          Get a value from the swap   
*                                            cache.                     
*       pc_pfgword                          Put a value to the swap     
*                                            cache.                     
*       pc_pfflush                          Flush the swap cache to disk.
*       pc_clzero                           Write zeros to a cluster on
*                                            disk.                      
*       pc_drno2dr                          Convert a drive number to a 
*                                            drive structure.           
*       pc_dskfree                          Free resources associated   
*                                            with a drive.              
*       pc_ifree                            Calculate free space from   
*                                            the FAT.                   
*       pc_sec2cluster                      Convert a sector number to  
*                                            a cluster value.           
*       pc_sec2index                        Given a block number, offset 
*                                            from the beginning of the  
*                                            drive.                     
*       pc_cl2sector                        Convert cluster number to a 
*                                            blocknumber.               
*                                                                       
* DEPENDENCIES                                                          
*                                                                       
*       pcdisk.h                            File common definitions     
*                                                                       
*************************************************************************/
#include        "pcdisk.h"
#include "retcodes.h"
#include "stbcfg.h"
#include "kal.h"
#include "trace.h"
#include  "ata.h"

extern _PC_BDEVSW       pc_bdevsw[];
extern UNSIGNED         *NUF_Drive_Pointers[];
extern INT              NUF_Drive_Fat_Size[];


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_alloc_chain                                                  
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Reserve up to n_clusters contiguous clusters from the FAT and   
*       set the number of contiguous clusters reserved.              
*       If pstart_cluster points to a valid cluster, link the new chain  
*       to it.                                                          
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *n_contig                           Contiguous clusters pointer 
*       *pdr                                Drive object                
*       *pstart_cluster                     Search start cluster        
*       n_clusters                          Process clusters            
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       NU_SUCCESS                          If service is successful.   
*       NUF_NOSPC                           No free cluster.            
*       NUF_IO_ERROR                        Driver I/O error.            
*       NUF_INTERNAL                        Nucleus FILE internal error.
                                                                       
*************************************************************************/
STATUS pc_alloc_chain(UINT32 *n_contig, DDRIVE *pdr, 
                      UINT32 *pstart_cluster, UINT32 n_clusters)
{
STATUS      ret_stat = NU_SUCCESS;
UINT32      start_cluster;
UINT32      first_new_cluster = 0;
UINT32      clno;
UINT32      value;


    start_cluster = *pstart_cluster;

    if (start_cluster)
    {
        if ((start_cluster < 2L) || (start_cluster > pdr->maxfindex))
        {
            trace_new(TRACE_ATA | TRACE_LEVEL_ALWAYS, "pc_alloc_chain: Invalid cluster number specified: start_cluster number %d maxfindex %d line %d\n",start_cluster,pdr->maxfindex,__LINE__); 
            ret_stat = NUF_INTERNAL;
        }
    }

    if (ret_stat == NU_SUCCESS)
    {
    /* If the user provided a cluster we find the next cluster beyond that
       one. Otherwise we look at the disk structure and find the next 
       free cluster in the free cluster region after the current best guess
       of the region. If that fails we look to the beginning of the region
       and if that fails we look in the non-contiguous region. */

        ret_stat = pc_find_free_cluster(&clno, pdr, 
                                pdr->free_contig_pointer, pdr->maxfindex + 1);
    }

    /* Check the beginning of the disk where we typically write 
       fragments. */
    if (ret_stat == NUF_NOSPC)
    {
        ret_stat = pc_find_free_cluster(&clno, pdr, 2L, 
                                        pdr->free_contig_pointer);
    }

    if (ret_stat == NU_SUCCESS)
    {
        /* Set new cluster. */
        first_new_cluster = clno;
        value = 0;
        *n_contig = 1;
    }

    /* look up the FAT. If the next cluster is free we link to it
       and up the contig count. */
    while ( (*n_contig < n_clusters) && (clno < pdr->maxfindex) && (ret_stat == NU_SUCCESS) )
    {
        ret_stat = pc_faxx(pdr, clno+1, &value);
        if (ret_stat != NU_SUCCESS)
            break;

        /* If the next cluster is in use, we're done. */
        if (value)
           break;

        /* Link the current cluster to the next one */
        ret_stat = pc_pfaxx(pdr, clno, clno+1);
        if (ret_stat != NU_SUCCESS)
            break;

        /* Yep.. we got another. */
        *n_contig += 1;
        /* Up the FAT table. */
        clno += 1;
    }

    if (ret_stat == NU_SUCCESS)
    {
        /* Terminate the list we just made */
        ret_stat = pc_pfaxx(pdr, clno, ((UINT32) -1));
    }

    /* Update the hint of most likeley place to find a free cluster */
    if ((clno < pdr->maxfindex) && (ret_stat == NU_SUCCESS))
    {
            if (clno >= pdr->free_contig_pointer)
            {
                pdr->free_contig_pointer = clno + 1;
            }
     }

    /* If we were handed a starting cluster we have to stitch our new
       chain after it. */
    if ((start_cluster) && (ret_stat == NU_SUCCESS))
    {
        ret_stat = pc_pfaxx(pdr, start_cluster, first_new_cluster);
    }

    if (ret_stat == NU_SUCCESS)
    {
        /* Set start cluster number. */
        *pstart_cluster = first_new_cluster;

/* Add start 2002.11.26 */
        /* Update the free cluster count. */
        if ((pdr->valid_fsinfo) && (pdr->free_clusters_count >= *n_contig))
            pdr->free_clusters_count -= *n_contig;
/* Add end 2002.11.26 */
    }

/* Del start 2002.11.26 */
#if 0
    /* Update the free cluster count. */
    if ((pdr->free_clusters_count) && (ret_stat == NU_SUCCESS))
        pdr->free_clusters_count -= *n_contig;
#endif
/* Del end 2002.11.26 */

    return(ret_stat);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_find_free_cluster                                            
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Find the first free cluster in a range.                         
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *freecl                             Free cluster number.        
*       *pdr                                Drive information           
*       startpt                             Search start cluster        
*       endpt                               Search end cluster          
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       NU_SUCCESS                          If service is successful.   
*       NUF_NOSPC                           No free cluster.            
*       NUF_IO_ERROR                        Driver I/O error.            
*       NUF_INTERNAL                        Nucleus FILE internal error.
*                                                                       
*************************************************************************/
STATUS pc_find_free_cluster(UINT32 *freecl, DDRIVE *pdr, UINT32 startpt, 
                            UINT32 endpt)
{
UINT32      i;
UINT32      value;
STATUS      ret_stat = NUF_NOSPC;


    for (i = startpt; i < endpt; i++)
    {
        /* Get the FAT entry value. */
        ret_stat = pc_faxx(pdr, i, &value);

        if (ret_stat != NU_SUCCESS)
            break;

        /* Free cluster? */
        if (value == 0)
        {
            /* Set the free cluster number. */
            *freecl = i;
            break;
        }
        else
        {
            ret_stat = NUF_NOSPC;
        }

    }

    return(ret_stat);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_clalloc                                                      
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Given a DDRIVE, mark the next available cluster in the file     
*       allocation table as used and return the associated cluster      
*       number. "clno" provides a means of selecting clusters that are   
*       near each other. This should reduce fragmentation.               
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *clno                               Return a new cluster number 
*       *pdr                                Drive information           
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       NU_SUCCESS                          If service is successful.   
*       NUF_NOSPC                           No free cluster.            
*       NUF_IO_ERROR                        Driver I/O error.            
*       NUF_INTERNAL                        Nucleus FILE internal error.
*                                                                       
*************************************************************************/
STATUS pc_clalloc(UINT32 *clno, DDRIVE *pdr)
{
STATUS ret_stat;


    *clno = 0L;

    /* Look in the "fragmentable" region first from pc_find_free_cluster 
       up. */
    ret_stat = pc_find_free_cluster(clno, pdr, 
                                pdr->free_contig_pointer, pdr->maxfindex + 1);

    /* Look in the  "fragmentable" region up to pc_find_free_cluster */
    if (ret_stat == NUF_NOSPC)
    {
        ret_stat = pc_find_free_cluster(clno, pdr, 2L, 
                                        pdr->free_contig_pointer);
    }

    if (ret_stat == NU_SUCCESS)
        /* Mark the cluster in use */
        ret_stat = pc_pfaxx(pdr, *clno, ((UINT32) -1));

    if (ret_stat == NU_SUCCESS)
    {
        /* Update the number of next free cluster. */
        if (*clno >= pdr->free_contig_pointer)
        {
            pdr->free_contig_pointer = *clno + 1;
        }
        /* Update the free cluster count. */
        if ((pdr->valid_fsinfo) && (pdr->free_clusters_count > 0L))
            pdr->free_clusters_count -= 1L;
    }

    return(ret_stat);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_clgrow                                                       
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Given a DDRIVE and a cluster, extend the chain containing the   
*       cluster by allocating a new cluster and linking "clno" to it. If  
*       "clno" is zero assume it is the start of a new file and allocate  
*       a new cluster.                                                  
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *nxt                                Return new allocated cluster
*                                            number                     
*       *pdr                                Drive information           
*       clno                                Extend cluster number       
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       NU_SUCCESS                          If service is successful.   
*       NUF_NOSPC                           No free cluster.            
*       NUF_IO_ERROR                        Driver I/O error.            
*       NUF_INTERNAL                        Nucleus FILE internal error.
*                                                                       
*************************************************************************/
STATUS pc_clgrow(UINT32 *nxt, DDRIVE *pdr, UINT32 clno)
{
STATUS      ret_stat;
UINT32      nextcluster;


    if (!clno)
	{
        trace_new(TRACE_ATA | TRACE_LEVEL_ALWAYS, "pc_clgrow: Invalid cluster number specified line %d\n.", __LINE__); 
    } 

    /* Make sure we are at the end of chain */
    ret_stat = pc_clnext(&nextcluster, pdr, clno);
    if (ret_stat == NU_SUCCESS)
    {
        while (nextcluster)     /* Data end cluster. */
        {
            /* Get the next cluster in a cluster chain. */
            clno = nextcluster;
            pc_clnext(&nextcluster, pdr, clno);
        }

        /* Get a cluster, clno provides a hint for more efficient cluster
           allocation */
        ret_stat = pc_clalloc(nxt, pdr);
        if (ret_stat == NU_SUCCESS)
        {
            /* Update the cluster in use. */
            ret_stat = pc_pfaxx(pdr, clno, *nxt);
        }
    }

    return(ret_stat);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_clnext                                                       
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Given a DDRIVE and a cluster number, return the next cluster in 
*       the chain containing "clno". Return 0 on end of chain.            
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *nxt                                Next cluster pointer(cluster
*                                            value)                     
*       *pdr                                Drive information           
*       clno                                Check the cluster number    
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       NU_SUCCESS                          If service is successful.   
*       NUF_DEFECTIVEC                      Defective cluster detected. 
*       NUF_IO_ERROR                        Driver I/O error.            
*       NUF_INTERNAL                        Nucleus FILE internal error.
*                                                                       
*************************************************************************/
STATUS pc_clnext(UINT32 *nxt, DDRIVE *pdr, UINT32 clno)
{
STATUS      ret_stat;


    /* Get the value at clno. return 0 on any I/O errors */
    ret_stat = pc_faxx(pdr, clno, nxt);
    if (ret_stat == NU_SUCCESS)
    {
        if (pdr->fasize == 3)       /* 3 nibble ? */
        {
            if ( (0xff7 < *nxt) && (*nxt <= 0xfff) )
                *nxt = 0;                            /* end of chain */

            if (*nxt == 0xff7)  /* Defective Cluster */
                ret_stat = NUF_DEFECTIVEC;
            if (*nxt > 0xfff)
			{
               trace_new(TRACE_ATA | TRACE_LEVEL_ALWAYS, "Invalid FAT entry %x %d  %s \r\n",*nxt, __LINE__, __FILE__); 
			}
        }
        else if (pdr->fasize == 4)  /* 4 nibble ? */
        {
            if (0xfff7 < *nxt)
                *nxt = 0;                            /* end of chain */
            if (*nxt == 0xfff7)  /* Defective Cluster */
                ret_stat = NUF_DEFECTIVEC;
            if (*nxt > 0xffff)
			{
               trace_new(TRACE_ATA | TRACE_LEVEL_ALWAYS, "Invalid FAT entry %x %d  %s \r\n",*nxt, __LINE__, __FILE__); 
			}
        }
        else
        {   /* FAT32 */
            if (0x0ffffff7 < *nxt)
                *nxt = 0;                            /* end of chain */
            if (*nxt == 0x0ffffff7)  /* Defective Cluster */
                ret_stat = NUF_DEFECTIVEC;
            if (*nxt > 0x0fffffff)
			{
               trace_new(TRACE_ATA | TRACE_LEVEL_ALWAYS, "Invalid FAT entry %x %d  %s \r\n",*nxt, __LINE__, __FILE__); 
			}
        }
    }
    return(ret_stat);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_clrelease                                                    
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Given a DDRIVE and a cluster, mark the cluster in the file      
*       allocation table as free. It will be used again by calls to     
*       pc_clalloc().                                                   
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *pdr                                Drive information           
*       clno                                Release the cluster number  
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       NU_SUCCESS                          If service is successful.   
*       NUF_IO_ERROR                        Driver I/O error.            
*       NUF_INTERNAL                        Nucleus FILE internal error.
*                                                                       
*************************************************************************/
STATUS pc_clrelease(DDRIVE *pdr, UINT32 clno)
{
STATUS      ret_stat;


    /* Don't catch any lower level errors here. You'll catch them soon 
       enough. */
    /* Mark it as free. */
    ret_stat = pc_pfaxx(pdr, clno, ((UINT32) 0));        /* Mark it as free */
    if (ret_stat == NU_SUCCESS)
    {
        /* If freeing in the "contiguous" region, reset the "hint" if we
           free space earlier than it. */
        if (clno < pdr->free_contig_pointer)
                 pdr->free_contig_pointer = clno;
        
/* Del start 2002.11.26 */
#if 0
        if (pdr->free_clusters_count)
#else
/* Del end 2002.11.26 */
/* Add start 2002.11.26 */
        if (pdr->valid_fsinfo)
#endif
/* Add end 2002.11.26 */
            pdr->free_clusters_count += 1L;
    }

    return(ret_stat);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_faxx                                                         
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Given a DDRIVE and a cluster number. Get the value in the FAT at
*       "clno" (the next cluster in a chain.)                        
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *pdr                                Drive information.          
*       clno                                Cluster number to get FAT   
*                                            entry value.               
*       pvalue                              FAT entry value.            
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       If any error occurred while FAT swapping, return negative value;   
*       else return NU_SUCCESS                                          
*                                                                       
*       NU_SUCCESS                          If service is successful.   
*       NUF_NO_MEMORY                       Can't allocate FAT cache    
*                                            buffer. (FAT12 only )      
*       NUF_IO_ERROR                        Driver I/O error.            
*       NUF_INTERNAL                        Nucleus FILE internal error.
*                                                                       
*************************************************************************/
STATUS pc_faxx(DDRIVE *pdr, UINT32 clno, UINT32 *pvalue)
{
STATUS      ret_stat = NU_SUCCESS;
UINT16      wtry;
UINT32      ltry;
UINT8       wrdbuf[4];          /* Temp storage area */
UINT8 FAR   *fat_data = pdr->fat_swap_structure.data_array;
UINT8 FAR   *ppage;
UINT32      offset;
INT16       driveno;
UINT16      wvalue;


    if (pdr->fasize != 8)       /* 3 nibble ? */
	{
        trace_new(TRACE_ATA | TRACE_LEVEL_4, "pc_faxx enter fasize=%d line %d\n",pdr->fasize,__LINE__); 
	}

    driveno= pdr->driveno;
    /* ======================== FAT12 ======================== */
    if (pdr->fasize == 3)       /* 3 nibble ? */
    {

        /* We do not read FAT to FAT cache yet. */
        if ( (fat_data == NU_NULL) || 
             ((fat_data[0] & (UINT8)0xF0) != (UINT8)0xF0) )
        {
            if (fat_data == NU_NULL)
            {
                pdr->fat_swap_structure.data_array =
                                (UINT8 FAR *)NUF_Alloc(pdr->fat_swap_structure.n_blocks_total << 9 );
                trace_new(TRACE_ATA | TRACE_LEVEL_4, "pc_faxx 12bit FAT fat_data == NULL %d  %s \r\n", __LINE__, __FILE__); 
                if (pdr->fat_swap_structure.data_array == NU_NULL )
                {
                    trace_new(TRACE_ATA | TRACE_LEVEL_ALWAYS, "pc_faxx 12bit FAT can't allocate FAT cache buffer %d  %s \r\n", __LINE__, __FILE__); 
                    return(NUF_NO_MEMORY);
                }
            }
#ifdef DEBUG
            else 
			{
                    trace_new(TRACE_ATA | TRACE_LEVEL_ALWAYS, "pc_faxx 12bit FAT fat_data[0] != 0xF9 %d  %s \r\n", __LINE__, __FILE__); 
			}
#endif
            /* Grab the device driver. */
            PC_DRIVE_IO_ENTER(driveno)
            /* Read FAT sectors */
            if ( !pc_bdevsw[driveno].io_proc(driveno, (UINT32)(pdr->numhide + pdr->fatblock),
                                pdr->fat_swap_structure.data_array, (UINT16) pdr->secpfat, YES))
            {
                /* Release the drive I/O locks. */
                PC_DRIVE_IO_EXIT(driveno)
                pc_report_error(PCERR_FATREAD);
                NU_Deallocate_Memory((VOID *)pdr->fat_swap_structure.data_array);

                return(NUF_IO_ERROR);
            }

            /* Release the drive I/O locks. */
            PC_DRIVE_IO_EXIT(driveno)
        }

        wtry = (UINT16) (clno + (UINT16) (clno >> 1));    /* multiply by 1.5 */
           /* And use the product as index */
        wrdbuf[0] = fat_data[wtry];     /* Use a temp variable since in */
        wrdbuf[1] = fat_data[wtry+1];   /* small model we can't call with
                                           fat_data (it's a far pointer) */

        SWAP16((UINT16 *)&wvalue, (UINT16 *)wrdbuf);
        *pvalue = (UINT32) wvalue;   /* And use the product as index */

        if ( (UINT16)((clno << 1) + clno) == (wtry << 1) )  /* If whole number */
            *pvalue &= 0xfff;            /* Return it */
        else
             /* Shift right 4. */
            *pvalue =(UINT32) ((UINT16) (*pvalue >> 4) & 0xfff ); /* shift right 4 */
    }


    /* ======================== FAT16 ======================== */
    else if (pdr->fasize == 4)
    {
        /* 16 BIT fat. ret the value at 2 * clno */
        /* Some FAT data is cached */
        if (pdr->use_fatbuf)
        {
            ret_stat = pc_pfgword(pdr, (UINT16)clno, (UINT16 *) &wrdbuf[0]);
            if (ret_stat != NU_SUCCESS)
            {
                trace_new(TRACE_ATA | TRACE_LEVEL_4, "pc_faxx: pc_pfgword returned error %d  %s \r\n", __LINE__, __FILE__); 
                return(ret_stat);
            }
        }
        /* All FAT data is cached */
        else
        {
            ltry = (UINT32) clno;
            ltry <<= 1;
            wrdbuf[0] = fat_data[ltry];     /* Use a temp variable since in */
            wrdbuf[1] = fat_data[ltry+1];   /* intel small model we can't call
                                               with  fat_data (it's a far pointer) */
        }
        SWAP16(&wvalue, (UINT16 *)wrdbuf);
        *pvalue = wvalue;
    }


    /* ======================== FAT32 ======================== */
    else if (pdr->fasize == 8)
    {

        /* Some FAT data is cached */
        if (pdr->use_fatbuf)
        {
            /* Make sure we have access to the page. Don't Mark it for writing */
            ret_stat = pc_pfswap(&ppage, pdr, clno, NO);
            if (ret_stat != NU_SUCCESS)
            {
                trace_new(TRACE_ATA | TRACE_LEVEL_ALWAYS, "pc_faxx: pc_pfswap returned error %d  %s \r\n", __LINE__, __FILE__); 
                return(ret_stat);
            }
            else
            {
                /* there are 128 entries per page */
                offset = (UINT32) (clno & 0x7f);
                *((UINT32 *)wrdbuf) = *((UINT32 *)ppage + offset);
            }
        }
        /* All FAT data is cached */
        else
        {
            /* 32-bit FAT */
            ltry = (UINT32) clno;
            ltry <<= 2;
            wrdbuf[0] = fat_data[ltry];     /* Use a temp variable since in */
            wrdbuf[1] = fat_data[ltry+1];   /* intel small model we can't call */
            wrdbuf[2] = fat_data[ltry+2];   /*   with  fat_data (it's a far pointer) */
            wrdbuf[3] = fat_data[ltry+3];
        }
        SWAP32(pvalue,(UINT32 *)&wrdbuf[0]);   /* And use the product as index */
    }
    else
    {
        trace_new(TRACE_ATA | TRACE_LEVEL_ALWAYS, "pc_faxx: pdr->fasize error %d  %s \r\n", __LINE__, __FILE__);
        return(NUF_INTERNAL);
    }
    return(NU_SUCCESS);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_flushfat                                                     
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Given a valid drive number. Write any FAT blocks to disk that   
*       have been modified. Updates all copies of the FAT.              
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *pdr                                Drive management structure  
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       NU_SUCCESS                          FAT flush success.          
*       NU_IO_ERROR                         Write failed.               
*       NU_INTERNAL                         FAT cache is NULL           
*                                                                       
*************************************************************************/
STATUS pc_flushfat(DDRIVE *pdr)
{
FATSWAP     *pfs;
UINT8 FAR   *pf;
UINT16      *pd;
UINT32      i;
UINT32      imax;
UINT16      j;
UINT32      baseblock;
INT         is_last;
STATUS      ret_stat = NU_SUCCESS;
STATUS      done = 0;


    /* The FAT does not have a dirty block which needs to be written. */
    if (!pdr->fat_is_dirty)
        done = 1;

    /* Whether we use the FAT cache as buffer ? */
    if (pdr->use_fatbuf && !done)
    {
        /* Flush FAT */
        ret_stat = pc_pfflush(pdr);
        if (ret_stat == NU_SUCCESS)
        {
            /* FAT flush successful complete */
            pdr->fat_is_dirty = NO;
        }
        done = 1;
    }

    if (!done)
    {
        /* Grab the device driver. */
        PC_DRIVE_IO_ENTER(pdr->driveno)

        /* Move the number of FAT to local */
        imax = (UINT32) pdr->secpfat;

        for (j = 0; j < pdr->numfats; j++)
        {
            /* Work out start block number of FAT */
            baseblock = (UINT32) j;
            baseblock *= imax;
            baseblock += (pdr->numhide + pdr->fatblock);

            /* Is this FAT last ? */
            if ( j == (UINT16) (pdr->numfats - 1) )
                is_last = YES;
            else
                is_last = NO;

            /* NOTE: data_array is the FAT data. data_map is a 2 bytes map of dirty
               FAT blocks                                                       */
            pf = pdr->fat_swap_structure.data_array;
            pd = pdr->fat_swap_structure.data_map;
            pfs = &pdr->fat_swap_structure;
            if (!pf)
            {
                /* Release the drive I/O locks. */
                PC_DRIVE_IO_EXIT(pdr->driveno)
                pc_report_error(PCERR_FAT_NULLP);
                ret_stat = NUF_INTERNAL;
                done = 1;
                trace_new(TRACE_ATA | TRACE_LEVEL_ALWAYS, "pc_flushfat FAT_NULLP %d\n",__LINE__); 
                break;
            }

            /* Check each block in the FAT */
            for (i = 0; i < imax; i++,pd++,baseblock++)
            {
                /* If the block is dirty */
                if ( (*pd) || ((i == 0) && (pfs->block_0_is_valid)) )
                {
                    /* Write it */
                    if ( !pc_bdevsw[pdr->driveno].io_proc(pdr->driveno, 
                             baseblock+pfs->base_block, pf, (UINT16) 1, NO) )
                    {
                        /* Release the drive I/O locks. */
                        PC_DRIVE_IO_EXIT(pdr->driveno)
                        pc_report_error(PCERR_FAT_FLUSH);
                        ret_stat = NUF_IO_ERROR;
                        done = 1;
                        break;
                    }
                    if (is_last)
                        *pd = (UINT8) 0;
                }
                pf += 512;
            }
            if (done)
                break;
        }

        if (!done)
        {
            pdr->fat_is_dirty = NO;
            /* Release the drive I/O locks. */
            PC_DRIVE_IO_EXIT(pdr->driveno)
        }
    }

    /* If FAT32 update FSINFO. */
    if ((pdr->fasize == 8) && (pdr->valid_fsinfo))
    {
        ret_stat = pc_update_fsinfo(pdr);
    }

    return(ret_stat);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_freechain                                                    
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Trace the cluster chain starting at cluster and return all the  
*       clusters to the free state for re-use. The FAT is not flushed.  
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *pdr                                Drive information           
*       cluster                             Free start cluser number    
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       NU_SUCCESS                          If service is successful.   
*       NUF_IO_ERROR                        Driver I/O error.            
*       NUF_INTERNAL                        Nucleus FILE internal error.
*                                                                       
*************************************************************************/
STATUS pc_freechain(DDRIVE *pdr, UINT32 cluster)
{
STATUS      ret_stat;
UINT32      nextcluster;


    /* Get the next cluster in a cluster chain. */
    ret_stat = pc_clnext(&nextcluster, pdr, cluster);

    while ( (cluster) && (ret_stat == NU_SUCCESS) )
    {
        /* Release the cluster. */
        ret_stat = pc_clrelease(pdr, cluster);
        if (ret_stat == NU_SUCCESS)
        {
            /* Get the next cluster in a cluster chain. */
            cluster = nextcluster;
            ret_stat = pc_clnext(&nextcluster, pdr, nextcluster);
        }
    }

    return(ret_stat);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_get_chain                                                    
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Starting at start_cluster return the number of contiguous       
*       clusters allocated in the chain containing start_cluster or     
*       n_clusters, whichever is less.                                  
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *pdr                                Drive information           
*       start cluster                       Start of the search cluster  
*       *pnext_cluster                      Next cluster number         
*       n_clusters                          Number of the cluster that  
*                                            is needed.                 
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       This function should always return at least one,   
*       unless an error occurs.                        
*                                                                       
*       NUF_IO_ERROR                        Driver I/O error.            
*       NUF_INTERNAL                        Nucleus FILE internal error.
*                                                                       
*************************************************************************/
INT32 pc_get_chain(DDRIVE *pdr, UINT32 start_cluster, UINT32 *pnext_cluster,
                   UINT32 n_clusters)
{
STATUS      ret_stat;
UINT32      clno;
UINT32      n_contig;
UINT32      value;
INT16       nibs_per_entry;


    /* Check range. */
    if (start_cluster < 2L)
        return(NUF_INTERNAL);
    if (start_cluster > pdr->maxfindex)
        return(NUF_INTERNAL);

    /* Get FAT size. */
    nibs_per_entry = pdr->fasize;

    clno = start_cluster;
    n_contig = 1L;
    *pnext_cluster = 0L;

    /* Get each FAT entry. If its value points to the next contiguous entry
       continue. Otherwise we have reached the end of the contiguous chain.
       At which point we return the number of contig's found and by reference
       the address of the FAT entry beginning the next chain segment.
    */
    while (1)
    {
        /* Get the FAT entry value. */
        ret_stat = pc_faxx(pdr, clno, &value);
        if (ret_stat != NU_SUCCESS)
           return(ret_stat);

            /* check for a bad cluster and skip it if we see it */
        if ( (value == 0xff7 && nibs_per_entry == 3)
             || (value == 0xfff7 && nibs_per_entry == 4)
             || (value == 0x0ffffff7) )
        {
            clno += 1L;
        }
            /* check for end markers set next cluster to the last 
               cluster in the chain if we are at the end */
        else if ( (value > 0xff7 && nibs_per_entry == 3)
             || (value > 0xfff7 && nibs_per_entry == 4)
             || (value > 0x0ffffff7) )
        {
            value = clno;
            break;
        }
        else if (value == ++clno)
        {
            if (n_contig >= n_clusters)
               break;

            /* Increment contiguous clusters. */
            n_contig++;
        }
        else
            break;
    }

    /* Set end of the contiguous chain. */
    *pnext_cluster = value;

    return(n_contig);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_pfaxx                                                        
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Given a DDRIVE, cluster number, and value, write the value in the 
*       fat at "clno". Handle 32, 16 and 12 bit FATs correctly.      
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *pdr                                Drive information.          
*       clno                                Cluster number to get FAT   
*                                            entry value.               
*       pvalue                              FAT entry value.            
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       If any error occurred while FAT swapping return negative value   
*       else return NU_SUCCESS                                          
*                                                                       
*       NU_SUCCESS                          If service is successful.   
*       NUF_NO_MEMORY                       Can't allocate FAT cache    
*                                            buffer. (FAT12 only )      
*       NUF_IO_ERROR                        Driver I/O error.            
*       NUF_INTERNAL                        Nucleus FILE internal error.
*                                                                       
*************************************************************************/
STATUS pc_pfaxx(DDRIVE *pdr, UINT32 clno, UINT32 value)
{
STATUS      ret_stat = NU_SUCCESS;
UINT16      wtry;
UINT32      ltry;
UINT8       wrdbuf[4];          /* Temp storage area */
UINT8 FAR   *fat_data = pdr->fat_swap_structure.data_array;
UINT32 FAR  *ppage;
UINT32      offset;
INT16       driveno;
UINT16      wvalue;


    pdr->fat_is_dirty = YES;
    driveno = pdr->driveno;
    /* ======================== FAT12 ======================== */
    if (pdr->fasize == 3)       /* 3 nibble ? */
    {
        if ( (fat_data == NU_NULL) || ((fat_data[0] & (UINT8)0xF0) != (UINT8)0xF0) )
        {
            if (fat_data == NU_NULL)
            {
                pdr->fat_swap_structure.data_array =
                                    (UINT8 FAR *)NUF_Alloc(pdr->fat_swap_structure.n_blocks_total << 9 );
                trace_new(TRACE_ATA | TRACE_LEVEL_4, "pc_faxx: 12bit FAT fat_data == NULL %d  %s \r\n", __LINE__, __FILE__); 
                if (pdr->fat_swap_structure.data_array == NU_NULL)
                {
                    trace_new(TRACE_ATA | TRACE_LEVEL_ALWAYS, "pc_faxx: 12bit FAT can't read all FAT %d  %s \r\n", __LINE__, __FILE__); 
                    return(NUF_NO_MEMORY);
                }
            }

#ifdef DEBUG
            else 
			{
              trace_new(TRACE_ATA | TRACE_LEVEL_ALWAYS, "pc_faxx: 12bit FAT fat_data[0] != 0xF9 %d  %s \r\n", __LINE__, __FILE__); 
			}
#endif

            /* Grab the device driver. */
            PC_DRIVE_IO_ENTER(driveno)
            if ( !pc_bdevsw[driveno].io_proc(driveno, (UINT32)(pdr->numhide + pdr->fatblock),
                            pdr->fat_swap_structure.data_array, (UINT16) pdr->secpfat, YES) )
            {
                /* Release the drive I/O locks. */
                PC_DRIVE_IO_EXIT(driveno)
                pc_report_error(PCERR_FATREAD);
                NU_Deallocate_Memory((VOID *)pdr->fat_swap_structure.data_array);

                return(NUF_IO_ERROR);
            }
            PC_DRIVE_IO_EXIT(driveno)
        }

        value &= 0x0fff;        /* 3 nibble clusters */
        wtry = (UINT16) (clno + (UINT16) (clno >> 1)); /* multiply by 1.5 */
        if ( (UINT16)((clno << 1) + clno) == (wtry << 1) )  /* If whole number */
        {
            fat_data[wtry] = (UINT8)((UINT8)value & 0xff); /* Low Byte to NIBBLE 1 & 2 */
            fat_data[wtry+1] &= 0xf0; /* clr low NIBBLE of next byte */
            /* Put the high nibble of value in the low nibble of next byte */
            fat_data[wtry+1] |= ( ((UINT8)(value >> 8)) & 0x0f );
        }
        else
        {
            fat_data[wtry+1]= (UINT8)((UINT8)(value >> 4) & 0xff); /* high to NIB 2 & 3*/
            fat_data[wtry] &= 0x0f; /* clr high NIBBLE of byte */
            /* Put the low nibble of value in the high nibble of byte */
            fat_data[wtry] |= ( ((UINT8)(value & 0xf) << 4) & 0xf0 );
        }
        /* Now mark the dirty flags block == index/512 */
        /* Note try >> 9 and try+1 >> 9 are usually the same */
        /* NOTE: data_map is a byte map of dirty fat blocks */
        pdr->fat_swap_structure.data_map[(wtry >> 9)]  = (UINT8) 1;
        pdr->fat_swap_structure.data_map[(wtry+1) >> 9] = (UINT8) 1;
    }


    /* ======================== FAT16 ======================== */
    else if (pdr->fasize == 4)
    {
        value &= 0xffff;        /* 4 nibble clusters */

        /* Use temp buffer since fat_data is far , can't call far word with
          it directly in intel small model */
        wvalue = value;
        SWAP16((UINT16 *)&wrdbuf[0],&wvalue);

        if (!pdr->use_fatbuf)
        {
        /* Using the in-memory FAT Buffers */
           /* 16 BIT entries */
            /* Byte offset in fat == cluster number * 2 */
            ltry = (UINT32) clno;
            ltry <<= 1;

            fat_data[ltry] = wrdbuf[0];
            fat_data[ltry+1] = wrdbuf[1];
            /* Now mark the dirty flags block == index/512 */
            /* NOTE: data_map is a byte map of dirty fat blocks */
            pdr->fat_swap_structure.data_map[(ltry >> 9)]  = (UINT8) 1;
        }
        else
        {   /* fat swapping. Always 16 or 32 bit entries */

            /* Now put the values back into the FAT */
            ret_stat = pc_pfpword(pdr, (UINT16)clno, (UINT16 *) &wrdbuf[0]);
            if (ret_stat != NU_SUCCESS)
            {
                return(ret_stat);
            }
        }
    }


    /* ======================== FAT32 ======================== */
    else if (pdr->fasize == 8)
    {   /* 32 BIT entries */
        value &= 0x0fffffff;        /* 7 nibble clusters */

        SWAP32((UINT32 *)&wrdbuf[0],&value);

        if (!pdr->use_fatbuf)
        {
        /* Using the in-memory FAT Buffers */

            /* Byte offset in FAT == cluster number * 4 */
            ltry = (UINT32) clno;
            ltry <<= 2;

            /* Use temp buffer since fat_data is far , can't call far word with
               it directly in intel small model */
            fat_data[ltry] = wrdbuf[0];
            fat_data[ltry+1] = wrdbuf[1];
            fat_data[ltry+2] = wrdbuf[2];
            fat_data[ltry+3] = wrdbuf[3];
            /* Now mark the dirty flags block == index/512 */
            /* NOTE: data_map is a byte map of dirty FAT blocks */
            pdr->fat_swap_structure.data_map[(ltry >> 9)]  = (UINT8) 1;
        }
        else 
        {   /* FAT swapping. Always 16 or 32 bit entries */


            /* Make sure we have access to the page. Mark it for writing */
             ret_stat = pc_pfswap((UINT8 FAR **)&ppage, pdr, clno, YES);
            if (ret_stat != NU_SUCCESS)
            {
                trace_new(TRACE_ATA | TRACE_LEVEL_ALWAYS, "pc_faxx: pc_pfswap returned error %d  %s \r\n", __LINE__, __FILE__); 
                return(ret_stat);
            }
            /* there are 128 entries per page */
            offset = (UINT16) (clno & 0x7f);
            ppage[offset] = *((UINT32 *)wrdbuf);
        }
    }
    else
    {
        trace_new(TRACE_ATA | TRACE_LEVEL_ALWAYS, "pc_faxx:pdr->fasize error %d  %s \r\n", __LINE__, __FILE__); 
        return(NUF_INTERNAL);
    }

    return(NU_SUCCESS);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_pfswap                                                       
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Provide a caching mechanism for the file allocation table. We  
*       have a byte map table indicating the offset in our data cache   
*       for each block in the FAT. If the value at dat_map[block] is    
*       zero, the block is not cached. We also maintain a bitmap of each 
*       block in the FAT. If the bit is set, the corresponding block must
*       be flushed.                                                     
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       **pdata                             FAT buffer pointer          
*       *pdr                                Drive information           
*       index                               FAT entry number(cluster no)
*       for_write                           FAT update flag             
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       NU_SUCCESS                          If service is successful.   
*       NUF_IO_ERROR                        Driver I/O error.            
*       NUF_INTERNAL                        Nucleus FILE internal error.
*                                                                       
*************************************************************************/
STATUS pc_pfswap(UINT8 FAR **pdata, DDRIVE *pdr, UINT32 index, INT for_write)
{
FATSWAP     *pfs = &pdr->fat_swap_structure;
UINT32      newblock;
UINT32      block_offset_in_fat;
UINT32      i;
UINT8       uc;
UINT16      byte_offset;
UINT16      bit_offset;
UINT16      data_block_to_use;
UINT32      ltemp;
STATUS      ret_stat;


    if (pdr->fasize <= 4)
    {   /* FAT16 */
        /* Convert the index (in words) to block values 
           divide by 256 since there are 256 FAT entries per block */
        block_offset_in_fat = (UINT32) (index >> 8);
    }
    else
    {   /* FAT32 */
        /* Convert the index (in words) to block values 
           divide by 128 since there are 128 FAT entries per block */
        block_offset_in_fat = (UINT32) (index >> 7);
    }

    if (block_offset_in_fat >= pdr->secpfat) /* Check range */
	{
        trace_new(TRACE_ATA | TRACE_LEVEL_ALWAYS,"pc_pfswap: Internal RANGE: index %d, block_offset_in_fat %d >= secpfat %d line %d\n",index,block_offset_in_fat,pdr->secpfat,__LINE__); 
        return(NUF_INTERNAL);
    }

    /* check if the block is already mapped. 
           Notice we use block_0_is_valid as a flag denoting whether
           the first block in the fat is buffered. We did this because
           we want to use:
                data_map[block_offset_in_fat] == 0;
           Where data_map[] indicates the offset into our cache memory
           (in blocks) of the the cached buffer.  
           To indicate that the disk block is not cached. Since offset 0
           is a valid number we chose to preload the first disk block in the
           fat to location 0 in our buffer cache. After this has occurred we
           set block_0_is_valid. Yuk, but effective.. 
    */
    /* Block 0 read.  */
    *pdata = (UINT8 FAR *)0;

    if ( (block_offset_in_fat < pfs->base_block ) ||
        (block_offset_in_fat >= pfs->base_block + pfs->data_map_size) )
    {
        /* The block is not in FAT cache */
        ret_stat = pc_pfflush(pdr);
        if (ret_stat != NU_SUCCESS)
            return(ret_stat);

        /* Clear the data_map. */
        for (i = 0; (INT32)i < pfs->data_map_size; i++)
        {
            pfs->data_map[i] = 0;
        }

        /* Initialize FATSWAP. */
        pfs->block_0_is_valid = 0;
        pfs->base_block = block_offset_in_fat;
        block_offset_in_fat = 0;
        pfs->n_blocks_used = 0;
        pfs->n_to_swap = 1;
    }
    else
    {
        /* What is the index number in FAT cache */
        block_offset_in_fat -= pfs->base_block;

        if ( ((block_offset_in_fat == 0) && pfs->block_0_is_valid) || 
              (pfs->data_map[block_offset_in_fat] != 0) )
        {
            /* Already in our cache. set up a pointer to it */
            data_block_to_use = pfs->data_map[block_offset_in_fat];
            ltemp = (UINT32) data_block_to_use;
            *pdata = pfs->data_array + (ltemp << 9);
        }
    }

    if (*pdata == (UINT8 FAR *)0)
    {
        /* data_map > FAT cache size */
        if (pfs->n_blocks_total < pfs->data_map_size)
        {
            /* Not mapped, we have to read it in */
            if (pfs->n_blocks_used < (UINT16)(pfs->n_blocks_total))
            {
            /* If we haven't reached steady state: use core until we 
               get there. */
                data_block_to_use = pfs->n_blocks_used;
                pfs->n_blocks_used += 1;
            }
            else
            {
                /* Flush the buffer
                    Later make this more selective by only flushing the block we
                    are swapping */
                ret_stat = pc_pfflush(pdr);
                if (ret_stat != NU_SUCCESS)
                    return(ret_stat);

                /* Select the block to swap out. Use a simple round-robin
                   selection algorithm: pfs->n_to_swap is the current 
                   target for swapping (block 0 is never swapped out)  */        
                if (!pfs->n_to_swap)
                    pfs->n_to_swap += 1;

                if (pfs->n_to_swap >= (UINT16)(pfs->n_blocks_total))
                    pfs->n_to_swap = 1;

                data_block_to_use = 0;
                for (i = 0; (INT)i < pfs->data_map_size; i++)
                {
                    if (pfs->data_map[i] == pfs->n_to_swap)
                    {
                        /* swap this one out */
                       data_block_to_use = pfs->data_map[i];
                       pfs->data_map[i] = 0;
                       break;
                    }
                }
                pfs->n_to_swap += 1;

#ifdef DEBUG
                if (!data_block_to_use)
                {
                    trace_new(TRACE_ATA | TRACE_LEVEL_ALWAYS,"pc_pfswap:  FAT cache broken \r\n"); 
                }
#endif
            }
        }       /* END IF we had to swap */
        else
        {
            data_block_to_use = block_offset_in_fat;
        }

        /* Whether we are reusing a block or still coming up to speed,
           if the block is not currently in our pool, we must mark the
           data block used and read it in */
        pfs->data_map[block_offset_in_fat] = data_block_to_use;

        newblock = pdr->numhide + pdr->fatblock + block_offset_in_fat + pfs->base_block;
        ltemp = (UINT32) data_block_to_use;
        *pdata = pfs->data_array + (ltemp << 9);

        /* READ */
        /* Grab the device driver. */
        PC_DRIVE_IO_ENTER(pdr->driveno)
        if ( !pc_bdevsw[pdr->driveno].io_proc(pdr->driveno, newblock, *pdata, (UINT16) 1, YES) )
        {
            pfs->data_map[block_offset_in_fat] = 0;
            /* Release the drive I/O locks. */
            PC_DRIVE_IO_EXIT(pdr->driveno)

            trace_new(TRACE_ATA | TRACE_LEVEL_ALWAYS,"pc_pfswap: IO_ERROR block %d line %d\r\n",newblock,__LINE__); 

            return(NUF_IO_ERROR);
        }

        if (block_offset_in_fat == 0)
            pfs->block_0_is_valid = 1;

        /* Release the drive I/O locks. */
        PC_DRIVE_IO_EXIT(pdr->driveno)
    }        /* END IF the data needed reading in */

    /* If we should mark it dirty, do so in the bit map */
    if (for_write)
    {
        byte_offset = (UINT16) (block_offset_in_fat >> 3);    /* divide by 8 */
        bit_offset = (UINT16) (block_offset_in_fat & 0x7);    /* mod 8 */
        uc = 1;
        uc <<= bit_offset;
        pfs->pdirty[byte_offset] |= uc;
    }

    return(NU_SUCCESS);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_pfpword                                                      
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Put a WORD value into the FAT at index.                         
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *pdr                                Drive information           
*       index                               FAT entry number            
*       *value                              FAT value pointer           
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       NU_SUCCESS                          If service is successful.   
*       NUF_IO_ERROR                        Driver I/O error.            
*       NUF_INTERNAL                        Nucleus FILE internal error.
*                                                                       
*************************************************************************/
STATUS pc_pfpword(DDRIVE *pdr, UINT16 index, UINT16 *pvalue)
{
UINT16 FAR  *ppage;
UINT16      offset;
STATUS      ret_stat;


    /* Make sure we have access to the page. Mark it for writing */
    ret_stat = pc_pfswap((UINT8 FAR **)&ppage, pdr, (UINT32)index, YES);

    if (ret_stat == NU_SUCCESS)
    {
        /* there are 256 entries per page */
        offset = (UINT16) (index & 0xff);
        ppage[offset] = *pvalue;
    }

    return(ret_stat);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_pfgword                                                      
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Get a WORD value from the FAT at index.                         
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *pdr                                Drive information           
*       index                               FAT entry number            
*       *value                              FAT value pointer           
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       NU_SUCCESS                          If service is successful.   
*       NUF_IO_ERROR                        Driver I/O error.            
*       NUF_INTERNAL                        Nucleus FILE internal error.
*                                                                       
*************************************************************************/
STATUS pc_pfgword(DDRIVE *pdr, UINT16 index, UINT16 *value)
{
STATUS      ret_stat;
UINT16 FAR  *ppage;
UINT16      offset;


    /* Make sure we have access to the page. Don't Mark it for writing */
    ret_stat = pc_pfswap((UINT8 FAR **)&ppage, pdr, (UINT32)index, NO);

    if (ret_stat == NU_SUCCESS)
    {
        /* there are 256 entries per page */
        offset = (UINT16) (index & 0xff);
        *value = ppage[offset];
    }

    return(ret_stat);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_pfflush                                                      
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Consult the dirty fat block list and write any. write all copies
*       of the FAT.                                                     
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *pdr                                Drive management structure  
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       NU_SUCCESS                          Success all FAT cache flushed.
*       NUF_IO_ERROR                        Driver I/O error.            
*                                                                       
*************************************************************************/
STATUS pc_pfflush(DDRIVE *pdr)
{
FATSWAP     *pfs = &pdr->fat_swap_structure;
UINT8 FAR   *pdata;
INT16       i;
UINT32      j, jmax;
UINT8       uc;
UINT8       bit_in_array;
UINT16      offset_div_8;
UINT32      blockno;
UINT16      offset_in_block_map;
UINT32      data_offset;
UINT32      baseblock;
INT         is_last;
STATUS      stat;

    /* Lock this drive. */
    PC_DRIVE_IO_ENTER(pdr->driveno)

    stat = NU_SUCCESS;

    /* Move the number of FAT to local */
    jmax = pdr->secpfat;

    /* Loop for the number of FAT */
    for (j = 0; j < pdr->numfats; j++)
    {
        /* Work out the FAT start block number */
        baseblock = (UINT32) j; 
        baseblock *= (UINT32) jmax; 
        baseblock += (pdr->numhide + pdr->fatblock); 
        
        /* Is this FAT last ? */
        if ( j == (UINT16) (pdr->numfats - 1) )
            is_last = YES;
        else
            is_last = NO;

        /* Check the pfs->pdirty. If the bit of this is 0,  
           corresponding FAT cache block needs to be flushed */
        for (offset_div_8 = 0, i = 0; i < (pfs->data_map_size >> 3 ); i++,offset_div_8 += 8)
        {
            uc = pfs->pdirty[i];
            if (is_last)
                pfs->pdirty[i] = 0;
            bit_in_array = 0;
            while (uc)
            {
                if (uc & 0x01)          /* If bit is dirty. */
                {
                    /* add (by oring) the byte index and the bit index
                       to get the block offset in the fat */
                    offset_in_block_map = (UINT16)(offset_div_8 | bit_in_array); 
                    /* map the offset to data through our map */
                    data_offset = (UINT32) pfs->data_map[offset_in_block_map];
                    /* Convert block offset to byte offset */
                    data_offset <<= 9;
                    /* Get its address in our data array */
                    pdata = pfs->data_array + data_offset;
                    /* Convert offset in fat to logical disk block */
                    blockno = baseblock + offset_in_block_map + pfs->base_block;

                    /* WRITE IT */
                    if ( !pc_bdevsw[pdr->driveno].io_proc(pdr->driveno, blockno, pdata, (UINT16) 1, NO) )
                    {
                         pc_report_error(PCERR_FAT_FLUSH);
                         stat = NUF_IO_ERROR;
                         break;
                    }
                }
                uc >>= 1;
                bit_in_array++;
            }
            if ( stat == NUF_IO_ERROR )
                break;
        }
        if ( stat == NUF_IO_ERROR )
            break;
    }

    /* Clear the dirty map */
    PC_DRIVE_IO_EXIT(pdr->driveno)
    return(stat);
}





/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_clzero                                                       
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Write zeros into the cluster at "cluster" on the drive pointed 
*       to by pdrive. Used to zero out directory and data file clusters 
*       to eliminate any residual data.                                 
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *pdrive                             Drive information           
*       cluster                             Clear cluster number        
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       NU_SUCCESS                          If service is successful.   
*       NUF_IO_ERROR                        Driver I/O error.            
*                                                                       
*************************************************************************/
STATUS pc_clzero(DDRIVE *pdrive, UINT32 cluster)
{
STATUS      status = NU_SUCCESS;
UINT16      i;
UINT32      currbl;
//UINT8       buf[512];
static UINT8 *buf = (UINT8 *)0;

    if( !buf )
	{
       buf = (UINT8 *)mem_nc_malloc(512);
	   if( !buf )
	   {
	     trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS, "pc_clzero: NU_NO_MEMORY line %d.\n",__LINE__);
	     return( NU_NO_MEMORY );
	   }
    }
#if 0
/* DAVEM - 30GB WD300 ONLY */
            if( (UINT32)cluster > 457672 )
			{
	          trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"pc_clzero: out of range=%d line %d.\n",cluster,__LINE__);
			}
#endif

    currbl = pc_cl2sector(pdrive, cluster);
    pc_memfill(buf, 512, '\0');

    /*Init and write a block for each block in cl. Note: init clears the core*/
    for (i = 0; i < pdrive->secpalloc; i++, currbl++ )
    {
        /* Initialize a BLKBUFF. */
        status = pc_init_blk(pdrive, currbl);
        if (status != NU_SUCCESS)
        {
            break;
        }
        PC_DRIVE_IO_ENTER(pdrive->driveno)
        /* WRITE IT */
        if ( !pc_bdevsw[pdrive->driveno].io_proc(pdrive->driveno, currbl, buf, (UINT16) 1, NO) )
        {
            PC_DRIVE_IO_EXIT(pdrive->driveno)
            status = NUF_IO_ERROR;
            break;
        }

        /* Release the drive I/O locks. */
        PC_DRIVE_IO_EXIT(pdrive->driveno)
    }

    return(status);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_drno2dr                                                      
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Given a drive number look up the DDRIVE structure associated    
*       with it.                                                        
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       driveno                             Drive number                
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       Returns NULL if driveno is not an open drive.                   
*                                                                       
*************************************************************************/
DDRIVE *pc_drno2dr(INT16 driveno)
{
DDRIVE      *pdr;
DDRIVE      *ret_val = NU_NULL;


    /* Check drive number */
    if ( (driveno >= 0) && (driveno < NDRIVES) )
    {
        /* Find the drive. */
        pdr = (DDRIVE *)NUF_Drive_Pointers[driveno];
        if (pdr != NU_NULL)
        {
            if (pdr->opencount)
                ret_val = pdr;
        }
    }

    return(ret_val);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_dskfree                                                      
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Given a valid drive number. If the drive open count goes to     
*       zero, free the file allocation table and the block zero         
*       information associated with the drive. If unconditional is true,
*       ignore the open count and release the drive.                    
*       If open count reaches zero or unconditional, all future accesses
*       to driveno will fail until re-opened.                           
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       driveno                             Drive number                
*       unconditional                       Free flag                   
*                                                                       
* OUTPUTS
*                                                                       
*       NU_SUCCESS                          If service is successful.   
*       NUF_NOT_OPENED                      The disk is not opened yet. 
*       NUF_IO_ERROR                        Driver I/O error.            
*                                                                       
*************************************************************************/
STATUS pc_dskfree(INT16 driveno, INT unconditional)
{
DDRIVE      *pdr;
STATUS      ret_stat = 1;


    pdr = (DDRIVE *)NUF_Drive_Pointers[driveno];

    if (!pdr)
        ret_stat = NUF_NOT_OPENED;

    /* If unconditional close. Fake it out by setting open count to 1 */
    if (unconditional && (ret_stat != NUF_NOT_OPENED))
        pdr->opencount = 1;

    if ((pdr->opencount == 1) && (ret_stat != NUF_NOT_OPENED))
    {
       /* Free the current working directory for this drive for all users */
        pc_free_all_users(driveno);
        /* Free all files, finodes & blocks associated with the drive */
        pc_free_all_fil(pdr);
        pc_free_all_i(pdr);
        pc_free_all_blk(pdr);

        /* Free fat blocks */
        if (pdr->fat_swap_structure.data_array)
            NU_Deallocate_Memory(pdr->fat_swap_structure.data_array);

        /* Tell the device driver we are done. */
        ret_stat = pc_bdevsw[driveno].close_proc(driveno);

        pdr->fat_swap_structure.data_array = (UINT8 FAR *)0;

        if (pdr->fat_swap_structure.data_map)
            NU_Deallocate_Memory(pdr->fat_swap_structure.data_map);

        if (pdr->fat_swap_structure.pdirty)
            NU_Deallocate_Memory(pdr->fat_swap_structure.pdirty);

        /* Deallocate Drive memory. */
        NU_Deallocate_Memory(pdr);

        /* Clear DDRIVE pointer. */
        NUF_Drive_Pointers[driveno] = (UNSIGNED *)0;
    }

    if (pdr->opencount && (ret_stat != NUF_NOT_OPENED))
        pdr->opencount -= 1;

    if (ret_stat == 1)
        ret_stat = NU_SUCCESS;
    else if (ret_stat != NUF_NOT_OPENED)
        ret_stat = NUF_IO_ERROR;

    return(ret_stat);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_ifree                                                        
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Given a drive number count the number of free clusters on the   
*       drive.                                                          
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       driveno                             Drive number                
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       The number of free cluster or zero if the drive is full or it   
*       not open or out of range.                                       
*                                                                       
*************************************************************************/
UINT32 pc_ifree(INT16 driveno)
{
DDRIVE      *pdr;
UINT32      i;
UINT32      nxt;
UINT32      freecount = 0L;
INT32       nbad = 0L;
INT32       nres = 0L;
UINT32      ret_val = 1;


    /* Find the drive */
    pdr = pc_drno2dr(driveno);
    if (!pdr)
    {
        ret_val = 0L;
    }

    /* Set the free cluster count */
	/* NUF_FSINFO_UPDATE */
    if (pdr->valid_fsinfo == 2 && ret_val)
    {
        freecount = pdr->free_clusters_count;
    }
    else if (ret_val)
    {
        /* Search the free cluster on the drive. */
        for (i = 2; i <= pdr->maxfindex; i++)
        {
            /* Get the FAT entry value. */
            if (pc_faxx(pdr, i, &nxt) < 0)
            {
                ret_val = 0L;
                break;
            }
            if (nxt == 0)
			{
                freecount++;
            }

            /* Bad cluster mark? */
            if ( (pdr->fasize == 3 && nxt == 0xff7) ||
                 (pdr->fasize == 4 && nxt == 0xfff7) ||
                 (pdr->fasize == 8 && nxt == 0x0ffffff7) )
            {
                nbad += 1;
            }

            /* Increment use cluster count. */
            /* FAT16 */
            if ( pdr->fasize == 4 && (nxt >= 0xfff0 && nxt <= 0xfff6) )
            {
                nres += 1;
            }
            /* FAT12 */
            else if ( pdr->fasize == 3 && (nxt >= 0xff0 && nxt <= 0xff6) )
            {
                nres += 1;
            }
            /* FAT32 */
            else if ( pdr->fasize == 8 && 
                      (nxt >= 0x0ffffff0 && nxt <= 0x0ffffff6) )
            {
                nres += 1;
            }

        } /* for (i = 2; i <= pdr->maxfindex; i++) */

        if (ret_val)
        {
            /* Set the free cluster count. */
            pdr->free_clusters_count = freecount;
            /* NUF_FSINFO_UPDATE */
            pdr->valid_fsinfo = 2;
        }
    }
    if (ret_val)
        ret_val = freecount;


    return(ret_val);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_sec2cluster                                                  
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Convert blockno to its cluster representation if it is in       
*       cluster space.                                                  
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *pdrive                             Drive information           
*       blockno                             Convert blockno             
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       Returns 0 if the block is not in cluster space, else returns the
*       cluster number associated with block.                           
*                                                                       
*************************************************************************/
UINT32 pc_sec2cluster(DDRIVE *pdrive, UINT32 blockno)           /*__fn__*/
{
UINT32      answer = 1;


    /* Check data area. */
    if (blockno >= pdrive->numsecs)
        answer = 0L;
    /* Root directort area? */
    else if (pdrive->firstclblock > blockno)
        answer = 0L;
    else if (answer)
    {
        /* Convert sector number to cluser number. */
        /*  (2 + (blockno - pdrive->firstclblock)/pdrive->secpalloc) */
        answer = blockno - pdrive->firstclblock;
        answer = answer >> pdrive->log2_secpalloc;
        answer += 2;
    }

    return(answer);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_sec2index                                                    
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Given a block number offset from the beginning of the drive,    
*       calculate which block number within a cluster it will be. If the
*       block number coincides with a cluster boundary, the return value
*       will be zero. If it coincides with a cluster boundary + 1 block,
*       the value will be 1, etc.                                       
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *pdrive                             Drive information           
*       blockno                             Block number                
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       Offset block number of in this cluster.                         
*                                                                       
*************************************************************************/
UINT16 pc_sec2index(DDRIVE *pdrive, UINT32 blockno)
{
UINT32      answer;


    /*  ((blockno - pdrive->firstclblock) % pdrive->secpalloc) ); */

    answer = blockno - pdrive->firstclblock;
    answer = answer % pdrive->secpalloc;

    return((UINT16) answer);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_cl2sector                                                    
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Convert cluster number to a blocknumber.                        
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *pdrive                             Drive information           
*       cluster                             Convert cluster number      
*                                                                       
* OUTPUTS                                                               
*                                                                       
*      Returns 0 if the cluster is out of range; else returns the       
*      block number of the beginning of the cluster.                    
*                                                                       
*************************************************************************/
UINT32 pc_cl2sector(DDRIVE *pdrive, UINT32 cluster)             /*__fn__*/
{
UINT32      blockno = 0;
UINT32      t;
UINT32      ret_val = 1;


    if (cluster < 2L)
    {
        /*trace_new(TRACE_ATA | TRACE_LEVEL_4, "pc_cl2sector: cluster %d < 2, line %d\n",cluster,__LINE__);*/
        ret_val = BLOCKEQ0;
    }

    if (ret_val)
    {
        /* Calculate sector number. */
        t = cluster - 2L;
        t = t << pdrive->log2_secpalloc;

        /* Get physical sector number. */
        blockno = pdrive->firstclblock + t;
    }

    /* Check data area. */
    if ((blockno >= pdrive->numsecs) && (ret_val))
    {
        /*trace_new(TRACE_ATA | TRACE_LEVEL_4, "pc_cl2sector blockno %d >= pdrive->numsecs %d line %d\n",blockno,pdrive->numsecs,__LINE__); */
        /*trace_new(TRACE_ATA | TRACE_LEVEL_4, "pc_cl2sector pdrive->firstclblock=%d, t=%d, cluster=%d, pdrive->log2_secpalloc=%d line %d\n",pdrive->firstclblock,t,cluster,pdrive->log2_secpalloc,__LINE__); */
        ret_val = BLOCKEQ0;
    }
    else
        ret_val = blockno;

    return(ret_val);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_update_fsinfo                                                
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Update the FSINFO in order to keep Free Clusters item up to date   
*       on FAT32 disks.                                                 
*                                                                       
*                                                                       
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *pdr                                Drive management structure  
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       NU_SUCCESS                          Success all FAT cache flushe.
*       NUF_IO_ERROR                        Driver I/O error.            
*                                                                       
*************************************************************************/
STATUS pc_update_fsinfo(DDRIVE *pdr)
{

static UINT8 *b = (UINT8 *)0;
UINT32      fstemp;
UINT16      signature, driveno, i;
STATUS      ret_val;


   if( !b )
	{
       b = (UINT8 *)mem_nc_malloc(512);
	   if( !b )
	   {
         trace_new(TRACE_ATA | TRACE_LEVEL_ALWAYS, "pc_update_fsinfo: NU_NO_MEMORY line %d\n.", __LINE__); 
	     return( NU_NO_MEMORY );
	   }
    }
    ret_val = NU_SUCCESS;
    driveno = pdr->driveno;

    /* Build up a Partition FSINFO(File System INFOrmation)
        Note: PBR include a BIOS Parameter Block(BPB) */
    pc_memfill(&b[0], 512, '\0');

    /* FSI signature offset 0 */
    pc_cppad(&b[0], (UINT8 *)"RRaA", 4);

    /* FSI signature offset 0x1E0 */
    pc_cppad(&b[0x1e4], (UINT8 *)"rrAa", 4);

    SWAP32((UINT32 *)&(b[0x1e8]), (UINT32 *)&pdr->free_clusters_count);

    SWAP32((UINT32 *)&(b[0x1ec]), (UINT32 *)&pdr->free_contig_pointer);

    /* Signature word */
    signature = 0xAA55;
    SWAP16((UINT16 *)&(b[0x1fe]), (UINT16 *)&signature);

    /* Update FSINFO */
    fstemp = (UINT32) pdr->fsinfo;

    /* Grab the device driver. */
    PC_DRIVE_IO_ENTER(driveno)

    for (i = 0; i < 2; i++)
    {
        if ( !pc_bdevsw[driveno].io_proc(driveno, fstemp, &(b[0]), (UINT16) 1, NO) )
        {
            pc_report_error(PCERR_FMTWPBR);
            ret_val = NUF_IO_ERROR;
            break;
        }
        fstemp += 6;
    }

    /* Release the drive io locks. */
    PC_DRIVE_IO_EXIT(driveno)

    return(ret_val);

}

/****************************************************************************
 * Modifications:
 * $Log: 
 *  3    mpeg      1.2         4/3/04 12:47:21 AM     Nagaraja Kolur  CR(s) 
 *        8762 8763 : Merged 2.5.6 Nup File System.
 *        Test: 1. pvr demo 2. nup file test 3. CR 8181
 *        
 *  2    mpeg      1.1         1/7/04 4:12:57 PM      Tim Ross        CR(s) 
 *        8181 : Reference FAT32 partition from it's sector offset, not sector 
 *        0.
 *  1    mpeg      1.0         8/22/03 5:32:44 PM     Dave Moore      
 * $
 * 
 *    Rev 1.0   22 Aug 2003 16:32:44   mooreda
 * SCR(s) 7350 :
 * Nucleus File sourcecode (Sector-Cluster Manipulation)
 * 
 *
 ****************************************************************************/

