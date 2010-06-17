/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                      Conexant Systems Inc. (c) 2003                      */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       apiutil.c
 *
 *
 * Description:    see ATI header below.
 *                 Initial version based on Nucleus File v2.4.4
 *
 ****************************************************************************/
/* $Header: apiutil.c, 5, 4/3/04 12:30:11 AM, Nagaraja Kolur$
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
*       APIUTIL.C                                 2.5              
*                                                                       
* COMPONENT                                                             
*                                                                       
*       Nucleus FILE                                                    
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Contains support code for user API level source code.           
*                                                                       
* DATA STRUCTURES                                                       
*                                                                       
*       None.                                                           
*                                                                       
* FUNCTIONS                                                             
*                                                                       
*       pc_dskinit                          Mount a disk.               
*       pc_idskclose                        Unmount a disk.             
*       pc_fd2file                          Map a file descriptor to a  
*                                            file structure.           
*       pc_allocfile                        Allocate a file structure.  
*       pc_freefile                         Release a file structure.   
*       pc_free_all_fil                     Release all file structures 
*                                            for a drive.               
*       pc_log_base_2                       Calculate log2(N).          
*       pc_get_cwd                          Determine cwd string from   
*                                            current directory inode.   
*       pc_upstat                           Copy directory entry info   
*                                            to a user's stat buffer.   
* DEPENDENCIES                                                          
*                                                                       
*       pcdisk.h                            File common definitions     
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

extern PC_FILE      *mem_file_pool;         /* Memory file pool list.   */
extern _PC_BDEVSW   pc_bdevsw[];            /* Driver dispatch table.   */
extern UNSIGNED     *NUF_Drive_Pointers[];  /* DDRIVE pointer list.     */
extern INT          NUF_Fat_Type[];         /* FAT type list.           */
extern INT          NUF_Drive_Fat_Size[];   /* FAT bufffer size list.   */

/* Uncomment if you want to blast away a disk! - DAVEM */ 
/*extern void    wipe_drive_config( UINT16 driveno ); DAVEM for testing  */

