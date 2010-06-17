/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                 Conexant Systems Inc. (c) 2003                           */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
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
/************************************************************************
*
*  FILE NAME                                  VERSION
*
*      fileinit.c                           Nucleus FILE\ARM7TDMI\ADS 2.5.6 
*
*  COMPONENT
*
*      Nucleus FILE
* Description:    see ATI header below.
*                 Initial version based on Nucleus File v2.4.4
*
*
****************************************************************************/
/* $Header: fileinit.c, 4, 4/3/04 1:37:28 AM, Nagaraja Kolur$
****************************************************************************/

/*******************************************************************/
/*                                                                 */
/* File Initialization Module                                      */
/*                                                                 */
/*                                                                 */
/* This module contains functions used to initialize the hardware  */
/* in your system.  It must be modified for your individual setup. */
/* The function file_init() must be called from a task to setup    */
/* the file system before any other tasks attempt to use it.       */
/* The task that calls file_init() will automatically be setup     */
/* as a FILE User.                                                 */
/*                                                                 */
/* NOTE: This module is linked in with File.lib.  After you make   */
/*       changes, you must rebuild File.lib.                       */
/*                                                                 */
/*  FUNCTIONS                                                      */ 
/*                                                                 */ 
/*      file_init                     Initialize the file system   */
/*      setup_ramdisk                 Initialize and format ramdisk*/
/*      setup_disk                    Sets up non-volatile device  */        
/*                                                                 */
/*                                                                 */ 
/*  DEPENDENCIES                                                   */       
/*                                                                 */      
/*       pcdisk.h                       File common definitions    */ 
/*                                                                 */
/*                                                                 */
/*******************************************************************/

/* Include necessary Nucleus PLUS files.  */
#include  "nucleus.h"
#include  "pcdisk.h"
#include  "file_mmu.h"
/*cnxt*/
#include  "retcodes.h"
#include  "stbcfg.h"
#include  "kal.h"
#include  "ata.h"

/* These #defines are used for initialization */
#define         RAM_DRIVE    0       /* 0  */
#define         RAM_DISK     "A:"    /* A: */  
#define         HARD_DRIVE    2       /* 2  */
#define         HARD_DISK     "C:"    /* C: */  

/* Function prototypes */
#if RAMDISK           /* Defined in pcdisk.h */
STATUS setup_ramdisk(VOID);
#endif
#if IDE_ATA           /* Defined in pcdisk.h */
STATUS setup_disk(VOID);
extern INT   pc_get_ataparams(INT16 driveno, FMTPARMS *pfmt);
#endif
#if IDE_PCM
extern STATUS   init_controller(void);
extern STATUS   init_card(UNSIGNED);
#endif           
#if EBS_FLOPPY
extern STATUS init_floppy_timer(void);
#define         FLOP_DRIVE    0       /* 0  */
#define         FLOP_DISK     "A:"    /* A: */  
#endif

/************************************************************************/
/*                                                                      */
/*  FUNCTION                           "file_init"                      */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function is responsible for initializing the file system   */
/*      This module needs to be modified to reflect the actual          */
/*      hardware present in your system.                                */
/*                                                                      */
/*  ROUTINES CALLED                                                     */
/*                                                                      */
/*      NU_Become_File_User                 Registers this task as a    */
/*                                          user of the file system.    */
/*      setup_ramdisk                       Sets up the ramdisk         */
/*      setup_disk                          Sets up a hard disk.        */
/*                                          on the disk.                */
/*                                                                      */
/*  INPUTS                                                              */
/*       NONE.                                                          */
/*                                                                      */
/*  OUTPUTS                                                             */
/*       status                             Status from NU_Open_Disk or */
/*                                          Status from NU_Format       */
/*                                                                      */
/************************************************************************/
STATUS   file_init(VOID)
{
    volatile STATUS  status;                 /* Return value from funtion calls */


    /* Disable time slice until disk is set up */
    /*   preempt_status = NU_Change_Preemption(NU_NO_PREEMPT); */
    NU_Change_Preemption(NU_NO_PREEMPT);

    /* Initialize memory. This only needs to be done once system wide */
    if (!pc_memory_init())
    {
        return(NUF_INTERNAL);
    }
    
    /* Each task must register as a File User user */
    if (!NU_Become_File_User())
    {
        
        return (NUF_BAD_USER);
    }

#if RAMDISK
    /* Set up the ramdisk for use */
    status = setup_ramdisk();

    if(status == NU_SUCCESS)
        status = NU_Set_Default_Drive(RAM_DRIVE);

    if(status == NU_SUCCESS)
        status = NU_Set_Current_Dir(RAM_DISK);
#endif
    
#if IDE_ATA
    /* Set up the hard disk for use */
    status = setup_disk();

    if(status == NU_SUCCESS)
        status = NU_Set_Default_Drive(HARD_DRIVE);

    if(status == NU_SUCCESS)
        status = NU_Set_Current_Dir(HARD_DISK);
#endif
    
#if IDE_PCM


    /* Initialize the controller hardware */
    status = init_controller();

    /* Return error */
    if(status != NU_SUCCESS)
        return(status);

    /* Setup the card in slot 0 for operation */
    status = init_card(0);

    /* Return error */
    if(status != NU_SUCCESS)
        return(status);


    status = NU_Open_Disk(HARD_DISK); /* Open disk for use */

	status = NU_Set_Default_Drive(HARD_DRIVE);

	status = NU_Set_Current_Dir(HARD_DISK);

#endif

#if EBS_FLOPPY


    /* Open the disk for use  */
    status = NU_Open_Disk("FLOP_DISK"); 


#endif

    /* Enable time slice */
    NU_Change_Preemption(NU_PREEMPT);

    return(status);



}


