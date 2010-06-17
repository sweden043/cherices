/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                 Conexant Systems Inc. (c) 2003                           */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       devtable.c
 *
 *
 * Description:    see ATI header below.
 *                 Initial version based on Nucleus File v2.4.4
 *
 *
 ****************************************************************************/
/* $Header: devtable.c, 2, 4/2/04 11:38:08 PM, Nagaraja Kolur$
 ****************************************************************************/

/************************************************************************
*                                                                       
*               Copyright Mentor Graphics Corporation 2003              
*                         All Rights Reserved.                          
*                                                                       
* THIS WORK CONTAINS TRADE SECRET AND PROPRIETARY INFORMATION WHICH IS  
* THE PROPERTY OF MENTOR GRAPHICS CORPORATION OR ITS LICENSORS AND IS   
* SUBJECT TO LICENSE TERMS.                                             
*                                                                       
*                                                                       
*************************************************************************

*************************************************************************
* FILE NAME                                       VERSION                
*                                                                       
*       devtable.c                        Nucleus FILE\ARM7TDMI\ADS 2.5.6 
*                                                                       
* COMPONENT                                                             
*                                                                       
*       Nucleus FILE                                                    
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       User supplied Device driver.                                    
*                                                                       
*       When you have your device drivers written plug them into this   
*       table.                                                          
*                                                                       
*                                                                       
* DATA STRUCTURES                                                       
*                                                                       
*       _PC_BDEVSW                          Device driver dispatch table.
*       NUF_Drive_Fat_Size                  Drive FAT bufffer size list.
*                                                                       
* FUNCTIONS                                                             
*                                                                       
*       nodev_ioctl                         Dummy device I/O control.   
*       nodev_init_drive                    Dummy device init drive.    
*       nodev_open                          Dummy device open.          
*       nodev_raw_open                      Dummy device raw open.      
*       nodev_close                         Dummy device close.         
*       nodev_io                            Dummy device I/O.           
*                                                                       
* DEPENDENCIES                                                          
*                                                                       
*       pcdisk.h                            File common definitions     
*                                                                       
* NOTE: This module is linked in with File.lib.  After you make   
*       changes, you must rebuild File.lib.                       
*************************************************************************/
#include "retcodes.h"
#include "stbcfg.h"
#include "kal.h"
#include "trace.h"

#include "pcdisk.h"
#include "ata.h" /* conexant ata driver */

/* Used to prevent compiler warnings */
extern UINT32 FILE_Unused_Param;

/* External device functions. Replace these with your drivers. */
/* RAMDISK, EBS_FLOPPY, etc are defined in pcdisk.h            */

/* Ramdisk */
#if (RAMDISK)
extern INT pc_rd_open(UINT16 driveno);
extern INT pc_rd_raw_open(UINT16 driveno);
extern INT pc_rd_close(UINT16 driveno);
extern INT pc_rd_io(UINT16 driveno, UINT32 block, VOID FAR *buffer, UINT16 count, INT do_read);
extern INT pc_rd_ioctl(UINT16 driveno, UINT16 command, VOID *buffer);
#endif


/* Bios */
#if ( (EBS_FLOPPY == 0) && (EBS_IDE == 0) )
extern INT bios_open(UINT16 driveno);
extern INT bios_raw_open(UINT16 driveno);
extern INT bios_close(UINT16 driveno);
extern INT bios_io(UINT16 driveno, UINT32 block, VOID FAR *buffer, UINT16 count, INT do_read);
extern INT bios_ioctl(UINT16 driveno, UINT16 command, VOID *buffer);
#endif


/* ide 
#if (EBS_IDE)
extern INT ide_ioctl(UINT16 driveno, UINT16 command, VOID *buffer);
extern INT ide_init_drive(INT16 driveno);
extern INT ide_open(UINT16 driveno);
extern INT ide_raw_open(UINT16 driveno);
extern INT ide_close(UINT16 driveno);
extern INT ide_io(UINT16 driveno, UINT32 sector, VOID FAR *buffer, UINT16 count, INT reading);
#endif
*/

#if (EBS_IDE) /* CNXT version */
extern INT cnxt_ide_ioctl(UINT16 driveno, UINT16 command, VOID *buffer);
extern INT cnxt_ide_init_drive(INT16 driveno);
extern INT cnxt_ide_open(UINT16 driveno);
extern INT cnxt_ide_raw_open(UINT16 driveno);
extern INT cnxt_ide_close(UINT16 driveno);
extern INT cnxt_ide_io(UINT16 driveno, UINT32 sector, VOID FAR *buffer, UINT16 count, INT reading);
#endif

#if (defined(USB_IDE)&&(USB_IDE))/* CNXT version */
extern INT cnxt_usb_ioctl(UINT16 driveno, UINT16 command, VOID *buffer);
extern INT cnxt_usb_open(UINT16 driveno);
extern INT cnxt_usb_raw_open(UINT16 driveno);
extern INT cnxt_usb_close(UINT16 driveno);
extern INT cnxt_usb_io(UINT16 driveno, UINT32 sector, VOID FAR *buffer, UINT16 count, INT reading);
#endif