/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_dskinit                                                      
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Given a valid drive number, read block zero and convert its     
*       contents to File system drive information.                       
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       driveno                             Drive number                
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       NU_SUCCESS                          Mount successful.           
*       NUF_FATCORE                         Fat cache table too small.  
*       NUF_NO_PARTITION                    No partition in disk.       
*       NUF_FORMAT                          Disk is not formatted.    
*       NUF_NO_MEMORY                       Can't allocate internal     
*                                            buffer.                    
*       NUF_IO_ERROR                        Driver returned error.      
*       NUF_INTERNAL                        Nucleus FILE internal error.
*                                                                       
*************************************************************************/
STATUS pc_dskinit(INT16 driveno)
{
STATUS      ret_stat;
DDRIVE      *pdr;
UINT16      min_needed;
FATSWAP     *pfr;
UINT16      nblocks;
UINT8 FAR   *pdata;
UINT32      fatoffset;
INT         i;
UINT16      wvalue;
UINT32      ltemp;
static UINT8 *b = (UINT8 *)0;


    if( !b )
	{
       /* Uncomment if you want to blast away a disk! - DAVEM */ 
       /*wipe_drive_config( driveno );*/
       b = (u_int8 *)mem_nc_malloc(512);
       if (!b)
       {
	     trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"pc_dskinit: NUF_NO_MEMORY line %d.\n",__LINE__);
		 return( NUF_NO_MEMORY );
       }
       /* Zero the buffer */
       pc_memfill(b, 512, (UINT8) 0);
    }

    /* Grab the device driver. */
    PC_DRIVE_IO_ENTER(driveno)

    /* Get 1 block starting at 0 from driveno. (Read MBR to get partition table.) */
    /* READ */
    if ( !pc_bdevsw[driveno].io_proc(driveno, 0L, &b[0], (UINT16) 1, YES) )
    {
        PC_DRIVE_IO_EXIT(driveno)
        pc_report_error(PCERR_INITREAD);
	    trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"pc_dskinit: NUF_IO_ERROR line %d.\n",__LINE__);
        return(NUF_IO_ERROR);
    }

    /* Release the drive io locks. */
    PC_DRIVE_IO_EXIT(driveno)

    /* Find the drive. */
    pdr = (DDRIVE *)NUF_Drive_Pointers[driveno];
    if (!pdr)
    {
        /* Allocate Drive memory */
        pdr = (DDRIVE *)NUF_Alloc(sizeof(DDRIVE));
        if (!pdr)
        {
            pc_report_error(PCERR_DRVALLOC);
	        trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"pc_dskinit: NUF_NO_MEMORY line %d.\n",__LINE__);
            return(NUF_NO_MEMORY);
        }
        /* Zero the structure so all of our initial values are right */
        pc_memfill(pdr, sizeof(DDRIVE), (UINT8) 0);

        /* Move FATSWAP pointer to local. */
        pfr = &pdr->fat_swap_structure;

        /* Set FAT buffer data map size.
            Note: See DEVTABLE.C. */
        if (NUF_Drive_Fat_Size[driveno] < 256)
        {
            pfr->data_map_size = 256;
        }
        else
        {
            pfr->data_map_size = NUF_Drive_Fat_Size[driveno];
        }

	    trace_new( TRACE_ATA | TRACE_LEVEL_4,"pc_dskinit: NUF_Drive_Fat_Size %d line %d.\n",NUF_Drive_Fat_Size[driveno],__LINE__);

        /* Allocate FAT buffer data map. */
        pfr->data_map =
            (UINT16 *)NUF_Alloc(sizeof(UINT16) * pfr->data_map_size);
        if (!pfr->data_map)
        {
            pc_report_error(PCERR_DRVALLOC);
	        trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"pc_dskinit: NUF_NO_MEMORY line %d.\n",__LINE__);
            return(NUF_NO_MEMORY);
        }

        /* Allocate FAT BIT-map of blocks. */
        pfr->pdirty = (UINT8 *)NUF_Alloc(pfr->data_map_size >> 3 );
        if (!pfr->pdirty)
        {
            pc_report_error(PCERR_DRVALLOC);
	        trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"pc_dskinit: NUF_NO_MEMORY line %d.\n",__LINE__);
            return(NUF_NO_MEMORY);
        }

        /* Initialize FATSWAP structure. */
        for (i = 0; i < pfr->data_map_size; i++)
        {
            pfr->data_map[i] = 0;
            pfr->pdirty[i >> 3] = 0;
        }
        pfr->block_0_is_valid = 0;
        pfr->base_block = 0;
        /* Set drive open count. */
        pdr->opencount = 1;
        /* Set DDRIVE pointer. */
        NUF_Drive_Pointers[driveno] = (UNSIGNED *)pdr;
    }

    /* Verify MBR is valid. */
    if (b[0x1ff] != 0xaa || b[0x1fe] != 0x55)
    {
        pc_report_error(PCERR_INITMEDI);
        pdr->opencount = 0;
	     trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"pc_dskinit: invalid FAT32 volume, NUF_FORMAT reqd, b[0]=0x%x, line %d.\n",b[0],__LINE__);
        return(NUF_FORMAT);
    }

    /* Find first FAT32 primary partition. */
    for (i=0x1be; i<=0x1ee; i+=16)
    {
        if (b[i+4] == 0xb || b[i+4] == 0xc)
        {
            break;
        }
    }
    if (i > 0x1ee)
    {
        /* No FAT32 primary partition found. */
        pc_report_error(PCERR_INITMEDI);
        pdr->opencount = 0;
	    trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"pc_dskinit: no FAT32 primary partition found, NUF_FORMAT reqd, line %d.\n",__LINE__);
        return(NUF_FORMAT);
    }
    
    /* Grab the device driver. */
    PC_DRIVE_IO_ENTER(driveno)

    /* Read PBR of FAT32 volume. */
    ltemp = b[i+8];
    if ( !pc_bdevsw[driveno].io_proc(driveno, ltemp, &b[0], (UINT16) 1, YES) )
    {
        PC_DRIVE_IO_EXIT(driveno)
        pc_report_error(PCERR_INITREAD);
	    trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"pc_dskinit: NUF_IO_ERROR line %d.\n",__LINE__);
        return(NUF_IO_ERROR);
    }

    /* Release the drive io locks. */
    PC_DRIVE_IO_EXIT(driveno)

    /* Verify that we have a good dos formatted disk */
    SWAP16(&wvalue,(UINT16 *)&b[0x1fe]);
    if (wvalue != 0xAA55)
    {
        pc_report_error(PCERR_INITMEDI);
        pdr->opencount = 0;
	    trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"pc_dskinit: not a dos disk, NUF_FORMAT reqd, b[0]=0x%x, line %d.\n",b[0],__LINE__);
        return(NUF_FORMAT);
    }

    /* This drive is FAT file system */
    pdr->fs_type  = FAT_FILE_SYSTEM;

    /* Now load the structure from the buffer */
    /* OEMNAME. */
    copybuff(&pdr->oemname[0],&b[3],8);
    /* Bytes per sector. */
    SWAP16((UINT16 *)&pdr->bytspsector, (UINT16 *)&b[0xb]);
    /* Sector per allocation. */
    pdr->secpalloc = b[0xd];
    /* Reserved sectors. */
    SWAP16((UINT16 *)&pdr->fatblock,(UINT16 *)&b[0xe]);
    /* Number of FATs. */
    pdr->numfats = b[0x10];
    /* Root dir entries. */
    SWAP16((UINT16 *)&pdr->numroot,(UINT16 *)&b[0x11]);
    /* Total # sectors. */
    SWAP16(&wvalue,(UINT16 *)&b[0x13]);
    pdr->numsecs = wvalue;
    /* trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"pc_dskinit: numsecs=%d line %d\n",pdr->numsecs,__LINE__); */

    /* Media descriptor. */
    pdr->mediadesc = b[0x15];
    /* FAT size. */
    SWAP16(&wvalue,(UINT16 *)&b[0x16]);
    pdr->secpfat = wvalue;
    /* Sectors per track. */
    SWAP16((UINT16 *)&pdr->secptrk,(UINT16 *)&b[0x18]);
    /* Number of heads. */
    SWAP16((UINT16 *)&pdr->numhead,(UINT16 *)&b[0x1a]);
    /* Hidden sectors. */
    SWAP32((UINT32 *)&pdr->numhide,(UINT32 *)&b[0x1c]);
    /* Huge Sectors. */
    SWAP32((UINT32 *)&pdr->bignumsecs,(UINT32 *)&b[0x20]); 


    /* Check if running on a DOS (4.0) huge partition */
    /* If traditional total # sectors is zero, use value in extended BPB */
    if (pdr->numsecs == 0L)
        pdr->numsecs = pdr->bignumsecs;


        /* FSINFO
        Set to 0 when the free clusters count is not necessarily correct.
        Set to 1 when the free clusters count is correct. */
	/* NUF_FSINFO_DISABLE */
        pdr->valid_fsinfo = 0;

    /* Check root dir entries. */
    if ( (pdr->numroot != 0) && (pdr->secpfat != 0L) )
    {   /* FAT12/FAT16 */

        /* The first block of the root is just past the fat copies */
        pdr->rootblock = pdr->numhide + pdr->fatblock + pdr->secpfat * pdr->numfats;

        /* The first block of the cluster area is just past the root */
        /* Round up if we have to */
        pdr->firstclblock = pdr->rootblock +
                            (pdr->numroot + INOPBLOCK - 1)/INOPBLOCK;

        /*  Calculate the largest index in the file allocation table.
            Total # block in the cluster area)/Blockpercluster =='s total
            Number of clusters. Entries 0 & 1 are reserved so the highest
            valid fat index is 1 + total # clusters.
            Note: Max cluster number  = Number of clusters + 1 */
        pdr->maxfindex = (UINT32)
                (1 + (pdr->numsecs - pdr->firstclblock)/pdr->secpalloc);

        /* if < 4085 clusters then 12 bit else 16. */
        if (pdr->maxfindex < 4086)
            pdr->fasize = 3/* NU_FAT12_SIG */;
        /* < 65525 clusters 16bit fat. */
        else if (pdr->maxfindex < 65526)
            pdr->fasize = 4/* NU_FAT16_SIG */;
        else
        {
            pdr->opencount = 0;

            return(NUF_FORMAT);
        }

        pdr->phys_num = b[0x24];
        pdr->xtbootsig = b[0x26];

        /* Unique number per volume (4.0). */
        SWAP32((UINT32 *)&pdr->volid,(UINT32 *)&b[0x27]);

        /* Volume label (4.0). */
        copybuff(&pdr->vollabel[0],&b[0x2b],11);

        /* Minimum of FATs buffer block. */
        if (pdr->fasize == 3)
            min_needed =(UINT16)(pdr->secpfat & 0x00FFFFL);
        else
            min_needed = 12;

         /* set the pointer to where to look for free clusters to the contiguous
            area. On the first call to write this will hunt for the real free
            blocks. */
        pdr->free_contig_pointer = 2L;

         /* Keep track of how much free space is on the drive. (This will
            speed up pc_free()) when calculating free space */
        pdr->free_clusters_count = 0L;
    }
    else
    {   /* FAT32 */

        /* FAT size. */
        SWAP32((UINT32 *)&pdr->bigsecpfat,(UINT32 *)&b[0x24]);
        /* FAT flag. */
        SWAP16((UINT16 *)&pdr->fat_flag,(UINT16 *)&b[0x28]);
        /* File system version. */
        SWAP16((UINT16 *)&pdr->file_version,(UINT16 *)&b[0x2a]);

        /* Nucleus FILE can only operate on FAT32 ver0 */
        if ( pdr->file_version != 0 )
        {
            pdr->opencount = 0;
            return(NUF_INTERNAL);
        }


        /* Root dir start cluster. */
        SWAP32((UINT32 *)&pdr->rootdirstartcl,(UINT32 *)&b[0x2c]);
        /* Unique number per volume (4.0). */
        SWAP32((UINT32 *)&pdr->volid,(UINT32 *)&b[0x43]);
        /* Volume label (4.0). */
        copybuff(&pdr->vollabel[0],&b[0x47],11);

        /* NU_FAT32_SIG */
        pdr->fasize = 8;

        /* The first block of the root is given. */
        pdr->rootblock = pdr->numhide + pdr->fatblock + pdr->bigsecpfat * pdr->numfats;

	    trace_new( TRACE_ATA | TRACE_LEVEL_4,"pc_dskinit: rootblock %d line %d.\n",pdr->rootblock,__LINE__);
        trace_new( TRACE_ATA | TRACE_LEVEL_4,"pc_dskinit: rootdirstartcl %d line %d\n",pdr->rootdirstartcl,__LINE__);
        if (pdr->rootdirstartcl != 2L)
        {
            if (pdr->rootdirstartcl > 2L)
            {
                pdr->rootblock += ( (pdr->rootdirstartcl - 2L) * 
                                        pdr->secpalloc );
            }
            else
            {
                pdr->opencount = 0;
	            trace_new( TRACE_ATA | TRACE_LEVEL_4,"pc_dskinit: NUF_FORMAT line %d.\n",__LINE__);
                return(NUF_FORMAT);
            }
        }

        /* The first block of the cluster area is the root dir. */
        pdr->firstclblock = pdr->rootblock;
        /*  Calculate the largest index in the file allocation table.
            Total # block in the cluster area)/Blockpercluster =='s total
            Number of clusters. Entries 0 & 1 are reserved so the highest
            valid fat index is 1 + total # clusters.
            Note: Max cluster number  = Number of clusters + 1 */
        pdr->maxfindex = (UINT32)
                (1 + (pdr->numsecs - pdr->firstclblock)/pdr->secpalloc);

        trace_new( TRACE_ATA | TRACE_LEVEL_4,"pc_dskinit: largest FAT index %d line %d.\n",pdr->maxfindex,__LINE__);        /* Minimum of FATs buffer block. */
        /* Minimum of FATs buffer block. */
        min_needed = 16;
        /* Copy FAT size. */
        pdr->secpfat = pdr->bigsecpfat;

        /* FSINFO block. */
        SWAP16((UINT16 *)&pdr->fsinfo,(UINT16 *)&b[0x30]);

        /* Grab the device driver. */
        PC_DRIVE_IO_ENTER(driveno)

        /* Read the File System INFOrmation. */
        /* READ */
	/*trace_new(TRACE_ATA|TRACE_LEVEL_4,"pc_dskinit read File system info from LBA %d line %d\n",(UINT32)(pdr->numhide + pdr->fsinfo),__LINE__);*/
        if ( !pc_bdevsw[driveno].io_proc(driveno, (UINT32)(pdr->numhide + pdr->fsinfo), 
                                            &b[0], (UINT16) 1, YES) )
        {
            /* Release the drive io locks. */
            PC_DRIVE_IO_EXIT(driveno)
            pc_report_error(PCERR_INITREAD);
            pdr->opencount = 0;
	        trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"pc_dskinit: NUF_IO_ERROR line %d.\n",__LINE__);
            return(NUF_IO_ERROR);
        }
        /* Release the drive io locks. */
        PC_DRIVE_IO_EXIT(driveno)


        /* Check the signature of the file system information sector.*/
        if ( ((b[0] == 'R') && (b[1] == 'R') && (b[2] == 'a') && (b[3] == 'A') &&
             (b[0x1e4] == 'r') && (b[0x1e5] == 'r') && (b[0x1e6] == 'A') && (b[0x1e7] == 'a')) ||
             ((b[0x1fe] == 0x55) && (b[0x1ff] == 0xAA)) )
        {
		    trace_new(TRACE_ATA|TRACE_LEVEL_4,"pc_dskinit found File system signature line %d\n",__LINE__); 
            /* The count of free clusters on the drive.
                Note: Set to -1(0xffffffff) when the count is unknown. */
            SWAP32((UINT32 *)&pdr->free_clusters_count,(UINT32 *)&b[0x1e8]);
	        trace_new( TRACE_ATA | TRACE_LEVEL_4,"pc_dskinit: free cluster count 0x%x\n",pdr->free_clusters_count,__LINE__);

            /* Check the number of total free cluster  */
            if ( (pdr->free_clusters_count == (UINT32)0xffffffff) || 
                 (pdr->free_clusters_count > pdr->maxfindex) )
                pdr->free_clusters_count = 0L;
            else
                /* The free cluster count is known. */
                pdr->valid_fsinfo = 1/* NUF_FSINFO_ENABLE */;

            /* The cluster number of the cluster that was most recently allocated. */
            SWAP32((UINT32 *)&pdr->free_contig_pointer,(UINT32 *)&b[0x1ec]);
	        trace_new( TRACE_ATA | TRACE_LEVEL_4,"pc_dskinit: last alloc cluster 0x%x\n",pdr->free_contig_pointer,__LINE__);
            /* Check the number of next free cluster  */
            if (pdr->free_contig_pointer > pdr->maxfindex)
                pdr->free_contig_pointer = 2L;
	/*trace_new( TRACE_ATA | TRACE_LEVEL_4,"pc_dskinit: next free cluster %d line %d.\n",pdr->free_contig_pointer,__LINE__);*/
        }
        else
        {
            /* FSINFO Error case. */
	        trace_new( TRACE_ATA | TRACE_LEVEL_4,"pc_dskinit: did not find File system signature line %d\n",__LINE__); 
            pdr->free_clusters_count = 0L;
            pdr->free_contig_pointer = 2L;
        }
    }


    if (pdr->fasize == 3)
    {
        if (pdr->maxfindex > 0x0ff6L)
        {
            pdr->opencount = 0;
            return(NUF_FORMAT);
        }
    }
    else if (pdr->fasize == 4)
    {
        if (pdr->maxfindex > 0xfff6L)
        {
            pdr->opencount = 0;
            return(NUF_FORMAT);
        }
    }
    else    /* FAT32 */
    {
        if (pdr->maxfindex > 0x0ffffff6L)
        {
            pdr->opencount = 0;
	        trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"pc_dskinit: ret NUF_FORMAT line %d.\n",__LINE__);
            return(NUF_FORMAT);
        }
    }

    /* Check FATs buffer block. */
    if ((UINT16)(NUF_Drive_Fat_Size[driveno]) < min_needed)
    {
        pc_report_error(PCERR_FATCORE);
        pdr->opencount = 0;
		trace_new(TRACE_ATA|TRACE_LEVEL_ALWAYS,"pc_dskinit FATCORE: min_needed %d, NUF_Drive_Fat_Size[%d]=%d, line %d\n",	
		          min_needed, driveno, NUF_Drive_Fat_Size[driveno], __LINE__ ); 
        return(NUF_FATCORE);
    }

    /* Remember how many blocks we alloced. */
    pdr->fat_swap_structure.n_blocks_total = NUF_Drive_Fat_Size[driveno];
    if (pdr->fat_swap_structure.n_blocks_total >  (INT)pdr->secpfat)
    {
        pdr->fat_swap_structure.n_blocks_total = (INT)pdr->secpfat;
    }

    /* Allocate FAT cache buffer */
    pdr->fat_swap_structure.data_array =
                    (UINT8 FAR *)NUF_Alloc(pdr->fat_swap_structure.n_blocks_total << 9 );

    if (!pdr->fat_swap_structure.data_array)
    {
        pdr->opencount = 0;
	    trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"pc_dskinit: NUF_NO_MEMORY line %d.\n",__LINE__);
        return(NUF_NO_MEMORY);
    }

    /* Set FAT buffer flag. */
    if (pdr->fat_swap_structure.n_blocks_total == (INT)pdr->secpfat)
        pdr->use_fatbuf = 0;
    else
        pdr->use_fatbuf = 1;