#if RAMDISK
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                           "setup_ramdisk"                    */
/*                                                                        */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*      This task will set up the format parameters and then              */
/*      call NU_Format() to format the ramdisk.                           */
/*                                                                        */
/*  CALLED FROM                                                           */
/*                                                                        */
/*      file_init                                                         */
/*                                                                        */
/*  ROUTINES CALLED                                                       */
/*                                                                        */
/*      NU_Format                           Formats the disk              */
/*                                          user of the file system.      */
/*                                          on the disk.                  */
/*                                                                        */
/*  INPUTS                                                                */
/*       driveno                            The number of the             */
/*                                          drive to format               */
/*  OUTPUTS                                                               */
/*       NU_SUCCESS                          If the filesystem disk was   */
/*                                            successfully initialized.   */
/*       NUF_BAD_USER                        Not a file user.             */
/*       NUF_BADDRIVE                        Invalid drive specified.     */
/*       NUF_NOT_OPENED                      The disk is not opened yet.  */
/*       NUF_FATCORE                         Fat cache table too small.   */
/*       NUF_BADPARM                         Invalid parameter specified. */
/*       NUF_BADDISK                         Bad Disk.                    */
/*       NUF_NO_PARTITION                    No partition in disk.        */
/*       NUF_NOFAT                           No FAT type in this          */
/*                                            partition.                  */
/*       NUF_FMTCSIZE                        Too many clusters for this   */
/*                                            partition.                  */
/*       NUF_FMTFSIZE                        File allocation table too    */
/*                                            small.                      */
/*       NUF_FMTRSIZE                        Numroot must be an even      */
/*                                            multiple of 16.             */
/*       NUF_INVNAME                         Volume lable includes        */
/*                                            invalid character.          */
/*       NUF_NO_MEMORY                       Can't allocate internal      */
/*                                            buffer.                     */
/*       NUF_NO_BLOCK                        No block buffer available.   */
/*       NUF_IO_ERROR                        Driver IO error.             */
/*       NUF_INTERNAL                        Nucleus FILE internal error. */
/*                                                                        */
/**************************************************************************/ 
STATUS setup_ramdisk(VOID)
{
STATUS          status;
FMTPARMS    fmt;


    /* Since the ramdisk is not yet formatted, this call will      */
    /* return an NUF_FORMAT error.  This is normal operation.      */
    /* After the ramdisk is formatted, other calls to NU_Open_Disk */
    /* will return an NU_SUCCES                                    */
    status = NU_Open_Disk(RAM_DISK); /* Open the ramdisk for use */

    /* Keep compilers happy */
    if(status != NU_SUCCESS)
    {

    /* Setup Format parameters */
    fmt.oemname[0] = 'N';
    fmt.oemname[1] = 'U';
    fmt.oemname[2] = 'F';
    fmt.oemname[3] = 'I';
    fmt.oemname[4] = 'L';
    fmt.oemname[5] = 'E';
    fmt.oemname[6] = ' ';
    fmt.oemname[7] = ' ';
    fmt.oemname[8] = '\0';
    fmt.secpalloc =      (UINT8)  2; 
                                        /* must be a multiple of 2.  This
                                          number indicates the number of
                                          sectors (blocks) per cluster.
                                          This value is the minimum number
                                          of bytes that are written to or
                                          read from the disk at a time.  */
    fmt.secreserved =    (UINT16) 1;
    fmt.numfats     =    (UINT8)  1;
                                        /* since redundant fats aren't used
                                           only have one fat.  */
    fmt.numroot     =    (UINT16) 64;   /* number of files in root */
    fmt.bytepsec    =    (UINT16) 512;
    fmt.mediadesc   =    (UINT8)  0xFD;
    fmt.partdisk    =    (UINT8)0;
                                         /* No partition */
    fmt.secptrk     =    (UINT16) NRAMDISKBLOCKS/4;
    fmt.numhead     =    (UINT16) 2;
    fmt.numcyl      =    (UINT16) 2;
    fmt.physical_drive_no =   0;
    fmt.binary_volume_label = 0x12345678L;
    fmt.text_volume_label[0] = NU_NULL;    /* 11 chars max */


    /* Format the Ram Disk */
    status = NU_Format(RAM_DRIVE, &fmt); 
    }
    
    /* Return status from NU_Format() */
    return(status);

}
#endif