/* floppy */
#if (EBS_FLOPPY)
extern INT floppy_ioctl(UINT16 driveno, UINT16 command, VOID *buffer);
extern INT floppy_init_drive(INT16 driveno);
extern INT floppy_open(UINT16 driveno);
extern INT floppy_raw_open(UINT16 driveno);
extern INT floppy_close(UINT16 driveno);
extern INT floppy_io(UINT16 driveno, UINT32 sector, VOID FAR *buffer, UINT16 count, INT reading);
#endif


/* Dummy device - These functions are never called */
INT nodev_ioctl(UINT16 driveno, UINT16 command, VOID *buffer) 
{
    FILE_Unused_Param = (UINT32)driveno;
    FILE_Unused_Param = (UINT32)command;
    FILE_Unused_Param = (UINT32)buffer;
    return(NO); 
}
INT nodev_init_drive(INT16 driveno) 
{
    FILE_Unused_Param = (UINT32)driveno;
    return(NO); 
}
INT nodev_open(UINT16 driveno) 
{ 
    FILE_Unused_Param = (UINT32)driveno;
    return(NO); 
}
INT nodev_raw_open(UINT16 driveno) 
{ 
    FILE_Unused_Param = (UINT32)driveno;
    return(NO); 
}
INT nodev_close(UINT16 driveno) 
{
    FILE_Unused_Param = (UINT32)driveno;
    return(NO); 
}
INT nodev_io(UINT16 driveno, UINT32 sector, VOID FAR *buffer, UINT16 count, INT reading) 
{ 
    FILE_Unused_Param = (UINT32)driveno;
    FILE_Unused_Param = sector;
    FILE_Unused_Param = (UINT32)buffer;
    FILE_Unused_Param = (UINT32)count;
    FILE_Unused_Param = (UINT32)reading;
    return(NO); 
}


    /* Note:
        The first field in each of these records (0,0,0,0,4) is the lowest
        drive number that shares the same lock. In the bios driver, only one
        device may be accessed at a time so all locks are done on drive "A:"
        0. */


#if (RAMDISK)
/* ============================= RAMDISK ============================= */
_PC_BDEVSW pc_bdevsw[] =
{
    {0,  pc_rd_open, pc_rd_raw_open, pc_rd_close, pc_rd_io, pc_rd_ioctl,(INT(*)(UINT16))0}          /* C: */
};

INT  NUF_Drive_Fat_Size[NDRIVES] =
{ 
    NUF_FATSIZE_A
};

/* #elif (IDE_ATA)
 ============================ IDE_ATA disk ============================ 
_PC_BDEVSW pc_bdevsw[] =
{
    {0,  nodev_open, nodev_raw_open, nodev_close, nodev_io, nodev_ioctl, (INT(*)(UINT16))0},        // A: //
    {1,  nodev_open, nodev_raw_open, nodev_close, nodev_io, nodev_ioctl, (INT(*)(UINT16))0},        // B: //
    {2,  ide_open, ide_raw_open, ide_close, ide_io, ide_ioctl, (INT(*)(UINT16))0},                  // C: //
    {2,  ide_open, ide_raw_open, ide_close, ide_io, ide_ioctl, (INT(*)(UINT16))0},                  // D: //
    {2,  ide_open, ide_raw_open, ide_close, ide_io, ide_ioctl, (INT(*)(UINT16))0},                  // E: //
    {2,  ide_open, ide_raw_open, ide_close, ide_io, ide_ioctl, (INT(*)(UINT16))0},                  // F: //
    {5,  nodev_open, nodev_raw_open, nodev_close, nodev_io, nodev_ioctl, (INT(*)(UINT16))0}         // G: //
};

INT  NUF_Drive_Fat_Size[NDRIVES] =
{ 
    NUF_FATSIZE_A,
    NUF_FATSIZE_B,
    NUF_FATSIZE_C,
    NUF_FATSIZE_D,
    NUF_FATSIZE_E,
    NUF_FATSIZE_F
};
*/

#elif (IDE_ATA) /* CNXT VERSION */

_PC_BDEVSW pc_bdevsw[] =
{
    {0,  nodev_open, nodev_raw_open, nodev_close, nodev_io, nodev_ioctl, (INT(*)(UINT16))0},        /* A: */
    {1,  nodev_open, nodev_raw_open, nodev_close, nodev_io, nodev_ioctl, (INT(*)(UINT16))0},        /* B: */
    {2,  cnxt_ide_open, cnxt_ide_raw_open, cnxt_ide_close, cnxt_ide_io, cnxt_ide_ioctl, (INT(*)(UINT16))0}, /* C: */
};

INT  NUF_Drive_Fat_Size[NDRIVES] =
{ 
    NUF_FATSIZE_A,
    NUF_FATSIZE_B,
    NUF_FATSIZE_C,
};
#elif (IDE_USB) /* CNXT VERSION */

