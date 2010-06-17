/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                 Conexant Systems Inc. (c) 2003                           */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       fileutil.c
 *
 *
 * Description:    see ATI header below.
 *                 Initial version based on Nucleus File v2.4.4
 *
 *
 ****************************************************************************/
/* $Header: fileutil.c, 2, 4/2/04 8:17:48 PM, Nagaraja Kolur$
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
*       FILEUTIL.C                                2.5
*                                                                       
* COMPONENT                                                             
*                                                                       
*       Nucleus FILE                                                    
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*                                                                       
* DATA STRUCTURES                                                       
*                                                                       
*       None.                                                           
*                                                                       
* FUNCTIONS                                                             
*                                                                       
*       pc_allspace                         Test if size characters in  
*                                            a string are spaces.       
*       copybuff                            Copy one buffer to another. 
*       pc_cppad                            Copy one buffer to another  
*                                            and right fill with spaces.
*       pc_isdot                            Test to see if fname is     
*                                            exactly '.'.               
*       pc_isdotdot                         Test if a filename is       
*                                            exactly {'.','.'};         
*       pc_memfill                          Fill a buffer with a        
*                                            character.                 
*       pc_parsedrive                       Get a drive number from a   
*                                            path specifier.            
*       pc_parsenewname                     Setup the new file name.    
*       pc_next_fparse                      Next upperbar file name.    
*       pc_fileparse                        Copy the short file name and
*                                            the short file extension.  
*       pc_nibbleparse                      Nibble off the left most    
*                                            part of a path specifier.        
*       pc_parsepath                        Parse a path specifier into 
*                                            path : file : ext.            
*       pc_patcomp                          Compare strings1 and        
*                                            strings2 as upper letter.  
*       pc_strcat                           strcat                      
*       pc_usewdcard                        Check the use of wild cards.    
*       pc_use_upperbar                     Check the use of upperbar name.
*       pc_checkpath                        Check the name.             
*       _swap16                             16bit data byte swap.       
*       _swap32                             32bit data byte swap.       
*       _through16                          16bit data byte through.    
*       _through32                          32bit data byte through.    
*                                                                       
* DEPENDENCIES                                                          
*                                                                       
*       pcdisk.h                            File common definitions     
*                                                                       
*************************************************************************/