/*#define DEBUG0*/

#ifdef DEBUG

        DEBUG_PRINT ("Oem NAME  %8s\n",pdr->oemname);
        DEBUG_PRINT ("Bytspsec  %d\n",pdr->bytspsector);
        DEBUG_PRINT ("secpallc  %d\n",pdr->secpalloc);
        DEBUG_PRINT ("secres    %d\n",pdr->fatblock);
        DEBUG_PRINT ("numfat    %d\n",pdr->numfats);
        DEBUG_PRINT ("numrot    %d\n",pdr->numroot);
        DEBUG_PRINT ("numsecs   %lu\n",pdr->numsecs);
        DEBUG_PRINT ("mediac    %d\n",pdr->mediadesc);
        DEBUG_PRINT ("secfat    %lu\n",pdr->secpfat);
        DEBUG_PRINT ("sectrk    %d\n",pdr->secptrk);
        DEBUG_PRINT ("numhed    %d\n",pdr->numhead);
        DEBUG_PRINT ("numhide   %lu\n",pdr->numhide);
        /* File system version. */
        DEBUG_PRINT ("file_version  %d\n",pdr->file_version);
        /* Root dir start cluster. */
        DEBUG_PRINT ("rootdirstartcl %lu\n",pdr->rootdirstartcl);
        /* The first block of the cluster area is the root dir */
        DEBUG_PRINT ("1st block of cluster area   %lu\n",pdr->firstclblock);
        /* FAT flag. */
        DEBUG_PRINT ("FAT flag  0x%x\n",(unsigned int)&pdr->fat_flag);
