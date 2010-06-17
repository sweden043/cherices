/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                      Conexant Systems Inc. (c) 2003                      */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       nu_file.c
 *
 *
 * Description:    see ATI header below.
 *                 Initial version based on Nucleus File v2.4.4
 *
 *
 ****************************************************************************/
/* $Header: nu_file.c, 4, 4/3/04 1:20:40 AM, Nagaraja Kolur$
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
*       NU_FILE.C                                 2.5                 
*                                                                       
* COMPONENT                                                             
*                                                                       
*       Nucleus FILE                                                    
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       This file contains Nucleus FILE 32 application interface.        
*                                                                       
* DATA STRUCTURES                                                       
*                                                                       
*       None.                                                           
*                                                                       
* FUNCTIONS                                                             
*                                                                       
*       NU_Open_Disk                        Open a disk for business.   
*       NU_Close_Disk                       Flush all buffers for a disk
*                                            and free all core.         
*       NU_Disk_Abort                       Abort all operations on a   
*                                            disk.                      
*       NU_Open                             Open a file.                
*       NU_Read                             Read bytes from a file.     
*       NU_Write                            Write Bytes to a file.      
*       NU_Seek                             Move the file pointer.      
*       NU_Close                            Close a file and flush the  
*                                            file allocation table.     
*       NU_Set_Attributes                   Set file attributes.      
*       NU_Get_Attributes                   Get a file's attributes.      
*       NU_Flush                            Flush an open file.         
*       NU_Truncate                         Truncate an open file.      
*       NU_Rename                           Rename a file(s).              
*       NU_Delete                           Delete a file(s).              
*       NU_Make_Dir                         Create a directory.         
*       NU_Remove_Dir                       Delete a directory.         
*       pc_fat_size                         Calculate blocks required   
*                                            for a volume's Allocation  
*                                            Table.                     
*       NU_Format                           Create a file system.       
*       NU_FreeSpace                        Calculate and return the    
*                                            free space on a disk.      
*       NU_Get_First                        Get stats on the first file 
*                                            to match a pattern.        
*       NU_Get_Next                         Get stats on the next file  
*                                            to match a pattern.        
*       NU_Done                             Free resources used by      
*                                            NU_Get_First/NU_Get_Next.  
*       NU_Set_Default_Drive                Set the default drive number.
*       NU_Get_Default_Drive                Get the default drive number.
*       NU_Set_Current_Dir                  Set the current working     
*                                            directory.                 
*       NU_Current_Dir                      Get string representation of
*                                            current working dir.       
*                                                                       
* DEPENDENCIES                                                          
*                                                                       
*       pcdisk.h                            File common definitions     
*                                                                       
*************************************************************************/

#include        "pcdisk.h"
#include	"file_mmu.h"
/*cnxt*/
#include <stdio.h>
#include "retcodes.h"
#include "stbcfg.h"
#include "kal.h"
#include "trace.h"
#include "ata.h"

extern _PC_BDEVSW   pc_bdevsw[];            /* Driver dispatch table.   */
extern UNSIGNED     *NUF_Drive_Pointers[];  /* DDRIVE pointer list.     */
extern INT          NUF_Fat_Type[];         /* FAT type list.           */
extern INT          NUF_Drive_Fat_Size[];   /* FAT bufffer size list.   */

static INT pc_l_pwd(UINT8 *, DROBJ *);
static INT pc_gm_name(UINT8 *path, DROBJ *pmom, DROBJ *pdotdot);
extern void create_primary_partition( INT16 driveno, UINT8 *buff ); // DAVEM
UINT32 FILE_Unused_Param; /* Used to prevent compiler warnings */