#include        "pcdisk.h"


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_allspace                                                     
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Test if the first size spaces of string are ' ' characters.     
*       Test if size characters in a string are spaces                  
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       p                                   String                      
*       i                                   Size                        
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       Return YES if n bytes of p are spaces                           
*       YES if all spaces.                                              
*                                                                       
*************************************************************************/
INT pc_allspace(UINT8 *p, INT i)
{
INT ret_val = YES;

    while (i--)
    {
        if (*p++ != ' ')
        {
            ret_val = NO;
            break;
        }
    }

    return(ret_val);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       copybuf                                                         
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Copy one buffer to another.                                      
*       Essentially strncpy. Copy 'size' bytes from "vfrom" to "vto".           
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       vto                                 Copy to data buffer         
*       vfrom                               Copy from data buffer       
*       size                                Size                        
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       None.                                                           
*                                                                       
*************************************************************************/
VOID copybuff(VOID *vto, VOID *vfrom, INT size)
{
UINT8       *to   = (UINT8 *) vto;
UINT8       *from = (UINT8 *) vfrom;


    while (size--)
        *to++ = *from++;
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_cppad                                                        
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Copy one buffer to another and right fill with spaces           
*       Copy up to size characters from "from" to "to". If less than    
*       size characters are transferred before reaching \0 fill "to"    
*       with ' ' characters until its length reaches size.              
*       Note: "to" is NOT Null terminated!                            
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       to                                  Copy to data buffer         
*       from                                Copy from data buffer       
*       size                                Size                        
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       None.                                                           
*                                                                       
*************************************************************************/
VOID pc_cppad(UINT8 *to, UINT8 *from, INT size)
{

    while (size--)
    {
        if (*from)
            *to++ = *from++;
        else
            *to++ = ' ';
    }
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_ncpbuf                                                       
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Copy up to size characters from "from" to "to".  Stop copy                 
*       when at end of "from"                                                                
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       to                                  Copy to data buffer.        
*       from                                Copy from data buffer.      
*       size                                Size of buffer.             
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       None.                                                           
*                                                                       
*************************************************************************/
VOID pc_ncpbuf(UINT8 *to, UINT8 *from, INT size)
{

    if (from != NU_NULL)
    {
        while (size--)
        {
            if (*from)
                *to++ = *from++;
            else
            {
                *to = '\0';
                break;
            }
        }
    }
    else
        *to = '\0';
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_isdot                                                        
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Test to see if fname is exactly '.' followed by seven spaces and
*       fext is exactly three spaces.                                   
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       fname                               File name                   
*       fext                                File extension              
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       Return YES if File is exactly '.'                               
*       YES if file:ext == '.'                                          
*                                                                       
*************************************************************************/
INT pc_isdot(UINT8 *fname, UINT8 *fext)
{
INT         stat,stat2,stat3;


    stat =  (*fname == '.');
    stat2 = pc_allspace(fname+1, 7) && pc_allspace(fext, 3);
    stat3 = (*(fname+1) == '\0') && (fext == NU_NULL);

    return( stat && (stat2 | stat3) );
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_isdotdot                                                     
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Test if a filename is exactly {'.','.'};                        
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       fname                               File name                   
*       fext                                File extension              
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       Return YES if File is exactly '.'                               
*       YES if file:ext == {'.','.'}                                    
*                                                                       
*************************************************************************/
INT pc_isdotdot(UINT8 *fname, UINT8 *fext)
{
INT         stat,stat2,stat3;


    stat =  ( (*fname == '.') && (*(fname+1) == '.') );
    stat2 = ( pc_allspace(fname+2, 6) && pc_allspace(fext, 3) );
    stat3 = (*(fname+2) == '\0') && (fext == NU_NULL);

    return( stat && (stat2 | stat3) );
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_memfill                                                      
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Fill "vto" with "size" instances of "c"                                
*       Fill a buffer with a character                                  
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       vto                                 Copy to data buffer         
*       size                                Size                        
*       c                                   Character                   
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       None.                                                           
*                                                                       
*************************************************************************/
VOID pc_memfill(VOID *vto, INT size, UINT8 c)
{
UINT8       *to = (UINT8 *) vto;


    while (size--)
        *to++ = c;
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_parsedrive                                                   
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Get a drive number from a path specifier                        
*       Take a path specifier in path and extract the drive number from 
*       it. If the second character in path is ':' then the first char  
*       is assumed to be a drive specifier and 'A' is subtracted from it
*       to give the drive number. If the drive number is valid, driveno 
*       is updated and a pointer to the text just beyond ':' is returned.
*       Otherwise NU_NULL is returned. If the second character in path is  
*       not ':' then the default drive number is put in driveno and path
*       is returned.                                                    
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       driveno                             Drive number                
*       path                                Path name                   
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       Returns NU_NULL on a bad drive number otherwise a pointer to the   
*       first character in the rest of the path specifier.              
*                                                                       
*************************************************************************/
UINT8 *pc_parsedrive(INT16 *driveno, UINT8  *path)
{
UINT8       *p = path;
INT16       dno = -1;


    /* Get drive number. */
    if ( p && *p && (*(p+1) == ':') )
    {
        if ( ((*p) >= 'A') && ((*p) <= 'Z') )
            dno = (INT16) (*p - 'A');

        if ( ((*p) >= 'a') && ((*p) <= 'z') )
            dno = (INT16) (*p - 'a');

        /* Move to drive character and ':'. */
        p += 2;
    }
    else
        /* Get default_drive number.  */
        dno = NU_Get_Default_Drive();

    if ( (dno < 0) || (dno >= NDRIVES) )
        p = NU_NULL;
    else
    {
        /* Set drive number. */
        *driveno = dno;
    }
    return(p);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_parsenewname                                                 
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       If a wild card is used, setup the new file name.                      
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *pfi                                File directory entry        
*       **newname                           File name(wild card)        
*       **newext                            File extension(wild card)   
*       *fname                              File name buffer            
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       Always YES                          Success complete.          
*                                                                       
*************************************************************************/
INT pc_parsenewname(DROBJ *pobj, UINT8 *name, UINT8 *ext, 
                    VOID **new_name, VOID **new_ext, UINT8 *fname)
{
LNAMINFO    *linfo;
DOSINODE    pi;
UINT8       *p;
UINT8       *old;
UINT8       *op_nmend;
UINT8       *op_exend;
UINT8       old_name[EMAXPATH+1];
FINODE      *pfi;


    /* Initialize new name. */
    *new_name = *new_ext = NU_NULL;

    /* get long file name info */
    linfo = &pobj->linfo;

    /* Setup the old file name */
    if (linfo->lnament)             /* Long file name */
    {
        /* Convert directory entry long file name to character long file name. */
        pc_cre_longname((UINT8 *)old_name, linfo);
    }
    else                            /* Short file name */
    {
        /* get file directory entry */
        pfi = (FINODE *)pobj->finode;
        pc_ino2dos(&pi, pfi);
        /* Convert directory entry short file name to character short file name. */
        pc_cre_shortname((UINT8 *)old_name, pi.fname, pi.fext);
    }

    /* Mark the old file name end. */
    p = old_name;
    op_nmend = NU_NULL;
    while (*p)
    {
        if (*p == '.')
        {
            if (*(p+1) != '.')
                op_nmend = p;
        }
        p++;
    }
    op_exend = p;

    if (!op_nmend)
        op_nmend = op_exend;

   /* Setup the new file name pointer */
    *new_name = fname;

    /* Setup the new file name */
    p = name;
    old = old_name;

    while (*p)
    {
        if (*p == '*')
        {
            /* More of the new file name. */
            if (old < op_nmend)
            {
                while (old != op_nmend)
                {
                    *fname = *old++;
                    fname++;
                }
                break;
            }
        }
        else if (*p == '?')
        {
            *fname = *old;
        }
        else
        {
            *fname = *p;
        }

        fname++; old++; p++;

        /* File name end ? */
        if ( ext && (p == (ext-1)) )
            break;
    }


    /* Setup the new file extension. */
    if (ext)
    {
        /* file name .ext */
        *fname = '.'; fname++;
        p = ext;
        old = op_nmend + 1;

        /* Setup the new file extension pointer */
        *new_ext = fname;

        while (*p)
        {
            if (*p == '*')
            {
                /* More of the new file extension. */
                if (old < op_exend)
                {
                    while (old != op_exend)
                    {
                        *fname = *old++;
                        fname++;
                    }
                    break;
                }
            }
            else if (*p == '?')
            {
                *fname = *old;
            }
            else
            {
                *fname = *p;
            }

            fname++; old++; p++;
        }
    }
    *fname = '\0';

    return(YES);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_next_fparse                                                  
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Next upperbar file name.                                        
*       Create a filename~XXX.                                          
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       filename                            Upperbar file name.         
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       YES    Success complete.                                       
*       NO     Can't create new filename.                               
*                                                                       
*************************************************************************/
INT pc_next_fparse(UINT8 *filename)
{
UINT8       *pfr;
UINT8       *pupbar;
UINT8       *pnamend;
UINT32      tailno = 0L;
UINT32      temp;
UINT8       nm;
INT16       n = 0;
INT16       i;
STATUS      ret_stat = YES;

    /* Defaults */
    pfr = filename;
    pnamend = pupbar = 0;

    /* Mark the upperbar pointer and filenumber pointer. */
    while (*pfr)
    {
        if( pnamend == 0 )
        {

            if ( (*(pfr+1) == '\0') || (*(pfr+1) == ' ') )
                pnamend = pfr;
        }

        if (*pfr == '~')
            pupbar = pfr;

        pfr++;
        /* File name end ? */
        if (*pfr == '.')
            break;
    }

    /* Not use the filename upperbar and filenumber. */
    if ( (!pnamend) || (!pupbar) )
        ret_stat = NO;
    else
    {
        /* Take a filename number(~XXX) */
        pfr = pnamend;
        temp = 1L;
        while (pfr != pupbar)
        {
            tailno += (*pfr - '0') * temp;
            temp *= 10L;
            pfr--;
        }
        /* Next a filename number. */
        tailno +=1;

        /* How many single figures */
        temp = tailno;
        while (temp)
        {
            temp /= 10L;
            n++;
        }

        if ((pnamend-n) <= filename)
            ret_stat = NO;
        else
        {
            /* Set the upperbar mark. */
            *(pnamend-n) = '~';

            /* Set the next number. */
            temp = 1L;
            for (i = 0; i < n; i++)
            {
                nm = (UINT8) (((tailno%(temp*10))/temp) + '0');
                temp *= 10L;

                if (filename < (pnamend-i))
                    *(pnamend-i) = nm;
                else
                {
                    ret_stat = NO;
                    break;
                }
            }
        }
    }

    return(ret_stat);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_fileparse                                                    
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Copy the short file name and the short file extension to the    
*       buffer of an output from the start pointer of the file name and 
*       the extension. End each name with '\0'.  It will be              
*       completed unsuccessfully if the character that can not be used
*       in the file name is used. When the long file name uses more than
*       eight characters in the file name, uses lower case,             
*       uses ' = ' ' [ '' ] ' ' ; ' ' + ' ' , ' '  '  ' . ', or uses    
*       more than four characters in the extension, the short file name 
*       is created by using the first six characters of the long file   
*       name with an upper bar and 1 following it.  The upper function   
*       that calls this function has to prepare at least nine short    
*       file name buffer and four short file name extension buffer, both
*       in Char.                                                        
*                                                                       
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       filename                            File name                   
*       fileext                             File extension              
*       pfname                              File name pointer           
*       pfext                               File extension pointer      
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       2                                   File name is a long file name 
*                                            and uses the upperbar file  
*                                            name.                      
*       1                                   File name is a long file name 
*                                            and does not use the upperbar   
*                                            file name.                 
*       0                                   File name is a short file name.
*       NUF_INVNAME                         Path or filename includes   
*                                            invalid character.         
*                                                                       
*************************************************************************/
INT pc_fileparse(UINT8 *filename, UINT8 *fileext, VOID *pfname, VOID *pfext)
{
INT16       i;
UINT8       *fname;
UINT8       *fext;
INT         upbar;
INT         upbar_set;
INT         longfile;

    /* Defaults */
    pc_memfill(filename, 8, ' ');
    filename[8] = '\0';
    pc_memfill(fileext, 3, ' ');
    fileext[3] = '\0';

    upbar_set = 0;
    longfile = 0;

    /* Check the file name and file extension. */
    if (!pc_checkpath((UINT8 *)pfname, NO))
        return(NUF_INVNAME);

    if (!pc_checkpath((UINT8 *)pfext, NO))
        return(NUF_INVNAME);

    /* Setup pointer. */
    fname = (UINT8 *)pfname;
    fext = (UINT8 *)pfext;

    /* Special cases of . and .. */
    if (*fname == '.')
    {
        /* . */
        *filename = '.';

        /* .. */
        if (*(fname+1) == '.')
            *(++filename) = '.';

        return(0);
    }

    /* Check use of the upperbar.
        Note: filename~xxx */
    upbar = pc_use_upperbar(fname);

    /* Setup the short filename. */
    i = 1;
    while (*fname)
    {
        /* Use upperbar ? */
        if (upbar)
        {
            if ( (*fname != ' ') && (*fname != '.') )
            {
                if ( (*fname == '=') ||
                     (*fname == '[') ||
                     (*fname == ']') ||
                     (*fname == ';') ||
                     (*fname == '+') ||
                     (*fname == ',') )
                {
                    *filename++ = '_';
                }
                else
                {
                    if ( (*fname >= 'a') && (*fname <= 'z') )
                        *filename++ = (UINT8) ('A' + *fname - 'a');
                    else
                        *filename++ = *fname;
                }
                i++;
            }

            /* Filename end or upper 6 */
            if ( ((fname+2) == ((UINT8 *)pfext)) || (i > 6) )
            {
                *filename++ = '~';
                *filename = '1';

                upbar_set = 1;      /* upper bar set flag */

                break;
            }
        }
        else
        {
            if ( (*fname >= 'a') && (*fname <= 'z') )
            {
                *filename++ = (UINT8) ('A' + *fname - 'a');
                if (!longfile)
                     longfile = 1;
            }
            else
                *filename++ = *fname;
            i++;
            if (i > 8)
                break;
        }
        fname++;

        /* File name end ? */
        if ((fname+1) == ((UINT8 *)pfext))
            break;
    }

    /* Setup the short fileext. */
    if (fext)
    {
        i = 0;
        while (*fext)
        {
            if ( (*fext != ' ') && (*fext != '.') )
            {
                if (i++ < 3)
                {
                    if ( (*fext == '=') ||
                         (*fext == '[') ||
                         (*fext == ']') ||
                         (*fext == ';') ||
                         (*fext == '+') ||
                         (*fext == ',') )
                    {
                        *fileext++ = '_';
                    }
                    else if ( (*fext >= 'a') && (*fext <= 'z') )
                    {
                        *fileext++ = (UINT8) ('A' + *fext - 'a');
                        if (!longfile)
                             longfile = 1;
                    }
                    else
                        *fileext++ = *fext;
                }
            }
            fext++;
        }
    }
    else
    {
        if ( (upbar) && (!upbar_set) )
        {
            /* No fileextension */
            *filename++ = '~';
            *filename = '1';
        }
    }

    if (upbar)
        return(2);

    return(longfile);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_nibbleparse                                                  
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Nibble off the left most part of a path specifier              
*       Take a path specifier (no leading D:). and parse the left most 
*       element into filename and file ext. (SPACE right filled.).             
*                                                                       
*       Parse a path. Return NULL if problems or a pointer to the ""    
*       If input path is NULL, return NULL and set NO to stat.          
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       filename                            File name                   
*       fileext                             File extension              
*       path                                Path name                   
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       Returns a pointer to the rest of the path specifier beyond      
*       file.ext                                                        
*                                                                       
*************************************************************************/
UINT8 *pc_nibbleparse(UINT8 *topath)
{
UINT8       *p;
UINT8       *ppend;


    /* Defaults. */
    ppend = 0;
    p = topath;

    /* Mark the next backslash */
    while (*p) 
    {
        if (*p == BACKSLASH)
        {
            ppend = p;
            break;
        }
        p++;
    }

    /* Check the path end */
    if (!ppend)
        ppend = NU_NULL;
    else /* Return next path */
        ppend += 1;     

    return(ppend);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_parsepath                                                    
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Parse a path specifier into path,file,ext                       
*       Take a path specifier in path and break it into three null      
*       terminated strings "topath", "pfname", and "pfext".                
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       topath                              Path name pointer.          
*       pfname                              File name pointer           
*       pfext                               File extension pointer      
*       path                                Path name                   
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       NU_SUCCESS                          If service is successful.   
*       NUF_BADPARM                         Path is NULL.  
*       NUF_LONGPATH                        Path or filename too long.  
*       NUF_INVNAME                         Path or filename includes   
*                                            invalid character.         
*                                                                       
*                                                                       
*************************************************************************/
STATUS pc_parsepath(VOID **topath, VOID **pfname, VOID **pfext, UINT8 *path)
{
UINT8       *pfr;
UINT8       *pslash;
UINT8       *pperiod;
UINT8       *temp;
INT         colon;
UINT16      length;

    /* Path is NULL */
    if( ! path )
        return  NUF_BADPARM;
        
    /* Move path pointer to local */
    pfr = path;

    /* Initialize local variable */
    pslash = pperiod = NU_NULL;
    *topath = *pfname = *pfext = NU_NULL;

    /* Check is there colon? */
    if ( path[0] && path[1] == ':' )
    {
        colon = 1;
    }
    else
    {
        colon = 0;
    }

    /* Parse a path
        Mark the last backslash and last period */
    length = 1;
    
    /* Get last slash position and last period position and check length. */
    while (*pfr)
    {
        /* Is current pointer is slash ? */
        if (*pfr == '\\')
            pslash = pfr;

        /* Is current pointer is period ? */
        if (*pfr == '.')
        {
            /* Note: Not mark the next name period, backslash and NULL  */
            if ( (*(pfr+1) != '.') && (*(pfr+1) != '\\') && (*(pfr+1) != '\0') )
                pperiod = pfr;
        }
        pfr++;
        length++;
        /* Check length */
        if (length > EMAXPATH)
            return(NUF_LONGPATH);

    }


    /**** Setup the path pointer ****/
    if (pslash)  /* Is the path include slash ? */
    {
        /* Clear period if in middle of path */
        if(pslash > pperiod )
            pperiod = NU_NULL;

        /* Yes, we assume pathname is specified. */
        if (colon)
        {
            *topath = path + 2;
        }
        else
        {
            *topath = path;
        }
    }
    else
    {
        /* No slash */
        if (colon)
        {
            *pfname = path + 2;
        }
        else
        {
            *pfname = path;
        }

        /* Special cases of "." or  ".." */
        if ( ((**(UINT8 **)pfname == '.') && (*((*(UINT8 **)pfname)+1) == '\0')) ||
            ((**(UINT8 **)pfname == '.') && (*((*(UINT8 **)pfname)+1) == '.')) && (*((*(UINT8 **)pfname)+2) == '\0') )
        {
            /**topath = path;*/
            return(NU_SUCCESS);
        }
        else
            *topath = NU_NULL;
    }

    /**** Setup the filename pointer ****/
    if (pslash) /* The path include slash.*/
    {
        if ( *(pslash+1) != '\0' )
            *pfname = pslash + 1;
        else
            *pfname = NU_NULL;
    }
    else  /*  No slash */
    {
        if (colon)  /* C:filename...... */
        {
            if ( *(path+2) != '\0' )
                *pfname = path + 2;
            else
                *pfname = NU_NULL;
        }
        else    /* Only filename */
        {
            *pfname = path;
        }
    }

    /* Check the filename */
    if (*pfname)
    {
        temp = (UINT8 *)(*pfname);

        /* Special cases of .\ or ..\ or . or ..*/
        if (*temp == '.')
        {
            if (*(temp+1) == '.')
            {
                if ((*(temp+2) != '\\') && (*(temp+2) != '\0'))     /* ..\ or .. */
                    return(NUF_INVNAME);
            }
            else if (*(temp+1) == '\\')     /* .\ */
            {
                if (*(temp+2) != '\0')
                    return(NUF_INVNAME);
            }
            else                    /* . */
            {
                if (*(temp+1) != '\0')
                    return(NUF_INVNAME);
            }
        }
        else
        {
            /* Nothing file extension
                Delete filename period. filename..... */
            if (!pperiod)
            {
                while (*temp)
                {
                    /* Special cases of "*." */
                    if ( (*temp == '*') && (*(temp+1) == '.') )
                    {
                        temp++;     /* skip */
                    }
                    else if (*temp == '.')
                    {
                        /* filename...... -> filename */
                        if ( (*(temp+1) == '\0') || (*(temp+1) == '.') )
                        {
                            *temp = '\0';
                            break;
                        }
                    }
                    temp++;
                }
            }
        }
    }

    /* Check the path name */
    temp = (UINT8 *)(*topath);
    length = 0;
    if (*topath)
    {
        while (temp[length])
        {

            switch (temp[length])
            {
                case '.': /* Special cases of .\ or ..\ */
                {
                    if (temp[length + 1] == '.')
                    {
                        if (temp[length + 2] == '\\')
                            length += 1;
                        else
                            return(NUF_INVNAME);
                    }
                    else if (temp[length + 1] == '\\')
                    {
                        break;
                    }
                    break;
                }
                case '\\': /* \..\ or \.\ or \. or \..  */
                {
                    if (temp[length + 1] == '\\')
                    {
                        return(NUF_INVNAME);
                    }
                    break;
                }
                default:
                {
                    break;
                }
            }/* End switch */


            if ( (temp + length) == pslash)
            {
                break;
            }

            length++;

        } /* End While */
    }

    /**** Setup the file extension pointer ****/
    if ((*pfname) && pperiod)
    {
        /* Check the file extension
            Delete extension period.
            xxxx.extension...... -> extension */
        temp = pperiod + 1;
        while (*temp)
        {
            if (*temp == '.')
            {
                if ( (*(temp+1) == '\0') || (*(temp+1) == '.') )
                {
                    *temp = '\0';
                }
            }
            temp++;
        }

        *pfext = pperiod + 1;
    }
    else
    {
        *pfext = NU_NULL;
    }

    return(NU_SUCCESS);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_patcmp                                                      
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Compare "disk_fnam" and "in_fnam" in uppercase letter. Wild card is     
*       available.                                                      
*                                                                       
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *disk_fnam                          Pointer to filename in disk 
*       *in_fnam                            Pointer to filename which   
*                                            application specified.     
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       YES                                 compare match.              
*       NO                                  Not match.                  
*                                                                       
*                                                                       
*************************************************************************/
INT pc_patcmp(UINT8 *disk_fnam, UINT8 *in_fnam )
{
UINT8       c1, c2;
INT         pd;

    /* But E5 in the Pattern matches 0x5 */
    if (in_fnam == NU_NULL)
        return(NO);

    if (*in_fnam == PCDELETE)
    {
        if (*disk_fnam == 0x5)
        {
            disk_fnam++;
            in_fnam++;
        }
        else
            return(NO);
    }
        
    for (;;)
    {
        if (! *disk_fnam) /* End of the filename */
        {
            if ( (! *in_fnam ) || (*in_fnam == BACKSLASH) )
                return(YES);
            else
                return(NO);
        }
        if (*in_fnam == '*')    /* '*' matches the rest of the name */
        {
            in_fnam++;
            if (! *in_fnam)
                return(YES);

            pd = 0;
            /* Compare after wild card. */
            for (; *disk_fnam; disk_fnam++)
            {
                if (*disk_fnam == '.')
                    pd = 1;

                if (YES == pc_patcmp(disk_fnam, in_fnam))
                    return(YES);
            }
            if ( (*in_fnam == '.') && (*(in_fnam+1) == '*') && (*(in_fnam+2)=='\0') )
                return(YES);

            if ( (!pd) && (*in_fnam == '.') && (*(in_fnam+1)=='\0') )
                return(YES);

            return(NO);
        }
        if (*disk_fnam == '*')    /* '*' matches the rest of the name */
        {
            disk_fnam++;
            if (! *disk_fnam)
                return(YES);

            for (; *in_fnam; in_fnam++) /* Compare after wild card */
            {
                if (YES == pc_patcmp(in_fnam, disk_fnam) )
                    return(YES);
            }
            if ( (*disk_fnam == '.') && (*(disk_fnam+1) == '*') && (*(disk_fnam+2) =='\0') )
                return(YES);
            return(NO);
        }

        /* Convert to upper letter */
        if ( (*disk_fnam >= 'a') && (*disk_fnam <= 'z') )
            c1 = (UINT8) ('A' + *disk_fnam - 'a');
        else
            c1 = *disk_fnam;

        /* Convert to upper letter */
        if ( (*in_fnam >= 'a') && (*in_fnam <= 'z') )
            c2 = (UINT8) ('A' + *in_fnam - 'a');
        else
            c2 = *in_fnam;

        if (c1 != c2 )
            if (c2 != '?')  /* ? is wild card */
                return(NO);
        disk_fnam++;
        in_fnam++;
    }

}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_strcat                                                       
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       strcat                                                          
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       to                          To data buffer                      
*       from                        From data buffer                    
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       None.                                                           
*                                                                       
*************************************************************************/
VOID pc_strcat(UINT8 *to, UINT8 *from)
{

    while (*to)  to++;
    while (*from) *to++ = *from++;
    *to = '\0';
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_use_wdcard                                                    
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Check the use of wild card.                                        
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       code                                name                        
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       YES                                 Uses the wild card.          
*       NO                                  Does not use the wild card.      
*                                                                       
*************************************************************************/
INT pc_use_wdcard(UINT8 *code)
{
INT    ret_val = NO;

    while (*code)
    {
        if ( (*code == '*') || (*code == '?') )
        {
            ret_val = YES;
            break;
        }
        code++;
    }
    return(ret_val);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_use_upperbar                                                 
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Check the use of upperbar name.                                    
*       ' '(space), '.'(period), '=', '[', ']', ';', ':', '+', ','      
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       code                        File name or File extension         
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       YES                         Uses the upperbar name.      
*       NO                          Does not use the upperbar name.  
*                                                                       
*************************************************************************/
INT pc_use_upperbar(UINT8 *code)
{
UINT16      n, j;
UINT16      period = 0;
STATUS      ret_stat = NO;

    n = j = 0;

    while (*code)
    {
        switch (*code)
        {
        case ' ':
        case '=':
        case '[':
        case ']':
        case ';':
        case '+':
        case ',':
        {
            ret_stat = YES;
            break;
        }
        case '.':
        {
            /* filename.... or extension..... */
            if (*(code+1) == '\0')
            {
                period = j = 0;
            }
            else
            {
                period++;
            }
            break;
        }
        default:
            break;
        }

        /* Check the length of extension */
        if (period)
        {
            if (j++ > 3)
                ret_stat = YES;
        }
        /* Check the length of filename */
        else
        {
            n++;
            if (n > 8)
                ret_stat = YES;
        }
        if (ret_stat == YES)
            break;
        code++;
    }

    /* Another way to differentiate filename and extension ? */
    if (period > 1)
        ret_stat = YES;

    return(ret_stat);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_checkpath                                                    
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Check the path name or Volume label.                            
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       code                                Path name or Volume label.  
*       vol                                 YES : Volume label          
*                                           NO  : Path name             
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       YES                                 Success.                    
*       NO                                  Path name error.            
*                                                                       
*************************************************************************/
INT pc_checkpath(UINT8 *code, INT vol)
{
UINT16      n = 0;
INT         ret_val = YES;

    if (code)
    {
        while (code[n])
        {
            if ( (code[n] < 0x20) || (code[n] > 0x7E) )
            {
                ret_val = NO;
                break;
            }
            if (!vol)
            {
                if (code[n] == ':')
                {
                    if ( (n != 1) || (code[n+1] != '\\') )
                    {
                        ret_val = NO;
                        break;
                    }
                }

                if (code[n] == '\\')
                {
                    if (code[n+1] == '\\')
                    {
                        ret_val = NO;
                        break;
                    }
                }
            }
            else
            {
                switch (code[n])
                {
                case '.':
                case ':':
                case '=':
                case '[':
                case '\\':
                case ']':
                    ret_val = NO;
                    break;

                default:
                    break;
                }
                if (ret_val == NO)
                    break;
            }

            switch (code[n])
            {
            case '"':
            case '*':
            case ',':
            case '/':
            case ';':
            case '<':
            case '>':
            case '?':
            case '|':
                ret_val = NO;
                break;

            default:
                break;
            }
            if (ret_val == NO)
                break;

            n++;
        }
    }
    return(ret_val);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       _swap16                                                         
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Convert a 16 bit intel item to a portable 16 bit.               
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *to                                 After Convert data.         
*       *from                               Before Convert data.        
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       None.                                                           
*                                                                       
*************************************************************************/
VOID _swap16(UINT16 *to, UINT16 *from)
{
UINT8       *fptr;
UINT8       *tptr;


    fptr = (UINT8 *)from;
    tptr = (UINT8 *)to;

    *tptr = *(fptr + 1);
    *(tptr + 1) = *(fptr);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       _swap32                                                         
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Convert a 32 bit intel item to a portable 32 bit.               
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *to                                 After Convert data.         
*       *from                               Before Convert data.        
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       None.                                                           
*                                                                       
*************************************************************************/
VOID _swap32(UINT32 *to, UINT32 *from)
{
UINT8       *fptr;
UINT8       *tptr;


    fptr = (UINT8 *)from;
    tptr = (UINT8 *)to;

    *tptr = *(fptr + 3);
    *(tptr + 1) = *(fptr + 2);
    *(tptr + 2) = *(fptr + 1);
    *(tptr + 3) = *(fptr);

}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       _through16                                                      
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       16bit data byte through                                         
*                                                                       
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *to                                 After Convert data.         
*       *from                               Before Convert data.        
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       None.                                                           
*                                                                       
*************************************************************************/
VOID _through16(UINT16 *to, UINT16 *from)
{
UINT8       *fptr;
UINT8       *tptr;


    fptr = (UINT8 *)from;
    tptr = (UINT8 *)to;

    *tptr = *fptr;
    *(tptr + 1) = *(fptr + 1);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       _through32                                                      
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       32bit data byte through.                                        
*                                                                       
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *to                                 After Convert data.         
*       *from                               Before Convert data.        
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       None.                                                           
*                                                                       
*************************************************************************/
VOID _through32(UINT32 *to, UINT32 *from)
{
UINT8       *fptr;
UINT8       *tptr;


    fptr = (UINT8 *)from;
    tptr = (UINT8 *)to;

    *tptr = *fptr;
    *(tptr + 1) = *(fptr + 1);
    *(tptr + 2) = *(fptr + 2);
    *(tptr + 3) = *(fptr + 3);
}



/************************************************************************
* FUNCTION                                                              
*                                                                       
*       fswap16                                                         
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Swap the byte order of a Big Endian 2-byte value.               
*       Preseve the byte order of a Little Endian 2-byte value. 
*       This also does a byte to byte transfer to assure 
*       correct alignment.
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *to                                 After Converted data.         
*       *from                               Before Converted data.        
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       None.                                                           
*                                                                       
*************************************************************************/
void fswap16(UINT16 *to, UINT16 *from)
{

    UINT8       *fptr;
    UINT8       *tptr;

    UINT16  num;

    fptr = (UINT8 *)from;

    num = (UINT16)((fptr[1] << 8) + fptr[0]);

    fptr = (UINT8 *)&num;
    tptr = (UINT8 *)to;
    *tptr = *fptr;
    *(tptr+1) = *(fptr+1);


}

/************************************************************************
* FUNCTION                                                              
*                                                                       
*       fswap32                                                         
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Swap the byte order of a Big Endian 4-byte value.               
*       Preseve the byte order of a Little Endian 4-byte value. 
*       This also does a byte to byte transfer to assure 
*       correct alignment.
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       *to                                 After Converted data.         
*       *from                               Before Converted data.        
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       None.                                                           
*                                                                       
*************************************************************************/
void fswap32(UINT32 *to, UINT32 *from)
{

    UINT8       *fptr;
    UINT8       *tptr;

    UINT32  num;

    fptr = (UINT8 *)from;
    num = ((UINT32)fptr[3] << 24) + 
          ((UINT32)fptr[2] << 16) + 
          ((UINT32)fptr[1] << 8) + 
          ((UINT32)fptr[0]);
    fptr = (UINT8 *)&num;
    tptr = (UINT8 *)to;
    *tptr = *fptr;
    *(tptr+1) = *(fptr+1);
    *(tptr+2) = *(fptr+2);
    *(tptr+3) = *(fptr+3);


}


/****************************************************************************
 * Modifications:
 * $Log: 
 *  2    mpeg      1.1         4/2/04 8:17:48 PM      Nagaraja Kolur  CR(s) 
 *        8762 8763 : Merged 2.5.6 Nup File System.
 *        Test: 1. pvr demo 2. nup file test 3. CR 8181
 *        
 *  1    mpeg      1.0         8/22/03 6:21:00 PM     Dave Moore      
 * $
 * 
 *    Rev 1.0   22 Aug 2003 17:21:00   mooreda
 * SCR(s) 7350 :
 * Nucleus File sourcecode (File Utilities)
 * 
 *
 ****************************************************************************/