#if IDE_ATA


INT pc_get_ataparams( INT16 driveno, FMTPARMS *pfmt )
{

    extern ata_query_t q_buf;

    /* Setup Format parameters */
    pfmt->oemname[0] = 'N';
    pfmt->oemname[1] = 'U';
    pfmt->oemname[2] = 'F';
    pfmt->oemname[3] = 'I';
    pfmt->oemname[4] = 'L';
    pfmt->oemname[5] = 'E';
    pfmt->oemname[6] = ' ';
    pfmt->oemname[7] = ' ';
    pfmt->oemname[8] = '\0';
	/* WARNING!! DO NOT set secpalloc > 64! Nucleus File breaks!! - DAVEM */

    pfmt->secpalloc =  (UINT8)64;       /* Cluster Size in #512 byte sectors.       */ 
	                                     /* Value must be a multiple of 2.           */
                                        /* 128 is maximum -> 64K cluster            */
                                        /*          64*512 = 32K (max) per FAT spec */
									  	 /*          32*512 = 16K (what most tools   */
										 /*                   seem to use)           */

    pfmt->secreserved =    (UINT16)32;   /* Typically 32 for FAT32 volumes per MS spec */
    pfmt->numfats     =    (UINT8)2;     /* "2" recommended by Microsoft spec */
    pfmt->numroot     =    (UINT16)0;    /* number of files in root, for FAT32 MBZ */
    pfmt->bytepsec    =    (UINT16)512;  /* bytes per sector */
    pfmt->mediadesc   =    (UINT8)0xF8;  /* 0xF8 == Fixed Disk */
    pfmt->partdisk    =    (UINT8)1;     /* We create Primary DOS partition (only) */
    pfmt->secptrk     =    (UINT16)q_buf.param.sectors; 
    pfmt->numhead     =    (UINT16)q_buf.param.heads; 
    pfmt->numcyl      =    (UINT16)q_buf.param.cylinders; 
    pfmt->totalsec    =    (UINT32)(q_buf.param.sectors1 << 16) | (UINT32)(q_buf.param.sectors0) ;
    pfmt->physical_drive_no =   0x80;    /* boot drive */
    pfmt->binary_volume_label = 0x12345678L;

    pfmt->text_volume_label[0] = 'C';
    pfmt->text_volume_label[1] = 'N';
    pfmt->text_volume_label[2] = 'X';
    pfmt->text_volume_label[3] = 'T';
    pfmt->text_volume_label[4] = '_';
    pfmt->text_volume_label[5] = 'P';
    pfmt->text_volume_label[6] = 'V';
    pfmt->text_volume_label[7] = 'R';
    pfmt->text_volume_label[8] = '\0';    /* 11 chars max */


   /* ----------------- Partition Type values -----------------
               0x01     12-bit FAT.
               0x04     16-bit FAT. Partition smaller than 32MB.
               0x06     16-bit FAT. Partition larger than or equal to 
                        32MB.
               0x0E     Same as PART_DOS4_FAT(06h),
                        but uses Logical Block Address Int 13h 
                        extensions.
               0x0B     32-bit FAT. Partitions up to 2047GB.
               0x0C     Same as PART_DOS32(0Bh), 
                        but uses Logical Block Address Int 13h 
                        extensions.
      -------------------------------------------------------- */

    return( YES );
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                           "setup_disk"                       */
/*                                                                        */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*      This task will set up the format parameters and then              */
/*      call NU_Format() to format the hard disk if necessary             */
/*                                                                        */
/*  CALLED FROM                                                           */
/*                                                                        */
/*      file_init                                                         */
/*                                                                        */
/*  ROUTINES CALLED                                                       */
/*                                                                        */
/*      NU_Format                           Formats the disk              */
/*                                          user of the file system.      */
/*                                          on the disk.                  */
/*                                                                        */
/*  INPUTS                                                                */
/*       driveno                            The number of the             */
/*                                          drive to format               */
/*  OUTPUTS                                                               */
/*       NU_SUCCESS                          If the filesystem disk was   */
/*                                            successfully initialized.   */
/*       NUF_BAD_USER                        Not a file user.             */
/*       NUF_BADDRIVE                        Invalid drive specified.     */
/*       NUF_NOT_OPENED                      The disk is not opened yet.  */
/*       NUF_FATCORE                         Fat cache table too small.   */
/*       NUF_BADPARM                         Invalid parameter specified. */
/*       NUF_BADDISK                         Bad Disk.                    */
/*       NUF_NO_PARTITION                    No partition in disk.        */
/*       NUF_NOFAT                           No FAT type in this          */
/*                                            partition.                  */
/*       NUF_FMTCSIZE                        Too many clusters for this   */
/*                                            partition.                  */
/*       NUF_FMTFSIZE                        File allocation table too    */
/*                                            small.                      */
/*       NUF_FMTRSIZE                        Numroot must be an even      */
/*                                            multiple of 16.             */
/*       NUF_INVNAME                         Volume lable includes        */
/*                                            invalid character.          */
/*       NUF_NO_MEMORY                       Can't allocate internal      */
/*                                            buffer.                     */
/*       NUF_NO_BLOCK                        No block buffer available.   */
/*       NUF_IO_ERROR                        Driver IO error.             */
/*       NUF_INTERNAL                        Nucleus FILE internal error. */
/*                                                                        */
/**************************************************************************/ 
STATUS setup_disk(VOID)
{
volatile STATUS          status;
FMTPARMS    fmt;


    /* If the disk not yet formatted, this call will               */
    /* return an NUF_FORMAT error.  This is normal operation.      */
    /* After the disk is formatted, other calls to NU_Open_Disk    */
    /* will return an NU_SUCCESS                                    */
    status = NU_Open_Disk(HARD_DISK); /* Open the disk for use  */

    /* If disk not formatted, then format */
    if( status == NUF_FORMAT )
    {
	    /*printf("setup_disk: FORMAT NEEDED\n");*/
        /* Get the disk parameters */
        if( pc_get_ataparams( HARD_DRIVE, &fmt) == NO )
		    return( NUF_NO_PARTITION );

        /*printf("setup_disk: pfmt->totalsec=%d\n",fmt.totalsec);*/

        /* Format the disk */
        status = NU_Format(HARD_DRIVE, &fmt); 
     	/*printf("setup_disk: FORMAT returned %d\n",status);*/

    }

    /* Return status */
    return(status);

}
#endif



/****************************************************************************
 * Modifications:
 * $Log: 
 *  4    mpeg      1.3         4/3/04 1:37:28 AM      Nagaraja Kolur  CR(s) 
 *        8762 8763 : Merged 2.5.6 Nup File System.
 *        Test: 1. pvr demo 2. nup file test 3. CR 8181
 *        
 *  3    mpeg      1.2         1/7/04 4:12:12 PM      Tim Ross        CR(s) 
 *        8181 : Correct interpretation of ATA drive parameters to properly 
 *        read the drive's
 *        total sector count.
 *  2    mpeg      1.1         8/22/03 6:48:28 PM     Dave Moore      SCR(s) 
 *        7350 :
 *        Fixed compiler warnings
 *        
 *        
 *  1    mpeg      1.0         8/22/03 5:31:38 PM     Dave Moore      
 * $
 * 
 *    Rev 1.1   22 Aug 2003 17:48:28   mooreda
 * SCR(s) 7350 :
 * Fixed compiler warnings
 * 
 * 
 *    Rev 1.0   22 Aug 2003 16:31:38   mooreda
 * SCR(s) 7350 :
 * Nucleus File sourcecode (Hardware Init Routines)
 * 
 *
 ****************************************************************************/