/************************************************************************
* FUNCTION                                                              
*                                                                       
*       NU_Open_Disk                                                    
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Given a path specifier containing a valid drive specifier open    
*       the disk by reading all of the block zero information and
*       converting it to native byte order.                                        
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       path                                Path name            
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       NU_SUCCESS                          Disk was initialized        
*                                            successfully.              
*       NUF_BAD_USER                        Not a file user.            
*       NUF_BADDRIVE                        Invalid drive specified.    
*       NUF_NO_DISK                         Disk is removed.            
*       NUF_FATCORE                         FAT cache table too small.  
*       NUF_NO_PARTITION                    No partition in disk.       
*       NUF_FORMAT                          Disk not formatted.    
*       NUF_NO_MEMORY                       Can't allocate internal     
*                                            buffer.                    
*       NUF_IO_ERROR                        Driver returned error.      
*       NUF_INTERNAL                        Nucleus FILE internal error.
*       Other error code                    See the driver error code.  
*                                                                       
*************************************************************************/
STATUS NU_Open_Disk(CHAR *path)
{
STATUS      ret_val;
DDRIVE      *pdr;
DROBJ       *pcwd;
INT         open_needed = NU_TRUE;
INT16       driveno;

    
    /* Must be last line in declarations */
    PC_FS_ENTER() 
    /* Check if a valid user if multitasking */
    CHECK_USER(INT, NUF_BAD_USER) 

	/*printf("NU_Open_Disk enter\n");*/

    /* Start by assuming success. */
    ret_val = NU_SUCCESS;

    /* Get drive no */
    if (pc_parsedrive(&driveno, (UINT8 *)path))
    {
        /* Grab exclusive access to the drive. */
        PC_DRIVE_ENTER(driveno, YES)

        /* Check drive exist ( if there is check driver service ) */
        if (pc_bdevsw[driveno].dskchk_proc)
        {
            if ( !pc_bdevsw[driveno].dskchk_proc(driveno) )
                ret_val = NUF_NO_DISK;
        }

        if (ret_val == NU_SUCCESS)
        {
            /* Find the drive. */
            pdr = (DDRIVE *)NUF_Drive_Pointers[driveno];
            if (pdr)
            {
                /* Don't do anything on reopens */
                if (pdr->opencount > 0)
                {
                    pdr->opencount++;
                    open_needed = NU_FALSE;
                }
            }
        }
        else
        {
            open_needed = NU_FALSE;
        }

        if (open_needed == NU_TRUE)
        {
            /* Open the disk */
            if ( !pc_bdevsw[driveno].open_proc(driveno) )
            {
                pc_report_error(PCERR_INITDEV);

                /* FAT type check. */
                if (NUF_Fat_Type[driveno] == NUF_NO_PARTITION)
                    ret_val = NUF_NO_PARTITION;
                else
                    ret_val = NUF_IO_ERROR;
            }
            else
            {
                /* File system initialization */
                ret_val = pc_dskinit(driveno);
            }
            if (ret_val == NU_SUCCESS)
            {
                /* Find the drive */
                pdr = pc_drno2dr(driveno);

                /* Set the current root directory */
                pcwd = pc_get_root(pdr);
                fs_user->lcwd[driveno] = pcwd;
            }
        }

        /* Release non-exclusive use of drive. */
        PC_DRIVE_EXIT(driveno)
    }
    else
    {
        ret_val = NUF_BADDRIVE;
    }

    /* Restore the kernel state */
    PC_FS_EXIT()

    return(ret_val);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       NU_Close_Disk                                                   
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Given a path name containing a valid drive specifier, flush the 
*       file allocation table and purge any buffers or objects          
*       associated with the drive.                                      
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       path                                Path name            
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       NU_SUCCESS                          If service is successful.   
*       NUF_BAD_USER                        Not a file user.            
*       NUF_BADDRIVE                        Invalid drive specified.    
*       NUF_NOT_OPENED                      Drive not opened.           
*       NUF_NO_DISK                         Disk is removed.            
*       NUF_IO_ERROR                        I/O error occurred.           
*       NUF_INTERNAL                        Nucleus FILE internal error.
*                                                                       
*************************************************************************/
INT NU_Close_Disk(CHAR *path)
{
STATUS      ret_val;
INT16       driveno;


    /* Must be last line in declarations */
    PC_FS_ENTER()
    /* Check if a valid user if multitasking */
    CHECK_USER(INT, NUF_BAD_USER)

    /* Start by assuming success. */
    ret_val = NU_SUCCESS;

    /* Get drive no */
    if (pc_parsedrive(&driveno, (UINT8 *)path))
    {
        /* Check drive exist ( if there is check driver service ). */
        if (pc_bdevsw[driveno].dskchk_proc)
        {
            if ( !pc_bdevsw[driveno].dskchk_proc(driveno) )
            {
                ret_val = NUF_NO_DISK;
            }
        }

        if (ret_val == NU_SUCCESS)
        {
            /* Grab exclusive access to the drive */
            PC_DRIVE_ENTER(driveno, YES)

            /* Unmount a disk. */
        ret_val = pc_idskclose(driveno);

            /* Release non-excl use of drive. */
        PC_DRIVE_EXIT(driveno)
    }
    }
    else
    {
        ret_val = NUF_BADDRIVE;
    }

    /* Restore the kernel state */
    PC_FS_EXIT()

    return(ret_val);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       NU_Disk_Abort                                                   
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       If an application senses that there are problems with a disk, it
*       should call NU_Disk_Abort("D:"). This will cause all resources  
*       associated with that drive to be freed, but no disk writes will 
*       be attempted. All file descriptors associated with the drive    
*       become invalid. After correcting the problem call               
*       NU_Open_Disk("D:") to re-mount the disk and re-open your files. 
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       path                                Path name            
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       None.                                                           
*                                                                       
*************************************************************************/
VOID NU_Disk_Abort(CHAR *path)
{
INT16       driveno;


    /* Must be last line in declarations. */
    PC_FS_ENTER()
    /* Check if a valid user if multitasking. */
    VOID_CHECK_USER()

    /* Get the drive number. */
    if (pc_parsedrive(&driveno, (UINT8 *)path))
    {
        /* Grab exclusive access to the drive. */
        PC_DRIVE_ENTER(driveno, YES)

        /* Release the drive unconditionally. */
        pc_dskfree(driveno, YES);

        /* Release non-exclusive use of drive. */
        PC_DRIVE_EXIT(driveno)
    }

    /* Restore the kernel state */
    PC_FS_EXIT()

    return;
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       NU_Make_Dir                                                     
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Create a sudirectory in the path specified by name. Fails if a  
*       file or directory of the same name already exists or if the path
*       is not found.                                                   
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       name                                Path name                   
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       NU_SUCCESS                          Directory was          
*                                            made successfully.          
*       NUF_BAD_USER                        Not a file user.            
*       NUF_BADDRIVE                        Invalid drive specified.    
*       NUF_NOT_OPENED                      The disk is not opened yet. 
*       NUF_NO_DISK                         Disk is removed.            
*       NUF_NOSPC                           No space to create directory
*       NUF_LONGPATH                        Path or directory name too  
*                                            long.                      
*       NUF_INVNAME                         Path or filename includes   
*                                            invalid character.         
*       NUF_ROOT_FULL                       Root directry full         
*                                            in this disk.              
*       NUF_NOFILE                          The specified file not      
*                                            found.                     
*       NUF_EXIST                           The directory already exists.
*       NUF_NO_BLOCK                        No block buffer available.  
*       NUF_NO_FINODE                       No FINODE buffer available. 
*       NUF_NO_DROBJ                        No DROBJ buffer available.  
*       NUF_IO_ERROR                        I/O error occurred.           
*       NUF_INTERNAL                        Nucleus FILE internal error.
*                                                                       
*       fs_user->p_errno is set to one of these values                  
*                                                                       
*       PENOENT                             File not found or path to   
*                                            file not found.            
*       PEEXIST                             Exclusive access requested  
*                                            but file already exists.   
*       PENOSPC                             Create failed.              
*                                                                       
*************************************************************************/
STATUS NU_Make_Dir(CHAR *name)
{
STATUS      ret_stat;
DROBJ       *pobj;
DROBJ       *parent_obj;
VOID        *path;
VOID        *filename;
VOID        *fileext;
INT16       driveno;
UINT16      len;


    /* Must be last line in declarations */
    PC_FS_ENTER()
    /* Check if a valid user if multitasking */
    CHECK_USER(INT, NUF_BAD_USER)

    /* Start by assuming success. */
    ret_stat = NU_SUCCESS;
    fs_user->p_errno = 0;

    parent_obj = NU_NULL;
    pobj = NU_NULL;

    /* Get the drive number. */
    if (pc_parsedrive(&driveno, (UINT8 *)name))
    {
        /* Check drive exist ( if there is check driver service ). */
        if (pc_bdevsw[driveno].dskchk_proc)
        {
            if ( !pc_bdevsw[driveno].dskchk_proc(driveno) )
            {
                ret_stat = NUF_NO_DISK;
            }
        }

        if (ret_stat == NU_SUCCESS)
    {
            /* Get out the filename and d:parent */
            ret_stat = 
                pc_parsepath(&path, &filename, &fileext, (UINT8 *)name);
        }
    }
    else
    {
        ret_stat = NUF_BADDRIVE;
    }


    if (ret_stat != NU_SUCCESS)
    {
        fs_user->p_errno = PENOENT;
    }
    else
    {
        /* Register drive in use */
        PC_DRIVE_ENTER(driveno, NO)

        /* Find the parent and make sure it is a directory */
        ret_stat = pc_fndnode(driveno, &parent_obj, (UINT8 *)path);
        if ( (ret_stat == NU_SUCCESS) && (pc_isadir(parent_obj)) && (!pc_isavol(parent_obj)) )
        {
            /* Lock the parent */
            PC_INODE_ENTER(parent_obj->finode, YES)

            /* Check if the new directory will make the path too long */
            for (len = 0; *((UINT8 *)filename + len); len++);
            if ( (parent_obj->finode->abs_length + len) >= (EMAXPATH - 12) )
            {
                ret_stat = NUF_LONGPATH;
            }
            else
            {
                /* Fail if the directory exists */
                pobj = NU_NULL;
                ret_stat = pc_get_inode(&pobj, parent_obj, (UINT8 *)filename);
            }
            if (ret_stat == NU_SUCCESS)
            {
                fs_user->p_errno = PEEXIST;        /* Exclusive fail */
                /* Free the current object. */
                pc_freeobj(pobj);
                ret_stat = NUF_EXIST;
            }
            else if (ret_stat == NUF_NOFILE)  /* File not found */
            {
                /* Create Directory */
                ret_stat = pc_mknode(&pobj, parent_obj, (UINT8 *)filename, (UINT8 *)fileext, ADIRENT);
                if (ret_stat != NU_SUCCESS)
                {
                    fs_user->p_errno = PENOSPC;
                }
                else
                {
                    pc_freeobj(pobj);
                }
            }

            /* Release excl use of finode. */
            PC_INODE_EXIT(parent_obj->finode)
            /* Free the parent object. */
            pc_freeobj(parent_obj);
        }
        else
        {
            fs_user->p_errno = PENOENT;
        }

        /* Release non-excl use of drive. */
        PC_DRIVE_EXIT(driveno)
    }

    /* Restore the kernel state */
    PC_FS_EXIT()

    return(ret_stat);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       _synch_file_ptrs                                                
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Synchronize file pointers. Read write Seek and close all call   
*       here.                                                           
*       This fixes the following BUGS:                                  
*       1. If a file is created and left open and then opened again with
*          a new file handle before any writing takes place, neither    
*          file will get its fptr_cluster set correctly initially. The  
*          first one to write would get set up correctly but the other  
*          wouldn't. Thus if fptr_cluster is zero we see if we can set it.
*       2. If one file seeked to the end of the file or has written to  
*          the end of the file its file pointer will point beyond the   
*          last cluster in the chain, the next call to write will notice
*          the fptr is beyond the file size and extend the file by      
*          allocating a new cluster to the chain. During this time the  
*          cluster/block and byte offsets are out of synch. If another  
*          instance extends the file during this time the next call to  
*          write will miss this condition since fptr is not >= fsize    
*          any more. To fix this we note in the file when this condition
*          is true AND, afterwards each time we work with the file we   
*          see if the file has grown and adjust the cluster pointer and 
*          block pointer if needed.                                     
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *pfile                              Internal file representation.
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       None.                                                           
*                                                                       
*************************************************************************/
VOID _synch_file_ptrs(PC_FILE *pfile)
{
UINT32      clno = 0;


    if (!pfile->fptr_cluster)
    {
        /* Current cluster - note on a new file this will be zero */
        pfile->fptr_cluster = pfile->pobj->finode->fcluster; 
#if 0
//DAVEM
            if( (UINT32)pfile->fptr_cluster > 457672 )
			{
	          trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"_synch_file_ptrs: out of range=%d line %d.\n",pfile->fptr_cluster,__LINE__);
			}
//DAVEM
#endif
        if (pfile->fptr_cluster)
            pfile->fptr_block = pc_cl2sector(pfile->pobj->pdrive, pfile->fptr_cluster);
        else
            pfile->fptr_block = 0L;
    }
    if (pfile->at_eof)
    {
        if (pfile->fptr_cluster)
        {
            /* Get the next cluster in a cluster chain. */
            pc_clnext(&clno, pfile->pobj->pdrive, pfile->fptr_cluster);
            /* Not file end cluster. */
            if (clno)
            {
#if 0
//DAVEM
            if( (UINT32)pfile->fptr_cluster > 457672 )
			{
	          trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"_synch_file_ptrs: out of range=%d line %d.\n",pfile->fptr_cluster,__LINE__);
			}
//DAVEM
#endif
                /* Set file cluster and sector number. */
                pfile->fptr_cluster = clno;
                pfile->fptr_block = pc_cl2sector(pfile->pobj->pdrive, pfile->fptr_cluster);
                pfile->at_eof = NO;
            }
        }
    }
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       NU_Open                                                         
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Open the file for access as specified in flag. If creating, use  
*       mode to set the access permissions on the file.                 
*                                                                       
*       Flag values are                                                 
*                                                                       
*       PO_RDONLY                           Open for read only.         
*       PO_WRONLY                           Open for write only.        
*       PO_RDWR                             Read/write access allowed.  
*                                                                       
*       PO_APPEND                           Seek to eof on each write.  
*       PO_CREAT                            Create the file if it does  
*                                            not exist.                 
*       PO_TRUNC                            Truncate the file if it     
*                                            already exists.            
*       PO_EXCL                             If flag contains            
*                                            (PO_CREAT | PO_EXCL) and   
*                                            the file already exists    
*                                            fail and set               
*                                            fs_user->p_errno to EEXIST.
*                                                                       
*       PO_BINARY                           Ignored. All file access is 
*                                            binary.                    
*       PO_TEXT                             Ignored.                    
*                                                                       
*       PO_NOSHAREANY                       Fail if the file is already 
*                                            open.                      
*       PO_NOSHAREWRITE                     Fail if the file is already 
*                                            open for write.            
*                                                                       
*       Mode values are                                                 
*                                                                       
*       PS_IWRITE                           Write permitted.            
*       PS_IREAD                            Read permitted.             
*                                           (Always true anyway)        
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *name                               Open file name              
*       flag                                Open flag                   
*       mode                                Open mode                   
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       Returns a non-negative integer to be used as a file descriptor. 
*       Returning a negative integer indicates an error as follows:       
*       NUF_BAD_USER                        Not a file user.            
*       NUF_BADDRIVE                        Invalid drive specified.    
*       NUF_NOT_OPENED                      The disk is not opened yet. 
*       NUF_NO_DISK                         Disk is removed.            
*       NUF_NOSPC                           No space to create directory
*       NUF_LONGPATH                        Path or directory name too  
*                                            long.                      
*       NUF_INVNAME                         Path or filename includes   
*                                            invalid character.         
*       NUF_INVPARM                         Invalid Flag/Mode is specified.
*       NUF_INVPARCMB                       Invalid Flag/Mode combination
*       NUF_PEMFILE                         No file descriptors.        
*                                            available.                 
*                                           (too many files open)       
*       NUF_ACCES                           You can't open the file     
*                                            which has Directory or     
*                                            VOLLABEL attributes.       
*       NUF_NOSPC                           No space to create directory
*                                            in this disk.              
*       NUF_SHARE                           The access conflict from    
*                                            multiple task to a specific
*                                            file.                      
*       NUF_NOFILE                          The specified file not      
*                                            found.                     
*       NUF_EXIST                           The directory already exists.
*       NUF_NO_BLOCK                        No block buffer available.  
*       NUF_NO_FINODE                       No FINODE buffer available. 
*       NUF_NO_DROBJ                        No DROBJ buffer available.  
*       NUF_IO_ERROR                        I/O error occurred.           
*       NUF_INTERNAL                        Nucleus FILE internal error.
*                                                                       
*       fs_user->p_errno is set to one of these values                  
*                                                                       
*       PENOENT                             File not found or path to   
*                                            file not found.            
*       PEMFILE                             No file descriptors         
*                                            available (too many files  
*                                            open).                     
*       PEEXIST                             Exclusive access requested  
*                                            but file already exists.   
*       PEACCES                             Attempt to open a read only 
*                                            file or a special          
*                                            (directory) file.          
*       PENOSPC                             Create failed.              
*       PESHARE                             Already open in exclusive   
*                                            mode or we want exclusive  
*                                            and it's already open.      
*                                                                       
*************************************************************************/
INT NU_Open(CHAR *name, UINT16 flag, UINT16 mode)
{
INT         fd;
STATUS      ret_stat;
PC_FILE     *pfile;
UINT32      cluster;
DROBJ       *parent_obj;
DROBJ       *pobj;
VOID        *path;
VOID        *filename;
VOID        *fileext;
INT         open_for_write;
INT         sharing_error;
INT16       driveno;


    /* Must be last line in declarations */
    PC_FS_ENTER()
    /* Check if a valid user if multitasking */
    CHECK_USER(INT, NUF_BAD_USER)

    /* Start by assuming success. NU_SUCCESS(0). */
    fd = 0;
    fs_user->p_errno = 0;

    parent_obj = NU_NULL;
    pobj = NU_NULL;

    /* We'll need to know this in a few places. */
    if ( flag & (PO_WRONLY | PO_RDWR ) )
        open_for_write = YES;
    else
        open_for_write = NO;

    /* Get the drive number. */
    if (pc_parsedrive(&driveno, (UINT8 *)name))
    {
        /* Check drive exist ( if there is check driver service ). */
        if (pc_bdevsw[driveno].dskchk_proc)
        {
            if ( !pc_bdevsw[driveno].dskchk_proc(driveno) )
            {
                fd = NUF_NO_DISK;
            }
        }
    }
    else
    {
        fd = NUF_BADDRIVE;
    }

    if (fd  < 0)
    {
        /* Bad drive or No disk. */
        fs_user->p_errno = PENOENT;
    }
    else if( flag & (~(PO_WRONLY|PO_RDWR|PO_APPEND|
                    PO_CREAT|PO_TRUNC|PO_EXCL|PO_TEXT|
                PO_BINARY|PO_NOSHAREANY|PO_NOSHAREWRITE)))
    {
        /* Invalid parameter is specified. */
        fd = NUF_INVPARM;
    }
    else if ((flag & PO_TRUNC) && (! open_for_write))
    {
        /* Invalid parameter combination is specified. */
        fd = NUF_INVPARCMB;
    }
    else if ((flag & PO_APPEND) && (!open_for_write))
    {
        /* Invalid parameter combination is specified. */
        fd = NUF_INVPARCMB;
    }
    else if ((flag & PO_EXCL) && (!(flag & PO_CREAT)))
    {
        /* Invalid parameter combination is specified. */
        fd = NUF_INVPARCMB;
    }
    else if ((flag & PO_WRONLY) && (flag & PO_RDWR))
    {
        /* Invalid parameter combination is specified. */
        fd = NUF_INVPARCMB;
    }
    else if ((flag & PO_TRUNC) && (flag & PO_APPEND))
    {
        /* Invalid parameter combination is specified. */
        fd = NUF_INVPARCMB;
    }
    else if((mode & (~(PS_IWRITE|PS_IREAD)))||(!(mode)))
    {
        /* Invalid parameter is specified. */
        fd = NUF_INVPARM;
    }
    else if ((fd = pc_allocfile()) < 0)     /* Grab a file */
    {
        /* File descriptor is not available */
        fs_user->p_errno = PEMFILE;
        fd = NUF_PEMFILE;
    }
    else
    {
        /* Get the FILE. This will never fail */

	    /*printf("NU_Open PARMs OK line %d\n",__LINE__);*/

        pfile = pc_fd2file(fd);
        /* Paranoia. Set the obj to null until we have one */
        pfile->pobj = NU_NULL;
        /* Clear the File update flag */
        pfile->fupdate = NO;

        /* Register drive in use */
        PC_DRIVE_ENTER(driveno, NO)

        /* Get out the filename and d:parent */
        ret_stat = pc_parsepath(&path, &filename, &fileext, (UINT8 *)name);
        if (ret_stat != NU_SUCCESS)
        {
            fs_user->p_errno = PENOENT;
            /* Release a file structure. */
            pc_freefile(fd);
		    trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Open PENOENT line %d.\n",__LINE__);
            fd = (INT)ret_stat;
        }
        else
        {
            /* Find the parent */
            ret_stat = pc_fndnode(driveno, &parent_obj, (UINT8 *)path);
            if (ret_stat != NU_SUCCESS)
            {
                fs_user->p_errno = PENOENT;
                /* Release a file structure. */
                pc_freefile(fd);
                fd = (INT)ret_stat;
		        trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Open PENOENT line %d.\n",__LINE__);
            }
            else if ( (!pc_isadir(parent_obj)) || (pc_isavol(parent_obj)) )
            {
                /* Release a file structure. */
                pc_freefile(fd);
                /* Free the parent object. */
                pc_freeobj(parent_obj);

                fs_user->p_errno = PENOENT;
                fd = NUF_ACCES;
		
                trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Open PENOENT line %d.\n",__LINE__);
            }
            else
            {
                /* Lock the parent finode. */
                PC_INODE_ENTER(parent_obj->finode, YES)

                /* Find the file and init the structure. */
                pobj = NU_NULL;
                ret_stat =  pc_get_inode(&pobj, parent_obj, (UINT8 *)filename);
                if (ret_stat == NU_SUCCESS)
                {
                    /* If we goto exit: we want them linked so we can clean up */
                    pfile->pobj = pobj;            /* Link the file to the object */

                    /* The file is already open by someone. Lets see if we are
                           compatible */
                    sharing_error = NO;

                    /* Check the sharing conditions */
                    if (pobj->finode->opencount != 1)
                    {
                        /* 1. We don't want to share with anyone */
                        if (flag & PO_NOSHAREANY)
                            sharing_error = YES;
                        /* 2. Someone else doesn't want to share */
                        if (pobj->finode->openflags & OF_EXCLUSIVE)
                            sharing_error = YES;
                        /* 3. We want exclusive write but already open for write */
                        if ( open_for_write && (flag & PO_NOSHAREWRITE) &&
                             (pobj->finode->openflags & OF_WRITE) )
                            sharing_error = YES;
                        /* 4. We want open for write but it's already open for
                              exclusive */
                        if ( (open_for_write) && 
                             (pobj->finode->openflags & OF_WRITEEXCLUSIVE) )
                            sharing_error = YES;
                        /* 5. Open for trunc when already open */
                        if (flag & PO_TRUNC)
                            sharing_error = YES;
                    }
                    if (sharing_error)
                    {
                        /* Release a file structure. */
                        pc_freefile(fd);
                        fs_user->p_errno = PESHARE;
                        fd = NUF_SHARE;
                    }
                    else if ( pc_isadir(pobj) || pc_isavol(pobj) )
                    {
                        /* Release a file structure. */
                        pc_freefile(fd);
                        fs_user->p_errno = PEACCES;        /* is a directory */
                        fd = NUF_ACCES;
                    }
                    else if ( (flag & (PO_EXCL | PO_CREAT)) == (PO_EXCL | PO_CREAT) )
                    {
                        /* Release a file structure. */
                        pc_freefile(fd);
                        fs_user->p_errno = PEEXIST;        /* Exclusive fail */
                        fd = NUF_EXIST;
                    }
                    else if (flag & PO_TRUNC)
                    {
                        cluster = pobj->finode->fcluster;
                        pobj->finode->fcluster = 0L;
                        pobj->finode->fsize = 0L;

                        /* Update an inode to disk. */
                        ret_stat = pc_update_inode(pobj, DSET_UPDATE);
                        if (ret_stat == NU_SUCCESS)
                        {
                            /* And clear up the space */
                            /* Grab exclusive access to the FAT. */
                            PC_FAT_ENTER(pobj->pdrive->driveno)

                            /* Release the chain. */
                            ret_stat = pc_freechain(pobj->pdrive,cluster);
                            if (ret_stat == NU_SUCCESS)
                            {
                                /* Flush the file allocation table. */
                                ret_stat = pc_flushfat(pobj->pdrive);
                            }

                            /* Release non-excl use of FAT. */
                            PC_FAT_EXIT(pobj->pdrive->driveno)
                        }
                    }
                    if (ret_stat != NU_SUCCESS)
                    {
                        fs_user->p_errno = PEACCES;
                        /* Release a file structure. */
                        pc_freefile(fd);
                        fd = (INT)ret_stat;
                    }
                }
                else if (ret_stat == NUF_NOFILE)   /* File not found */
                {
                    if (!(flag & PO_CREAT))
                    {
                        /* Release a file structure. */
                        pc_freefile(fd);
                        fs_user->p_errno = PENOENT;        /* File does not exist */
                        fd = NUF_NOFILE;
                    }
                    /* Do not allow create if write bits not set */
                    else if (!open_for_write)
                    {
                        /* Release a file structure. */
                        pc_freefile(fd);
                        fs_user->p_errno = PEACCES;        /* read only file */
                        fd = NUF_ACCES;
                    }
                    else
                    {
                        /* Create for read only if write perm not allowed */
                        ret_stat = pc_mknode(&pobj, parent_obj, (UINT8 *)filename, (UINT8 *)fileext, 
                                            (UINT8) ((mode == PS_IREAD) ? (ARDONLY | ARCHIVE) : ARCHIVE));
                        if (ret_stat == NU_SUCCESS)
                        {
                            pfile->pobj = pobj;            /* Link the file to the object */
                        }
                        else
                        {
                            /* Release a file structure. */
                            pc_freefile(fd);
                            fs_user->p_errno = PENOSPC;
                            fd = (INT)ret_stat;
		                    trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Open PENOSPC line %d.\n",__LINE__);
                        }
                    }
                }
                else  /* I/O Error */
                    fd = (INT)ret_stat;

                if (fd >= 0) /* No error */
                {
                    /* Set the file sharing flags in the shared finode structure */
                    /* clear flags if we just opened it . */
                    if (pobj->finode->opencount == 1)
                        pobj->finode->openflags = 0;
                    if (open_for_write)
                    {
                        pobj->finode->openflags |= OF_WRITE;
                        if (flag & PO_NOSHAREWRITE)
                            pobj->finode->openflags |= OF_WRITEEXCLUSIVE;
                    }
                    if (flag & PO_NOSHAREANY)
                        pobj->finode->openflags |= OF_EXCLUSIVE;

                    pfile->flag = flag;            /* Access flags */
                    pfile->fptr = 0L;             /* File pointer */

                    /* Set the cluster and block file pointers */
                    _synch_file_ptrs(pfile);

                    fs_user->p_errno = 0;

                    if (flag & PO_APPEND)
                    {
                        /* Call the internal seek routine to set file pointer to file end  */
                         ret_stat = _po_lseek(pfile, 0, PSEEK_END);

                        if(ret_stat != NU_SUCCESS)
                        {
                            fd = (INT)ret_stat;
                        }
                    }
                }

                /* Release excl use of finode. */
                PC_INODE_EXIT(parent_obj->finode)
                /* Free the parent object. */
                pc_freeobj(parent_obj);
            }
        }

        /* Release non-excl use of drive. */
        PC_DRIVE_EXIT(driveno)
    }

    /* Restore the kernel state */
    PC_FS_EXIT()

    return(fd);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       NU_Read                                                         
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Attempt to read count bytes from the current file pointer of    
*       file at fd and put them in buf. The file pointer is updated.    
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       fd                                  File descriptor             
*       buf                                 Buffer to read data.        
*       count                               Number of read bytes.       
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       Returns the number of bytes read or negative value on error.    
*                                                                       
*       INT32                               A non-negative integer to be
*                                            used as a number of bytes  
*                                            read.                      
*       NUF_BAD_USER                        Not a file user.            
*       NUF_BADPARM                         Invalid parameter specified.
*       NUF_NO_DISK                         Disk is removed.            
*       NUF_BADFILE                         Invalid file descriptor.    
*       NUF_ACCES                           Open flag is  PO_WRONLY.    
*       NUF_IO_ERROR                        I/O error occurred.           
*       NUF_INTERNAL                        Nucleus FILE internal error.
*                                                                       
*       fs_user->p_errno is set to one of these values                  
*                                                                       
*       PEBADF                              Invalid file descriptor.    
*       PENOSPC                             Create failed.              
*                                                                       
*************************************************************************/
INT32 NU_Read(INT fd, CHAR *buf, INT32 count)
{
INT32       ret_val;
PC_FILE     *pfile;
UINT32      block_in_cluster;
UINT16      byte_offset_in_block;
UINT32      n_bytes;
UINT32      next_cluster;
UINT32      n_clusters;
UINT32      block_to_read;
DDRIVE      *pdrive;
UINT32      saved_ptr;
UINT32      saved_ptr_block;
UINT32      saved_ptr_cluster;
INT32       n_left;
UINT32      n_to_read;
UINT32      utemp;
UINT16      nblocks;
static UINT8 *local_buf = (UINT8 *)0;


    /* Must be last line in declarations */
    PC_FS_ENTER()
    /* Check if a valid user if multitasking */
    CHECK_USER(INT32, NUF_BAD_USER)

    if( !local_buf )
	{
       local_buf = (UINT8 *)mem_nc_malloc(512);
	   if ( !local_buf )
	   {
	     trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Read NU_NO_MEMORY line %d.\n",__LINE__);
	     return( NU_NO_MEMORY );
	   }
	}

    /* Start by assuming success. NU_SUCCESS(0). */
    ret_val = 0;
    fs_user->p_errno = 0;

    /* Get the FILE. Second argument is ignored */
    if ((pfile = pc_fd2file(fd)) == NU_NULL)
    {
        fs_user->p_errno = PEBADF;
        ret_val = NUF_BADFILE;
    }
    /* Opened for read ? */
    else if (pfile->flag & PO_WRONLY)
    {
        fs_user->p_errno = PEACCES;
        ret_val = NUF_ACCES;
    }
    /* return -1 if bad arguments */
    else if ( !buf || count < 0)
    {
        ret_val = NUF_BADPARM;
	    trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Read NUF_BADPARM line %d.\n",__LINE__);
    }
    else
    {
        /* Move ddrive pointer to local. */
        pdrive = pfile->pobj->pdrive;

        /* Check drive exist ( if there is check driver service ). */
        if (pc_bdevsw[pdrive->driveno].dskchk_proc)
        {
            if ( !pc_bdevsw[pdrive->driveno].dskchk_proc(pdrive->driveno) )
            {
                ret_val = NUF_NO_DISK;
            }
        }
    }

    if ( (!ret_val) &&
            /* Dont read if done. */
            ((pfile->fptr < pfile->pobj->finode->fsize) && (count)) )
    {
        /* Register Drive in use */
        PC_DRIVE_ENTER(pdrive->driveno, NO)  
        /* Grab exclusive access to the drobj */
        PC_INODE_ENTER(pfile->pobj->finode, YES)

        /* Set the cluster and block file pointers if not already set */
        _synch_file_ptrs(pfile);

        saved_ptr           = pfile->fptr;
        saved_ptr_block     = pfile->fptr_block;
        saved_ptr_cluster   = pfile->fptr_cluster;


        /* Truncate the read count if we need to */
        if ( (pfile->fptr + count) >= pfile->pobj->finode->fsize )
            count = (INT32) (pfile->pobj->finode->fsize - pfile->fptr);

        ret_val = n_left = count;

        while (n_left) /* Loop until read specified count bytes */
        {
            block_in_cluster = (UINT32) (pfile->fptr & pdrive->byte_into_cl_mask);
            block_in_cluster >>= 9;
            block_to_read = pfile->fptr_block + block_in_cluster;

            /* how many clusters are left */
            n_to_read = (UINT32) ( (n_left + 511) >> 9 );
            n_clusters =(UINT32) ((n_to_read+pdrive->secpalloc-1) >> pdrive->log2_secpalloc);

            /* Grab exclusive access to the FAT. */
            PC_FAT_ENTER(pdrive->driveno)
            /* how many contiguous clusters can we get ? <= n_clusters */
            ret_val = pc_get_chain(pdrive, pfile->fptr_cluster, &next_cluster, n_clusters);
            /* Release non-excl use of FAT. */
            PC_FAT_EXIT(pdrive->driveno)

            if (ret_val < 0)
            {
                fs_user->p_errno = PENOSPC;
                break;
            }

            /* Set contiguous clusters. */
            n_clusters = ret_val;

            /* Are we inside a block */
            if ( (pfile->fptr & 0x1ffL) || (n_left < 512) )
            {
                /* Get byte offset in present sector. */
                byte_offset_in_block = (UINT16) (pfile->fptr & 0x1ffL);

                /* Grab the device driver. */
                PC_DRIVE_IO_ENTER(pdrive->driveno)
                /* READ */
                if ( !pc_bdevsw[pdrive->driveno].io_proc(pdrive->driveno, block_to_read, local_buf, (UINT16) 1, YES) )
                {
                    /* Release the drive io locks. */
                    PC_DRIVE_IO_EXIT(pdrive->driveno)
                    fs_user->p_errno = PENOSPC;
                    ret_val = NUF_IO_ERROR;
                    break;
                }
                /* Release the drive io locks. */
                PC_DRIVE_IO_EXIT(pdrive->driveno)

                /* Copy source data to the local buffer */
                n_bytes = (UINT32) (512 - byte_offset_in_block);
                if (n_bytes > (UINT32)n_left)
                    n_bytes = (UINT32)n_left;

                copybuff((UINT8 *)buf, &local_buf[byte_offset_in_block], (INT)n_bytes);

                buf += n_bytes;
                n_left -= n_bytes;
                pfile->fptr += n_bytes;
                
                /* Are we on a cluster boundary  ? */
                if ( !(pfile->fptr & pdrive->byte_into_cl_mask) )
                {
                    if (--n_clusters)             /* If contiguous */
                    {
                        pfile->fptr_block += pdrive->secpalloc;
                        pfile->fptr_cluster += 1;
                    }
                    else
                    {
#if 0
//DAVEM
            if( (UINT32)next_cluster > 457672 )
			{
	          trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Read: out of range=%d line %d.\n",next_cluster,__LINE__);
			}
//DAVEM
#endif
                        pfile->fptr_cluster = next_cluster;
                        pfile->fptr_block = pc_cl2sector(pdrive, next_cluster);
                    }   /* if (--nclusters) {} else {}; */
                }       /* if (!(pfile->fptr & pdrive->byte_into_cl_mask)) */
            }           /* if ( (pfile->fptr & 0x1ff) || (n_left < 512) ) */
            else
            {
                /* Read as many blocks as possible */
                /* How many blocks in the current chain */
                n_to_read = (UINT32) (n_clusters << pdrive->log2_secpalloc);
                /* subtract out any leading blocks */
                n_to_read -= block_in_cluster;
                /* How many blocks yet to read */
                utemp = (UINT32) (n_left >> 9);
                /* take the smallest of the two */
                if (n_to_read > utemp)
                    n_to_read = utemp;

                if (n_to_read)
                {
                /* If we get here we need to read contiguous blocks */
                    block_to_read = pfile->fptr_block + block_in_cluster;

                    utemp = n_to_read;
                    while(utemp)
                    {
                        if (utemp > MAXSECTORS)
                            nblocks = MAXSECTORS;
                        else
                            nblocks = (UINT16) utemp;

                        /* Grab the device driver. */
                        PC_DRIVE_IO_ENTER(pdrive->driveno)
                        /* READ */
                        if ( !pc_bdevsw[pdrive->driveno].io_proc(pdrive->driveno, block_to_read, (UINT8 *)buf, nblocks, YES))
                        {
                            /* Release the drive io locks. */
                            PC_DRIVE_IO_EXIT(pdrive->driveno)
                            fs_user->p_errno = PENOSPC;
                            ret_val = NUF_IO_ERROR;
                            break;
                        }
                        /* Release the drive io locks. */
                        PC_DRIVE_IO_EXIT(pdrive->driveno)

                        utemp -= nblocks;
                        block_to_read += nblocks;
                        buf += (nblocks << 9);
                    }

                    n_bytes = (UINT32) (n_to_read << 9);
                    n_left -= n_bytes;
                    pfile->fptr += n_bytes;

                    /* if we advanced to a cluster boundary advance the 
                       cluster pointer */
                    /* utemp ==s how many clusters we read */
                    utemp =(UINT32) ((n_to_read+block_in_cluster) >> pdrive->log2_secpalloc);

                    if (utemp == n_clusters)
                    {
                        pfile->fptr_cluster = next_cluster;
                    }
                    else
                    {
                        /* advance the pointer as many as we read */
                        pfile->fptr_cluster += utemp;
                    }

#if 0
/* DAVEM */
            if( (UINT32)pfile->fptr_cluster > 457672 )
			{
	          trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Read: out of range=%d line %d.\n",pfile->fptr_cluster,__LINE__);
			}
/* DAVEM */
#endif
            /* Set file pointer. */
                    pfile->fptr_block = pc_cl2sector(pdrive, pfile->fptr_cluster);
                }
            }
        }       /* while n_left */
        if (ret_val < 0)
        {
            /* Restore pointers and return */
            pfile->fptr = saved_ptr;
            pfile->fptr_block = saved_ptr_block;
            pfile->fptr_cluster = saved_ptr_cluster;
        }
        else
        {
            ret_val = count;
        }

        /* Release excl use of finode. */
        PC_INODE_EXIT(pfile->pobj->finode)
        /* Release non-excl use of drive. */
        PC_DRIVE_EXIT(pdrive->driveno)
    }

    /* Restore the kernel state */
    PC_FS_EXIT()

    return(ret_val);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       NU_Write                                                        
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Attempt to write count bytes from buf to the current file       
*       pointer of file at fd. The file pointer is updated.             
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       fd                                  File descriptor.            
*       *buf                                Pointer of buffer to write. 
*       count                               Write byte count.           
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       Returns the number of bytes written or negative value on error. 
*                                                                       
*       INT32                               A non-negative integer to be
*                                            used as a number of bytes  
*                                            written.                   
*                                                                       
*       If the return value is negative, the meaning is follows:        
*                                                                       
*       NUF_BAD_USER                        Not a file user.            
*       NUF_BADPARM                         Invalid parameter specified.
*       NUF_NO_DISK                         Disk is removed.            
*       NUF_BADFILE                         File descriptor invalid.    
*       NUF_ACCES                           Not a PO_WRONLY or PO_RDWR  
*                                            open flag or file          
*                                            attributes is ARDONLY.     
*       NUF_NOSPC                           Write failed. Presumably    
*                                            because of no space.       
*       NUF_IO_ERROR                        I/O error occurred.           
*       NUF_INTERNAL                        Nucleus FILE internal error.
*                                                                       
*       fs_user->p_errno is set to one of these values                  
*                                                                       
*       PEBADF                              Invalid file descriptor.    
*       PEACCES                             Attempt to open a read only 
*                                            file or a special          
*                                            (directory) file.          
*       PENOSPC                             Create failed.              
*                                                                       
*************************************************************************/
INT32 NU_Write(INT fd, CHAR *buf, INT32 count)
{
INT32       ret_val;
STATUS      status;
PC_FILE     *pfile;
DDRIVE      *pdrive;
UINT32      block_in_cluster;
UINT16      byte_offset_in_block;
UINT32      next_cluster;
UINT32      saved_ptr;
UINT32      saved_ptr_block;
UINT32      saved_ptr_cluster;
UINT32      ltemp;
UINT32      n_bytes;
UINT32      n_to_write;
UINT32      n_left;
UINT32      n_blocks_left;
//UINT16      nblocks;
UINT32      nblocks;
UINT32      n_clusters;
UINT32      alloced_size;
UINT32      block_to_write;
static UINT8 *local_buf = (UINT8 *)0;


    /* Must be last line in declarations */
    PC_FS_ENTER()
    /* Check if a valid user if multitasking */
    CHECK_USER(INT32, NUF_BAD_USER)

    if( !local_buf )
	{
       local_buf = (UINT8 *)mem_nc_malloc(512);
	   if( !local_buf )
	   {
	    trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Write NU_NO_MEMORY line %d.\n",__LINE__);
	    return( NU_NO_MEMORY );
	   }
    }

    /* Start by assuming success. NU_SUCCESS(0) */
    ret_val = 0;
    fs_user->p_errno = 0;

    /* Get the FILE. Second argument is ignored */
    if ( (pfile = pc_fd2file(fd)) == NU_NULL )
    {
        fs_user->p_errno = PEBADF;
        ret_val = NUF_BADFILE;
    }
    /* Check the file attributes */
    else if (pfile->pobj->finode->fattribute & ARDONLY)
    {
        fs_user->p_errno = PEACCES;
        ret_val = NUF_ACCES;
    }
    /* Opened for write ? */
    else if ( !((pfile->flag & PO_WRONLY) || (pfile->flag & PO_RDWR)) )
    {
        fs_user->p_errno = PEACCES;
        ret_val = NUF_ACCES;
    }
    /* Return -1 bad parameter */
    else if ( count < 0 || !buf )
    {
        ret_val = NUF_BADPARM;
	    trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Write NUF_BADPARM line %d.\n",__LINE__);
    }
    else
    {
        /* Move ddrive pointer to local. */
        pdrive = pfile->pobj->pdrive;

        /* Check drive exist ( if there is check driver service ). */
        if (pc_bdevsw[pdrive->driveno].dskchk_proc)
        {
            if ( !pc_bdevsw[pdrive->driveno].dskchk_proc(pdrive->driveno) )
            {
                ret_val = NUF_NO_DISK;
            }
        }
    }

    /* (!count) : Return 0 (none written) on bad args. */
    if ( (!ret_val) && (count) )
    {
        /* Register drive in use (non-exclusive) */
        PC_DRIVE_ENTER(pdrive->driveno, NO)

        /* Only one process may write at a time */
        PC_INODE_ENTER(pfile->pobj->finode, YES)

        /* if the file is size zero make sure the current cluster pointer
           is invalid */
        if (!pfile->pobj->finode->fsize)
            pfile->fptr_cluster = 0L;

        /* Set the cluster and block file pointers if not already set */
        _synch_file_ptrs(pfile);
        saved_ptr = pfile->fptr;
        saved_ptr_block = pfile->fptr_block;
        saved_ptr_cluster = pfile->fptr_cluster;

        /* calculate initial values */
        n_left = count;

        /* Round the file size up to its cluster size by adding in clustersize-1
           and masking off the low bits */
        alloced_size =  (pfile->pobj->finode->fsize + pdrive->byte_into_cl_mask) &
                         ~(pdrive->byte_into_cl_mask);

/* trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Write: pfile->pobj->finode->fsize=%d, pdrive->byte_into_cl_mask=%d, line %d.\n",pfile->pobj->finode->fsize,pdrive->byte_into_cl_mask,__LINE__); */

        while (n_left)
        {
            /* Get sector offset in present cluster. */
            block_in_cluster = (UINT32) (pfile->fptr & pdrive->byte_into_cl_mask);
            block_in_cluster >>= 9;

/* trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Write: n_left=%d, alloced_size=%d, line %d.\n",n_left,alloced_size,__LINE__); */

            if (pfile->fptr >= alloced_size)
            {
                /* Extending the file */

                /* Calculate write sectors. */
                n_blocks_left = (UINT32) ((n_left + 511) >> 9);
                /* how many clusters are left-
                 *  assume 1 for the current cluster.
                 *   subtract out the blocks in the current
                 *   round up by adding secpalloc-1 and then
                 *   divide by sectors per cluster

                  |  n_clusters = 1 + 
                  |      (n_blocks_left-
                  |          (pdrive->secpalloc-block_in_cluster)
                  |          + pdrive->secpalloc-1) >> pdrive->log2_secpalloc;
                    ==>
                 */
                n_clusters = ( UINT32) (1 + 
                ((n_blocks_left + block_in_cluster - 1) >> pdrive->log2_secpalloc));

 /* trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Write: n_left=%d, n_clusters=%d, line %d.\n",n_left,n_clusters,__LINE__); */
                /* Check file size. */
                if (pdrive->fasize == 8)    /* FAT32 */
                {
                    /* Calculate file total size */
                    ltemp = alloced_size >> pdrive->log2_secpalloc;
                    ltemp >>= 9;  /* Divide by 512 */
                    ltemp += n_clusters;

                    /* Chech the maximum file size(4GB) */
                    if (pdrive->maxfsize_cluster < ltemp)
                    {
                        ret_val = NUF_MAXFILE_SIZE;
                        break;  /* Exit from write loop */
                    }
                }


                /* Grab exclusive access to the FAT. */
                PC_FAT_ENTER(pdrive->driveno)
                /* Call pc_alloc_chain to build a chain up to n_cluster clusters
                   long. Return the first cluster in pfile->fptr_cluster and
                   return the # of clusters in the chain. If pfile->fptr_cluster
                   is non zero, link the current cluster to the new one */
                status = pc_alloc_chain(&n_clusters, pdrive, &(pfile->fptr_cluster), n_clusters);
                /* Release non-excl use of FAT. */
                PC_FAT_EXIT(pdrive->driveno)

/*  trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Write: n_clusters=%d, line %d.\n",n_clusters,__LINE__); */

                if (status != NU_SUCCESS)
                {
                    ret_val = (INT32)status;
                    break;  /* Exit from write loop */
                }
                /* Calculate the last cluster in this chain. */
                next_cluster = (UINT32) (pfile->fptr_cluster + n_clusters - 1);

                /* link the chain to the directory object if just starting */
                if (!pfile->pobj->finode->fcluster)
                    pfile->pobj->finode->fcluster = pfile->fptr_cluster;

#if 0
/* DAVEM 30GB WD300 ONLY */
                if( (UINT32)pfile->fptr_cluster > 457672 )
			    {
	              trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Write: out of range=%d line %d.\n",pfile->fptr_cluster,__LINE__);
			    }
			    else
			    {
	             trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Write: pfile->fptr_cluster=%d line %d.\n",pfile->fptr_cluster,__LINE__);
			    }
#endif

                /* calculate the new block pointer */
                pfile->fptr_block = pc_cl2sector(pdrive, pfile->fptr_cluster);

                /* calculate amount of space used by the file */
                ltemp = n_clusters << pdrive->log2_secpalloc; ltemp <<= 9;
                alloced_size += ltemp;
            }
            else        /* Not extending the file. (writing inside the file) */
            {
                /* Calculate write sectors. */
                n_blocks_left = (UINT32) ((n_left + 511) >> 9);

                /* how many clusters are left-
                 *  assume 1 for the current cluster.
                 *   subtract out the blocks in the current
                 *   round up by adding secpalloc-1 and then
                 *   divide by sectors per cluster

                  |  n_clusters = 1 + 
                  |      (n_blocks_left-
                  |          (pdrive->secpalloc-block_in_cluster)
                  |          + pdrive->secpalloc-1) >> pdrive->log2_secpalloc;
                    ==>
                */
                n_clusters = (UINT32) (1 + 
                   ((n_blocks_left + block_in_cluster - 1) >> pdrive->log2_secpalloc));
                    
                /* Grab exclusive access to the FAT. */
                PC_FAT_ENTER(pdrive->driveno)
                /* how many contiguous clusters can we get ? <= n_clusters */
                ret_val = pc_get_chain(pdrive, pfile->fptr_cluster,
                                              &next_cluster, n_clusters);
/*  trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Write: next_cluster=%d line %d.\n",next_cluster,__LINE__); */
                /* Release non-excl use of FAT. */
                PC_FAT_EXIT(pdrive->driveno)

                if (pdrive->fasize == 8)    /* FAT32 */
                {
                    /* Calculate file total size */
                    ltemp = alloced_size >> pdrive->log2_secpalloc;
                    ltemp >>= 9;  /* Divide by 512 */
                    ltemp += n_clusters;

                    /* Chech the maximum file size(2GB) */
                    if (pdrive->maxfsize_cluster < ltemp)
                    {
                        ret_val = NUF_MAXFILE_SIZE;
                        break;  /* Exit from write loop */
                    }

                }

                /* I/O error occurred */
                if (ret_val <= 0)
                {
                    break;  /* Exit from write loop */
                }

                /* Set contiguous clusters. */
                n_clusters = ret_val;
            }

            /* Are we inside a block */
            if ( (pfile->fptr & 0x1ffL) || (n_left < 512) )
            {
                block_in_cluster = (UINT32) (pfile->fptr & pdrive->byte_into_cl_mask);
                block_in_cluster >>= 9;
                block_to_write = pfile->fptr_block + block_in_cluster;

                byte_offset_in_block = (UINT16) (pfile->fptr & 0x1ffL);

                /* Copy source data to the local buffer */
                n_bytes = (UINT32) (512 - byte_offset_in_block);
                if (n_bytes > n_left)
                    n_bytes = n_left;

                /* Grab the device driver. */
                PC_DRIVE_IO_ENTER(pdrive->driveno)
                /* File pointer is not at block boundary, then we need to read the block. */
                /* Read */
                if ( !pc_bdevsw[pdrive->driveno].io_proc(pdrive->driveno, 
                              block_to_write, local_buf, (UINT16) 1, YES) )
                {
                    /* Release the drive io locks. */
                    PC_DRIVE_IO_EXIT(pdrive->driveno)
                    ret_val = NUF_IO_ERROR;
                    break;  /* Exit from write loop */
                }
                copybuff(&local_buf[byte_offset_in_block], (UINT8 *)buf, (INT)n_bytes);

                /* Write */
                if ( !pc_bdevsw[pdrive->driveno].io_proc(pdrive->driveno, 
                                                block_to_write, local_buf, (UINT16) 1, NO) )
                {
                    /* Release the drive io locks. */
                    PC_DRIVE_IO_EXIT(pdrive->driveno)
                    ret_val = NUF_IO_ERROR;
                    break;  /* Exit from write loop */
                }
                /* Release the drive io locks. */
                PC_DRIVE_IO_EXIT(pdrive->driveno)

                buf += n_bytes;
                n_left -= n_bytes;
                pfile->fptr += n_bytes;

                /* Are we on a cluster boundary  ? */
                if ( !(pfile->fptr & pdrive->byte_into_cl_mask) )
                {
                    if (--n_clusters)             /* If contiguous */
                    {
                        pfile->fptr_block += pdrive->secpalloc;
                        pfile->fptr_cluster += 1;
                    }
                    else
                    {
    /* NOTE:            Put the next cluster into the pointer. If we had
                        alloced a chain this value is the last cluster in 
                        the chain and does not concur with the byte file pointer.
                        This is not a problem since the cluster pointer is known 
                        to be off at this point anyway (fptr>=alloced_size) */
                        pfile->fptr_cluster = next_cluster;

#if 0
/* DAVEM 30GB WD300 ONLY */
            if( (UINT32)pfile->fptr_cluster > 457672 )
			{
	          trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Write: out of range=%d line %d.\n",pfile->fptr_cluster,__LINE__);
			}
			else
			{
              trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Write: next_cluster=%d line %d.\n",next_cluster,__LINE__);
            }
#endif

                        pfile->fptr_block = pc_cl2sector(pdrive, next_cluster);
                    }   /* if (--nclusters) {} else {}; */
                }       /* if (!(pfile->fptr & byte_into_cl_mask)) */
            }           /* if ( (pfile->fptr & 0x1ff) || (n_left < 512) ) */

            if (n_clusters && (n_left>511))
            {
                /* If we get here we need to write contiguous blocks */
                block_in_cluster = (UINT32) (pfile->fptr & pdrive->byte_into_cl_mask);
                block_in_cluster >>= 9;
                block_to_write = pfile->fptr_block + block_in_cluster;
                /* how many do we write ? */
                n_blocks_left = (UINT32) (n_left >> 9);
                n_to_write = (UINT32) ((n_clusters << pdrive->log2_secpalloc) - block_in_cluster);

                if (n_to_write > n_blocks_left)
                {
                    n_to_write = n_blocks_left;

                 /* If we are not writing to the end of the chain we may not
                    advance the cluster pointer to the beginning of the next
                    chain. We add in block_in_cluster so we account for the
                    partial cluster we've already seen */
                    next_cluster = (UINT32) (pfile->fptr_cluster +
                                     ((n_to_write+block_in_cluster) >>  pdrive->log2_secpalloc));
              /* trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Write: next_cluster=%d line %d.\n",next_cluster,__LINE__); */
                }

                ltemp = n_to_write;
                ret_val = 0;
                while (ltemp)
                {
                    if (ltemp > MAXSECTORS)
                        nblocks = MAXSECTORS;
                    else
                        nblocks = (UINT16) ltemp;

                    /* Grab the device driver. */
                    PC_DRIVE_IO_ENTER(pdrive->driveno)
                    /* WRITE */
                    if ( !pc_bdevsw[pdrive->driveno].io_proc(pdrive->driveno, 
                        block_to_write, (UINT8 *)buf, nblocks, NO) )
                    {
                        /* Release the drive io locks. */
                        PC_DRIVE_IO_EXIT(pdrive->driveno)
                        ret_val = NUF_IO_ERROR;
                        break;  /* Exit from write loop */
                    }
                    /* Release the drive io locks. */
                    PC_DRIVE_IO_EXIT(pdrive->driveno)

                    ltemp -= nblocks;
                    block_to_write += nblocks;
                    buf += (nblocks << 9);
                }
                if (ret_val != 0) /* I/O Error */
                {
                    break;  /* Exit from write loop */
                }
                n_bytes = (UINT32) (n_to_write << 9);

                n_left -= n_bytes;
                pfile->fptr += n_bytes;

#if 0
/* DAVEM 30GB WD300 ONLY */
            if( (UINT32)next_cluster > 457672 )
			{
	          trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Write: out of range=%d line %d.\n",next_cluster,__LINE__);
			}
			else
			{
              trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Write: next_cluster=%d line %d.\n",next_cluster,__LINE__);
			}
#endif

                /* See note above */
                pfile->fptr_cluster = next_cluster;
                pfile->fptr_block   = pc_cl2sector(pdrive, next_cluster);
            }
        }       /* while n_left */

        if (n_left == 0)
        {
            /* Update the directory entry file size. */
            if (pfile->fptr > pfile->pobj->finode->fsize)
                pfile->pobj->finode->fsize = pfile->fptr;

            /* If the file pointer is beyond the space allocated to the file note it.
               Since we may need to adjust this file's cluster and block pointers
               later if someone else extends the file behind our back */
            if (pfile->fptr >= alloced_size)
                pfile->at_eof = YES;
            else
                pfile->at_eof = NO;

            /* Set the File update flag */
            pfile->fupdate = YES;
            ret_val = count;
        }
        else
        {
            /* Restore pointers and return */
            pfile->fptr = saved_ptr;
            pfile->fptr_block = saved_ptr_block;
            pfile->fptr_cluster = saved_ptr_cluster;
            fs_user->p_errno = PENOSPC;
        }

        /* Release exclusive use of finode */
        PC_INODE_EXIT(pfile->pobj->finode)
        /* Release non-exclusive use of drive */
        PC_DRIVE_EXIT(pdrive->driveno)
    }

    /* Restore the kernel state */
    PC_FS_EXIT()

    return(ret_val);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       NU_Seek                                                         
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Move the file pointer offset bytes from the origin described by 
*       "origin". The file pointer is set according to the following rules.
*                                                                       
*       Origin                                  Rule                    
*       PSEEK_SET                           offset from begining of file.
*       PSEEK_CUR                           offset from current file.   
*                                            pointer.                   
*       PSEEK_END                           offset from end of file.    
*                                                                       
*       Attempting to seek beyond end of file puts the file pointer one 
*       byte past eof.                                                  
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       fd                                  File descriptor.            
*       offset                              Seek bytes.                 
*       origin                              Origin.                     
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       INT32                               A non-negative integer to be
*                                            used as a number of bytes  
*                                            seek.                      
*       NUF_BAD_USER                        Not a file user.            
*       NUF_BADPARM                         Invalid parameter specified.
*       NUF_NO_DISK                         Disk is removed.            
*       NUF_BADFILE                         File descriptor invalid.    
*       NUF_IO_ERROR                        Driver I/O error.            
*       NUF_INTERNAL                        Nucleus FILE internal error.
*                                                                       
*       fs_user->p_errno will be set with one of the following:         
*                                                                       
*       PEBADF                              File descriptor invalid.    
*       PEINVAL                             Seek to negative file       
*                                            pointer attempted.         
*                                                                       
*************************************************************************/
INT32 NU_Seek(INT fd, INT32 offset, INT16 origin)
{
INT32       ret_val;
STATUS      ret_stat;
PC_FILE     *pfile;
DDRIVE      *pdrive;


    /* Must be last line in declarations */
    PC_FS_ENTER()
    /* Check if a valid user if multitasking */
    CHECK_USER(INT32, NUF_BAD_USER)

    /* Start by assuming success. */
    ret_val = NU_SUCCESS;
    fs_user->p_errno = 0;

    /* Get the FILE. We don't want it if an error has occurred */
    if ( (pfile = pc_fd2file(fd)) == NU_NULL )
    {
        fs_user->p_errno = PEBADF;
        ret_val = NUF_BADFILE;
    }
    else
    {
        /* Move ddrive pointer to local. */
        pdrive = pfile->pobj->pdrive;

        /* Check drive exist ( if there is check driver service ). */
        if (pc_bdevsw[pdrive->driveno].dskchk_proc)
        {
            if ( !pc_bdevsw[pdrive->driveno].dskchk_proc(pdrive->driveno) )
    {
                ret_val = NUF_NO_DISK;
            }
        }
    }

    if (ret_val == NU_SUCCESS)
    {
        /* Register drive in use */
        PC_DRIVE_ENTER(pdrive->driveno, NO)
    /* Grab exclusive access to the drobj */
    PC_INODE_ENTER(pfile->pobj->finode, YES)

    /* Set the cluster and block file pointers if not already set */
    _synch_file_ptrs(pfile);

    /* Call the internal seek routine that we share with NU_Truncate */
    ret_stat =  _po_lseek(pfile, offset, origin);

    if (ret_stat == NU_SUCCESS)
        ret_val = pfile->fptr;
    else
        ret_val = (INT32)ret_stat;

        /* Release excl use of finode. */
    PC_INODE_EXIT(pfile->pobj->finode)
        /* Release non-excl use of drive. */
    PC_DRIVE_EXIT(pdrive->driveno)
    }

    /* Restore the kernel state */
    PC_FS_EXIT()

    return(ret_val);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       NU_Truncate                                                     
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Move the file pointer offset bytes from the beginning of the    
*       file and truncate the file beyond that point by adjusting the   
*       file size and freeing the cluster chain past the file pointer.  
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       fd                                  File descriptor.            
*       offset                              Truncate offset(bytes).     
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       NU_SUCCESS                          The file was       
*                                            successfully truncated.              
*       NUF_BAD_USER                        Not a file user.            
*       NUF_BADPARM                         Invalid parameter specified.
*       NUF_NO_DISK                         Disk is removed.            
*       NUF_BADFILE                         File descriptor invalid.    
*       NUF_ACCES                           You can't change the file   
*                                            which has PO_RDONLY or file
*                                            attributes is ARDONLY.     
*       NUF_SHARE                           The access conflict from    
*                                            multiple task to a specific
*                                            file.                      
*       NUF_NO_BLOCK                        No block buffer available.  
*       NUF_IO_ERROR                        Driver I/O error.            
*       NUF_INTERNAL                        Nucleus FILE internal error.
*                                                                       
*       fs_user->p_errno will be set with one of the following:         
*                                                                       
*       PEBADF                              File descriptor invalid or  
*                                            open read only.            
*       PENOSPC                             I/O error.                   
*       PEINVAL                             Invalid offset.             
*       PESHARE                             Can not truncate a file open
*                                            by more than one handle.   
*                                                                       
*************************************************************************/
STATUS NU_Truncate(INT fd, INT32 offset)
{
STATUS      ret_stat;
PC_FILE     *pfile;
DDRIVE      *pdrive = NU_NULL;
UINT32      first_cluster_to_release;
UINT32      last_cluster_in_chain;
UINT32      clno;
UINT32      saved_ptr;
UINT32      saved_ptr_block;
UINT32      saved_ptr_cluster;


    /* Must be last line in declarations. */
    PC_FS_ENTER()
    /* Check if a valid user if multitasking. */
    CHECK_USER(INT, NUF_BAD_USER)

    /* Start by assuming success. */
    ret_stat = NU_SUCCESS;
    fs_user->p_errno = 0;

    /* Get the FILE. We don't want it if an error has occurred. */
    if ( (pfile = pc_fd2file(fd)) == NU_NULL )
    {
        fs_user->p_errno = PEBADF;
        ret_stat = NUF_BADFILE;
    }
    /* Check the file attributes. */
    else if (pfile->pobj->finode->fattribute & ARDONLY)
    {
        fs_user->p_errno = PEACCES;
        ret_stat = NUF_ACCES;
    }
    /* Make sure we have write privileges. */
    else if ( !((pfile->flag & PO_WRONLY) || (pfile->flag & PO_RDWR)) )
    {
        fs_user->p_errno = PEACCES;
        ret_stat = NUF_ACCES;
    }
    /* Can only truncate a file that you hold exclusively. */
    else if (pfile->pobj->finode->opencount > 1)
    {
        fs_user->p_errno = PESHARE;
        ret_stat = NUF_SHARE;
    }
    else
    {
        /* Move ddrive pointer to local. */
        pdrive = pfile->pobj->pdrive;

        /* Check drive exist ( if there is check driver service ). */
        if (pc_bdevsw[pdrive->driveno].dskchk_proc)
        {
            if ( !pc_bdevsw[pdrive->driveno].dskchk_proc(pdrive->driveno) )
            {
                ret_stat = NUF_NO_DISK;
            }
        }
    }

    if (ret_stat == NU_SUCCESS)
    {
        /* Register drive in use. */
        PC_DRIVE_ENTER(pdrive->driveno, NO)
        /* Grab exclusive access to the file. */
        PC_INODE_ENTER(pfile->pobj->finode, YES)

        /* Set the cluster and block file pointers if not already set. */
        _synch_file_ptrs(pfile);

        /* Save pointers. */
        saved_ptr = pfile->fptr;
        saved_ptr_block = pfile->fptr_block;
        saved_ptr_cluster = pfile->fptr_cluster;

        /* Call the internal seek routine that we share with NU_Seek. 
           Seek to offset from the origin of zero. */
        ret_stat =  _po_lseek(pfile, offset, PSEEK_SET);

        if (ret_stat == NU_SUCCESS)
        {
            offset = pfile->fptr;
            if ((UINT32)offset >= pfile->pobj->finode->fsize)
            {
                ret_stat = NUF_BADPARM;
	            trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Truncate NUF_BADPARM line %d.\n",__LINE__);
                /* Restore pointers and return. */
                pfile->fptr = saved_ptr;
                pfile->fptr_block = saved_ptr_block;
                pfile->fptr_cluster = saved_ptr_cluster;
            }
            else
            {
                /* Are we on a cluster boundary? */
                if ( !(offset & pdrive->byte_into_cl_mask) )
                {
                    /* Free the current cluster and beyond since we're 
                       on a cluster boundary. */
                    first_cluster_to_release = pfile->fptr_cluster; 

                    /* Grab exclusive access to the FAT. */
                    PC_FAT_ENTER(pdrive->driveno)

                    /* Find the previous cluster so we can terminate 
                       the chain. */
                    clno = pfile->pobj->finode->fcluster;
                    last_cluster_in_chain = clno;

                    while (clno != first_cluster_to_release)
                    {
                        last_cluster_in_chain = clno;
                        ret_stat = pc_clnext(&clno, pdrive, clno);
                        if (ret_stat != NU_SUCCESS)
                        {
                            fs_user->p_errno = PENOSPC;
                            break;
                        }
                    }

                    /* Release non-exclusive use of FAT. */
                    PC_FAT_EXIT(pdrive->driveno)
                    if (ret_stat == NU_SUCCESS)
                    {
                        /* Set ptr_cluster to last in chain so 
                           read & write will work right. */
                        pfile->fptr_cluster = last_cluster_in_chain;
                        if (last_cluster_in_chain)
						{

#if 0
/* DAVEM 30GB WD300 ONLY */
            if( (UINT32)last_cluster_in_chain > 457672 )
			{
	          trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Truncate: out of range=%d line %d.\n",last_cluster_in_chain,__LINE__);
			}
#endif
                            pfile->fptr_block = 
                                pc_cl2sector(pdrive, last_cluster_in_chain);
						}
                        else
                            pfile->fptr_block = 0L;
                        pfile->at_eof = YES;
                    }
                }
                /* Simple case. We aren't on a cluster boundary, just free. */
                /* The chain beyond us and terminate the list. */
                else
                {
                    /* Grab exclusive access to the FAT. */
                    PC_FAT_ENTER(pdrive->driveno)

                    last_cluster_in_chain = pfile->fptr_cluster;
                    /* Make sure we are at the end of chain. */
                    ret_stat = pc_clnext(&first_cluster_to_release, 
                                            pdrive, pfile->fptr_cluster);

                    /* Release non-exclusive use of FAT. */
                    PC_FAT_EXIT(pdrive->driveno)
                    pfile->at_eof = YES;
                }

                if (ret_stat == NU_SUCCESS)
                {
                    /*  Now update the directory entry. */
                    pfile->pobj->finode->fsize = offset;
                    /* If the file goes to zero size unlink the chain. */
                    if (!offset)
                    {
                        pfile->pobj->finode->fcluster = 0L;
                        pfile->fptr_cluster = 0L;
                        pfile->fptr_block = 0L;
                        pfile->fptr = 0L;
                        pfile->at_eof = NO;
                        /* We're freeing the whole chain so we don't 
                           mark last_cluster in chain. */
                        last_cluster_in_chain = 0;
                    }

                    /* Update an inode to disk. */
                    ret_stat = pc_update_inode(pfile->pobj, DSET_UPDATE);
                    if (ret_stat != NU_SUCCESS)
                    {
                        fs_user->p_errno = PENOSPC;
                    }
                    else
                    {
                        /* Terminate the chain and free the lost chain 
                           part. */

                        /* Grab exclusive access to the FAT. */
                        PC_FAT_ENTER(pfile->pobj->pdrive->driveno)

                        /* Free the rest of the chain. */
                        if (first_cluster_to_release)
                        {
                            /* Release the chain. */
                            ret_stat = pc_freechain(pfile->pobj->pdrive, 
                                                first_cluster_to_release);
                        }
                        if (ret_stat == NU_SUCCESS)
                        {
                            /* Null terminate the chain. */
                            if (last_cluster_in_chain)
                            {
                                /* Terminate the list we just made. */
                                ret_stat = pc_pfaxx(pdrive, 
                                                    last_cluster_in_chain, 
                                                    ((UINT32) -1));
                                if (ret_stat != NU_SUCCESS)
                                {
                                    fs_user->p_errno = PENOSPC;
                                }
                            }
                        }
                        if (ret_stat == NU_SUCCESS)
                        {
                            /* Flush the file allocation table. */
                            ret_stat = pc_flushfat(pfile->pobj->pdrive);
                            if (ret_stat != NU_SUCCESS)
                            {
                                fs_user->p_errno = PENOSPC;
                            }
                        }

                        /* Release non-exclusive use of FAT. */
                        PC_FAT_EXIT(pfile->pobj->pdrive->driveno)

                        /* Set the File update flag. */
                        pfile->fupdate = YES;
                    }
                }
            }
        }

        /* Release exclusive use of finode. */
        PC_INODE_EXIT(pfile->pobj->finode)
        /* Release non-exclusive use of drive. */
        PC_DRIVE_EXIT(pdrive->driveno)
    }

    /* Restore the kernel state. */
    PC_FS_EXIT()

    return(ret_stat);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       _po_lseek                                                       
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Behaves as NU_Seek but takes a file instead of a file descriptor.
*       Attempting to seek beyond end of file puts the file pointer one 
*       byte past eof.                                                  
*       All setting up such as drive_enter and drobj_enter should have  
*       been done before calling here.                                  
*                                                                       
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *pfile                              Internal file representation.
*       offset                              Seek bytes.                 
*       origin                              Origin.                     
*                                                                       
* OUTPUTS                                                               
*                                                                       
*                                                                       
*       NU_SUCCESS                          If service is successful.   
*       NUF_BADPARM                         Invalid parameter specified.
*       NUF_IO_ERROR                        Driver I/O error.            
*       NUF_INTERNAL                        Nucleus FILE internal error.
*                                                                       
*       fs_user->p_errno will be set with one of the following:         
*                                                                       
*       PEBADF                              File descriptor invalid.    
*       PEINVAL                             Seek to negative file       
*                                            pointer attempted.         
*                                                                       
*************************************************************************/
STATUS _po_lseek(PC_FILE *pfile, INT32 offset, INT16 origin)
{
INT32       ret_val;
INT32       file_pointer;
DDRIVE      *pdrive;
UINT32      ltemp;
UINT32      ltemp2;
UINT32      n_clusters_to_seek;
UINT32      n_clusters;
UINT32      first_cluster;
UINT32      alloced_size;
INT         past_file;
INT16       log2_bytespcluster;


    fs_user->p_errno = 0;

    /*    offset from begining of file */
    if (origin == PSEEK_SET)
    {
        file_pointer = offset;
    }
    /* offset from current file pointer */
    else if (origin == PSEEK_CUR)
    {
        file_pointer = (INT32) pfile->fptr;
        file_pointer += offset;
    }
    /*     offset from end of file */
    else if (origin == PSEEK_END)
    {
        file_pointer = (INT32) pfile->pobj->finode->fsize;
        file_pointer += offset;
    }
    else    /* Illegal origin */
    {
        fs_user->p_errno = PEINVAL;
        return(NUF_BADPARM);
    }

    if (file_pointer < 0L)
    {
        fs_user->p_errno = PEINVAL;
        return(NUF_BADPARM);
    }

    pdrive = pfile->pobj->pdrive;
    /* If file is size zero, we are done */
    if (!pfile->pobj->finode->fsize)
    {
        return(NU_SUCCESS);
    }

    /* Check file size. */
    if (file_pointer > (INT32) pfile->pobj->finode->fsize)
    {
        file_pointer = (INT32) pfile->pobj->finode->fsize;
        past_file = YES;
    }
    else
        past_file = NO;

    /* Get byte offset into the cluster. */
    log2_bytespcluster = (INT16) (pdrive->log2_secpalloc + 9);

    /* How many clusters do we need to seek */
    /* use the current cluster as the starting point if we can */
    if (file_pointer >= (INT32)pfile->fptr)
    {
        first_cluster = pfile->fptr_cluster;
        ltemp  = file_pointer >> log2_bytespcluster;
        ltemp2 = pfile->fptr  >> log2_bytespcluster;
        n_clusters_to_seek = (UINT32) (ltemp - ltemp2);
    }
    else
    {
        /* seek from the beginning */
        first_cluster = pfile->pobj->finode->fcluster;
        ltemp = file_pointer >> log2_bytespcluster;
        n_clusters_to_seek = (UINT32) ltemp;
    }

    /* Cluster chain check. */
    while (n_clusters_to_seek)
    {
        /* Grab exclusive access to the FAT. */
        PC_FAT_ENTER(pdrive->driveno)

        /* Get cluster chain. */
        ret_val = pc_get_chain(pdrive, first_cluster,
                                  &first_cluster, n_clusters_to_seek);
        /* Release non-exclusive use of FAT. */
        PC_FAT_EXIT(pdrive->driveno)

        if (ret_val < 0)
        {
            fs_user->p_errno = PEINVAL;
            return(ret_val);
        }

        n_clusters = ret_val;
        n_clusters_to_seek -= n_clusters;
    }

    /* Update pointers. */
    pfile->fptr_cluster = first_cluster;
    pfile->fptr_block = pc_cl2sector(pdrive, first_cluster);
    pfile->fptr= file_pointer;

    /* If seeking to the end of file, see if we are beyond the allocated size of 
       the file. If we are, we set the at_eof flag so we know to try to move the
       cluster pointer in case another file instance extends the file */
    if (past_file)
    {
        /* Round the file size up to its cluster size by adding in clustersize-1
           and masking off the low bits */
        alloced_size =  (pfile->pobj->finode->fsize + pdrive->byte_into_cl_mask) &
                         ~(pdrive->byte_into_cl_mask);
        /* If the file pointer is beyond the space allocated to the file note it
           since we may need to adjust this file's cluster and block pointers
           later if someone else extends the file behind our back */
        if (pfile->fptr >= alloced_size)
            pfile->at_eof = YES;
        else
            pfile->at_eof = NO;
    }
    else
        pfile->at_eof = NO;

    return(NU_SUCCESS);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       _po_flush                                                       
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Internal version of NU_Flush() called by NU_Flush and NU_Close. 
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *pfile                              Internal file representation.
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       NU_SUCCESS                          If service is successful.   
*       NUF_NO_BLOCK                        No block buffer available.  
*       NUF_IO_ERROR                        I/O error occurred.           
*       NUF_INTERNAL                        Nucleus FILE internal error.
*                                                                       
*       fs_user->p_errno is set to one of these values                  
*                                                                       
*       PENOSPC                             I/O error.                   
*                                                                       
*************************************************************************/
STATUS _po_flush(PC_FILE *pfile)
{
STATUS      ret_val;
INT         setdate = DSET_ACCESS;


	/*printf("_po_flush enter %d\n",__LINE__);*/

    /* Start by assuming success */
    fs_user->p_errno = 0;

    /* File update ? */
    if (pfile->fupdate)
        setdate = DSET_UPDATE;

    /* Convert to native and overwrite the existing inode*/
    ret_val =  pc_update_inode(pfile->pobj, setdate);
    if (ret_val != NU_SUCCESS )
    {
        fs_user->p_errno = PENOSPC;
	    trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"_po_flush PENOSPC line %d.\n",__LINE__);
    }
    else
    {
        /* Grab exclusive access to the FAT. */
        PC_FAT_ENTER(pfile->pobj->pdrive->driveno)

        /* Flush the file allocation table. */
        ret_val = pc_flushfat(pfile->pobj->pdrive);

        /* Release non-exclusive use of FAT. */
        PC_FAT_EXIT(pfile->pobj->pdrive->driveno)
    }

    return(ret_val);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       NU_Flush                                                        
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Flush the file, updating the disk.                               
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       fd                                  File descriptor.            
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       NU_SUCCESS                          The file was 
*                                            successfully flushed.              
*       NUF_BAD_USER                        Not a file user.            
*       NUF_NO_DISK                         Disk is removed.            
*       NUF_BADFILE                         Invalid file descriptor.    
*       NUF_IO_ERROR                        I/O error occurred.           
*       NUF_INTERNAL                        Nucleus FILE internal error.
*                                                                       
*       fs_user->p_errno is set to one of these values                  
*                                                                       
*       PEBADF                              Invalid file descriptor.    
*       PENOSPC                             I/O error.                   
*                                                                       
*************************************************************************/
STATUS NU_Flush(INT fd)
{
STATUS      ret_val;
PC_FILE     *pfile;
INT16       driveno;


    /* Must be last line in declarations */
    PC_FS_ENTER()
    /* Check if a valid user if multitasking */
    CHECK_USER(INT, NUF_BAD_USER)

    /* Start by assuming success */
    ret_val = NU_SUCCESS;
    fs_user->p_errno = 0;

    /* Get the FILE. Take it even if an error has occurred */
    if ((pfile = pc_fd2file(fd)) == NU_NULL)
    {
        fs_user->p_errno = PEBADF;
        ret_val = NUF_BADFILE;
    }
    else
    {
        driveno = pfile->pobj->pdrive->driveno;

        /* Check drive exist ( if there is check driver service ). */
        if (pc_bdevsw[driveno].dskchk_proc)
        {
            if ( !pc_bdevsw[driveno].dskchk_proc(driveno) )
            {
                ret_val = NUF_NO_DISK;
            }
        }
    }

    if (ret_val == NU_SUCCESS)
    {
        /* Register drive in use */
        PC_DRIVE_ENTER(pfile->pobj->pdrive->driveno, NO)

        if (pfile->flag & ( PO_RDWR | PO_WRONLY ))
        {
            /* Claim exclusive access on flush */
            PC_INODE_ENTER(pfile->pobj->finode, YES)

            /* Flush directory entry and FAT. */
            ret_val = _po_flush(pfile);

            /* Release exclusive use of finode. */
            PC_INODE_EXIT(pfile->pobj->finode)
        }

        /* Release non-excl use of drive. */
        PC_DRIVE_EXIT(pfile->pobj->pdrive->driveno)
    }

    /* Restore the kernel state */
    PC_FS_EXIT()

    return(ret_val);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       NU_Close                                                        
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Close the file updating the disk and freeing all core associated
*       with FD.                                                        
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       fd                                  File descriptor.            
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       NU_SUCCESS                          The file was          
*                                            successfully closed.              
*       NUF_BAD_USER                        Not a file user.            
*       NUF_NO_DISK                         Disk is removed.            
*       NUF_BADFILE                         Invalid file descriptor.    
*       NUF_IO_ERROR                        I/O error occurred.           
*       NUF_INTERNAL                        Nucleus FILE internal error.
*                                                                       
*       fs_user->p_errno is set to one of these values                  
*                                                                       
*       PEBADF                              Invalid file descriptor.    
*       PENOSPC                             I/O error.                   
*                                                                       
*************************************************************************/
STATUS NU_Close(INT fd)
{
STATUS      ret_val;
PC_FILE     *pfile;
INT16       driveno;


    /* Must be last line in declarations */
    PC_FS_ENTER()
    /* Check if a valid user if multitasking */
    CHECK_USER(INT, NUF_BAD_USER)

    /* Start by assuming success. */
    ret_val = NU_SUCCESS;
    fs_user->p_errno = 0;

    /* Get the FILE. We don't want it if an error has occured. */
    if ((pfile = pc_fd2file(fd)) == NU_NULL)
    {
        fs_user->p_errno = PEBADF;
        ret_val = NUF_BADFILE;
	    trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Close NUF_BADFILE line %d.\n",__LINE__);
    }
    else
    {
        driveno = pfile->pobj->pdrive->driveno;

        /* Check drive exist ( if there is check driver service ). */
        if (pc_bdevsw[driveno].dskchk_proc)
        {
            if ( !pc_bdevsw[driveno].dskchk_proc(driveno) )
            {
                ret_val = NUF_NO_DISK;
            }
        }

        /* Register drive in use */
        PC_DRIVE_ENTER(driveno, NO)

        if (ret_val == NU_SUCCESS)
        {
        if (pfile->flag & ( PO_RDWR | PO_WRONLY ))
        {
                /* Grab exclusive access to the file. */
            PC_INODE_ENTER(pfile->pobj->finode, YES)

                /* Flush directory entry and FAT. */
            ret_val = _po_flush(pfile);

                /* Release excl use of finode. */
            PC_INODE_EXIT(pfile->pobj->finode)
        }
        }

        /* Release the FD and its core */
        pc_freefile(fd);
        /* Release non-exclusive use of drive. */
        PC_DRIVE_EXIT(driveno)
    }

    /* Restore the kernel state. */
    PC_FS_EXIT()

    return(ret_val);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       NU_Set_Attributes                                               
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Set a file attributes.                                          
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *name                               File name                   
*       newattr                             New file attibute           
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       NU_SUCCESS                          The attributes were      
*                                            set successfully.              
*       NUF_BAD_USER                        Not a file user.            
*       NUF_BADDRIVE                        Invalid drive specified.    
*       NUF_BADPARM                         Invalid parameter specified.
*       NUF_NOT_OPENED                      The disk is not opened yet. 
*       NUF_NO_DISK                         Disk is removed.            
*       NUF_INVNAME                         Path or filename includes   
*                                            invalid character.         
*       NUF_ACCES                           You can't change VOLLABELs.        
*       NUF_NOFILE                          The specified file not      
*                                            found.                     
*       NUF_NO_BLOCK                        No block buffer available.  
*       NUF_NO_FINODE                       No FINODE buffer available. 
*       NUF_NO_DROBJ                        No DROBJ buffer available.  
*       NUF_IO_ERROR                        Driver I/O error.            
*       NUF_INTERNAL                        Nucleus FILE internal error.
*                                                                       
*       fs_user->p_errno is set to one of these values                  
*                                                                       
*       PENOENT                             File not found or path to   
*                                            file not found.            
*       PEACCES                             Attempt to open a read only 
*                                            file or a special          
*                                            (directory) file.          
*       PENOSPC                             I/O error.                   
*                                                                       
*************************************************************************/
STATUS NU_Set_Attributes(CHAR *name, UINT8 newattr)
{
STATUS      ret_stat;
STATUS      ret_stat_w;
DROBJ       *pobj;
DROBJ       *parent_obj;
VOID        *mompath;
VOID        *filename;
VOID        *fileext;
INT         fattribute;
INT         wdcard;
INT16       driveno;


    /* Must be last line in declarations */
    PC_FS_ENTER()
    /* Check if a valid user if multitasking */
    CHECK_USER(INT, NUF_BAD_USER)

    /* Start by assuming success. */
    ret_stat = NU_SUCCESS;
    fs_user->p_errno = 0;

    pobj = NU_NULL;
    parent_obj = NU_NULL;

    /* Get the drive number. */
    if (pc_parsedrive(&driveno, (UINT8 *)name))
    {
        /* Check drive exist ( if there is check driver service ). */
        if (pc_bdevsw[driveno].dskchk_proc)
        {
            if ( !pc_bdevsw[driveno].dskchk_proc(driveno) )
            {
                ret_stat = NUF_NO_DISK;
            }
    }

        if (ret_stat == NU_SUCCESS)
        {
            /* Get the path */
            ret_stat = 
                pc_parsepath(&mompath, &filename, &fileext, (UINT8 *)name);
        }
    }
    else
    {
        ret_stat = NUF_BADDRIVE;
    }

    if (ret_stat != NU_SUCCESS)
    {
        fs_user->p_errno = PENOENT;
    }
    /* Check the "." or ".." */
    else if ( (pc_isdot((UINT8 *)filename, (UINT8 *)fileext)) ||
              (pc_isdotdot((UINT8 *)filename, (UINT8 *)fileext)) )
    {
        fs_user->p_errno = PENOENT;
        ret_stat = NUF_ACCES;
    }
    else 
    {
        /* Set the use wild card flag and check the parameters */
        wdcard = pc_use_wdcard((UINT8 *)filename);
        if (( wdcard ) && (newattr & ADIRENT) )
        {
            fs_user->p_errno = PEINVAL;
            ret_stat = NUF_BADPARM;
	        trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Set_Attributes - NUF_BADPARM line %d.\n",__LINE__);
        }
        else if (newattr & AVOLUME)
        {
            fs_user->p_errno = PEINVAL;
            ret_stat = NUF_BADPARM;
	        trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Set_Attributes - NUF_BADPARM line %d.\n",__LINE__);
        }
        else
        {
            /* Register access to the drive */
            PC_DRIVE_ENTER(driveno, NO)

            /* Find the parent */
            ret_stat = pc_fndnode(driveno, &parent_obj, (UINT8 *)mompath);
            if (ret_stat != NU_SUCCESS)
            {
                fs_user->p_errno = PENOENT;
            }
            else
            {
                /* Lock the parent finode. */
                PC_INODE_ENTER(parent_obj->finode, YES)

                /* Find the file and init the structure */
                pobj = NU_NULL;
                ret_stat =  pc_get_inode(&pobj, parent_obj, (UINT8 *)filename);
                if (ret_stat != NU_SUCCESS)
                {
                    fs_user->p_errno = PENOENT;
                }
                else
                {
                    /* Check the "." or ".." */
                    if ( (pc_isdot(pobj->finode->fname, pobj->finode->fext)) ||
                         (pc_isdotdot(pobj->finode->fname, pobj->finode->fext)) )
                    {
                        fs_user->p_errno = PENOENT;
                        ret_stat = NUF_NOFILE;
                    }
                    /* Check the access permissions */
                    else if ( (wdcard && (pobj->finode->fattribute & ADIRENT)) ||
                              (pobj->finode->fattribute & AVOLUME) )
                    {
                        fs_user->p_errno = PEACCES;
                        ret_stat = NUF_ACCES;
                    }
                    else
                    {
                        /* We can't remove attribute directory from directory file */
                        if ( ((pc_isadir(pobj)) && (newattr & ADIRENT)) ||
                             ((!pc_isadir(pobj)) && !(newattr & ADIRENT)) )
                        {
                            /* Set new attribute */
                            fattribute = pobj->finode->fattribute;
                            pobj->finode->fattribute = newattr;
                            /* Update an inode to disk. */
                            ret_stat = pc_update_inode(pobj, DSET_ACCESS);
                            if (ret_stat != NU_SUCCESS)
                            {
                                pobj->finode->fattribute = fattribute;
                            }
                        }
                        else
                        {
                            /* We can't remove attribute directory from directory file */
                            fs_user->p_errno = PEACCES;
                            ret_stat = NUF_ACCES;
                        }
                    }
                }

                /* If we couldn't change first file attribute, but as long as wild card is used 
                   for search, we need to find next file */
                if ( (wdcard) && ((ret_stat == NU_SUCCESS) || (ret_stat == NUF_ACCES) || (ret_stat == NUF_NOFILE)) )
                {
                    /* Search loop */
                    while(wdcard)
                    {
                        /* We need to clean long filename information */
                        if (pobj->linfo.lnament)
                        {
                            lnam_clean(&pobj->linfo, pobj->pblkbuff);
                            pc_free_buf(pobj->pblkbuff, NO);
                        }

                        /* Now find the next file */
                        ret_stat_w = pc_next_inode(pobj, parent_obj, (UINT8 *)filename, (AVOLUME | ADIRENT));
                        if (ret_stat_w != NU_SUCCESS)
                        {
                            if ( (ret_stat == NU_SUCCESS) && (ret_stat_w == NUF_NOFILE) )
                            {
                                fs_user->p_errno = 0;
                            }
                            else if ( (ret_stat != NU_SUCCESS) && (ret_stat_w == NUF_ACCES) )
                            {
                                fs_user->p_errno = PEACCES;
                                ret_stat = NUF_ACCES;
                            }
                            break;                      /* Next file is not found. */
                        }

                        /* Set new attribute */
                        fattribute = pobj->finode->fattribute;
                        pobj->finode->fattribute = newattr;

                        /* Update an inode to disk. */
                        ret_stat = pc_update_inode(pobj, DSET_ACCESS);
                        if (ret_stat != NU_SUCCESS)
                        {
                            /* I/O Error */
                            pobj->finode->fattribute = fattribute;
                            break;
                        }
                    }
                }
                /* Free the current object. */
                if (pobj)
                    pc_freeobj(pobj);

                /* Release excl use of finode. */
                PC_INODE_EXIT(parent_obj->finode)
                /* Free the parent object. */
                pc_freeobj(parent_obj);
            }

            /* Release non-excl use of drive. */
            PC_DRIVE_EXIT(driveno)
        }
    }

    /* Restore the kernel state */
    PC_FS_EXIT()

    return(ret_stat);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       NU_Get_Attributes                                               
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Get a file's attributes.                                          
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       attr                                Attribute         
*       name                                File name                   
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       File attributes (returned by referrence in "attr").                                                
*       NUF_BAD_USER                        Not a file user.            
*       NUF_BADDRIVE                        Invalid drive specified.    
*       NUF_BADPARM                         Invalid parameter specified.
*       NUF_NOT_OPENED                      The disk is not opened yet. 
*       NUF_NO_DISK                         Disk is removed.            
*       NUF_INVNAME                         Path or filename includes   
*                                            invalid character.         
*       NUF_ACCES                           You can't get attributes the
*                                            file which has "." or "..".
*       NUF_NOFILE                          The specified file not      
*                                            found.                     
*       NUF_NO_FINODE                       No FINODE buffer available. 
*       NUF_NO_DROBJ                        No DROBJ buffer available.  
*       NUF_IO_ERROR                        Driver I/O error.            
*       NUF_INTERNAL                        Nucleus FILE internal error.
*                                                                       
*       fs_user->p_errno is set to one of these values                  
*                                                                       
*       PENOENT                             File not found or path to   
*                                            file not found.            
*                                                                       
*************************************************************************/
STATUS NU_Get_Attributes(UINT8 *attr, CHAR *name)
{
STATUS      ret_stat;
DROBJ       *pobj;
DROBJ       *parent_obj;
VOID        *mompath;
VOID        *filename;
VOID        *fileext;
INT16       driveno;


    /* Must be last line in declarations */
    PC_FS_ENTER()
    /* Check if a valid user if multitasking */
    CHECK_USER(INT, NUF_BAD_USER)

    /* Start by assuming success. */
    ret_stat = NU_SUCCESS;
    fs_user->p_errno = 0;

    pobj = NU_NULL;
    parent_obj = NU_NULL;

    /* Get the drive number. */
    if (pc_parsedrive(&driveno, (UINT8 *)name))
    {
        /* Check drive exist ( if there is check driver service ). */
        if (pc_bdevsw[driveno].dskchk_proc)
        {
            if ( !pc_bdevsw[driveno].dskchk_proc(driveno) )
    {
                ret_stat = NUF_NO_DISK;
            }
    }
    
        if (ret_stat == NU_SUCCESS)
        {
            /* Get the path */
            ret_stat = 
                pc_parsepath(&mompath, &filename, &fileext, (UINT8 *)name);
        }
    }
    else
    {
        ret_stat = NUF_BADDRIVE;
    }

    if (ret_stat != NU_SUCCESS)
    {
        fs_user->p_errno = PENOENT;
    }
    /* Check the use of wild cards.
       Wild card is not available on get attribute service */
    else if (pc_use_wdcard((UINT8 *)filename))
    {
        fs_user->p_errno = PEINVAL;
        ret_stat = NUF_BADPARM;
	    trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Get_Attributes - NUF_BADPARM line %d.\n",__LINE__);
    }
    /* Check the "." or ".." */
    else if ( (pc_isdot((UINT8 *)filename, (UINT8 *)fileext)) ||
              (pc_isdotdot((UINT8 *)filename, (UINT8 *)fileext)) )
    {
        fs_user->p_errno = PENOENT;
        ret_stat = NUF_ACCES;
    }
    else
    {
        /* Register access to the drive */
        PC_DRIVE_ENTER(driveno, NO)

        /* Find the parent */
        ret_stat = pc_fndnode(driveno, &parent_obj, (UINT8 *)mompath);
        if (ret_stat != NU_SUCCESS)
        {
            fs_user->p_errno = PENOENT;
        }
        else
        {
            /* Lock the parent finode. */
            PC_INODE_ENTER(parent_obj->finode, YES)

            /* Find the file and init the structure */
            pobj = NU_NULL;
            ret_stat =  pc_get_inode(&pobj, parent_obj, (UINT8 *)filename);
            if (ret_stat != NU_SUCCESS)
            {
                fs_user->p_errno = PENOENT;
            }
            else
            {
                *attr = pobj->finode->fattribute;
                pc_freeobj(pobj);
            }

            /* Release excl use of finode. */
            PC_INODE_EXIT(parent_obj->finode)
            /* Free the parent object. */
            pc_freeobj(parent_obj);
        }

        /* Release non-excl use of drive. */
        PC_DRIVE_EXIT(driveno)
    }
    PC_DRIVE_EXIT(driveno)

    /* Restore the kernel state */
    PC_FS_EXIT()

    return(ret_stat);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       NU_Rename                                                       
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Renames the file in path ("name") to "newname". Fails if "name" is    
*       invalid, "newname" already exists or path not found. Does not test
*       if "name" is a simple file. It is possible to rename directories.
*       (This may change in the multiuser version)
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *name                               Old file name               
*       *newname                            New file name(Rename)       
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       NU_SUCCESS                          File was successfully renamed.    
*       NUF_BAD_USER                        Not a file user.            
*       NUF_BADDRIVE                        Invalid drive specified.    
*       NUF_BADPARM                         Invalid parameter specified.
*       NUF_NOT_OPENED                      The disk is not opened yet. 
*       NUF_NO_DISK                         Disk is removed.            
*       NUF_ROOT_FULL                       Root directry full.         
*       NUF_INVNAME                         Path or filename includes   
*                                            invalid character.         
*       NUF_ACCES                           You can't change the file   
*                                            which has VOLLABEL, HIDDEN,
*                                            or SYSTEM attributes.       
*       NUF_NOSPC                           No space to create directory
*       NUF_NOFILE                          The specified file not      
*                                            found.                     
*       NUF_NO_BLOCK                        No block buffer available.  
*       NUF_NO_FINODE                       No FINODE buffer available. 
*       NUF_NO_DROBJ                        No DROBJ buffer available.  
*       NUF_IO_ERROR                        Driver I/O error.            
*       NUF_INTERNAL                        Nucleus FILE internal error.
*                                                                       
*       fs_user->p_errno is set to one of these values                  
*                                                                       
*       PENOENT                             File not found or path to   
*                                            file not found.            
*       PEACCES                             Attempt to open a read only 
*                                            file or a special          
*                                            (directory) file.          
*       PEEXIST                             Exclusive access requested  
*                                            but file already exists.   
*                                                                       
*************************************************************************/
INT NU_Rename(CHAR *name, CHAR *newname)
{
STATUS      ret_stat;
STATUS      ret_stat_w;
DROBJ       *pobj;
DROBJ       *new_pobj;
DROBJ       *parent_obj;
VOID        *mompath;
VOID        *filename;
VOID        *fileext;
VOID        *newfilename;
VOID        *newext;
VOID        *new_name;
VOID        *new_ext;
INT         longdest;
INT         wdcard;
INT         len;
INT16       driveno;
UINT8       fnambuf[9];
UINT8       fextbuf[4];
UINT8       shortname[13];
UINT8       fname[EMAXPATH+1];


    /* Must be last line in declarations */
    PC_FS_ENTER()
    /* Check if a valid user if multitasking */
    CHECK_USER(INT, NUF_BAD_USER)

    /* Start by assuming success. */
    ret_stat = NU_SUCCESS;
    fs_user->p_errno = 0;

    pobj = NU_NULL;
    parent_obj = NU_NULL;

    /* Get the drive number. */
    if (pc_parsedrive(&driveno, (UINT8 *)name))
    {
        /* Check drive exist ( if there is check driver service ). */
        if (pc_bdevsw[driveno].dskchk_proc)
        {
            if ( !pc_bdevsw[driveno].dskchk_proc(driveno) )
            {
                ret_stat = NUF_NO_DISK;
            }
    }
    
        if (ret_stat == NU_SUCCESS)
        {
    /* Get the new filename.
       NOTE: We will re-use mompath.
             The new filename MAY NOT HAVE a path component */
            ret_stat = pc_parsepath(&mompath, &newfilename, 
                                        &newext, (UINT8 *)newname);
        }
    }
    else
    {
        ret_stat = NUF_BADDRIVE;
    }

    if (ret_stat != NU_SUCCESS)
    {
        fs_user->p_errno = PENOENT;
    }
    /* Don't specify newname path. */
    else if (mompath)
    {
        ret_stat = NUF_ACCES;
    }
    else
    {
        /* Get the path */
        ret_stat = pc_parsepath(&mompath, &filename, &fileext, (UINT8 *)name);
        if (ret_stat != NU_SUCCESS)
        {
            fs_user->p_errno = PENOENT;
        }
        else
        {
            /* Check the "." or ".." */
            if ( (pc_isdot((UINT8 *)newfilename, (UINT8 *)newext)) ||
                 (pc_isdotdot((UINT8 *)newfilename, (UINT8 *)newext)) ||
                 (pc_isdot((UINT8 *)filename, (UINT8 *)fileext)) ||
                 (pc_isdotdot((UINT8 *)filename, (UINT8 *)fileext)) )
            {
                fs_user->p_errno = PEACCES;
                ret_stat = NUF_ACCES;
            }
        }

        /* Check for the same oldname as newname. */
        if (YES == pc_patcmp((UINT8 *)filename, (UINT8 *)newfilename))
        {
            fs_user->p_errno = PEINVAL;
            ret_stat = NUF_BADPARM;
		    /*printf("NU_Rename - BADPARM %d\n",__LINE__);*/
        }
    }

    /* File syntax check OK */
    if (ret_stat == NU_SUCCESS)
    {
        /* Register access to the drive */
        PC_DRIVE_ENTER(driveno, NO)

        /* Find the parent */
        ret_stat = pc_fndnode(driveno, &parent_obj, (UINT8 *)mompath);
        if (ret_stat != NU_SUCCESS)
        {
            fs_user->p_errno = PENOENT;
        }
        else
        {
            /* Set the use wild card flag */
            wdcard = pc_use_wdcard((UINT8 *)filename);
            wdcard |= pc_use_wdcard((UINT8 *)newfilename);

            /* Lock the parent finode. */
            PC_INODE_ENTER(parent_obj->finode, YES)

            /* Find the source file and init the structure */
            pobj = NU_NULL;
            ret_stat =  pc_get_inode(&pobj, parent_obj, (UINT8 *)filename);
            if (ret_stat != NU_SUCCESS)
            {
                fs_user->p_errno = PENOENT;
            }
            else if ( pobj->finode->fattribute & ADIRENT )
            { 
                /* Check if the new directory name will make the path too long */
                for (len = 0; *((UINT8 *)newname + len); len++);
                if ( (parent_obj->finode->abs_length + len) >= (EMAXPATH - 12) )
                {
                    ret_stat = NUF_LONGPATH;
                }
            }
            if (ret_stat == NU_SUCCESS)
            {
                /* Check the "." or ".." */
                if ( (pc_isdot(pobj->finode->fname, pobj->finode->fext)) ||
                     (pc_isdotdot(pobj->finode->fname, pobj->finode->fext)) )
                {
                    fs_user->p_errno = PENOENT;
                    ret_stat = NUF_NOFILE;
                }
                /* Check the access permissions */
                else if ( (pobj->finode->fattribute & (AHIDDEN | AVOLUME| ASYSTEM)) ||
                          (wdcard && (pobj->finode->fattribute & ADIRENT)) )
                {
                    fs_user->p_errno = PEACCES;
                    ret_stat = NUF_ACCES;
                }
                else
                {
                    if (wdcard)
                    {
                        /* Setup the new file name */
                        pc_parsenewname(pobj, (UINT8 *)newfilename, (UINT8 *)newext, 
                                                &new_name, &new_ext, fname);
                    }
                    else
                    {
                        new_name = newfilename;
                        new_ext  = newext;
                    }

                    /* Find the source file and init the structure */
                    new_pobj = NU_NULL;
                    ret_stat =  pc_get_inode(&new_pobj, parent_obj, (UINT8 *)new_name);
                    if (ret_stat == NU_SUCCESS)
                    {
                        /* Free the current object. */
                        pc_freeobj(new_pobj);
                        fs_user->p_errno = PEEXIST;
                        ret_stat = NUF_EXIST;
                    }
                    /* New filename must not exist. */
                    else if (ret_stat == NUF_NOFILE)
                    {
                        /* Status clear. */
                        ret_stat = NU_SUCCESS;

                        /* Take a short filename and extension. */
                        longdest = pc_fileparse( fnambuf, fextbuf, new_name, new_ext );
                        if (longdest < 0)
                        {
                            fs_user->p_errno = PENOENT;
                            ret_stat = longdest;
                        }

                        /* New filename is a long filename */
                        /* Upperbar short filename ?  */
                        while( (longdest == FUSE_UPBAR) && (ret_stat == NU_SUCCESS) )
                        {
                            /* Search the short filename */
                            pc_cre_shortname(shortname, fnambuf, fextbuf);
                            new_pobj = NU_NULL;
                            ret_stat =  pc_get_inode(&new_pobj, parent_obj, shortname);
                            if (ret_stat == NUF_NOFILE)
                            {
                                ret_stat = NU_SUCCESS;
                                break;
                            }
                            else if (ret_stat == NU_SUCCESS)
                            {
                                /* Free the current object. */
                                pc_freeobj(new_pobj);
                                /* Get the next short filename */
                                pc_next_fparse(fnambuf);
                            }
                        }

                        /* measure long filename length */
                        for (len = 0; *((UINT8 *)new_name+len); len++);

                        if ((len + parent_obj->finode->abs_length) > EMAXPATH)
                        {
                            pc_report_error(PCERR_PATHL);
                            ret_stat = NUF_LONGPATH;
                        }
                        if (ret_stat == NU_SUCCESS)
                        {
                            /* Rename the file */
                            ret_stat = pc_renameinode(pobj, parent_obj, fnambuf, fextbuf, (UINT8 *)new_name, longdest);
                        }
                    }
                }

                /* If we couldn't change first filename, but as long as wild card is used 
                   for search, we need to find next file */
                if ( (wdcard) && ((ret_stat == NU_SUCCESS) || (ret_stat == NUF_ACCES) || (ret_stat == NUF_NOFILE)) )
                {
                    while (wdcard)
                    {
                        /* We need to clean long filename information */
                        if (pobj->linfo.lnament)
                        {
                            lnam_clean(&pobj->linfo, pobj->pblkbuff);
                            pc_free_buf(pobj->pblkbuff, NO);
                        }
                        /* Now find the next file */
                        ret_stat_w = pc_next_inode(pobj, parent_obj, (UINT8 *)filename,
                                                   (AHIDDEN | AVOLUME | ADIRENT | ASYSTEM));
                        if (ret_stat_w == NU_SUCCESS)
                        {
                            ret_stat = ret_stat_w;
                        }
                        else
                        {
                            if ( (ret_stat == NU_SUCCESS) && (ret_stat_w == NUF_NOFILE) )
                            {
                                fs_user->p_errno = 0;
                            }
                            else if ( (ret_stat != NU_SUCCESS) && (ret_stat_w == NUF_ACCES) )
                            {
                                fs_user->p_errno = PEACCES;
                                ret_stat = NUF_ACCES;
                            }
                            break;                      /* Next file is not found. */
                        }

                        /* Setup the new filename */
                        pc_parsenewname(pobj, (UINT8 *)newfilename, (UINT8 *)newext, 
                                                &new_name, &new_ext, fname);

                        /* Take a short filename and extension. */
                        longdest = pc_fileparse(fnambuf, fextbuf, new_name, new_ext);
                        if (longdest < 0)
                        {
                            fs_user->p_errno = PENOENT;
                            ret_stat = (STATUS)longdest;
                            break;
                        }

                        /* New filename is a long filename */
                        /* Upperbar short filename ?  */
                        while( (longdest == FUSE_UPBAR) && (ret_stat == NU_SUCCESS) )
                        {
                            pc_cre_shortname(shortname, fnambuf, fextbuf);
                            /* Search the short filename */
                            new_pobj = NU_NULL;
                            ret_stat =  pc_get_inode(&new_pobj, parent_obj, shortname);
                            if (ret_stat == NUF_NOFILE)
                            {
                                ret_stat = NU_SUCCESS;
                                break;
                            }
                            else if (ret_stat == NU_SUCCESS)
                            {
                                /* Free the current object. */
                                pc_freeobj(new_pobj);
                                /* Get the next short filename */
                                pc_next_fparse(fnambuf);
                            }
                        }
                        /* measure long filename length */
                        for (len = 0; *((UINT8 *)new_name+len); len++);
                        if ((len + parent_obj->finode->abs_length) > EMAXPATH)
                        {
                            pc_report_error(PCERR_PATHL);
                            ret_stat = NUF_LONGPATH;
                        }

                        if (ret_stat == NU_SUCCESS)
                        {
                            /* Find the source file and init the structure */
                            new_pobj = NU_NULL;
                            ret_stat =  pc_get_inode(&new_pobj, parent_obj, (UINT8 *)new_name);
                            if (ret_stat == NU_SUCCESS)
                            {
                                /* Free the current object. */
                                pc_freeobj(new_pobj);
                                fs_user->p_errno = PEEXIST;
                                ret_stat = NUF_EXIST;
                                break;
                            }
                            /* Rename the file */
                            ret_stat = pc_renameinode(pobj, parent_obj, fnambuf, fextbuf, (UINT8 *)new_name, longdest);
                            if (ret_stat != NU_SUCCESS)
                            {
                                /* I/O error */
                                break;
                            }
                        }
                    }
                }
            }
            /* Free the current object. */
            if (pobj)
                pc_freeobj(pobj);

            /* Release excl use of finode. */
            PC_INODE_EXIT(parent_obj->finode)
            /* Free the parent object. */
            pc_freeobj(parent_obj);
        }

        /* Release non-excl use of drive. */
        PC_DRIVE_EXIT(driveno)
    }

    /* Restore the kernel state */
    PC_FS_EXIT()

    return(ret_stat);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       NU_Delete                                                       
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Delete the file in "name". Fail if not a simple file, if it
*       is open, does not exist, or is read only.                                 
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *name                               File name to be deleted.    
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       NU_SUCCESS                          Delete request completed    
*                                            successfully.              
*       NUF_BAD_USER                        Not a file user.            
*       NUF_BADDRIVE                        Invalid drive specified.    
*       NUF_NOT_OPENED                      The disk is not opened yet. 
*       NUF_NO_DISK                         Disk is removed.            
*       NUF_INVNAME                         Path or filename includes   
*                                            invalid character.         
*       NUF_ACCES                           This file has at least one  
*                                            of the following attributes:
*                                            RDONLY,HIDDEN,SYSTEM,VOLUME,
*                                            DIRENT.                    
*       NUF_SHARE                           The access conflict from    
*                                            multiple task to a specific
*                                            file.                      
*       NUF_NOFILE                          The specified file not found.
*       NUF_NO_BLOCK                        No block buffer available.  
*       NUF_NO_FINODE                       No FINODE buffer available. 
*       NUF_NO_DROBJ                        No DROBJ buffer available.  
*       NUF_IO_ERROR                        Driver I/O function routine  
*       NUF_INTERNAL                        Nucleus FILE internal error.
*                                            returned error.            
*                                                                       
*       fs_user->p_errno is set to one of these values                  
*                                                                       
*       PENOENT                             File not found or path to   
*                                            file not found.            
*       PEACCES                             Attempt delete a directory  
*                                            or an open file.           
*       PENOSPC                             Write failed.               
*                                                                       
*************************************************************************/
STATUS NU_Delete(CHAR *name)
{
STATUS      ret_stat;
STATUS      ret_stat_w;
DROBJ       *pobj;
DROBJ       *parent_obj;
VOID        *path;
VOID        *filename;
VOID        *fileext;
INT         wdcard;
INT16       driveno;


    /* Must be last line in declarations */
    PC_FS_ENTER()
    /* Check if a valid user if multitasking */
    CHECK_USER(INT, NUF_BAD_USER)

    /* Start by assuming success. */
    ret_stat = NU_SUCCESS;
    fs_user->p_errno = 0;

    parent_obj  = NU_NULL;
    pobj        = NU_NULL;

    /* Get the drive number. */
    if (pc_parsedrive(&driveno, (UINT8 *)name))
    {
        /* Check drive exist ( if there is check driver service ). */
        if (pc_bdevsw[driveno].dskchk_proc)
        {
            if ( !pc_bdevsw[driveno].dskchk_proc(driveno) )
            {
                ret_stat = NUF_NO_DISK;
            }
        }

        if (ret_stat == NU_SUCCESS)
    {
            /* Get out the filename and d:parent */
            ret_stat = 
                pc_parsepath(&path, &filename, &fileext, (UINT8 *)name);
        }
    }
    else
    {
        ret_stat = NUF_BADDRIVE;
    }

    if (ret_stat != NU_SUCCESS)
    {
        fs_user->p_errno = PENOENT;
    }
    /* Check the "." or ".." */
    else if ( (pc_isdot((UINT8 *)filename, (UINT8 *)fileext)) ||
              (pc_isdotdot((UINT8 *)filename, (UINT8 *)fileext)) )
    {
        fs_user->p_errno = PEACCES;
        ret_stat = NUF_ACCES;
    }
    else
    {
        /* Register access to the drive */
        PC_DRIVE_ENTER(driveno, NO)

        /* Find the parent and make sure it is a directory */
        ret_stat = pc_fndnode(driveno, &parent_obj, (UINT8 *)path);
        if (ret_stat == NU_SUCCESS)
        {
            if ( !parent_obj || !pc_isadir(parent_obj) ||  pc_isavol(parent_obj))
            {
                fs_user->p_errno = PEACCES;
                ret_stat = NUF_ACCES;
            }
            else
            {
                /* Set the use wild card flag */
                wdcard = pc_use_wdcard((UINT8 *)filename);

                /* Lock the parent finode. */
                PC_INODE_ENTER(parent_obj->finode, YES)

                /* Find the file and init the structure. */
                pobj = NU_NULL;
                ret_stat = pc_get_inode(&pobj, parent_obj, (UINT8 *)filename);
                if (ret_stat != NU_SUCCESS)
                {
                    fs_user->p_errno = PENOENT;
                }
                else
                {
                    /* Be sure it is not the root. Since the root is an abstraction 
                        we can not delete it */
                    if (pc_isroot(pobj))
                    {
                        fs_user->p_errno = PEACCES;
                        ret_stat = NUF_ACCES;
                    }
                    else if (pobj->finode->opencount > 1)
                    {
                        fs_user->p_errno = PESHARE;
                        ret_stat = NUF_SHARE;
                    }
                    else if ( (pc_isdot(pobj->finode->fname, pobj->finode->fext)) ||
                              (pc_isdotdot(pobj->finode->fname, pobj->finode->fext)) )
                    {
                        fs_user->p_errno = PENOENT;
                        ret_stat = NUF_NOFILE;
                    }
                    else if ( pobj->finode->fattribute & (ARDONLY | AHIDDEN | ASYSTEM | AVOLUME | ADIRENT) )
                    {
                        fs_user->p_errno = PEACCES;
                        ret_stat = NUF_ACCES;
                    }
                    else
                    {
                        /* Delete an inode.  */
                        ret_stat = pc_rmnode(pobj);
                        if (ret_stat != NU_SUCCESS)
                        {
                            fs_user->p_errno = PENOSPC;
                            /* Even if wildcard is used, abort delete process on error. */
                            wdcard = 0;
                        }
                    }

                    while(wdcard)
                    {
                        /* We need to clean long filename information */
                        if (pobj->linfo.lnament)
                        {
                            lnam_clean(&pobj->linfo, pobj->pblkbuff);
                            pc_free_buf(pobj->pblkbuff, NO);
                        }

                        /* Now find the next file */
                        ret_stat_w = pc_next_inode(pobj, parent_obj, (UINT8 *)filename, 
                                                    (ARDONLY | AHIDDEN | ASYSTEM | AVOLUME | ADIRENT));
                        if (ret_stat_w != NU_SUCCESS)
                        {
                            if ( (ret_stat == NU_SUCCESS) && (ret_stat_w == NUF_NOFILE) )
                            {
                                fs_user->p_errno = 0;
                            }
                            else if ( (ret_stat != NU_SUCCESS) && (ret_stat_w == NUF_ACCES) )
                            {
                                fs_user->p_errno = PEACCES;
                                ret_stat = NUF_ACCES;
                            }
                            break;                      /* Next file is not found. */
                        }
                        /* Be sure it is not the root. Since the root is an abstraction 
                            we can not delete it */
                        else if (pc_isroot(pobj))
                        {
                            fs_user->p_errno = PEACCES;
                            ret_stat = NUF_ACCES;
                        }
                        else if (pobj->finode->opencount > 1)
                        {
                            fs_user->p_errno = PESHARE;
                            ret_stat = NUF_SHARE;
                        }
                        else
                        {
                            /* Delete an inode.  */
                            ret_stat = pc_rmnode(pobj);
                            if (ret_stat != NU_SUCCESS)
                            {
                                fs_user->p_errno = PENOSPC;
                                break;
                            }
                        }
                    } /* End while wild card */
                }

                /* Free the current object. */
                if (pobj)
                    pc_freeobj(pobj);

                /* Release exclusive use of finode. */
                PC_INODE_EXIT(parent_obj->finode)
                /* Free the parent object. */
                pc_freeobj(parent_obj);
            }
        }

        /* Release non-excl use of drive. */
        PC_DRIVE_EXIT(driveno)
    }

    /* Restore the kernel state */
    PC_FS_EXIT()

    return(ret_stat);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       NU_Remove_Dir                                                   
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Delete the directory specified in name. Fail if name is not a   
*       directory, directory is read only, or contains more than the       
*       entries "." and ".."                                                        
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *name                               Directory name to     
*                                            be deleted.                          
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       NU_SUCCESS                          If the directory was        
*                                            successfully removed.      
*       NUF_BAD_USER                        Not a file user.            
*       NUF_BADDRIVE                        Invalid drive specified.    
*       NUF_BADPARM                         Invalid parameter specified.
*       NUF_NOT_OPENED                      The disk is not opened yet. 
*       NUF_NO_DISK                         Disk is removed.            
*       NUF_INVNAME                         Path or filename includes   
*                                            invalid character.         
*       NUF_NOFILE                          The specified file not found.
*       NUF_ACCES                           This file has at least one  
*                                            of the following attributes:
*                                            RDONLY,HIDDEN,SYSTEM,VOLUME
*       NUF_SHARE                           The access conflict from    
*                                            multiple task to a specific
*                                            file.                      
*       NUF_NO_FINODE                       No FINODE buffer available. 
*       NUF_NO_DROBJ                        No DROBJ buffer available.  
*       NUF_IO_ERROR                        Driver I/O error.            
*       NUF_INTERNAL                        Nucleus FILE internal error.
*                                                                       
*       fs_user->p_errno is set to one of these values                  
*                                                                       
*       PENOENT                             Directory not found or path 
*                                            to file not found.         
*       PEACCES                             Not a directory, not empty  
*                                            or in use.                 
*       PENOSPC                             Write failed.               
*                                                                       
*************************************************************************/
STATUS NU_Remove_Dir(CHAR *name)
{
STATUS      ret_stat;
DROBJ       *parent_obj;
DROBJ       *pobj;
DROBJ       *pchild;
VOID        *path;
VOID        *filename;
VOID        *fileext;
INT16       driveno;


    /* Must be last line in declarations */
    PC_FS_ENTER()
    /* Check if a valid user if multitasking */
    CHECK_USER(INT, NUF_BAD_USER)

    /* Start by assuming success. */
    ret_stat = NU_SUCCESS;
    fs_user->p_errno = 0;

    parent_obj = NU_NULL;
    pchild = NU_NULL;
    pobj = NU_NULL;
    fs_user->p_errno = 0;

    /* Get the drive number. */
    if (pc_parsedrive(&driveno, (UINT8 *)name))
    {
        /* Check drive exist ( if there is check driver service ). */
        if (pc_bdevsw[driveno].dskchk_proc)
        {
            if ( !pc_bdevsw[driveno].dskchk_proc(driveno) )
    {
                ret_stat = NUF_NO_DISK;
            }
    }
    
        if (ret_stat == NU_SUCCESS)
        {
    /* Get out the filename and d:parent */
            ret_stat = 
                pc_parsepath(&path, &filename, &fileext, (UINT8 *)name);
        }
    }
    else
    {
        ret_stat = NUF_BADDRIVE;
    }

    if (ret_stat != NU_SUCCESS)
    {
        fs_user->p_errno = PENOENT;
    }
    else
    {
        /* Grab exclusive access to the drive. */
        PC_DRIVE_ENTER(driveno, YES)

        /* Find the parent and make sure it is a directory */
        ret_stat = pc_fndnode(driveno, &parent_obj, (UINT8 *)path);
        if (ret_stat == NU_SUCCESS)
        {
            if ( !pc_isadir(parent_obj) || pc_isavol(parent_obj) )
            {
                fs_user->p_errno = PENOENT;
                ret_stat = NUF_NOFILE;
            }
            /* Check the use of wild cards.
                Wild card is not available on remove directory service */
            else if (pc_use_wdcard((UINT8 *)filename))
            {
                fs_user->p_errno = PEINVAL;
                ret_stat = NUF_BADPARM;
	            trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Remove_Dir - NUF_BADPARM line %d.\n",__LINE__);
            }
            /* Check the "." or ".." */
            else if ( (pc_isdot((UINT8 *)filename, (UINT8 *)fileext)) ||
                      (pc_isdotdot((UINT8 *)filename, (UINT8 *)fileext)) )
            {
                fs_user->p_errno = PENOENT;
                ret_stat = NUF_ACCES;
            }

            if (ret_stat == NU_SUCCESS)
            {
                /* Lock the parent finode. */
                PC_INODE_ENTER(parent_obj->finode, YES)

                /* Find the file */
                pobj = NU_NULL;
                ret_stat = pc_get_inode(&pobj, parent_obj, (UINT8 *)filename);
                if (ret_stat != NU_SUCCESS)
                {
                    fs_user->p_errno = PENOENT;
                }
                else
                {
                    if (!pc_isadir(pobj))
                    {
                        fs_user->p_errno = PEACCES;
                        ret_stat = NUF_ACCES;
                    }
                    else if ( pobj->finode->fattribute & (ARDONLY | AHIDDEN | ASYSTEM | AVOLUME) )
                    {
                        fs_user->p_errno = PEACCES;
                        ret_stat = NUF_ACCES;
                    }
                    else if (pobj->finode->opencount > 1)
                    {
                        fs_user->p_errno = PESHARE;
                        ret_stat = NUF_SHARE;
                    }
                    else
                    {
                        /* Search through the directory. look at all files */
                        /* Any file that is not '.' or '..' is a problem */
                        /* Call pc_get_inode with NULL to give us an obj */
                        pchild = NU_NULL;
                        while(1)
                        {
                            /* Call pc_get_inode with NU_NULL to give us 
                               an obj. */
                            ret_stat = pc_get_inode(&pchild, pobj, (UINT8 *)"*");
                            if (ret_stat == NU_SUCCESS)
                            {
                                if ( (!(pc_isdot(pchild->finode->fname, pchild->finode->fext) ) ) &&
                                     (!(pc_isdotdot(pchild->finode->fname, pchild->finode->fext))) )
                                {
                                    fs_user->p_errno = PEACCES;
                                    ret_stat = NUF_NOEMPTY;
                                    break;
                                }   
                            }
                            else
                                break;
                        }
                        /* Make sure this directory has no file. */
                        if (ret_stat == NUF_NOFILE)
                        {
                            /* Delete an inode.  */
                            ret_stat = pc_rmnode(pobj);
                            if (ret_stat != NU_SUCCESS)
                            {
                                fs_user->p_errno = PENOSPC;
                            }
                        }
                        /* Free the child object. */
                        pc_freeobj(pchild);
                    }
                    /* Free the current object. */
                    pc_freeobj(pobj);
                }
            }

            /* Release exclusive use of finode. */
            PC_INODE_EXIT(parent_obj->finode)
            /* Free the parent object. */
            pc_freeobj(parent_obj);
        }

        /* Release non-exclusive use of drive. */
        PC_DRIVE_EXIT(driveno)
    }

    /* Restore the kernel state */
    PC_FS_EXIT()

    return(ret_stat);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_fat_size                                                     
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Calculate a disk's FAT size based on input parameters.           
*       Given a drive description, including number of reserved sectors  
*       (this includes block 0), number of root directory entries (must
*       be an even multiple of 16), cluster size, and number of fat     
*       copies, this routine calculates the size in blocks of  
*       one copy of the FAT. This routine may be used to format volumes 
*       without experimenting to find the best possible size for the FAT.
*                                                                       
*       Note: cluster_size must be 1 or a multiple of 2 (1 2 4 8 16 ..).
*             Root_entries must be a multiple of 16. nfat_copies should 
*             be 1 or 2. Use one on a ramdisk since redundancy is       
*             useless.                                                  
*                                                                       
*       Note: This algorithm is not perfect. When the number of clusters
*             is slightly greater than 4087 the decision about whether 3
*             or four nibble FAT entries could be made wrong. I added   
*             some code to try to resolve it but you are best off       
*             keeping total clusters less than 4087 or greater than     
*             say.. 4200.                                               
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       bytepsec                            Bytes per sectors           
*       nreserved                           Reserved sectors before the 
*                                            FAT                        
*       cluster_size                        Sectors per cluster         
*       n_fat_copies                        Number of FATs              
*       root_entries                        Root dir entries            
*       volume_size                         Volume size                 
*       fasize                              FAT size                    
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       Returns the size in blocks of the FAT.                          
*                                                                       
*************************************************************************/
UINT32 pc_fat_size(UINT16 bytepsec, UINT16 nreserved, UINT16 cluster_size, 
                    UINT16 n_fat_copies, UINT16 root_entries, UINT32 volume_size, UINT16 fasize)
{
UINT32      fat_size;
UINT32      fat_size_w;
UINT32      total_clusters;
UINT16      root_sectors;
UINT16      btemp;


    /* Calculate root directory using sectors
        Note: root_entries must be an even multiple of INOPBLOCK (16).
              FAT32 is always zero. Root directory into data area. */
    if (fasize <= 4)
        root_sectors = (UINT16) ((root_entries + INOPBLOCK - 1) / INOPBLOCK);
    else
        root_sectors = 0;

    /* Calculate total cluster size. Assuming zero size FAT:
       We round up to the nearest cluster boundary */
    total_clusters = (UINT32) (volume_size - nreserved - root_sectors);
    total_clusters /= cluster_size;

    /* Calculate the number of FAT entries per block in the FAT. If
       < 4087 clusters total, the FAT entries are 12 bits. Hence 341 
       will fit; else 256 will fit.
       We add in n_fat_copies * 12 here, since it takes 12 blocks to represent
       4087 clusters in 3 nibble form. So we add in the worst case FAT size
       here to enhance the accuracy of our guess of the total clusters.
    */
    btemp = (bytepsec * 8);

    fat_size = (((2 + total_clusters) * (fasize * 4)) + btemp - 1) / btemp;

    /* Recast accounts FAT size */
    total_clusters = (UINT32) (volume_size - nreserved - root_sectors);
    total_clusters -= (n_fat_copies * fat_size);
    total_clusters /= cluster_size;

    fat_size_w = (((2 + total_clusters) * (fasize * 4)) + btemp - 1) / btemp;
    fat_size = (fat_size + fat_size_w) / 2;

       return(fat_size);    
   }


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       NU_Format                                                       
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Given a drive number and a format parameter block, put a FAT   
*       file system on the drive.                                       
*       The disk MUST already have a low level format. All blocks on the
*       drive should be intitialize with E5's or zeros.                 
*                                                                       
*       Some common parameters. Note: For other drive types, use debug to
*       get the parameters from block zero after FORMAT has been run.   
*                                                                       
*       Note: If NU_Format is called with secpfat == zero, secpfat will be
*             calculated internally.                                    
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       driveno                             Drive number               
*       pfmt                                Format paramters            
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       NU_SUCCESS                          If the filesystem disk was  
*                                            successfully initialized.  
*       NUF_BAD_USER                        Not a file user.            
*       NUF_BADDRIVE                        Invalid drive specified.    
*       NUF_NOT_OPENED                      The disk is not opened yet. 
*       NUF_NO_DISK                         Disk is removed.            
*       NUF_FATCORE                         FAT cache table too small.  
*       NUF_BADPARM                         Invalid parameter specified.
*       NUF_BADDISK                         Bad Disk.                   
*       NUF_NO_PARTITION                    No partition in disk.       
*       NUF_NOFAT                           No FAT type in this         
*                                            partition.                 
*       NUF_FMTCSIZE                        Too many clusters for this  
*                                            partition.                 
*       NUF_FMTFSIZE                        File allocation table too   
*                                            small.                     
*       NUF_FMTRSIZE                        Numroot must be an even     
*                                            multiple of 16.            
*       NUF_INVNAME                         Volume lable includes       
*                                            invalid character.         
*       NUF_NO_MEMORY                       Can't allocate internal     
*                                            buffer.                    
*       NUF_NO_BLOCK                        No block buffer available.  
*       NUF_IO_ERROR                        Driver I/O error.            
*       NUF_INTERNAL                        Nucleus FILE internal error.
*                                                                       
*************************************************************************/
STATUS NU_Format(INT16 driveno, FMTPARMS *pfmt)
{
STATUS      ret_stat;
DDRIVE      *pdr;
DROBJ       *pcwd;
DROBJ       *pobj;
DATESTR     crdate;
static UINT8 *b = (UINT8 *)0;
UINT32      ltotsecs;
UINT32      fatsecs;
UINT32      nclusters;
UINT32      ldata_area;
INT16       fausize;
UINT32      clno;
UINT32      blockno;
UINT16      wvalue;
UINT32      lvalue;
UINT32      ltemp;
UINT16      i, j;
UINT8       volabel[12];
UINT32      bad_cluster;

    /* Must be last line in declarations */
    PC_FS_ENTER()
    /* Check if a valid user if multitasking */
    CHECK_USER(INT, NUF_BAD_USER)

    if( !b )
	{
       b = (UINT8 *)mem_nc_malloc(512);
	   if ( !b )
	   {
	      trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Format - NUF_NO_MEMORY line %d.\n",__LINE__);
          return(NUF_NO_MEMORY);
	   }
    }

    /* Start by assuming success. */
    ret_stat = NU_SUCCESS;

    /* Drive check */
    if ( (driveno < 0) || (driveno >= NDRIVES) )
    {
        ret_stat = NUF_BADDRIVE;
    }
    else
    {
        /* Check drive exist ( if there is check driver service ). */
        if (pc_bdevsw[driveno].dskchk_proc)
        {
            if ( !pc_bdevsw[driveno].dskchk_proc(driveno) )
            {
                ret_stat = NUF_NO_DISK;
            }
        }
    }

    if (ret_stat == NU_SUCCESS)
    {
    if (!pfmt->secpalloc)
    {
        ret_stat = NUF_BADPARM;
	trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Format: NUF_BADPARM line %d.\n",__LINE__);
    }

    /* The number of bytes per sector. */
        else if (pfmt->bytepsec != 512)
    {
        ret_stat = NUF_BADPARM;
	trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Format: NUF_BADPARM line %d.\n",__LINE__);
    }
    else
    {

    /* Set OEM name and Volume lable 
        default name:
            OEM name is NFILE2.0        Nucleus File Version 2.X
            Volume lable is NO NAME     */
            if (pfmt->oemname[0] == '\0')
                pc_cppad((UINT8 *)pfmt->oemname, (UINT8 *)"NFILE2.0", 8);

    pc_memfill(&volabel[0], 12, '\0');
    if (pfmt->text_volume_label[0] == '\0')
    {
        pc_cppad((UINT8 *)pfmt->text_volume_label, (UINT8 *)"NO NAME    ", 11);
    }
    else
    {
                /* Check the Volume lable */
        pfmt->text_volume_label[11] = '\0';
        if (!pc_checkpath((UINT8 *)pfmt->text_volume_label, YES))
        {
            ret_stat = NUF_INVNAME;
	    trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Format: NUF_INVNAME line %d.\n",__LINE__);
        }

        for (i = 0; i < 11; i++)
        {
            if ( (pfmt->text_volume_label[i] >= 'a') && 
                 (pfmt->text_volume_label[i] <= 'z') )
            {
                volabel[i] = (UINT8)('A' + pfmt->text_volume_label[i] - 'a');
            }
            else
            {
                volabel[i] = pfmt->text_volume_label[i];
            }
        }
        pc_cppad((UINT8 *)pfmt->text_volume_label, (UINT8 *)volabel, 11);
    }

            /* Check that NU_Open_Disk was called */
            pdr = (DDRIVE *)NUF_Drive_Pointers[driveno];
            if (!pdr)
            {
                ret_stat = NUF_NOT_OPENED;
            }

        }
    }

    if (ret_stat != NU_SUCCESS)
    {
        /* Restore the kernel state. */
        PC_FS_EXIT()

        return(ret_stat);
    }

    /* Create MBR partition table & write to disk absolute sector 0. */
    create_primary_partition(driveno, b); // DAVEM
    PC_DRIVE_IO_ENTER(driveno)          /* Grab the device driver */
    if ( !pc_bdevsw[driveno].io_proc(driveno, (UINT32) 0L, &(b[0]), (UINT16) 1, NO) )
    {
        pc_report_error(PCERR_FMTWMBR);
        PC_DRIVE_IO_EXIT(driveno)
        PC_DRIVE_EXIT(driveno)
        /* Restore the kernel state */
        PC_FS_EXIT()
	    trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Format: NUF_IO_ERROR line %d.\n",__LINE__);
        return(NUF_IO_ERROR);
    }

    /* Release the drive I/O locks. */
    PC_DRIVE_IO_EXIT(driveno)


    /* Grab exclusive access to the drive. */
    PC_DRIVE_ENTER(driveno, YES)

    /* =================================================================
        Build up a Partition Boot Record(PBR) 
            Note: PBR include a BIOS Parameter Block(BPB)
       =================================================================== */

    /* ============== Common paramter ============== */
    pc_memfill(&b[0], 512, '\0');
    b[0] = (UINT8) 0xE9;    /* Jump vector. Used to id MS-DOS disk */
    b[1] = (UINT8) 0x00;
    b[2] = (UINT8) 0x00;

    /* Copy the OEM name */
    pc_cppad(&b[3], (UINT8 *)pfmt->oemname, 8);

    /* The number of bytes per sector. */
    SWAP16((UINT16 *)&(b[0xb]), (UINT16 *) &pfmt->bytepsec);

    /* The number of sectors per cluster. */
    b[0xd] = pfmt->secpalloc;

    /* The number of File Allocation Tables. */
    b[0x10] = pfmt->numfats;

    /* The media descriptor. Values in this field are identical to standard BPB. */
    b[0x15] = pfmt->mediadesc;

    /* The number of sectors per track. */
    SWAP16((UINT16 *)&(b[0x18]), (UINT16 *)&pfmt->secptrk);

    /* The number of read/write heads on the drive. */
    SWAP16((UINT16 *)&(b[0x1a]), (UINT16 *)&pfmt->numhead);

    /* The number of hidden sectors on the drive. */
    SWAP16((UINT16 *)&(b[0x1c]), (UINT16 *)&pfmt->secptrk);


    /* Calculate total sectors in the volume. */

    /* Set totsecs to 0 if size > 64k. This triggers sensing huge 4.0 
       partitions. */
    if (pfmt->partdisk)
    {
        ltotsecs = pfmt->totalsec - pfmt->secptrk;
    }
    else
    {
        ltotsecs = pfmt->numcyl;
        ltotsecs *= pfmt->secptrk;
        ltotsecs *= pfmt->numhead;
        ltotsecs -= pfmt->secptrk;
    }

    /* Now fill in 4.0 specific section of the boot block */
    if (ltotsecs > 0xffffL)
    {
        /* HUGE partition  the 3.xx totsecs field is zeroed */
        b[0x13] = 0;
        b[0x14] = 0;

        /* HUGE partition */
        SWAP32((UINT32 *)&(b[0x20]), (UINT32 *)&ltotsecs);
	    /* trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Format: pfmt->totalsec=%d ltotsecs=%d line %d.\n",pfmt->totalsec,ltotsecs,__LINE__); */

    }
    else
    {
        wvalue = (UINT16) ltotsecs;
        SWAP16((UINT16 *)&(b[0x13]), &wvalue);
        b[0x20] = 0;
        b[0x21] = 0;
        b[0x22] = 0;
        b[0x23] = 0;
    }


    /* File System Type. */
    switch (NUF_Fat_Type[driveno])
    {
    case 0x01:
    {
        pc_cppad(&(b[0x36]), (UINT8 *)"FAT12   ", 8);
        fausize = 3;
        break;
    }
    case 0x04:
    case 0x06:
    case 0x0E:
    {
        pc_cppad(&(b[0x36]), (UINT8 *)"FAT16   ", 8);
        fausize = 4;
        break;
    }
    case 0x0B:
    case 0x0C:
    {
        pc_cppad(&(b[0x52]), (UINT8 *)"FAT32   ", 8);
        fausize = 8;
        break;
    }
    case NUF_NO_PARTITION:
    {
        /* Release non-excl use of drive */
        PC_DRIVE_EXIT(driveno)
        /* Restore the kernel state */
        PC_FS_EXIT()
	    trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Format: NO_PARTITION line %d.\n",__LINE__);
        return(NUF_NO_PARTITION);
    }
    default:
    {
        /* On trial calculate the clusters */
        ltemp = (UINT32) (1 + (ltotsecs)/pfmt->secpalloc);

        /* Not set the NUF_Fat_Type */
        if (ltemp < 0x0fff)
        {
            pc_cppad(&(b[0x36]), (UINT8 *)"FAT12   ", 8);
            fausize = 3;
        }
        else if (ltemp < 0xffff)
        {
            pc_cppad(&(b[0x36]), (UINT8 *)"FAT16   ", 8);
            fausize = 4;
        }
        else
        {
            pc_report_error(PCERR_NOFAT);
            /* Release non-excl use of drive */
            PC_DRIVE_EXIT(driveno)
            /* Restore the kernel state */
            PC_FS_EXIT()
	        trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Format: NUF_NOFAT line %d.\n",__LINE__);
            return(NUF_NOFAT);
        }
    }
    }

    /* ============== FAT16/FAT12 parameter ============== */
    if (fausize <= 4)
    {
        /* The number of reserved sectors, beginning with sector 0. 
            Note: Main FAT start sector number. There is always 1 sector. */
        SWAP16((UINT16 *)&(b[0xe]), (UINT16 *)&pfmt->secreserved);

        /* The number of dirents in root */
        SWAP16((UINT16 *)&(b[0x11]), (UINT16 *)&pfmt->numroot);

        /* Boot Drive Information
            80h     Boot Drive.
            00h     Not boot drive */
        b[0x24] = pfmt->physical_drive_no;

        /* Extended boot signature */
        b[0x26] = 0x29;

        /* Volume ID or Serial Number */
        SWAP32((UINT32 *)&(b[0x27]), (UINT32 *)&pfmt->binary_volume_label);

        /* Volume Label */
        pc_cppad(&(b[0x2b]), (UINT8 *)pfmt->text_volume_label, 11);
    }

    /* ============== Only FAT32 parameter ============== */
    /* FAT32
        FAT32 BPB is an extended version of the FAT16/FAT12 BPB.
        Note: 
         offset Description
          11h   Number of dirents in root. This field is ignored on FAT32 drives. 
          28h   Activate FAT.   Always this value set zero.
                Bit 15-08   Reserved
                       07   Mask indicating FAT mirroring state. 0:enable 1:disabled
                    06-04   Reserved
                    03-00   The number of Activate FAT.
          2Ah   File System Version.    Always set this value to zero. */
    else
    {
        /* The number of reserved sectors, beginning with sector 0. 
            Note: Main FAT start sector number. */
        SWAP16((UINT16 *)&(b[0xe]), (UINT16 *)&pfmt->secreserved);

        /* The cluster number of the root directory first cluster. */
        b[0x2c] = 0x02;           /* Always 02 cluster. */

        /* The sector number of the FSINFO(File System INFOrmation) sector.*/
        b[0x30] = 0x01;           /* Always 01 sector. */

        /* The sector number of the backup boot sector. */
        b[0x32] = 0x06;           /* Always 06 sector */

        /* Boot Drive Information
            80h     Boot Drive.
            00h     Not boot drive */
        b[0x40] = pfmt->physical_drive_no;

        /* Extended Boot Signature */
        b[0x42] = 0x29;

        /* Volume ID or Serial Number */
        SWAP32((UINT32 *)&(b[0x43]), (UINT32 *)&pfmt->binary_volume_label);

        /* Volume Label */
        pc_cppad(&(b[0x47]), (UINT8 *)pfmt->text_volume_label, 11);

    }

    /* if secpfat was not provided calculate it here */
    fatsecs = pc_fat_size(pfmt->bytepsec, pfmt->secreserved, pfmt->secpalloc,
                          pfmt->numfats, pfmt->numroot, ltotsecs, fausize);

    /* Sectors per fat */
    if (fausize <= 4)
    {
        wvalue=  (UINT16)fatsecs;
        SWAP16((UINT16 *)&(b[0x16]), &wvalue);
    }
    else
        SWAP32((UINT32 *)&(b[0x24]), &fatsecs);

    /* Signature word */
    wvalue = 0xAA55;
    SWAP16((UINT16 *)&(b[0x1fe]), &wvalue);
    /* ============== End of PBR set parameter ============== */


    /* Count the size of the area managed by the FAT. */
    /* Calculate max cluster number */
    ldata_area = ltotsecs;
	/*trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Format: ldata_area=%d, line %d.\n",ldata_area,__LINE__); */
    ldata_area -= pfmt->numfats * fatsecs;
    ldata_area -= pfmt->secreserved;
	/* trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Format: ldata_area=%d, line %d.\n",ldata_area,__LINE__);*/

    /* Note: numroot must be an even multiple of INOPBLOCK.
             FAT32 format Root directory entries(pfmt->numroot) are ignored. */
    if (fausize <= 4)
        ldata_area -= pfmt->numroot/INOPBLOCK;

    /* Nibbles/fat entry if < 4087 clusters then 12 bit, else 16 */
    ltemp =  ldata_area/pfmt->secpalloc;
	/*trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Format: lnclusters=%d, ldata_area=%d, line %d.\n",lnclusters,ldata_area,__LINE__);*/

    if (fausize == 3)       /* FAT12 */
    {
        if (ltemp > 0x0ff5L)
        {
            pc_report_error(PCERR_FMTCSIZE);
            /* Release non-excl use of drive */
            PC_DRIVE_EXIT(driveno)
            /* Restore the kernel state */
            PC_FS_EXIT()

            return(NUF_FMTCSIZE);
        }
        else
        {
            nclusters = (UINT16) ltemp;
        }
    }
    else if (fausize == 4)      /* FAT16 */
    {
        if (ltemp > 0xfff5L)
        {
            pc_report_error(PCERR_FMTCSIZE);
            /* Release non-excl use of drive */
            PC_DRIVE_EXIT(driveno)
            /* Restore the kernel state */
            PC_FS_EXIT()

            return(NUF_FMTCSIZE);
        }
        else
        {
            nclusters = (UINT16) ltemp;
        }
    }
    else    /* FAT32 */
    {
        if (ltemp > 0x0ffffff5L)
        {
            pc_report_error(PCERR_FMTCSIZE);
            /* Release non-excl use of drive */
            PC_DRIVE_EXIT(driveno)
            /* Restore the kernel state */
            PC_FS_EXIT()
	        trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Format: NUF_FMTCSIZE line %d.\n",__LINE__);
            return(NUF_FMTCSIZE);
        }
        else
        {
            nclusters = ltemp;
        }
    }

	/* trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Format: nclusters=%d, line %d.\n",nclusters,__LINE__); */

    /* Check the FAT. 
    if ( (nibbles needed) > (nibbles if fatblocks)
            trouble;
    */
    {
    UINT32 ltotnibbles;
    UINT32 lnibsinfatbls;

        /* Total nibbles = (# clusters * nibbles/cluster) */
        ltotnibbles = nclusters;
        ltotnibbles *= fausize;

        /* How many nibbles are available. */
        lnibsinfatbls = fatsecs;

        lnibsinfatbls <<= 10;            /* 1024 nibbles/block */

        if (ltotnibbles > lnibsinfatbls)
        {
            pc_report_error(PCERR_FMTFSIZE);
            /* Release non-excl use of drive */
            PC_DRIVE_EXIT(driveno)
            /* Restore the kernel state */
            PC_FS_EXIT()
	         trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Format: NUF_FMTCSIZE line %d.\n",__LINE__);
            return(NUF_FMTFSIZE);
        }
    }

    /* Check the root directory entries. */
    if (pfmt->numroot % INOPBLOCK)
    {
        pc_report_error(PCERR_FMTRSIZE);
        /* Release non-excl use of drive */
        PC_DRIVE_EXIT(driveno)
        /* Restore the kernel state */
        PC_FS_EXIT()
	    trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Format: NUF_FMTRSIZE line %d.\n",__LINE__);
        return(NUF_FMTRSIZE);
    }

    /* Grab the device driver */
    PC_DRIVE_IO_ENTER(driveno)
    /* ============== Update the boot sector(PBR) ============== */
    if ( !pc_bdevsw[driveno].io_proc(driveno, (UINT32) pfmt->secptrk , &(b[0]), (UINT16) 1, NO) )
    {
        pc_report_error(PCERR_FMTWPBR);
        /* Release the drive io locks */
        PC_DRIVE_IO_EXIT(driveno)
        /* Release non-excl use of drive */
        PC_DRIVE_EXIT(driveno)
        /* Restore the kernel state */
        PC_FS_EXIT()
	    trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Format: NUF_IO_ERROR line %d.\n",__LINE__);
        return(NUF_IO_ERROR);
    }

    /* Update the backup boot sector and FSINFO(File System INFOrmation).
       Only the FAT32 file system. */
    if (fausize == 8)       /* FAT32 */
    {
        /*trace_new( TRACE_ATA|TRACE_LEVEL_ALWAYS,"NU_Format: Update the backup boot sector and FSINFO(File System INFOrmation)\n");*/
        /* ============== Update the backup boot sector ============== */
        if ( !pc_bdevsw[driveno].io_proc(driveno, (UINT32)(pfmt->secptrk + 6), &(b[0]), (UINT16) 1, NO) )
        {
            pc_report_error(PCERR_FMTWPBR);
            /* Release the drive io locks */
            PC_DRIVE_IO_EXIT(driveno)
            /* Release non-excl use of drive */
            PC_DRIVE_EXIT(driveno)
            /* Restore the kernel state */
            PC_FS_EXIT()

            return(NUF_IO_ERROR);
        }

        /* ===================================================================
            Build up a Partition FSINFO(File System INFOrmation)
                Note: PBR include a BIOS Parameter Block(BPB)
           =================================================================== */
        pc_memfill(&b[0], 512, '\0');

        /* FSI signature offset 0 */
        pc_cppad(&b[0], (UINT8 *)"RRaA", 4);

        /* FSI signature offset 0x1E0 */
        pc_cppad(&b[0x1e4], (UINT8 *)"rrAa", 4);

        /* The cluster number of the cluster that was most recently allocated.
            Note: Total data clusters =
            data area total sectors / sector per cluster - 1(root directory clusters) */
        ltemp = nclusters - 1L;        /* root directory cluster */
        SWAP32((UINT32 *)&b[0x1e8], (UINT32 *)&ltemp);

        /* The count of free clusters on the drive. 
            Always 02 cluster. This cluster is root directory's first cluster.
            0xFFFFFFFF  when the count is unknown.*/
        ltemp = 2L;
        SWAP32((UINT32 *)&b[0x1ec], (UINT32 *)&ltemp);

        /* Signature word */
        wvalue = 0xAA55;
        SWAP16((UINT16 *)&(b[0x1fe]), &wvalue);

        /* ============== Update FSINFO ============== */
        /* Update
            first   FSINFO(File System INFOrmation)     offset 1 sector.
            next    Backup FSINFO.                      offset 7 sector. */
        ltemp = (UINT32)(pfmt->secptrk + 1);
        for (i = 0; i < 2; i++)
        {
            if ( !pc_bdevsw[driveno].io_proc(driveno, ltemp, &(b[0]), (UINT16) 1, NO) )
            {
                pc_report_error(PCERR_FMTWFSINFO);
                /* Release the drive io locks */
                PC_DRIVE_IO_EXIT(driveno)
                /* Release non-excl use of drive */
                PC_DRIVE_EXIT(driveno)
                /* Restore the kernel state */
                PC_FS_EXIT()

                return(NUF_IO_ERROR);
            }
            ltemp += 6L;
        }
    }

    /* Find the drive. */
    pdr = (DDRIVE *)NUF_Drive_Pointers[driveno];

    /* Reset FAT cache buffer */
    if (pdr->secpfat != fatsecs)
    {
        /* Deallocate FAT cache buffer */
        if( pdr->fat_swap_structure.data_array != NU_NULL )
            NU_Deallocate_Memory(pdr->fat_swap_structure.data_array);

        /* Remember how many blocks we alloced */
        pdr->fat_swap_structure.n_blocks_total = NUF_Drive_Fat_Size[driveno];

        if (pdr->fat_swap_structure.n_blocks_total >  (INT)fatsecs)
        {
            pdr->fat_swap_structure.n_blocks_total = (INT)fatsecs;
        }

        /* Allocate FAT cache buffer */
        pdr->fat_swap_structure.data_array = (UINT8 FAR *)
            NUF_Alloc(pdr->fat_swap_structure.n_blocks_total << 9 );

        if (!pdr->fat_swap_structure.data_array)
        {
            pdr->opencount = 0;

            /* Release the drive io locks */
            PC_DRIVE_IO_EXIT(driveno)
            /* Release non-excl use of drive */
            PC_DRIVE_EXIT(driveno)
            /* Restore the kernel state */
            PC_FS_EXIT()

            return(NUF_NO_MEMORY);
        }

        /* Set FAT buffer flag */
        if (pdr->fat_swap_structure.n_blocks_total == (INT)fatsecs)
            pdr->use_fatbuf = 0;
        else
            pdr->use_fatbuf = 1;
    }

    /* Now load the structure from the buffer */
    pdr->bytspsector = (UINT16) pfmt->bytepsec;
    pdr->fatblock = pfmt->secreserved;
    pdr->numfats = pfmt->numfats;
    pdr->secpfat = fatsecs;

    /* Set the DDRIVE file system type. */
    pdr->fasize = fausize;

    /* Set driveno now becuse the drive structure is valid */  
    pdr->driveno = driveno;

    /* ============== Now write the FATs out ============== */
	trace_new( TRACE_ATA | TRACE_LEVEL_4,"NU_Format: write FATs out line %d.\n",__LINE__);
    lvalue = 0L;
    if (!pdr->opencount)
    {
        /* Not open disk */
        lvalue = 1L;
    }
    else
    {
        /* The first byte of a FAT media descriptor */
        /* Get the FAT media descriptor value */
        if ( !pc_bdevsw[driveno].io_proc(driveno, (UINT32)(pfmt->secptrk + pfmt->secreserved), &(b[0]), (UINT16) 1, YES) )
        {
            pc_report_error(PCERR_FMTWFAT);
            /* Release the drive io locks */
            PC_DRIVE_IO_EXIT(driveno)
            /* Release non-excl use of drive */
            PC_DRIVE_EXIT(driveno)
            /* Restore the kernel state */
            PC_FS_EXIT()
	        trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Format: NUF_IO_ERROR line %d.\n",__LINE__);
            return(NUF_IO_ERROR);
        }

        /* Check the disk format. */
        if ( (b[0] & (UINT8)0xf0) != (UINT8)0xf0 )
            lvalue = 1L;
    }

    /* Release the drive I/O locks. */
    PC_DRIVE_IO_EXIT(driveno)

	/*trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Format: DONE - write FATs out line %d.\n",__LINE__);*/

    lvalue = 1L;

    /* Not format disk format */
    if (lvalue)
    {
        for (i = 0; i < pfmt->numfats; i++)
        {
            pc_memfill(&b[0], 512, '\0');

            b[0] = pfmt->mediadesc;
            b[1] = (UINT8) 0xff;
            b[2] = (UINT8) 0xff;
            if (fausize == 4)
            {
                b[3] = (UINT8) 0xff;
            }
            else if (fausize == 8)
            {
                b[3] = (UINT8) 0x0f;
                ltemp = (UINT32)0x0fffffff;
                SWAP32((UINT32 *)&b[4], &ltemp);
                SWAP32((UINT32 *)&b[8], &ltemp); /* root directory */
            }

            blockno = pfmt->secptrk + pfmt->secreserved + (i * fatsecs);

            /* Grab the device driver */
            PC_DRIVE_IO_ENTER(driveno)          
	    /*trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Format: fatsecs=%d, blockno=%d.\n",fatsecs,blockno);*/
            for (j = 0; j < fatsecs; j++)
            {
                /* WRITE */
                if ( !pc_bdevsw[driveno].io_proc(driveno, blockno, &(b[0]), (UINT16) 1, NO) )
                {
                    pc_report_error(PCERR_FMTWFAT);
                    /* Release the drive I/O locks */
                    PC_DRIVE_IO_EXIT(driveno)
                    /* Release non-excl use of drive */
                    PC_DRIVE_EXIT(driveno)
                    /* Restore the kernel state */
                    PC_FS_EXIT()
	            trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Format: NUF_IO_ERROR line %d.\n",__LINE__);
                    return(NUF_IO_ERROR);
                }
                blockno += 1;

                if (j == 0)
                    pc_memfill(&b[0], 512, '\0');
            }
            /* Release the drive I/O locks. */
            PC_DRIVE_IO_EXIT(driveno)
        }
    }

    else    /* Format disk */
    {
	    /*trace_new( TRACE_ATA | TRACE_LEVEL_4,"NU_Format: Format Disk line %d.\n",__LINE__);*/

        if (pdr->fasize == 3)            /* FAT12 */
        {
            lvalue = (UINT32) (0x0f00 | pfmt->mediadesc);
        }
        else if (pdr->fasize == 4)       /* FAT16 */
        {
            lvalue = (UINT32) (0xff00 | pfmt->mediadesc);
        }
        else                        /* FAT32 */
        {
            lvalue = (UINT32) (0x0fffff00 | pfmt->mediadesc);
        }

        /* Write the FAT fat media descriptor */
        ret_stat = pc_pfaxx(pdr, (UINT32) 0, lvalue);
        if (ret_stat != NU_SUCCESS)
        {
	        trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Format: pc_pfaxx failed line %d.\n",__LINE__);
            /* Release non-excl use of drive */
            PC_DRIVE_EXIT(driveno)
            /* Restore the kernel state */
            PC_FS_EXIT()

            return(ret_stat);
        }

        ret_stat = pc_pfaxx(pdr, (UINT32) 1, (UINT32) -1);
        if (ret_stat != NU_SUCCESS)
        {
            /* Release non-excl use of drive */
            PC_DRIVE_EXIT(driveno)
            /* Restore the kernel state */
            PC_FS_EXIT()
	        trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Format: pc_pfaxx failed line %d.\n",__LINE__);
            return(ret_stat);
        }

        if (pdr->fasize == 8)
        {
            /* Get the FAT entry value */
            ret_stat = pc_faxx(pdr, (UINT32) 2, &lvalue);
            if (ret_stat != NU_SUCCESS)
            {
                /* Release non-excl use of drive */
                PC_DRIVE_EXIT(driveno)
                /* Restore the kernel state */
                PC_FS_EXIT()
	            trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Format: pc_pfaxx failed line %d.\n",__LINE__);
                return(ret_stat);
            }

            if (lvalue == 0x0ffffff7)
            {
                /* Release non-excl use of drive */
                PC_DRIVE_EXIT(driveno)
                /* Restore the kernel state */
                PC_FS_EXIT()
	            trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Format: NUF_BADDISK line %d.\n",__LINE__);
                return(NUF_BADDISK);
            }
            else
            {
                ret_stat = pc_pfaxx(pdr, (UINT32) 2, (UINT32) -1);
                if (ret_stat != NU_SUCCESS)
                {
                    /* Release non-excl use of drive */
                    PC_DRIVE_EXIT(driveno)
                    /* Restore the kernel state */
                    PC_FS_EXIT()
	                trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Format: pc_pfaxx failed line %d.\n",__LINE__);
                    return(ret_stat);
                }
            }
            clno = 3L;
        }
        else
            clno = 2L;

	    /*trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Format: check for bad cluster line %d.\n",__LINE__);*/
/*#ifdef DEBUG*/
#if 1
        for (bad_cluster = 0L; clno < nclusters; clno++)
#else
        for (; clno < nclusters; clno++)
#endif
        {
            /* Get the FAT entry value */
            ret_stat = pc_faxx(pdr, clno, &lvalue);
            if (ret_stat != NU_SUCCESS)
            {
                /* Release non-excl use of drive */
                PC_DRIVE_EXIT(driveno)
                /* Restore the kernel state */
                PC_FS_EXIT()
	            trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Format: pc_pfaxx failed line %d.\n",__LINE__);
                return(ret_stat);
            }

            /* check for a bad cluster and skip it if we see it */
            if ( (lvalue != 0x0ffffff7 && pdr->fasize == 8)  ||
                 (lvalue != 0xfff7 && pdr->fasize == 4)      ||
                 (lvalue != 0xff7 && pdr->fasize == 3) )
            {
	            trace_new( TRACE_ATA | TRACE_LEVEL_4,"NU_Format: found bad cluster line %d.\n",__LINE__);
                /* Clear the FAT entry */
                ret_stat = pc_pfaxx(pdr, clno, (UINT32) 0);
                if (ret_stat != NU_SUCCESS)
                {
                    /* Release non-excl use of drive */
                    PC_DRIVE_EXIT(driveno)
                    /* Restore the kernel state */
                    PC_FS_EXIT()
	                trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Format: pc_pfaxx failed line %d.\n",__LINE__);
                    return(ret_stat);
                }
            }
#ifdef DEBUG
            else
                bad_cluster++;

#endif
        }

        /* Flush the fat */

        /* Grab exclusive access to the FAT */
        PC_FAT_ENTER(driveno)

        /* We don't need to update of FSINFO. */
		/* NUF_FSINFO_DISABLE */
        pdr->valid_fsinfo = 0;

        /* Flush the file allocation table */
        ret_stat = pc_flushfat(pdr);
        if (ret_stat != NU_SUCCESS)
        {
            /* Release non-excl use of FAT */
            PC_FAT_EXIT(driveno)
            /* Release non-excl use of drive */
            PC_DRIVE_EXIT(driveno)
            /* Restore the kernel state */
            PC_FS_EXIT()
	        trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Format: pc_flushfat failed line %d.\n",__LINE__);
            return(ret_stat);
        }

        /* Release non-excl use of FAT */
        PC_FAT_EXIT(driveno)
    }


    /* ============== Now write the root directory ============== */
    blockno = (UINT32)(pfmt->secptrk + pfmt->secreserved) + pfmt->numfats * fatsecs;
    pc_memfill(&b[0], 512, '\0');

    /* Grab the device driver */
    PC_DRIVE_IO_ENTER(driveno)
    if (fausize <= 4)       /* FAT16/12 */
    {
        for (j = 0; j < (pfmt->numroot/INOPBLOCK); j++)
        {
            if ( !pc_bdevsw[driveno].io_proc(driveno, blockno, &(b[0]), (UINT16) 1, NO) )
            {
                pc_report_error(PCERR_FMTWROOT);
                /* Release the drive io locks */
                PC_DRIVE_IO_EXIT(driveno)
                /* Release non-excl use of drive */
                PC_DRIVE_EXIT(driveno)
                /* Restore the kernel state */
                PC_FS_EXIT()
	            trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Format: NUF_IO_ERROR line %d.\n",__LINE__);
                return(NUF_IO_ERROR);
            }
            blockno += 1L;
        }
    }
    else        /* FAT32 */
    {
	    /*trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Format: write root dir line %d.\n",__LINE__);*/
        for (j = 0; j < pfmt->secpalloc; j++)
        {
            if ( !pc_bdevsw[driveno].io_proc(driveno, blockno, &(b[0]), (UINT16) 1, NO) )
            {
                pc_report_error(PCERR_FMTWROOT);
                /* Release the drive io locks */
                PC_DRIVE_IO_EXIT(driveno)
                /* Release non-excl use of drive */
                PC_DRIVE_EXIT(driveno)
                /* Restore the kernel state */
                PC_FS_EXIT()
	            trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Format: NUF_IO_ERROR line %d.\n",__LINE__);
                return(NUF_IO_ERROR);
            }
            blockno += 1L;
        }
    }
    /* Release the drive I/O locks. */
    PC_DRIVE_IO_EXIT(driveno)

    /* Disk reset */

	/*trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Format: disk reset line %d.\n",__LINE__);*/

     /* Free the current working directory for this drive for all users */
    pc_free_all_users(driveno);
     /* Free all files, finodes & blocks associated with the drive */
    pc_free_all_fil(pdr);
    pc_free_all_i(pdr);
    pc_free_all_blk(pdr);

     /* Free fat blocks */
    if( pdr->fat_swap_structure.data_array != NU_NULL )
        NU_Deallocate_Memory(pdr->fat_swap_structure.data_array);

    pdr->fat_swap_structure.data_array = (UINT8 FAR *)0;
    NU_Deallocate_Memory(pdr->fat_swap_structure.data_map);
    NU_Deallocate_Memory(pdr->fat_swap_structure.pdirty);
    NU_Deallocate_Memory(pdr);
    NUF_Drive_Pointers[driveno] = (UNSIGNED *)0;

    /* File system initialization */
	trace_new( TRACE_ATA | TRACE_LEVEL_4,"NU_Format: init filesystem line %d.\n",__LINE__);
    ret_stat = pc_dskinit(driveno);
    if (ret_stat != NU_SUCCESS)
    {
        /* Release non-excl use of drive */
        PC_DRIVE_EXIT(driveno)
        /* Restore the kernel state */
        PC_FS_EXIT()
	    trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Format: pc_dskinit failed line %d.\n",__LINE__);
        return(ret_stat);
    }

     /* Find the drive */
    pdr = pc_drno2dr(driveno);

    /* The free cluster count is known. */
	/* NUF_FSINFO_UPDATE */
    pdr->valid_fsinfo = 2;

     /* Set the current root directory */
    pcwd = pc_get_root(pdr);
    fs_user->lcwd[driveno] = pcwd;

    /* Create Volume Label entry. */
    if (volabel[0] != '\0')
    {
	    trace_new( TRACE_ATA | TRACE_LEVEL_4,"NU_Format: create vol label line %d.\n",__LINE__);
        /* Allocate an empty DROBJ and FINODE to hold the new file */
        pobj = pc_allocobj();
        if (!pobj)
        {
            /* Release non-excl use of drive */
            PC_DRIVE_EXIT(driveno)
            /* Restore the kernel state */
            PC_FS_EXIT()
	        trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Format: pc_allocobj failed line %d.\n",__LINE__);
            return(NUF_NO_DROBJ);
        }

        /* Load the inode copy name, ext, attr, cluster, size, date, and time */
        pc_init_inode( pobj->finode, &volabel[0], &volabel[8], 
                        AVOLUME, 0L, 0L, pc_getsysdate(&crdate) );

        /* Convert pobj to native and stitch it into mom */
        ret_stat = pc_insert_inode(pobj, pcwd, &volabel[0], 0);
        if (ret_stat != NU_SUCCESS) 
        {
            /* Free the current object */
            pc_freeobj(pobj);
            /* Release non-excl use of drive */
            PC_DRIVE_EXIT(driveno)
            /* Restore the kernel state */
            PC_FS_EXIT()
	        trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Format: pc_insert_inode failed line %d.\n",__LINE__);
            return(ret_stat);
        }

        /* Free the Volume Label object */
        pc_freeobj(pobj);
    }

    /* Release non-excl use of drive */
    PC_DRIVE_EXIT(driveno)
    /* Restore the kernel state */
    PC_FS_EXIT()

    return(NU_SUCCESS);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       NU_FreeSpace                                                    
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Given a path containing a valid drive specifier count the number
*       of free clusters, sector per cluster, bytes per sector and number
*       of total clusters on the drive.                                  
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       path                                Drive character            
*       secpcluster                         Sector per cluster          
*       bytepsec                            Bytes per sector            
*       freecluster                         Number of free clusters      
*       totalcluster                        Number of total clusters     
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       NU_SUCCESS                          If service is successful.   
*                                           Returns the sector per      
*                                            cluster, bytes per sector, 
*                                            number of free clusters and
*                                            number of total clusters.  
*       NUF_BAD_USER                        Not a file user.            
*       NUF_BADDRIVE                        Invalid drive specified.    
*       NUF_NOT_OPENED                      Drive not opened.           
*       NUF_NO_DISK                         Disk is removed.            
*                                                                       
*************************************************************************/
INT NU_FreeSpace(CHAR *path, UINT8 *secpcluster, UINT16 *bytepsec, 
                   UINT32 *freecluster, UINT32 *totalcluster)
{
STATUS      ret_stat;
DDRIVE      *pdr;
INT16       driveno;


    /* Must be last line in declarations */
    PC_FS_ENTER()
    /* Check if a valid user if multitasking */
    CHECK_USER(INT, NUF_BAD_USER)

    /* Start by assuming success. */
    ret_stat = NU_SUCCESS;

    /* Get the drive number. */
    if (pc_parsedrive(&driveno, (UINT8 *)path))
    {
        /* Check drive exist ( if there is check driver service ). */
        if (pc_bdevsw[driveno].dskchk_proc)
        {
            if ( !pc_bdevsw[driveno].dskchk_proc(driveno) )
            {
                ret_stat = NUF_NO_DISK;
            }
        }

        /* Find the drive */
        pdr = (DDRIVE *)pc_drno2dr(driveno);
        if (!pdr)
        {
            ret_stat = NUF_NOT_OPENED;
        }
    }
    else
    {
        ret_stat = NUF_BADDRIVE;
    }

    if (ret_stat == NU_SUCCESS)
    {
        /* Sector per cluster */
        *secpcluster = pdr->secpalloc;

        /* Bytes per sector */
        *bytepsec = pdr->bytspsector;

        /* Number of total clusters */
        *totalcluster = pdr->maxfindex - 1;

        /* Grab exclusive access to the drive */
        PC_DRIVE_ENTER(driveno, YES)

        /* Number of free clusters */
        *freecluster = pc_ifree(driveno);

        /* Release non-excl use of drive. */
        PC_DRIVE_EXIT(driveno)
    }

    /* Restore the kernel state */
    PC_FS_EXIT()

    return(ret_stat);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       NU_Get_First                                                    
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Given a pattern which contains both a path specifier and a      
*       search pattern fill in the structure at statobj with information
*       about the file and set up internal parts of statobj to supply   
*       appropriate information for calls to NU_Get_Next.               
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *statobj                            Caller's buffer to put file 
*                                            info.                      
*       *name                               Path to find                
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       NU_SUCCESS                          Search for the first match  
*                                            pattern was successful.  
*       NUF_BAD_USER                        Not a file user.            
*       NUF_BADDRIVE                        Invalid drive specified.    
*       NUF_NOT_OPENED                      The disk is not opened yet. 
*       NUF_NO_DISK                         Disk is removed.            
*       NUF_INVNAME                         Path or filename includes   
*                                            invalid character.         
*       NUF_NOFILE                          The specified file not found.
*       NUF_NO_FINODE                       No FINODE buffer available. 
*       NUF_NO_DROBJ                        No DROBJ buffer available.  
*       NUF_IO_ERROR                        Driver I/O function routine  
*                                            returned error.            
*       NUF_INTERNAL                        Nucleus FILE internal error.
*                                                                       
*************************************************************************/
STATUS NU_Get_First(DSTAT *statobj, CHAR *pattern)
{
STATUS      ret_stat;
VOID        *mompath;
VOID        *filename;
VOID        *fileext;
INT16       driveno;


    /* Must be last line in declarations. */
    PC_FS_ENTER()
    /* Check if a valid user if multitasking. */
    CHECK_USER(INT, NUF_BAD_USER)

    /* Start by assuming success. */
    ret_stat = NU_SUCCESS;

    statobj->pobj = NU_NULL;
    statobj->pmom = NU_NULL;

    /* Get the drive number. */
    if (pc_parsedrive(&driveno, (UINT8 *)pattern))
    {
        /* Check that the drive exists (if there is check driver service). */
        if (pc_bdevsw[driveno].dskchk_proc)
        {
            if ( !pc_bdevsw[driveno].dskchk_proc(driveno) )
            {
                ret_stat = NUF_NO_DISK;
            }
        }

        if (ret_stat == NU_SUCCESS)
        {
            /* Get the path, filename and file extension. */
            ret_stat = pc_parsepath(&mompath, &filename, 
                                        &fileext, (UINT8 *)pattern);
            if (ret_stat != NU_SUCCESS)
            {
                ret_stat = NUF_INVNAME;
            }
        }
    }
    else
    {
        ret_stat = NUF_BADDRIVE;
    }

    if (ret_stat == NU_SUCCESS)
    {
        /* Register drive in use. */
        PC_DRIVE_ENTER(driveno, NO)

        /* Save the pattern. we'll need it in NU_Get_Next. */
        pc_ncpbuf(statobj->pname, filename, EMAXPATH+1);
        if (fileext)
            pc_ncpbuf(statobj->pext, fileext, 4);
        else
            statobj->pext[0] = (UINT8)0;
        if (mompath)
            /* Copy over the path. we will need it later. */
            pc_ncpbuf((UINT8 *)statobj->path, mompath, EMAXPATH+1);
        else
            statobj->path[0] = (INT8)0;

        /* Find the file and init the structure. */
        ret_stat = pc_fndnode(driveno, &statobj->pmom, (UINT8 *)mompath);
        if (ret_stat == NU_SUCCESS)
            /* Found it. Check access permissions. */
        {
            if (pc_isadir(statobj->pmom)) 
            {
                /* Lock the finode. */
                PC_INODE_ENTER(statobj->pmom->finode, NO)
                /* Now find pattern in the directory. */
                statobj->pobj = NU_NULL;
                ret_stat = pc_get_inode(&statobj->pobj, 
                                        statobj->pmom, (UINT8 *)filename);
                if (ret_stat == NU_SUCCESS)
                {
                    /* And update the stat structure. */
                    pc_upstat(statobj);
                    /* Long file name? */
                    if (statobj->pobj->linfo.lnament)
                    {
                        /* We need to clean long filename information. */
                        lnam_clean(&statobj->pobj->linfo, 
                                            statobj->pobj->pblkbuff);
                        pc_free_buf( statobj->pobj->pblkbuff, NO);
                    }
                }
                /* Release exclusive use of finode. */
                PC_INODE_EXIT(statobj->pmom->finode)
            }
            /* Find first fail, free the pmom. */
            if (ret_stat != NU_SUCCESS)
            {
                /* Free the search object. */
                pc_freeobj(statobj->pmom);
                statobj->pmom = NU_NULL;
            }
        }

        /* Release non-exclusive use of drive. */
        PC_DRIVE_EXIT(driveno)
    }

    /* Restore the kernel state. */
    PC_FS_EXIT()

    return(ret_stat);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       NU_Get_Next                                                     
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Given a pointer to a DSTAT structure that has been set up by a  
*       call to NU_Get_First(), search for the next match of the        
*       original pattern in the original path. Return yes if found and  
*       update statobj for subsequent calls to NU_Get_Next.             
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *statobj                            Caller's buffer to put file 
*                                            info.                      
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       NU_SUCCESS                          Search for the next match   
*                                            pattern was successful.  
*       NUF_BAD_USER                        Not a file user.            
*       NUF_BADPARM                         Invalid parameter specified.
*       NUF_NO_DISK                         Disk is removed.            
*       NUF_NOFILE                          The specified file not found.
*       NUF_NO_FINODE                       No FINODE buffer available. 
*       NUF_IO_ERROR                        Driver I/O function routine  
*                                            returned error.            
*       NUF_INTERNAL                        Nucleus FILE internal error.
*                                                                       
*************************************************************************/
STATUS NU_Get_Next(DSTAT *statobj)
{
STATUS      ret_stat;
INT16       driveno;


    /* Must be last line in declarations. */
    PC_FS_ENTER()
    /* Check if a valid user if multitasking. */
    CHECK_USER(INT, NUF_BAD_USER)

    /* Start by assuming success. */
    ret_stat = NU_SUCCESS;

    /* Do we get correct statobj? */
    if ( (!statobj) || (!statobj->pmom) )
    {
        /* Restore the kernel state. */
        PC_FS_EXIT()
	    trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"NU_Get_Next: NUF_BADPARM line %d.\n",__LINE__);
        return(NUF_BADPARM);
    }

    driveno = statobj->pmom->pdrive->driveno;

    /* Check that the drive exists (if there is check driver service). */
    if (pc_bdevsw[driveno].dskchk_proc)
    {
        if ( !pc_bdevsw[driveno].dskchk_proc(driveno) )
        {
            ret_stat = NUF_NO_DISK;
        }
    }

    if (ret_stat == NU_SUCCESS)
    {
        /* Register drive in use. */
        PC_DRIVE_ENTER(driveno, NO)
        /* Lock the finode. */
        PC_INODE_ENTER(statobj->pmom->finode, NO)

        /* Now find the next instance of pattern in the directory. */
        ret_stat = pc_get_inode(&statobj->pobj, 
                                    statobj->pmom, statobj->pname);
        if (ret_stat == NU_SUCCESS)
        {
            /* And update the stat structure. */
            pc_upstat(statobj);

            /* Long filename? */
            if (statobj->pobj->linfo.lnament)
            {
                /* We need to clean long filename information. */
                lnam_clean(&statobj->pobj->linfo, statobj->pobj->pblkbuff);
                pc_free_buf(statobj->pobj->pblkbuff, NO);
            }
        }

        /* Release exclusive use of finode. */
        PC_INODE_EXIT(statobj->pmom->finode)
        /* Release non-exclusive use of drive. */
        PC_DRIVE_EXIT(driveno)
    }

    /* Restore the kernel state. */
    PC_FS_EXIT()

    return(ret_stat);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       NU_Done                                                         
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Given a pointer to a DSTAT structure that has been set up by a  
*       call to NU_Get_First(), free internal elements used by the       
*       statobj.                                                        
*                                                                       
*       Note: You MUST call this function when done searching through a 
*             directory.                                                
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *statobj                            Caller's buffer to put file 
*                                            info.                      
*                                                                       
* OUTPUTS                                                               
*                                                                       
*      None.                                                            
*                                                                       
*************************************************************************/
VOID NU_Done(DSTAT *statobj)
{
INT16       driveno;


    /* Must be last line in declarations */
    PC_FS_ENTER()
    /* Check if a valid user if multitasking */
    VOID_CHECK_USER()

    if ( (statobj) && (statobj->pmom) )
    {   /* okay, release the elements */
        driveno = statobj->pmom->pdrive->driveno;

        /* Register drive in use. */
        PC_DRIVE_ENTER(driveno, NO)
        /* Free the search object. */
        if (statobj->pobj)
            pc_freeobj(statobj->pobj);
        pc_freeobj(statobj->pmom);
        /* Release non-excl use of drive. */
        PC_DRIVE_EXIT(driveno)
    }

    /* Restore the kernel state */
    PC_FS_EXIT()
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       NU_Set_Default_Drive                                            
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Use this function to set the current default drive that will be 
*       used when a path specifier does not contain a drive specifier. 
*       Note: The "default" default is zero (drive A:)                    
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       drivno                              Drive number                
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       NU_SUCCESS                          Setting the current default     
*                                            drive was successful.    
*       NUF_BAD_USER                        Not a file user.            
*       NUF_NOT_OPENED                      The disk is not opened yet. 
*       NUF_NO_DISK                         Disk is removed.            
*       NUF_INVNAME                         If the drive is out of range.
*                                                                       
*************************************************************************/
STATUS NU_Set_Default_Drive(UINT16 driveno)
{
STATUS      ret_stat;
DDRIVE      *pdr;


    /* Must be last line in declarations */
    PC_FS_ENTER()
    /* Check if a valid user if multitasking */
    CHECK_USER(INT, NUF_BAD_USER)

    /* Start by assuming success. */
    ret_stat = NU_SUCCESS;

    /* Check drive number */
    if (driveno >= NDRIVES) 
    {
        ret_stat = NUF_INVNAME;
    }
    else
    {
        /* Check drive exist ( if there is check driver service ). */
        if (pc_bdevsw[driveno].dskchk_proc)
        {
            if ( !pc_bdevsw[driveno].dskchk_proc(driveno) )
            {
                ret_stat = NUF_NO_DISK;
            }
        }
    }

    if (ret_stat == NU_SUCCESS)
    {
        /* Find the drive. */
        pdr = (DDRIVE *)pc_drno2dr(driveno);
        if (pdr)
        {
            /* Set the default drive number. */
        fs_user->dfltdrv = driveno;
        }
        else
            ret_stat = NUF_NOT_OPENED;
    }

    /* Restore the kernel state */
    PC_FS_EXIT()

    return(ret_stat);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       NU_Get_Default_Drive                                            
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Use this function to get the current default drive when a path  
*       specifier does not contain a drive specifier.                   
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       None.                                                           
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       Return the current default drive.                               
*                                                                       
*************************************************************************/
UINT16 NU_Get_Default_Drive(VOID)
{
UINT16 ret_val;
NU_SUPERV_USER_VARIABLES  

    NU_SUPERVISOR_MODE();

	ret_val = fs_user->dfltdrv;

    NU_USER_MODE();

    return(ret_val);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       NU_Set_Current_Dir                                              
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Find path. If it is a subdirectory make it the current working  
*       directory for the drive.                                        
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *name                               Set directory name          
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       NU_SUCCESS                          If the current working      
*                                            directory was changed.     
*       NUF_BAD_USER                        Not a file user.            
*       NUF_BADDRIVE                        Invalid drive specified.    
*       NUF_NOT_OPENED                      The disk is not opened yet. 
*       NUF_NO_DISK                         Disk is removed.            
*       NUF_LONGPATH                        Path or directory name too  
*                                            long.                      
*       NUF_ACCES                           Not a directory attributes. 
*       NUF_NOFILE                          The specified file not      
*                                            found.                     
*       NUF_NO_BLOCK                        No block buffer available.  
*       NUF_NO_FINODE                       No FINODE buffer available. 
*       NUF_NO_DROBJ                        No DROBJ buffer available.  
*       NUF_IO_ERROR                        Driver I/O function routine  
*                                            returned error.            
*       NUF_INTERNAL                        Nucleus FILE internal error.
*                                                                       
*       fs_user->p_errno is set to one of these values                  
*                                                                       
*       PENOENT                             File not found or path to   
*                                            file not found.            
*                                                                       
*************************************************************************/
STATUS NU_Set_Current_Dir(CHAR *path)
{
STATUS      ret_stat;
DROBJ       *pobj;
DROBJ       *parent_obj;
VOID        *path2;
VOID        *filename;
VOID        *fileext;
INT16       driveno;


    /* Must be last line in declarations */
    PC_FS_ENTER()
    /* Check if a valid user if multitasking */
    CHECK_USER(INT, NUF_BAD_USER)


    /* Check path - Wildcards not allowed */
	ret_stat = pc_use_wdcard((UINT8 *)path);
	if(ret_stat == YES)
	{
		ret_stat = NUF_INVNAME;
	}
	else
	{

        /* Start by assuming success. */
        ret_stat = NU_SUCCESS;

    parent_obj = NU_NULL;
    pobj = NU_NULL;
    fs_user->p_errno = 0;

        /* Get the drive number. */
        if (pc_parsedrive(&driveno, (UINT8 *)path))
        {
            /* Check drive exist ( if there is check driver service ). */
            if (pc_bdevsw[driveno].dskchk_proc)
            {
                if ( !pc_bdevsw[driveno].dskchk_proc(driveno) )
    {
                    ret_stat = NUF_NO_DISK;
                }
    }
    
            if (ret_stat == NU_SUCCESS)
            {
                /* Get the path, file name and file extension. */
                ret_stat = pc_parsepath(&path2, &filename, 
                                            &fileext, (UINT8 *)path);
            }
        }
        else
        {
            ret_stat = NUF_BADDRIVE;
        }

    if (ret_stat != NU_SUCCESS)
    {
        fs_user->p_errno = PENOENT;
    }
    else
    {
            /* Register drive in use. */
            PC_DRIVE_ENTER(driveno, NO)

        /* Find the parent and make sure it is a directory  */
        ret_stat = pc_fndnode(driveno, &parent_obj, (UINT8 *)path2);
        if (ret_stat != NU_SUCCESS)
        {
            fs_user->p_errno = PENOENT;
        }
        else if ( (!pc_isadir(parent_obj)) || (pc_isavol(parent_obj)) )
        {
            fs_user->p_errno = PENOENT;
            ret_stat = NUF_ACCES;
        }
        else
        {
            /* Get the directory */
            if ( filename == (VOID *)'\0' || filename == (VOID *)' ' )
            {
                pobj = parent_obj;
            }
            else
            {
                /* Lock the parent */
                PC_INODE_ENTER(parent_obj->finode, YES)
                    /* Find the file and init the structure. */
                pobj = NU_NULL;
                ret_stat = pc_get_inode(&pobj, parent_obj, (UINT8 *)filename);
                    /* Release excl use of finode. */
                PC_INODE_EXIT(parent_obj->finode)
                if (ret_stat != NU_SUCCESS)
                {
                        /* Free the parent object. */
                    pc_freeobj(parent_obj);
                    fs_user->p_errno = PENOENT;
                }
                /* If the request is cd .. then we just found the .. directory entry.  
                   We have to call get_mom to access the parent. */                   
                else if ( pc_isdotdot((UINT8 *)filename, (UINT8 *)fileext) )
                {
                        /* Free the parent object. */
                    pc_freeobj(parent_obj);
                    parent_obj = pobj;

                    /* Find parent_obj's parent. By looking back from ".." */
                    ret_stat = pc_get_mom(&pobj, parent_obj);

                        /* Free the parent object. */
                    pc_freeobj(parent_obj);
                }
                /* Specified path is not a directory */
                else if ( !pc_isadir(pobj) )
                {
                        /* Free the current and parent object. */
                    pc_freeobj(pobj);
                    pc_freeobj(parent_obj);
                    ret_stat = NUF_ACCES;
                }
                else
                {
                    /* Free the parent object */
                    pc_freeobj(parent_obj);
                }
            }
            if (ret_stat == NU_SUCCESS) /* Everything is fine. */
            {
                driveno = pobj->pdrive->driveno;
                    /* Free the current directory object. */
                if (fs_user->lcwd[driveno] != NU_NULL)
                    pc_freeobj(fs_user->lcwd[driveno]);
                fs_user->lcwd[driveno] = pobj;
            }
        }

            /* Release non-excl use of drive. */
            PC_DRIVE_EXIT(driveno)
    }

	} /* End wildcard check */

    /* Restore the kernel state */
    PC_FS_EXIT()

    return(ret_stat);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       NU_Current_Dir                                                  
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Fill in a string with the full path name of the current working 
*       directory. Return NO if something went wrong.                   
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *drive                              Drive character            
*       *path                               Current directory pointer   
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       NU_SUCCESS                          Returns the path name in    
*                                            "path".                      
*       NUF_BAD_USER                        Not a file user.            
*       NUF_BADDRIVE                        Invalid drive specified.    
*       NUF_NO_DISK                         Disk is removed.            
*       NUF_INVNAME                         Disk not opend yet.         
*                                                                       
*************************************************************************/
STATUS NU_Current_Dir(UINT8 *drive, CHAR *path)
{
STATUS      ret_stat;
DDRIVE      *pdrive;
DROBJ       *pobj;
INT16       driveno;


    /* Must be last line in declarations */
    PC_FS_ENTER()
    /* Check if a valid user if multitasking */
    CHECK_USER(INT, NUF_BAD_USER)

    /* Start by assuming success. */
    ret_stat = NU_SUCCESS;

    /* Get the drive number. */
    if (pc_parsedrive(&driveno, drive))
    {
        /* Check drive exist ( if there is check driver service ). */
        if (pc_bdevsw[driveno].dskchk_proc)
        {
            if ( !pc_bdevsw[driveno].dskchk_proc(driveno) )
    {
                ret_stat = NUF_NO_DISK;
            }
        }
    }
    else
    {
        ret_stat = NUF_BADDRIVE;
    }
    
    if (ret_stat == NU_SUCCESS)
    {
        /* Lock the drive since we work upwards in the directory tree */
        PC_DRIVE_ENTER(driveno, YES)
        /* Find the drive. */
    pdrive = pc_drno2dr(driveno);
    if (pdrive)
    {
            /* Get current directory. */
        pobj = pc_get_cwd(pdrive);
        if (pobj)
            {
                /* Get the full path name. */
                if ( !pc_l_pwd((UINT8 *)path, pobj) )
                    ret_stat = NUF_INVNAME;
    }
        }
        /* Release non-excl use of drive. */
    PC_DRIVE_EXIT(driveno)
    }

    /* Restore the kernel state */
    PC_FS_EXIT()

    return(ret_stat);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_l_pwd                                                        
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Get the full path name.                                         
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *path                               Path name buffer            
*       *pobj                               Drive object                
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       YES                                 If getting the path name    
*                                            was successful.                
*       NO                                  Following  error code:      
*        NUF_NOFILE                          The specified file not     
*                                             found.                    
*        NUF_NO_BLOCK                       No block buffer available.  
*        NUF_NO_FINODE                      No FINODE buffer available. 
*        NUF_NO_DROBJ                       No DROBJ buffer available.  
*        NUF_IO_ERROR                       Driver I/O function routine  
*                                            returned error.            
*        NUF_INTERNAL                       Nucleus FILE internal error.
*                                                                       
*************************************************************************/
static INT pc_l_pwd(UINT8 *path, DROBJ *pobj)
{
STATUS      ret_stat;
DROBJ       *pchild;
INT16       lnend;
INT16       n;
INT16       ci;
static UINT8 *lname = (UINT8 *)0;

    if( !lname )
	{
       lname = (UINT8 *)mem_nc_malloc(EMAXPATH+1);
	   if ( !lname )
	   {
	     trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"pc_l_pwd NU_NO_MEMORY line %d.\n",__LINE__);
	     return( NO );
	   }
	}

    lnend = 0;

    while (YES)
    {
        /* Root directory */
        lname[lnend] = BACKSLASH;

        /* Root directory? */
        if (pc_isroot(pobj))
        {
            /* Free the current object. */
            pc_freeobj(pobj);
            break;      /* path search end. */
        }
        else
        {
            lnend++;    /* for BACKSLASH */

            /* Find '..' so we can find the parent. */
            pchild = NU_NULL;
            ret_stat = pc_get_inode(&pchild, pobj, (UINT8 *)"..");
            if (ret_stat != NU_SUCCESS)
            {
                /* Free the current object. */
                pc_freeobj(pobj);
                return(NO);
            }

            /* Free the current object. */
            pc_freeobj(pobj);

            /* Get the parent directory. */
            ret_stat = pc_get_mom(&pobj, pchild);
            if (ret_stat != NU_SUCCESS)
            {
                /* Free the child object. */
                 pc_freeobj(pchild);
                 return(NO);
            }

            /* Get the name of the current directory by searching the 
               parent for an inode with cluster matching the cluster 
               value in ".." */
            if ( !pc_gm_name(&lname[lnend], pobj, pchild) )
            {
                /* Free the child and current object. */
                pc_freeobj(pchild);
                pc_freeobj(pobj);
                return(NO);
            }

            /* Free the child  object. */
            pc_freeobj(pchild);

            /* Measure path name length. */
            for (; *(lname + lnend); lnend++);
        }
    }

    /* Path string set up the right order. */
    *path++ = BACKSLASH;    /* first BACKSLASH is root. */
    for (n = lnend-1; n >= 0; n--)
    {
        /* Is current name is slash? */
        if (lname[n] == '\\')
        {
            for (ci = n+1; ci < lnend; ci++)
                *path++ = lname[ci];

            *path++ = BACKSLASH;
            lnend = n;  /* Set BACKSLASH index. */
        }
    }
    *path = '\0';

    return(YES);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_gm_name                                                      
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Check the directory entry links.                                
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *path                               Output path pointer         
*       *parent_obj                         Parent object               
*       *pdotdot                            ".." entry drive object     
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       YES                                 If the check path was       
*                                            successful.                
*       NO                                  Following  error code:      
*        NUF_NOFILE                          The specified file not     
*                                             found.                    
*        NUF_NO_FINODE                      No FINODE buffer available. 
*        NUF_NO_DROBJ                       No DROBJ buffer available.  
*        NUF_IO_ERROR                       Driver I/O function routine  
*                                            returned error.            
*        NUF_INTERNAL                       Nucleus FILE internal error.
*                                                                       
*************************************************************************/
static INT pc_gm_name(UINT8 *path, DROBJ *parent_obj, DROBJ *pdotdot)
{
STATUS      ret_stat;
DROBJ       *pchild;
LNAMINFO    *linfo;
DOSINODE    pi;
UINT32      clusterno;
UINT32      fcluster;
INT         ret_val;


    ret_val = NO;

    /* Convert sector to cluster. */
    clusterno = pc_sec2cluster(pdotdot->pdrive, pdotdot->blkinfo.my_frstblock);

    /* Now find pattern in the directory. */
    pchild = NU_NULL;
    ret_stat = pc_get_inode(&pchild, parent_obj, (UINT8 *)"*");
    if (ret_stat == NU_SUCCESS)
    {
        for (;;)
        {
            fcluster = pchild->finode->fcluster;
            if ( (pchild->finode->fname[0] != PCDELETE) && (fcluster == clusterno) )
            {
                /* Setup the filename */

                /* get long filename info */
                linfo = &pchild->linfo;
                if (linfo->lnament)         /* Long filename */
                {
                    /* Convert directory entry long filename to character long filename. */
                    pc_cre_longname((UINT8 *)path, linfo);
                }
                else        /* Short file name */
                {
                    pc_ino2dos(&pi, pchild->finode);
                    /* Convert directory entry short filename to character short filename. */
                    pc_cre_shortname((UINT8 *)path, pi.fname, pi.fext);
                }
                ret_val = YES;
                break;
            }

            if (pchild->linfo.lnament)
            {
                /* We need to clean long filename information */
                lnam_clean(&pchild->linfo, pchild->pblkbuff);
                pc_free_buf(pchild->pblkbuff, NO);
            }

            ret_val = pc_get_inode(&pchild, parent_obj, (UINT8 *)"*");
            if (ret_val != NU_SUCCESS)
            {
                ret_val = NO;
                break;
            }
        }
        /* Free the child  object. */
        pc_freeobj(pchild);
    }

    return(ret_val);
}


/****************************************************************************
 * Modifications:
 * $Log: 
 *  4    mpeg      1.3         4/3/04 1:20:40 AM      Nagaraja Kolur  CR(s) 
 *        8762 8763 : Merged 2.5.6 Nup File System.
 *        Test: 1. pvr demo 2. nup file test 3. CR 8181
 *        
 *  3    mpeg      1.2         1/7/04 4:14:25 PM      Tim Ross        CR(s) 
 *        8181 : Properly create MBR w/ partition table, FAT32 PBR, and FAT32 
 *        FSINFO sectors
 *        in proper locations when NU_Format is called.
 *  2    mpeg      1.1         10/15/03 4:57:09 PM    Tim White       CR(s): 
 *        7660 Remove ATA header files from NUP_FILE code.
 *        
 *  1    mpeg      1.0         8/22/03 5:35:42 PM     Dave Moore      
 * $
 ****************************************************************************/