_PC_BDEVSW pc_bdevsw[] =
{
    {0,  nodev_open, nodev_raw_open, nodev_close, nodev_io, nodev_ioctl, (INT(*)(UINT16))0},        /* A: */
    {1,  nodev_open, nodev_raw_open, nodev_close, nodev_io, nodev_ioctl, (INT(*)(UINT16))0},        /* B: */
    {2,  cnxt_usb_open, cnxt_usb_raw_open, cnxt_usb_close, cnxt_usb_io, cnxt_usb_ioctl, (INT(*)(UINT16))0}, /* C: */
};

INT  NUF_Drive_Fat_Size[NDRIVES] =
{ 
    NUF_FATSIZE_A,
    NUF_FATSIZE_B,
    NUF_FATSIZE_C,
};

#elif (IDE_PCM)
/* ============================= IDE w/PCMCIA card ============================= */
_PC_BDEVSW pc_bdevsw[] =
{
    {0,  nodev_open, nodev_raw_open, nodev_close, nodev_io, nodev_ioctl, (INT(*)(UINT16))0},        /* A: */
    {1,  nodev_open, nodev_raw_open, nodev_close, nodev_io, nodev_ioctl, (INT(*)(UINT16))0},        /* B: */
    {2,  ide_open, ide_raw_open, ide_close, ide_io, ide_ioctl, (INT(*)(UINT16))0},                  /* C: */
    {3,  ide_open, ide_raw_open, ide_close, ide_io, ide_ioctl, (INT(*)(UINT16))0},                  /* D: */
    {4,  nodev_open, nodev_raw_open, nodev_close, nodev_io, nodev_ioctl, (INT(*)(UINT16))0},        /* E: */
    {5,  nodev_open, nodev_raw_open, nodev_close, nodev_io, nodev_ioctl, (INT(*)(UINT16))0},        /* F: */
    {6,  nodev_open, nodev_raw_open, nodev_close, nodev_io, nodev_ioctl, (INT(*)(UINT16))0}         /* G: */
};

INT  NUF_Drive_Fat_Size[NDRIVES] =
{ 
    NUF_FATSIZE_A,
    NUF_FATSIZE_B,
    NUF_FATSIZE_C,
    NUF_FATSIZE_D,
    NUF_FATSIZE_E,
    NUF_FATSIZE_F
};

#elif (EBS_FLOPPY)
/* ============================= Floppy ============================ */
_PC_BDEVSW pc_bdevsw[] =
{
    {0,  floppy_open, floppy_raw_open, floppy_close, floppy_io, floppy_ioctl, (int(*)(UINT16))0}   /* A: */
};

INT  NUF_Drive_Fat_Size[NDRIVES] =
{ 
    NUF_FATSIZE_A
};
#endif/* ================ Device Select ================ */

#if (0)
/* ============================= User port ============================ */
_PC_BDEVSW pc_bdevsw[] =
{
    {0,  floppy_open, floppy_raw_open, floppy_close, floppy_io, floppy_ioctl, (INT(*)(UINT16))0},   /* A: */
    {1,  nodev_open, nodev_raw_open, nodev_close, nodev_io, nodev_ioctl, (INT(*)(UINT16))0},        /* B: */
    {2,  ide_open, ide_raw_open, ide_close, ide_io, ide_ioctl, (INT(*)(UINT16))0},                  /* C: */
    {2,  ide_open, ide_raw_open, ide_close, ide_io, ide_ioctl, (INT(*)(UINT16))0},                  /* D: */
    {2,  ide_open, ide_raw_open, ide_close, ide_io, ide_ioctl, (INT(*)(UINT16))0},                  /* E: */
    {4,  ide_open, ide_raw_open, ide_close, ide_io, ide_ioctl, (INT(*)(UINT16))0},                  /* F: */
    {5,  nodev_open, nodev_raw_open, nodev_close, nodev_io, nodev_ioctl, (INT(*)(UINT16))0}         /* G: */
};

INT  NUF_Drive_Fat_Size[NDRIVES] =
{ 
    NUF_FATSIZE_A,
    NUF_FATSIZE_B,
    NUF_FATSIZE_C,
    NUF_FATSIZE_D,
    NUF_FATSIZE_E,
    NUF_FATSIZE_F
};
#endif


/****************************************************************************
 * Modifications:
 * $Log: 
 *  2    mpeg      1.1         4/2/04 11:38:08 PM     Nagaraja Kolur  CR(s) 
 *        8762 8763 : Merged 2.5.6 Nup File System.
 *        Test: 1. pvr demo 2. nup file test 3. CR 8181
 *        
 *  1    mpeg      1.0         8/22/03 5:29:08 PM     Dave Moore      
 * $
 * 
 *    Rev 1.0   22 Aug 2003 16:29:08   mooreda
 * SCR(s) 7350 :
 * Nucleus File sourcecode (Driver Dispatch Table)
 * 
 *
 ****************************************************************************/

