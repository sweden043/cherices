/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                      Conexant Systems Inc. (c) 2003                      */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       drobj.c
 *
 *
 * Description:    see ATI header below.
 *                 Initial version based on Nucleus File v2.4.4
 *                 
 *
 ****************************************************************************/
/* $Header: drobj.c, 3, 4/3/04 1:04:09 AM, Nagaraja Kolur$
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
*       DROBJ.C                                   2.5              
*                                                                       
* COMPONENT                                                             
*                                                                       
*       Nucleus FILE                                                    
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Directory object manipulation routines.                         
*                                                                       
*                                                                       
* DATA STRUCTURES                                                       
*                                                                       
*       None.                                                           
*                                                                       
* FUNCTIONS                                                             
*                                                                       
*       pc_fndnode                          Find a file or directory on 
*                                            disk and return a DROBJ.   
*       pc_get_inode                        Find a filename within a    
*                                            subdirectory.              
*       pc_next_inode                       Find a next file or         
*                                            directory on disk and      
*                                            return a DROBJ.            
*       chk_sum                             Calculating short file name 
*                                            check byte.                
*       pc_cre_longlen                      Create long-filename ascii  
*                                            strings.                   
*       pc_cre_shortfilename                Create short-filename ascii 
*                                            strings.                   
*       lnam_clean                          Clear long filname         
*                                            information structure.     
*       pc_findin                           Find a filename in the same 
*                                            directory as argument.     
*       pc_get_mom                          Find the parent inode of a  
*                                            subdirectory.              
*       pc_mkchild                          Allocate a DROBJ and fill   
*                                            it based on parent object.
*       pc_mknode                           Create an empty subdirectory
*                                            or file.                   
*       pc_insert_inode                     Called only by pc_mknode.   
*       pc_del_lname_block                  Delete a long filename      
*                                            entry.                     
*       pc_renameinode                      Rename an inode.            
*       pc_rmnode                           Delete an inode             
*                                            unconditionally.           
*       pc_update_inode                     Flush an inode to disk.     
*       pc_get_root                         Create the special ROOT     
*                                            object for a drive.        
*       pc_firstblock                       Return the absolute block   
*                                            number of a directory.     
*       pc_next_block                       Calculate the next block    
*                                            owned by an object.        
*       pc_l_next_block                     Calculate the next block in 
*                                            a chain.                   
*       pc_marki                            Set dr:sec:index, and stitches  
*                                            FINODE into the inode list.
*       pc_scani                            Search for an inode in the  
*                                            internal inode list.       
*       pc_allocobj                         Alocates and zeros the     
*                                            space needed to store a    
*                                            DROBJ structure.           
*       pc_alloci                           Allocates and zeros a      
*                                            FINODE structure.          
*       pc_free_all_i                       Release all inode buffers.  
*       pc_freei                            Release FINODE structure.   
*       pc_freeobj                          Return a drobj structure to 
*                                            the heap.                  
*       pc_dos2inode                        Take the data from pbuff    
*                                            which is a raw disk        
*                                            directory entry.           
*       pc_ino2dos                          Take in memory native format
*                                            inode information.         
*       pc_init_inode                       Take an uninitialized inode.
*       pc_isadir                           Check the root or           
*                                            subdirectory.              
*       pc_isroot                           Check the root directory.   
*                                                                       
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

extern FINODE       *inoroot;               /* Beginning of inode pool.  */


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_fndnode                                                      
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Take a full path name and traverse the path until we get to the 
*       file or subdir at the end of the path specifier. When found  
*       allocate and initialize (OPEN) a DROBJ.                                 
*                   
*                                                                       
* INPUTS                                                                
*                                                                       
*       driveno                             Drive number                
*       **pobj                              Drive object structure      
*       path                                Path name                   
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       NU_SUCCESS                          File was found.             
*       NUF_NOT_OPENED                      Drive not opened.           
*       NUF_LONGPATH                        Path or directory name too  
*                                            long.                      
*       NUF_NOFILE                          The specified file not      
*                                            found.                     
*       NUF_NO_FINODE                       No FINODE buffer available. 
*       NUF_NO_DROBJ                        No DROBJ buffer available.  
*       NUF_IO_ERROR                        Driver I/O error.            
*       NUF_INTERNAL                        Nucleus FILE internal error.
*                                                                       
*************************************************************************/
STATUS pc_fndnode(INT16 driveno, DROBJ **pobj, UINT8 *path)
{
STATUS      ret_stat;
DROBJ       *pmom;
DROBJ       *pchild;
DDRIVE      *pdrive;
UINT8       *childpath;


    /* Find the drive */
    pdrive = pc_drno2dr(driveno);
    if (!pdrive)
        return(NUF_NOT_OPENED);

    /* Get the top of the current path */
    if (path)
    {
        if ( *path == BACKSLASH )
        {
            /* Get root directory. */
            *pobj = pc_get_root(pdrive);
            if (!*pobj)
                return(NUF_NO_DROBJ);

            /* Increment path. */
            path++;
        }
        else
        {
            /* Get current directory. */
            *pobj = pc_get_cwd(pdrive);
            if (!*pobj)
                return(NUF_NO_DROBJ);
        }
    }
    else
    {
        /* Get current directory. */
        *pobj = pc_get_cwd(pdrive);
        if (!*pobj)
            return(NUF_NO_DROBJ);
        return(NU_SUCCESS);
    }

    ret_stat = NU_SUCCESS;

    /* Search through the path until exhausted */
    while (*path)
    {
        /* Move to the next path. */
        childpath = pc_nibbleparse(path);
        if (!childpath)
        {
            break;
        }
        /* is dot */
        if ( (*path == '.') && ((*(path+1) == BACKSLASH) || (*(path+1) == '\0')) )
            ;
        else
        {
            /* Lock the finode. */
            PC_INODE_ENTER((*pobj)->finode, NO)

            /* Find Filename in pobj and initialize pchild with result. */
            pchild = NU_NULL;
            ret_stat = pc_get_inode(&pchild, (*pobj), path);
            if (ret_stat != NU_SUCCESS)
            {
                /* Release exclusive use of finode. */
                PC_INODE_EXIT((*pobj)->finode)
                pc_freeobj(*pobj);
                break;
            }
            
            /* We found it. We have one special case. if "..", we need
               to shift up a level so we are not the child of mom
               but of grand mom. */
            if ( (*path == '.') && (*(path+1) == '.') &&
                            ((*(path+2) == BACKSLASH) || (*(path+2) == '\0')) )

            {
                 /* Find pobj's parent. By looking back from ".." */
                ret_stat = pc_get_mom(&pmom, pchild);
                /* Release exclusive use of finode. */
                PC_INODE_EXIT((*pobj)->finode)
                /* We're done with pobj for now */
                pc_freeobj(*pobj);

                if (ret_stat != NU_SUCCESS)
                {
                    /* We're done with pchild for now. */
                    pc_freeobj(pchild);
                    break;
                }
                else
                {
                    /* We found the parent now free the child */
                    *pobj = pmom;
                    pc_freeobj(pchild);
                }
            }
            else
            {
                /* Release exclusive use of finode. */
                PC_INODE_EXIT((*pobj)->finode)
                /* We're done with pobj for now */
                pc_freeobj(*pobj);
                /* Make sure pobj points at the next inode */
                *pobj = pchild;
            }
        }
        /* Move to the next path. */
        path = childpath;
    }
    return(ret_stat);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_get_inode                                                    
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Search the directory for the pattern or name in filename.       
*       If pobj is NULL start the search at the top of pmom (getfirst)  
*       and allocate pobj before returning it.                          
*       Otherwise start the search at pobj .                            
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       **pobj                              Output search drive object  
*       *pmom                               Search the drive object     
*       *filename                           Search file name            
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       NU_SUCCESS                          Search successful.          
*       NUF_LONGPATH                        Path or directory name too  
*                                            long.                      
*       NUF_NOFILE                          The specified file not      
*                                            found.                     
*       NUF_NO_BLOCK                        No block buffer available.  
*       NUF_NO_FINODE                       No FINODE buffer available. 
*       NUF_NO_DROBJ                        No DROBJ buffer available.  
*       NUF_IO_ERROR                        Driver I/O error.            
*       NUF_INTERNAL                        Nucleus FILE internal error.
*                                                                       
*************************************************************************/
STATUS pc_get_inode(DROBJ **pobj, DROBJ *pmom, UINT8 *filename)
{
INT         starting = NO;
INT         len;
STATUS      ret_stat = NU_SUCCESS;


    /* measure long file name length */
    if (filename != NU_NULL)
    {
        for (len = 0; *(filename+len); len++);
    }
    else
        len = 0;

    /* Check file name length. */
    if ((len + pmom->finode->abs_length) > EMAXPATH)
    {
        pc_report_error(PCERR_PATHL);
        ret_stat = NUF_LONGPATH;
    }
    
    /* Create the child if just starting */
    if (!*pobj && (ret_stat == NU_SUCCESS) )
    {
        starting = YES;
        /* Allocate DROBJ. */
        *pobj = pc_mkchild(pmom);
        if (!*pobj)
            ret_stat = NUF_NO_DROBJ;
    }
    /* If doing a gnext don't get stuck in and endless loop */
    else if (ret_stat == NU_SUCCESS)
    {
        /* Increment entry index. */
        if ( ++((*pobj)->blkinfo.my_index) >= INOPBLOCK )
        {
            /* Move to the next block directory entry. */
            ret_stat = pc_next_block(*pobj);
            if (ret_stat != NU_SUCCESS)
            {
                if (ret_stat == NUF_NOSPC)
                    ret_stat = NUF_NOFILE;
            }
            else
                (*pobj)->blkinfo.my_index = 0;
        }
        if (ret_stat == NU_SUCCESS)
            (*pobj)->finode->abs_length = pmom->finode->abs_length;
    }

    if (ret_stat == NU_SUCCESS)
    {
        /* Find the filename */
        ret_stat = pc_findin(*pobj, filename);
        if (ret_stat != NU_SUCCESS)
        {
            /* Mark the fname[0] == "\0" entry blocks and index */
            if (ret_stat == NUF_NOFILE)
            {
                pmom->blkinfo.end_block = (*pobj)->blkinfo.my_block;
                pmom->blkinfo.end_index = (*pobj)->blkinfo.my_index;
            }
            if (starting)
            {
                /* We're done with pobj for now. */
                pc_freeobj(*pobj);
                *pobj = NU_NULL;
            }
        }
    }
    return(ret_stat);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_next_inode                                                   
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Next search the directory for the pattern or name in filename.  
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       pobj                                Pobj must not be NULL.         
*       *pmom                               Search the drive object     
*       *filename                           Search file name            
*       attrib                              File attributes             
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       NU_SUCCESS                          Search successful.          
*       NUF_ACCES                           Attempt to open a read only 
*                                            file or a special.         
*       NUF_NOFILE                          The specified file not      
*                                            found.                     
*       NUF_NO_FINODE                       No FINODE buffer available. 
*       NUF_IO_ERROR                        Driver I/O error.            
*       NUF_INTERNAL                        Nucleus FILE internal error.
*                                                                       
*************************************************************************/
STATUS pc_next_inode(DROBJ *pobj, DROBJ *pmom, UINT8 *filename, INT attrib)
{
STATUS      ret_stat;
STATUS      ret_stat_w;


    /* Defaults. */
    ret_stat = NUF_NOFILE;

    while (pobj)
    {
        /* Now find the next file */
        ret_stat_w = pc_get_inode(&pobj, pmom, (UINT8 *)filename);
        if (ret_stat_w != NU_SUCCESS)
        {
            if (ret_stat != NUF_ACCES)
                ret_stat = ret_stat_w;
            break;
        }

        /* Check file attributes. */
        if (pobj->finode->fattribute & attrib)
        {
            /* Special entry? */
            if (! (pc_isdot(pobj->finode->fname, pobj->finode->fext)) &&
                ! (pc_isdotdot(pobj->finode->fname, pobj->finode->fext)) )
            {
                ret_stat = NUF_ACCES;
            }
            if (pobj->linfo.lnament)
            {
                /* We need to clean long filename information */
                lnam_clean(&pobj->linfo, pobj->pblkbuff);
                pc_free_buf(pobj->pblkbuff, NO);
            }
        }
        else
        {
            ret_stat = NU_SUCCESS;
            break;
        }
    }

    return(ret_stat);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       chk_sum                                                         
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Calculating Short Filename Check byte                          
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *sname                              Short Filename String      
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       Check byte                                                      
*                                                                       
*************************************************************************/
UINT8 chk_sum(UINT8 *sname)
{
INT         i;
UINT8       aa,bb,cc,dd;


    dd = 0;
    for (i = 0; i < 10; i++)
    {
        aa = sname[i] + dd;
        bb = aa >> 1;
        cc = bb + 0x80;
        if (aa % 2)
        {
            aa = cc;
        }
        else
        {
            aa = bb;
        }
        dd = aa;
    }

    return(dd + sname[i]);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_cre_longname                                                 
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Create long filename ascii strings from directory entry.        
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *filename                           Pointer to filename to write
*                                            filename.                  
*       *info                               Long filename information.  
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       Always YES                          Success complete.          
*                                                                       
*************************************************************************/
INT pc_cre_longname(UINT8 *filename, LNAMINFO *linfo)
{
INT         i, ci;
INT         my_index;
BLKBUFF     *bbuf;
LNAMENT     *ent;
INT16       fileend_flag;


    if (linfo->lnamblk3)
    {
        /* Top entry is in rbuf. */
        my_index = linfo->lnament - 
                    (INOPBLOCK - linfo->lnam_index) - 1 - INOPBLOCK;
        bbuf = linfo->lnamblk3;
    }
    else if (linfo->lnamblk2)
    {
        /* Top entry is in rbuf. */
        my_index = linfo->lnament - (INOPBLOCK - linfo->lnam_index) - 1;
        bbuf =  linfo->lnamblk2;
    }
    else
    {
        /* Top entry is in buf. */
        my_index = linfo->lnam_index + linfo->lnament - 1;
        bbuf = linfo->lnamblk1;
    }

    fileend_flag = 0;

    for (i = 0; i < linfo->lnament; )
    {
        /* Long filename start block. */
        ent = (LNAMENT *)bbuf->data;

        /* Long filename start entry. */
        ent +=  my_index;

        for (; (my_index >= 0) && (i < linfo->lnament); my_index--, i++)
        {
            for (ci = 0; ci < 10; ci += 2)
            {
                if (!fileend_flag)
                {
                    /* Convert Unicode to ASCII. */
                    *filename = uni2asc(&ent->str1[ci]);
                    if (!*filename)
                        fileend_flag = 1;
                    else
                        filename++;
                }
            }
            for (ci = 0; ci < 12; ci += 2)
            {
                if (!fileend_flag)
                {
                    /* Convert Unicode to ASCII. */
                    *filename = uni2asc(&ent->str2[ci]);
                    if (!*filename)
                        fileend_flag = 1;
                    else
                        filename++;
                }
            }
            for (ci = 0; ci < 4; ci += 2)
            {
                if (!fileend_flag)
                {
                    /* Convert Unicode to ASCII. */
                    *filename = uni2asc(&ent->str3[ci]);
                    if (!*filename)
                        fileend_flag = 1;
                    else
                        filename++;
                }
            }
            *filename = '\0';
            ent--;
        }

        /* End directory entry index on the sector. */
        my_index = INOPBLOCK - 1;

        /* Long file name block. */
        if (bbuf == linfo->lnamblk3)
            bbuf = linfo->lnamblk2;
        else if (bbuf == linfo->lnamblk2)
            bbuf = linfo->lnamblk1;
    }

    return(YES);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_cre_shortfilename                                            
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Create short filename ascii strings from directory entry.       
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *filename                           Pointer to filename to write
*                                            filename.                  
*       *fname                              Pointer to filename.       
*       *fext                               Pointer to file extension.  
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       Short filename length                                          
*                                                                       
*************************************************************************/
INT pc_cre_shortname(UINT8 *filename, UINT8 *fname, UINT8 *fext)
{
INT16       i;
UINT8       *top = filename;
INT         len;


    i = 0;

    /* Delete mark? */
    if (*fname == 0x05)
    {
        *filename = PCDELETE;
        filename++;
        fname++;
        i++;
    }

    /* Setup the filename. */
    while(*fname)
    {
        if (*fname == ' ')
            break;
        else
        {
            /* Copy filename. */
            *filename++ = *fname++;
            i++;
        }
        if (i == 8)
            break;
    }

    /* save filename length */
    len = i;
    
    /* Setup the file extention. */
    i = 0;
    if ( (fext) && (*fext!= ' ') )
    {
        /* Extention mark. */
        *filename++ = '.';
        while (*fext)
        {
            if (*fext == ' ')
                break;
            else
            {
                /* Copy file extention. */
                *filename++ = *fext++;
                i++;
            }
            if (i == 3)
                break;
        }
    }

    len += i;

    /* Get rid of trailing '.' s */
    if ( (i == 0) && (*(filename-1) == '.') && (*top!= '.') )
        filename--;
    *filename = '\0';

    return(len);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       lnam_clean                                                      
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Clear long filname information structure. This function does   
*       not free the block that contains the short filename entry of this   
*       long filename.                                                  
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *linfo                              Clear long filename        
*                                            information                
*       *rbuf                               Clear long filename block  
*                                            buffer                     
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       None.                                                           
*                                                                       
*************************************************************************/
VOID lnam_clean(LNAMINFO *linfo, BLKBUFF *rbuf)
{

    /* Previous block. */
    if (linfo->lnamblk1 != rbuf)
    {
        /* Free long filename entry block. */
        pc_free_buf(linfo->lnamblk1, NO);
        linfo->lnamblk1 = (BLKBUFF *)0;
    }

    /* Previous block. */
    if ( (linfo->lnamblk2 ) && (linfo->lnamblk2 != rbuf) )/* Previous block */
    {
        /* Free long filename entry block. */
        pc_free_buf(linfo->lnamblk2, NO);
        linfo->lnamblk2 = (BLKBUFF *)0;
    }

    /* Clear long filename start block */
    linfo->lnament = 0;

}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_findin                                                       
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Find a filename in the same directory as the argument.          
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *pobj                               Drive object structure      
*       *filename                           Search file name            
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       NU_SUCCESS                          Found the file.             
*       NUF_NOFILE                          The specified file not      
*                                            found.                     
*       NUF_NO_BLOCK                        No block buffer available.  
*       NUF_NO_FINODE                       No FINODE buffer available. 
*       NUF_IO_ERROR                        Driver I/O error.            
*       NUF_INTERNAL                        Nucleus FILE internal error.
*                                                                       
*************************************************************************/
STATUS pc_findin(DROBJ *pobj, UINT8 *filename)
{
STATUS      ret_stat;
BLKBUFF     *rbuf;
DIRBLK      *pd;
DOSINODE    *pi;
FINODE      *pfi;
INT         found_flag = 0;
LNAMINFO    *linfo;
UINT8       ckval;
UINT8       namebuf[EMAXPATH+1];
UINT16       lent_cou;
INT         len = 0;
INT         short_name;


    /* For convenience, we want to get at block info here */
    pd = &pobj->blkinfo;

    /* Move long filename information pointer into internal pointer.  */
    linfo = &pobj->linfo;

    /* Clear the number of long filename entry */
    linfo->lnament = 0;

    /* Clear the seqence number of long file name entry. */
    lent_cou = 0;

    /* Initialize short file name flag. */
    short_name = 0;

    /* Read the directory entry data. */
    ret_stat = pc_read_blk(&rbuf, pobj->pdrive, pobj->blkinfo.my_block);

    /* Move BLKBUFF pointer to DROBJ. */
    pobj->pblkbuff = rbuf;

    while (ret_stat == NU_SUCCESS)
    {
        /* Move directory entry pointer into internal pointer */
        pi = (DOSINODE *) &rbuf->data[0];

        /* Look at the current inode */
        pi += pd->my_index;

        /* And look for a match */
        while ( pd->my_index < INOPBLOCK )
        {
            /* End of dir if name is 0 */
            if (!pi->fname[0])
            {
                /* Is there long filename information in buffer */
                if (linfo->lnament)
                {
                    /* Clean long filename information */
                    lnam_clean(linfo, rbuf);
                }
                /* Free work block */
                pc_free_buf(rbuf, NO);

                return(NUF_NOFILE);
            }

            /* This entry is long filename entry */
            if ( *((UINT8 *)pi + 0x0b) == 0x0f )
            {
                /* Initialize the short file name flag. */
                short_name = 0;

                /* It is not the first entry of the long filename */
                if (linfo->lnament)
                {
                    /* Long filename entry numbers are between 1 and 14h */
                    if ( (*((UINT8 *)pi) == 0x00) || (*((UINT8 *)pi) > 0x14) )
                    {
#ifdef DEBUG2
                        if (*((UINT8 *)pi) != PCDELETE)
                                DEBUG_PRINT("pc_findin  long filename error \r\n");
#endif
                        /* Clean long filename information */
                        lnam_clean(linfo, rbuf);
                    }
                    /* Checking the check value of short filename */
                    else if ( linfo->lnamchk != *((UINT8 *)pi + 0x0d) )
                    {
#ifdef DEBUG2
                        DEBUG_PRINT("pc_findin  short filename check value error \r\n");
#endif

                        /* Clean long filename information */
                        lnam_clean(linfo, rbuf);
                    }
                    /* Checking the long filename entry number */
                    else if ( (lent_cou ) != *((UINT8 *)pi) )
                    {
#ifdef DEBUG2
                        DEBUG_PRINT("pc_findin  long filename entry number error \r\n");
#endif

                        /* Clean long filename information */
                        lnam_clean(linfo, rbuf);
                    }
                    else
                    {
                        /* Increment the number of long filename entry */
                        linfo->lnament++;

                        /* Decrement the seqence number of long filename entry */
                        lent_cou--;

                        /* Save BLKBUFF pointer into long filename info structure */
                        /* BLKBUFF pointer 2 is not used yet */
                        if (linfo->lnamblk2 == 0)
                        {
                            /* Read block is changed */
                            if (linfo->lnamblk1 != rbuf)
                            {
                                /* Save new BLKBUFF pointer into long filename info structure */
                                linfo->lnamblk2 = rbuf;
                            }
                        }
                        /* BLKBUFF pointer 2 is used and BLKBUFF pointer 3 is not used yet */
                        else if (linfo->lnamblk3 == 0)
                        {
                            /* Read block is changed */
                            if (linfo->lnamblk2 != rbuf)
                            {
                                /* Save new BLKBUFF pointer into long filename info structure */
                                linfo->lnamblk3 = rbuf;
                            }
                        }
                    }
                }
                else /* Long filename first entry */
                {
                    /* Long filename first entry must be added 0x40 */
                    if ( ((*((UINT8 *)pi) & 0xF0) != 0x40) &&
                                    ((*((UINT8 *)pi) & 0xF0) != 0x50) )
                    {
#ifdef DEBUG2
                        if (*((UINT8 *)pi) != PCDELETE)
                                DEBUG_PRINT("pc_findin  long filename error \r\n");
#endif
                    }
                    else
                    {
                        /* Set long filename start block */
                        linfo->lnamblk1 = rbuf;
                        linfo->lnamblk2 = NU_NULL;
                        linfo->lnamblk3 = NU_NULL;

                        /* Set long filename start index */
                        linfo->lnam_index = pd->my_index;

                        /* Set check value of short filename */
                        linfo->lnamchk = *((UINT8 *)pi + 0x0d);

                        /* Increment long filename entry */
                        linfo->lnament++;

                        /* Save long filename entry number */
                        lent_cou = *((UINT8 *)pi) - 0x40;

                        /* Next entry number */
                        lent_cou--;
                    }
                }
            }
            /* Short filename entry of the long filename, */
            else if (linfo->lnament)
            {
                if (*((UINT8 *)pi) == PCDELETE)
                {
#ifdef DEBUG2
                    DEBUG_PRINT("pc_findin short filename of long filename deleted \r\n");
#endif

                    /* Clean long filename information */
                    lnam_clean(linfo, rbuf);
                }
                else
                {
                    /* Calculate check value of short filename */
                    ckval = chk_sum((UINT8 *)pi);

                    /* Checking the check value of short filename */
                    if (linfo->lnamchk != ckval)
                    {
#ifdef DEBUG2
                        DEBUG_PRINT("pc_findin  short filename check value error %s %d\r\n",
                                                        __FILE__, __LINE__);
#endif

                        /* Clean long filename information */
                        lnam_clean(linfo, rbuf);
                    }
                    /* The sequence number of long filename entry must be 0 */
                    else if (lent_cou)
                    {
#ifdef DEBUG2
                        DEBUG_PRINT("pc_findin  long filename entry number error %s %d\r\n",
                                                        __FILE__, __LINE__);
#endif

                        /* Clean long filename information */
                        lnam_clean(linfo, rbuf);
                    }
                    else
                    {
                        /* Create long filename string from directory entry */
                        pc_cre_longname(namebuf, linfo);

                        /* Compare long filename */
                        if (YES == pc_patcmp(namebuf, filename ))
                        {
                            /* measure long filename length */
                            for (len = 0; *(namebuf+len); len++);

                            /* Set the file found flag */
                            found_flag = 1;
                        }
                    }
                }
                /* Set the short filename flag. */
                short_name = 1;
            }

            /* Short filename entry */
            if ( (!linfo->lnament) || (short_name) )
            {
                /* The file is not deleted */
                if (*((UINT8 *)pi) != PCDELETE)
                {
                    if (!found_flag)
                    {
                        /* Create short filename string from directory entry */
                        len = pc_cre_shortname((UINT8 *)namebuf, pi->fname, pi->fext);
                        if (len)
                        {
                            /* Compare the filename */
                            if (YES == pc_patcmp(namebuf, filename ))
                            {
                                /* Set the file found flag */
                                found_flag = 1;
                            }
                            /* Long filename? */
                            else if (linfo->lnament)
                            {
                                /* Clean long filename information */
                                lnam_clean(linfo, rbuf);
                            }
                        }
                    }
                }
            }
            /* The file is found */
            if (found_flag)
            {
#ifdef DEBUG1
                DEBUG_PRINT("pc_findin  file found my_block=%d  my_index=%d my_first=%d\r\n",
                                pd->my_block, pd->my_index, pd->my_frstblock );
#endif
                /* We found it */
                /* See if it already exists in the inode list.
                   If so.. we use the copy from the inode list */
                pfi = pc_scani(pobj->pdrive, rbuf->blockno, pd->my_index);

                if (pfi)
                {
                    /* Free the inode. */
                    pc_freei(pobj->finode);
                    /* Since we changed the list go back to the top. */
                    pobj->finode = pfi;
                }
                else
                {
                    /* No inode in the inode list. Copy the data over
                           and mark where it came from */
                    pfi = pc_alloci();
                    if (pfi)
                    {
                        /* Calculate Absolute path length */
                        pfi->abs_length = pobj->finode->abs_length + len;

                        /* Release the current inode. */
                        pc_freei(pobj->finode);
                        /* Since we changed the list go back to the top. */
                        pobj->finode = pfi;

                        /* Convert a dos inode to in mem form. */
                        pc_dos2inode(pobj->finode, pi);

                        /* Mark the inode in the inode buffer. */
                        pc_marki(pobj->finode, pobj->pdrive, pd->my_block, 
                                  pd->my_index);
                    }
                    else
                    {
                        /* Is there long filename information in buffer */
                        if (linfo->lnament)
                        {
                            /* Clean long filename information */
                            lnam_clean(linfo, rbuf);
                        }
                        /* Error. Free current buffer. */
                        pc_free_buf(rbuf, NO);

                        return(NUF_NO_FINODE);
                    }
                }
                /* This is not a long filename */
                if (!linfo->lnament)
                {
                   /* Free current buffer. */
                    pc_free_buf(rbuf, NO);
                }

                return(NU_SUCCESS);
            }                   /* if (found_flag) */

            pd->my_index++;
            pi++;
        }
        /* Is there long filename information in buffer */
        if (!linfo->lnament)
        {
            /* Free current buffer. */
            pc_free_buf(rbuf, NO);
        }

        /* Update the objects block pointer */
        ret_stat = pc_next_block(pobj);
        if (ret_stat != NU_SUCCESS)
        {
            if (ret_stat == NUF_NOSPC)
                ret_stat = NUF_NOFILE;
            break;
        }

        pd->my_index = 0;

        /* Read the next block directory data. */
        ret_stat = pc_read_blk(&rbuf, pobj->pdrive,  pobj->blkinfo.my_block);
        pobj->pblkbuff = rbuf;

    }

    /* Is there long filename information in buffer 
        Note: Short filename buffer is already free */
    if (linfo->lnament)
    {
        /* Clean long filename information */
        lnam_clean(linfo, rbuf);
    }

    /* Always error return */
    return(ret_stat);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_get_mom                                                      
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Given a DROBJ initialized with the contents of a subdirectory's 
*       ".." entry, initialize a DROBJ which is the parent of the       
*       current directory.                                              
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       **pmom                              Output parent drive object  
*       **pdotdot                           ".." entry drive object     
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       NU_SUCCESS                          If service is successful.   
*       NUF_NO_BLOCK                        No block buffer available.  
*       NUF_NO_DROBJ                        No DROBJ buffer available.  
*       NUF_IO_ERROR                        Driver I/O error.            
*       NUF_INTERNAL                        Nucleus FILE internal error.
*                                                                       
*************************************************************************/
STATUS pc_get_mom(DROBJ **pmom, DROBJ *pdotdot)
{
STATUS      ret_stat = NU_SUCCESS;
DDRIVE      *pdrive = pdotdot->pdrive;
UINT32      sectorno;
BLKBUFF     *rbuf;
DIRBLK      *pd;
DOSINODE    *pi;
FINODE      *pfi;


    /* We have to be a subdir */
    if (!pc_isadir(pdotdot))
        ret_stat = NUF_INTERNAL;

    /* If ..->cluster is zero then parent is root */
    if (!pdotdot->finode->fcluster && (ret_stat == NU_SUCCESS) )
    {
        /* Get root directory. */
        *pmom = pc_get_root(pdrive);
        if (!*pmom)
            ret_stat = NUF_NO_DROBJ;
    }
    /* Otherwise : cluster points to the beginning of our parent.
                   We also need the position of our parent in it's parent   */
    else if (ret_stat == NU_SUCCESS)
    {
        *pmom = pc_allocobj();
        if (!*pmom)
            ret_stat = NUF_NO_DROBJ;
        else
        {
            (*pmom)->pdrive = pdrive;
            /* Find .. in our parent's directory */

#if 0
/* DAVEM - 30GB WD300 ONLY */
            if( (UINT32)pdotdot->finode->fcluster > 457672 )
			{
	          trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"pc_get_mom: out of range=%d line %d.\n",(UINT32)pdotdot->finode->fcluster,__LINE__);
			}
#endif
            sectorno = pc_cl2sector(pdrive, (UINT32)pdotdot->finode->fcluster);
            /* We found .. in our parents dir. */
            (*pmom)->pdrive = pdrive;
            (*pmom)->blkinfo.my_frstblock =  sectorno; 
            (*pmom)->blkinfo.my_block     =  sectorno;
            (*pmom)->blkinfo.my_index     =  0;
            (*pmom)->isroot = NO;

            /* Read the data */
            ret_stat = pc_read_blk(&rbuf, (*pmom)->pdrive, (*pmom)->blkinfo.my_block);
            (*pmom)->pblkbuff = rbuf;
            if (ret_stat == NU_SUCCESS)
            {
                /* Convert a dos inode to in mem form. */
                pi = (DOSINODE *) &rbuf->data[0];
                pc_dos2inode((*pmom)->finode, pi);

                /* Free current buffer. */
                pc_free_buf(rbuf, NO);

                /* See if the inode is in the buffers */
                pfi = pc_scani(pdrive, sectorno, 0);
                if (pfi)
                {
                    /* Since we changed the list go back to the top. */
                    pc_freei((*pmom)->finode);
                    (*pmom)->finode = pfi;
                }
                else
                {
                    /* Mark the inode in the inode buffer. */
                    pd = &((*pmom)->blkinfo);
                    pc_marki((*pmom)->finode, (*pmom)->pdrive, pd->my_block, 
                             pd->my_index);
                }
            }
            else    /* Error, something didn't work */
            {
                /* We're done with pmom for now. */
                pc_freeobj(*pmom);
            }
        }
    }
    return(ret_stat);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_mkchild                                                      
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Allocate an object and fill in as much of the the block pointer 
*       section as possible based on the parent.                        
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *pmom                               Drive object                
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       Returns a partially initialized DROBJ if enough core available  
*       and pmom was a valid subdirectory.                              
*                                                                       
*************************************************************************/
DROBJ *pc_mkchild(DROBJ *pmom)
{
DROBJ       *pobj;
DIRBLK      *pd;

    /* Allocate an empty DROBJ and FINODE. */
    pobj = pc_allocobj();
    if (!pobj)
        pobj = NU_NULL;
    else
    {
        pd = &pobj->blkinfo;

        pobj->isroot = NO;              /* Child can not be root */
        pobj->pdrive =  pmom->pdrive;   /* Child inherits moms drive */

        /* Now initialize the fields storing where the child inode lives */
        pd->my_index = 0;
        pd->my_block = pd->my_frstblock = pc_firstblock(pmom);

        if (!pd->my_block)
        {
            /* We're done with pobj for now. */
            pc_freeobj(pobj);
            pobj = NU_NULL;
        }
        else
        {
            /* Set absolute path length. */
            pobj->finode->abs_length = pmom->finode->abs_length + 1;
        }
    }

    return(pobj);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_mknode                                                       
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Creates a file or subdirectory ("inode") depending on the flag  
*       values in attributes. A pointer to an inode is returned for     
*       further processing.                                             
*       Note: After processing, the DROBJ must be released by calling   
*       pc_freeobj.                                                     
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       **pobj                              Create file's DROBJ pointer 
*       *pmom                               Drive object                
*       *filename                           Create filename            
*       *fileext                            Create file extension       
*       attributes                          Create file attributes      
*                                                                       
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       NU_SUCCESS                          If service is successful.   
*       NUF_ROOT_FULL                       Root directry full.         
*       NUF_INVNAME                         Path or filename includes   
*                                            invalid character.         
*       NUF_NOSPC                           No space to create directory
*                                             in this disk.             
*       NUF_NO_BLOCK                        No block buffer available.  
*       NUF_NO_FINODE                       No FINODE buffer available. 
*       NUF_NO_DROBJ                        No DROBJ buffer available.  
*       NUF_IO_ERROR                        Driver I/O error.            
*       NUF_INTERNAL                        Nucleus FILE internal error.
*                                                                       
*************************************************************************/
STATUS pc_mknode(DROBJ **pobj, DROBJ *pmom, UINT8 *filename, 
                  UINT8 *fileext, UINT8 attributes)
{
STATUS      ret_stat;
DOSINODE    *pdinodes;
FINODE      lfinode;
UINT32      cluster;
DATESTR     crdate;
BLKBUFF     *pbuff;
DDRIVE      *pdrive;
DROBJ       *psobj;
UINT8       attr;
UINT8       fname[9];
UINT8       fext[4];
UINT8       shortname[13];
INT         longfile;
STATUS      status;


    /* Defaults. */
    *pobj = NU_NULL;
    ret_stat = NU_SUCCESS;

    /* Move DDRIVE pointer into local pointer. */
    pdrive = pmom->pdrive;

    /* Make a directory? */
    if (attributes & ADIRENT)
    {
        /*Grab a cluster for a new dir and clear it */
        PC_FAT_ENTER(pdrive->driveno)

        /* Allocate a cluster. */
        ret_stat = pc_clalloc(&cluster, pdrive);

#ifdef DEBUG1
        DEBUG_PRINT("pc_mknode  allocate directory cluster number %d\r\n", cluster);
#endif

        /* Release non-exclusive use of FAT. */
        PC_FAT_EXIT(pdrive->driveno)

        if (ret_stat == NU_SUCCESS)
        {
            /* Zero out the cluster. */
            ret_stat = pc_clzero(pdrive, cluster);
            if (ret_stat != NU_SUCCESS)
            {
                /* Grab exclusive access to the FAT. */
                PC_FAT_ENTER(pdrive->driveno)
                /* Release the cluster. */
                pc_clrelease(pdrive, cluster);
                /* Release non-exclusive use of FAT. */
                PC_FAT_EXIT(pdrive->driveno)
            }
        }
    }
    else    /* File case. */
        cluster = 0L;

    if (ret_stat != NU_SUCCESS)
    {
        return(ret_stat);
    }

    /* For a subdirectory, first make it a simple file. We will change the
       attribute after all is clean */

    attr = attributes;
    if (attr & ADIRENT)
        attr = ANORMAL;

    /* Allocate an empty DROBJ and FINODE to hold the new file */
    *pobj = pc_allocobj();
    if (!(*pobj))
    {
        if (cluster)
        {
            /* Grab exclusive access to the FAT. */
            PC_FAT_ENTER(pdrive->driveno)
            /* Release the cluster. */
            pc_clrelease(pdrive, cluster);
            /* Release non-exclusive use of FAT. */
            PC_FAT_EXIT(pdrive->driveno)
        }

        return(NUF_NO_DROBJ);
    }

    /* Take a short filename and extension. */
    longfile = pc_fileparse(fname, fext, filename, fileext);
    if (longfile < 0)
    {
        ret_stat = (STATUS)longfile;
    }
    else
    {
        /* Upperbar short filename ?  */
        while( (longfile == FUSE_UPBAR) && (ret_stat == NU_SUCCESS) )
        {
            /* Search the short filename */
            pc_cre_shortname((UINT8 *)shortname, fname, fext);
            psobj = NU_NULL;
            ret_stat = pc_get_inode(&psobj, pmom, (UINT8 *)shortname);
            /* File not found. We can use this short filename. */
            if (ret_stat == NUF_NOFILE)
            {
                ret_stat = NU_SUCCESS;
                break;
            }
            else if (ret_stat == NU_SUCCESS)
            {
                /* Free the short filename object */
                pc_freeobj(psobj);
                /* Get the next short filename */
                pc_next_fparse(fname);
            }
        }
    }

    if (ret_stat != NU_SUCCESS)
    {
        /* We're done with pobj for now. */
        pc_freeobj(*pobj);
        if (cluster)
        {
            /* Grab exclusive access to the FAT. */
            PC_FAT_ENTER(pdrive->driveno)
            /* Release the cluster. */
            pc_clrelease(pdrive, cluster);
            /* Release non-exclusive use of FAT. */
            PC_FAT_EXIT(pdrive->driveno)
        }

        return(ret_stat);
    }

    /* Load the inode copy name, ext, attr, cluster, size, date, and time*/
    pc_init_inode( (*pobj)->finode, fname, fext, 
                    attr, cluster, /*size*/ 0L, pc_getsysdate(&crdate) );

    /* Convert pobj to native and stitch it in to mom */
    ret_stat = pc_insert_inode(*pobj, pmom, filename, longfile);
    if (ret_stat != NU_SUCCESS)
    {
        /* Root directory entry full or Disk full ? */
        if ( (ret_stat == NUF_ROOT_FULL) || (ret_stat == NUF_NOSPC) )
        {
            /* Try again */
            ret_stat = pc_insert_inode(*pobj, pmom, filename, longfile);
            if (ret_stat != NU_SUCCESS) 
            {
                /* We're done with pobj for now. */
                pc_freeobj(*pobj);
                if (cluster)
                {
                    /* Grab exclusive access to the FAT. */
                    PC_FAT_ENTER(pdrive->driveno)
                    /* Release the cluster. */
                    pc_clrelease(pdrive, cluster);
                    /* Release non-exclusive use of FAT. */
                    PC_FAT_EXIT(pdrive->driveno)
                }

                return(ret_stat);
            }
        }
        else
        {
            /* We're done with pobj for now. */
            pc_freeobj(*pobj);
            if (cluster)
            {
                /* Grab exclusive access to the FAT. */
                PC_FAT_ENTER(pdrive->driveno)
                /* Release the cluster. */
                pc_clrelease(pdrive, cluster);
                /* Release non-exclusive use of FAT. */
                PC_FAT_EXIT(pdrive->driveno)
            }

            return(ret_stat);
        }
    }

    /* Now if we are creating a subdirectory we have to make the DOT 
      and DOT DOT inodes and then change pobj's attribute to ADIRENT.
      The DOT and DOTDOT are not buffered inodes. */
    if (attributes & ADIRENT)
    {
#if 0
/* DAVEM - 30GB WD300 ONLY */
            if( (UINT32)cluster > 457672 )
			{
	          trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"pc_mknode: out of range=%d line %d.\n",cluster,__LINE__);
			}
#endif

        /* Set up a buffer to do surgery */
        status = pc_alloc_blk(&pbuff, pdrive, pc_cl2sector(pdrive, cluster));
        if (status < 0)
        {
            /* We're done with pobj for now. */
            pc_freeobj(*pobj);
            /* Grab exclusive access to the FAT. */
            PC_FAT_ENTER(pdrive->driveno)
            /* Release the cluster. */
            pc_clrelease(pdrive, cluster);
            /* Release non-exclusive use of FAT. */
            PC_FAT_EXIT(pdrive->driveno)

            return((STATUS)status);
        }
        pc_memfill(pbuff->data, 512, '\0');

        pdinodes = (DOSINODE *) &pbuff->data[0];
        /* Load DOT and DOTDOT in native form */
        /* DOT first. It points to the begining of this sector */
        pc_init_inode(&lfinode, (UINT8 *)".", (UINT8 *)"", ADIRENT, cluster, /*size*/ 0L, &crdate);

        /* And to the buffer in intel form */
        pc_ino2dos (pdinodes, &lfinode);

        /* Now DOTDOT points to mom's cluster */
        if (pmom->isroot)
        {
            pc_init_inode(&lfinode, (UINT8 *)"..", (UINT8 *)"", ADIRENT, 0L, /*size*/ 0L, &crdate);
        }
        else
        {
            pc_init_inode(&lfinode, (UINT8 *)"..", (UINT8 *)"", ADIRENT, 
                          pc_sec2cluster(pdrive, (*pobj)->blkinfo.my_frstblock),
                         /*size*/ 0L, &crdate);
        }
        /* And to the buffer in intel form */
        pc_ino2dos (++pdinodes, &lfinode);

        /* Write the cluster out */
        ret_stat = pc_write_blk(pbuff);
        if (ret_stat != NU_SUCCESS)
        {
            /* Error. Free current buffer. */
            pc_free_buf(pbuff, YES);
            /* We're done with pobj for now. */
            pc_freeobj(*pobj);
            /* Grab exclusive access to the FAT. */
            PC_FAT_ENTER(pdrive->driveno)
            /* Release the cluster. */
            pc_clrelease(pdrive, cluster);
            /* Release non-exclusive use of FAT. */
            PC_FAT_EXIT(pdrive->driveno)

            return(ret_stat);
        }
        else
            /* Free current buffer. */
            pc_free_buf(pbuff, NO);

        /* And write the node out with the original attributes */
        (*pobj)->finode->fattribute = attributes;

        /* Convert to native and overwrite the existing inode*/
        ret_stat = pc_update_inode(*pobj, DSET_CREATE);
        if (ret_stat != NU_SUCCESS)
        {
            /* We're done with pobj for now. */
            pc_freeobj(*pobj);
            /* Grab exclusive access to the FAT. */
            PC_FAT_ENTER(pdrive->driveno)
            /* Release the cluster. */
            pc_clrelease(pdrive, cluster);
            /* Release non-exclusive use of FAT. */
            PC_FAT_EXIT(pdrive->driveno)

            return(ret_stat);
        }
    }

    /* Grab exclusive access to the FAT. */
    PC_FAT_ENTER(pdrive->driveno)
    /* Flush the file allocation table. */
    ret_stat = pc_flushfat(pdrive);
    /* Release non-exclusive use of FAT. */
    PC_FAT_EXIT(pdrive->driveno)

    if (ret_stat != NU_SUCCESS)
    {
        /* We're done with pobj for now. */
        pc_freeobj(*pobj);
        /* Grab exclusive access to the FAT. */
        PC_FAT_ENTER(pdrive->driveno)
        /* Release the cluster. */
        pc_clrelease(pdrive, cluster);
        /* Release non-exclusive use of FAT. */
        PC_FAT_EXIT(pdrive->driveno)
    }

    return(ret_stat);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_insert_inode                                                 
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Take mom, a fully defined DROBJ, and pobj, a DROBJ with a      
*       finode containing name, ext, etc., but not yet stitched into the 
*       inode buffer pool, and fill in pobj and its inode. Write it to  
*       disk and make the inode visible in the inode buffer pool.       
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *pobj                               Create file's DROBJ.        
*       *pmom                               Drive object.               
*       *filename                           Create file name.           
*       longfile                            If 1, long file name is     
*                                            given.                     
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       NU_SUCCESS                          If service is successful.   
*       NUF_ROOT_FULL                       Root directry full.         
*       NUF_NOSPC                           No space to create directory
*                                             in this disk.             
*       NUF_NO_BLOCK                        No block buffer available.  
*       NUF_IO_ERROR                        Driver I/O error.            
*       NUF_INTERNAL                        Nucleus FILE internal error.
*                                                                       
*************************************************************************/
STATUS pc_insert_inode(DROBJ *pobj, DROBJ *pmom, UINT8 *lfilename, 
                       INT longfile)
{
STATUS      ret_stat;
INT         ret_val;
BLKBUFF     *pbuff;
DIRBLK      *pd;
DOSINODE    *pi;
INT16       i;
UINT32      cluster;
INT16       len;
INT16       entry = 0,entry_cou;
INT16       found_flag = 0;
LNAMINFO    *linfo;
BLKBUFF     *cblk;
UINT8       *lmake_ptr;
LNAMENT     *ent;
INT16       ci;
INT16       fileend_flag;
UINT16      add_entrysec;
UINT8       chk_byte;


    /* Set up pobj. */
    pobj->pdrive = pmom->pdrive;
    pobj->isroot = NO;
    pd = &pobj->blkinfo;

    /* Long filename is given. */
    if (longfile)
    {
        /* measure long filename length. */
        for (len = 0; *(lfilename+len); len++);

        /* Calculate how many entry is needed for long filename. */
        entry = (len + 12) / 13;

        /* Increase for short filename. */
        entry++;
    }

    /* Initialize long filename entry counter. */
    entry_cou = 0;

    /* Now get the start of the dir. */
    pd->my_block = pd->my_frstblock = pc_firstblock(pmom);
    if (!pd->my_block)
    {
	    trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"pc_insert_inode: NUF_INTERNAL line %d.\n",__LINE__);
        return(NUF_INTERNAL);
    }

    pd->my_index = 0;

    /* Set the blank entry blocks and index. */
    if (pmom->blkinfo.end_block)
    {
        pd->my_block = pmom->blkinfo.end_block;
        pd->my_index = pmom->blkinfo.end_index;
    }

    /* Move long filename information pointer into internal pointer. */
    linfo = &pobj->linfo;

    /* Long filename is given. */
    if (longfile)
    {
        /* Set the long file name entry. */
        linfo->lnament = entry - 1;
    }

    /* Read the directory entry data. */
    ret_stat = pc_read_blk(&pbuff, pobj->pdrive, pobj->blkinfo.my_block);
    pobj->pblkbuff = pbuff;
	if( ret_stat != NU_SUCCESS )
	{
	  trace_new( TRACE_ATA | TRACE_LEVEL_4,"pc_insert_inode: pc_read_blk(0x%x,%d,%d) returns %d line %d.\n",&pbuff, pobj->pdrive, pobj->blkinfo.my_block,ret_stat,__LINE__);
	}
    while (ret_stat == NU_SUCCESS)
    {
        /* Move directory entry pointer into local pointer. */
        i = pd->my_index;
        pi = (DOSINODE *) &pbuff->data[0];
        pi += i;

        /* Look for a slot. */
        while (i < INOPBLOCK)
        {
            /* End of dir if name is 0. */
            if ( (pi->fname[0] == '\0') || (pi->fname[0] == PCDELETE) )
            {
                /* Long filename is given. */
                if (longfile)
                {
                    /* This is long filename first entry. */
                    if (!entry_cou)
                    {
                        /* Set long filename start block. */
                        linfo->lnamblk1 = pbuff;
                        linfo->lnamblk2 = NU_NULL;
                        linfo->lnamblk3 = NU_NULL;

                        /* Set long filename start index. */
                        linfo->lnam_index = i;

                    }
                    /* Save BLKBUFF pointer into long filename info 
                       structure. */
                    /* BLKBUFF pointer 2 is not used yet. */
                    if (linfo->lnamblk2 == 0)
                    {
                        /* Read block is changed. */
                        if (linfo->lnamblk1 != pbuff)
                        {
                            /* Save new BLKBUFF pointer into long filename
                               info structure. */
                            linfo->lnamblk2 = pbuff;
                        }
                    }
                    /* BLKBUFF pointer 2 is used and BLKBUFF pointer 3 
                       is not used yet. */
                    else if (linfo->lnamblk3 == 0)
                    {
                        /* Read block is changed. */
                        if (linfo->lnamblk2 != pbuff)
                        {
                            /* Save new BLKBUFF pointer into long filename
                               info structure. */
                            linfo->lnamblk3 = pbuff;
                        }
                    }
                    /* Long filename entry counter increment. */
                    entry_cou++;
                    /* Found room to put the long filename entry. */
                    if (entry_cou >= entry)
                        /* We found enough space. */
                        found_flag = 1;
                }
                else    /* Short filename. */
                    found_flag = 1;

                if (found_flag)     /* Found enough space. */
                {
                    /* Set current index */
                    pd->my_index = i;

#ifdef DEBUG1
                    DEBUG_PRINT("pc_insert_inode my_block index %d\r\n", 
                                                pd->my_block, pd->my_index);
#endif
                    /* Update the DOS disk. */
                    pc_ino2dos(pi, pobj->finode);

                    /* Long filename case. */
                    if (longfile)
                    {
                        /* Calculate check value of short filename. */
                        chk_byte = chk_sum((unsigned char *)pi);

                        /* Calculate long filename entry. */
                        entry--;

                        /* Get LNAMENT pointer. */
                        ent = (LNAMENT *)pi;
                        /* Set long filename pointer. */
                        lmake_ptr = lfilename;
                        /* Clear file name end flag. */
                        fileend_flag = 0;

                        /* Set the current block. */
                        cblk = pbuff;

                        /* Create long filename entry. */
                        for (entry_cou = 0; entry_cou < entry; entry_cou++)
                        {
                            if ( ent == (LNAMENT *)&cblk->data[0] )
                            {
                                /* Is short filename entry in current 
                                   block? */
                                if (cblk != pbuff)
                                {
                                    /* Write out current block. */
                                    ret_stat = pc_write_blk(cblk);
                                    if (ret_stat != NU_SUCCESS)
                                    {
	                                    trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"pc_insert_inode: pc_write_blk returns %d line %d.\n",ret_stat,__LINE__);
                                        /* Clean long filename 
                                           information. */
                                        lnam_clean(linfo, pbuff);
                                        /* Free current buffer. */
                                        pc_free_buf(pbuff, YES);

                                        return(ret_stat);
                                    }
                                    /* Free current buffer. */
                                    pc_free_buf(cblk, NO);
                                }

                                /* Current block is 3rd. */
                                if (cblk == linfo->lnamblk3)
                                {
                                    /* Set current block pointer block 
                                       2nd. */
                                    cblk = linfo->lnamblk2;
                                }
                                /* Current block is 2nd. */
                                else if (cblk == linfo->lnamblk2)
                                {
                                    /* Set current block pointer block 
                                       3rd. */
                                    cblk = linfo->lnamblk1;
                                }

                                /* Move long filename entry pointer into 
                                   local pointer. */
                                ent = (LNAMENT *)
                                        &cblk->data[(INOPBLOCK-1) << 5];
                            }
                            else
                                ent--;

                            /* Set sequence number. */
                            ent->ent_num = (entry_cou + 1);

                            /* Top entry. */
                            if ( (entry_cou + 1) == entry )
                                ent->ent_num += 0x40;

                            /* Set file attributes and checksum. */
                            ent->attrib = 0x0f;
                            ent->reserve = 0;
                            ent->snamchk = chk_byte;
                            ent->fatent[0] = 0;
                            ent->fatent[1] = 0;

                            /* Write 1st filename block. */
                            for (ci = 0; ci < 10; ci += 2)
                            {
                                if (fileend_flag)
                                {
                                    ent->str1[ci] = 0xFF;
                                    ent->str1[ci+1] = 0xFF;
                                }
                                else
                                {
                                    /* Convert ASCII to Unicode. */
                                    asc2uni(&ent->str1[ci], *lmake_ptr);
                                    if (!*lmake_ptr)
                                        fileend_flag = 1;
                                    else
                                        lmake_ptr++;
                                }
                            }
                            /* Write 2nd filename block. */
                            for (ci = 0; ci < 12; ci += 2)
                            {
                                if (fileend_flag)
                                {
                                    ent->str2[ci] = 0xFF;
                                    ent->str2[ci+1] = 0xFF;
                                }
                                else
                                {
                                    /* Convert ASCII to Unicode. */
                                    asc2uni(&ent->str2[ci], *lmake_ptr);
                                    if (!*lmake_ptr)
                                        fileend_flag = 1;
                                    else
                                        lmake_ptr++;
                                }
                            }
                            /* Write 3rd filename block. */
                            for (ci = 0; ci < 4; ci += 2)
                            {
                                if (fileend_flag)
                                {
                                    ent->str3[ci] = 0xFF;
                                    ent->str3[ci+1] = 0xFF;
                                }
                                else
                                {
                                    /* Convert ASCII to Unicode. */
                                    asc2uni(&ent->str3[ci], *lmake_ptr);
                                    if (!*lmake_ptr)
                                        fileend_flag = 1;
                                    else
                                        lmake_ptr++;
                                }
                            }
                        }
                        if (cblk != pbuff)
                        {
                            /* Write out current block. */
                            ret_stat = pc_write_blk(cblk);
                            if (ret_stat != NU_SUCCESS)
                            {
                                /* Clean long filename information. */
                                lnam_clean(linfo, pbuff);
                                /* Free current buffer. */
                                pc_free_buf(pbuff, YES);

                                return(ret_stat);
                            }
                            /* Free current buffer. */
                            pc_free_buf(cblk, NO);
                        }
                    }

                    /* Write the data. */
                    ret_stat = pc_write_blk(pobj->pblkbuff);
                    /* Mark the inode in the inode buffer. */
                    if (ret_stat == NU_SUCCESS)
                    {
                        /* Mark the inode in the inode buffer. */
                        pc_marki(pobj->finode, pobj->pdrive,
                                    pd->my_block, pd->my_index );
                        /* Free current buffer. */
                        pc_free_buf(pbuff, NO);
                    }
                    else
					{
	                    trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"pc_insert_inode: pc_write_blk returns %d line %d.\n",ret_stat,__LINE__);
                        /* Error. Free current buffer. */
                        pc_free_buf(pbuff, YES);
                    }
                    return(ret_stat);
                }
            }
            else
            {
                /* Save the long filename BLKBUFF pointer. */
                if (entry_cou)
                {
                    /* Clean long filename information. */
                    lnam_clean(linfo, pbuff);
                    /* Resetup linfo */
                    linfo = &pobj->linfo;
                }
                entry_cou = 0;
            }
            i++;
            pi++;
        }

        if (! entry_cou)
        {
            /* Not in that block. Try again. */
            pc_free_buf(pbuff, NO);
        }

        /* Update the objects block pointer. */
        ret_stat = pc_next_block(pobj);

        /* No more next allocated block or error. */
        if (ret_stat != NU_SUCCESS)
            break;

        /* Read the next directory sector. */
        ret_stat = pc_read_blk(&pbuff, pobj->pdrive, 
                                    pobj->blkinfo.my_block);
        pobj->pblkbuff = pbuff;
        pd->my_index = 0;
    }

    if ( (ret_stat != NU_SUCCESS) && (ret_stat != NUF_NOSPC) )
    {
        return(ret_stat);
    }

    /* Check if the search start pointer is set. */
    if (pmom->blkinfo.end_block)
    {

         /* Crean long filename BLKBUFF pointer. */
        if (entry_cou)
        {
            /* Clean long filename information. */
            lnam_clean(linfo, pbuff);

            /* Free the buffer. */
            pc_free_buf(pbuff, NO);

        }

        /* Clear the block entry blocks and index. */
        pmom->blkinfo.end_block = (UINT32)0;
        pmom->blkinfo.end_index = (UINT16)0;

        /* To serch again from first block, return once. */
        return(NUF_NOSPC);
    }

    if (pobj->pdrive->fasize <= 4)
    {
        /* Hmmm - root full?. This is a problem. */
        if (pc_isroot(pmom))
        {
#ifdef DEBUG2
            DEBUG_PRINT("Root directory full\r\n");
#endif 

            return(NUF_ROOT_FULL);
        }
    }

    /* There are no slots in mom. We have to make one. 
       And copy our stuff in. */

    /* Grab exclusive access to the FAT. */
    PC_FAT_ENTER(pobj->pdrive->driveno)

    /* Grow a directory chain. */
    ret_stat = pc_clgrow(&cluster, pobj->pdrive, 
                    pc_sec2cluster(pmom->pdrive, pobj->blkinfo.my_block));

    /* Release non-exclusive use of FAT. */
    PC_FAT_EXIT(pobj->pdrive->driveno)

    if (ret_stat != NU_SUCCESS)
    {
        return(ret_stat);
    }

    /* Zero out the cluster. */
    ret_stat = pc_clzero(pobj->pdrive, cluster);
    if (ret_stat != NU_SUCCESS)
    {
        /* Grab exclusive access to the FAT. */
        PC_FAT_ENTER(pobj->pdrive->driveno)
        /* Release the cluster. */
        pc_clrelease(pobj->pdrive, cluster);
        /* Release non-exclusive use of FAT. */
        PC_FAT_EXIT(pobj->pdrive->driveno)

        return(ret_stat);
    }

#if 0
/* DAVEM - 30GB WD300 ONLY */
            if( (UINT32)cluster > 457672 )
			{
	          trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"pc_insert_inode: out of range=%d line %d.\n",cluster,__LINE__);
			}
#endif

    /* Don't forget where the new item is. */
    pd->my_block = pc_cl2sector(pobj->pdrive, cluster);
    pd->my_index = 0;

    /* Copy the item into the first block. */
    /* Setup the short filename entry block buffer. */
    if (longfile)
    {
        /* Use entry blocks is short filename block only. */
        add_entrysec = 0;

        /* Setup the short filename entry. */
        if (linfo->lnamblk1)
        {
            /* Three sector step over to directory entry? */
            if ( ( (linfo->lnam_index + entry - INOPBLOCK) > INOPBLOCK ) )
            {
                if (!linfo->lnamblk2)
                {
                    /* Number of add allocate blocks. */
                    add_entrysec++;

                    /* Set the short filename entry block number. */
                    pd->my_block += 1;
                }
                /* Set the short filename entry index. */
                if (linfo->lnam_index + entry > INOPBLOCK)
                {
                    pd->my_index = linfo->lnam_index + 
                                    entry - INOPBLOCK - 1 - INOPBLOCK;
                }
                else
                {
                    pd->my_index = linfo->lnam_index + entry - 1;
                }
            }
            else
            {
                if (linfo->lnam_index + entry > INOPBLOCK)
                    pd->my_index = linfo->lnam_index + entry - INOPBLOCK - 1;
                else
                    pd->my_index = linfo->lnam_index + entry - 1;
            }
        }
        else
        {
            /* Two sector step over to directory entry? */
            if (entry > INOPBLOCK)
            {
                /* Number of add allocate blocks. */
                add_entrysec++;

                /* Set the short filename entry block number and entry 
                   index. */
                pd->my_block += 1;
                pd->my_index = entry - INOPBLOCK - 1;
            }
            else
                pd->my_index = entry - 1;
        }

        if (add_entrysec)
        {
            /* Set up the first block of the long filename entry. */
            ret_val = pc_alloc_blk(&cblk, pobj->pdrive, (pd->my_block - 1));
            if (ret_val < 0)
            {
                /* Grab exclusive access to the FAT. */
                PC_FAT_ENTER(pobj->pdrive->driveno)
                /* Release the cluster. */
                pc_clrelease(pobj->pdrive, cluster);
                /* Release non-exclusive use of FAT. */
                PC_FAT_EXIT(pobj->pdrive->driveno)

                return((STATUS)ret_val);
            }

            /* Clear the data block. */
            pc_memfill(cblk->data, 512, '\0');

            /* Set the long filename block buffer. */
            if (linfo->lnamblk1)
            {
                /* Three sector step over to directory entrys
                    Note: Long filename information setup
                          (linfo->lnamblk2 == NULL)
                    linfo->lnamblk1 : already setup(First long filename 
                                      entry blocks).
                    linfo->lnamblk2 : cblk(Next long filename entry blocks).
                    linfo->lnamblk3 : Short filename entry blocks.
                */
                linfo->lnamblk2 = cblk;
                linfo->lnamblk3 = NU_NULL;
            }
            else
            {
                /* Two sector step over to directory entrys
                    Note: Long filename information setup
                    linfo->lnamblk1 : cblk(First long filename entry 
                                      blocks).
                    linfo->lnamblk2 : Short filename entry blocks.
                    linfo->lnamblk3 : NULL.
                */
                linfo->lnamblk1 = cblk;
                linfo->lnamblk2 = NU_NULL;
                linfo->lnamblk3 = NU_NULL;
            }
        }
    }

    /* Allocate short filename entry block buffer. */
    ret_val = pc_alloc_blk(&pbuff, pobj->pdrive, pd->my_block);
    if (ret_val < 0)
    {
        /* Grab exclusive access to the FAT. */
        PC_FAT_ENTER(pobj->pdrive->driveno)
        /* Release the cluster. */
        pc_clrelease(pobj->pdrive, cluster);
        /* Release non-exclusive use of FAT. */
        PC_FAT_EXIT(pobj->pdrive->driveno)

        return((STATUS)ret_val);
    }

    /* Clear the data block. */
    pc_memfill(pbuff->data, 512, '\0');

    if (longfile) /* Long filename */
    {
        /* Move to end directory entry index on the sector. */
        pi = (DOSINODE *)&pbuff->data[pd->my_index << 5];

        /* Create short filename entry. */
        pc_ino2dos((DOSINODE *)pi, pobj->finode);

        /* Calculate check value of short filename. */
        chk_byte = chk_sum((unsigned char *)pi);

        /* Calculate long filename entry. */
        entry--;

        /* Get LNAMENT pointer. */
        ent = (LNAMENT *)pi;
        /* Set long filename pointer. */
        lmake_ptr = lfilename;
        /* Clear filename end flag. */
        fileend_flag = 0;

        /* Set the current block. */
        cblk = pbuff;

        /* Set the BLKBUFF pointer into long filename info structure. */
        if (linfo->lnamblk1 == 0)
        {
            linfo->lnamblk1 = cblk;
        }
        else if (linfo->lnamblk2 == 0)
        {
            linfo->lnamblk2 = cblk;
        }
        else if (linfo->lnamblk3 == 0)
        {
            linfo->lnamblk3 = cblk;
        }

        for (entry_cou = 0; entry_cou < entry; entry_cou++)
        {
            if ( ent == (LNAMENT *)&cblk->data[0] )
            {
                /* Is short filename entry in current block? */
                if (cblk != pbuff)
                {
                    /* Write out current block. */
                    ret_stat = pc_write_blk(cblk);
                    if (ret_stat != NU_SUCCESS)
                    {
                        /* Clean long filename information. */
                        lnam_clean(linfo, pbuff);
                        /* Error. Free current buffer. */
                        pc_free_buf(pbuff, YES);

                        /* Grab exclusive access to the FAT. */
                        PC_FAT_ENTER(pobj->pdrive->driveno)
                        /* Release the cluster. */
                        pc_clrelease(pobj->pdrive, cluster);
                        /* Release non-exclusive use of FAT. */
                        PC_FAT_EXIT(pobj->pdrive->driveno)

                        return(ret_stat);
                    }
                    /* Free current buffer. */
                    pc_free_buf(cblk, NO);
                }

                /* Current block is 3st. */
                if (cblk == linfo->lnamblk3)
                {
                    /* Set current block pointer block 2nd. */
                    cblk = linfo->lnamblk2;
                }
                /* Current block is 2nd. */
                else if (cblk == linfo->lnamblk2)
                {
                    /* Set current block pointer block 1rd. */
                    cblk = linfo->lnamblk1;
                }
                /* Move long filename entry pointer into local pointer. */
                ent = (LNAMENT *)&cblk->data[(INOPBLOCK-1) << 5];
            }
            else
                ent--;

            /* Set sequence number. */
            ent->ent_num = (entry_cou + 1);

            if ( (entry_cou + 1) == entry ) /* Top entry. */
                ent->ent_num += 0x40;

            /* Set file attributes and checksum. */
            ent->attrib = 0x0f;
            ent->reserve = 0;
            ent->snamchk = chk_byte;
            ent->fatent[0] = 0;
            ent->fatent[1] = 0;

            for (ci = 0; ci < 10; ci += 2)
            {
                if (fileend_flag)
                {
                    ent->str1[ci] = 0xFF;
                    ent->str1[ci+1] = 0xFF;
                }
                else
                {
                    /* Convert ASCII to Unicode. */
                    asc2uni(&ent->str1[ci], *lmake_ptr);
                    if (!*lmake_ptr)
                        fileend_flag = 1;
                    else
                        lmake_ptr++;
                }
            }
            for (ci = 0; ci < 12; ci += 2)
            {
                if (fileend_flag)
                {
                    ent->str2[ci] = 0xFF;
                    ent->str2[ci+1] = 0xFF;
                }
                else
                {
                    /* Convert ASCII to Unicode. */
                    asc2uni(&ent->str2[ci], *lmake_ptr);
                    if (!*lmake_ptr)
                        fileend_flag = 1;
                    else
                        lmake_ptr++;
                }
            }
            for (ci = 0; ci < 4; ci += 2)
            {
                if (fileend_flag)
                {
                    ent->str3[ci] = 0xFF;
                    ent->str3[ci+1] = 0xFF;
                }
                else
                {
                    /* Convert ASCII to Unicode. */
                    asc2uni(&ent->str3[ci], *lmake_ptr);
                    if (!*lmake_ptr)
                        fileend_flag = 1;
                    else
                        lmake_ptr++;
                }
            }
        }
        if (cblk != pbuff)
        {
            /* Write out current block. */
            ret_stat = pc_write_blk(cblk);
            if (ret_stat != NU_SUCCESS)
            {
                /* Clean long filename information. */
                lnam_clean(linfo, pbuff);
                /* Error. Free current buffer. */
                pc_free_buf(pbuff, YES);

                /* Grab exclusive access to the FAT. */
                PC_FAT_ENTER(pobj->pdrive->driveno)
                /* Release the cluster. */
                pc_clrelease(pobj->pdrive, cluster);
                /* Release non-exclusive use of FAT. */
                PC_FAT_EXIT(pobj->pdrive->driveno)

                return(ret_stat);
            }
            /* Free current buffer. */
            pc_free_buf(cblk, NO);
        }
    }
    else /* short filename */
    {
        /* Create short filename entry. */
        pc_ino2dos((DOSINODE *)&pbuff->data[0], pobj->finode);
    }

    /* Write it out. */
    ret_stat = pc_write_blk(pbuff);
    if (ret_stat != NU_SUCCESS)
    {
        /* Error. Free current buffer. */
        pc_free_buf(pbuff, YES);
        /* Grab exclusive access to the FAT. */
        PC_FAT_ENTER(pobj->pdrive->driveno)
        /* Release the cluster. */
        pc_clrelease(pobj->pdrive, cluster);
        /* Release non-exclusive use of FAT. */
        PC_FAT_EXIT(pobj->pdrive->driveno)

        return(ret_stat);
    }

    /* We made a new slot. Mark the inode as belonging there. */
    pc_marki(pobj->finode, pobj->pdrive, pd->my_block, pd->my_index);

   /* Free current buffer. */
    pc_free_buf(pbuff, NO);

    return(NU_SUCCESS);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_del_lname_block                                              
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Delete all long filename directory entry sectors.               
*                  
*                                                                       
* INPUTS                                                                
*                                                                       
*       *pobj                               Drive object(Delete long    
*                                            filename information )     
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       None.                                                           
*                                                                       
*************************************************************************/
VOID pc_del_lname_block(DROBJ *pobj)
{
LNAMINFO    *linfo;
BLKBUFF     *cblk;
LNAMENT     *ent;
INT16       my_index;
INT         i;


    /* get long filename info */
    linfo = &pobj->linfo;

    /* Get long filename start block */
    cblk = linfo->lnamblk1;
    /* Get long filename start index */
    my_index = linfo->lnam_index;

    /* Move long filename entry pointer into local pointer */ 
    ent = (LNAMENT *)&cblk->data[0];

    /* adjusting entry pointer */
    ent += my_index;

    /* Delete all long filename entry */
    for (i = 0; i < linfo->lnament; i++)
    {
        /* Put delete character */
        ent->ent_num = PCDELETE;
        /* Increment entry pointer */
        ent++;
        /* Increment index */
        my_index++;

        /* This block end */
        if (my_index >= INOPBLOCK)
        {
            /* Reset index */
            my_index = 0;

            /* Write out current block */
            pc_write_blk(cblk);

            /* Is short filename entry in current block ? */

            if (cblk != pobj->pblkbuff)
            {
                /* Free current buffer */
                pc_free_buf(cblk, NO);
            }

            /* Current block is 1st */
            if (cblk == linfo->lnamblk1)
            {
                /* Set current block pointer block 2nd */
                cblk = linfo->lnamblk2;
            }
            /* Current block is 2nd */
            else if (cblk == linfo->lnamblk2)
            {
                /* Set current block pointer block 3rd */
                cblk = linfo->lnamblk3;
            }
            /* Move long filename entry pointer into local pointer */ 
            ent = (LNAMENT *)&cblk->data[0];
        }
    }
    /* Last block also need to be flush */
    pc_write_blk(pobj->pblkbuff);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_renameinode                                                  
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Rename an inode.                                                
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *pobj                               Rename file drive object    
*       *pmom                               Parent drive object         
*       *fnambf                             Short filename buffer      
*       *fextbf                             Short file extension buffer 
*       *new_nme                            Long filename pointer      
*       longdest                            1 : Old file is long filename
*       longsrc                             1 : New file is long filename
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       NU_SUCCESS                          If service is successful.   
*       NUF_ROOT_FULL                       Root directry full.         
*       NUF_NOSPC                           No space to create directory
*                                             in this disk.             
*       NUF_NO_BLOCK                        No block buffer available.  
*       NUF_IO_ERROR                        Driver I/O error.            
*       NUF_INTERNAL                        Nucleus FILE internal error.
*                                                                       
*************************************************************************/
STATUS pc_renameinode(DROBJ *pobj, DROBJ *pmom, UINT8 *fnambuf, 
                      UINT8 *fextbuf, UINT8 *new_name, INT longdest)
{
DROBJ       *new_pobj;
STATUS      ret_stat;


    if (!longdest) /* New filename is short filename */
    {
        /* Copy the filename. */
        copybuff(pobj->finode->fname, fnambuf, 8);
        copybuff(pobj->finode->fext, fextbuf, 3);

        /* Original filename is long filename */
        if (pobj->linfo.lnament) /* long filename */
        {
            /* We need to delete long filename entry */
            pc_del_lname_block(pobj);

        }

        /* Update an inode to disk. */
        ret_stat = pc_update_inode(pobj, DSET_ACCESS);
    }
    else /* new filename is long filename */
    {
        /* Allocate an empty DROBJ and FINODE to hold the new file */
        new_pobj = pc_allocobj();
        if (!new_pobj)
        {
            ret_stat = NUF_NO_DROBJ;
        }
        else
        {
            /* Load the inode copy name, ext, attr, cluster, size, date, and time*/
            pc_init_inode(new_pobj->finode, fnambuf, fextbuf,
                        0, 0, /*size*/ 0L, 0);

            /* copy attribute from original file */
            new_pobj->finode->fattribute = pobj->finode->fattribute;

            new_pobj->finode->fcrcmsec    = pobj->finode->fcrcmsec;
            new_pobj->finode->fcrtime     = pobj->finode->fcrtime;
            new_pobj->finode->fcrdate     = pobj->finode->fcrdate;

            new_pobj->finode->faccdate    = pobj->finode->faccdate;

            new_pobj->finode->fuptime     = pobj->finode->fuptime;
            new_pobj->finode->fupdate     = pobj->finode->fupdate;

            new_pobj->finode->fcluster  = pobj->finode->fcluster;
            new_pobj->finode->fsize     = pobj->finode->fsize;

            /* Convert pobj to native and stitch it into mom */
            ret_stat = pc_insert_inode(new_pobj, pmom, new_name, 1 /* Long filename is given */ );
            if (ret_stat != NU_SUCCESS)
            {
                /* Root directory entry full or Disk full ? */
                if ( (ret_stat == NUF_ROOT_FULL) || (ret_stat == NUF_NOSPC) )
                {
                    /* Try again */
                    ret_stat = pc_insert_inode(new_pobj, pmom, new_name, 1 /* Long filename is given */ );
                    if (ret_stat != NU_SUCCESS)
                    {
                        /* We're done with new_pobj for now. */
                        pc_freeobj(new_pobj);
                    }
                }
                else
                {
                    /* We're done with new_pobj for now. */
                    pc_freeobj(new_pobj);
                }
            }

            if (ret_stat == NU_SUCCESS)
            {
                /* Mark it deleted and unlink the cluster chain */
                pobj->finode->fname[0] = PCDELETE;

                if (pobj->linfo.lnament) /* long filename */
                {
                    /* We need to delete long filename entry */
                    pc_del_lname_block(pobj);
                }
                /* We free up store right away. Don't leave cluster pointer 
                hanging around to cause problems. */
                pobj->finode->fcluster = 0L;

                /* Update an inode to disk. */
                ret_stat = pc_update_inode(pobj, DSET_ACCESS);
                if (ret_stat == NU_SUCCESS)
                {
                    /* Grab exclusive access to the FAT. */
                    PC_FAT_ENTER(new_pobj->pdrive->driveno)

                    /* Flush the file allocation table. */
                    ret_stat = pc_flushfat(new_pobj->pdrive);

                    /* Release non-exclusive use of FAT. */
                    PC_FAT_EXIT(new_pobj->pdrive->driveno)
                }

                /* We're done with new_pobj for now. */
                pc_freeobj(new_pobj);
            }
        }
    }

    return(ret_stat);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_rmnode                                                       
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Delete the inode at pobj and flush the file allocation table.   
*       Does not check file permissions or if the file is already open. 
*       The inode is marked deleted on the disk and the cluster chain   
*       associated with the inode is freed. (Un-delete won't work)      
*               
*                                                                       
* INPUTS                                                                
*                                                                       
*       *pobj                               Delete file drive object    
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       NU_SUCCESS                          If service is successful.   
*                                            file or a special.         
*       NUF_NO_BLOCK                        No block buffer available.  
*       NUF_IO_ERROR                        Driver I/O error.            
*       NUF_INTERNAL                        Nucleus FILE internal error.
*                                                                       
*************************************************************************/
STATUS pc_rmnode(DROBJ *pobj)
{
UINT32      cluster;
STATUS      ret_stat;


    /* Mark it deleted and unlink the cluster chain */
    pobj->finode->fname[0] = PCDELETE;
    cluster = pobj->finode->fcluster;

    if (pobj->linfo.lnament) /* long filename */
    {
        /* We need to delete long filename entry */
        pc_del_lname_block(pobj);
    }

    /* We free up store right away. Don't leave cluster pointer 
    hanging around to cause problems. */
    pobj->finode->fcluster = 0L;


    /* Update an inode to disk. */
    ret_stat = pc_update_inode(pobj, DSET_ACCESS);
    if (ret_stat == NU_SUCCESS)
    {
        /* And clear up the space */

        /* Grab exclusive access to the FAT. */
        PC_FAT_ENTER(pobj->pdrive->driveno)

        /* Release the chain. */
        pc_freechain(pobj->pdrive, cluster);
        /* Flush the file allocation table. */
        ret_stat = pc_flushfat(pobj->pdrive);

        /* Release non-exclusive use of FAT. */
        PC_FAT_EXIT(pobj->pdrive->driveno)
    }


     /* If it gets here we had a probblem */
    return(ret_stat);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_update_inode                                                 
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Read the disk inode information stored in pobj and write it to  
*       the block and offset on the disk where it belongs. The disk is  
*       first read to get the block and then the inode info is merged in
*       and the block is written. (see also pc_mknode() )               
*                                                                       
*       setdate values are                                              
*                                                                       
*       DSET_ACCESS                         Set the only access-time,   
*                                            date.                      
*       DSET_UPDATE                         Set the access-time,date and
*                                            update-time,date.          
*       DSET_CREATE                         Set the all time and date.  
*                    
*                                                                       
* INPUTS                                                                
*                                                                       
*       *pobj                               Update drive object         
*       setdate                             Set date flag               
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       NU_SUCCESS                          If all went well.           
*       NUF_NO_BLOCK                        No block buffer available.  
*       NUF_IO_ERROR                        Driver I/O error.            
*       NUF_INTERNAL                        Nucleus FILE internal error.
*                                                                       
*************************************************************************/
STATUS pc_update_inode(DROBJ *pobj, INT setdate)
{
STATUS      ret_stat = NU_SUCCESS;
BLKBUFF     *pbuff;
DOSINODE    *pi;
INT16       i;
DIRBLK      *pd;
DATESTR     fds;


	//printf("pc_update_inode: enter %d\n",__LINE__);

    /* Move DIRBLK pointer to local. */
    pd = &pobj->blkinfo;
    /* Get a directory entry index. */
    i  = pd->my_index;

    /* Index into block. */
    if ( i >= INOPBLOCK || i < 0 )
        ret_stat = NUF_INTERNAL;
    
    if (ret_stat == NU_SUCCESS)
    {
	    //printf("pc_update_inode: call pc_getsysdate %d\n",__LINE__);
        /* Get the date */
        pc_getsysdate(&fds);

        /* Set the file create time and date */
        if (setdate == DSET_CREATE)
        {
            pobj->finode->fcrcmsec = fds.cmsec;
            pobj->finode->fcrtime = fds.time;
            pobj->finode->fcrdate = fds.date;
        }

        /* Directory does not need file access date. */
        if ((pobj->finode->fattribute & ADIRENT) != ADIRENT)
        {
            /* Set the file update time and date. */
            if (setdate)
            {
                pobj->finode->fuptime = fds.time;
                pobj->finode->fupdate = fds.date;
            }

            /* Always set the access date. */
            pobj->finode->faccdate = fds.date;
        }

        /* Read the directory data. */
	    //printf("pc_update_inode: call pc_read_blk %d\n",__LINE__);
        ret_stat = pc_read_blk(&pbuff, pobj->pdrive, pobj->blkinfo.my_block);
        if (ret_stat == NU_SUCCESS)
        {
            pi = (DOSINODE *) &pbuff->data[0];
	        //printf("pc_update_inode: call pc_ino2dos %d\n",__LINE__);
            /* Copy it off and write it */
            pc_ino2dos((pi+i), pobj->finode);

            /* Write the directory data. */
            pobj->pblkbuff = pbuff;
            ret_stat = pc_write_blk(pbuff);
	        //printf("pc_update_inode: pc_write_blk %d\n",__LINE__);

            /* Free the buff. If ret_stat != NU_SUCCESS, it will discard 
               the pbuff. */
            if (ret_stat == NU_SUCCESS)
                pc_free_buf(pbuff, NO);
            else
                pc_free_buf(pbuff, YES);
        }
    }
    return(ret_stat);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_get_root                                                     
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Use the information in pdrive to create a special object for    
*       accessing files in the root directory.                          
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *pdrive                             Drive information           
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       Returns a pointer to a DROBJ, or NULL if no core available.     
*                                                                       
*************************************************************************/
DROBJ *pc_get_root(DDRIVE *pdrive)
{
DIRBLK      *pd;
DROBJ       *pobj;
FINODE      *pfi;


    /* Allocate an empty DROBJ and FINODE. */
    pobj = pc_allocobj();
    if (!pobj)
        pobj = NU_NULL;
    else
    {
        /* Search for an inode list. */
        pfi = pc_scani(pdrive, 0, 0);

        if (pfi)
        {
            /* Free the inode that comes with allocobj. */
            pc_freei(pobj->finode);
            /* Since we changed the list go back to the top. */
            pobj->finode = pfi;
        }
        else    /* No inode in the inode list. Copy the data over
                   and mark where it came from */
        {
            /* Mark the inode in the inode buffer. */
            pc_marki(pobj->finode, pdrive, 0, 0);
            pfi = pobj->finode;
        }
        pobj->pdrive = pdrive;
        /* Set up the tree stuff so we know it is the root */
        pd = &pobj->blkinfo;
        pd->my_frstblock = pdrive->rootblock;
        pd->my_block = pdrive->rootblock;
        pd->my_index = 0;
        pobj->isroot = YES;
        pfi->abs_length = 3; /* "A:\" */
    }
    return(pobj);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_firstblock                                                   
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Returns the block number of the first inode in the subdirectory.
*       If pobj is the root directory the first block of the root will  
*       be returned.                                                    
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *pobj                               Drive object                
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       Returns 0 if the obj does not point to a directory, otherwise   
*       the first block in the directory is returned.                   
*                                                                       
*************************************************************************/
UINT32 pc_firstblock(DROBJ *pobj)
{
UINT32      ret_val;

    /* Check the directory. */
    if (!pc_isadir(pobj))
        ret_val = BLOCKEQ0;

    else if (pobj->isroot)    /* Root directory? */
    {
        /* Root directory first block. */
        ret_val = pobj->pdrive->rootblock;
    }
    else
    {
        /* Convert the cluster number. */
        ret_val = pc_cl2sector(pobj->pdrive, pobj->finode->fcluster);
    }

    return(ret_val);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_next_block                                                   
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Find the next block owned by an object in either the root or a  
*       cluster chain and update the blockinfo section of the object.   
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *pobj                               Drive object                
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       NU_SUCCESS                          If service is successful.   
*       NUF_NOSPC                           No space to create directory
*                                            on this disk.              
*       NUF_IO_ERROR                        Driver I/O error.            
*       NUF_INTERNAL                        Nucleus FILE internal error.
*                                                                       
*************************************************************************/
STATUS pc_next_block(DROBJ *pobj)
{
STATUS      ret_stat;
UINT32      next;


    /* Get the next block. */
    ret_stat = pc_l_next_block(&next, pobj->pdrive, pobj->blkinfo.my_block);

    if (ret_stat == NU_SUCCESS)
    {
        /* Set the directory entry block. */
        pobj->blkinfo.my_block = next;
    }

    return(ret_stat);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_l_next_block                                                 
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Find the next block in either the root or a cluster chain.      
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *block                              Next block number.          
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       NU_SUCCESS                          If service is successful.   
*       NUF_NOSPC                           No space to create directory
*                                            on this disk.              
*       NUF_IO_ERROR                        Driver I/O error.            
*       NUF_INTERNAL                        Nucleus FILE internal error.
*                                                                       
*************************************************************************/
STATUS pc_l_next_block(UINT32 *block, DDRIVE *pdrive, UINT32 curblock)
{
STATUS      ret_stat = NU_SUCCESS;
UINT32      cluster;


    /* If the block is in the root area */
    if ( (pdrive->fasize < 8) && (curblock < pdrive->firstclblock) )
    {
        /* Check root directory area. */
        if (curblock < pdrive->rootblock)
        {
            ret_stat = NUF_INTERNAL;
        }
        else if (++curblock < pdrive->firstclblock)
        {
            /* Set the next block. */
            *block = curblock;
            ret_stat = NU_SUCCESS;
        }
        else
            ret_stat = NUF_NOSPC;
    }
    else if (curblock >= pdrive->numsecs) /* Check data area. */
        ret_stat = NUF_INTERNAL;

    else    /* In cluster space */
    {
        /* Get the next block */
        curblock += 1;

        /* If the next block is not on a cluster edge then it must be
           in the same cluster as the current. - otherwise we have to
           get the firt block from the next cluster in the chain */
        if (pc_sec2index(pdrive, curblock))
        {
            /* Set the next block. */
            *block = curblock;
        }
        else
        {
            /* Original current block. */
            curblock -= 1;

            /* Get the old cluster number - No error test needed */
            cluster = pc_sec2cluster(pdrive, curblock);

            /* Grab exclusive access to the FAT. */
            PC_FAT_ENTER(pdrive->driveno)

            /* Consult the FAT for the next cluster. */
            ret_stat = pc_clnext(&cluster, pdrive, cluster);

            /* Release non-exclusive use of FAT. */
            PC_FAT_EXIT(pdrive->driveno)

            if (ret_stat == NU_SUCCESS)
            {
#if 0
/* DAVEM - 30GB WD300 ONLY */
            if( (UINT32)cluster > 457672 )
			{
	          trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"pc_l_next_block: out of range=%d line %d.\n",cluster,__LINE__);
			}
#endif

                if (cluster != 0)      /* Not end of chain */
                    /* Convert the new cluster number. */
                    *block = pc_cl2sector(pdrive, cluster);
                else
                    ret_stat = NUF_NOSPC;
            }
        }
    }
    return(ret_stat);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_marki                                                        
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Each inode is uniquely determined by DRIVE, BLOCK and Index into
*       that block. This routine takes an inode structure assumed to    
*       contain the equivalent of a DOS directory entry, and stitches it
*       into the current active inode list. Drive block and index are   
*       stored for later calls to pc_scani and the inode's opencount is 
*       set to one.                                                     
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *pfi                                File directory entry        
*       *pdrive                             Drive information           
*       sectorno                            Start sector number         
*       index                               Entry number                
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       None.                                                           
*                                                                       
*************************************************************************/
VOID pc_marki(FINODE *pfi, DDRIVE *pdrive, UINT32 sectorno, INT16 index)
{

    /* Mark the drive, sectorno, index. */
    pfi->my_drive = pdrive;
    pfi->my_block = sectorno;
    pfi->my_index = index;
    pfi->opencount = 1;

    /* Stitch the inode at the front of the list */
    if (inoroot)
        inoroot->pprev = pfi;

    pfi->pprev = NU_NULL;
    pfi->pnext = inoroot;

    inoroot = pfi;
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_scani                                                        
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Each inode is uniquely determined by DRIVE, BLOCK and Index into
*       that block. This routine searches the current active inode list 
*       to see if the inode is in use. If so the opencount is changed   
*       and a pointer is returned. This guarantees that two processes   
*       will work on the same information when manipulating the same    
*       file or directory.                                              
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *pdrive                             Drive object                
*       sectorno                            Sector number               
*       index                               Entry number                
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       A pointer to the FINODE for pdrive:sector:index or NULL if not  
*       found.                                                          
*                                                                       
*************************************************************************/
FINODE *pc_scani(DDRIVE *pdrive, UINT32 sectorno, INT16 index)
{
FINODE      *pfi;
STATUS      found = 0;

    /* Get inode list. */
    pfi = inoroot;
    while (pfi)
    {
        /* Search a drive's directory entry. */
        if ( (pfi->my_drive == pdrive) &&
             (pfi->my_block == sectorno) &&
             (pfi->my_index == index) )
        {
            /* Increment inode opencount. */
            pfi->opencount += 1;
            found = 1;
            break;
        }

        /* Move to the next inode. */
        pfi = pfi->pnext;
    }

    if (!found)
        pfi = NU_NULL;

    return(pfi);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_allocobj                                                     
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Allocates and zeros the space needed to store a DROBJ structure.
*       Also allocates and zeros a FINODE structure and links the two  
*       via the finode field in the DROBJ structure.                    
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*      None.                                                            
*                                                                       
* OUTPUTS                                                               
*                                                                       
*      Returns a valid pointer or NULL if no more core.                 
*                                                                       
*************************************************************************/
DROBJ *pc_allocobj(VOID)
{
DROBJ       *pobj;
DROBJ       *ret_val;


    /* Alloc a DROBJ */
    pobj = pc_memory_drobj(NU_NULL);
    if (!pobj)
        ret_val = NU_NULL;
    else
    {
        /* Allocate a FINODE. */
        pobj->finode = pc_alloci();
        if (!pobj->finode)
        {
            /* Free the DROBJ */
            pc_memory_drobj(pobj);
            ret_val = NU_NULL;
        }
        else
        {
            ret_val = pobj;
        }
    }
    return(ret_val);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_alloci                                                       
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Allocates and zeros a FINODE structure.                        
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       None.                                                           
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       Returns a valid pointer or NULL if no more core.                
*                                                                       
*************************************************************************/
FINODE *pc_alloci(VOID)
{

    /* Allocate a FINODE structure. */
    return(pc_memory_finode(NU_NULL));
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_free_all_i                                                   
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Release all inode buffers associated with a drive.              
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *pdr                                Drive information           
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       None.                                                           
*                                                                       
*************************************************************************/
VOID pc_free_all_i(DDRIVE *pdrive)
{
FINODE      *pfi;


    /* Get inode list. */
    pfi = inoroot;
    while (pfi)
    {
        /* Search a drive. */
        if (pfi->my_drive == pdrive)
        {
            /* Set the opencount to 1 so freei releases the inode */
            pfi->opencount = 1;
            pc_report_error(PCERR_FREEINODE);
            /* Free the inode that comes with allocobj. */
            pc_freei(pfi);
            /* Since we changed the list go back to the top */
            pfi = inoroot;
        }
        else
            /* Move to the next inode. */
            pfi = pfi->pnext;
    }
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_freei                                                        
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       If the FINODE structure is only being used by one file or DROBJ,
*       unlink it from the internal active inode list and return it to  
*       the heap; otherwise reduce its open count.                      
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *pfi                                File directory entry        
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       None.                                                           
*                                                                       
*************************************************************************/
VOID pc_freei(FINODE *pfi)
{
STATUS      exit = 0;

    if (!pfi)
    {
        pc_report_error(PCERR_FREEINODE);
        exit = 1;
    }
    else if (pfi->opencount)
    {
        /* Decrement opencount and exit if non-zero. */
        if (--pfi->opencount)
            exit = 1;
        else
        {
            /* Point the guy behind us at the guy in front. */
            if (pfi->pprev)
            {
                pfi->pprev->pnext = pfi->pnext;
            }
            else
            {
                /* No prev, we were at the front so make the next guy 
                   the front. */
                inoroot = pfi->pnext;
            }

            /* Make the next guy point behind. */
            if (pfi->pnext)
            {
                pfi->pnext->pprev = pfi->pprev;
            }
        }
    }

    if (!exit)
        pc_memory_finode(pfi);    /* release the core */

}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_freeobj                                                      
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Return a drobj structure to the heap. Calls pc_freei to reduce  
*       the open count of the finode structure it points to and return  
*       it to the heap if appropriate.                                  
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *pobj                               Drive object                
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       None.                                                           
*                                                                       
*************************************************************************/
VOID pc_freeobj(DROBJ *pobj)
{

    if (pobj)
    {
        /* Free the inode that comes with allocobj. */
        pc_freei(pobj->finode);

        /* Free the Long filename entry and Short filename entry BLKBUFF
            Note: See pc_freebuf not call case in pc_findin */
        if (pobj->linfo.lnament)
        {
            /* Clear the short filename BLKBUFF */
            pc_free_buf(pobj->pblkbuff, NO);

            lnam_clean(&pobj->linfo, pobj->pblkbuff);
        }
        /* Release the core */
        pc_memory_drobj(pobj);
    }
    else
        pc_report_error(PCERR_FREEDROBJ);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_dos2inode                                                    
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Take the data from pbuff which is a raw disk directory entry and
*       copy it to the inode at pdir. The data goes from INTEL byte     
*       ordering to native during the transfer.                         
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *pdir                               File directory entry        
*       *pbuff                              Dos directory entry         
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       None.                                                           
*                                                                       
*************************************************************************/
VOID pc_dos2inode(FINODE *pdir, DOSINODE *pbuff)
{
UINT16      clusterhigh;
UINT16      clusterlow;


    /* Copy filename and file extension. */
    copybuff(&pdir->fname[0], &pbuff->fname[0], 8);
    copybuff(&pdir->fext[0], &pbuff->fext[0], 3);

    /* Set file attributes. */
    pdir->fattribute = pbuff->fattribute;

    /* Reserved. */
    pdir->reserve = pbuff->reserve;

    /* Set file date and time. */
    pdir->fcrcmsec = pbuff->fcrcmsec;
    SWAP16(&pdir->fcrtime, &pbuff->fcrtime);
    SWAP16(&pdir->fcrdate, &pbuff->fcrdate);

    SWAP16(&pdir->faccdate, &pbuff->faccdate);

    SWAP16(&pdir->fuptime, &pbuff->fuptime);
    SWAP16(&pdir->fupdate, &pbuff->fupdate);

    /* Set cluster for data file. */
    SWAP16(&clusterhigh, &pbuff->fclusterhigh);
    SWAP16(&clusterlow, &pbuff->fclusterlow);
    pdir->fcluster =  (UINT32) ((clusterhigh << 16) | clusterlow);

    /* Set file size. */
    SWAP32(&pdir->fsize, &pbuff->fsize);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_init_inode                                                   
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Take an uninitialized inode (pdir) and fill in some fields. No  
*       other processing is done. This routine simply copies the        
*       arguments into the FINODE structure.                            
*       Note: filename & fileext do not need null termination.          
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *pdir                               File directory entry        
*       *filename                           File name                   
*       *fileext                            File extension              
*       attr                                File attributes             
*       cluster                             File start cluster number   
*       size                                File size                   
*       *fds                                Date stamping buffer        
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       None.                                                           
*                                                                       
*************************************************************************/
VOID pc_init_inode(FINODE *pdir, UINT8 *filename, UINT8 *fileext, 
                   UINT8 attr, UINT32 cluster, UINT32 size, DATESTR *fds)
{

    /* Copy the filenames and pad with ' ''s */
    pc_cppad(pdir->fname, (UINT8 *)filename, 8);
    pc_cppad(pdir->fext, (UINT8 *)fileext, 3);

    /* Set file attributes. */
    pdir->fattribute = attr;

    /* Reserved. */
    pdir->reserve = '\0';

    if (fds)
    {
        /* Set file date and time. */
        pdir->fcrcmsec = fds->cmsec;
        pdir->fcrtime = fds->time;
        pdir->fcrdate = fds->date;

        pdir->faccdate = fds->date;

        pdir->fuptime = fds->time;
        pdir->fupdate = fds->date;
    }
    /* Set cluster for data file. */
    pdir->fcluster = cluster;
    /* Set file size. */
    pdir->fsize = size;
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_ino2dos                                                      
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Take in memory native format inode information and copy it to a 
*       buffer. Translate the inode to INTEL byte ordering during the   
*       transfer.                                                       
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *pbuf                               Dos Directory Entry         
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       None.                                                           
*                                                                       
*************************************************************************/
VOID pc_ino2dos(DOSINODE *pbuff, FINODE *pdir)
{
UINT16      cltemp;


    /* Copy filename. */
    copybuff(&pbuff->fname[0], &pdir->fname[0], 8);
    /* Delete mark? */
    if (pbuff->fname[0] == (UINT8) 0xE5)
        pbuff->fname[0] = PCDELETE;

    /* Copy file extension. */
    copybuff(&pbuff->fext[0], &pdir->fext[0], 3);

    /* Set file attributes. */
    pbuff->fattribute = pdir->fattribute;

    /* Reserved. */
    pbuff->reserve = pdir->reserve;

    /* Set file date and time. */
    pbuff->fcrcmsec = pdir->fcrcmsec;
    SWAP16(&pbuff->fcrtime, &pdir->fcrtime);
    SWAP16(&pbuff->fcrdate, &pdir->fcrdate);

    SWAP16(&pbuff->faccdate,&pdir->faccdate);

    cltemp = (UINT16) (pdir->fcluster >> 16);
    SWAP16(&pbuff->fclusterhigh,&cltemp);

    SWAP16(&pbuff->fuptime,&pdir->fuptime);
    SWAP16(&pbuff->fupdate,&pdir->fupdate);

    /* Set cluster for data file. */
    cltemp = (UINT16) (pdir->fcluster & 0x0000ffff);
    SWAP16(&pbuff->fclusterlow,&cltemp);

    /* Set file size. */
    SWAP32(&pbuff->fsize,&pdir->fsize);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_isavol                                                       
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Looks at the appropriate elements in pobj and determines if it  
*       is a volume.                                      
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *pobj                               Drive object                
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       Returns NO if the obj does not point to a volume.            
*                                                                       
*************************************************************************/
INT pc_isavol(DROBJ *pobj)                                      /*__fn__*/
{

    return(pobj->finode->fattribute & AVOLUME);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_isadir                                                       
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Looks at the appropriate elements in pobj and determines if it  
*       is a root or subdirectory.                                      
*                                                                       
                                                                       
* INPUTS                                                                
*                                                                       
*       *pobj                               Drive object                
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       Returns NO if the obj does not point to a directory.            
*                                                                       
*************************************************************************/
INT pc_isadir(DROBJ *pobj)                                      /*__fn__*/
{

    return((pobj->isroot) || (pobj->finode->fattribute & ADIRENT));
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_isroot                                                       
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Looks at the appropriate elements in pobj and determines if it  
*       is a root directory.                                            
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *pobj                               Drive object                
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       Returns NO if the obj does not point to the root directory.     
*                                                                       
*************************************************************************/
INT pc_isroot(DROBJ *pobj)                                      /*__fn__*/
{

    return(pobj->isroot);
}



/****************************************************************************
 * Modifications:
 * $Log: 
 *  3    mpeg      1.2         4/3/04 1:04:09 AM      Nagaraja Kolur  CR(s) 
 *        8762 8763 : Merged 2.5.6 Nup File System.
 *        Test: 1. pvr demo 2. nup file test 3. CR 8181
 *        
 *  2    mpeg      1.1         10/15/03 4:56:55 PM    Tim White       CR(s): 
 *        7660 Remove ATA header files from NUP_FILE code.
 *        
 *  1    mpeg      1.0         8/22/03 5:30:16 PM     Dave Moore      
 * $
 ****************************************************************************/