#endif

    /* Clear the current working directory */
    fs_user->lcwd[driveno] = NU_NULL;

    pdr->bytespcluster = (UINT16) (512 * pdr->secpalloc);

    /* bits to mask in to calculate byte offset in cluster from file pointer.
        AND file pointer with this to get byte offset in cluster a shift right
        9 to get block offset in cluster */
    pdr->byte_into_cl_mask = (UINT32) pdr->bytespcluster;
    pdr->byte_into_cl_mask -= 1L;

    /* DAVEM - the following trace will show up a problem if you increase cluster size to 64K */
	/*trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"pc_dskinit: pdr->byte_into_cl_mask=%d line %d.\n",pdr->byte_into_cl_mask,nblocks,__LINE__);*/

    /* save away log of sectors per alloc */
    pdr->log2_secpalloc = pc_log_base_2((UINT16)pdr->secpalloc);

    /* Number of maximum file clusters */
    pdr->maxfsize_cluster = MAXFILE_SIZE >> pdr->log2_secpalloc;

    /* Initialize the fat management code */

    /* Set driveno now becuse the drive structure is valid */  
    pdr->driveno = driveno;

    if (pdr->use_fatbuf)
    {
        /* Swap in item 0. (ie read the first page of the FAT) */
        ret_stat = pc_pfswap(&pdata, pdr, (UINT32)0, NO);
        if (ret_stat != NU_SUCCESS)
        {
            pc_report_error(PCERR_FATREAD);
            NU_Deallocate_Memory((VOID *)pdr->fat_swap_structure.data_array);
            pdr->opencount = 0;
            return(ret_stat);
        }
    }
    else
    {
        /* No fat swapping. We use drive.fat_swap_structure elements
           to implement an in memory caching scheme. data_map[255] is used
           to indicate which blocks are dirty and data_array points to
           the buffer to hold the in memory fat */

        ltemp = pdr->secpfat;
        pdata = pdr->fat_swap_structure.data_array;
        fatoffset = pdr->numhide + pdr->fatblock;
        while (ltemp)
        {
            if (ltemp > MAXSECTORS)
                nblocks = MAXSECTORS;
            else
                nblocks = (UINT16) ltemp;

            /* Grab the device driver. */
            PC_DRIVE_IO_ENTER(driveno)

	        /*trace_new( TRACE_ATA | TRACE_LEVEL_4,"pc_dskinit: fatoffset %d nblocks %d line %d.\n",fatoffset,nblocks,__LINE__);*/

            /* The dirty blocks table data_map[255] was zeroed when we 
               zeroed the drive structure. */
            /* Now read the fat. */
            /* READ */
            if ( !pc_bdevsw[driveno].io_proc(driveno, (UINT32)fatoffset, 
                                                pdata, nblocks, YES) )
            {
                /* Release the drive io locks. */
                PC_DRIVE_IO_EXIT(driveno)
                pc_report_error(PCERR_FATREAD);
                NU_Deallocate_Memory((VOID *)pdr->fat_swap_structure.data_array);
                pdr->opencount = 0;
	            trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"pc_dskinit: NUF_IO_ERROR line %d.\n",__LINE__);
                return(NUF_IO_ERROR);
            }
            /* Release the drive io locks. */
            PC_DRIVE_IO_EXIT(driveno)

            ltemp -= nblocks;
            fatoffset += nblocks;
            pdata += (nblocks << 9);
        }
    }
    return(NU_SUCCESS);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_idskclose                                                    
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Given a valid drive number. Flush the file allocation table and 
*       purge any buffers or objects associated with the drive.         
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       driveno                             Drive number                
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       NU_SUCCESS                          Unmount successful.         
*       NUF_NOT_OPENED                      Drive not opened.           
*       NUF_IO_ERROR                        Driver returned error.      
*       NUF_INTERNAL                        Nucleus FILE internal error.
*                                                                       
*************************************************************************/
STATUS pc_idskclose(INT16 driveno)
{
DDRIVE      *pdr;
STATUS      ret_val;

    /* Find the drive. */
    pdr = pc_drno2dr(driveno);

    /* Check drive number */
    if (pdr)
    {
        /* Grab exclusive access to the FAT. */
        PC_FAT_ENTER(driveno)
        /* Flush the file allocation table. */
        ret_val = pc_flushfat(pdr);
        /* Release non-excl use of FAT. */
        PC_FAT_EXIT(driveno)

        if (ret_val == NU_SUCCESS)
        {
             /* Release the drive if opencount == 0 */
             ret_val = pc_dskfree(driveno, NO);
        }
    }
    else
    {
        /* pc_drno2dr Error. */
        ret_val = NUF_NOT_OPENED;
    }
    return(ret_val);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_fd2file                                                      
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Map a file descriptor to a file structure. Return null if the   
*       file is not open. If an error has occurred on the file return    
*       NU_NULL unless allow_err is true.                               
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       fd                                  File descriptor             
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       File representation.                                         
*       NU_NULL                             Invalid FD.                 
*                                                                       
*************************************************************************/
PC_FILE *pc_fd2file(INT fd)
{
PC_FILE     *pfile;
PC_FILE     *ret_val = NU_NULL;

    /* Is this descriptor value in correct range ? */
    if (0 <= fd && fd <= NUSERFILES)
    {
        /* Get PFILE memory pool pointer */
        pfile = mem_file_pool+fd;
        if (pfile && !pfile->is_free)
        {
            ret_val = pfile;
        }
    }

    return(ret_val);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_allocfile                                                    
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Allocate PC_FILE structure.                                     
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       None                                                            
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       File descriptor.                                                
*       -1                                  Over the NUSERFILES.        
*                                                                       
*************************************************************************/
INT pc_allocfile(VOID)
{
PC_FILE     *pfile;
INT         i, retval = -1;


    /* Get PC_FILE memory pool pointer. */
    pfile = mem_file_pool;

    for (i = 0; i < NUSERFILES; i++, pfile++)
    {
        /* Check free flag. */
        if (pfile->is_free)
        {
            /* Initialize file descriptor. */
            pc_memfill(pfile, sizeof(PC_FILE), (UINT8) 0);

            /* Set retval = file descriptor number, break. */
            retval = i;
            break;
        }
    }
    return(retval);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_freefile                                                     
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Free all core associated with a file descriptor and make the    
*       descriptor available for future calls to allocfile.             
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       fd                                  File descriptor.            
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       None.                                                           
*                                                                       
*************************************************************************/
VOID pc_freefile(INT fd)
{
PC_FILE     *pfile;


    /* Get file descriptor. */
    if ((pfile = pc_fd2file(fd)) != NU_NULL)
    {
        /* Release a DROBJ. */
        if (pfile->pobj)
            pc_freeobj(pfile->pobj);
        /* Set free flag. */
        pfile->is_free = YES;
    }
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_free_all_fil                                                 
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Release all file descriptors associated with a drive and free   
*       up all core associated with the files called by dsk_close       
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       pdrive                              Drive management structure  
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       None.                                                           
*                                                                       
*************************************************************************/
VOID pc_free_all_fil(DDRIVE *pdrive)
{
PC_FILE     *pfile;
INT         i;


    for (i = 0; i < NUSERFILES; i++)
    {
        /* Get file descriptor. */
        pfile = pc_fd2file(i);
        if (pfile != NU_NULL)
        {
            if ( (pfile->pobj) && (pfile->pobj->pdrive == pdrive) )
            {
                /* print a debug message since in normal operation 
                   all files should be close closed before closing the drive */
                pc_report_error(PCERR_FSTOPEN);
                /* Release a file structure. */
                pc_freefile(i);
            }
        }
    }
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_log_base_2                                                   
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Calculate log2(N).                                              
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       n                                   Sector per cluster          
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       log2(N)                                                         
*                                                                       
*************************************************************************/
INT16 pc_log_base_2(UINT16 n)                                   /*__fn__*/
{
INT16       log;
INT16       ret_val = 0;

    log = 0;
    if ( n > 1 )
    {
        while (n)
        {
            log += 1;
            n >>= 1;
        }
        ret_val = (INT16)(log-1);
    }
    return(ret_val);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_get_cwd                                                      
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Return the current directory inode for the drive represented    
*       by ddrive.                                                      
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       pdr                                 Drive management structure. 
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       DROBJ pointer                                                   
*       Return NU_NULL on error.                                        
*                                                                       
*************************************************************************/
DROBJ *pc_get_cwd(DDRIVE *pdrive)
{
DROBJ       *pcwd;
DROBJ       *pobj;
DROBJ       *ret_val;


    /* Get the current working directory. */
    pcwd = fs_user->lcwd[pdrive->driveno];

    /* If no current working dir set it to the root */
    if (!pcwd)
    {
        /* Get root directory. */
        pcwd = pc_get_root(pdrive);
        fs_user->lcwd[pdrive->driveno] = pcwd;
    }

    if (pcwd)
    {
        /* Allocate an empty DROBJ and FINODE. */
        pobj = pc_allocobj();
        if (!pobj)
            return(NU_NULL);
        /* Free the inode that comes with allocobj */
        pc_freei(pobj->finode);
        copybuff(pobj, pcwd, sizeof(DROBJ));
        pobj->finode->opencount += 1;

        ret_val = pobj;
    }
    else    /* If no cwd is set error */
    {
        ret_val = NU_NULL;
    }

    return(ret_val);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_upstat                                                       
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Given a pointer to a DSTAT structure that contains a pointer to 
*       an initialized DROBJ structure, load the public elements of     
*       DSTAT with name, filesize, date of modification, et al.           
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       statobj                             Caller's buffer to put file 
*                                            info.                      
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       None.                                                           
*                                                                       
*************************************************************************/
VOID pc_upstat(DSTAT *statobj)
{
DROBJ       *pobj;
FINODE      *pi;
INT         ii, jj; 


    /* Move FINODE pointer to local. */
    pobj = statobj->pobj;
    pi = pobj->finode;
 
    /* Copy file name and file extension. */
    copybuff(statobj->sfname, pi->fname, 8);
    statobj->sfname[8] = '\0';
    copybuff(statobj->fext, pi->fext, 3);
    statobj->fext[3] = '\0';

    /* Initialize long file name. */
    statobj->lfname[0] = '\0';

    /* Set file attributes. */
    statobj->fattribute = pi->fattribute;
    if (statobj->pobj->linfo.lnament) /* long file name */
    {                   
        pc_cre_longname((UINT8 *)statobj->lfname, &statobj->pobj->linfo);
    } 
    else  /* Copy short file name to long name field */
    {
        /* No copy on a volume label */
        if(!(statobj->fattribute & AVOLUME)) 
        {
            ii = 0;
            while( statobj->sfname[ii] != '\0' && 
                        statobj->sfname[ii] != ' ')
            {

                statobj->lfname[ii] = statobj->sfname[ii];
                ii++;
            }
            
            if( statobj->fext[0] != ' ' && statobj->fext[0] != 0)
            {
                statobj->lfname[ii] = (UINT8)'.';
                ii++;


                for( jj = 0; jj<3; jj++)
                {
                    if( statobj->fext[jj] == ' ')
                    {
                         statobj->lfname[ii] = 0;
                    }
                    else
                    {
                        statobj->lfname[ii] = statobj->fext[jj];
                    }

                    ii++;
                }

                statobj->lfname[ii] = (UINT8)0;
            }
            else
            {
                statobj->lfname[ii] = (UINT8)0;
            }
        }
    }


    statobj->fcrcmsec = pi->fcrcmsec;
    statobj->fcrtime = pi->fcrtime;
    statobj->fcrdate = pi->fcrdate;

    statobj->faccdate = pi->faccdate;

    statobj->fuptime = pi->fuptime;
    statobj->fupdate = pi->fupdate;

    statobj->fsize = pi->fsize;
}


/****************************************************************************
 * Modifications:
 * $Log: 
 *  5    mpeg      1.4         4/3/04 12:30:11 AM     Nagaraja Kolur  CR(s) 
 *        8762 8763 : Merged 2.5.6 Nup File System.
 *        Test: 1. pvr demo 2. nup file test 3. CR 8181
 *        
 *  4    mpeg      1.3         1/7/04 4:10:56 PM      Tim Ross        CR(s) 
 *        8181 : During pc_diskinit, read partition table, find first FAT32 
 *        partition,
 *        and reference that partition from it's sector offset instead of the 
 *        beginning of the drive (sector 0).
 *  3    mpeg      1.2         10/15/03 4:54:17 PM    Tim White       CR(s): 
 *        7660 Remove ATA header files from NUP_FILE code.
 *        
 *  2    mpeg      1.1         8/22/03 6:48:24 PM     Dave Moore      SCR(s) 
 *        7350 :
 *        Fixed compiler warnings
 *        
 *        
 *  1    mpeg      1.0         8/22/03 5:21:56 PM     Dave Moore      
 * $
 ****************************************************************************/

