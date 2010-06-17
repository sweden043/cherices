/****************************************************************************/
/*                            Conexant Systems                              */
/****************************************************************************/
/*                                                                          */
/* Filename:       eefs.c                                                   */
/*                                                                          */
/* Description:    EEPROM File System Driver Source file                    */
/*                                                                          */
/* Author:         Senthil Veluswamy                                        */
/*                                                                          */
/* Date:           02/08/00                                                 */
/*                                                                          */
/* Copyright Conexant Systems, 1999                                         */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/
/****************************************************************************
 * $Header: eefs.c, 11, 1/20/04 3:34:50 PM, Dave Wilson$
 ****************************************************************************/

/*****************/
/* Include Files */
/*****************/
#include <hwconfig.h>
#include "eefsx.h"
#include "eepriv.h"

size_t  guSizeFileSystem = EE_FS_SIZE;
u_int32 guAddrFileSystem = EE_FS_START_ADDR;

/************************/
/* Function definitions */
/************************/
bool ee_fs_init(char *strPath, int iFileSysId, char *strName){

    int iRetcode;

    // Initialise the low level EEPROM interface
    if(ee_init()){
    
        /* OpenTV fails to initialise any EE file system where the number of */
        /* sectors is greater than 256. Limit the size if we would otherwise */
        /* hit this constraint.                                              */
        guSizeFileSystem = min(EE_FS_SIZE, 256*EE_FS_SECTOR_SIZE);
        
        // Register the file system and access functions with OpenTV if the
        // low level driver initialised OK.

        iRetcode = ee_filesys_init( strPath, 
                                iFileSysId, 
                                strName, 
                                guSizeFileSystem,
                                EE_FS_SECTOR_SIZE, 
                                ee_fs_read, 
                                ee_fs_write, 
                                NULL);

        if(!iRetcode){
            return(TRUE);
        }
        else{
            return(FALSE);
        }
    }
    else{
        return(FALSE);
    }
}

/****************************************************************************/
/* File system read/write functions. These are thin wrappers around ee_read */
/* and ee_write which check that the address supplied is not within the     */
/* NVRAM private area of the device.                                        */
/****************************************************************************/
u_int16 ee_fs_read(u_int16 address, voidF buffer, u_int16 count, void* private){

   #ifndef DRIVER_INCL_NV_CMSG
    if(address<guSizeFileSystem)
    {
        return(ee_read(address+EE_FS_START_ADDR, buffer, count, private));
    }
   #endif /* DRIVER_INCL_NVCMSG */

    return(0);
}

u_int16 ee_fs_write(u_int16 address, voidF buffer, u_int16 count, void* private){

   #ifndef DRIVER_INCL_NV_CMSG
    if(address<guSizeFileSystem)
    {
        return(ee_write(address+EE_FS_START_ADDR, buffer, count, private));
    }
   #endif /* DRIVER_INCL_NVCMSG */

    return(0);
}

/****************************************************************************
* $Log: 
*  11   mpeg      1.10        1/20/04 3:34:50 PM     Dave Wilson     CR(s) 8250
*         8249 : Fixed address validity checks in ee_fs_read and ee_fs_write.
*        
*  10   mpeg      1.9         1/23/03 4:30:34 PM     Dave Wilson     SCR(s) 
*        5102 :
*        
*        
*        
*        
*        
*        Limited the size of the EE file system so that it does not occupy more
*         than
*        256 sectors. This seems to be a hard limit for OpenTV since the file 
*        system
*        registration call fails in these cases.
*        
*  9    mpeg      1.8         6/17/02 5:25:42 PM     Senthil Veluswamy SCR(s) 
*        3955 :
*        wrapped fucntion code to remove warnings when EEFS is not allocated 
*        any bytes
*        
*  8    mpeg      1.7         6/6/02 6:51:06 PM      Senthil Veluswamy SCR(s) 
*        3955 :
*        fix to remove eefs build warnings
*        
*  7    mpeg      1.6         4/20/00 1:08:18 PM     Senthil Veluswamy added 
*        "eefsx.h"
*        made changes to reserve space for Config data (NVSTORE) in EE
*        
*  6    mpeg      1.5         4/11/00 5:44:56 PM     Senthil Veluswamy no 
*        change
*        
*  5    mpeg      1.4         3/31/00 9:43:10 AM     Dave Wilson     Added 
*        hwconfig.h to get rid of compiler warnings.
*        
*  4    mpeg      1.3         2/23/00 5:08:00 PM     Senthil Veluswamy moved 
*        the r/w function prototypes to the private header file
*        
*  3    mpeg      1.2         2/18/00 3:03:50 PM     Dave Wilson     Added fake
*         RAM support to aid integration.
*        Added wrapper functions to prevent o-code access to private data area
*        
*  2    mpeg      1.1         2/8/00 8:16:38 PM      Senthil Veluswamy 
*        populated init
*        
*  1    mpeg      1.0         2/8/00 6:45:46 PM      Senthil Veluswamy 
* $
 * 
 *    Rev 1.9   23 Jan 2003 16:30:34   dawilson
 * SCR(s) 5102 :
 * 
 * 
 * 
 * 
 * 
 * Limited the size of the EE file system so that it does not occupy more than
 * 256 sectors. This seems to be a hard limit for OpenTV since the file system
 * registration call fails in these cases.
 * 
 *    Rev 1.8   17 Jun 2002 16:25:42   velusws
 * SCR(s) 3955 :
 * wrapped fucntion code to remove warnings when EEFS is not allocated any bytes
 ****************************************************************************/
